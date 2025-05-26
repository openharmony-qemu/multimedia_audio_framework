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
#define LOG_TAG "AudioPolicyService"
#endif

#include "audio_policy_service.h"
#include <ability_manager_client.h>
#include <dlfcn.h>
#include "iservice_registry.h"

#include "audio_manager_listener_stub.h"
#include "parameter.h"
#include "parameters.h"
#include "data_share_observer_callback.h"
#include "device_init_callback.h"
#include "audio_inner_call.h"
#ifdef FEATURE_DEVICE_MANAGER
#endif

#include "audio_affinity_manager.h"
#include "audio_spatialization_service.h"
#include "audio_converter_parser.h"
#include "media_monitor_manager.h"
#include "client_type_manager.h"
#include "audio_safe_volume_notification.h"
#include "audio_setting_provider.h"
#include "audio_spatialization_service.h"
#include "audio_usb_manager.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_policy_global_parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

namespace {
static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
static const char* PREDICATES_STRING = "settings.general.device_name";
static const char* SETTINGS_DATA_BASE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const char* SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static const char* AUDIO_SERVICE_PKG = "audio_manager_service";
constexpr int32_t BOOTUP_MUSIC_UID = 1003;
}

static const std::vector<AudioVolumeType> VOLUME_TYPE_LIST = {
    STREAM_VOICE_CALL,
    STREAM_RING,
    STREAM_MUSIC,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_SYSTEM,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_ALL
};

static const char* CONFIG_AUDIO_BALANACE_KEY = "master_balance";
static const char* CONFIG_AUDIO_MONO_KEY = "master_mono";
const int32_t UID_AUDIO = 1041;
static const int64_t WATI_PLAYBACK_TIME = 200000; // 200ms
static const uint32_t DEVICE_CONNECTED_FLAG_DURATION_MS = 3000000; // 3s

static int16_t IsDistributedOutput(const AudioDeviceDescriptor &desc)
{
    return (desc.deviceType_ == DEVICE_TYPE_SPEAKER && desc.networkId_ != LOCAL_NETWORK_ID) ? 1 : 0;
}

mutex g_dataShareHelperMutex;
#ifdef BLUETOOTH_ENABLE
mutex g_btProxyMutex;
#endif
bool AudioPolicyService::isBtListenerRegistered = false;
bool AudioPolicyService::isBtCrashed = false;
mutex g_policyMgrListenerMutex;

AudioPolicyService::~AudioPolicyService()
{
    AUDIO_INFO_LOG("~AudioPolicyService()");
    Deinit();
}

bool AudioPolicyService::LoadAudioPolicyConfig()
{
    bool ret = audioConfigManager_.Init();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Policy Config Load Configuration failed");
        audioCapturerSession_.SetConfigParserFlag();
    }
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Policy Config Load Configuration failed");
    audioCapturerSession_.SetConfigParserFlag();
    isFastControlled_ = getFastControlParam();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Config Parse failed");
    }
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Config Parse failed");
    SetDefaultAdapterEnable(audioConfigManager_.GetDefaultAdapterEnable());
    return ret;
}

bool AudioPolicyService::Init(void)
{
    serviceFlag_.reset();
    audioPolicyManager_.Init();
    audioEffectService_.EffectServiceInit();
    audioDeviceManager_.ParseDeviceXml();
    audioAffinityManager_.ParseAffinityXml();
#ifdef AUDIO_WIRED_DETECT
    audioPnpServer_.init();
#endif
    audioGlobalConfigManager_.ParseGlobalConfigXml();
    audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (audioA2dpOffloadManager_ != nullptr) {audioA2dpOffloadManager_->Init();}

    bool ret = LoadAudioPolicyConfig();
    if (!ret) {
        return ret;
    }

#ifdef FEATURE_DTMF_TONE
    ret = audioToneManager_.LoadToneDtmfConfig();
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Tone Load Configuration failed");
#endif

    int32_t status = deviceStatusListener_->RegisterDeviceStatusListener();
    if (status != SUCCESS) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("[Policy Service] Register for device status "
            "events failed");
    }
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, false, "[Policy Service] Register for device status events failed");

    audioVolumeManager_.Init(audioPolicyServerHandler_);
    audioDeviceCommon_.Init(audioPolicyServerHandler_);
    audioRecoveryDevice_.Init(audioA2dpOffloadManager_);

    audioDeviceStatus_.Init(audioA2dpOffloadManager_, audioPolicyServerHandler_);
    audioDeviceLock_.Init(audioA2dpOffloadManager_);
    audioCapturerSession_.Init(audioA2dpOffloadManager_);

    CreateRecoveryThread();
    std::string versionType = OHOS::system::GetParameter("const.logsystem.versiontype", "commercial");
    AudioDump::GetInstance().SetVersionType(versionType);

    int32_t ecEnableState = system::GetBoolParameter("const.multimedia.audio.fwk_ec.enable", 0);
    int32_t micRefEnableState = system::GetBoolParameter("const.multimedia.audio.fwk_pnr.enable", 0);

    audioEcManager_.Init(ecEnableState, micRefEnableState);
#ifdef HAS_FEATURE_INNERCAPTURER
    AudioServerProxy::GetInstance().SetInnerCapLimitProxy(audioGlobalConfigManager_.GetCapLimit());
#endif
    return true;
}

void AudioPolicyService::CreateRecoveryThread()
{
    if (RecoveryDevicesThread_ != nullptr) {
        RecoveryDevicesThread_->detach();
    }
    RecoveryDevicesThread_ = std::make_unique<std::thread>([this] {
        this->RecoverExcludedOutputDevices();
        this->RecoveryPreferredDevices();
    });
    pthread_setname_np(RecoveryDevicesThread_->native_handle(), "APSRecovery");
}

void AudioPolicyService::RecoverExcludedOutputDevices()
{
    audioRecoveryDevice_.RecoverExcludedOutputDevices();
}

void AudioPolicyService::RecoveryPreferredDevices()
{
    return audioRecoveryDevice_.RecoveryPreferredDevices();
}

void AudioPolicyService::InitKVStore()
{
    audioVolumeManager_.InitKVStore();
}

void AudioPolicyService::SettingsDataReady()
{
    AudioServerProxy::GetInstance().NotifySettingsDataReady();
}

bool AudioPolicyService::ConnectServiceAdapter()
{
    bool ret = audioPolicyManager_.ConnectServiceAdapter();
    CHECK_AND_RETURN_RET_LOG(ret, false, "Error in connecting to audio service adapter");

    OnServiceConnected(AudioServiceIndex::AUDIO_SERVICE_INDEX);

    return true;
}

void AudioPolicyService::Deinit(void)
{
    AUDIO_WARNING_LOG("Policy service died. closing active ports");
    std::unordered_map<std::string, AudioIOHandle> mapCopy = audioIOHandleMap_.GetCopy();
    std::for_each(mapCopy.begin(), mapCopy.end(), [&](std::pair<std::string, AudioIOHandle> handle) {
        audioPolicyManager_.CloseAudioPort(handle.second);
    });
    audioPolicyManager_.Deinit();
    audioIOHandleMap_.DeInit();
    deviceStatusListener_->UnRegisterDeviceStatusListener();
#ifdef AUDIO_WIRED_DETECT
    audioPnpServer_.StopPnpServer();
#endif

    if (isBtListenerRegistered) {
        UnregisterBluetoothListener();
    }

    audioVolumeManager_.DeInit();
    if (RecoveryDevicesThread_ != nullptr && RecoveryDevicesThread_->joinable()) {
        RecoveryDevicesThread_->join();
        RecoveryDevicesThread_.reset();
        RecoveryDevicesThread_ = nullptr;
    }

    audioDeviceCommon_.DeInit();
    audioRecoveryDevice_.DeInit();
    audioDeviceStatus_.DeInit();
    audioDeviceLock_.DeInit();
    audioCapturerSession_.DeInit();
    return;
}

int32_t AudioPolicyService::SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback)
{
    return audioPolicyManager_.SetAudioStreamRemovedCallback(callback);
}

int32_t AudioPolicyService::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "SetAudioDeviceRefinerCallback object is nullptr");
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return deviceStatusListener_->SetAudioDeviceAnahsCallback(object);
}

int32_t AudioPolicyService::UnsetAudioDeviceAnahsCallback()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return deviceStatusListener_->UnsetAudioDeviceAnahsCallback();
}

int32_t AudioPolicyService::GetMaxVolumeLevel(AudioVolumeType volumeType) const
{
    return audioVolumeManager_.GetMaxVolumeLevel(volumeType);
}

int32_t AudioPolicyService::GetMinVolumeLevel(AudioVolumeType volumeType) const
{
    return audioVolumeManager_.GetMinVolumeLevel(volumeType);
}

void SafeVolumeEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    if (eventReceiver_ == nullptr) {
        AUDIO_ERR_LOG("eventReceiver_ is nullptr.");
        return;
    }
    AUDIO_INFO_LOG("receive DATA_SHARE_READY action success.");
    eventReceiver_(eventData);
}

void AudioPolicyService::SubscribeSafeVolumeEvent()
{
    AUDIO_INFO_LOG("enter");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(AUDIO_RESTORE_VOLUME_EVENT);
    matchingSkills.AddEvent(AUDIO_INCREASE_VOLUME_EVENT);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto commonSubscribePtr = std::make_shared<SafeVolumeEventSubscriber>(subscribeInfo,
        std::bind(&AudioPolicyService::OnReceiveEvent, this, std::placeholders::_1));
    if (commonSubscribePtr == nullptr) {
        AUDIO_ERR_LOG("commonSubscribePtr is nullptr");
        return;
    }
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonSubscribePtr);
}

void AudioPolicyService::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    audioVolumeManager_.OnReceiveEvent(eventData);
}

int32_t AudioPolicyService::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel)
{
    return audioVolumeManager_.SetSystemVolumeLevel(streamType, volumeLevel);
}

int32_t AudioPolicyService::SetSystemVolumeLevelWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    return audioVolumeManager_.SetSystemVolumeLevelWithDevice(streamType, volumeLevel, deviceType);
}

int32_t AudioPolicyService::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    // update dump appvolume
    audioDeviceLock_.UpdateAppVolume(appUid, volumeLevel);
    return audioVolumeManager_.SetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioPolicyService::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    return audioVolumeManager_.SetAppVolumeMuted(appUid, muted);
}

bool AudioPolicyService::IsAppVolumeMute(int32_t appUid, bool owned)
{
    return audioVolumeManager_.IsAppVolumeMute(appUid, owned);
}

int32_t AudioPolicyService::SetVoiceRingtoneMute(bool isMute)
{
    return audioVolumeManager_.SetVoiceRingtoneMute(isMute);
}

int32_t AudioPolicyService::GetSystemVolumeLevel(AudioStreamType streamType)
{
    return audioVolumeManager_.GetSystemVolumeLevel(streamType);
}

int32_t AudioPolicyService::GetAppVolumeLevel(int32_t appUid)
{
    return audioVolumeManager_.GetAppVolumeLevel(appUid);
}

int32_t AudioPolicyService::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    return audioVolumeManager_.GetSystemVolumeLevelNoMuteState(streamType);
}

float AudioPolicyService::GetSystemVolumeDb(AudioStreamType streamType) const
{
    return audioPolicyManager_.GetSystemVolumeDb(streamType);
}

int32_t AudioPolicyService::SetLowPowerVolume(int32_t streamId, float volume) const
{
    return streamCollector_.SetLowPowerVolume(streamId, volume);
}

float AudioPolicyService::GetLowPowerVolume(int32_t streamId) const
{
    return streamCollector_.GetLowPowerVolume(streamId);
}

void AudioPolicyService::OffloadStreamSetCheck(uint32_t sessionId)
{
    Trace trace("AudioPolicyService::OffloadStreamSetCheck: sessionid:" + std::to_string(sessionId));
    audioOffloadStream_.OffloadStreamSetCheck(sessionId);
    return;
}

void AudioPolicyService::OffloadStreamReleaseCheck(uint32_t sessionId)
{
    audioOffloadStream_.OffloadStreamReleaseCheck(sessionId);
    return;
}

void AudioPolicyService::HandlePowerStateChanged(PowerMgr::PowerState state)
{
    audioOffloadStream_.HandlePowerStateChanged(state);
}

float AudioPolicyService::GetSingleStreamVolume(int32_t streamId) const
{
    return streamCollector_.GetSingleStreamVolume(streamId);
}

int32_t AudioPolicyService::SetStreamMute(AudioStreamType streamType, bool mute, const StreamUsage &streamUsage,
    const DeviceType &deviceType)
{
    return audioVolumeManager_.SetStreamMute(streamType, mute, streamUsage, deviceType);
}

int32_t AudioPolicyService::SetSourceOutputStreamMute(int32_t uid, bool setMute) const
{
    int32_t status = audioPolicyManager_.SetSourceOutputStreamMute(uid, setMute);
    if (status > 0) {
        streamCollector_.UpdateCapturerInfoMuteStatus(uid, setMute);
    }
    return status;
}

bool AudioPolicyService::GetStreamMute(AudioStreamType streamType)
{
    return audioVolumeManager_.GetStreamMute(streamType);
}

std::string AudioPolicyService::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType)
{
    (void)streamType;

    std::string selectedDevice = audioRouteMap_.GetDeviceInfoByUidAndPid(uid, pid);
    if (selectedDevice == "") {
        return selectedDevice;
    }

    if (LOCAL_NETWORK_ID == selectedDevice) {
        AUDIO_INFO_LOG("uid[%{public}d]-->local.", uid);
        return "";
    }
    // check if connected.
    if (audioConnectedDevice_.CheckDeviceConnected(selectedDevice)) {
        AUDIO_INFO_LOG("result[%{public}s]", selectedDevice.c_str());
        return selectedDevice;
    } else {
        audioRouteMap_.DelRouteMapInfoByKey(uid);
        AUDIO_INFO_LOG("device already disconnected.");
        return "";
    }
}

void AudioPolicyService::NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value)
{
    audioDeviceLock_.NotifyRemoteRenderState(networkId, condition, value);
}

bool AudioPolicyService::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    return audioDeviceLock_.IsArmUsbDevice(desc);
}

void AudioPolicyService::RestoreSession(const uint32_t &sessionID, RestoreInfo restoreInfo)
{
    AudioServerProxy::GetInstance().RestoreSessionProxy(sessionID, restoreInfo);
}

int32_t AudioPolicyService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("AudioPolicyService::SelectOutputDevice");
    if (!selectedDesc.empty() && selectedDesc[0]) {
        int16_t isDistOld = IsDistributedOutput(audioActiveDevice_.GetCurrentOutputDevice());
        int16_t isDistNew = IsDistributedOutput(selectedDesc[0]);
        AUDIO_INFO_LOG("Entry. Check Distributed Output Change[%{public}d-->%{public}d]", isDistOld, isDistNew);
        int16_t flag = isDistNew - isDistOld;
        if (audioPolicyServerHandler_ && flag != 0) {
            audioPolicyServerHandler_->SendDistribuitedOutputChangeEvent(selectedDesc[0], flag > 0);
        }
    }
    return audioDeviceLock_.SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioPolicyService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return audioDeviceLock_.SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioPolicyService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    Trace trace("AudioPolicyService::ExcludeOutputDevices");
    return audioDeviceLock_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioPolicyService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    return audioDeviceLock_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    return audioDeviceLock_.GetExcludedDevices(audioDevUsage);
}

bool AudioPolicyService::IsStreamActive(AudioStreamType streamType) const
{
    return audioSceneManager_.IsStreamActive(streamType);
}

void AudioPolicyService::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    StoreDistributedRoutingRoleInfo(descriptor, type);
    audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReason::OVERRODE);
    audioDeviceCommon_.FetchDevice(false);
}

void AudioPolicyService::StoreDistributedRoutingRoleInfo(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    distributedRoutingInfo_.descriptor = descriptor;
    distributedRoutingInfo_.type = type;
}

DistributedRoutingInfo AudioPolicyService::GetDistributedRoutingRoleInfo()
{
    return distributedRoutingInfo_;
}

int32_t AudioPolicyService::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    return audioCapturerSession_.SetWakeUpAudioCapturerFromAudioServer(config);
}

int32_t AudioPolicyService::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    int32_t error = SUCCESS;
    audioPolicyServerHandler_->SendCapturerCreateEvent(capturerInfo, streamInfo, sessionId, true, error);
    return error;
}

int32_t AudioPolicyService::NotifyWakeUpCapturerRemoved()
{
    audioPolicyServerHandler_->SendWakeupCloseEvent(false);
    return SUCCESS;
}

bool AudioPolicyService::IsAbsVolumeSupported()
{
    return IsAbsVolumeScene();
}

