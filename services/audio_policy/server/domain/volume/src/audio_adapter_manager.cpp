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
#include "audio_mute_factor_manager.h"
#include "audio_server_proxy.h"
#include "audio_scene_manager.h"
#include "audio_spatialization_service.h"
#include "audio_manager_listener_stub_impl.h"
#include "audio_format_utils.h"
#include "audio_bundle_manager.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B83
using namespace std;

namespace OHOS {
namespace AudioStandard {
static const char* DO_NOT_DISTURB_STATUS = "focus_mode_enable";
static const char* DO_NOT_DISTURB_STATUS_WHITE_LIST = "intelligent_scene_notification_white_list";
constexpr const char *APP_INDIVIDUAL_VOLUME_ENABLE = "app_individual_volume_enable";
mutex g_deviceVolumeBehaviorListenerMutex;

static const std::vector<DeviceType> VOLUME_GROUP_TYPE_LIST = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_WIRED_HEADSET
};

static const std::vector<std::string> SYSTEM_SOUND_KEY_LIST = {
    // all keys for system sound uri
    "ringtone_for_sim_card_0",
    "ringtone_for_sim_card_1",
    "system_tone_for_sim_card_0",
    "system_tone_for_sim_card_1",
    "system_tone_for_notification"
};

namespace {
const std::unordered_map<DeviceType, std::vector<std::string>> DEVICE_CLASS_MAP = {
    {DEVICE_TYPE_SPEAKER, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS, REMOTE_CLASS, MMAP_CLASS, MMAP_VOIP_CLASS,
        DIRECT_VOIP_CLASS}},
    {DEVICE_TYPE_USB_HEADSET, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS,
        MMAP_VOIP_CLASS, DIRECT_VOIP_CLASS, DIRECT_CLASS}},
    {DEVICE_TYPE_BLUETOOTH_A2DP, {A2DP_CLASS, PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS, DP_CLASS, DP_MCH_CLASS,
        MMAP_CLASS}},
    {DEVICE_TYPE_BLUETOOTH_SCO, {PRIMARY_CLASS, MCH_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS, MMAP_VOIP_CLASS}},
    {DEVICE_TYPE_NEARLINK, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS,
        MMAP_VOIP_CLASS}},
    {DEVICE_TYPE_EARPIECE, {PRIMARY_CLASS, MCH_CLASS, MMAP_CLASS, MMAP_VOIP_CLASS, MMAP_VOIP_CLASS}},
    {DEVICE_TYPE_WIRED_HEADSET, {PRIMARY_CLASS, MCH_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS, MMAP_VOIP_CLASS}},
    {DEVICE_TYPE_WIRED_HEADPHONES, {PRIMARY_CLASS, MCH_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS, MMAP_VOIP_CLASS}},
    {DEVICE_TYPE_USB_ARM_HEADSET, {PRIMARY_CLASS, USB_CLASS, DP_CLASS, DP_MCH_CLASS, MMAP_CLASS}},
    {DEVICE_TYPE_REMOTE_CAST, {REMOTE_CAST_INNER_CAPTURER_SINK_NAME}},
    {DEVICE_TYPE_DP, {DP_CLASS, DP_MCH_CLASS}},
    {DEVICE_TYPE_FILE_SINK, {FILE_CLASS}},
    {DEVICE_TYPE_FILE_SOURCE, {FILE_CLASS}},
    {DEVICE_TYPE_HDMI, {PRIMARY_CLASS, DP_CLASS, DP_MCH_CLASS}},
    {DEVICE_TYPE_ACCESSORY, {ACCESSORY_CLASS}},
    {DEVICE_TYPE_HEARING_AID, {HEARING_AID_CLASS}},
    {DEVICE_TYPE_LINE_DIGITAL, {PRIMARY_CLASS, DP_CLASS, DP_MCH_CLASS}},
    {DEVICE_TYPE_REMOTE_DAUDIO, {DP_CLASS, DP_MCH_CLASS}}
};
} // namespace

// LCOV_EXCL_START
void AudioAdapterManager::RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime)
{
    AudioCoreService::GetCoreService()->RecordBootStateTime(state, curTime);
}

void AudioAdapterManager::RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime)
{
    AudioCoreService::GetCoreService()->RecordBootStateTime(state, uid, curTime);
}

bool AudioAdapterManager::Init()
{
    char testMode[10] = {0}; // 10 for system parameter usage
    auto ret = GetParameter("debug.audio_service.testmodeon", "0", testMode, sizeof(testMode));
    if (ret == 1 && testMode[0] == '1') {
        AUDIO_DEBUG_LOG("testMode on");
        testModeOn_ = true;
    }

    std::unique_ptr<AudioVolumeParser> audiovolumeParser = make_unique<AudioVolumeParser>();
    CHECK_AND_RETURN_RET_LOG(audiovolumeParser, false, "audiovolumeParser is null");
    auto lret = audiovolumeParser->LoadConfig(streamVolumeInfos_);
    if (streamVolumeInfos_.find(STREAM_APP) == streamVolumeInfos_.end()) {
        AUDIO_INFO_LOG("APP volume config not found in product config, load from default");
        audiovolumeParser->LoadDefaultAppVolumeConfig(streamVolumeInfos_);
    }
    RecordBootStateTime(AudioPolicyServerBootState::LOAD_AUDIO_VOLUME_CONFIG, ClockTime::GetCurMilli());
    lowerVolumeInfos_ = audiovolumeParser->GetLowerVolumeInfoCfg();
    AudioVolumeUtils::GetInstance().Init();
    defaultVolumeTypeList_ = (VolumeUtils::IsPCVolumeEnable()) ? PC_VOLUME_TYPE_LIST : BASE_VOLUME_TYPE_LIST;
    volumeDataMaintainer_.SetVolumeList(defaultVolumeTypeList_);
    if (!lret) {
        AUDIO_INFO_LOG("Audio Volume Config Load Configuration successfully");
        useNonlinearAlgo_ = true;
        UpdateVolumeMapIndex();
    }

    // init volume before kvstore start by local prop for bootanimation
    InitBootAnimationVolume();
    AudioVolume::GetInstance()->SetDefaultAppVolume(appConfigVolume_.defaultVolume);
    std::string defaultSafeVolume = std::to_string(GetMaxVolumeLevel(STREAM_MUSIC, DEVICE_TYPE_SPEAKER));
    AUDIO_INFO_LOG("defaultSafeVolume %{public}s", defaultSafeVolume.c_str());
    char currentSafeVolumeValue[4] = {0};
    ret = GetParameter("const.audio.safe_media_volume", defaultSafeVolume.c_str(),
        currentSafeVolumeValue, sizeof(currentSafeVolumeValue));
    if (ret > 0) {
        safeVolume_ = atoi(currentSafeVolumeValue);
        AUDIO_INFO_LOG("Get currentSafeVolumeValue success %{public}d", safeVolume_);
    } else {
        safeVolume_ = GetMaxVolumeLevel(STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
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

    char mdmMuteStatus[6] = {0};
    ret = GetParameter("persist.edm.unmute_device_disallowed", "false", mdmMuteStatus, sizeof(mdmMuteStatus));
    if (ret > 0) {
        bool isMdmMute = (strcmp(mdmMuteStatus, "true") == 0);
        AudioMuteFactorManager::GetInstance().SetMdmMuteStatus(isMdmMute);
        AUDIO_INFO_LOG("Get mdmMuteStatus success %{public}d", isMdmMute);
    } else {
        AUDIO_ERR_LOG("Get mdmMuteStatus failed %{public}d", ret);
    }
    RegisterMdmMuteSwitchCallback();

    handler_ = std::make_shared<AudioAdapterManagerHandler>();
    return true;
}

void AudioAdapterManager::InitBootAnimationVolume()
{
    char currentVolumeValue[3] = {0};
    AudioVolumeType typeForBootAnimation = VolumeUtils::IsPCVolumeEnable() ? STREAM_SYSTEM : STREAM_NOTIFICATION;
    int32_t bootAnimationVolume = GetStreamVolume(typeForBootAnimation);
    AUDIO_DEBUG_LOG("Init: Type[%{public}d],volume[%{public}d]", typeForBootAnimation, bootAnimationVolume);
    std::string defaultVolume = std::to_string(bootAnimationVolume);
    auto ret = GetParameter("persist.multimedia.audio.ringtonevolume", defaultVolume.c_str(),
        currentVolumeValue, sizeof(currentVolumeValue));
    if (ret > 0) {
        auto desc = audioActiveDevice_.GetDeviceForVolume(typeForBootAnimation);
        SaveVolumeData(desc, typeForBootAnimation, atoi(currentVolumeValue), false, true);
        AUDIO_INFO_LOG("Init: Get Type[%{public}d] volume to map volume [%{public}d]",
            typeForBootAnimation, GetStreamVolumeInternal(desc, typeForBootAnimation));
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
    AudioSpatializationService::GetAudioSpatializationService().InitSpatializationState();
    RecordBootStateTime(AudioPolicyServerBootState::INIT_VOLUME_DATABASE, ClockTime::GetCurMilli());
}

void AudioAdapterManager::HandleKvData(bool isFirstBoot)
{
    AUDIO_INFO_LOG("firstBoot %{public}d", isFirstBoot);
    InitVolumeMap(isFirstBoot);
    InitRingerMode(isFirstBoot);
    InitMuteStatusMap(isFirstBoot);
    InitSafeStatus(isFirstBoot);
    InitSafeTime(isFirstBoot);
    InitLegacyHighVolumeCheckTime(isFirstBoot);
    InitSystemAppVolume(isFirstBoot);

    if (isNeedCopySystemUrlData_) {
        CloneSystemSoundUrl();
    }

    LoadVolumeValueMapFromDb();
    UpdateVolumeForAllPipes();

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
}

void AudioAdapterManager::LoadVolumeValueMapFromDb()
{
    const std::list<DeviceType> DEVICE_TYPE_LIST_FOR_EACH_GROUP = {
        // one device for each DeviceGroupType.
        DEVICE_TYPE_EARPIECE,
        DEVICE_TYPE_SPEAKER,
        DEVICE_TYPE_DP,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_HEARING_AID,
        DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_REMOTE_CAST,
        DEVICE_TYPE_LINE_DIGITAL
    };

    for (auto deviceType : DEVICE_TYPE_LIST_FOR_EACH_GROUP) {
        auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
        volumeDataMaintainer_.SaveLoadFlagToMap(desc, VolumeKeyType::LEVEL, false);
        volumeDataMaintainer_.SaveLoadFlagToMap(desc, VolumeKeyType::MUTE, false);
        UpdateVolumeWhenDeviceConnect(desc);
#ifdef FEATURE_AUDIO_ZONE
        if (AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(DEFAULT_ZONEID, desc->deviceType_)) {
            TriggerAudioZoneVolumeSync(DEFAULT_ZONEID, desc);
        }
#endif
    }
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
int32_t AudioAdapterManager::GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.maxVolume;
    }
    std::shared_ptr<AudioDeviceDescriptor> desc;
    if (deviceType == DeviceType::DEVICE_TYPE_NONE) {
        desc = audioActiveDevice_.GetDeviceForVolume(volumeType);
    } else {
        desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    }
    return AudioVolumeUtils::GetInstance().GetMaxVolumeLevel(desc, volumeType, zoneId);
}

int32_t AudioAdapterManager::GetMaxVolumeLevel(AudioVolumeType volumeType, std::shared_ptr<AudioDeviceDescriptor> desc,
    int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.maxVolume;
    }
    return AudioVolumeUtils::GetInstance().GetMaxVolumeLevel(desc, volumeType, zoneId);
}

int32_t AudioAdapterManager::GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.minVolume;
    }
    std::shared_ptr<AudioDeviceDescriptor> desc;
    if (deviceType == DeviceType::DEVICE_TYPE_NONE) {
        desc = audioActiveDevice_.GetDeviceForVolume(volumeType);
    } else {
        desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    }
    return AudioVolumeUtils::GetInstance().GetMinVolumeLevel(desc, volumeType, zoneId);
}


int32_t AudioAdapterManager::GetMinVolumeLevel(AudioVolumeType volumeType, std::shared_ptr<AudioDeviceDescriptor> desc,
    int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    if (volumeType == STREAM_APP) {
        return appConfigVolume_.minVolume;
    }
    return AudioVolumeUtils::GetInstance().GetMinVolumeLevel(desc, volumeType, zoneId);
}

void AudioAdapterManager::SaveNotificationVolumeToLocal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioVolumeType volumeType, int32_t volumeLevel)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    AudioVolumeType audioVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(volumeType);
    // Only use the volume of speaker as the Boot Animation Volume.
    // use STREAM_NOTIFICATION on phone and use STREAM_SYSTEM on PC.
    bool isRightVolumeType = VolumeUtils::IsPCVolumeEnable() ?
        (audioVolumeMap == STREAM_SYSTEM) : (audioVolumeMap == STREAM_NOTIFICATION);
    if (isRightVolumeType && device->deviceType_ == DEVICE_TYPE_SPEAKER) {
        int32_t volumeLevel =
            GetStreamVolumeInternal(device, audioVolumeMap) * (GetStreamMuteInternal(device, audioVolumeMap) ? 0 : 1);
        // To keep the startup volume consistent between the new and old versions, still use "ringtonevolume".
        int32_t ret = SetParameter("persist.multimedia.audio.ringtonevolume", std::to_string(volumeLevel).c_str());
        if (ret == 0) {
            AUDIO_INFO_LOG("Save notification volume for boot success %{public}d", volumeLevel);
        } else {
            AUDIO_ERR_LOG("Save notification volume for boot failed, result %{public}d", ret);
        }
    }
}

void AudioAdapterManager::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    volumeDataMaintainer_.SetDataShareReady(std::atomic_load(&isDataShareReady));
    isDataShareReady_ = isDataShareReady.load();

    CHECK_AND_RETURN_LOG(isDataShareReady, "isDataShareReady is false");
    InitKVStoreInternal();
}

void AudioAdapterManager::UpdateSafeVolumeByS4()
{
    AUDIO_INFO_LOG("Update Safevolume by S4 reboot,reset wired and bt once");
    isWiredBoot_ = true;
    isBtBoot_ = true;
    UpdateSafeVolume();
    UpdateTypeVolumeForAllPipes(STREAM_MUSIC);
}

void AudioAdapterManager::SendLoudVolumeModeToDsp(LoudVolumeHoldType funcHoldType, bool state)
{
    std::string key = "LOUD_VOLUME_MODE";
    std::string value = "super_loudness_mode=voice_off";
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    CHECK_AND_RETURN_LOG(audioServerProxy_ != nullptr, "audioServerProxy_ null");

    if (LOUD_VOLUME_MODE_VOICE == funcHoldType) {
        value = state ? "super_loudness_mode=voice_on" : "super_loudness_mode=voice_off";
    } else if (LOUD_VOLUME_MODE_MUSIC == funcHoldType) {
        value = state ? "super_loudness_mode=music_on" : "super_loudness_mode=music_off";
    } else {
        AUDIO_ERR_LOG("funcHoldType error : %{public}d", funcHoldType);
        IPCSkeleton::SetCallingIdentity(identity);
        return;
    }

    audioServerProxy_->SetAudioParameter(key, value);
    IPCSkeleton::SetCallingIdentity(identity);
    AUDIO_INFO_LOG("update LoudVolume [%{public}s]", value.c_str());
    return;
}

int32_t AudioAdapterManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevel: appUid: %{public}d, volumeLevel:%{public}d",
        appUid, volumeLevel);
    volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    return SetAppVolumeDb(appUid);
}

int32_t AudioAdapterManager::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevel: appUid: %{public}d, muted:%{public}d",
        appUid, muted);
    volumeDataMaintainer_.SetAppVolumeMuted(appUid, muted);
    return SetAppVolumeMutedDB(appUid, muted);
}

