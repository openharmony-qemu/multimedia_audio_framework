/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H
#define TAIHE_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H

#include <optional>

#include "event_handler.h"
#include "taihe_audio_renderer.h"
#include "taihe_audio_renderer_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
class TaiheRendererWriteDataCallback : public OHOS::AudioStandard::AudioRendererWriteCallback,
    public std::enable_shared_from_this<TaiheRendererWriteDataCallback> {
public:
    TaiheRendererWriteDataCallback(AudioRendererImpl *taiheRenderer, size_t bufferSize);
    virtual ~TaiheRendererWriteDataCallback();
    void OnWriteData(size_t length) override;

    void AddCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> &callback);

private:
    struct RendererWriteDataJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::BufferDesc bufDesc {};
        AudioRendererImpl *rendererTaiheObj;
        bool enqueued = false;
    };

    static void SafeJsCallbackWriteDataWork(RendererWriteDataJsCallback &event);
    static std::optional<AudioRendererWriteDataCallbackResult> CallWriteDataCallback(
        RendererWriteDataJsCallback &event);
    static void CopyWriteDataCallbackBuffer(OHOS::AudioStandard::BufferDesc &bufDesc,
        const taihe::array<uint8_t> &data);
    void OnJsRendererWriteDataCallback(const std::shared_ptr<RendererWriteDataJsCallback> &jsCb);
    bool WaitForWriteDataCallbackComplete(const std::shared_ptr<RendererWriteDataJsCallback> &jsCb);
    static void CheckWriteDataCallbackResult(OHOS::AudioStandard::BufferDesc &bufDesc,
        AudioDataCallbackResult result);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> rendererWriteDataCallback_ = nullptr;
    AudioRendererImpl *taiheRenderer_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
    const size_t bufferSize_ = 0;
    const std::unique_ptr<uint8_t[]> callbackBuffer_ = nullptr;

#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
    static vector<AudioRendererImpl*> activeRenderers_;
#endif
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RENDERER_WRITE_DATA_CALLBACK_H
