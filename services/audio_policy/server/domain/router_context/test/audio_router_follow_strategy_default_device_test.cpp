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

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_InCall_001, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);

    sceneManager.SetAudioSceneOwnerUid(0);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_InCall_002, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");
    auto earpiece = AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:00:02");

    AudioRouterFollowStrategy strategy;

    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);
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
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);

    result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10002, TEST_STREAM_ID_3);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result->deviceId_, speaker->deviceId_);

    sceneManager.SetAudioSceneOwnerUid(0);
}

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_InCall_003, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);

    sceneManager.SetAudioSceneOwnerUid(0);
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

HWTEST_F(AudioRouterFollowStrategyDefaultDeviceTest, GetCallDefaultOutputDevice_NotInCall_004, TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    auto &sceneManager = AudioSceneManager::GetInstance();

    auto speaker = AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:00:01");

    AudioRouterFollowStrategy strategy;

    sceneManager.SetAudioSceneOwnerUid(TEST_APP_UID_10001);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1, STREAM_USAGE_VOICE_COMMUNICATION,
        RendererState::RENDERER_STOPPED);
    strategy.UpdateCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1, speaker);

    auto result = strategy.GetCallDefaultOutputDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result, nullptr);

    sceneManager.SetAudioSceneOwnerUid(0);
}

}
}
