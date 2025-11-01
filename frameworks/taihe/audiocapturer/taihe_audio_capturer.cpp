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
#ifndef LOG_TAG
#define LOG_TAG "AudioCapturerImpl"
#endif

#include "taihe_audio_capturer.h"
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_audio_capturer_callbacks.h"
#include "taihe_audio_capturer_device_change_callback.h"
#include "taihe_audio_capturer_info_change_callback.h"
#include "taihe_audio_capturer_read_data_callback.h"
#include "taihe_capturer_period_position_callback.h"
#include "taihe_capturer_position_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
std::unique_ptr<OHOS::AudioStandard::AudioCapturerOptions> AudioCapturerImpl::sCapturerOptions_ = nullptr;
std::mutex AudioCapturerImpl::createMutex_;
int32_t AudioCapturerImpl::isConstructSuccess_ = OHOS::AudioStandard::SUCCESS;

constexpr int64_t DEFAULT_BUFFER_SIZE = 0;
constexpr uint32_t DEFAULT_ARRAY_SIZE = 0;
constexpr uint64_t SEC_TO_NANOSECOND = 1000000000;

template <typename T>
static void GetCapturerTaiheCallback(std::shared_ptr<uintptr_t> &callback, const std::string &cbName,
    std::list<std::shared_ptr<TaiheAudioCapturerCallbackInner>> audioCapturerCallbacks, std::shared_ptr<T> *cb)
{
    if (audioCapturerCallbacks.size() == 0) {
        AUDIO_ERR_LOG("no callback to get");
        return;
    }
    for (auto &iter:audioCapturerCallbacks) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (!iter->CheckIfTargetCallbackName(cbName)) {
            continue;
        }
        std::shared_ptr<T> temp = std::static_pointer_cast<T>(iter);
        if (temp->ContainSameJsCallbackInner(cbName, callback)) {
            *cb = temp;
            return;
        }
    }
}

template <typename T>
static void UnregisterAudioCapturerSingletonCallbackTemplate(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, std::shared_ptr<T> cb,
    std::function<int32_t(std::shared_ptr<T> callbackPtr,
        std::shared_ptr<uintptr_t> callback)> removeFunction = nullptr)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(cbName, callback), "callback not exists!");
    }
    cb->RemoveCallbackReference(cbName, callback);

    if (removeFunction == nullptr) {
        return;
    }
    int32_t ret = removeFunction(cb, callback);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of Capturer callback failed");
    return;
}

template <typename T>
static void UnregisterAudioCapturerCallbackTemplate(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, std::shared_ptr<uintptr_t> callback)> removeFunction,
    AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "taiheCapturer is nullptr");
    if (callback != nullptr) {
        std::shared_ptr<T> cb = nullptr;
        GetCapturerTaiheCallback(callback, cbName, taiheCapturer->audioCapturerCallbacks_, &cb);
        CHECK_AND_RETURN_LOG(cb != nullptr, "GetCapturerTaiheCallback is null");
        UnregisterAudioCapturerSingletonCallbackTemplate(callback, cbName, cb, removeFunction);
        return;
    }

    auto isPresent = [&callback, &cbName, &removeFunction]
        (std::shared_ptr<TaiheAudioCapturerCallbackInner> &iter) {
            CHECK_AND_RETURN_RET_LOG(iter != nullptr, false, "iter is null");
            if (!iter->CheckIfTargetCallbackName(cbName)) {
                return false;
            }
            std::shared_ptr<T> cbInner = std::static_pointer_cast<T>(iter);
            UnregisterAudioCapturerSingletonCallbackTemplate(callback, cbName, cbInner, removeFunction);
            return true;
        };
    taiheCapturer->audioCapturerCallbacks_.remove_if(isPresent);
    AUDIO_DEBUG_LOG("UnregisterAudioCapturerCallback success");
}

AudioCapturerImpl::AudioCapturerImpl()
    : audioCapturer_(nullptr), sourceType_(OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC) {}