int32_t AudioPolicyService::CloseWakeUpAudioCapturer()
{
    return audioCapturerSession_.CloseWakeUpAudioCapturer();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetDevices(DeviceFlag deviceFlag)
{
    return audioDeviceLock_.GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetDevicesInner(DeviceFlag deviceFlag)
{
    return audioConnectedDevice_.GetDevicesInner(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    return audioDeviceLock_.GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    return audioDeviceLock_.GetPreferredInputDeviceDescriptors(captureInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    return audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredInputDeviceDescInner(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    return audioDeviceCommon_.GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (audioRendererFilter->uid != -1) {
        shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetRendererDevice(audioRendererFilter->uid);
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*preferredDesc);
        deviceList.push_back(devDesc);
    }
    return deviceList;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (audioCapturerFilter->uid != -1) {
        shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetCapturerDevice(audioCapturerFilter->uid);
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*preferredDesc);
        deviceList.push_back(devDesc);
    }
    return deviceList;
}

int32_t AudioPolicyService::SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable)
{
    if (audioPolicyServerHandler_ != nullptr) {
        return audioPolicyServerHandler_->SetClientCallbacksEnable(callbackchange, enable);
    } else {
        AUDIO_ERR_LOG("audioPolicyServerHandler_ is nullptr");
        return AUDIO_ERR;
    }
}

int32_t AudioPolicyService::SetMicrophoneMute(bool isMute)
{
    return audioMicrophoneDescriptor_.SetMicrophoneMute(isMute);
}

int32_t AudioPolicyService::SetMicrophoneMutePersistent(const bool isMute)
{
    return audioMicrophoneDescriptor_.SetMicrophoneMutePersistent(isMute);
}

bool AudioPolicyService::GetPersistentMicMuteState()
{
    return audioMicrophoneDescriptor_.GetPersistentMicMuteState();
}

int32_t AudioPolicyService::InitPersistentMicrophoneMuteState(bool &isMute)
{
    return audioMicrophoneDescriptor_.InitPersistentMicrophoneMuteState(isMute);
}

bool AudioPolicyService::IsMicrophoneMute()
{
    return audioMicrophoneDescriptor_.IsMicrophoneMute();
}

int32_t AudioPolicyService::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return audioPolicyManager_.SetSystemSoundUri(key, uri);
}

std::string AudioPolicyService::GetSystemSoundUri(const std::string &key)
{
    return audioPolicyManager_.GetSystemSoundUri(key);
}

int32_t AudioPolicyService::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    return audioDeviceLock_.SetDeviceActive(deviceType, active, uid);
}

bool AudioPolicyService::IsDeviceActive(InternalDeviceType deviceType)
{
    return audioActiveDevice_.IsDeviceActive(deviceType);
}

DeviceType AudioPolicyService::GetActiveOutputDevice()
{
    return audioActiveDevice_.GetCurrentOutputDeviceType();
}

shared_ptr<AudioDeviceDescriptor> AudioPolicyService::GetActiveOutputDeviceDescriptor()
{
    return make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice());
}

DeviceType AudioPolicyService::GetActiveInputDevice()
{
    return audioActiveDevice_.GetCurrentInputDeviceType();
}

int32_t AudioPolicyService::SetRingerMode(AudioRingerMode ringMode)
{
    int32_t result = audioPolicyManager_.SetRingerMode(ringMode);
    if (result == SUCCESS) {
        if (Util::IsRingerAudioScene(audioSceneManager_.GetAudioScene(true))) {
            AUDIO_INFO_LOG("fetch output device after switch new ringmode.");
            audioDeviceCommon_.FetchDevice(true);
        }
        Volume vol = {false, 1.0f, 0};
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        vol.isMute = (ringMode == RINGER_MODE_NORMAL) ? false : true;
        vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevel(STREAM_RING));
        vol.volumeFloat = GetSystemVolumeInDb(STREAM_RING, vol.volumeInt, curOutputDeviceType);
        audioVolumeManager_.SetSharedVolume(STREAM_RING, curOutputDeviceType, vol);
    }
    return result;
}

AudioRingerMode AudioPolicyService::GetRingerMode() const
{
    return audioPolicyManager_.GetRingerMode();
}

int32_t AudioPolicyService::SetAudioScene(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    return audioDeviceLock_.SetAudioScene(audioScene, uid, pid);
}

AudioScene AudioPolicyService::GetAudioScene(bool hasSystemPermission) const
{
    return audioSceneManager_.GetAudioScene(hasSystemPermission);
}

AudioScene AudioPolicyService::GetLastAudioScene() const
{
    return audioSceneManager_.GetLastAudioScene();
}

void AudioPolicyService::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    deviceStatusListener_->UpdateAnahsPlatformType(anahsShowType);
}

void AudioPolicyService::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    audioDeviceLock_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioPolicyService::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    audioDeviceLock_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioPolicyService::ResetToSpeaker(DeviceType devType)
{
    if (devType != audioActiveDevice_.GetCurrentOutputDeviceType()) {
        return;
    }
    if (devType == DEVICE_TYPE_BLUETOOTH_SCO || devType == DEVICE_TYPE_USB_HEADSET ||
        devType == DEVICE_TYPE_WIRED_HEADSET || devType == DEVICE_TYPE_WIRED_HEADPHONES) {
        audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);
    }
}

void AudioPolicyService::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    audioDeviceLock_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioPolicyService::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    audioDeviceLock_.OnDeviceStatusUpdated(updatedDesc, isConnected);
}

#ifdef FEATURE_DTMF_TONE
std::vector<int32_t> AudioPolicyService::GetSupportedTones(const std::string &countryCode)
{
    return audioToneManager_.GetSupportedTones(countryCode);
}

std::shared_ptr<ToneInfo> AudioPolicyService::GetToneConfig(int32_t ltonetype, const std::string &countryCode)
{
    return audioToneManager_.GetToneConfig(ltonetype, countryCode);
}
#endif
void AudioPolicyService::UpdateA2dpOffloadFlagBySpatialService(
    const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap)
{
    DeviceType spatialDevice = audioDeviceCommon_.GetSpatialDeviceType(macAddress);
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatializationEnableMap, spatialDevice);
    }
}

void AudioPolicyService::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    audioDeviceLock_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioPolicyService::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    audioDeviceLock_.SetDisplayName(deviceName, isLocalDevice);
}

void AudioPolicyService::SetDmDeviceType(const uint16_t dmDeviceType)
{
    audioDeviceLock_.SetDmDeviceType(dmDeviceType);
}

void AudioPolicyService::RegisterRemoteDevStatusCallback()
{
#ifdef FEATURE_DEVICE_MANAGER
    std::shared_ptr<DistributedHardware::DmInitCallback> initCallback = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(AUDIO_SERVICE_PKG, initCallback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Init device manage failed");
    std::shared_ptr<DistributedHardware::DeviceStatusCallback> callback = std::make_shared<DeviceStatusCallbackImpl>();
    DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(AUDIO_SERVICE_PKG, "", callback);
    AUDIO_INFO_LOG("Done");
#endif
}

std::shared_ptr<DataShare::DataShareHelper> AudioPolicyService::CreateDataShareHelperInstance()
{
    return AudioPolicyUtils::GetInstance().CreateDataShareHelperInstance();
}

int32_t AudioPolicyService::GetDeviceNameFromDataShareHelper(std::string &deviceName)
{
    return AudioPolicyUtils::GetInstance().GetDeviceNameFromDataShareHelper(deviceName);
}

bool AudioPolicyService::IsDataShareReady()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, false, "[Policy Service] Get samgr failed.");
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, false, "[Policy Service] audio service remote object is NULL.");
    WatchTimeout guard("DataShare::DataShareHelper::Create:IsDataShareReady", CALL_IPC_COST_TIME_MS);
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> res = DataShare::DataShareHelper::Create(remoteObject,
        SETTINGS_DATA_BASE_URI, SETTINGS_DATA_EXT_URI);
    guard.CheckCurrTimeout();
    if (res.first == DataShare::E_OK) {
        AUDIO_INFO_LOG("DataShareHelper is ready.");
        auto helper = res.second;
        if (helper != nullptr) {
            helper->Release();
        }
        return true;
    } else {
        AUDIO_WARNING_LOG("DataShareHelper::Create failed: E_DATA_SHARE_NOT_READY");
        return false;
    }
}

void AudioPolicyService::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    audioPolicyManager_.SetDataShareReady(std::atomic_load(&isDataShareReady));
}

void AudioPolicyService::SetFirstScreenOn()
{
    audioDeviceCommon_.SetFirstScreenOn();
}

int32_t AudioPolicyService::SetVirtualCall(const bool isVirtual)
{
    return audioDeviceCommon_.SetVirtualCall(isVirtual);
}

void AudioPolicyService::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    AudioServerProxy::GetInstance().GetAllSinkInputsProxy(sinkInputs);
}

void AudioPolicyService::RegisterNameMonitorHelper()
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper
        = AudioPolicyUtils::GetInstance().CreateDataShareHelperInstance();
    CHECK_AND_RETURN_LOG(dataShareHelper != nullptr, "dataShareHelper is NULL");

    auto uri = std::make_shared<Uri>(std::string(SETTINGS_DATA_BASE_URI) + "&key=" + PREDICATES_STRING);
    sptr<AAFwk::DataAbilityObserverStub> settingDataObserver = std::make_unique<DataShareObserverCallBack>().release();
    dataShareHelper->RegisterObserver(*uri, settingDataObserver);

    dataShareHelper->Release();
}

