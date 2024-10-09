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
#ifndef PRO_AUDIO_STREAM_MANAGER_H
#define PRO_AUDIO_STREAM_MANAGER_H

#include <map>
#include <mutex>
#include "i_stream_manager.h"
#include "audio_playback_engine.h"

namespace OHOS {
namespace AudioStandard {
class ProAudioStreamManager : public IStreamManager {
public:
    explicit ProAudioStreamManager(ManagerType type);
    ~ProAudioStreamManager();
    int32_t CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream) override;
    int32_t StartRender(uint32_t streamIndex) override;
    int32_t StopRender(uint32_t streamIndex) override;
    int32_t ReleaseRender(uint32_t streamIndex) override;
    int32_t PauseRender(uint32_t streamIndex) override;
    int32_t GetStreamCount() const noexcept override;
    int32_t TriggerStartIfNecessary() override;

    int32_t CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream) override;
    int32_t ReleaseCapturer(uint32_t streamIndex) override;

private:
    std::shared_ptr<IRendererStream> CreateRendererStream(AudioProcessConfig processConfig);
    int32_t CreatePlayBackEngine(const std::shared_ptr<IRendererStream> &stream);

private:
    std::mutex streamMapMutex_;
    std::mutex paElementsMutex_;
    ManagerType managerType_;
    std::unique_ptr<AudioPlaybackEngine> playbackEngine_;
    std::map<int32_t, std::shared_ptr<IRendererStream>> rendererStreamMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PRO_AUDIO_STREAM_MANAGER_H
