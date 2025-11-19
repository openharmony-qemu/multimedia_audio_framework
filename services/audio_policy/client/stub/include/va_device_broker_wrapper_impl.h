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
#ifndef VA_DEVICE_BROKER_WRAPPER_IMPL_H
#define VA_DEVICE_BROKER_WRAPPER_IMPL_H

#include "virtual_audio_interface.h"

#include "iaudio_policy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {

class VADeviceBrokerWrapperImpl : public VADeviceBrokerWrapper {
public:
    VADeviceBrokerWrapperImpl();
    virtual ~VADeviceBrokerWrapperImpl();
    int32_t OnDevicesConnected(
        const VADevice& device, const std::shared_ptr<VADeviceControllerCallback>& controllerCallback) override;
    int32_t OnDevicesDisconnected(const VADevice& device)override;

    static const sptr<IAudioPolicy> GetAudioPolicyProxyFromSamgr(bool block = true);
};
} // namespace AudioStandard
} // namespace OHOS
#endif  //VA_DEVICE_BROKER_WRAPPER_IMPL_H
