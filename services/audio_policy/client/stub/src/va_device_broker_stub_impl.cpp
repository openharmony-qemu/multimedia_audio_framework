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
#define LOG_TAG "VADeviceBrokerStubImpl"
#endif

#include "va_device_broker_stub_impl.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

#include "va_device_manager.h"
#include "audio_policy_server.h"

#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

sptr<VADeviceBrokerStubImpl> VADeviceBrokerStubImpl::Create()
{
    sptr<VADeviceBrokerStubImpl> vaDeviceBroker = sptr<VADeviceBrokerStubImpl>::MakeSptr();
    return vaDeviceBroker;
}

VADeviceBrokerStubImpl::VADeviceBrokerStubImpl()
{}

VADeviceBrokerStubImpl::~VADeviceBrokerStubImpl()
{}

int32_t VADeviceBrokerStubImpl::OnDevicesConnected(const VADevice &device, const sptr<IRemoteObject>& controller)
{
    CHECK_AND_RETURN_RET_LOG(
        PermissionUtil::VerifySystemPermission(), ERROR, "connect virtual audio denied: no system permission");
    sptr<IVADeviceController> vaDeviceController = iface_cast<IVADeviceController>(controller);
    CHECK_AND_RETURN_RET_LOG(vaDeviceController != nullptr, ERR_INVALID_PARAM, "controller is null");
    auto sharedDevice = std::make_shared<VADevice>(device);

    VADeviceManager::GetInstance().OnDevicesConnected(sharedDevice, vaDeviceController);

    return SUCCESS;
}

int32_t VADeviceBrokerStubImpl::OnDevicesDisconnected(const VADevice &device)
{
    auto sharedDevice = std::make_shared<VADevice>(device);
    VADeviceManager::GetInstance().OnDevicesDisconnected(sharedDevice);
    return SUCCESS;
}

}  //namespace AudioStandard
}  //namespace OHOS