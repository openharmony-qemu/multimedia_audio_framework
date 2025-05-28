/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OH_AUDIO_CAPTURER_H
#define OH_AUDIO_CAPTURER_H

#include "native_audiocapturer.h"
#include "audio_capturer.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioCapturerModeCallback : public AudioCapturerReadCallback {
public:
    OHAudioCapturerModeCallback(OH_AudioCapturer_Callbacks callbacks, OH_AudioCapturer* audioCapturer, void* userData)
        : callbacks_(callbacks), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }
    OHAudioCapturerModeCallback(OH_AudioCapturer_OnReadDataCallback onReadDataCallback,
        OH_AudioCapturer* audioCapturer, void* userData)
        : onReadDataCallback_(onReadDataCallback), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnReadData(size_t length) override;
private:
    OH_AudioCapturer_Callbacks callbacks_ = {};
    OH_AudioCapturer_OnReadDataCallback onReadDataCallback_ = nullptr;
    OH_AudioCapturer* ohAudioCapturer_ = nullptr;
    void* userData_ = nullptr;
};

class OHAudioCapturerCallback : public AudioCapturerCallback {
public:
    OHAudioCapturerCallback(OH_AudioCapturer_Callbacks callbacks, OH_AudioCapturer* audioCapturer, void* userData)
        : callbacks_(callbacks), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    OHAudioCapturerCallback(OH_AudioCapturer_OnInterruptCallback OnInterruptEventCallback,
        OH_AudioCapturer *audioCapturer, void *userData) : onInterruptEventCallback_(OnInterruptEventCallback),
        ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnInterrupt(const InterruptEvent &interruptEvent) override;

    void OnStateChange(const CapturerState state) override
    {
        AUDIO_DEBUG_LOG("OHAudioCapturerCallback:: OnStateChange");
    }

private:
    OH_AudioCapturer_Callbacks callbacks_ = {};
    OH_AudioCapturer_OnInterruptCallback onInterruptEventCallback_ = nullptr;
    OH_AudioCapturer* ohAudioCapturer_ = nullptr;
    void* userData_ = nullptr;
};

class OHCapturerServiceDiedCallback : public AudioCapturerPolicyServiceDiedCallback {
public:
    OHCapturerServiceDiedCallback(OH_AudioCapturer_OnErrorCallback errorCallback, OH_AudioCapturer *audioCapturer,
        void *userData) : errorCallback_(errorCallback), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnAudioPolicyServiceDied() override;

private:
    OH_AudioCapturer_OnErrorCallback errorCallback_ = nullptr;
    OH_AudioCapturer* ohAudioCapturer_;
    void *userData_;
};

class OHAudioCapturerErrorCallback : public AudioCapturerErrorCallback {
public:
    OHAudioCapturerErrorCallback(OH_AudioCapturer_Callbacks callbacks, OH_AudioCapturer *audioCapturer,
        void *userData) : callbacks_(callbacks), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }
    OHAudioCapturerErrorCallback(OH_AudioCapturer_OnErrorCallback errorCallback, OH_AudioCapturer *audioCapturer,
        void *userData) : errorCallback_(errorCallback), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnError(AudioErrors errorCode) override;

    OH_AudioStream_Result GetErrorResult(AudioErrors errorCode) const;

private:
    OH_AudioCapturer_Callbacks callbacks_ = {};
    OH_AudioCapturer_OnErrorCallback errorCallback_ = nullptr;
    OH_AudioCapturer *ohAudioCapturer_;
    void *userData_;
};

class OHAudioCapturerDeviceChangeCallback : public AudioCapturerDeviceChangeCallback {
public:
    OHAudioCapturerDeviceChangeCallback(OH_AudioCapturer_Callbacks callbacks, OH_AudioCapturer* audioCapturer,
        void* userData) : callbacks_(callbacks), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }
    OHAudioCapturerDeviceChangeCallback(OH_AudioCapturer_OnDeviceChangeCallback onDeviceChangeCallback,
        OH_AudioCapturer* audioCapturer, void* userData)
        : onDeviceChangeCallback_(onDeviceChangeCallback), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnStateChange(const AudioDeviceDescriptor &deviceInfo) override;

