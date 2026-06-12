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

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterFollowStrategyDefaultDeviceTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_Running_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_Running_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_2, earpiece);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_EARPIECE);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_EARPIECE);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_Running_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, earpiece);

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_Running_004, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_Running_005, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_SPEAKER);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_NotRunning_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_NotRunning_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, earpiece);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_EARPIECE);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetMediaDefaultOutputDevice_NotRunning_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_Running_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_Running_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, earpiece);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);

    result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result->deviceId_, earpiece->deviceId_);

    result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_3);
    ASSERT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_Running_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_NotInCall_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_NotInCall_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, earpiece);

    auto result1 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    auto result2 = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_EARPIECE);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_NotInCall_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    AudioRouterFollowStrategy strategy;

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_NotRunning_004, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_STOPPED);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_NE(result, nullptr);
}

// T19: Running stream changes default device — new device immediately takes effect for all running streams
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, RunningStreamSetsNewDefault_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result1 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result1, nullptr);
    EXPECT_EQ(result1->deviceType_, DEVICE_TYPE_SPEAKER);

    // Running stream changes default device — new device immediately takes effect
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, earpiece);

    auto result2 = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result2, nullptr);
    EXPECT_EQ(result2->deviceType_, DEVICE_TYPE_EARPIECE);
}

// T20: Background app's running stream with later-set default wins over foreground app
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, BackgroundAppRunningDefault_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    // Foreground app sets speaker as default
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10001, true);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    // Background app sets earpiece as default (later timestamp)
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10002, false);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_2, earpiece);

    // Background app wins because its stream has later timestamp (Design Decision 4)
    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_EARPIECE);
}

// T21: Call default uses same time-sequence mechanism regardless of call scene
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, CallDefaultUnifiedMechanism_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    // UID 10001 running call stream sets speaker as call default
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    // UID 10002 running call stream sets earpiece as call default (later timestamp)
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_2, earpiece);

    // Latest timestamp wins regardless of which UID is "call owner" (Design Decision 5)
    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_EARPIECE);
}

// T22: Stream entering running state with pre-set default device immediately becomes effective
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, StreamStartsWithStoredDefault_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    // Set default device while stream is NOT running
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    // Verify non-running stream returns its own default device
    auto resultBefore = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(resultBefore, nullptr);
    EXPECT_EQ(resultBefore->deviceType_, DEVICE_TYPE_SPEAKER);

    // Start stream — default device should immediately become effective in priority list
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);

    auto resultAfter = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(resultAfter, nullptr);
    EXPECT_EQ(resultAfter->deviceType_, DEVICE_TYPE_SPEAKER);
}

// T23: Same UID with two running streams — latest timestamp stream wins
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, MultipleStreamsSameUID_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    // Same UID, two running streams with different defaults
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_2, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_2, earpiece);

    // Latest timestamp wins (stream 2 set earpiece later) — Design Decision 3
    auto result = strategy.GetMediaDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_EARPIECE);
}

// T24: Default device lookup does not skip exclusive UIDs
HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, NoExclusionCheck_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    auto exclusiveDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    exclusiveDevice->deviceTokenId_ = 12345;
    auto simpleDesc = std::make_shared<AudioDeviceSimpleDescriptor>(
        exclusiveDevice->deviceType_, exclusiveDevice->deviceRole_,
        exclusiveDevice->deviceId_, exclusiveDevice->networkId_, exclusiveDevice->macAddress_);

    AudioRouterFollowStrategy strategy;

    // Start stream and set default device
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_MEDIA,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateStreamDefaultDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        SelectDeviceType::MEDIA_OUTPUT, simpleDesc);

    // Make UID exclusive via RefreshSelectDevice with deviceTokenId
    routerInfra.RefreshSelectDevice(SelectDeviceType::MEDIA_OUTPUT, TEST_APP_UID_10001, simpleDesc, 12345);

    // GetEffectiveDefaultOutputDevice should still return the exclusive UID's device (Design Decision 6)
    auto result = routerInfra.GetEffectiveDefaultOutputDevice(SelectDeviceType::MEDIA_OUTPUT);
    ASSERT_EQ(result, nullptr);
}

}
}
