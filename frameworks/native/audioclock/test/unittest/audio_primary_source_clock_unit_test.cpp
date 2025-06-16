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
#include "audio_primary_source_clock.h"
#include "capturer_clock_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t MOCK_FIRST_TIMESTAMP = 1000'000;
constexpr uint64_t MOCK_POSITION_INC = 960 * 2 * 2;
constexpr size_t REGULAR_TIME_DETLA = 20'000;
constexpr uint32_t MOCK_SAMPLE_RATE = 48'000;
constexpr AudioSampleFormat MOCK_FORMAT = AudioSampleFormat::SAMPLE_S16LE;
constexpr uint32_t MOCK_CHANNEL = 2;

class AudioPrimarySourceClockUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown() {}
private:
    static std::shared_ptr<AudioCapturerSourceClock> srcClock_;
};

std::shared_ptr<AudioCapturerSourceClock> AudioPrimarySourceClockUnitTest::srcClock_ = nullptr;

void AudioPrimarySourceClockUnitTest::SetUpTestCase()
{
    srcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock_);
    srcClock_->Init(MOCK_SAMPLE_RATE, MOCK_FORMAT, MOCK_CHANNEL);
}

void AudioPrimarySourceClockUnitTest::TearDownTestCase()
{
    CapturerClockManager::GetInstance().DeleteAudioSourceClock(1);
}

void AudioPrimarySourceClockUnitTest::SetUp()
{
    if (srcClock_ == nullptr) {
        return;
    }
    srcClock_->Reset();
}

/**
 * @tc.name   : Test Audio primary source clock
 * @tc.number : AudioPrimarySourceClockUnitTest_001
 * @tc.desc   : Test Audio primary source clock normal case
 */
HWTEST_F(AudioPrimarySourceClockUnitTest, AudioPrimarySourceClockUnitTest_001, TestSize.Level1)
{
    EXPECT_EQ(srcClock_->GetFrameCnt(), 0);
    srcClock_->SetFirstTimestampFromHdi(MOCK_FIRST_TIMESTAMP);
    srcClock_->Renew(0);
    EXPECT_EQ(srcClock_->lastTs_, 0);
    EXPECT_EQ(srcClock_->firstTimeStamp_, MOCK_FIRST_TIMESTAMP);
    srcClock_->Renew(MOCK_POSITION_INC);
    EXPECT_EQ(srcClock_->GetFrameCnt(), 1);
    EXPECT_TRUE(srcClock_->lastTs_ > 0);
}

/**
 * @tc.name   : Test Audio primary source clock
 * @tc.number : AudioPrimarySourceClockUnitTest_002
 * @tc.desc   : Test Audio primary source clock normal case 2
 */
HWTEST_F(AudioPrimarySourceClockUnitTest, AudioPrimarySourceClockUnitTest_002, TestSize.Level1)
{
    EXPECT_EQ(srcClock_->GetFrameCnt(), 0);
    srcClock_->SetFirstTimestampFromHdi(MOCK_FIRST_TIMESTAMP);
    EXPECT_FALSE(srcClock_->isGetTimeStampFromSystemClock_);
    // 10 : loop cnt
    for (size_t i = 0; i < 10; i++) {
        srcClock_->Renew(MOCK_POSITION_INC);
    }
    EXPECT_FALSE(srcClock_->isGetTimeStampFromSystemClock_);
    // 20 : loop cnt
    for (size_t i = 0; i < 20; i++) {
        srcClock_->Renew(MOCK_POSITION_INC);
    }
    // 21: 20 frames from first ts and 1 for last one
    EXPECT_EQ(srcClock_->GetFrameCnt(), 21);
    EXPECT_TRUE(srcClock_->isGetTimeStampFromSystemClock_);
}

/**
 * @tc.name   : Test Audio primary source clock
 * @tc.number : AudioPrimarySourceClockUnitTest_003
 * @tc.desc   : Test get timestamp when first timestamp set 0
 */
HWTEST_F(AudioPrimarySourceClockUnitTest, AudioPrimarySourceClockUnitTest_003, TestSize.Level1)
{
    EXPECT_EQ(srcClock_->GetFrameCnt(), 0);
    srcClock_->SetFirstTimestampFromHdi(0);
    srcClock_->Renew(MOCK_POSITION_INC);
    EXPECT_TRUE(srcClock_->isGetTimeStampFromSystemClock_);
}

/**
 * @tc.name   : Test Audio primary source clock
 * @tc.number : AudioPrimarySourceClockUnitTest_004
 * @tc.desc   : Test Audio primary source clock normal case 4
 */
HWTEST_F(AudioPrimarySourceClockUnitTest, AudioPrimarySourceClockUnitTest_004, TestSize.Level1)
{
    EXPECT_EQ(srcClock_->GetFrameCnt(), 0);
    srcClock_->SetFirstTimestampFromHdi(MOCK_FIRST_TIMESTAMP);
    EXPECT_FALSE(srcClock_->isGetTimeStampFromSystemClock_);
    srcClock_->Renew(MOCK_POSITION_INC);
    usleep(REGULAR_TIME_DETLA);
    srcClock_->Renew(MOCK_POSITION_INC);
    EXPECT_TRUE(srcClock_->isGetTimeStampFromSystemClock_);
}

} // namespace AudioStandard
} // namespace OHOS
