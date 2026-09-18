/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "AudioVolumeManager"
#endif

#include "audio_volume_manager.h"
#include "app_volume_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "i_policy_provider.h"
#include "audio_spatialization_service.h"
#include "audio_safe_volume_notification.h"
#include "safe_volume_manager.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "sle_audio_device_manager.h"
#include "audio_mute_factor_manager.h"
#include "audio_active_device.h"
#include "audio_volume_utils.h"
#include "audio_zone_service.h"
#include "audio_connected_device.h"
#include "ipc_skeleton.h"
#include "audio_loud_volume_manager.h"
#include "audio_core_service.h"
#include "audio_interrupt_service.h"
#include "stream_dfx_manager.h"
#include "audio_utils.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
class CheckActiveMusicTimeAction : public AsyncActionHandler::AsyncAction {
public:
    explicit CheckActiveMusicTimeAction(const std::string &reason) : reason_(reason)
    {}

    void Exec() override
    {
        AudioVolumeManager::GetInstance().CheckActiveMusicTime(reason_);
    }

private:
    const std::string reason_;
};

static const int64_t MAX_LEGACY_HIGH_VOLUME_CHECK_TIME_S = 24 * 60 * 60; // 24h
const int32_t DUAL_TONE_RING_VOLUME = 0;
constexpr int32_t DEFAULT_ZONE_ID = 0;
constexpr const char *HIPLAY_DEVICE = "hiplay";
constexpr int32_t RSS_UID = 1096;
constexpr int32_t KARAOKE_DISABLE_MUTE_DURATION_US = 850000;

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

const int32_t ONE_MINUTE = 60;
const uint32_t ABS_VOLUME_SUPPORT_RETRY_INTERVAL_IN_MICROSECONDS = 10000;
const int32_t MUSIC_ACTIVE_PERIOD_MS = 60000;
constexpr int32_t CANCEL_FORCE_CONTROL_VOLUME_TYPE = -1;

static const std::vector<AudioStreamType> AUDIO_STREAMTYPE_VOLUME_LIST = {
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_SYSTEM,
    STREAM_NOTIFICATION,
    STREAM_ALARM,
    STREAM_DTMF,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
#ifdef FEATURE_AUDIO_ZONE_VOLUME
    STREAM_NAVIGATION,
#endif
};

AudioVolumeManager::AudioVolumeManager() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
                                           audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
                                           audioSceneManager_(AudioSceneManager::GetInstance()),
                                           audioActiveDevice_(AudioActiveDevice::GetInstance()),
                                           audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
                                           audioOffloadStream_(AudioOffloadStream::GetInstance())
{
    ringerModeManager_.Init();
    volumeStep_ = system::GetIntParameter("const.multimedia.audio.volumestep", 1);
    AUDIO_INFO_LOG("Get volumeStep parameter success %{public}d", volumeStep_);
}

bool AudioVolumeManager::Init(std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler,
    std::shared_ptr<AudioInterruptService> interruptService)
{
    audioPolicyServerHandler_ = audioPolicyServerHandler;
    interruptService_ = interruptService;
    if (policyVolumeMap_ == nullptr) {
        size_t mapSize = IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume) + sizeof(bool) + sizeof(bool);
        AUDIO_INFO_LOG("InitSharedVolume create shared volume map with size %{public}zu", mapSize);
        policyVolumeMap_ = AudioSharedMemory::CreateFromLocal(mapSize, "PolicyVolumeMap");
        CHECK_AND_RETURN_RET_LOG(policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr,
            false, "Get shared memory failed!");
        volumeVector_ = reinterpret_cast<Volume *>(policyVolumeMap_->GetBase());
        sharedAbsVolumeScene_ = reinterpret_cast<bool *>(policyVolumeMap_->GetBase()) +
            IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume);
        sharedSleAbsVolumeScene_ = reinterpret_cast<bool *>(policyVolumeMap_->GetBase()) +
            IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume) + sizeof(bool);
    }
    if (forceControlVolumeTypeMonitor_ == nullptr) {
        forceControlVolumeTypeMonitor_ = std::make_shared<ForceControlVolumeTypeMonitor>();
    }
    audioPolicyStateMonitor_ = DelayedSingleton<AudioPolicyStateMonitor>::GetInstance();

#ifdef FEATURE_MULTIMODALINPUT_INPUT
    loudVolumeSupportMode_ = system::GetIntParameter("const.audio.loudvolume", 0);
    if (loudVolumeSupportMode_ != LOUD_VOLUME_NOT_SUPPORT) {
        loudVolumeManager_ = std::make_shared<LoudVolumeManager>();
        if (loudVolumeManager_ == nullptr) {
            AUDIO_ERR_LOG("loudVolumeManager_ is nullptr");
            return false;
        }
    }
#endif
    return true;
}
void AudioVolumeManager::DeInit(void)
{
    volumeVector_ = nullptr;
    sharedAbsVolumeScene_ = nullptr;
    sharedSleAbsVolumeScene_ = nullptr;
    policyVolumeMap_ = nullptr;
    safeVolumeExit_ = true;
    forceControlVolumeTypeMonitor_ = nullptr;
    audioPolicyStateMonitor_ = nullptr;
    if (calculateLoopSafeTime_ != nullptr && calculateLoopSafeTime_->joinable()) {
        calculateLoopSafeTime_->join();
        calculateLoopSafeTime_.reset();
        calculateLoopSafeTime_ = nullptr;
    }
    if (safeVolumeDialogThrd_ != nullptr && safeVolumeDialogThrd_->joinable()) {
        safeVolumeDialogThrd_->join();
        safeVolumeDialogThrd_.reset();
        safeVolumeDialogThrd_ = nullptr;
    }
    audioPolicyServerHandler_ = nullptr;
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    loudVolumeSupportMode_ = 0;
    loudVolumeManager_ = nullptr;
#endif
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
std::shared_ptr<LoudVolumeManager> AudioVolumeManager::GetLoudVolumeManager()
{
    return loudVolumeManager_;
}
#endif
void AudioVolumeManager::SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> &handler)
{
    asyncHandler_ = handler;
    appVolumeManager_.SetAsyncActionHandler(handler);
}

int32_t AudioVolumeManager::ClearLoudVolumeHoldMapForCall()
{
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    if (loudVolumeManager_ != nullptr) {
        (void)loudVolumeManager_->ReloadLoudVolumeModeSwitch(LoudVolumeHoldType::LOUD_VOLUME_MODE_VOICE,
            SetLoudVolMode::LOUD_VOLUME_SWITCH_OFF);
    }
    return SUCCESS;
#endif
    return SUCCESS;
}

int32_t AudioVolumeManager::GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = deviceType == DEVICE_TYPE_NONE ?
        audioActiveDevice_.GetDeviceForVolume(volumeType) : audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return audioPolicyManager_.GetMaxVolumeLevel(volumeType, deviceDesc->deviceType_, zoneId);
}

int32_t AudioVolumeManager::GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = deviceType == DEVICE_TYPE_NONE ?
        audioActiveDevice_.GetDeviceForVolume(volumeType) : audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return audioPolicyManager_.GetMinVolumeLevel(volumeType, deviceDesc->deviceType_, zoneId);
}

bool AudioVolumeManager::SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol)
{
    CHECK_AND_RETURN_RET_LOG(volumeVector_ != nullptr, false, "Set shared memory failed!");
    size_t index = 0;
    if (!IPolicyProvider::GetVolumeIndex(streamType, GetVolumeGroupForDevice(deviceType), index) ||
        index >= IPolicyProvider::GetVolumeVectorSize()) {
        AUDIO_INFO_LOG("not find device %{public}d, stream %{public}d", deviceType, streamType);
        return false;
    }
    if (deviceType == DEVICE_TYPE_NEARLINK && streamType == STREAM_VOICE_CALL) {
        vol.volumeFloat = 1.0f;
    }
#ifdef FEATURE_AUDIO_ZONE_VOLUME
    if (AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(DEFAULT_ZONEID, deviceType)) {
        vol.volumeFloat = 1.0f;
    }
#endif
    volumeVector_[index].isMute = vol.isMute;
    volumeVector_[index].volumeFloat = vol.volumeFloat;
    volumeVector_[index].volumeInt = vol.volumeInt;
    AUDIO_INFO_LOG("Success Set Shared Volume with StreamType:%{public}d, DeviceType:%{public}d, \
        volume:%{public}d, mute:%{public}d, volumeFloat:%{public}f",
        streamType, deviceType, vol.volumeInt, vol.isMute, vol.volumeFloat);
    float mdmMuteFactor = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus();
    float volumeActual = vol.volumeFloat * mdmMuteFactor;
    AudioServerProxy::GetInstance().NotifyStreamVolumeChangedProxy(streamType, volumeActual);
    return true;
}

int32_t AudioVolumeManager::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    AUDIO_INFO_LOG("InitSharedVolume start");
    CHECK_AND_RETURN_RET_LOG(policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr,
        ERR_OPERATION_FAILED, "Get shared memory failed!");

    // init volume map
    // todo device
    for (size_t i = 0; i < IPolicyProvider::GetVolumeVectorSize(); i++) {
        auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(g_volumeIndexVector[i].first);
        bool isMute = audioPolicyManager_.GetStreamMute(g_volumeIndexVector[i].first, deviceDesc);
        int32_t currentVolumeLevel = audioPolicyManager_.GetSystemVolumeLevelNoMuteState(
            g_volumeIndexVector[i].first, deviceDesc);
        float volFloat = audioPolicyManager_.GetSystemVolumeInDbByDegree(g_volumeIndexVector[i].first,
            deviceDesc, isMute);
        volumeVector_[i].isMute = isMute;
        volumeVector_[i].volumeFloat = volFloat;
        volumeVector_[i].volumeInt = static_cast<uint32_t>(currentVolumeLevel);
    }
    SetSharedAbsVolumeScene(false);
    SetSharedSleAbsVolumeScene(true);
    buffer = policyVolumeMap_;

    return SUCCESS;
}

void AudioVolumeManager::SetSharedAbsVolumeScene(const bool support)
{
    CHECK_AND_RETURN_LOG(sharedAbsVolumeScene_ != nullptr, "sharedAbsVolumeScene is nullptr");
    *sharedAbsVolumeScene_ = support;
}

void AudioVolumeManager::SetSharedSleAbsVolumeScene(const bool support)
{
    CHECK_AND_RETURN_LOG(sharedSleAbsVolumeScene_ != nullptr, "sharedSleAbsVolumeScene is nullptr");
    *sharedSleAbsVolumeScene_ = support;
}

int32_t AudioVolumeManager::GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    return appVolumeManager_.GetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioVolumeManager::GetSystemVolumeLevel(AudioStreamType streamType, int32_t zoneId)
{
#ifndef FEATURE_AUDIO_ZONE_VOLUME
    if (zoneId > 0) {
        return audioPolicyManager_.GetZoneVolumeLevel(zoneId, streamType);
    }
#endif
    if (streamType == STREAM_RING && !IsRingerModeMute()) {
        AUDIO_PRERELEASE_LOGW("return 0 when dual tone ring");
        return DUAL_TONE_RING_VOLUME;
    }
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(volumeType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc, DUAL_TONE_RING_VOLUME, "deviceDesc is null");
    {
        DeviceType curOutputDeviceType = deviceDesc->deviceType_;
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (volumeType == STREAM_MUSIC &&
            curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            A2dpDeviceConfigInfo info;
            bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
            if (ret && info.absVolumeSupport) {
                return info.mute ? 0 : info.volumeLevel;
            }
        }
    }
    if (deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK &&
        (volumeType == STREAM_MUSIC || volumeType == STREAM_VOICE_CALL)) {
        if (IsNearlinkKaraokeVolumeManaged(volumeType)) {
            return GetNearlinkKaraokeVolume();
        }
        return SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(volumeType, *deviceDesc);
    }
    int32_t volume = audioPolicyManager_.GetSystemVolumeLevel(deviceDesc, streamType, zoneId);
    Trace trace("AudioVolumeManager::GetSystemVolumeLevel device" + std::to_string(deviceDesc->deviceType_) + " stream "
        + std::to_string(streamType) + " volume " + std::to_string(volume));
    return volume;
}

int32_t AudioVolumeManager::GetVolumeForHiplayDevice(AudioStreamType streamType)
{
    auto zoneDescs = AudioZoneService::GetInstance().GetZoneDevices();
    CHECK_AND_RETURN_RET(!zoneDescs.empty() && zoneDescs[0] != nullptr &&
        zoneDescs[0]->model_ == HIPLAY_DEVICE, 0);
    return audioPolicyManager_.GetSystemVolumeLevel(zoneDescs[0], streamType, DEFAULT_ZONE_ID);
}

int32_t AudioVolumeManager::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(streamType);
    return audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType, deviceDesc);
}

int32_t AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch(AudioDeviceDescriptor deviceDescriptor,
    bool enableSetVoiceCallVolume, std::shared_ptr<AudioStreamDescriptor> targetStream)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(deviceDescriptor);
    if (!AudioVolumeUtils::GetInstance().IsDeviceWithSafeVolume(desc)) {
        std::thread cancelSafeNotificationThrd(
            &AudioVolumeManager::CancelSafeVolumeNotificationWhenSwitchDevice, this);
        cancelSafeNotificationThrd.detach();
    }

    Trace trace("AudioVolumeManager::SetVoiceCallVolumeOnDeviceSwitch:" + std::to_string(deviceDescriptor.deviceType_));

    // The volume of voice_call needs to be adjusted separately
    if (enableSetVoiceCallVolume && audioSceneManager_.GetAudioScene(true) == AUDIO_SCENE_PHONE_CALL) {
        SetVoiceCallVolume(GetSystemVolumeLevel(STREAM_VOICE_CALL));
    }
    return SUCCESS;
}

int32_t AudioVolumeManager::SetVoiceRingtoneMute(bool isMute)
{
    isVoiceRingtoneMute_ = isMute ? true : false;
    SetVoiceCallVolume(GetSystemVolumeLevel(STREAM_VOICE_CALL));
    return SUCCESS;
}

