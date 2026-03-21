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
    void AddPairDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> device1,
        std::shared_ptr<AudioDeviceDescriptor> device2);

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
