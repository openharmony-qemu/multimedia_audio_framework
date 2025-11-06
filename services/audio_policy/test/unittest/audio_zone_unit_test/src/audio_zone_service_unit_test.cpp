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
    int32_t zoneId1 = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeDegree(0, volumeType, volumeDegree, 0), 0);
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeDegree(zoneId1, volumeType, volumeDegree, 0), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(0, volumeType), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(zoneId1, volumeType), 0);

    auto zone = AudioZoneService::GetInstance().FindZone(zoneId1);
    ASSERT_NE(zone, nullptr);
    int32_t clientPid = 1;
    zone->EnableSystemVolumeProxy(clientPid, true);
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeDegree(zoneId1, volumeType, volumeDegree, 0), 0);
    EXPECT_NE(AudioZoneService::GetInstance().GetSystemVolumeDegree(zoneId1, volumeType), volumeDegree);

    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId1);
}

/**
 * @tc.name  : Test AudioZoneServiceUnitTest.
 * @tc.number: CheckExistUidInAudioZone_001
 * @tc.desc  : Test CheckExistUidInAudioZone interface.
 */
HWTEST_F(AudioZoneServiceUnitTest, CheckExistUidInAudioZone_001, TestSize.Level1)
{
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
    std::shared_ptr<AudioZone> audioZone = AudioZoneService::GetInstance().FindZone(zoneId);
    audioZone->BindByKey(AudioZoneBindKey(0, "", "", STREAM_USAGE_UNKNOWN));
    EXPECT_EQ(AudioZoneService::GetInstance().CheckExistUidInAudioZone(), false);
    audioZone->BindByKey(AudioZoneBindKey(1, "", "", STREAM_USAGE_UNKNOWN));
    EXPECT_EQ(AudioZoneService::GetInstance().CheckExistUidInAudioZone(), true);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}
} // namespace AudioStandard
} // namespace OHOS
 