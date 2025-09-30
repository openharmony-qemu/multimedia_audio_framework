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
#define LOG_TAG "AudioAdapterManager"
#endif

#include "audio_adapter_manager.h"


#include "parameter.h"
#include "parameters.h"

#include "audio_policy_service.h"
#include "audio_volume_parser.h"
#include "audio_policy_server.h"
#include "audio_volume.h"
#include "audio_utils.h"
#include "audio_zone_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static const char* DO_NOT_DISTURB_STATUS = "focus_mode_enable";
static const char* DO_NOT_DISTURB_STATUS_WHITE_LIST = "intelligent_scene_notification_white_list";
mutex g_deviceVolumeBehaviorListenerMutex;

static const std::vector<AudioStreamType> DISTRIBUTED_VOLUME_TYPE_LIST = {
    // Distributed Volume Type.
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_MUSIC
};

static const std::vector<AudioStreamType> VOLUME_TYPE_LIST = {
    // all volume types except STREAM_ALL
    STREAM_RING,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_MUSIC
};

static const std::vector<AudioStreamType> PC_VOLUME_TYPE_LIST = {
    // all volume types except STREAM_ALL
    STREAM_RING,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_SYSTEM,
    STREAM_ULTRASONIC,
    STREAM_VOICE_CALL_ASSISTANT,
    // adjust the type of music from the head of list to end, make sure music is updated last.
    // avoid interference from ring updates on special platform.
    // when the device is switched to headset,ring and alarm is dualtone type.
    // dualtone type use fixed volume curve of speaker.
    // the ring and alarm are classified into the music group.
    // the music volume becomes abnormal when the db value of music is modified.
    STREAM_MUSIC
};

static const std::vector<DeviceType> VOLUME_GROUP_TYPE_LIST = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_REMOTE_CAST
};

static const std::vector<std::string> SYSTEM_SOUND_KEY_LIST = {
    // all keys for system sound uri
    "ringtone_for_sim_card_0",
    "ringtone_for_sim_card_1",
    "system_tone_for_sim_card_0",
    "system_tone_for_sim_card_1",
    "system_tone_for_notification"
};

static const std::unordered_map<DeviceType, DeviceVolumeType> DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP = {
    {DEVICE_TYPE_EARPIECE, EARPIECE_VOLUME_TYPE},
    {DEVICE_TYPE_SPEAKER, SPEAKER_VOLUME_TYPE},
    {DEVICE_TYPE_WIRED_HEADSET, HEADSET_VOLUME_TYPE}
};

namespace {
const std::unordered_map<DeviceType, std::vector<std::string>> DEVICE_CLASS_MAP = {
    {DEVICE_TYPE_SPEAKER, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
    {DEVICE_TYPE_USB_HEADSET, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
    {DEVICE_TYPE_BLUETOOTH_A2DP, {A2DP_CLASS, PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
    {DEVICE_TYPE_BLUETOOTH_SCO, {PRIMARY_CLASS, MCH_CLASS}},
    {DEVICE_TYPE_NEARLINK, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
    {DEVICE_TYPE_EARPIECE, {PRIMARY_CLASS, MCH_CLASS}},
    {DEVICE_TYPE_WIRED_HEADSET, {PRIMARY_CLASS, MCH_CLASS}},
    {DEVICE_TYPE_WIRED_HEADPHONES, {PRIMARY_CLASS, MCH_CLASS}},
    {DEVICE_TYPE_USB_ARM_HEADSET, {PRIMARY_CLASS, USB_CLASS}},
    {DEVICE_TYPE_REMOTE_CAST, {REMOTE_CAST_INNER_CAPTURER_SINK_NAME}},
    {DEVICE_TYPE_DP, {DP_CLASS}},
    {DEVICE_TYPE_FILE_SINK, {FILE_CLASS}},
    {DEVICE_TYPE_FILE_SOURCE, {FILE_CLASS}},
    {DEVICE_TYPE_HDMI, {PRIMARY_CLASS}},
    {DEVICE_TYPE_ACCESSORY, {ACCESSORY_CLASS}},
    {DEVICE_TYPE_HEARING_AID, {HEARING_AID_CLASS}},
};
} // namespace

// LCOV_EXCL_START
bool AudioAdapterManager::Init()
{
    currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    char testMode[10] = {0}; // 10 for system parameter usage
    auto ret = GetParameter("debug.audio_service.testmodeon", "0", testMode, sizeof(testMode));
    if (ret == 1 && testMode[0] == '1') {
        AUDIO_DEBUG_LOG("testMode on");
        testModeOn_ = true;
    }

    std::unique_ptr<AudioVolumeParser> audiovolumeParser = make_unique<AudioVolumeParser>();
    CHECK_AND_RETURN_RET_LOG(audiovolumeParser, false, "audiovolumeParser is null");
    auto lret = audiovolumeParser->LoadConfig(streamVolumeInfos_);
    defaultVolumeTypeList_ = (VolumeUtils::IsPCVolumeEnable()) ? PC_VOLUME_TYPE_LIST : VOLUME_TYPE_LIST;
    if (!lret) {
        AUDIO_INFO_LOG("Audio Volume Config Load Configuration successfully");
        useNonlinearAlgo_ = 1;
        UpdateVolumeMapIndex();
    }

    // init volume before kvstore start by local prop for bootanimation
    InitBootAnimationVolume();
    AudioVolume::GetInstance()->SetDefaultAppVolume(appConfigVolume_.defaultVolume);
    std::string defaultSafeVolume = std::to_string(GetMaxVolumeLevel(STREAM_MUSIC));
    AUDIO_INFO_LOG("defaultSafeVolume %{public}s", defaultSafeVolume.c_str());
    char currentSafeVolumeValue[4] = {0};
    ret = GetParameter("const.audio.safe_media_volume", defaultSafeVolume.c_str(),
        currentSafeVolumeValue, sizeof(currentSafeVolumeValue));
    if (ret > 0) {
        safeVolume_ = atoi(currentSafeVolumeValue);
        AUDIO_INFO_LOG("Get currentSafeVolumeValue success %{public}d", safeVolume_);
    } else {
        safeVolume_ = GetMaxVolumeLevel(STREAM_MUSIC);
        AUDIO_ERR_LOG("Get currentSafeVolumeValue failed %{public}d", ret);
    }

    char safeVolumeTimeout[6] = {0};
    ret = GetParameter("persist.multimedia.audio.safevolume.timeout", "1140",
        safeVolumeTimeout, sizeof(safeVolumeTimeout));
    if (ret > 0) {
        safeVolumeTimeout_ = atoi(safeVolumeTimeout);
        AUDIO_INFO_LOG("Get safeVolumeTimeout success %{public}d", safeVolumeTimeout_);
    } else {
        AUDIO_ERR_LOG("Get safeVolumeTimeout failed %{public}d", ret);
    }

    isVolumeUnadjustable_ = system::GetBoolParameter("const.multimedia.audio.fixedvolume", false);
    AUDIO_INFO_LOG("Get fixdvolume parameter success %{public}d", isVolumeUnadjustable_);

    handler_ = std::make_shared<AudioAdapterManagerHandler>();
    return true;
}

void AudioAdapterManager::InitBootAnimationVolume()
{
    char currentVolumeValue[3] = {0};
    AudioVolumeType typeForBootAnimation = VolumeUtils::IsPCVolumeEnable() ? STREAM_SYSTEM : STREAM_RING;
    int32_t bootAnimationVolume = volumeDataMaintainer_.GetStreamVolume(typeForBootAnimation);
    AUDIO_DEBUG_LOG("Init: Type[%{public}d],volume[%{public}d]", typeForBootAnimation, bootAnimationVolume);
    std::string defaultVolume = std::to_string(bootAnimationVolume);
    auto ret = GetParameter("persist.multimedia.audio.ringtonevolume", defaultVolume.c_str(),
        currentVolumeValue, sizeof(currentVolumeValue));
    if (ret > 0) {
        volumeDataMaintainer_.SetStreamVolume(typeForBootAnimation, atoi(currentVolumeValue));
        AUDIO_INFO_LOG("Init: Get Type[%{public}d] volume to map volume [%{public}d]",
            typeForBootAnimation, volumeDataMaintainer_.GetStreamVolume(typeForBootAnimation));
    } else {
        AUDIO_ERR_LOG("Init: Get volume parameter failed %{public}d", ret);
    }
}

bool AudioAdapterManager::ConnectServiceAdapter()
{
    std::unique_ptr<PolicyCallbackImpl> policyCallbackImpl = std::make_unique<PolicyCallbackImpl>(this);
    audioServiceAdapter_ = AudioServiceAdapter::CreateAudioAdapter(std::move(policyCallbackImpl));
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, false,
        "[AudioAdapterManager] Error in audio adapter initialization");

    bool result = audioServiceAdapter_->Connect();
    CHECK_AND_RETURN_RET_LOG(result, false, "[AudioAdapterManager] Error in connecting audio adapter");

    return true;
}

void AudioAdapterManager::InitKVStore()
{
    InitKVStoreInternal();
}

void AudioAdapterManager::InitKVStoreInternal()
{
    CHECK_AND_RETURN_LOG(!isLoaded_, "InitKVStore: the database value is loaded");

    AUDIO_INFO_LOG("in");
    bool isFirstBoot = false;
    volumeDataMaintainer_.RegisterCloned();
    InitAudioPolicyKvStore(isFirstBoot);

    if (handler_ != nullptr) {
        handler_->SendKvDataUpdate(isFirstBoot);
    }
}

void AudioAdapterManager::HandleKvData(bool isFirstBoot)
{
    InitVolumeMap(isFirstBoot);
    InitRingerMode(isFirstBoot);
    InitMuteStatusMap(isFirstBoot);
    InitSafeStatus(isFirstBoot);
    InitSafeTime(isFirstBoot);

    if (isNeedCopySystemUrlData_) {
        CloneSystemSoundUrl();
    }

    if (!isNeedCopyVolumeData_ && !isNeedCopyMuteData_ && !isNeedCopyRingerModeData_ && !isNeedCopySystemUrlData_) {
        isAllCopyDone_ = true;
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendRingerModeUpdatedCallback(ringerMode_);
            SetVolumeCallbackAfterClone();
        }
    }

    if (isAllCopyDone_ && audioPolicyKvStore_ != nullptr) {
        // delete KvStore
        InitSafeStatus(true);
        InitSafeTime(true);
        AUDIO_INFO_LOG("Copy audio_policy private database success to settings database, delete private database...");
        DeleteAudioPolicyKvStore();
    }

    // Make sure that the volume value is applied.
    auto iter = defaultVolumeTypeList_.begin();
    while (iter != defaultVolumeTypeList_.end()) {
        SetVolumeDb(*iter);
        iter++;
    }

    UpdateVolumeForLowLatency();
}

int32_t AudioAdapterManager::ReInitKVStore()
{
    CHECK_AND_RETURN_RET_LOG(audioPolicyKvStore_ != nullptr, ERR_INVALID_OPERATION,
        "audioPolicyKvStore_ is already nullptr");
    audioPolicyKvStore_ = nullptr;
    DistributedKvDataManager manager;
    Options options;

    AppId appId;
    appId.appId = "audio_policy_manager";
    options.baseDir = std::string("/data/service/el1/public/database/") + appId.appId;

    StoreId storeId;
    storeId.storeId = "audiopolicy";
    Status status = Status::SUCCESS;

    status = manager.CloseKvStore(appId, storeId);
    AUDIO_ERR_LOG("CloseKvStore status: %{public}d", status);
    CHECK_AND_RETURN_RET_LOG(status == Status::SUCCESS, ERR_ILLEGAL_STATE, "CloseKvStore failed!");

    status = manager.DeleteKvStore(appId, storeId, options.baseDir);
    CHECK_AND_RETURN_RET_LOG(status == Status::SUCCESS, ERR_ILLEGAL_STATE, "CloseKvStore failed!");

    InitKVStoreInternal();
    return SUCCESS;
}

void AudioAdapterManager::Deinit(void)
{
    CHECK_AND_RETURN_LOG(audioServiceAdapter_, "Deinit audio adapter null");

    if (handler_ != nullptr) {
        AUDIO_INFO_LOG("release handler");
        handler_->ReleaseEventRunner();
        handler_ = nullptr;
    }

    return audioServiceAdapter_->Disconnect();
}

int32_t AudioAdapterManager::SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetAudioStreamRemovedCallback callback == nullptr");

    sessionCallback_ = callback;
    return SUCCESS;
}

// LCOV_EXCL_STOP
int32_t AudioAdapterManager::GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.maxVolume;
    }

    if (streamVolumeInfos_.end() != streamVolumeInfos_.find(volumeType)) {
        DeviceType type = currentActiveDevice_.deviceType_;
        if (deviceType != DEVICE_TYPE_NONE) {
            type = deviceType;
        }
        auto deviceIt = DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.find(type);
        if (deviceIt != DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.end()) {
            DeviceVolumeType deviceVolumeType = deviceIt->second;
            if ((streamVolumeInfos_[volumeType] != nullptr) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
                streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceVolumeType)) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType] != nullptr) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType]->maxLevel != -1)) {
                return streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType]->maxLevel;
            }
        }
    }

    if (maxVolumeIndexMap_.end() != maxVolumeIndexMap_.find(volumeType)) {
        return maxVolumeIndexMap_[volumeType];
    } else if (maxVolumeIndexMap_.end() != maxVolumeIndexMap_.find(STREAM_MUSIC)) {
        AUDIO_WARNING_LOG("can't find volumeType:%{public}d and use default STREAM_MUSIC", volumeType);
        return maxVolumeIndexMap_[STREAM_MUSIC];
    } else {
        AUDIO_ERR_LOG("use default max volume level %{public}d", MAX_VOLUME_LEVEL);
        return MAX_VOLUME_LEVEL;
    }
}

int32_t AudioAdapterManager::GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.minVolume;
    }

    if (streamVolumeInfos_.end() != streamVolumeInfos_.find(volumeType)) {
        DeviceType type = currentActiveDevice_.deviceType_;
        if (deviceType != DEVICE_TYPE_NONE) {
            type = deviceType;
        }
        auto deviceIt = DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.find(type);
        if (deviceIt != DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.end()) {
            DeviceVolumeType deviceVolumeType = deviceIt->second;
            if ((streamVolumeInfos_[volumeType] != nullptr) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
                streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceVolumeType)) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType] != nullptr) &&
                (streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType]->minLevel != -1)) {
                return streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceVolumeType]->minLevel;
            }
        }
    }

    if (minVolumeIndexMap_.end() != minVolumeIndexMap_.find(volumeType)) {
        return minVolumeIndexMap_[volumeType];
    } else if (minVolumeIndexMap_.end() != minVolumeIndexMap_.find(STREAM_MUSIC)) {
        AUDIO_WARNING_LOG("can't find volumeType:%{public}d and use default STREAM_MUSIC", volumeType);
        return minVolumeIndexMap_[STREAM_MUSIC];
    } else {
        AUDIO_ERR_LOG("use default max volume level %{public}d", MIN_VOLUME_LEVEL);
        return MIN_VOLUME_LEVEL;
    }
}

void AudioAdapterManager::SaveRingtoneVolumeToLocal(AudioVolumeType volumeType, int32_t volumeLevel)
{
    AudioVolumeType audioVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(volumeType);
    // PC Boot Animation Volume use STREAM_SYSTEM
    if ((volumeType == STREAM_RING && !VolumeUtils::IsPCVolumeEnable()) || (audioVolumeMap == STREAM_SYSTEM &&
        currentActiveDevice_.deviceType_ == DEVICE_TYPE_SPEAKER)) {
        int32_t volumeLevel =
            volumeDataMaintainer_.GetStreamVolume(audioVolumeMap) * (GetStreamMute(audioVolumeMap) ? 0 : 1);
        int32_t ret = SetParameter("persist.multimedia.audio.ringtonevolume", std::to_string(volumeLevel).c_str());
        if (ret == 0) {
            AUDIO_INFO_LOG("Save ringtone volume for boot success %{public}d", volumeLevel);
        } else {
            AUDIO_ERR_LOG("Save ringtone volume for boot failed, result %{public}d", ret);
        }
    }
}

void AudioAdapterManager::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    volumeDataMaintainer_.SetDataShareReady(std::atomic_load(&isDataShareReady));
}

void AudioAdapterManager::UpdateSafeVolumeByS4()
{
    AUDIO_INFO_LOG("Update Safevolume by S4 reboot,reset wired and bt once");
    isWiredBoot_ = true;
    isBtBoot_ = true;
    UpdateSafeVolume();
    SetVolumeDb(STREAM_MUSIC);
}

int32_t AudioAdapterManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevel: appUid: %{public}d, deviceType: %{public}d, volumeLevel:%{public}d",
        appUid, currentActiveDevice_.deviceType_, volumeLevel);
    volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    return SetAppVolumeDb(appUid);
}

int32_t AudioAdapterManager::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevel: appUid: %{public}d, deviceType: %{public}d, muted:%{public}d",
        appUid, currentActiveDevice_.deviceType_, muted);
    volumeDataMaintainer_.SetAppVolumeMuted(appUid, muted);
    return SetAppVolumeMutedDB(appUid, muted);
}

