/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_DEVICE_INIT_CALLBACK_H
#define ST_DEVICE_INIT_CALLBACK_H

#include "audio_policy_service.h"
#ifdef FEATURE_DEVICE_MANAGER
#include "device_manager_callback.h"
#include "device_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
#ifdef FEATURE_DEVICE_MANAGER
class DeviceInitCallBack : public DistributedHardware::DmInitCallback {
public:
    explicit DeviceInitCallBack() = default;
    ~DeviceInitCallBack() override {};
    void OnRemoteDied() override {};
};
#endif

#ifdef FEATURE_DEVICE_MANAGER
class DeviceStatusCallbackImpl : public DistributedHardware::DeviceStatusCallback,
    public DistributedHardware::DeviceStateCallback {
public:
    explicit DeviceStatusCallbackImpl();
    ~DeviceStatusCallbackImpl() override {};
    void OnDeviceChanged(const DistributedHardware::DmDeviceBasicInfo &dmDeviceBasicInfo) override;
    void OnDeviceOnline(const DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {};
    void OnDeviceOffline(const DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {};
    void OnDeviceReady(const DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override {};
    void OnDeviceChanged(const DistributedHardware::DmDeviceInfo &dmDeviceInfo) override;
    void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &dmDeviceInfo) override;
    void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &dmDeviceInfo) override;
    void OnDeviceReady(const DistributedHardware::DmDeviceInfo &dmDeviceInfo) override {};

private:
    AudioPolicyService& audioPolicyService_;
};
#endif

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DEVICE_INIT_CALLBACK_H
