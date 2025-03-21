/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CALLBACK_WRAPPER_H
#define CALLBACK_WRAPPER_H

#include <iostream>
#include <vector>
#include <mutex>
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"

namespace OHOS {
namespace AudioStandard {
class SinkCallbackWrapper : public IAudioSinkCallback {
public:
    SinkCallbackWrapper() = default;
    ~SinkCallbackWrapper() = default;

    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSinkCallback> cb);
    void RegistCallback(uint32_t type, IAudioSinkCallback *cb);
    void RegistCallbackGenerator(uint32_t type,
        std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> cbGenerator);
    std::shared_ptr<IAudioSinkCallback> GetCallback(uint32_t type, uint32_t renderId);
    IAudioSinkCallback *GetRawCallback(uint32_t type);

    void OnRenderSinkParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;
    void OnRenderSinkStateChange(uint32_t uniqueId, bool started) override;

private:
    std::unordered_map<uint32_t, std::shared_ptr<IAudioSinkCallback> > cbs_;
    std::unordered_map<uint32_t, IAudioSinkCallback *> rawCbs_;
    std::unordered_map<uint32_t, std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> > cbGenerators_;
    std::mutex cbMtx_;
    std::mutex rawCbMtx_;
    std::mutex cbGeneratorMtx_;
};

class SourceCallbackWrapper : public IAudioSourceCallback {
public:
    SourceCallbackWrapper() = default;
    ~SourceCallbackWrapper() = default;

    void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> cb);
    void RegistCallback(uint32_t type, IAudioSourceCallback *cb);
    void RegistCallbackGenerator(uint32_t type,
        std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> cbGenerator);
    std::shared_ptr<IAudioSourceCallback> GetCallback(uint32_t type, uint32_t captureId);
    IAudioSourceCallback *GetRawCallback(uint32_t type);

    void OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;
    void OnCaptureState(bool isActive) override;
    void OnWakeupClose(void) override;

private:
    std::unordered_map<uint32_t, std::shared_ptr<IAudioSourceCallback> > cbs_;
    std::unordered_map<uint32_t, IAudioSourceCallback *> rawCbs_;
    std::unordered_map<uint32_t, std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> > cbGenerators_;
    std::mutex cbMtx_;
    std::mutex rawCbMtx_;
    std::mutex cbGeneratorMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // CALLBACK_WRAPPER_H