int32_t AudioAdapterManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("appUid: %{public}d, muted: %{public}d", appUid, muted);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume != nullptr, ERR_INVALID_PARAM, "audioVolume handle null");
    bool isSetSuccess = audioVolume->SetAppRingMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(isSetSuccess, ERROR, "set app ring muted: %{public}d fail", muted);
    volumeDataMaintainer_.SetAppStreamMuted(appUid, STREAM_RING, muted);
    return SUCCESS;
}

bool AudioAdapterManager::IsAppRingMuted(int32_t appUid)
{
    AudioStreamType streamType = STREAM_RING;
    return volumeDataMaintainer_.IsAppStreamMuted(appUid, streamType);
}

int32_t AudioAdapterManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(volumeDataMapMutex_);
    volumeAdjustZoneId_ = zoneId;
    if (zoneId == 0) {
        return SUCCESS;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    if (volumeDataExtMaintainer_.find(devices[0]->GetKey()) == volumeDataExtMaintainer_.end()) {
        volumeDataExtMaintainer_[devices[0]->GetKey()] = std::make_shared<VolumeDataMaintainer>();
        if (devices[0]->IsDistributedSpeaker()) {
            for (auto streamType : DISTRIBUTED_VOLUME_TYPE_LIST) {
                int32_t maxVolumeLevel = GetMaxVolumeLevel(streamType);
                volumeDataExtMaintainer_[devices[0]->GetKey()]->SetStreamVolume(streamType, maxVolumeLevel);
                volumeDataExtMaintainer_[devices[0]->GetKey()]->SetStreamMuteStatus(streamType, false);
            }
        } else {
            LoadMuteStatusMap(devices[0]);
            LoadVolumeMap(devices[0]);
        }
        auto iter = defaultVolumeTypeList_.begin();
        while (iter != defaultVolumeTypeList_.end()) {
            SetVolumeDb(devices[0], *iter);
            iter++;
        }
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::GetVolumeAdjustZoneId()
{
    return volumeAdjustZoneId_;
}

int32_t AudioAdapterManager::SetZoneMute(int32_t zoneId, AudioStreamType streamType, bool mute,
    StreamUsage streamUsage, const DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(volumeDataMapMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, streamUsage, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    return SetStreamMuteInternal(devices[0], streamType, mute, streamUsage);
}

bool AudioAdapterManager::GetZoneMute(int32_t zoneId, AudioStreamType streamType)
{
    std::lock_guard<std::mutex> lock(volumeDataMapMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, false,
        "zone device error");
    CHECK_AND_RETURN_RET_LOG(volumeDataExtMaintainer_.find(devices[0]->GetKey()) !=
        volumeDataExtMaintainer_.end(), false, "volumeDataExtMaintainer_ error");
    return GetStreamMuteInternal(devices[0], streamType);
}

int32_t AudioAdapterManager::GetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType)
{
    std::lock_guard<std::mutex> lock(volumeDataMapMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    CHECK_AND_RETURN_RET_LOG(volumeDataExtMaintainer_.find(devices[0]->GetKey()) !=
        volumeDataExtMaintainer_.end(), ERR_OPERATION_FAILED, "volumeDataExtMaintainer_ error");
    if (GetStreamMuteInternal(devices[0], streamType)) {
        return MIN_VOLUME_LEVEL;
    }
    return volumeDataExtMaintainer_[devices[0]->GetKey()]->GetStreamVolume(streamType);
}

int32_t AudioAdapterManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("IsAppVolumeMute: appUid: %{public}d, deviceType: %{public}d, owned:%{public}d",
        appUid, currentActiveDevice_.deviceType_, owned);
    if (owned) {
        volumeDataMaintainer_.GetAppMuteOwned(appUid, isMute);
    } else {
        volumeDataMaintainer_.GetAppMute(appUid, isMute);
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::SetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType, int32_t volumeLevel)
{
    std::lock_guard<std::mutex> lock(volumeDataMapMutex_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1, ERR_OPERATION_FAILED, "zone device error");
    int32_t mimRet = GetMinVolumeLevel(streamType);
    int32_t maxRet = GetMaxVolumeLevel(streamType);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);

    CHECK_AND_RETURN_RET_LOG(volumeDataExtMaintainer_.find(devices[0]->GetKey()) != volumeDataExtMaintainer_.end(),
        ERR_OPERATION_FAILED, "volumeDataExtMaintainer_ error");
    volumeDataExtMaintainer_[devices[0]->GetKey()]->SetStreamVolume(streamType, volumeLevel);
    if (handler_ != nullptr && devices[0]->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        handler_->SendSaveVolume(devices[0]->deviceType_, streamType, volumeLevel, devices[0]->networkId_);
    }
    return SetVolumeDb(devices[0], streamType);
}

int32_t AudioAdapterManager::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel)
{
    Trace trace("KeyAction AudioAdapterManager::SetSystemVolumeLevel streamType:"
        + std::to_string(streamType) + ", volumeLevel:" + std::to_string(volumeLevel));
    AUDIO_INFO_LOG("streamType: %{public}d, deviceType: %{public}d, volumeLevel:%{public}d",
        streamType, currentActiveDevice_.deviceType_, volumeLevel);
    if (currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled) {
        AUDIO_WARNING_LOG("currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled is true!");
        return ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED;
    }
    if (GetSystemVolumeLevel(streamType) == volumeLevel &&
        currentActiveDevice_.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO &&
        currentActiveDevice_.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP && !VolumeUtils::IsPCVolumeEnable()) {
        AUDIO_INFO_LOG("The volume is the same as before.");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("streamType: %{public}d, deviceType: %{public}d, volumeLevel:%{public}d",
        streamType, currentActiveDevice_.deviceType_, volumeLevel);
    if (volumeLevel == 0 && !VolumeUtils::IsPCVolumeEnable() &&
        (streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("this type can not set mute");
        return SUCCESS;
    }
    int32_t mimRet = GetMinVolumeLevel(streamType);
    int32_t maxRet = GetMaxVolumeLevel(streamType);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);

    // Save the volume to volumeLevelMap_.
    volumeDataMaintainer_.SetStreamVolume(streamType, volumeLevel);
    // Save the volume to settingsdata.
    if (currentActiveDevice_.volumeBehavior_.databaseVolumeName != "" && IsDistributedVolumeType(streamType)) {
        volumeDataMaintainer_.SaveVolumeWithDatabaseVolumeName(
            currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType, volumeLevel);
    } else if (handler_ != nullptr) {
        if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_.deviceType_ != DEVICE_TYPE_REMOTE_CAST) {
            AUDIO_INFO_LOG("DualToneStreamType. Save volume for speaker.");
            handler_->SendSaveVolume(DEVICE_TYPE_SPEAKER, streamType, volumeLevel, "LocalDevice");
        } else {
            handler_->SendSaveVolume(currentActiveDevice_.deviceType_, streamType, volumeLevel,
                currentActiveDevice_.networkId_);
        }
    }

    return SetVolumeDb(streamType);
}

int32_t AudioAdapterManager::SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("%{public}s: streamType: %{public}d, currentDeviceType: %{public}d, volumeLevel: %{public}d, "
        "deviceType: %{public}d",  __func__, streamType, currentActiveDevice_.deviceType_, volumeLevel,
        deviceType);
    int32_t mimRet = GetMinVolumeLevel(streamType);
    int32_t maxRet = GetMaxVolumeLevel(streamType);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);
    handler_->SendSaveVolume(deviceType, streamType, volumeLevel);
    return SUCCESS;
}

int32_t AudioAdapterManager::GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetDeviceVolume(deviceType, streamType);
}

void AudioAdapterManager::HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel,
    std::string networkId)
{
    volumeDataMaintainer_.SaveVolume(deviceType, streamType, volumeLevel, networkId);
}

void AudioAdapterManager::HandleSaveVolumeDegree(DeviceType deviceType, AudioStreamType streamType,
    int32_t volumeDegree, std::string networkId)
{
    volumeDataMaintainer_.SaveVolumeDegree(deviceType, streamType, volumeDegree, networkId);
}

void AudioAdapterManager::HandleStreamMuteStatus(AudioStreamType streamType, bool mute, StreamUsage streamUsage,
    const DeviceType &deviceType, std::string networkId)
{
    if (deviceType != DEVICE_TYPE_NONE) {
        volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, mute, networkId);
    } else {
        volumeDataMaintainer_.SaveMuteStatus(currentActiveDevice_.deviceType_, streamType, mute, networkId);
    }
}

void AudioAdapterManager::HandleRingerMode(AudioRingerMode ringerMode)
{
    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(STREAM_RING) * ((ringerMode != RINGER_MODE_NORMAL) ? 0 : 1);
    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(STREAM_RING, volumeLevel);

    volumeDataMaintainer_.SaveRingerMode(ringerMode);
}

void AudioAdapterManager::SetAudioServerProxy(sptr<IStandardAudioService> gsp)
{
    CHECK_AND_RETURN_LOG(gsp != nullptr, "audioServerProxy null");
    audioServerProxy_ = gsp;
}

int32_t AudioAdapterManager::SetAppVolumeDb(int32_t appUid)
{
    int32_t volumeLevel = volumeDataMaintainer_.GetAppVolume(appUid);
    float volumeDb = 1.0f;
    volumeDb = CalculateVolumeDbNonlinear(STREAM_APP, currentActiveDevice_.deviceType_, volumeLevel);
    AUDIO_INFO_LOG("volumeDb:%{public}f volume:%{public}d devicetype:%{public}d",
        volumeDb, volumeLevel, currentActiveDevice_.deviceType_);
    SetAppAudioVolume(appUid, volumeDb);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetAppVolumeMutedDB(int32_t appUid, bool muted)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume != nullptr, ERR_INVALID_PARAM, "audioVolume handle null");
    AUDIO_INFO_LOG("appUid:%{public}d muted:%{public}d devicetype:%{public}d",
        appUid, muted, currentActiveDevice_.deviceType_);
    audioVolume->SetAppVolumeMute(appUid, muted);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetVolumeDb(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType)
{
    int32_t volumeLevel =
        volumeDataExtMaintainer_[device->GetKey()]->GetStreamVolume(streamType) *
        (GetStreamMute(device, streamType) ? 0 : 1);

    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        if (Util::IsDualToneStreamType(streamType) &&
            device->deviceType_ != DEVICE_TYPE_REMOTE_CAST && !VolumeUtils::IsPCVolumeEnable()) {
            volumeDb = CalculateVolumeDbNonlinear(streamType, DEVICE_TYPE_SPEAKER, volumeLevel);
        } else {
            volumeDb = CalculateVolumeDbNonlinear(streamType, device->deviceType_, volumeLevel);
        }
    } else {
        volumeDb = CalculateVolumeDb(volumeLevel);
    }
    // Set voice call assistant stream to full volume
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        volumeDb = 1.0f;
    }
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetSystemVolumeLevel audio adapter null");
    // audio volume
    SetAudioVolume(device, streamType, volumeDb);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetVolumeDb(AudioStreamType streamType)
{
    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(streamType) * (GetStreamMute(streamType) ? 0 : 1);
    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(streamType, volumeLevel);

    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        if (Util::IsDualToneStreamType(streamType) &&
            currentActiveDevice_.deviceType_ != DEVICE_TYPE_REMOTE_CAST && !VolumeUtils::IsPCVolumeEnable()) {
            volumeDb = CalculateVolumeDbNonlinear(streamType, DEVICE_TYPE_SPEAKER, volumeLevel);
        } else {
            volumeDb = CalculateVolumeDbNonlinear(streamType, currentActiveDevice_.deviceType_, volumeLevel);
        }
    } else {
        volumeDb = CalculateVolumeDb(volumeLevel);
    }
    // Set voice call assistant stream to full volume
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        volumeDb = 1.0f;
    }

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetSystemVolumeLevel audio adapter null");

    AUDIO_INFO_LOG("streamType:%{public}d volumeDb:%{public}f volume:%{public}d devicetype:%{public}d",
        streamType, volumeDb, volumeLevel, currentActiveDevice_.deviceType_);

    // audio volume
    SetAudioVolume(streamType, volumeDb);

    return SUCCESS;
}

void AudioAdapterManager::SetAppAudioVolume(int32_t appUid, float volumeDb)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");
    bool isMuted = GetAppMute(appUid);
    int32_t appVolumeLevel = volumeDataMaintainer_.GetAppVolume(appUid) * (isMuted ? 0 : 1);
    AppVolume appVolume(appUid, volumeDb, appVolumeLevel, isMuted);
    audioVolume->SetAppVolume(appVolume);
}

void AudioAdapterManager::SetAudioVolume(AudioStreamType streamType, float volumeDb)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    bool isMuted = GetStreamMute(volumeType);
    int32_t volumeLevel = volumeDataMaintainer_.GetStreamVolume(volumeType) * (isMuted ? 0 : 1);
    if (GetActiveDevice() == DEVICE_TYPE_BLUETOOTH_A2DP && IsAbsVolumeScene() && volumeType == STREAM_MUSIC) {
        isMuted = IsAbsVolumeMute();
        volumeLevel = volumeDataMaintainer_.GetStreamVolume(volumeType) * (isMuted ? 0 : 1);
        volumeDb = isMuted ? 0.0f : 0.63957f; // 0.63957 = -4dB
    }
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");
    if (currentActiveDevice_.IsDistributedSpeaker()) {
        SystemVolume systemVolume(volumeType, REMOTE_CLASS, volumeDb, volumeLevel, isMuted);
        audioVolume->SetSystemVolume(systemVolume);
        SetOffloadVolume(volumeType, volumeDb, REMOTE_CLASS, currentActiveDevice_.networkId_);
        return;
    }
    if (GetActiveDevice() == DEVICE_TYPE_NEARLINK) {
        if (volumeType == STREAM_MUSIC && !isSleVoiceStatus_.load()) {
            isMuted = IsAbsVolumeMute();
            volumeDb = isMuted ? 0.0f : 0.63957f; //  0.63957 = -4dB
        } else if (volumeType == STREAM_VOICE_CALL) {
            volumeDb = 1.0f;
        }
    }
    auto it = DEVICE_CLASS_MAP.find(GetActiveDevice());
    if (it == DEVICE_CLASS_MAP.end()) {
        AUDIO_ERR_LOG("unkown device type %{public}d", GetActiveDevice());
        return;
    }
    for (auto &deviceClass : it->second) {
        SystemVolume systemVolume(volumeType, deviceClass, volumeDb, volumeLevel, isMuted);
        if (deviceClass != OFFLOAD_CLASS) {
            audioVolume->SetSystemVolume(systemVolume);
        } else if (deviceClass == OFFLOAD_CLASS && volumeType == STREAM_MUSIC) {
            audioVolume->SetSystemVolume(systemVolume);
            SetOffloadVolume(volumeType, volumeDb, OFFLOAD_CLASS);
        }
    }
}

void AudioAdapterManager::SetAudioVolume(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, float volumeDb)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    bool isMuted = GetStreamMute(device, volumeType);
    int32_t volumeLevel =
        volumeDataExtMaintainer_[device->GetKey()]->GetStreamVolume(volumeType) *(isMuted ? 0 : 1);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");
    if (device->IsDistributedSpeaker()) {
        SystemVolume systemVolume(volumeType, REMOTE_CLASS, volumeDb, volumeLevel, isMuted);
        audioVolume->SetSystemVolume(systemVolume);
        SetOffloadVolume(volumeType, volumeDb, REMOTE_CLASS, device->networkId_);
        return;
    }
    auto it = DEVICE_CLASS_MAP.find(device->deviceType_);
    if (it == DEVICE_CLASS_MAP.end()) {
        AUDIO_ERR_LOG("unknown device type %{public}d", device->deviceType_);
        return;
    }
    for (auto &deviceClass : it->second) {
        SystemVolume systemVolume(volumeType, deviceClass, volumeDb, volumeLevel, isMuted);
        if (deviceClass != OFFLOAD_CLASS) {
            audioVolume->SetSystemVolume(systemVolume);
        }
    }
}

