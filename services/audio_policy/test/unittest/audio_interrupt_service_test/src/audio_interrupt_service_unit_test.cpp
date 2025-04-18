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

#include "audio_interrupt_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptServiceUnitTest::SetUpTestCase(void) {}
void AudioInterruptServiceUnitTest::TearDownTestCase(void) {}
void AudioInterruptServiceUnitTest::SetUp(void) {}
void AudioInterruptServiceUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_001
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_001, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.end());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_002
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_002, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_003
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_003, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_004
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_004, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, STOP);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_005
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_005, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::PAUSEDBYREMOTE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.end());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_006
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_006, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.begin());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_007
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_007, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.begin());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_008
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_008, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService->dfxCollector_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    audioInterruptService->zonesMap_.insert({0, audioInterruptZone});
    int32_t pid = 0;

    audioInterruptService->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_009
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_009, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService->dfxCollector_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    AudioFocuState audioFocuState;
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, audioFocuState);
    audioInterruptService->zonesMap_.insert({0, audioInterruptZone});
    int32_t pid = 5;

    audioInterruptService->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}
} // namespace AudioStandard
} // namespace OHOS