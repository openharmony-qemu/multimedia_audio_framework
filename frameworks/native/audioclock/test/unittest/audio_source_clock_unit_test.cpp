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
#include "audio_source_clock.h"
#include "capturer_clock_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t MOCK_POSITION_INC = 960 * 2 * 2;
constexpr uint32_t MOCK_SAMPLE_RATE = 48'000;
constexpr AudioSampleFormat MOCK_FORMAT = AudioSampleFormat::SAMPLE_S16LE;
constexpr uint32_t MOCK_CHANNEL = 2;

class AudioSourceClockUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}
private:
    static std::shared_ptr<AudioSourceClock> srcClock_;
    static std::shared_ptr<CapturerClock> capturerClock_;
};

std::shared_ptr<AudioSourceClock> AudioSourceClockUnitTest::srcClock_ = nullptr;
std::shared_ptr<CapturerClock> AudioSourceClockUnitTest::capturerClock_ = nullptr;

void AudioSourceClockUnitTest::SetUpTestCase()
{
    srcClock_ = std::make_shared<AudioSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock_);
    srcClock_->Init(MOCK_SAMPLE_RATE, MOCK_FORMAT, MOCK_CHANNEL);

    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    capturerClock_ = CapturerClockManager::GetInstance().GetCapturerClock(1);
    capturerClock_->Start();
}

void AudioSourceClockUnitTest::TearDownTestCase()
{
    srcClock_ = nullptr;
    capturerClock_ = nullptr;
    CapturerClockManager::GetInstance().DeleteAudioSourceClock(1);
    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
}

/**
 * @tc.name   : Test Audio source clock
 * @tc.number : AudioSourceClockUnitTest_001
 * @tc.desc   : Test Audio source clock normal case
 */
HWTEST_F(AudioSourceClockUnitTest, AudioSourceClockUnitTest_001, TestSize.Level1)
{
    EXPECT_EQ(srcClock_->sizePerPos_, 4);   // 4:data size

    srcClock_->logTimestamp_ = 0;
    srcClock_->Renew(MOCK_POSITION_INC);
    EXPECT_TRUE(capturerClock_->timestamp_ == 0);

    std::vector<int32_t> sessionIdList = { 1 };
    srcClock_->UpdateSessionId(sessionIdList);
    EXPECT_EQ(srcClock_->sessionIdList_.size(), 1);

    srcClock_->Renew(MOCK_POSITION_INC);
    EXPECT_TRUE(capturerClock_->timestamp_ > 0);
}

/**
 * @tc.name   : Test Init
 * @tc.number : Init_001
 * @tc.desc   : Test Init
 */
HWTEST_F(AudioSourceClockUnitTest, Init_001, TestSize.Level1)
{
    AudioSampleFormat format = AudioSampleFormat::SAMPLE_U8;
    srcClock_->Init(MOCK_SAMPLE_RATE, format, MOCK_CHANNEL);
    EXPECT_EQ(srcClock_->sizePerPos_, 2);
}

/**
 * @tc.name   : Test Init
 * @tc.number : Init_002
 * @tc.desc   : Test Init
 */
HWTEST_F(AudioSourceClockUnitTest, Init_002, TestSize.Level1)
{
    AudioSampleFormat format = AudioSampleFormat::SAMPLE_S24LE;
    srcClock_->Init(MOCK_SAMPLE_RATE, format, MOCK_CHANNEL);
    EXPECT_EQ(srcClock_->sizePerPos_, 6);
}

/**
 * @tc.name   : Test Init
 * @tc.number : Init_003
 * @tc.desc   : Test Init
 */
HWTEST_F(AudioSourceClockUnitTest, Init_003, TestSize.Level1)
{
    AudioSampleFormat format = AudioSampleFormat::SAMPLE_S32LE;
    srcClock_->Init(MOCK_SAMPLE_RATE, format, MOCK_CHANNEL);
    EXPECT_EQ(srcClock_->sizePerPos_, 8);
}

/**
 * @tc.name   : Test Init
 * @tc.number : Init_004
 * @tc.desc   : Test Init
 */
HWTEST_F(AudioSourceClockUnitTest, Init_004, TestSize.Level1)
{
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    srcClock_->Init(MOCK_SAMPLE_RATE, format, MOCK_CHANNEL);
    EXPECT_EQ(srcClock_->sizePerPos_, 4);
}

} // namespace AudioStandard
} // namespace OHOS
