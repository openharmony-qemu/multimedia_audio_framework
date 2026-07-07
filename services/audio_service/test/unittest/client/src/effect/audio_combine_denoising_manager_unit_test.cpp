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

#include <gtest/gtest.h>

#include "audio_combine_denoising_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioCombineDenoisingManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioCombineDenoisingManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest SetUpTestCase");
}

void AudioCombineDenoisingManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest TearDownTestCase");
}

void AudioCombineDenoisingManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest SetUp");
}

void AudioCombineDenoisingManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest TearDown");
}

/**
 * @tc.name   : Test RegisterNnStateEventListener API
 * @tc.type  : FUNC
 * @tc.number: RegisterNnStateEventListener_001
 * @tc.desc  : Test RegisterNnStateEventListener with null callback.
 */
HWTEST_F(AudioCombineDenoisingManagerUnitTest, RegisterNnStateEventListener_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest RegisterNnStateEventListener_001 start");
    int32_t result = AudioCombineDenoisingManager::GetInstance()->RegisterNnStateEventListener(nullptr);
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest RegisterNnStateEventListener_001 result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test UnregisterNnStateEventListener API
 * @tc.type  : FUNC
 * @tc.number: UnregisterNnStateEventListener_001
 * @tc.desc  : Test UnregisterNnStateEventListener interface.
 */
HWTEST_F(AudioCombineDenoisingManagerUnitTest, UnregisterNnStateEventListener_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest UnregisterNnStateEventListener_001 start");
    int32_t result = AudioCombineDenoisingManager::GetInstance()->UnregisterNnStateEventListener();
    AUDIO_INFO_LOG("AudioCombineDenoisingManagerUnitTest UnregisterNnStateEventListener_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::GetInstance()
    // Just check function doesn't crash
    (void)result;
}

} // namespace AudioStandard
} // namespace OHOS
