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
public:
  AudioVolume* audioVolumeTest;
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
    bool isVKB = false;
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    audioVolumeTest = AudioVolume::GetInstance();
    EXPECT_NE(nullptr, audioVolumeTest);
}

void AudioVolumeUnitTest::TearDown(void)
{
    uint32_t sessionId = 1;
    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);

    audioVolumeTest = nullptr;
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
 * @tc.number: GetVolume_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_003, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t volumeType = STREAM_MUSIC;
    std::string deviceClass = "speaker";
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    SystemVolume systemVolume(STREAM_MUSIC, "speaker", 0.5f, 5, true);
    AudioVolume::GetInstance()->SetSystemVolume(systemVolume);

    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, deviceClass, &volumes);
    EXPECT_EQ(volume, 0.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_004
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_004, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t volumeType = STREAM_MUSIC;
    std::string deviceClass = "speaker";
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    SystemVolume systemVolume(STREAM_MUSIC, "speaker", 0.5f, 5, false);
    AudioVolume::GetInstance()->SetSystemVolume(systemVolume);

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
    int32_t streamType = STREAM_MUSIC_TEST;
    int32_t streamUsage = STREAM_USAGE_MEDIA_TEST;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
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
    bool isVKB = true;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);
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
    int32_t appUid = 123;
    int32_t sessionId = 123;
    AudioVolume::GetInstance()->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);
    int32_t ret = AudioVolume::GetInstance()->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
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
    int32_t appUid = 123;
    int32_t sessionId = 123;
    AudioVolume::GetInstance()->SetDoNotDisturbStatus(isDoNotDisturbStatus);
    int32_t ret = AudioVolume::GetInstance()->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, doNotDisturbStatusVolume);
}