void AudioVolumeManager::SetVoiceCallVolume(int32_t volumeLevel)
{
    Trace trace("AudioVolumeManager::SetVoiceCallVolume" + std::to_string(volumeLevel));
    // set voice volume by the interface from hdi.
    CHECK_AND_RETURN_LOG(volumeLevel != 0, "SetVoiceVolume: volume of voice_call cannot be set to 0");
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_VOICE_CALL);
    DeviceType deviceType = deviceDesc != nullptr ? deviceDesc->deviceType_ : DEVICE_TYPE_SPEAKER;
    float volumeDb = static_cast<float>(volumeLevel) /
        static_cast<float>(audioPolicyManager_.GetMaxVolumeLevel(STREAM_VOICE_CALL, deviceType));
    volumeDb = isVoiceRingtoneMute_ ? 0 : volumeDb;
    if (deviceType == DEVICE_TYPE_NEARLINK) {
        volumeDb = 1;
    }
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(volumeDb);
    AUDIO_INFO_LOG("%{public}f", volumeDb);
}

void AudioVolumeManager::InitKVStore()
{
    audioPolicyManager_.InitKVStore();
    AudioSpatializationService::GetAudioSpatializationService().InitSpatializationState();
}

void AudioVolumeManager::CheckToCloseNotification(AudioStreamType streamType, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("enter.");
    int32_t sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    std::shared_ptr<AudioDeviceDescriptor> curOutputDevice = audioActiveDevice_.GetDeviceForVolume();
    if (volumeLevel < sVolumeLevel && safeVolumeManager_.DeviceIsSupportSafeVolume(curOutputDevice) &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC) {
        AUDIO_INFO_LOG("user select lower volume should close notification.");
        if (increaseNIsShowing_) {
            safeVolumeManager_.CancelSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
            increaseNIsShowing_ = false;
        }
        if (restoreNIsShowing_) {
            safeVolumeManager_.CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
            restoreNIsShowing_ = false;
        }
    }
    if (isLegacyHighVolumeNtfShowing_) {
        HandleLegacyHighVolumeNotification(VolumeNotifyActionType::ACTOPN_CANCEL);
    }
}

int32_t AudioVolumeManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return appVolumeManager_.SetAppVolumeLevel(appUid, volumeLevel, deviceDesc->deviceType_);
}

int32_t AudioVolumeManager::SetSelfAppVolumeLevelWithCallback(int32_t appUid, int32_t volumeLevel, bool isUpdateUi)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return appVolumeManager_.SetSelfAppVolumeLevelWithCallback(
        appUid, volumeLevel, isUpdateUi, deviceDesc->deviceType_);
}

int32_t AudioVolumeManager::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    return appVolumeManager_.SetAppVolumeMuted(appUid, muted);
}

int32_t AudioVolumeManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    return appVolumeManager_.IsAppVolumeMute(appUid, owned, isMute);
}

int32_t AudioVolumeManager::SetVoipRendererMute(int32_t callerUid, int32_t appUid, int64_t streamId, bool muted)
{
    return appVolumeManager_.SetVoipRendererMute(callerUid, appUid, streamId, muted);
}

int32_t AudioVolumeManager::SetVoipCapturerMute(int32_t callerUid, int32_t appUid, int64_t streamId, bool muted)
{
    return appVolumeManager_.SetVoipCapturerMute(callerUid, appUid, streamId, muted);
}

int32_t AudioVolumeManager::ClearVoipMuteByCallerUid(int32_t callerUid)
{
    return appVolumeManager_.ClearVoipMuteByCallerUid(callerUid);
}

int32_t AudioVolumeManager::ClearVoipMuteByStreamId(int64_t streamId)
{
    return appVolumeManager_.ClearVoipMuteByStreamId(streamId);
}

int32_t AudioVolumeManager::SetSystemAppVolumePercentage(int32_t appUid, int32_t volumePercentage)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return appVolumeManager_.SetSystemAppVolumePercentage(appUid, volumePercentage, deviceDesc->deviceType_);
}

int32_t AudioVolumeManager::CheckVolumeProtectionConfig(const VolumeProtectionConfig &config) const
{
    const size_t ruleCount = config.rules.size();
    CHECK_AND_RETURN_RET_LOG(ruleCount > 0 && ruleCount <= MAX_VOLUME_PROTECTION_RULES,
        ERR_INVALID_PARAM, "invalid rule count");
    CHECK_AND_RETURN_RET_LOG(config.timeDuration > 0, ERR_INVALID_PARAM, "invalid protection duration");

    for (const auto &entry : config.rules) {
        CHECK_AND_RETURN_RET_LOG(AudioVolumeUtils::GetInstance().IsVolumeTypeValid(
            static_cast<AudioStreamType>(entry.first)),
            ERR_INVALID_PARAM, "invalid volume type");
        CHECK_AND_RETURN_RET_LOG(entry.second >= 0 && entry.second <= MAX_VOLUME_DEGREE,
            ERR_INVALID_PARAM, "invalid volume threshold");
    }
    return SUCCESS;
}

int32_t AudioVolumeManager::SetVolumeProtectionInfo(const VolumeProtectionConfig &config)
{
    int32_t ret = CheckVolumeProtectionConfig(config);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    const int64_t bootTimeNs = ClockTime::GetBootNano();
    CHECK_AND_RETURN_RET_LOG(bootTimeNs >= 0, ERR_OPERATION_FAILED, "get boot time failed");
    std::lock_guard<std::mutex> lock(volumeProtectionMutex_);
    volumeProtectionConfig_ = config;
    volumeLimitExpiryTime_ = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    AUDIO_INFO_LOG("volume protection configured: ruleCount=%{public}zu, duration=%{public}d, "
        "volumeLimitExpiryTime=%{public}" PRId64, config.rules.size(), config.timeDuration,
        volumeLimitExpiryTime_);
    return SUCCESS;
}

int32_t AudioVolumeManager::ClearVolumeProtection()
{
    const int64_t bootTimeNs = ClockTime::GetBootNano();
    CHECK_AND_RETURN_RET_LOG(bootTimeNs >= 0, ERR_OPERATION_FAILED, "get boot time failed");
    std::lock_guard<std::mutex> lock(volumeProtectionMutex_);
    volumeLimitExpiryTime_ = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    return SUCCESS;
}

int32_t AudioVolumeManager::ConfirmVolumeLimitExceeded(AudioVolumeType volumeType, bool confirmResult)
{
    CHECK_AND_RETURN_RET_LOG(AudioVolumeUtils::GetInstance().IsVolumeTypeValid(
        static_cast<AudioStreamType>(volumeType)), ERR_INVALID_PARAM, "invalid volume type");
    CHECK_AND_RETURN_RET(confirmResult, SUCCESS);

    const int64_t bootTimeNs = ClockTime::GetBootNano();
    CHECK_AND_RETURN_RET_LOG(bootTimeNs >= 0, ERR_OPERATION_FAILED, "get boot time failed");
    int32_t volumeThreshold = 0;
    {
        std::lock_guard<std::mutex> lock(volumeProtectionMutex_);
        const auto rule = volumeProtectionConfig_.rules.find(volumeType);
        CHECK_AND_RETURN_RET_LOG(volumeProtectionConfig_.timeDuration > 0 &&
            rule != volumeProtectionConfig_.rules.end(),
            ERR_OPERATION_FAILED, "volume protection is not configured for the volume type");
        volumeThreshold = rule->second;
        volumeLimitExpiryTime_ = bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND) +
            volumeProtectionConfig_.timeDuration;
        AUDIO_INFO_LOG("volumeType=%{public}d, volumeThreshold=%{public}d, volumeLimitExpiryTime=%{public}" PRId64,
            volumeType, volumeThreshold, volumeLimitExpiryTime_);
    }
    return SetVolumeToProtectionThreshold(volumeType, volumeThreshold);
}

int32_t AudioVolumeManager::SetVolumeToProtectionThreshold(AudioVolumeType volumeType, int32_t volumeThreshold)
{
    const int32_t maxVolumeLevel = GetMaxVolumeLevel(volumeType);
    CHECK_AND_RETURN_RET_LOG(maxVolumeLevel > 0, ERR_OPERATION_FAILED, "invalid maximum volume level");
    const int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(volumeThreshold, maxVolumeLevel);
    VolumeUpdateOption option { false, false, DEFAULT_ZONE_ID, volumeThreshold };
    return SetSingleStreamVolume(static_cast<AudioStreamType>(volumeType), volumeLevel, option);
}

int32_t AudioVolumeManager::GetSystemAppVolumePercentage(int32_t appUid, int32_t &volumePercentage)
{
    return appVolumeManager_.GetSystemAppVolumePercentage(appUid, volumePercentage);
}

int32_t AudioVolumeManager::IsSystemAppVolumeMuted(int32_t appUid, bool &isMute)
{
    return appVolumeManager_.IsSystemAppVolumeMuted(appUid, isMute);
}

int32_t AudioVolumeManager::SetSystemAppVolumeMutedForUid(int32_t appUid, bool muted)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return appVolumeManager_.SetSystemAppVolumeMutedForUid(appUid, muted, deviceDesc->deviceType_);
}

int32_t AudioVolumeManager::SetSystemAppVolumePercentageWithCallback(int32_t appUid, int32_t volumePercentage)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return appVolumeManager_.SetSystemAppVolumePercentageWithCallback(
        appUid, volumePercentage, deviceDesc->deviceType_);
}

int32_t AudioVolumeManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    return appVolumeManager_.SetAppRingMuted(appUid, muted);
}

bool AudioVolumeManager::IsAppRingMuted(int32_t appUid)
{
    return appVolumeManager_.IsAppRingMuted(appUid);
}

int32_t AudioVolumeManager::GetVolumeAdjustZoneId()
{
    return audioPolicyManager_.GetVolumeAdjustZoneId();
}

int32_t AudioVolumeManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    audioActiveDevice_.SetAdjustVolumeForZone(zoneId);
    return audioPolicyManager_.SetAdjustVolumeForZone(zoneId);
}

int32_t AudioVolumeManager::HandleA2dpAbsVolume(AudioStreamType streamType, const VolumeScale &volume,
    DeviceType curOutputDeviceType)
{
    int32_t volumeLevel = volume.VolumeLevel();
    std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    VolumeScale buffedVolume = volume;
    int32_t result = SetA2dpDeviceVolume(btDevice, buffedVolume, true);

    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.CalculateVolumeDbByDegree(curOutputDeviceType,
        streamType, buffedVolume.VolumeDegree());
    SetSharedVolume(streamType, curOutputDeviceType, vol);
#ifdef BLUETOOTH_ENABLE
    if (result == SUCCESS) {
        // set to avrcp device
        return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice, volumeLevel);
    } else if (result == ERR_UNKNOWN) {
        AUDIO_INFO_LOG("UNKNOWN RESULT set abs safe volume");
        return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice,
            audioPolicyManager_.GetSafeVolumeLevel());
    } else {
        AUDIO_ERR_LOG("AudioVolumeManager::SetSystemVolumeLevel set abs volume failed");
    }
    return result;
#else
    return SUCCESS;
#endif
}

int32_t AudioVolumeManager::HandleNearlinkDeviceAbsVolume(AudioStreamType streamType, const VolumeScale &volume,
    DeviceType curOutputDeviceType)
{
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "desc is nullptr");
    std::string nearlinkDevice = desc->macAddress_;
    if (nearlinkDevice.empty()) {
        AUDIO_ERR_LOG("nearlink device is empty");
        return ERR_UNKNOWN;
    }
    int32_t volumeLevel = volume.VolumeLevel();
    VolumeScale buffedVolume = volume;
    int32_t result = SetNearlinkDeviceVolume(nearlinkDevice, streamType, buffedVolume, true);

    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.CalculateVolumeDbByDegree(curOutputDeviceType,
        streamType, buffedVolume.VolumeDegree());
    SetSharedVolume(streamType, curOutputDeviceType, vol);

    if (result == SUCCESS) {
        auto volumeValue = SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(streamType,
            AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice());
        return SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(nearlinkDevice, streamType, volumeValue);
    } else if (result == ERR_UNKNOWN) {
        AUDIO_INFO_LOG("UNKNOWN RESULT set abs safe volume");
        return SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(nearlinkDevice, streamType,
            audioPolicyManager_.GetSafeVolumeLevel());
    }
    return result;
}

bool AudioVolumeManager::IsNearlinkKaraokeVolumeManaged(AudioStreamType streamType)
{
    if (streamType != STREAM_MUSIC || audioActiveDevice_.GetCurrentOutputDeviceType() != DEVICE_TYPE_NEARLINK) {
        return false;
    }
    return GetNearlinkKaraokeMode() == NearlinkKaraokeVolumeMode::VOLUME_FWK;
}

int32_t AudioVolumeManager::GetCurrentNearlinkAbsVolume()
{
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(STREAM_MUSIC);
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(volumeType);
    CHECK_AND_RETURN_RET(deviceDesc != nullptr, ERROR);
    CHECK_AND_RETURN_RET(deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK, ERROR);
    int32_t volume = SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(STREAM_MUSIC, *deviceDesc);
    return volume;
}

NearlinkKaraokeVolumeMode AudioVolumeManager::GetNearlinkKaraokeMode()
{
    std::lock_guard<std::mutex> lock(nearlinkKaraokeMutex_);
    return nearlinkKaraokeMode_;
}

int32_t AudioVolumeManager::GetNearlinkKaraokeVolume()
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    CHECK_AND_RETURN_RET(deviceDesc != nullptr, 0);
    return audioPolicyManager_.GetSystemVolumeLevel(deviceDesc, STREAM_MUSIC, 0);
}

int32_t AudioVolumeManager::SetNearlinkKaraokeVolumeMode(bool enable)
{
    Trace trace("AudioVolumeManager::SetNearlinkKaraokeVolumeMode");
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (curOutputDeviceType != DEVICE_TYPE_NEARLINK) {
        CHECK_AND_RETURN_RET_LOG(!enable, ERROR, "KARAOKE: nearlink karaoke volume mode only supports nearlink output");
        std::lock_guard<std::mutex> lock(nearlinkKaraokeMutex_);
        nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_NEARLINK;
        AUDIO_INFO_LOG("KARAOKE: reset karaoke mode, no nearlink device active");
        return ERROR;
    }
    return enable ? EnableNearlinkKaraokeVolume() : DisableNearlinkKaraokeVolume();
}

