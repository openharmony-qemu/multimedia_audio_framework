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
const uint32_t TEST_PIPE_ID = 100;
const uint32_t TEST_PIPE_ID_2 = 200;
const uint32_t TEST_PIPE_ID_INVALID = 0xFFFFFFFF;

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
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
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
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
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
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);

    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
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
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);
    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);

    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
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
 * @tc.number: SetDualStreamVolumeMute_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetDualStreamVolumeMute_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    bool isDualMuted = true;
    AudioVolume::GetInstance()->SetDualStreamVolumeMute(sessionId, isDualMuted);
    float retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, 0);
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
    audioVolume->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB)});
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
    VolumeValues volumes;

    audioVolumeTest->streamVolume_.clear();

    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = true;
    int32_t mode = 0;
    bool isVKB = false;
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB)});

    audioVolumeTest->GetVolume(sessionId, streamType, TEST_PIPE_ID, &volumes);
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
    bool isSystemApp = false;
    int32_t mode = 0;
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});
    
    std::vector<std::map<std::string, std::string>> doNotDisturbStatusWhiteList;
    std::map<std::string, std::string> obj;
    obj["1"] = "1";
    doNotDisturbStatusWhiteList.push_back(obj);
    audioVolumeTest->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);

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
    int32_t appUid = 1001;
    uint32_t sessionId = 123;

    audioVolumeTest->isDoNotDisturbStatus_ = true;

    int32_t streamType = 0;
    int32_t streamUsage = 0;
    int32_t uid = 0;
    int32_t pid = 0;
    bool isSystemApp = false;
    int32_t mode = 0;
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});
    
    std::vector<std::map<std::string, std::string>> doNotDisturbStatusWhiteList;
    std::map<std::string, std::string> obj;
    obj[std::to_string(appUid)] = "1";
    doNotDisturbStatusWhiteList.push_back(obj);
    audioVolumeTest->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);

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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});
    
    std::vector<std::map<std::string, std::string>> doNotDisturbStatusWhiteList;
    std::map<std::string, std::string> obj;
    obj["1"] = "1";
    doNotDisturbStatusWhiteList.push_back(obj);
    audioVolumeTest->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);

    uint32_t ret = audioVolumeTest->GetDoNotDisturbStatusVolume(volumeType, appUid, sessionId);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name  : Test GetDoNotDisturbStatusVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDoNotDisturbStatusVolume_004
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});
    
    std::vector<std::map<std::string, std::string>> doNotDisturbStatusWhiteList;
    std::map<std::string, std::string> obj;
    obj["1"] = "1";
    doNotDisturbStatusWhiteList.push_back(obj);
    audioVolumeTest->SetDoNotDisturbStatusWhiteListVolume(doNotDisturbStatusWhiteList);

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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_NE(it, audioVolumeTest->streamVolume_.end());
    EXPECT_EQ(it->second->appVolume_, 0.0f);
    EXPECT_EQ(it->second->GetTotalVolume(), 0.0f);
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_EQ(it->second->appVolume_, 1.0f);
    EXPECT_EQ(it->second->GetTotalVolume(), 1.0f);
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolumeMute(appUid, isMuted);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);

    auto it = audioVolumeTest->streamVolume_.find(sessionId);
    EXPECT_EQ(it->second->appVolume_, 1.0f);
    EXPECT_EQ(it->second->GetTotalVolume(), 1.0f);
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 0.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second->appVolume_, 0.0f);
    EXPECT_EQ(it->second->GetTotalVolume(), 0.0f);
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 2.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second->appVolume_, 1.0f);
    EXPECT_EQ(it->second->GetTotalVolume(), 1.0f);
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
    audioVolumeTest->streamVolume_.insert({sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, isSystemApp, mode, false)});

    audioVolumeTest->SetAppVolume(appVolume);
    EXPECT_EQ(audioVolumeTest->appVolume_.empty(), false);
    EXPECT_EQ(audioVolumeTest->streamVolume_.empty(), false);
    EXPECT_EQ(appVolume.totalVolume_, 2.0f);

    auto it = audioVolumeTest->streamVolume_.find(appUid);
    EXPECT_EQ(it->second->appVolume_, 2.0f);
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
    struct VolumeValues volumes;
    float result = GetCurVolume(sessionId, nullptr, TEST_PIPE_ID_INVALID, &volumes);
    EXPECT_FLOAT_EQ(result, 1.0f);

    const char *streamType = "stream";
    result = GetCurVolume(sessionId, streamType, TEST_PIPE_ID_INVALID, &volumes);
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
    auto result = static_cast<FadePauseState>(GetStopFadeoutState(streamIndex));
    EXPECT_EQ(result, INVALID_STATE);

    streamIndex = 1;
    result = static_cast<FadePauseState>(GetStopFadeoutState(streamIndex));
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

    audioVolumeTest->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

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

    audioVolumeTest->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

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

    audioVolumeTest->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

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

    audioVolumeTest->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

    bool result = audioVolumeTest->SetAppRingMuted(anotherAppUid, isMuted);

    EXPECT_EQ(result, false);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_006
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_006, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t volumeType = STREAM_VOICE_ASSISTANT;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);

    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);

    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volume, 0.0f);

    streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, true, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volumes.volumeStream, 1.0f);

    volumeType = STREAM_SYSTEM;
    volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volumes.volumeStream, 1.0f);

    volumeType = STREAM_VOICE_CALL;
    volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volumes.volumeStream, 1.0f);

    volumeType = STREAM_VOICE_COMMUNICATION;
    volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volumes.volumeStream, 1.0f);

    volumes = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    volume = AudioVolume::GetInstance()->GetVolume(sessionId, volumeType, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volumes.volumeStream, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_007
 * @tc.desc  : Test GetVolume interface with min fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_007, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = 0.0f;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_SYSTEM_ENFORCED, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volume, 0.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_008
 * @tc.desc  : Test GetVolume interface with valid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_008, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = 0.5f;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_SYSTEM_ENFORCED, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volume, 0.5f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_009
 * @tc.desc  : Test GetVolume interface with max fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_009, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = 1.0f;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_SYSTEM_ENFORCED, TEST_PIPE_ID, &volumes);
    EXPECT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_010
 * @tc.desc  : Test GetVolume interface with invalid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolume_010, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = -1.0f;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_SYSTEM_ENFORCED, TEST_PIPE_ID, &volumes);
    EXPECT_NE(volume, -1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeValues_001
 * @tc.desc  : Test GetVolumeValues interface with invalid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeValues_001, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = -1.0f;
    AudioVolume::GetInstance()->currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_SCO;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_VOICE_COMMUNICATION, TEST_PIPE_ID, &volumes);
    EXPECT_NE(volume, -1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeValues_002
 * @tc.desc  : Test GetVolumeValues interface with invalid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeValues_002, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = -1.0f;
    AudioVolume::GetInstance()->currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_SCO;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_VOICE_COMMUNICATION, TEST_PIPE_ID, &volumes);
    EXPECT_NE(volume, -1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeValues_003
 * @tc.desc  : Test GetVolumeValues interface with invalid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeValues_003, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = -1.0f;
    AudioVolume::GetInstance()->currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_VOICE_COMMUNICATION, TEST_PIPE_ID, &volumes);
    EXPECT_NE(volume, -1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeValues_004
 * @tc.desc  : Test GetVolumeValues interface with invalid fixed system enforced tone volume
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeValues_004, TestSize.Level1)
{
    uint32_t sessionId = 123;
    struct VolumeValues volumes = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    VolumeUtils::enforcedToneVolume_ = -1.0f;
    AudioVolume::GetInstance()->currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_SCO;
    float volume = AudioVolume::GetInstance()->GetVolume(sessionId, STREAM_VOICE_RING, TEST_PIPE_ID, &volumes);
    EXPECT_NE(volume, -1.0f);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStreamVolume_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamVolume_002, TestSize.Level1)
{
    uint32_t sessionId = 531;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MUSIC;
    int32_t uid = 1000;
    int32_t pid = 1000;
    int32_t mode = 1;
    bool isVKB = true;
    ASSERT_TRUE(AudioVolume::GetInstance() != nullptr);

    StreamVolumeParams streamVolumeParams = { sessionId, streamType, streamUsage, uid, pid, false, mode, isVKB };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    bool isMuted = true;
    AudioVolume::GetInstance()->SetStreamVolumeMute(sessionId, isMuted);

    float volumeStream = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(volumeStream, 0.0f);

    volumeStream = AudioVolume::GetInstance()->GetStreamVolume(sessionId + 1);
    EXPECT_EQ(volumeStream, 1.0f);
}

