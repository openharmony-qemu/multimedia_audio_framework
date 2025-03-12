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
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    usleep(SLEEP_TIME); // 30ms sleep time

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_005
 * @tc.desc  : Test create->LoadTone->StartTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_005, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    bool stopRet = toneplayer->StopTone();
    EXPECT_EQ(true, stopRet);

    bool checkRet = toneplayer->CheckToneStopped();
    EXPECT_EQ(true, checkRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_006
 * @tc.desc  : Test create->StartTone->StopTone->LoadTone->LoadTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_006, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->StartTone();
    EXPECT_EQ(false, ret);

    ret = toneplayer->StopTone();
    EXPECT_EQ(false, ret);

    ret = toneplayer->LoadTone(NUM_TONES);
    ret = toneplayer->LoadTone(NUM_TONES);
    EXPECT_EQ(false, ret);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_StartTone_001
 * @tc.desc  : Test StartTone interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_StartTone_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->isRendererInited_ = true;
    bool ret = toneplayer->StartTone();
    ret = toneplayer->StopTone();

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_StartTone_002
 * @tc.desc  : Test StartTone interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_StartTone_002, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->isRendererInited_ = true;
    toneplayer->audioRenderer_ = nullptr;
    toneplayer->StartTone();
    toneplayer->StopTone();

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_StartTone_003
 * @tc.desc  : Test StartTone interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_StartTone_003, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->isRendererInited_ = false;
    toneplayer->StartTone();
    toneplayer->StopTone();

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_StartTone_004
 * @tc.desc  : Test StartTone interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_StartTone_004, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->isRendererInited_ = false;
    toneplayer->audioRenderer_ = nullptr;
    toneplayer->StartTone();
    toneplayer->StopTone();

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test Create API
 * @tc.type  : FUNC
 * @tc.number: Create_002
 * @tc.desc  : Test Create interface.
 */
HWTEST(AudioToneplayerUnitTest, Create_002, TestSize.Level1)
{
    std::string cachePath = "/data/local/tmp";
    std::shared_ptr<TonePlayer> toneplayer = nullptr;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    toneplayer = TonePlayer::Create(cachePath, rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->Release();
}

/**
 * @tc.name  : Test GetCurrentSegmentUpdated API
 * @tc.type  : FUNC
 * @tc.number: GetCurrentSegmentUpdated_001
 * @tc.desc  : Test GetCurrentSegmentUpdated interface.
 */
HWTEST(AudioToneplayerUnitTest, GetCurrentSegmentUpdated_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].loopCnt = 0;
    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(1, toneplayer->currSegment_);

    toneplayer->loopCounter_ = 0;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].loopCnt = 1;
    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(1, toneplayer->loopCounter_);

    toneplayer->loopCounter_ = 2;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].loopCnt = 1;
    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(0, toneplayer->loopCounter_);
}

/**
 * @tc.name  : Test CheckToneContinuity API
 * @tc.type  : FUNC
 * @tc.number: CheckToneContinuity_001
 * @tc.desc  : Test CheckToneContinuity interface.
 */
HWTEST(AudioToneplayerUnitTest, CheckToneContinuity_001, TestSize.Level1)
{
    bool ret = false;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    ret = toneplayer->CheckToneContinuity();
    EXPECT_EQ(true, ret);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 0;
    toneplayer->currCount_ = 0;
    toneplayer->toneInfo_->repeatCnt = 1;
    ret = toneplayer->CheckToneContinuity();
    EXPECT_EQ(true, ret);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 0;
    toneplayer->currCount_ = 1;
    toneplayer->toneInfo_->repeatCnt = 0;
    ret = toneplayer->CheckToneContinuity();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test OnInterrupt API
 * @tc.type  : FUNC
 * @tc.number: OnInterrupt_001
 * @tc.desc  : Test OnInterrupt interface.
 */
HWTEST(AudioToneplayerUnitTest, OnInterrupt_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    InterruptEvent interruptEvent;
    interruptEvent.eventType = InterruptType::INTERRUPT_TYPE_END;
    toneplayer->OnInterrupt(interruptEvent);
    EXPECT_EQ(ToneType::NUM_TONES, toneplayer->toneType_);
}

/**
 * @tc.name  : Test ContinueToneplay API
 * @tc.type  : FUNC
 * @tc.number: ContinueToneplay_001
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, ContinueToneplay_001, TestSize.Level1)
{
    bool ret = false;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);
    uint32_t reqSample = 4;
    int8_t buff[8] = {0};
    int8_t *audioBuffer = buff;

    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_STOPPED;
    ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, false);

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    toneplayer->totalSample_ = 2;
    toneplayer->nextSegSample_ = 1;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    ret = toneplayer->CheckToneContinuity();
    EXPECT_EQ(true, ret);
    ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);

    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 0;
    ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test ContinueToneplay API
 * @tc.type  : FUNC
 * @tc.number: ContinueToneplay_002
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, ContinueToneplay_002, TestSize.Level1)
{
    bool ret = false;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);
    uint32_t reqSample = 4;
    int8_t buff[8] = {0};
    int8_t *audioBuffer = buff;

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    toneplayer->totalSample_ = 0;
    toneplayer->nextSegSample_ = 1;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 0;
    ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test ContinueToneplay API
 * @tc.type  : FUNC
 * @tc.number: ContinueToneplay_003
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, ContinueToneplay_003, TestSize.Level1)
{
    bool ret = false;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);
    uint32_t reqSample = 4;
    int8_t buff[8] = {0};
    int8_t *audioBuffer = buff;

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    toneplayer->totalSample_ = 0;
    toneplayer->nextSegSample_ = 1;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test CheckToneStopped API
 * @tc.type  : FUNC
 * @tc.number: CheckToneStopped_001
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, CheckToneStopped_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_STOPPED;
    bool ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(true, ret);

    toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_NE(nullptr, toneplayer->toneInfo_);
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 0;
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(true, ret);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    toneplayer->totalSample_ = 3;
    toneplayer->maxSample_ = 2;
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_RUNNING;
    ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(true, ret);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    toneplayer->totalSample_ = 1;
    toneplayer->maxSample_ = 2;
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_STOPPING;
    ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(true, ret);

    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;
    toneplayer->totalSample_ = 1;
    toneplayer->maxSample_ = 2;
    toneplayer->toneState_ = TonePlayerImpl::ToneState::TONE_IDLE;
    ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(false, ret);
}
} // namespace AudioStandard
} // namespace OHOS
