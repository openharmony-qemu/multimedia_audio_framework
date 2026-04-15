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
#include "audio_router_independent_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_infra.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterSelectStrategyExcludeDeviceUnitTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_001,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_002,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);
    strategy.ExcludeDevices(devices, MEDIA_INPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, ALL_MEDIA_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, ALL_MEDIA_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);
    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_006,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_007,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { speaker, earpiece };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(speaker, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(earpiece, MEDIA_OUTPUT_DEVICES));

    AudioRouterInfra::GetInstance().UnexcludeAllDevice();

    EXPECT_FALSE(strategy.IsDeviceExcluded(speaker, MEDIA_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(earpiece, MEDIA_OUTPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_008,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);
    strategy.ExcludeDevices(devices, CALL_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, ALL_MEDIA_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, ALL_CALL_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_009,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, ALL_MEDIA_DEVICES);
    strategy.ExcludeDevices(devices, ALL_CALL_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_INPUT_DEVICES));

    strategy.UnexcludeDevices(devices, ALL_MEDIA_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_INPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_010,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_011,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, sco);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_012,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };
    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    auto app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_013,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterIndependentStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, sco);
    auto stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    EXPECT_NE(stream1Result, nullptr);

    strategy.ExcludeDevices(devices, MEDIA_INPUT_DEVICES);
    auto stream2Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_014,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");
    AddPairDeviceDescriptor(sco, scoInput);

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, MEDIA_INPUT_DEVICES));

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(scoInput, MEDIA_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_015,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");
    AddPairDeviceDescriptor(sco, scoInput);

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(scoInput, MEDIA_INPUT_DEVICES));

    strategy.UnexcludeDevices(devices, MEDIA_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, MEDIA_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, MEDIA_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_016,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");
    AddPairDeviceDescriptor(sco, scoInput);

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices1 = { sco };
    strategy.ExcludeDevices(devices1, CALL_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices2 = { scoInput };
    strategy.UnexcludeDevices(devices2, CALL_INPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_017,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");
    AddPairDeviceDescriptor(sco, scoInput);

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, CALL_OUTPUT_DEVICES);

    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));

    strategy.UnexcludeDevices(devices, CALL_OUTPUT_DEVICES);

    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_018,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");
    AddPairDeviceDescriptor(sco, scoInput);

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco, nullptr };

    strategy.ExcludeDevices(devices, CALL_OUTPUT_DEVICES);
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_TRUE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
    strategy.UnexcludeDevices(devices, CALL_OUTPUT_DEVICES);
    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
}

HWTEST_F(AudioRouterSelectStrategyExcludeDeviceUnitTest, AudioRouterSelectStrategyExcludeDeviceUnitTest_019,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto sco = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = CreateDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = { sco };

    strategy.ExcludeDevices(devices, CALL_OUTPUT_DEVICES);
    EXPECT_TRUE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
    strategy.UnexcludeDevices(devices, CALL_OUTPUT_DEVICES);
    EXPECT_FALSE(strategy.IsDeviceExcluded(sco, CALL_OUTPUT_DEVICES));
    EXPECT_FALSE(strategy.IsDeviceExcluded(scoInput, CALL_INPUT_DEVICES));
}

}
}