int32_t AudioAdapterManager::GetSystemAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    if (volumeDataMaintainer_.IsSetSystemAppVolume(appUid)) {
        volumeLevel = volumeDataMaintainer_.IsSystemAppVolumeMuted(appUid) ?
            0 : volumeDataMaintainer_.GetSystemAppVolume(appUid);
    } else {
        volumeLevel = APP_DEFAULT_VOLUME_LEVEL;
    }
    AUDIO_INFO_LOG("uid:%{public}d, volumeLevel:%{public}d", appUid, volumeLevel);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetSystemAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("appUid: %{public}d, volumeLevel: %{public}d", appUid, volumeLevel);
    volumeDataMaintainer_.SetSystemAppVolume(appUid, volumeLevel);
    bool muted = volumeLevel > 0 ? false : true;
    volumeDataMaintainer_.SetSystemAppVolumeMuted(appUid, muted);
    return SetSystemAppVolumeDb(appUid);
}

int32_t AudioAdapterManager::SetSystemAppVolumeDb(int32_t appUid)
{
    int32_t volumeLevel = volumeDataMaintainer_.IsSystemAppVolumeMuted(appUid) ?
        0 : volumeDataMaintainer_.GetSystemAppVolume(appUid);
    auto desc = audioActiveDevice_.GetDeviceForVolume(appUid);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "desc is null");
    float volumeDb = CalculateVolumeDbNonlinear(STREAM_APP, desc->deviceType_, volumeLevel);
    SetSystemAppAudioVolume(appUid, volumeDb);
    SendOffloadVolume(desc->networkId_);
    return SUCCESS;
}

void AudioAdapterManager::SetSystemAppAudioVolume(int32_t appUid, float volumeDb)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");
    bool isMuted = volumeDataMaintainer_.IsSystemAppVolumeMuted(appUid);
    int32_t systemAppVolumeLevel = volumeDataMaintainer_.GetSystemAppVolume(appUid) * (isMuted ? 0 : 1);
    SystemAppVolume systemAppVolume(appUid, volumeDb, systemAppVolumeLevel, isMuted);
    audioVolume->SetSystemAppVolume(systemAppVolume);
}

int32_t AudioAdapterManager::SetSystemAppVolumeMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("appUid: %{public}d, muted: %{public}d", appUid, muted);
    volumeDataMaintainer_.SetSystemAppVolumeMuted(appUid, muted);
    return SetSystemAppVolumeDb(appUid);
}

int32_t AudioAdapterManager::IsSystemAppVolumeMuted(int32_t appUid, bool &isMute)
{
    isMute = volumeDataMaintainer_.IsSystemAppVolumeMuted(appUid);
    AUDIO_INFO_LOG("appUid: %{public}d, isMute: %{public}d", appUid, isMute);
    return SUCCESS;
}

void AudioAdapterManager::SaveSystemAppVolumeLevelToDb()
{
    volumeDataMaintainer_.SaveSystemAppVolumeLevelToDb();
}

void AudioAdapterManager::SaveSystemAppVolumeMuteStatusToDb(bool isAsync)
{
    auto task = [this] {
        volumeDataMaintainer_.SaveSystemAppVolumeMuteStatusToDb();
    };
    isAsync ? std::thread(task).detach() : task();
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
    AUDIO_INFO_LOG("set adjust volume for zone %{public}d", zoneId);
    volumeAdjustZoneId_ = zoneId;
    return SUCCESS;
}

int32_t AudioAdapterManager::GetVolumeAdjustZoneId()
{
    return volumeAdjustZoneId_;
}

int32_t AudioAdapterManager::SetZoneMute(int32_t zoneId, AudioStreamType streamType, bool mute,
    StreamUsage streamUsage, const DeviceType &deviceType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, streamUsage, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    return SetStreamMuteInternal(devices[0], streamType, mute, streamUsage);
}

bool AudioAdapterManager::GetZoneMute(int32_t zoneId, AudioStreamType streamType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, false,
        "zone device error");
    return GetStreamMuteInternal(devices[0], streamType);
}

int32_t AudioAdapterManager::GetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    if (GetStreamMuteInternal(devices[0], streamType, zoneId)) {
        return MIN_VOLUME_LEVEL;
    }
    return volumeDataMaintainer_.LoadVolumeFromMap(devices[0], streamType, zoneId);
}

int32_t AudioAdapterManager::GetZoneVolumeDegree(int32_t zoneId, AudioStreamType streamType)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, ERR_OPERATION_FAILED,
        "zone device error");
    if (GetStreamMuteInternal(devices[0], streamType)) {
        return 0;
    }
    return volumeDataMaintainer_.LoadVolumeDegreeFromMap(devices[0], streamType);
}

int32_t AudioAdapterManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("IsAppVolumeMute: appUid: %{public}d, owned:%{public}d",
        appUid, owned);
    if (owned) {
        volumeDataMaintainer_.GetAppMuteOwned(appUid, isMute);
    } else {
        volumeDataMaintainer_.GetAppMute(appUid, isMute);
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::SetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType, const VolumeScale &volume,
    std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().FetchOutputDevices(zoneId, STREAM_USAGE_UNKNOWN, 0, ROUTER_TYPE_DEFAULT);
    CHECK_AND_RETURN_RET_LOG(devices.size() >= 1, ERR_OPERATION_FAILED, "zone device error");
    volDeviceDesc = devices[0];
    int32_t mimRet = GetMinVolumeLevel(streamType, devices[0], zoneId);
    int32_t maxRet = GetMaxVolumeLevel(streamType, devices[0], zoneId);
    int32_t volumeLevel = volume.VolumeLevel();
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);
    AUDIO_INFO_LOG("zoneId: %{public}d, streamType: %{public}d, volumeLevel: %{public}d, volumeDegree: %{public}d",
        zoneId, streamType, volumeLevel, volume.VolumeDegree());

    VolumeScale buffedVolume = volume;
    volumeDataMaintainer_.SaveVolumeToMap(devices[0], streamType, buffedVolume, zoneId);
    SetRemoteVolumeForPassThroughDevice(devices[0], volumeLevel);
    return SetVolumeDbForDeviceInPipe(devices[0], streamType);
}

int32_t AudioAdapterManager::SetSystemVolumeLevel(AudioStreamType streamType, const VolumeScale &volume,
    std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc, int32_t zoneId)
{
    int32_t volumeLevel = volume.VolumeLevel();
    Trace trace("KeyAction AudioAdapterManager::SetSystemVolumeLevel streamType:"
        + std::to_string(streamType) + ", volumeLevel:" + std::to_string(volumeLevel));
    AUDIO_INFO_LOG("In");
    std::lock_guard<std::mutex> lock(deviceConnectMutex_);
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    volDeviceDesc = desc;
    AUDIO_INFO_LOG("streamType: %{public}d, device: %{public}s, volumeLevel:%{public}d",
        streamType, desc->GetName().c_str(), volumeLevel);
    if (desc->volumeBehavior_.isVolumeControlDisabled) {
        AUDIO_WARNING_LOG("desc->volumeBehavior_.isVolumeControlDisabled is true!");
        return ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED;
    }

    if (GetSystemVolumeLevel(streamType) == volumeLevel &&
        GetSystemVolumeDegree(streamType) == volume.VolumeDegree() &&
        desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO &&
        desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP && !VolumeUtils::IsPCVolumeEnable()) {
        AUDIO_INFO_LOG("The volume is the same as before.");
        return SUCCESS;
    }
    if (volumeLevel == 0 && !VolumeUtils::IsPCVolumeEnable() &&
        (streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("this type can not set mute");
        return SUCCESS;
    }
    int32_t mimRet = GetMinVolumeLevel(streamType, desc);
    int32_t maxRet = GetMaxVolumeLevel(streamType, desc);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);
    VolumeScale buffedVolume = volume;
    volumeDataMaintainer_.SaveVolumeToMap(desc, streamType, buffedVolume);

    // Save the volume to database
    SaveVolumeToDbAsync(desc, streamType, buffedVolume);
    SetRemoteVolumeForPassThroughDevice(desc, volumeLevel);
    if (AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(zoneId, desc->deviceType_)) {
        AUDIO_INFO_LOG("Set system volume for proxy device, zoneId: %{public}d, device: %{public}s",
            zoneId, desc->GetName().c_str());
        AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId, desc->deviceType_, streamType, buffedVolume);
        return SUCCESS;
    }

    return SetVolumeDbForDeviceInPipe(desc, streamType);
}

int32_t AudioAdapterManager::SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("%{public}s: streamType: %{public}d, volumeLevel: %{public}d, "
        "deviceType: %{public}d",  __func__, streamType, volumeLevel, deviceType);
    int32_t mimRet = GetMinVolumeLevel(streamType, deviceType);
    int32_t maxRet = GetMaxVolumeLevel(streamType, deviceType);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope,mimRet:%{public}d maxRet:%{public}d", mimRet, maxRet);
    SaveSpecifiedDeviceVolumeInner(streamType, volumeLevel, deviceType);
    return SUCCESS;
}

void AudioAdapterManager::SaveSpecifiedDeviceVolumeInner(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    std::thread([this, streamType, volumeLevel, deviceType]() {
        AUDIO_INFO_LOG("SaveSpecifiedDeviceVolumeInner: streamType: %{public}d, volumeLevel: %{public}d, "
            "deviceType: %{public}d", streamType, volumeLevel, deviceType);
        auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
        volumeDataMaintainer_.SaveVolumeToDb(desc, streamType, volumeLevel);
        auto descs = audioConnectedDevice_.GetCopy();
        for (auto &temp : descs) {
            CHECK_AND_CONTINUE(temp != nullptr);
            CHECK_AND_CONTINUE(temp->deviceType_ == deviceType);
            volumeDataMaintainer_.InitDeviceVolumeMap(temp);
            UpdateSafeVolumeInner(temp);
        }
    }).detach();
}

int32_t AudioAdapterManager::GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    return volumeDataMaintainer_.LoadVolumeFromDb(desc, streamType);
}

void AudioAdapterManager::HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel,
    std::string networkId)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    SaveVolumeToDbAsync(desc, streamType, volumeLevel);
}

void AudioAdapterManager::HandleStreamMuteStatus(AudioStreamType streamType, bool mute,
    const DeviceType &deviceType, std::string networkId)
{
        std::shared_ptr<AudioDeviceDescriptor> desc;
    if (deviceType == DeviceType::DEVICE_TYPE_NONE) {
        desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    } else {
        desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType, networkId);
    }
    SaveMuteToDbAsync(desc, streamType, mute);
}

void AudioAdapterManager::HandleRingerMode(AudioRingerMode ringerMode)
{
    auto desc = audioActiveDevice_.GetDeviceForVolume(STREAM_NOTIFICATION);
    int32_t muteFactor = (ringerMode != RINGER_MODE_NORMAL) ? 0 : 1;
    int32_t volumeLevel = volumeDataMaintainer_.LoadVolumeFromMap(desc, STREAM_NOTIFICATION) * muteFactor;
    // Save volume in local prop for bootanimation
    SaveNotificationVolumeToLocal(desc, STREAM_NOTIFICATION, volumeLevel);

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
    auto desc = audioActiveDevice_.GetDeviceForVolume(appUid);
    volumeDb = CalculateVolumeDbNonlinear(STREAM_APP, desc->deviceType_, volumeLevel);
    SetAppAudioVolume(appUid, volumeDb);
    SendOffloadVolume(desc->networkId_);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetAppVolumeMutedDB(int32_t appUid, bool muted)
{
    std::lock_guard<std::mutex> lock(audioVolumeMutex_);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume != nullptr, ERR_INVALID_PARAM, "audioVolume handle null");
    audioVolume->SetAppVolumeMute(appUid, muted);
    auto desc = audioActiveDevice_.GetDeviceForVolume(appUid);
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volumeDb = 0.0f;
    AUDIO_INFO_LOG("appUid:%{public}d muted:%{public}d device:%{public}s volumeDb:%{public}f isDs:%{public}d",
        appUid, muted, desc->GetName().c_str(), volumeDb, desc->IsDistributedSpeaker());
    SendOffloadVolume(desc->networkId_);
    return SUCCESS;
}

int32_t AudioAdapterManager::SetSystemVolumeToEffect(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetSystemVolumeLevel failed audio adapter null");
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERR_INVALID_PARAM, "device is null");
    // audio volume
    int32_t volumeLevelTemp = GetSystemVolumeForEffect(device->deviceType_, streamType);
    float volumeDbTemp = CalculateVolumeDbNonlinear(streamType, device->deviceType_, volumeLevelTemp);
    AUDIO_INFO_LOG("SetSystemVolumeToEffect streamType: %{public}d, volumeDb: %{public}f, device:%{public}s",
        streamType, volumeDbTemp, device->GetName().c_str());
    return audioServiceAdapter_->SetSystemVolumeToEffect(streamType, volumeDbTemp);
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

void AudioAdapterManager::SendOffloadVolume(const std::string &networkId)
{
    CHECK_AND_RETURN_LOG(audioServerProxy_ != nullptr, "audioServerProxy_ null");
    for (auto &pair : ioHandleSessionMap_) {
        uint32_t ioHandle = pair.first;
        uint32_t sessionId = pair.second;
        struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_MUSIC, ioHandle, &volumes);
        if (volumes.volumeHistory == volume) {
            AUDIO_INFO_LOG("volume not change, no need to send, sessionId[%{public}d] ioHandle[%{public}d]",
                sessionId, ioHandle);
            continue;
        }
        std::string routeDeviceClass = OFFLOAD_IN_REMOTE == offloadSessionID_[ioHandleSessionMap_[ioHandle]] ?
            "remote_offload" : "offload";
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        audioServerProxy_->OffloadSetVolume(volume, routeDeviceClass, networkId);  //todo
        AudioVolume::GetInstance()->Monitor(ioHandleSessionMap_[ioHandle], true);
        IPCSkeleton::SetCallingIdentity(identity);
        AUDIO_INFO_LOG("SendOffloadVolume for ioHandle[%{public}d] sessionId[%{public}d] volume[%{public}f]",
            ioHandle, sessionId, volume);
    }
}

void AudioAdapterManager::SetOffloadSessionId(uint32_t sessionId, OffloadAdapter offloadAdapter, uint32_t ioHandle)
{
    if (sessionId < MIN_STREAMID || sessionId > MAX_STREAMID) {
        AUDIO_PRERELEASE_LOGE("set sessionId[%{public}d] error", sessionId);
    } else {
        AUDIO_PRERELEASE_LOGI("set sessionId[%{public}d]", sessionId);
    }
    offloadSessionID_[sessionId] = offloadAdapter;
    ioHandleSessionMap_[ioHandle] = sessionId;
}

void AudioAdapterManager::ResetOffloadSessionId(uint32_t sessionId)
{
    if (offloadSessionID_.find(sessionId) != offloadSessionID_.end()) {
        AUDIO_PRERELEASE_LOGI("reset offload sessionId[%{public}d]", offloadSessionID_[sessionId]);
        offloadSessionID_.erase(sessionId);
        auto it = std::find_if(ioHandleSessionMap_.begin(), ioHandleSessionMap_.end(),
            [&sessionId](const auto& pair) {
                return pair.second == sessionId;
        });
        if (it != ioHandleSessionMap_.end()) {
            ioHandleSessionMap_.erase(it);
        }
    }
}

int32_t AudioAdapterManager::SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel)
{
    float volumeDb = 1.0f;
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    if (useNonlinearAlgo_) {
        volumeDb = CalculateVolumeDbNonlinear(streamType, desc->deviceType_, volumeLevel);
    } else {
        volumeDb = CalculateVolumeDb(volumeLevel);
    }
    AUDIO_INFO_LOG("SetDoubleRingVolumeDb: %{public}d, %{public}f", volumeLevel, volumeDb);

    AudioIOHandle ioHandle;
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(desc->deviceType_);
    bool result = AudioIOHandleMap::GetInstance().GetModuleIdByKey(sinkName, ioHandle);
    CHECK_AND_RETURN_RET_LOG(result, ERROR, "GetModuleIdByKey failed for %{public}s", sinkName.c_str());

    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioVolume != nullptr, ERROR, "audioVolume handle null");
    PipeVolume pipeVolume(ioHandle, streamType, volumeDb, volumeLevel, false);
    audioVolume->SetPipeVolume(pipeVolume);
    return SUCCESS;
}