/**
 * @tc.name  : Test GetCurVolume_002 API
 * @tc.type  : FUNC
 * @tc.number: GetCurVolume_004
 * @tc.desc  : Test GetCurVolume_002 interface
 */
HWTEST_F(AudioVolumeUnitTest, GetStopFadeoutState_004, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    AudioVolume::GetInstance()->SetStopFadeoutState(streamIndex, 1);
    uint32_t result = AudioVolume::GetInstance()->GetStopFadeoutState(streamIndex);
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name  : ShouldApplySystemAppVolume_PCVolumeEnabled
 * @tc.number: ShouldApplySystemAppVolume_001
 * @tc.desc  : Test when IsPCVolumeEnable returns true
 */
HWTEST_F(AudioVolumeUnitTest, ShouldApplySystemAppVolume_PCVolumeEnabled, TestSize.Level1)
{
    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    bool isPCVolumeEnabled = true;
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnabled);
    AudioStreamType streamType = STREAM_MUSIC;

    bool result = audioVolumeTest->ShouldApplySystemAppVolume(streamType);
    EXPECT_TRUE(result);

    VolumeUtils::SetPCVolumeEnable(false);
}

/**
 * @tc.name  : ShouldApplySystemAppVolume_AppIndividualVolumeDisabled
 * @tc.number: ShouldApplySystemAppVolume_002
 * @tc.desc  : Test when appIndividualVolumeEnabled is false
 */
