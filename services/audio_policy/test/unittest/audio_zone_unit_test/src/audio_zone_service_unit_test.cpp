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

#include "audio_zone_service_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_zone.h"
#include "audio_connected_device.h"
#include "audio_device_info.h"
#include "audio_common_utils.h"
#include "audio_zone_client_manager_unit_test.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneServiceUnitTest::SetUpTestCase(void) {}
void AudioZoneServiceUnitTest::TearDownTestCase(void) {}
void AudioZoneServiceUnitTest::SetUp(void)
{
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
}
void AudioZoneServiceUnitTest::TearDown(void) {}

static void ClearZone()
{
    auto zoneList = AudioZoneService::GetInstance().GetAllAudioZone();
    for (auto &zone : zoneList) {
        AudioZoneService::GetInstance().ReleaseAudioZone(zone->zoneId_);
    }
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_001
 * @tc.desc  : Test EnableAudioZoneReport interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_001, TestSize.Level1)
{
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneReport(0, true), SUCCESS);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableAudioZoneReport(0, false), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_002
 * @tc.desc  : Test CheckIsZoneValid interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_002, TestSize.Level1)
{
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(-1), false);
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(1), false);
    EXPECT_EQ(AudioZoneService::GetInstance().CheckIsZoneValid(1), false);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_003
 * @tc.desc  : Test InjectInterruptToAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_003, TestSize.Level1)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;
    EXPECT_NE(AudioZoneService::GetInstance().InjectInterruptToAudioZone(0, "", interrupts), 0);
    EXPECT_NE(AudioZoneService::GetInstance().InjectInterruptToAudioZone(0, "0", interrupts), 0);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: CheckDeviceInAudioZone_001
 * @tc.desc  : Test CheckDeviceInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, CheckDeviceInAudioZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone", context, 0);

    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc.networkId_ = "NOTLOCALDEVICE";
    std::shared_ptr<AudioDeviceDescriptor> deviceDescPtr = make_shared<AudioDeviceDescriptor>(deviceDesc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescList;
    deviceDescList.emplace_back(deviceDescPtr);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    audioZone->AddDeviceDescriptor(deviceDescList);
    audioZone->SetDeviceDescriptorState(deviceDescPtr, true);

    EXPECT_EQ(AudioZoneService::GetInstance().CheckDeviceInAudioZone(deviceDesc), true);

    audioZone->RemoveDeviceDescriptor(deviceDescList);
    EXPECT_EQ(AudioZoneService::GetInstance().CheckDeviceInAudioZone(deviceDesc), false);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_DegreeTest_001
 * @tc.desc  : Test SetSystemVolumeDegree interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_DegreeTest_001, TestSize.Level1)
{
    AudioZoneContext context;
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeDegree = 10;
    // VolumeScale(level, degree)
    VolumeScale volume{volumeDegree, volumeDegree};
    int32_t zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);

    // calls must match signature: (zoneId, deviceType, volumeType, volume, flag)
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeLevel(0, DEVICE_TYPE_SPEAKER, volumeType, volume, 0), 0);
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId1, DEVICE_TYPE_SPEAKER, volumeType,
        volume, 0), 0);

    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(0, DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(zoneId1, DEVICE_TYPE_SPEAKER, volumeType), 0);

    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeLevel(0, DEVICE_TYPE_SPEAKER, volumeType), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeLevel(zoneId1, DEVICE_TYPE_SPEAKER, volumeType), 0);

    auto zone = AudioZoneService::GetInstance().FindZone(zoneId1);
    ASSERT_NE(zone, nullptr);
    int32_t clientPid = 1;
    // enable device proxy on zone object (unit test simulates client registered)
    zone->EnableSystemVolumeProxy(clientPid, DEVICE_TYPE_SPEAKER, true);

    // now SetSystemVolumeLevel should be accepted when proxy enabled (still uses deviceType param)
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId1, DEVICE_TYPE_SPEAKER, volumeType,
        volume, 0), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(zoneId1, DEVICE_TYPE_SPEAKER, volumeType),
        volumeDegree);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: HasVolumeControllableDeviceInAudioZone_001
 * @tc.desc  : Test HasVolumeControllableDeviceInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, HasVolumeControllableDeviceInZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    EXPECT_EQ(AudioZoneService::GetInstance().HasVolumeControllableDeviceInAudioZone(), false);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE
    );
    desc->volumeBehavior_.isVolumeControlDisabled = false;
    devices.push_back(desc);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(desc);
    audioZone->AddDeviceDescriptor(devices);
    EXPECT_EQ(AudioZoneService::GetInstance().HasVolumeControllableDeviceInAudioZone(), true);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}


