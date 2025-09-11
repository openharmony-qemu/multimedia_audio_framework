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
#define LOG_TAG "AudioSystemManager"
#endif

#include "audio_system_manager.h"

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "audio_manager_listener_stub_impl.h"
#include "audio_policy_interface.h"
#include "audio_focus_info_change_callback_impl.h"
#include "audio_workgroup_callback_stub.h"
#include "audio_qosmanager.h"
#include "rtg_interface.h"
#include "qos.h"
#include "istandard_audio_service.h"
#include "audio_workgroup_callback_impl.h"
using namespace OHOS::RME;

namespace OHOS {
namespace AudioStandard {
using namespace std;
constexpr unsigned int GET_BUNDLE_INFO_TIME_OUT_SECONDS = 10;
constexpr unsigned int XCOLLIE_TIME_OUT_SECONDS = 10;
constexpr unsigned int MS_PER_SECOND = 1000;
constexpr size_t VALID_REMOTE_NETWORK_ID_LENGTH = 64;
const map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::streamTypeMap_
    = AudioSystemManager::CreateStreamMap();
mutex g_asProxyMutex;
mutex g_audioListenerMutex;
sptr<IStandardAudioService> g_asProxy = nullptr;
sptr<AudioManagerListenerStubImpl> g_audioListener = nullptr;

AudioSystemManager::AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager start");
}

AudioSystemManager::~AudioSystemManager()
{
    AUDIO_DEBUG_LOG("~AudioSystemManager");
    if (cbClientId_ != -1) {
        UnsetRingerModeCallback(cbClientId_);
    }

    if (volumeChangeClientPid_ != -1) {
        AUDIO_DEBUG_LOG("UnregisterVolumeKeyEventCallback");
        (void)UnregisterVolumeKeyEventCallback(volumeChangeClientPid_);
        (void)UnregisterStreamVolumeChangeCallback(volumeChangeClientPid_);
        (void)UnregisterSystemVolumeChangeCallback(volumeChangeClientPid_);
    }
}

AudioSystemManager *AudioSystemManager::GetInstance()
{
    static AudioSystemManager audioManager;
    return &audioManager;
}

int32_t AudioSystemManager::GetCallingPid() const
{
    return getpid();
}

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::CreateStreamMap()
{
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    AudioSystemManager::CreateStreamMap(streamMap);
    return streamMap;
}

void AudioSystemManager::CreateStreamMap(map<pair<ContentType, StreamUsage>, AudioStreamType> &streamMap)
{
    // Only use stream usage to choose stream type
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_RINGTONE)] = STREAM_VOICE_RING;
}

AudioStreamType AudioSystemManager::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    } else {
        AUDIO_ERR_LOG("The pair of contentType and streamUsage is not in design. Use the default stream type");
    }

    if (streamType == AudioStreamType::STREAM_MEDIA) {
        streamType = AudioStreamType::STREAM_MUSIC;
    }

    return streamType;
}

inline const sptr<IStandardAudioService> GetAudioSystemManagerProxy()
{
    AudioXCollie xcollieGetAudioSystemManagerProxy("GetAudioSystemManagerProxy", XCOLLIE_TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    lock_guard<mutex> lock(g_asProxyMutex);
    if (g_asProxy == nullptr) {
        AudioXCollie xcollieGetSystemAbilityManager("GetSystemAbilityManager", XCOLLIE_TIME_OUT_SECONDS,
             nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        xcollieGetSystemAbilityManager.CancelXCollieTimer();

        AudioXCollie xcollieGetSystemAbility("GetSystemAbility", XCOLLIE_TIME_OUT_SECONDS);
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        g_asProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_asProxy != nullptr, nullptr, "get audio service proxy failed");
        xcollieGetSystemAbility.CancelXCollieTimer();

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
                AudioSystemManager::AudioServerDied(pid, uid);
            });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_asProxy;
    return gasp;
}

void AudioSystemManager::AudioServerDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("audio server died, will restore proxy in next call");
    {
        lock_guard<mutex> lock(g_asProxyMutex);
        g_asProxy = nullptr;
    }
    {
        lock_guard<mutex> lock(g_audioListenerMutex);
        g_audioListener = nullptr;
    }
}

int32_t AudioSystemManager::RegisterRendererDataTransferCallback(const DataTransferMonitorParam &param,
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR_INVALID_PARAM, "callback is null");
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "Audio service unavailable.");

    int32_t ret = SUCCESS;

    lock_guard<mutex> lock(g_audioListenerMutex);
    if (g_audioListener == nullptr) {
        g_audioListener = new(std::nothrow) AudioManagerListenerStubImpl();
        if (g_audioListener == nullptr) {
            AUDIO_ERR_LOG("g_audioListener is null");
            return ERROR;
        }

        sptr<IRemoteObject> object = g_audioListener->AsObject();
        if (object == nullptr) {
            AUDIO_ERR_LOG("as object result is null");
            g_audioListener = nullptr;
            return ERROR;
        }

        ret = gasp->RegisterDataTransferCallback(object);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "ret: %{public}d", ret);
    }

    auto callbackId = g_audioListener->AddDataTransferStateChangeCallback(param, callback);
    CHECK_AND_RETURN_RET_LOG(callbackId != -1, ERROR_SYSTEM, "out of max register count");
    ret = gasp->RegisterDataTransferMonitorParam(callbackId, param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "ret: %{public}d", ret);
    return ret;
}


int32_t AudioSystemManager::UnregisterRendererDataTransferCallback(
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR_INVALID_PARAM, "callback is null");
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "Audio service unavailable.");

    lock_guard<mutex> lock(g_audioListenerMutex);
    CHECK_AND_RETURN_RET_LOG(g_audioListener != nullptr, ERROR_INVALID_PARAM, "audio listener is null");
    auto callbackIds = g_audioListener->RemoveDataTransferStateChangeCallback(callback);

    for (auto callbackId : callbackIds) {
        gasp->UnregisterDataTransferMonitorParam(callbackId);
    }

    return SUCCESS;
}

int32_t AudioSystemManager::SetRingerMode(AudioRingerMode ringMode)
{
    // Deprecated. Please use the SetRingerMode interface of AudioGroupManager.
    AUDIO_WARNING_LOG("Use the deprecated SetRingerMode func. ringer mode [%{public}d]", ringMode);
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    ringModeBackup_ = ringMode;
    if (ringerModeCallback_ != nullptr) {
        ringerModeCallback_->OnRingerModeUpdated(ringModeBackup_);
    }

    return SUCCESS;
}

std::string AudioSystemManager::GetSelfBundleName(int32_t uid)
{
    AudioXCollie audioXCollie("AudioSystemManager::GetSelfBundleName_FromUid", GET_BUNDLE_INFO_TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::string bundleName = "";

    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():GetSelfBundleName");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    guard.CheckCurrTimeout();
    sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, bundleName, "remoteObject is null");

    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(iBundleMgr != nullptr, bundleName, "bundlemgr interface is null");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetSelfBundleName");
    iBundleMgr->GetNameForUid(uid, bundleName);
    reguard.CheckCurrTimeout();
    return bundleName;
}

AudioRingerMode AudioSystemManager::GetRingerMode()
{
    return ringModeBackup_;
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    AUDIO_DEBUG_LOG("audioScene_=%{public}d done", scene);
    return AudioPolicyManager::GetInstance().SetAudioScene(scene);
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    auto audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    AUDIO_DEBUG_LOG("origin audioScene: %{public}d", audioScene);
    switch (audioScene) {
        case AUDIO_SCENE_CALL_START:
        case AUDIO_SCENE_CALL_END:
            return AUDIO_SCENE_DEFAULT;

        case AUDIO_SCENE_VOICE_RINGING:
            return AUDIO_SCENE_RINGING;

        default:
            return audioScene;
    }
}

int32_t AudioSystemManager::SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientUid) const
{
    int32_t uid = clientUid == -1 ? getuid() : clientUid;
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetDeviceActive */
    return (AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag, uid));
}

