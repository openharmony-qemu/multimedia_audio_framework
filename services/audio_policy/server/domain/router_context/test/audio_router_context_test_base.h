#ifndef AUDIO_ROUTER_CONTEXT_TEST_BASE_H
#define AUDIO_ROUTER_CONTEXT_TEST_BASE_H

#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterContextTestBase : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void);

    void SimulateBluetoothHeadsetConnect(const std::string &macAddress);
    void SimulateWiredHeadsetConnect(const std::string &macAddress);
    std::shared_ptr<AudioDeviceDescriptor> CreateDeviceDescriptor(
        DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress);
    std::shared_ptr<AudioDeviceDescriptor> AddDeviceDescriptor(
        DeviceType deviceType, DeviceRole deviceRole, const std::string &macAddress);

public:
    static constexpr int32_t TEST_APP_UID_10001 = 10001;
    static constexpr int32_t TEST_APP_UID_10002 = 10002;
    static constexpr int32_t TEST_APP_UID_10003 = 10003;
    static constexpr int32_t TEST_APP_UID_MAX = TEST_APP_UID_10003 + 1;
    static constexpr int32_t TEST_STREAM_INVALID_ID = 0;
    static constexpr int32_t TEST_STREAM_ID_1 = 1;
    static constexpr int32_t TEST_STREAM_ID_2 = 2;
    static constexpr int32_t TEST_STREAM_ID_3 = 3;

protected:
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> addDevices_;
};

} 
} 
#endif 