HWTEST_F(AudioVolumeUnitTest, ShouldApplySystemAppVolume_AppIndividualVolumeDisabled, TestSize.Level1)
{
    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    bool isPCVolumeEnabled = false;
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnabled);
    AudioStreamType streamType = STREAM_MUSIC;

    bool result = audioVolumeTest->ShouldApplySystemAppVolume(streamType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : ShouldApplySystemAppVolume_StreamMusic
 * @tc.number: ShouldApplySystemAppVolume_003
 * @tc.desc  : Test returns true for STREAM_MUSIC when appIndividualVolumeEnabled is true
 */
HWTEST_F(AudioVolumeUnitTest, ShouldApplySystemAppVolume_StreamMusic, TestSize.Level1)
{
    audioVolumeTest->appIndividualVolumeEnabled_ = true;
    bool isPCVolumeEnabled = false;
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnabled);
    AudioStreamType streamType = STREAM_MUSIC;

    bool result = audioVolumeTest->ShouldApplySystemAppVolume(streamType);
    EXPECT_TRUE(result);

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
}

/**
 * @tc.name  : ShouldApplySystemAppVolume_StreamNotMusic
 * @tc.number: ShouldApplySystemAppVolume_004
 * @tc.desc  : Test returns false for non-MUSIC stream when appIndividualVolumeEnabled is true
 */
HWTEST_F(AudioVolumeUnitTest, ShouldApplySystemAppVolume_StreamNotMusic, TestSize.Level1)
{
    audioVolumeTest->appIndividualVolumeEnabled_ = true;
    bool isPCVolumeEnabled = false;
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnabled);
    AudioStreamType streamType = STREAM_VOICE_CALL;

    bool result = audioVolumeTest->ShouldApplySystemAppVolume(streamType);
    EXPECT_FALSE(result);

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
}

