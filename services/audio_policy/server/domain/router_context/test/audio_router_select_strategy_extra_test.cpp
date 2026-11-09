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

#include "audio_router_context_test_base.h"
#include "audio_device_simple_descriptor.h"
#include "audio_router_independent_strategy.h"
#include "audio_router_follow_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_infra.h"
#include "audio_zone_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterSelectStrategyExtraTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentOutputDevice_001, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { speaker, earpiece };

    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10001, devices);

    auto result = strategy.GetCurrentOutputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result[0]->deviceId_, speaker->deviceId_);
    EXPECT_EQ(result[1]->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result[1]->deviceId_, earpiece->deviceId_);

    auto result2 = strategy.GetCurrentOutputDevice(TEST_APP_UID_10002);
    EXPECT_FALSE(result2.empty());
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentOutputDevice_002, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> emptyDevices;

    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10001, emptyDevices);

    auto result = strategy.GetCurrentOutputDevice(TEST_APP_UID_10001);
    EXPECT_TRUE(result.empty());
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentOutputDevice_004, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:03");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices1 = { speaker, earpiece };
    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10001, devices1);
    auto result1 = strategy.GetCurrentOutputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result1.size(), 2);
    EXPECT_EQ(result1[0]->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result1[1]->deviceType_, DEVICE_TYPE_EARPIECE);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices2 = { bluetooth };
    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10002, devices2);
    auto result2 = strategy.GetCurrentOutputDevice(TEST_APP_UID_10002);
    ASSERT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentInputDevice_001, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { mic, bluetooth };

    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, devices);

    auto result = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(result[0]->deviceId_, mic->deviceId_);
    EXPECT_EQ(result[1]->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(result[1]->deviceId_, bluetooth->deviceId_);

    auto result2 = strategy.GetCurrentInputDevice(TEST_APP_UID_10002);
    EXPECT_FALSE(result2.empty());
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentInputDevice_002, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> emptyDevices;

    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, emptyDevices);

    auto result = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    EXPECT_TRUE(result.empty());
}


HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentInputDevice_004, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");
    auto a2dp = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, "00:00:00:00:00:03");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices1 = { mic, bluetooth };
    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, devices1);
    auto result1 = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result1.size(), 2);
    EXPECT_EQ(result1[0]->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(result1[1]->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices2 = { a2dp };
    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10002, devices2);
    auto result2 = strategy.GetCurrentInputDevice(TEST_APP_UID_10002);
    ASSERT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentInputDevice_007, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { mic, bluetooth };

    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, devices);

    auto result1 = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result1.size(), 2);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> emptyDevices;
    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, emptyDevices);

    auto result2 = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    EXPECT_TRUE(result2.empty());
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateCurrentInputDevice_008, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { mic, nullptr, bluetooth };

    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, devices);

    auto result1 = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    EXPECT_TRUE(result1.empty());
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, SetRecognitionInputDevice_001, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    strategy.SetRecognitionInputDevice(mic);

    auto result = strategy.GetRecognitionInputDevice();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, SetRecognitionInputDevice_002, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.SetRecognitionInputDevice(mic);

    auto result1 = strategy.GetRecognitionInputDevice();
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_MIC);

    strategy.SetRecognitionInputDevice(bluetooth);

    auto result2 = strategy.GetRecognitionInputDevice();
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(result2->deviceId_, bluetooth->deviceId_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, SetRecognitionInputDevice_003, TestSize.Level1)
{
    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    strategy.SetRecognitionInputDevice(mic);

    auto result1 = strategy.GetRecognitionInputDevice();
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_MIC);

    strategy.SetRecognitionInputDevice(nullptr);

    auto result2 = strategy.GetRecognitionInputDevice();
    EXPECT_TRUE(result2 == nullptr || result2->deviceType_ == DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, Get1stCurrentInputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    auto curr = strategy.Get1stCurrentInputDevice(SYSTEM_UID);
    EXPECT_EQ(curr.deviceId_, TEST_APP_UID_10001);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, Get1stCurrentOutputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    auto curr = strategy.Get1stCurrentOutputDevice(SYSTEM_UID);
    EXPECT_EQ(curr.deviceId_, TEST_APP_UID_10001);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, IsCurrentInputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    bool ret = strategy.IsCurrentInputDevice(TEST_APP_UID_10001, SYSTEM_UID);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, IsCurrentOutputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    auto ret = strategy.IsCurrentOutputDevice(TEST_APP_UID_10001, SYSTEM_UID);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, FindCurrentInputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    auto descs = strategy.FindCurrentInputDevice({DEVICE_TYPE_BLUETOOTH_A2DP_IN});
    EXPECT_EQ(descs.size(), 1);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, FindCurrentOutputDevice_001, TestSize.Level1)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = TEST_APP_UID_10001;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    strategy.UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    auto descs = strategy.FindCurrentOutputDevice({DEVICE_TYPE_BLUETOOTH_A2DP});
    EXPECT_EQ(descs.size(), 1);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetRecognitionInputDevice_001, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetRecognitionInputDevice();
    EXPECT_TRUE(result == nullptr || result->deviceType_ == DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetRecognitionInputDevice_002, TestSize.Level1)
{
    auto a2dp = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    strategy.SetRecognitionInputDevice(a2dp);

    auto result = strategy.GetRecognitionInputDevice();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(result->deviceId_, a2dp->deviceId_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_001, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, 1, speaker);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, 2, sco);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);

    auto result3 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 3);
    EXPECT_EQ(result3, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_002, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, 1, speaker);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, 1, nullptr);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result2, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_003, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, 1, speaker);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10002, 2, bluetooth);

    auto result1 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10002, 2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_004, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, 1, speaker);

    auto result1 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, 1, nullptr);

    auto result2 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result2, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetStreamDefaultDevice_005, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result, nullptr);
    result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetOnlineDeviceDescriptor_001, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:01");
    AudioDeviceSimpleDescriptor simpleDesc(speaker->deviceType_, speaker->deviceRole_, speaker->deviceId_,
        speaker->networkId_, speaker->macAddress_);

    auto result = simpleDesc.GetOnlineDeviceDescriptor();

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, speaker->deviceType_);
    EXPECT_EQ(result->deviceRole_, speaker->deviceRole_);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
    EXPECT_EQ(result->networkId_, speaker->networkId_);
    EXPECT_EQ(result->macAddress_, speaker->macAddress_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetOnlineDeviceDescriptor_002, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:02");
    AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:10:03");
    AudioDeviceSimpleDescriptor simpleDesc(speaker->deviceType_, speaker->deviceRole_, speaker->deviceId_,
        "mismatch_network", "");

    auto result = simpleDesc.GetOnlineDeviceDescriptor();

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, speaker->deviceType_);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
    EXPECT_EQ(result->macAddress_, speaker->macAddress_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetOnlineDeviceDescriptor_003, TestSize.Level1)
{
    AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:04");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "11:22:33:44:55:66");
    AudioDeviceSimpleDescriptor simpleDesc(sco->deviceType_, INPUT_DEVICE, sco->deviceId_,
        "another_network", sco->macAddress_);

    auto result = simpleDesc.GetOnlineDeviceDescriptor();

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, sco->deviceType_);
    EXPECT_EQ(result->macAddress_, sco->macAddress_);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetOnlineDeviceDescriptor_004, TestSize.Level1)
{
    AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:05");
    AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:10:06");
    AudioDeviceSimpleDescriptor simpleDesc(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, 99999,
        "unknown_network", "ff:ee:dd:cc:bb:aa");

    auto result = simpleDesc.GetOnlineDeviceDescriptor();

    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetOnlineDeviceDescriptor_005, TestSize.Level1)
{
    AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:07");
    AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:10:08");
    AudioDeviceSimpleDescriptor simpleDesc(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, 99998,
        "unknown_network", "ff:ee:dd:cc:bb:ab");

    auto result = simpleDesc.GetOnlineDeviceDescriptor();

    EXPECT_EQ(result, nullptr);
}

