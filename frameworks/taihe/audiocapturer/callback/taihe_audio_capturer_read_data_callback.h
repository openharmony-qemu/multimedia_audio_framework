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

#ifndef TAIHE_AUDIO_CAPTURER_READ_DATA_CALLBACK_H
#define TAIHE_AUDIO_CAPTURER_READ_DATA_CALLBACK_H

#include "event_handler.h"
#include "taihe_audio_capturer.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class TaiheCapturerReadDataCallback : public OHOS::AudioStandard::AudioCapturerReadCallback,
    public std::enable_shared_from_this<TaiheCapturerReadDataCallback> {
public:
    explicit TaiheCapturerReadDataCallback(AudioCapturerImpl *taiheCapturer);
    virtual ~TaiheCapturerReadDataCallback();
    void OnReadData(size_t length) override;
    void AddCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> &callback);
    void RemoveTaiheCapturer();

private:
    struct CapturerReadDataJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::BufferDesc bufDesc {};
        AudioCapturerImpl *capturerTaiheObj;
        TaiheCapturerReadDataCallback *readDataCallbackPtr;
    };

    static void SafeJsCallbackCapturerReadDataWork(CapturerReadDataJsCallback *event);
    static void SafeJsCallbackCapturerReadDataWorkInner(CapturerReadDataJsCallback *event);
    void OnJsCapturerReadDataCallback(std::unique_ptr<CapturerReadDataJsCallback> &jsCb);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> capturerReadDataCallback_ = nullptr;
    AudioCapturerImpl *taiheCapturer_;
    bool isCallbackInited_ = false;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_CAPTURER_READ_DATA_CALLBACK_H