int32_t AudioVolumeManager::RefreshKaraokePipeVolume()
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERROR, "KARAOKE: get device for volume failed");
    return audioPolicyManager_.SetVolumeDbForDeviceInPipe(deviceDesc, STREAM_MUSIC);
}

int32_t AudioVolumeManager::EnableNearlinkKaraokeVolume()
{
    {
        std::lock_guard<std::mutex> lock(nearlinkKaraokeMutex_);
        if (nearlinkKaraokeMode_ == NearlinkKaraokeVolumeMode::VOLUME_FWK) {
            return SUCCESS;
        }
        nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_FWK;
    }
    int32_t ret = RefreshKaraokePipeVolume();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AUDIO_INFO_LOG("KARAOKE: enable karaoke volume mode");
    return SUCCESS;
}

int32_t AudioVolumeManager::DisableNearlinkKaraokeVolume()
{
    auto &ioHandleMap = AudioIOHandleMap::GetInstance();
    ioHandleMap.MuteSinkPort(PRIMARY_SPEAKER, KARAOKE_DISABLE_MUTE_DURATION_US, true, false);
    ioHandleMap.MuteSinkPort(OFFLOAD_PRIMARY_SPEAKER, KARAOKE_DISABLE_MUTE_DURATION_US, true, false);
    {
        std::lock_guard<std::mutex> lock(nearlinkKaraokeMutex_);
        if (nearlinkKaraokeMode_ == NearlinkKaraokeVolumeMode::VOLUME_NEARLINK) {
            return SUCCESS;
        }
        nearlinkKaraokeMode_ = NearlinkKaraokeVolumeMode::VOLUME_NEARLINK;
    }
    int32_t ret = RefreshKaraokePipeVolume();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AUDIO_INFO_LOG("KARAOKE: disable karaoke volume mode");
    return SUCCESS;
}

int32_t AudioVolumeManager::SetSystemVolumeLevel(AudioStreamType streamType, const VolumeScale &volume,
    std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc, int32_t zoneId)
{
    VolumeScale buffedVolume = {volume.VolumeLevel(), volume.VolumeDegree(), GetMaxVolumeLevel(streamType)};
    int32_t ret = ERROR;
    do {
        if (zoneId > 0) {
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = AudioZoneService::GetInstance().
                FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
            CHECK_AND_BREAK_LOG(devices.size() >= 1, "zone device error");
            volDeviceDesc = devices[0];
            ret = audioPolicyManager_.SetZoneVolumeLevel(zoneId,
                VolumeUtils::GetVolumeTypeFromStreamType(streamType), buffedVolume, volDeviceDesc);
            break;
        }
        volDeviceDesc = audioActiveDevice_.GetDeviceForVolume(streamType);
        DeviceType deviceType = volDeviceDesc != nullptr ? volDeviceDesc->deviceType_ : DEVICE_TYPE_SPEAKER;
        ret = SetSystemVolumeExternal(streamType, buffedVolume, deviceType);
        if (ret != SUCCESS) {
            ret = SetSystemVolumeLevelInternal(streamType, buffedVolume, volDeviceDesc, zoneId);
        }
    } while (0);
    CheckReduceOtherActiveVolume(streamType, volume.VolumeLevel());
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_FAILED, "Set system volume level failed", false));
    return ret;
}

int32_t AudioVolumeManager::SetSystemVolumeExternal(AudioStreamType streamType,
    const VolumeScale &volume, DeviceType deviceType)
{
    int32_t result = ERROR;
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_MUSIC &&
        streamType != STREAM_VOICE_CALL &&
        deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        result = HandleA2dpAbsVolume(streamType, volume, deviceType);
    }

    if (deviceType == DEVICE_TYPE_NEARLINK &&
        (volumeType == STREAM_MUSIC || volumeType == STREAM_VOICE_CALL)) {
        result = HandleNearlinkDeviceAbsVolume(volumeType, volume, deviceType);
    }
    return result;
}

int32_t AudioVolumeManager::SetSystemVolumeLevelInternal(AudioStreamType streamType, const VolumeScale &volume,
    std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc, int32_t zoneId)
{
    VolumeScale buffedVolume = volume;
    int32_t volumeLevel = buffedVolume.VolumeLevel();
    int32_t result = ERROR;
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    DeviceType deviceType = volDeviceDesc != nullptr ? volDeviceDesc->deviceType_ : DEVICE_TYPE_SPEAKER;
    buffedVolume = SelectDealSafeVolume(streamType, buffedVolume);

    audioPolicyManager_.SaveSystemVolumeForEffect(deviceType, volumeType, buffedVolume.VolumeLevel());
    CheckToCloseNotification(streamType, volumeLevel);
    volumeLevel = buffedVolume.VolumeLevel(); // the volume level after safe volume judgement.
    buffedVolume.Refresh(); // Ensure that the volume degree is correct.

    result = audioPolicyManager_.SetSystemVolumeLevel(volumeType, buffedVolume, volDeviceDesc, zoneId);
    if (result == SUCCESS && (streamType == STREAM_VOICE_CALL || streamType == STREAM_VOICE_COMMUNICATION)) {
        SetVoiceCallVolume(volumeLevel);
    }

    // todo
    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDbByDegree(streamType, volDeviceDesc, false);
    SetSharedVolume(streamType, deviceType, vol);
    return result;
}

int32_t AudioVolumeManager::SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    return audioPolicyManager_.SaveSpecifiedDeviceVolume(
        VolumeUtils::GetVolumeTypeFromStreamType(streamType), volumeLevel, deviceType);
}

VolumeScale AudioVolumeManager::SelectDealSafeVolume(AudioStreamType streamType, VolumeScale volume,
    DeviceType deviceType)
{
    VolumeScale safeVolume = volume;
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) != STREAM_MUSIC) {
        // Safe Volume only applying to STREAM_MUSIC
        return safeVolume;
    }
    auto targetDevice = audioActiveDevice_.GetDeviceForVolume(streamType);
    DeviceType curOutputDeviceType = (deviceType == DEVICE_TYPE_NONE) ?
        targetDevice->deviceType_ : deviceType;
    DeviceCategory curOutputDeviceCategory = targetDevice->deviceCategory_;
    if (safeVolume.VolumeLevel() > audioPolicyManager_.GetSafeVolumeLevel() ||
        safeVolume.VolumeDegree() > audioPolicyManager_.GetSafeVolumeDegree()) {
        // Determine whether to use the safe volume level and reset volume degree.
        switch (curOutputDeviceType) {
            case DEVICE_TYPE_BLUETOOTH_A2DP:
            case DEVICE_TYPE_BLUETOOTH_SCO:
                if (curOutputDeviceCategory == BT_SOUNDBOX || curOutputDeviceCategory == BT_CAR) {
                    return safeVolume;
                }
                if (isBtFirstBoot_) {
                    // Need to use safe volume level and reset volume degree.
                    safeVolume.SetVolumeLevelAndResetDegree(audioPolicyManager_.GetSafeVolumeLevel());
                    AUDIO_INFO_LOG("Btfirstboot set volume use safe volume");
                } else {
                    safeVolume = DealWithSafeVolume(safeVolume, true);
                }
                break;
            case DEVICE_TYPE_WIRED_HEADSET:
            case DEVICE_TYPE_WIRED_HEADPHONES:
            case DEVICE_TYPE_USB_HEADSET:
            case DEVICE_TYPE_USB_ARM_HEADSET:
                safeVolume = DealWithSafeVolume(safeVolume, false);
                break;
            case DEVICE_TYPE_NEARLINK:
                safeVolume = DealWithSafeVolume(safeVolume, false);
                break;
            default:
                AUDIO_INFO_LOG("unsupport safe volume:%{public}d", curOutputDeviceType);
                break;
        }
    }
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP || curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        isBtFirstBoot_ = false;
    }
    if (curOutputDeviceType == DEVICE_TYPE_NEARLINK) {
        isSleFirstBoot_ = false;
    }
    return safeVolume;
}

int32_t AudioVolumeManager::SetA2dpDeviceVolume(const std::string &macAddress,
    VolumeScale &volume, bool internalCall)
{
    volume.SetMaxVolumeLevel(GetMaxVolumeLevel(STREAM_MUSIC));
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        volume.IsVolumeDegreeValid(GetMinVolumeDegree(STREAM_MUSIC), MAX_VOLUME_DEGREE),
        ERR_INVALID_PARAM, StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_INVALID_PARAM, "Invaild volume parameter in SetA2dpDevieVolume", false),
        "invalid input, volumeDegree:[%{public}d]", volume.VolumeDegree());

    if (audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume) == false) {
        return ERROR;
    }
    int32_t volumeLevel = volume.VolumeLevel(); // Record the initial volume value.
    if (volume.VolumeLevel() > audioPolicyManager_.GetSafeVolumeLevel() ||
        volume.VolumeDegree() > audioPolicyManager_.GetSafeVolumeDegree()) {
        if (internalCall) {
            volume = DealWithSafeVolume(volume, true);
        } else {
            volume = HandleAbsBluetoothVolume(macAddress, volume);
        }
    }
    isBtFirstBoot_ = false;
    volume.Refresh(); // Ensure that the volume degree is correct.
    if (audioA2dpDevice_.SetA2dpDeviceVolume(macAddress, volume) == false) {
        return ERROR;
    }
    int32_t sVolumeLevel = volume.VolumeLevel(); // the volume level after safe volume judgement.
    bool mute = sVolumeLevel == 0 ? true : false;

    if (internalCall) {
        CheckToCloseNotification(STREAM_MUSIC, volumeLevel);
    }

    audioA2dpDevice_.SetA2dpDeviceMute(macAddress, mute);
    audioPolicyManager_.SetAbsVolumeMute(mute);
    AUDIO_INFO_LOG("success for macaddress:[%{public}s], volumeLevel:[%{public}d], volumeDegree:[%{public}d]",
        GetEncryptAddr(macAddress).c_str(), volume.VolumeLevel(), volume.VolumeDegree());
    HILOG_COMM_INFO("[SetA2dpDeviceVolume]SetA2dpAbsVolume streamType: STREAM_MUSIC, volumeLevel: %{public}d",
        sVolumeLevel);
    float volumeDbTemp = audioPolicyManager_.CalculateVolumeDbNonlinear(STREAM_MUSIC, DEVICE_TYPE_BLUETOOTH_A2DP,
        sVolumeLevel);
    audioPolicyManager_.SaveSystemVolumeForEffect(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_MUSIC, sVolumeLevel);
    if (Util::IsEnhancedRoutingSupported()) {
        audioPolicyManager_.SetSystemVolumeToEffectWithDeviceType(STREAM_MUSIC, volumeDbTemp,
            DEVICE_TYPE_BLUETOOTH_A2DP);
    } else {
        audioPolicyManager_.SetSystemVolumeToEffect(STREAM_MUSIC, volumeDbTemp);
    }
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_UNKNOWN, "safevolume did not deal");
    return SUCCESS;
}

VolumeScale AudioVolumeManager::HandleAbsBluetoothVolume(const std::string &macAddress, const VolumeScale &volume,
    bool isNearlinkDevice, AudioStreamType streamType)
{
    VolumeScale safeVolume = volume;
    int32_t sVolumeLevel = 0;
    if (isBtFirstBoot_) {
        // Need to use safe volume level and reset volume degree.
        safeVolume.SetVolumeLevelAndResetDegree(audioPolicyManager_.GetSafeVolumeLevel());
        sVolumeLevel = safeVolume.VolumeLevel();
        AUDIO_INFO_LOG("Btfirstboot set volume use safe volume");
        isBtFirstBoot_ = false;
        if (!isNearlinkDevice) {
            Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
        } else {
            SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(macAddress, streamType, sVolumeLevel);
        }
    } else {
        safeVolume = DealWithSafeVolume(safeVolume, true);
        if (safeVolume.VolumeLevel() != volume.VolumeLevel()) {
            sVolumeLevel = safeVolume.VolumeLevel();
            if (!isNearlinkDevice) {
                Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
            } else {
                SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(macAddress, streamType, sVolumeLevel);
            }
        }
    }
    return safeVolume;
}

int32_t AudioVolumeManager::SetNearlinkDeviceVolume(const std::string &macAddress, AudioStreamType streamType,
    VolumeScale &volume, bool internalCall)
{
    volume.SetMaxVolumeLevel(GetMaxVolumeLevel(streamType));
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        volume.IsVolumeDegreeValid(GetMinVolumeDegree(streamType), MAX_VOLUME_DEGREE),
        ERR_INVALID_PARAM, StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_INVALID_PARAM, "Invaild volume parameter in SetNearlinkDeviceVolume", false),
        "invalid input, volumeDegree:[%{public}d]", volume.VolumeDegree());

    int32_t volumeLevel = volume.VolumeLevel(); // Record the initial volume value.
    int32_t ret = SleAudioDeviceManager::GetInstance().SetNearlinkDeviceVolume(macAddress, streamType, volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetNearlinkDeviceVolumeLevel failed");

    // Voice call does not support safe volume
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_MUSIC) {
        float volumeDbTemp = audioPolicyManager_.CalculateVolumeDbNonlinear(STREAM_MUSIC,
            DEVICE_TYPE_NEARLINK, volume.VolumeLevel());
        audioPolicyManager_.SaveSystemVolumeForEffect(DEVICE_TYPE_NEARLINK, STREAM_MUSIC, volume.VolumeLevel());
        if (Util::IsEnhancedRoutingSupported()) {
            audioPolicyManager_.SetSystemVolumeToEffectWithDeviceType(STREAM_MUSIC, volumeDbTemp,
                DEVICE_TYPE_NEARLINK);
        } else {
            audioPolicyManager_.SetSystemVolumeToEffect(STREAM_MUSIC, volumeDbTemp);
        }
        if (volume.VolumeLevel() > audioPolicyManager_.GetSafeVolumeLevel() ||
            volume.VolumeDegree() > audioPolicyManager_.GetSafeVolumeDegree()) {
            if (internalCall) {
                volume = DealWithSafeVolume(volume, true);
            } else {
                volume = HandleAbsBluetoothVolume(macAddress, volume, true, streamType);
            }
        }
        isBtFirstBoot_ = false;
    }
    volume.Refresh(); // Ensure that the volume degree is correct.
    ret = SleAudioDeviceManager::GetInstance().SetNearlinkDeviceVolume(macAddress, streamType, volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetDeviceAbsVolume failed");
    int32_t sVolumeLevel = volume.VolumeLevel(); // the volume level after safe volume judgement.
    ret = SetNearlinkDeviceVolumeEx(streamType, sVolumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetSystemVolumeLevel failed");

    bool mute = sVolumeLevel == 0 && (volumeType == STREAM_MUSIC);

    if (internalCall) {
        CheckToCloseNotification(streamType, volumeLevel);
    }

    SleAudioDeviceManager::GetInstance().SetNearlinkDeviceMute(macAddress, streamType, mute);
    audioPolicyManager_.SetAbsVolumeMuteNearlink(mute);
    HILOG_COMM_INFO("[SetNearlinkDeviceVolume]success for macaddress:[%{public}s], volume value:[%{public}d], "
        "streamType [%{public}d]", GetEncryptAddr(macAddress).c_str(), sVolumeLevel, streamType);
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_UNKNOWN, "safevolume did not deal");
    return SUCCESS;
}