/**
 * @tc.name  : Test IsSameVolume API
 * @tc.type  : FUNC
 * @tc.number: IsSameVolume_001
 * @tc.desc  : Test IsSameVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, IsSameVolume_001, TestSize.Level4)
{
    float x = 0.0f;
    float y = 0.0f;
    EXPECT_TRUE(AudioVolume::GetInstance()->IsSameVolume(x, y));;
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadEnable_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetOffloadEnable_001, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    int32_t offloadEnable = 1;
    AudioVolume::GetInstance()->SetOffloadEnable(streamIndex, offloadEnable);
    int32_t getOffloadType = AudioVolume::GetInstance()->GetOffloadEnable(streamIndex);
    EXPECT_EQ(getOffloadType, offloadEnable);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetOffloadEnable_002
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetOffloadEnable_002, TestSize.Level1)
{
    uint32_t streamIndex = 1;
    AudioVolume::GetInstance()->offloadEnable_.clear();
    uint32_t ret = AudioVolume::GetInstance()->GetOffloadEnable(streamIndex);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: GetCurrentActiveDevice_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, GetCurrentActiveDevice_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioVolume::GetInstance()->currentActiveDevice_ = deviceType;
    DeviceType ret = AudioVolume::GetInstance()->GetCurrentActiveDevice();
    EXPECT_EQ(ret, deviceType);
}

/**
 * @tc.name  : Test AudioVolume API
 * @tc.type  : FUNC
 * @tc.number: SetNonInterruptMute_001
 * @tc.desc  : Test AudioVolume interface.
 */
HWTEST_F(AudioVolumeUnitTest, SetNonInterruptMute_001, TestSize.Level1)
{
    bool isMuted = false;
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 1;
    AudioStreamType streamType = STREAM_GAME;
    StreamUsage streamUsage = STREAM_USAGE_GAME;

    AppVolume appVolume(appUid, 1.0f, 0, true);
    audioVolumeTest->appVolume_.emplace(appUid, appVolume);

    audioVolumeTest->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

    AudioVolume::GetInstance()->SetNonInterruptMute(sessionId, isMuted);
    float retVolume = AudioVolume::GetInstance()->GetStreamVolume(sessionId);
    EXPECT_EQ(retVolume, 0);

    audioVolumeTest->appVolume_.clear();
    audioVolumeTest->streamVolume_.clear();
}

/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_001
 * @tc.desc  : Test SetPipeVolume interface - normal case
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_001, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    auto it = AudioVolume::GetInstance()->pipeVolume_.find(key);
    EXPECT_TRUE(it != AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_002
 * @tc.desc  : Test SetPipeVolume interface - update case
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_002, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    PipeVolume pipeVolume2(TEST_PIPE_ID, STREAM_MUSIC, 1.0f, 10, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume2);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    auto it = AudioVolume::GetInstance()->pipeVolume_.find(key);
    EXPECT_TRUE(it != AudioVolume::GetInstance()->pipeVolume_.end());
    EXPECT_EQ(it->second->volume_, 1.0f);
}

/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_003
 * @tc.desc  : Test SetPipeVolume interface - different volumeType on same pipeId
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_003, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume1(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume1);
    PipeVolume pipeVolume2(TEST_PIPE_ID, STREAM_VOICE_CALL, 0.8f, 8, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume2);
    std::string key1 = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    std::string key2 = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_VOICE_CALL);
    EXPECT_TRUE(AudioVolume::GetInstance()->pipeVolume_.find(key1) != AudioVolume::GetInstance()->pipeVolume_.end());
    EXPECT_TRUE(AudioVolume::GetInstance()->pipeVolume_.find(key2) != AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_004
 * @tc.desc  : Test SetPipeVolume interface - different pipeId
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_004, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume1(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume1);
    PipeVolume pipeVolume2(TEST_PIPE_ID_2, STREAM_MUSIC, 0.8f, 8, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume2);
    std::string key1 = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    std::string key2 = std::to_string(TEST_PIPE_ID_2) + "|" + std::to_string(STREAM_MUSIC);
    EXPECT_TRUE(AudioVolume::GetInstance()->pipeVolume_.find(key1) != AudioVolume::GetInstance()->pipeVolume_.end());
    EXPECT_TRUE(AudioVolume::GetInstance()->pipeVolume_.find(key2) != AudioVolume::GetInstance()->pipeVolume_.end());
}

