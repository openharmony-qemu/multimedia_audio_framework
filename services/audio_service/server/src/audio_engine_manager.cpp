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
#include "audio_engine_manager.h"
#include "none_mix_engine.h"

namespace OHOS {
namespace AudioStandard {
AudioEngineManager AudioEngineManager::GetInstance()
{
    static AudioEngineManager enginManager;
    return enginManager;
}

void AudioEngineManager::AddRenderer(std::shared_ptr<IRendererStream> stream, DeviceInfo device)
{
    AudioProcessConfig config = stream->GetAudioProcessConfig();
    bool isDirect = true;
    PlaybackType playbackType_ = PlaybackType::DIRECT;
    if (config.streamType != STREAM_MUSIC) {
        playbackType_ = PlaybackType::VOIP;
        isDirect = false;
    }
    auto iter = renderEngines_.find(playbackType_);
    if (iter == renderEngines_.end()) {
        std::shared_ptr<AudioPlaybackEngine> playbackEngine = std::make_shared<NoneMixEngine>();
        playbackEngine->Init(device, !isDirect);
        playbackEngine->AddRenderer(stream);
        renderEngines_.emplace(playbackType_, playbackEngine);
    }
}
void AudioEngineManager::RemoveRenderer(std::shared_ptr<IRendererStream> stream)
{
    AudioProcessConfig config = stream->GetAudioProcessConfig();
    PlaybackType playbackType_ = PlaybackType::DIRECT;
    if (config.streamType != STREAM_MUSIC) {
        playbackType_ = PlaybackType::VOIP;
    }
    auto iter = renderEngines_.find(playbackType_);
    if (iter != renderEngines_.end()) {
        renderEngines_.erase(iter);
    }
}
} // namespace AudioStandard
} // namespace OHOS
