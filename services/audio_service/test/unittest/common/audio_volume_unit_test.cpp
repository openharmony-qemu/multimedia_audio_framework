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

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_volume.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t STREAM_MUSIC_TEST = STREAM_MUSIC;
const int32_t STREAM_VOICE_TEST = STREAM_VOICE_CALL;
const int32_t STREAM_USAGE_MEDIA_TEST = 1;

class AudioVolumeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioVolumeUnitTest::SetUpTestCase(void)
{
}

void AudioVolumeUnitTest::TearDownTestCase(void)
{
}

void AudioVolumeUnitTest::SetUp(void)
{
    uint32_t sessionId = 1;
    int32_t streamType = STREAM_MUSIC_TEST;
    int32_t streamUsage = STREAM_USAGE_MEDIA_TEST;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    AudioVolume::GetInstance()->AddStreamVolume(sessionId, streamType, streamUsage, uid, pid, false, mode);
}

void AudioVolumeUnitTest::TearDown(void)
{
    uint32_t sessionId = 1;
    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    int32_t volumeType = STREAM_MUSIC_TEST;
    std::string deviceClass = "speaker";
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, deviceClass, &volumes);
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_002, TestSize.Level1)
{
    uint32_t sessionId = 1;
    int32_t volumeType = STREAM_VOICE_TEST;
    std::string deviceClass = "speaker";
    AudioVolume::GetInstance()->SetVgsVolumeSupported(true);
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, deviceClass, &volumes);
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetHistoryVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetHistoryVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float volume = AudioVolume::GetInstance()->GetHistoryVolume(sessionId);
    EXPECT_EQ(volume, 0.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetHistoryVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetHistoryVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float volume = 0.5f;
    AudioVolume::GetInstance()->SetHistoryVolume(sessionId, volume);
    float getVolume = AudioVolume::GetInstance()->GetHistoryVolume(sessionId);
    EXPECT_EQ(getVolume, volume);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float volume = 0.5f;
    AudioVolume::GetInstance()->SetStreamVolume(sessionId, volume);
    float  retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, volume);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolume_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolume_002, TestSize.Level1)
{
    uint32_t sessionId = 1;
    AudioVolume::GetInstance()->streamVolume_.clear();
    AudioVolume::GetInstance()->SetStreamVolume(sessionId, 1.0f);
    auto it = AudioVolume::GetInstance()->streamVolume_.find(sessionId);
    EXPECT_EQ(it == AudioVolume::GetInstance()->streamVolume_.end(), true);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolumeDuckFactor_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolumeDuckFactor_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float duckFactor = 0.5f;
    AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(sessionId, duckFactor);
    float retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, duckFactor);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolumeLowPowerFactor_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolumeLowPowerFactor_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float lowPowerFactor = 0.5f;
    AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(sessionId, lowPowerFactor);
    float retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, lowPowerFactor);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolumeMute_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolumeMute_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    bool isMuted = true;
    AudioVolume::GetInstance()->SetStreamVolumeMute(sessionId, isMuted);
    float retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, 0);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolume_001, TestSize.Level1)
{
    SystemVolume systemVolume(STREAM_MUSIC_TEST, "speaker", 0.5f, 5, false);
    AudioVolume::GetInstance()->SetSystemVolume(systemVolume);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1speaker");
    EXPECT_TRUE(it != AudioVolume::GetInstance()->systemVolume_.end());
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolume_002, TestSize.Level1)
{
    SystemVolume systemVolume(STREAM_MUSIC_TEST, "speaker", 0.5f, 5, false);
    AudioVolume::GetInstance()->SetSystemVolume(systemVolume);
    SystemVolume systemVolume2(STREAM_MUSIC_TEST, "speaker", 1.0f, 5, false);
    AudioVolume::GetInstance()->SetSystemVolume(systemVolume2);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1speaker");
    EXPECT_EQ(it->second.volume_, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolume_003, TestSize.Level1)
{
    AudioVolume::GetInstance()->SetSystemVolume(STREAM_MUSIC_TEST, "speaker", 0.5f, 5);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1speaker");
    EXPECT_TRUE(it != AudioVolume::GetInstance()->systemVolume_.end());
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_004
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolume_004, TestSize.Level1)
{
    AudioVolume::GetInstance()->SetSystemVolume(STREAM_MUSIC_TEST, "speaker", 0.5f, 5);
    AudioVolume::GetInstance()->SetSystemVolume(STREAM_MUSIC_TEST, "speaker", 1.0f, 5);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1speaker");
    EXPECT_EQ(it->second.volume_, 1.0f);
}
/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolumeMute_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolumeMute_001, TestSize.Level1)
{
    int32_t volumeType = STREAM_MUSIC_TEST;
    std::string deviceClass = "speaker";
    bool isMuted = true;
    AudioVolume::GetInstance()->SetSystemVolumeMute(volumeType, deviceClass, isMuted);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1speaker");
    EXPECT_TRUE(it != AudioVolume::GetInstance()->systemVolume_.end());
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolumeMute_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolumeMute_002, TestSize.Level1)
{
    int32_t volumeType = STREAM_MUSIC_TEST;
    std::string deviceClass = "test";
    bool isMuted = true;
    AudioVolume::GetInstance()->SetSystemVolumeMute(volumeType, deviceClass, isMuted);
    auto it = AudioVolume::GetInstance()->systemVolume_.find("1test");
    EXPECT_TRUE(it != AudioVolume::GetInstance()->systemVolume_.end());
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetFadeoutState_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetFadeoutState_001, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    uint32_t fadeoutState = DO_FADE;
    AudioVolume::GetInstance()->SetFadeoutState(streamIndex, fadeoutState);
    uint32_t getFadeoutState = AudioVolume::GetInstance()->GetFadeoutState(streamIndex);
    EXPECT_EQ(getFadeoutState, fadeoutState);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetFadeoutState_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetFadeoutState_002, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    AudioVolume::GetInstance()->fadeoutState_.clear();
    uint32_t ret = AudioVolume::GetInstance()->GetFadeoutState(streamIndex);
    EXPECT_EQ(ret, INVALID_STATE);
}
/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStreamVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamVolume_001, TestSize.Level1)
{
    float volumeStream = AudioVolume::GetInstance()->GetStreamVolume(1);
    EXPECT_EQ(volumeStream, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: AddStreamVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, AddStreamVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    int32_t sample = AudioVolume::GetInstance()->streamVolume_.size();
    int32_t streamType = STREAM_MUSIC_TEST;
    int32_t streamUsage = STREAM_USAGE_MEDIA_TEST;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    AudioVolume::GetInstance()->AddStreamVolume(sessionId, streamType, streamUsage, uid, pid, false, mode);
    int32_t ret = AudioVolume::GetInstance()->streamVolume_.size();
    EXPECT_EQ(ret, sample);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: ConvertStreamTypeStrToInt_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, ConvertStreamTypeStrToInt_001, TestSize.Level1)
{
    std::string streamType ="ring";
    int32_t ret = AudioVolume::GetInstance()->ConvertStreamTypeStrToInt(streamType);
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: ConvertStreamTypeStrToInt_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, ConvertStreamTypeStrToInt_002, TestSize.Level1)
{
    std::string streamType ="test";
    int32_t ret = AudioVolume::GetInstance()->ConvertStreamTypeStrToInt(streamType);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SaveAdjustStreamVolumeInfo_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SaveAdjustStreamVolumeInfo_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    float volume = 0.5f;
    uint32_t sessionId = 0;
    std::string invocationTime  = GetTime();
    uint32_t code = static_cast<uint32_t>(AdjustStreamVolume::STREAM_VOLUME_INFO);
    audioVolume->SaveAdjustStreamVolumeInfo(volume, sessionId, invocationTime, code);
    auto ret = audioVolume->GetStreamVolumeInfo(AdjustStreamVolume::STREAM_VOLUME_INFO);
    EXPECT_TRUE(ret.size() != 0);

    code = static_cast<uint32_t>(AdjustStreamVolume::LOW_POWER_VOLUME_INFO);
    audioVolume->SaveAdjustStreamVolumeInfo(volume, sessionId, invocationTime, code);
    ret = audioVolume->GetStreamVolumeInfo(AdjustStreamVolume::LOW_POWER_VOLUME_INFO);
    EXPECT_TRUE(ret.size() != 0);

    code = static_cast<uint32_t>(AdjustStreamVolume::DUCK_VOLUME_INFO);
    audioVolume->SaveAdjustStreamVolumeInfo(volume, sessionId, invocationTime, code);
    ret = audioVolume->GetStreamVolumeInfo(AdjustStreamVolume::DUCK_VOLUME_INFO);
    EXPECT_TRUE(ret.size() != 0);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: RemoveStreamVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, RemoveStreamVolume_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint32_t sessionId = 1;
    audioVolume->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SaveAdjustStreamVolumeInfo_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SaveAdjustStreamVolumeInfo_002, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    float volume = 0.5f;
    uint32_t sessionId = 0;
    std::string invocationTime  = GetTime();
    uint32_t code = 10;
    audioVolume->SaveAdjustStreamVolumeInfo(volume, sessionId, invocationTime, code);
    AdjustStreamVolume adjustStreamVolume = static_cast<AdjustStreamVolume>(10);
    auto ret = audioVolume->GetStreamVolumeInfo(adjustStreamVolume);
    EXPECT_TRUE(ret.size() == 0);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetAppVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetAppVolume_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    int32_t appUid = 0;
    float volume = 0.1f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    AudioVolumeMode mode = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    auto ret = audioVolume->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, 1.0f);

    audioVolume->appVolume_.insert({appUid, appVolume});
    ret = audioVolume->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    int32_t appUid = 0;
    float volume = 0.1f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    audioVolume->appVolume_.insert({appUid, appVolume});

    audioVolume->SetAppVolumeMute(appUid, false);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    int32_t appUid = 0;
    float volume = 0.1f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    audioVolume->appVolume_.insert({appUid, appVolume});

    audioVolume->SetAppVolume(appVolume);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: Monitor_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, Monitor_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint32_t sessionId = 0;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode);
    audioVolume->streamVolume_.insert({sessionId, streamVolume});
    audioVolume->Monitor(sessionId, true);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStopFadeoutState_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetStopFadeoutState_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint32_t streamIndex = 0;
    auto ret = audioVolume->GetStopFadeoutState(streamIndex);
    EXPECT_EQ(ret, INVALID_STATE);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStopFadeoutState_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetStopFadeoutState_002, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint32_t streamIndex = 0;
    audioVolume->stopFadeoutState_.insert({0, 0});
    auto ret = audioVolume->GetStopFadeoutState(streamIndex);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetSimpleBufferAvg_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetSimpleBufferAvg_001, TestSize.Level1)
{
    uint8_t ffer = 0;
    uint8_t *buffer = &ffer;
    int32_t length = 0;
    auto ret = GetSimpleBufferAvg(buffer, length);
    EXPECT_EQ(ret, -1);

    length = 1;
    GetSimpleBufferAvg(buffer, length);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetFadeStrategy_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetFadeStrategy_001, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint64_t expectedPlaybackDurationMs = 0;
    auto ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_DEFAULT);

    expectedPlaybackDurationMs = 50;
    ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_DEFAULT);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetFadeStrategy_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetFadeStrategy_002, TestSize.Level1)
{
    auto audioVolume = std::make_shared<AudioVolume>();
    ASSERT_TRUE(audioVolume != nullptr);

    uint64_t expectedPlaybackDurationMs = 5;
    auto ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_NONE);

    expectedPlaybackDurationMs = -1;
    ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_DEFAULT);

    expectedPlaybackDurationMs = 15;
    ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_SHORTER);

    expectedPlaybackDurationMs = 15;
    ret = GetFadeStrategy(expectedPlaybackDurationMs);
    EXPECT_EQ(ret, FADE_STRATEGY_SHORTER);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetDoNotDisturbStatusWhiteListVolume_001
 * @tc.desc  : Test AudioVolume interface.
 */
 HWTEST_F(AudioVolumeUnitTest, SetDoNotDisturbStatusWhiteListVolume_001, TestSize.Level1)
 {
    std::vector<std::map<std::string, std::string>> doNotDisturbStatusWhiteList;
    std::map<std::string, std::string> obj;
    obj["123"] = "1";
    doNotDisturbStatusWhiteList.push_back(obj);
    int32_t doNotDisturbStatusVolume = 1;
    int32_t volumeType = 5;
    int32_t sessionId = 123;
    AudioVolume::GetInstance()->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);
    int32_t ret = AudioVolume::GetInstance()->GetDoNotDisturbStatusVolume(volumeType, sessionId);
    EXPECT_EQ(ret, doNotDisturbStatusVolume);
}

 /**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetDoNotDisturbStatus_001
 * @tc.desc  : Test AudioVolume interface.
 */
 HWTEST_F(AudioVolumeUnitTest, SetDoNotDisturbStatus_001, TestSize.Level1)
 {
    bool isDoNotDisturbStatus = true;
    int32_t doNotDisturbStatusVolume = 0;
    int32_t volumeType = 5;
    int32_t sessionId = 123;
    AudioVolume::GetInstance()->SetDoNotDisturbStatus(isDoNotDisturbStatus);
    int32_t ret = AudioVolume::GetInstance()->GetDoNotDisturbStatusVolume(volumeType, sessionId);
    EXPECT_EQ(ret, doNotDisturbStatusVolume);
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
