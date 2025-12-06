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
    CHECK_AND_RETURN_RET_LOG(controllerCallback != nullptr, ERROR);
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR);
    sptr<IRemoteObject> brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERROR);
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERROR);

    auto controllerStubImpl = new (std::nothrow) VADeviceControllerStubImpl();
    CHECK_AND_RETURN_RET_LOG(controllerStubImpl != nullptr, ERROR);

    controllerStubImpl->SetVADeviceControllerCallback (controllerCallback);
    sptr<IRemoteObject> controllerStubImplObject = controllerStubImpl->AsObject();
    if (controllerStubImplObject == nullptr) {
        AUDIO_ERR_LOG("Weird, controllerStubImplObject is nullptr");
        return ERROR;
    }
    ivaBroker->OnDevicesConnected(device, controllerStubImplObject);
    return SUCCESS;
}
int32_t VADeviceBrokerWrapperImpl::OnDevicesDisconnected(const VADevice& device)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyProxyFromSamgr();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, ERROR);
    sptr<IRemoteObject>brokerObject;
    gsp->GetVADeviceBroker(brokerObject);
    CHECK_AND_RETURN_RET_LOG(brokerObject != nullptr, ERROR);
    sptr<IVADeviceBroker> ivaBroker = iface_cast<IVADeviceBroker>(brokerObject);
    CHECK_AND_RETURN_RET_LOG(ivaBroker != nullptr, ERROR);

    ivaBroker->OnDeviceDisconnected(device);
    return SUCCESS;
}

const sptr<IAudioPolicy> VADeviceBrokerWrapperImpl::GetAudioPolicyProxyFromSamgr(bool block)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr);
    sptr<IRemoteObject> object = nullptr;
    if (!block) {
        object = samgr->CheckSystemAbility(AUDIO_POLICY_SERVICE_ID);
    } else {
        object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    }
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr);
    sptr<IAudioPolicy> apProxy= iface_cast<IAudioPolicy>(object);
    CHECK_AND_RETURN_RET_LOG(apProxy != nullptr, nullptr);
    return apProxy;
}
}  //namespace AudioStandard
}  //namespace OHOS