/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: CheckExistUidInAudioZone_001
 * @tc.desc  : Test CheckExistUidInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AddUidUsagesToAudioZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone3", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(audioZone, nullptr);

    const std::set<StreamUsage> usages = {
        STREAM_USAGE_MEDIA,
        STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_ALARM,
    };

    EXPECT_EQ(AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId, 1000, usages), SUCCESS);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_MEDIA)), true);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_VOICE_COMMUNICATION)), true);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_ALARM)), true);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);

    // add invalid zone
    EXPECT_NE(AudioZoneService::GetInstance().AddUidUsagesToAudioZone(-1, 1000, usages), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: RemoveUidUsagesFromAudioZone_001
 * @tc.desc  : Test RemoveUidUsagesFromAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, RemoveUidUsagesFromAudioZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone4", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(audioZone, nullptr);
    const std::set<StreamUsage> usages = {
        STREAM_USAGE_MEDIA,
        STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_ALARM,
    };

    EXPECT_EQ(AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId, 1000, usages), SUCCESS);
    EXPECT_EQ(AudioZoneService::GetInstance().RemoveUidUsagesFromAudioZone(zoneId, 1000, usages), SUCCESS);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_MEDIA)), false);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_VOICE_COMMUNICATION)), false);
    EXPECT_EQ(audioZone->IsContainKey(AudioZoneBindKey(1000, "", "", STREAM_USAGE_ALARM)), false);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    // remove invalid zone
    EXPECT_NE(AudioZoneService::GetInstance().RemoveUidUsagesFromAudioZone(-1, 1000, usages), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: FindAudioZone
 * @tc.desc  : Test FindAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, FindAudioZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone5", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(audioZone, nullptr);

    const std::set<StreamUsage> usages = {
        STREAM_USAGE_MEDIA,
        STREAM_USAGE_RINGTONE,
        STREAM_USAGE_INVALID,
    };

    AudioZoneService::GetInstance().AddUidUsagesToAudioZone(zoneId, 1234, usages);
    EXPECT_EQ(AudioZoneService::GetInstance().FindAudioZone(1234, STREAM_USAGE_MEDIA), zoneId); // uid and usage match
    EXPECT_EQ(AudioZoneService::GetInstance().FindAudioZone(1234, STREAM_USAGE_DTMF), zoneId); // match uid only
    EXPECT_EQ(AudioZoneService::GetInstance().FindAudioZone(12345, STREAM_USAGE_RINGTONE), 0); // not found
    EXPECT_EQ(AudioZoneService::GetInstance().FindAudioZone(12345, STREAM_USAGE_ALARM), 0); // not found
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: GetDeviceDescriptor
 * @tc.desc  : Test GetDeviceDescriptor interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, GetDeviceDescriptor, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone5", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    ASSERT_NE(audioZone, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->networkId_ = "Test";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId, devices);
    EXPECT_EQ(AudioZoneService::GetInstance().GetDeviceDescriptor(DEVICE_TYPE_NONE, ""), nullptr);
    EXPECT_EQ(AudioZoneService::GetInstance().GetDeviceDescriptor(DEVICE_TYPE_SPEAKER, ""), nullptr);
    EXPECT_EQ(AudioZoneService::GetInstance().GetDeviceDescriptor(DEVICE_TYPE_NONE, "Test"), nullptr);

    EXPECT_NE(AudioZoneService::GetInstance().GetDeviceDescriptor(DEFAULT_ZONEID, DEVICE_TYPE_SPEAKER), nullptr);
    EXPECT_EQ(AudioZoneService::GetInstance().GetDeviceDescriptor(zoneId, DEVICE_TYPE_SPEAKER), nullptr);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: CheckDeviceInAudioZone_002
 * @tc.desc  : Test CheckDeviceInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, CheckDeviceInAudioZone_002, TestSize.Level1)
{
    AudioZoneContext context;
    std::string name = "TestZone";
    std::shared_ptr<AudioZoneClientManager> manager = nullptr;
    manager = std::make_shared<AudioZoneClientManager>(nullptr);
    EXPECT_NE(manager, nullptr);
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(
        manager, name, context);
    EXPECT_NE(zone, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = desc->networkId_;

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, true));

    int32_t id = 0;
    AudioZoneService::GetInstance().zoneMaps_.clear();
    AudioZoneService::GetInstance().zoneMaps_.insert({id, zone});

    auto ret = AudioZoneService::GetInstance().CheckDeviceInAudioZone(device);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: CheckDeviceInAudioZone_003
 * @tc.desc  : Test CheckDeviceInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, CheckDeviceInAudioZone_003, TestSize.Level1)
{
    AudioZoneContext context;
    std::string name = "TestZone";
    std::shared_ptr<AudioZoneClientManager> manager = nullptr;
    manager = std::make_shared<AudioZoneClientManager>(nullptr);
    EXPECT_NE(manager, nullptr);
    std::shared_ptr<AudioZone> zone = std::make_shared<AudioZone>(
        manager, name, context);
    EXPECT_NE(zone, nullptr);

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->networkId_ = "test";
    AudioDeviceDescriptor device;
    device.deviceType_ = desc->deviceType_;
    device.networkId_ = desc->networkId_;

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(desc, false));

    int32_t id = 0;
    AudioZoneService::GetInstance().zoneMaps_.clear();
    AudioZoneService::GetInstance().zoneMaps_.insert({id, zone});

    auto ret = AudioZoneService::GetInstance().CheckDeviceInAudioZone(device);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_GetZoneNameById_001
 * @tc.desc  : Test GetZoneNameById interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_GetZoneNameById_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);

    auto zone = AudioZoneService::GetInstance().FindZone(zoneId1);
    ASSERT_NE(zone, nullptr);

    EXPECT_EQ(AudioZoneService::GetInstance().GetZoneNameById(DEFAULT_ZONEID), PRIMARY_ZONE_NAME);
    EXPECT_EQ(AudioZoneService::GetInstance().GetZoneNameById(zoneId1), "TestZone1");
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: AudioZoneService_EnableSystemVolumeProxy_001
 * @tc.desc  : Test EnableSystemVolumeProxy interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, AudioZoneService_EnableSystemVolumeProxy_001, TestSize.Level1)
{
    ClearZone();
    AudioZoneService &svc = AudioZoneService::GetInstance();

    pid_t missingPid = 12345;
    int32_t invalidZone = 99999;
    EXPECT_EQ(svc.EnableSystemVolumeProxy(missingPid, 0, DEVICE_TYPE_SPEAKER, true), ERROR);
    EXPECT_EQ(svc.EnableSystemVolumeProxy(missingPid, invalidZone, DEVICE_TYPE_SPEAKER, true), ERROR);

    pid_t mainPid = 1000;
    sptr<IStandardAudioZoneClient> mainClient = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(mainClient, nullptr);
    EXPECT_EQ(svc.RegisterAudioZoneClient(mainPid, mainClient), SUCCESS);

    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_SPEAKER, true), SUCCESS);
    EXPECT_TRUE(svc.IsSystemVolumeProxyEnable(DEFAULT_ZONEID, DEVICE_TYPE_SPEAKER));

    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_SPEAKER, false), SUCCESS);
    EXPECT_FALSE(svc.IsSystemVolumeProxyEnable(DEFAULT_ZONEID, DEVICE_TYPE_SPEAKER));

    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_NONE, true), SUCCESS);
    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_NONE, false), SUCCESS);

    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_INVALID, true), SUCCESS);
    EXPECT_EQ(svc.EnableSystemVolumeProxy(mainPid, DEFAULT_ZONEID, DEVICE_TYPE_INVALID, false), SUCCESS);

    AudioZoneContext context;
    int32_t zoneId = svc.CreateAudioZone("UTZone", context, 0);
    ASSERT_NE(zoneId, ERROR);
    pid_t zonePid = 2000;
    sptr<IStandardAudioZoneClient> zoneClient = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(zoneClient, nullptr);
    EXPECT_EQ(svc.RegisterAudioZoneClient(zonePid, zoneClient), SUCCESS);

    EXPECT_EQ(svc.EnableSystemVolumeProxy(zonePid, zoneId, DEVICE_TYPE_SPEAKER, true), SUCCESS);
    EXPECT_TRUE(svc.IsSystemVolumeProxyEnable(zoneId, DEVICE_TYPE_SPEAKER));

    EXPECT_EQ(svc.EnableSystemVolumeProxy(zonePid, zoneId, DEVICE_TYPE_SPEAKER, false), SUCCESS);
    EXPECT_FALSE(svc.IsSystemVolumeProxyEnable(zoneId, DEVICE_TYPE_SPEAKER));

    EXPECT_EQ(svc.EnableSystemVolumeProxy(zonePid, invalidZone, DEVICE_TYPE_SPEAKER, true), ERROR);

    svc.UnRegisterAudioZoneClient(mainPid);
    svc.UnRegisterAudioZoneClient(zonePid);
    svc.ReleaseAudioZone(zoneId);
}
} // namespace AudioStandard
} // namespace OHOS