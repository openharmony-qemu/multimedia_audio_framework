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
#define LOG_TAG "AudioStreamManager"
#endif

#include "audio_stream_manager.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "i_audio_stream.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
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
    vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioStreamManager::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

bool AudioStreamManager::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_DEBUG_LOG("IsAudioRendererLowLatencySupported");
    return AudioPolicyManager::GetInstance().IsAudioRendererLowLatencySupported(audioStreamInfo);
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
    std::string effectScene = IAudioStream::GetEffectSceneName(streamUsage);
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
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_RING:
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

int32_t AudioStreamManager::GetHardwareOutputSamplingRate(sptr<AudioDeviceDescriptor> &desc)
{
    int32_t result = 0;

    if (desc == nullptr) {
        sptr<AudioDeviceDescriptor> desc = new (std::nothrow) AudioDeviceDescriptor();
        desc->deviceType_ = DEVICE_TYPE_SPEAKER;
        desc->deviceRole_ = OUTPUT_DEVICE;
    }

    result = AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
    return result;
}
} // namespace AudioStandard
} // namespace OHOS
