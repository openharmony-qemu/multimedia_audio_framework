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

#include "multimedia_audio_renderer_callback.h"
#include "multimedia_audio_common.h"

namespace OHOS {
namespace AudioStandard {
void CjRendererPositionCallback::RegisterFunc(std::function<void(int64_t)> cjCallback)
{
    func_ = cjCallback;
}

void CjRendererPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ != nullptr) {
        func_(framePosition);
    }
}

void CjRendererPeriodPositionCallback::RegisterFunc(std::function<void(int64_t)> cjCallback)
{
    func_ = cjCallback;
}

void CjRendererPeriodPositionCallback::OnPeriodReached(const int64_t &frameNumber)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ != nullptr) {
        func_(frameNumber);
    }
}

void CjAudioRendererOutputDeviceChangeCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioRendererOutputDeviceChangeCallback::OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
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

void CjAudioRendererOutputDeviceChangeWithInfoCallback::RegisterFunc(
    std::function<void(CAudioStreamDeviceChangeInfo)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioRendererOutputDeviceChangeWithInfoCallback::OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CAudioStreamDeviceChangeInfo info{};
    CArrDeviceDescriptor arr{};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptorByDeviceInfo(arr, deviceInfo, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return;
    }
    info.deviceDescriptors = arr;
    info.changeReason = static_cast<int32_t>(reason);
    func_(info);
    FreeCArrDeviceDescriptor(arr);
}

void CjAudioRendererWriteCallback::RegisterFunc(std::function<int32_t(CArrUI8)> cjCallback,
    std::shared_ptr<AudioRenderer> audioRenderer)
{
    func_ = cjCallback;
    audioRenderer_ = audioRenderer;
}

void CjAudioRendererWriteCallback::OnWriteData(size_t length)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrUI8 arr{};
    BufferDesc buf{};
    audioRenderer_->GetBufferDesc(buf);
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

void CjAudioRendererCallback::RegisterFunc(std::function<void(int32_t)> cjCallback)
{
    stateChangeCallback_ = cjCallback;
}

void CjAudioRendererCallback::RegisterInterruptFunc(std::function<void(CInterruptEvent)> cjCallback)
{
    interruptCallback_ = cjCallback;
}

void CjAudioRendererCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (interruptCallback_ == nullptr) {
        return;
    }
    CInterruptEvent event{};
    event.eventType = static_cast<int32_t>(interruptEvent.eventType);
    event.forceType = static_cast<int32_t>(interruptEvent.forceType);
    event.hintType = static_cast<int32_t>(interruptEvent.hintType);
    interruptCallback_(event);
}

void CjAudioRendererCallback::OnStateChange(const RendererState state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (stateChangeCallback_ == nullptr) {
        return;
    }
    stateChangeCallback_(static_cast<int32_t>(state));
}
} // namespace AudioStandard
} // namespace OHOS