int32_t AudioAdapterManager::GetSystemVolumeLevel(AudioStreamType streamType, int32_t zoneId)
{
    std::shared_ptr<AudioDeviceDescriptor> desc;
    if (zoneId > 0) {
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            AudioZoneService::GetInstance().GetAllOutputDevices(zoneId);
        desc = descs.size() > 0 ? descs[0] : nullptr;
    } else {
        desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    }
    if (GetStreamMuteInternal(desc, streamType, zoneId)) {
        return MIN_VOLUME_LEVEL;
    }

    return GetStreamVolumeInternal(desc, streamType, zoneId);
}

int32_t AudioAdapterManager::GetSystemVolumeLevel(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioStreamType streamType, int32_t zoneId)
{
    if (GetStreamMuteInternal(desc, streamType, zoneId)) {
        return MIN_VOLUME_LEVEL;
    }

    return GetStreamVolumeInternal(desc, streamType, zoneId);
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
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    return GetStreamVolumeInternal(desc, streamType);
}

float AudioAdapterManager::GetSystemVolumeDb(AudioStreamType streamType)
{
    int32_t volumeLevel = GetStreamVolume(streamType);
    return CalculateVolumeDb(volumeLevel);
}

void AudioAdapterManager::SetDeviceNoMuteForRinger(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device null");
    AUDIO_INFO_LOG("Set %{public}s no mute for ringer", device->GetName().c_str());
    {
        std::lock_guard<std::mutex> lock(ringerNoMuteDeviceMutex_);
        ringerNoMuteDevice_ = device;
    }
    SetVolumeDbForDeviceInPipe(device, STREAM_RING);
}

void AudioAdapterManager::ClearDeviceNoMuteForRinger()
{
    AUDIO_INFO_LOG("clear no mute device for ringer");
    std::shared_ptr<AudioDeviceDescriptor> tmp;
    {
        std::lock_guard<std::mutex> lock(ringerNoMuteDeviceMutex_);
        tmp = ringerNoMuteDevice_;
        ringerNoMuteDevice_ = nullptr;
    }
    CHECK_AND_RETURN(tmp != nullptr);
    SetVolumeDbForDeviceInPipe(tmp, STREAM_RING);
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
    return ERROR;
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

int32_t AudioAdapterManager::SetSourceOutputStreamMuteByStreamId(int32_t sessionId, bool setMute)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERR_OPERATION_FAILED,
        "SetSourceOutputStreamMute audio adapter null");
    return audioServiceAdapter_->SetSourceOutputStreamMuteByStreamId(sessionId, setMute);
}

bool AudioAdapterManager::GetAppMute(int32_t appUid)
{
    bool isMute = false;
    volumeDataMaintainer_.GetAppMute(appUid, isMute);
    return isMute;
}

// 操作volumeDataMaintainer_
int32_t AudioAdapterManager::GetStreamVolume(AudioStreamType streamType)
{
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    return GetStreamVolumeInternal(desc, streamType);
}

int32_t AudioAdapterManager::GetStreamVolumeInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, int32_t zoneId)
{
    return volumeDataMaintainer_.LoadVolumeFromMap(device, streamType, zoneId);
}

int32_t AudioAdapterManager::GetStreamVolumeDegreeInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, int32_t zoneId)
{
    return volumeDataMaintainer_.LoadVolumeDegreeFromMap(device, streamType, zoneId);
}

bool AudioAdapterManager::GetStreamMute(AudioStreamType streamType, int32_t zoneId)
{
    std::shared_ptr<AudioDeviceDescriptor> desc;
    if (zoneId == 0) {
        desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    } else {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices =
        AudioZoneService::GetInstance().GetAllOutputDevices(zoneId);
        CHECK_AND_RETURN_RET_LOG(devices.size() >= 1 && devices[0] != nullptr, false,
            "zone device error");
        desc = devices[0];
    }
    return GetStreamMuteInternal(desc, streamType, zoneId);
}

bool AudioAdapterManager::GetStreamMuteInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is null");
    return volumeDataMaintainer_.LoadMuteFromMap(device, streamType);
}

bool AudioAdapterManager::GetStreamMuteForVolumeSet(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is null");
    {
        std::lock_guard<std::mutex> lock(ringerNoMuteDeviceMutex_);
        if (Util::IsDualToneStreamType(streamType) && device->IsSameDeviceDescPtr(ringerNoMuteDevice_)) {
            AUDIO_INFO_LOG("get no mute when ringermode no normal on %{public}s", device->GetName().c_str());
            return false;
        }
    }

    return volumeDataMaintainer_.LoadMuteFromMap(device, streamType, zoneId);
}

int32_t AudioAdapterManager::SetStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage,
    const DeviceType &deviceType, std::string networkId, int32_t zoneId)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType, networkId);
    return SetStreamMuteInternal(desc, streamType, mute, streamUsage, zoneId);
}

int32_t AudioAdapterManager::SetStreamMuteInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, bool mute, StreamUsage streamUsage, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERR_INVALID_PARAM, "device is null");
    int32_t isSetStreamMute = IsHandleStreamMute(streamType, mute, streamUsage);
    if (isSetStreamMute == SUCCESS) {
        return SUCCESS;
    }

    volumeDataMaintainer_.SaveMuteToMap(device, streamType, mute);
    SaveMuteToDbAsync(device, streamType, mute);
    if (device->volumeBehavior_.controlMode == PASS_THROUGH_MODE) {
        AudioServerProxy::GetInstance().SetRemoteAudioParameterProxy(device->networkId_, false, mute);
        AUDIO_INFO_LOG("set remote mute: %{public}d", mute);
    }
    if (device->volumeBehavior_.controlMode == HILINK_MODE) {
        AudioServerProxy::GetInstance().SetRemoteAudioParameterProxy(device->networkId_, false, mute);
        AUDIO_INFO_LOG("set remote mute: %{public}d", mute);
    }
    if (AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(zoneId, device->deviceType_)) {
        VolumeScale volume(GetSystemVolumeLevel(device, streamType, zoneId),
            GetSystemVolumeDegree(device, streamType, zoneId));
        AUDIO_INFO_LOG("Set system volume for proxy device, zoneId: %{public}d, device: %{public}s",
            zoneId, device->GetName().c_str());
        AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId, device->deviceType_, streamType,
            volume);
        return SUCCESS;
    }
    return SetVolumeDbForDeviceInPipe(device, streamType, zoneId);
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
    uint32_t ioHandle = OPEN_PORT_FAILURE;
    audioIOHandleMap_.GetModuleIdByKey(sinkName, ioHandle);
    CHECK_AND_RETURN_RET_LOG(ioHandle != OPEN_PORT_FAILURE, false,
        "invalid ioHandle for sinkName: %{public}s", sinkName.c_str());
    for (auto &volumeType : defaultVolumeTypeList_) {
        if (it != sinkNameMap.end()) {
            if ((it->second == OFFLOAD_CLASS && volumeType == STREAM_MUSIC) ||
                it->second != OFFLOAD_CLASS) {
                audioVolume->SetPipeVolumeMute(ioHandle, volumeType, isMute);
            }
        } else if (sinkName.find("_out") != std::string::npos &&
            sinkName.find(LOCAL_NETWORK_ID) == std::string::npos) {
            audioVolume->SetPipeVolumeMute(ioHandle, volumeType, isMute);
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

int32_t AudioAdapterManager::SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_deviceVolumeBehaviorListenerMutex);
    deviceVolumeBehaviorListener_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    return SUCCESS;
}

bool AudioAdapterManager::IsDistributedVolumeType(AudioStreamType streamType)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    bool ret = std::count(DISTRIBUTED_VOLUME_TYPE_LIST.begin(), DISTRIBUTED_VOLUME_TYPE_LIST.end(), volumeType) != 0;
    return ret;
}

void AudioAdapterManager::SetSleVoiceStatusFlag(bool isSleVoiceStatus)
{
    std::lock_guard<std::mutex> lock(setVoiceStatusMutex_);
    CHECK_AND_RETURN_LOG(isSleVoiceStatus_ != isSleVoiceStatus, "the isSleVoiceStatus state has not changed");
    isSleVoiceStatus_ = isSleVoiceStatus;
    AUDIO_INFO_LOG("SetSleVoiceStatusFlag: %{public}d", isSleVoiceStatus);
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_NEARLINK);
    SetVolumeDbForDeviceInPipe(desc, STREAM_MUSIC);
}

void AudioAdapterManager::RedirectVolumeType(std::shared_ptr<AudioStreamDescriptor> streamDescriptor,
    AudioVolumeType &volumeType)
{
    CHECK_AND_RETURN_LOG(streamDescriptor != nullptr, "streamDescriptor is null");
    if (volumeType == STREAM_VOICE_ASSISTANT &&
        !CheckoutSystemAppUtil::CheckoutSystemApp(audioActiveDevice_.GetRealUid(streamDescriptor)) &&
        streamDescriptor->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_ASSISTANT) {
        AUDIO_INFO_LOG("RedirectVolumeType to STREAM_MUSIC when not system app for uid %{public}d",
            audioActiveDevice_.GetRealUid(streamDescriptor));
        volumeType = STREAM_MUSIC;
    }
}

void AudioAdapterManager::SaveSystemVolumeForSwitchDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t volumeLevel)
{
    if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP || streamType != STREAM_MUSIC) {
        // volume manager no save BLUETOOTH_A2DP abs volume, so this value is wrongs
        // BLUETOOTH_A2DP abs volume save in other place
        SaveSystemVolumeForEffect(desc->deviceType_, streamType, volumeLevel);
    }
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
    ringerMode_.store(ringerMode);

    if (handler_ != nullptr) {
        handler_->SendRingerModeUpdate(ringerMode);
    }
    return SUCCESS;
}

AudioRingerMode AudioAdapterManager::GetRingerMode() const
{
    return ringerMode_.load();
}

 bool AudioAdapterManager::IsPaRoute(uint32_t routeFlag)
{
    int32_t enableNewRoute = 0;
    GetSysPara("persist.multimedia.enable_new_route", enableNewRoute);
    if (IsLowLatencyRoute(routeFlag) || IsDirectRoute(routeFlag)) {
        return enableNewRoute == 1;
    }
    if ((routeFlag & AUDIO_OUTPUT_FLAG_HWDECODING)) {
        return false;
    }
    return true;
}

bool AudioAdapterManager::IsLowLatencyRoute(uint32_t routeFlag)
{
    return (routeFlag & AUDIO_OUTPUT_FLAG_FAST) || (routeFlag & AUDIO_OUTPUT_FLAG_VOIP_FAST) ||
        (routeFlag & AUDIO_INPUT_FLAG_FAST) || (routeFlag & AUDIO_INPUT_FLAG_VOIP_FAST);
}

void AudioAdapterManager::DepressVolume(float &volume, int32_t volumeLevel,
    AudioStreamType streamType, std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    DeviceType deviceType = device->deviceType_;
    if (streamType == STREAM_VOICE_CALL_ASSISTANT ||
        streamType == STREAM_ULTRASONIC) {
        return;
    }
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    static const AudioVolumeType voiceCallType = VolumeUtils::GetVolumeTypeFromStreamType(STREAM_VOICE_CALL);

    auto devForCall = audioActiveDevice_.GetDeviceForVolume(voiceCallType);
    CHECK_AND_RETURN_LOG(devForCall != nullptr, "device is null");
    if (device->networkId_ != devForCall->networkId_) {
        AUDIO_INFO_LOG("[streamType:%{public}d] volume only depressed in same device", streamType);
        return;
    }

    auto &sceneManager = AudioSceneManager::GetInstance();
    AudioScene curScene = sceneManager.GetAudioScene(true);

    bool streamInCall = curScene == AUDIO_SCENE_PHONE_CALL || curScene == AUDIO_SCENE_PHONE_CHAT;
    bool updateLimit = volumeType == voiceCallType || volumeLimit_.load() == MAX_STREAM_VOLUME;
    if (streamInCall) {
        if (updateLimit) {
            int32_t volumeLevelForCall = GetStreamVolumeInternal(device, voiceCallType);
            float newLimit = volumeType == voiceCallType?
                volume : GetSystemVolumeInDb(voiceCallType, volumeLevelForCall, deviceType);
            SetVolumeLimit(newLimit);
        }
    } else {
        SetVolumeLimit(MAX_STREAM_VOLUME);
    }

    float volumeReduction = streamInCall ? GetVolumeReductionRatio(streamType) : 0;
    float expect = volumeLimit_.load();
    if (volumeType != voiceCallType &&
        volumeReduction > std::numeric_limits<float>::epsilon() &&
        volumeReduction < MAX_STREAM_VOLUME + std::numeric_limits<float>::epsilon()) {
        expect *= volumeReduction;
        AUDIO_INFO_LOG("expect:%{public}f volume:%{public}f, volumeReduction:%{public}f",
            expect, volume, volumeReduction);
        expect = expect > std::numeric_limits<float>::epsilon() ? expect : volumeLimit_.load();
    }

    volume = std::min(volume, expect);
}

float AudioAdapterManager::GetVolumeReductionRatio(AudioStreamType streamType)
{
    float volumeReduction = 0;
    auto iter = lowerVolumeInfos_.find(streamType);
    CHECK_AND_RETURN_RET_LOG(iter != lowerVolumeInfos_.end(), volumeReduction,
        "streamType:%{public}d is not supported", streamType);
    auto info = iter->second;
    CHECK_AND_RETURN_RET_LOG(info != nullptr, volumeReduction, "info is null");

    float duckedDb = info->duckedDb;
    CHECK_AND_RETURN_RET_LOG(duckedDb <= std::numeric_limits<float>::epsilon(), volumeReduction,
        "duckedDb:%{public}f should be negative", duckedDb);

    const int base = 10;
    const int divider = 20;
    volumeReduction = pow(base, duckedDb / divider);
    AUDIO_INFO_LOG("volumeReduction:%{public}f", volumeReduction);

    return volumeReduction;
}

void AudioAdapterManager::UpdateOtherStreamVolume(AudioStreamType streamType)
{
    auto pipeDeviceVolumeInfo = AudioPipeManager::GetPipeManager()->GetAllPipeDeviceVolumeInfo();
    for (auto &info : pipeDeviceVolumeInfo) {
        for (auto volumeType : info.volumeTypes_) {
            CHECK_AND_CONTINUE(volumeType != streamType);
            UpdateTypeVolumeForPipe(info.ioHandle_, volumeType, info.deviceDesc_);
        }
    }
}

void AudioAdapterManager::SetVolumeLimit(float volume)
{
    if (volumeLimit_ != volume) {
        AUDIO_INFO_LOG("volume limit set to %{public}f", volume);
        volumeLimit_ = volume;
    }
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
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);

    deviceVolumeInfo = std::make_shared<AllDeviceVolumeInfo>();
    deviceVolumeInfo->deviceType = deviceType;
    deviceVolumeInfo->streamType = streamType;
    deviceVolumeInfo->volumeValue = volumeDataMaintainer_.LoadVolumeFromMap(desc, streamType);

    return deviceVolumeInfo;
}

// LCOV_EXCL_START
AudioIOHandle AudioAdapterManager::OpenAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(pipeInfo->moduleInfo_);
    AUDIO_INFO_LOG("[PipeExecInfo] pipe name %{public}s", pipeInfo->name_.c_str());
    curActiveCount_++;
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    if (IsPaRoute(pipeInfo->routeFlag_)) {
        return OpenPaAudioPort(pipeInfo, paIndex, moduleArgs);
    }
    return OpenNotPaAudioPort(pipeInfo, paIndex);
}

