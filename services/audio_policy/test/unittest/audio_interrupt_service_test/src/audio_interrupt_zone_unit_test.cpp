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

void AudioInterruptZoneUnitTest::SetUpTestCase(void) {}
void AudioInterruptZoneUnitTest::TearDownTestCase(void) {}
void AudioInterruptZoneUnitTest::SetUp(void) {}
void AudioInterruptZoneUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_001
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_001, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 5;
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_002
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_002, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 0;
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_003
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_003, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.deviceTag = "1";
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, audioFocuState);
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 0;
    std::string deviceTag = "1";
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_004
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_004, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.deviceTag = "1";
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, audioFocuState);
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 0;
    std::string deviceTag = "0";
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_005
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_005, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 0;
    std::string deviceTag = "0";
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_006
* @tc.desc  : Test AudioInterruptZoneManager
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_006, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    audioInterruptZoneManager->service_->zonesMap_.insert({0, audioInterruptZone});

    int32_t zoneId = 1;
    std::string deviceTag = "0";
    AudioFocusList focusInfoList;

    auto ret = audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 0);
}
} // namespace AudioStandard
} // namespace OHOS