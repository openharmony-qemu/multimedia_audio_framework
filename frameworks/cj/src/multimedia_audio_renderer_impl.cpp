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

#include "multimedia_audio_renderer_impl.h"

#include "audio_errors.h"
#include "audio_interrupt_info.h"
#include "audio_renderer_log.h"
#include "cj_lambda.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {

MMAAudioRendererImpl::MMAAudioRendererImpl() {}

MMAAudioRendererImpl::~MMAAudioRendererImpl()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_.reset();
    }
}

int32_t MMAAudioRendererImpl::CreateAudioRenderer(CAudioRendererOptions options)
{
    AudioRendererOptions rendererOptions {};
    Convert2AudioRendererOptions(rendererOptions, options);
    audioRenderer_ = AudioRenderer::CreateRenderer(rendererOptions);
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("Create AudioRenderer failed.");
        return ERR_INVALID_INSTANCE_CODE;
    }
    if (callback_ == nullptr) {
        callback_ = std::make_shared<CjAudioRendererCallback>();
    }
    int ret = audioRenderer_->SetRendererCallback(callback_);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("SetRendererCallback failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

int32_t MMAAudioRendererImpl::GetState()
{
    if (audioRenderer_ == nullptr) {
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return audioRenderer_->GetStatus();
}

int64_t MMAAudioRendererImpl::GetAudioTime(int32_t* errorCode)
{
    Timestamp timestamp {};
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    bool ret = audioRenderer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
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

uint32_t MMAAudioRendererImpl::GetBufferSize(int32_t* errorCode)
{
    size_t bufferSize = 0;
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    int32_t ret = audioRenderer_->GetBufferSize(bufferSize);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get bufferSize failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return bufferSize;
}

int32_t MMAAudioRendererImpl::Flush()
{
    if (audioRenderer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioRenderer_->Flush();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer flush failed.");
        return CJ_ERR_ILLEGAL_STATE;
    }
    return SUCCESS_CODE;
}

int32_t MMAAudioRendererImpl::Drain()
{
    if (audioRenderer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioRenderer_->Drain();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer drain failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

int32_t MMAAudioRendererImpl::Pause()
{
    if (audioRenderer_ == nullptr) {
        return CJ_ERR_SYSTEM;
    }
    bool isSuccess = audioRenderer_->Pause();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer pause failed.");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

CArrDeviceDescriptor MMAAudioRendererImpl::GetCurrentOutputDevices(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = audioRenderer_->GetCurrentOutputDevices(deviceInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("GetCurrentOutputDevices failure!");
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

double MMAAudioRendererImpl::GetSpeed(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return audioRenderer_->GetSpeed();
}

bool MMAAudioRendererImpl::GetSilentModeAndMixWithOthers(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    bool on = audioRenderer_->GetSilentModeAndMixWithOthers();
    if (!on) {
        AUDIO_ERR_LOG("AudioRenderer GetSilentModeAndMixWithOthers failed.");
    }
    return on;
}

double MMAAudioRendererImpl::GetVolume(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return audioRenderer_->GetVolume();
}

uint32_t MMAAudioRendererImpl::GetUnderflowCount(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return audioRenderer_->GetUnderflowCount();
}

void MMAAudioRendererImpl::SetVolumeWithRamp(double volume, int32_t duration, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (volume < MIN_VOLUME_IN_DOUBLE || volume > MAX_VOLUME_IN_DOUBLE) {
        *errorCode = CJ_ERR_UNSUPPORTED;
        return;
    }
    int32_t ret = audioRenderer_->SetVolumeWithRamp(static_cast<float>(volume), duration);
    if (ret == ERR_ILLEGAL_STATE) {
        *errorCode = CJ_ERR_ILLEGAL_STATE;
    }
}

void MMAAudioRendererImpl::SetSpeed(double speed, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (speed < MIN_STREAM_SPEED_LEVEL || speed > MAX_STREAM_SPEED_LEVEL) {
        *errorCode = CJ_ERR_UNSUPPORTED;
        return;
    }
    int32_t ret = audioRenderer_->SetSpeed(static_cast<float>(speed));
    if (ret == ERR_ILLEGAL_STATE) {
        *errorCode = CJ_ERR_ILLEGAL_STATE;
    }
}

void MMAAudioRendererImpl::SetVolume(double volume, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (volume < MIN_VOLUME_IN_DOUBLE || volume > MAX_VOLUME_IN_DOUBLE) {
        *errorCode = CJ_ERR_UNSUPPORTED;
        return;
    }
    int32_t ret = audioRenderer_->SetVolume(static_cast<float>(volume));
    if (ret != SUCCESS_CODE) {
        *errorCode = CJ_ERR_SYSTEM;
    }
}

void MMAAudioRendererImpl::SetSilentModeAndMixWithOthers(bool on, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    audioRenderer_->SetSilentModeAndMixWithOthers(on);
}

void MMAAudioRendererImpl::SetInterruptMode(int32_t mode, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    audioRenderer_->SetInterruptMode(static_cast<InterruptMode>(mode));
}

void MMAAudioRendererImpl::SetChannelBlendMode(int32_t mode, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    int32_t ret = audioRenderer_->SetChannelBlendMode(static_cast<ChannelBlendMode>(mode));
    if (ret == ERR_ILLEGAL_STATE) {
        *errorCode = CJ_ERR_ILLEGAL_STATE;
    }
}

void MMAAudioRendererImpl::SetDefaultOutputDevice(int32_t type, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    int32_t ret = audioRenderer_->SetDefaultOutputDevice(static_cast<DeviceType>(type));
    if (ret != SUCCESS_CODE) {
        *errorCode = CJ_ERR_ILLEGAL_STATE;
    }
}

void MMAAudioRendererImpl::RegisterOutputDeviceCallback(int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    if (callbackType == AudioRendererCallbackType::AR_OUTPUT_DEVICE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register OUTPUT_DEVICE_CHANGE event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (rendererDeviceChangeCallback_ != nullptr) {
            AUDIO_ERR_LOG("OutputDeviceChangeCallback already subscribed!");
            return;
        }
        rendererDeviceChangeCallback_ = std::make_shared<CjAudioRendererOutputDeviceChangeCallback>();
        rendererDeviceChangeCallback_->RegisterFunc(func);
        int ret = audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(rendererDeviceChangeCallback_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetOutputDeviceChangeCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
    if (callbackType == AudioRendererCallbackType::AR_OUTPUT_DEVICE_CHANGE_WITH_INFO) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CAudioStreamDeviceChangeInfo)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register OUTPUT_DEVICE_CHANGE_WITH_INFO event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (rendererOutputDeviceChangeWithInfoCallback_ != nullptr) {
            AUDIO_ERR_LOG("OutputDeviceChangeWithInfoCallback already subscribed!");
            return;
        }
        rendererOutputDeviceChangeWithInfoCallback_ =
            std::make_shared<CjAudioRendererOutputDeviceChangeWithInfoCallback>();
        rendererOutputDeviceChangeWithInfoCallback_->RegisterFunc(func);
        int ret =
            audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(rendererOutputDeviceChangeWithInfoCallback_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetOutputDeviceChangeWithInfoCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
}

void MMAAudioRendererImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (callbackType == AudioRendererCallbackType::AR_AUDIO_INTERRUPT) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CInterruptEvent)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register AUDIO_INTERRUPT event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callback_->RegisterInterruptFunc(func);
    }
    if (callbackType == AudioRendererCallbackType::AR_WRITE_DATA) {
        auto func = CJLambda::Create(reinterpret_cast<int32_t (*)(CArrUI8)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register WRITE_DATA event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        rendererWriteDataCallback_ = std::make_shared<CjAudioRendererWriteCallback>();
        rendererWriteDataCallback_->RegisterFunc(func, audioRenderer_);
        audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);
        int32_t ret = audioRenderer_->SetRendererWriteCallback(rendererWriteDataCallback_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetWriteDataCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
    if (callbackType == AudioRendererCallbackType::AR_STATE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register STATE_CHANGE event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callback_->RegisterFunc(func);
    }
    RegisterOutputDeviceCallback(callbackType, callback, errorCode);
}

void MMAAudioRendererImpl::RegisterCallbackWithFrame(
    int32_t callbackType, void (*callback)(), int64_t frame, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    if (callbackType == AudioRendererCallbackType::AR_MARK_REACH) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int64_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register MARK_REACH event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (frame <= 0) {
            AUDIO_ERR_LOG("mark position not supported!");
            *errorCode = CJ_ERR_INVALID_PARAM;
            return;
        }
        positionCb_ = std::make_shared<CjRendererPositionCallback>();
        positionCb_->RegisterFunc(func);
        int32_t ret = audioRenderer_->SetRendererPositionCallback(frame, positionCb_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetRendererPositionCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
    if (callbackType == AudioRendererCallbackType::AR_PERIOD_REACH) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(int64_t)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register PERIOD_REACH event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        if (frame <= 0) {
            AUDIO_ERR_LOG("framecount not supported!");
            *errorCode = CJ_ERR_INVALID_PARAM;
            return;
        }
        if (periodPositionCb_ != nullptr) {
            AUDIO_ERR_LOG("PERIOD_REACH already subscribed!");
            return;
        }
        periodPositionCb_ = std::make_shared<CjRendererPeriodPositionCallback>();
        periodPositionCb_->RegisterFunc(func);
        int32_t ret = audioRenderer_->SetRendererPeriodPositionCallback(frame, periodPositionCb_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("SetRendererPeriodPositionCallback failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
}

int32_t MMAAudioRendererImpl::GetAudioEffectMode(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_VALUE;
    }
    return audioRenderer_->GetAudioEffectMode();
}

void MMAAudioRendererImpl::SetAudioEffectMode(int32_t mode, int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto ret = audioRenderer_->SetAudioEffectMode(static_cast<AudioEffectMode>(mode));
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get SetAudioEffectMode failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
    }
}

double MMAAudioRendererImpl::GetMinStreamVolume(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return audioRenderer_->GetMinStreamVolume();
}

double MMAAudioRendererImpl::GetMaxStreamVolume(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return audioRenderer_->GetMaxStreamVolume();
}

void MMAAudioRendererImpl::Release(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    bool isSuccess = audioRenderer_->Release();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer Release failed!");
    }
}

uint32_t MMAAudioRendererImpl::GetStreamId(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    uint32_t sessionId = 0;
    auto ret = audioRenderer_->GetAudioStreamId(sessionId);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get StreamId failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
    }
    return sessionId;
}

void MMAAudioRendererImpl::Stop(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    bool isSuccess = audioRenderer_->Stop();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer Stop failed.");
    }
}

void MMAAudioRendererImpl::Start(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    bool isSuccess = audioRenderer_->Start();
    if (!isSuccess) {
        AUDIO_ERR_LOG("AudioRenderer Start failed.");
    }
}

CAudioStreamInfo MMAAudioRendererImpl::GetStreamInfo(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    AudioStreamInfo streamInfo {};
    auto ret = audioRenderer_->GetStreamInfo(streamInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get StreamInfo failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
        return CAudioStreamInfo();
    }
    CAudioStreamInfo cInfo {};
    Convert2CAudioStreamInfo(cInfo, streamInfo);
    return cInfo;
}

CAudioRendererInfo MMAAudioRendererImpl::GetRendererInfo(int32_t* errorCode)
{
    if (audioRenderer_ == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioRendererInfo();
    }
    AudioRendererInfo rendererInfo {};
    auto ret = audioRenderer_->GetRendererInfo(rendererInfo);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("Get RendererInfo failed.");
        *errorCode = CJ_ERR_INVALID_VALUE;
        return CAudioRendererInfo();
    }
    CAudioRendererInfo cInfo {};
    Convert2AudioRendererInfo(cInfo, rendererInfo);
    return cInfo;
}
}
} // namespace AudioStandard
} // namespace OHOS
