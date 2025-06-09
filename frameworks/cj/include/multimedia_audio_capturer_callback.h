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

#ifndef MULTIMEDIA_AUDIO_CAPTURER_CALLBACK_H
#define MULTIMEDIA_AUDIO_CAPTURER_CALLBACK_H
#include "audio_capturer.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjAudioCapturerCallback : public AudioCapturerCallback {
public:
    CjAudioCapturerCallback() = default;
    virtual ~CjAudioCapturerCallback() = default;

    void OnInterrupt(const InterruptEvent& interruptEvent) override;

    void OnStateChange(const CapturerState state) override;

    void RegisterInterruptFunc(std::function<void(CInterruptEvent)> cjCallback);

    void RegisterStateChangeFunc(std::function<void(int32_t)> cjCallback);

private:
    std::function<void(CInterruptEvent)> interruptEventfunc_ {};
    std::function<void(int32_t)> stateChangefunc_ {};
    std::mutex cbMutex_;
};

class CjCapturerPositionCallback : public CapturerPositionCallback {
public:
    CjCapturerPositionCallback() = default;
    virtual ~CjCapturerPositionCallback() = default;
    void RegisterFunc(std::function<void(int64_t)> cjCallback);

    void OnMarkReached(const int64_t& framePosition) override;

private:
    std::function<void(int64_t)> func_ {};
    std::mutex cbMutex_;
};

class CjCapturerPeriodPositionCallback : public CapturerPeriodPositionCallback {
public:
    CjCapturerPeriodPositionCallback() = default;
    virtual ~CjCapturerPeriodPositionCallback() = default;
    void RegisterFunc(std::function<void(int64_t)> cjCallback);
    void OnPeriodReached(const int64_t& frameNumber) override;

private:
    std::function<void(int64_t)> func_ {};
    std::mutex cbMutex_;
};

class CjAudioCapturerReadCallback : public AudioCapturerReadCallback {
public:
    CjAudioCapturerReadCallback() = default;
    virtual ~CjAudioCapturerReadCallback() = default;

    void RegisterFunc(std::function<void(CArrUI8)> cjCallback, std::shared_ptr<AudioCapturer> audioCapturer);
    void OnReadData(size_t length) override;

private:
    std::function<void(CArrUI8)> func_ {};
    std::shared_ptr<AudioCapturer> audioCapturer_ {};
    std::mutex cbMutex_;
};

class CjAudioCapturerInfoChangeCallback : public AudioCapturerInfoChangeCallback {
public:
    CjAudioCapturerInfoChangeCallback() = default;
    virtual ~CjAudioCapturerInfoChangeCallback() = default;

    void RegisterFunc(std::function<void(CAudioCapturerChangeInfo)> cjCallback);

    void OnStateChange(const AudioCapturerChangeInfo& capturerChangeInfo) override;

private:
    std::function<void(CAudioCapturerChangeInfo)> func_ {};
    std::mutex cbMutex_;
};

class CjAudioCapturerDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
public:
    CjAudioCapturerDeviceChangeCallback() = default;
    virtual ~CjAudioCapturerDeviceChangeCallback() = default;

    void RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback);

    void OnStateChange(const AudioDeviceDescriptor& deviceInfo) override;

private:
    std::function<void(CArrDeviceDescriptor)> func_ {};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_CAPTURER_CALLBACK_H
