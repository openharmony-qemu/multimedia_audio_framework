#include "audio_router_context_test_util.h"
#include "audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioDeviceDescriptor> AudioRouterContextTestUtil::CreateDeviceDescriptor(
    DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(deviceType, deviceRole);
    desc->macAddress_ = macAddress;
    desc->networkId_ = macAddress;
    desc->deviceName_ = "TestDevice";
    desc->displayName_ = "TestDevice";
    desc->connectState_ = CONNECTED;
    return desc;
}

void AudioRouterContextTestUtil::SimulateBluetoothHeadsetConnect(const std::string &macAddress)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    auto scoOutput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(scoOutput);
    
    auto a2dpOutput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(a2dpOutput);
    
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(scoInput);
    
    auto a2dpInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(a2dpInput);
}

void AudioRouterContextTestUtil::SimulateWiredHeadsetConnect(const std::string &macAddress)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    auto output = CreateDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, OUTPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(output);
    
    auto input = CreateDeviceDescriptor(DEVICE_TYPE_WIRED_HEADSET, INPUT_DEVICE, macAddress);
    deviceManager.AddNewDevice(input);
}

} 
} 