int32_t AudioVolumeManager::SetNearlinkDeviceVolumeEx(AudioVolumeType streamType, int32_t volumeLevel)
{
    return audioPolicyManager_.SetNearlinkDeviceVolume(streamType, volumeLevel);
}

void AudioVolumeManager::HandleLegacyHighVolumeEvent()
{
    HandleLegacyHighVolumeNotification(VolumeNotifyActionType::ACTOPN_CANCEL);
    int64_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    audioPolicyManager_.SetLegacyHighVolumeCheckTime(currentTime);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    int32_t ret = SetSystemVolumeLevel(legacyHighVolumeStreamType_, legacyHighVolumeLevel_, deviceDesc);
    AUDIO_INFO_LOG("handle AUDIO_LEGACY_HIGH_VOLUME_EVENT result: %{public}d", ret);
    SetSafeVolumeCallback(legacyHighVolumeStreamType_, false);
}

bool AudioVolumeManager::IsLegacyHighVolumeChecked()
{
    int64_t checkedTime = audioPolicyManager_.GetLegacyHighVolumeCheckTime();
    if (checkedTime <= 0) {
        AUDIO_INFO_LOG("legacy high volume notification not checked");
        return false;
    }
    int64_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return currentTime > checkedTime && currentTime - checkedTime < MAX_LEGACY_HIGH_VOLUME_CHECK_TIME_S;
}

void AudioVolumeManager::HandleLegacyHighVolumeNotification(VolumeNotifyActionType actionType,
    AudioStreamType streamType, int32_t volumeLevel)
{
    if (actionType == ACTION_PUBLISH) {
        legacyHighVolumeStreamType_ = streamType;
        legacyHighVolumeLevel_ = volumeLevel;
    }
    AUDIO_INFO_LOG("legacy high volume notification type: %{public}d, streamType: %{public}d, volumeLevel: %{public}d"
        "isShowing: %{public}d", actionType, streamType, volumeLevel, isLegacyHighVolumeNtfShowing_.load());
    if (actionType == ACTION_PUBLISH) {
        safeVolumeManager_.PublishLegacyHighVolumeNotification();
        isLegacyHighVolumeNtfShowing_ = true;
    } else {
        safeVolumeManager_.CancelLegacyHighVolumeNotification();
        isLegacyHighVolumeNtfShowing_ = false;
    }
}

void AudioVolumeManager::CancelSafeVolumeNotificationWhenSwitchDevice()
{
    if (increaseNIsShowing_) {
        safeVolumeManager_.CancelSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
        increaseNIsShowing_ = false;
    }

    if (restoreNIsShowing_) {
        safeVolumeManager_.CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = false;
    }
}

VolumeScale AudioVolumeManager::DealWithSafeVolume(const VolumeScale &volume, bool isBtDevice)
{
    auto curDeviceDesc = audioActiveDevice_.GetDeviceForVolume();
    CHECK_AND_RETURN_RET_LOG(curDeviceDesc != nullptr, volume, "curDeviceDesc is nullptr");
    DeviceType curOutputDeviceType = curDeviceDesc->deviceType_;
    DeviceCategory curOutputDeviceCategory = curDeviceDesc->deviceCategory_;
    bool isDealSafeVolumeForCurDevice = curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP ||
        curOutputDeviceType == DEVICE_TYPE_NEARLINK ? true : false;
    if (isBtDevice && isDealSafeVolumeForCurDevice) {
        AUDIO_INFO_LOG("bluetooth Category:%{public}d", curOutputDeviceCategory);
        if (curOutputDeviceCategory == BT_SOUNDBOX || curOutputDeviceCategory == BT_CAR) {
            return volume;
        }
    }

    safeStatusBt_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP);
    safeStatus_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET);
    safeStatusSle_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_NEARLINK);
    if ((safeStatusBt_ == SAFE_INACTIVE && isBtDevice) ||
        (safeStatus_ == SAFE_INACTIVE && !isBtDevice) ||
        (safeStatusSle_ == SAFE_INACTIVE && !isBtDevice)) {
        CreateCheckMusicActiveThread();
        return volume;
    }

    VolumeScale safeVolume = volume;
    if ((isBtDevice && safeStatusBt_ == SAFE_ACTIVE) ||
        (!isBtDevice && safeStatus_ == SAFE_ACTIVE) ||
        (!isBtDevice && safeStatusSle_ == SAFE_ACTIVE)) {
        // Need to use safe volume level and reset volume degree.
        safeVolume.SetVolumeLevelAndResetDegree(audioPolicyManager_.GetSafeVolumeLevel());
        if (restoreNIsShowing_) {
            safeVolumeManager_.CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
            restoreNIsShowing_ = false;
        }
        safeVolumeManager_.PublishSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
        increaseNIsShowing_ = true;
        return safeVolume;
    }
    return safeVolume;
}

void AudioVolumeManager::CreateCheckMusicActiveThread()
{
    if (calculateLoopSafeTime_ == nullptr) {
        calculateLoopSafeTime_ = std::make_unique<std::thread>([this] { this->CheckActiveMusicTime(); });
        pthread_setname_np(calculateLoopSafeTime_->native_handle(), "OS_AudioPolicySafe");
    }
}

void AudioVolumeManager::SetRestoreVolumeLevel(DeviceType deviceType, int32_t curDeviceVolume)
{
    int32_t btDeviceVol = audioPolicyManager_.GetDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_MUSIC);
    int32_t wiredDeviceVol = audioPolicyManager_.GetDeviceVolume(DEVICE_TYPE_WIRED_HEADSET, STREAM_MUSIC);
    int32_t sleDeviceVol = audioPolicyManager_.GetDeviceVolume(DEVICE_TYPE_NEARLINK, STREAM_MUSIC);
    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();

    btRestoreVol_ = btDeviceVol > safeVolume ? btDeviceVol : btRestoreVol_;
    audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, btRestoreVol_);
    wiredRestoreVol_ = wiredDeviceVol > safeVolume ? wiredDeviceVol : wiredRestoreVol_;
    audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, wiredRestoreVol_);
    sleRestoreVol_ = sleDeviceVol > safeVolume ? sleDeviceVol : sleRestoreVol_;
    audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, sleRestoreVol_);

    AUDIO_INFO_LOG("btDeviceVol : %{public}d, wiredDeviceVol : %{public}d, sleDeviceVol : %{public}d, "\
        "curDeviceVolume : %{public}d", btDeviceVol, wiredDeviceVol, sleDeviceVol, curDeviceVolume);

    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("set bt restore volume to db");
        btRestoreVol_ = curDeviceVolume > safeVolume ? curDeviceVolume : btRestoreVol_;
        audioPolicyManager_.SetRestoreVolumeLevel(deviceType, btRestoreVol_);
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        AUDIO_INFO_LOG("set wired restore volume to db");
        wiredRestoreVol_ = curDeviceVolume > safeVolume ? curDeviceVolume : wiredRestoreVol_;
        audioPolicyManager_.SetRestoreVolumeLevel(deviceType, wiredRestoreVol_);
    } else if (deviceType == DEVICE_TYPE_NEARLINK) {
        AUDIO_INFO_LOG("set sle restore volume to db");
        sleRestoreVol_ = curDeviceVolume > safeVolume ? curDeviceVolume : sleRestoreVol_;
        audioPolicyManager_.SetRestoreVolumeLevel(deviceType, sleRestoreVol_);
    }
}

void AudioVolumeManager::OnCheckActiveMusicTime(const std::string &reason)
{
    AUDIO_INFO_LOG("reason:%{public}s", reason.c_str());
    std::shared_ptr<CheckActiveMusicTimeAction> action =
        std::make_shared<CheckActiveMusicTimeAction>(reason);
    CHECK_AND_RETURN_LOG(action != nullptr, "action is nullptr");
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    desc.delayTimeMs = 0;
    if (asyncHandler_ != nullptr) {
        asyncHandler_->PostAsyncAction(desc);
    }
}

void AudioVolumeManager::DealWithPauseAndStop(const std::string &reason)
{
    if (std::string("Paused") == reason || std::string("Stopped") == reason) {
        startSafeTime_ = 0;
        startSafeTimeBt_ = 0;
        startSafeTimeSle_ = 0;
    }
}

std::string AudioVolumeManager::DoLoopCheck(const std::string &reason)
{
    std::string innerReason = "";
    if (std::string("Default") == reason) {
        std::shared_ptr<CheckActiveMusicTimeAction> action =
            std::make_shared<CheckActiveMusicTimeAction>(reason);
        CHECK_AND_RETURN_RET_LOG(action != nullptr, "", "action is nullptr");
        AsyncActionHandler::AsyncActionDesc desc;
        desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
        desc.delayTimeMs = MUSIC_ACTIVE_PERIOD_MS;
        if (asyncHandler_ != nullptr) {
            asyncHandler_->PostAsyncAction(desc);
        }
        innerReason = "Default";
    } else {
        innerReason = "Offload";
    }
    return innerReason;
}

int32_t AudioVolumeManager::CheckActiveMusicTime(const std::string &reason)
{
    std::lock_guard<std::mutex> lock(checkMusicActiveThreadMutex_);
    AUDIO_INFO_LOG("enter");
    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    int32_t safeVolumeDegree = audioPolicyManager_.GetSafeVolumeDegree();
    std::string innerReason = "";
    if (!safeVolumeExit_) {
        innerReason = DoLoopCheck(reason);
        bool activeMusic = audioSceneManager_.IsStreamActive(STREAM_MUSIC);
        int32_t curDeviceVolume = GetSystemVolumeLevel(STREAM_MUSIC);
        int32_t curDeviceVolumeDegree = GetSystemVolumeDegree(STREAM_MUSIC);
        bool isUpSafeVolume = (curDeviceVolume > safeVolume || curDeviceVolumeDegree > safeVolumeDegree) ? true : false;
        auto deviceDesc = audioActiveDevice_.GetDeviceForVolume();
        CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
        DeviceType deviceType = deviceDesc->deviceType_;
        AUDIO_INFO_LOG("activeMusic:%{public}d, deviceType_:%{public}d, isUpSafeVolume:%{public}d " \
            "reason:%{public}s", activeMusic, deviceType, isUpSafeVolume, innerReason.c_str());
        if ((activeMusic || std::string("Offload") == innerReason) && (safeStatusBt_ == SAFE_INACTIVE) &&
            isUpSafeVolume && safeVolumeManager_.IsBlueTooth(deviceDesc)) {
            SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, curDeviceVolume);
            CheckBlueToothActiveMusicTime(safeVolume);
        } else if ((activeMusic || std::string("Offload") == innerReason) && (safeStatus_ == SAFE_INACTIVE) &&
            isUpSafeVolume && safeVolumeManager_.IsWiredHeadSet(deviceType)) {
            SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, curDeviceVolume);
            CheckWiredActiveMusicTime(safeVolume);
        } else if ((activeMusic || std::string("Offload") == innerReason) && (safeStatusSle_ == SAFE_INACTIVE) &&
            isUpSafeVolume && safeVolumeManager_.IsNearLink(deviceType)) {
            SetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, curDeviceVolume);
            CheckNearlinkActiveMusicTime(safeVolume);
        } else {
            startSafeTime_ = 0;
            startSafeTimeBt_ = 0;
            startSafeTimeSle_ = 0;
        }
        DealWithPauseAndStop(reason);
    }
    return SUCCESS;
}

bool AudioVolumeManager::CheckMixActiveMusicTime(int32_t safeVolume)
{
    int64_t mixSafeTime = activeSafeTimeBt_ + activeSafeTime_ + activeSafeTimeSle_;
    AUDIO_INFO_LOG("mix device cumulative time: %{public}" PRId64, mixSafeTime);
    if (mixSafeTime >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("mix device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTimeBt_ = 0;
        startSafeTime_ = 0;
        startSafeTimeSle_ = 0;
        activeSafeTimeBt_ = 0;
        activeSafeTime_ = 0;
        activeSafeTimeSle_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, 0);
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, 0);
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_NEARLINK, 0);
        return true;
    }
    return false;
}