bool AudioAdapterManager::IsDirectRoute(uint32_t routeFlag) const
{
    return (routeFlag & AUDIO_OUTPUT_FLAG_DIRECT);
}

int32_t AudioAdapterManager::CreateHdiPort(std::shared_ptr<AudioPipeInfo> pipeInfo, AudioIOHandle & ioHandle)
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t result = SUCCESS;
    if (pipeInfo->pipeRole_ == PIPE_ROLE_OUTPUT) {
        IAudioSinkAttr attr = GetAudioSinkAttr(pipeInfo->moduleInfo_);
        attr.routeFlag = pipeInfo->routeFlag_;
        pipeInfo->moduleInfo_.routeFlag = pipeInfo->routeFlag_;
        std::string idInfo = HDI_ID_INFO_DEFAULT;
        HdiIdType idType = HDI_ID_TYPE_PRIMARY;
        if (IsLowLatencyRoute(pipeInfo->routeFlag_)) {
            GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
            if ((pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) ||
                (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP_FAST)) {
                AUDIO_INFO_LOG("Use voip mmap");
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
            } else {
                attr.audioStreamFlag = AUDIO_FLAG_MMAP;
            }
            result = audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, idType, idInfo, attr, ioHandle);
        } else if (IsDirectRoute(pipeInfo->routeFlag_)) {
            GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
            if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_VOIP) {
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_DIRECT;
            } else {
                attr.audioStreamFlag = AUDIO_FLAG_DIRECT;
                idInfo = HDI_ID_INFO_DIRECT;
            }
            audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, idType, idInfo, attr, ioHandle);
        } else {
            idInfo = GetHdiSinkIdInfo(pipeInfo->moduleInfo_);
            result = audioServerProxy_->CreateHdiSinkPort(pipeInfo->moduleInfo_.className, idInfo, attr, ioHandle);
        }
    } else if (pipeInfo->pipeRole_ == PIPE_ROLE_INPUT) {
        std::string idInfo = GetHdiSourceIdInfo(pipeInfo->moduleInfo_);
        IAudioSourceAttr attr = GetAudioSourceAttr(pipeInfo->moduleInfo_);
        if (IsLowLatencyRoute(pipeInfo->routeFlag_)) {
            if ((pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOIP)) {
                AUDIO_INFO_LOG("Use voip mmap");
                attr.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
            } else {
                attr.audioStreamFlag = AUDIO_FLAG_MMAP;
            }
        }
        attr.routeFlag = pipeInfo->routeFlag_;
        pipeInfo->moduleInfo_.routeFlag = pipeInfo->routeFlag_;
        result = audioServerProxy_->CreateHdiSourcePort(pipeInfo->moduleInfo_.className, idInfo, attr, ioHandle);
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return result;
}

AudioIOHandle AudioAdapterManager::OpenPaAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex,
    std::string moduleArgs)
{
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");
    int32_t result = CreateHdiPort(pipeInfo, ioHandle);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ioHandle, "Call audioServer open port failed:%{public}d", result);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        int32_t ret = audioServiceAdapter_->OpenAudioPort(pipeInfo->moduleInfo_.lib, pipeInfo->moduleInfo_);
        paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);
    } else {
        paIndex = audioServiceAdapter_->OpenAudioPort(pipeInfo->moduleInfo_.lib, moduleArgs.c_str());
    }
    HILOG_COMM_INFO("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
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
        if (IsLowLatencyRoute(pipeInfo->routeFlag_)) {
            if ((pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOIP) ||
                (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOIP_FAST)) {
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
    HILOG_COMM_INFO("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
    return ioHandle;
}

void AudioAdapterManager::GetSinkIdInfoAndIdType(
    std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &idInfo, HdiIdType &idType)
{
    if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_HWDECODING) {
        idType = HDI_ID_TYPE_HWDECODE;
        idInfo = HDI_ID_INFO_DP;
    }

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
        } else if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_INTERPHONE) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_INTERPHONE;
        }
    } else if (pipeInfo->adapterName_ == "a2dp") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_BLUETOOTH;
            idInfo = HDI_ID_INFO_MMAP;
        }
    } else if (pipeInfo->adapterName_ == "dp") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_FAST;
            idInfo = HDI_ID_INFO_DP_FAST;
        }
    } else if (pipeInfo->adapterName_ == "usb") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_FAST;
            idInfo = HDI_ID_INFO_USB;
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
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_AI) {
            idType = HDI_ID_TYPE_AI;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_UNPROCESS) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_UNPROCESS;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_LIVE) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_LIVE;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_ULTRASONIC) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_ULTRASONIC;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOICE_RECOGNITION) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_VOICE_RECOGNITION;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_RAW_AI) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_RAW_AI;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_INTERPHONE) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_INTERPHONE;
        }
        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_CAMCORDER) {
            idType = HDI_ID_TYPE_PRIMARY;
            idInfo = HDI_ID_INFO_CAMCORDER;
        }
    } else if (pipeInfo->adapterName_ == "usb") {
        if (pipeInfo->routeFlag_ & AUDIO_OUTPUT_FLAG_FAST) {
            idType = HDI_ID_TYPE_FAST;
            idInfo = HDI_ID_INFO_USB;
        }
    }
}

void AudioAdapterManager::ReloadAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);
    AUDIO_INFO_LOG("[PipeExecInfo] PA moduleArgs %{public}s", moduleArgs.c_str());

    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "ServiceAdapter is null");
    CHECK_AND_RETURN_LOG(audioServerProxy_ != nullptr, "audioServerProxy_ null");

    int32_t ret = audioServiceAdapter_->ReloadAudioPort(audioModuleInfo.lib, audioModuleInfo);
    paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);

    HILOG_COMM_INFO("[PipeExecInfo] Reload audio port, paIndex: %{public}u end", paIndex);
}

AudioIOHandle AudioAdapterManager::ReloadA2dpAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t &paIndex)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);
    AUDIO_INFO_LOG("[PipeExecInfo] PA moduleArgs %{public}s", moduleArgs.c_str());

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    AudioIOHandle ioHandle = HDI_INVALID_ID;
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ioHandle, "audioServerProxy_ null");
    curActiveCount_++;

    int32_t result = SUCCESS;
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (audioModuleInfo.lib == "libmodule-inner-capturer-sink.z.so") {
        std::string idInfo = audioModuleInfo.name;
        IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
        audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, idInfo, attr, ioHandle);
    } else {
        if (audioModuleInfo.role == HDI_AUDIO_PORT_SINK_ROLE) {
            std::string idInfo = GetHdiSinkIdInfo(audioModuleInfo);
            IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
            result = audioServerProxy_->CreateHdiSinkPort(audioModuleInfo.className, idInfo, attr, ioHandle);
        } else if (audioModuleInfo.role == HDI_AUDIO_PORT_SOURCE_ROLE) {
            std::string idInfo = GetHdiSourceIdInfo(audioModuleInfo);
            IAudioSourceAttr attr = GetAudioSourceAttr(audioModuleInfo);
            result = audioServerProxy_->CreateHdiSourcePort(audioModuleInfo.className, idInfo, attr, ioHandle);
        }
    }
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ioHandle, "Call audioServer open port failed:%{public}d", result);
    int32_t ret = audioServiceAdapter_->ReloadAudioPort(audioModuleInfo.lib, audioModuleInfo);
    paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);

    HILOG_COMM_INFO("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
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
    int32_t result = SUCCESS;
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (audioModuleInfo.lib == "libmodule-inner-capturer-sink.z.so") {
        std::string idInfo = audioModuleInfo.name;
        IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
        result = audioServerProxy_->CreateSinkPort(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, idInfo, attr, ioHandle);
    } else {
        if (audioModuleInfo.role == HDI_AUDIO_PORT_SINK_ROLE) {
            std::string idInfo = GetHdiSinkIdInfo(audioModuleInfo);
            IAudioSinkAttr attr = GetAudioSinkAttr(audioModuleInfo);
            result = audioServerProxy_->CreateHdiSinkPort(audioModuleInfo.className, idInfo, attr, ioHandle);
        } else if (audioModuleInfo.role == HDI_AUDIO_PORT_SOURCE_ROLE) {
            std::string idInfo = GetHdiSourceIdInfo(audioModuleInfo);
            IAudioSourceAttr attr = GetAudioSourceAttr(audioModuleInfo);
            result = audioServerProxy_->CreateHdiSourcePort(audioModuleInfo.className, idInfo, attr, ioHandle);
        }
    }
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ioHandle, "Call audioServer open port failed:%{public}d", result);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ioHandle, "audioServiceAdapter_ null");
        int32_t ret = audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, audioModuleInfo);
        paIndex = ret < 0 ? HDI_INVALID_ID : static_cast<uint32_t>(ret);
    } else {
        paIndex = audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, moduleArgs.c_str());
    }

    HILOG_COMM_INFO("[PipeExecInfo] Open %{public}u port, paIndex: %{public}u end", ioHandle, paIndex);
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
    HILOG_COMM_INFO("[PipeExecInfo] Close %{public}u port, paIndex: %{public}u end", handleToClose, ioHandle);
    return ret;
}

int32_t AudioAdapterManager::GetCurActivateCount() const
{
    return curActiveCount_ > 0 ? curActiveCount_ : 0;
}

int32_t AudioAdapterManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) const
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    int32_t ret = 0;
    AudioEffectPropertyArray effectPropertyArray = {};
    ret = audioServiceAdapter_->GetAudioEffectProperty(effectPropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetAudioEffectProperty failed");
    propertyArray.property.insert(propertyArray.property.end(),
        effectPropertyArray.property.begin(), effectPropertyArray.property.end());
    AudioEffectPropertyArray enhancePropertyArray = {};
    ret = audioServiceAdapter_->GetAudioEnhanceProperty(enhancePropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetAudioEnhanceProperty failed");
    propertyArray.property.insert(propertyArray.property.end(),
        enhancePropertyArray.property.begin(), enhancePropertyArray.property.end());
    return ret;
}

int32_t AudioAdapterManager::UpdateCollaborativeState(bool isCollaborationEnabled)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    AUDIO_INFO_LOG("AudioCollaborativeService UpdateCollaborativeState entered!");
    audioPolicyServerHandler_->SendCollaborationEnabledChangeForCurrentDeviceEvent(isCollaborationEnabled);
    return audioServiceAdapter_->UpdateCollaborativeState(isCollaborationEnabled);
}

int32_t AudioAdapterManager::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    AUDIO_INFO_LOG("AudioSpatializationService UpdateSpatializationState entered!");
    return audioServiceAdapter_->UpdateSpatializationState(spatializationState);
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
    if (!audioModuleInfo.busAddress.empty()) {
        return audioModuleInfo.busAddress;
    }
    if (audioModuleInfo.className == "remote" || audioModuleInfo.className == "remote_offload") {
        return audioModuleInfo.networkId;
    }
    if (audioModuleInfo.className == "primary_extra") {
        return audioModuleInfo.name;
    }
    return HDI_ID_INFO_DEFAULT;
}

std::string AudioAdapterManager::GetHdiSourceIdInfo(const AudioModuleInfo &audioModuleInfo) const
{
    if (!audioModuleInfo.busAddress.empty()) {
        return audioModuleInfo.busAddress;
    }
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
    } else if (format == "s16le" || format == "s16") {
        return SAMPLE_S16LE;
    } else if (format == "s24le" || format == "s24") {
        return SAMPLE_S24LE;
    } else if (format == "s32le" || format == "s32") {
        return SAMPLE_S32LE;
    }
    AUDIO_ERR_LOG("invalid param: %{public}s", format.c_str());
    return INVALID_WIDTH;
}

