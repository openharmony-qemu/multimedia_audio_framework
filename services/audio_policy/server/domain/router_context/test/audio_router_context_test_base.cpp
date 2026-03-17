#include "audio_router_context_test_util.h"
#include "audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

void TearDown(void)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    for (auto &device : addDevices_) {
        deviceManager->RemoveNewDevice(device);
    }
    addDevices_.clear();
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterContextTestUtil::CreateDeviceDescriptor(
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
std::shared_ptr<AudioDeviceDescriptor> AudioRouterContextTestUtil::AddDeviceDescriptor(
    DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto device = CreateDeviceDescriptor(deviceType, deviceRole, macAddress);
    ASSERT_NE(device, nullptr);
    deviceManager.AddNewDevice(device);
    addDevices_.push_back(device);
    return device;
}

void AudioRouterContextTestUtil::SimulateBluetoothHeadsetConnect(const std::string &macAddress)
{
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, macAddress);
    AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, macAddress);
}

void AudioRouterContextTestUtil::SimulateWiredHeadsetConnect(const std::string &macAddress)
{  
    AddDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, OUTPUT_DEVICE, macAddress);
    CreateDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, INPUT_DEVICE, macAddress);
}

} 
} 
