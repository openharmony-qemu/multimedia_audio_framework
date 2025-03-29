/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_volume_parser.h"
#include "audio_utils.h"
#include "audio_volume.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static const std::vector<AudioStreamType> VOLUME_TYPE_LIST = {
    // all volume types except STREAM_ALL
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_VOICE_CALL_ASSISTANT
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

// LCOV_EXCL_START
bool AudioAdapterManager::Init()
{
    char testMode[10] = {0}; // 10 for system parameter usage
    auto res = GetParameter("debug.audio_service.testmodeon", "0", testMode, sizeof(testMode));
    if (res == 1 && testMode[0] == '1') {
        AUDIO_DEBUG_LOG("testMode on");
        testModeOn_ = true;
    }

    std::unique_ptr<AudioVolumeParser> audiovolumeParser = make_unique<AudioVolumeParser>();
    if (!audiovolumeParser->LoadConfig(streamVolumeInfos_)) {
        AUDIO_INFO_LOG("Audio Volume Config Load Configuration successfully");
        useNonlinearAlgo_ = 1;
        UpdateVolumeMapIndex();
    }

    // init volume before kvstore start by local prop for bootanimation
    char currentVolumeValue[3] = {0};
    auto ret = GetParameter("persist.multimedia.audio.ringtonevolume", "7",
        currentVolumeValue, sizeof(currentVolumeValue));
    if (ret > 0) {
        int32_t ringtoneVolumeLevel = atoi(currentVolumeValue);
        volumeDataMaintainer_.SetStreamVolume(STREAM_RING, ringtoneVolumeLevel);
        AUDIO_INFO_LOG("Init: Get ringtone volume to map success %{public}d",
            volumeDataMaintainer_.GetStreamVolume(STREAM_RING));
    } else {
        AUDIO_ERR_LOG("Init: Get volume parameter failed %{public}d", ret);
    }

    std::string defaultSafeVolume = std::to_string(GetMaxVolumeLevel(STREAM_MUSIC));
    AUDIO_INFO_LOG("defaultSafeVolume %{public}s", defaultSafeVolume.c_str());
    char currentSafeVolumeValue[3] = {0};
    ret = GetParameter("const.audio.safe_media_volume", defaultSafeVolume.c_str(),
        currentSafeVolumeValue, sizeof(currentSafeVolumeValue));
    if (ret > 0) {
        safeVolume_ = atoi(currentSafeVolumeValue);
        AUDIO_INFO_LOG("Get currentSafeVolumeValue success %{public}d", safeVolume_);
    } else {
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

    handler_ = std::make_shared<AudioAdapterManagerHandler>();

    isVolumeUnadjustable_ = system::GetBoolParameter("const.multimedia.audio.fixedvolume", false);
    AUDIO_INFO_LOG("Get fixdvolume parameter success %{public}d", isVolumeUnadjustable_);

    return true;
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
    auto iter = VOLUME_TYPE_LIST.begin();
    while (iter != VOLUME_TYPE_LIST.end()) {
        SetVolumeDb(*iter);
        iter++;
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
int32_t AudioAdapterManager::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    return maxVolumeIndexMap_[volumeType];
}

int32_t AudioAdapterManager::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    CHECK_AND_RETURN_RET_LOG(volumeType >= STREAM_VOICE_CALL && volumeType <= STREAM_TYPE_MAX,
        ERR_INVALID_PARAM, "Invalid stream type");
    return minVolumeIndexMap_[volumeType];
}

void AudioAdapterManager::SaveRingtoneVolumeToLocal(AudioVolumeType volumeType, int32_t volumeLevel)
{
    if (volumeType == STREAM_RING) {
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

int32_t AudioAdapterManager::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel)
{
    if (GetSystemVolumeLevel(streamType) == volumeLevel && currentActiveDevice_ != DEVICE_TYPE_BLUETOOTH_SCO &&
        currentActiveDevice_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("The volume is the same as before.");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("SetSystemVolumeLevel: streamType: %{public}d, deviceType: %{public}d, volumeLevel:%{public}d",
        streamType, currentActiveDevice_, volumeLevel);
    if (volumeLevel == 0 &&
        (streamType == STREAM_VOICE_ASSISTANT || streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("SetSystemVolumeLevel this type can not set mute");
        return SUCCESS;
    }
    int32_t mimRet = GetMinVolumeLevel(streamType);
    int32_t maxRet = GetMaxVolumeLevel(streamType);
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= mimRet && volumeLevel <= maxRet, ERR_OPERATION_FAILED,
        "volumeLevel not in scope.");

    // In case if KvStore didnot connect during bootup
    if (!isLoaded_) {
        InitKVStoreInternal();
    }

    if (currentActiveDevice_ == DEVICE_TYPE_BLUETOOTH_SCO || currentActiveDevice_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        if (isBtFirstSetVolume_ && volumeLevel > safeVolume_) {
            volumeLevel = safeVolume_;
            isBtFirstSetVolume_ = false;
        }
    }

    volumeDataMaintainer_.SetStreamVolume(streamType, volumeLevel);

    if (handler_ != nullptr) {
        if (Util::IsDualToneStreamType(streamType)) {
            AUDIO_INFO_LOG("DualToneStreamType. Save volume for speaker.");
            handler_->SendSaveVolume(DEVICE_TYPE_SPEAKER, streamType, volumeLevel);
        } else {
            handler_->SendSaveVolume(currentActiveDevice_, streamType, volumeLevel);
        }
    }

    return SetVolumeDb(streamType);
}

void AudioAdapterManager::HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel)
{
    volumeDataMaintainer_.SaveVolume(deviceType, streamType, volumeLevel);
}

void AudioAdapterManager::HandleStreamMuteStatus(AudioStreamType streamType, bool mute, StreamUsage streamUsage)
{
    volumeDataMaintainer_.SaveMuteStatus(currentActiveDevice_, streamType, mute);
}

void AudioAdapterManager::HandleRingerMode(AudioRingerMode ringerMode)
{
    // In case if KvStore didnot connect during bootup
    if (!isLoaded_) {
        InitKVStoreInternal();
    }

    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(STREAM_RING);
    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(STREAM_RING) * ((ringerMode != RINGER_MODE_NORMAL) ? 0 : 1);

    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(streamForVolumeMap, volumeLevel);

    volumeDataMaintainer_.SaveRingerMode(ringerMode);
}

void AudioAdapterManager::SetAudioServerProxy(sptr<IStandardAudioService> gsp)
{
    CHECK_AND_RETURN_LOG(gsp != nullptr, "audioServerProxy null");
    audioServerProxy_ = gsp;
}

int32_t AudioAdapterManager::SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel)
{
    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        if (Util::IsDualToneStreamType(streamType)) {
            volumeDb = CalculateVolumeDbNonlinear(streamType, DEVICE_TYPE_SPEAKER, volumeLevel);
        } else {
            volumeDb = CalculateVolumeDbNonlinear(streamType, currentActiveDevice_, volumeLevel);
        }
    } else {
        volumeDb = CalculateVolumeDb(volumeLevel);
    }
    SetAudioVolume(streamType, volumeDb);

    return SUCCESS;
}