static AudioSampleFormat ParseSourceAudioSampleFormat(const std::string &format)
{
    if (format == "u8" || format == "s8") {
        return SAMPLE_U8;
    } else if (format == "s16le" || format == "s16be" || format == "s16") {
        return SAMPLE_S16LE;
    } else if (format == "s24le" || format == "s24be" || format == "s24") {
        return SAMPLE_S24LE;
    } else if (format == "s32le" || format == "s32be" || format == "s32") {
        return SAMPLE_S32LE;
    } else if (format == "f32le" || format == "f32be" || format == "f32") {
        return SAMPLE_F32LE;
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

static void UpdateSourceAttrWithMicInConfig(IAudioSourceAttr &attr, const AudioModuleInfo &audioModuleInfo)
{
    // When micIn stream attributes are configured, use them as capture source attributes.
    if (!audioModuleInfo.micInRate.empty()) {
        attr.sampleRateMicIn = static_cast<uint32_t>(std::stoul(audioModuleInfo.micInRate));
    }
    if (!audioModuleInfo.micInChannels.empty()) {
        attr.channelMicIn = static_cast<uint32_t>(std::stoul(audioModuleInfo.micInChannels));
    }
    if (!audioModuleInfo.micInFormat.empty()) {
        attr.formatMicIn = ParseSourceAudioSampleFormat(audioModuleInfo.micInFormat);
    }
}

IAudioSinkAttr AudioAdapterManager::GetAudioSinkAttr(const AudioModuleInfo &audioModuleInfo) const
{
    IAudioSinkAttr attr;
    attr.adapterName = audioModuleInfo.adapterName.c_str();
    if (!audioModuleInfo.busAddress.empty()) {
        attr.address = audioModuleInfo.busAddress;
    }
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
    attr.address = audioModuleInfo.macAddress;
    if (!audioModuleInfo.deviceType.empty()) {
        attr.deviceType = std::stoi(audioModuleInfo.deviceType);
    }
    if (!audioModuleInfo.channelLayout.empty()) {
        attr.channelLayout = static_cast<uint64_t>(std::stoul(audioModuleInfo.channelLayout));
    }
    attr.address = audioModuleInfo.macAddress;
    if (!audioModuleInfo.hdPlayBackMode.empty()) {
        attr.hdPlayBackMode = static_cast<int32_t>(std::stoul(audioModuleInfo.hdPlayBackMode));
    }

    // Calculate period for ultra_fast to achieve 2.5ms low latency
    if (audioModuleInfo.className == "ultra_fast" || audioModuleInfo.className == "usb_arm_ultra_fast") {
        uint32_t formatByteSize = GetFormatByteSize(attr.format);
        attr.period = static_cast<uint32_t>(
            static_cast<float>(attr.sampleRate * attr.channel * formatByteSize) *
            ULTRA_FAST_PERIOD_TIME_IN_MS / static_cast<float>(MILLISECOND_PER_SECOND));
        AUDIO_INFO_LOG("GetAudioSinkAttr: ultra_fast period=%{public}u (2.5ms latency)", attr.period);
    }

    return attr;
}

void AudioAdapterManager::GetHdiSourceTypeToAudioSourceAttr(IAudioSourceAttr &attr, int32_t sourceType) const
{
    auto sourceStrategyMapget = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    if (sourceStrategyMapget == nullptr) {
        return;
    }
    auto sampIt = sourceStrategyMapget->find((SourceType)sourceType);
    if (sampIt == sourceStrategyMapget->end()) {
        return;
    }
    attr.hdiSourceType = sampIt->second.hdiSource;
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
    attr.macAddress = audioModuleInfo.macAddress.c_str();
    if (!audioModuleInfo.deviceType.empty()) {
        attr.deviceType = std::stoi(audioModuleInfo.deviceType);
    }
    if (!audioModuleInfo.sourceType.empty()) {
        attr.sourceType = std::stoi(audioModuleInfo.sourceType);
    }

    UpdateSourceAttrWithMicInConfig(attr, audioModuleInfo);

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
    GetHdiSourceTypeToAudioSourceAttr(attr, attr.sourceType);
    attr.isPrimarySinkExist_ = isPrimarySinkExist_.load();
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
        case DEVICE_TYPE_LINE_DIGITAL:
            return SPEAKER_VOLUME_TYPE;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_NEARLINK:
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
    RecordBootStateTime(AudioPolicyServerBootState::INIT_DATABASE, ClockTime::GetCurMilli());

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

void AudioAdapterManager::UpdateUsbSafeVolume(std::shared_ptr<AudioDeviceDescriptor> &device)
{
    if (!isDataShareReady_) {
        AUDIO_INFO_LOG("DataShare is not ready, return");
        return;
    }
    if (GetStreamVolumeInternal(device, STREAM_MUSIC) <= safeVolume_) {
        AUDIO_INFO_LOG("1st connect bt device volume is safe");
        isWiredBoot_ = false;
        return;
    }
    AUDIO_INFO_LOG("isWiredBoot_:%{public}d, safeStatus_:%{public}d", isWiredBoot_, safeStatus_);
    if (isWiredBoot_ || safeStatus_) {
        AUDIO_INFO_LOG("1st connect wired device:%{public}d after boot, update current volume to safevolume",
            device->deviceType_);
        SaveVolumeData(device, STREAM_MUSIC, safeVolume_, true, true);
        isWiredBoot_ = false;
    }
}

void AudioAdapterManager::UpdateSafeVolumeInner(std::shared_ptr<AudioDeviceDescriptor> &device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    AUDIO_INFO_LOG("current device type:%{public}d", device->deviceType_);
    switch (device->deviceType_) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            UpdateUsbSafeVolume(device);
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (GetStreamVolumeInternal(device, STREAM_MUSIC) <= safeVolume_) {
                AUDIO_INFO_LOG("1st connect bt device volume is safe");
                isBtBoot_ = false;
                return;
            }
            if (device->deviceCategory_ == BT_CAR || device->deviceCategory_ == BT_SOUNDBOX) {
                AUDIO_ERR_LOG("current device: %{public}d is not support", device->deviceCategory_);
                return;
            }
            if (isBtBoot_ || safeStatusBt_) {
                AUDIO_INFO_LOG("1st connect bt device:%{public}d after boot, update current volume to safevolume",
                    device->deviceType_);
                SaveVolumeData(device, STREAM_MUSIC, safeVolume_, true, true);
                isBtBoot_ = false;
            }
            break;
        case DEVICE_TYPE_NEARLINK:
            if (GetStreamVolumeInternal(device, STREAM_MUSIC) <= safeVolume_) {
                AUDIO_INFO_LOG("1st connect sle device volume is safe");
                isSleBoot_ = false;
                return;
            }
            if (isSleBoot_ || safeStatusSle_) {
                AUDIO_INFO_LOG("1st connect sle device:%{public}d after boot, update current volume to safevolume",
                    device->deviceType_);
                SaveVolumeData(device, STREAM_MUSIC, safeVolume_, true, true);
                isSleBoot_ = false;
            }
            break;
        default:
            AUDIO_ERR_LOG("current device: %{public}d is not support", device->deviceType_);
            break;
    }
}

void AudioAdapterManager::UpdateSafeVolume()
{
    auto descs = audioConnectedDevice_.GetCopy();
    for (auto &device: descs) {
        UpdateSafeVolumeInner(device);
    }
}

void AudioAdapterManager::InitVolumeMap(bool isFirstBoot)
{
    if (!isFirstBoot) {
        LoadVolumeMap();
        return;
    }
    bool resetFirstFlag = false;
    AUDIO_INFO_LOG("InitVolumeMap: Write defalut stream volumes to database");
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
        for (auto &streamType : defaultVolumeTypeList_) {
            CHECK_AND_CONTINUE(volumeDataMaintainer_.CheckVolumeState(desc, streamType) != SUCCESS);
            int32_t volumeLevel = AudioVolumeUtils::GetInstance().GetDefaultVolumeLevel(desc, streamType);
            VolumeScale buffedVolume = {volumeLevel, -1, GetMaxVolumeLevel(streamType, desc)};
            auto ret = volumeDataMaintainer_.SaveVolumeToDb(desc, streamType, buffedVolume);
            resetFirstFlag = (ret == SUCCESS) ? resetFirstFlag : true;
        }
    }
    CHECK_AND_RETURN(resetFirstFlag);
    AUDIO_INFO_LOG("reset first boot init settingsdata");
    SetFirstBoot(true);
}

void AudioAdapterManager::InitRingerMode(bool isFirstBoot)
{
    AudioRingerMode ringerMode = ringerMode_.load();
    if (isFirstBoot) {
        isLoaded_ = true;
        if (!volumeDataMaintainer_.GetRingerMode(ringerMode)) {
            isLoaded_ = volumeDataMaintainer_.SaveRingerMode(ringerMode_);
        }
        AUDIO_INFO_LOG("InitRingerMode first boot ringermode:%{public}d", ringerMode);
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
        isLoaded_ = volumeDataMaintainer_.GetRingerMode(ringerMode);
    }

    ringerMode_.store(ringerMode);
    auto desc = audioActiveDevice_.GetDeviceForVolume(STREAM_NOTIFICATION);
    int32_t volumeLevel =
        GetStreamVolumeInternal(desc, STREAM_NOTIFICATION) * ((ringerMode_ != RINGER_MODE_NORMAL) ? 0 : 1);
    // Save volume in local prop for bootanimation
    SaveNotificationVolumeToLocal(desc, STREAM_NOTIFICATION, volumeLevel);
}

void AudioAdapterManager::CloneVolumeMap(void)
{
    CHECK_AND_RETURN_LOG(audioPolicyKvStore_ != nullptr, "clone volumemap failed, audioPolicyKvStore_nullptr");
    // read volume from private Kvstore
    AUDIO_INFO_LOG("Copy Volume from private database to shareDatabase");
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
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
            SaveVolumeData(desc, streamType, volumeLevel, true);
        }
        UpdateSafeVolumeInner(desc);
    }

    isNeedCopyVolumeData_ = false;
}

void AudioAdapterManager::HandleDpConnection()
{
    AUDIO_INFO_LOG("no use");
}

bool AudioAdapterManager::LoadVolumeMap(void)
{
    if (isNeedCopyVolumeData_ && (audioPolicyKvStore_ != nullptr)) {
        CloneVolumeMap();
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
    }
    // reLoad the current device mute status
    LoadMuteStatusMap();
}

void  AudioAdapterManager::CheckAndDealMuteStatus(const DeviceType &deviceType, const AudioStreamType &streamType)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(deviceType);
    bool muteSate = false;
    if (streamType == STREAM_RING && !VolumeUtils::IsPCVolumeEnable()) {
        muteSate = (ringerMode_ == RINGER_MODE_NORMAL) ? false : true;
        AUDIO_INFO_LOG("fist boot ringer mode:%{public}d, stream ring mute state:%{public}d", ringerMode_.load(),
            muteSate);
    }
    CHECK_AND_RETURN(volumeDataMaintainer_.CheckMuteState(desc, streamType) != SUCCESS);
    volumeDataMaintainer_.SaveMuteToDb(desc, streamType, muteSate);
}

void AudioAdapterManager::SetVolumeCallbackAfterClone()
{
    for (auto &streamType : defaultVolumeTypeList_) {
        VolumeEvent volumeEvent;
        volumeEvent.volumeType = streamType;
        volumeEvent.volume = GetSystemVolumeLevel(streamType);
        volumeEvent.volumeDegree = GetSystemVolumeDegree(streamType);
        volumeEvent.updateUi = false;
        volumeEvent.volumeGroupId = 0;
        volumeEvent.networkId = LOCAL_NETWORK_ID;
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
            audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent);
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
            auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
            if (desc->deviceType_ == deviceType) {
                SetStreamMuteInternal(desc, streamType, muteStatus);
            }
            SaveMuteToDbAsync(desc, streamType, muteStatus);
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
            if (!volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_NEARLINK, safeStatusSle_) &&
                (deviceType == DEVICE_TYPE_NEARLINK)) {
                volumeDataMaintainer_.SaveSafeStatus(DEVICE_TYPE_NEARLINK, SAFE_ACTIVE);
            }
        }
    } else {
        volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);
        volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
        volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_NEARLINK, safeStatusSle_);
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
            if (!volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_NEARLINK, safeActiveSleTime_) &&
                (deviceType == DEVICE_TYPE_NEARLINK)) {
                volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_NEARLINK, 0);
            }
            ConvertSafeTime();
            isNeedConvertSafeTime_ = false;
        }
    } else {
        volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_WIRED_HEADSET, safeActiveTime_);
        volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_BLUETOOTH_A2DP, safeActiveBtTime_);
        volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_NEARLINK, safeActiveSleTime_);
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
    if (safeActiveSleTime_ > 0) {
        safeActiveSleTime_ = safeActiveSleTime_ / CONVERT_FROM_MS_TO_SECONDS;
        volumeDataMaintainer_.SaveSafeVolumeTime(DEVICE_TYPE_NEARLINK, safeActiveSleTime_);
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
            volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
            return safeStatusBt_;
        case DEVICE_TYPE_NEARLINK:
            volumeDataMaintainer_.GetSafeStatus(DEVICE_TYPE_NEARLINK, safeStatusSle_);
            return safeStatusSle_;
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
        case DEVICE_TYPE_NEARLINK:
            volumeDataMaintainer_.GetSafeVolumeTime(DEVICE_TYPE_NEARLINK, safeActiveSleTime_);
            return safeActiveSleTime_;
        default:
            AUDIO_ERR_LOG("current device : %{public}d is not support", deviceType);
            break;
    }

    return -1;
}

void AudioAdapterManager::InitSystemAppVolume(bool isFirstBoot)
{
    AUDIO_INFO_LOG("enter");
    CHECK_AND_RETURN_LOG(isDataShareReady_ == true, "DataShare not ready");
    HandleAppIndividualVolumeSettingChanged();
    if (!isFirstBoot) {
        RestoreSystemAppVolumeLevel();
        RestoreSystemAppVolumeMuteStatus();
    }
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
        case DEVICE_TYPE_NEARLINK:
            volumeDataMaintainer_.GetRestoreVolumeLevel(DEVICE_TYPE_NEARLINK, safeActiveSleVolume_);
            return safeActiveSleVolume_;
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
    } else if (deviceType == DEVICE_TYPE_NEARLINK) {
        safeStatusSle_ = status;
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
    } else if (deviceType == DEVICE_TYPE_NEARLINK) {
        safeActiveSleTime_ = time;
    }
    bool ret = volumeDataMaintainer_.SaveSafeVolumeTime(deviceType, time);
    CHECK_AND_RETURN_RET(ret, ERROR, "SetDeviceSafeTime failed");
    return SUCCESS;
}

void AudioAdapterManager::InitLegacyHighVolumeCheckTime(bool isFirstBoot)
{
    int32_t notifyVolumeLevel = VolumeUtils::GetLegacyVolumeNotificationLevel();
    if (isFirstBoot && notifyVolumeLevel > 0) {
        int64_t time = 0;
        CHECK_AND_RETURN_LOG(volumeDataMaintainer_.GetLegacyHighVolumeTime(time),
            "init legacy high volume check time failed");
        legacyHighVolumeCheckTime_ = time;
        AUDIO_INFO_LOG("init legacy high volume check time: %{public}" PRIu64, legacyHighVolumeCheckTime_.load());
    }
}

int32_t AudioAdapterManager::SetLegacyHighVolumeCheckTime(int64_t checkTime)
{
    legacyHighVolumeCheckTime_ = checkTime;
    CHECK_AND_RETURN_RET_LOG(volumeDataMaintainer_.SaveLegacyHighVolumeTime(checkTime), ERROR,
        "save legacy high volume time failed");
    return SUCCESS;
}

int64_t AudioAdapterManager::GetLegacyHighVolumeCheckTime()
{
    CHECK_AND_RETURN_RET(legacyHighVolumeCheckTime_ <= 0, legacyHighVolumeCheckTime_);
    int64_t time = 0;
    CHECK_AND_RETURN_RET_LOG(volumeDataMaintainer_.GetLegacyHighVolumeTime(time), 0,
        "get legacy high volume check time failed");
    legacyHighVolumeCheckTime_ = time;
    return legacyHighVolumeCheckTime_;
}

