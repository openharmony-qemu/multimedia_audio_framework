/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_volume_client_manager.h"
#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_service_proxy.h"
#include "audio_utils.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {
AudioVolumeClientManager &AudioVolumeClientManager::GetInstance()
{
    static AudioVolumeClientManager instance;
    return instance;
}

AudioVolumeClientManager::AudioVolumeClientManager()
{
}

AudioVolumeClientManager::~AudioVolumeClientManager()
{
    if (volumeChangeClientPid_ != -1) {
        AUDIO_DEBUG_LOG("UnregisterVolumeKeyEventCallback");
        (void)UnregisterVolumeKeyEventCallback(volumeChangeClientPid_);
        (void)UnregisterStreamVolumeChangeCallback(volumeChangeClientPid_);
        (void)UnregisterSystemVolumeChangeCallback(volumeChangeClientPid_);
        (void)UnregisterVolumeDegreeCallback(volumeChangeClientPid_);
    }
}

int32_t AudioVolumeClientManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel, int32_t uid)
{
    HILOG_COMM_INFO("SetSystemVolume: volumeType[%{public}d], volumeLevel[%{public}d]", volumeType, volumeLevel);
    std::lock_guard<std::mutex> lock(volumeMutex_);

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
    return AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, uid == 0, 0, uid);
}

int32_t AudioVolumeClientManager::SetVolumeWithDevice(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType)
{
    AUDIO_INFO_LOG("%{public}s: volumeType[%{public}d], volumeLevel[%{public}d], deviceType[%{public}d]",
        __func__, volumeType, volumeLevel, deviceType);
    std::lock_guard<std::mutex> lock(volumeMutex_);

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

int32_t AudioVolumeClientManager::GetVolume(AudioVolumeType volumeType, int32_t uid) const
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

    return AudioPolicyManager::GetInstance().GetSystemVolumeLevel(volumeType, uid);
}

int32_t AudioVolumeClientManager::SetLowPowerVolume(int32_t streamId, float volume) const
{
    AUDIO_INFO_LOG("streamId:%{public}d, vol:%{public}f.", streamId, volume);
    CHECK_AND_RETURN_RET_LOG((volume >= 0) && (volume <= 1.0), ERR_INVALID_PARAM,
        "Invalid Volume Input!");

    return AudioPolicyManager::GetInstance().SetLowPowerVolume(streamId, volume);
}

float AudioVolumeClientManager::GetLowPowerVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetLowPowerVolume(streamId);
}

float AudioVolumeClientManager::GetSingleStreamVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetSingleStreamVolume(streamId);
}

int32_t AudioVolumeClientManager::GetMaxVolume(AudioVolumeType volumeType)
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

int32_t AudioVolumeClientManager::GetMinVolume(AudioVolumeType volumeType)
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

int32_t AudioVolumeClientManager::GetDeviceMaxVolume(AudioVolumeType volumeType, DeviceType deviceType)
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

int32_t AudioVolumeClientManager::GetDeviceMinVolume(AudioVolumeType volumeType, DeviceType deviceType)
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

int32_t AudioVolumeClientManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType)
{
    AUDIO_INFO_LOG("SetStreamMute for volumeType [%{public}d], mute [%{public}d]", volumeType, mute);
    std::lock_guard<std::mutex> lock(volumeMutex_);
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

bool AudioVolumeClientManager::IsStreamMute(AudioVolumeType volumeType) const
{
    AUDIO_DEBUG_LOG("AudioVolumeClientManager::GetMute Client");

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

float AudioVolumeClientManager::GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device)
{
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::GetVolumeInUnitOfDb");
    return AudioPolicyManager::GetInstance().GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

int32_t AudioVolumeClientManager::SetMicrophoneMute(bool isMute)
{
    return AudioPolicyManager::GetInstance().SetMicrophoneMute(isMute);
}

int32_t AudioVolumeClientManager::SetVoiceRingtoneMute(bool isMute)
{
    AUDIO_INFO_LOG("Set Voice Ringtone is %{public}d", isMute);
    return AudioPolicyManager::GetInstance().SetVoiceRingtoneMute(isMute);
}

bool AudioVolumeClientManager::IsMicrophoneMute()
{
    std::shared_ptr<AudioGroupManager> groupManager = GetGroupManager(DEFAULT_VOLUME_GROUP_ID);
    CHECK_AND_RETURN_RET_LOG(groupManager != nullptr, false, "failed, groupManager is null");
    return groupManager->IsMicrophoneMuteLegacy();
}

std::shared_ptr<AudioGroupManager> AudioVolumeClientManager::GetGroupManager(int32_t groupId)
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

int32_t AudioVolumeClientManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("AudioVolumeClientManager RegisterVolumeKeyEventCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterVolumeKeyEventCallbackcallback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback, api_v);
}

int32_t AudioVolumeClientManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnregisterVolumeKeyEventCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("UnsetVolumeKeyEventCallback success");
    }
    return ret;
}

