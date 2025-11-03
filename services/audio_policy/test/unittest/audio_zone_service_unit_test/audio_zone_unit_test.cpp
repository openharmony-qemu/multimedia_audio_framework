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

class AudioZoneUnitTest : public AudioZoneUnitTestPreset {
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

static void ClearZone()
{
    auto zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    for (auto zone : zoneList) {
        AudioZoneService::GetInstance().ReleaseAudioZone(zone->zoneId_);
    }
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_001
 * @tc.desc  : Test create audio zone.
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_001, TestSize.Level1)
{
    ClearZone();
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
    ClearZone();
    auto zone = CreateZone("TestZone");
    zone->BindByKey(AudioZoneBindKey(1, "d1"));
    zone->BindByKey(AudioZoneBindKey(1));
    zone->BindByKey(AudioZoneBindKey(2, "d1", "test"));
    zone->BindByKey(AudioZoneBindKey(2, "", "test"));
    zone->BindByKey(AudioZoneBindKey(2, "", "temp"));
    zone->RemoveKey(AudioZoneBindKey(2, "", "temp"));
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "", "temp")), false);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "", "test")), true);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "", "test")), false);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(1)), true);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(1, "")), true);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_003
 * @tc.desc  : Test bind key to audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_003, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto device1 = CreateDevice(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "", "LocalDevice");
    auto device2 = CreateDevice(DEVICE_TYPE_MIC, INPUT_DEVICE, "", "LocalDevice");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    devices.push_back(device2);
    EXPECT_EQ(zone->AddDeviceDescriptor(devices), 0);
    EXPECT_EQ(zone->IsDeviceConnect(device1), true);
    EXPECT_EQ(zone->IsDeviceConnect(device2), true);

    EXPECT_EQ(zone->DisableDeviceDescriptor(device2), 0);
    EXPECT_EQ(zone->IsDeviceConnect(device2), false);
    EXPECT_EQ(zone->EnableDeviceDescriptor(device2), 0);
    EXPECT_EQ(zone->IsDeviceConnect(device2), true);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> tempDevices;
    tempDevices.push_back(device2);
    EXPECT_EQ(zone->RemoveDeviceDescriptor(tempDevices), 0);
    EXPECT_EQ(zone->IsDeviceConnect(device2), false);

    EXPECT_EQ(zone->AddDeviceDescriptor(devices), 0);
    auto fechOutputDevice = zone->FetchOutputDevices(STREAM_USAGE_MUSIC, 0, ROUTER_TYPE_DEFAULT);
    auto fechInputDevice = zone->FetchInputDevice(SOURCE_TYPE_MIC, 0);
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
    ClearZone();
    AudioZoneContext context;
    auto zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    EXPECT_NE(zoneId1, 0);
    auto zoneId2 = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
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
 * @tc.name  : Test EnableChangeReport.
 * @tc.number: EnableChangeReport_001
 * @tc.desc  : Test EnableChangeReport interface.
 */
HWTEST_F(AudioZoneUnitTest, EnableChangeReport_001, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    pid_t clientPid = 1;
    bool enable = true;
    EXPECT_EQ(zone->EnableChangeReport(clientPid, enable), 0);
}

/**
 * @tc.name  : Test EnableChangeReport.
 * @tc.number: EnableChangeReport_002
 * @tc.desc  : Test EnableChangeReport interface.
 */
HWTEST_F(AudioZoneUnitTest, EnableChangeReport_002, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    pid_t clientPid = 1;
    bool enable = false;
    EXPECT_EQ(zone->EnableChangeReport(clientPid, enable), 0);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_005
 * @tc.desc  : Test release audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_005, TestSize.Level1)
{
    ClearZone();
    AudioZoneContext context;
    auto zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, 20);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    auto ret = AudioZoneService::GetInstance().GetAudioZone(zoneId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_006
 * @tc.desc  : Test release audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_006, TestSize.Level1)
{
    ClearZone();
    int32_t fakeUid = 1234;
    int32_t fakePid = 4321;
    auto ret = AudioZoneService::GetInstance().FindAudioSessionZoneid(fakeUid, fakePid, false);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_007
 * @tc.desc  : Test release audio zone
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_007, TestSize.Level1)
{
    ClearZone();
    int32_t fakeUid = 1234;
    int32_t fakePid = 4321;
    AudioZoneContext context;
    auto zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, 20);
    AudioSessionService &audioSessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    int ret = audioSessionService.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioZoneService::GetInstance().FindAudioSessionZoneid(fakeUid, fakePid, false);
    EXPECT_EQ(ret, 0);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    audioSessionService.sessionMap_.clear();
}

/**
 * @tc.name  : Test AudioZone.
 * @tc.number: AudioZone_DegreeTest_001
 * @tc.desc  : Test SetSystemVolumeDegree interface.
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_DegreeTest_001, TestSize.Level1)
{
    ClearZone();
    AudioZoneContext context;
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeDegree = 10;
    auto zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    auto zone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(zone, nullptr);
    EXPECT_NE(zone->GetSystemVolumeDegree(volumeType), 0);
    EXPECT_NE(zone->SetSystemVolumeDegree(volumeType, volumeDegree, 0), 0);
    EXPECT_NE(zone->SetSystemVolumeDegree(volumeType, volumeDegree, 0), 0);

    int32_t clientPid = 1;
    zone->EnableSystemVolumeProxy(clientPid, true);
    EXPECT_NE(zone->SetSystemVolumeDegree(volumeType, volumeDegree, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeDegree(volumeType), volumeDegree);

    zone->clientManager_.reset();
    EXPECT_NE(zone->SetSystemVolumeDegree(volumeType, volumeDegree, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeDegree(volumeType), 0);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

} // namespace AudioStandard
} // namespace OHOS