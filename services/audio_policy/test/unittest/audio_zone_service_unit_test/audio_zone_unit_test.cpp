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

class AudioZoneUnitTest : public AudioZoneUnitTestBase {
};

static std::shared_ptr<AudioZone> CreateZone(const std::string &name)
{
    std::shared_ptr<AudioZoneClientManager> manager = nullptr;
    AudioZoneContext context;
    manager = std::make_shared<AudioZoneClientManager>(nullptr);
    EXPECT_NE(manager, nullptr);
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(
        manager, name, context);
    EXPECT_NE(zone, nullptr);
    return zone;
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_001
 * @tc.desc  : Test create audio zone.
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_001, TestSize.Level1)
{
    auto zone = CreateZone("TestZone");
    auto desc = zone->GetDescriptor();
    EXPECT_NE(desc, nullptr);
    EXPECT_EQ(desc->zoneId_, zone->GetId());
    EXPECT_EQ(desc->name_, "TestZone");
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_002
 * @tc.desc  : Test bind key to audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_002, TestSize.Level1)
{
    auto zone = CreateZone("TestZone");
    zone->BindByKey(AudioZoneBindKey(1, 1));
    zone->BindByKey(AudioZoneBindKey(1));
    zone->BindByKey(AudioZoneBindKey(2, 1, "test"));
    zone->BindByKey(AudioZoneBindKey(2, -1, "test"));
    zone->BindByKey(AudioZoneBindKey(2, -1, "temp"));
    zone->RemoveKey(AudioZoneBindKey(2, -1, "temp"));
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, -1, "temp")), false);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, -1, "test")), true);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, 1, "test")), false);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(1)), true);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(1, -1)), false);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_003
 * @tc.desc  : Test bind key to audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_003, TestSize.Level1)
{
    auto zone = CreateZone("TestZone");
    auto device1 = CreateDevice(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "", "LocalDevice");
    auto device2 = CreateDevice(DEVICE_TYPE_MIC, INPUT_DEVICE, "", "LocalDevice");
    std::vector<sptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    devices.push_back(device2);
    EXPECT_EQ(zone->AddDeviceDescriptors(devices), 0);
    EXPECT_EQ(zone->IsDeviceConnected(device1), true);
    EXPECT_EQ(zone->IsDeviceConnected(device2), true);

    EXPECT_EQ(zone->DisableDeviceDescriptor(device2), 0);
    EXPECT_EQ(zone->IsDeviceConnected(device2), false);
    EXPECT_EQ(zone->EnableDeviceDescriptor(device2), 0);
    EXPECT_EQ(zone->IsDeviceConnected(device2), true);
    EXPECT_EQ(zone->RemoveDeviceDescriptor(device2), 0);
    EXPECT_EQ(zone->IsDeviceConnected(device2), false);

    EXPECT_EQ(zone->AddDeviceDescriptors(devices), 0);
    auto fechOutputDevice = zone->FetchOutputDevices(STREAM_USAGE_MUSIC, 0, ROUTER_TYPE_DEFAULT);
    auto fechInputDevice = zone->FetchInputDevices(SOURCE_TYPE_MIC, 0);
    EXPECT_EQ(fechOutputDevice.size(), 1);
    EXPECT_EQ(fechOutputDevice[0]->IsSameDeviceDesc(device1), true);
    EXPECT_NE(fechInputDevice, nullptr);
    EXPECT_EQ(fechInputDevice->IsSameDeviceDesc(device2), true);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_004
 * @tc.desc  : Test release audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_004, TestSize.Level1)
{
    AudioZoneContext context;
    auto zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
    EXPECT_NE(zoneId1, 0);
    auto zoneId2 = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context);
    EXPECT_NE(zoneId2, 0);

    auto zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    EXPECT_EQ(zoneList.size(), 2);
    EXPECT_NE(AudioZoneService::GetInstance().GetAudioZone(zoneId1), nullptr);
    EXPECT_NE(AudioZoneService::GetInstance().GetAudioZone(zoneId2), nullptr);
    EXPECT_EQ(AudioZoneService::GetInstance().GetAudioZone(zoneId1 + zoneId2), nullptr);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
    zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    EXPECT_EQ(zoneList.size(), 1);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1 + zoneId2);
    zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    EXPECT_EQ(zoneList.size(), 1);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId2);
    zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    EXPECT_EQ(zoneList.size(), 0);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_005
 * @tc.desc  : Test audio zone report
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_005, TestSize.Level1)
{
    AudioZoneContext context;
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneReport(TEST_PID_1000, true), 0);
    auto client = RegisterTestClient(TEST_PID_1000);
    EXPECT_NE(client, nullptr);
    auto zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
    EXPECT_NE(zoneId1, 0);
    client->Wait();
    EXPECT_EQ(client->recvEvent_.type_, AUDIO_ZONE_ADD_EVENT);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
    client->Wait();
    EXPECT_EQ(client->recvEvent_.type_, AUDIO_ZONE_REMOVE_EVENT);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_006
 * @tc.desc  : Test audio zone change report
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_006, TestSize.Level1)
{
    AudioZoneContext context;
    EXPECT_NE(AudioZoneService::GetInstance().EnableAudioZoneReport(TEST_PID_1000, 1, true), 0);
    auto client = RegisterTestClient(TEST_PID_1000);
    EXPECT_NE(client, nullptr);
    auto zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
    EXPECT_NE(zoneId1, 0);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneChangeReport(TEST_PID_1000, zoneId1, true), 0);

    EXPECT_EQ(AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, TEST_PID_1000), 0);
    client->Wait();
    EXPECT_NE(client->recvEvent_.type_, AUDIO_ZONE_CHANGE_EVENT);
}
} // namespace AudioStandard
} // namespace OHOS