void AudioVolumeManager::CheckBlueToothActiveMusicTime(int32_t safeVolume)
{
    if (startSafeTimeBt_ == 0) {
        startSafeTimeBt_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (activeSafeTimeBt_ >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("bluetooth device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTimeBt_ = 0;
        activeSafeTimeBt_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, 0);
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (CheckMixActiveMusicTime(safeVolume)) {
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else {
        activeSafeTimeBt_ = audioPolicyManager_.GetCurentDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP);
        activeSafeTimeBt_ += currentTime - startSafeTimeBt_;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, activeSafeTimeBt_);
        startSafeTimeBt_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        AUDIO_INFO_LOG("bluetooth safe volume timeout, cumulative time: %{public}" PRId64, activeSafeTimeBt_);
    }
    startSafeTime_ = 0;
    startSafeTimeSle_ = 0;
}

void AudioVolumeManager::CheckNearlinkActiveMusicTime(int32_t safeVolume)
{
    if (startSafeTimeSle_ == 0) {
        startSafeTimeSle_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (activeSafeTimeSle_ >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("nearlink device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTimeSle_ = 0;
        activeSafeTimeSle_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_NEARLINK, 0);
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (CheckMixActiveMusicTime(safeVolume)) {
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else {
        activeSafeTimeSle_ = audioPolicyManager_.GetCurentDeviceSafeTime(DEVICE_TYPE_NEARLINK);
        activeSafeTimeSle_ += currentTime - startSafeTimeSle_;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_NEARLINK, activeSafeTimeSle_);
        startSafeTimeSle_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        AUDIO_INFO_LOG("nearlink safe volume timeout, cumulative time: %{public}" PRId64, activeSafeTimeSle_);
    }
    startSafeTimeBt_ = 0;
    startSafeTime_ = 0;
}

void AudioVolumeManager::CheckWiredActiveMusicTime(int32_t safeVolume)
{
    if (startSafeTime_ == 0) {
        startSafeTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (activeSafeTime_ >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("wired device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTime_ = 0;
        activeSafeTime_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, 0);
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (CheckMixActiveMusicTime(safeVolume)) {
        safeVolumeManager_.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else {
        activeSafeTime_ = audioPolicyManager_.GetCurentDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET);
        activeSafeTime_ += currentTime - startSafeTime_;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, activeSafeTime_);
        startSafeTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        AUDIO_INFO_LOG("wired safe volume timeout, cumulative time: %{public}" PRId64, activeSafeTime_);
    }
    startSafeTimeBt_ = 0;
    startSafeTimeSle_ = 0;
}

void AudioVolumeManager::CheckLowerDeviceVolume(DeviceType deviceType)
{
    int32_t btVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t wiredVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET);
    int32_t sleVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK);

    AUDIO_INFO_LOG("btVolume : %{public}d, wiredVolume : %{public}d, sleVolume : %{public}d",
        btVolume, wiredVolume, sleVolume);

    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (btVolume > safeVolume) {
                AUDIO_INFO_LOG("wired device timeout, set bt device to safe volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, safeVolume, DEVICE_TYPE_BLUETOOTH_A2DP);
            }
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (wiredVolume > safeVolume) {
                AUDIO_INFO_LOG("bt device timeout, set wired device to safe volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, safeVolume, DEVICE_TYPE_WIRED_HEADSET);
            }
            break;
        case DEVICE_TYPE_NEARLINK:
            if (sleVolume > safeVolume) {
                AUDIO_INFO_LOG("sle device timeout, set sle device to safe volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, safeVolume, DEVICE_TYPE_NEARLINK);
            }
            break;
        default:
            AUDIO_ERR_LOG("current device not set safe volume");
            break;
    }
}

void AudioVolumeManager::RestoreSafeVolume(AudioStreamType streamType, int32_t safeVolume)
{
    userSelect_ = false;
    isDialogSelectDestroy_.store(false);

    if (GetSystemVolumeLevel(streamType) <= safeVolume) {
        AUDIO_INFO_LOG("current volume <= safe volume, don't update volume.");
        return;
    }

    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume();
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;

    AUDIO_INFO_LOG("restore safe volume.");
    std::shared_ptr<AudioDeviceDescriptor> tempDeviceDesc = nullptr;
    SetSystemVolumeLevel(streamType, safeVolume, tempDeviceDesc);
    CheckLowerDeviceVolume(deviceType);
    SetSafeVolumeCallback(streamType);
}

void AudioVolumeManager::SetSafeVolumeCallback(AudioStreamType streamType, bool updateUi)
{
    CHECK_AND_RETURN_LOG(VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC,
        "streamtype:%{public}d no need to set safe volume callback.", streamType);
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevel(streamType);
    volumeEvent.volumeDegree = GetSystemVolumeDegree(streamType);
    volumeEvent.updateUi = updateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (audioPolicyServerHandler_ != nullptr && IsRingerModeMute()) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
        audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent);
    }
}

void AudioVolumeManager::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    AUDIO_INFO_LOG("enter.");
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == AUDIO_RESTORE_VOLUME_EVENT) {
        AUDIO_INFO_LOG("AUDIO_RESTORE_VOLUME_EVENT has been received");
        std::lock_guard<std::mutex> lock(notifyMutex_);
        safeVolumeManager_.CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = false;
        ChangeDeviceSafeStatus(SAFE_INACTIVE);
        DealWithEventVolume(RESTORE_VOLUME_NOTIFICATION_ID);
        SetSafeVolumeCallback(STREAM_MUSIC);
    } else if (action == AUDIO_INCREASE_VOLUME_EVENT) {
        AUDIO_INFO_LOG("AUDIO_INCREASE_VOLUME_EVENT has been received");
        std::lock_guard<std::mutex> lock(notifyMutex_);
        safeVolumeManager_.CancelSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
        increaseNIsShowing_ = false;
        ChangeDeviceSafeStatus(SAFE_INACTIVE);
        DealWithEventVolume(INCREASE_VOLUME_NOTIFICATION_ID);
        SetSafeVolumeCallback(STREAM_MUSIC);
    } else if (action == AUDIO_LEGACY_HIGH_VOLUME_EVENT) {
        std::lock_guard<std::mutex> lock(notifyMutex_);
        HandleLegacyHighVolumeEvent();
    }
}

void AudioVolumeManager::SetDeviceSafeVolumeStatus()
{
    if (!userSelect_) {
        return;
    }

    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume();
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            safeStatusBt_ = SAFE_INACTIVE;
            audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
            CreateCheckMusicActiveThread();
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            safeStatus_ = SAFE_INACTIVE;
            audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);
            CreateCheckMusicActiveThread();
            break;
        case DEVICE_TYPE_NEARLINK:
            safeStatusSle_ = SAFE_INACTIVE;
            audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_NEARLINK, safeStatusSle_);
            CreateCheckMusicActiveThread();
            break;
        default:
            AUDIO_INFO_LOG("safeVolume unsupported device:%{public}d", deviceType);
            break;
    }
}

void AudioVolumeManager::ChangeDeviceSafeStatus(SafeStatus safeStatus)
{
    AUDIO_INFO_LOG("change all support safe volume devices status.");

    safeStatusBt_ = safeStatus;
    audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);

    safeStatus_ = safeStatus;
    audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);

    safeStatusSle_ = safeStatus;
    audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_NEARLINK, safeStatusSle_);

    CreateCheckMusicActiveThread();
}

int32_t AudioVolumeManager::DisableSafeMediaVolume()
{
    AUDIO_INFO_LOG("Enter");
    std::lock_guard<std::mutex> lock(dialogMutex_);
    userSelect_ = true;
    isDialogSelectDestroy_.store(true);
    dialogSelectCondition_.notify_all();
    SetDeviceSafeVolumeStatus();
    return SUCCESS;
}

void AudioVolumeManager::SetAbsVolumeSceneAsync(const std::string &macAddress, const bool support, int32_t volume)
{
    usleep(SET_BT_ABS_SCENE_DELAY_MS);
    std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    AUDIO_INFO_LOG("success for macAddress:[%{public}s], support: %{public}d, active bt:[%{public}s]",
        GetEncryptAddr(macAddress).c_str(), support, GetEncryptAddr(btDevice).c_str());

    if (btDevice == macAddress) {
        audioPolicyManager_.SetAbsVolumeScene(support, volume);
        SetSharedAbsVolumeScene(support);
        auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
        CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
        DeviceType deviceType = deviceDesc->deviceType_;
        // for absVolumescene support or not support endpoint volume
        if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP && !support) {
            audioPolicyManager_.GetAllDeviceVolumeInfo(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_MUSIC);
            int32_t volumeLevel = audioPolicyManager_.GetSystemVolumeLevelNoMuteState(STREAM_MUSIC, deviceDesc);
            VolumeScale volumeScale = {volumeLevel, -1, GetMaxVolumeLevel(STREAM_MUSIC)};
            audioPolicyManager_.SetSystemVolumeLevel(STREAM_MUSIC, volumeScale, deviceDesc);
        } else if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP && support) {
            Volume vol = {false, 1.0f, 0};
            vol.isMute = volume == 0 ? true : false;
            vol.volumeInt = static_cast<uint32_t>(volume);
            SetSharedVolume(STREAM_MUSIC, deviceType, vol);
        }
    }
}

int32_t AudioVolumeManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support,
    int32_t volume)
{
    // Maximum number of attempts, preventing situations where a2dp device has not yet finished coming online.
    int maxRetries = 3;
    int retryCount = 0;
    while (retryCount < maxRetries) {
        retryCount++;
        auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
        int32_t currentVolume = support ? volume :
            audioPolicyManager_.GetSystemVolumeLevelNoMuteState(STREAM_MUSIC, deviceDesc);
        bool currentMute = support ? (volume == 0) : audioPolicyManager_.GetStreamMute(STREAM_MUSIC, deviceDesc);
        VolumeScale volumeScale = {currentVolume, -1, GetMaxVolumeLevel(STREAM_MUSIC)};
        if (audioA2dpDevice_.SetA2dpDeviceAbsVolumeSupport(macAddress, support, volumeScale, currentMute)) {
            break;
        }
        CHECK_AND_RETURN_RET_LOG(retryCount != maxRetries, ERROR,
            "failed, can't find device for macAddress:[%{public}s]", GetEncryptAddr(macAddress).c_str());;
        usleep(ABS_VOLUME_SUPPORT_RETRY_INTERVAL_IN_MICROSECONDS);
    }

    // The delay setting is due to move a2dp sink after this
    std::thread setAbsSceneThrd(&AudioVolumeManager::SetAbsVolumeSceneAsync, this, macAddress, support, volume);
    setAbsSceneThrd.detach();

    return SUCCESS;
}

int32_t AudioVolumeManager::SetSleVoiceStatusFlag(bool isSleVoiceStatus)
{
    std::lock_guard<std::mutex> lock(setSharedSleAbsVolumeSceneMutex_);
    SetSharedSleAbsVolumeScene(!isSleVoiceStatus);
    audioPolicyManager_.SetSleVoiceStatusFlag(isSleVoiceStatus);
    return SUCCESS;
}

int32_t AudioVolumeManager::SetStreamMute(AudioStreamType streamType, bool mute, const StreamUsage &streamUsage,
    const DeviceType &deviceType, int32_t zoneId)
{
#ifndef FEATURE_AUDIO_ZONE_VOLUME
    if (zoneId > 0) {
        return audioPolicyManager_.SetZoneMute(zoneId, streamType, mute, streamUsage, deviceType);
    }
#endif
    int32_t result = HandleStreamMute(streamType, mute, streamUsage, deviceType, zoneId);
    CHECK_AND_CALL_FUNC_RETURN_RET(result == SUCCESS, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_FAILED, "SetStreamMute failed", false));
    return result;
}

int32_t AudioVolumeManager::HandleStreamMute(AudioStreamType streamType, bool mute, const StreamUsage &streamUsage,
    DeviceType deviceType, int32_t zoneId)
{
    auto volType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    auto deviceDesc = deviceType == DEVICE_TYPE_NONE ?
        audioActiveDevice_.GetDeviceForVolume(streamType) : audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    deviceType = deviceDesc->deviceType_;
    std::string deviceMacAddr = deviceDesc->macAddress_;

    if (volType == STREAM_MUSIC && deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (audioA2dpDevice_.SetA2dpDeviceMute(btDevice, mute)) {
            audioPolicyManager_.SetAbsVolumeMute(mute);
            Volume vol = {false, 1.0f, 0};
            vol.isMute = mute;
            vol.volumeInt = static_cast<uint32_t>(
                audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType, deviceDesc));
            vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDbByDegree(streamType, deviceDesc, mute);
            SetSharedVolume(streamType, deviceType, vol);
#ifdef BLUETOOTH_ENABLE
            int32_t volumeLevel;
            audioA2dpDevice_.GetA2dpDeviceVolumeLevel(btDevice, volumeLevel);
            return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice, volumeLevel);
#endif
        }
    }

    if (volType == STREAM_MUSIC && deviceType == DEVICE_TYPE_NEARLINK) {
        int32_t resultSet = SUCCESS;
        if (!IsNearlinkKaraokeVolumeManaged(streamType)) {
            resultSet = SleAudioDeviceManager::GetInstance().SetNearlinkDeviceMute(deviceMacAddr, streamType, mute);
        }
        if (resultSet == SUCCESS) {
            audioPolicyManager_.SetAbsVolumeMuteNearlink(mute);
            Volume vol = {false, 1.0f, 0};
            vol.isMute = mute;
            vol.volumeInt = static_cast<uint32_t>(
                audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType, deviceDesc));
            vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDbByDegree(streamType, deviceDesc, mute);
            SetSharedVolume(streamType, deviceType, vol);
            int32_t volumeLevel = SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(
                streamType, deviceDesc);
            return SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(deviceMacAddr, streamType, volumeLevel);
        }
    }

    int32_t result = audioPolicyManager_.SetStreamMute(deviceDesc, streamType, mute, streamUsage, zoneId);

    Volume vol = {false, 1.0f, 0};
    vol.isMute = mute;
    vol.volumeInt = static_cast<uint32_t>(audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType, deviceDesc));
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDbByDegree(streamType, deviceDesc, mute);
    SetSharedVolume(streamType, deviceType, vol);

    return result;
}

bool AudioVolumeManager::GetStreamMute(AudioStreamType streamType, int32_t zoneId) const
{
#ifndef FEATURE_AUDIO_ZONE_VOLUME
    if (zoneId > 0) {
        return audioPolicyManager_.GetZoneMute(zoneId, streamType);
    }
#endif
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(streamType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, false, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_MUSIC &&
        deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        A2dpDeviceConfigInfo info;
        bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
        if (ret == false || !info.absVolumeSupport) {
            AUDIO_WARNING_LOG("Get failed for macAddress:[%{public}s]", GetEncryptAddr(btDevice).c_str());
        } else {
            return info.mute;
        }
    }
    if (volumeType == STREAM_MUSIC && deviceType == DEVICE_TYPE_NEARLINK) {
        bool muteStatus = SleAudioDeviceManager::GetInstance().GetMuteStatusByVolumeType(
            streamType, deviceDesc->macAddress_);
        return muteStatus;
    }
    return audioPolicyManager_.GetStreamMute(streamType, deviceDesc, zoneId);
}

