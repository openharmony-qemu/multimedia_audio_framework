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

#include "audio_zone_unit_test_base.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioZoneDeviceUnitTest : public AudioZoneUnitTestBase {
};

/**
* @tc.name  : Test AudioZone.
* @tc.number: AudioZoneDevice_001
* @tc.desc  : Test bind device to audio zone.
*/
HWTEST_F(AudioZoneDeviceUnitTest, AudioZoneDevice_001, TestSize.Level1)
{
    auto device1 = CreateDevice(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "", "LocalDevice");
    auto device2 = CreateDevice(DEVICE_TYPE_MIC, INPUT_DEVICE, "", "LocalDevice");
    std::vector<sptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    devices.push_back(device2);
    EXPECT_EQ(AudioZoneService::GetInstance().BindDevicesToAudioZone(zoneId1_, devices), 0);

    auto fechOutputDevice = AudioZoneService::GetInstance().FetchOutputDevices(zoneId1_,
        STREAM_USAGE_MUSIC, 0, ROUTER_TYPE_DEFAULT);
    auto fechInputDevice = AudioZoneService::GetInstance().FetchInputDevices(zoneId1_,
        SOURCE_TYPE_MIC, 0);
    EXPECT_EQ(fechOutputDevice.size(), 1);
    EXPECT_EQ(fechOutputDevice[0]->IsSameDeviceDesc(device1), true);
    EXPECT_NE(fechInputDevice, nullptr);
    EXPECT_EQ(fechInputDevice->IsSameDeviceDesc(device2), true);
}
} // namespace AudioStandard
} // namespace OHOS