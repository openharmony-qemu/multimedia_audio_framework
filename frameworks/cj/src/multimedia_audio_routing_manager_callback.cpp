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
#include "multimedia_audio_common.h"

namespace OHOS {
namespace AudioStandard {
void CjAudioManagerMicrophoneBlockedCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioManagerMicrophoneBlockedCallback::OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CArrDeviceDescriptor arr;
    int32_t *errorCode = static_cast<int32_t *>(malloc(sizeof(int32_t)));
    Convert2CArrDeviceDescriptor(arr, microphoneBlockedInfo.devices, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        free(errorCode);
        errorCode = nullptr;
        return;
    }
    func_(arr);
    FreeCArrDeviceDescriptor(arr);
    free(errorCode);
    errorCode = nullptr;
}

void CjAudioPreferredInputDeviceChangeCallback::RegisterFunc(std::function<void(CArrDeviceDescriptor)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioPreferredInputDeviceChangeCallback::OnPreferredInputDeviceUpdated(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CArrDeviceDescriptor arr;
    int32_t *errorCode = static_cast<int32_t *>(malloc(sizeof(int32_t)));
    Convert2CArrDeviceDescriptor(arr, desc, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        free(errorCode);
        errorCode = nullptr;
        return;
    }
    func_(arr);
    FreeCArrDeviceDescriptor(arr);
    free(errorCode);
    errorCode = nullptr;
}

void CjAudioManagerDeviceChangeCallback::RegisterFunc(std::function<void(CDeviceChangeAction)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioManagerDeviceChangeCallback::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CArrDeviceDescriptor arr;
    int32_t *errorCode = static_cast<int32_t *>(malloc(sizeof(int32_t)));
    Convert2CArrDeviceDescriptor(arr, deviceChangeAction.deviceDescriptors, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        free(errorCode);
        errorCode = nullptr;
        return;
    }
    CDeviceChangeAction action;
    action.deviceDescriptors = arr;
    action.changeType = deviceChangeAction.type;
    func_(action);
    FreeCArrDeviceDescriptor(arr);
    free(errorCode);
    errorCode = nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
