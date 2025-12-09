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

#include "audio_interrupt_custom_unit_test.h"
#include "audio_utils.h"
#include "audio_source_type.h"
#include "audio_interrupt_custom.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptCustomUnitTest::SetUpTestCase(void) {}
void AudioInterruptCustomUnitTest::TearDownTestCase(void) {}
void AudioInterruptCustomUnitTest::SetUp(void) {}
void AudioInterruptCustomUnitTest::TearDown(void) {}

/**
* @tc.name  : Test ProcessActiveStreamCustomFocus.
* @tc.number: ProcessActiveStreamCustomFocus_01
* @tc.desc  : Test ProcessActiveStreamCustomFocus.
*/
HWTEST_F(AudioInterruptCustomUnitTest, ProcessActiveStreamCustomFocus_01, TestSize.Level1)
{
    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_ULTRASONIC, 100, "SOURCE_TYPE_ULTRASONIC");

    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;

    AudioInterruptCustom interruptCustom;
    interruptCustom.ProcessActiveStreamCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent);
    EXPECT_EQ(incomingState, PAUSE);
}

/**
* @tc.name  : Test ProcessActiveStreamCustomFocus.
* @tc.number: ProcessActiveStreamCustomFocus_02
* @tc.desc  : Test ProcessActiveStreamCustomFocus.
*/
HWTEST_F(AudioInterruptCustomUnitTest, ProcessActiveStreamCustomFocus_02, TestSize.Level1)
{
    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_ULTRASONIC, 100, "SOURCE_TYPE_ULTRASONIC");

    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_ULTRASONIC;

    AudioInterruptCustom interruptCustom;
    interruptCustom.ProcessActiveStreamCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent);
    EXPECT_EQ(incomingState, ACTIVE);
}

/**
* @tc.name  : Test ProcessActiveStreamCustomFocus.
* @tc.number: ProcessActiveStreamCustomFocus_03
* @tc.desc  : Test ProcessActiveStreamCustomFocus.
*/
HWTEST_F(AudioInterruptCustomUnitTest, ProcessActiveStreamCustomFocus_03, TestSize.Level1)
{
    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_ULTRASONIC, 100, "SOURCE_TYPE_ULTRASONIC");

    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_ULTRASONIC;

    AudioInterruptCustom interruptCustom;
    interruptCustom.ProcessActiveStreamCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent);
    EXPECT_EQ(incomingState, PAUSE);
}

/**
* @tc.name  : Test ProcessActiveStreamCustomFocus.
* @tc.number: ProcessActiveStreamCustomFocus_04
* @tc.desc  : Test ProcessActiveStreamCustomFocus.
*/
HWTEST_F(AudioInterruptCustomUnitTest, ProcessActiveStreamCustomFocus_04, TestSize.Level1)
{
    SolePipe::SetSolePipeSourceInfo(SOURCE_TYPE_ULTRASONIC, 100, "SOURCE_TYPE_ULTRASONIC");

    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_ULTRASONIC;

    AudioInterruptCustom interruptCustom;
    interruptCustom.ProcessActiveStreamCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent);
    EXPECT_EQ(incomingState, ACTIVE);
}

/**
* @tc.name  : Test CeliaCustomFocus_01.
* @tc.number: CeliaCustomFocus_01
* @tc.desc  : Test CeliaCustomFocus_01.
*/
HWTEST_F(AudioInterruptCustomUnitTest, CeliaCustomFocus_01, TestSize.Level1)
{
    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    std::string appName = "vassistant";
    AudioInterruptCustom interruptCustom;
    interruptCustom.CeliaCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent, appName);
    EXPECT_EQ(incomingState, ACTIVE);
}

/**
* @tc.name  : Test CeliaCustomFocus_02.
* @tc.number: CeliaCustomFocus_02
* @tc.desc  : Test CeliaCustomFocus_02.
*/
HWTEST_F(AudioInterruptCustomUnitTest, CeliaCustomFocus_02, TestSize.Level1)
{
    AudioFocuState incomingState = PAUSE;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_INTERNAL_FORCE_STOP;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    std::string appName = "vassistant";
    AudioInterruptCustom interruptCustom;
    interruptCustom.CeliaCustomFocus(incomingInterrupt, activeInterrupt,
    incomingState, interruptEvent, appName);
    EXPECT_NE(incomingState, ACTIVE);
}

} // namespace AudioStandard
} // namespace OHOS
