/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioAccessoryInputStream"
#endif

#include "audio_accessory_input_stream.h"

#include <limits>

#include "audio_accessory.h"
#include "audio_common_log.h"
#include "audio_device_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t CYCLES_PER_SECOND = 50;

uint32_t GetAudioAccessorySampleBytes(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return 1;
        case SAMPLE_S16LE:
            return 2;
        case SAMPLE_S24LE:
            return 3;
        case SAMPLE_S32LE:
        case SAMPLE_F32LE:
            return 4;
        default:
            return 0;
    }
}

AudioAccessoryInputStream::AudioAccessoryInputStream(AudioAccessory &accessory,
    const VAAudioStreamProperty &streamProperty) : accessory_(accessory), streamProperty_(streamProperty)
{}

int32_t AudioAccessoryInputStream::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!released_ && startCallback_ != nullptr, ERROR, "invalid start state");
    bool ret = startCallback_(reinterpret_cast<OH_AudioAccessory *>(&accessory_),
        reinterpret_cast<OH_AudioAccessoryInputStream *>(this));
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "start callback failed");
    started_ = true;
    return SUCCESS;
}

int32_t AudioAccessoryInputStream::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!released_ && stopCallback_ != nullptr, ERROR, "invalid stop state");
    bool ret = stopCallback_(reinterpret_cast<OH_AudioAccessory *>(&accessory_),
        reinterpret_cast<OH_AudioAccessoryInputStream *>(this));
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "stop callback failed");
    started_ = false;
    return SUCCESS;
}

int32_t AudioAccessoryInputStream::Close()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!released_ && releaseCallback_ != nullptr, ERROR, "invalid close state");
    bool ret = releaseCallback_(reinterpret_cast<OH_AudioAccessory *>(&accessory_),
        reinterpret_cast<OH_AudioAccessoryInputStream *>(this));
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "release callback failed");
    released_ = true;
    started_ = false;
    bufferOperator_ = nullptr;
    sharedBuffer_ = nullptr;
    return SUCCESS;
}

int32_t AudioAccessoryInputStream::GetStreamProperty(VAAudioStreamProperty &streamProp)
{
    std::lock_guard<std::mutex> lock(mutex_);
    streamProp = streamProperty_;
    return SUCCESS;
}

int32_t AudioAccessoryInputStream::RequestSharedMem(const VASharedMemInfo &memInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!released_, ERROR, "stream is released");
    sharedBuffer_ = VASharedBuffer::CreateFromRemote(memInfo);
    CHECK_AND_RETURN_RET_LOG(sharedBuffer_ != nullptr, ERROR, "create va shared buffer failed");
    bufferOperator_ = std::make_shared<VASharedBufferOperator>(*sharedBuffer_);
    CHECK_AND_RETURN_RET_LOG(bufferOperator_ != nullptr, ERROR, "create buffer operator failed");
    return SUCCESS;
}

int32_t AudioAccessoryInputStream::GetCapturePosition(uint64_t &frames, uint64_t &timestamp)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!released_ && framePositionCallback_ != nullptr, ERROR, "invalid position state");
    int64_t framePosition = 0;
    int64_t frameTimestamp = 0;
    bool ret = framePositionCallback_(reinterpret_cast<OH_AudioAccessory *>(&accessory_),
        reinterpret_cast<OH_AudioAccessoryInputStream *>(this), &framePosition, &frameTimestamp);
    CHECK_AND_RETURN_RET_LOG(ret && framePosition >= 0 && frameTimestamp >= 0, ERROR, "position callback failed");
    frames = static_cast<uint64_t>(framePosition);
    timestamp = static_cast<uint64_t>(frameTimestamp);
    return SUCCESS;
}

void AudioAccessoryInputStream::NotifyStartStreamFailed()
{
    AUDIO_INFO_LOG("NotifyStartStreamFailed called");
    accessory_.NotifyStartStreamFailed();
}

