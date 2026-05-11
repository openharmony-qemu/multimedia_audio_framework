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
    zone->BindByKey(AudioZoneBindKey(2, "d1"));
    zone->BindByKey(AudioZoneBindKey(2, ""));
    zone->BindByKey(AudioZoneBindKey(2, "d2"));
    zone->RemoveKey(AudioZoneBindKey(2, "d2"));
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "d2")), false);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "")), true);
    EXPECT_EQ(zone->IsContainKey(AudioZoneBindKey(2, "d1")), false);
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

    auto outputDevices = zone->GetAllOutputDevices();
    EXPECT_EQ(outputDevices.size(), 1);
    EXPECT_EQ(outputDevices[0]->IsSameDeviceDesc(device1), true);
    auto outputDeviceType = zone->GetAllOutputDeviceTypes();
    EXPECT_EQ(outputDeviceType.size(), 1);
    EXPECT_EQ(outputDeviceType.count(DEVICE_TYPE_SPEAKER), 1);
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
 * @tc.number: AudioZone_DegreeTest_001
 * @tc.desc  : Test SetSystemVolumeDegree interface.
 */
HWTEST_F(AudioZoneUnitTest, AudioZone_DegreeTest_001, TestSize.Level1)
{
    ClearZone();
    AudioZoneContext context;
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeDegree = 10;
    VolumeScale volume{volumeDegree, volumeDegree};
    auto zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    auto zone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(zone, nullptr);
    EXPECT_NE(zone->GetSystemVolumeDegree(DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(zone->SetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType, volume, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType), 0);

    int32_t clientPid = 1;
    zone->EnableSystemVolumeProxy(clientPid, DEVICE_TYPE_SPEAKER, true);
    EXPECT_NE(zone->GetSystemVolumeDegree(DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(zone->SetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType, volume, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType), volumeDegree);
    zone->EnableSystemVolumeProxy(clientPid, DEVICE_TYPE_SPEAKER, false);

    zone->EnableSystemVolumeProxy(clientPid, DEVICE_TYPE_NONE, true);
    EXPECT_NE(zone->GetSystemVolumeDegree(DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(zone->SetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType, volume, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType), volumeDegree);
    zone->EnableSystemVolumeProxy(clientPid, DEVICE_TYPE_NONE, false);

    zone->clientManager_.reset();
    EXPECT_NE(zone->SetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType, volume, 0), 0);
    EXPECT_NE(zone->GetSystemVolumeLevel(DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(zone->GetSystemVolumeDegree(DEVICE_TYPE_SPEAKER, volumeType), 0);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_001
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_001, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = desc->networkId_;
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_002
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_002, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = "networkId";
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_003
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_003, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));
    AudioDeviceDescriptor device;
    device.deviceType_ = DEVICE_TYPE_SPEAKER;
    device.networkId_ = desc->networkId_;
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_004
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_004, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = desc->networkId_;
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_005
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_005, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));
    AudioDeviceDescriptor device;
    device.deviceType_ = DEVICE_TYPE_SPEAKER;
    device.networkId_ = desc->networkId_;
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_006
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_006, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));
    AudioDeviceDescriptor device;
    device.deviceType_ = DEVICE_TYPE_SPEAKER;
    device.networkId_ = "networkId";
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_007
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_007, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = "networkId";
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckDeviceInZone.
 * @tc.number: CheckDeviceInZone_008
 * @tc.desc  : Test CheckDeviceInZone interface.
 */
