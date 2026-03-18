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