void AudioAdapterManager::SetOffloadVolume(AudioStreamType streamType, float volumeDb, const std::string &deviceClass,
    const std::string &networkId)
{
    float volume = volumeDb; // maybe only system volume
    if (!(streamType == STREAM_MUSIC || streamType == STREAM_SPEECH)) {
        return;
    }
    CHECK_AND_RETURN_LOG(audioServerProxy_ != nullptr, "audioServerProxy_ null");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (offloadSessionID_.has_value()) { // need stream volume and system volume
        struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        volume = AudioVolume::GetInstance()->GetVolume(offloadSessionID_.value(), streamType, deviceClass, &volumes);
        std::string routeDeviceClass = deviceClass == REMOTE_CLASS ? "remote_offload" : "offload";
        AUDIO_INFO_LOG("routeDeviceClass:%{public}s, networkId:%{public}s, volume:%{public}f", routeDeviceClass.c_str(),
            networkId.c_str(), volume);
        audioServerProxy_->OffloadSetVolume(volume, routeDeviceClass, networkId);
    }
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioAdapterManager::SetOffloadSessionId(uint32_t sessionId)
{
    if (sessionId < MIN_STREAMID || sessionId > MAX_STREAMID) {
        AUDIO_PRERELEASE_LOGE("set sessionId[%{public}d] error", sessionId);
    } else {
        AUDIO_PRERELEASE_LOGI("set sessionId[%{public}d]", sessionId);
    }
    offloadSessionID_ = sessionId;
}

void AudioAdapterManager::ResetOffloadSessionId()
{
    if (offloadSessionID_.has_value()) {
        AUDIO_PRERELEASE_LOGI("reset offload sessionId[%{public}d]", offloadSessionID_.value());
        offloadSessionID_.reset();
    }
}

int32_t AudioAdapterManager::SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel)
{
    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_.deviceType_ != DEVICE_TYPE_REMOTE_CAST) {
            volumeDb = CalculateVolumeDbNonlinear(streamType, DEVICE_TYPE_SPEAKER, volumeLevel);
        } else {
            volumeDb = CalculateVolumeDbNonlinear(streamType, currentActiveDevice_.deviceType_, volumeLevel);
        }
    } else {
        volumeDb = CalculateVolumeDb(volumeLevel);
    }
    SetAudioVolume(streamType, volumeDb);

    return SUCCESS;
}

int32_t AudioAdapterManager::GetSystemVolumeLevel(AudioStreamType streamType)
{
    if (GetStreamMuteInternal(streamType)) {
        return MIN_VOLUME_LEVEL;
    }

    return volumeDataMaintainer_.GetStreamVolume(streamType);
}

int32_t AudioAdapterManager::GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    if (volumeDataMaintainer_.IsSetAppVolume(appUid)) {
        volumeLevel = volumeDataMaintainer_.GetAppVolume(appUid);
    } else {
        volumeLevel = appConfigVolume_.defaultVolume;
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetStreamVolume(streamType);
}

float AudioAdapterManager::GetSystemVolumeDb(AudioStreamType streamType)
{
    int32_t volumeLevel = volumeDataMaintainer_.GetStreamVolume(streamType);
    return CalculateVolumeDb(volumeLevel);
}

int32_t AudioAdapterManager::SetStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage,
    const DeviceType &deviceType, std::string networkId)
{
    return SetStreamMuteInternal(streamType, mute, streamUsage, deviceType, networkId);
}

int32_t AudioAdapterManager::SetStreamMuteInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, bool mute,
    StreamUsage streamUsage, const DeviceType &deviceType)
{
    if (Util::IsDualToneStreamType(streamType) && device->deviceType_ != DEVICE_TYPE_SPEAKER &&
        GetRingerMode() != RINGER_MODE_NORMAL && mute && Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        AUDIO_INFO_LOG("Dual tone stream type %{public}d, current active device:[%{public}d] is no speaker, dont mute",
            streamType, mute);
        return SUCCESS;
    }
    volumeDataExtMaintainer_[device->GetKey()]->SetStreamMuteStatus(streamType, mute);

    if (handler_ != nullptr) {
        handler_->SendStreamMuteStatusUpdate(streamType, mute, streamUsage, deviceType, device->networkId_);
    }
    return SetVolumeDb(device, streamType);
}

int32_t AudioAdapterManager::SetInnerStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("stream type %{public}d, mute:%{public}d, streamUsage:%{public}d", streamType, mute, streamUsage);
    int32_t isSetStreamMute = IsHandleStreamMute(streamType, mute, streamUsage);
    if (isSetStreamMute == SUCCESS) {
        return SUCCESS;
    }
    // set stream mute status to mem.
    volumeDataMaintainer_.SetStreamMuteStatus(streamType, mute);

    if (currentActiveDevice_.deviceType_ == DEVICE_TYPE_NEARLINK) {
        SetAbsVolumeMute(mute);
    }

    int32_t volume = GetSystemVolumeLevel(streamType);
    VolumeEvent volumeEvent = VolumeEvent(streamType, volume, false);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }

    return SetVolumeDb(streamType);
}

int32_t AudioAdapterManager::IsHandleStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage)
{
    if (mute && !VolumeUtils::IsPCVolumeEnable() &&
        (streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("SetStreamMute: this type can not set mute");
        return SUCCESS;
    }
    if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_.deviceType_ != DEVICE_TYPE_SPEAKER &&
        GetRingerMode() != RINGER_MODE_NORMAL && mute && Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        AUDIO_INFO_LOG("Dual tone stream type %{public}d, current active device:[%{public}d] is no speaker, dont mute",
            streamType, mute);
        return SUCCESS;
    }
    return ERROR;
}

int32_t AudioAdapterManager::SetStreamMuteInternal(AudioStreamType streamType, bool mute,
    StreamUsage streamUsage, const DeviceType &deviceType, std::string networkId)
{
    AUDIO_INFO_LOG("stream type %{public}d, mute:%{public}d, streamUsage:%{public}d", streamType, mute, streamUsage);
    if (currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled) {
        AUDIO_WARNING_LOG("currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled is true!");
        return ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED;
    }
    int32_t isSetStreamMute = IsHandleStreamMute(streamType, mute, streamUsage);
    if (isSetStreamMute == SUCCESS) {
        return SUCCESS;
    }

    // set stream mute status to mem.
    volumeDataMaintainer_.SetStreamMuteStatus(streamType, mute);

    if (currentActiveDevice_.volumeBehavior_.databaseVolumeName != "" && IsDistributedVolumeType(streamType)) {
        volumeDataMaintainer_.SaveMuteStatusWithDatabaseVolumeName(
            currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType, mute);
    } else if (handler_ != nullptr) {
        handler_->SendStreamMuteStatusUpdate(streamType, mute, streamUsage, deviceType, networkId);
    }

    // Achieve the purpose of adjusting the mute status by adjusting the stream volume.
    return SetVolumeDb(streamType);
}

int32_t AudioAdapterManager::SetPersistMicMuteState(const bool isMute)
{
    AUDIO_INFO_LOG("Save mute state: %{public}d in setting db", isMute);
    bool res = volumeDataMaintainer_.SaveMicMuteState(isMute);

    return res == true ? SUCCESS : ERROR;
}

int32_t AudioAdapterManager::GetPersistMicMuteState(bool &isMute)
{
    bool res = volumeDataMaintainer_.GetMicMuteState(isMute);
    AUDIO_INFO_LOG("Get mute state from setting db is: %{public}d", isMute);

    return res == true ? SUCCESS : ERROR;
}

int32_t AudioAdapterManager::SetSourceOutputStreamMute(int32_t uid, bool setMute)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetSourceOutputStreamMute audio adapter null");
    return audioServiceAdapter_->SetSourceOutputMute(uid, setMute);
}

bool AudioAdapterManager::GetStreamMute(AudioStreamType streamType)
{
    return GetStreamMuteInternal(streamType);
}

bool AudioAdapterManager::GetStreamMute(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType)
{
    return GetStreamMuteInternal(device, streamType);
}

bool AudioAdapterManager::GetAppMute(int32_t appUid)
{
    bool isMute = false;
    volumeDataMaintainer_.GetAppMute(appUid, isMute);
    return isMute;
}

int32_t AudioAdapterManager::GetStreamVolume(AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetStreamVolume(streamType);
}

int32_t AudioAdapterManager::GetStreamVolume(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType)
{
    return volumeDataExtMaintainer_[device->GetKey()]->GetStreamVolume(streamType);
}

bool AudioAdapterManager::GetStreamMuteInternal(AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetStreamMute(streamType);
}

bool AudioAdapterManager::GetStreamMuteInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType)
{
    return volumeDataExtMaintainer_[device->GetKey()]->GetStreamMute(streamType);
}

// LCOV_EXCL_START
vector<SinkInfo> AudioAdapterManager::GetAllSinks()
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetAllSinks audio adapter null");
        vector<SinkInfo> sinkInputList;
        return sinkInputList;
    }

    return audioServiceAdapter_->GetAllSinks();
}

void AudioAdapterManager::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    AudioPolicyService::GetAudioPolicyService().GetAllSinkInputs(sinkInputs);
}

vector<SourceOutput> AudioAdapterManager::GetAllSourceOutputs()
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetAllSourceOutputs audio adapter null");
        vector<SourceOutput> sourceOutputList;
        return sourceOutputList;
    }

    return audioServiceAdapter_->GetAllSourceOutputs();
}

int32_t AudioAdapterManager::SuspendAudioDevice(std::string &portName, bool isSuspend)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SuspendAudioDevice audio adapter null");

    return audioServiceAdapter_->SuspendAudioDevice(portName, isSuspend);
}

bool AudioAdapterManager::SetSinkMute(const std::string &sinkName, bool isMute, bool isSync)
{
    static std::unordered_map<std::string, std::string> sinkNameMap = {
        {PRIMARY_SPEAKER, PRIMARY_CLASS},
        {OFFLOAD_PRIMARY_SPEAKER, OFFLOAD_CLASS},
        {BLUETOOTH_SPEAKER, A2DP_CLASS},
        {MCH_PRIMARY_SPEAKER, MCH_CLASS},
        {USB_SPEAKER, USB_CLASS},
        {DP_SINK, DP_CLASS},
        {FILE_SINK, FILE_CLASS},
        {REMOTE_CAST_INNER_CAPTURER_SINK_NAME, REMOTE_CAST_INNER_CAPTURER_SINK_NAME},
    };
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, false, "SetSinkMute audio adapter null");
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume, false, "SetSinkMute audioVolume handle null");
    auto it = sinkNameMap.find(sinkName);
    for (auto &volumeType : defaultVolumeTypeList_) {
        if (it != sinkNameMap.end()) {
            if ((it->second == OFFLOAD_CLASS && volumeType == STREAM_MUSIC) ||
                it->second != OFFLOAD_CLASS) {
                audioVolume->SetSystemVolumeMute(volumeType, it->second, isMute);
            }
        } else if (sinkName.find("_out") != std::string::npos &&
            sinkName.find(LOCAL_NETWORK_ID) == std::string::npos) {
            audioVolume->SetSystemVolumeMute(volumeType, REMOTE_CLASS, isMute);
        } else {
            AUDIO_ERR_LOG("unkown sink name %{public}s", sinkName.c_str());
        }
    }

    return audioServiceAdapter_->SetSinkMute(sinkName, isMute, isSync);
}

int32_t AudioAdapterManager::SelectDevice(DeviceRole deviceRole, InternalDeviceType deviceType, std::string name)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SelectDevice audio adapter null");
    switch (deviceRole) {
        case DeviceRole::INPUT_DEVICE:
            return audioServiceAdapter_->SetDefaultSource(name);
        case DeviceRole::OUTPUT_DEVICE: {
            AUDIO_INFO_LOG("SetDefaultSink %{public}d", deviceType);
            return audioServiceAdapter_->SetDefaultSink(name);
        }
        default:
            AUDIO_ERR_LOG("SelectDevice error deviceRole %{public}d", deviceRole);
            return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::SetDeviceActive(InternalDeviceType deviceType,
    std::string name, bool active, DeviceFlag flag)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetDeviceActive audio adapter null");

    switch (deviceType) {
        case InternalDeviceType::DEVICE_TYPE_USB_ARM_HEADSET: {
            if (name == USB_SPEAKER) {
                return audioServiceAdapter_->SetDefaultSink(name);
            } else {
                return audioServiceAdapter_->SetDefaultSource(name);
            }
        }
        default: {
            int32_t ret = SUCCESS;
            int32_t errs[2]{SUCCESS, SUCCESS};
            if (IsInputDevice(deviceType) && (flag & INPUT_DEVICES_FLAG)) {
                AUDIO_INFO_LOG("SetDefaultSource %{public}d", deviceType);
                errs[0] = audioServiceAdapter_->SetDefaultSource(name);
                if (errs[0] != SUCCESS) {
                    AUDIO_ERR_LOG("SetDefaultSource err: %{public}d", errs[0]);
                    ret = errs[0];
                }
            }
            if (IsOutputDevice(deviceType) && (flag & OUTPUT_DEVICES_FLAG)) {
                AUDIO_INFO_LOG("SetDefaultSink %{public}d", deviceType);
                errs[1] = audioServiceAdapter_->SetDefaultSink(name);
                if (errs[1] != SUCCESS) {
                    AUDIO_ERR_LOG("SetDefaultSink err: %{public}d", errs[1]);
                    ret = errs[1];
                }
            }
            // Ensure compatibility across different platforms and versions
            if (errs[0] == SUCCESS || errs[1] == SUCCESS) {
                return SUCCESS;
            }
            return ret;
        }
    }
    return SUCCESS;
}

void AudioAdapterManager::AdjustBluetoothVoiceAssistantVolume(InternalDeviceType deviceType, bool isA2dpSwitchToSco)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP && IsAbsVolumeScene() && !VolumeUtils::IsPCVolumeEnable()) {
        volumeDataMaintainer_.SetStreamVolume(STREAM_VOICE_ASSISTANT, MAX_VOLUME_LEVEL);
        AUDIO_INFO_LOG("a2dp ok");
    }

    if (deviceType == DEVICE_TYPE_BLUETOOTH_SCO && isA2dpSwitchToSco) {
        if (!volumeDataMaintainer_.GetVolume(deviceType, STREAM_VOICE_ASSISTANT)) {
            AUDIO_ERR_LOG("sco voice assistant volume does not exist, use default.");
            volumeDataMaintainer_.SetStreamVolume(STREAM_VOICE_ASSISTANT, DEFAULT_VOLUME_LEVEL);
        } else {
            AUDIO_INFO_LOG("sco ok");
        }
    }
}

int32_t AudioAdapterManager::SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_deviceVolumeBehaviorListenerMutex);
    deviceVolumeBehaviorListener_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    return SUCCESS;
}

void AudioAdapterManager::HandleDistributedDeviceVolume()
{
    if (!currentActiveDevice_.volumeBehavior_.isReady) {
        VolumeBehavior volumeBehavior = AudioDeviceManager::GetAudioDeviceManager().GetDeviceVolumeBehavior(
            currentActiveDevice_.networkId_, currentActiveDevice_.deviceType_);
        currentActiveDevice_.volumeBehavior_ = volumeBehavior;
        if (!currentActiveDevice_.volumeBehavior_.isReady) {
            std::lock_guard<std::mutex> lock(g_deviceVolumeBehaviorListenerMutex);
            CHECK_AND_RETURN_LOG(deviceVolumeBehaviorListener_ != nullptr, "deviceVolumeBehaviorListener_ is nullptr!");
            (void)deviceVolumeBehaviorListener_->OnQueryDeviceVolumeBehavior(volumeBehavior);
            currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled = volumeBehavior.isVolumeControlDisabled;
            currentActiveDevice_.volumeBehavior_.databaseVolumeName = volumeBehavior.databaseVolumeName;
            currentActiveDevice_.volumeBehavior_.isReady = true;
        }
    }

    for (auto streamType : DISTRIBUTED_VOLUME_TYPE_LIST) {
        int32_t maxVolumeLevel = GetMaxVolumeLevel(streamType);
        volumeDataMaintainer_.SetStreamVolume(streamType, maxVolumeLevel);
        volumeDataMaintainer_.SetStreamMuteStatus(streamType, false);
    }

    if (currentActiveDevice_.volumeBehavior_.databaseVolumeName != "") {
        for (auto streamType : DISTRIBUTED_VOLUME_TYPE_LIST) {
            // if GetVolume failed, write the max volume as default value.
            if (!volumeDataMaintainer_.GetVolumeWithDatabaseVolumeName(
                currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType)) {
                int32_t maxVolumeLevel = GetMaxVolumeLevel(streamType);
                volumeDataMaintainer_.SaveVolumeWithDatabaseVolumeName(
                    currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType, maxVolumeLevel);
            }
            if (!volumeDataMaintainer_.GetMuteStatusWithDatabaseVolumeName(
                currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType)) {
                volumeDataMaintainer_.SaveMuteStatusWithDatabaseVolumeName(
                    currentActiveDevice_.volumeBehavior_.databaseVolumeName, streamType, false);
            }
        }
    }

    for (auto streamType : DISTRIBUTED_VOLUME_TYPE_LIST) {
        SetVolumeDb(streamType);
    }
}

bool AudioAdapterManager::IsDistributedVolumeType(AudioStreamType streamType)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    bool ret = std::count(DISTRIBUTED_VOLUME_TYPE_LIST.begin(), DISTRIBUTED_VOLUME_TYPE_LIST.end(), volumeType) != 0;
    return ret;
}

void AudioAdapterManager::SetSleVoiceStatusFlag(bool isSleVoiceStatus)
{
    isSleVoiceStatus_ = isSleVoiceStatus;
    AUDIO_INFO_LOG("SetSleVoiceStatusFlag: %{public}d", isSleVoiceStatus);
}