int32_t AudioVolumeClientManager::RegisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("register");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeDegreeCallback(clientPid, callback, api_v);
}

int32_t AudioVolumeClientManager::UnregisterVolumeDegreeCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_DEBUG_LOG("unregister");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeDegreeCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("success");
    }
    return ret;
}

int32_t AudioVolumeClientManager::RegisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioVolumeClientManager RegisterSystemVolumeChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterSystemVolumeChangeCallback callback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetSystemVolumeChangeCallback(clientPid, callback);
}

int32_t AudioVolumeClientManager::UnregisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("UnregisterSystemVolumeChangeCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetSystemVolumeChangeCallback(callback);
    if (!ret) {
        AUDIO_DEBUG_LOG("UnsetSystemVolumeChangeCallback success");
    }
    return ret;
}

int32_t AudioVolumeClientManager::GetVolumeBySessionId(const uint32_t &sessionId, float &volume)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->GetVolumeBySessionId(sessionId, volume);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioVolumeClientManager::SetVolumeDegree(AudioVolumeType volumeType, int32_t degree, int32_t uid)
{
    AUDIO_INFO_LOG("volumeType[%{public}d], volumeDegree[%{public}d]", volumeType, degree);

    /* Validate volume type and return INVALID_PARAMS error */
    switch (volumeType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_RING:
        case STREAM_MUSIC:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_ASSISTANT:
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

    return AudioPolicyManager::GetInstance().SetSystemVolumeDegree(volumeType, degree, 0, uid);
}

int32_t AudioVolumeClientManager::GetVolumeDegree(AudioVolumeType volumeType, int32_t uid)
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

    return AudioPolicyManager::GetInstance().GetSystemVolumeDegree(volumeType, uid);
}

int32_t AudioVolumeClientManager::GetMinVolumeDegree(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL ||
        volumeType == STREAM_ULTRASONIC) {
        bool ret = PermissionUtil::VerifySelfPermission();
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
            "volumeType=%{public}d has no system permission", volumeType);
    }

    return AudioPolicyManager::GetInstance().GetMinVolumeDegree(volumeType);
}

int32_t AudioVolumeClientManager::GetMaxVolumeByUsage(StreamUsage streamUsage)
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

int32_t AudioVolumeClientManager::GetMinVolumeByUsage(StreamUsage streamUsage)
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

int32_t AudioVolumeClientManager::GetVolumeByUsage(StreamUsage streamUsage)
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

int32_t AudioVolumeClientManager::IsStreamMuteByUsage(StreamUsage streamUsage, bool &isMute)
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

float AudioVolumeClientManager::GetVolumeInDbByStream(StreamUsage streamUsage,
    int32_t volumeLevel, DeviceType deviceType)
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

std::vector<AudioVolumeType> AudioVolumeClientManager::GetSupportedAudioVolumeTypes()
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, {}, "No system App");
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::GetSupportedAudioVolumeTypes");
    return AudioPolicyManager::GetInstance().GetSupportedAudioVolumeTypes();
}

AudioVolumeType AudioVolumeClientManager::GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, STREAM_DEFAULT, "No system App");
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::GetAudioVolumeTypeByStreamUsage");
    return AudioPolicyManager::GetInstance().GetAudioVolumeTypeByStreamUsage(streamUsage);
}

std::vector<StreamUsage> AudioVolumeClientManager::GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, {}, "No system App");
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::GetStreamUsagesByVolumeType");
    return AudioPolicyManager::GetInstance().GetStreamUsagesByVolumeType(audioVolumeType);
}


int32_t AudioVolumeClientManager::RegisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::set<StreamUsage> &streamUsages, const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("register StreamVolumeChangeCallback clientPid:%{public}d", clientPid);

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetStreamVolumeChangeCallback(clientPid, streamUsages, callback);
}

int32_t AudioVolumeClientManager::UnregisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_DEBUG_LOG("unregister StreamVolumeChangeCallback clientPid:%{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnsetStreamVolumeChangeCallback(callback);
}

