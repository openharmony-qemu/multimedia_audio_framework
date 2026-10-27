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

#ifndef AUDIO_ACCESSORY_INPUT_STREAM_H
#define AUDIO_ACCESSORY_INPUT_STREAM_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

#include "native_audio_accessory_input_stream_manager.h"
#include "va_shared_buffer_operator.h"
#include "virtual_audio_interface.h"

namespace OHOS {
namespace AudioStandard {

class AudioAccessory;

class AudioAccessoryInputStream final : public VAInputStreamCallback {
public:
    AudioAccessoryInputStream(AudioAccessory &accessory, const VAAudioStreamProperty &streamProperty);
    ~AudioAccessoryInputStream() = default;

    int32_t Start() override;
    int32_t Stop() override;
    int32_t Close() override;
    int32_t GetStreamProperty(VAAudioStreamProperty &streamProp) override;
    int32_t RequestSharedMem(const VASharedMemInfo &memInfo) override;
    int32_t GetCapturePosition(uint64_t &frames, uint64_t &timestamp) override;
    void NotifyStartStreamFailed() override;

    OH_AudioCommon_Result RegisterStartCallback(OH_AudioAccessoryInputStream_StartCallback callback);
    OH_AudioCommon_Result RegisterStopCallback(OH_AudioAccessoryInputStream_StopCallback callback);
    OH_AudioCommon_Result RegisterReleaseCallback(OH_AudioAccessoryInputStream_ReleaseCallback callback);
    bool AreAllCallbacksRegistered() const;
    std::string GetMissingCallbacksInfo() const;
    void ForceRelease();
    OH_AudioCommon_Result RegisterLatencyCallback(OH_AudioAccessoryInputStream_GetLatencyCallback callback);
    OH_AudioCommon_Result RegisterFramePositionCallback(
        OH_AudioAccessoryInputStream_GetFramePositionCallback callback);
    OH_AudioCommon_Result Write(const uint8_t *data, uint32_t dataSize);
    OH_AudioCommon_Result GetWritableSize(uint32_t *writableSize);
    bool HasRequiredCallbacks() const;

    void SetRegistering(bool registering);

private:
    enum class State { OPENING, OPENED, RUNNING, STOPPED, RELEASED };
    State state_ = State::OPENING;
    std::mutex streamMutex_;
    std::atomic<bool> callbackInProgress_ = false;

    std::shared_ptr<AudioAccessory> accessoryStrong_;
    std::weak_ptr<AudioAccessory> accessoryWeak_;
    VAAudioStreamProperty streamProp_;
    OH_AudioStreamInfo streamConfig_;

    size_t GetFrameSizeInBytes() const;

    AudioAccessory &accessory_;
    VAAudioStreamProperty streamProperty_;
    std::shared_ptr<VASharedBuffer> sharedBuffer_;
    std::shared_ptr<VASharedBufferOperator> bufferOperator_;
    OH_AudioAccessoryInputStream_StartCallback startCallback_ = nullptr;
    OH_AudioAccessoryInputStream_StopCallback stopCallback_ = nullptr;
    OH_AudioAccessoryInputStream_ReleaseCallback releaseCallback_ = nullptr;
    OH_AudioAccessoryInputStream_GetLatencyCallback latencyCallback_ = nullptr;
    OH_AudioAccessoryInputStream_GetFramePositionCallback framePositionCallback_ = nullptr;
    
    // Individual SDK-C callbacks
    OH_AudioAccessoryInputStream_StartCallback onStart_ = nullptr;
    
    // Bitmask: 1=onStart, 2=onStop, 4=onRelease
    std::atomic<uint32_t> registeredMask_ = 0;
    static constexpr uint32_t REQUIRED_MASK = 0x7; // 1 | 2 | 4
    void SafeCallOnStart();
    void SafeCallOnStop();
    void SafeCallOnRelease();

    bool registering_ = false;
    bool started_ = false;
    bool released_ = false;
    mutable std::mutex mutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_ACCESSORY_INPUT_STREAM_H