void AudioVolumeManager::UpdateGroupInfo(GroupType type, std::string groupName, int32_t& groupId,
    std::string networkId, bool connected, int32_t mappingId)
{
    std::lock_guard<std::mutex> lock(volumeGroupsMutex_);
    ConnectType connectType = CONNECT_TYPE_LOCAL;
    if (networkId != LOCAL_NETWORK_ID) {
        connectType = CONNECT_TYPE_DISTRIBUTED;
    }
    if (type == GroupType::VOLUME_TYPE) {
        auto isPresent = [&groupName, &networkId] (const sptr<VolumeGroupInfo> &volumeInfo) {
            return ((groupName == volumeInfo->groupName_) || (networkId == volumeInfo->networkId_));
        };

        auto iter = std::find_if(volumeGroups_.begin(), volumeGroups_.end(), isPresent);
        if (iter != volumeGroups_.end()) {
            groupId = (*iter)->volumeGroupId_;
            // if status is disconnected, remove the group that has none audio device
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> devsInGroup =
                audioConnectedDevice_.GetDevicesForGroup(type, groupId);
            if (!connected && devsInGroup.size() == 0) {
                volumeGroups_.erase(iter);
            }
            return;
        }
        if (groupName != GROUP_NAME_NONE && connected) {
            groupId = AudioGroupHandle::GetInstance().GetNextId(type);
            sptr<VolumeGroupInfo> volumeGroupInfo = new(std::nothrow) VolumeGroupInfo(groupId,
                mappingId, groupName, networkId, connectType);
            CHECK_AND_RETURN_LOG(volumeGroupInfo != nullptr, "volumeGroupInfo is nullptr.");
            volumeGroups_.push_back(volumeGroupInfo);
        }
    } else {
        auto isPresent = [&groupName, &networkId] (const sptr<InterruptGroupInfo> &info) {
            return ((groupName == info->groupName_) || (networkId == info->networkId_));
        };

        auto iter = std::find_if(interruptGroups_.begin(), interruptGroups_.end(), isPresent);
        if (iter != interruptGroups_.end()) {
            groupId = (*iter)->interruptGroupId_;
            // if status is disconnected, remove the group that has none audio device
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> devsInGroup =
                audioConnectedDevice_.GetDevicesForGroup(type, groupId);
            if (!connected && devsInGroup.size() == 0) {
                interruptGroups_.erase(iter);
            }
            return;
        }
        if (groupName != GROUP_NAME_NONE && connected) {
            groupId = AudioGroupHandle::GetInstance().GetNextId(type);
            sptr<InterruptGroupInfo> interruptGroupInfo = new(std::nothrow) InterruptGroupInfo(groupId, mappingId,
                groupName, networkId, connectType);
            CHECK_AND_RETURN_LOG(interruptGroupInfo != nullptr, "interruptGroupInfo is nullptr.");
            interruptGroups_.push_back(interruptGroupInfo);
        }
    }
}

void AudioVolumeManager::GetVolumeGroupInfo(std::vector<sptr<VolumeGroupInfo>>& volumeGroupInfos)
{
    std::lock_guard<std::mutex> lock(volumeGroupsMutex_);
    for (auto& v : volumeGroups_) {
        sptr<VolumeGroupInfo> info = new(std::nothrow) VolumeGroupInfo(v->volumeGroupId_, v->mappingId_, v->groupName_,
            v->networkId_, v->connectType_);
        CHECK_AND_RETURN_LOG(info != nullptr, "info is nullptr.");
        volumeGroupInfos.push_back(info);
    }
}

int32_t AudioVolumeManager::CheckRestoreDeviceVolume(DeviceType deviceType)
{
    int32_t ret = 0;
    int32_t btRestoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t wiredRestoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET);
    int32_t sleRestoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK);

    AUDIO_INFO_LOG("btRestoreVolume: %{public}d, wiredRestoreVolume: %{public}d, sleRestoreVolume: %{public}d",
        btRestoreVolume, wiredRestoreVolume, sleRestoreVolume);

    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (wiredRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore active wired device volume");
                ret = SetSystemVolumeLevel(STREAM_MUSIC, wiredRestoreVolume, deviceDesc);
            }
            if (btRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore other bt device volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, btRestoreVolume, DEVICE_TYPE_BLUETOOTH_A2DP);
            }
            if (sleRestoreVolume > safeVolume) {
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, sleRestoreVolume, DEVICE_TYPE_NEARLINK);
            }
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (btRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore active bt device volume");
                ret = SetSystemVolumeLevel(STREAM_MUSIC, btRestoreVolume, deviceDesc);
            }
            if (wiredRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore other wired device volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, wiredRestoreVolume, DEVICE_TYPE_WIRED_HEADSET);
            }
            if (sleRestoreVolume > safeVolume) {
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, sleRestoreVolume, DEVICE_TYPE_NEARLINK);
            }
            break;
        case DEVICE_TYPE_NEARLINK:
            ret = CheckRestoreDeviceVolumeNearlink(btRestoreVolume, wiredRestoreVolume, sleRestoreVolume, safeVolume);
            break;
        default:
            ret = ERROR;
            AUDIO_ERR_LOG("current device not set safe volume");
            break;
    }

    return ret;
}

int32_t AudioVolumeManager::CheckRestoreDeviceVolumeNearlink(int32_t btRestoreVolume, int32_t wiredRestoreVolume,
    int32_t sleRestoreVolume, int32_t safeVolume)
{
    int32_t ret = 0;
    if (sleRestoreVolume > safeVolume) {
        AUDIO_INFO_LOG("restore active sle device volume");
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
        ret = SetSystemVolumeLevel(STREAM_MUSIC, sleRestoreVolume, deviceDesc);
    }
    if (btRestoreVolume > safeVolume) {
        AUDIO_INFO_LOG("restore other bt device volume");
        SaveSpecifiedDeviceVolume(STREAM_MUSIC, btRestoreVolume, DEVICE_TYPE_BLUETOOTH_A2DP);
    }
    if (wiredRestoreVolume > safeVolume) {
        AUDIO_INFO_LOG("restore other wired device volume");
        SaveSpecifiedDeviceVolume(STREAM_MUSIC, wiredRestoreVolume, DEVICE_TYPE_WIRED_HEADSET);
    }
    return ret;
}

int32_t AudioVolumeManager::DealWithEventVolume(const int32_t notificationId)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;
    int32_t safeVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    const int32_t ONE_VOLUME_LEVEL = 1;
    int32_t ret = 0;
    if (safeVolumeManager_.IsBlueTooth(deviceDesc)) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                ret = CheckRestoreDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP);
                break;
            case INCREASE_VOLUME_NOTIFICATION_ID:
                ret = SetSystemVolumeLevel(STREAM_MUSIC, safeVolumeLevel + ONE_VOLUME_LEVEL, deviceDesc);
                break;
            default:
                AUDIO_ERR_LOG("current state unsupport safe volume");
        }
    } else if (safeVolumeManager_.IsWiredHeadSet(deviceType)) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                ret = CheckRestoreDeviceVolume(DEVICE_TYPE_WIRED_HEADSET);
                break;
            case INCREASE_VOLUME_NOTIFICATION_ID:
                ret = SetSystemVolumeLevel(STREAM_MUSIC, safeVolumeLevel + ONE_VOLUME_LEVEL, deviceDesc);
                break;
            default:
                AUDIO_ERR_LOG("current state unsupport safe volume");
        }
    } else if (deviceType == DEVICE_TYPE_NEARLINK) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                ret = CheckRestoreDeviceVolume(DEVICE_TYPE_NEARLINK);
                break;
            case INCREASE_VOLUME_NOTIFICATION_ID:
                ret = SetSystemVolumeLevel(STREAM_MUSIC, safeVolumeLevel + ONE_VOLUME_LEVEL, deviceDesc);
                break;
            default:
                AUDIO_ERR_LOG("current state unsupport safe volume");
        }
    } else {
        AUDIO_ERR_LOG("current output device unsupport safe volume");
        ret = ERROR;
    }
    return ret;
}

bool AudioVolumeManager::IsRingerModeMute()
{
    return ringerModeManager_.IsRingerModeMute();
}

void AudioVolumeManager::SetRingerModeMute(bool flag)
{
    ringerModeManager_.SetRingerModeMute(flag);
}

int32_t AudioVolumeManager::ResetRingerModeMute()
{
    return ringerModeManager_.ResetRingerModeMute();
}

bool AudioVolumeManager::IsRingerModeValid(AudioRingerMode ringMode)
{
    return ringerModeManager_.IsRingerModeValid(ringMode);
}

bool AudioVolumeManager::GetVolumeGroupInfosNotWait(std::vector<sptr<VolumeGroupInfo>> &infos)
{
    if (!isPrimaryMicModuleInfoLoaded_) {
        return false;
    }

    GetVolumeGroupInfo(infos);
    return true;
}

void AudioVolumeManager::SetDefaultDeviceLoadFlag(bool isLoad)
{
    isPrimaryMicModuleInfoLoaded_.store(isLoad);
}

bool AudioVolumeManager::GetLoadFlag()
{
    return isPrimaryMicModuleInfoLoaded_.load();
}

void AudioVolumeManager::NotifyVolumeGroup()
{
    std::lock_guard<std::mutex> lock(defaultDeviceLoadMutex_);
    SetDefaultDeviceLoadFlag(true);
}

void AudioVolumeManager::UpdateSafeVolumeByS4()
{
    AUDIO_INFO_LOG("Reset isBtFirstBoot by S4 reboot");
    isBtFirstBoot_ = true;
    return audioPolicyManager_.UpdateSafeVolumeByS4();
}

std::vector<std::shared_ptr<AllDeviceVolumeInfo>> AudioVolumeManager::GetAllDeviceVolumeInfo()
{
    std::vector<std::shared_ptr<AllDeviceVolumeInfo>> allDeviceVolumeInfo = {};
    std::shared_ptr<AllDeviceVolumeInfo> deviceVolumeInfo = std::make_shared<AllDeviceVolumeInfo>();
    auto deviceList = audioConnectedDevice_.GetDevicesInner(DeviceFlag::ALL_L_D_DEVICES_FLAG);
    for (auto &device : deviceList) {
        for (auto &streamType : AUDIO_STREAMTYPE_VOLUME_LIST) {
            if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
                continue;
            }
            deviceVolumeInfo = audioPolicyManager_.GetAllDeviceVolumeInfo(device->deviceType_, streamType);
            if (deviceVolumeInfo != nullptr) {
                allDeviceVolumeInfo.push_back(deviceVolumeInfo);
            }
        }
    }
    return allDeviceVolumeInfo;
}

void AudioVolumeManager::SaveSystemVolumeLevelInfo(AudioStreamType streamType, int32_t volumeLevel,
    int32_t appUid, std::string invocationTime)
{
    AdjustVolumeInfo systemVolumeLevelInfo;
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume();
    systemVolumeLevelInfo.deviceType = deviceDesc != nullptr ? deviceDesc->deviceType_ : DEVICE_TYPE_NONE;
    systemVolumeLevelInfo.streamType = streamType;
    systemVolumeLevelInfo.volumeLevel = volumeLevel;
    systemVolumeLevelInfo.appUid = appUid;
    systemVolumeLevelInfo.invocationTime = invocationTime;
    systemVolumeLevelInfo_->Add(systemVolumeLevelInfo);
}

void AudioVolumeManager::SaveVolumeKeyRegistrationInfo(std::string keyType, std::string registrationTime,
    int32_t subscriptionId, bool registrationResult)
{
    VolumeKeyEventRegistration volumeKeyEventRegistration;
    volumeKeyEventRegistration.keyType = keyType;
    volumeKeyEventRegistration.subscriptionId = subscriptionId;
    volumeKeyEventRegistration.registrationTime = registrationTime;
    volumeKeyEventRegistration.registrationResult = registrationResult;
    volumeKeyRegistrations_->Add(volumeKeyEventRegistration);
}

void AudioVolumeManager::GetSystemVolumeLevelInfo(std::vector<AdjustVolumeInfo> &systemVolumeLevelInfo)
{
    systemVolumeLevelInfo = systemVolumeLevelInfo_->GetData();
}

void AudioVolumeManager::GetVolumeKeyRegistrationInfo(std::vector<VolumeKeyEventRegistration> &keyRegistrationInfo)
{
    keyRegistrationInfo = volumeKeyRegistrations_->GetData();
}

int32_t AudioVolumeManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG(duration >= CANCEL_FORCE_CONTROL_VOLUME_TYPE, ERR_INVALID_PARAM, "invalid duration");
    CHECK_AND_RETURN_RET_LOG(forceControlVolumeTypeMonitor_ != nullptr, ERR_UNKNOWN,
        "forceControlVolumeTypeMonitor_ is nullptr");
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    needForceControlVolumeType_ = (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE ? false : true);
    forceControlVolumeType_ = (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE ? STREAM_DEFAULT : volumeType);
    forceControlVolumeTypeMonitor_->SetTimer(duration, forceControlVolumeTypeMonitor_);
    return SUCCESS;
}

void AudioVolumeManager::OnTimerExpired()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    needForceControlVolumeType_ = false;
    forceControlVolumeType_ = STREAM_DEFAULT;
}

bool AudioVolumeManager::IsNeedForceControlVolumeType()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    return needForceControlVolumeType_;
}

void AudioVolumeManager::CheckReduceOtherActiveVolume(AudioStreamType streamType,
    int32_t volumeLevel)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(streamType);
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "deviceDesc is nullptr");
    DeviceType deviceType = deviceDesc->deviceType_;
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    AudioScene curScene = audioSceneManager_.GetAudioScene(true);
    bool streamInCall = curScene == AUDIO_SCENE_PHONE_CALL || curScene == AUDIO_SCENE_PHONE_CHAT;
    if (streamInCall && volumeType == STREAM_VOICE_CALL) {
        float volumeDb = audioPolicyManager_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
        audioPolicyManager_.SetVolumeLimit(volumeDb);
        audioPolicyManager_.UpdateOtherStreamVolume(streamType);
    }
}

