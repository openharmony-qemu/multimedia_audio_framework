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

#include "audio_policy_utils.h"
#include "audio_active_device_new_unit_test.h"
#include "audio_zone.h"
#include "audio_zone_service.h"
#include "audio_router_select_strategy.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const int32_t MEDIA_SERVICE_UID = 1013;
void AudioActiveDeviceNewUnitTest::SetUpTestCase(void) {}
void AudioActiveDeviceNewUnitTest::TearDownTestCase(void) {}
void AudioActiveDeviceNewUnitTest::SetUp(void) {}
void AudioActiveDeviceNewUnitTest::TearDown(void) {}

/**
* @tc.name  : Test GetActiveA2dpDeviceStreamInfo.
* @tc.number: AudioActiveDeviceNewUnitTest_GetActiveA2dpDeviceStreamInfo_001.
* @tc.desc  : Test GetActiveA2dpDeviceStreamInfo interface.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetActiveA2dpDeviceStreamInfo_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->activeBTInDevice_ = "AA:BB:CC:DD:EE:FF";
    auto& a2dpDevice = audioActiveDevice->audioA2dpDevice_;
    A2dpDeviceConfigInfo info;
    info.streamInfo.samplingRate = {SAMPLE_RATE_8000, SAMPLE_RATE_48000};
    info.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    info.streamInfo.channelLayout = {AudioChannelLayout::CH_LAYOUT_STEREO};
    info.absVolumeSupport = true;
    info.volumeLevel = 8;
    info.mute = false;
    a2dpDevice.connectedA2dpInDeviceMap_["AA:BB:CC:DD:EE:FF"] = info;
    AudioStreamInfo streamInfo;
    bool result = audioActiveDevice->GetActiveA2dpDeviceStreamInfo(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, streamInfo);

    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_GetMaxAmplitude_001.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetMaxAmplitude_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->deviceId_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = 2;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(0, audioInterrupt);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_GetMaxAmplitude_002.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetMaxAmplitude_002, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(0, audioInterrupt);
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventToBt_001.
* @tc.desc  : Test NotifyUserSelectionEventToBt.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventToBt_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    auto outputDevice = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, INPUT_DEVICE);
    outputDevice->deviceId_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(outputDevice);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {outputDevice});
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, INPUT_DEVICE, 0, 0,
        "nearlink_net_001"
    );
    nearLinkDevice->macAddress_ = "AA:BB:CC:DD:EE:FF";
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    audioActiveDevice->NotifyUserSelectionEventToBt(nearLinkDevice, streamUsage);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_001.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_BLUETOOTH_SCO,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_002.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_002, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_BLUETOOTH_A2DP_IN,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_003.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_003, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = 1;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_NEARLINK_IN,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test SortDevicesByPriority.
* @tc.number: SortDevicesByPriority.
* @tc.desc  : Test SortDevicesByPriority.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SortDevicesByPriority, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    descs.push_back(nullptr);

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    descs[0]->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs[0]->deviceRole_ = OUTPUT_DEVICE;
    descs[0]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = nullptr;
    descs[1] = std::make_shared<AudioDeviceDescriptor>();
    descs[1]->deviceType_ = DEVICE_TYPE_EARPIECE;
    descs[1]->deviceRole_ = OUTPUT_DEVICE;
    descs[1]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    descs[0]->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs[0]->deviceRole_ = OUTPUT_DEVICE;
    descs[0]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);
}

/**
* @tc.name  : Test SortDevicesByPriority2.
* @tc.number: SortDevicesByPriority2.
* @tc.desc  : Test SortDevicesByPriority audio zone branch.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SortDevicesByPriority2, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    ASSERT_NE(audioActiveDevice, nullptr);

    auto runningRemoteCast = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    runningRemoteCast->networkId_ = LOCAL_NETWORK_ID;
    runningRemoteCast->hasStreamRunning_ = true;

    AudioZoneContext context;
    auto manager = std::make_shared<AudioZoneClientManager>(nullptr);
    ASSERT_NE(manager, nullptr);
    auto zone = std::make_shared<AudioZone>(manager, "testzone", context);
    ASSERT_NE(zone, nullptr);

    auto zoneDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    zoneDevice->networkId_ = "device1";
    zoneDevice->hasStreamRunning_ = false;

    auto zoneRunningDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE);
    zoneRunningDevice->networkId_ = "device2";
    zoneRunningDevice->hasStreamRunning_ = true;

    zone->devices_.clear();
    zone->devices_.push_back(std::make_pair(zoneDevice, true));
    zone->devices_.push_back(std::make_pair(zoneRunningDevice, true));

    auto &zoneService = AudioZoneService::GetInstance();
    auto backupZoneMaps = std::move(zoneService.zoneMaps_);
    zoneService.zoneMaps_.clear();
    zoneService.zoneMaps_.insert({1, zone});

    audioActiveDevice->volumeAdjustZoneId_ = 1;

    auto inZoneDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    inZoneDevice->networkId_ = "device1";
    inZoneDevice->hasStreamRunning_ = false;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {runningRemoteCast, inZoneDevice};
    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.front(), inZoneDevice);

    auto anotherInZoneDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE);
    anotherInZoneDevice->networkId_ = "device2";
    anotherInZoneDevice->hasStreamRunning_ = true;

    descs = {inZoneDevice, anotherInZoneDevice};
    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.front(), anotherInZoneDevice);

    zoneService.zoneMaps_.clear();
    zoneService.zoneMaps_ = std::move(backupZoneMaps);
}

/**
* @tc.name  : Test SortDevicesByPriority3.
* @tc.number: SortDevicesByPriority3.
* @tc.desc  : Test SortDevicesByPriority hasStreamRunning branch.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SortDevicesByPriority3, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    ASSERT_NE(audioActiveDevice, nullptr);

    auto localSpeaker = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    localSpeaker->networkId_ = LOCAL_NETWORK_ID;
    localSpeaker->hasStreamRunning_ = false;

    auto runningRemoteCast = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    runningRemoteCast->networkId_ = LOCAL_NETWORK_ID;
    runningRemoteCast->hasStreamRunning_ = true;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {localSpeaker, runningRemoteCast};
    audioActiveDevice->volumeAdjustZoneId_ = 0;
    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.front(), runningRemoteCast);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetDeviceForVolume_001, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_ALL), nullptr);
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_RING), nullptr);
    tmp->volumeTypeDeviceMap_[STREAM_MUSIC] = {};
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
    tmp->volumeTypeDeviceMap_[STREAM_MUSIC].push_back(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetDeviceForVolume_002, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
    tmp->streamUsageDeviceMap_[STREAM_USAGE_MUSIC] = {};
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
    tmp->streamUsageDeviceMap_[STREAM_USAGE_MUSIC].push_back(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, IsAvailableFrontDeviceInVector, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    EXPECT_EQ(tmp->IsAvailableFrontDeviceInVector(descs), false);
    descs.push_back(nullptr);
    EXPECT_EQ(tmp->IsAvailableFrontDeviceInVector(descs), false);
    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_EQ(tmp->IsAvailableFrontDeviceInVector(descs), true);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetRealUid, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 0;
    EXPECT_EQ(tmp->GetRealUid(streamDesc), 0);
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    streamDesc->appInfo_.appUid = 0;
    EXPECT_EQ(tmp->GetRealUid(streamDesc), 0);
}

} // namespace AudioStandard
} // namespace OHOS