int32_t AudioAdapterManager::SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        safeActiveBtVolume_ = volume;
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        safeActiveVolume_ = volume;
    } else if (deviceType == DEVICE_TYPE_NEARLINK) {
        safeActiveSleVolume_ = volume;
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

float AudioAdapterManager::CalculateVolumeDb(int32_t volumeLevel)
{
    float value = static_cast<float>(volumeLevel) / MAX_VOLUME_LEVEL;
    float roundValue = static_cast<int>(value * CONST_FACTOR);

    return static_cast<float>(roundValue) / CONST_FACTOR;
}

float AudioAdapterManager::CalculateVolumeDbExt(int32_t volumeInt, int32_t limit)
{
    if (limit == 0) {
        limit = MAX_VOLUME_DEGREE;
    }

    float value = static_cast<float>(volumeInt) / limit;
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

float AudioAdapterManager::GetSystemVolumeInDbByDegree(AudioVolumeType volumeType,
    DeviceType deviceType, bool mute)
{
    int32_t volumeDegree = GetSystemVolumeDegree(volumeType, false);
    volumeDegree = mute ? 0 : volumeDegree;
    if (useNonlinearAlgo_) {
        getSystemVolumeInDb_ = CalculateVolumeDbNonlinearExt(volumeType, deviceType, volumeDegree);
    } else {
        getSystemVolumeInDb_ = CalculateVolumeDbExt(volumeDegree);
    }

    AUDIO_DEBUG_LOG("volumeType: %{public}d deviceType:%{public}d \
        volumeDegree:%{public}d volumeDb:%{public}f",
        volumeType, deviceType, volumeDegree, getSystemVolumeInDb_.load());

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
    if (Util::IsDualToneStreamType(streamType)) {
        deviceType = DEVICE_TYPE_SPEAKER;
    }
    int32_t minVolIndex = GetMinVolumeLevel(streamAlias, deviceType);
    int32_t maxVolIndex = GetMaxVolumeLevel(streamAlias, deviceType);
    if (minVolIndex < 0 || maxVolIndex < 0 || minVolIndex >= maxVolIndex) {
        return 0.0f;
    }
    volumeLevel = volumeLevel < minVolIndex ? minVolIndex : volumeLevel;
    volumeLevel = volumeLevel > maxVolIndex ? maxVolIndex : volumeLevel;

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
        // for smart display, position 0 and decibel 0,return 1.0f
        if (volumePoints[0].dbValue == 0) {
            AUDIO_INFO_LOG("volumePoints[0]dbValue == 0, return 1.0f");
            return 1.0f;
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

float AudioAdapterManager::CalculateVolumeDbByDegree(DeviceType deviceType,
    AudioStreamType streamType, int32_t volumeDegree)
{
    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        volumeDb = CalculateVolumeDbNonlinearExt(streamType, deviceType, volumeDegree);
    } else {
        volumeDb = CalculateVolumeDbExt(volumeDegree);
    }

    return volumeDb;
}

float AudioAdapterManager::CalculateVolumeDbNonlinearExt(AudioStreamType streamType,
    DeviceType deviceType, int32_t volumeDegree)
{
    float dbValue = 0.0f;
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t volumeLevelMax = GetMaxVolumeLevel(volumeType, deviceType);
    int32_t volumeLevelMin = GetMinVolumeLevel(volumeType, deviceType);
    CHECK_AND_RETURN_RET_LOG(volumeLevelMax >= MIN_VOLUME_LEVEL && volumeLevelMin >= MIN_VOLUME_LEVEL,
        dbValue, "invalid level range:[%{public}d, %{public}d]", volumeLevelMin, volumeLevelMax);

    int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(volumeDegree, volumeLevelMax);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= 0,
        dbValue, "invalid volume level from degree:%{public}d", volumeDegree);

    int32_t preVolumeLevel = std::max(volumeLevel - 1, volumeLevelMin);
    int32_t nextVolumeLevel = std::min(volumeLevel + 1, volumeLevelMax);

    float curDbFactor = CalculateVolumeDbNonlinear(streamType, deviceType, volumeLevel);
    float preDbFactor = CalculateVolumeDbNonlinear(streamType, deviceType, preVolumeLevel);
    float nextDbFactor = CalculateVolumeDbNonlinear(streamType, deviceType, nextVolumeLevel);

    int32_t curDegreeBase = VolumeUtils::VolumeLevelToDegree(volumeLevel, volumeLevelMax);
    int32_t preDegreeCeiling = VolumeUtils::GetVolumeLevelMaxDegree(preVolumeLevel, volumeLevelMax);
    if (volumeDegree < curDegreeBase) {
        int32_t preDegreeBase = VolumeUtils::VolumeLevelToDegree(preVolumeLevel, volumeLevelMax);
        int32_t divide1 = std::max(curDegreeBase - preDegreeBase + preDegreeCeiling - preDegreeBase, 1);
        int32_t divide2 = std::max(curDegreeBase - preDegreeBase, 1);
        float baseDbFactor = preDbFactor + (curDbFactor - preDbFactor) * (preDegreeCeiling - preDegreeBase) / divide1;
        dbValue = baseDbFactor + (curDbFactor - preDbFactor) * (volumeDegree - preDegreeCeiling) / divide2;
    } else {
        int32_t nextDegreeBase = VolumeUtils::VolumeLevelToDegree(nextVolumeLevel, volumeLevelMax);
        int32_t curDegreeCeiling = VolumeUtils::GetVolumeLevelMaxDegree(volumeLevel, volumeLevelMax);
        int32_t divide = std::max(nextDegreeBase - curDegreeBase + curDegreeCeiling - curDegreeBase, 1);
        dbValue = curDbFactor + (nextDbFactor - curDbFactor) * (volumeDegree - curDegreeBase) / divide;
    }

    AUDIO_DEBUG_LOG("volumeDegree=%{public}d, curDegreeBase=%{public}d, "
        "volumeLevel=%{public}d, preDegreeCeiling=%{public}d, db=%{public}f",
        volumeDegree, curDegreeBase, volumeLevel, preDegreeCeiling, dbValue);
    return dbValue;
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
        auto desc = audioActiveDevice_.GetDeviceForVolume(CurStreamType);
        SaveVolumeData(desc, streamVolInfo->streamType, streamVolInfo->defaultLevel, false, true);
        AUDIO_DEBUG_LOG("update streamType %{public}d index = [%{public}d, %{public}d, %{public}d]",
            streamVolInfo->streamType, minVolumeIndexMap_[CurStreamType], maxVolumeIndexMap_[CurStreamType],
            GetStreamVolumeInternal(desc, CurStreamType));
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
        // Failed to find the volume config in streamVolumeInfos_
        if (streamType == STREAM_NOTIFICATION) {
            // Failed to find the config of notification. Use the config of ringtone.
            AUDIO_INFO_LOG("streamType == STREAM_NOTIFICATION. Use the volume config of ringtone");
            streamVolInfo = streamVolumeInfos_.find(STREAM_RING);
        } else {
            AUDIO_DEBUG_LOG("Cannot find stream type %{public}d and try to use STREAM_MUSIC", streamType);
            streamVolInfo = streamVolumeInfos_.find(STREAM_MUSIC);
        }
        CHECK_AND_RETURN_LOG(streamVolInfo != streamVolumeInfos_.end(),
            "Cannot find the alternative configuration!");
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

DeviceCategory AudioAdapterManager::GetCurrentOutputDeviceCategory()
{
    return audioActiveDevice_.GetCurrentOutputDeviceCategory();
}

DeviceType AudioAdapterManager::GetActiveDevice()
{
    return audioActiveDevice_.GetCurrentOutputDeviceType();
}

int32_t AudioAdapterManager::SetNearlinkDeviceVolume(AudioVolumeType volumeType, int32_t volume)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_NEARLINK);
    CHECK_AND_RETURN_RET_LOG(desc, ERROR, "DEVICE_TYPE_NEARLINK device is null");
    SaveVolumeData(desc, volumeType, volume, false, true);
    SetVolumeDbForDeviceInPipe(desc, volumeType);
    return SUCCESS;
}

void AudioAdapterManager::SetAbsVolumeScene(bool isAbsVolumeScene, int32_t volume)
{
    AUDIO_PRERELEASE_LOGI("SetAbsVolumeScene: %{public}d, volume: %{public}d", isAbsVolumeScene, volume);
    isAbsVolumeScene_ = isAbsVolumeScene;
    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "SetAbsVolumeScene audio adapter null");
    audioServiceAdapter_->SetAbsVolumeStateToEffect(isAbsVolumeScene);
    AudioVolumeManager::GetInstance().SetSharedAbsVolumeScene(isAbsVolumeScene_);
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_BLUETOOTH_A2DP);

    if (isAbsVolumeScene) {
        bool mute = volume == 0;
        isAbsVolumeMute_ = mute;
    }

    SetVolumeDbForDeviceInPipe(desc, STREAM_MUSIC);

    if (IsAbsVolumeScene() && !VolumeUtils::IsPCVolumeEnable()) {
        SaveVolumeData(desc, STREAM_VOICE_ASSISTANT,
            GetMaxVolumeLevel(STREAM_MUSIC, DEVICE_TYPE_BLUETOOTH_A2DP), false, true);
        SetStreamMuteInternal(desc, STREAM_VOICE_ASSISTANT, false);
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
    auto descA2DP = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_BLUETOOTH_A2DP);
    SetVolumeDbForDeviceInPipe(descA2DP, STREAM_MUSIC);
}

bool AudioAdapterManager::IsAbsVolumeMute() const
{
    return isAbsVolumeMute_;
}

void AudioAdapterManager::SetAbsVolumeMuteNearlink(bool mute)
{
    AUDIO_INFO_LOG("SetAbsVolumeMuteNearlink: %{public}d", mute);
    isAbsVolumeMuteNearlink_ = mute;
    auto descNearlink = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_NEARLINK);
    SetVolumeDbForDeviceInPipe(descNearlink, STREAM_MUSIC);
}

void AudioAdapterManager::NotifyAccountsChanged(const int &id)
{
    AUDIO_INFO_LOG("start reload the kv data, current id:%{public}d", id);
    AudioRingerMode ringerMode = ringerMode_.load();
    volumeDataMaintainer_.GetRingerMode(ringerMode);
    ringerMode_.store(ringerMode);
    LoadVolumeValueMapFromDb();
    LoadMuteStatusMap();
    UpdateVolumeForAllPipes();
    DealDoNotDisturbStatus();
    DealDoNotDisturbStatusWhiteList();
    HandleAppIndividualVolumeSettingChanged();
}

void AudioAdapterManager::MuteMediaWhenAccountsChanged()
{
    AUDIO_INFO_LOG("mute media when accounts changed!");
    auto desc = audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    volumeDataMaintainer_.SaveMuteToMap(desc, STREAM_MUSIC, true);
    UpdateTypeVolumeForAllPipes(STREAM_MUSIC);
}

int32_t AudioAdapterManager::DoRestoreData()
{
    isLoaded_ = false;
    isNeedConvertSafeTime_ = true; // reset convert safe volume status
    volumeDataMaintainer_.SaveMuteTransferStatus(true); // reset mute convert status
    InitKVStore();
    auto descs = audioConnectedDevice_.GetCopy();
    for (auto &desc : descs) {
        UpdateVolumeWhenDeviceConnect(desc);
    }
    UpdateVolumeForAllPipes();
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
        auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
        AppendFormat(dumpString, "  - samplingAudioStreamTypeate: %d", streamType);
        AppendFormat(dumpString, "   volumeLevel: %d\n", GetStreamVolumeInternal(desc, streamType));
        AppendFormat(dumpString, "  - AudioStreamType: %d", streamType);
        AppendFormat(dumpString, "   streamMuteStatus: %d\n", GetStreamMuteInternal(desc, streamType));
    }
    if (isSafeBoot_) {
        safeStatusBt_ = GetCurrentDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP);
        safeStatus_ = GetCurrentDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET);
        safeStatusSle_ = GetCurrentDeviceSafeStatus(DEVICE_TYPE_NEARLINK);
        safeActiveBtTime_ = GetCurentDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP);
        safeActiveTime_ = GetCurentDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET);
        safeActiveSleTime_ = GetCurentDeviceSafeTime(DEVICE_TYPE_NEARLINK);
        isSafeBoot_ = false;
    }
    std::string statusBt = (safeStatusBt_ == SAFE_ACTIVE) ? "SAFE_ACTIVE" : "SAFE_INACTIVE";
    std::string status = (safeStatus_ == SAFE_ACTIVE) ? "SAFE_ACTIVE" : "SAFE_INACTIVE";
    std::string statusSle = (safeStatusSle_ == SAFE_ACTIVE) ? "SAFE_ACTIVE" : "SAFE_INACTIVE";
    AppendFormat(dumpString, "  - ringerMode: %d\n", ringerMode_.load());
    AppendFormat(dumpString, "  - SafeVolume: %d\n", safeVolume_);
    AppendFormat(dumpString, "  - BtSafeStatus: %s\n", statusBt.c_str());
    AppendFormat(dumpString, "  - SafeStatus: %s\n", status.c_str());
    AppendFormat(dumpString, "  - SleSafeStatus: %s\n", statusSle.c_str());
    AppendFormat(dumpString, "  - ActiveBtSafeTime: %lld\n", safeActiveBtTime_);
    AppendFormat(dumpString, "  - ActiveSafeTime: %lld\n", safeActiveTime_);
    AppendFormat(dumpString, "  - ActiveSleSafeTime: %lld\n", safeActiveSleTime_);
}

void AudioAdapterManager::SetVgsVolumeSupported(bool isVgsSupported)
{
    AUDIO_INFO_LOG("Set Vgs Supported: %{public}d", isVgsSupported);
    isVgsVolumeSupported_ = isVgsSupported;
    UpdatePipeVolumeWhenVgsStatusChanged();
}

bool AudioAdapterManager::IsVgsVolumeSupported() const
{
    CHECK_AND_RETURN_RET(audioActiveDevice_.IsDeviceInActiveOutputDevices(DEVICE_TYPE_BLUETOOTH_SCO, false), false);
    return isVgsVolumeSupported_;
}

std::vector<AdjustStreamVolumeInfo> AudioAdapterManager::GetStreamVolumeInfo(AdjustStreamVolume volumeType)
{
    return AudioVolume::GetInstance()->GetStreamVolumeInfo(volumeType);
}

void AudioAdapterManager::UpdateVolumeForLowLatency(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioVolumeType volumeType)
{
    Trace trace("AudioAdapterManager::UpdateVolumeForLowLatency");
    CHECK_AND_RETURN_LOG(device != nullptr, "device handle null, UpdateVolumeForLowLatency failed");
    // update volumes for low latency streams when loading volumes from the database.
    Volume vol = {false, 1.0f, 0};
    if (volumeType == STREAM_MUSIC && device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP
        && IsAbsVolumeScene()) {
        vol.isMute = isAbsVolumeMute_;
    } else if (volumeType == STREAM_MUSIC && device->deviceType_ == DEVICE_TYPE_NEARLINK) {
        vol.isMute = isAbsVolumeMuteNearlink_.load();
    } else {
        vol.isMute = GetStreamMuteInternal(device, volumeType);
    }
    vol.volumeInt = static_cast<uint32_t>(GetStreamVolumeInternal(device, volumeType));
    vol.volumeFloat = GetSystemVolumeInDbByDegree(volumeType, device->deviceType_, vol.isMute);
    AudioVolumeManager::GetInstance().SetSharedVolume(volumeType, device->deviceType_, vol);
}

void AudioAdapterManager::RegisterDoNotDisturbStatus()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncDoNotDisturb = [&](const std::string &key) {
        DealDoNotDisturbStatus();
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
        DealDoNotDisturbStatusWhiteList();
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

void AudioAdapterManager::DealDoNotDisturbStatus()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t isDoNotDisturb = 0;
    int32_t ret = settingProvider.GetIntValue(DO_NOT_DISTURB_STATUS, isDoNotDisturb, "secure");
    JUDGE_AND_ERR_LOG(ret != SUCCESS, "get doNotDisturbStatus failed");
    AUDIO_INFO_LOG("doNotDisturbStatus = %{public}s", isDoNotDisturb != 0 ? "true" : "false");
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null, set DoNotDisturbStatus failed");
    audioVolume->SetDoNotDisturbStatus(isDoNotDisturb != 0);
}

void AudioAdapterManager::DealDoNotDisturbStatusWhiteList()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    std::vector<std::map<std::string, std::string>> doNotDisturbWhiteList;
    int32_t ret = settingProvider.GetMapValue(DO_NOT_DISTURB_STATUS_WHITE_LIST,
        doNotDisturbWhiteList, "secure");
    JUDGE_AND_ERR_LOG(ret != SUCCESS, "get doNotDisturbStatus WhiteList failed");
    AUDIO_INFO_LOG("doNotDisturbStatusWhiteList changed");
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null, \
        set doNotDisturbStatusWhiteList failed");
    audioVolume->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbWhiteList);
}

void AudioAdapterManager::RegisterMdmMuteSwitchCallback()
{
    int32_t ret = WatchParameter("persist.edm.unmute_device_disallowed",
        [](const char* key, const char* value, void* context) {
            CHECK_AND_RETURN_LOG(value != nullptr, "value is null");
            bool isMute = strcmp(value, "true") == 0;
            CHECK_AND_RETURN_LOG(context != nullptr, "context is null");
            AudioAdapterManager* audioAdapterManager = static_cast<AudioAdapterManager*>(context);
            audioAdapterManager->MdmMuteSwitchCallback(isMute);
        },
        this);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Register mdmMuteStatus failed! Err: %{public}d", ret);
    }
}

void AudioAdapterManager::MdmMuteSwitchCallback(bool isMute)
{
    AUDIO_INFO_LOG("mdmMuteStatus = %{public}d", isMute);
    AudioMuteFactorManager& audioMuteFactorManager = AudioMuteFactorManager::GetInstance();
    audioMuteFactorManager.SetMdmMuteStatus(isMute);
    for (auto &streamType : defaultVolumeTypeList_) {
        if (streamType == STREAM_VOICE_CALL) {
            int32_t volumeLevel = GetStreamVolume(streamType);
            DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
            float volumeFloat = isMute ? 0 : GetSystemVolumeInDb(streamType, volumeLevel, curOutputDeviceType);
            AudioServerProxy::GetInstance().NotifyStreamVolumeChangedProxy(streamType, volumeFloat);
        }
    }
}

void AudioAdapterManager::RegisterAppIndividualVolumeEnableObserver()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc handleAppIndividualVolumeSettingChanged = [&](const std::string &key) {
        HandleAppIndividualVolumeSettingChanged();
    };
    sptr observer = settingProvider.CreateObserver(APP_INDIVIDUAL_VOLUME_ENABLE,
        handleAppIndividualVolumeSettingChanged);
    ErrCode ret = settingProvider.RegisterObserver(observer, "system");
    CHECK_AND_RETURN_LOG(ret == ERR_OK, "Register AppIndividualVolumeEnable fail");
    AUDIO_INFO_LOG("Register AppIndividualVolumeEnable success");
}

void AudioAdapterManager::HandleAppIndividualVolumeSettingChanged()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t appIndividualVolumeEnabledInt = 0;
    int32_t ret = settingProvider.GetIntValue(APP_INDIVIDUAL_VOLUME_ENABLE,
        appIndividualVolumeEnabledInt, "system");
    bool appIndividualVolumeEnabled = appIndividualVolumeEnabledInt == 1 ? true : false;
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "GetIntValue failed");
    AUDIO_INFO_LOG("appVolumeEnable:%{public}d", appIndividualVolumeEnabled);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume is nullptr");
    audioVolume->HandleAppIndividualVolumeSettingChanged(appIndividualVolumeEnabled);
    std::set<int32_t> appUidSet = audioVolume->GetStreamVolumeAppUids();
    for (const auto &appUid : appUidSet) {
        auto desc = audioActiveDevice_.GetDeviceForVolume(appUid);
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
        SendOffloadVolume(desc->networkId_);
    }
}

