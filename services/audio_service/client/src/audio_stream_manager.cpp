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
#define LOG_TAG "AudioStreamManager"
#endif

#include "audio_stream_manager.h"

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_manager_util.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "i_audio_stream.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
static const std::map<std::string, AudioEffectMode> effectModeMap = {
    {"EFFECT_NONE", EFFECT_NONE},
    {"EFFECT_DEFAULT", EFFECT_DEFAULT}
};
AudioStreamManager *AudioStreamManager::GetInstance()
{
    static AudioStreamManager audioStreamManager;
    return &audioStreamManager;
}

int32_t AudioStreamManager::RegisterAudioRendererEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("client id: %{public}d", clientPid);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    std::lock_guard<std::mutex> lock(rendererStateChangeCallbacksMutex_);
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);

    rendererStateChangeCallbacks_.push_back(callback);
    return ret;
}

int32_t AudioStreamManager::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    AUDIO_INFO_LOG("client id: %{public}d", clientPid);

    std::lock_guard<std::mutex> lock(rendererStateChangeCallbacksMutex_);
    int32_t ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(
        rendererStateChangeCallbacks_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);

    rendererStateChangeCallbacks_.clear();
    return ret;
}

int32_t AudioStreamManager::RegisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");

    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);

    return ret;
}

int32_t AudioStreamManager::UnregisterAudioRendererEventListener(
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");

    int32_t ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);

    return ret;
}

int32_t AudioStreamManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("client id: %{public}d", clientPid);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientPid, callback);
}

int32_t AudioStreamManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    AUDIO_INFO_LOG("client id: %{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientPid);
}

int32_t AudioStreamManager::GetCurrentRendererChangeInfos(
    vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioStreamManager::GetCurrentCapturerChangeInfos(
    vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

static void UpdateEffectInfoArray(SupportedEffectConfig &supportedEffectConfig,
    int32_t i, AudioSceneEffectInfo &audioSceneEffectInfo)
{
    uint32_t j;
    AudioEffectMode audioEffectMode;
    for (j = 0; j < supportedEffectConfig.postProcessNew.stream[i].streamEffectMode.size(); j++) {
        audioEffectMode = effectModeMap.at(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].mode);
        audioSceneEffectInfo.mode.push_back(audioEffectMode);
    }
    auto index = std::find(audioSceneEffectInfo.mode.begin(), audioSceneEffectInfo.mode.end(), 0);
    if (index == audioSceneEffectInfo.mode.end()) {
        audioEffectMode = effectModeMap.at("EFFECT_NONE");
        audioSceneEffectInfo.mode.push_back(audioEffectMode);
    }
    index = std::find(audioSceneEffectInfo.mode.begin(), audioSceneEffectInfo.mode.end(), 1);
    if (index == audioSceneEffectInfo.mode.end()) {
        audioEffectMode = effectModeMap.at("EFFECT_DEFAULT");
        audioSceneEffectInfo.mode.push_back(audioEffectMode);
    }
    std::sort(audioSceneEffectInfo.mode.begin(), audioSceneEffectInfo.mode.end());
}

int32_t AudioStreamManager::GetEffectInfoArray(AudioSceneEffectInfo &audioSceneEffectInfo, StreamUsage streamUsage)
{
    std::string effectScene = AudioManagerUtil::GetEffectSceneName(streamUsage);
    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    uint32_t streamNum = supportedEffectConfig.postProcessNew.stream.size();
    if (streamNum >= 0) {
        int32_t sceneFlag = 0;
        for (uint32_t i = 0; i < streamNum; i++) {
            if (effectScene == supportedEffectConfig.postProcessNew.stream[i].scene) {
                UpdateEffectInfoArray(supportedEffectConfig, i, audioSceneEffectInfo);
                sceneFlag = 1;
                break;
            }
        }
        if (sceneFlag == 0) {
            AudioEffectMode audioEffectMode = effectModeMap.at("EFFECT_NONE");
            audioSceneEffectInfo.mode.push_back(audioEffectMode);
            audioEffectMode = effectModeMap.at("EFFECT_DEFAULT");
            audioSceneEffectInfo.mode.push_back(audioEffectMode);
        }
    }
    return ret;
}

bool AudioStreamManager::IsStreamActive(AudioVolumeType volumeType) const
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
        case STREAM_NAVIGATION:
            break;
        case STREAM_ULTRASONIC:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, false, "volumeType=%{public}d. No system permission", volumeType);
            break;
        }
        case STREAM_ALL:
        default:
            AUDIO_ERR_LOG("IsStreamActive: volumeType=%{public}d not supported", volumeType);
            return false;
    }

    return AudioPolicyManager::GetInstance().IsStreamActive(volumeType);
}