void AudioAdapterManager::SetVolumeForSwitchDevice(AudioDeviceDescriptor deviceDescriptor)
{
    std::lock_guard<std::mutex> lock(activeDeviceMutex_);
    if (!AudioZoneService::GetInstance().IsZoneDeviceVisible()) {
        std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(deviceDescriptor);
        if (!audioDeviceManager_.IsDeviceConnected(desc)) {
            return;
        }
    }
    // The same device does not set the volume
    bool isSameVolumeGroup = ((GetVolumeGroupForDevice(currentActiveDevice_.deviceType_) ==
        GetVolumeGroupForDevice(deviceDescriptor.deviceType_)) &&
        (currentActiveDevice_.networkId_ == deviceDescriptor.networkId_));
    if ((currentActiveDevice_.deviceType_ == deviceDescriptor.deviceType_) &&
        (currentActiveDevice_.networkId_ == deviceDescriptor.networkId_)) {
        AUDIO_INFO_LOG("Old device: %{public}d. New device: %{public}d. No need to update volume",
            currentActiveDevice_.deviceType_, deviceDescriptor.deviceType_);
        return;
    }

    AUDIO_INFO_LOG("Load volume and mute status for new device %{public}d,"
        "same volume group %{public}d", deviceDescriptor.deviceType_, isSameVolumeGroup);
    // Current device must be updated even if kvStore is nullptr.
    currentActiveDevice_ = deviceDescriptor;
    AudioVolume::GetInstance()->SetCurrentActiveDevice(currentActiveDevice_.deviceType_);

    if (deviceDescriptor.deviceType_ == DEVICE_TYPE_SPEAKER && deviceDescriptor.networkId_ != LOCAL_NETWORK_ID) {
        HandleDistributedDeviceVolume();
        UpdateVolumeForLowLatency();
        return;
    }

    if (!isSameVolumeGroup) {
        // If there's no os account available when trying to get one, audio_server would sleep for 1 sec
        // and retry for 5 times, which could cause a sysfreeze. Check if any os account is ready. If not,
        // skip interacting with datashare.
        bool osAccountReady = volumeDataMaintainer_.CheckOsAccountReady();
        if (osAccountReady) {
            LoadVolumeMap();
            LoadMuteStatusMap();
            UpdateSafeVolume();
        } else {
            AUDIO_WARNING_LOG("Os account is not ready, skip visiting datashare.");
        }
    }

    AdjustBluetoothVoiceAssistantVolume(deviceDescriptor.deviceType_, isSameVolumeGroup);

    auto iter = defaultVolumeTypeList_.begin();
    while (iter != defaultVolumeTypeList_.end()) {
        // update volume level and mute status for each stream type
        SetVolumeDb(*iter);
        AUDIO_INFO_LOG("volume: %{public}d, mute: %{public}d for stream type %{public}d",
            volumeDataMaintainer_.GetStreamVolume(*iter), volumeDataMaintainer_.GetStreamMute(*iter), *iter);
        iter++;
    }

    UpdateVolumeForLowLatency();
}

int32_t AudioAdapterManager::MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName)
{
    return audioServiceAdapter_->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

int32_t AudioAdapterManager::MoveSourceOutputByIndexOrName(uint32_t sourceOutputId, uint32_t sourceIndex,
    std::string sourceName)
{
    return audioServiceAdapter_->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
}

// LCOV_EXCL_STOP
int32_t AudioAdapterManager::SetRingerMode(AudioRingerMode ringerMode)
{
    return SetRingerModeInternal(ringerMode);
}

int32_t AudioAdapterManager::SetRingerModeInternal(AudioRingerMode ringerMode)
{
    AUDIO_INFO_LOG("SetRingerMode: %{public}d", ringerMode);
    ringerMode_ = ringerMode;

    if (handler_ != nullptr) {
        handler_->SendRingerModeUpdate(ringerMode);
    }
    return SUCCESS;
}

AudioRingerMode AudioAdapterManager::GetRingerMode() const
{
    return ringerMode_;
}

bool AudioAdapterManager::IsPaRoute(uint32_t routeFlag)
{
    if ((routeFlag & AUDIO_OUTPUT_FLAG_DIRECT) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_FAST) ||
        (routeFlag & AUDIO_INPUT_FLAG_FAST)) {
        return false;
    }
    return true;
}

void AudioAdapterManager::SaveRingerModeInfo(AudioRingerMode ringMode, std::string callerName,
    std::string invocationTime)
{
    RingerModeAdjustInfo ringerModeAdjustInfo;
    ringerModeAdjustInfo.ringMode = ringMode;
    ringerModeAdjustInfo.callerName = callerName;
    ringerModeAdjustInfo.invocationTime = invocationTime;
    saveRingerModeInfo_->Add(ringerModeAdjustInfo);
}

void AudioAdapterManager::GetRingerModeInfo(std::vector<RingerModeAdjustInfo> &ringerModeInfo)
{
    ringerModeInfo = saveRingerModeInfo_->GetData();
}

std::shared_ptr<AllDeviceVolumeInfo> AudioAdapterManager::GetAllDeviceVolumeInfo(DeviceType deviceType,
    AudioStreamType streamType)
{
    std::shared_ptr<AllDeviceVolumeInfo> deviceVolumeInfo = nullptr;
    if (volumeDataMaintainer_.GetVolume(deviceType, streamType)) {
        deviceVolumeInfo = std::make_shared<AllDeviceVolumeInfo>();
        deviceVolumeInfo->deviceType = deviceType;
        deviceVolumeInfo->streamType = streamType;
        deviceVolumeInfo->volumeValue = volumeDataMaintainer_.GetStreamVolume(streamType);
    }
    return deviceVolumeInfo;
}

// LCOV_EXCL_START
AudioIOHandle AudioAdapterManager::OpenAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(pipeInfo->moduleInfo_);
    AUDIO_INFO_LOG("[PipeExecInfo] pipe name %{public}s, moduleArgs %{public}s",
        pipeInfo->name_.c_str(), moduleArgs.c_str());
    curActiveCount_++;
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    if (IsPaRoute(pipeInfo->routeFlag_)) {
        return OpenPaAudioPort(pipeInfo, paIndex, moduleArgs);
    }
    return OpenNotPaAudioPort(pipeInfo, paIndex);
}

AudioIOHandle AudioAdapterManager::OpenPaAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex,
    std::string moduleArgs)
{
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT) {
        std::string idInfo = GetHdiSinkIdInfo(pipeInfo->moduleInfo_);
        IAudioSinkAttr attr = GetAudioSinkAttr(pipeInfo->moduleInfo_);
        audioServerProxy_->CreateHdiSinkPort(pipeInfo->moduleInfo_.className, idInfo, attr, ioHandle);
    } else if (pipeInfo->pipeRole_ == PIPE_ROLE_INPUT) {
        std::string idInfo = GetHdiSourceIdInfo(pipeInfo->moduleInfo_);
        IAudioSourceAttr attr = GetAudioSourceAttr(pipeInfo->moduleInfo_);
        audioServerProxy_->CreateHdiSourcePort(pipeInfo->moduleInfo_.className, idInfo, attr, ioHandle);
    } else {
        AUDIO_ERR_LOG("Invalid pipe role: %{public}u", pipeInfo->pipeRole_);
    }
    IPCSkeleton::SetCallingIdentity(identity);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        int32_t ret = audioServiceAdapter_->OpenAudioPort(pipeInfo->moduleInfo_.lib, pipeInfo->moduleInfo_);
        paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);
    } else {
        paIndex = audioServiceAdapter_->OpenAudioPort(pipeInfo->moduleInfo_.lib, moduleArgs.c_str());
    }
    AUDIO_INFO_LOG("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end.", ioHandle, paIndex);
    return ioHandle;
}

AudioIOHandle AudioAdapterManager::OpenNotPaAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex)
{
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");
    if (pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT) {
        std::string idInfo = HDI_ID_INFO_DEFAULT;
        HdiIdType idType = HDI_ID_TYPE_PRIMARY;
        GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
        IAudioSinkAttr attr = GetAudioSinkAttr(pipeInfo->moduleInfo_);
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) {
                AUDIO_INFO_LOG("Use voip mmap");
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
            } else {
                attr.audioStreamFlag = AUDIO_FLAG_MMAP;
            }
        } else if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_DIRECT) {
            if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) {
                AUDIO_INFO_LOG("Use voip direct");
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_DIRECT;
            } else {
                AUDIO_INFO_LOG("Use direct");
                attr.audioStreamFlag = AUDIO_FLAG_DIRECT;
            }
        }
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, idType, idInfo, attr, ioHandle);
        IPCSkeleton::SetCallingIdentity(identity);
    } else if (pipeInfo->pipeRole_ == PIPE_ROLE_INPUT) {
        std::string idInfo = HDI_ID_INFO_DEFAULT;
        HdiIdType idType = HDI_ID_TYPE_PRIMARY;
        GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
        IAudioSourceAttr attr = GetAudioSourceAttr(pipeInfo->moduleInfo_);
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_FAST) {
            if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOIP) {
                AUDIO_INFO_LOG("Use voip mmap");
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
            } else {
                attr.audioStreamFlag = AUDIO_FLAG_MMAP;
            }
        }
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        audioServerProxy_->CreateSourcePort(HDI_ID_BASE_CAPTURE, idType, idInfo, attr, ioHandle);
        IPCSkeleton::SetCallingIdentity(identity);
    } else {
        AUDIO_ERR_LOG("Invalid pipe role: %{public}u", pipeInfo->pipeRole_);
    }
    AUDIO_INFO_LOG("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end.", ioHandle, paIndex);
    return ioHandle;
}

void AudioAdapterManager::GetSinkIdInfoAndIdType(
    std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &idInfo, HdiIdType &idType)
{
    if (pipeInfo->adapterName_ == "primary") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_FAST;
            if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) {
                idInfo = HDI_ID_INFO_VOIP;
            }
        } else if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_DIRECT) {
            idType = HDI_ID_TYPE_PRIMARY;
            if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) {
                idInfo = HDI_ID_INFO_VOIP;
            }
        }
    } else if (pipeInfo->adapterName_ == "a2dp") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_BLUETOOTH;
            idInfo = HDI_ID_INFO_MMAP;
        }
    }
}

void AudioAdapterManager::GetSourceIdInfoAndIdType(
    std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &idInfo, HdiIdType &idType)
{
    if (pipeInfo->adapterName_ == "primary") {
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_FAST;
            if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOIP) {
                idInfo = HDI_ID_INFO_VOIP;
            }
        }
    }
}

AudioIOHandle AudioAdapterManager::ReloadAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);
    AUDIO_INFO_LOG("[PipeExecInfo] PA moduleArgs %{public}s", moduleArgs.c_str());

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");
    curActiveCount_++;

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (audioModuleInfo.lib == "libmodule-inner-capturer-sink.z.so") {
        std::string idInfo = audioModuleInfo.name;
        IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
        audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, idInfo, attr, ioHandle);
    } else {
        if (audioModuleInfo.role == HDI_AUDIO_PORT_SINK_ROLE) {
            std::string idInfo = GetHdiSinkIdInfo(audioModuleInfo);
            IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
            audioServerProxy_->CreateHdiSinkPort(audioModuleInfo.className, idInfo, attr, ioHandle);
        } else if (audioModuleInfo.role == HDI_AUDIO_PORT_SOURCE_ROLE) {
            std::string idInfo = GetHdiSourceIdInfo(audioModuleInfo);
            IAudioSourceAttr attr = GetAudioSourceAttr(audioModuleInfo);
            audioServerProxy_->CreateHdiSourcePort(audioModuleInfo.className, idInfo, attr, ioHandle);
        }
    }
    IPCSkeleton::SetCallingIdentity(identity);

    int32_t ret = audioServiceAdapter_->ReloadAudioPort(audioModuleInfo.lib, audioModuleInfo);
    paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);

    AUDIO_INFO_LOG("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
    return ioHandle;
}

AudioIOHandle AudioAdapterManager::OpenAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);
    AUDIO_INFO_LOG("[PipeExecInfo] PA moduleArgs %{public}s", moduleArgs.c_str());

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    curActiveCount_++;
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (audioModuleInfo.lib == "libmodule-inner-capturer-sink.z.so") {
        std::string idInfo = audioModuleInfo.name;
        IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
        audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, idInfo, attr, ioHandle);
    } else {
        if (audioModuleInfo.role == HDI_AUDIO_PORT_SINK_ROLE) {
            std::string idInfo = GetHdiSinkIdInfo(audioModuleInfo);
            IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
            audioServerProxy_->CreateHdiSinkPort(audioModuleInfo.className, idInfo, attr, ioHandle);
        } else if (audioModuleInfo.role == HDI_AUDIO_PORT_SOURCE_ROLE) {
            std::string idInfo = GetHdiSourceIdInfo(audioModuleInfo);
            IAudioSourceAttr attr = GetAudioSourceAttr(audioModuleInfo);
            audioServerProxy_->CreateHdiSourcePort(audioModuleInfo.className, idInfo, attr, ioHandle);
        }
    }
    IPCSkeleton::SetCallingIdentity(identity);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ioHandle, "audioServiceAdapter_ null");
        int32_t ret = audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, audioModuleInfo);
        paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);
    } else {
        paIndex = audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, moduleArgs.c_str());
    }

    AUDIO_INFO_LOG("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
    return ioHandle;
}

int32_t AudioAdapterManager::CloseAudioPort(AudioIOHandle ioHandle, uint32_t paIndex)
{
    AUDIO_INFO_LOG("[PipeExecInfo] ioHandle: %{public}u, paIndex: %{public}u, curCount: %{public}d",
        ioHandle, paIndex, curActiveCount_);
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ERROR, "audioServerProxy_ null");
    curActiveCount_--;
    int32_t ret = audioServiceAdapter_->CloseAudioPort(paIndex);
    AudioIOHandle handleToClose = ioHandle;
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    audioServerProxy_->DestroyHdiPort(ioHandle);
    IPCSkeleton::SetCallingIdentity(identity);
    AUDIO_INFO_LOG("[PipeExecInfo] Close %{public}u port, paIndex: %{public}u end", handleToClose, paIndex);
    return ret;
}

int32_t AudioAdapterManager::GetCurActivateCount() const
{
    return curActiveCount_ > 0 ? curActiveCount_ : 0;
}

int32_t AudioAdapterManager::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) const
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    int32_t ret = 0;
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    ret = audioServiceAdapter_->GetAudioEffectProperty(effectPropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetAudioEffectProperty failed");
    propertyArray.property.insert(propertyArray.property.end(),
        effectPropertyArray.property.begin(), effectPropertyArray.property.end());
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    ret = audioServiceAdapter_->GetAudioEnhanceProperty(enhancePropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetAudioEnhanceProperty failed");
    propertyArray.property.insert(propertyArray.property.end(),
        enhancePropertyArray.property.begin(), enhancePropertyArray.property.end());
    return ret;
}

int32_t AudioAdapterManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) const
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    return audioServiceAdapter_->GetAudioEffectProperty(propertyArray);
}

int32_t AudioAdapterManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType) const
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    return audioServiceAdapter_->GetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t AudioAdapterManager::UpdateCollaborativeState(bool isCollaborationEnabled)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    AUDIO_INFO_LOG("AudioCollaborativeService UpdateCollaborativeState entered!");
    return audioServiceAdapter_->UpdateCollaborativeState(isCollaborationEnabled);
}

void AudioAdapterManager::UpdateSinkArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
{
    if (!audioModuleInfo.name.empty()) {
        args.append(" sink_name=");
        args.append(audioModuleInfo.name);
    }

    if (!audioModuleInfo.adapterName.empty()) {
        args.append(" adapter_name=");
        args.append(audioModuleInfo.adapterName);
    }

    if (!audioModuleInfo.className.empty()) {
        args.append(" device_class=");
        args.append(audioModuleInfo.className);
    }

    if (!audioModuleInfo.fileName.empty()) {
        args.append(" file_path=");
        args.append(audioModuleInfo.fileName);
    }
    if (!audioModuleInfo.sinkLatency.empty()) {
        args.append(" sink_latency=");
        args.append(audioModuleInfo.sinkLatency);
    }

    if (!audioModuleInfo.networkId.empty()) {
        args.append(" network_id=");
        args.append(audioModuleInfo.networkId);
    } else {
        args.append(" network_id=LocalDevice");
    }

    if (!audioModuleInfo.deviceType.empty()) {
        args.append(" device_type=");
        args.append(audioModuleInfo.deviceType);
    }

    if (!audioModuleInfo.extra.empty()) {
        args.append(" split_mode=");
        args.append(audioModuleInfo.extra);
    }
    if (audioModuleInfo.needEmptyChunk) {
        args.append(" need_empty_chunk=");
        args.append(std::to_string(*audioModuleInfo.needEmptyChunk));
    }
}

void UpdateEcAndMicRefArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
{
    if (!audioModuleInfo.ecType.empty()) {
        args.append(" ec_type=");
        args.append(audioModuleInfo.ecType);
    }

    if (!audioModuleInfo.ecAdapter.empty()) {
        args.append(" ec_adapter=");
        args.append(audioModuleInfo.ecAdapter);
    }

    if (!audioModuleInfo.ecSamplingRate.empty()) {
        args.append(" ec_sampling_rate=");
        args.append(audioModuleInfo.ecSamplingRate);
    }

    if (!audioModuleInfo.ecFormat.empty()) {
        args.append(" ec_format=");
        args.append(audioModuleInfo.ecFormat);
    }

    if (!audioModuleInfo.ecChannels.empty()) {
        args.append(" ec_channels=");
        args.append(audioModuleInfo.ecChannels);
    }

    if (!audioModuleInfo.openMicRef.empty()) {
        args.append(" open_mic_ref=");
        args.append(audioModuleInfo.openMicRef);
    }

    if (!audioModuleInfo.micRefRate.empty()) {
        args.append(" mic_ref_rate=");
        args.append(audioModuleInfo.micRefRate);
    }

    if (!audioModuleInfo.micRefFormat.empty()) {
        args.append(" mic_ref_format=");
        args.append(audioModuleInfo.micRefFormat);
    }

    if (!audioModuleInfo.micRefChannels.empty()) {
        args.append(" mic_ref_channels=");
        args.append(audioModuleInfo.micRefChannels);
    }
}

void UpdateSourceArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
{
    if (!audioModuleInfo.name.empty()) {
        args.append(" source_name=");
        args.append(audioModuleInfo.name);
    }

    if (!audioModuleInfo.adapterName.empty()) {
        args.append(" adapter_name=");
        args.append(audioModuleInfo.adapterName);
    }

    if (!audioModuleInfo.className.empty()) {
        args.append(" device_class=");
        args.append(audioModuleInfo.className);
    }

    if (!audioModuleInfo.fileName.empty()) {
        args.append(" file_path=");
        args.append(audioModuleInfo.fileName);
    }

    if (!audioModuleInfo.networkId.empty()) {
        args.append(" network_id=");
        args.append(audioModuleInfo.networkId);
    } else {
        args.append(" network_id=LocalDevice");
    }

    if (!audioModuleInfo.deviceType.empty()) {
        args.append(" device_type=");
        args.append(audioModuleInfo.deviceType);
    }

    if (!audioModuleInfo.sourceType.empty()) {
        args.append(" source_type=");
        args.append(audioModuleInfo.sourceType);
    }
}

void UpdateCommonArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
{
    if (!audioModuleInfo.rate.empty()) {
        args = "rate=";
        args.append(audioModuleInfo.rate);
    }

    if (!audioModuleInfo.channels.empty()) {
        args.append(" channels=");
        args.append(audioModuleInfo.channels);
    }

    if (!audioModuleInfo.bufferSize.empty()) {
        args.append(" buffer_size=");
        args.append(audioModuleInfo.bufferSize);
    }

    if (!audioModuleInfo.format.empty()) {
        args.append(" format=");
        args.append(audioModuleInfo.format);
    }

    if (!audioModuleInfo.fixedLatency.empty()) {
        args.append(" fixed_latency=");
        args.append(audioModuleInfo.fixedLatency);
    }

    if (!audioModuleInfo.renderInIdleState.empty()) {
        args.append(" render_in_idle_state=");
        args.append(audioModuleInfo.renderInIdleState);
    }

    if (!audioModuleInfo.OpenMicSpeaker.empty()) {
        args.append(" open_mic_speaker=");
        args.append(audioModuleInfo.OpenMicSpeaker);
    }

    if (!audioModuleInfo.offloadEnable.empty()) {
        args.append(" offload_enable=");
        args.append(audioModuleInfo.offloadEnable);
    }

    if (!audioModuleInfo.defaultAdapterEnable.empty()) {
        args.append(" default_adapter_enable=");
        args.append(audioModuleInfo.defaultAdapterEnable);
    }
    AUDIO_INFO_LOG("[Adapter load-module] [PolicyManager] common args:%{public}s", args.c_str());
}

// Private Members
std::string AudioAdapterManager::GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const
{
    std::string args;

    if (audioModuleInfo.lib == HDI_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        UpdateSinkArgs(audioModuleInfo, args);
        if (testModeOn_) {
            args.append(" test_mode_on=");
            args.append("1");
        }
    } else if (audioModuleInfo.lib == SPLIT_STREAM_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        UpdateSinkArgs(audioModuleInfo, args);
    } else if (audioModuleInfo.lib == HDI_SOURCE) {
        UpdateCommonArgs(audioModuleInfo, args);
        UpdateSourceArgs(audioModuleInfo, args);
        UpdateEcAndMicRefArgs(audioModuleInfo, args);
    } else if (audioModuleInfo.lib == PIPE_SINK) {
        if (!audioModuleInfo.fileName.empty()) {
            args = "file=";
            args.append(audioModuleInfo.fileName);
        }
    } else if (audioModuleInfo.lib == PIPE_SOURCE) {
        if (!audioModuleInfo.fileName.empty()) {
            args = "file=";
            args.append(audioModuleInfo.fileName);
        }
    } else if (audioModuleInfo.lib == CLUSTER_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        if (!audioModuleInfo.name.empty()) {
            args.append(" sink_name=");
            args.append(audioModuleInfo.name);
        }
    } else if (audioModuleInfo.lib == EFFECT_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        if (!audioModuleInfo.name.empty()) {
            args.append(" sink_name=");
            args.append(audioModuleInfo.name);
        }
        if (!audioModuleInfo.sceneName.empty()) {
            args.append(" scene_name=");
            args.append(audioModuleInfo.sceneName);
        }
    } else if (audioModuleInfo.lib == INNER_CAPTURER_SINK || audioModuleInfo.lib == RECEIVER_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        if (!audioModuleInfo.name.empty()) {
            args.append(" sink_name=");
            args.append(audioModuleInfo.name);
        }
    }
    return args;
}

std::string AudioAdapterManager::GetHdiSinkIdInfo(const AudioModuleInfo &audioModuleInfo) const
{
    if (audioModuleInfo.className == "remote" || audioModuleInfo.className == "remote_offload") {
        return audioModuleInfo.networkId;
    }
    return HDI_ID_INFO_DEFAULT;
}

std::string AudioAdapterManager::GetHdiSourceIdInfo(const AudioModuleInfo &audioModuleInfo) const
{
    if (audioModuleInfo.className == "primary" && audioModuleInfo.sourceType == "SOURCE_TYPE_WAKEUP") {
        return audioModuleInfo.name;
    }
    if (audioModuleInfo.className == "remote") {
        return audioModuleInfo.networkId;
    }
    return HDI_ID_INFO_DEFAULT;
}

static AudioSampleFormat ParseSinkAudioSampleFormat(const std::string &format)
{
    if (format == "u8") {
        return SAMPLE_U8;
    } else if (format == "s16le") {
        return SAMPLE_S16LE;
    } else if (format == "s24le") {
        return SAMPLE_S24LE;
    } else if (format == "s32le") {
        return SAMPLE_S32LE;
    }
    return INVALID_WIDTH;
}

static AudioSampleFormat ParseSourceAudioSampleFormat(const std::string &format)
{
    if (format == "u8") {
        return SAMPLE_U8;
    } else if (format == "s16le" || format == "s16be") {
        return SAMPLE_S16LE;
    } else if (format == "s24le" || format == "s24be") {
        return SAMPLE_S24LE;
    } else if (format == "s32le" || format == "s32be") {
        return SAMPLE_S32LE;
    }
    return SAMPLE_S16LE;
}

static bool IsBigEndian(const std::string &format)
{
    if (format == "s16be" || format == "s24be" || format == "s32be" || format == "f32be") {
        return true;
    }
    return false;
}

IAudioSinkAttr AudioAdapterManager::GetAudioSinkAttr(const AudioModuleInfo &audioModuleInfo) const
{
    IAudioSinkAttr attr;
    attr.adapterName = audioModuleInfo.adapterName.c_str();
    if (!audioModuleInfo.OpenMicSpeaker.empty()) {
        attr.openMicSpeaker = static_cast<uint32_t>(std::stoul(audioModuleInfo.OpenMicSpeaker));
    }
    attr.format = ParseSinkAudioSampleFormat(audioModuleInfo.format);
    if (!audioModuleInfo.rate.empty()) {
        attr.sampleRate = static_cast<uint32_t>(std::stoul(audioModuleInfo.rate));
    }
    if (!audioModuleInfo.channels.empty()) {
        attr.channel = static_cast<uint32_t>(std::stoul(audioModuleInfo.channels));
    }
    attr.volume = HDI_MAX_SINK_VOLUME_LEVEL;
    attr.filePath = audioModuleInfo.fileName.c_str();
    attr.deviceNetworkId = audioModuleInfo.networkId.c_str();
    attr.aux = audioModuleInfo.extra;
    if (!audioModuleInfo.deviceType.empty()) {
        attr.deviceType = std::stoi(audioModuleInfo.deviceType);
    }
    if (audioModuleInfo.className == "multichannel") {
        attr.channelLayout = HDI_DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
    }
    return attr;
}

IAudioSourceAttr AudioAdapterManager::GetAudioSourceAttr(const AudioModuleInfo &audioModuleInfo) const
{
    IAudioSourceAttr attr;
    attr.adapterName = audioModuleInfo.adapterName.c_str();
    if (!audioModuleInfo.OpenMicSpeaker.empty()) {
        attr.openMicSpeaker = static_cast<uint32_t>(std::stoul(audioModuleInfo.OpenMicSpeaker));
    }
    attr.format = ParseSourceAudioSampleFormat(audioModuleInfo.format);
    if (!audioModuleInfo.rate.empty()) {
        attr.sampleRate = static_cast<uint32_t>(std::stoul(audioModuleInfo.rate));
    }
    if (!audioModuleInfo.channels.empty()) {
        attr.channel = static_cast<uint32_t>(std::stoul(audioModuleInfo.channels));
    }
    if (!audioModuleInfo.bufferSize.empty()) {
        attr.bufferSize = static_cast<uint32_t>(std::stoul(audioModuleInfo.bufferSize));
    }
    if (!audioModuleInfo.channelLayout.empty()) {
        AUDIO_INFO_LOG("use custom channelLayout, %{public}s", audioModuleInfo.channelLayout.c_str());
        attr.channelLayout = static_cast<uint64_t>(std::stoul(audioModuleInfo.channelLayout));
    }
    attr.isBigEndian = IsBigEndian(audioModuleInfo.format);
    attr.filePath = audioModuleInfo.fileName.c_str();
    attr.deviceNetworkId = audioModuleInfo.networkId.c_str();
    if (!audioModuleInfo.deviceType.empty()) {
        attr.deviceType = std::stoi(audioModuleInfo.deviceType);
    }
    if (!audioModuleInfo.sourceType.empty()) {
        attr.sourceType = std::stoi(audioModuleInfo.sourceType);
    }
    if ((!audioModuleInfo.ecType.empty()) && static_cast<uint32_t>(std::stoul(audioModuleInfo.ecType)) ==
        HDI_EC_SAME_ADAPTER) {
        attr.hasEcConfig = true;
        attr.formatEc = ParseSourceAudioSampleFormat(audioModuleInfo.ecFormat);
        if (!audioModuleInfo.ecSamplingRate.empty()) {
            attr.sampleRateEc = static_cast<uint32_t>(std::stoul(audioModuleInfo.ecSamplingRate));
        }
        if (!audioModuleInfo.ecChannels.empty()) {
            attr.channelEc = static_cast<uint32_t>(std::stoul(audioModuleInfo.ecChannels));
        }
    }
    return attr;
}

std::string AudioAdapterManager::GetVolumeKeyForKvStore(DeviceType deviceType, AudioStreamType streamType)
{
    DeviceGroup type = GetVolumeGroupForDevice(deviceType);
    std::string typeStr = std::to_string(type);
    CHECK_AND_RETURN_RET_LOG(type != DEVICE_GROUP_INVALID, typeStr,
        "Device %{public}d is not supported for kvStore", deviceType);

    switch (streamType) {
        case STREAM_MUSIC:
            return typeStr + "_music_volume";
        case STREAM_RING:
        case STREAM_VOICE_RING:
            return typeStr + "_ring_volume";
        case STREAM_SYSTEM:
            return typeStr + "_system_volume";
        case STREAM_NOTIFICATION:
            return typeStr + "_notification_volume";
        case STREAM_ALARM:
            return typeStr + "_alarm_volume";
        case STREAM_DTMF:
            return typeStr + "_dtmf_volume";
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
            return typeStr + "_voice_call_volume";
        case STREAM_VOICE_ASSISTANT:
            return typeStr + "_voice_assistant_volume";
        case STREAM_ACCESSIBILITY:
            return typeStr + "_accessibility_volume";
        case STREAM_ULTRASONIC:
            return typeStr + "_ultrasonic_volume";
        case STREAM_WAKEUP:
            return typeStr + "wakeup";
        default:
            AUDIO_ERR_LOG("GetVolumeKeyForKvStore: streamType %{public}d is not supported for kvStore", streamType);
            return "";
    }
}

AudioStreamType AudioAdapterManager::GetStreamIDByType(std::string streamType)
{
    AudioStreamType stream = STREAM_MUSIC;

    if (!streamType.compare(std::string("music")))
        stream = STREAM_MUSIC;
    else if (!streamType.compare(std::string("ring")))
        stream = STREAM_RING;
    else if (!streamType.compare(std::string("voice_call")))
        stream = STREAM_VOICE_CALL;
    else if (!streamType.compare(std::string("system")))
        stream = STREAM_SYSTEM;
    else if (!streamType.compare(std::string("notification")))
        stream = STREAM_NOTIFICATION;
    else if (!streamType.compare(std::string("alarm")))
        stream = STREAM_ALARM;
    else if (!streamType.compare(std::string("voice_assistant")))
        stream = STREAM_VOICE_ASSISTANT;
    else if (!streamType.compare(std::string("accessibility")))
        stream = STREAM_ACCESSIBILITY;
    else if (!streamType.compare(std::string("ultrasonic")))
        stream = STREAM_ULTRASONIC;
    else if (!streamType.compare(std::string("camcorder")))
        stream = STREAM_CAMCORDER;
    return stream;
}

DeviceVolumeType AudioAdapterManager::GetDeviceCategory(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
            return EARPIECE_VOLUME_TYPE;
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_FILE_SOURCE:
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
        case DEVICE_TYPE_ACCESSORY:
            return SPEAKER_VOLUME_TYPE;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return HEADSET_VOLUME_TYPE;
        default:
            return SPEAKER_VOLUME_TYPE;
    }
}

bool AudioAdapterManager::InitAudioPolicyKvStore(bool& isFirstBoot)
{
    DistributedKvDataManager manager;
    Options options;

    AppId appId;
    appId.appId = "audio_policy_manager";

    options.securityLevel = S1;
    options.createIfMissing = false;
    options.encrypt = false;
    options.autoSync = false;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.area = EL1;
    options.baseDir = std::string("/data/service/el1/public/database/") + appId.appId;

    StoreId storeId;
    storeId.storeId = "audiopolicy";
    Status status = Status::SUCCESS;
    std::vector<StoreId> storeIds;
    status = manager.GetAllKvStoreId(appId, storeIds);

    // open and initialize kvstore instance.
    if (audioPolicyKvStore_ == nullptr && storeIds.size() != static_cast<size_t>(0)) {
        uint32_t retries = 0;

        do {
            status = manager.GetSingleKvStore(options, appId, storeId, audioPolicyKvStore_);
            AUDIO_ERR_LOG("GetSingleKvStore status: %{public}d", status);
            if ((status == Status::SUCCESS) || (status == Status::INVALID_ARGUMENT) ||
                (status == Status::DATA_CORRUPTED) || (status == Status::CRYPT_ERROR)) {
                break;
            } else {
                AUDIO_ERR_LOG("InitAudioPolicyKvStore: Kvstore Connect failed! Retrying.");
                retries++;
                usleep(KVSTORE_CONNECT_RETRY_DELAY_TIME);
            }
        } while (retries <= KVSTORE_CONNECT_RETRY_COUNT);
    }

    if (audioPolicyKvStore_ != nullptr) {
        isNeedCopyVolumeData_ = true;
        isNeedCopyMuteData_ = true;
        isNeedCopyRingerModeData_ = true;
        isNeedCopySystemUrlData_ = true;
        SetFirstBoot(false);
        return true;
    }
    // first boot
    char firstboot[3] = {0};
    GetParameter("persist.multimedia.audio.firstboot", "0", firstboot, sizeof(firstboot));
    if (atoi(firstboot) == 1) {
        AUDIO_INFO_LOG("first boot, ready init data to database");
        isFirstBoot = true;
        SetFirstBoot(false);
    }

    return true;
}

void AudioAdapterManager::DeleteAudioPolicyKvStore()
{
    DistributedKvDataManager manager;
    Options options;

    AppId appId;
    appId.appId = "audio_policy_manager";

    options.securityLevel = S1;
    options.createIfMissing = false;
    options.encrypt = false;
    options.autoSync = false;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.area = EL1;
    options.baseDir = std::string("/data/service/el1/public/database/") + appId.appId;

    StoreId storeId;
    storeId.storeId = "audiopolicy";
    Status status = Status::SUCCESS;

    if (audioPolicyKvStore_ != nullptr) {
        status = manager.CloseKvStore(appId, storeId);
        if (status != Status::SUCCESS) {
            AUDIO_ERR_LOG("close KvStore failed");
        }
        status = manager.DeleteKvStore(appId, storeId, options.baseDir);
        if (status != Status::SUCCESS) {
            AUDIO_ERR_LOG("DeleteKvStore failed");
        }
        audioPolicyKvStore_ = nullptr;
    }
}