void AudioAdapterManager::SaveSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType,
    int32_t volumeLevel)
{
    return volumeDataMaintainer_.SaveSystemVolumeForEffect(deviceType, streamType, volumeLevel);
}

int32_t AudioAdapterManager::GetSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetSystemVolumeForEffect(deviceType, streamType);
}

int32_t AudioAdapterManager::SetSystemVolumeToEffect(AudioStreamType streamType, float volume)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERROR, "audioServiceAdapter is null");
    return audioServiceAdapter_->SetSystemVolumeToEffect(streamType, volume);
}

int32_t AudioAdapterManager::StopAudioPort(std::string oldSinkName)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERROR, "audioServiceAdapter_ is null");
    return audioServiceAdapter_->StopAudioPort(oldSinkName);
}

int32_t AudioAdapterManager::AddCaptureInjector()
{
    AudioInjectorPolicy &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    AudioModuleInfo &info = audioInjectorPolicy.GetAudioModuleInfo();
    uint32_t rendererPortIdx = audioInjectorPolicy.GetRendererPortIdx();
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ERROR, "audioServerProxy_ null");
    return audioServerProxy_->AddCaptureInjector(rendererPortIdx, info.rate, info.format,
        info.channels, info.bufferSize);
}

int32_t AudioAdapterManager::RemoveCaptureInjector()
{
    AudioInjectorPolicy &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t rendererPortIdx = audioInjectorPolicy.GetRendererPortIdx();
    CHECK_AND_RETURN_RET_LOG(audioServerProxy_ != nullptr, ERROR, "audioServerProxy_ null");
    return audioServerProxy_->RemoveCaptureInjector(rendererPortIdx);
}

void AudioAdapterManager::AddCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
    const SourceType &sourceType)
{
    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "ServiceAdapter is null");
    audioServiceAdapter_->AddCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

void AudioAdapterManager::RemoveCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
    const SourceType &sourceType)
{
    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "ServiceAdapter is null");
    audioServiceAdapter_->RemoveCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

void AudioAdapterManager::UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo)
{
    CHECK_AND_RETURN_LOG(audioServiceAdapter_ != nullptr, "ServiceAdapter is null");
    audioServiceAdapter_->UpdateAudioPortInfo(sinkPortIndex, audioPortInfo);
}

void AudioAdapterManager::QueryDeviceVolumeBehavior(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    VolumeBehavior behavior;
    CHECK_AND_RETURN_LOG(desc != nullptr, "QueryDeviceVolumeBehavior desc is null");
    CHECK_AND_RETURN_LOG(AudioVolumeUtils::GetInstance().IsDistributedDevice(desc),
        "QueryDeviceVolumeBehavior desc is not dmsdp");
    CHECK_AND_RETURN_LOG(deviceVolumeBehaviorListener_ != nullptr,
        "QueryDeviceVolumeBehavior deviceVolumeBehaviorListener_ is null");
    (void)deviceVolumeBehaviorListener_->OnQueryDeviceVolumeBehavior(behavior);
    // only update target info
    desc->volumeBehavior_.isReady = behavior.isReady;
    desc->volumeBehavior_.isVolumeControlDisabled = behavior.isVolumeControlDisabled;
    desc->volumeBehavior_.databaseVolumeName = behavior.databaseVolumeName;
    AUDIO_INFO_LOG("query ok");
}

void AudioAdapterManager::UpdateVolumeWhenDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "UptdateVolumeWhenDeviceConnect desc is null");
    CHECK_AND_RETURN_LOG(desc->deviceRole_ == OUTPUT_DEVICE, "%{public}s is not output", desc->GetName().c_str());
    CHECK_AND_RETURN_LOG(isDataShareReady_, "isDataShareReady_ is false, not init");

#ifdef FEATURE_AUDIO_ZONE
    CHECK_AND_RETURN_LOG(AudioVolumeUtils::GetInstance().HasZoneVolumeConfiged(std::string(PRIMARY_ZONE_NAME)),
        "Zone volume config is not ready");
#endif

    AUDIO_INFO_LOG("In");
    std::lock_guard<std::mutex> lock(deviceConnectMutex_);
    if (desc->volumeBehavior_.controlMode == PASS_THROUGH_MODE ||
        desc->volumeBehavior_.controlMode == HILINK_MODE) {
        UpdateVolumeWhenPassThroughDeviceConnect(desc);
        return;
    }
    volumeDataMaintainer_.InitDeviceVolumeMap(desc);
    volumeDataMaintainer_.InitDeviceMuteMap(desc);
    UpdateRingerMuteByRingerMode(desc);
    if (IsAbsVolumeScene() && !VolumeUtils::IsPCVolumeEnable()) {
        SaveVolumeData(desc, STREAM_VOICE_ASSISTANT,
            GetMaxVolumeLevel(STREAM_VOICE_ASSISTANT, DEVICE_TYPE_BLUETOOTH_A2DP), false, true);
        SetStreamMuteInternal(desc, STREAM_VOICE_ASSISTANT, false);
        AUDIO_INFO_LOG("a2dp ok");
    }
    UpdateSafeVolumeInner(desc);
    CHECK_AND_RETURN_LOG(isCastingConnect_ && (desc->deviceType_ == DEVICE_TYPE_DP), "update ok");
    SetMaxVolumeForDpBoardcast();
    AUDIO_INFO_LOG("update ok for dp casting");
}

void AudioAdapterManager::InitAudioZoneVolume(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "device is null");
    AUDIO_INFO_LOG("Init volume, zoneId: %{public}d, deviceType: %{public}d", zoneId, desc->deviceType_);
    volumeDataMaintainer_.InitZoneVolumeMap(zoneId, desc);
    volumeDataMaintainer_.InitZoneMuteMap(zoneId, desc);
    TriggerAudioZoneVolumeSync(zoneId, desc);
}

void AudioAdapterManager::TriggerAudioZoneVolumeSync(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(isDataShareReady_, "isDataShareReady_ is false");
    if (desc != nullptr && desc->deviceType_ != DEVICE_TYPE_NONE && desc->deviceType_ != DEVICE_TYPE_INVALID) {
        SendAudioZoneVolumeForDevice(zoneId, desc);
    } else {
        SendAudioZoneVolumeForZone(zoneId);
    }
}

void AudioAdapterManager::SendAudioZoneVolumeForDevice(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    for (auto volumeType : defaultVolumeTypeList_) {
        int32_t volumeLevel = GetSystemVolumeLevel(desc, volumeType, zoneId);
        int32_t volumeDegree = GetSystemVolumeDegree(desc, volumeType, zoneId);
        AUDIO_INFO_LOG("zoneId: %{public}d, deviceType: %{public}d, volumeType: %{public}d",
            zoneId, desc->deviceType_, volumeType);
        AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId, desc->deviceType_, volumeType,
            VolumeScale(volumeLevel, volumeDegree));
    }
}

void AudioAdapterManager::SendAudioZoneVolumeForZone(int32_t zoneId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        AudioZoneService::GetInstance().GetAllOutputDevices(zoneId);
    for (auto &desc : descs) {
        CHECK_AND_RETURN_LOG(desc != nullptr, "device is null");
        for (auto volumeType : defaultVolumeTypeList_) {
            int32_t volumeLevel = GetSystemVolumeLevel(desc, volumeType, zoneId);
            int32_t volumeDegree = GetSystemVolumeDegree(desc, volumeType, zoneId);
            AUDIO_INFO_LOG("zoneId: %{public}d, deviceType: %{public}d, volumeType: %{public}d",
                zoneId, desc->deviceType_, volumeType);
            AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId, desc->deviceType_, volumeType,
                VolumeScale(volumeLevel, volumeDegree));
        }
    }
}

int32_t AudioAdapterManager::GetSystemVolumeDegree(AudioStreamType streamType, bool checkMuteState, int32_t zoneId)
{
    auto desc = audioActiveDevice_.GetDeviceForVolume(streamType);
    if (checkMuteState && GetStreamMuteInternal(desc, streamType, zoneId)) {
        return 0;
    }

    return volumeDataMaintainer_.LoadVolumeDegreeFromMap(desc, streamType, zoneId);
}

int32_t AudioAdapterManager::GetSystemVolumeDegree(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioVolumeType volumeType, int32_t zoneId)
{
    if (GetStreamMuteInternal(desc, volumeType, zoneId)) {
        return 0;
    }
    return volumeDataMaintainer_.LoadVolumeDegreeFromMap(desc, volumeType, zoneId);
}

int32_t AudioAdapterManager::GetMinVolumeDegree(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId)
{
    int32_t minLevel = GetMinVolumeLevel(volumeType, deviceType, zoneId);
    int32_t maxLevel = GetMaxVolumeLevel(volumeType, deviceType, zoneId);
    int32_t minDegree = VolumeUtils::VolumeLevelToDegree(minLevel, maxLevel);
    return std::min(MIN_VALID_VOLUME_DEGREE, minDegree);
}

int32_t AudioAdapterManager::GetMinVolumeDegree(AudioVolumeType volumeType,
    std::shared_ptr<AudioDeviceDescriptor> desc, int32_t zoneId)
{
    int32_t minLevel = GetMinVolumeLevel(volumeType, desc, zoneId);
    int32_t maxLevel = GetMaxVolumeLevel(volumeType, desc, zoneId);
    int32_t minDegree = VolumeUtils::VolumeLevelToDegree(minLevel, maxLevel);
    return std::min(MIN_VALID_VOLUME_DEGREE, minDegree);
}

void AudioAdapterManager::SaveVolumeData(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioStreamType streamType, int32_t volumeLevel, bool updateDb, bool updateMem, int32_t zoneId)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    VolumeScale buffedVolume{volumeLevel, -1, GetMaxVolumeLevel(volumeType, desc, zoneId)};
    if (updateMem) {
        volumeDataMaintainer_.SaveVolumeToMap(desc, streamType, buffedVolume, zoneId);
    }
    if (updateDb) {
        SaveVolumeToDbAsync(desc, streamType, buffedVolume, zoneId);
    }
}

void AudioAdapterManager::UpdateVolumeWhenDeviceDisconnect(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "UptdateVolumeWhenDeviceConnect desc is null");
    if (desc->volumeBehavior_.controlMode == PASS_THROUGH_MODE ||
        desc->volumeBehavior_.controlMode == HILINK_MODE) {
        AudioServerProxy::GetInstance().UnRegistAdapterManagerCallback(desc->networkId_);
    }
    AUDIO_INFO_LOG("update ok");
}

void AudioAdapterManager::SaveVolumeToDbAsync(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioStreamType streamType, const VolumeScale &volume, int32_t zoneId)
{
    std::thread([this, desc, streamType, volume, zoneId]() {
        VolumeScale buffedVolume = {volume.VolumeLevel(), volume.VolumeDegree(),
            GetMaxVolumeLevel(streamType, desc, zoneId)};
        volumeDataMaintainer_.SaveVolumeToDb(desc, streamType, buffedVolume, zoneId);
    }).detach();
}

void AudioAdapterManager::SaveMuteToDbAsync(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioStreamType streamType, bool mute)
{
    std::thread([this, desc, streamType, mute]() {
        volumeDataMaintainer_.SaveMuteToDb(desc, streamType, mute);
    }).detach();
}

bool AudioAdapterManager::IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, false, "audioServiceAdapter is null");
    return audioServiceAdapter_->IsChannelLayoutSupportedForDspEffect(channelLayout);
}

bool AudioAdapterManager::IsStreamTypeInUse(AudioStreamType streamType,
    std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is null");
    if (VolumeUtils::GetVolumeTypeFromStreamUsage(streamDesc->rendererInfo_.streamUsage) ==
        VolumeUtils::GetVolumeTypeFromStreamType(streamType)) {
        return true;
    }
    if (streamType == STREAM_MUSIC &&
        !CheckoutSystemAppUtil::CheckoutSystemApp(audioActiveDevice_.GetRealUid(streamDesc)) &&
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_ASSISTANT) {
        return true;
    }
    return false;
}

void AudioAdapterManager::HandleCastingConnection()
{
    isCastingConnect_ = true;
}

void AudioAdapterManager::HandleCastingDisconnection()
{
    isCastingConnect_ = false;
}

bool AudioAdapterManager::IsDPCastingConnect()
{
    return isCastingConnect_;
}

void AudioAdapterManager::SetMaxVolumeForDpBoardcast()
{
    std::lock_guard<std::mutex> lock(setMaxVolumeMutex_);
    CHECK_AND_RETURN_LOG(isCastingConnect_, "casting disconnected");
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_DP);
    CHECK_AND_RETURN_LOG(desc != nullptr, "there is no dp device connected");
    bool temp = (GetMaxVolumeLevel(STREAM_MUSIC, desc) == MAX_VOLUME_LEVEL) && !VolumeUtils::IsPCVolumeEnable();
    CHECK_AND_RETURN_LOG(temp, "current dp device need not max volume");
    SaveVolumeData(desc, STREAM_MUSIC, GetMaxVolumeLevel(STREAM_MUSIC, desc), false, true);
    SaveVolumeData(desc, STREAM_VOICE_CALL, GetMaxVolumeLevel(STREAM_VOICE_CALL, desc), false, true);
    SaveVolumeData(desc, STREAM_VOICE_ASSISTANT, GetMaxVolumeLevel(STREAM_VOICE_ASSISTANT, desc), false, true);
    volumeDataMaintainer_.SaveMuteToMap(desc, STREAM_MUSIC, false);
    volumeDataMaintainer_.SaveMuteToMap(desc, STREAM_VOICE_CALL, false);
    volumeDataMaintainer_.SaveMuteToMap(desc, STREAM_VOICE_ASSISTANT, false);
}

void AudioAdapterManager::SetPrimarySinkExist(bool isPrimarySinkExist)
{
    isPrimarySinkExist_ = isPrimarySinkExist;
}

int32_t AudioAdapterManager::UpdateCollaborativeProductId(const std::string &productId)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERROR, "audioServiceAdapter is null");
    return audioServiceAdapter_->UpdateCollaborativeProductId(productId);
}

void AudioAdapterManager::LoadCollaborationConfig()
{
    CHECK_AND_RETURN_LOG(audioServiceAdapter_, "audioServiceAdapter is null");
    audioServiceAdapter_->LoadCollaborationConfig();
}

void AudioAdapterManager::UpdateRingerMuteByRingerMode(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    CHECK_AND_RETURN(!VolumeUtils::IsPCVolumeEnable());
    bool mute = (ringerMode_ == RINGER_MODE_NORMAL) ? false : true;
    volumeDataMaintainer_.SaveMuteToMap(device, STREAM_RING, mute);
    volumeDataMaintainer_.SaveMuteToDb(device, STREAM_RING, mute);
    if (mute) {
        volumeDataMaintainer_.SaveMuteToMap(device, STREAM_NOTIFICATION, mute);
        volumeDataMaintainer_.SaveMuteToDb(device, STREAM_NOTIFICATION, mute);
    }
    AUDIO_INFO_LOG("update mute: %{public}d for ring by ringermode: %{public}d", mute, ringerMode_.load());
}

void AudioAdapterManager::SetDualStreamVolumeMute(int32_t sessionId, bool isDualMute)
{
    AudioVolume::GetInstance()->SetDualStreamVolumeMute(sessionId, isDualMute);
    SendOffloadVolume();
}

