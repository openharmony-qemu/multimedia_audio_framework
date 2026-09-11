/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "VADeviceBrokerWrapperImpl"
#endif

#include "va_device_broker_wrapper_impl.h"
#include "va_device_controller_stub_impl.h"
#include "iv_a_device_broker.h"
#include "iservice_registry.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
VADeviceBrokerWrapperImpl::VADeviceBrokerWrapperImpl()
{
}

VADeviceBrokerWrapperImpl::~VADeviceBrokerWrapperImpl()
{
}
int32_t VADeviceBrokerWrapperImpl::OnDevicesConnected(
    const VADevice& device, const std::shared_ptr<VADeviceControllerCallback>& controllerCallback)
{
    CHECK_AND_RETURN_RET_LOG(controllerCallback != nullptr, ERR_INVALID_PARAM, "controllerCallback is nullptr");
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "get audio policy proxy failed");
    sptr<IRemoteObject> brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERR_OPERATION_FAILED, "get broker object failed");
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERR_OPERATION_FAILED, "get iva broker failed");

    auto controllerStubImpl = new (std::nothrow) VADeviceControllerStubImpl();
    CHECK_AND_RETURN_RET_LOG(controllerStubImpl != nullptr, ERR_OPERATION_FAILED, "new controller stub failed");

    controllerStubImpl->SetVADeviceControllerCallback (controllerCallback);
    sptr<IRemoteObject> controllerStubImplObject = controllerStubImpl->AsObject();
    if (controllerStubImplObject == nullptr) {
        AUDIO_ERR_LOG("Weird, controllerStubImplObject is nullptr");
        return ERROR;
    }
    return ivaBroker->OnDevicesConnected(device, controllerStubImplObject);
}
int32_t VADeviceBrokerWrapperImpl::OnDevicesDisconnected(const VADevice& device)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "get audio policy proxy failed");
    sptr<IRemoteObject>brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERR_OPERATION_FAILED, "get broker object failed");
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERR_OPERATION_FAILED, "get iva broker failed");

    return ivaBroker->OnDevicesDisconnected(device);
}

int32_t VADeviceBrokerWrapperImpl::SetAssociatedMacAddresses(const std::string &macAddress, bool isEnable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "get audio policy proxy failed");
    sptr<IRemoteObject> brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERR_OPERATION_FAILED, "get broker object failed");
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERR_OPERATION_FAILED, "get iva broker failed");

    ivaBroker->SetAssociatedMacAddresses(macAddress, isEnable);
    return SUCCESS;
}

int32_t VADeviceBrokerWrapperImpl::SetNoiseReductionMode(const VADevice &device, int32_t noiseReductionMode)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERR_OPERATION_FAILED, "get audio policy proxy failed");
    sptr<IRemoteObject> brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERR_OPERATION_FAILED, "get broker object failed");
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERR_OPERATION_FAILED, "get iva broker failed");
 
    return ivaBroker->SetNoiseReductionMode(device, noiseReductionMode);
}

const sptr<IAudioPolicy> VADeviceBrokerWrapperImpl::GetAudioPolicyProxyFromSamgr(bool block)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get system ability failed");
    sptr<IRemoteObject> object = nullptr;
    if (!block) {
        object = samgr->CheckSystemAbility(AUDIO_POLICY_SERVICE_ID);
    } else {
        object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    }
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio policy service failed");
    sptr<IAudioPolicy> apProxy= iface_cast<IAudioPolicy>(object);
    CHECK_AND_RETURN_RET_LOG(apProxy != nullptr, nullptr, "get audio policy proxy failed");
    return apProxy;
}
}  //namespace AudioStandard
}  //namespace OHOS