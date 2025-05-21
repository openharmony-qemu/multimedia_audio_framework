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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "capturer_clock_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t MOCK_SAMPLE_RATE = 48'000;

class AudioClockManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/**
 * @tc.name   : Test Audio clock manager
 * @tc.number : AudioClockManagerUnitTest_001
 * @tc.desc   : Test Audio clock manager normal case
 */
HWTEST_F(AudioClockManagerUnitTest, AudioClockManagerUnitTest_001, TestSize.Level1)
{
    EXPECT_EQ(CapturerClockManager::GetInstance().capturerClockPool_.size(), 0);
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);

    EXPECT_TRUE(CapturerClockManager::GetInstance().GetCapturerClock(1) != nullptr);
    EXPECT_TRUE(CapturerClockManager::GetInstance().GetCapturerClock(0) == nullptr);
    EXPECT_EQ(CapturerClockManager::GetInstance().capturerClockPool_.size(), 1);

    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
    EXPECT_EQ(CapturerClockManager::GetInstance().capturerClockPool_.size(), 0);
}

/**
 * @tc.name   : Test Audio clock manager
 * @tc.number : AudioClockManagerUnitTest_002
 * @tc.desc   : Test Audio clock manager normal case
 */
HWTEST_F(AudioClockManagerUnitTest, AudioClockManagerUnitTest_002, TestSize.Level1)
{
    shared_ptr<AudioSourceClock> srcClock = make_shared<AudioSourceClock>();
    EXPECT_EQ(CapturerClockManager::GetInstance().audioSrcClockPool_.size(), 0);

    EXPECT_TRUE(CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock));
    EXPECT_FALSE(CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock));
    EXPECT_EQ(CapturerClockManager::GetInstance().audioSrcClockPool_.size(), 1);

    EXPECT_TRUE(CapturerClockManager::GetInstance().GetAudioSourceClock(1) != nullptr);
    EXPECT_TRUE(CapturerClockManager::GetInstance().GetAudioSourceClock(0) == nullptr);
    EXPECT_EQ(CapturerClockManager::GetInstance().audioSrcClockPool_.size(), 1);

    CapturerClockManager::GetInstance().DeleteAudioSourceClock(1);
    EXPECT_EQ(CapturerClockManager::GetInstance().audioSrcClockPool_.size(), 0);
}

} // namespace AudioStandard
} // namespace OHOS
