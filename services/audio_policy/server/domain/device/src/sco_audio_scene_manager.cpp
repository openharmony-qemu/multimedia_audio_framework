/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "ScoAudioSceneManager"
#endif

#include "sco_audio_scene_manager.h"
#include "audio_policy_log.h"
#include "audio_info.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87

namespace OHOS {
namespace AudioStandard {

ScoAudioSceneManager::ScoAudioSceneManager()
{
    AUDIO_INFO_LOG("ScoAudioSceneManager constructed");
}

ScoAudioSceneManager::~ScoAudioSceneManager()
{
    ClearAllScoStreams();
}

ScoAudioScenePriority ScoAudioSceneManager::DeterminePriority(AudioScene audioScene, bool isRecognition) const
{
    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        return SCO_PRIORITY_CELLULAR;
    }
    
    if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
        return SCO_PRIORITY_VOIP;
    }
    
    if (audioScene == AUDIO_SCENE_VOICE_RINGING) {
        return SCO_PRIORITY_VOIP;
    }
    
    if (audioScene == AUDIO_SCENE_RINGING) {
        return SCO_PRIORITY_VOIP;
    }
    
    if (isRecognition) {
        return SCO_PRIORITY_VOICE_RECOGNITION;
    }
    
    return SCO_PRIORITY_FORCE_SCO;
}

void ScoAudioSceneManager::AddScoStream(uint32_t streamId, AudioScene audioScene, int32_t uid, bool isRecognition)
{
    std::lock_guard<std::mutex> lock(scoMutex_);
    
    ScoAudioSceneInfo info;
    info.audioScene = audioScene;
    info.streamId = streamId;
    info.uid = uid;
    info.isRecognition = isRecognition;
    info.priority = DeterminePriority(audioScene, isRecognition);
    
    scoStreamsMap_[streamId] = info;
    
    AUDIO_INFO_LOG("Add SCO stream %{public}u, audioScene %{public}d, priority %{public}d, uid %{public}d, "
        "isRecognition %{public}d", streamId, audioScene, info.priority, uid, isRecognition);
}

void ScoAudioSceneManager::RemoveScoStream(uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(scoMutex_);
    
    auto it = scoStreamsMap_.find(streamId);
    if (it != scoStreamsMap_.end()) {
        AUDIO_INFO_LOG("Remove SCO stream %{public}u, audioScene %{public}d, priority %{public}d",
            streamId, it->second.audioScene, it->second.priority);
        scoStreamsMap_.erase(it);
    }
}

AudioScene ScoAudioSceneManager::GetHighestPriorityAudioScene() const
{
    std::lock_guard<std::mutex> lock(scoMutex_);
    
    if (scoStreamsMap_.empty()) {
        return AUDIO_SCENE_DEFAULT;
    }
    
    ScoAudioSceneInfo highestPriorityInfo;
    highestPriorityInfo.priority = SCO_PRIORITY_DEFAULT;
    highestPriorityInfo.audioScene = AUDIO_SCENE_DEFAULT;
    
    for (const auto &pair : scoStreamsMap_) {
        const ScoAudioSceneInfo &info = pair.second;
        if (info.priority < highestPriorityInfo.priority) {
            highestPriorityInfo = info;
        }
    }
    
    AUDIO_INFO_LOG("Get highest priority audioScene %{public}d, priority %{public}d",
        highestPriorityInfo.audioScene, highestPriorityInfo.priority);
    
    return highestPriorityInfo.audioScene;
}

ScoAudioScenePriority ScoAudioSceneManager::GetAudioScenePriority(AudioScene audioScene, bool isRecognition) const
{
    return DeterminePriority(audioScene, isRecognition);
}

bool ScoAudioSceneManager::HasScoStream() const
{
    std::lock_guard<std::mutex> lock(scoMutex_);
    return !scoStreamsMap_.empty();
}

void ScoAudioSceneManager::ClearAllScoStreams()
{
    std::lock_guard<std::mutex> lock(scoMutex_);
    AUDIO_INFO_LOG("Clear all SCO streams, count %{public}zu", scoStreamsMap_.size());
    scoStreamsMap_.clear();
}

}
}