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
#include "multimedia_audio_routing_manager_impl.h"

#include "audio_policy_log.h"
#include "cj_lambda.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioRoutingManagerImpl::MMAAudioRoutingManagerImpl()
{
    routingMgr_ = AudioRoutingManager::GetInstance();
    audioMgr_ = AudioSystemManager::GetInstance();
    deviceUsageCallback_ = std::make_shared<CjAudioManagerAvailableDeviceChangeCallback>();
    preferredInputDeviceChangeCallBack_ = std::make_shared<CjAudioPreferredInputDeviceChangeCallback>();
    preferredOutputDeviceChangeCallBack_ = std::make_shared<CjAudioPreferredOutputDeviceChangeCallback>();
    deviceChangeCallBack_ = std::make_shared<CjAudioManagerDeviceChangeCallback>();
}

MMAAudioRoutingManagerImpl::~MMAAudioRoutingManagerImpl()
{
    routingMgr_ = nullptr;
    audioMgr_ = nullptr;
}

bool MMAAudioRoutingManagerImpl::IsCommunicationDeviceActive(int32_t deviceType)
{
    return audioMgr_->IsDeviceActive(static_cast<DeviceType>(deviceType));
}

int32_t MMAAudioRoutingManagerImpl::SetCommunicationDevice(int32_t deviceType, bool active)
{
    auto ret = audioMgr_->SetDeviceActive(static_cast<DeviceType>(deviceType), active);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("set communication device failure!");
        return CJ_ERR_SYSTEM;
    }
    return SUCCESS_CODE;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetDevices(int32_t flags, int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors =
        audioMgr_->GetDevices(static_cast<DeviceFlag>(flags));
    if (deviceDescriptors.empty()) {
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor arr {};
    Convert2CArrDeviceDescriptor(arr, deviceDescriptors, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return CArrDeviceDescriptor();
    }
    return arr;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetAvailableDevices(uint32_t deviceUsage, int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors =
        routingMgr_->GetAvailableDevices(static_cast<AudioDeviceUsage>(deviceUsage));
    CArrDeviceDescriptor arr {};
    if (deviceDescriptors.empty()) {
        return arr;
    }
    Convert2CArrDeviceDescriptor(arr, deviceDescriptors, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return CArrDeviceDescriptor();
    }
    return arr;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetPreferredInputDeviceForCapturerInfo(
    CAudioCapturerInfo cInfo, int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outDeviceDescriptors {};
    AudioCapturerInfo capturerInfo(static_cast<SourceType>(cInfo.source), cInfo.capturerFlags);
    routingMgr_->GetPreferredInputDeviceForCapturerInfo(capturerInfo, outDeviceDescriptors);
    if (outDeviceDescriptors.empty()) {
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor arr {};
    Convert2CArrDeviceDescriptor(arr, outDeviceDescriptors, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return CArrDeviceDescriptor();
    }
    return arr;
}

CArrDeviceDescriptor MMAAudioRoutingManagerImpl::GetPreferredOutputDeviceForRendererInfo(
    CAudioRendererInfo cInfo, int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outDeviceDescriptors {};
    AudioRendererInfo rendererInfo {};
    rendererInfo.streamUsage = static_cast<StreamUsage>(cInfo.usage);
    rendererInfo.rendererFlags = cInfo.rendererFlags;
    routingMgr_->GetPreferredOutputDeviceForRendererInfo(rendererInfo, outDeviceDescriptors);
    if (outDeviceDescriptors.empty()) {
        return CArrDeviceDescriptor();
    }
    CArrDeviceDescriptor arr {};
    Convert2CArrDeviceDescriptor(arr, outDeviceDescriptors, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        FreeCArrDeviceDescriptor(arr);
        return CArrDeviceDescriptor();
    }
    return arr;
}

void MMAAudioRoutingManagerImpl::RegisterCallback(
    int32_t callbackType, uint32_t deviceUsage, void (*callback)(), int32_t* errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::AVAILABLE_DEVICE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CDeviceChangeAction)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register avaibleDeviceChange event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        deviceUsageCallback_->RegisterFunc(deviceUsage, func);
        AudioDeviceUsage audioDeviceUsage = static_cast<AudioDeviceUsage>(deviceUsage);
        audioMgr_->SetAvailableDeviceChangeCallback(audioDeviceUsage, deviceUsageCallback_);
    }
}

void MMAAudioRoutingManagerImpl::RegisterPreferredInputDeviceChangeCallback(
    int32_t callbackType, void (*callback)(), CAudioCapturerInfo info, int32_t* errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::INPUT_DEVICE_CHANGE_FOR_CAPTURER_INFO) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register preferredInputDeviceChangeForCapturerInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        AudioCapturerInfo capturerInfo(static_cast<SourceType>(info.source), info.capturerFlags);
        preferredInputDeviceChangeCallBack_->RegisterFunc(func);
        routingMgr_->SetPreferredInputDeviceChangeCallback(capturerInfo, preferredInputDeviceChangeCallBack_);
    }
}

void MMAAudioRoutingManagerImpl::RegisterPreferredOutputDeviceChangeCallback(
    int32_t callbackType, void (*callback)(), CAudioRendererInfo info, int32_t* errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::OUTPUT_DEVICE_CHANGE_FOR_RENDERER_INFO) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrDeviceDescriptor)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register preferredOutputDeviceChangeForRendererInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        AudioRendererInfo rendererInfo {};
        rendererInfo.streamUsage = static_cast<StreamUsage>(info.usage);
        rendererInfo.rendererFlags = info.rendererFlags;
        preferredOutputDeviceChangeCallBack_->RegisterFunc(func);
        routingMgr_->SetPreferredOutputDeviceChangeCallback(rendererInfo, preferredOutputDeviceChangeCallBack_);
    }
}

void MMAAudioRoutingManagerImpl::RegisterDeviceChangeCallback(
    int32_t callbackType, void (*callback)(), int32_t flags, int32_t* errorCode)
{
    if (callbackType == AudioRoutingManagerCallbackType::DEVICE_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CDeviceChangeAction)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register DeviceChangeAction event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        DeviceFlag deviceFlag = static_cast<DeviceFlag>(flags);
        deviceChangeCallBack_->RegisterFunc(func);
        audioMgr_->SetDeviceChangeCallback(deviceFlag, deviceChangeCallBack_);
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
