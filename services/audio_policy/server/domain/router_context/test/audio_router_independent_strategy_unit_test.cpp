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
#include "audio_router_independent_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_router_infra.h"
#include "audio_scene_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterIndependentStrategyUnitTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaInput_001,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE,
        "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE,
        "00:00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);

    auto systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(systemResult->deviceId_, mic->deviceId_);

    auto appResult = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(appResult->deviceId_, mic->deviceId_);

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    appResult = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaInput_002,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaInput_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(systemResult->deviceId_, mic->deviceId_);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10002Result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaInput_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    auto systemResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_NONE);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    auto app10002Result = strategy.GetMediaInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_NONE);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaInput_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);
    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, sco);

    auto stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(stream1Result->deviceId_, sco->deviceId_);

    auto stream2Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream2Result->deviceId_, mic->deviceId_);

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, nullptr);

    stream1Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SOURCE_TYPE_MIC);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream1Result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, MediaInput_006, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);

    auto callInputResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(callInputResult, nullptr);
    EXPECT_EQ(callInputResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(callInputResult->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, MediaInput_007, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, sco);

    auto mediaInputResult = strategy.GetMediaInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(mediaInputResult, nullptr);
    EXPECT_EQ(mediaInputResult->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(mediaInputResult->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, MediaOutput_001, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);

    auto callOutputResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(callOutputResult, nullptr);
    EXPECT_EQ(callOutputResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(callOutputResult->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, MediaOutput_002, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, earpiece);

    auto mediaOutputResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(mediaOutputResult, nullptr);
    EXPECT_EQ(mediaOutputResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(mediaOutputResult->deviceId_, earpiece->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AppMediaInput_001, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto callInputResult = strategy.GetCallInputDevice(TEST_APP_UID_10001,
        TEST_STREAM_INVALID_ID);
    ASSERT_NE(callInputResult, nullptr);
    EXPECT_EQ(callInputResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(callInputResult->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AppMediaInput_002, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto mediaInputResult = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, SOURCE_TYPE_MIC);
    ASSERT_NE(mediaInputResult, nullptr);
    EXPECT_EQ(mediaInputResult->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(mediaInputResult->deviceId_, sco->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AppMediaOutput_001, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto callOutputResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001,
        TEST_STREAM_INVALID_ID);
    ASSERT_NE(callOutputResult, nullptr);
    EXPECT_EQ(callOutputResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(callOutputResult->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AppMediaOutput_002, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, earpiece);

    auto mediaOutputResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(mediaOutputResult, nullptr);
    EXPECT_EQ(mediaOutputResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(mediaOutputResult->deviceId_, earpiece->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, Preference_001, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_LOW_LATENCY);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_HIGH_QUALITY);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, Preference_002, TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");
    auto a2dpIn = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE,
        "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_MIC,
        CapturerState::CAPTURER_RUNNING);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_LOW_LATENCY);

    auto app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_MIC);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_CAMCORDER,
        CapturerState::CAPTURER_RUNNING);
    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_HIGH_QUALITY);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_CAMCORDER);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);

    routerInfra.UpdateInputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, SOURCE_TYPE_LIVE,
        CapturerState::CAPTURER_RUNNING);

    app10001Result = strategy.GetMediaInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID,
        SOURCE_TYPE_LIVE);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP_IN);
    EXPECT_EQ(app10001Result->deviceId_, a2dpIn->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(systemResult->deviceId_, speaker->deviceId_);

    auto appResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(appResult->deviceId_, speaker->deviceId_);

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    appResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(appResult, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10001Result->deviceId_, speaker->deviceId_);

    auto app10002Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10001Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, earpiece);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(systemResult->deviceId_, speaker->deviceId_);

    auto app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(app10001Result->deviceId_, earpiece->deviceId_);

    auto app10002Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10002Result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_006,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, earpiece);

    strategy.SetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(app10001Result->deviceId_, earpiece->deviceId_);

    auto app10002Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_007,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, earpiece);

    auto stream1Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(stream1Result->deviceId_, earpiece->deviceId_);

    auto stream2Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(stream2Result->deviceId_, speaker->deviceId_);

    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, nullptr);

    stream1Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(stream1Result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_MediaOutput_008,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto ad2p = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterIndependentStrategy strategy;
    strategy.SetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto stream1Result = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(stream1Result->deviceId_, ad2p->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallInput_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);

    auto systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(systemResult->deviceId_, mic->deviceId_);

    auto appResult = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(appResult->deviceId_, mic->deviceId_);

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    appResult = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(appResult, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallInput_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);

    auto systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10001Result->deviceId_, mic->deviceId_);

    auto app10002Result = strategy.GetCallInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);

    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    app10001Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10001Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallInput_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);
    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    auto systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(systemResult->deviceId_, mic->deviceId_);

    auto app10001Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    auto app10002Result = strategy.GetCallInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(app10002Result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallInput_006,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, mic);
    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, sco);

    strategy.SetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    auto systemResult = strategy.GetCallInputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(app10001Result->deviceId_, sco->deviceId_);

    auto app10002Result = strategy.GetCallInputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallInput_007,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto mic = AddDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE, "00:00:00:00:00:01");
    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, mic);
    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, sco);

    auto stream1Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(stream1Result->deviceId_, sco->deviceId_);

    auto stream2Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream2Result->deviceId_, mic->deviceId_);

    strategy.SetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, nullptr);

    stream1Result = strategy.GetCallInputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(stream1Result->deviceId_, mic->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_003,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(systemResult->deviceId_, speaker->deviceId_);

    auto appResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(appResult->deviceId_, speaker->deviceId_);

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    appResult = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(appResult, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_004,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10001Result->deviceId_, speaker->deviceId_);

    auto app10002Result = strategy.GetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, nullptr);

    systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10001Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_005,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, earpiece);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(systemResult->deviceId_, speaker->deviceId_);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(app10001Result->deviceId_, earpiece->deviceId_);

    auto app10002Result = strategy.GetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10002Result, nullptr);
    EXPECT_EQ(app10002Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(app10002Result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_006,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, earpiece);

    strategy.SetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, nullptr);

    auto systemResult = strategy.GetCallOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(systemResult, nullptr);

    auto app10001Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID);
    ASSERT_NE(app10001Result, nullptr);
    EXPECT_EQ(app10001Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(app10001Result->deviceId_, earpiece->deviceId_);

    auto app10002Result = strategy.GetCallOutputDevice(TEST_APP_UID_10002, TEST_STREAM_INVALID_ID);
    EXPECT_EQ(app10002Result, nullptr);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_007,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");

    AudioRouterIndependentStrategy strategy;

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, speaker);
    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, earpiece);

    auto stream1Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(stream1Result->deviceId_, earpiece->deviceId_);

    auto stream2Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(stream2Result, nullptr);
    EXPECT_EQ(stream2Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(stream2Result->deviceId_, speaker->deviceId_);

    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, nullptr);

    stream1Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(stream1Result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterIndependentStrategyUnitTest, AudioRouterIndependentStrategyUnitTest_CallOutput_008,
    TestSize.Level1)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto sco = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto ad2p = AddDeviceDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, "00:00:00:00:00:01");

    AudioRouterIndependentStrategy strategy;
    strategy.SetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, ad2p);

    auto stream1Result = strategy.GetCallOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(stream1Result, nullptr);
    EXPECT_EQ(stream1Result->deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(stream1Result->deviceId_, sco->deviceId_);
}

}
}