AudioCapturerImpl::AudioCapturerImpl(std::shared_ptr<AudioCapturerImpl> obj)
    : audioCapturer_(nullptr), sourceType_(OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC)
{
    if (obj != nullptr) {
        audioCapturer_ = obj->audioCapturer_;
        callbackTaihe_ = obj->callbackTaihe_;
        positionCbTaihe_ = obj->positionCbTaihe_;
        periodPositionCbTaihe_ = obj->periodPositionCbTaihe_;
        capturerReadDataCallbackTaihe_ = obj->capturerReadDataCallbackTaihe_;
        sourceType_ = obj->sourceType_;
    }
}

AudioCapturerImpl::~AudioCapturerImpl() = default;

int64_t AudioCapturerImpl::GetImplPtr()
{
    return reinterpret_cast<uintptr_t>(this);
}

std::shared_ptr<OHOS::AudioStandard::AudioCapturer> AudioCapturerImpl::GetNativePtr()
{
    return audioCapturer_;
}

std::shared_ptr<AudioCapturerImpl> AudioCapturerImpl::CreateAudioCapturerNativeObject()
{
    std::shared_ptr<AudioCapturerImpl> audioCapturerImpl = std::make_shared<AudioCapturerImpl>();
    if (audioCapturerImpl == nullptr) {
        AUDIO_ERR_LOG("No memory");
        return nullptr;
    }
    CHECK_AND_RETURN_RET_LOG(sCapturerOptions_ != nullptr, nullptr, "sCapturerOptions_ is nullptr");
    audioCapturerImpl->sourceType_ = sCapturerOptions_->capturerInfo.sourceType;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions = *sCapturerOptions_;
    /* AudioCapturer not support other capturerFlags, only support flag 0 */
    if (capturerOptions.capturerInfo.capturerFlags != 0) {
        capturerOptions.capturerInfo.capturerFlags = 0;
    }
    capturerOptions.capturerInfo.recorderType = OHOS::AudioStandard::RECORDER_TYPE_ARKTS_AUDIO_RECORDER;
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    audioCapturerImpl->audioCapturer_ = OHOS::AudioStandard::AudioCapturer::CreateCapturer(capturerOptions);
#else
    std::string cacheDir = "/data/storage/el2/base/temp";
    audioCapturerImpl->audioCapturer_ = OHOS::AudioStandard::AudioCapturer::Create(capturerOptions, cacheDir);
#endif
    if (audioCapturerImpl->audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Capturer Create failed");
        AudioCapturerImpl::isConstructSuccess_ = TAIHE_ERR_SYSTEM;
        return nullptr;
    }

    if (audioCapturerImpl->audioCapturer_ != nullptr && audioCapturerImpl->callbackTaihe_ == nullptr) {
        audioCapturerImpl->callbackTaihe_ = std::make_shared<TaiheAudioCapturerCallback>();
        CHECK_AND_RETURN_RET_LOG(audioCapturerImpl->callbackTaihe_ != nullptr, audioCapturerImpl, "No memory");
        int32_t ret = audioCapturerImpl->audioCapturer_->SetCapturerCallback(audioCapturerImpl->callbackTaihe_);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
            audioCapturerImpl, "Construct SetCapturerCallback failed");
    }
    return audioCapturerImpl;
}

AudioCapturer AudioCapturerImpl::CreateAudioCapturerWrapper(OHOS::AudioStandard::AudioCapturerOptions capturerOptions)
{
    std::lock_guard<std::mutex> lock(createMutex_);
    if (sCapturerOptions_ != nullptr) {
        sCapturerOptions_.release();
    }
    sCapturerOptions_ = std::make_unique<OHOS::AudioStandard::AudioCapturerOptions>();
    if (sCapturerOptions_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "sCapturerOptions create failed");
        return make_holder<AudioCapturerImpl, AudioCapturer>(nullptr);
    }
    *sCapturerOptions_ = capturerOptions;
    std::shared_ptr<AudioCapturerImpl> impl = AudioCapturerImpl::CreateAudioCapturerNativeObject();
    if (impl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "failed to CreateAudioCapturerNativeObject");
        return make_holder<AudioCapturerImpl, AudioCapturer>(nullptr);
    }
    return make_holder<AudioCapturerImpl, AudioCapturer>(impl);
}

