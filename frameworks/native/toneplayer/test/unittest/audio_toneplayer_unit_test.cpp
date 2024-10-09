/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "tone_player_impl.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
constexpr uint32_t SLEEP_TIME = 30000;
static size_t g_reqBufLen = 0;
class AudioToneplayerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioToneplayerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioToneplayerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioToneplayerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioToneplayerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

class AudioRendererCallbackTest : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}
};

class AudioToneplayerCallbackTest : public AudioRendererCallback {
public:
    void OnWriteData(size_t length);
};

void AudioToneplayerCallbackTest::OnWriteData(size_t length)
{
    g_reqBufLen = length;
}

/**
 * @tc.name  : Test Create API
 * @tc.type  : FUNC
 * @tc.number: Create_001
 * @tc.desc  : Test Create interface.
 */
HWTEST(AudioToneplayerUnitTest, Create_001, TestSize.Level1)
{
    std::shared_ptr<TonePlayer> toneplayer = nullptr;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);
    
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->Release();
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_001
 * @tc.desc  : Test create->LoadTone->StartTone->StopTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_1);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    usleep(SLEEP_TIME); // 30ms sleep time
    bool stopRet = toneplayer->StopTone();
    EXPECT_EQ(true, stopRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_002
 * @tc.desc  : Test create->StartTone->StopTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_002, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_2);
    EXPECT_EQ(true, ret);

    usleep(SLEEP_TIME); // 30ms sleep time
    bool stopRet = toneplayer->StopTone();
    EXPECT_EQ(false, stopRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_003
 * @tc.desc  : Test create->StartTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_003, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(false, startRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_004
 * @tc.desc  : Test create->LoadTone->StartTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_004, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    usleep(SLEEP_TIME); // 30ms sleep time

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}
} // namespace AudioStandard
} // namespace OHOS