void AudioPolicyService::RegisterAccessibilityMonitorHelper()
{
    RegisterAccessiblilityBalance();
    RegisterAccessiblilityMono();
}

void AudioPolicyService::RegisterAccessiblilityBalance()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncBalance = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        float balance = 0;
        int32_t ret = settingProvider.GetFloatValue(CONFIG_AUDIO_BALANACE_KEY, balance, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get balance value failed");
        if (balance < -1.0f || balance > 1.0f) {
            AUDIO_WARNING_LOG("audioBalance value is out of range [-1.0, 1.0]");
        } else {
            OnAudioBalanceChanged(balance);
        }
    };
    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_BALANACE_KEY, updateFuncBalance);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver balance failed");
    }
    AUDIO_INFO_LOG("Register accessibility balance successfully");
}

void AudioPolicyService::RegisterAccessiblilityMono()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncMono = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        int32_t value = 0;
        ErrCode ret = settingProvider.GetIntValue(CONFIG_AUDIO_MONO_KEY, value, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get mono value failed");
        OnMonoAudioConfigChanged(value != 0);
    };
    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_MONO_KEY, updateFuncMono);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver mono failed");
    }
    AUDIO_INFO_LOG("Register accessibility mono successfully");
}

void AudioPolicyService::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    audioDeviceLock_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioPolicyService::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    AUDIO_INFO_LOG("[module_load]::OnServiceConnected for [%{public}d]", serviceIndex);
    CHECK_AND_RETURN_LOG(serviceIndex >= HDI_SERVICE_INDEX && serviceIndex <= AUDIO_SERVICE_INDEX, "invalid index");

    // If audio service or hdi service is not ready, donot load default modules
    lock_guard<mutex> lock(serviceFlagMutex_);
    serviceFlag_.set(serviceIndex, true);
    if (serviceFlag_.count() != MIN_SERVICE_COUNT) {
        AUDIO_INFO_LOG("[module_load]::hdi service or audio service not up. Cannot load default module now");
        return;
    }

    int32_t ret = audioDeviceLock_.OnServiceConnected(serviceIndex);
    if (ret == SUCCESS) {
#ifdef USB_ENABLE
        AudioUsbManager::GetInstance().Init(this);
#endif
        audioEffectService_.SetMasterSinkAvailable();
    }
    // RegisterBluetoothListener() will be called when bluetooth_host is online
    // load hdi-effect-model
    LoadHdiEffectModel();
    AudioServerProxy::GetInstance().NotifyAudioPolicyReady();
}

void AudioPolicyService::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Start for [%{public}d]", serviceIndex);
}

void AudioPolicyService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    audioDeviceLock_.OnForcedDeviceSelected(devType, macAddress);
}

void AudioPolicyService::OnMonoAudioConfigChanged(bool audioMono)
{
    AUDIO_INFO_LOG("audioMono = %{public}s", audioMono? "true": "false");
    AudioServerProxy::GetInstance().SetAudioMonoStateProxy(audioMono);
}

void AudioPolicyService::OnAudioBalanceChanged(float audioBalance)
{
    AUDIO_INFO_LOG("audioBalance = %{public}f", audioBalance);
    AudioServerProxy::GetInstance().SetAudioBalanceValueProxy(audioBalance);
}

void AudioPolicyService::LoadEffectLibrary()
{
    // IPC -> audioservice load library
    OriginalEffectConfig oriEffectConfig = {};
    audioEffectService_.GetOriginalEffectConfig(oriEffectConfig);
    vector<Effect> successLoadedEffects;

    bool loadSuccess = AudioServerProxy::GetInstance().LoadAudioEffectLibrariesProxy(oriEffectConfig.libraries,
        oriEffectConfig.effects, successLoadedEffects);
    if (!loadSuccess) {
        AUDIO_ERR_LOG("Load audio effect failed, please check log");
    }

    audioEffectService_.UpdateAvailableEffects(successLoadedEffects);
    audioEffectService_.BuildAvailableAEConfig();

    // Initialize EffectChainManager in audio service through IPC
    SupportedEffectConfig supportedEffectConfig;
    audioEffectService_.GetSupportedEffectConfig(supportedEffectConfig);
    EffectChainManagerParam effectChainManagerParam;
    EffectChainManagerParam enhanceChainManagerParam;
    audioEffectService_.ConstructEffectChainManagerParam(effectChainManagerParam);
    audioEffectService_.ConstructEnhanceChainManagerParam(enhanceChainManagerParam);

    bool ret = AudioServerProxy::GetInstance().CreateEffectChainManagerProxy(supportedEffectConfig.effectChains,
        effectChainManagerParam, enhanceChainManagerParam);
    CHECK_AND_RETURN_LOG(ret, "EffectChainManager create failed");

    audioEffectService_.SetEffectChainManagerAvailable();
    AudioSpatializationService::GetAudioSpatializationService().Init(supportedEffectConfig.effectChains);
}

void AudioPolicyService::AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb)
{
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, cb);
    }
}

void AudioPolicyService::ReduceAudioPolicyClientProxyMap(pid_t clientPid)
{
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(clientPid);
    }
}

int32_t AudioPolicyService::SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
    const sptr<IRemoteObject> &object, bool hasBTPermission)
{
    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        callback->hasBTPermission_ = hasBTPermission;

        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientId, usage, callback);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyService::SetQueryClientTypeCallback(const sptr<IRemoteObject> &object)
{
#ifdef FEATURE_APPGALLERY
    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        ClientTypeManager::GetInstance()->SetQueryClientTypeCallback(callback);
    } else {
        AUDIO_ERR_LOG("Client type callback is null");
    }
#endif
    return SUCCESS;
}

int32_t AudioPolicyService::SetAudioClientInfoMgrCallback(const sptr<IRemoteObject> &object)
{
    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        return audioStateManager_.SetAudioClientInfoMgrCallback(callback);
    } else {
        AUDIO_ERR_LOG("Client info manager callback is null");
    }
    return SUCCESS;
}

int32_t AudioPolicyService::UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage)
{
    AUDIO_INFO_LOG("Start");
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientId, usage);
    }
    return SUCCESS;
}

static void UpdateCapturerInfoWhenNoPermission(const shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfos,
    bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioCapturerChangeInfos->clientUID = 0;
        audioCapturerChangeInfos->capturerState = CAPTURER_INVALID;
    }
}

int32_t AudioPolicyService::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    return audioDeviceLock_.RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioPolicyService::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    return audioDeviceLock_.UpdateTracker(mode, streamChangeInfo);
}

void AudioPolicyService::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    audioDeviceLock_.FetchOutputDeviceForTrack(streamChangeInfo, reason);
}

void AudioPolicyService::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    audioDeviceLock_.FetchInputDeviceForTrack(streamChangeInfo);
}

int32_t AudioPolicyService::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    return audioDeviceLock_.GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission,
        hasSystemPermission);
}

int32_t AudioPolicyService::GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>>
    &audioCapturerChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    int status = streamCollector_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get capturer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDevices = GetDevicesInner(INPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    DeviceRole activeDeviceRole = INPUT_DEVICE;
    for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            size_t capturerInfosSize = audioCapturerChangeInfos.size();
            for (size_t i = 0; i < capturerInfosSize; i++) {
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
            }
            break;
        }
    }

    return status;
}

void AudioPolicyService::RegisteredTrackerClientDied(pid_t uid)
{
    audioDeviceLock_.RegisteredTrackerClientDied(uid);
}

int32_t AudioPolicyService::ReconfigureAudioChannel(const uint32_t &channelCount, DeviceType deviceType)
{
    if (audioActiveDevice_.GetCurrentOutputDeviceType() != DEVICE_TYPE_FILE_SINK) {
        AUDIO_INFO_LOG("FILE_SINK_DEVICE is not active. Cannot reconfigure now");
        return ERROR;
    }

    std::string module = FILE_SINK;

    if (deviceType == DeviceType::DEVICE_TYPE_FILE_SINK) {
        CHECK_AND_RETURN_RET_LOG(channelCount <= CHANNEL_8 && channelCount >= MONO, ERROR, "Invalid sink channel");
        module = FILE_SINK;
    } else if (deviceType == DeviceType::DEVICE_TYPE_FILE_SOURCE) {
        CHECK_AND_RETURN_RET_LOG(channelCount <= CHANNEL_6 && channelCount >= MONO, ERROR, "Invalid src channel");
        module = FILE_SOURCE;
    } else {
        AUDIO_ERR_LOG("Invalid DeviceType");
        return ERROR;
    }

    audioIOHandleMap_.ClosePortAndEraseIOHandle(module);

    std::list<AudioModuleInfo> moduleInfoList;
    audioConfigManager_.GetModuleListByType(ClassType::TYPE_FILE_IO, moduleInfoList);
    for (auto &moduleInfo : moduleInfoList) {
        if (module == moduleInfo.name) {
            moduleInfo.channels = to_string(channelCount);
            audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
            audioPolicyManager_.SetDeviceActive(deviceType, module, true);
        }
    }
    return SUCCESS;
}