void AudioAdapterManager::SetVolumeFromRemote(std::string networkId, int32_t volumeDegree)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_SPEAKER, networkId);
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is not exist");
    int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(volumeDegree, GetMaxVolumeLevel(STREAM_MUSIC, desc));
    SaveVolumeData(desc, STREAM_MUSIC, volumeLevel, false, true);
    SendVolumeKeyEventCbWithUpdateUi(STREAM_MUSIC, desc);
}

void AudioAdapterManager::SetMuteFromRemote(std::string networkId, bool mute)
{
    auto desc = audioConnectedDevice_.GetDeviceByDeviceType(DEVICE_TYPE_SPEAKER, networkId);
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is not exist");
    volumeDataMaintainer_.SaveMuteToMap(desc, STREAM_MUSIC, mute);
    SendVolumeKeyEventCbWithUpdateUi(STREAM_MUSIC, desc);
}

void AudioAdapterManager::SendVolumeKeyEventCbWithUpdateUi(AudioStreamType streamType,
    std::shared_ptr<AudioDeviceDescriptor> device)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    bool mute = GetStreamMuteInternal(device, streamType);
    volumeEvent.volume = GetStreamVolumeInternal(device, streamType) * (mute ? 0 : 1);
    volumeEvent.volumeDegree = GetStreamVolumeDegreeInternal(device, streamType) * (mute ? 0 : 1);
    volumeEvent.updateUi = false;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = device->networkId_;
    volumeEvent.deviceType = device->deviceType_;
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is null");
    audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    audioPolicyServerHandler_->SendVolumeDegreeEventCallback(volumeEvent);
}

void AudioAdapterManager::RegistAdapterManagerCallback(std::string networkId)
{
    remoteVolumeCallback_ = std::make_shared<RemoteVolumeCallback>();
    sptr<AudioManagerListenerStubImpl> parameterChangeCbStub = new(std::nothrow) AudioManagerListenerStubImpl();
    CHECK_AND_RETURN_LOG(parameterChangeCbStub != nullptr, "parameterChangeCbStub is nullptr");
    parameterChangeCbStub->SetParameterCallback(remoteVolumeCallback_);

    sptr<IRemoteObject> object = parameterChangeCbStub->AsObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "parameterChangeCbStub object is nullptr");
    AudioServerProxy::GetInstance().RegistAdapterManagerCallback(object, networkId);
}

void AudioAdapterManager::RemoteVolumeCallback::OnAudioParameterChange(const std::string networkId,
    const AudioParamKey key, const std::string& condition, const std::string& value)
{
    AUDIO_INFO_LOG("OnAudioParameterChange key: %{public}d, condition: %{public}s, value: %{public}s",
        key, condition.c_str(), value.c_str());
    switch (key) {
        case AudioParamKey::VOLUME:
            if (condition.find("VOLUME_CHANAGE") == 0) {
                int32_t volumeDegree = FormatUtils::StringToInt32(value, 0);
                AudioAdapterManager::GetInstance().SetVolumeFromRemote(networkId, volumeDegree);
                return;
            }
            if (condition.find("MUTE_CHANGE") == 0) {
                int32_t mute = FormatUtils::StringToInt32(value, 0);
                AudioAdapterManager::GetInstance().SetMuteFromRemote(networkId, mute);
                return;
            }
            break;
        default:
            break;
    }
}

void AudioAdapterManager::SetRemoteVolumeForPassThroughDevice(std::shared_ptr<AudioDeviceDescriptor> device,
    int32_t volumeLevel)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    CHECK_AND_RETURN(device->volumeBehavior_.controlMode == PASS_THROUGH_MODE ||
        device->volumeBehavior_.controlMode == HILINK_MODE);

    int32_t maxRet = GetMaxVolumeLevel(STREAM_MUSIC, device);
    int32_t volumeDegree = VolumeUtils::VolumeLevelToDegree(volumeLevel, maxRet);
    AudioServerProxy::GetInstance().SetRemoteAudioParameterProxy(device->networkId_, true, volumeDegree);
    AUDIO_INFO_LOG("set remote volume %{public}d", volumeDegree);
}

void AudioAdapterManager::UpdateVolumeWhenPassThroughDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    CHECK_AND_RETURN(device->volumeBehavior_.controlMode == PASS_THROUGH_MODE ||
        device->volumeBehavior_.controlMode == HILINK_MODE);

    int32_t maxRet = GetMaxVolumeLevel(STREAM_MUSIC, device);
    int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(device->volumeBehavior_.controlInitVolume, maxRet);
    SaveVolumeData(device, STREAM_MUSIC, volumeLevel, false, true);
    volumeDataMaintainer_.SaveMuteToMap(device, STREAM_MUSIC, device->volumeBehavior_.controlInitMute);
    AUDIO_INFO_LOG("init volume %{public}d, mute: %{public}d by remote control",
        device->volumeBehavior_.controlInitVolume, device->volumeBehavior_.controlInitMute);
    SendOffloadVolume(device->networkId_);
    RegistAdapterManagerCallback(device->networkId_);
}

int32_t AudioAdapterManager::RestoreSystemAppVolumeLevel()
{
    std::unordered_map<std::string, int32_t> systemAppVolumeLevelMap = {};
    int32_t ret = volumeDataMaintainer_.LoadSystemAppVolumeLevelFromDb(systemAppVolumeLevelMap);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "load system app volume level fail");
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t userId = settingProvider.GetCurrentUserId();
    for (const auto &[bundleName, level] : systemAppVolumeLevelMap) {
        bool isInstall = true;
        AudioBundleManager::IsBundleInstalled(bundleName, userId, 0, isInstall);
        if (!isInstall) continue;
        int32_t uid = AudioBundleManager::GetUidByBundleName(bundleName, userId);
        CHECK_AND_CONTINUE_LOG(uid >= SUCCESS, "GetUidByBundleName failed");
        SetSystemAppVolumeLevel(uid, level);
    }
    volumeDataMaintainer_.SaveSystemAppVolumeLevelToDb();
    return SUCCESS;
}

int32_t AudioAdapterManager::RestoreSystemAppVolumeMuteStatus()
{
    std::unordered_map<std::string, bool> systemAppVolumeMuteStatusMap;
    int32_t ret = volumeDataMaintainer_.LoadSystemAppVolumeMuteStatusFromDb(systemAppVolumeMuteStatusMap);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "load system app volume mute status fail");
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t userId = settingProvider.GetCurrentUserId();
    for (const auto &[bundleName, muteStatus] : systemAppVolumeMuteStatusMap) {
        bool isInstall = true;
        AudioBundleManager::IsBundleInstalled(bundleName, userId, 0, isInstall);
        if (!isInstall) continue;
        int32_t uid = AudioBundleManager::GetUidByBundleName(bundleName, userId);
        CHECK_AND_CONTINUE_LOG(uid >= SUCCESS, "GetUidByBundleName failed");
        SetSystemAppVolumeMuted(uid, muteStatus);
    }
    volumeDataMaintainer_.SaveSystemAppVolumeMuteStatusToDb();
    return SUCCESS;
}

int32_t AudioAdapterManager::SetAudioZoneVolumeConfig(const std::string &zoneName,
    const std::vector<AudioZoneVolumeConfig> &configs)
{
    int ret = AudioVolumeUtils::GetInstance().SetAudioZoneVolumeConfig(zoneName, configs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioZoneVolumeConfig failed");
    if (zoneName == PRIMARY_ZONE_NAME) {
        AUDIO_INFO_LOG("Primary zone volume config applied");
        auto descs = audioConnectedDevice_.GetCopy();
        for (auto &desc : descs) {
            UpdateVolumeWhenDeviceConnect(desc);
        }
    }
    return ret;
}

void AudioAdapterManager::RemoveAudioPipeVolume(uint32_t IoHandle)
{
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");
    for (auto volumeType : defaultVolumeTypeList_) {
        audioVolume->RemovePipeVolume(IoHandle, volumeType);
    }
}

void AudioAdapterManager::UpdateAudioPipeVolume(std::vector<PipeDeviceVolumeInfo> pipeDeviceVolumeInfo)
{
    for (auto &info : pipeDeviceVolumeInfo) {
        for (auto volumeType : info.volumeTypes_) {
            UpdateTypeVolumeForPipe(info.ioHandle_, volumeType, info.deviceDesc_);
        }
    }
}

void AudioAdapterManager::UpdateTypeVolumeForAllPipes(AudioVolumeType volumeType)
{
    auto pipeDeviceVolumeInfo = AudioPipeManager::GetPipeManager()->GetAllPipeDeviceVolumeInfo();
    for (auto &info : pipeDeviceVolumeInfo) {
        CHECK_AND_CONTINUE(info.volumeTypes_.count(volumeType) > 0);
        UpdateTypeVolumeForPipe(info.ioHandle_, volumeType, info.deviceDesc_);
    }
}

void AudioAdapterManager::UpdateVolumeForAllPipes()
{
    auto pipeDeviceVolumeInfo = AudioPipeManager::GetPipeManager()->GetAllPipeDeviceVolumeInfo();
    for (auto &info : pipeDeviceVolumeInfo) {
        for (auto volumeType : info.volumeTypes_) {
            UpdateTypeVolumeForPipe(info.ioHandle_, volumeType, info.deviceDesc_);
        }
    }
}

void AudioAdapterManager::UpdatePipeVolumeWhenVgsStatusChanged()
{
    auto pipeDeviceVolumeInfo = AudioPipeManager::GetPipeManager()->GetAllPipeDeviceVolumeInfo();
    for (auto &info : pipeDeviceVolumeInfo) {
        std::shared_ptr<AudioDeviceDescriptor> device = info.deviceDesc_;
        CHECK_AND_CONTINUE(device->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO);
        UpdateTypeVolumeForPipe(info.ioHandle_, STREAM_VOICE_CALL, device);
    }
}

int32_t AudioAdapterManager::SetVolumeDbForDeviceInPipe(std::shared_ptr<AudioDeviceDescriptor> desc,
    AudioStreamType volumeType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERROR, "desc is null");
    int32_t volumeLevel =
        GetStreamVolumeInternal(desc, volumeType) * (GetStreamMuteInternal(desc, volumeType) ? 0 : 1);
    SaveNotificationVolumeToLocal(desc, volumeType, volumeLevel);
    if (desc->deviceType_ == DEVICE_TYPE_HEARING_AID) {
        AudioIOHandle ioHandle;
        std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_HEARING_AID);
        bool result = AudioIOHandleMap::GetInstance().GetModuleIdByKey(sinkName, ioHandle);
        CHECK_AND_RETURN_RET_LOG(result, ERROR, "GetModuleIdByKey failed for hearing aid");
        UpdateTypeVolumeForPipe(ioHandle, volumeType, desc);
        return SUCCESS;
    }
    auto pipeDeviceVolumeInfo = AudioPipeManager::GetPipeManager()->GetPipeDeviceVolumeInfoForDevice(desc);
    AUDIO_INFO_LOG("pipe size: %{public}zu", pipeDeviceVolumeInfo.size());
    for (auto &info : pipeDeviceVolumeInfo) {
        AUDIO_INFO_LOG("device: %{public}d, networkId: %{public}s, pipeId: %{public}d",
            desc->deviceType_, desc->networkId_.c_str(), info.ioHandle_);
        UpdateTypeVolumeForPipe(info.ioHandle_, volumeType, info.deviceDesc_);
    }
    return SUCCESS;
}

void AudioAdapterManager::UpdateTypeVolumeForPipe(uint32_t pipeId, AudioVolumeType volumeType,
    std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    std::lock_guard<std::mutex> lock1(activeDeviceMutex_);
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    AudioVolume::GetInstance()->SetCurrentActiveDevice(audioActiveDevice_.GetDeviceForVolume()->deviceType_);
    CHECK_AND_RETURN_LOG(device->volumeBehavior_.controlMode != PASS_THROUGH_MODE,
        "device is in pass-through mode");
    CHECK_AND_RETURN_LOG(device->volumeBehavior_.controlMode != HILINK_MODE, "device is in hi-link mode");

    bool isMuted = GetStreamMuteForVolumeSet(device, volumeType);
    int32_t volumeLevel = GetStreamVolumeInternal(device, volumeType) * (isMuted ? 0 : 1);
    SaveSystemVolumeForSwitchDevice(device, volumeType, volumeLevel);

    bool useSpeaker = Util::IsDualToneStreamType(volumeType);
    DeviceType deviceType = useSpeaker ? DEVICE_TYPE_SPEAKER : device->deviceType_;
    int32_t volumeDegree = GetStreamVolumeDegreeInternal(device, volumeType) * (isMuted ? 0 : 1);

    float volumeDb = volumeAdjustZoneId_ == 0 ? CalculateVolumeDbByDegree(deviceType, volumeType, volumeDegree) :
        CalculateVolumeDbNonlinear(volumeType, deviceType, volumeLevel);

    AjustVolumeForSpecialType(volumeType, volumeDb);
    DepressVolume(volumeDb, volumeLevel, volumeType, device);
    SetSystemVolumeToEffect(device, volumeType);

    AdjustVolumeForSpecialDevice(device, volumeType, isMuted, volumeLevel, volumeDb);
#ifdef FEATURE_AUDIO_ZONE
    ApplySystemVolumeProxy(zoneId, device, isMuted, volumeDb);
#endif
    std::lock_guard<std::mutex> lock2(audioVolumeMutex_);
    auto audioVolume = AudioVolume::GetInstance();
    CHECK_AND_RETURN_LOG(audioVolume != nullptr, "audioVolume handle null");

    PipeVolume pipeVolume(pipeId, volumeType, volumeDb, volumeLevel, isMuted);
    audioVolume->SetPipeVolume(pipeVolume);

    AUDIO_INFO_LOG("pipeId:%{public}d volumeType:%{public}d volumeDb:%{public}f volumeLevel:%{public}d \
        volumeDegree:%{public}d device:%{public}s",
        pipeId, volumeType, volumeDb, volumeLevel, volumeDegree, device->GetName().c_str());

    SendOffloadVolume(device->networkId_);
}

void AudioAdapterManager::AdjustVolumeForSpecialDevice(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioVolumeType volumeType, bool &isMuted, int32_t &volumeLevel, float &volumeDb)
{
    isMuted = VolumeUtils::IsVolumeFixEnable() ? false : isMuted;
    if (device->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && IsAbsVolumeScene() && volumeType == STREAM_MUSIC) {
        isMuted = IsAbsVolumeMute();
        volumeLevel = isMuted ? 0 : volumeLevel;
        volumeDb = isMuted ? 0.0f : 0.63957f; // 0.63957 = -4dB
        return;
    } else if (device->deviceType_ == DEVICE_TYPE_NEARLINK) {
        if (volumeType == STREAM_MUSIC && !isSleVoiceStatus_.load()) {
            isMuted = isAbsVolumeMuteNearlink_.load();
            volumeDb = isMuted ? 0.0f : 0.63957f;
        } else if (volumeType == STREAM_VOICE_CALL) {
            volumeDb = 1.0f;
        }
        return;
    } else if (device->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && IsVgsVolumeSupported() &&
        volumeType == STREAM_VOICE_CALL) {
        volumeDb = 1.0f;
    }
}

void AudioAdapterManager::AjustVolumeForSpecialType(AudioStreamType streamType, float &volumeDb)
{
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        volumeDb = 1.0f;
    }
}

void AudioAdapterManager::ApplySystemVolumeProxy(int32_t zoneId,
    std::shared_ptr<AudioDeviceDescriptor> device, bool &isMuted, float &volumeDb)
{
    if (AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(zoneId, device->deviceType_)) {
        isMuted = false;
        volumeDb = 1.0f;
    }
}

int32_t AudioAdapterManager::UpdatePersonalizedHRTFBin(const int32_t &fd, const long &length)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_, ERROR, "ServiceAdapter is null");
    return audioServiceAdapter_->UpdatePersonalizedHRTFBin(fd, length);
}
} // namespace AudioStandard
} // namespace OHOS
