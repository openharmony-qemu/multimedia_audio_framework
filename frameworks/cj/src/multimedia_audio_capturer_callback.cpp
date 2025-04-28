/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "multimedia_audio_capturer_callback.h"
#include "multimedia_audio_common.h"

namespace OHOS {
namespace AudioStandard {
void CjAudioCapturerCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (interruptEventfunc_ == nullptr) {
        return;
    }
    CInterruptEvent event{};
    event.eventType = static_cast<int32_t>(interruptEvent.eventType);
    event.forceType = static_cast<int32_t>(interruptEvent.forceType);
    event.hintType = static_cast<int32_t>(interruptEvent.hintType);
    interruptEventfunc_(event);
}

void CjAudioCapturerCallback::OnStateChange(const CapturerState state)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (stateChangefunc_ == nullptr) {
        return;
    }
    stateChangefunc_(static_cast<int32_t>(state));
}

void CjAudioCapturerCallback::RegisterInterruptFunc(std::function<void(CInterruptEvent)> cjCallback)
{
    interruptEventfunc_ = cjCallback;
}

void CjAudioCapturerCallback::RegisterStateChangeFunc(std::function<void(int32_t)> cjCallback)
{
    stateChangefunc_ = cjCallback;
}

void CjCapturerPositionCallback::RegisterFunc(std::function<void(int64_t)> cjCallback)
{
    func_ = cjCallback;
}

void CjCapturerPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ != nullptr) {
        func_(framePosition);
    }
}

void CjCapturerPeriodPositionCallback::RegisterFunc(std::function<void(int64_t)> cjCallback)
{
    func_ = cjCallback;
}
void CjCapturerPeriodPositionCallback::OnPeriodReached(const int64_t &frameNumber)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ != nullptr) {
        func_(frameNumber);
    }
}

void CjAudioCapturerReadCallback::RegisterFunc(std::function<void(CArrUI8)> cjCallback,
    std::shared_ptr<AudioCapturer> audioCapturer)
{
    func_ = cjCallback;
    audioCapturer_ = audioCapturer;
}

void CjAudioCapturerReadCallback::OnReadData(size_t length)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrUI8 arr{};
    BufferDesc buf{};
    audioCapturer_->GetBufferDesc(buf);
    if (buf.buffer == nullptr) {
        return;
    }
    arr.size = std::min(length, buf.bufLength);
    int32_t mallocSize = static_cast<int32_t>(sizeof(uint8_t)) * static_cast<int32_t>(arr.size);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(uint8_t) * MAX_MEM_MALLOC_SIZE)) {
        return;
    }
    arr.head = static_cast<uint8_t *>(malloc(mallocSize));
    if (arr.head == nullptr) {
        return;
    }
    if (memset_s(arr.head, mallocSize, 0, mallocSize) != EOK) {
        free(arr.head);
        arr.head = nullptr;
        return;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(arr.size); i++) {
        arr.head[i] = buf.buffer[i];
    }
    func_(arr);
    free(arr.head);
    arr.head = nullptr;
}

void CjAudioCapturerInfoChangeCallback::RegisterFunc(std::function<void(CAudioCapturerChangeInfo)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioCapturerInfoChangeCallback::OnStateChange(const AudioCapturerChangeInfo &capturerChangeInfo)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CAudioCapturerChangeInfo cInfo{};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CAudioCapturerChangeInfo(cInfo, capturerChangeInfo, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(cInfo.deviceDescriptors);
        return;
    }
    func_(cInfo);
    FreeCArrDeviceDescriptor(cInfo.deviceDescriptors);
}

void CjAudioCapturerDeviceChangeCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioCapturerDeviceChangeCallback::OnStateChange(const AudioDeviceDescriptor &deviceInfo)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrDeviceDescriptor arr{};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptorByDeviceInfo(arr, deviceInfo, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return;
    }
    func_(arr);
    FreeCArrDeviceDescriptor(arr);
}
} // namespace AudioStandard
} // namespace OHOS
