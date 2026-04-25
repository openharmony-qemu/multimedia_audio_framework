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

#include "va_device_broker_wrapper_impl.h"

#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

VADeviceBrokerWrapperImpl::VADeviceBrokerWrapperImpl() = default;

VADeviceBrokerWrapperImpl::~VADeviceBrokerWrapperImpl() = default;

int32_t VADeviceBrokerWrapperImpl::OnDevicesConnected(
    const VADevice &, const std::shared_ptr<VADeviceControllerCallback> &)
{
    return SUCCESS;
}

int32_t VADeviceBrokerWrapperImpl::OnDevicesDisconnected(const VADevice &)
{
    return SUCCESS;
}

const sptr<IAudioPolicy> VADeviceBrokerWrapperImpl::GetAudioPolicyProxyFromSamgr(bool)
{
    return nullptr;
}

} // namespace AudioStandard
} // namespace OHOS