void AudioCapturerImpl::UnregisterCapturerCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->callbackTaihe_ != nullptr, "taiheCaptureCallback is nullptr");
    std::shared_ptr<TaiheAudioCapturerCallback> cb =
        std::static_pointer_cast<TaiheAudioCapturerCallback>(taiheCapturer->callbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    UnregisterAudioCapturerSingletonCallbackTemplate(callback, cbName, cb);
    AUDIO_DEBUG_LOG("UnregisterCapturerCallback is successful");
}

void AudioCapturerImpl::UnregisterAudioCapturerDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheAudioCapturerDeviceChangeCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheCapturer] (std::shared_ptr<TaiheAudioCapturerDeviceChangeCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            int32_t ret = taiheCapturer->audioCapturer_->RemoveAudioCapturerDeviceChangeCallback(callbackPtr);
            CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
                OHOS::AudioStandard::ERR_OPERATION_FAILED,
                "unregister capture device change callback function failed");
            AUDIO_INFO_LOG("UnregisterCapturerDeviceChangeCallback success");
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioCapturerCallbackTemplate(callback, cbName, removeFunction, taiheCapturer);
    AUDIO_INFO_LOG("UnregisterCapturerDeviceChangeCallback is successful");
}

void AudioCapturerImpl::UnregisterAudioCapturerInfoChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheAudioCapturerInfoChangeCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheCapturer] (std::shared_ptr<TaiheAudioCapturerInfoChangeCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            int32_t ret = taiheCapturer->audioCapturer_->RemoveAudioCapturerInfoChangeCallback(callbackPtr);
            CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
                OHOS::AudioStandard::ERR_OPERATION_FAILED,
                "unregister capture info change callback function failed");
            AUDIO_INFO_LOG("UnregisterCapturerDeviceChangeCallback success");
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioCapturerCallbackTemplate(callback, cbName, removeFunction, taiheCapturer);
    AUDIO_INFO_LOG("UnregisterCapturerInfoChangeCallback is successful");
}

void AudioCapturerImpl::UnregisterCapturerReadDataCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->capturerReadDataCallbackTaihe_ != nullptr,
        "capturerReadDataCallbackTaihe_ is nullptr");
    std::shared_ptr<TaiheCapturerReadDataCallback> cb =
        std::static_pointer_cast<TaiheCapturerReadDataCallback>(taiheCapturer->capturerReadDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->RemoveCallbackReference(callback);
    AUDIO_INFO_LOG("UnregisterCapturerReadDataCallback is successful");
}

void AudioCapturerImpl::UnregisterCapturerPeriodPositionCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->periodPositionCbTaihe_ != nullptr, "taiheCaptureCallback is nullptr");
    std::shared_ptr<TaiheCapturerPeriodPositionCallback> cb =
        std::static_pointer_cast<TaiheCapturerPeriodPositionCallback>(taiheCapturer->periodPositionCbTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheCapturerPeriodPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheCapturer] (std::shared_ptr<TaiheCapturerPeriodPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            taiheCapturer->audioCapturer_->UnsetCapturerPeriodPositionCallback();
            AUDIO_INFO_LOG("UnregisterCapturerPeriodPositionCallback success");
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioCapturerSingletonCallbackTemplate(callback, cbName, cb, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterCapturerPeriodPositionCallback is successful");
}

void AudioCapturerImpl::UnregisterCapturerPositionCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->positionCbTaihe_ != nullptr, "taiheCaptureCallback is nullptr");
    std::shared_ptr<TaiheCapturerPositionCallback> cb =
        std::static_pointer_cast<TaiheCapturerPositionCallback>(taiheCapturer->positionCbTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    std::function<int32_t(std::shared_ptr<TaiheCapturerPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction)> removeFunction =
        [&taiheCapturer] (std::shared_ptr<TaiheCapturerPositionCallback> callbackPtr,
        std::shared_ptr<uintptr_t> callbackFunction) {
            taiheCapturer->audioCapturer_->UnsetCapturerPositionCallback();
            AUDIO_INFO_LOG("UnregisterCapturerPositionCallback success");
            return OHOS::AudioStandard::SUCCESS;
        };
    UnregisterAudioCapturerSingletonCallbackTemplate(callback, cbName, cb, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterCapturerPositionCallback is successful");
}

AudioState AudioCapturerImpl::GetState()
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return AudioState::key_t::STATE_INVALID;
    }
    OHOS::AudioStandard::CapturerState state = audioCapturer_->GetStatus();
    return TaiheAudioEnum::ToTaiheAudioState(state);
}

