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

#include <unistd.h>
#include "audio_router_context_test_base.h"
#include "audio_router_follow_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_infra.h"
#include "audio_scene_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterFollowStrategyUnitTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_001, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, earpiece);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(systemResult->deviceId_, earpiece->deviceId_);

    auto appResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(appResult->deviceId_, earpiece->deviceId_);

    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, std::make_shared<AudioDeviceDescriptor>());

    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    appResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_002, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto scoInput = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallInputDevice(SYSTEM_UID, 1, mic);

    auto systemResult = strategy.GetCallInputDevice(SYSTEM_UID,  TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(systemResult->deviceId_, mic->deviceId_);

    auto appResult = strategy.GetCallInputDevice(TEST_APP_UID_10001,  TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(appResult->deviceId_, mic->deviceId_);

    strategy.SetCallInputDevice(SYSTEM_UID,  TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetCallInputDevice(SYSTEM_UID, 1);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    appResult = strategy.GetCallInputDevice(TEST_APP_UID_10001,  TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_004, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);
    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_RUNNING);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_CAMCORDER);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_CAMCORDER);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_STOPPED);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_CAMCORDER);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);

    routerInfra.OnAppDied(TEST_APP_UID_10001);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_CAMCORDER);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
    app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_005, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_STOPPED);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_006, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, sco);
    usleep(10000); // sleep 10ms
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, nullptr);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_007, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_008, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_CAMCORDER,
        CapturerState::CAPTURER_RUNNING);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_009, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_LIVE,
        CapturerState::CAPTURER_RUNNING);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_010, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_HIGH_QUALITY);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_LOW_LATENCY);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_011, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10002, PREFERRED_LOW_LATENCY);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10002Result->deviceId_, sco->deviceId_);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_STOPPED);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10002Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_012, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_RUNNING);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);
    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_VOICE_MESSAGE,
        CapturerState::CAPTURER_STOPPED);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_VOICE_MESSAGE);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_013, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = CreateDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    mic->connectState_ = DEACTIVE_CONNECTED;

    AudioRouterFollowStrategy strategy;
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_014, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_NONE);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_015, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_HIGH_QUALITY);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_016, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    AudioRouterFollowStrategy strategy;
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);
    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10002Result->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_001,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(systemResult->deviceId_, earpiece->deviceId_);

    auto appResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(appResult->deviceId_, earpiece->deviceId_);

    strategy.SetCallOutputDevice(
        SYSTEM_UID, TEST_STREAM_INVALID_ID, std::make_shared<AudioDeviceDescriptor>());

    systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    appResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_002,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10001Result->deviceId_, speaker->deviceId_);

    auto app10002Result = strategy.GetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10001Result->deviceId_, speaker->deviceId_);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(systemResult->deviceId_, earpiece->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    strategy.SetCallOutputDevice(
        SYSTEM_UID, TEST_STREAM_INVALID_ID, std::make_shared<AudioDeviceDescriptor>());

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    auto appResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    strategy.SetCallOutputDevice(
        TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, std::make_shared<AudioDeviceDescriptor>());

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_006,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID, earpiece);

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10001Result->deviceId_, speaker->deviceId_);

    auto app10002Result = strategy.GetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10002Result->deviceId_, speaker->deviceId_);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(systemResult->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_007,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);
    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);
    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(app10001Result->deviceId_, earpiece->deviceId_);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(systemResult->deviceId_, earpiece->deviceId_);
    sceneManager.SetAudioSceneOwnerUid(0);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_008,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);

    auto invalidUidResult = strategy.GetCallOutputDevice(INVALID_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(invalidUidResult, nullptr);
    EXPECT_EQ(invalidUidResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(invalidUidResult->deviceId_, speaker->deviceId_);
    sceneManager.SetAudioSceneOwnerUid(0);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_009,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    auto invalidUidResult = strategy.GetCallOutputDevice(INVALID_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(invalidUidResult, nullptr);
    EXPECT_EQ(invalidUidResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(invalidUidResult->deviceId_, earpiece->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_010,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;
    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto invalidUidResult = strategy.GetCallOutputDevice(INVALID_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(invalidUidResult, nullptr);
    EXPECT_EQ(invalidUidResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_CallOutput_011,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto noneDevice = std::make_shared<AudioDeviceDescriptor>();
    noneDevice->deviceType_ = DEVICE_TYPE_NONE;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, noneDevice);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_017, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE,
        "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, earpiece);
    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, INVALID_STREAM_ID, speaker);
    AudioRouterInfra::GetInstance().UpdateOutputStreamState(
        TEST_APP_UID_10001, 100000, STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    AudioRouterInfra::GetInstance().UpdateAppForegroundState(TEST_APP_UID_10001, true);
    auto appResult = strategy.GetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID);
    ASSERT_NE(appResult, nullptr);

    strategy.SetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID, speaker);
    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);

    strategy.SetMediaOutputDevice(
        SYSTEM_UID, INVALID_STREAM_ID, std::make_shared<AudioDeviceDescriptor>());
    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, INVALID_STREAM_ID);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    strategy.SetMediaOutputDevice(
        TEST_APP_UID_10001, INVALID_STREAM_ID, std::make_shared<AudioDeviceDescriptor>());
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_StreamLevel_001,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, mic);

    auto stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream1Result->deviceId_, mic->deviceId_);

    auto stream2Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_StreamLevel_002,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, mic);

    auto stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream1Result->deviceId_, mic->deviceId_);

    auto appLevelResult = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(appLevelResult, nullptr);
    EXPECT_EQ(appLevelResult->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(appLevelResult->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_StreamLevel_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, mic);

    auto stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream1Result->deviceId_, mic->deviceId_);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, nullptr);

    stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_NONE);
}
}
}