class AudioDeviceSimpleDescriptorZoneTest : public AudioRouterContextTestBase {
public:
    void SetUp() override
    {
        AudioRouterContextTestBase::SetUp();
        savedZones_.swap(AudioZoneService::GetInstance().zoneMaps_);
    }

    void TearDown() override
    {
        auto &zones = AudioZoneService::GetInstance().zoneMaps_;
        zones.clear();
        zones.swap(savedZones_);
        AudioRouterContextTestBase::TearDown();
    }

    std::shared_ptr<AudioZone> AddZone()
    {
        auto zone = std::make_shared<AudioZone>(nullptr, "DescriptorTestZone", AudioZoneContext());
        AudioZoneService::GetInstance().zoneMaps_[zone->GetId()] = zone;
        return zone;
    }

private:
    std::unordered_map<int32_t, std::shared_ptr<AudioZone>> savedZones_;
};

HWTEST_F(AudioDeviceSimpleDescriptorZoneTest, GetOnlineDeviceDescriptor_ZoneDevices, TestSize.Level1)
{
    auto speaker = CreateDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "zone_speaker");
    auto mic = CreateDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "zone_mic");
    AddZone()->devices_.emplace_back(speaker, true);
    AddZone()->devices_.emplace_back(mic, true);

    for (const auto &device : {speaker, mic}) {
        ASSERT_EQ(AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(device->deviceId_), nullptr);
        AudioDeviceSimpleDescriptor simpleDesc(device->deviceType_, device->deviceRole_, -1,
            device->networkId_, "");
        auto result = simpleDesc.GetOnlineDeviceDescriptor();
        ASSERT_NE(result, nullptr);
        EXPECT_NE(result.get(), device.get());
        EXPECT_EQ(result->deviceId_, device->deviceId_);
        EXPECT_EQ(result->deviceRole_, device->deviceRole_);
        EXPECT_EQ(result->networkId_, device->networkId_);
        result->deviceName_ = "ChangedCopy";
        EXPECT_NE(result->deviceName_, device->deviceName_);
    }
}

HWTEST_F(AudioDeviceSimpleDescriptorZoneTest, GetOnlineDeviceDescriptor_GlobalFirst, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "global_speaker");
    auto zoneSpeaker = std::make_shared<AudioDeviceDescriptor>(*speaker);
    zoneSpeaker->deviceName_ = "ZoneSpeaker";
    AddZone()->devices_.emplace_back(zoneSpeaker, true);
    AudioDeviceSimpleDescriptor simpleDesc(speaker->deviceType_, speaker->deviceRole_, speaker->deviceId_,
        speaker->networkId_, speaker->macAddress_);

    auto result = simpleDesc.GetOnlineDeviceDescriptor();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceName_, speaker->deviceName_);
    EXPECT_NE(result.get(), speaker.get());

    AudioDeviceManager::GetAudioDeviceManager().RemoveNewDevice(speaker);
    result = simpleDesc.GetOnlineDeviceDescriptor();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceName_, zoneSpeaker->deviceName_);
}

HWTEST_F(AudioDeviceSimpleDescriptorZoneTest, GetOnlineDeviceDescriptor_UnavailableZoneDevice, TestSize.Level1)
{
    auto speaker = CreateDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "zone_speaker");
    auto zone = AddZone();
    zone->devices_.emplace_back(speaker, false);
    AudioDeviceSimpleDescriptor simpleDesc(speaker->deviceType_, speaker->deviceRole_, speaker->deviceId_,
        speaker->networkId_, speaker->macAddress_);

    EXPECT_EQ(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
    zone->devices_.back().second = true;
    EXPECT_NE(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
    simpleDesc.deviceId_ = -1;
    EXPECT_NE(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
    simpleDesc.networkId_ = "unknown_network";
    EXPECT_EQ(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
    simpleDesc.networkId_ = speaker->networkId_;
    simpleDesc.deviceType_ = DEVICE_TYPE_MIC;
    EXPECT_EQ(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
    simpleDesc.deviceType_ = speaker->deviceType_;
    zone->devices_.clear();
    EXPECT_EQ(simpleDesc.GetOnlineDeviceDescriptor(), nullptr);
}

}
}
