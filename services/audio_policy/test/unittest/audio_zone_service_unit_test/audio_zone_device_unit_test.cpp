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

#include <condition_variable>
#include <mutex>
#include "gtest/gtest.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_zone_service.h"
#include "audio_interrupt_service.h"
#include "i_audio_zone_event_dispatcher.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioZoneDeviceUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        AudioZoneService::GetInstance().Init(handler, interruptService);
        AudioZoneContext context;
        zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
        zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
    }

    void TearDown() override
    {
        zoneId1_ = 0;
        zoneId2_ = 0;
        AudioZoneService::GetInstance().DeInit();
    }

    int32_t zoneId1_ = 0;
    int32_t zoneId2_ = 0;
};

static std::shared_ptr<AudioDeviceDescriptor> CreateDevice(DeviceType type, DeviceRole role,
    const std::string &macAddress, const std::string &networkId)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(type, role);
    EXPECT_NE(desc, nullptr);
    desc->macAddress_ = macAddress;
    desc->networkId_ = networkId;
    return desc;
}

/**
* @tc.name  : Test AudioZone.
* @tc.number: AudioZoneDevice_001
* @tc.desc  : Test bind device to audio zone.
*/
HWTEST_F(AudioZoneDeviceUnitTest, AudioZoneDevice_001, TestSize.Level1)
{
    auto device1 = CreateDevice(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "", "LocalDevice");
    auto device2 = CreateDevice(DEVICE_TYPE_MIC, INPUT_DEVICE, "", "LocalDevice");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    devices.push_back(device2);
    EXPECT_EQ(AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices), 0);

    AudioConnectedDevice::GetInstance().AddConnectedDevice(device1);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device2);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(device1);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(device2);
    auto fechOutputDevice = AudioZoneService::GetInstance().FetchOutputDevices(zoneId1_,
        STREAM_USAGE_MUSIC, 0, ROUTER_TYPE_DEFAULT);
    auto fechInputDevice = AudioZoneService::GetInstance().FetchInputDevice(zoneId1_,
        SOURCE_TYPE_MIC, 0);
    EXPECT_EQ(fechOutputDevice.size(), 1);
    EXPECT_EQ(fechOutputDevice[0]->IsSameDeviceDesc(*device1), true);
    EXPECT_NE(fechInputDevice, nullptr);
    EXPECT_EQ(fechInputDevice->IsSameDeviceDesc(*device2), true);
}
} // namespace AudioStandard
} // namespace OHOS