void AudioCapturerImpl::StartSync()
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return;
    }
    bool ret = audioCapturer_->Start();
    if (!ret) {
        AUDIO_ERR_LOG("StartSync failure!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

void AudioCapturerImpl::StopSync()
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return;
    }
    bool ret = audioCapturer_->Stop();
    if (!ret) {
        AUDIO_ERR_LOG("StopSync failure!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

void AudioCapturerImpl::ReleaseSync()
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return;
    }
    bool ret = audioCapturer_->Release();
    if (!ret) {
        AUDIO_ERR_LOG("ReleaseSync failure!");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

int64_t AudioCapturerImpl::GetBufferSizeSync()
{
    size_t bufferSize = 0;
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return DEFAULT_BUFFER_SIZE;
    }
    if (audioCapturer_->GetBufferSize(bufferSize) != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetBufferSizeSync failure!");
        return DEFAULT_BUFFER_SIZE;
    }
    return static_cast<int64_t>(bufferSize);
}

AudioCapturerInfo AudioCapturerImpl::GetCapturerInfoSync()
{
    AudioCapturerInfo emptyAudioCapturerInfo {
        .source = ohos::multimedia::audio::SourceType::key_t::SOURCE_TYPE_INVALID,
        .capturerFlags = 0
    };
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return emptyAudioCapturerInfo;
    }
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    int32_t ret = audioCapturer_->GetCapturerInfo(capturerInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCapturerInfo failure!");
        return emptyAudioCapturerInfo;
    }
    AudioCapturerInfo result = TaiheParamUtils::ToTaiheCapturerInfo(capturerInfo);
    return result;
}

AudioStreamInfo AudioCapturerImpl::GetStreamInfoSync()
{
    AudioStreamInfo emptyStreamInfo {
        .samplingRate = ohos::multimedia::audio::AudioSamplingRate::key_t::SAMPLE_RATE_48000,
        .channels = ohos::multimedia::audio::AudioChannel::key_t::CHANNEL_2,
        .sampleFormat = ohos::multimedia::audio::AudioSampleFormat::key_t::SAMPLE_FORMAT_S16LE,
        .encodingType = ohos::multimedia::audio::AudioEncodingType::key_t::ENCODING_TYPE_RAW,
    };
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return emptyStreamInfo;
    }
    OHOS::AudioStandard::AudioStreamInfo streamInfo;
    int32_t ret = audioCapturer_->GetStreamInfo(streamInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetStreamInfo failure!");
        return emptyStreamInfo;
    }
    std::shared_ptr<OHOS::AudioStandard::AudioStreamInfo> streamInfoPtr =
        std::make_shared<OHOS::AudioStandard::AudioStreamInfo>(streamInfo);
    AudioStreamInfo result = TaiheParamUtils::ToTaiheAudioStreamInfo(streamInfoPtr);
    return result;
}

int64_t AudioCapturerImpl::GetAudioStreamIdSync()
{
    uint32_t audioStreamId = 0;
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return static_cast<int64_t>(audioStreamId);
    }
    int32_t ret = audioCapturer_->GetAudioStreamId(audioStreamId);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERR_INVALID_INDEX) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioStreamId failure!");
        } else if (ret == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetAudioStreamId failure!");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioStreamId failure!");
        }
        return audioStreamId;
    }
    return static_cast<int64_t>(audioStreamId);
}