void AudioPolicyService::UpdateDescWhenNoBTPermission(vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("No bt permission");

    for (std::shared_ptr<AudioDeviceDescriptor> &desc : deviceDescs) {
        if ((desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) || (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) {
            std::shared_ptr<AudioDeviceDescriptor> copyDesc = std::make_shared<AudioDeviceDescriptor>(desc);
            copyDesc->deviceName_ = "";
            copyDesc->macAddress_ = "";
            desc = copyDesc;
        }
    }
}

int32_t AudioPolicyService::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    return audioVolumeManager_.SetDeviceAbsVolumeSupported(macAddress, support);
}

bool AudioPolicyService::IsAbsVolumeScene() const
{
    return audioPolicyManager_.IsAbsVolumeScene();
}

int32_t AudioPolicyService::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volumeLevel,
    bool internalCall)
{
    return audioVolumeManager_.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
}

int32_t AudioPolicyService::GetAudioLatencyFromXml() const
{
    return audioConfigManager_.GetAudioLatencyFromXml();
}

uint32_t AudioPolicyService::GetSinkLatencyFromXml() const
{
    return audioConfigManager_.GetSinkLatencyFromXml();
}

bool AudioPolicyService::getFastControlParam()
{
    int32_t fastControlFlag = 0; // default 0, set isFastControlled_ false
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", fastControlFlag);
    if (fastControlFlag == 1) {
        isFastControlled_ = true;
    }
    return isFastControlled_;
}

int32_t AudioPolicyService::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo,
    const std::string &bundleName)
{
    // Use GetPreferredOutputDeviceDescriptors instead of currentActiveDevice, if prefer != current, recreate stream
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        GetPreferredOutputDeviceDescriptors(rendererInfo);
    if (preferredDeviceList.size() == 0) {
        return AUDIO_FLAG_NORMAL;
    }

    int32_t flag = audioDeviceCommon_.GetPreferredOutputStreamTypeInner(rendererInfo.streamUsage,
        preferredDeviceList[0]->deviceType_, rendererInfo.rendererFlags, preferredDeviceList[0]->networkId_,
        rendererInfo.samplingRate);
    if (isFastControlled_ && (rendererInfo.playerType != PLAYER_TYPE_SOUND_POOL) &&
        (flag == AUDIO_FLAG_MMAP || flag == AUDIO_FLAG_VOIP_FAST)) {
        std::string bundleNamePre = CHECK_FAST_BLOCK_PREFIX + bundleName;
        std::string result = AudioServerProxy::GetInstance().GetAudioParameterProxy(bundleNamePre);
        if (result == "true") {
            AUDIO_INFO_LOG("%{public}s not in fast list", bundleName.c_str());
            return AUDIO_FLAG_NORMAL;
        }
    }
    if (flag == AUDIO_FLAG_VOIP_FAST && audioSceneManager_.GetAudioScene() == AUDIO_SCENE_PHONE_CALL) {
        AUDIO_INFO_LOG("Current scene is phone call, concede incoming voip fast output stream");
        flag = AUDIO_FLAG_NORMAL;
    }
    return flag;
}

void AudioPolicyService::SetNormalVoipFlag(const bool &normalVoipFlag)
{
    audioConfigManager_.SetNormalVoipFlag(normalVoipFlag);
}

int32_t AudioPolicyService::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    // Use GetPreferredInputDeviceDescriptors instead of currentActiveDevice, if prefer != current, recreate stream
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        GetPreferredInputDeviceDescriptors(capturerInfo);
    if (preferredDeviceList.size() == 0) {
        return AUDIO_FLAG_NORMAL;
    }
    int32_t flag = audioDeviceCommon_.GetPreferredInputStreamTypeInner(capturerInfo.sourceType,
        preferredDeviceList[0]->deviceType_,
        capturerInfo.originalFlag, preferredDeviceList[0]->networkId_, capturerInfo.samplingRate);
    if (flag == AUDIO_FLAG_VOIP_FAST && audioSceneManager_.GetAudioScene() == AUDIO_SCENE_PHONE_CALL) {
        AUDIO_INFO_LOG("Current scene is phone call, concede incoming voip fast input stream");
        flag = AUDIO_FLAG_NORMAL;
    }
    return flag;
}

int32_t AudioPolicyService::ResumeStreamState()
{
    return streamCollector_.ResumeStreamState();
}

int32_t AudioPolicyService::UpdateStreamState(int32_t clientUid,
    StreamSetStateEventInternal &streamSetStateEventInternal)
{
    return streamCollector_.UpdateStreamState(clientUid, streamSetStateEventInternal);
}

int32_t AudioPolicyService::GetUid(int32_t sessionId)
{
    return streamCollector_.GetUid(sessionId);
}

void AudioPolicyService::RemoveDeviceForUid(int32_t uid)
{
    audioAffinityManager_.DelSelectCapturerDevice(uid);
    audioAffinityManager_.DelSelectRendererDevice(uid);
}

void AudioPolicyService::SetDefaultDeviceLoadFlag(bool isLoad)
{
    audioVolumeManager_.SetDefaultDeviceLoadFlag(isLoad);
}

std::vector<sptr<VolumeGroupInfo>> AudioPolicyService::GetVolumeGroupInfos()
{
    return audioDeviceLock_.GetVolumeGroupInfos();
}

void AudioPolicyService::RegiestPolicy()
{
    AUDIO_INFO_LOG("Start");
    const sptr<IStandardAudioService> gsp = AudioServerProxy::GetInstance().GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "RegiestPolicy, Audio Server Proxy is null");
    audioPolicyManager_.SetAudioServerProxy(gsp);

    sptr<PolicyProviderWrapper> wrapper = new(std::nothrow) PolicyProviderWrapper(this);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "Get null PolicyProviderWrapper");
    sptr<IRemoteObject> object = wrapper->AsObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "RegiestPolicy AsObject is nullptr");

    int32_t ret = AudioServerProxy::GetInstance().RegiestPolicyProviderProxy(object);
    AUDIO_DEBUG_LOG("result:%{public}d", ret);
}

/*
 * lockFlag is use to determinewhether GetPreferredOutputDeviceDescriptor or
*  GetPreferredOutputDeviceDescInner is invoked.
 * If deviceStatusUpdateSharedMutex_ write lock is not invoked at the outer layer, lockFlag can be set to true.
 * When deviceStatusUpdateSharedMutex_ write lock has been invoked, lockFlag must be set to false.
 */

int32_t AudioPolicyService::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("%{public}s", ProcessConfig::DumpProcessConfig(config).c_str());
    AudioSamplingRate samplingRate = config.streamInfo.samplingRate;
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            config.rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
            AudioRendererInfo rendererInfo = config.rendererInfo;
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
                (lockFlag ? GetPreferredOutputDeviceDescriptors(rendererInfo, LOCAL_NETWORK_ID)
                          : audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID));
            int32_t type = audioDeviceCommon_.GetPreferredOutputStreamTypeInner(rendererInfo.streamUsage,
                preferredDeviceList[0]->deviceType_, rendererInfo.originalFlag, preferredDeviceList[0]->networkId_,
                samplingRate);
            deviceInfo.deviceRole_ = OUTPUT_DEVICE;
            return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
        }
        AudioDeviceDescriptor curOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
        deviceInfo.deviceId_ = curOutputDeviceDesc.deviceId_;
        deviceInfo.networkId_ = curOutputDeviceDesc.networkId_;
        deviceInfo.deviceType_ = curOutputDeviceDesc.deviceType_;
        deviceInfo.deviceRole_ = OUTPUT_DEVICE;
    } else {
        if (config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            AudioCapturerInfo capturerInfo = config.capturerInfo;
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
                (lockFlag ? GetPreferredInputDeviceDescriptors(capturerInfo, LOCAL_NETWORK_ID)
                          : audioDeviceCommon_.GetPreferredInputDeviceDescInner(capturerInfo, LOCAL_NETWORK_ID));
            int32_t type = audioDeviceCommon_.GetPreferredInputStreamTypeInner(capturerInfo.sourceType,
                preferredDeviceList[0]->deviceType_, capturerInfo.originalFlag, preferredDeviceList[0]->networkId_,
                samplingRate);
            deviceInfo.deviceRole_ = INPUT_DEVICE;
            return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
        }
        deviceInfo.deviceId_ = audioActiveDevice_.GetCurrentInputDevice().deviceId_;
        deviceInfo.networkId_ = audioActiveDevice_.GetCurrentInputDevice().networkId_;
        deviceInfo.deviceRole_ = INPUT_DEVICE;
        deviceInfo.deviceType_ = audioActiveDevice_.GetCurrentInputDeviceType();
    }

    // todo
    // check process in routerMap, return target device for it
    // put the currentActiveDevice_ in deviceinfo, so it can create with current using device.
    // genarate the unique deviceid?
    AudioStreamInfo targetStreamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO}; // note: read from xml
    deviceInfo.audioStreamInfo_ = targetStreamInfo;
    deviceInfo.deviceName_ = "mmap_device";
    audioRouteMap_.GetNetworkIDInFastRouterMap(config.appInfo.appUid, deviceInfo.deviceRole_, deviceInfo.networkId_);
    deviceInfo.a2dpOffloadFlag_ = GetA2dpOffloadFlag();
    return SUCCESS;
}