bool AudioSystemManager::IsDeviceActive(DeviceType deviceType) const
{
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy IsDeviceActive */
    return (AudioPolicyManager::GetInstance().IsDeviceActive(static_cast<InternalDeviceType>(deviceType)));
}

DeviceType AudioSystemManager::GetActiveOutputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveOutputDevice();
}

DeviceType AudioSystemManager::GetActiveInputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveInputDevice();
}

bool AudioSystemManager::IsStreamActive(AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
        case STREAM_CAMCORDER:
            break;
        case STREAM_ULTRASONIC:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, false, "volumeType=%{public}d. No system permission", volumeType);
            break;
        }
        case STREAM_ALL:
        default:
            AUDIO_ERR_LOG("volumeType=%{public}d not supported", volumeType);
            return false;
    }

    return AudioPolicyManager::GetInstance().IsStreamActive(volumeType);
}

int32_t AudioSystemManager::SetAsrAecMode(const AsrAecMode asrAecMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrAecMode(static_cast<int32_t>(asrAecMode));
}

int32_t AudioSystemManager::GetAsrAecMode(AsrAecMode &asrAecMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrAecMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrAec Mode audio parameters failed");
    asrAecMode = static_cast<AsrAecMode>(mode);
    return 0;
}

int32_t AudioSystemManager::SetAsrNoiseSuppressionMode(const AsrNoiseSuppressionMode asrNoiseSuppressionMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrNoiseSuppressionMode(static_cast<int32_t>(asrNoiseSuppressionMode));
}

int32_t AudioSystemManager::GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrNoiseSuppressionMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrAec Mode audio parameters failed");
    asrNoiseSuppressionMode = static_cast<AsrNoiseSuppressionMode>(mode);
    return 0;
}

int32_t AudioSystemManager::SetAsrWhisperDetectionMode(const AsrWhisperDetectionMode asrWhisperDetectionMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrWhisperDetectionMode(static_cast<int32_t>(asrWhisperDetectionMode));
}

int32_t AudioSystemManager::GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t mode = 0;
    int32_t ret = gasp->GetAsrWhisperDetectionMode(mode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrWhisperDetection Mode audio parameters failed");
    asrWhisperDetectionMode = static_cast<AsrWhisperDetectionMode>(mode);
    return 0;
}

int32_t AudioSystemManager::SetAsrVoiceControlMode(const AsrVoiceControlMode asrVoiceControlMode, bool on)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrVoiceControlMode(static_cast<int32_t>(asrVoiceControlMode), on);
}

int32_t AudioSystemManager::SetAsrVoiceMuteMode(const AsrVoiceMuteMode asrVoiceMuteMode, bool on)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->SetAsrVoiceMuteMode(static_cast<int32_t>(asrVoiceMuteMode), on);
}

int32_t AudioSystemManager::IsWhispering()
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t whisperRes = 0;
    gasp->IsWhispering(whisperRes);
    return whisperRes;
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, "", "Audio service unavailable.");
    std::string value = "";
    gasp->GetAudioParameter(key, value);
    return value;
}

void AudioSystemManager::SetAudioParameter(const std::string &key, const std::string &value)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioParameter(key, value);
}

int32_t AudioSystemManager::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    std::vector<StringPair> resultPair;
    int32_t ret = gasp->GetExtraParameters(mainKey, subKeys, resultPair);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "Get extra parameters failed");
    for (auto &pair : resultPair) {
        result.push_back(std::make_pair(pair.firstParam, pair.secondParam));
    }
    return ret;
}

int32_t AudioSystemManager::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    std::vector<StringPair> pairs;
    for (const auto &pair : kvpairs) {
        pairs.push_back({pair.first, pair.second});
    }
    return gasp->SetExtraParameters(key, pairs);
}

uint64_t AudioSystemManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    uint64_t transactionId = 0;
    int32_t res = gasp->GetTransactionId(deviceType, deviceRole, transactionId);
    CHECK_AND_RETURN_RET_LOG(res == 0, 0, "GetTransactionId failed");
    return transactionId;
}

int32_t AudioSystemManager::SetSelfAppVolume(int32_t volume, int32_t flag)
{
    AUDIO_INFO_LOG("enter AudioSystemManager::SetSelfAppVolume");
    return AudioPolicyManager::GetInstance().SetSelfAppVolumeLevel(volume);
}

// LCOV_EXCL_START
int32_t AudioSystemManager::SetAppVolume(int32_t appUid, int32_t volume, int32_t flag)
{
    AUDIO_INFO_LOG("enter AudioSystemManager::SetAppVolume");
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "SetAppVolume: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "SetAppVolume: No system permission");
    return AudioPolicyManager::GetInstance().SetAppVolumeLevel(appUid, volume);
}

int32_t AudioSystemManager::GetAppVolume(int32_t appUid, int32_t &volumeLevel) const
{
    AUDIO_INFO_LOG("enter AudioSystemManager::GetAppVolume");
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "GetAppVolume: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "GetAppVolume: No system permission");
    return AudioPolicyManager::GetInstance().GetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioSystemManager::GetSelfAppVolume(int32_t &volumeLevel) const
{
    AUDIO_INFO_LOG("enter AudioSystemManager::GetSelfAppVolume");
    return AudioPolicyManager::GetInstance().GetSelfAppVolumeLevel(volumeLevel);
}

int32_t AudioSystemManager::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    AUDIO_INFO_LOG("SetAppVolumeMuted: appUid[%{public}d], muted[%{public}d], flag[%{public}d]",
        appUid, muted, volumeFlag);
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "SetAppVolumeMuted: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "SetAppVolumeMuted: No system permission");
    return AudioPolicyManager::GetInstance().SetAppVolumeMuted(appUid, muted, volumeFlag);
}

int32_t AudioSystemManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "SetAppRingMuted: No system permission");
    return AudioPolicyManager::GetInstance().SetAppRingMuted(appUid, muted);
}
// LCOV_EXCL_STOP

int32_t AudioSystemManager::UnsetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetSelfAppVolumeCallback(callback);
}

int32_t AudioSystemManager::SetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetSelfAppVolumeCallback: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetSelfAppVolumeChangeCallback(callback);
}

int32_t AudioSystemManager::SetAppVolumeCallbackForUid(const int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetAppVolumeCallbackForUid: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAppVolumeChangeCallbackForUid(appUid, callback);
}

int32_t AudioSystemManager::UnsetAppVolumeCallbackForUid(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetAppVolumeCallbackForUid(callback);
}

int32_t AudioSystemManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("IsAppVolumeMute: appUid[%{public}d], muted[%{public}d]", appUid, owned);
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "IsAppVolumeMute: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "IsAppVolumeMute: No system permission");
    return AudioPolicyManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

int32_t AudioSystemManager::UnsetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetActiveVolumeTypeCallback(callback);
}

int32_t AudioSystemManager::SetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetActiveVolumeTypeCallback: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetActiveVolumeTypeCallback(callback);
}

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel) const
{
    AUDIO_INFO_LOG("SetSystemVolume: volumeType[%{public}d], volumeLevel[%{public}d]", volumeType, volumeLevel);

    /* Validate volumeType and return INVALID_PARAMS error */
    switch (volumeType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_RING:
        case STREAM_MUSIC:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType[%{public}d] is not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetSystemVolumeLevel */
    return AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, true);
}

int32_t AudioSystemManager::SetVolumeWithDevice(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType) const
{
    AUDIO_INFO_LOG("%{public}s: volumeType[%{public}d], volumeLevel[%{public}d], deviceType[%{public}d]",
        __func__, volumeType, volumeLevel, deviceType);

    /* Validate volumeType and return INVALID_PARAMS error */
    switch (volumeType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_RING:
        case STREAM_MUSIC:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType[%{public}d] is not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetSystemVolumeLevel */
    return AudioPolicyManager::GetInstance().SetSystemVolumeLevelWithDevice(volumeType, volumeLevel, deviceType);
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType[%{public}d] is not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    return AudioPolicyManager::GetInstance().GetSystemVolumeLevel(volumeType);
}

int32_t AudioSystemManager::SetLowPowerVolume(int32_t streamId, float volume) const
{
    AUDIO_INFO_LOG("streamId:%{public}d, vol:%{public}f.", streamId, volume);
    CHECK_AND_RETURN_RET_LOG((volume >= 0) && (volume <= 1.0), ERR_INVALID_PARAM,
        "Invalid Volume Input!");

    return AudioPolicyManager::GetInstance().SetLowPowerVolume(streamId, volume);
}

float AudioSystemManager::GetLowPowerVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetLowPowerVolume(streamId);
}