int64_t AudioCapturerImpl::GetAudioTimeSync()
{
    int64_t resultTime = 0;
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return resultTime;
    }
    OHOS::AudioStandard::Timestamp timestamp;
    bool ret = audioCapturer_->GetAudioTime(timestamp, OHOS::AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (ret != true) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioTime failure!");
        return resultTime;
    }
    uint64_t time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
        static_cast<uint64_t>(timestamp.time.tv_sec) * SEC_TO_NANOSECOND;
    resultTime = static_cast<int64_t>(time);
    return resultTime;
}

AudioTimestampInfo AudioCapturerImpl::GetAudioTimestampInfoSync()
{
    AudioTimestampInfo emptyTimestampInfo {
        .framePos = 0,
        .timestamp = 0
    };
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return emptyTimestampInfo;
    }

    OHOS::AudioStandard::Timestamp timestamp;
    int32_t ret = audioCapturer_->GetAudioTimestampInfo(timestamp,
        OHOS::AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetAudioTimestampInfo failure!");
        return emptyTimestampInfo;
    }
    AudioTimestampInfo result = TaiheParamUtils::ToTaiheAudioTimestampInfo(timestamp);
    return result;
}

int64_t AudioCapturerImpl::GetOverflowCountSync()
{
    int64_t retOverflowCount = 0;
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return retOverflowCount;
    }
    uint32_t overflowCount = audioCapturer_->GetOverflowCount();
    return static_cast<int64_t>(overflowCount);
}

void AudioCapturerImpl::SetWillMuteWhenInterruptedSync(bool muteWhenInterrupted)
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return;
    }
    int32_t ret = audioCapturer_->SetInterruptStrategy(muteWhenInterrupted ?
        OHOS::AudioStandard::InterruptStrategy::MUTE : OHOS::AudioStandard::InterruptStrategy::DEFAULT);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "SetWillMuteWhenInterrupted failed");
        return;
    }
}

taihe::array<AudioDeviceDescriptor> AudioCapturerImpl::GetCurrentInputDevices()
{
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return taihe::array<AudioDeviceDescriptor>(nullptr, DEFAULT_ARRAY_SIZE);
    }
    OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo(OHOS::AudioStandard::AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = audioCapturer_->GetCurrentInputDevices(deviceInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCurrentInputDevices failure!");
        return taihe::array<AudioDeviceDescriptor>(nullptr, DEFAULT_ARRAY_SIZE);
    }
    return TaiheParamUtils::SetValueDeviceInfo(deviceInfo);
}

AudioCapturerChangeInfo AudioCapturerImpl::GetCurrentAudioCapturerChangeInfo()
{
    AudioCapturerInfo emptyCaptureInfo {
        .source = ohos::multimedia::audio::SourceType::key_t::SOURCE_TYPE_INVALID,
        .capturerFlags = 0,
    };
    std::vector<AudioDeviceDescriptor> emptyDeviceDescriptor;
    AudioCapturerChangeInfo emptyCapturerChangeInfo {
        .streamId = 0,
        .clientUid = 0,
        .capturerInfo = emptyCaptureInfo,
        .capturerState = ohos::multimedia::audio::AudioState::key_t::STATE_INVALID,
        .deviceDescriptors = taihe::array<AudioDeviceDescriptor>(emptyDeviceDescriptor),
    };
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return emptyCapturerChangeInfo;
    }
    OHOS::AudioStandard::AudioCapturerChangeInfo capturerChangeInfo;
    int32_t ret = audioCapturer_->GetCurrentCapturerChangeInfo(capturerChangeInfo);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "GetCurrentCapturerChangeInfo failure!");
        return emptyCapturerChangeInfo;
    }

    std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo> capturerChangeInfoPtr =
        std::make_shared<OHOS::AudioStandard::AudioCapturerChangeInfo>(capturerChangeInfo);
    AudioCapturerChangeInfo result = TaiheParamUtils::ToTaiheAudioCapturerChangeInfo(capturerChangeInfoPtr);
    return result;
}

void AudioCapturerImpl::SetInputDeviceToAccessory()
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
        return;
    }
    int32_t ret = audioCapturer_->SetInputDevice(OHOS::AudioStandard::DeviceType::DEVICE_TYPE_ACCESSORY);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "Illegal state");
        return;
    }
}

