/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "playback_capturer_adapter.h"
#include "playback_capturer_manager.h"
#include "audio_info.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
static PlaybackCapturerManager *playbackCapturerMgr_ = PlaybackCapturerManager::GetInstance();

class PlaybackPlaybackCapturerManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PlaybackPlaybackCapturerManagerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void PlaybackPlaybackCapturerManagerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void PlaybackPlaybackCapturerManagerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void PlaybackPlaybackCapturerManagerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test IsPrivacySupportInnerCapturer API via legal state
 * @tc.type  : FUNC
 * @tc.number: IsPrivacySupportInnerCapturer_001
 * @tc.desc  : Test IsPrivacySupportInnerCapturer interface. Is privacy support innter capturer and return ret.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, IsPrivacySupportInnerCapturer_001, TestSize.Level1)
{
    bool ret = IsPrivacySupportInnerCapturer(PRIVACY_TYPE_PUBLIC);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test IsPrivacySupportInnerCapturer API via legal state
 * @tc.type  : FUNC
 * @tc.number: IsPrivacySupportInnerCapturer_002
 * @tc.desc  : Test IsPrivacySupportInnerCapturer interface. Is privacy support innter capturer and return ret.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, IsPrivacySupportInnerCapturer_002, TestSize.Level1)
{
    bool ret = IsPrivacySupportInnerCapturer(PRIVACY_TYPE_PRIVATE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test SetInnerCapturerState API via legal and illegal state
 * @tc.type  : FUNC
 * @tc.number: SetInnerCapturerState_001
 * @tc.desc  : Test SetInnerCapturerState interface. Set inner capturer state.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, SetInnerCapturerState_001, TestSize.Level1)
{
    SetInnerCapturerState(true);
    bool ret = GetInnerCapturerState();
    EXPECT_TRUE(ret);
    SetInnerCapturerState(false);
    ret = GetInnerCapturerState();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test RegisterCapturerFilterListener API via legal and illegal state
 * @tc.type  : FUNC
 * @tc.number: RegisterCapturerFilterListener_001
 * @tc.desc  : Test RegisterCapturerFilterListener interface
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, RegisterCapturerFilterListener_001, TestSize.Level1)
{
    bool ret = playbackCapturerMgr_->RegisterCapturerFilterListener(nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: CheckCaptureLimit_001
 * @tc.desc  : Test CheckCaptureLimit interface
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, CheckCaptureLimit_001, TestSize.Level1)
{
    AudioPlaybackCaptureConfig config;
    int32_t innerCapId = 0;
    int32_t ret = playbackCapturerMgr_->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
    ret = playbackCapturerMgr_->SetInnerCapLimit(++innerCapId);
    EXPECT_EQ(ret, SUCCESS);
    ret = playbackCapturerMgr_->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
    config.filterOptions.usages.push_back(STREAM_USAGE_MUSIC);
    ret = playbackCapturerMgr_->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
    playbackCapturerMgr_->CheckReleaseUnloadModernInnerCapSink(innerCapId--);
    playbackCapturerMgr_->CheckReleaseUnloadModernInnerCapSink(innerCapId);
    bool checkRet = playbackCapturerMgr_->CheckReleaseUnloadModernInnerCapSink(1);
    EXPECT_TRUE(checkRet);

    innerCapId = 100;
    ret = playbackCapturerMgr_->CheckCaptureLimit(config, innerCapId);
    EXPECT_EQ(ret, SUCCESS);
    playbackCapturerMgr_->CheckReleaseUnloadModernOffloadCapSource();
    playbackCapturerMgr_->CheckCaptureLimit(config, innerCapId);
    bool checkOffloadRet = playbackCapturerMgr_->CheckReleaseUnloadModernOffloadCapSource();
    EXPECT_FALSE(checkOffloadRet);
}
}
}