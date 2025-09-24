/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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
#include "multimedia_audio_capturer_impl.h"

#include "audio_capturer_log.h"
#include "cj_lambda.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioCapturerImpl::MMAAudioCapturerImpl()
{
    capturerReadDataCb_ = std::make_shared<CjAudioCapturerReadCallback>();
}

int32_t MMAAudioCapturerImpl::CreateAudioCapturer(CAudioCapturerOptions options)
{
    AudioCapturerOptions capturerOptions {};
    Convert2AudioCapturerOptions(capturerOptions, options);
    audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions);
    if (audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Create AudioCapturer failed.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    if (callback_ == nullptr) {
        callback_ = std::make_shared<CjAudioCapturerCallback>();
        int ret = audioCapturer_->SetCapturerCallback(callback_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetCapturerCallback failed.");
            return CJ_ERR_SYSTEM;
        }
    }
    return SUCCESS_CODE;
}

MMAAudioCapturerImpl::~MMAAudioCapturerImpl()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_.reset();
    }
}

int32_t MMAAudioCapturerImpl::GetState()
{
    if (audioCapturer_ == nullptr) {
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return audioCapturer_->GetStatus();
}

uint32_t MMAAudioCapturerImpl::GetStreamId(int32_t* errorCode)
{
    uint32_t id = 0;
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    int32_t ret = audioCapturer_->GetAudioStreamId(id);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get streamId failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
    }
    return id;
}