AudioVolumeType AudioVolumeManager::GetForceControlVolumeType()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    return forceControlVolumeType_;
}

ForceControlVolumeTypeMonitor::~ForceControlVolumeTypeMonitor()
{
    std::lock_guard<std::mutex> lock(monitorMtx_);
    StopMonitor();
}

void ForceControlVolumeTypeMonitor::OnTimeOut()
{
    {
        std::lock_guard<std::mutex> lock(monitorMtx_);
        StopMonitor();
    }
    audioVolumeManager_.OnTimerExpired();
}

void ForceControlVolumeTypeMonitor::StartMonitor(int32_t duration,
    std::shared_ptr<ForceControlVolumeTypeMonitor> cb)
{
    int32_t cbId = DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->RegisterCallback(
        cb, duration, CallbackType::ONE_TIME);
    if (cbId == INVALID_CB_ID) {
        AUDIO_ERR_LOG("Register AudioPolicyStateMonitor failed");
    } else {
        cbId_ = cbId;
    }
}

void ForceControlVolumeTypeMonitor::StopMonitor()
{
    if (cbId_ != INVALID_CB_ID) {
        DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->UnRegisterCallback(cbId_);
        cbId_ = INVALID_CB_ID;
    }
}

void ForceControlVolumeTypeMonitor::SetTimer(int32_t duration,
    std::shared_ptr<ForceControlVolumeTypeMonitor> cb)
{
    std::lock_guard<std::mutex> lock(monitorMtx_);
    StopMonitor();
    if (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE) {
        return;
    }
    duration_ = (duration > MAX_DURATION_TIME_S ? MAX_DURATION_TIME_S : duration);
    StartMonitor(duration_, cb);
}

int32_t AudioVolumeManager::GetSystemVolumeDegree(AudioStreamType streamType, int32_t zoneId)
{
    if (zoneId > 0) {
        return audioPolicyManager_.GetZoneVolumeDegree(zoneId, streamType);
    }

    if (streamType == STREAM_RING && !IsRingerModeMute()) {
        AUDIO_PRERELEASE_LOGW("return 0 when dual tone ring");
        return DUAL_TONE_RING_VOLUME;
    }

    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(volumeType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, DUAL_TONE_RING_VOLUME, "deviceDesc is null");

    int32_t specialVolumeDegree = 0;
    if (GetSpecialDeviceVolumeDegree(streamType, volumeType, deviceDesc, specialVolumeDegree)) {
        return specialVolumeDegree;
    }

    return audioPolicyManager_.GetSystemVolumeDegree(deviceDesc, streamType, zoneId);
}

bool AudioVolumeManager::GetSpecialDeviceVolumeDegree(AudioStreamType streamType, AudioVolumeType volumeType,
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, int32_t &volumeDegree)
{
    CHECK_AND_RETURN_RET(deviceDesc != nullptr, false);

    {
        DeviceType curOutputDeviceType = deviceDesc->deviceType_;
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (volumeType == STREAM_MUSIC &&
            curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            A2dpDeviceConfigInfo info;
            bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
            if (ret && info.absVolumeSupport) {
                volumeDegree = info.mute ? 0 : info.volumeDegree;
                return true;
            }
        }
    }

    if (deviceDesc->deviceType_ == DEVICE_TYPE_NEARLINK &&
        (volumeType == STREAM_MUSIC || volumeType == STREAM_VOICE_CALL)) {
        if (IsNearlinkKaraokeVolumeManaged(streamType)) {
            volumeDegree = VolumeUtils::VolumeLevelToDegree(GetNearlinkKaraokeVolume(),
                GetMaxVolumeLevel(STREAM_MUSIC));
            return true;
        }
        volumeDegree = SleAudioDeviceManager::GetInstance().GetVolumeDegreeByVolumeType(volumeType, *deviceDesc);
        return true;
    }

    return false;
}

int32_t AudioVolumeManager::GetSystemVolumeDegree(AudioStreamType streamType, DeviceType deviceType,
    const std::string &networkId)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    auto deviceDesc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType, networkId);
    int32_t specialVolumeDegree = 0;
    if (GetSpecialDeviceVolumeDegree(streamType, volumeType, deviceDesc, specialVolumeDegree)) {
        return specialVolumeDegree;
    }
    return audioPolicyManager_.GetSystemVolumeDegree(deviceDesc, streamType);
}

int32_t AudioVolumeManager::GetMinVolumeDegree(AudioVolumeType volumeType, DeviceType deviceType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMinVolumeDegree(volumeType, deviceType);
}

int32_t AudioVolumeManager::UpdateSafeVolumeByStrMode()
{
    int32_t safeVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    userSelect_ = false;
    isDialogSelectDestroy_.store(false);

    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is null");
    DeviceType deviceType = deviceDesc->deviceType_;
    if (deviceType != DEVICE_TYPE_BLUETOOTH_A2DP && deviceType != DEVICE_TYPE_BLUETOOTH_SCO
        && deviceType != DEVICE_TYPE_NEARLINK) {
        isBtFirstBoot_ = true;
    }

    if (GetSystemVolumeLevel(STREAM_MUSIC) <= safeVolumeLevel) {
        AUDIO_INFO_LOG("current volume <= safe volume, don't update volume.");
        return SUCCESS;
    }

    AUDIO_INFO_LOG("restore safe volume.");
    SetSystemVolumeExternal(STREAM_MUSIC, safeVolumeLevel, deviceType);
    audioPolicyManager_.UpdateSafeVolumeByS4();
    CheckLowerDeviceVolume(deviceType);
    SetSafeVolumeCallback(STREAM_MUSIC, false);
    return SUCCESS;
}

bool AudioVolumeManager::IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel)
{
    bool result = true;
    if (volumeLevel < GetMinVolumeLevel(streamType) ||
        volumeLevel > GetMaxVolumeLevel(streamType)) {
        AUDIO_ERR_LOG("IsVolumeLevelValid: volumeLevel[%{public}d] is out of valid range for streamType[%{public}d]",
            volumeLevel, streamType);
        result = false;
    }
    return result;
}

bool AudioVolumeManager::GetStreamMuteInterface(AudioStreamType streamType, int32_t zoneId)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    bool isMuted = GetStreamMute(streamType, zoneId);
    AUDIO_DEBUG_LOG("GetMute streamType[%{public}d],mute[%{public}d]", streamType, isMuted);
    return isMuted;
}

int32_t AudioVolumeManager::SetRingerMode(AudioRingerMode inputRingerMode, bool hasUpdatedRingtoneVolume)
{
    // PC ringmode not support silent or vibrate
    AudioRingerMode ringerMode = VolumeUtils::IsPCVolumeEnable() ? RINGER_MODE_NORMAL : inputRingerMode;
    AUDIO_INFO_LOG("Set ringer mode to %{public}d. hasUpdatedRingtoneVolume %{public}d",
        ringerMode, hasUpdatedRingtoneVolume);
    std::shared_ptr<AudioCoreService> coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_UNKNOWN, "coreService is nullptr");
    int32_t ret = coreService->SetRingerMode(ringerMode);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_FAILED, "SetRingerMode failed", false),
        "Fail to set ringer mode!");

    // need to set volume according to ringermode
    bool muteState = (ringerMode == RINGER_MODE_NORMAL) ? false : true;
    if (!hasUpdatedRingtoneVolume) {
        AudioInterrupt audioInterrupt;
        int32_t zoneID = 0;
        CHECK_AND_RETURN_RET_LOG(interruptService_ != nullptr, ERR_UNKNOWN, "interruptService is nullptr");
        interruptService_->GetSessionInfoInFocus(audioInterrupt, zoneID);
        int32_t previousRingtoneVolume = GetSystemVolumeLevelInterface(STREAM_RING);
        SetStreamMute(STREAM_RING, muteState, audioInterrupt.streamUsage);
        if (!muteState && GetSystemVolumeLevelNoMuteStateInterface(STREAM_RING) == 0) {
            // if mute state is false but volume is 0, set volume to 1. Send volumeChange callback.
            VolumeUpdateOption info = { false };
            SetSystemVolumeLevelWithOption(STREAM_RING, 1, DEFAULT_ZONE_ID, info);
        }
        SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_RING, false, 0, nullptr, previousRingtoneVolume);
    }
    int32_t previousNotificationVolume = GetSystemVolumeLevelInterface(STREAM_NOTIFICATION);
    SetStreamMute(STREAM_NOTIFICATION, muteState);
    if (!muteState && GetSystemVolumeLevelNoMuteStateInterface(STREAM_NOTIFICATION) == 0) {
        // if mute state is false but volume is 0, set volume to 1. Send volumeChange callback.
        VolumeUpdateOption info = { false };
        SetSystemVolumeLevelWithOption(STREAM_NOTIFICATION, 1, DEFAULT_ZONE_ID, info);
    }
    SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_NOTIFICATION, false, 0, nullptr, previousNotificationVolume);

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRingerModeUpdatedCallback(ringerMode);
    }
    return ret;
}

void AudioVolumeManager::ProcUpdateRingerMode()
{
    int32_t curRingVolumeLevel = GetSystemVolumeLevelNoMuteStateInterface(STREAM_RING);
    AudioRingerMode ringerMode = ringerModeManager_.GetRingerModeForVolumeLevel(curRingVolumeLevel);
    AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring volume level", ringerMode);
    // Update ringer mode but no need to update volume again.
    SetRingerMode(ringerMode, true);
}

void AudioVolumeManager::UpdateMuteStateAccordingToVolLevel(const VolInfoForUpdateMute &info, const bool &isUpdateUi,
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc, int32_t previousVolume)
{
    AudioStreamType streamType = info.streamType;
    int32_t volumeLevel = info.volumeLevel;
    bool mute = info.mute;
    int32_t zoneId = info.zoneId;

    bool muteStatus = mute;
    if (volumeLevel == 0 && !mute) {
        muteStatus = true;
        SetStreamMute(streamType, true, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE, zoneId);
    } else if (volumeLevel > 0 && mute) {
        muteStatus = false;
        SetStreamMute(streamType, false, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE, zoneId);
    }
    SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId, deviceDesc, previousVolume);
    if (VolumeUtils::IsPCVolumeEnable()) {
        // system mute status should be aligned with music mute status.
        AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
        if (volumeType == STREAM_MUSIC &&
            muteStatus != GetStreamMuteInterface(STREAM_SYSTEM, zoneId)) {
            AUDIO_DEBUG_LOG("set system mute to %{public}d when STREAM_MUSIC.", muteStatus);
            SetStreamMute(STREAM_SYSTEM, muteStatus, STREAM_USAGE_UNKNOWN,
                DEVICE_TYPE_NONE, zoneId);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, false, zoneId, deviceDesc, previousVolume);
        } else if (volumeType == STREAM_SYSTEM &&
            muteStatus != GetStreamMuteInterface(STREAM_MUSIC, zoneId)) {
            bool isMute = (GetSystemVolumeLevelInterface(STREAM_MUSIC, zoneId) == 0) ? true : false;
            AUDIO_DEBUG_LOG("set system same to music muted or level is zero to %{public}d.", isMute);
            SetStreamMute(STREAM_SYSTEM, isMute, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE, zoneId);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, false, zoneId, deviceDesc, previousVolume);
        }
    }
}

void AudioVolumeManager::SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType,
    const bool& isUpdateUi, int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> deviceDesc, int32_t previousVolume)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInterface(streamType, zoneId);
    volumeEvent.volumeDegree = GetSystemVolumeDegreeInterface(streamType, zoneId);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = deviceDesc == nullptr ? LOCAL_NETWORK_ID : deviceDesc->networkId_;
    volumeEvent.deviceType = deviceDesc == nullptr ? DEVICE_TYPE_NONE : deviceDesc->deviceType_;
    volumeEvent.previousVolume = previousVolume;
    bool ringerModeMute = IsRingerModeMute();
    if (audioPolicyServerHandler_ != nullptr && ringerModeMute) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent, zoneId);
        audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent, zoneId);
    }
}

void AudioVolumeManager::SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType,
    const bool& isUpdateUi, int32_t zoneId, int32_t previousVolume)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInterface(streamType, zoneId);
    volumeEvent.volumeDegree = GetSystemVolumeDegreeInterface(streamType, zoneId);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    volumeEvent.previousVolume = previousVolume;
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    volumeEvent.deviceType = desc != nullptr ? desc->deviceType_ : DEVICE_TYPE_NONE;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
        audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent);
    }
}

int32_t AudioVolumeManager::GetSystemVolumeLevelNoMuteStateInterface(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel = GetSystemVolumeLevelNoMuteState(streamType);
    AUDIO_DEBUG_LOG("GetVolumeNoMute streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    return volumeLevel;
}

int32_t AudioVolumeManager::GetSystemVolumeDegreeInterface(AudioStreamType streamType, int32_t zoneId)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeDegree = GetSystemVolumeDegree(streamType, zoneId);
    AUDIO_DEBUG_LOG("GetVolume streamType[%{public}d],volumeDegree[%{public}d]", streamType, volumeDegree);
    return volumeDegree;
}

int32_t AudioVolumeManager::GetSystemVolumeLevelInterface(AudioStreamType streamType, int32_t zoneId)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel = GetSystemVolumeLevel(streamType, zoneId);
    AUDIO_DEBUG_LOG("GetVolume streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    CHECK_AND_RETURN_RET(volumeLevel != 0 || IPCSkeleton::GetCallingUid() != RSS_UID,
        GetVolumeForHiplayDevice(streamType));
    return volumeLevel;
}

