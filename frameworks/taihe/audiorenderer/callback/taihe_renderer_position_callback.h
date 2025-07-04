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

#ifndef TAIHE_RENDERER_POSITION_CALLBACK_H
#define TAIHE_RENDERER_POSITION_CALLBACK_H

#include "event_handler.h"
#include "audio_renderer.h"
#include "taihe_audio_renderer_callback.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
class TaiheRendererPositionCallback : public OHOS::AudioStandard::RendererPositionCallback,
    public TaiheAudioRendererCallbackInner, public std::enable_shared_from_this<TaiheRendererPositionCallback> {
public:
    explicit TaiheRendererPositionCallback();
    ~TaiheRendererPositionCallback() override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
    void OnMarkReached(const int64_t &framePosition) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct RendererPositionJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        int64_t position = 0;
    };
    void OnJsRendererPositionCallback(std::unique_ptr<RendererPositionJsCallback> &jsCb);
    static void SafeJsCallbackPositionWork(RendererPositionJsCallback *event);
    std::mutex mutex_;
    std::shared_ptr<AutoRef> renderPositionCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_RENDERER_POSITION_CALLBACK_H
