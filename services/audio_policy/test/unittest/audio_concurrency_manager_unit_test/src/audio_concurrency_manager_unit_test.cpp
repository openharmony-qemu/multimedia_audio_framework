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

#include "audio_errors.h"
#include "audio_concurrency_manager_unit_test.h"

#include "audio_policy_server.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioConcurrencyManagerUnitTest::SetUpTestCase(void) {}
void AudioConcurrencyManagerUnitTest::TearDownTestCase(void) {}
void AudioConcurrencyManagerUnitTest::SetUp(void) {}
void AudioConcurrencyManagerUnitTest::TearDown(void) {}

/**
 * @tc.name: GetPipeTypeByRouteFlag_001
 * @tc.number: GetPipeTypeByRouteFlag_001
 * @tc.desc: Test GetPipeTypeByRouteFlag - all conditions pass, find matching pipeType
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_001, TestSize.Level1)
{
    uint32_t flag = 0x01;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_OUT_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_002
 * @tc.number: GetPipeTypeByRouteFlag_002
 * @tc.desc: Test GetPipeTypeByRouteFlag - first if conditions fails (audioMode mismatch)
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_002, TestSize.Level1)
{
    uint32_t flag = 0x01;
    AudioMode audioMode = AUDIO_MODE_RECORD;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_IN_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_003
 * @tc.number: GetPipeTypeByRouteFlag_003
 * @tc.desc: Test GetPipeTypeByRouteFlag - second if conditions fails (audioMode mismatch)
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_003, TestSize.Level1)
{
    uint32_t flag = 0x0;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_OUT_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_004
 * @tc.number: GetPipeTypeByRouteFlag_004
 * @tc.desc: Test GetPipeTypeByRouteFlag
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_004, TestSize.Level1)
{
    uint32_t flag = 0x02;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_OUT_DIRECT_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_005
 * @tc.number: GetPipeTypeByRouteFlag_005
 * @tc.desc: Test GetPipeTypeByRouteFlag
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_005, TestSize.Level1)
{
    uint32_t flag = 0xFF;
    AudioMode audioMode = AUDIO_MODE_RECORD;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_IN_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_006
 * @tc.number: GetPipeTypeByRouteFlag_006
 * @tc.desc: Test GetPipeTypeByRouteFlag
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_006, TestSize.Level1)
{
    uint32_t flag = 0x400000;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_OUT_NORMAL);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_007
 * @tc.number: GetPipeTypeByRouteFlag_007
 * @tc.desc: Test GetPipeTypeByRouteFlag for live independent input pipe
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_007, TestSize.Level1)
{
    uint32_t flag = AUDIO_INPUT_FLAG_LIVE;
    AudioMode audioMode = AUDIO_MODE_RECORD;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_IN_NORMAL_LIVE);
}

/**
 * @tc.name: GetPipeTypeByRouteFlag_008
 * @tc.number: GetPipeTypeByRouteFlag_008
 * @tc.desc: Test voice message independent input pipe mapping.
 */
HWTEST_F(AudioConcurrencyManagerUnitTest, GetPipeTypeByRouteFlag_008, TestSize.Level1)
{
    uint32_t flag = AUDIO_INPUT_FLAG_VOICE_MESSAGE;
    AudioMode audioMode = AUDIO_MODE_RECORD;

    AudioConcurrencyManager &concurrencyService = AudioConcurrencyManager::GetInstance();

    AudioPipeType result = concurrencyService.GetPipeTypeByRouteFlag(flag, audioMode);

    EXPECT_EQ(result, PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE);
}
/** @tc.desc: Ultra fast concurrency uses only explicit rules and defaults to PLAY_BOTH when absent. */
HWTEST_F(AudioConcurrencyManagerUnitTest, UltraFastRecordingConcurrency_001, TestSize.Level1)
{
    AudioConcurrencyManager manager;
    EXPECT_EQ(manager.GetPipeTypeByRouteFlag(AUDIO_INPUT_FLAG_ULTRA_FAST, AUDIO_MODE_RECORD), PIPE_TYPE_IN_ULTRA_FAST);
    manager.concurrencyConfigMap_[{PIPE_TYPE_IN_LOWLATENCY, PIPE_TYPE_IN_VOIP}] = CONCEDE_INCOMING;
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_ULTRA_FAST, PIPE_TYPE_IN_VOIP), PLAY_BOTH);
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_LOWLATENCY, PIPE_TYPE_IN_VOIP), CONCEDE_INCOMING);
    manager.concurrencyConfigMap_[{PIPE_TYPE_IN_VOIP, PIPE_TYPE_IN_LOWLATENCY}] = CONCEDE_EXISTING;
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_VOIP, PIPE_TYPE_IN_ULTRA_FAST), PLAY_BOTH);
    manager.concurrencyConfigMap_[{PIPE_TYPE_IN_LOWLATENCY, PIPE_TYPE_IN_LOWLATENCY}] = CONCEDE_INCOMING;
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_ULTRA_FAST, PIPE_TYPE_IN_ULTRA_FAST), PLAY_BOTH);
    manager.concurrencyConfigMap_[{PIPE_TYPE_IN_ULTRA_FAST, PIPE_TYPE_IN_VOIP}] = CONCEDE_EXISTING;
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_ULTRA_FAST, PIPE_TYPE_IN_VOIP), CONCEDE_EXISTING);
    EXPECT_EQ(manager.GetConcurrencyAction(PIPE_TYPE_IN_ULTRA_FAST, PIPE_TYPE_OUT_NORMAL), PLAY_BOTH);
}

} // namespace AudioStandard
} // namespace OHOS