/**
 * @tc.name  : Test GetVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_005
 * @tc.desc  : Test GetVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_005, TestSize.Level1)
{
    uint32_t sessionId = 123;
    int32_t streamType = 5;
    const std::string deviceClass = "test";
    VolumeValues volumes;

    audioVolumeTest->streamVolume_.clear();

    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = true;
    int32_t mode = 0;
    bool isVKB = false;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->GetVolume(sessionId, streamType, deviceClass, &volumes);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_001
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_001, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = true;
    int32_t mode = 0;
    bool isVKB = false;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_002
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_002, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 5523;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_003
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_003, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});
    audioVolumeTest->doNotDisturbStatusWhiteListVolume_.insert({appUid, 1});

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_004
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_004, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});
    audioVolumeTest->doNotDisturbStatusWhiteListVolume_.insert({1, 1});

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_005
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_005, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});
    audioVolumeTest->doNotDisturbStatusWhiteListVolume_.insert({1, 1});

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_006
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_006, TestSize.Level1)
{
    int32_t volumeType = STREAM_DTMF;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_007
 * @tc.desc  : Test GetDoNotDisturbStatusVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetDoNotDisturbStatusVolume_007, TestSize.Level1)
{
    int32_t volumeType = STREAM_MEDIA;
    int32_t appUid = 123;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;
    audioVolumeTest->streamVolume_.clear();
    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolume_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetStreamVolume_003, TestSize.Level1)
{
    uint32_t sessionId = 1;

    audioVolumeTest->streamVolume_.clear();

    float ret = audioVolumeTest->GetStreamVolume(sessionId);
    EXPECT_EQ(ret, 1.0f);
}

/**
 * @tc.name  : Test SaveAdjustStreamVolumeInfo API
 * @tc.type  : FUNC
 * @tc.number: SetStreamVolume_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SaveAdjustStreamVolumeInfo_003, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float volume = 0.1f;
    std::string invocationTime = "test";
    uint32_t code = 3;

    audioVolumeTest->SaveAdjustStreamVolumeInfo(volume, sessionId, invocationTime, code);
    EXPECT_NE(nullptr, audioVolumeTest);
}

/**
 * @tc.name  : Test GetStreamVolumeInfo API
 * @tc.type  : FUNC
 * @tc.number: GetStreamVolumeInfo_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamVolumeInfo_001, TestSize.Level1)
{
    AdjustStreamVolume volumeType = static_cast<AdjustStreamVolume>(3);
    std::vector<AdjustStreamVolumeInfo> adjustStreamVolumeInfoTest;

    adjustStreamVolumeInfoTest = audioVolumeTest->GetStreamVolumeInfo(volumeType);
    EXPECT_EQ(adjustStreamVolumeInfoTest.empty(), true);
}

/**
 * @tc.name  : Test GetAppVolumeInternal API
 * @tc.type  : FUNC
 * @tc.number: GetAppVolumeInternal_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetAppVolumeInternal_001, TestSize.Level1)
{
    int32_t appUid = 1;
    AudioVolumeMode mode = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    audioVolumeTest->appVolume_.clear();

    float ret = audioVolumeTest->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, 1.0f);
}

/**
 * @tc.name  : Test GetAppVolumeInternal API
 * @tc.type  : FUNC
 * @tc.number: GetAppVolumeInternal_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetAppVolumeInternal_002, TestSize.Level1)
{
    int32_t appUid = 1;
    AudioVolumeMode mode = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;
    audioVolumeTest->appVolume_.insert({appUid, appVolume});

    float ret = audioVolumeTest->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, appVolume.totalVolume_);
    EXPECT_EQ(ret, 3.0f);
}

/**
 * @tc.name  : Test GetAppVolumeInternal API
 * @tc.type  : FUNC
 * @tc.number: GetAppVolumeInternal_003
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetAppVolumeInternal_003, TestSize.Level1)
{
    int32_t appUid = 1;
    AudioVolumeMode mode = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = false;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;
    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->appVolume_.insert({appUid, appVolume});

    float ret = audioVolumeTest->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, 1.0);
}

/**
 * @tc.name  : Test GetAppVolumeInternal API
 * @tc.type  : FUNC
 * @tc.number: GetAppVolumeInternal_004
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetAppVolumeInternal_004, TestSize.Level1)
{
    int32_t appUid = 1;
    AudioVolumeMode mode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = false;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;
    audioVolumeTest->appVolume_.insert({appUid, appVolume});

    float ret = audioVolumeTest->GetAppVolume(appUid, mode);
    EXPECT_EQ(ret, appVolume.totalVolume_);
    EXPECT_EQ(ret, 3.0f);
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_002, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isMuted = false;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), true);
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_003
 * @tc.desc  : Test AudioVolume interface  stream.GetAppUid() != appUid.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_003, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isMuted = false;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 123;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_004
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_004, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isMuted = true;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 1;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_NE(it, audioVolumeTest->streamVolume_.end());
    EXPECT_EQ(it->second.appVolume_, 0.0f);
    EXPECT_EQ(it->second.totalVolume_, 0.0f);
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_005
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_005, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isMuted = false;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 1;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_EQ(it->second.appVolume_, 1.0f);
    EXPECT_EQ(it->second.totalVolume_, 1.0f);
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_006
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_006, TestSize.Level1)
{
    int32_t appUid = 1;
    bool isMuted = false;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 1;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 1;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_EQ(it->second.appVolume_, 1.0f);
    EXPECT_EQ(it->second.totalVolume_, 1.0f);
}

/**
 * @tc.name  : Test SetAppVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_002
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_002, TestSize.Level1)
{
    int32_t appUid = 1;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), true);

    EXPECT_EQ(appVolume.totalVolume_, 0.0f);
}

/**
 * @tc.name  : Test SetAppVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_003
 * @tc.desc  : Test AudioVolume interface stream.GetAppUid() != appUid
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_003, TestSize.Level1)
{
    int32_t appUid = 1;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = false;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 1;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 1;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 2.0f);
}

/**
 * @tc.name  : Test SetAppVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_004
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_004, TestSize.Level1)
{
    int32_t appUid = 123;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = true;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 1;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 0.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second.appVolume_, 0.0f);
    EXPECT_EQ(it->second.totalVolume_, 0.0f);
}

/**
 * @tc.name  : Test SetAppVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_005
 * @tc.desc  : Test AudioVolume interface stream.GetVolumeMode() == AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_005, TestSize.Level1)
{
    int32_t appUid = 123;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = false;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 1;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 2.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second.appVolume_, 1.0f);
    EXPECT_EQ(it->second.totalVolume_, 1.0f);
}

/**
 * @tc.name  : Test SetAppVolume API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolume_006
 * @tc.desc  : Test AudioVolume interface stream.GetVolumeMode() != AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolume_006, TestSize.Level1)
{
    int32_t appUid = 123;
    float volume = 2.0f;
    int32_t volumeLevel = 1;
    bool isMuted = false;
    AppVolume appVolume(appUid, volume, volumeLevel, isMuted);
    appVolume.totalVolume_ = 3.0f;

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();

    uint32_t sessionId = 123;
    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, isSystemApp, mode, false);
    audioVolumeTest->streamVolume_.insert({sessionId, streamVolume});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 2.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second.appVolume_, 2.0f);
}

/**
 * @tc.name  : Test SetSystemVolume API
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_001
 * @tc.desc  : Test AudioVolume interface
 */