float AudioSystemManager::GetSingleStreamVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetSingleStreamVolume(streamId);
}

int32_t AudioSystemManager::GetMaxVolume(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    }

    if (volumeType == STREAM_ULTRASONIC) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    }

    return AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
}

int32_t AudioSystemManager::GetMinVolume(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    }

    if (volumeType == STREAM_ULTRASONIC) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "STREAM_ULTRASONIC No system permission");
    }

    return AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
}

int32_t AudioSystemManager::GetDeviceMaxVolume(AudioVolumeType volumeType, DeviceType deviceType)
{
    if (volumeType == STREAM_ALL) {
        bool ret1 = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret1, ERR_PERMISSION_DENIED, "No system permission");
    }

    if (volumeType == STREAM_ULTRASONIC) {
        bool ret2 = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret2, ERR_PERMISSION_DENIED, "STREAM_ULTRASONIC No system permission");
    }

    return AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType, deviceType);
}

int32_t AudioSystemManager::GetDeviceMinVolume(AudioVolumeType volumeType, DeviceType deviceType)
{
    if (volumeType == STREAM_ALL) {
        bool ret1 = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret1, ERR_PERMISSION_DENIED, "No system permission");
    }

    if (volumeType == STREAM_ULTRASONIC) {
        bool ret2 = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret2, ERR_PERMISSION_DENIED, "STREAM_ULTRASONIC No system permission");
    }

    return AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType, deviceType);
}

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType) const
{
    AUDIO_INFO_LOG("SetStreamMute for volumeType [%{public}d], mute [%{public}d]", volumeType, mute);
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetStreamMute */
    return AudioPolicyManager::GetInstance().SetStreamMute(volumeType, mute, true, deviceType);
}

bool AudioSystemManager::IsStreamMute(AudioVolumeType volumeType) const
{
    AUDIO_DEBUG_LOG("AudioSystemManager::GetMute Client");

    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("volumeType=%{public}d not supported", volumeType);
            return false;
    }

    return AudioPolicyManager::GetInstance().GetStreamMute(volumeType);
}

float AudioSystemManager::GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device)
{
    AUDIO_INFO_LOG("enter AudioSystemManager::GetVolumeInUnitOfDb");
    return AudioPolicyManager::GetInstance().GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

int32_t AudioSystemManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
}

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag,
    std::shared_ptr<AudioManagerDeviceChangeCallback> cb)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId, flag, cb);
}

int32_t AudioSystemManager::SetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetMicrophoneBlockedCallback(clientId, callback);
}

int32_t AudioSystemManager::UnsetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().UnsetMicrophoneBlockedCallback(clientId, callback);
}

int32_t AudioSystemManager::SetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetAudioSceneChangeCallback(clientId, callback);
}

int32_t AudioSystemManager::UnsetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().UnsetAudioSceneChangeCallback(callback);
}

int32_t AudioSystemManager::SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryClientTypeCallback(callback);
}

int32_t AudioSystemManager::SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAudioClientInfoMgrCallback(callback);
}

int32_t AudioSystemManager::SetAudioVKBInfoMgrCallback(const std::shared_ptr<AudioVKBInfoMgrCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAudioVKBInfoMgrCallback(callback);
}

int32_t AudioSystemManager::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    AUDIO_INFO_LOG("In");
    return AudioPolicyManager::GetInstance().CheckVKBInfo(bundleName, isValid);
}

int32_t AudioSystemManager::SetQueryBundleNameListCallback(
    const std::shared_ptr<AudioQueryBundleNameListCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryBundleNameListCallback(callback);
}

int32_t AudioSystemManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    cbClientId_ = clientId;
    ringerModeCallback_ = callback;

    return SUCCESS;
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    CHECK_AND_RETURN_RET(clientId == cbClientId_, ERR_INVALID_OPERATION);

    return SUCCESS;
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute)
{
    return AudioPolicyManager::GetInstance().SetMicrophoneMute(isMute);
}

int32_t AudioSystemManager::SetVoiceRingtoneMute(bool isMute)
{
    AUDIO_INFO_LOG("Set Voice Ringtone is %{public}d", isMute);
    return AudioPolicyManager::GetInstance().SetVoiceRingtoneMute(isMute);
}

bool AudioSystemManager::IsMicrophoneMute()
{
    std::shared_ptr<AudioGroupManager> groupManager = GetGroupManager(DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_RET_LOG(groupManager != nullptr, false, "failed, groupManager is null");
    return groupManager->IsMicrophoneMuteLegacy();
}

int32_t AudioSystemManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");
    if (audioDeviceDescriptors[0]->networkId_ != LOCAL_NETWORK_ID &&
        audioDeviceDescriptors[0]->networkId_.size() != VALID_REMOTE_NETWORK_ID_LENGTH) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid networkId.");
        return ERR_INVALID_PARAM;
    }
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    return ret;
}

int32_t AudioSystemManager::SelectInputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() == 1 && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_PARAM, "invalid parameter");
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::INPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

std::string AudioSystemManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const
{
    return AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioSystemManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    // basic check
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr && audioDeviceDescriptors.size() != 0,
        ERR_INVALID_PARAM, "invalid parameter");

    size_t validDeviceSize = 1;
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() <= validDeviceSize &&
        audioDeviceDescriptors[0] != nullptr, ERR_INVALID_OPERATION, "device error");
    audioRendererFilter->streamType = AudioSystemManager::GetStreamType(audioRendererFilter->rendererInfo.contentType,
        audioRendererFilter->rendererInfo.streamUsage);
    // operation chack
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::OUTPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an output device.");

    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->networkId_ == LOCAL_NETWORK_ID ||
        audioDeviceDescriptors[0]->networkId_.size() == VALID_REMOTE_NETWORK_ID_LENGTH,
        ERR_INVALID_PARAM, "invalid networkId.");
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter->uid >= 0 || (audioRendererFilter->uid == -1),
        ERR_INVALID_PARAM, "invalid uid.");

    AUDIO_DEBUG_LOG("[%{public}d] SelectOutputDevice: uid<%{public}d> streamType<%{public}d> device<name:%{public}s>",
        getpid(), audioRendererFilter->uid, static_cast<int32_t>(audioRendererFilter->streamType),
        (audioDeviceDescriptors[0]->networkId_.c_str()));

    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    // basic check
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr && audioDeviceDescriptors.size() != 0,
        ERR_INVALID_PARAM, "invalid parameter");

    size_t validDeviceSize = 1;
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors.size() <= validDeviceSize && audioDeviceDescriptors[0] != nullptr,
        ERR_INVALID_OPERATION, "device error.");
    // operation chack
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptors[0]->deviceRole_ == DeviceRole::INPUT_DEVICE,
        ERR_INVALID_OPERATION, "not an input device");
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter->uid >= 0 || (audioCapturerFilter->uid == -1),
        ERR_INVALID_PARAM, "invalid uid.");
    AUDIO_DEBUG_LOG("[%{public}d] SelectInputDevice: uid<%{public}d> device<type:%{public}d>",
        getpid(), audioCapturerFilter->uid, static_cast<int32_t>(audioDeviceDescriptors[0]->deviceType_));

    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

