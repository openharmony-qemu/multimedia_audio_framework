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

#include "audio_unit_test.h"
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

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_007
* @tc.desc  : Test ReleaseAudioInterruptZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_007, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamUsage == STREAM_USAGE_MOVIE;
    interruptZone->context.backStrategy_ = MediaBackStrategy::STOP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    auto ret = audioInterruptZoneManager->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(getZoneFunc, nullptr);
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_008
* @tc.desc  : Test ReleaseAudioInterruptZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_008, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamUsage == STREAM_USAGE_MOVIE;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->context.backStrategy_ = MediaBackStrategy::KEEP;
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    auto ret = audioInterruptZoneManager->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(getZoneFunc, nullptr);
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_009
* @tc.desc  : Test ReleaseAudioInterruptZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_009, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamUsage == STREAM_USAGE_INVALID;
    interruptZone->context.backStrategy_ = MediaBackStrategy::KEEP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    auto ret = audioInterruptZoneManager->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(getZoneFunc, nullptr);
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_010
* @tc.desc  : Test ReleaseAudioInterruptZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_010, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamUsage == STREAM_USAGE_MEDIA;
    interruptZone->context.backStrategy_ = MediaBackStrategy::KEEP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::MUTED});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    auto getZoneFunc = [](int32_t uid, const std::string &deviceTag,
        const std::string &streamTag, const StreamUsage &usage)->int32_t {
        return 0;
    };
    auto ret = audioInterruptZoneManager->ReleaseAudioInterruptZone(zoneId, getZoneFunc);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(getZoneFunc, nullptr);
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_011
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_011, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    interruptZone->context.backStrategy_ = MediaBackStrategy::STOP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    auto ret = audioInterruptZoneManager->InjectInterruptToAudioZone(zoneId, interruptZone->audioFocusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(audioInterruptZoneManager->service_->zonesMap_.empty());
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_012
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_012, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamId = 1;
    interruptZone->context.backStrategy_ = MediaBackStrategy::STOP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    AudioInterrupt a2;
    a2.streamId = 2;
    AudioFocusList interrupts;
    interrupts.push_back({a2, AudioFocuState::ACTIVE});
    auto ret = audioInterruptZoneManager->InjectInterruptToAudioZone(zoneId, interrupts);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(audioInterruptZoneManager->service_->zonesMap_.empty());
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_013
* @tc.desc  : Test InjectInterruptToAudioZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_013, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamId = 0;
    interruptZone->context.backStrategy_ = MediaBackStrategy::STOP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    AudioInterrupt a2;
    a2.streamId = 0;
    AudioFocusList interrupts;
    interrupts.push_back({a2, AudioFocuState::PAUSE});
    auto ret = audioInterruptZoneManager->InjectInterruptToAudioZone(zoneId, interrupts);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(audioInterruptZoneManager->service_->zonesMap_.empty());
    MockNative::Resume();
}

/**
* @tc.name  : Test AudioInterruptZoneManager
* @tc.number: AudioInterruptZoneManager_014
* @tc.desc  : Test TryActiveAudioFocusForZone
*/
HWTEST(AudioInterruptZoneUnitTest, AudioInterruptZoneManager_014, TestSize.Level1)
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    ASSERT_NE(audioInterruptZoneManager, nullptr);

    audioInterruptZoneManager->service_ = new AudioInterruptService();
    ASSERT_NE(audioInterruptZoneManager->service_, nullptr);

    int32_t zoneId = 1;
    audioInterruptZoneManager->service_->zonesMap_.clear();
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamId = 0;
    interruptZone->context.backStrategy_ = MediaBackStrategy::STOP;
    interruptZone->audioFocusInfoList.clear();
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});
    audioInterruptZoneManager->service_->zonesMap_[zoneId] = interruptZone;
 
    audioInterruptZoneManager->TryActiveAudioFocusForZone(zoneId, interruptZone->audioFocusInfoList);
    EXPECT_TRUE(interruptZone->audioFocusInfoList.size() > 0);

    interruptZone->audioFocusInfoList.clear();
    audioInterruptZoneManager->TryActiveAudioFocusForZone(zoneId, interruptZone->audioFocusInfoList);
    EXPECT_TRUE(interruptZone->audioFocusInfoList.size() == 0);
}
} // namespace AudioStandard
} // namespace OHOS