HWTEST_F(AudioVolumeUnitTest, SetSystemVolume_005, TestSize.Level1)
{
    audioVolumeTest->systemVolume_.clear();
    int32_t volumeType = 0;
    std::string deviceClass = "test";
    float volume = 2.0f;
    int32_t volumeLevel = 2;
    audioVolumeTest->SetSystemVolume(volumeType, deviceClass, volume, volumeLevel);

    EXPECT_EQ(audioVolumeTest->systemVolume_.empty(), false);

    std::string key = std::to_string(volumeType) + deviceClass;
    auto it = audioVolumeTest->systemVolume_.find(key);
    EXPECT_EQ(it->second.totalVolume_, volume);
}

/**
 * @tc.name  : Test GetSimpleBufferAvg API
 * @tc.type  : FUNC
 * @tc.number: GetSimpleBufferAvg_002
 * @tc.desc  : Test GetSimpleBufferAvg interface
 */
HWTEST_F(AudioVolumeUnitTest, GetSimpleBufferAvg_002, TestSize.Level1)
{
    uint8_t buffer = 1;
    int32_t length = 1;
    auto ret = GetSimpleBufferAvg(&buffer, length);
    EXPECT_EQ(ret, 1);
}
/**
 * @tc.name  : Test GetCurVolume_001 API
 * @tc.type  : FUNC
 * @tc.number: GetCurVolume_001
 * @tc.desc  : Test GetCurVolume_001 interface
 */
HWTEST_F(AudioVolumeUnitTest, GetCurVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    const char *deviceClass = "device";
    struct VolumeValues volumes;
    float result = GetCurVolume(sessionId, nullptr, deviceClass, &volumes);
    EXPECT_FLOAT_EQ(result, 1.0f);

    const char *streamType = "stream";
    result = GetCurVolume(sessionId, streamType, nullptr, &volumes);
    EXPECT_FLOAT_EQ(result, 1.0f);
}

/**
 * @tc.name  : Test GetCurVolume_002 API
 * @tc.type  : FUNC
 * @tc.number: GetCurVolume_002
 * @tc.desc  : Test GetCurVolume_002 interface
 */
HWTEST_F(AudioVolumeUnitTest, GetStopFadeoutState_003, TestSize.Level1)
{
    uint32_t streamIndex = -1;
    float result = GetStopFadeoutState(streamIndex);
    EXPECT_EQ(result, INVALID_STATE);

    streamIndex = 1;
    result = GetStopFadeoutState(streamIndex);
    EXPECT_EQ(result, INVALID_STATE);
}

/**
 * @tc.name  : Test GetCurVolume_002 API
 * @tc.type  : FUNC
 * @tc.number: GetCurVolume_002
 * @tc.desc  : Test GetCurVolume_002 interface
 */
HWTEST_F(AudioVolumeUnitTest, GetFadeStrategy_003, TestSize.Level1)
{
    uint64_t DURATION_TIME_DEFAULT = 40;
    uint64_t DURATION_TIME_SHORT = 10;
    uint64_t DURATION_INIT = 0;
    EXPECT_EQ(FADE_STRATEGY_DEFAULT, GetFadeStrategy(DURATION_INIT));
    EXPECT_EQ(FADE_STRATEGY_DEFAULT, GetFadeStrategy(DURATION_TIME_DEFAULT + 1));
    EXPECT_EQ(FADE_STRATEGY_NONE, GetFadeStrategy(DURATION_TIME_SHORT));
    EXPECT_EQ(FADE_STRATEGY_NONE, GetFadeStrategy(DURATION_INIT + 1));
    EXPECT_EQ(FADE_STRATEGY_SHORTER, GetFadeStrategy(DURATION_TIME_SHORT + 1));
    EXPECT_EQ(FADE_STRATEGY_SHORTER, GetFadeStrategy(DURATION_TIME_DEFAULT - 1));
    EXPECT_EQ(FADE_STRATEGY_SHORTER, GetFadeStrategy(DURATION_TIME_DEFAULT));
}