HWTEST_F(AudioZoneUnitTest, CheckDeviceInZone_008, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));
    AudioDeviceDescriptor device;
    device.deviceType_ = DEVICE_TYPE_SPEAKER;
    device.networkId_ = "networkId";
    auto ret = zone->CheckDeviceInZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_001
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_001, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));

    DeviceType type = desc->deviceType_;
    std::string networkId = desc->networkId_;
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_002
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_002, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));

    DeviceType type = desc->deviceType_;
    std::string networkId = desc->networkId_;
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_003
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_003, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));

    DeviceType type = DEVICE_TYPE_SPEAKER;
    std::string networkId = desc->networkId_;
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_004
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_004, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));

    DeviceType type = desc->deviceType_;
    std::string networkId = "networkId";
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_005
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_005, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));

    DeviceType type = DEVICE_TYPE_SPEAKER;
    std::string networkId = desc->networkId_;
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_006
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_006, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));

    DeviceType type = DEVICE_TYPE_SPEAKER;
    std::string networkId = "networkId";
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_007
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_007, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));

    DeviceType type = desc->deviceType_;
    std::string networkId = "networkId";
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDeviceDescriptor.
 * @tc.number: GetDeviceDescriptor_008
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneUnitTest, GetDeviceDescriptor_008, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));

    DeviceType type = DEVICE_TYPE_SPEAKER;
    std::string networkId = "networkId";
    auto ret = zone->GetDeviceDescriptor(type, networkId);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test GetDescriptor.
 * @tc.number: GetDescriptorNoLock_001
 * @tc.desc  : Test GetDescriptor with valid streamId insertion
 */
HWTEST_F(AudioZoneUnitTest, GetDescriptorNoLock_001, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    uint32_t streamId1 = 100;
    uint32_t streamId2 = 200;
    int32_t uid = 1;
    zone->BindByKey(AudioZoneBindKey(streamId1));
    zone->BindByKey(AudioZoneBindKey(uid));
    zone->BindByKey(AudioZoneBindKey(streamId2));

    auto descriptor = zone->GetDescriptor();
    EXPECT_NE(descriptor, nullptr);
    EXPECT_EQ(descriptor->streamIds_.size(), 2);
    EXPECT_EQ(descriptor->streamIds_.count(streamId1), 1);
    EXPECT_EQ(descriptor->streamIds_.count(streamId2), 1);
}

/**
 * @tc.name  : Test GetDescriptor.
 * @tc.number: GetDescriptorNoLock_002
 * @tc.desc  : Test GetDescriptor with INVALID_STREAM_ID skip
 */
HWTEST_F(AudioZoneUnitTest, GetDescriptorNoLock_002, TestSize.Level1)
{
    ClearZone();
    auto zone = CreateZone("TestZone");
    zone->BindByKey(AudioZoneBindKey(1));
    zone->BindByKey(AudioZoneBindKey(2, STREAM_USAGE_MUSIC));

    auto descriptor = zone->GetDescriptor();
    EXPECT_NE(descriptor, nullptr);
    EXPECT_EQ(descriptor->streamIds_.size(), 0);
}

/**
 * @tc.name  : Test GetSupportKeys.
 * @tc.number: GetSupportKeys_001
 * @tc.desc  : Test GetSupportKeys with valid streamId
 */
HWTEST_F(AudioZoneUnitTest, GetSupportKeys_001, TestSize.Level1)
{
    int32_t uid = 1000;
    std::string deviceTag = "test_device";
    uint32_t streamId = 100;
    StreamUsage usage = STREAM_USAGE_MUSIC;

    auto keys = AudioZoneBindKey::GetSupportKeys(uid, deviceTag, streamId, usage);
    EXPECT_FALSE(keys.empty());

    AudioZoneBindKey targetKey(streamId);
    bool found = false;
    for (const auto &key : keys) {
        if (key == targetKey) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name  : Test GetSupportKeys.
 * @tc.number: GetSupportKeys_002
 * @tc.desc  : Test GetSupportKeys with INVALID_STREAM_ID
 */
HWTEST_F(AudioZoneUnitTest, GetSupportKeys_002, TestSize.Level1)
{
    int32_t uid = 1000;
    std::string deviceTag = "test_device";
    uint32_t streamId = INVALID_STREAM_ID;
    StreamUsage usage = STREAM_USAGE_MUSIC;

    auto keys = AudioZoneBindKey::GetSupportKeys(uid, deviceTag, streamId, usage);
    EXPECT_FALSE(keys.empty());

    AudioZoneBindKey targetKey(INVALID_STREAM_ID);
    bool found = false;
    for (const auto &key : keys) {
        if (key == targetKey) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
}
} // namespace AudioStandard
} // namespace OHOS