int32_t AudioVolumeClientManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support,
    int32_t volume)
{
    AUDIO_INFO_LOG("AudioVolumeClientManager::SetDeviceAbsVolumeSupported");
    return AudioPolicyManager::GetInstance().SetDeviceAbsVolumeSupported(macAddress, support, volume);
}

int32_t AudioVolumeClientManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    return AudioPolicyManager::GetInstance().SetAdjustVolumeForZone(zoneId);
}

int32_t AudioVolumeClientManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    AUDIO_INFO_LOG("volume: %{public}d, update ui: %{public}d", volume, updateUi);
    return AudioPolicyManager::GetInstance().SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

int32_t AudioVolumeClientManager::SetNearlinkDeviceVolume(const std::string &macAddress, AudioVolumeType volumeType,
    const int32_t volume, const bool updateUi)
{
    AUDIO_INFO_LOG("volume: %{public}d, update ui: %{public}d", volume, updateUi);
    return AudioPolicyManager::GetInstance().SetNearlinkDeviceVolume(macAddress, volumeType, volume, updateUi);
}

int32_t AudioVolumeClientManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
}


int32_t AudioVolumeClientManager::SetSelfAppVolume(int32_t volume, int32_t flag)
{
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::SetSelfAppVolume");
    return AudioPolicyManager::GetInstance().SetSelfAppVolumeLevel(volume);
}

// LCOV_EXCL_START
int32_t AudioVolumeClientManager::SetAppVolume(int32_t appUid, int32_t volume, int32_t flag)
{
    AUDIO_INFO_LOG("enter AudioSystemManager::SetAppVolume");
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "SetAppVolume: No system permission");
    return AudioPolicyManager::GetInstance().SetAppVolumeLevel(appUid, volume);
}

int32_t AudioVolumeClientManager::GetAppVolume(int32_t appUid, int32_t &volumeLevel) const
{
    AUDIO_INFO_LOG("enter AudioSystemManager::GetAppVolume");
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "GetAppVolume: No system permission");
    return AudioPolicyManager::GetInstance().GetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioVolumeClientManager::GetSelfAppVolume(int32_t &volumeLevel) const
{
    AUDIO_INFO_LOG("enter AudioVolumeClientManager::GetSelfAppVolume");
    return AudioPolicyManager::GetInstance().GetSelfAppVolumeLevel(volumeLevel);
}

int32_t AudioVolumeClientManager::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    AUDIO_INFO_LOG("SetAppVolumeMuted: appUid[%{public}d], muted[%{public}d], flag[%{public}d]",
        appUid, muted, volumeFlag);
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "SetAppVolumeMuted: No system permission");
    return AudioPolicyManager::GetInstance().SetAppVolumeMuted(appUid, muted, volumeFlag);
}

int32_t AudioVolumeClientManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    bool ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "SetAppRingMuted: No system permission");
    return AudioPolicyManager::GetInstance().SetAppRingMuted(appUid, muted);
}
// LCOV_EXCL_STOP

int32_t AudioVolumeClientManager::UnsetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetSelfAppVolumeCallback(callback);
}

int32_t AudioVolumeClientManager::SetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetSelfAppVolumeCallback: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetSelfAppVolumeChangeCallback(callback);
}

int32_t AudioVolumeClientManager::SetAppVolumeCallbackForUid(const int32_t appUid,
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetAppVolumeCallbackForUid: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAppVolumeChangeCallbackForUid(appUid, callback);
}

int32_t AudioVolumeClientManager::UnsetAppVolumeCallbackForUid(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetAppVolumeCallbackForUid(callback);
}

int32_t AudioVolumeClientManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("IsAppVolumeMute: appUid[%{public}d], muted[%{public}d]", appUid, owned);
    bool ret = PermissionUtil::VerifyIsSystemApp();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "IsAppVolumeMute: No system permission");
    ret = PermissionUtil::VerifySelfPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "IsAppVolumeMute: No system permission");
    return AudioPolicyManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

int32_t AudioVolumeClientManager::UnsetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetActiveVolumeTypeCallback(callback);
}

int32_t AudioVolumeClientManager::SetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetActiveVolumeTypeCallback: callback is nullptr");
    return AudioPolicyManager::GetInstance().SetActiveVolumeTypeCallback(callback);
}
} // namespace AudioStandard
} // namespace OHOS