// LCOV_EXCL_START
int32_t AudioSystemManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES,
        ERR_INVALID_PARAM, "invalid parameter: only support output device");
    CHECK_AND_RETURN_RET_LOG(!audioDeviceDescriptors.empty(), ERR_INVALID_PARAM, "invalid parameter: empty list");
    for (const auto &devDesc : audioDeviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->networkId_ == LOCAL_NETWORK_ID ||
            devDesc->networkId_.size() == VALID_REMOTE_NETWORK_ID_LENGTH,
            ERR_INVALID_PARAM, "invalid parameter: invalid networkId.");
    }
    return AudioPolicyManager::GetInstance().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES,
        ERR_INVALID_PARAM, "invalid parameter: only support output device");
    CHECK_AND_RETURN_RET_LOG(!audioDeviceDescriptors.empty(), ERR_INVALID_PARAM, "invalid parameter: empty list");
    for (const auto &devDesc : audioDeviceDescriptors) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->networkId_ == LOCAL_NETWORK_ID ||
            devDesc->networkId_.size() == VALID_REMOTE_NETWORK_ID_LENGTH,
            ERR_INVALID_PARAM, "invalid parameter: invalid networkId.");
    }
    return AudioPolicyManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const
{
    CHECK_AND_RETURN_RET_LOG(audioDevUsage == MEDIA_OUTPUT_DEVICES || audioDevUsage == CALL_OUTPUT_DEVICES,
        ERR_INVALID_PARAM, "invalid parameter: only support output device");
    auto unexcludeOutputDevices = GetExcludedDevices(audioDevUsage);
    if (unexcludeOutputDevices.empty()) {
        return SUCCESS;
    }
    for (const auto &devDesc : unexcludeOutputDevices) {
        CHECK_AND_RETURN_RET_LOG(devDesc != nullptr, ERR_INVALID_PARAM, "invalid parameter: mull pointer in list");
        CHECK_AND_RETURN_RET_LOG(!(devDesc->deviceType_ == DEVICE_TYPE_SPEAKER &&
            devDesc->networkId_ == LOCAL_NETWORK_ID),
            ERR_INVALID_PARAM, "invalid parameter: speaker can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->deviceType_ != DEVICE_TYPE_EARPIECE, ERR_INVALID_PARAM,
            "invalid parameter: earpiece can not be excluded.");
        CHECK_AND_RETURN_RET_LOG(devDesc->networkId_ == LOCAL_NETWORK_ID ||
            devDesc->networkId_.size() == VALID_REMOTE_NETWORK_ID_LENGTH,
            ERR_INVALID_PARAM, "invalid parameter: invalid networkId.");
    }
    return AudioPolicyManager::GetInstance().UnexcludeOutputDevices(audioDevUsage, unexcludeOutputDevices);
}
// LCOV_EXCL_STOP

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage) const
{
    return AudioPolicyManager::GetInstance().GetExcludedDevices(audioDevUsage);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevicesInner(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevicesInner(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    AudioRendererInfo rendererInfo;
    return AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
}

int32_t AudioSystemManager::GetPreferredInputDeviceDescriptors()
{
    AudioCapturerInfo capturerInfo;
    auto dec = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    CHECK_AND_RETURN_RET(dec.size() > 0, ERROR_INVALID_PARAM);
    return SUCCESS;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    return AudioPolicyManager::GetInstance().GetOutputDevice(audioRendererFilter);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    return AudioPolicyManager::GetInstance().GetInputDevice(audioCapturerFilter);
}

int32_t AudioSystemManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().GetAudioFocusInfoList(focusInfoList);
}

int32_t AudioSystemManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    int32_t clientId = GetCallingPid();
    AUDIO_DEBUG_LOG("RegisterFocusInfoChangeCallback clientId:%{public}d", clientId);
    if (audioFocusInfoCallback_ == nullptr) {
        audioFocusInfoCallback_ = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(audioFocusInfoCallback_ != nullptr, ERROR,
            "Failed to allocate memory for audioInterruptCallback");
        int32_t ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId,
            audioFocusInfoCallback_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed set callback");
    }

    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    CHECK_AND_RETURN_RET_LOG(cbFocusInfo != nullptr, ERROR, "cbFocusInfo is nullptr");
    cbFocusInfo->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioSystemManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    int32_t clientId = GetCallingPid();
    int32_t ret = 0;

    if (callback == nullptr) {
        ret = AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
        audioFocusInfoCallback_.reset();
        audioFocusInfoCallback_ = nullptr;
        if (!ret) {
            AUDIO_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback success");
        }
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(audioFocusInfoCallback_ != nullptr, ERROR,
        "Failed to allocate memory for audioInterruptCallback");
    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    cbFocusInfo->RemoveCallback(callback);

    return ret;
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("AudioSystemManager RegisterVolumeKeyEventCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterVolumeKeyEventCallbackcallback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback, api_v);
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnregisterVolumeKeyEventCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("UnsetVolumeKeyEventCallback success");
    }
    return ret;
}

int32_t AudioSystemManager::RegisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("register");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeDegreeCallback(clientPid, callback);
}

int32_t AudioSystemManager::UnregisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("unregister");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeDegreeCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("success");
    }
    return ret;
}

int32_t AudioSystemManager::RegisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioSystemManager RegisterSystemVolumeChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterSystemVolumeChangeCallback callback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetSystemVolumeChangeCallback(clientPid, callback);
}

int32_t AudioSystemManager::UnregisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnregisterSystemVolumeChangeCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetSystemVolumeChangeCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("UnsetSystemVolumeChangeCallback success");
    }
    return ret;
}

void AudioSystemManager::SetAudioMonoState(bool monoState)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioMonoState(monoState);
}

void AudioSystemManager::SetAudioBalanceValue(float balanceValue)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_LOG(gasp != nullptr, "Audio service unavailable.");
    gasp->SetAudioBalanceValue(balanceValue);
}

int32_t AudioSystemManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return AudioPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
}

std::string AudioSystemManager::GetSystemSoundUri(const std::string &key)
{
    return AudioPolicyManager::GetInstance().GetSystemSoundUri(key);
}

// Below stub implementation is added to handle compilation error in call manager
// Once call manager adapt to new interrupt implementation, this will be removed
int32_t AudioSystemManager::SetAudioManagerCallback(const AudioVolumeType streamType,
                                                    const std::shared_ptr<AudioManagerCallback> &callback)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioVolumeType streamType) const
{
    AUDIO_DEBUG_LOG("stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::ActivateAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::SetAppConcurrencyMode(const int32_t appUid, const int32_t mode)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().SetAppConcurrencyMode(appUid, mode);
}

int32_t AudioSystemManager::SetAppSlientOnDisplay(const int32_t displayId)
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().SetAppSlientOnDisplay(displayId);
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    AUDIO_DEBUG_LOG("stub implementation");
    return AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::ActivatePreemptMode() const
{
    AUDIO_DEBUG_LOG("ActivatePreemptMode");
    return AudioPolicyManager::GetInstance().ActivatePreemptMode();
}

int32_t AudioSystemManager::DeactivatePreemptMode() const
{
    AUDIO_DEBUG_LOG("DeactivatePreemptMode");
    return AudioPolicyManager::GetInstance().DeactivatePreemptMode();
}

int32_t AudioSystemManager::SetForegroundList(std::vector<std::string> list)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->SetForegroundList(list);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioSystemManager::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioSystemManager::CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->CheckCaptureLimit(config, innerCapId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioSystemManager::ReleaseCaptureLimit(int32_t innerCapId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_ILLEGAL_STATE, "Audio service unavailable.");
    int32_t ret = gasp->ReleaseCaptureLimit(innerCapId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}
#endif

int32_t AudioSystemManager::GenerateSessionId(uint32_t &sessionId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    int32_t ret = gasp->GenerateSessionId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get sessionId failed");
    return 0;
}

int32_t AudioSystemManager::SetAudioInterruptCallback(const uint32_t sessionID,
    const std::shared_ptr<AudioInterruptCallback> &callback, uint32_t clientUid, const int32_t zoneID)
{
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID, callback, clientUid, zoneID);
}

int32_t AudioSystemManager::UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId)
{
    return AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(zoneId, sessionId);
}