int64_t MMAAudioCapturerImpl::GetAudioTime(int32_t* errorCode)
{
    Timestamp timestamp {};
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    bool ret = audioCapturer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    if (!ret) {
        AUDIO_ERR_LOG("Get audioTime failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    const uint64_t secToNanosecond = 1000000000;
    uint64_t time =
        static_cast<uint64_t>(timestamp.time.tv_nsec) + static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;
    return static_cast<int64_t>(time);
}

uint32_t MMAAudioCapturerImpl::GetBufferSize(int32_t* errorCode)
{
    size_t bufferSize = 0;
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    int32_t ret = audioCapturer_->GetBufferSize(bufferSize);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get bufferSize failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return bufferSize;
}

uint32_t MMAAudioCapturerImpl::GetOverflowCount()
{
    if (audioCapturer_ == nullptr) {
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return audioCapturer_->GetOverflowCount();
}

int32_t MMAAudioCapturerImpl::Start()
{
    if (audioCapturer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioCapturer_->Start();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioCapturer start failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

int32_t MMAAudioCapturerImpl::Stop()
{
    if (audioCapturer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioCapturer_->Stop();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioCapturer stop failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

int32_t MMAAudioCapturerImpl::Release()
{
    if (audioCapturer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioCapturer_->Release();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioCapturer Release failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

CAudioCapturerInfo MMAAudioCapturerImpl::GetCurrentCapturerInfo(int32_t* errorCode)
{
    AudioCapturerInfo capturerInfo {};
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerInfo();
    }
    int32_t ret = audioCapturer_->GetCapturerInfo(capturerInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("GetCapturerInfo failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerInfo();
    }
    CAudioCapturerInfo cInfo {};
    Convert2CAudioCapturerInfo(cInfo, capturerInfo);
    return cInfo;
}

CAudioStreamInfo MMAAudioCapturerImpl::GetStreamInfo(int32_t* errorCode)
{
    AudioStreamInfo streamInfo {};
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    int32_t ret = audioCapturer_->GetStreamInfo(streamInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("GetStreamInfo failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    CAudioStreamInfo cInfo {};
    Convert2CAudioStreamInfo(cInfo, streamInfo);
    return cInfo;
}

CAudioCapturerChangeInfo MMAAudioCapturerImpl::GetAudioCapturerChangeInfo(int32_t* errorCode)
{
    AudioCapturerChangeInfo changeInfo {};
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerChangeInfo();
    }
    int32_t ret = audioCapturer_->GetCurrentCapturerChangeInfo(changeInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("GetAudioCapturerChangeInfo failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerChangeInfo();
    }
    CAudioCapturerChangeInfo cInfo {};
    Convert2CAudioCapturerChangeInfo(cInfo, changeInfo, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(cInfo.deviceDescriptors);
        return CAudioCapturerChangeInfo();
    }
    return cInfo;
}

CArrDeviceDescriptor MMAAudioCapturerImpl::GetInputDevices(int32_t* errorCode)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    int32_t ret = audioCapturer_->GetCurrentInputDevices(deviceInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("GetCurrentInputDevices failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor devices {};
    Convert2CArrDeviceDescriptorByDeviceInfo(devices, deviceInfo, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(devices);
        return CArrDeviceDescriptor();
    }
    return devices;
}

void MMAAudioCapturerImpl::RegisterCArrCallback(int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (callbackType == AudioCapturerCallbackType::INPUT_DEVICE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register preferredInputDeviceChangeForCapturerInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        auto cb = std::make_shared<CjAudioCapturerDeviceChangeCallback>();
        cb->RegisterFunc(func);
        audioCapturer_->SetAudioCapturerDeviceChangeCallback(cb);
        deviceChangeCallbacks_.push_back(cb);
    }
    if (callbackType == AudioCapturerCallbackType::READ_DATA) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrUI8)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register read_data event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        auto ret = audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetCaptureMode failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        capturerReadDataCb_->RegisterFunc(func, audioCapturer_);
        ret = audioCapturer_->SetCapturerReadCallback(capturerReadDataCb_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetCapturerReadCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
}

void MMAAudioCapturerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (callbackType == AudioCapturerCallbackType::AUDIO_CAPTURER_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CAudioCapturerChangeInfo)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register audiocapturerchange event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        auto cb = std::make_shared<CjAudioCapturerInfoChangeCallback>();
        cb->RegisterFunc(func);
        audioCapturer_->SetAudioCapturerInfoChangeCallback(cb);
        capturerInfoChangeCallbacks_.emplace_back(cb);
    }
    if (callbackType == AudioCapturerCallbackType::AUDIO_INTERRUPT) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CInterruptEvent)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register audio_interrupt event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callback_->RegisterInterruptFunc(func);
    }
    if (callbackType == AudioCapturerCallbackType::STATE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register state_change event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callback_->RegisterStateChangeFunc(func);
        auto state = audioCapturer_->GetStatus();
        if (state == CAPTURER_PREPARED) {
            callback_->OnStateChange(state);
        }
    }
    RegisterCArrCallback(callbackType, callback, errorCode);
}

void MMAAudioCapturerImpl::RegisterCallbackWithFrame(
    int32_t callbackType, void (*callback)(), int64_t frame, int32_t* errorCode)
{
    if (audioCapturer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (callbackType == AudioCapturerCallbackType::MARK_REACH) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int64_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register mark_reach event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (frame <= 0) {
            AUDIO_ERR_LOG("mark position not supported!");
            *errorCode = CJ_ERR_INVALID_PARAM;
            return;
        }
        positionCb_ = std::make_shared<CjCapturerPositionCallback>();
        positionCb_->RegisterFunc(func);
        int32_t ret = audioCapturer_->SetCapturerPositionCallback(frame, positionCb_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetCapturerPositionCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
    if (callbackType == AudioCapturerCallbackType::PERIOD_REACH) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int64_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register period_reach event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (frame <= 0) {
            AUDIO_ERR_LOG("framecount not supported!");
            *errorCode = CJ_ERR_INVALID_PARAM;
            return;
        }
        if (periodPositionCb_ != nullptr) {
            AUDIO_ERR_LOG("period_reach already subscribed!");
            return;
        }
        periodPositionCb_ = std::make_shared<CjCapturerPeriodPositionCallback>();
        periodPositionCb_->RegisterFunc(func);
        int32_t ret = audioCapturer_->SetCapturerPeriodPositionCallback(frame, periodPositionCb_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetCapturerPeriodPositionCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