#ifdef MULTI_ALARM_LEVEL
/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_005
 * @tc.desc  : Test SetPipeVolume interface - STREAM_ANNOUNCEMENT not settable
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_005, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_ANNOUNCEMENT, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_ANNOUNCEMENT);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : Test SetPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolume_006
 * @tc.desc  : Test SetPipeVolume interface - STREAM_EMERGENCY not settable
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolume_006, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_EMERGENCY, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_EMERGENCY);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}
#endif

/**
 * @tc.name  : Test SetPipeVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolumeMute_001
 * @tc.desc  : Test SetPipeVolumeMute interface - set mute true
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolumeMute_001, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    AudioVolume::GetInstance()->SetPipeVolumeMute(TEST_PIPE_ID, STREAM_MUSIC, true);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    auto it = AudioVolume::GetInstance()->pipeVolume_.find(key);
    EXPECT_TRUE(it != AudioVolume::GetInstance()->pipeVolume_.end());
    EXPECT_EQ(it->second->isMuted_, true);
}

/**
 * @tc.name  : Test SetPipeVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolumeMute_002
 * @tc.desc  : Test SetPipeVolumeMute interface - set mute false
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolumeMute_002, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, true);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    AudioVolume::GetInstance()->SetPipeVolumeMute(TEST_PIPE_ID, STREAM_MUSIC, false);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    auto it = AudioVolume::GetInstance()->pipeVolume_.find(key);
    EXPECT_TRUE(it != AudioVolume::GetInstance()->pipeVolume_.end());
    EXPECT_EQ(it->second->isMuted_, false);
}

/**
 * @tc.name  : Test SetPipeVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolumeMute_003
 * @tc.desc  : Test SetPipeVolumeMute interface - non-existent entry
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolumeMute_003, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    AudioVolume::GetInstance()->SetPipeVolumeMute(TEST_PIPE_ID, STREAM_MUSIC, true);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}

#ifdef MULTI_ALARM_LEVEL
/**
 * @tc.name  : Test SetPipeVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolumeMute_004
 * @tc.desc  : Test SetPipeVolumeMute interface - STREAM_ANNOUNCEMENT not mutable
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolumeMute_004, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    AudioVolume::GetInstance()->SetPipeVolumeMute(TEST_PIPE_ID, STREAM_ANNOUNCEMENT, true);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_ANNOUNCEMENT);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : Test SetPipeVolumeMute API
 * @tc.type  : FUNC
 * @tc.number: SetPipeVolumeMute_005
 * @tc.desc  : Test SetPipeVolumeMute interface - STREAM_EMERGENCY not mutable
 */
HWTEST_F(AudioVolumeUnitTest, SetPipeVolumeMute_005, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    AudioVolume::GetInstance()->SetPipeVolumeMute(TEST_PIPE_ID, STREAM_EMERGENCY, true);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_EMERGENCY);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}
#endif

/**
 * @tc.name  : Test RemovePipeVolume API
 * @tc.type  : FUNC
 * @tc.number: RemovePipeVolume_001
 * @tc.desc  : Test RemovePipeVolume interface - remove existing entry
 */
