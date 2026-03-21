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

#include "audio_router_context_test_base.h"
#include "audio_device_manager.h"
#include "audio_policy_utils.h"
#include "audio_router_infra.h"

namespace OHOS {
namespace AudioStandard {

void AudioRouterContextTestBase::TearDown(void)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    for (auto &device : addDevices_) {
        deviceManager.RemoveNewDevice(device);
    }
    addDevices_.clear();

    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::MEDIA_INPUT);
    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::MEDIA_OUTPUT);
    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_INPUT);
    routerInfra.ClearAllSelectSelectDevice(SelectDeviceType::CALL_OUTPUT);
    for (int i = TEST_APP_UID_10001; i < TEST_APP_UID_MAX; i++) {
        routerInfra.OnAppDied(i);
    }
    routerInfra.SetRecognitionCaptureDevice(nullptr);
    routerInfra.UnexcludeAllDevice();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterContextTestBase::CreateDeviceDescriptor(
    DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(deviceType, deviceRole);
    desc->macAddress_ = macAddress;
    desc->networkId_ = macAddress;
    desc->deviceName_ = "TestDevice";
    desc->displayName_ = "TestDevice";
    desc->connectState_ = CONNECTED;
    desc->deviceId_ = AudioPolicyUtils::startDeviceId++;
    return desc;
}
std::shared_ptr<AudioDeviceDescriptor> AudioRouterContextTestBase::AddDeviceDescriptor(
    DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto device = CreateDeviceDescriptor(deviceType, deviceRole, macAddress);
    deviceManager.AddNewDevice(device);
    addDevices_.push_back(device);
    return device;
}

void AudioRouterContextTestBase::AddPairDeviceDescriptor(
    std::shared_ptr<AudioDeviceDescriptor> device1,
    std::shared_ptr<AudioDeviceDescriptor> device2)
{
    device1->pairDeviceDescriptor_ = device2;
    device2->pairDeviceDescriptor_ = device1;

    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    deviceManager.AddNewDevice(device1);
    deviceManager.AddNewDevice(device2);
    addDevices_.push_back(device1);
    addDevices_.push_back(device2);
}

void AudioRouterContextTestBase::SimulateBluetoothHeadsetConnect(const std::string &macAddress)
{
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, macAddress);
}

void AudioRouterContextTestBase::SimulateWiredHeadsetConnect(const std::string &macAddress)
{
    AddDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, OUTPUT_DEVICE, macAddress);
    CreateDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, INPUT_DEVICE, macAddress);
}

}
}