private:
    OH_AudioCapturer_Callbacks callbacks_ = {};
    OH_AudioCapturer_OnDeviceChangeCallback onDeviceChangeCallback_ = nullptr;
    OH_AudioCapturer* ohAudioCapturer_ = nullptr;
    void* userData_ = nullptr;
};

class OHAudioCapturerFastStatusChangeCallback : public AudioCapturerFastStatusChangeCallback {
public:
    OHAudioCapturerFastStatusChangeCallback(OH_AudioCapturer_OnFastStatusChange callback,
        OH_AudioCapturer *audioCapturer, void *userData)
        : callback_(callback), ohAudioCapturer_(audioCapturer), userData_(userData)
    {
    }

    void OnFastStatusChange(FastStatus status) override;
private:
    OH_AudioCapturer_OnFastStatusChange callback_;
    OH_AudioCapturer *ohAudioCapturer_;
    void *userData_;
};

struct CapturerCallback {
    OH_AudioCapturer_Callbacks callbacks = {};

    OH_AudioCapturer_OnReadDataCallback onReadDataCallback = {};

    OH_AudioCapturer_OnDeviceChangeCallback onDeviceChangeCallback = {};

    OH_AudioCapturer_OnInterruptCallback onInterruptEventCallback = {};

    OH_AudioCapturer_OnErrorCallback onErrorCallback = {};
};

class OHAudioCapturer {
public:
    OHAudioCapturer();
    ~OHAudioCapturer();

    bool Initialize(const AudioCapturerOptions& capturerOptions);
    bool Start();
    bool Pause();
    bool Stop();
    bool Flush();
    bool Release();
    CapturerState GetCurrentState();
    void GetStreamId(uint32_t& streamId);
    AudioChannel GetChannelCount();
    int32_t GetSamplingRate();
    AudioEncodingType GetEncodingType();
    AudioSampleFormat GetSampleFormat();
    void GetCapturerInfo(AudioCapturerInfo& capturerInfo);
    int64_t GetFramesRead();
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base);
    int32_t GetFrameSizeInCallback();
    int32_t GetBufferDesc(BufferDesc &bufDesc) const;
    int32_t Enqueue(const BufferDesc &bufDesc) const;
    int32_t SetInputDevice(DeviceType deviceType);
    FastStatus GetFastStatus();
    uint32_t GetOverflowCount() const;

    void SetInterruptCallback(CapturerCallback capturerCallbacks, void *userData);
    void SetErrorCallback(CapturerCallback capturerCallbacks, void *userData);
    void RegisterErrorCallback(CapturerCallback capturerCallbacks, void *userData, void *metadataUserData,
        AudioEncodingType encodingType);
    void SetCapturerInterruptEventCallbackType(InterruptEventCallbackType callbackType);
    void SetCapturerErrorCallbackType(ErrorCallbackType errorCallbackType);
    InterruptEventCallbackType GetCapturerInterruptEventCallbackType();
    ErrorCallbackType GetCapturerErrorCallbackType();
    void SetCapturerFastStatusChangeCallback(OH_AudioCapturer_OnFastStatusChange callback, void *userData);

    void SetCapturerCallback(CapturerCallback capturerCallbacks, void* userData);
    void SetReadDataCallback(CapturerCallback capturerCallbacks, void* userData);
    void SetStreamEventCallback(CapturerCallback capturerCallbacks, void* userData);
    void SetCapturerReadDataCallbackType(ReadDataCallbackType readDataCallbackType);
    void SetCapturerStreamEventCallbackType(StreamEventCallbackType streamEventCallbackType);
    void SetCapturerWillMuteWhenInterrupted(InterruptStrategy strategy);
    ReadDataCallbackType GetCapturerReadDataCallbackType();
    StreamEventCallbackType GetCapturerStreamEventCallbackType();

private:
    std::shared_ptr<AudioCapturer> audioCapturer_;
    std::shared_ptr<AudioCapturerCallback> audioCapturerCallback_;
    ReadDataCallbackType readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    StreamEventCallbackType streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    ErrorCallbackType errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    InterruptEventCallbackType interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    std::shared_ptr<OHAudioCapturerFastStatusChangeCallback> audioCapturerFastStatusChangeCallback_;
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // OH_AUDIO_CAPTURER_H