HWTEST_F(AudioVolumeUnitTest, RemovePipeVolume_001, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 5, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    AudioVolume::GetInstance()->RemovePipeVolume(TEST_PIPE_ID, STREAM_MUSIC);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : Test RemovePipeVolume API
 * @tc.type  : FUNC
 * @tc.number: RemovePipeVolume_002
 * @tc.desc  : Test RemovePipeVolume interface - remove non-existent entry
 */
HWTEST_F(AudioVolumeUnitTest, RemovePipeVolume_002, TestSize.Level1)
{
    audioVolumeTest->pipeVolume_.clear();
    AudioVolume::GetInstance()->RemovePipeVolume(TEST_PIPE_ID, STREAM_MUSIC);
    std::string key = std::to_string(TEST_PIPE_ID) + "|" + std::to_string(STREAM_MUSIC);
    EXPECT_EQ(AudioVolume::GetInstance()->pipeVolume_.find(key), AudioVolume::GetInstance()->pipeVolume_.end());
}

/**
 * @tc.name  : GetSystemAppVolume_NonMusicStream
 * @tc.number: GetSystemAppVolume_001
 * @tc.desc  : Test GetSystemAppVolume returns 1.0f when streamType is not MUSIC
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolume_NonMusicStream, TestSize.Level1)
{
    int32_t uid = 1001;
    AudioStreamType streamType = STREAM_VOICE_CALL;

    audioVolumeTest->appIndividualVolumeEnabled_ = true;
    VolumeUtils::SetPCVolumeEnable(false);

    float result = audioVolumeTest->GetSystemAppVolume(uid, streamType);
    EXPECT_EQ(1.0f, result);
}

/**
 * @tc.name  : GetSystemAppVolume_AppIndividualVolumeDisabled
 * @tc.number: GetSystemAppVolume_002
 * @tc.desc  : Test GetSystemAppVolume returns 1.0f when appIndividualVolumeEnabled is false
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolume_AppIndividualVolumeDisabled, TestSize.Level1)
{
    int32_t uid = 1001;
    AudioStreamType streamType = STREAM_MUSIC;

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    VolumeUtils::SetPCVolumeEnable(false);

    float result = audioVolumeTest->GetSystemAppVolume(uid, streamType);
    EXPECT_EQ(1.0f, result);
}

/**
 * @tc.name  : GetSystemAppVolume_PCVolumeEnabled
 * @tc.number: GetSystemAppVolume_003
 * @tc.desc  : Test GetSystemAppVolume returns 1.0f when uid not found in systemAppVolume_
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolume_PCVolumeEnabled, TestSize.Level1)
{
    int32_t uid = 1001;
    AudioStreamType streamType = STREAM_MUSIC;

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    VolumeUtils::SetPCVolumeEnable(true);
    audioVolumeTest->systemAppVolume_.clear();

    float result = audioVolumeTest->GetSystemAppVolume(uid, streamType);
    EXPECT_EQ(1.0f, result);
}

/**
 * @tc.name  : GetSystemAppVolume_WithVolumeRecord
 * @tc.number: GetSystemAppVolume_004
 * @tc.desc  : Test GetSystemAppVolume returns stored volume when uid exists
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolume_WithVolumeRecord, TestSize.Level1)
{
    int32_t uid = 1001;
    AudioStreamType streamType = STREAM_MUSIC;

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    VolumeUtils::SetPCVolumeEnable(true);
    audioVolumeTest->systemAppVolume_.clear();

    SystemAppVolume systemAppVolume(uid, 0.5f, 1, false);
    systemAppVolume.totalVolume_ = 0.8f;
    audioVolumeTest->systemAppVolume_.insert({uid, systemAppVolume});

    float result = audioVolumeTest->GetSystemAppVolume(uid, streamType);
    EXPECT_EQ(0.8f, result);

    audioVolumeTest->systemAppVolume_.clear();
    VolumeUtils::SetPCVolumeEnable(false);
}

/**
 * @tc.name  : GetSystemAppVolumeEffective_NonMusicStream
 * @tc.number: GetSystemAppVolumeEffective_001
 * @tc.desc  : Test GetSystemAppVolumeEffective returns 1.0f when streamType is not MUSIC
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolumeEffective_NonMusicStream, TestSize.Level1)
{
    int32_t sessionId = 1;
    int32_t streamType = STREAM_VOICE_CALL;
    int32_t streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    int32_t uid = 1001;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);

    audioVolumeTest->appIndividualVolumeEnabled_ = true;
    VolumeUtils::SetPCVolumeEnable(false);

    float result = audioVolumeTest->GetSystemAppVolumeEffective(streamVolume);
    EXPECT_EQ(1.0f, result);
}

/**
 * @tc.name  : GetSystemAppVolumeEffective_AppIndividualVolumeDisabled
 * @tc.number: GetSystemAppVolumeEffective_002
 * @tc.desc  : Test GetSystemAppVolumeEffective returns 1.0f when appIndividualVolumeEnabled is false
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolumeEffective_AppIndividualVolumeDisabled, TestSize.Level1)
{
    int32_t sessionId = 1;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1001;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);

    audioVolumeTest->appIndividualVolumeEnabled_ = false;
    VolumeUtils::SetPCVolumeEnable(false);

    float result = audioVolumeTest->GetSystemAppVolumeEffective(streamVolume);
    EXPECT_EQ(1.0f, result);
}

/**
 * @tc.name  : GetSystemAppVolumeEffective_WithVolumeRecord
 * @tc.number: GetSystemAppVolumeEffective_003
 * @tc.desc  : Test GetSystemAppVolumeEffective returns stream.systemAppVolume_ when should apply
 */
HWTEST_F(AudioVolumeUnitTest, GetSystemAppVolumeEffective_WithVolumeRecord, TestSize.Level1)
{
    int32_t sessionId = 1;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1001;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolume streamVolume(sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB);
    streamVolume.systemAppVolume_ = 0.8f;

    audioVolumeTest->appIndividualVolumeEnabled_ = true;
    VolumeUtils::SetPCVolumeEnable(false);

    float result = audioVolumeTest->GetSystemAppVolumeEffective(streamVolume);
    EXPECT_EQ(0.8f, result);
}

/**
 * @tc.name  : Test GetStreamAndPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStreamAndPipeVolume_001
 * @tc.desc  : Test GetStreamAndPipeVolume returns shared_ptr successfully.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamAndPipeVolume_001, TestSize.Level1)
{
    uint32_t sessionId = 2;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1002;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 1.0f, 15, false);
    audioVolumeTest->SetPipeVolume(pipeVolume);
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, TEST_PIPE_ID, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, 0);
    EXPECT_NE(streamVolumePtr, nullptr);
    EXPECT_NE(pipeVolumePtr, nullptr);
    EXPECT_EQ(streamVolumePtr->GetSessionId(), sessionId);
    EXPECT_EQ(pipeVolumePtr->GetPipeId(), TEST_PIPE_ID);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test GetStreamAndPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStreamAndPipeVolume_002
 * @tc.desc  : Test GetStreamAndPipeVolume returns -1 when sessionId not found.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamAndPipeVolume_002, TestSize.Level1)
{
    uint32_t sessionId = 9999;
    int32_t streamType = STREAM_MUSIC;
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, TEST_PIPE_ID, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, -1);
    EXPECT_EQ(streamVolumePtr, nullptr);
    EXPECT_EQ(pipeVolumePtr, nullptr);
}

/**
 * @tc.name  : Test GetStreamAndPipeVolume API
 * @tc.type  : FUNC
 * @tc.number: GetStreamAndPipeVolume_003
 * @tc.desc  : Test GetStreamAndPipeVolume returns -1 when pipe not found.
 */
HWTEST_F(AudioVolumeUnitTest, GetStreamAndPipeVolume_003, TestSize.Level1)
{
    uint32_t sessionId = 3;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1003;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    uint32_t invalidPipeId = 99999;
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, invalidPipeId, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, -1);
    EXPECT_EQ(streamVolumePtr, nullptr);
    EXPECT_EQ(pipeVolumePtr, nullptr);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test GetVolume API with shared_ptr
 * @tc.type  : FUNC
 * @tc.number: GetVolumeSharedPtr_001
 * @tc.desc  : Test GetVolume(shared_ptr version) returns correct volume.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeSharedPtr_001, TestSize.Level1)
{
    uint32_t sessionId = 4;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1004;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    audioVolumeTest->SetStreamVolume(sessionId, 0.8f);
    
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 10, false);
    audioVolumeTest->SetPipeVolume(pipeVolume);
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, TEST_PIPE_ID, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, 0);
    
    VolumeValues volumes;
    float volume = audioVolumeTest->GetVolume(streamVolumePtr, pipeVolumePtr, &volumes);
    
    EXPECT_GT(volume, 0.0f);
    EXPECT_LE(volume, 1.0f);
    EXPECT_FLOAT_EQ(volumes.volumePipe, 0.5f);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test GetVolume API with shared_ptr
 * @tc.type  : FUNC
 * @tc.number: GetVolumeSharedPtr_002
 * @tc.desc  : Test GetVolume(shared_ptr version) returns 1.0f when streamVolume is null.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeSharedPtr_002, TestSize.Level1)
{
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 10, false);
    audioVolumeTest->SetPipeVolume(pipeVolume);
    
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    std::shared_ptr<StreamVolume> streamVolumePtr;
    audioVolumeTest->GetStreamAndPipeVolume(99999, STREAM_MUSIC, TEST_PIPE_ID,
        streamVolumePtr, pipeVolumePtr);
    
    VolumeValues volumes;
    float volume = audioVolumeTest->GetVolume(nullptr, pipeVolumePtr, &volumes);
    
    EXPECT_FLOAT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test GetVolume API with shared_ptr
 * @tc.type  : FUNC
 * @tc.number: GetVolumeSharedPtr_003
 * @tc.desc  : Test GetVolume(shared_ptr version) returns 1.0f when pipeVolume is null.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeSharedPtr_003, TestSize.Level1)
{
    uint32_t sessionId = 5;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1005;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    audioVolumeTest->GetStreamAndPipeVolume(sessionId, streamType, TEST_PIPE_ID + 99,
        streamVolumePtr, pipeVolumePtr);
    
    VolumeValues volumes;
    float volume = audioVolumeTest->GetVolume(streamVolumePtr, nullptr, &volumes);
    
    EXPECT_FLOAT_EQ(volume, 1.0f);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test GetVolume API with shared_ptr
 * @tc.type  : FUNC
 * @tc.number: GetVolumeSharedPtr_004
 * @tc.desc  : Test GetVolume(shared_ptr version) calculates volume correctly.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeSharedPtr_004, TestSize.Level1)
{
    uint32_t sessionId = 6;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1006;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = false;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    audioVolumeTest->SetStreamVolume(sessionId, 0.8f);
    audioVolumeTest->SetStreamVolumeDuckFactor(sessionId, 0.9f, 0);
    
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.7f, 12, false);
    audioVolumeTest->SetPipeVolume(pipeVolume);
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, TEST_PIPE_ID, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, 0);
    
    VolumeValues volumes;
    float volume = audioVolumeTest->GetVolume(streamVolumePtr, pipeVolumePtr, &volumes);
    
    float expectedStreamVolume = 0.8f * 0.9f;
    float expectedTotalVolume = expectedStreamVolume * 0.7f;
    
    EXPECT_GT(volume, 0.0f);
    EXPECT_FLOAT_EQ(volumes.volumePipe, expectedTotalVolume);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}