int32_t AudioVolumeManager::SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel,
    const VolumeUpdateOption &option)
{
    bool isUpdateUi = option.isUpdateUi;
    bool mute = option.mute;
    int32_t zoneId = option.zoneId;

    bool updateRingerMode = false;
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if ((streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) &&
        volumeType == AudioStreamType::STREAM_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone volume level.
        if (ringerModeManager_.ShouldUpdateRingerMode(volumeLevel)) {
            // When isUpdateUi is false, the func is called by others. Need to verify permission.
            if (!isUpdateUi && !PermissionUtil::VerifyGeneralPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
                AUDIO_ERR_LOG("ACCESS_NOTIFICATION_POLICY_PERMISSION permission denied for ringtone volume!");
                return ERR_PERMISSION_DENIED;
            }
            updateRingerMode = true;
        }
    }

    int32_t previousVolume = GetSystemVolumeLevelInterface(streamType, zoneId);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    int32_t ret = SetSystemVolumeLevel(streamType, {volumeLevel, option.volumeDegree},
        deviceDesc, zoneId);
    if (ret == SUCCESS) {
        std::string currentTime = GetTime();
        int32_t appUid = IPCSkeleton::GetCallingUid();
        AUDIO_INFO_LOG("SetSystemVolumeLevelInfo streamType: %{public}d, volumeLevel: %{public}d,"
            " appUid: %{public}d, setTime: %{public}s",
            streamType, volumeLevel, appUid, currentTime.c_str());
        SaveSystemVolumeLevelInfo(streamType, volumeLevel, appUid, currentTime);
        if (updateRingerMode) {
            ProcUpdateRingerMode();
        }
        VolInfoForUpdateMute info = { streamType, volumeLevel, mute, zoneId };
        UpdateMuteStateAccordingToVolLevel(info, isUpdateUi, deviceDesc, previousVolume);
    } else if (ret == ERR_SET_VOL_FAILED_BY_SAFE_VOL) {
        SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId, deviceDesc);
        AUDIO_ERR_LOG("fail to set system volume level by safe vol");
    } else {
        AUDIO_ERR_LOG("fail to set system volume level, ret is %{public}d", ret);
    }

    return ret;
}

bool AudioVolumeManager::CheckVolumeLimitExceeded(AudioStreamType streamType, int32_t uid,
    int32_t targetVolumeDegree, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(targetVolumeDegree > GetSystemVolumeDegreeInterface(streamType, zoneId),
        false, "target volume is not higher than current volume");
    const int64_t bootTimeNs = ClockTime::GetBootNano();
    const int64_t currentTime = bootTimeNs < 0 ? -1 : bootTimeNs / static_cast<int64_t>(AUDIO_NS_PER_SECOND);
    const AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    VolumeLimitExceedEvent volumeLimitEvent;
    {
        std::lock_guard<std::mutex> lock(volumeProtectionMutex_);
        const auto rule = volumeProtectionConfig_.rules.find(volumeType);
        if (rule == volumeProtectionConfig_.rules.end() || targetVolumeDegree <= rule->second ||
            (currentTime >= 0 && currentTime < volumeLimitExpiryTime_)) {
            return false;
        }

        volumeLimitEvent.volumeType = volumeType;
        volumeLimitEvent.currentVolume = targetVolumeDegree;
        volumeLimitEvent.volumeThreshold = rule->second;
        volumeLimitEvent.uid = uid;
    }

    if (audioPolicyServerHandler_ != nullptr) {
        (void)audioPolicyServerHandler_->SendVolumeLimitExceededCallback(volumeLimitEvent);
    }
    return true;
}

bool AudioVolumeManager::CheckLoudVolumeMode(bool mute, int32_t volumeLevel, AudioStreamType streamType)
{
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    if (loudVolumeSupportMode_ == LOUD_VOLUME_NOT_SUPPORT || loudVolumeManager_ == nullptr) {
        return false;
    }
    int32_t volumeLevelMax = GetMaxVolumeLevel(static_cast<AudioVolumeType>(streamType));
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = audioActiveDevice_.GetDeviceForVolume();

    int32_t keyCode = (volumeLevelMax > volumeLevel) ? OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN
        : OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP;
    keyCode = mute ? OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN : keyCode;
    if (loudVolumeManager_->CheckLoudVolumeMode(volumeLevel, keyCode, streamType, deviceDesc)) {
        AUDIO_INFO_LOG("stream %{public}d, loud volume mode operation", streamType);
        return true;
    }
#endif
    return false;
}

int32_t AudioVolumeManager::SetSystemVolumeLevelWithOption(AudioStreamType streamType, int32_t volumeLevel,
    int32_t zoneId, const VolumeUpdateOption &info)
{
    AUDIO_INFO_LOG("streamType: %{public}d, volumeLevel: %{public}d, hasCheckLoudVolume: %{public}d,"
        " updateUi: %{public}d", streamType, volumeLevel, info.hasCheckLoudVolume, info.isUpdateUi);
    bool adjustable = audioPolicyManager_.IsVolumeUnadjustable();
    if (adjustable) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }

    if (VolumeUtils::IsNotificationFollowRingtone() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_NOTIFICATION) {
        AUDIO_INFO_LOG("notificationFollowRingtone_ is true. Use ringtone instead of notification");
        streamType = STREAM_RING;
    }

    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_NOTIFICATION &&
        audioPolicyManager_.GetRingerMode() != RINGER_MODE_NORMAL) {
        AUDIO_INFO_LOG("Cannot adjust the notification volume because of the SILENT or VIBRATE mode");
        return ERR_OPERATION_FAILED;
    }
#ifdef MULTI_USER_ZONE
    int32_t volumeDegree = info.volumeDegree >= 0 ? info.volumeDegree : VolumeUtils::VolumeLevelToDegree(
        volumeLevel, GetMaxVolumeLevel(streamType, DEVICE_TYPE_NONE, zoneId));
    CHECK_AND_RETURN_RET(!CheckVolumeLimitExceeded(streamType, IPCSkeleton::GetCallingUid(), volumeDegree, zoneId),
        ERR_OPERATION_FAILED);
#endif
    return SetSystemVolumeLevelWithOptionInternal(streamType, volumeLevel, zoneId, info);
}

int32_t AudioVolumeManager::SetSystemVolumeLevelWithOptionInternal(AudioStreamType streamType, int32_t volumeLevel,
    int32_t zoneId, const VolumeUpdateOption &info)
{
    bool mute = GetStreamMuteInterface(streamType, zoneId);
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    if (loudVolumeSupportMode_ != LOUD_VOLUME_NOT_SUPPORT && !info.hasCheckLoudVolume) {
        std::shared_ptr<AudioDeviceDescriptor> device = audioActiveDevice_.GetDeviceForVolume(streamType);
        CheckLoudVolumeMode(mute, volumeLevel, streamType);
    }
#endif
    VolumeUpdateOption option{info.isUpdateUi, mute, zoneId, info.volumeDegree};
    if (streamType == STREAM_ALL) {
        for (auto audioStreamType : GET_STREAM_ALL_VOLUME_TYPES) {
            bool isMutedForType = GetStreamMuteInterface(audioStreamType, zoneId);
            AUDIO_INFO_LOG("SetVolume of STREAM_ALL, SteamType = %{public}d, mute = %{public}d, level = %{public}d",
                audioStreamType, isMutedForType, volumeLevel);
            option.mute = isMutedForType;
            int32_t setResult = SetSingleStreamVolume(audioStreamType, volumeLevel, option);
            if (setResult != SUCCESS && setResult != ERR_SET_VOL_FAILED_BY_SAFE_VOL &&
                setResult != ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED) {
                return setResult;
            }
        }
        return SUCCESS;
    }

    if (VolumeUtils::IsNotificationFollowRingtone() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_RING) {
        // The notificationFollowRingtone is true. Keep the volume of notification and ringtone the same.
        int32_t setResult1 = SetSingleStreamVolume(STREAM_RING, volumeLevel, option);
        CHECK_AND_RETURN_RET_LOG(setResult1 == SUCCESS, setResult1, "Failed for ringtone: %{public}d", setResult1);
        int32_t setResult2 = SetSingleStreamVolume(STREAM_NOTIFICATION, volumeLevel, option);
        AUDIO_INFO_LOG("degree=%{public}d, notification result=%{public}d", option.volumeDegree, setResult2);
        return SUCCESS;
    }
    return SetSingleStreamVolume(streamType, volumeLevel, option);
}

void AudioVolumeManager::RefreshActiveDeviceVolume()
{
    audioPolicyManager_.UpdateVolumeForAllPipes();
}

void AudioVolumeManager::InitAudioZoneVolume(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    audioPolicyManager_.InitAudioZoneVolume(zoneId, desc);
}

int32_t AudioVolumeManager::SetAudioZoneVolumeConfig(const std::string &zoneName,
    const std::vector<AudioZoneVolumeConfig> &configs)
{
    return audioPolicyManager_.SetAudioZoneVolumeConfig(zoneName, configs);
}

void AudioVolumeManager::TriggerAudioZoneVolumeSync(int32_t zoneId)
{
    audioPolicyManager_.TriggerAudioZoneVolumeSync(zoneId);
}

void AudioVolumeManager::ProcUpdateRingerModeForMute(bool updateRingerMode, bool mute)
{
    if (updateRingerMode) {
        AudioRingerMode ringerMode = ringerModeManager_.GetRingerModeForMute(mute);
        AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring mute state", ringerMode);
        // Update ringer mode but no need to update mute state again.
        SetRingerMode(ringerMode, true);
    }
}

bool AudioVolumeManager::CheckCanMuteVolumeTypeByStep(AudioVolumeType volumeType, int32_t volumeLevel)
{
    if ((volumeLevel - volumeStep_) == 0 && !VolumeUtils::IsPCVolumeEnable() && (volumeType == STREAM_VOICE_ASSISTANT
        || volumeType == STREAM_VOICE_CALL || volumeType == STREAM_ALARM || volumeType == STREAM_ACCESSIBILITY ||
        volumeType == STREAM_VOICE_COMMUNICATION)) {
        return false;
    }
    return true;
}

bool AudioVolumeManager::ShouldNotifyForLegacyVolume(int32_t volumeLevel)
{
    int32_t notifyVolumeLevel = VolumeUtils::GetLegacyVolumeNotificationLevel();
    if (notifyVolumeLevel <= 0) {
        AUDIO_DEBUG_LOG("legacy high volume notification not enable");
        return false;
    }
    return volumeLevel > notifyVolumeLevel && !IsLegacyHighVolumeChecked();
}

int32_t AudioVolumeManager::SetSingleStreamVolumeWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, DeviceType deviceType)
{
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "desc is nullptr");
    DeviceType curOutputDeviceType = desc->deviceType_;

    if (VolumeUtils::IsNotificationFollowRingtone() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_NOTIFICATION) {
        AUDIO_INFO_LOG("notificationFollowRingtone_ is true. Use ringtone instead of notification");
        streamType = STREAM_RING;
    }

    int32_t ret = SUCCESS;
    if (curOutputDeviceType != deviceType) {
        ret = SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    } else {
        bool mute = GetStreamMuteInterface(streamType);
        VolumeUpdateOption option{isUpdateUi, mute};
        ret = SetSingleStreamVolume(streamType, volumeLevel, option);
    }

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to set volume: %{public}d", ret);
    if (VolumeUtils::IsNotificationFollowRingtone() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_RING) {
        int32_t resultOfNotification = SUCCESS;
        if (curOutputDeviceType != deviceType) {
            resultOfNotification = SaveSpecifiedDeviceVolume(STREAM_NOTIFICATION, volumeLevel, deviceType);
        } else {
            bool mute = GetStreamMuteInterface(STREAM_NOTIFICATION);
            VolumeUpdateOption option{isUpdateUi, mute};
            resultOfNotification = SetSingleStreamVolume(STREAM_NOTIFICATION, volumeLevel, option);
        }
        AUDIO_INFO_LOG(" Set notification volume result: %{public}d", resultOfNotification);
    }
    return ret;
}

void AudioVolumeManager::UpdateSystemMuteStateAccordingMusicState(AudioStreamType streamType, bool mute,
    bool isUpdateUi)
{
    // This function only applies to mute/unmute scenarios where the input type is music on the PC platform
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType != AudioStreamType::STREAM_MUSIC || !VolumeUtils::IsPCVolumeEnable()) {
        return;
    }
    if (mute && !GetStreamMuteInterface(STREAM_SYSTEM)) {
        // If the STREAM_MUSIC wants mute, synchronize the mute STREAM_SYSTEM
        SetStreamMute(STREAM_SYSTEM, mute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is mute or volume change to 0 and need mute system stream");
    } else if (!mute && GetStreamMuteInterface(STREAM_SYSTEM)) {
        // If you STREAM_MUSIC unmute, you need to determine whether the volume is 0
        // if it is 0, the prompt sound will continue to be mute, and if it is not 0
        // you need to synchronize the unmute prompt sound
        bool isMute = (GetSystemVolumeLevelInterface(STREAM_MUSIC) == 0) ? true : false;
        SetStreamMute(STREAM_SYSTEM, isMute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is unmute and volume is 0 and need %{public}d system stream", isMute);
    }
}

bool AudioVolumeManager::CheckLoudVolumeMode(int32_t volLevel, int32_t keyType, AudioStreamType streamInFocus)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = audioActiveDevice_.GetDeviceForVolume();
    CHECK_AND_RETURN_RET_LOG(loudVolumeManager_ != nullptr, false, "loudVolumeManager_ is nullptr");
    return loudVolumeManager_->CheckLoudVolumeMode(volLevel, keyType, streamInFocus, deviceDesc);
}

bool AudioVolumeManager::ReloadLoudVolumeMode(
    const AudioStreamType streamInFocus, SetLoudVolMode setVolMode)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(streamInFocus);
    CHECK_AND_RETURN_RET_LOG(loudVolumeManager_ != nullptr && deviceDesc != nullptr, false,
        "loudVolumeManager_ or deviceDesc is nullptr");
    return loudVolumeManager_->ReloadLoudVolumeMode(streamInFocus, setVolMode, deviceDesc);
}

int32_t AudioVolumeManager::SetDoubleRingVolumeDb(const AudioStreamType &streamType,
    const int32_t &volumeLevel)
{
    auto deviceDesc = audioActiveDevice_.GetDeviceForVolume(streamType);
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    return audioPolicyManager_.SetDoubleRingVolumeDb(streamType, volumeLevel, deviceDesc->deviceType_);
}
}
}
