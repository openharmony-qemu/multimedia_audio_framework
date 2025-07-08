/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "multimedia_audio_routing_manager_callback.h"

#include "audio_policy_log.h"
#include "multimedia_audio_common.h"

namespace OHOS {
namespace AudioStandard {
void CjAudioManagerAvailableDeviceChangeCallback::RegisterFunc(
    const uint32_t usage, std::function<void(CDeviceChangeAction)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    callbackList_.push_back({ usage, cjCallback });
}

void CjAudioManagerAvailableDeviceChangeCallback::OnAvailableDeviceChange(
    const AudioDeviceUsage usage, const DeviceChangeAction& deviceChangeAction)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    std::function<void(CDeviceChangeAction)> func;
    bool isFind { false };
    for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
        if (usage == it->first) {
            func = it->second;
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        AUDIO_ERR_LOG("[OnAvailableDeviceChange] Registered func is not found.");
        return;
    }
    CDeviceChangeAction cDeviceChangeAct {};
    cDeviceChangeAct.changeType = deviceChangeAction.type;
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptor(cDeviceChangeAct.deviceDescriptors, deviceChangeAction.deviceDescriptors, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(cDeviceChangeAct.deviceDescriptors);
        return;
    }
    func(cDeviceChangeAct);
    FreeCArrDeviceDescriptor(cDeviceChangeAct.deviceDescriptors);
}

void CjAudioPreferredInputDeviceChangeCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    func_ = cjCallback;
}

void CjAudioPreferredInputDeviceChangeCallback::OnPreferredInputDeviceUpdated(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrDeviceDescriptor arr {};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptor(arr, desc, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return;
    }
    func_(arr);
    FreeCArrDeviceDescriptor(arr);
}

void CjAudioPreferredOutputDeviceChangeCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    func_ = cjCallback;
}

void CjAudioPreferredOutputDeviceChangeCallback::OnPreferredOutputDeviceUpdated(
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CArrDeviceDescriptor arr {};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptor(arr, desc, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return;
    }
    func_(arr);
    FreeCArrDeviceDescriptor(arr);
}

void CjAudioManagerDeviceChangeCallback::RegisterFunc(std::function<void(CDeviceChangeAction)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    func_ = cjCallback;
}

void CjAudioManagerDeviceChangeCallback::OnDeviceChange(const DeviceChangeAction& deviceChangeAction)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrDeviceDescriptor arr {};
    int32_t errorCode = SUCCESS_CODE;
    Convert2CArrDeviceDescriptor(arr, deviceChangeAction.deviceDescriptors, &errorCode);
    if (errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return;
    }
    CDeviceChangeAction action {};
    action.deviceDescriptors = arr;
    action.changeType = deviceChangeAction.type;
    func_(action);
    FreeCArrDeviceDescriptor(arr);
}
} // namespace AudioStandard
} // namespace OHOS