/**
 * @tc.name  : Test GetVolume API with shared_ptr
 * @tc.type  : FUNC
 * @tc.number: GetVolumeSharedPtr_005
 * @tc.desc  : Test GetVolume(shared_ptr version) with virtual keyboard.
 */
HWTEST_F(AudioVolumeUnitTest, GetVolumeSharedPtr_005, TestSize.Level1)
{
    uint32_t sessionId = 7;
    int32_t streamType = STREAM_MUSIC;
    int32_t streamUsage = STREAM_USAGE_MEDIA;
    int32_t uid = 1007;
    int32_t pid = 1000;
    bool isSystemApp = false;
    int32_t mode = 1;
    bool isVKB = true;
    StreamVolumeParams params = { sessionId, streamType, streamUsage, uid, pid, isSystemApp, mode, isVKB };
    audioVolumeTest->AddStreamVolume(params);
    
    PipeVolume pipeVolume(TEST_PIPE_ID, STREAM_MUSIC, 0.5f, 10, true);
    audioVolumeTest->SetPipeVolume(pipeVolume);
    
    std::shared_ptr<StreamVolume> streamVolumePtr;
    std::shared_ptr<PipeVolume> pipeVolumePtr;
    
    int32_t result = audioVolumeTest->GetStreamAndPipeVolume(
        sessionId, streamType, TEST_PIPE_ID, streamVolumePtr, pipeVolumePtr);
    
    EXPECT_EQ(result, 0);
    
    VolumeValues volumes;
    float volume = audioVolumeTest->GetVolume(streamVolumePtr, pipeVolumePtr, &volumes);
    
    EXPECT_EQ(volume, 0.0f);
    
    audioVolumeTest->RemoveStreamVolume(sessionId);
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
