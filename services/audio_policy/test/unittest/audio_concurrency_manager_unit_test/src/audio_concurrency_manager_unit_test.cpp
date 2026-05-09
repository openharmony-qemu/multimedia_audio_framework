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
} // namespace AudioStandard
} // namespace OHOS