OH_AudioCommon_Result AudioAccessoryInputStream::RegisterStartCallback(
    OH_AudioAccessoryInputStream_StartCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(registering_ && !released_, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "register start callback in illegal state");
    startCallback_ = callback;
    registeredMask_ |= 0x1;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result AudioAccessoryInputStream::RegisterStopCallback(
    OH_AudioAccessoryInputStream_StopCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(registering_ && !released_, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "register stop callback in illegal state");
    stopCallback_ = callback;
    registeredMask_ |= 0x2;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result AudioAccessoryInputStream::RegisterReleaseCallback(
    OH_AudioAccessoryInputStream_ReleaseCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(registering_ && !released_, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "register release callback in illegal state");
    releaseCallback_ = callback;
    registeredMask_ |= 0x4;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

bool AudioAccessoryInputStream::AreAllCallbacksRegistered() const
{
    return (registeredMask_.load() & REQUIRED_MASK) == REQUIRED_MASK;
}

std::string AudioAccessoryInputStream::GetMissingCallbacksInfo() const
{
    uint32_t mask = registeredMask_.load();
    std::string info;
    if (!(mask & 0x1)) {
        info += "onStart ";
    }
    if (!(mask & 0x2)) {
        info += "onStop ";
    }
    if (!(mask & 0x4)) {
        info += "onRelease ";
    }
    return info;
}

void AudioAccessoryInputStream::ForceRelease()
{
    {
        std::lock_guard<std::mutex> lock(streamMutex_);
        if (state_ == State::RELEASED) {
            return;
        }
        state_ = State::RELEASED;
        bufferOperator_ = nullptr;
    }
    SafeCallOnRelease();
}

void AudioAccessoryInputStream::SafeCallOnRelease()
{
    bool expected = false;
    if (!callbackInProgress_.compare_exchange_strong(expected, true)) {
        return;
    }

    struct CallbackGuard {
        std::atomic<bool>& flag;
        ~CallbackGuard() { flag.store(false); }
    } guard{callbackInProgress_};

    State currentState;
    OH_AudioAccessoryInputStream_ReleaseCallback localCallback;
    {
        std::lock_guard<std::mutex> lock(streamMutex_);
        currentState = state_;
        localCallback = releaseCallback_;
    }
    if (currentState == State::RELEASED) {
        return;
    }

    if (auto accessory = accessoryWeak_.lock()) {
        auto accessoryHandle = reinterpret_cast<OH_AudioAccessory*>(accessory.get());
        auto streamHandle = reinterpret_cast<OH_AudioAccessoryInputStream*>(this);
        if (localCallback) {
            localCallback(accessoryHandle, streamHandle);
        }
    }
}

void AudioAccessoryInputStream::SafeCallOnStart()
{
    bool expected = false;
    if (!callbackInProgress_.compare_exchange_strong(expected, true)) {
        return;
    }

    struct CallbackGuard {
        std::atomic<bool>& flag;
        ~CallbackGuard() { flag.store(false); }
    } guard{callbackInProgress_};

    State currentState;
    OH_AudioAccessoryInputStream_StartCallback localCallback;
    {
        std::lock_guard<std::mutex> lock(streamMutex_);
        currentState = state_;
        localCallback = onStart_;
    }
    if (currentState == State::RELEASED) {
        return;
    }

    if (auto accessory = accessoryWeak_.lock()) {
        auto accessoryHandle = reinterpret_cast<OH_AudioAccessory*>(accessory.get());
        auto streamHandle = reinterpret_cast<OH_AudioAccessoryInputStream*>(this);
        if (localCallback) {
            localCallback(accessoryHandle, streamHandle);
        }
    }
}

OH_AudioCommon_Result AudioAccessoryInputStream::RegisterLatencyCallback(
    OH_AudioAccessoryInputStream_GetLatencyCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(registering_ && !released_, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "register latency callback in illegal state");
    latencyCallback_ = callback;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result AudioAccessoryInputStream::RegisterFramePositionCallback(
    OH_AudioAccessoryInputStream_GetFramePositionCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(registering_ && !released_, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "register frame position callback in illegal state");
    framePositionCallback_ = callback;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result AudioAccessoryInputStream::Write(const uint8_t *data, uint32_t dataSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(data != nullptr && dataSize > 0, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "invalid write parameter");
    CHECK_AND_RETURN_RET_LOG(started_ && !released_ && bufferOperator_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "stream is not writable");
    size_t frameSize = GetFrameSizeInBytes();
    CHECK_AND_RETURN_RET_LOG(frameSize > 0 && dataSize == frameSize,
        AUDIOCOMMON_RESULT_ERROR_FRAME_LENGTH_MISMATCH, "frame length mismatch");
    size_t written = bufferOperator_->Write(const_cast<uint8_t *>(data), dataSize);
    CHECK_AND_RETURN_RET_LOG(written == dataSize, AUDIOCOMMON_RESULT_ERROR_SYSTEM, "write shared buffer failed");
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result AudioAccessoryInputStream::GetWritableSize(uint32_t *writableSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(writableSize != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "writableSize is null");
    CHECK_AND_RETURN_RET_LOG(!released_ && bufferOperator_ != nullptr, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
        "stream buffer is not ready");
    size_t size = bufferOperator_->GetWritableSizeNoLock();
    *writableSize = size > std::numeric_limits<uint32_t>::max() ? std::numeric_limits<uint32_t>::max() :
        static_cast<uint32_t>(size);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

bool AudioAccessoryInputStream::HasRequiredCallbacks() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return startCallback_ != nullptr && stopCallback_ != nullptr && releaseCallback_ != nullptr &&
        latencyCallback_ != nullptr && framePositionCallback_ != nullptr;
}

void AudioAccessoryInputStream::SetRegistering(bool registering)
{
    std::lock_guard<std::mutex> lock(mutex_);
    registering_ = registering;
}

size_t AudioAccessoryInputStream::GetFrameSizeInBytes() const
{
    uint32_t sampleBytes = GetAudioAccessorySampleBytes(streamProperty_.sampleFormat_);
    uint8_t channels = ConvertLayoutToAudioChannel(streamProperty_.channelLayout_);
    uint32_t samplesPerCycle = streamProperty_.samplesPerCycle_ != 0 ? streamProperty_.samplesPerCycle_ :
        streamProperty_.sampleRate_ / CYCLES_PER_SECOND;
    return static_cast<size_t>(channels) * samplesPerCycle * sampleBytes;
}

} // namespace AudioStandard
} // namespace OHOS