int32_t AudioSystemManager::SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    if (audioInterruptCallback_ != nullptr) {
        callback->cbMutex_.lock();
        AUDIO_DEBUG_LOG("reset existing callback object");
        AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
        callback->cbMutex_.unlock();
    }

    audioInterruptCallback_ = std::make_shared<AudioManagerInterruptCallbackImpl>();
    CHECK_AND_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
        "Failed to allocate memory for audioInterruptCallback");

    int32_t ret =
        AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, audioInterruptCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed set callback");

    std::shared_ptr<AudioManagerInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioManagerInterruptCallbackImpl>(audioInterruptCallback_);
    CHECK_AND_RETURN_RET_LOG(cbInterrupt != nullptr, ERROR, "cbInterrupt is nullptr");
    cbInterrupt->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerInterruptCallback()
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("client id: %{public}d", clientId);

    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
    if (audioInterruptCallback_ != nullptr) {
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
    }

    return ret;
}

int32_t AudioSystemManager::RequestAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("RequestAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().RequestAudioFocus(clientId, audioInterrupt);
}

int32_t AudioSystemManager::AbandonAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("AbandonAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().AbandonAudioFocus(clientId, audioInterrupt);
}

int32_t AudioSystemManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
}

std::shared_ptr<AudioGroupManager> AudioSystemManager::GetGroupManager(int32_t groupId)
{
    std::lock_guard<std::mutex> lock(groupManagerMapMutex_);
    std::vector<std::shared_ptr<AudioGroupManager>>::iterator iter = groupManagerMap_.begin();
    while (iter != groupManagerMap_.end()) {
        if ((*iter)->GetGroupId() == groupId) {
            return *iter;
        } else {
            iter++;
        }
    }

    std::shared_ptr<AudioGroupManager> groupManager = std::make_shared<AudioGroupManager>(groupId);
    if (groupManager->Init() == SUCCESS) {
        groupManagerMap_.push_back(groupManager);
    } else {
        groupManager = nullptr;
    }
    return groupManager;
}

AudioManagerInterruptCallbackImpl::AudioManagerInterruptCallbackImpl()
{
    AUDIO_INFO_LOG("AudioManagerInterruptCallbackImpl constructor");
}

AudioManagerInterruptCallbackImpl::~AudioManagerInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioManagerInterruptCallbackImpl: instance destroy");
}

void AudioManagerInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback)
{
    auto wp = callback.lock();
    if (wp != nullptr) {
        callback_ = callback;
    } else {
        AUDIO_ERR_LOG("callback is nullptr");
    }
}

void AudioManagerInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    cb_ = callback_.lock();
    if (cb_ != nullptr) {
        cb_->cbMutex_.lock();
        InterruptAction interruptAction = {};
        interruptAction.actionType = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN)
            ? TYPE_INTERRUPT : TYPE_ACTIVATED;
        interruptAction.interruptType = interruptEvent.eventType;
        interruptAction.interruptHint = interruptEvent.hintType;
        interruptAction.activated = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN) ? false : true;
        cb_->OnInterrupt(interruptAction);
        AUDIO_DEBUG_LOG("Notify event to app complete");
        cb_->cbMutex_.unlock();
    } else {
        AUDIO_ERR_LOG("callback is null");
    }

    return;
}

bool AudioSystemManager::RequestIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("RequestIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = GetCallingPid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.streamId = static_cast<uint32_t>(clientId);
    int32_t result = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);

    AUDIO_DEBUG_LOG("Rresult -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}
bool AudioSystemManager::AbandonIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("AbandonIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = GetCallingPid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.streamId = static_cast<uint32_t>(clientId);
    int32_t result = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
    AUDIO_DEBUG_LOG("result -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}

int32_t AudioSystemManager::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("clientUid:%{public}d streamSetState:%{public}d streamUsage:%{public}d",
        clientUid, streamSetState, streamUsage);
    return AudioPolicyManager::GetInstance().UpdateStreamState(clientUid, streamSetState, streamUsage);
}

std::string AudioSystemManager::GetSelfBundleName()
{
    AudioXCollie audioXCollie("AudioSystemManager::GetSelfBundleName", GET_BUNDLE_INFO_TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);

    std::string bundleName = "";

    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();GetSelfBundleName");
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    guard.CheckCurrTimeout();
    sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, bundleName, "remoteObject is null");

    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(iBundleMgr != nullptr, bundleName, "bundlemgr interface is null");

    AppExecFwk::BundleInfo bundleInfo;
    WatchTimeout reguard("iBundleMgr->GetBundleInfoForSelf:GetSelfBundleName");
    if (iBundleMgr->GetBundleInfoForSelf(0, bundleInfo) == ERR_OK) {
        bundleName = bundleInfo.name;
    } else {
        AUDIO_DEBUG_LOG("Get bundle info failed");
    }
    reguard.CheckCurrTimeout();
    return bundleName;
}

int32_t AudioSystemManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    AUDIO_INFO_LOG("AudioSystemManager::SetDeviceAbsVolumeSupported");
    return AudioPolicyManager::GetInstance().SetDeviceAbsVolumeSupported(macAddress, support);
}

int32_t AudioSystemManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    return AudioPolicyManager::GetInstance().SetAdjustVolumeForZone(zoneId);
}

int32_t AudioSystemManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    AUDIO_INFO_LOG("volume: %{public}d, update ui: %{public}d", volume, updateUi);
    return AudioPolicyManager::GetInstance().SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

int32_t AudioSystemManager::SetNearlinkDeviceVolume(const std::string &macAddress, AudioVolumeType volumeType,
    const int32_t volume, const bool updateUi)
{
    AUDIO_INFO_LOG("volume: %{public}d, update ui: %{public}d", volume, updateUi);
    return AudioPolicyManager::GetInstance().SetNearlinkDeviceVolume(macAddress, volumeType, volume, updateUi);
}

AudioPin AudioSystemManager::GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const
{
    AudioPin pin = AUDIO_PIN_NONE;
    uint16_t dmDeviceType = 0;
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_NONE:
        case OHOS::AudioStandard::DEVICE_TYPE_INVALID:
            pin = AUDIO_PIN_NONE;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_DEFAULT:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_DAUDIO_DEFAULT;
            } else {
                pin = AUDIO_PIN_OUT_DAUDIO_DEFAULT;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_SPEAKER:
            pin = AUDIO_PIN_OUT_SPEAKER;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_MIC:
        case OHOS::AudioStandard::DEVICE_TYPE_WAKEUP:
            pin = AUDIO_PIN_IN_MIC;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET:
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES:
        case OHOS::AudioStandard::DEVICE_TYPE_DP:
        case OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET:
        case OHOS::AudioStandard::DEVICE_TYPE_HDMI:
        case OHOS::AudioStandard::DEVICE_TYPE_ACCESSORY:
            pin = GetPinValueForPeripherals(deviceType, deviceRole, dmDeviceType);
            break;
        default:
            OtherDeviceTypeCases(deviceType);
            break;
    }
    return pin;
}

void AudioSystemManager::OtherDeviceTypeCases(DeviceType deviceType) const
{
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SINK:
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SOURCE:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_SCO:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP:
        case OHOS::AudioStandard::DEVICE_TYPE_MAX:
            AUDIO_INFO_LOG("don't supported the device type");
            break;
        default:
            AUDIO_INFO_LOG("invalid input parameter");
            break;
    }
}

AudioPin AudioSystemManager::GetPinValueForPeripherals(DeviceType deviceType, DeviceRole deviceRole,
    uint16_t dmDeviceType) const
{
    AudioPin pin = AUDIO_PIN_NONE;
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_HS_MIC;
            } else {
                pin = AUDIO_PIN_OUT_HEADSET;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES:
            pin = AUDIO_PIN_OUT_HEADPHONE;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_DP:
            pin = AUDIO_PIN_OUT_DP;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_USB_HEADSET;
            } else {
                pin = AUDIO_PIN_OUT_USB_HEADSET;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_HDMI:
            pin = AUDIO_PIN_OUT_HDMI;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_ACCESSORY:
            dmDeviceType = GetDmDeviceType();
            if (dmDeviceType == DM_DEVICE_TYPE_PENCIL) {
                pin = AUDIO_PIN_IN_PENCIL;
            } else if (dmDeviceType == DM_DEVICE_TYPE_UWB) {
                pin = AUDIO_PIN_IN_UWB;
            }
            break;
        default:
            AUDIO_INFO_LOG("other case");
    }
    return pin;
}