void AudioAdapterManager::UpdateUsbSafeVolume()
{
    if (volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC) <= safeVolume_) {
        AUDIO_INFO_LOG("1st connect bt device volume is safe");
        isWiredBoot_ = false;
        return;
    }
    if (isWiredBoot_ || safeStatus_) {
        AUDIO_INFO_LOG("1st connect wired device:%{public}d after boot, update current volume to safevolume",
            currentActiveDevice_.deviceType_);
        volumeDataMaintainer_.SetStreamVolume(STREAM_MUSIC, safeVolume_);
        volumeDataMaintainer_.SaveVolume(currentActiveDevice_.deviceType_, STREAM_MUSIC, safeVolume_,
            currentActiveDevice_.networkId_);
        isWiredBoot_ = false;
    }
}

void AudioAdapterManager::UpdateSafeVolume()
{
    switch (currentActiveDevice_.deviceType_) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            UpdateUsbSafeVolume();
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_NEARLINK:
            if (volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC) <= safeVolume_) {
                AUDIO_INFO_LOG("1st connect bt device volume is safe");
                isBtBoot_ = false;
                return;
            }
            if (currentActiveDevice_.deviceCategory_ == BT_CAR || currentActiveDevice_.deviceCategory_ == BT_SOUNDBOX) {
                AUDIO_ERR_LOG("current device: %{public}d is not support", currentActiveDevice_.deviceCategory_);
                return;
            }
            if (isBtBoot_ || safeStatusBt_) {
                AUDIO_INFO_LOG("1st connect bt device:%{public}d after boot, update current volume to safevolume",
                    currentActiveDevice_.deviceType_);
                volumeDataMaintainer_.SetStreamVolume(STREAM_MUSIC, safeVolume_);
                volumeDataMaintainer_.SaveVolume(currentActiveDevice_.deviceType_, STREAM_MUSIC, safeVolume_,
                    currentActiveDevice_.networkId_);
                isBtBoot_ = false;
            }
            break;
        default:
            AUDIO_ERR_LOG("current device: %{public}d is not support", currentActiveDevice_.deviceType_);
            break;
    }
}

void AudioAdapterManager::InitVolumeMap(bool isFirstBoot)
{
    if (!isFirstBoot) {
        LoadVolumeMap();
        UpdateSafeVolume();
        return;
    }
    bool resetFirstFlag = false;
    AUDIO_INFO_LOG("InitVolumeMap: Wrote default stream volumes to KvStore");
    std::unordered_map<AudioStreamType, int32_t> volumeLevelMapTemp = volumeDataMaintainer_.GetVolumeMap();
    for (auto &deviceType: VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType: defaultVolumeTypeList_) {
            // if GetVolume failed, wirte default value
            if (!volumeDataMaintainer_.GetVolume(deviceType, streamType)) {
                int32_t volumeLevel = GetDefaultVolumeLevel(volumeLevelMapTemp, streamType, deviceType);
                auto ret = volumeDataMaintainer_.SaveVolume(deviceType, streamType, volumeLevel);
                resetFirstFlag = ret ? resetFirstFlag : true;
            }

            if (!volumeDataMaintainer_.GetVolumeDegree(deviceType, streamType)) {
                int32_t defVolumeDegree = MIN_VOLUME_LEVEL;
                volumeDataMaintainer_.SaveVolumeDegree(deviceType, streamType, defVolumeDegree);
            }
        }
    }
    if (resetFirstFlag) {
        AUDIO_INFO_LOG("reset first boot init settingsdata");
        SetFirstBoot(true);
    }
    // reLoad the current device volume
    LoadVolumeMap();
    UpdateSafeVolume();
}

// If the device specified by the VolumeType has a default volume level configured,
// use that default volume level. Otherwise, use the default volume level for the VolumeType.
int32_t AudioAdapterManager::GetDefaultVolumeLevel(
    std::unordered_map<AudioStreamType, int32_t> &volumeLevelMapTemp,
    AudioVolumeType volumeType, DeviceType deviceType) const
{
    AudioVolumeType internalVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(volumeType);

    // find the volume level corresponding the the volume type
    auto volumeIt = volumeLevelMapTemp.find(internalVolumeType);
    int32_t defaultVolumeLevel = DEFAULT_VOLUME_LEVEL;
    if (volumeIt != volumeLevelMapTemp.end()) {
        defaultVolumeLevel = volumeIt->second;
    } else {
        AUDIO_ERR_LOG("Failed to get the volume level corresponding to the volume type");
    }

    // find the volume level corresponding to the device specified by the volume type
    int32_t defaultDeviceVolumeLevel = -1;
    auto deviceIt = DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.find(deviceType);
    auto streamVolumeInfoIt = streamVolumeInfos_.find(internalVolumeType);
    if (deviceIt != DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP.end() &&
        streamVolumeInfoIt != streamVolumeInfos_.end()) {
        std::shared_ptr<StreamVolumeInfo> streamVolumeInfo = streamVolumeInfoIt->second;
        DeviceVolumeType deviceVolumeType = deviceIt->second;
        if (streamVolumeInfo != nullptr) {
            auto deviceVolumeInfoIt = streamVolumeInfo->deviceVolumeInfos.find(deviceVolumeType);
            if (deviceVolumeInfoIt != streamVolumeInfo->deviceVolumeInfos.end() &&
                deviceVolumeInfoIt->second != nullptr) {
                defaultDeviceVolumeLevel = deviceVolumeInfoIt->second->defaultLevel;
            } else {
                AUDIO_ERR_LOG("deviceVolumeInfo is nullptr");
            }
        } else {
            AUDIO_ERR_LOG("streamVolumeInfo is nullptr");
        }
    }

    int32_t volumeLevel = (defaultDeviceVolumeLevel == -1) ? defaultVolumeLevel : defaultDeviceVolumeLevel;
    return volumeLevel;
}

void AudioAdapterManager::ResetRemoteCastDeviceVolume()
{
    for (auto &streamType: defaultVolumeTypeList_) {
        AudioStreamType streamAlias = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
        int32_t volumeLevel = GetMaxVolumeLevel(streamAlias);
        volumeDataMaintainer_.SaveVolume(DEVICE_TYPE_REMOTE_CAST, streamType, volumeLevel);
        if (streamType != STREAM_RING) {
            volumeDataMaintainer_.SaveMuteStatus(DEVICE_TYPE_REMOTE_CAST, streamType, false);
        }
    }
}

void AudioAdapterManager::InitRingerMode(bool isFirstBoot)
{
    if (isFirstBoot) {
        isLoaded_ = true;
        if (!volumeDataMaintainer_.GetRingerMode(ringerMode_)) {
            isLoaded_ = volumeDataMaintainer_.SaveRingerMode(ringerMode_);
        }
        AUDIO_INFO_LOG("InitRingerMode first boot ringermode:%{public}d", ringerMode_);
    } else {
        // read ringerMode from private kvStore
        if (isNeedCopyRingerModeData_ && audioPolicyKvStore_ != nullptr) {
            AUDIO_INFO_LOG("copy ringerMode from private database to share database");
            Key key = "ringermode";
            Value value;
            Status status = audioPolicyKvStore_->Get(key, value);
            if (status == Status::SUCCESS) {
                ringerMode_ = static_cast<AudioRingerMode>(TransferByteArrayToType<int>(value.Data()));
                volumeDataMaintainer_.SaveRingerMode(ringerMode_);
            }
            isNeedCopyRingerModeData_ = false;
        }
        // if read ringer mode success, data is loaded.
        isLoaded_ = volumeDataMaintainer_.GetRingerMode(ringerMode_);
    }

    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(STREAM_RING) * ((ringerMode_ != RINGER_MODE_NORMAL) ? 0 : 1);
    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(STREAM_RING, volumeLevel);
}

void AudioAdapterManager::CloneVolumeMap(void)
{
    CHECK_AND_RETURN_LOG(audioPolicyKvStore_ != nullptr, "clone volumemap failed, audioPolicyKvStore_nullptr");
    // read volume from private Kvstore
    AUDIO_INFO_LOG("Copy Volume from private database to shareDatabase");
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType : defaultVolumeTypeList_) {
            std::string volumeKey = GetVolumeKeyForKvStore(deviceType, streamType);
            Key key = volumeKey;
            Value value;
            Status status = audioPolicyKvStore_->Get(volumeKey, value);
            if (status != SUCCESS) {
                AUDIO_WARNING_LOG("get volumeLevel failed, deviceType:%{public}d, streanType:%{public}d",
                    deviceType, streamType);
                continue;
            }
            int32_t volumeLevel = TransferByteArrayToType<int>(value.Data());
            // clone data to VolumeToShareData
            volumeDataMaintainer_.SaveVolume(deviceType, streamType, volumeLevel);
        }
    }

    isNeedCopyVolumeData_ = false;
}

void AudioAdapterManager::HandleDistributedVolume(AudioStreamType streamType)
{
    if (currentActiveDevice_.IsDistributedSpeaker()) {
        AUDIO_INFO_LOG("distributed device first connect, use default volume");
        if (streamType == STREAM_MUSIC || streamType == STREAM_VOICE_CALL ||
            streamType == STREAM_VOICE_ASSISTANT) {
            volumeDataMaintainer_.SetStreamVolume(streamType, MAX_VOLUME_LEVEL);
            SetSystemVolumeLevel(streamType, MAX_VOLUME_LEVEL);
        }
    }

    if ((currentActiveDevice_.deviceType_ == DEVICE_TYPE_DP || currentActiveDevice_.deviceType_ == DEVICE_TYPE_HDMI)
        && streamType == STREAM_MUSIC) {
        AUDIO_INFO_LOG("first time switch dp or hdmi, use default volume");
        int32_t initialVolume = GetMaxVolumeLevel(streamType) > MAX_VOLUME_LEVEL ?
            DP_DEFAULT_VOLUME_LEVEL : GetMaxVolumeLevel(streamType);
        volumeDataMaintainer_.SetStreamVolume(STREAM_MUSIC, initialVolume);
        SetSystemVolumeLevel(STREAM_MUSIC, initialVolume);
    }
}

void AudioAdapterManager::HandleDpConnection()
{
    AUDIO_INFO_LOG("dp device connect, set max volume of stream music");
    isDpReConnect_ = true;
}

void AudioAdapterManager::RefreshVolumeWhenDpReConnect()
{
    // dp reconnect need to set max volume
    AUDIO_INFO_LOG("DP reconnect, set max volume");
    SetSystemVolumeLevel(STREAM_MUSIC, GetMaxVolumeLevel(STREAM_MUSIC));
    SetSystemVolumeLevel(STREAM_VOICE_CALL, GetMaxVolumeLevel(STREAM_VOICE_CALL));
    SetSystemVolumeLevel(STREAM_VOICE_ASSISTANT, GetMaxVolumeLevel(STREAM_VOICE_ASSISTANT));
    isDpReConnect_ = false;
}

bool AudioAdapterManager::LoadVolumeMap(void)
{
    if (isNeedCopyVolumeData_ && (audioPolicyKvStore_ != nullptr)) {
        CloneVolumeMap();
    }

    bool result = false;
    for (auto &streamType: defaultVolumeTypeList_) {
        if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_.deviceType_ != DEVICE_TYPE_REMOTE_CAST) {
            result = volumeDataMaintainer_.GetVolume(DEVICE_TYPE_SPEAKER, streamType, currentActiveDevice_.networkId_);
            volumeDataMaintainer_.GetVolumeDegree(DEVICE_TYPE_SPEAKER, streamType, currentActiveDevice_.networkId_);
        } else {
            result = volumeDataMaintainer_.GetVolume(currentActiveDevice_.deviceType_, streamType,
                currentActiveDevice_.networkId_);
            volumeDataMaintainer_.GetVolumeDegree(currentActiveDevice_.deviceType_, streamType,
                currentActiveDevice_.networkId_);
        }
        if (!result) {
            AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for streamType[%{public}d] from kvStore", streamType);
            HandleDistributedVolume(streamType);
            HandleHearingAidVolume(streamType);
        }
    }

    return true;
}

bool AudioAdapterManager::LoadVolumeMap(std::shared_ptr<AudioDeviceDescriptor> &device)
{
    bool result = false;
    for (auto &streamType: defaultVolumeTypeList_) {
        result = volumeDataExtMaintainer_[device->GetKey()]->GetVolume(device->deviceType_, streamType);
        if (!result) {
            AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for streamType[%{public}d] from kvStore", streamType);
        }
    }

    return true;
}

void AudioAdapterManager::TransferMuteStatus(void)
{
    // read mute_streams_affected and transfer
    int32_t mute_streams_affected = 0;
    bool isNeedTransferMute = true;
    bool ret = volumeDataMaintainer_.GetMuteAffected(mute_streams_affected) &&
        volumeDataMaintainer_.GetMuteTransferStatus(isNeedTransferMute);
    if (!ret && (mute_streams_affected > 0) && isNeedTransferMute) {
        AUDIO_INFO_LOG("start transfer mute value");
        volumeDataMaintainer_.SetMuteAffectedToMuteStatusDataBase(mute_streams_affected);
        volumeDataMaintainer_.SaveMuteTransferStatus(false);
    }
}

void AudioAdapterManager::InitMuteStatusMap(bool isFirstBoot)
{
    if (isFirstBoot) {
        for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
            for (auto &streamType : defaultVolumeTypeList_) {
                CheckAndDealMuteStatus(deviceType, streamType);
            }
        }
        TransferMuteStatus();
    } else {
        LoadMuteStatusMap();
    }
}

void  AudioAdapterManager::CheckAndDealMuteStatus(const DeviceType &deviceType, const AudioStreamType &streamType)
{
    if (streamType == STREAM_RING) {
        bool muteStateForStreamRing = (ringerMode_ == RINGER_MODE_NORMAL) ? false : true;
        AUDIO_INFO_LOG("fist boot ringer mode:%{public}d, stream ring mute state:%{public}d", ringerMode_,
            muteStateForStreamRing);
        // set stream mute status to mem.
        if (currentActiveDevice_.deviceType_ == deviceType) {
            volumeDataMaintainer_.SetStreamMuteStatus(streamType, muteStateForStreamRing);
        }
        volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, muteStateForStreamRing,
            currentActiveDevice_.networkId_);
    } else if (!volumeDataMaintainer_.GetMuteStatus(deviceType, streamType)) {
        if (currentActiveDevice_.deviceType_ == deviceType) {
            volumeDataMaintainer_.SetStreamMuteStatus(streamType, false);
        }
        volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, false, currentActiveDevice_.networkId_);
    }
    if (currentActiveDevice_.deviceType_ == deviceType) {
        SetVolumeDb(streamType);
    }
}

void AudioAdapterManager::SetVolumeCallbackAfterClone()
{
    for (auto &streamType : defaultVolumeTypeList_) {
        VolumeEvent volumeEvent;
        volumeEvent.volumeType = streamType;
        volumeEvent.volume = GetSystemVolumeLevel(streamType);
        volumeEvent.updateUi = false;
        volumeEvent.volumeGroupId = 0;
        volumeEvent.networkId = LOCAL_NETWORK_ID;
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
        }
    }
}

void AudioAdapterManager::CloneMuteStatusMap(void)
{
    // read mute status from private Kvstore
    CHECK_AND_RETURN_LOG(audioPolicyKvStore_ != nullptr, "clone mute status failed, audioPolicyKvStore_ nullptr");
    AUDIO_INFO_LOG("Copy mute from private database to shareDatabase");
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType : defaultVolumeTypeList_) {
            std::string muteKey = GetMuteKeyForKvStore(deviceType, streamType);
            Key key = muteKey;
            Value value;
            Status status = audioPolicyKvStore_->Get(key, value);
            if (status != SUCCESS) {
                AUDIO_WARNING_LOG("get muteStatus:failed, deviceType:%{public}d, streanType:%{public}d",
                    deviceType, streamType);
                continue;
            }
            bool muteStatus = TransferByteArrayToType<int>(value.Data());
            // clone data to VolumeToShareData
            if (currentActiveDevice_.deviceType_ == deviceType) {
                volumeDataMaintainer_.SetStreamMuteStatus(streamType, muteStatus);
            }
            volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, muteStatus, currentActiveDevice_.networkId_);
        }
    }
    isNeedCopyMuteData_ = false;
}