int32_t AudioPolicyService::GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo,
    int32_t type, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList)
{
    if (type == AUDIO_FLAG_NORMAL) {
        AUDIO_WARNING_LOG("Current device %{public}d not support", type);
        return ERROR;
    }
    deviceInfo.deviceId_ = preferredDeviceList[0]->deviceId_;
    deviceInfo.networkId_ = preferredDeviceList[0]->networkId_;
    deviceInfo.deviceType_ = preferredDeviceList[0]->deviceType_;
    deviceInfo.deviceName_ = preferredDeviceList[0]->deviceName_;
    if (config.streamInfo.samplingRate <= SAMPLE_RATE_16000) {
        deviceInfo.audioStreamInfo_ = {SAMPLE_RATE_16000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    } else {
        deviceInfo.audioStreamInfo_ = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    }
    if (type == AUDIO_FLAG_VOIP_DIRECT) {
        AUDIO_INFO_LOG("Direct VoIP stream, deviceInfo has been updated: deviceInfo.deviceType %{public}d",
            deviceInfo.deviceType_);
        return SUCCESS;
    }
    audioRouteMap_.GetNetworkIDInFastRouterMap(config.appInfo.appUid, deviceInfo.deviceRole_, deviceInfo.networkId_);
    deviceInfo.a2dpOffloadFlag_ = GetA2dpOffloadFlag();
    deviceInfo.isLowLatencyDevice_ = true;
    return SUCCESS;
}

int32_t AudioPolicyService::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    return audioVolumeManager_.InitSharedVolume(buffer);
}

bool AudioPolicyService::GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol)
{
    return audioVolumeManager_.GetSharedVolume(streamType, deviceType, vol);
}

void AudioPolicyService::SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback)
{
    AUDIO_INFO_LOG("Start");
    sptr<AudioManagerListenerStub> parameterChangeCbStub = new(std::nothrow) AudioManagerListenerStub();
    CHECK_AND_RETURN_LOG(parameterChangeCbStub != nullptr,
        "parameterChangeCbStub null");
    parameterChangeCbStub->SetParameterCallback(callback);

    sptr<IRemoteObject> object = parameterChangeCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub object is nullptr");
        return;
    }
    AUDIO_DEBUG_LOG("done");
    AudioServerProxy::GetInstance().SetParameterCallbackProxy(object);
}

void AudioPolicyService::CheckStreamMode(const int64_t activateSessionId)
{
    Trace trace("AudioPolicyService::CheckStreamMode:activateSessionId:" + std::to_string(activateSessionId));
    audioOffloadStream_.CheckStreamMode(activateSessionId);
}

int32_t AudioPolicyService::MoveToNewPipe(uint32_t sessionId, AudioPipeType pipeType)
{
    return audioOffloadStream_.MoveToNewPipe(sessionId, pipeType);
}

int32_t AudioPolicyService::DynamicUnloadModule(const AudioPipeType pipeType)
{
    switch (pipeType) {
        case PIPE_TYPE_OFFLOAD:
            audioOffloadStream_.DynamicUnloadOffloadModule();
            break;
        case PIPE_TYPE_MULTICHANNEL:
            return audioOffloadStream_.UnloadMchModule();
        default:
            AUDIO_WARNING_LOG("not supported for pipe type %{public}d", pipeType);
            break;
    }
    return SUCCESS;
}

int32_t AudioPolicyService::GetMaxRendererInstances()
{
    return audioConfigManager_.GetMaxRendererInstances();
}

#ifdef BLUETOOTH_ENABLE
void RegisterBluetoothDeathCallback()
{
    lock_guard<mutex> lock(g_btProxyMutex);
    AUDIO_INFO_LOG("Start RegisterBluetoothDeathCallback");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgr != nullptr,
        "RegisterBluetoothDeathCallback: get sa manager failed");
    sptr<IRemoteObject> object = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG(object != nullptr,
        "RegisterBluetoothDeathCallback: get audio service remote object failed");
    // register death recipent
    sptr<AudioServerDeathRecipient> asDeathRecipient =
        new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
    if (asDeathRecipient != nullptr) {
        asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
            AudioPolicyService::BluetoothServiceCrashedCallback(pid, uid);
        });
        bool result = object->AddDeathRecipient(asDeathRecipient);
        if (!result) {
            AUDIO_ERR_LOG("RegisterBluetoothDeathCallback: failed to add deathRecipient");
        }
    }
}

void AudioPolicyService::BluetoothServiceCrashedCallback(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("Bluetooth sa crashed, will restore proxy in next call");
    lock_guard<mutex> lock(g_btProxyMutex);
    isBtListenerRegistered = false;
    isBtCrashed = true;
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSink();
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSource();
    Bluetooth::AudioHfpManager::DisconnectBluetoothHfpSink();
}
#endif

void AudioPolicyService::RegisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::RegisterDeviceObserver(deviceStatusListener_->deviceObserver_);
    if (isBtListenerRegistered) {
        AUDIO_INFO_LOG("audio policy service already register bt listerer, return");
        return;
    }

    if (!isBtCrashed) {
        Bluetooth::AudioA2dpManager::RegisterBluetoothA2dpListener();
        Bluetooth::AudioHfpManager::RegisterBluetoothScoListener();
    }
    
    isBtListenerRegistered = true;
    isBtCrashed = false;
    RegisterBluetoothDeathCallback();
    AudioPolicyUtils::GetInstance().SetBtConnecting(true);
    Bluetooth::AudioA2dpManager::CheckA2dpDeviceReconnect();
    Bluetooth::AudioHfpManager::CheckHfpDeviceReconnect();
    AudioPolicyUtils::GetInstance().SetBtConnecting(false);
#endif
}

void AudioPolicyService::UnregisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::UnregisterDeviceObserver();
    Bluetooth::AudioA2dpManager::UnregisterBluetoothA2dpListener();
    Bluetooth::AudioHfpManager::UnregisterBluetoothScoListener();
    isBtListenerRegistered = false;
#endif
}

void AudioPolicyService::SubscribeAccessibilityConfigObserver()
{
#ifdef ACCESSIBILITY_ENABLE
    RegisterAccessibilityMonitorHelper();
    AUDIO_INFO_LOG("Subscribe accessibility config observer successfully");
#endif
}

float AudioPolicyService::GetMinStreamVolume()
{
    return audioPolicyManager_.GetMinStreamVolume();
}

float AudioPolicyService::GetMaxStreamVolume()
{
    return audioPolicyManager_.GetMaxStreamVolume();
}

bool AudioPolicyService::IsVolumeUnadjustable()
{
    return audioPolicyManager_.IsVolumeUnadjustable();
}

void AudioPolicyService::GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfoMap)
{
    return audioPolicyManager_.GetStreamVolumeInfoMap(streamVolumeInfoMap);
}