DeviceType AudioSystemManager::GetTypeValueFromPin(AudioPin pin) const
{
    DeviceType type = DEVICE_TYPE_NONE;
    switch (pin) {
        case OHOS::AudioStandard::AUDIO_PIN_NONE:
            type = DEVICE_TYPE_NONE;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_SPEAKER:
            type = DEVICE_TYPE_SPEAKER;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADSET:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_LINEOUT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HDMI:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_MIC:
            type = DEVICE_TYPE_MIC;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_HS_MIC:
            type = DEVICE_TYPE_WIRED_HEADSET;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_LINEIN:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_PENCIL:
        case OHOS::AudioStandard::AUDIO_PIN_IN_UWB:
            type = DEVICE_TYPE_ACCESSORY;
            break;
        default:
            AUDIO_INFO_LOG("invalid input parameter");
            break;
    }
    return type;
}

int32_t AudioSystemManager::RegisterWakeupSourceCallback()
{
    AUDIO_INFO_LOG("RegisterWakeupSourceCallback");
    remoteWakeUpCallback_ = std::make_shared<WakeUpCallbackImpl>(this);

    sptr<AudioManagerListenerStubImpl> wakeupCloseCbStub = new(std::nothrow) AudioManagerListenerStubImpl();
    CHECK_AND_RETURN_RET_LOG(wakeupCloseCbStub != nullptr, ERROR,
        "wakeupCloseCbStub is null");
    wakeupCloseCbStub->SetWakeupSourceCallback(remoteWakeUpCallback_);

    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "GetAudioParameter::Audio service unavailable.");

    sptr<IRemoteObject> object = wakeupCloseCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("SetWakeupCloseCallback listenerStub object is nullptr");
        return ERROR;
    }
    return gasp->SetWakeupSourceCallback(object);
}

int32_t AudioSystemManager::SetAudioCapturerSourceCallback(
    const std::shared_ptr<AudioCapturerSourceCallback> &callback)
{
    audioCapturerSourceCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

int32_t AudioSystemManager::SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback)
{
    audioWakeUpSourceCloseCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

int32_t AudioSystemManager::SetAvailableDeviceChangeCallback(const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().SetAvailableDeviceChangeCallback(clientId, usage, callback);
}

int32_t AudioSystemManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    int32_t clientId = GetCallingPid();
    return AudioPolicyManager::GetInstance().UnsetAvailableDeviceChangeCallback(clientId, usage);
}

int32_t AudioSystemManager::ConfigDistributedRoutingRole(
    std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    if (descriptor == nullptr) {
        AUDIO_ERR_LOG("ConfigDistributedRoutingRole: invalid parameter");
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG(" Entered ConfigDistributedRoutingRole casttype %{public}d", type);
    if (descriptor->deviceRole_ != DeviceRole::OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("ConfigDistributedRoutingRole: not an output device");
        return ERR_INVALID_PARAM;
    }

    if (descriptor->networkId_ != LOCAL_NETWORK_ID &&
        descriptor->networkId_.size() != VALID_REMOTE_NETWORK_ID_LENGTH) {
        AUDIO_ERR_LOG("ConfigDistributedRoutingRole: invalid networkId");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = AudioPolicyManager::GetInstance().ConfigDistributedRoutingRole(descriptor, type);
    return ret;
}

int32_t AudioSystemManager::SetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetDistributedRoutingRoleCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    if (audioDistributedRoutingRoleCallback_ == nullptr) {
        audioDistributedRoutingRoleCallback_ = std::make_shared<AudioDistributedRoutingRoleCallbackImpl>();
        if (audioDistributedRoutingRoleCallback_ == nullptr) {
            AUDIO_ERR_LOG("AudioSystemManger failed to allocate memory for distributedRoutingRole callback");
            return ERROR;
        }
        int32_t ret = AudioPolicyManager::GetInstance().
            SetDistributedRoutingRoleCallback(audioDistributedRoutingRoleCallback_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("AudioSystemManger failed to set distributedRoutingRole callback");
            return ERROR;
        }
    }

    std::shared_ptr<AudioDistributedRoutingRoleCallbackImpl> cbImpl =
        std::static_pointer_cast<AudioDistributedRoutingRoleCallbackImpl>(audioDistributedRoutingRoleCallback_);
    if (cbImpl == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManger cbImpl is nullptr");
        return ERROR;
    }
    cbImpl->SaveCallback(callback);
    return SUCCESS;
}

int32_t AudioSystemManager::UnsetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    int32_t ret = AudioPolicyManager::GetInstance().UnsetDistributedRoutingRoleCallback();
    if (audioDistributedRoutingRoleCallback_ != nullptr) {
        audioDistributedRoutingRoleCallback_.reset();
        audioDistributedRoutingRoleCallback_ = nullptr;
    }

    std::shared_ptr<AudioDistributedRoutingRoleCallbackImpl> cbImpl =
        std::static_pointer_cast<AudioDistributedRoutingRoleCallbackImpl>(audioDistributedRoutingRoleCallback_);
    if (cbImpl == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManger cbImpl is nullptr");
        return ERROR;
    }
    cbImpl->RemoveCallback(callback);
    return ret;
}

void AudioDistributedRoutingRoleCallbackImpl::SaveCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    bool hasCallback = false;
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
        if ((*it) == callback) {
            hasCallback = true;
        }
    }
    if (!hasCallback) {
        callbackList_.push_back(callback);
    }
}

void AudioDistributedRoutingRoleCallbackImpl::RemoveCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    callbackList_.remove_if([&callback](std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback_) {
        return callback_ == callback;
    });
}

void AudioDistributedRoutingRoleCallbackImpl::OnDistributedRoutingRoleChange(
    std::shared_ptr<AudioDeviceDescriptor>descriptor, const CastType type)
{
    std::vector<std::shared_ptr<AudioDistributedRoutingRoleCallback>> temp_;
    std::unique_lock<mutex> cbListLock(cbListMutex_);
    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
        cb_ = (*callback);
        if (cb_ != nullptr) {
            AUDIO_DEBUG_LOG("OnDistributedRoutingRoleChange : Notify event to app complete");
            temp_.push_back(cb_);
        } else {
            AUDIO_ERR_LOG("OnDistributedRoutingRoleChange: callback is null");
        }
    }
    cbListLock.unlock();
    for (uint32_t i = 0; i < temp_.size(); i++) {
        temp_[i]->OnDistributedRoutingRoleChange(descriptor, type);
    }
    return;
}

AudioDistributedRoutingRoleCallbackImpl::AudioDistributedRoutingRoleCallbackImpl()
{
    AUDIO_INFO_LOG("AudioDistributedRoutingRoleCallbackImpl constructor");
}

AudioDistributedRoutingRoleCallbackImpl::~AudioDistributedRoutingRoleCallbackImpl()
{
    AUDIO_INFO_LOG("AudioDistributedRoutingRoleCallbackImpl destroy");
}

int32_t AudioSystemManager::SetCallDeviceActive(DeviceType deviceType, bool flag, std::string address,
    const int32_t clientUid) const
{
    int32_t uid = clientUid == -1 ? getuid() : clientUid;
    return (AudioPolicyManager::GetInstance().SetCallDeviceActive(static_cast<InternalDeviceType>(deviceType),
        flag, address, uid));
}

uint32_t AudioSystemManager::GetEffectLatency(const std::string &sessionId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    uint32_t latency = 0;
    int32_t res = gasp->GetEffectLatency(sessionId, latency);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "GetEffectLatency failed");
    return latency;
}

int32_t AudioSystemManager::DisableSafeMediaVolume()
{
    return AudioPolicyManager::GetInstance().DisableSafeMediaVolume();
}

