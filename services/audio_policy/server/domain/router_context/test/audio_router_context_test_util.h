#ifndef AUDIO_ROUTER_CONTEXT_TEST_UTIL_H
#define AUDIO_ROUTER_CONTEXT_TEST_UTIL_H

#include <memory>
#include <string>
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterContextTestUtil {
public:
    static void SimulateBluetoothHeadsetConnect(const std::string &macAddress);
    static void SimulateWiredHeadsetConnect(const std::string &macAddress);
    static std::shared_ptr<AudioDeviceDescriptor> CreateDeviceDescriptor(
        DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress);
};

} 
} 
#endif 