void AudioCapturerImpl::RegisterCapturerCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->callbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "callbackTaihe_ is nullptr");

    std::shared_ptr<TaiheAudioCapturerCallback> cb =
        std::static_pointer_cast<TaiheAudioCapturerCallback>(taiheCapturer->callbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);

    if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME)) {
        OHOS::AudioStandard::CapturerState state = taiheCapturer->audioCapturer_->GetStatus();
        if (state == OHOS::AudioStandard::CAPTURER_PREPARED) {
            taiheCapturer->callbackTaihe_->OnStateChange(state);
        }
    }
}

void AudioCapturerImpl::RegisterAudioCapturerDeviceChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    std::shared_ptr<TaiheAudioCapturerDeviceChangeCallback> cb = nullptr;
    GetCapturerTaiheCallback(callback, cbName, taiheCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer device callback!");

    cb = std::make_shared<TaiheAudioCapturerDeviceChangeCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, callback);
    int32_t ret =
        taiheCapturer->audioCapturer_->SetAudioCapturerDeviceChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Registering of capturer device change callback failed");

    taiheCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerDeviceChangeCallback is successful");
}

void AudioCapturerImpl::RegisterAudioCapturerInfoChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    std::shared_ptr<TaiheAudioCapturerInfoChangeCallback> cb = nullptr;
    GetCapturerTaiheCallback(callback, cbName, taiheCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer info change callback!");
    cb = std::make_shared<TaiheAudioCapturerInfoChangeCallback>();

    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, callback);
    int32_t ret =
        taiheCapturer->audioCapturer_->SetAudioCapturerInfoChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Registering of capturer info change callback failed");

    taiheCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerInfoChangeCallback is successful");
}

void AudioCapturerImpl::RegisterCapturerReadDataCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->capturerReadDataCallbackTaihe_ == nullptr, "readData already subscribed.");

    taiheCapturer->capturerReadDataCallbackTaihe_ = std::make_shared<TaiheCapturerReadDataCallback>(taiheCapturer);
    taiheCapturer->audioCapturer_->SetCaptureMode(OHOS::AudioStandard::CAPTURE_MODE_CALLBACK);
    CHECK_AND_RETURN_LOG(taiheCapturer->capturerReadDataCallbackTaihe_ != nullptr, "readDataTaihe_ is nullptr");
    int32_t ret = taiheCapturer->audioCapturer_->SetCapturerReadCallback(taiheCapturer->capturerReadDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "SetCapturerCallback failed");
    std::shared_ptr<TaiheCapturerReadDataCallback> cb =
        std::static_pointer_cast<TaiheCapturerReadDataCallback>(taiheCapturer->capturerReadDataCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->AddCallbackReference(cbName, callback);

    AUDIO_INFO_LOG("Register Callback is successful");
}

void AudioCapturerImpl::RegisterPeriodPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(frame > 0, TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
        "parameter verification failed: The param of frame is not supported"), "frame value not supported");

    CHECK_AND_RETURN_RET_LOG(taiheCapturer->periodPositionCbTaihe_ == nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM),
        "periodReach already subscribed.");

    std::shared_ptr<TaiheCapturerPeriodPositionCallback> cb =
        std::make_shared<TaiheCapturerPeriodPositionCallback>();
    CHECK_AND_RETURN_RET_LOG(cb != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "periodPositionCbTaihe_ id nullptr");
    taiheCapturer->periodPositionCbTaihe_ = cb;

    int32_t ret = taiheCapturer->audioCapturer_->SetCapturerPeriodPositionCallback(frame, cb);
    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "SetCapturerPeriodPositionCallback failed");

    cb->SaveCallbackReference(cbName, callback);
}

