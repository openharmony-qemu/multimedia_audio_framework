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

#ifndef SCO_AUDIO_SCENE_MANAGER_H
#define SCO_AUDIO_SCENE_MANAGER_H

#include <mutex>
#include <unordered_map>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

enum ScoAudioScenePriority {
    SCO_PRIORITY_CELLULAR = 0,
    SCO_PRIORITY_VOIP = 1,
    SCO_PRIORITY_VOICE_RECOGNITION = 2,
    SCO_PRIORITY_FORCE_SCO = 3,
    SCO_PRIORITY_DEFAULT = 4
};

struct ScoAudioSceneInfo {
    AudioScene audioScene;
    ScoAudioScenePriority priority;
    uint32_t streamId;
    int32_t uid;
    bool isRecognition;
};

class ScoAudioSceneManager {
public:
    static ScoAudioSceneManager& GetInstance()
    {
        static ScoAudioSceneManager instance;
        return instance;
    }
    
    void AddScoStream(uint32_t streamId, AudioScene audioScene, int32_t uid, bool isRecognition);
    void RemoveScoStream(uint32_t streamId);
    AudioScene GetHighestPriorityAudioScene() const;
    ScoAudioScenePriority GetAudioScenePriority(AudioScene audioScene, bool isRecognition) const;
    bool HasScoStream() const;
    void ClearAllScoStreams();

private:
    ScoAudioSceneManager();
    ~ScoAudioSceneManager();
    ScoAudioSceneManager(const ScoAudioSceneManager&) = delete;
    ScoAudioSceneManager& operator=(const ScoAudioSceneManager&) = delete;
    
    std::unordered_map<uint32_t, ScoAudioSceneInfo> scoStreamsMap_;
    mutable std::mutex scoMutex_;
    
    ScoAudioScenePriority DeterminePriority(AudioScene audioScene, bool isRecognition) const;
};

}
}

#endif