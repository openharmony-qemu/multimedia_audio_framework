/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEffectVolume"
#endif

#include "audio_effect_volume.h"
#include "audio_effect_log.h"

namespace OHOS {
namespace AudioStandard {
AudioEffectVolume::AudioEffectVolume()
{
    AUDIO_DEBUG_LOG("created!");
    SceneTypeToVolumeMap_.clear();
    SceneTypeToSystemVolumeMap_ = {
        {"SCENE_RING", 1.0f},
        {"SCENE_SPEECH", 1.0f},
        {"SCENE_MUSIC", 1.0f},
        {"SCENE_OTHERS", 1.0f},
    };
}

AudioEffectVolume::~AudioEffectVolume()
{
    AUDIO_DEBUG_LOG("destructor!");
}

std::shared_ptr<AudioEffectVolume> AudioEffectVolume::GetInstance()
{
    static std::shared_ptr<AudioEffectVolume> effectVolume = std::make_shared<AudioEffectVolume>();
    return effectVolume;
}

void AudioEffectVolume::SetSystemVolume(const std::string sceneType, const float systemVolume)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    AUDIO_DEBUG_LOG("systemVolume: %{public}f", systemVolume);
    SceneTypeToSystemVolumeMap_[sceneType] = systemVolume;
}

float AudioEffectVolume::GetSystemVolume(const std::string sceneType)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    auto it = SceneTypeToSystemVolumeMap_.find(sceneType);
    if (it == SceneTypeToSystemVolumeMap_.end()) {
        return SceneTypeToSystemVolumeMap_["SCENE_MUSIC"];
    } else {
        return SceneTypeToSystemVolumeMap_[sceneType];
    }
}

void AudioEffectVolume::SetStreamVolume(const std::string sessionID, const float streamVolume)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    AUDIO_DEBUG_LOG("SetStreamVolume: %{public}f", streamVolume);
    SessionIDToVolumeMap_[sessionID] = streamVolume;
}

float AudioEffectVolume::GetStreamVolume(const std::string sessionID)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    if (!SessionIDToVolumeMap_.count(sessionID)) {
        return 1.0;
    } else {
        return SessionIDToVolumeMap_[sessionID];
    }
}

int32_t AudioEffectVolume::StreamVolumeDelete(const std::string sessionID)
{
    std::lock_guard<std::mutex> lock(volumeMutex_);
    if (!SessionIDToVolumeMap_.count(sessionID)) {
        return 0;
    } else {
        SessionIDToVolumeMap_.erase(sessionID);
        return 0;
    }
}

void AudioEffectVolume::SetDspVolume(const float volume)
{
    AUDIO_DEBUG_LOG("setDspVolume: %{public}f", volume);
    dspVolume_ = volume;
}

float AudioEffectVolume::GetDspVolume()
{
    return dspVolume_;
}
}  // namespace AudioStandard
}  // namespace OHOS