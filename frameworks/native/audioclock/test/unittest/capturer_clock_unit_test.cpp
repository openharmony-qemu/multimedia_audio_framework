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
#include "capturer_clock.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t MOCK_POSITION_INC = 960;
constexpr uint32_t MOCK_SAMPLE_RATE = 48'000;
constexpr uint32_t MOCK_SAMPLE_RATE_2 = 96'000;

constexpr uint64_t MOCK_POSITION_1 = 0;
constexpr uint64_t MOCK_POSITION_2 = 960;
constexpr uint64_t MOCK_POSITION_3 = 1920;
constexpr uint64_t MOCK_POSITION_4 = 2880;
constexpr uint64_t MOCK_POSITION_5 = 3840;
constexpr uint64_t MOCK_TIMESTAMP_1 = 1'000'000'000;
constexpr uint64_t MOCK_TIMESTAMP_2 = 1'020'000'000;
constexpr uint64_t MOCK_TIMESTAMP_3 = 1'040'000'000;
constexpr uint64_t MOCK_TIMESTAMP_4 = 1'100'000'000;
constexpr uint64_t MOCK_TIMESTAMP_4_IN_CAPTURER = 1'120'000'000;
constexpr uint64_t MOCK_TIMESTAMP_5 = 1'120'000'000;
constexpr uint64_t MOCK_TIMESTAMP_5_IN_CAPTURER = 1'140'000'000;

class CapturerClockUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}
private:
    static std::shared_ptr<CapturerClock> capturerClock_;
};

std::shared_ptr<CapturerClock> CapturerClockUnitTest::capturerClock_ = nullptr;

void CapturerClockUnitTest::SetUpTestCase()
{
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    capturerClock_ = CapturerClockManager::GetInstance().GetCapturerClock(1);
}

void CapturerClockUnitTest::TearDownTestCase()
{
    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
}

/**
 * @tc.name   : Test capturer clock
 * @tc.number : CapturerClockUnitTest_001
 * @tc.desc   : Test capturer clock normal case
 */
HWTEST_F(CapturerClockUnitTest, CapturerClockUnitTest_001, TestSize.Level1)
{
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_1, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);
    EXPECT_EQ(capturerClock_->timestamp_, 0);

    capturerClock_->Start();
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_1, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_2, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);
    uint64_t timestamp;
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_1, timestamp);
    EXPECT_EQ(timestamp, MOCK_TIMESTAMP_1);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_2, timestamp);
    EXPECT_EQ(timestamp, MOCK_TIMESTAMP_2);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_3, timestamp);
    EXPECT_EQ(timestamp, MOCK_TIMESTAMP_3);
    capturerClock_->Stop();
}

/**
 * @tc.name   : Test capturer clock
 * @tc.number : CapturerClockUnitTest_002
 * @tc.desc   : Test capturer clock in different period
 */
HWTEST_F(CapturerClockUnitTest, CapturerClockUnitTest_002, TestSize.Level1)
{
    capturerClock_->Start();

    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_4, MOCK_SAMPLE_RATE_2, MOCK_POSITION_INC * 2);
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_5, MOCK_SAMPLE_RATE_2, MOCK_POSITION_INC * 2);

    uint64_t timestamp;
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_4, timestamp);
    EXPECT_EQ(timestamp, MOCK_TIMESTAMP_4_IN_CAPTURER);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_5, timestamp);
    EXPECT_EQ(timestamp, MOCK_TIMESTAMP_5_IN_CAPTURER);
}

} // namespace AudioStandard
} // namespace OHOS