int32_t AudioSystemManager::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    return AudioPolicyManager::GetInstance().InjectInterruption(networkId, event);
}

int32_t AudioSystemManager::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    return AudioPolicyManager::GetInstance().LoadSplitModule(splitArgs, networkId);
}

int32_t AudioSystemManager::SetVirtualCall(const bool isVirtual)
{
    return AudioPolicyManager::GetInstance().SetVirtualCall(isVirtual);
}

int32_t AudioSystemManager::SetQueryAllowedPlaybackCallback(
    const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetQueryAllowedPlaybackCallback(callback);
}

int32_t AudioSystemManager::SetBackgroundMuteCallback(
    const std::shared_ptr<AudioBackgroundMuteCallback> &callback)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetBackgroundMuteCallback(callback);
}

int32_t AudioSystemManager::OnVoiceWakeupState(bool state)
{
    AUDIO_INFO_LOG("%{public}d", state);
    return SUCCESS;
}

uint16_t AudioSystemManager::GetDmDeviceType() const
{
    return AudioPolicyManager::GetInstance().GetDmDeviceType();
}

int32_t AudioSystemManager::NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession)
{
    AUDIO_INFO_LOG("Set uid:%{public}d, pid:%{public}d, Session state:%{public}d", uid, pid, hasSession);
    return AudioPolicyManager::GetInstance().NotifySessionStateChange(uid, pid, hasSession);
}

int32_t AudioSystemManager::NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze)
{
    AUDIO_INFO_LOG("In");
    return AudioPolicyManager::GetInstance().NotifyFreezeStateChange(pidList, isFreeze);
}

int32_t AudioSystemManager::ResetAllProxy()
{
    AUDIO_INFO_LOG("RSS IN");
    return AudioPolicyManager::GetInstance().ResetAllProxy();
}

int32_t AudioSystemManager::NotifyProcessBackgroundState(const int32_t uid, const int32_t pid)
{
    AUDIO_INFO_LOG("RSS IN");
    return AudioPolicyManager::GetInstance().NotifyProcessBackgroundState(uid, pid);
}

int32_t AudioSystemManager::GetMaxVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("GetMaxVolumeByUsage for streamUsage [%{public}d]", streamUsage);
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC: {
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("streamUsage=%{public}d not supported", streamUsage);
            return ERR_NOT_SUPPORTED;
    }
    return AudioPolicyManager::GetInstance().GetMaxVolumeLevelByUsage(streamUsage);
}

int32_t AudioSystemManager::GetMinVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("GetMinVolumeByUsage for streamUsage [%{public}d]", streamUsage);
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC: {
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("streamUsage=%{public}d not supported", streamUsage);
            return ERR_NOT_SUPPORTED;
    }
    return AudioPolicyManager::GetInstance().GetMinVolumeLevelByUsage(streamUsage);
}

int32_t AudioSystemManager::GetVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_INFO_LOG("GetVolumeByUsage for streamUsage [%{public}d]", streamUsage);
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC: {
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("streamUsage=%{public}d not supported", streamUsage);
            return ERR_NOT_SUPPORTED;
    }
    return AudioPolicyManager::GetInstance().GetVolumeLevelByUsage(streamUsage);
}

int32_t AudioSystemManager::IsStreamMuteByUsage(StreamUsage streamUsage, bool &isMute)
{
    AUDIO_INFO_LOG("IsStreamMuteByUsage for streamUsage [%{public}d]", streamUsage);
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC: {
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("streamUsage=%{public}d not supported", streamUsage);
            return ERR_NOT_SUPPORTED;
    }
    isMute = AudioPolicyManager::GetInstance().GetStreamMuteByUsage(streamUsage);
    return SUCCESS;
}

float AudioSystemManager::GetVolumeInDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType deviceType)
{
    AUDIO_INFO_LOG("GetVolumeInDbByStream for streamUsage [%{public}d]", streamUsage);
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MUSIC:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC: {
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
            break;
        }
        default:
            AUDIO_ERR_LOG("streamUsage=%{public}d not supported", streamUsage);
            return ERR_NOT_SUPPORTED;
    }
    return AudioPolicyManager::GetInstance().GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
}

std::vector<AudioVolumeType> AudioSystemManager::GetSupportedAudioVolumeTypes()
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, {}, "No system App");
    AUDIO_INFO_LOG("enter AudioSystemManager::GetSupportedAudioVolumeTypes");
    return AudioPolicyManager::GetInstance().GetSupportedAudioVolumeTypes();
}

AudioVolumeType AudioSystemManager::GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, STREAM_DEFAULT, "No system App");
    AUDIO_INFO_LOG("enter AudioSystemManager::GetAudioVolumeTypeByStreamUsage");
    return AudioPolicyManager::GetInstance().GetAudioVolumeTypeByStreamUsage(streamUsage);
}

std::vector<StreamUsage> AudioSystemManager::GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, {}, "No system App");
    AUDIO_INFO_LOG("enter AudioSystemManager::GetStreamUsagesByVolumeType");
    return AudioPolicyManager::GetInstance().GetStreamUsagesByVolumeType(audioVolumeType);
}


int32_t AudioSystemManager::RegisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::set<StreamUsage> &streamUsages, const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("register StreamVolumeChangeCallback clientPid:%{public}d", clientPid);

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetStreamVolumeChangeCallback(clientPid, streamUsages, callback);
}

int32_t AudioSystemManager::UnregisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("unregister StreamVolumeChangeCallback clientPid:%{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnsetStreamVolumeChangeCallback(callback);
}

int32_t AudioSystemManager::CreateAudioWorkgroup()
{
    hasSystemPermission_ = PermissionUtil::VerifySelfPermission();
    sptr<AudioWorkgroupCallbackImpl> workgroup = new(std::nothrow) AudioWorkgroupCallbackImpl();
    if (workgroup == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient] workgroup is null");
        return ERROR;
    }

    auto callback = std::make_shared<AudioWorkgroupChangeCallbackImpl>();
    if (callback == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient]workgroupChangeCallback_ Allocation Failed");
        return ERROR;
    }
    workgroup->AddWorkgroupChangeCallback(callback);

    sptr<IRemoteObject> object = workgroup->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("[WorkgroupInClient] object is null");
        return ERROR;
    }

    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    int32_t workgroupId = 0;
    int32_t res = gasp->CreateAudioWorkgroup(getpid(), object, workgroupId);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS && workgroupId >= 0, AUDIO_ERR,
        "CreateAudioWorkgroup failed, res:%{public}d workgroupId:%{public}d", res, workgroupId);

    std::lock_guard<std::mutex> lock(workgroupPrioRecorderMutex_);
    workgroupPrioRecorderMap_.emplace(workgroupId, std::make_shared<WorkgroupPrioRecorder>(workgroupId));
    return workgroupId;
}

int32_t AudioSystemManager::ReleaseAudioWorkgroup(int32_t workgroupId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    int32_t ret = gasp->ReleaseAudioWorkgroup(getpid(), workgroupId);

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        if (recorder->RestoreGroupPrio(false) != AUDIO_OK) {
            AUDIO_ERR_LOG("[WorkgroupInClient] restore grp:%{public}d prio failed", workgroupId);
        } else {
            std::lock_guard<std::mutex> lock(workgroupPrioRecorderMutex_);
            workgroupPrioRecorderMap_.erase(workgroupId);
        }
    }

    int32_t pid = getpid();
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    startGroupPermissionMap_[pid].erase(workgroupId);
    if (startGroupPermissionMap_[pid].size() == 0) {
        startGroupPermissionMap_.erase(pid);
    }
    return ret;
}

int32_t AudioSystemManager::AddThreadToGroup(int32_t workgroupId, int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        recorder->RecordThreadPrio(tokenId);
    }

    return gasp->AddThreadToGroup(getpid(), workgroupId, tokenId);
}

