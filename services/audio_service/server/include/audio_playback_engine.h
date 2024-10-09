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
#ifndef AUDIO_PLAYBACK_ENGINE_H
#define AUDIO_PLAYBACK_ENGINE_H
#include <memory>
#include "i_audio_engine.h"
#include "i_renderer_stream.h"
#include "audio_renderer_sink.h"
#include "audio_thread_task.h"

namespace OHOS {
namespace AudioStandard {
class AudioPlaybackEngine : public IAudioEngine {
public:
    AudioPlaybackEngine();
    virtual ~AudioPlaybackEngine() override;
    virtual int32_t AddRenderer(const std::shared_ptr<IRendererStream> &stream);
    virtual void RemoveRenderer(const std::shared_ptr<IRendererStream> &stream);

    virtual int32_t Init(const DeviceInfo &type, bool isVoip) override;
    virtual int32_t Start() override;
    virtual int32_t Stop() override;
    virtual int32_t Pause() override;
    virtual int32_t Flush() override;

    virtual bool IsPlaybackEngineRunning() const noexcept override;

protected:
    virtual void MixStreams() {}

protected:
    IAudioRendererSink *renderSink_;
    std::unique_ptr<AudioThreadTask> playbackThread_;
    std::vector<std::shared_ptr<IRendererStream>> streams_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