bool AudioStreamManager::IsStreamActiveByStreamUsage(StreamUsage streamUsage) const
{
    switch (streamUsage) {
        case STREAM_USAGE_MEDIA:
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
        case STREAM_USAGE_RANGING:
        case STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case STREAM_USAGE_VOICE_RINGTONE:
            break;
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_VOICE_CALL_ASSISTANT:
        case STREAM_USAGE_ULTRASONIC:{
            bool ret = PermissionUtil::VerifySelfPermission();
            CHECK_AND_RETURN_RET_LOG(ret, false, "streamUsage=%{public}d. No system permission", streamUsage);
            break;
        }
        default:
            AUDIO_ERR_LOG("IsStreamActiveByStreamUsage: streamUsage=%{public}d not supported", streamUsage);
            return false;
    }
    return AudioPolicyManager::GetInstance().IsStreamActiveByStreamUsage(streamUsage);
}

bool AudioStreamManager::IsFastPlaybackSupported(AudioStreamInfo &streamInfo, StreamUsage usage)
{
    return AudioPolicyManager::GetInstance().IsFastPlaybackSupported(streamInfo, usage);
}

bool AudioStreamManager::IsFastRecordingSupported(AudioStreamInfo &streamInfo, SourceType source)
{
    return AudioPolicyManager::GetInstance().IsFastRecordingSupported(streamInfo, source);
}

int32_t AudioStreamManager::GetHardwareOutputSamplingRate(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    int32_t result = 0;

    if (desc == nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_INVALID_PARAM, "GetHardwareOutputSamplingRate fail");
        desc->deviceType_ = DEVICE_TYPE_SPEAKER;
        desc->deviceRole_ = OUTPUT_DEVICE;
    }

    result = AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
    return result;
}

DirectPlaybackMode AudioStreamManager::GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo,
    const StreamUsage &streamUsage)
{
    CHECK_AND_RETURN_RET_LOG(streamUsage > STREAM_USAGE_UNKNOWN && streamUsage < STREAM_USAGE_MAX,
        DIRECT_PLAYBACK_NOT_SUPPORTED, "invalid streamUsage: %{public}d", streamUsage);
    return AudioPolicyManager::GetInstance().GetDirectPlaybackSupport(streamInfo, streamUsage);
}

int32_t AudioStreamManager::SetAudioFormatUnsupportedErrorCallback(
    const std::shared_ptr<AudioFormatUnsupportedErrorCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioFormatUnsupportedErrorCallback(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);
    return ret;
}

int32_t AudioStreamManager::UnsetAudioFormatUnsupportedErrorCallback()
{
    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioFormatUnsupportedErrorCallback();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "ret: %{public}d", ret);
    return ret;
}

int32_t AudioStreamManager::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioStreamManager::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetSupportedAudioEnhanceProperty(propertyArray);
}

int32_t AudioStreamManager::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().SetAudioEffectProperty(propertyArray);
}

int32_t AudioStreamManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetAudioEffectProperty(propertyArray);
}

int32_t AudioStreamManager::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().SetAudioEnhanceProperty(propertyArray);
}

int32_t AudioStreamManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    return AudioPolicyManager::GetInstance().GetAudioEnhanceProperty(propertyArray);
}

bool AudioStreamManager::IsAcousticEchoCancelerSupported(SourceType sourceType)
{
    return AudioPolicyManager::GetInstance().IsAcousticEchoCancelerSupported(sourceType);
}

int32_t AudioStreamManager::ForceStopAudioStream(StopAudioType audioType)
{
    return AudioPolicyManager::GetInstance().ForceStopAudioStream(audioType);
}

bool AudioStreamManager::IsCapturerFocusAvailable(const AudioCapturerInfo &capturerInfo)
{
    return AudioPolicyManager::GetInstance().IsCapturerFocusAvailable(capturerInfo);
}

bool AudioStreamManager::IsAudioLoopbackSupported(AudioLoopbackMode mode)
{
    return AudioPolicyManager::GetInstance().IsAudioLoopbackSupported(mode);
}
} // namespace AudioStandard
} // namespace OHOS