/**
 * @tc.name  : Test SetAppVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetAppVolumeMute_007
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppVolumeMute_007, TestSize.Level1)
{
    bool isMuted = true;
    int32_t appuid = 123;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    AudioVolume::GetInstance()->SetAppVolumeMute(appuid, isMuted);

    isMuted = false;
    AudioVolume::GetInstance()->SetAppVolumeMute(appuid, isMuted);

    appuid = -1;
    AudioVolume::GetInstance()->SetAppVolumeMute(appuid, isMuted);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadType_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetOffloadType_001, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    int32_t offloadType = OFFLOAD_ACTIVE_BACKGROUND;
    AudioVolume::GetInstance()->SetOffloadType(streamIndex, offloadType);
    int32_t getOffloadType = AudioVolume::GetInstance()->GetOffloadType(streamIndex);
    EXPECT_EQ(getOffloadType, offloadType);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadType_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetOffloadType_002, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    AudioVolume::GetInstance()->offloadType_.clear();
    uint32_t ret = AudioVolume::GetInstance()->GetOffloadType(streamIndex);
    EXPECT_EQ(ret, OFFLOAD_DEFAULT);
}

/**
 * @tc.name  : Test AudioVolume
 * @tc.number: SetAppRingMuted_001
 * @tc.desc  : Test SetAppRingMuted interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppRingMuted_001, TestSize.Level1)
{
    bool isMuted = true;
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 1;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    AppVolume appVolume(appUid, 1.0f, 0, true);
    audioVolumeTest->appVolume_.emplace(appUid, appVolume);

    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolumeTest->streamVolume_.emplace(sessionId, streamVolume);

    bool result = audioVolumeTest->SetAppRingMuted(appUid, isMuted);

    EXPECT_EQ(result, true);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}

/**
 * @tc.name  : Test AudioVolume
 * @tc.number: SetAppRingMuted_002
 * @tc.desc  : Test SetAppRingMuted interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppRingMuted_002, TestSize.Level1)
{
    bool isMuted = false;
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 1;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolumeTest->streamVolume_.emplace(sessionId, streamVolume);

    bool result = audioVolumeTest->SetAppRingMuted(appUid, isMuted);

    EXPECT_EQ(result, true);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}

/**
 * @tc.name  : Test AudioVolume
 * @tc.number: SetAppRingMuted_003
 * @tc.desc  : Test SetAppRingMuted interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppRingMuted_003, TestSize.Level1)
{
    bool isMuted = false;
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 1;
    AudioStreamType streamType = STREAM_VOICE_COMMUNICATION;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;

    AppVolume appVolume(appUid, 1.0f, 0, true);
    audioVolumeTest->appVolume_.emplace(appUid, appVolume);

    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolumeTest->streamVolume_.emplace(sessionId, streamVolume);

    bool result = audioVolumeTest->SetAppRingMuted(appUid, isMuted);

    EXPECT_EQ(result, false);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}

/**
 * @tc.name  : Test AudioVolume
 * @tc.number: SetAppRingMuted_004
 * @tc.desc  : Test SetAppRingMuted interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetAppRingMuted_004, TestSize.Level1)
{
    bool isMuted = false;
    int32_t appUid = 123;
    int32_t anotherAppUid = 456;
    int32_t sessionId = 10001;
    int32_t pid = 1;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    AppVolume appVolume(appUid, 1.0f, 0, true);
    audioVolumeTest->appVolume_.emplace(appUid, appVolume);

    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolumeTest->streamVolume_.emplace(sessionId, streamVolume);

    bool result = audioVolumeTest->SetAppRingMuted(anotherAppUid, isMuted);

    EXPECT_EQ(result, false);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
