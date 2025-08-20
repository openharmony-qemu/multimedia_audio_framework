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

#include <gtest/gtest.h>
#include "tone_player_impl.h"
#include "audio_renderer_private.h"
#include "audio_errors.h"
#include "parameter.h"

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

#ifdef TEMP_DISABLE
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
#endif

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

#ifdef TEMP_DISABLE
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
#endif

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

#ifdef TEMP_DISABLE
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
#endif

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_001
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(nullptr, toneplayer);

    toneplayer->initialToneInfo_ = nullptr;
    auto ret = toneplayer->InitToneWaveInfo();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_002
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_002, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(nullptr, toneplayer);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
    toneplayer->isRendererInited_ = false;
    auto ret = toneplayer->StartTone();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_003
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_003, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(nullptr, toneplayer);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
    toneplayer->isRendererInited_ = true;
    toneplayer->audioRenderer_ = nullptr;

    auto ret = toneplayer->StartTone();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_004
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_004, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
    toneplayer->isRendererInited_ = true;
    toneplayer->currSegment_ = 0;

    toneplayer->InitAudioRenderer();
    EXPECT_NE(toneplayer->audioRenderer_, nullptr);

    auto ret = toneplayer->StartTone();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_005
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_005, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
    toneplayer->audioRenderer_ = nullptr;

    size_t length = 0;

    toneplayer->OnWriteData(length);
    EXPECT_NE(toneplayer, nullptr);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_006
 * @tc.desc  : Test TonePlayerImpl interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_006, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);
    toneplayer->isRendererInited_ = true;
    toneplayer->currSegment_ = 0;
    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
    toneplayer->InitAudioRenderer();
    EXPECT_NE(toneplayer->audioRenderer_, nullptr);
    size_t length = 0;

    toneplayer->OnWriteData(length);
    EXPECT_NE(toneplayer, nullptr);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_007
 * @tc.desc  : Test ~TonePlayerImpl interface. audioRenderer_ != nullptr
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_007, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    {
        std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
        EXPECT_NE(toneplayer, nullptr);
        toneplayer->isRendererInited_ = true;
        toneplayer->currSegment_ = 0;

        toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
        EXPECT_NE(toneplayer->toneInfo_, nullptr);

        toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;
        toneplayer->InitAudioRenderer();
        EXPECT_NE(toneplayer->audioRenderer_, nullptr);
    }
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_008
 * @tc.desc  : Test GetCurrentSegmentUpdated interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_008, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->currSegment_ = 0;
    toneplayer->loopCounter_ = 0;
    toneplayer->toneInfo_->segments[0].loopCnt = 1;
    toneplayer->toneInfo_->segments[0].loopIndx = 1;

    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(toneplayer->currSegment_, 1);
    EXPECT_EQ(toneplayer->loopCounter_, 1);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_009
 * @tc.desc  : Test GetCurrentSegmentUpdated interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_009, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->currSegment_ = 1;
    toneplayer->loopCounter_ = 1;
    toneplayer->toneInfo_->segments[1].loopCnt = 3;
    toneplayer->toneInfo_->segments[1].loopIndx = 3;

    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(toneplayer->currSegment_, 3);
    EXPECT_EQ(toneplayer->loopCounter_, 2);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_010
 * @tc.desc  : Test GetCurrentSegmentUpdated interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_010, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->currSegment_ = 0;
    toneplayer->loopCounter_ = 3;
    toneplayer->toneInfo_->segments[0].loopCnt = 1;
    toneplayer->toneInfo_->segments[0].loopIndx = 1;

    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(toneplayer->currSegment_, 1);
    EXPECT_EQ(toneplayer->loopCounter_, 0);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_011
 * @tc.desc  : Test GetCurrentSegmentUpdated interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_011, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->currSegment_ = 1;
    toneplayer->loopCounter_ = 1;
    toneplayer->toneInfo_->segments[1].loopCnt = 1;
    toneplayer->toneInfo_->segments[1].loopIndx = 1;

    toneplayer->GetCurrentSegmentUpdated();
    EXPECT_EQ(toneplayer->currSegment_, 2);
    EXPECT_EQ(toneplayer->loopCounter_, 0);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_012
 * @tc.desc  : Test CheckToneContinuity interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_012, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->currSegment_ = 1;
    toneplayer->loopCounter_ = 1;
    toneplayer->toneInfo_->segments[1].loopCnt = 1;
    toneplayer->toneInfo_->segments[1].loopIndx = 1;
    toneplayer->toneInfo_->segments[2].duration = 1;

    bool ret = toneplayer->CheckToneContinuity();
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->currSegment_, 2);
    EXPECT_EQ(toneplayer->loopCounter_, 0);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_013
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_013, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STOPPED;
    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, false);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_014
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_014, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_RUNNING;
    toneplayer->totalSample_ = 8000;
    toneplayer->nextSegSample_ = 8000;
    toneplayer->currSegment_ = 1;
    toneplayer->sampleCount_ = 1;
    toneplayer->needFadeOut_ = false;
    toneplayer->toneInfo_->segments[1].duration = 1;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->needFadeOut_, true);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_015
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_015, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_RUNNING;
    toneplayer->totalSample_ = 8000;
    toneplayer->nextSegSample_ = 8000;
    toneplayer->currSegment_ = 0;
    toneplayer->sampleCount_ = 1;
    toneplayer->needFadeOut_ = false;
    toneplayer->toneInfo_->segments[0].duration = 0;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->needFadeOut_, true);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_016
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_016, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_RUNNING;
    toneplayer->totalSample_ = 8000;
    toneplayer->nextSegSample_ = 8000;
    toneplayer->currSegment_ = 1;
    toneplayer->sampleCount_ = 1;
    toneplayer->needFadeOut_ = false;
    toneplayer->toneInfo_->segments[1].duration = 0;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->needFadeOut_, true);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_017
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_017, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_RUNNING;
    toneplayer->totalSample_ = 16000;
    toneplayer->nextSegSample_ = 8000;
    toneplayer->currSegment_ = 0;
    toneplayer->currCount_ = 1;
    toneplayer->sampleCount_ = 1;
    toneplayer->toneInfo_->repeatCnt = 2;
    toneplayer->toneInfo_->repeatSegment = 0;
    toneplayer->toneInfo_->segments[1].duration = 0;
    toneplayer->toneInfo_->segments[0].duration = 1;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->sampleCount_, 0);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_018
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_018, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_RUNNING;
    toneplayer->totalSample_ = 16000;
    toneplayer->nextSegSample_ = 8000;
    toneplayer->currSegment_ = 0;
    toneplayer->currCount_ = 1;
    toneplayer->sampleCount_ = 1;
    toneplayer->toneInfo_->repeatCnt = 2;
    toneplayer->toneInfo_->repeatSegment = 2;
    toneplayer->toneInfo_->segments[1].duration = 0;
    toneplayer->toneInfo_->segments[2].duration = 1;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->sampleCount_, 0);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_019
 * @tc.desc  : Test GetCountryCode interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_019, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    SetParameter("debug.toneplayer.country", "TEST");


    std::string ret = toneplayer->GetCountryCode();
    EXPECT_EQ(ret, "test");
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_020
 * @tc.desc  : Test CheckToneStarted interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_020, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STARTING;
    toneplayer->currSegment_ = 0;
    toneplayer->sampleCount_ = 1;
    toneplayer->toneInfo_->segments[0].duration = 1;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->CheckToneStarted(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->toneState_, TonePlayerImpl::TONE_RUNNING);
    EXPECT_EQ(toneplayer->sampleCount_, 0);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_021
 * @tc.desc  : Test CheckToneStarted interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_021, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STARTING;
    toneplayer->currSegment_ = 1;
    toneplayer->sampleCount_ = 1;
    toneplayer->toneInfo_->segments[0].duration = 0;

    uint32_t reqSample = 0;
    const size_t bufferSize = 1024;
    int8_t* audioBuffer = new int8_t[bufferSize];

    bool ret = toneplayer->CheckToneStarted(reqSample, audioBuffer);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(toneplayer->toneState_, TonePlayerImpl::TONE_RUNNING);
    EXPECT_EQ(toneplayer->sampleCount_, 1);

    delete[] audioBuffer;
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_022
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_022, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STARTING;
    toneplayer->currSegment_ = 0;
    toneplayer->totalSample_ = 8000;
    toneplayer->maxSample_ = 8000;
    toneplayer->toneInfo_->segments[0].duration = 1;

    bool ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_023
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_023, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STOPPING;
    toneplayer->currSegment_ = 0;
    toneplayer->totalSample_ = 8000;
    toneplayer->maxSample_ = 8000;
    toneplayer->toneInfo_->segments[1].duration = 1;

    bool ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_024
 * @tc.desc  : Test AudioToneSequenceGen interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_024, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_INIT;

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);
    toneplayer->currSegment_ = 1;
    toneplayer->totalSample_ = 8000;
    toneplayer->maxSample_ = 8000;
    toneplayer->toneInfo_->segments[1].duration = 1;
    toneplayer->processSize_ = 100;

    BufferDesc bufDesc;
    const size_t bufferSize = sizeof(int16_t);
    bufDesc.buffer = new uint8_t[bufferSize];
    
    bool ret = toneplayer->AudioToneSequenceGen(bufDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_025
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_025, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STARTING;
    toneplayer->currSegment_ = 0;
    toneplayer->totalSample_ = 9000;
    toneplayer->maxSample_ = 8000;
    toneplayer->toneInfo_->segments[1].duration = 1;

    bool ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test TonePlayerImpl API
 * @tc.type  : FUNC
 * @tc.number: TonePlayerImpl_026
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, TonePlayerImpl_026, TestSize.Level4)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<TonePlayerImpl> toneplayer = std::make_shared<TonePlayerImpl>("", rendererInfo);
    EXPECT_NE(toneplayer, nullptr);

    toneplayer->toneInfo_ = std::make_shared<ToneInfo>();
    EXPECT_NE(toneplayer->toneInfo_, nullptr);

    toneplayer->toneState_ = TonePlayerImpl::TONE_STARTING;
    toneplayer->currSegment_ = 0;
    toneplayer->totalSample_ = 8000;
    toneplayer->maxSample_ = 8000;
    toneplayer->toneInfo_->segments[1].duration = 0;

    bool ret = toneplayer->CheckToneStopped();
    EXPECT_EQ(ret, true);
}
} // namespace AudioStandard
} // namespace OHOS
