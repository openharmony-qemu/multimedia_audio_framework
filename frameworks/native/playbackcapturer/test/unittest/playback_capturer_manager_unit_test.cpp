/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "playback_capturer_manager.h"
#include "audio_info.h"

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
 * @tc.name  : Test IsStreamSupportInnerCapturer API via legal state
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupportInnerCapturer_001
 * @tc.desc  : Test IsStreamSupportInnerCapturer interface. Is stream support inner capturer and return ret.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, IsStreamSupportInnerCapturer_001, TestSize.Level1)
{
    std::vector<int32_t> usage;
    usage.push_back(STREAM_USAGE_MEDIA);
    usage.push_back(STREAM_USAGE_MUSIC);
    usage.push_back(STREAM_USAGE_GAME);
    usage.push_back(STREAM_USAGE_MOVIE);
    usage.push_back(STREAM_USAGE_AUDIOBOOK);
    playbackCapturerMgr_->SetSupportStreamUsage(usage);
    for (int32_t usageItem : usage) {
        bool ret = IsStreamSupportInnerCapturer(usageItem);
        EXPECT_TRUE(ret);
    }
}

/**
 * @tc.name  : Test IsStreamSupportInnerCapturer API via illegal state
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupportInnerCapturer_002
 * @tc.desc  : Test IsStreamSupportInnerCapturer interface. Is stream support inner capturer and return ret.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, IsStreamSupportInnerCapturer_002, TestSize.Level1)
{
    std::vector<int32_t> usage;
    playbackCapturerMgr_->SetSupportStreamUsage(usage);
    bool ret = playbackCapturerMgr_->IsStreamSupportInnerCapturer(STREAM_USAGE_RANGING);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test IsStreamSupportInnerCapturer API via legal state
 * @tc.type  : FUNC
 * @tc.number: IsStreamSupportInnerCapturer_003
 * @tc.desc  : Test IsStreamSupportInnerCapturer interface. Is stream support inner capturer and return ret.
 */
HWTEST(PlaybackPlaybackCapturerManagerUnitTest, IsStreamSupportInnerCapturer_003, TestSize.Level1)
{
    std::vector<int32_t> usage;
    usage.push_back(STREAM_USAGE_MEDIA);
    usage.push_back(STREAM_USAGE_MUSIC);
    usage.push_back(STREAM_USAGE_GAME);
    usage.push_back(STREAM_USAGE_MOVIE);
    usage.push_back(STREAM_USAGE_AUDIOBOOK);
    playbackCapturerMgr_->SetSupportStreamUsage(usage);
    bool ret = playbackCapturerMgr_->IsStreamSupportInnerCapturer(STREAM_USAGE_MEDIA);
    EXPECT_TRUE(ret);
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

}
}