int32_t AudioAdapterManager::SetVolumeDb(AudioStreamType streamType)
{
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(streamType) * (GetStreamMute(streamType) ? 0 : 1);

    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(streamForVolumeMap, volumeLevel);

    float volumeDb = 1.0f;
    if (useNonlinearAlgo_) {
        if (Util::IsDualToneStreamType(streamType)) {
            volumeDb = CalculateVolumeDbNonlinear(streamType, DEVICE_TYPE_SPEAKER, volumeLevel);
        } else {
            volumeDb = CalculateVolumeDbNonlinear(streamType, currentActiveDevice_, volumeLevel);
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

    AUDIO_INFO_LOG("streamType:%{public}d volumeDb:%{public}f volume:%{public}d", streamType, volumeDb, volumeLevel);

    // audio volume
    SetAudioVolume(streamType, volumeDb);

    return SUCCESS;
}

void AudioAdapterManager::SetAudioVolume(AudioStreamType streamType, float volumeDb)
{
    static std::unordered_map<DeviceType, std::vector<std::string>> deviceClassMap = {
        {DEVICE_TYPE_SPEAKER, {PRIMARY_CLASS, MCH_CLASS, REMOTE_CLASS, OFFLOAD_CLASS}},
        {DEVICE_TYPE_USB_HEADSET, {PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
        {DEVICE_TYPE_BLUETOOTH_A2DP, {A2DP_CLASS, PRIMARY_CLASS, MCH_CLASS, OFFLOAD_CLASS}},
        {DEVICE_TYPE_BLUETOOTH_SCO, {PRIMARY_CLASS, MCH_CLASS}},
        {DEVICE_TYPE_EARPIECE, {PRIMARY_CLASS, MCH_CLASS}},
        {DEVICE_TYPE_WIRED_HEADSET, {PRIMARY_CLASS, MCH_CLASS}},
        {DEVICE_TYPE_WIRED_HEADPHONES, {PRIMARY_CLASS, MCH_CLASS}},
        {DEVICE_TYPE_USB_ARM_HEADSET, {USB_CLASS}},
        {DEVICE_TYPE_REMOTE_CAST, {REMOTE_CAST_INNER_CAPTURER_SINK_NAME}},
        {DEVICE_TYPE_DP, {DP_CLASS}},
        {DEVICE_TYPE_FILE_SINK, {FILE_CLASS}},
        {DEVICE_TYPE_FILE_SOURCE, {FILE_CLASS}},
    };

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
    auto it = deviceClassMap.find(GetActiveDevice());
    if (it == deviceClassMap.end()) {
        AUDIO_ERR_LOG("unkown device type %{public}d", GetActiveDevice());
        return;
    }
    for (auto &deviceClass : it->second) {
        SystemVolume systemVolume(volumeType, deviceClass, volumeDb, volumeLevel, isMuted);
        if (deviceClass != OFFLOAD_CLASS) {
            audioVolume->SetSystemVolume(systemVolume);
        } else if (deviceClass == OFFLOAD_CLASS && volumeType == STREAM_MUSIC) {
            audioVolume->SetSystemVolume(systemVolume);
            SetOffloadVolume(volumeType, volumeDb);
        }
    }
}

void AudioAdapterManager::SetOffloadVolume(AudioStreamType streamType, float volumeDb)
{
    float volume = volumeDb; // maybe only system volume
    if (!(streamType == STREAM_MUSIC || streamType == STREAM_SPEECH)) {
        return;
    }
    DeviceType dev = GetActiveDevice();
    if (!(dev == DEVICE_TYPE_SPEAKER || dev == DEVICE_TYPE_BLUETOOTH_A2DP || dev == DEVICE_TYPE_USB_HEADSET)) {
        return;
    }
    CHECK_AND_RETURN_LOG(audioServerProxy_ != nullptr, "audioServerProxy_ null");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    if (offloadSessionID_.has_value()) { // need stream volume and system volume
        struct VolumeValues volumes = {0.0f, 0.0f, 0.0f};
        volume = AudioVolume::GetInstance()->GetVolume(offloadSessionID_.value(), streamType, OFFLOAD_CLASS, &volumes);
        audioServerProxy_->OffloadSetVolume(volume);
    }
    IPCSkeleton::SetCallingIdentity(identity);
}

void AudioAdapterManager::SetOffloadSessionId(uint32_t sessionId)
{
    if (sessionId < MIN_SESSIONID || sessionId > MAX_SESSIONID) {
        AUDIO_PRERELEASE_LOGE("set sessionId[%{public}d] error", sessionId);
    } else {
        AUDIO_PRERELEASE_LOGI("set sessionId[%{public}d]", sessionId);
    }
    offloadSessionID_ = sessionId;
}

void AudioAdapterManager::ResetOffloadSessionId()
{
    AUDIO_PRERELEASE_LOGI("reset offload sessionId[%{public}d]", offloadSessionID_.value());
    offloadSessionID_.reset();
}

int32_t AudioAdapterManager::GetSystemVolumeLevel(AudioStreamType streamType)
{
    if (GetStreamMuteInternal(streamType)) {
        return MIN_VOLUME_LEVEL;
    }

    return volumeDataMaintainer_.GetStreamVolume(streamType);
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

int32_t AudioAdapterManager::SetStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage)
{
    return SetStreamMuteInternal(streamType, mute, streamUsage);
}

int32_t AudioAdapterManager::SetStreamMuteInternal(AudioStreamType streamType, bool mute,
    StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("stream type %{public}d, mute:%{public}d, streamUsage:%{public}d", streamType, mute, streamUsage);
    if (mute &&
        (streamType == STREAM_VOICE_ASSISTANT || streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY ||
        streamType == STREAM_VOICE_COMMUNICATION)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("SetStreamMute: this type can not set mute");
        return SUCCESS;
    }
    if (Util::IsDualToneStreamType(streamType) && currentActiveDevice_ != DEVICE_TYPE_SPEAKER &&
        GetRingerMode() != RINGER_MODE_NORMAL && mute && Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        AUDIO_INFO_LOG("Dual tone stream type %{public}d, current active device:[%{public}d] is no speaker, dont mute",
            streamType, currentActiveDevice_);
        return SUCCESS;
    }

    // set stream mute status to mem.
    volumeDataMaintainer_.SetStreamMuteStatus(streamType, mute);

    if (handler_ != nullptr) {
        handler_->SendStreamMuteStatusUpdate(streamType, mute, streamUsage);
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

int32_t AudioAdapterManager::GetPersistMicMuteState(bool &isMute) const
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

int32_t AudioAdapterManager::GetStreamVolume(AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetStreamVolume(streamType);
}

bool AudioAdapterManager::GetStreamMuteInternal(AudioStreamType streamType)
{
    return volumeDataMaintainer_.GetStreamMute(streamType);
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

vector<SinkInput> AudioAdapterManager::GetAllSinkInputs()
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetAllSinkInputs audio adapter null");
        vector<SinkInput> sinkInputList;
        return sinkInputList;
    }

    return audioServiceAdapter_->GetAllSinkInputs();
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
    for (auto &volumeType : VOLUME_TYPE_LIST) {
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

void AudioAdapterManager::SetVolumeForSwitchDevice(InternalDeviceType deviceType)
{
    if (!isLoaded_) {
        AUDIO_ERR_LOG("The data base is not loaded. Can not load new volume for new device!");
        // The ring volume is also saved in audio_config.para.
        // So the boot animation can still play with right volume.
        return;
    }

    std::lock_guard<std::mutex> lock(activeDeviceMutex_);
    // The same device does not set the volume
    bool isSameVolumeGroup = GetVolumeGroupForDevice(currentActiveDevice_) == GetVolumeGroupForDevice(deviceType);
    if (currentActiveDevice_ == deviceType) {
        AUDIO_INFO_LOG("Old device: %{public}d. New device: %{public}d. No need to update volume",
            currentActiveDevice_, deviceType);
        return;
    }

    AUDIO_INFO_LOG("SetVolumeForSwitchDevice: Load volume and mute status for new device %{public}d,"
        "same volume group %{public}d", deviceType, isSameVolumeGroup);
    // Current device must be updated even if kvStore is nullptr.
    currentActiveDevice_ = deviceType;

    if (!isSameVolumeGroup) {
        LoadVolumeMap();
        LoadMuteStatusMap();
        UpdateSafeVolume();
    }

    auto iter = VOLUME_TYPE_LIST.begin();
    while (iter != VOLUME_TYPE_LIST.end()) {
        // update volume level and mute status for each stream type
        SetVolumeDb(*iter);
        AUDIO_INFO_LOG("SetVolumeForSwitchDevice: volume: %{public}d, mute: %{public}d for stream type %{public}d",
            volumeDataMaintainer_.GetStreamVolume(*iter), volumeDataMaintainer_.GetStreamMute(*iter), *iter);
        iter++;
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

// LCOV_EXCL_START
AudioIOHandle AudioAdapterManager::OpenAudioPort(const AudioModuleInfo &audioModuleInfo)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);

    AUDIO_INFO_LOG("[Adapter load-module] %{public}s %{public}s",
        audioModuleInfo.lib.c_str(), audioModuleInfo.className.c_str());

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    curActiveCount_++;
    AudioIOHandle ioHandle = audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, moduleArgs.c_str());
    AUDIO_INFO_LOG("Open %{public}d port end.", static_cast<int32_t>(ioHandle));
    return ioHandle;
}

int32_t AudioAdapterManager::CloseAudioPort(AudioIOHandle ioHandle)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    curActiveCount_--;
    int32_t ret = audioServiceAdapter_->CloseAudioPort(ioHandle);
    AUDIO_INFO_LOG("Close %{public}d port end.", static_cast<int32_t>(ioHandle));
    return ret;
}

int32_t AudioAdapterManager::GetCurActivateCount() const
{
    return curActiveCount_ > 0 ? curActiveCount_ : 0;
}

void UpdateSinkArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
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
            return SPEAKER_VOLUME_TYPE;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_DP:
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

    // open and initialize kvstore instance.
    if (audioPolicyKvStore_ == nullptr) {
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
        volumeDataMaintainer_.SetFirstBoot(false);
        return true;
    }
    // first boot
    if (!volumeDataMaintainer_.GetFirstBoot(isFirstBoot)) {
        AUDIO_INFO_LOG("first boot, ready init data to database");
        isFirstBoot = true;
        volumeDataMaintainer_.SetFirstBoot(false);
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

void AudioAdapterManager::UpdateSafeVolume()
{
    if (volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC) <= safeVolume_) {
        return;
    }
    AUDIO_INFO_LOG("update current volume to safevolume, device:%{public}d", currentActiveDevice_);
    switch (currentActiveDevice_) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (isWiredBoot_) {
                volumeDataMaintainer_.SetStreamVolume(STREAM_MUSIC, safeVolume_);
                volumeDataMaintainer_.SaveVolume(currentActiveDevice_, STREAM_MUSIC, safeVolume_);
                isWiredBoot_ = false;
            }
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (isBtBoot_) {
                volumeDataMaintainer_.SetStreamVolume(STREAM_MUSIC, safeVolume_);
                volumeDataMaintainer_.SaveVolume(currentActiveDevice_, STREAM_MUSIC, safeVolume_);
                isBtBoot_ = false;
            }
            break;
        default:
            AUDIO_ERR_LOG("current device: %{public}d is not support", currentActiveDevice_);
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
    AUDIO_INFO_LOG("InitVolumeMap: Wrote default stream volumes to KvStore");
    std::unordered_map<AudioStreamType, int32_t> volumeLevelMapTemp = volumeDataMaintainer_.GetVolumeMap();
    for (auto &deviceType: VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType: VOLUME_TYPE_LIST) {
            // if GetVolume failed, wirte default value
            if (!volumeDataMaintainer_.GetVolume(deviceType, streamType)) {
                volumeDataMaintainer_.SaveVolume(deviceType, streamType, volumeLevelMapTemp[streamType]);
            }
        }
    }
    // reLoad the current device volume
    LoadVolumeMap();
    UpdateSafeVolume();
}

void AudioAdapterManager::ResetRemoteCastDeviceVolume()
{
    for (auto &streamType: VOLUME_TYPE_LIST) {
        AudioStreamType streamAlias = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
        int32_t volumeLevel = GetMaxVolumeLevel(streamAlias);
        volumeDataMaintainer_.SaveVolume(DEVICE_TYPE_REMOTE_CAST, streamType, volumeLevel);
    }
}

void AudioAdapterManager::InitRingerMode(bool isFirstBoot)
{
    if (isFirstBoot) {
        ringerMode_ = RINGER_MODE_NORMAL;
        isLoaded_ = true;
        if (!volumeDataMaintainer_.GetRingerMode(ringerMode_)) {
            isLoaded_ = volumeDataMaintainer_.SaveRingerMode(RINGER_MODE_NORMAL);
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
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(STREAM_RING);
    int32_t volumeLevel =
        volumeDataMaintainer_.GetStreamVolume(STREAM_RING) * ((ringerMode_ != RINGER_MODE_NORMAL) ? 0 : 1);
    // Save volume in local prop for bootanimation
    SaveRingtoneVolumeToLocal(streamForVolumeMap, volumeLevel);
}

void AudioAdapterManager::CloneVolumeMap(void)
{
    CHECK_AND_RETURN_LOG(audioPolicyKvStore_ != nullptr, "clone volumemap failed, audioPolicyKvStore_nullptr");
    // read volume from private Kvstore
    AUDIO_INFO_LOG("Copy Volume from private database to shareDatabase");
    for (auto &deviceType : VOLUME_GROUP_TYPE_LIST) {
        for (auto &streamType : VOLUME_TYPE_LIST) {
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

bool AudioAdapterManager::LoadVolumeMap(void)
{
    if (isNeedCopyVolumeData_ && (audioPolicyKvStore_ != nullptr)) {
        CloneVolumeMap();
    }

    bool result = false;
    for (auto &streamType: VOLUME_TYPE_LIST) {
        if (Util::IsDualToneStreamType(streamType)) {
            result = volumeDataMaintainer_.GetVolume(DEVICE_TYPE_SPEAKER, streamType);
        } else {
            result = volumeDataMaintainer_.GetVolume(currentActiveDevice_, streamType);
        }
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
            for (auto &streamType : VOLUME_TYPE_LIST) {
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
        if (currentActiveDevice_ == deviceType) {
            volumeDataMaintainer_.SetStreamMuteStatus(streamType, muteStateForStreamRing);
        }
        volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, muteStateForStreamRing);
    } else if (!volumeDataMaintainer_.GetMuteStatus(deviceType, streamType)) {
        if (currentActiveDevice_ == deviceType) {
            volumeDataMaintainer_.SetStreamMuteStatus(streamType, false);
        }
        volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, false);
    }
    if (currentActiveDevice_ == deviceType) {
        SetVolumeDb(streamType);
    }
}

void AudioAdapterManager::SetVolumeCallbackAfterClone()
{
    for (auto &streamType : VOLUME_TYPE_LIST) {
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
        for (auto &streamType : VOLUME_TYPE_LIST) {
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
            if (currentActiveDevice_ == deviceType) {
                volumeDataMaintainer_.SetStreamMuteStatus(streamType, muteStatus);
            }
            volumeDataMaintainer_.SaveMuteStatus(deviceType, streamType, muteStatus);
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

    for (auto &streamType: VOLUME_TYPE_LIST) {
        bool result = volumeDataMaintainer_.GetMuteStatus(currentActiveDevice_, streamType);
        if (!result) {
            AUDIO_WARNING_LOG("Could not load mute status for stream type %{public}d from database.", streamType);
        }
        if (streamType == STREAM_RING) {
            bool muteStateForStreamRing = (ringerMode_ == RINGER_MODE_NORMAL) ? false : true;
            if (currentActiveDevice_ != DEVICE_TYPE_SPEAKER) {
                continue;
            }
            AUDIO_INFO_LOG("ringer mode:%{public}d, stream ring mute state:%{public}d", ringerMode_,
                muteStateForStreamRing);
            if (muteStateForStreamRing == GetStreamMute(streamType)) {
                continue;
            }
            volumeDataMaintainer_.SaveMuteStatus(currentActiveDevice_, streamType, muteStateForStreamRing);
            SetStreamMute(streamType, muteStateForStreamRing);
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

std::string AudioAdapterManager::GetMuteKeyForKvStore(DeviceType deviceType, AudioStreamType streamType)
{
    std::string type = "";
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
            type = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            type = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_DP:
            type = "wired";
            break;
        default:
            AUDIO_ERR_LOG("GetMuteKeyForKvStore: device %{public}d is not supported for kvStore", deviceType);
            return "";
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

float AudioAdapterManager::CalculateVolumeDb(int32_t volumeLevel)
{
    float value = static_cast<float>(volumeLevel) / MAX_VOLUME_LEVEL;
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
    AUDIO_DEBUG_LOG("GetSystemVolumeInDb for volumeType: %{public}d deviceType:%{public}d volumeLevel:%{public}d",
        volumeType, deviceType, volumeLevel);
    if (useNonlinearAlgo_) {
        getSystemVolumeInDb_ = CalculateVolumeDbNonlinear(volumeType, deviceType, volumeLevel);
    } else {
        getSystemVolumeInDb_ = CalculateVolumeDb(volumeLevel);
    }
    AUDIO_DEBUG_LOG("Get system volume in db success %{public}f", getSystemVolumeInDb_);

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
    for (auto streamType : VOLUME_TYPE_LIST) {
        minVolumeIndexMap_[streamType] = MIN_VOLUME_LEVEL;
        maxVolumeIndexMap_[streamType] = MAX_VOLUME_LEVEL;
        volumeDataMaintainer_.SetStreamVolume(streamType, DEFAULT_VOLUME_LEVEL);
        AUDIO_DEBUG_LOG("streamType %{public}d index = [%{public}d, %{public}d, %{public}d]",
            streamType, minVolumeIndexMap_[streamType],
            maxVolumeIndexMap_[streamType], volumeDataMaintainer_.GetStreamVolume(streamType));
    }

    volumeDataMaintainer_.SetStreamVolume(STREAM_VOICE_CALL_ASSISTANT, MAX_VOLUME_LEVEL);
    volumeDataMaintainer_.SetStreamVolume(STREAM_ULTRASONIC, MAX_VOLUME_LEVEL);
}

void AudioAdapterManager::UpdateVolumeMapIndex()
{
    for (auto streamVolInfoPair : streamVolumeInfos_) {
        auto streamVolInfo = streamVolInfoPair.second;
        minVolumeIndexMap_[streamVolInfo->streamType] = streamVolInfo->minLevel;
        maxVolumeIndexMap_[streamVolInfo->streamType] = streamVolInfo->maxLevel;
        volumeDataMaintainer_.SetStreamVolume(streamVolInfo->streamType, streamVolInfo->defaultLevel);
        AUDIO_DEBUG_LOG("update streamType %{public}d index = [%{public}d, %{public}d, %{public}d]",
            streamVolInfo->streamType, minVolumeIndexMap_[streamVolInfo->streamType],
            maxVolumeIndexMap_[streamVolInfo->streamType],
            volumeDataMaintainer_.GetStreamVolume(streamVolInfo->streamType));
    }
}

void AudioAdapterManager::GetVolumePoints(AudioVolumeType streamType, DeviceVolumeType deviceType,
    std::vector<VolumePoint> &volumePoints)
{
    auto streamVolInfo = streamVolumeInfos_.find(streamType);
    auto deviceVolInfo = streamVolInfo->second->deviceVolumeInfos.find(deviceType);
    volumePoints = deviceVolInfo->second->volumePoints;
}

void AudioAdapterManager::GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos)
{
    streamVolumeInfos = streamVolumeInfos_;
}

void AudioAdapterManager::SetActiveDevice(DeviceType deviceType)
{
    AUDIO_PRERELEASE_LOGI("SetActiveDevice deviceType %{public}d", deviceType);
    SetVolumeForSwitchDevice(deviceType);
}

DeviceType AudioAdapterManager::GetActiveDevice()
{
    return currentActiveDevice_;
}

void AudioAdapterManager::SetAbsVolumeScene(bool isAbsVolumeScene)
{
    AUDIO_PRERELEASE_LOGI("SetAbsVolumeScene: %{public}d", isAbsVolumeScene);
    isAbsVolumeScene_ = isAbsVolumeScene;
    if (currentActiveDevice_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        SetVolumeDb(STREAM_MUSIC);
    } else {
        AUDIO_INFO_LOG("The currentActiveDevice is not A2DP");
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
    if (currentActiveDevice_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        SetVolumeDb(STREAM_MUSIC);
    } else {
        AUDIO_INFO_LOG("The currentActiveDevice is not A2DP");
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
    LoadMuteStatusMap();
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

void AudioAdapterManager::SafeVolumeDump(std::string &dumpString)
{
    dumpString += "SafeVolume info:\n";
    for (auto &streamType : VOLUME_TYPE_LIST) {
        AppendFormat(dumpString, "  - samplingAudioStreamTypeate: %d", streamType);
        AppendFormat(dumpString, "   volumeLevel: %d\n", volumeDataMaintainer_.GetStreamVolume(streamType));
        AppendFormat(dumpString, "  - AudioStreamType: %d", streamType);
        AppendFormat(dumpString, "   streamMuteStatus: %d\n", volumeDataMaintainer_.GetStreamMute(streamType));
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
    isVgsVolumeSupported_ = isVgsSupported;
}

bool AudioAdapterManager::IsVgsVolumeSupported() const
{
    return isVgsVolumeSupported_;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
