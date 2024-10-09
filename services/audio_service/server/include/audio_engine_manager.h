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
#ifndef AUDIO_ENGINE_MANAGER_H
#define AUDIO_ENGINE_MANAGER_H
#include <unordered_map>
#include "audio_playback_engine.h"

namespace OHOS {
namespace AudioStandard {
enum class PlaybackType : int32_t {
    DIRECT,
    VOIP
};
class AudioEngineManager {
public:
    static AudioEngineManager GetInstance();
    ~AudioEngineManager() = default;
    void AddRenderer(std::shared_ptr<IRendererStream> stream, DeviceInfo device);
    void RemoveRenderer(std::shared_ptr<IRendererStream> stream);

private:
    AudioEngineManager() = default;

private:
    std::unordered_map<PlaybackType, std::shared_ptr<AudioPlaybackEngine>> renderEngines_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
