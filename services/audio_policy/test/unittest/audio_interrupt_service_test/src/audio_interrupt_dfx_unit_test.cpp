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

#include "audio_interrupt_dfx_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptDfxUnitTest::SetUpTestCase(void) {}
void AudioInterruptDfxUnitTest::TearDownTestCase(void) {}
void AudioInterruptDfxUnitTest::SetUp(void) {}
void AudioInterruptDfxUnitTest::TearDown(void) {}

/**
* @tc.name  : Test GetInterruptSessionInfo
* @tc.number: GetInterruptSessionInfo_001
* @tc.desc  : Test GetInterruptSessionInfo_001
*/
HWTEST_F(AudioInterruptDfxUnitTest, GetInterruptSessionInfo_001, TestSize.Level1)
{
    std::shared_ptr<AudioInterruptDfx> interruptDfx = std::make_shared<AudioInterruptDfx>();
    EXPECT_NE(interruptDfx, nullptr);

    uint32_t fakePid = 12345;
    AudioSessionService &sessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = fakePid;
    auto ret = interruptDfx->GetInterruptSessionInfo(audioInterrupt);
    EXPECT_EQ(ret, "");

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(0, strategy, audioSessionStateMonitor_);
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    sessionService.sessionMap_.clear();
    sessionService.sessionMap_[fakePid] = audioSession;
    ret = interruptDfx->GetInterruptSessionInfo(audioInterrupt);
    EXPECT_NE(ret, "");

    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    ret = interruptDfx->GetInterruptSessionInfo(audioInterrupt);
    EXPECT_NE(ret, "");
}

/**
* @tc.name  : Test RecordMediaInterruptEvent
* @tc.number: RecordMediaInterruptEvent_001
* @tc.desc  : Test RecordMediaInterruptEvent_001
*/
HWTEST_F(AudioInterruptDfxUnitTest, RecordMediaInterruptEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioInterruptDfx> interruptDfx = std::make_shared<AudioInterruptDfx>();
    EXPECT_NE(interruptDfx, nullptr);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.pid = 12345;
    AudioSessionService &sessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    sessionService.sessionMap_.clear();
    interruptDfx->RecordMediaInterruptEvent(interruptEvent, activeInterrupt, incomingInterrupt);
    auto ret = interruptDfx->audioInterruptCheckInfo_.size();
    EXPECT_EQ(ret, 1);
}

/**
* @tc.name  : Test NotifyStreamSilentChange
* @tc.number: NotifyStreamSilentChange_001
* @tc.desc  : Test NotifyStreamSilentChange_001
*/
HWTEST_F(AudioInterruptDfxUnitTest, NotifyStreamSilentChange_001, TestSize.Level1)
{
    std::shared_ptr<AudioInterruptDfx> interruptDfx = std::make_shared<AudioInterruptDfx>();
    EXPECT_NE(interruptDfx, nullptr);

    uint32_t streamId = 10001;
    interruptDfx->NotifyStreamSilentChange(streamId);
    auto ret = interruptDfx->audioInterruptCheckInfo_.size();
    EXPECT_EQ(ret, 0);

    AudioInterruptCheckInfo interruptCheckInfo;
    interruptCheckInfo.isMuteStream = true;
    interruptDfx->audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    interruptDfx->NotifyStreamSilentChange(streamId);
    EXPECT_FALSE(interruptDfx->audioInterruptCheckInfo_[streamId].isMuteStream);
}

/**
* @tc.name  : Test CheckMuteInterruptEvent
* @tc.number: CheckMuteInterruptEvent_001
* @tc.desc  : Test CheckMuteInterruptEvent_001
*/
HWTEST_F(AudioInterruptDfxUnitTest, CheckMuteInterruptEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioInterruptDfx> interruptDfx = std::make_shared<AudioInterruptDfx>();
    EXPECT_NE(interruptDfx, nullptr);

    uint32_t streamId = 10001;
    AudioInterruptCheckInfo interruptCheckInfo;
    AudioFocusErrorEvent errorEvent;
    interruptCheckInfo.errorEvents = {errorEvent};
    interruptDfx->audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    interruptDfx->CheckMuteInterruptEvent(streamId);
    auto ret = interruptDfx->audioInterruptCheckInfo_[streamId].errorEvents.front().errorInfo;
    EXPECT_EQ(ret, "");

    interruptCheckInfo.isMuteStream = true;
    interruptDfx->audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    interruptDfx->CheckMuteInterruptEvent(streamId);
    ret = interruptDfx->audioInterruptCheckInfo_[streamId].errorEvents.front().errorInfo;
    EXPECT_EQ(ret, "");
}

/**
* @tc.name  : Test CheckShortInterruptEvent
* @tc.number: CheckShortInterruptEvent_001
* @tc.desc  : Test CheckShortInterruptEvent_001
*/
HWTEST_F(AudioInterruptDfxUnitTest, CheckShortInterruptEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioInterruptDfx> interruptDfx = std::make_shared<AudioInterruptDfx>();
    EXPECT_NE(interruptDfx, nullptr);

    uint32_t streamId = 10001;
    AudioInterruptCheckInfo interruptCheckInfo;
    interruptCheckInfo.startTime = std::chrono::steady_clock::now()- std::chrono::seconds(2);
    AudioFocusErrorEvent errorEvent;
    interruptCheckInfo.errorEvents = {errorEvent};
    interruptDfx->audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    interruptDfx->CheckShortInterruptEvent(streamId);
    auto ret = interruptDfx->audioInterruptCheckInfo_[streamId].errorEvents.front().errorInfo;
    EXPECT_EQ(ret, "");

    interruptCheckInfo.startTime = std::chrono::steady_clock::now();
    interruptDfx->audioInterruptCheckInfo_[streamId] = interruptCheckInfo;
    interruptDfx->CheckShortInterruptEvent(streamId);
    ret = interruptDfx->audioInterruptCheckInfo_[streamId].errorEvents.front().errorInfo;
    EXPECT_EQ(ret, "");
}
} // namespace AudioStandard
} // namespace OHOS