bool AudioAdapterManager::LoadMuteStatusMap(void)
{
    if (isNeedCopyMuteData_ && (audioPolicyKvStore_ != nullptr)) {
        CloneMuteStatusMap();
    }

    TransferMuteStatus();

    for (auto &streamType: defaultVolumeTypeList_) {
        bool result = volumeDataMaintainer_.GetMuteStatus(currentActiveDevice_.deviceType_, streamType,
            currentActiveDevice_.networkId_);
        if (!result) {
            AUDIO_WARNING_LOG("Could not load mute status for stream type %{public}d from database.", streamType);
        }
        if (streamType == STREAM_RING && VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_RING) {
            bool muteStateForStreamRing = (ringerMode_ == RINGER_MODE_NORMAL) ? false : true;
            if (currentActiveDevice_.deviceType_ != DEVICE_TYPE_SPEAKER) {
                continue;
            }
            AUDIO_INFO_LOG("ringer mode:%{public}d, stream ring mute state:%{public}d", ringerMode_,
                muteStateForStreamRing);
            if (muteStateForStreamRing == GetStreamMute(streamType)) {
                continue;
            }
            // set local speaker mute state in ring scene when ringermode change
            volumeDataMaintainer_.SaveMuteStatus(currentActiveDevice_.deviceType_, streamType, muteStateForStreamRing);
            SetStreamMute(streamType, muteStateForStreamRing);
        }
    }
    return true;
}

bool AudioAdapterManager::LoadMuteStatusMap(std::shared_ptr<AudioDeviceDescriptor> &device)
{
    for (auto &streamType: defaultVolumeTypeList_) {
        bool result = volumeDataExtMaintainer_[device->GetKey()]->GetMuteStatus(device->deviceType_, streamType);
        if (!result) {
            AUDIO_WARNING_LOG("Could not load mute status for stream type %{public}d from database.", streamType);
        }
    }
    return true;
}

void AudioAdapterManager::InitSafeStatus(bool isFirstBoot)
{
    if (isFirstBoot) {
        AUDIO_INFO_LOG("Wrote default safe status to KvStore");
        for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
            // Adapt to safe volume upgrade scenarios
            if (!volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_) &&
                (deviceType == DEVICE_TYPE_WIRED_HEADSET)) {
                volumeDataMaintainer_.SaveSafeStatus(DEVICE_TYPE_WIRED_HEADSET, SAFE_ACTIVE);
            }
            if (!volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_) &&
                (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP)) {
                volumeDataMaintainer_.SaveSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, SAFE_ACTIVE);
            }
        }
    } else {
        volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);
        volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
    }
}

void AudioAdapterManager::InitSafeTime(bool isFirstBoot)
{
    if (isFirstBoot) {
        AUDIO_INFO_LOG("Wrote default safe status to KvStore");
        for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
            if (!volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, safeActiveTime_) &&
                (deviceType == DEVICE_TYPE_WIRED_HEADSET)) {
                volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, 0);
            }
            if (!volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtTime_) &&
                (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP)) {
                volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, 0);
            }
            ConvertSafeTime();
            isNeedConvertSafeTime_ = false;
        }
    } else {
        volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, safeActiveTime_);
        volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtTime_);
        if (isNeedConvertSafeTime_) {
            ConvertSafeTime();
            isNeedConvertSafeTime_ = false;
        }
    }
}

void AudioAdapterManager::ConvertSafeTime(void)
{
    // Adapt to safe volume time when upgrade scenarios
    if (safeActiveTime_ > 0) {
        safeActiveTime_ = safeActiveTime_ / CONVERT_FROM_MS_TO_SECONDS;
        volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, safeActiveTime_);
    }
    if (safeActiveBtTime_ > 0) {
        safeActiveBtTime_ = safeActiveBtTime_ / CONVERT_FROM_MS_TO_SECONDS;
        volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtTime_);
    }
}

SafeStatus AudioAdapterManager::GetCurrentDeviceSafeStatus(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);
            return safeStatus_;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_NEARLINK:
            volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
            return safeStatusBt_;
        default:
            AUDIO_ERR_LOG("current device : %{public}d is not support", deviceType);
            break;
    }

    return SAFE_UNKNOWN;
}

int64_t AudioAdapterManager::GetCurentDeviceSafeTime(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, safeActiveTime_);
            return safeActiveTime_;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtTime_);
            return safeActiveBtTime_;
        default:
            AUDIO_ERR_LOG("current device : %{public}d is not support", deviceType);
            break;
    }

    return -1;
}

int32_t AudioAdapterManager::GetRestoreVolumeLevel(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            volumeDataMaintainer_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, safeActiveVolume_);
            return safeActiveVolume_;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            volumeDataMaintainer_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtVolume_);
            return safeActiveBtVolume_;
        default:
            AUDIO_ERR_LOG("current device : %{public}d is not support", deviceType);
            break;
    }

    return SAFE_UNKNOWN;
}

int32_t AudioAdapterManager::SetDeviceSafeStatus(DeviceType deviceType, SafeStatus status)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        safeStatusBt_ = status;
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        safeStatus_ = status;
    }
    bool ret = volumeDataMaintainer_.SaveSafeStatus(deviceType, status);
    CHECK_AND_RETURN_RET(ret, ERROR, "SaveSafeStatus failed");
    return SUCCESS;
}

int32_t AudioAdapterManager::SetDeviceSafeTime(DeviceType deviceType, int64_t time)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        safeActiveBtTime_ = time;
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        safeActiveTime_ = time;
    }
    bool ret = volumeDataMaintainer_.SaveSafeVolumeTime(deviceType, time);
    CHECK_AND_RETURN_RET(ret, ERROR, "SetDeviceSafeTime failed");
    return SUCCESS;
}

int32_t AudioAdapterManager::SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        safeActiveBtVolume_ = volume;
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        safeActiveVolume_ = volume;
    }
    bool ret = volumeDataMaintainer_.SetRestoreVolumeLevel(deviceType, volume);
    CHECK_AND_RETURN_RET(ret, ERROR, "SetRestoreVolumeLevel failed");
    return SUCCESS;
}

std::string AudioAdapterManager::GetMuteKeyForKvStore(DeviceType deviceType, AudioStreamType streamType)
{
    std::string type = "";
    GetMuteKeyForDeviceType(deviceType, type);
    if (type == "") {
        return type;
    }

    switch (streamType) {
        case STREAM_MUSIC:
            return type + "_music_mute_status";
        case STREAM_RING:
        case STREAM_VOICE_RING:
            return type + "_ring_mute_status";
        case STREAM_SYSTEM:
            return type + "_system_mute_status";
        case STREAM_NOTIFICATION:
            return type + "_notification_mute_status";
        case STREAM_ALARM:
            return type + "_alarm_mute_status";
        case STREAM_DTMF:
            return type + "_dtmf_mute_status";
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
            return type + "_voice_call_mute_status";
        case STREAM_VOICE_ASSISTANT:
            return type + "_voice_assistant_mute_status";
        case STREAM_ACCESSIBILITY:
            return type + "_accessibility_mute_status";
        case STREAM_ULTRASONIC:
            return type + "_unltrasonic_mute_status";
        default:
            AUDIO_ERR_LOG("GetMuteKeyForKvStore: streamType %{public}d is not supported for kvStore", streamType);
            return "";
    }
}

std::string AudioAdapterManager::GetMuteKeyForDeviceType(DeviceType deviceType, std::string &type)
{
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
            type = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_ACCESSORY:
            type = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            type = "wired";
            break;
        default:
            AUDIO_ERR_LOG("GetMuteKeyForKvStore: device %{public}d is not supported for kvStore", deviceType);
            return "";
    }
    return type;
}

float AudioAdapterManager::CalculateVolumeDb(int32_t volumeLevel, int32_t maxDegree)
{
    if (maxDegree == 0) {
        maxDegree = MAX_VOLUME_LEVEL;
    }

    float value = static_cast<float>(volumeLevel) / maxDegree;
    float roundValue = static_cast<int>(value * CONST_FACTOR);

    return static_cast<float>(roundValue) / CONST_FACTOR;
}

void AudioAdapterManager::CloneSystemSoundUrl(void)
{
    CHECK_AND_RETURN_LOG(isNeedCopySystemUrlData_ && (audioPolicyKvStore_ != nullptr),
        "audioPolicyKvStore_ is nullptr,clone systemurl failed");
    for (auto &key: SYSTEM_SOUND_KEY_LIST) {
        Value value;
        Status status = audioPolicyKvStore_->Get(key, value);
        if (status == Status::SUCCESS) {
            std::string systemSoundUri = value.ToString();
            systemSoundUriMap_[key] = systemSoundUri;
            volumeDataMaintainer_.SaveSystemSoundUrl(key, systemSoundUri);
        }
    }
    isNeedCopySystemUrlData_ = false;
}

void AudioAdapterManager::InitSystemSoundUriMap()
{
    for (auto &key: SYSTEM_SOUND_KEY_LIST) {
        std::string systemSoundUri = "";
        volumeDataMaintainer_.GetSystemSoundUrl(key, systemSoundUri);
        if (systemSoundUri == "") {
            AUDIO_WARNING_LOG("Could not load system sound uri for %{public}s from kvStore", key.c_str());
        }
        systemSoundUriMap_[key] = systemSoundUri;
    }
}

int32_t AudioAdapterManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    auto pos = std::find(SYSTEM_SOUND_KEY_LIST.begin(), SYSTEM_SOUND_KEY_LIST.end(), key);
    if (pos == SYSTEM_SOUND_KEY_LIST.end()) {
        AUDIO_ERR_LOG("Invalid key %{public}s for system sound uri", key.c_str());
        return ERR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(systemSoundMutex_);
    if (systemSoundUriMap_.size() == 0) {
        InitSystemSoundUriMap();
        CHECK_AND_RETURN_RET_LOG(systemSoundUriMap_.size() != 0, ERR_OPERATION_FAILED,
            "Failed to init system sound uri map.");
    }
    systemSoundUriMap_[key] = uri;
    if (!volumeDataMaintainer_.SaveSystemSoundUrl(key, uri)) {
        AUDIO_ERR_LOG("SetSystemSoundUri failed");
        return ERROR;
    }
    return SUCCESS;
}

std::string AudioAdapterManager::GetSystemSoundUri(const std::string &key)
{
    auto pos = std::find(SYSTEM_SOUND_KEY_LIST.begin(), SYSTEM_SOUND_KEY_LIST.end(), key);
    if (pos == SYSTEM_SOUND_KEY_LIST.end()) {
        AUDIO_ERR_LOG("Invalid key %{public}s for system sound uri", key.c_str());
        return "";
    }
    std::lock_guard<std::mutex> lock(systemSoundMutex_);
    if (systemSoundUriMap_.size() == 0) {
        InitSystemSoundUriMap();
        CHECK_AND_RETURN_RET_LOG(systemSoundUriMap_.size() != 0, "",
            "Failed to init system sound uri map.");
    }
    return systemSoundUriMap_[key];
}

float AudioAdapterManager::GetMinStreamVolume() const
{
    return MIN_STREAM_VOLUME;
}

float AudioAdapterManager::GetMaxStreamVolume() const
{
    return MAX_STREAM_VOLUME;
}

bool AudioAdapterManager::IsVolumeUnadjustable()
{
    return isVolumeUnadjustable_;
}

float AudioAdapterManager::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    AUDIO_DEBUG_LOG("for volumeType: %{public}d deviceType:%{public}d volumeLevel:%{public}d",
        volumeType, deviceType, volumeLevel);
    if (useNonlinearAlgo_) {
        getSystemVolumeInDb_ = CalculateVolumeDbNonlinear(volumeType, deviceType, volumeLevel);
    } else {
        getSystemVolumeInDb_ = CalculateVolumeDb(volumeLevel);
    }

    AUDIO_DEBUG_LOG("Get system volume in db success %{public}f", getSystemVolumeInDb_.load());

    return getSystemVolumeInDb_;
}

uint32_t AudioAdapterManager::GetPositionInVolumePoints(std::vector<VolumePoint> &volumePoints, int32_t idx)
{
    int32_t leftPos = 0;
    int32_t rightPos = static_cast<int32_t>(volumePoints.size() - 1);
    while (leftPos <= rightPos) {
        int32_t midPos = leftPos + (rightPos - leftPos)/NUMBER_TWO;
        int32_t c = static_cast<int32_t>(volumePoints[midPos].index) - idx;
        if (c == 0) {
            leftPos = midPos;
            break;
        } else if (c < 0) {
            leftPos = midPos + 1;
        } else {
            rightPos = midPos - 1;
        }
    }
    return leftPos;
}

float AudioAdapterManager::CalculateVolumeDbNonlinear(AudioStreamType streamType,
    DeviceType deviceType, int32_t volumeLevel)
{
    AUDIO_DEBUG_LOG("CalculateVolumeDbNonlinear for stream: %{public}d devicetype:%{public}d volumeLevel:%{public}d",
        streamType, deviceType, volumeLevel);
    AudioStreamType streamAlias = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t minVolIndex = GetMinVolumeLevel(streamAlias);
    int32_t maxVolIndex = GetMaxVolumeLevel(streamAlias);
    if (minVolIndex < 0 || maxVolIndex < 0 || minVolIndex >= maxVolIndex) {
        return 0.0f;
    }
    if (volumeLevel < minVolIndex) {
        volumeLevel = minVolIndex;
    }
    if (volumeLevel > maxVolIndex) {
        volumeLevel = maxVolIndex;
    }

    DeviceVolumeType deviceCategory = GetDeviceCategory(deviceType);
    std::vector<VolumePoint> volumePoints;
    GetVolumePoints(streamAlias, deviceCategory, volumePoints);
    uint32_t pointSize = volumePoints.size();

    CHECK_AND_RETURN_RET_LOG(pointSize != 0, 1.0f, "pointSize is 0");
    int32_t volSteps = static_cast<int32_t>(1 + volumePoints[pointSize - 1].index - volumePoints[0].index);
    int32_t idxRatio = (volSteps * (volumeLevel - minVolIndex)) / (maxVolIndex - minVolIndex);
    int32_t position = static_cast<int32_t>(GetPositionInVolumePoints(volumePoints, idxRatio));
    if (position == 0) {
        if (minVolIndex != 0) {
            AUDIO_INFO_LOG("Min volume index not zero, use min db: %{public}0.1f", volumePoints[0].dbValue / 100.0f);
            return exp((volumePoints[0].dbValue / 100.0f) * 0.115129f);
        }
        AUDIO_DEBUG_LOG("position = 0, return 0.0");
        return 0.0f;
    } else if (position >= static_cast<int32_t>(pointSize)) {
        AUDIO_DEBUG_LOG("position > pointSize, return %{public}f",
            exp(volumePoints[pointSize - 1].dbValue * 0.115129f));
        return exp((volumePoints[pointSize - 1].dbValue / 100.0f) * 0.115129f);
    }
    float indexFactor = (static_cast<float>(idxRatio - static_cast<int32_t>(volumePoints[position - 1].index))) /
        (static_cast<float>(volumePoints[position].index - volumePoints[position - 1].index));

    float dbValue = (volumePoints[position - 1].dbValue / 100.0f) +
        indexFactor * ((volumePoints[position].dbValue / 100.0f) - (volumePoints[position - 1].dbValue / 100.0f));

    AUDIO_DEBUG_LOG(" index=[%{public}d, %{public}d, %{public}d]"
        "db=[%{public}0.1f %{public}0.1f %{public}0.1f] factor=[%{public}f]",
        volumePoints[position - 1].index, idxRatio, volumePoints[position].index,
        (static_cast<float>(volumePoints[position - 1].dbValue) / 100.0f), dbValue,
        (static_cast<float>(volumePoints[position].dbValue) / 100.0f), exp(dbValue * 0.115129f));

    return exp(dbValue * 0.115129f);
}

void AudioAdapterManager::InitVolumeMapIndex()
{
    useNonlinearAlgo_ = 0;
    for (auto streamType : defaultVolumeTypeList_) {
        minVolumeIndexMap_[VolumeUtils::GetVolumeTypeFromStreamType(streamType)] = MIN_VOLUME_LEVEL;
        maxVolumeIndexMap_[VolumeUtils::GetVolumeTypeFromStreamType(streamType)] = MAX_VOLUME_LEVEL;
        volumeDataMaintainer_.SetStreamVolume(streamType, DEFAULT_VOLUME_LEVEL);
        AUDIO_DEBUG_LOG("streamType %{public}d index = [%{public}d, %{public}d, %{public}d]",
            streamType, minVolumeIndexMap_[VolumeUtils::GetVolumeTypeFromStreamType(streamType)],
            maxVolumeIndexMap_[VolumeUtils::GetVolumeTypeFromStreamType(streamType)],
            volumeDataMaintainer_.GetStreamVolume(streamType));
    }

    volumeDataMaintainer_.SetStreamVolume(STREAM_VOICE_CALL_ASSISTANT, MAX_VOLUME_LEVEL);
    volumeDataMaintainer_.SetStreamVolume(STREAM_ULTRASONIC, MAX_VOLUME_LEVEL);
}

