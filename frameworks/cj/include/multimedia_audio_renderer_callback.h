/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_RENDERER_CALLBACK_H
#define MULTIMEDIA_AUDIO_RENDERER_CALLBACK_H
#include "audio_renderer.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjRendererPositionCallback : public RendererPositionCallback {
public:
    CjRendererPositionCallback() = default;
    virtual ~CjRendererPositionCallback() = default;
    void RegisterFunc(std::function<void(int64_t)> cjCallback);

    void OnMarkReached(const int64_t &framePosition) override;

private:
    std::function<void(int64_t)> func_{};
    std::mutex cbMutex_;
};

class CjRendererPeriodPositionCallback : public RendererPeriodPositionCallback {
public:
    CjRendererPeriodPositionCallback() = default;
    virtual ~CjRendererPeriodPositionCallback() = default;
    void RegisterFunc(std::function<void(int64_t)> cjCallback);

    void OnPeriodReached(const int64_t &frameNumber) override;

private:
    std::function<void(int64_t)> func_{};
    std::mutex cbMutex_;
};

class CjAudioRendererOutputDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    CjAudioRendererOutputDeviceChangeCallback() = default;
    virtual ~CjAudioRendererOutputDeviceChangeCallback() = default;
    void RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback);

    void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason) override;

private:
    std::function<void(CArrDeviceDescriptor)> func_{};
    std::mutex cbMutex_;
};

class CjAudioRendererOutputDeviceChangeWithInfoCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    CjAudioRendererOutputDeviceChangeWithInfoCallback() = default;
    virtual ~CjAudioRendererOutputDeviceChangeWithInfoCallback() = default;
    void RegisterFunc(std::function<void(CAudioStreamDeviceChangeInfo)> cjCallback);

    void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason) override;

private:
    std::function<void(CAudioStreamDeviceChangeInfo)> func_{};
    std::mutex cbMutex_;
};

class CjAudioRendererWriteCallback : public AudioRendererWriteCallback {
public:
    CjAudioRendererWriteCallback() = default;
    virtual ~CjAudioRendererWriteCallback() = default;
    void RegisterFunc(std::function<int32_t(CArrUI8)> cjCallback, std::shared_ptr<AudioRenderer> audioRenderer);

    void OnWriteData(size_t length) override;

private:
    std::function<int32_t(CArrUI8)> func_{};
    std::shared_ptr<AudioRenderer> audioRenderer_{};
    std::mutex cbMutex_;
};

class CjAudioRendererCallback : public AudioRendererCallback {
public:
    CjAudioRendererCallback() = default;
    virtual ~CjAudioRendererCallback() = default;
    void RegisterFunc(std::function<void(int32_t)> cjCallback);
    void RegisterInterruptFunc(std::function<void(CInterruptEvent)> cjCallback);

    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType) override;

private:
    std::function<void(CInterruptEvent)> interruptCallback_{};
    std::function<void(int32_t)> stateChangeCallback_{};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_RENDERER_CALLBACK_H