int32_t AudioSystemManager::RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (recorder != nullptr) {
        if (recorder->RestoreThreadPrio(tokenId) != AUDIO_OK) {
            AUDIO_INFO_LOG("[WorkgroupInClient] restore thread:%{public}d prio failed", tokenId);
        }
    }

    return gasp->RemoveThreadFromGroup(getpid(), workgroupId, tokenId);
}

int32_t AudioSystemManager::ExcuteAudioWorkgroupPrioImprove(int32_t workgroupId,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    bool restoreByPermission = false;
    std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
    if (!recorder) {
        AUDIO_ERR_LOG("[WorkgroupInClient] GetRecorderByGrpId workgroupId:%{public}d failed", workgroupId);
        return AUDIO_ERR;
    }

    restoreByPermission = recorder->GetRestoreByPermission();
    if (needUpdatePrio || restoreByPermission) {
        const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
        CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
        int32_t ipcRet = gasp->ImproveAudioWorkgroupPrio(getpid(), threads);
        if (ipcRet != SUCCESS) {
            AUDIO_ERR_LOG("[WorkgroupInClient] change prio for grp:%{public}d failed, ret:%{public}d",
                workgroupId, ipcRet);
            return AUDIO_ERR;
        }
        needUpdatePrio = false;
        recorder->SetRestoreByPermission(false);
    }

    return AUDIO_OK;
}

int32_t AudioSystemManager::StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime,
    const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio)
{
    if (!IsValidToStartGroup(workgroupId)) {
        return AUDIO_ERR;
    }

    Trace trace("[WorkgroupInClient] StartGroup workgroupId:" + std::to_string(workgroupId) +
        " startTime:" + std::to_string(startTime) + " deadlineTime:" + std::to_string(deadlineTime));
    CHECK_AND_RETURN_RET_LOG(deadlineTime > startTime, ERR_INVALID_PARAM, "Invalid Audio Deadline params");
    RME::SetFrameRateAndPrioType(workgroupId, MS_PER_SECOND/(deadlineTime - startTime), 0);

    if (ExcuteAudioWorkgroupPrioImprove(workgroupId, threads, needUpdatePrio) != AUDIO_OK) {
        AUDIO_ERR_LOG("[WorkgroupInClient] excute audioworkgroup prio improve failed");
        return AUDIO_ERR;
    }

    if (RME::BeginFrameFreq(deadlineTime - startTime) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInClient] Audio Deadline BeginFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

int32_t AudioSystemManager::StopGroup(int32_t workgroupId)
{
    if (EndFrameFreq(0) != 0) {
        AUDIO_ERR_LOG("[WorkgroupInClient] Audio Deadline EndFrame failed");
        return AUDIO_ERR;
    }
    return AUDIO_OK;
}

void AudioSystemManager::AudioWorkgroupChangeCallbackImpl::OnWorkgroupChange(
    const AudioWorkgroupChangeInfo &info)
{
    AudioSystemManager::GetInstance()->OnWorkgroupChange(info);
}

void AudioSystemManager::OnWorkgroupChange(const AudioWorkgroupChangeInfo &info)
{
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    startGroupPermissionMap_[info.pid][info.groupId] = info.startAllowed;
    std::vector<int32_t> workgroupIdNeedRestore;

    for (const auto &pair : startGroupPermissionMap_) {
        uint32_t pid = pair.first;
        const std::unordered_map<uint32_t, bool>& permissions = pair.second;
        for (const auto &innerPair : permissions) {
            uint32_t grpId = innerPair.first;
            bool permissionValue = innerPair.second;
            AUDIO_INFO_LOG("[WorkgroupInClient] pid = %{public}d, groupId = %{public}d, startAllowed = %{public}d",
                pid, grpId, permissionValue);
            if (permissionValue == false) {
                workgroupIdNeedRestore.push_back(grpId);
            }
        }
    }

    for (const auto &workgroupId : workgroupIdNeedRestore) {
        std::shared_ptr<WorkgroupPrioRecorder> recorder = GetRecorderByGrpId(workgroupId);
        if (recorder != nullptr) {
            if (recorder->RestoreGroupPrio(true) != AUDIO_OK) {
                AUDIO_INFO_LOG("[WorkgroupInClient] restore grp:%{public}d prio in cb failed", workgroupId);
            }
        }
    }
}

bool AudioSystemManager::IsValidToStartGroup(int32_t workgroupId)
{
    if (hasSystemPermission_) {
        return true;
    }

    int32_t pid = getpid();
    std::lock_guard<std::mutex> lock(startGroupPermissionMapMutex_);
    auto outerIt = startGroupPermissionMap_.find(pid);
    if (outerIt == startGroupPermissionMap_.end()) {
        return false;
    }
    const auto& innerMap = outerIt->second;
    for (const auto& pair : innerMap) {
        if (pair.second) {
            return true;
        }
    }
    return false;
}

int32_t AudioSystemManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    AUDIO_INFO_LOG("volumeType:%{public}d, duration:%{public}d", volumeType, duration);
    return AudioPolicyManager::GetInstance().ForceVolumeKeyControlType(volumeType, duration);
}

int32_t AudioSystemManager::SetRenderWhitelist(std::vector<std::string> list)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->SetRenderWhitelist(list);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "failed: %{public}d", ret);
    return ret;
}

AudioSystemManager::WorkgroupPrioRecorder::WorkgroupPrioRecorder(int32_t grpId)
{
    grpId_ = grpId;
    restoreByPermission_ = false;
}
 
void AudioSystemManager::WorkgroupPrioRecorder::SetRestoreByPermission(bool isByPermission)
{
    restoreByPermission_ = isByPermission;
}
 
bool AudioSystemManager::WorkgroupPrioRecorder::GetRestoreByPermission()
{
    return restoreByPermission_;
}
 
void AudioSystemManager::WorkgroupPrioRecorder::RecordThreadPrio(int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    auto it = threads_.find(tokenId);
    if (it == threads_.end()) {
        OHOS::QOS::QosLevel qosLevel;
        int32_t ret = OHOS::QOS::GetThreadQos(qosLevel);
        threads_[tokenId] = (ret == -1 ? -1 : (int32_t)qosLevel);
    }
}
 
int32_t AudioSystemManager::WorkgroupPrioRecorder::RestoreGroupPrio(bool isByPermission)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");

    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    if (gasp->RestoreAudioWorkgroupPrio(getpid(), threads_) != AUDIO_OK) {
        AUDIO_ERR_LOG("[WorkgroupInClient] restore prio for workgroupId:%{public}d failed", GetGrpId());
        return AUDIO_ERR;
    }

    if (!isByPermission) {
        threads_.clear();
    } else {
        restoreByPermission_ = true;
    }
    return AUDIO_OK;
}
 
int32_t AudioSystemManager::WorkgroupPrioRecorder::RestoreThreadPrio(int32_t tokenId)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::lock_guard<std::mutex> lock(workgroupThreadsMutex_);
    auto it = threads_.find(tokenId);
    int ipcRet;
    if (it != threads_.end()) {
        std::unordered_map<int32_t, int32_t> thread = {{it->first, it->second}};
        ipcRet = gasp->RestoreAudioWorkgroupPrio(getpid(), thread);
        if (ipcRet != SUCCESS) {
            AUDIO_ERR_LOG("[WorkgroupInClient] change prio for tokenId:%{public}d failed, ret:%{public}d",
                tokenId, ipcRet);
            return AUDIO_ERR;
        } else {
            threads_.erase(tokenId);
        }
    }
    return AUDIO_OK;
}
 
int32_t AudioSystemManager::WorkgroupPrioRecorder::GetGrpId()
{
    return grpId_;
}
 
std::shared_ptr<AudioSystemManager::WorkgroupPrioRecorder> AudioSystemManager::GetRecorderByGrpId(int32_t grpId)
{
    std::lock_guard<std::mutex> lock(workgroupPrioRecorderMutex_);
    auto it = workgroupPrioRecorderMap_.find(grpId);
    if (it != workgroupPrioRecorderMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void AudioSystemManager::CleanUpResource()
{
    AudioPolicyManager::GetInstance().CleanUpResource();
}
} // namespace AudioStandard
} // namespace OHOS