float AudioPolicyService::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType) const
{
    return audioPolicyManager_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

int32_t AudioPolicyService::QueryEffectManagerSceneMode(SupportedEffectConfig& supportedEffectConfig)
{
    int32_t ret = audioEffectService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    return ret;
}

void AudioPolicyService::RegisterDataObserver()
{
    std::string devicesName = "";
    int32_t ret = AudioPolicyUtils::GetInstance().GetDeviceNameFromDataShareHelper(devicesName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterDataObserver get devicesName failed");
    SetDisplayName(devicesName, true);
    RegisterNameMonitorHelper();
}

int32_t AudioPolicyService::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    int32_t rate = 48000;

    CHECK_AND_RETURN_RET_LOG(desc != nullptr, -1, "desc is null!");

    bool ret = audioConnectedDevice_.IsConnectedOutputDevice(desc);
    CHECK_AND_RETURN_RET(ret, -1);

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);
    DeviceType clientDevType = desc->deviceType_;
    for (const auto &device : deviceClassInfo) {
        auto moduleInfoList = device.second;
        for (auto &moduleInfo : moduleInfoList) {
            auto serverDevType = AudioPolicyUtils::GetInstance().GetDeviceType(moduleInfo.name);
            if (clientDevType == serverDevType) {
                rate = atoi(moduleInfo.rate.c_str());
                return rate;
            }
        }
    }

    return rate;
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyService::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    return audioDeviceLock_.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyService::GetAvailableMicrophones()
{
    return audioDeviceLock_.GetAvailableMicrophones();
}

void AudioPolicyService::OnCapturerSessionRemoved(uint64_t sessionID)
{
    audioDeviceLock_.OnCapturerSessionRemoved(sessionID);
}

int32_t AudioPolicyService::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    return audioDeviceLock_.OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::DeviceFilterByUsageInner(AudioDeviceUsage usage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& descs)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> devicePrivacyMaps =
        audioDeviceManager_.GetDevicePrivacyMaps();
    for (const auto &dev : descs) {
        for (const auto &devicePrivacy : devicePrivacyMaps) {
            list<DevicePrivacyInfo> deviceInfos = devicePrivacy.second;
            audioDeviceManager_.GetAvailableDevicesWithUsage(usage, deviceInfos, dev, audioDeviceDescriptors);
        }
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors;
    for (const auto &dec : audioDeviceDescriptors) {
        std::shared_ptr<AudioDeviceDescriptor> tempDec = std::make_shared<AudioDeviceDescriptor>(*dec);
        deviceDescriptors.push_back(move(tempDec));
    }
    return deviceDescriptors;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetAvailableDevices(AudioDeviceUsage usage)
{
    return audioDeviceLock_.GetAvailableDevices(usage);
}

int32_t AudioPolicyService::OffloadStartPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->OffloadStartPlaying(sessionIds);
    }
    AUDIO_WARNING_LOG("Null audioA2dpOffloadManager");
#endif
    return SUCCESS;
}

int32_t AudioPolicyService::OffloadStopPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->OffloadStopPlaying(sessionIds);
    }
    AUDIO_WARNING_LOG("Null audioA2dpOffloadManager");
#endif
    return SUCCESS;
}

int32_t AudioPolicyService::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    Trace trace("AudioPolicyService::OffloadGetRenderPosition");
#ifdef BLUETOOTH_ENABLE
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    AUDIO_DEBUG_LOG("GetRenderPosition, deviceType: %{public}d, a2dpOffloadFlag: %{public}d",
        GetA2dpOffloadFlag(), curOutputDeviceType);
    int32_t ret = SUCCESS;
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID &&
        GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
        ret = Bluetooth::AudioA2dpManager::GetRenderPosition(delayValue, sendDataSize, timeStamp);
    } else {
        delayValue = 0;
        sendDataSize = 0;
        timeStamp = 0;
    }
    return ret;
#else
    return SUCCESS;
#endif
}

int32_t AudioPolicyService::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
#ifdef FEATURE_APPGALLERY
    ClientTypeManager::GetInstance()->GetAndSaveClientType(uid, bundleName);
#endif
    return SUCCESS;
}

void AudioPolicyService::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    audioDeviceLock_.OnDeviceInfoUpdated(desc, command);
}

int32_t AudioPolicyService::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    return audioDeviceLock_.SetCallDeviceActive(deviceType, active, address, uid);
}

std::shared_ptr<AudioDeviceDescriptor> AudioPolicyService::GetActiveBluetoothDevice()
{
    return audioDeviceLock_.GetActiveBluetoothDevice();
}

ConverterConfig AudioPolicyService::GetConverterConfig()
{
    AudioConverterParser &converterParser = AudioConverterParser::GetInstance();
    return converterParser.LoadConfig();
}

float AudioPolicyService::GetMaxAmplitude(const int32_t deviceId, const AudioInterrupt audioInterrupt)
{
    return audioActiveDevice_.GetMaxAmplitude(deviceId, audioInterrupt);
}

int32_t AudioPolicyService::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    return audioDeviceLock_.TriggerFetchDevice(reason);
}

int32_t AudioPolicyService::DisableSafeMediaVolume()
{
    return audioVolumeManager_.DisableSafeMediaVolume();
}

void AudioPolicyService::NotifyAccountsChanged(const int &id)
{
    audioPolicyManager_.NotifyAccountsChanged(id);
    RegisterDataObserver();
    SubscribeAccessibilityConfigObserver();
    AudioServerProxy::GetInstance().NotifyAccountsChanged();
}

int32_t AudioPolicyService::GetCurActivateCount()
{
    return audioPolicyManager_.GetCurActivateCount();
}

int32_t AudioPolicyService::SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object)
{
    return streamCollector_.SetAudioConcurrencyCallback(sessionID, object);
}

int32_t AudioPolicyService::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    return streamCollector_.UnsetAudioConcurrencyCallback(sessionID);
}

int32_t AudioPolicyService::ActivateAudioConcurrency(const AudioPipeType &pipeType)
{
    return streamCollector_.ActivateAudioConcurrency(pipeType);
}

int32_t AudioPolicyService::ResetRingerModeMute()
{
    return audioVolumeManager_.ResetRingerModeMute();
}

bool AudioPolicyService::IsRingerModeMute()
{
    return audioVolumeManager_.IsRingerModeMute();
}

void AudioPolicyService::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    audioDeviceLock_.OnReceiveBluetoothEvent(macAddress, deviceName);
}

void AudioPolicyService::LoadHdiEffectModel()
{
    return AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
}

int32_t AudioPolicyService::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    GetSupportedEffectProperty(effectPropertyArray);
    for (auto &effectItem : effectPropertyArray.property) {
        effectItem.flag = RENDER_EFFECT_FLAG;
        propertyArray.property.push_back(effectItem);
    }
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    GetSupportedEnhanceProperty(enhancePropertyArray);
    for (auto &enhanceItem : enhancePropertyArray.property) {
        enhanceItem.flag = CAPTURE_EFFECT_FLAG;
        propertyArray.property.push_back(enhanceItem);
    }
    return AUDIO_OK;
}

void AudioPolicyService::GetSupportedEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEffectPropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(OUTPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEffectPropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEffectPropertyV3{p.first, p.second, RENDER_EFFECT_FLAG};
        });
    return;
}

void AudioPolicyService::GetSupportedEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(INPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEffectPropertyV3{p.first, p.second, CAPTURE_EFFECT_FLAG};
        });
    return;
}

int32_t AudioPolicyService::CheckSupportedAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    const EffectFlag& flag)
{
    AudioEffectPropertyArrayV3 supportPropertyArray;
    if (flag == CAPTURE_EFFECT_FLAG) {
        GetSupportedEnhanceProperty(supportPropertyArray);
    } else {
        GetSupportedEffectProperty(supportPropertyArray);
    }
    for (auto &item : propertyArray.property) {
        auto oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set property not valid name:%{public}s,category:%{public}s,flag:%{public}d",
            item.name.c_str(), item.category.c_str(), item.flag);
    }
    return AUDIO_OK;
}

int32_t AudioPolicyService::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t ret = AUDIO_OK;
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    for (auto &item : propertyArray.property) {
        if (item.flag == CAPTURE_EFFECT_FLAG) {
            enhancePropertyArray.property.push_back(item);
        } else {
            effectPropertyArray.property.push_back(item);
        }
    }
    CHECK_AND_RETURN_RET_LOG(CheckSupportedAudioEffectProperty(enhancePropertyArray, CAPTURE_EFFECT_FLAG) == AUDIO_OK,
        ERR_INVALID_PARAM, "check Audio Enhance property failed");
    CHECK_AND_RETURN_RET_LOG(CheckSupportedAudioEffectProperty(effectPropertyArray, RENDER_EFFECT_FLAG) == AUDIO_OK,
        ERR_INVALID_PARAM, "check Audio Effect property failed");
    if (enhancePropertyArray.property.size() > 0) {
        AudioEffectPropertyArrayV3 oldPropertyArray = {};
        ret = GetAudioEnhanceProperty(oldPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(enhancePropertyArray,
            audioActiveDevice_.GetCurrentInputDeviceType());
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio enhance property fail");
        audioCapturerSession_.ReloadSourceForEffect(oldPropertyArray, enhancePropertyArray);
    }
    if (effectPropertyArray.property.size() > 0) {
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(effectPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio effect property fail");
    }
    return ret;
}

int32_t AudioPolicyService::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t ret = AUDIO_OK;
    ret = AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");
    auto oIter = propertyArray.property.begin();
    while (oIter != propertyArray.property.end()) {
        if (oIter->flag == RENDER_EFFECT_FLAG) {
            oIter = propertyArray.property.erase(oIter);
        } else {
            oIter++;
        }
    }
    return ret;
}