void AudioCapturerImpl::RegisterPositionCallback(int64_t frame, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioCapturerImpl *taiheCapturer)
{
    CHECK_AND_RETURN_RET_LOG(taiheCapturer != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "taiheCapturer is nullptr");
    std::lock_guard<std::mutex> lock(taiheCapturer->mutex_);
    CHECK_AND_RETURN_RET_LOG(taiheCapturer->audioCapturer_ != nullptr, TaiheAudioError::ThrowErrorAndReturn(
        TAIHE_ERR_NO_MEMORY), "audioCapturer_ is nullptr");
    AUDIO_INFO_LOG("AudioCapturerImpl:RegisterPositionCallback start!");
    if (frame > 0) {
        taiheCapturer->positionCbTaihe_ = std::make_shared<TaiheCapturerPositionCallback>();
        CHECK_AND_RETURN_RET_LOG(taiheCapturer->positionCbTaihe_ != nullptr,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "positionCbTaihe_ is nullptr");
        int32_t ret = taiheCapturer->audioCapturer_->SetCapturerPositionCallback(frame,
            taiheCapturer->positionCbTaihe_);
        CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS,
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "SetCapturerPositionCallback failed");

        std::shared_ptr<TaiheCapturerPositionCallback> cb =
            std::static_pointer_cast<TaiheCapturerPositionCallback>(taiheCapturer->positionCbTaihe_);
        CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
        cb->SaveCallbackReference(cbName, callback);
    } else {
        AUDIO_ERR_LOG("AudioCapturerImpl: Mark Position value not supported!!");
        CHECK_AND_RETURN_RET_LOG(false, TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of frame is not supported"), "invailed callback");
    }
}

void AudioCapturerImpl::OnAudioInterrupt(callback_view<void(InterruptEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCapturerCallback(cacheCallback, AUDIO_INTERRUPT_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnStateChange(callback_view<void(AudioState)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCapturerCallback(cacheCallback, STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnInputDeviceChange(callback_view<void(array_view<AudioDeviceDescriptor>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioCapturerDeviceChangeCallback(cacheCallback, INPUTDEVICE_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnAudioCapturerChange(callback_view<void(AudioCapturerChangeInfo const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAudioCapturerInfoChangeCallback(cacheCallback, AUDIO_CAPTURER_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnReadData(callback_view<void(array_view<uint8_t>)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCapturerReadDataCallback(cacheCallback, READ_DATA_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnPeriodReach(int64_t frame, callback_view<void(int64_t)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPeriodPositionCallback(frame, cacheCallback, PERIOD_REACH_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OnMarkReach(int64_t frame, callback_view<void(int64_t)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterPositionCallback(frame, cacheCallback, MARK_REACH_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffStateChange(optional_view<callback<void(AudioState)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerCallback(cacheCallback, STATE_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffAudioInterrupt(optional_view<callback<void(InterruptEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerCallback(cacheCallback, AUDIO_INTERRUPT_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffInputDeviceChange(optional_view<callback<void(array_view<AudioDeviceDescriptor>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAudioCapturerDeviceChangeCallback(cacheCallback, INPUTDEVICE_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffAudioCapturerChange(optional_view<callback<void(AudioCapturerChangeInfo const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAudioCapturerInfoChangeCallback(cacheCallback, AUDIO_CAPTURER_CHANGE_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffReadData(optional_view<callback<void(array_view<uint8_t>)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerReadDataCallback(cacheCallback, READ_DATA_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffPeriodReach(optional_view<callback<void(int64_t)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerPeriodPositionCallback(cacheCallback, PERIOD_REACH_CALLBACK_NAME, this);
}

void AudioCapturerImpl::OffMarkReach(optional_view<callback<void(int64_t)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCapturerPositionCallback(cacheCallback, MARK_REACH_CALLBACK_NAME, this);
}

AudioCapturer CreateAudioCapturerSync(AudioCapturerOptions const &options)
{
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions;
    if (TaiheParamUtils::GetCapturerOptions(&capturerOptions, options) != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INPUT_INVALID,
            "parameter verification failed: The param of options must be interface AudioCapturerOptions");
        AUDIO_ERR_LOG("get captureOptions failed");
        return make_holder<AudioCapturerImpl, AudioCapturer>(nullptr);
    }
    return AudioCapturerImpl::CreateAudioCapturerWrapper(capturerOptions);
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_CreateAudioCapturerSync(ANI::Audio::CreateAudioCapturerSync);