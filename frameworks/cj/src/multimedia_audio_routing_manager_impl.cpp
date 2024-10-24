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
#include "multimedia_audio_routing_manager_impl.h"
#include "cj_lambda.h"
#include "audio_info.h"
#include "audio_log.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioRoutingManagerImpl::MMAAudioRoutingManagerImpl()
{
    routingMgr_ = AudioRoutingManager::GetInstance();
    audioMgr_ = AudioSystemManager::GetInstance();
    microphoneBlockedCallback_ = std::make_shared<CjAudioManagerMicrophoneBlockedCallback>();
    preferredInputDeviceChangeCallBack_ = std::make_shared<CjAudioPreferredInputDeviceChangeCallback>();
    deviceChangeCallBack_ = std::make_shared<CjAudioManagerDeviceChangeCallback>();
}

MMAAudioRoutingManagerImpl::~MMAAudioRoutingManagerImpl()
{
    routingMgr_ = nullptr;
    audioMgr_ = nullptr;
}

bool MMAAudioRoutingManagerImpl::IsCommunicationDeviceActive(int32_t deviceType)
{
    return audioMgr_->IsDeviceActive(static_cast<ActiveDeviceType>(deviceType));
}

int32_t MMAAudioRoutingManagerImpl::SetCommunicationDevice(int32_t deviceType, bool active)
{
    auto ret = audioMgr_->SetDeviceActive(static_cast<ActiveDeviceType>(deviceType), active);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("set communication device failure!");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetDevices(int32_t flags, int32_t *errorCode)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors = audioMgr_->GetDevices(static_cast<DeviceFlag>(flags));
    if (deviceDescriptors.empty()) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor arr;
    Convert2CArrDeviceDescriptor(arr, deviceDescriptors, errorCode);
    return arr;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetPreferredInputDeviceForCapturerInfo(CAudioCapturerInfo cInfo,
    int32_t *errorCode)
{
    std::vector<sptr<AudioDeviceDescriptor>> outDeviceDescriptors;
    AudioCapturerInfo capturerInfo(static_cast<SourceType>(cInfo.source), cInfo.capturerFlags);
    routingMgr_->GetPreferredInputDeviceForCapturerInfo(capturerInfo, outDeviceDescriptors);
    if (outDeviceDescriptors.empty()) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor arr;
    Convert2CArrDeviceDescriptor(arr, outDeviceDescriptors, errorCode);
    return arr;
}

void MMAAudioRoutingManagerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::MICROPHONE_BLOCKED) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register microphoneBlocked event failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
        microphoneBlockedCallback_->RegisterFunc(func);
        audioMgr_->SetMicrophoneBlockedCallback(microphoneBlockedCallback_);
    }
}

void MMAAudioRoutingManagerImpl::RegisterPreferredInputDeviceChangeCallback(int32_t callbackType, void (*callback)(),
    CAudioCapturerInfo info, int32_t *errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::INPUT_DEVICE_CHANGE_FOR_CAPTURER_INFO) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register preferredInputDeviceChangeForCapturerInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
        AudioCapturerInfo capturerInfo(static_cast<SourceType>(info.source), info.capturerFlags);
        preferredInputDeviceChangeCallBack_->RegisterFunc(func);
        routingMgr_->SetPreferredInputDeviceChangeCallback(capturerInfo, preferredInputDeviceChangeCallBack_);
    }
}

void MMAAudioRoutingManagerImpl::RegisterDeviceChangeCallback(int32_t callbackType, void (*callback)(), int32_t flags,
    int32_t *errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::DEVICE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CDeviceChangeAction)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register DeviceChangeAction event failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
        DeviceFlag deviceFlag = static_cast<DeviceFlag>(flags);
        deviceChangeCallBack_->RegisterFunc(func);
        audioMgr_->SetDeviceChangeCallback(deviceFlag, deviceChangeCallBack_);
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