void AudioAdapterManager::UpdateVolumeMapIndex()
{
    bool isAppConfigVolumeInit = false;
    for (auto streamVolInfoPair : streamVolumeInfos_) {
        auto streamVolInfo = streamVolInfoPair.second;
        if (streamVolInfo->streamType == STREAM_APP) {
            appConfigVolume_.defaultVolume = streamVolInfo->defaultLevel;
            appConfigVolume_.maxVolume = streamVolInfo->maxLevel;
            appConfigVolume_.minVolume = streamVolInfo->minLevel;
            isAppConfigVolumeInit = true;
            AUDIO_DEBUG_LOG("AppConfigVolume default = %{public}d, max = %{public}d, min = %{public}d",
                appConfigVolume_.defaultVolume, appConfigVolume_.maxVolume, appConfigVolume_.minVolume);
            continue;
        }
        AudioVolumeType CurStreamType = VolumeUtils::GetVolumeTypeFromStreamType(streamVolInfo->streamType);
        minVolumeIndexMap_[CurStreamType] = streamVolInfo->minLevel;
        maxVolumeIndexMap_[CurStreamType] = streamVolInfo->maxLevel;
        volumeDataMaintainer_.SetStreamVolume(streamVolInfo->streamType, streamVolInfo->defaultLevel);
        AUDIO_DEBUG_LOG("update streamType %{public}d index = [%{public}d, %{public}d, %{public}d]",
            streamVolInfo->streamType, minVolumeIndexMap_[CurStreamType], maxVolumeIndexMap_[CurStreamType],
            volumeDataMaintainer_.GetStreamVolume(CurStreamType));
    }
    if (isAppConfigVolumeInit) {
        return;
    } else {
        appConfigVolume_.defaultVolume = APP_DEFAULT_VOLUME_LEVEL;
        appConfigVolume_.maxVolume = APP_MAX_VOLUME_LEVEL;
        appConfigVolume_.minVolume = APP_MIN_VOLUME_LEVEL;
        isAppConfigVolumeInit = true;
        AUDIO_DEBUG_LOG("isAppConfigVolumeInit default = %{public}d, max = %{public}d, min = %{public}d",
            appConfigVolume_.defaultVolume, appConfigVolume_.maxVolume, appConfigVolume_.minVolume);
        return;
    }
    if (minVolumeIndexMap_.find(STREAM_MUSIC) != minVolumeIndexMap_.end() &&
        maxVolumeIndexMap_.find(STREAM_MUSIC) != maxVolumeIndexMap_.end()) {
        appConfigVolume_.defaultVolume = maxVolumeIndexMap_[STREAM_MUSIC];
        appConfigVolume_.maxVolume = maxVolumeIndexMap_[STREAM_MUSIC];
        appConfigVolume_.minVolume = minVolumeIndexMap_[STREAM_MUSIC];
    } else {
        appConfigVolume_.defaultVolume = MAX_VOLUME_LEVEL;
        appConfigVolume_.maxVolume = MAX_VOLUME_LEVEL;
        appConfigVolume_.minVolume = MIN_VOLUME_LEVEL;
    }
    isAppConfigVolumeInit = true;
    AUDIO_DEBUG_LOG("next AppConfigVolume default = %{public}d, max = %{public}d, min = %{public}d",
        appConfigVolume_.defaultVolume, appConfigVolume_.maxVolume, appConfigVolume_.minVolume);
}

void AudioAdapterManager::GetVolumePoints(AudioVolumeType streamType, DeviceVolumeType deviceType,
    std::vector<VolumePoint> &volumePoints)
{
    auto streamVolInfo = streamVolumeInfos_.find(streamType);
    if (streamVolInfo == streamVolumeInfos_.end()) {
        AUDIO_DEBUG_LOG("Cannot find stream type %{public}d and try to use STREAM_MUSIC", streamType);
        streamVolInfo = streamVolumeInfos_.find(STREAM_MUSIC);
        CHECK_AND_RETURN_LOG(streamVolInfo != streamVolumeInfos_.end(),
            "Cannot find stream type STREAM_MUSIC");
    }
    auto deviceVolInfo = streamVolInfo->second->deviceVolumeInfos.find(deviceType);
    if (deviceVolInfo == streamVolInfo->second->deviceVolumeInfos.end()) {
        AUDIO_ERR_LOG("Cannot find device type %{public}d", deviceType);
        return;
    }
    volumePoints = deviceVolInfo->second->volumePoints;
}

void AudioAdapterManager::GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos)
{
    streamVolumeInfos = streamVolumeInfos_;
}

void AudioAdapterManager::SetActiveDeviceDescriptor(AudioDeviceDescriptor deviceDescriptor)
{
    AUDIO_PRERELEASE_LOGI("SetActiveDevice deviceType %{public}d", deviceDescriptor.deviceType_);
    SetVolumeForSwitchDevice(deviceDescriptor);
}

AudioDeviceDescriptor AudioAdapterManager::GetActiveDeviceDescriptor()
{
    return currentActiveDevice_;
}

DeviceCategory AudioAdapterManager::GetCurrentOutputDeviceCategory()
{
    return currentActiveDevice_.deviceCategory_;
}

DeviceType AudioAdapterManager::GetActiveDevice()
{
    return currentActiveDevice_.deviceType_;
}

void AudioAdapterManager::SetAbsVolumeScene(bool isAbsVolumeScene)
{
    AUDIO_PRERELEASE_LOGI("SetAbsVolumeScene: %{public}d", isAbsVolumeScene);
    isAbsVolumeScene_ = isAbsVolumeScene;
    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "SetAbsVolumeScene audio adapter null");
    audioServiceAdapter_->SetAbsVolumeStateToEffect(isAbsVolumeScene);
    AudioVolumeManager::GetInstance().SetSharedAbsVolumeScene(isAbsVolumeScene_);
    if (currentActiveDevice_.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        volumeDataMaintainer_.GetVolume(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_MUSIC);
        SetVolumeDb(STREAM_MUSIC);
    } else {
        AUDIO_INFO_LOG("The currentActiveDevice is not A2DP or nearlink device");
    }
    if (currentActiveDevice_.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && IsAbsVolumeScene()
        && !VolumeUtils::IsPCVolumeEnable()) {
        volumeDataMaintainer_.SetStreamVolume(STREAM_VOICE_ASSISTANT, MAX_VOLUME_LEVEL);
        SetVolumeDb(STREAM_VOICE_ASSISTANT);
        AUDIO_INFO_LOG("a2dp ok");
    }
}

bool AudioAdapterManager::IsAbsVolumeScene() const
{
    return isAbsVolumeScene_;
}

void AudioAdapterManager::SetAbsVolumeMute(bool mute)
{
    AUDIO_INFO_LOG("SetAbsVolumeMute: %{public}d", mute);
    isAbsVolumeMute_ = mute;
    if (currentActiveDevice_.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
        currentActiveDevice_.deviceType_ == DEVICE_TYPE_NEARLINK) {
        SetVolumeDb(STREAM_MUSIC);
    } else {
        AUDIO_INFO_LOG("The currentActiveDevice is not A2DP or nearlink device");
    }
}


bool AudioAdapterManager::IsAbsVolumeMute() const
{
    return isAbsVolumeMute_;
}

void AudioAdapterManager::NotifyAccountsChanged(const int &id)
{
    AUDIO_INFO_LOG("start reload the kv data, current id:%{public}d", id);
    LoadVolumeMap();
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType : defaultVolumeTypeList_) {
            CheckAndDealMuteStatus(deviceType, streamType);
        }
    }
}

int32_t AudioAdapterManager::DoRestoreData()
{
    isLoaded_ = false;
    isNeedConvertSafeTime_ = true; // reset convert safe volume status
    volumeDataMaintainer_.SaveMuteTransferStatus(true); // reset mute convert status
    InitKVStore();
    return SUCCESS;
}

int32_t AudioAdapterManager::GetSafeVolumeLevel() const
{
    return safeVolume_;
}

int32_t AudioAdapterManager::GetSafeVolumeTimeout() const
{
    if (safeVolumeTimeout_ <= 0) {
        AUDIO_INFO_LOG("safeVolumeTimeout is invalid, return default value:%{public}d", DEFAULT_SAFE_VOLUME_TIMEOUT);
        return DEFAULT_SAFE_VOLUME_TIMEOUT;
    }
    return safeVolumeTimeout_;
}

void AudioAdapterManager::SetFirstBoot(bool isFirst)
{
    int32_t ret = 0;
    if (isFirst) {
        ret = SetParameter("persist.multimedia.audio.firstboot", std::to_string(1).c_str());
    } else {
        ret = SetParameter("persist.multimedia.audio.firstboot", std::to_string(0).c_str());
    }
    if (ret == 0) {
        AUDIO_INFO_LOG("Set first boot %{public}d success", isFirst);
    } else {
        AUDIO_ERR_LOG("Set first boot %{public}d failed, result %{public}d", isFirst, ret);
    }
}

void AudioAdapterManager::SafeVolumeDump(std::string &dumpString)
{
    dumpString += "SafeVolume info:\n";
    for (auto &streamType : defaultVolumeTypeList_) {
        AppendFormat(dumpString, "  - samplingAudioStreamTypeate: %d", streamType);
        AppendFormat(dumpString, "   volumeLevel: %d\n", volumeDataMaintainer_.GetStreamVolume(streamType));
        AppendFormat(dumpString, "  - AudioStreamType: %d", streamType);
        AppendFormat(dumpString, "   streamMuteStatus: %d\n", volumeDataMaintainer_.GetStreamMute(streamType));
    }
    if (isSafeBoot_) {
        safeStatusBt_ = GetCurrentDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP);
        safeStatus_ = GetCurrentDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET);
        safeActiveBtTime_ = GetCurentDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP);
        safeActiveTime_ = GetCurentDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET);
        isSafeBoot_ = false;
    }
    std::string statusBt = (safeStatusBt_ == SAFE_ACTIVE) ? "SAFE_ACTIVE" : "SAFE_INACTIVE";
    std::string status = (safeStatus_ == SAFE_ACTIVE) ? "SAFE_ACTIVE" : "SAFE_INACTIVE";
    AppendFormat(dumpString, "  - ringerMode: %d\n", ringerMode_);
    AppendFormat(dumpString, "  - SafeVolume: %d\n", safeVolume_);
    AppendFormat(dumpString, "  - BtSafeStatus: %s\n", statusBt.c_str());
    AppendFormat(dumpString, "  - SafeStatus: %s\n", status.c_str());
    AppendFormat(dumpString, "  - ActiveBtSafeTime: %lld\n", safeActiveBtTime_);
    AppendFormat(dumpString, "  - ActiveSafeTime: %lld\n", safeActiveTime_);
}

void AudioAdapterManager::SetVgsVolumeSupported(bool isVgsSupported)
{
    AUDIO_INFO_LOG("Set Vgs Supported: %{public}d", isVgsSupported);
    isVgsVolumeSupported_ = isVgsSupported;
    AudioVolume::GetInstance()->SetVgsVolumeSupported(isVgsSupported);
}

bool AudioAdapterManager::IsVgsVolumeSupported() const
{
    if (currentActiveDevice_.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO) {
        return false;
    }
    return isVgsVolumeSupported_;
}

std::vector<AdjustStreamVolumeInfo> AudioAdapterManager::GetStreamVolumeInfo(AdjustStreamVolume volumeType)
{
    return AudioVolume::GetInstance()->GetStreamVolumeInfo(volumeType);
}

void AudioAdapterManager::UpdateVolumeForLowLatency()
{
    Trace trace("AudioAdapterManager::UpdateVolumeForLowLatency");
    // update volumes for low latency streams when loading volumes from the database.
    Volume vol = {false, 1.0f, 0};
    DeviceType curOutputDeviceType = currentActiveDevice_.deviceType_;
    for (auto iter = VOLUME_TYPE_LIST.begin(); iter != VOLUME_TYPE_LIST.end(); iter++) {
        vol.isMute = GetStreamMute(*iter);
        vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevelNoMuteState(*iter));
        vol.volumeFloat = GetSystemVolumeInDb(*iter, (vol.isMute ? 0 : vol.volumeInt), curOutputDeviceType);
        AudioVolumeManager::GetInstance().SetSharedVolume(*iter, curOutputDeviceType, vol);
    }
    AudioVolumeManager::GetInstance().SetSharedAbsVolumeScene(IsAbsVolumeScene());
}

void AudioAdapterManager::RegisterDoNotDisturbStatus()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncDoNotDisturb = [&](const std::string &key) {
        int32_t isDoNotDisturb = 0;
        int32_t ret = settingProvider.GetIntValue(DO_NOT_DISTURB_STATUS, isDoNotDisturb, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get doNotDisturbStatus failed");
        AUDIO_INFO_LOG("doNotDisturbStatus = %{public}s", isDoNotDisturb != 0 ? "true" : "false");
        auto audioVolume = AudioVolume::GetInstance();
        CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null, set DoNotDisturbStatus failed");
        audioVolume->SetDoNotDisturbStatus(isDoNotDisturb != 0);
    };
    sptr observer = settingProvider.CreateObserver(DO_NOT_DISTURB_STATUS, updateFuncDoNotDisturb);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver doNotDisturbStatus failed! Err: %{public}d", ret);
    } else {
        AUDIO_INFO_LOG("Register doNotDisturbStatus successfully");
    }
}

void AudioAdapterManager::RegisterDoNotDisturbStatusWhiteList()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncDoNotDisturbWhiteList = [&](const std::string &key) {
        std::vector<std::map<std::string, std::string>> doNotDisturbWhiteList;
        int32_t ret = settingProvider.GetMapValue(DO_NOT_DISTURB_STATUS_WHITE_LIST,
            doNotDisturbWhiteList, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get doNotDisturbStatus WhiteList failed");
        AUDIO_INFO_LOG("doNotDisturbStatusWhiteList changed");
        auto audioVolume = AudioVolume::GetInstance();
        CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null, \
            set doNotDisturbStatusWhiteList failed");
        audioVolume->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbWhiteList);
    };
    sptr observer = settingProvider.CreateObserver(DO_NOT_DISTURB_STATUS_WHITE_LIST,
        updateFuncDoNotDisturbWhiteList);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver doNotDisturbStatus WhiteList failed! Err: %{public}d", ret);
    } else {
        AUDIO_INFO_LOG("Register doNotDisturbStatus WhiteList successfully");
    }
}

void AudioAdapterManager::HandleHearingAidVolume(AudioStreamType streamType)
{
    if (currentActiveDevice_.deviceType_ == DEVICE_TYPE_HEARING_AID) {
        if (streamType == STREAM_MUSIC || streamType == STREAM_VOICE_CALL ||
            streamType == STREAM_VOICE_ASSISTANT) {
            int32_t defaultVolume = static_cast<int32_t>(std::ceil(GetMaxVolumeLevel(streamType) * 0.8));
            AUDIO_INFO_LOG("first time switch hearingAid, use default volume");
            SetSystemVolumeLevel(streamType, defaultVolume);
        }
    }
}

int32_t AudioAdapterManager::SetSystemVolumeDegree(AudioStreamType streamType, int32_t volumeDegree)
{
    Trace trace("streamType:" + std::to_string(streamType) + ", volumeDegree:" + std::to_string(volumeDegree));
    AUDIO_INFO_LOG("streamType: %{public}d, deviceType: %{public}d, volumeDegree:%{public}d",
        streamType, currentActiveDevice_.deviceType_, volumeDegree);
    if (GetSystemVolumeDegree(streamType) == volumeDegree &&
        currentActiveDevice_.deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO &&
        currentActiveDevice_.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP && !VolumeUtils::IsPCVolumeEnable()) {
        AUDIO_INFO_LOG("The volume is the same as before.");
        return SUCCESS;
    }

    if (volumeDegree == 0 && !VolumeUtils::IsPCVolumeEnable() &&
        (streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("this type can not set mute");
        return SUCCESS;
    }

    int32_t minRet = GetMinVolumeDegree(streamType);
    CHECK_AND_RETURN_RET_LOG(volumeDegree >= minRet && volumeDegree <= MAX_VOLUME_DEGREE, ERR_OPERATION_FAILED,
        "volume not in scope, mimRet:%{public}d", minRet);

    volumeDataMaintainer_.SetVolumeDegree(streamType, volumeDegree);

    if (handler_ != nullptr) {
        if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_.deviceType_ != DEVICE_TYPE_REMOTE_CAST) {
            AUDIO_INFO_LOG("DualToneStreamType. Save volume for speaker.");
            handler_->SendSaveVolumeDegree(DEVICE_TYPE_SPEAKER, streamType, volumeDegree, "LocalDevice");
        } else {
            handler_->SendSaveVolumeDegree(currentActiveDevice_.deviceType_, streamType, volumeDegree,
                currentActiveDevice_.networkId_);
        }
    }

    float volumeDb = CalculateVolumeDb(volumeDegree, MAX_VOLUME_DEGREE);
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        volumeDb = 1.0f;
    }
    AUDIO_INFO_LOG("streamType:%{public}d volumeDb:%{public}f volumeDegree:%{public}d devicetype:%{public}d",
        streamType, volumeDb, volumeDegree, currentActiveDevice_.deviceType_);
    SetAudioVolume(streamType, volumeDb);
    return SUCCESS;
}

int32_t AudioAdapterManager::GetSystemVolumeDegree(AudioStreamType streamType)
{
    if (GetStreamMuteInternal(streamType)) {
        return MIN_VOLUME_LEVEL;
    }

    return volumeDataMaintainer_.GetVolumeDegree(streamType);
}

int32_t AudioAdapterManager::GetMinVolumeDegree(AudioVolumeType volumeType)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        MIN_VOLUME_LEVEL, "Invalid stream type");
    return MIN_VOLUME_LEVEL;
}

// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
