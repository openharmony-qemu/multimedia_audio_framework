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
#include "audio_router_follow_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_infra.h"

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
    EXPECT_TRUE(result2.empty());
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices2 = { bluetooth };

    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10001, devices1);
    strategy.UpdateCurrentOutputDevice(TEST_APP_UID_10002, devices2);

    auto result1 = strategy.GetCurrentOutputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result1.size(), 2);
    EXPECT_EQ(result1[0]->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result1[1]->deviceType_, DEVICE_TYPE_EARPIECE);

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
    EXPECT_TRUE(result2.empty());
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices2 = { a2dp };

    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10001, devices1);
    strategy.UpdateCurrentInputDevice(TEST_APP_UID_10002, devices2);

    auto result1 = strategy.GetCurrentInputDevice(TEST_APP_UID_10001);
    ASSERT_EQ(result1.size(), 2);
    EXPECT_EQ(result1[0]->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(result1[1]->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);

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
    EXPECT_EQ(result2, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetRecognitionInputDevice_001, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetRecognitionInputDevice();
    EXPECT_EQ(result, nullptr);
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
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10001, 1, speaker);
    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10001, 2, earpiece);

    auto result1 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_EARPIECE);

    auto result3 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 3);
    EXPECT_EQ(result3, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_002, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10001, 1, speaker);

    auto result1 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10001, 1, nullptr);

    auto result2 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result2, nullptr);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, UpdateStreamDefaultDevice_003, TestSize.Level1)
{
    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto bluetooth = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10001, 1, speaker);
    strategy.UpdateStreamDefaultDevice(TEST_APP_UID_10002, 2, bluetooth);

    auto result1 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetStreamDefaultDevice(TEST_APP_UID_10002, 2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
}

HWTEST_F(AudioRouterSelectStrategyExtraTest, GetStreamDefaultDevice_004, TestSize.Level1)
{
    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetStreamDefaultDevice(TEST_APP_UID_10001, 1);
    EXPECT_EQ(result, nullptr);
}

}
}