int32_t AudioPolicyService::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
}

int32_t AudioPolicyService::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEffectPropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(OUTPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEffectPropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEffectProperty{p.first, p.second};
        });
    return AUDIO_OK;
}

int32_t AudioPolicyService::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(INPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEnhanceProperty{p.first, p.second};
        });
    return AUDIO_OK;
}

int32_t AudioPolicyService::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    AudioEffectPropertyArray supportPropertyArray;
    std::vector<AudioEffectProperty>::iterator oIter;
    (void)GetSupportedAudioEffectProperty(supportPropertyArray);
    for (auto &item : propertyArray.property) {
        oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set audio effect property not valid %{public}s:%{public}s",
            item.effectClass.c_str(), item.effectProp.c_str());
    }
    return AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(propertyArray);
}

int32_t AudioPolicyService::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
}

int32_t AudioPolicyService::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    AudioEnhancePropertyArray supportPropertyArray;
    std::vector<AudioEnhanceProperty>::iterator oIter;
    (void)GetSupportedAudioEnhanceProperty(supportPropertyArray);
    for (auto &item : propertyArray.property) {
        oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set audio enhance property not valid %{public}s:%{public}s",
            item.enhanceClass.c_str(), item.enhanceProp.c_str());
    }
    AudioEnhancePropertyArray oldPropertyArray = {};
    int32_t ret = AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(oldPropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");

    ret = AudioServerProxy::GetInstance().SetAudioEnhancePropertyProxy(propertyArray,
        audioActiveDevice_.GetCurrentInputDeviceType());
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio enhance property fail");

    audioCapturerSession_.ReloadSourceForEffect(oldPropertyArray, propertyArray);
    return ret;
}

int32_t AudioPolicyService::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    return AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(propertyArray);
}

int32_t AudioPolicyService::GetAudioEnhancePropertyByDevice(DeviceType deviceType,
    AudioEnhancePropertyArray &propertyArray)
{
    return AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(propertyArray, deviceType);
}

void AudioPolicyService::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(isSupported);
    return;
}

void AudioPolicyService::SetRotationToEffect(const uint32_t rotate)
{
    AudioServerProxy::GetInstance().SetRotationToEffectProxy(rotate);
}

bool AudioPolicyService::IsA2dpOffloadConnected()
{
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->IsA2dpOffloadConnected();
    }
    AUDIO_WARNING_LOG("Null audioA2dpOffloadManager");
    return true;
}

int32_t  AudioPolicyService::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    AUDIO_INFO_LOG("start audio stream split, the split args is %{public}s", splitArgs.c_str());
    if (splitArgs.empty() || networkId.empty()) {
        std::string anonymousNetworkId = networkId.empty() ? "" : networkId.substr(0, 2) + "***";
        AUDIO_ERR_LOG("LoadSplitModule, invalid param, splitArgs:'%{public}s', networkId:'%{public}s'",
            splitArgs.c_str(), anonymousNetworkId.c_str());
        return ERR_INVALID_PARAM;
    }
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, OUTPUT_DEVICE);
    std::string currentActivePort = REMOTE_CLASS;
    audioPolicyManager_.SuspendAudioDevice(currentActivePort, true);
    audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);

    AudioModuleInfo moudleInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
        OUTPUT_DEVICE, DEVICE_TYPE_SPEAKER);
    moudleInfo.lib = "libmodule-split-stream-sink.z.so";
    moudleInfo.extra = splitArgs;

    int32_t openRet = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleName, moudleInfo);
    if (openRet != 0) {
        AUDIO_ERR_LOG("open fail, OpenPortAndInsertIOHandle ret: %{public}d", openRet);
    }
    AudioServerProxy::GetInstance().NotifyDeviceInfoProxy(networkId, true);
    return openRet;
}

bool AudioPolicyService::IsCurrentActiveDeviceA2dp()
{
    return audioPolicyManager_.GetActiveDevice() == DEVICE_TYPE_BLUETOOTH_A2DP;
}

bool AudioPolicyService::IsAllowedPlayback(const int32_t &uid, const int32_t &pid)
{
#ifdef AVSESSION_ENABLE
    // Temporary solution to avoid performance issues
    if (uid == BOOTUP_MUSIC_UID) {
        return true;
    }
    lock_guard<mutex> lock(g_policyMgrListenerMutex);
    bool allowed = false;
    if (policyManagerListener_ != nullptr) {
        allowed = policyManagerListener_->OnQueryAllowedPlayback(uid, pid);
    }
    if (!allowed) {
        usleep(WATI_PLAYBACK_TIME); //wait for 200ms
        AUDIO_INFO_LOG("IsAudioPlaybackAllowed Try again after 200ms");
        if (policyManagerListener_ != nullptr) {
            allowed = policyManagerListener_->OnQueryAllowedPlayback(uid, pid);
        }
    }
    return allowed;
#endif
    return true;
}

int32_t AudioPolicyService::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    CHECK_AND_RETURN_RET_LOG(audioConfigManager_.GetHasEarpiece(), ERR_NOT_SUPPORTED, "the device has no earpiece");
    int32_t ret = audioDeviceManager_.SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    if (ret == NEED_TO_FETCH) {
        audioDeviceCommon_.FetchDevice(true, AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
        return SUCCESS;
    }
    return ret;
}

bool AudioPolicyService::GetAudioEffectOffloadFlag()
{
    // check if audio effect offload
    return AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
}

void AudioPolicyService::SetA2dpOffloadFlag(BluetoothOffloadState state)
{
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->SetA2dpOffloadFlag(state);
    }
}

BluetoothOffloadState AudioPolicyService::GetA2dpOffloadFlag()
{
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->GetA2dpOffloadFlag();
    }
    return NO_A2DP_DEVICE;
}

void AudioPolicyService::SetDefaultAdapterEnable(bool isEnable)
{
    return AudioServerProxy::GetInstance().SetDefaultAdapterEnableProxy(isEnable);
}

int32_t AudioPolicyService::ActivateConcurrencyFromServer(AudioPipeType incomingPipe)
{
    return audioOffloadStream_.ActivateConcurrencyFromServer(incomingPipe);
}

int32_t AudioPolicyService::NotifyCapturerRemoved(uint64_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is nullptr");
    audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionId, false);
    return SUCCESS;
}

void AudioPolicyService::CheckConnectedDevice()
{
    bool flag = audioPolicyManager_.GetActiveDevice() == DEVICE_TYPE_USB_ARM_HEADSET ||
        audioPolicyManager_.GetActiveDevice() ==  DEVICE_TYPE_USB_HEADSET;
    AudioServerProxy::GetInstance().SetDeviceConnectedFlag(flag);
}

void AudioPolicyService::SetDeviceConnectedFlagFalseAfterDuration()
{
    usleep(DEVICE_CONNECTED_FLAG_DURATION_MS); // 3s
    AudioServerProxy::GetInstance().SetDeviceConnectedFlag(false);
}

void AudioPolicyService::CheckHibernateState(bool hibernate)
{
    AudioServerProxy::GetInstance().CheckHibernateStateProxy(hibernate);
}

void AudioPolicyService::UpdateSafeVolumeByS4()
{
    return audioVolumeManager_.UpdateSafeVolumeByS4();
}

void AudioPolicyService::UpdateSpatializationSupported(const std::string macAddress, const bool support)
{
    audioDeviceLock_.UpdateSpatializationSupported(macAddress, support);
}
#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioPolicyService::LoadModernInnerCapSink(int32_t innerCapId)
{
    AUDIO_INFO_LOG("Start");
    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    std::string name = INNER_CAPTURER_SINK;
    moduleInfo.name = name + std::to_string(innerCapId);

    moduleInfo.format = "s16le";
    moduleInfo.channels = "2"; // 2 channel
    moduleInfo.rate = "48000";
    moduleInfo.bufferSize = "3840"; // 20ms

    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
    return SUCCESS;
}

int32_t AudioPolicyService::UnloadModernInnerCapSink(int32_t innerCapId)
{
    AUDIO_INFO_LOG("Start");
    std::string name = INNER_CAPTURER_SINK;
    name += std::to_string(innerCapId);

    audioIOHandleMap_.ClosePortAndEraseIOHandle(name);
    return SUCCESS;
}
#endif

int32_t AudioPolicyService::SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object)
{
    lock_guard<mutex> lock(g_policyMgrListenerMutex);
    policyManagerListener_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
