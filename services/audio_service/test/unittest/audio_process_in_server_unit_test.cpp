/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "audio_process_in_server.h"
#include "audio_errors.h"
#include "audio_service.h"
#include "audio_device_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
    constexpr int32_t DEFAULT_STREAM_ID = 10;
    const uint32_t SPAN_SIZE_IN_FRAME = 1000;
    const uint32_t TOTAL_SIZE_IN_FRAME = 1000;
    const int32_t INTELL_VOICE_SERVICR_UID = 1042;
    constexpr uint32_t MIN_STREAMID_2 = UINT32_MAX - MIN_STREAMID + DEFAULT_STREAM_ID;
    constexpr uint32_t MIN_STREAMID_3 = UINT32_MAX - MIN_STREAMID - DEFAULT_STREAM_ID;
class AudioProcessInServerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioProcessInServerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioProcessInServerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioProcessInServerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioProcessInServerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

AudioStreamInfo g_audioStreamInfo = {
    SAMPLE_RATE_48000,
    ENCODING_PCM,
    SAMPLE_S16LE,
    STEREO,
    CH_LAYOUT_STEREO
};

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appFullTokenId = 1;
    config.appInfo.appTokenId = 1;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_001
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_001, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    uint32_t sessionIdRet = 0;
    bool muteFlagRet = true;
    auto ret = audioProcessInServer->GetSessionId(sessionIdRet);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioProcessInServer->GetSessionId(), sessionIdRet);
    ret = audioProcessInServer->GetMuteState();
    EXPECT_EQ(ret, false);
    audioProcessInServer->SetNonInterruptMute(muteFlagRet);
    EXPECT_EQ(audioProcessInServer->GetMuteState(), true);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_002
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_002, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> bufferRet = nullptr;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 1000;
    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    uint32_t spanSizeInFrame;
    audioProcessInServerRet.ResolveBufferBaseAndGetServerSpanSize(
        bufferRet, spanSizeInFrame);
    audioProcessInServerRet.processBuffer_ = std::make_shared<OHAudioBufferBase>(
        bufferHolder, TOTAL_SIZE_IN_FRAME, byteSizePerFrame);
    EXPECT_NE(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ResolveBufferBaseAndGetServerSpanSize(
        bufferRet, spanSizeInFrame);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_003
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_003, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 1000;
    audioProcessInServerRet.processBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder,
        TOTAL_SIZE_IN_FRAME, byteSizePerFrame);
    EXPECT_NE(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.RequestHandleInfo();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_004
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_004, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_005
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_005, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_U8;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_006
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_006, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S16LE;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_007
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_007, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S24LE;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_008
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_008, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_F32LE;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_009
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_009, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = INVALID_WIDTH;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;

    EXPECT_EQ(audioProcessInServerRet.processBuffer_, nullptr);
    auto ret = audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    EXPECT_EQ(ret, SUCCESS);
}

// /**
//  * @tc.name  : Test AudioProcessInServer API
//  * @tc.type  : FUNC
//  * @tc.number: AudioProcessInServer_010
//  * @tc.desc  : Test AudioProcessInServer interface.
//  */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_010, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STAND_BY);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STAND_BY);

    auto ret = audioProcessInServerRet.Start();
    EXPECT_NE(ret, SUCCESS);
}

// /**
//  * @tc.name  : Test AudioProcessInServer API
//  * @tc.type  : FUNC
//  * @tc.number: AudioProcessInServer_011
//  * @tc.desc  : Test AudioProcessInServer interface.
//  */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_011, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STAND_BY);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STAND_BY);
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    auto ret = audioProcessInServerRet.Start();
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    EXPECT_NE(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_012
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_012, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.callerUid = INTELL_VOICE_SERVICR_UID;
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    auto ret = audioProcessInServerRet.Start();
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    EXPECT_NE(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_013
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_013, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.callerUid = INTELL_VOICE_SERVICR_UID;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);
    auto ret = audioProcessInServerRet.Start();
    EXPECT_NE(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Start();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_014
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_014, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    audioProcessInServerRet.needCheckBackground_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_PAUSING);
    bool isFlush = true;

    auto ret = audioProcessInServerRet.Pause(isFlush);
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = false;
    ret = audioProcessInServerRet.Pause(isFlush);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_015
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_015, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_PAUSING);
    bool isFlush = true;

    auto ret = audioProcessInServerRet.Pause(isFlush);
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Pause(isFlush);
    EXPECT_EQ(ret, SUCCESS);
}

// /**
//  * @tc.name  : Test AudioProcessInServer API
//  * @tc.type  : FUNC
//  * @tc.number: AudioProcessInServer_016
//  * @tc.desc  : Test AudioProcessInServer interface.
//  */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_016, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);

    auto ret = audioProcessInServerRet.Resume();
    EXPECT_NE(ret, SUCCESS);
}

// /**
//  * @tc.name  : Test AudioProcessInServer API
//  * @tc.type  : FUNC
//  * @tc.number: AudioProcessInServer_017
//  * @tc.desc  : Test AudioProcessInServer interface.
//  */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_017, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);
    auto ret = audioProcessInServerRet.Resume();
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Resume();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_018
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_018, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.callerUid = INTELL_VOICE_SERVICR_UID;
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    auto ret = audioProcessInServerRet.Resume();
    EXPECT_EQ(audioProcessInServerRet.needCheckBackground_, false);
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Resume();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_019
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_019, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.callerUid = INTELL_VOICE_SERVICR_UID;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);

    audioProcessInServerRet.streamStatus_->store(STREAM_STARTING);
    EXPECT_EQ(audioProcessInServerRet.streamStatus_->load(), STREAM_STARTING);
    auto ret = audioProcessInServerRet.Resume();
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Resume();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_020
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_020, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    audioProcessInServerRet.needCheckBackground_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_STOPPING);

    int32_t ret = 0;
    audioProcessInServerRet.Stop(ret);
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = false;
    audioProcessInServerRet.Stop(ret);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_021
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_021, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_STOPPING);

    int32_t ret = 0;
    audioProcessInServerRet.Stop(ret);
    EXPECT_EQ(ret, SUCCESS);

    audioProcessInServerRet.needCheckBackground_ = true;
    audioProcessInServerRet.Stop(ret);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_022
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_022, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    audioProcessInServerRet.needCheckBackground_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_STOPPING);
    bool isSwitchStream = false;

    EXPECT_NE(audioProcessInServerRet.releaseCallback_, nullptr);
    auto ret = audioProcessInServerRet.Release(isSwitchStream);
    EXPECT_NE(ret, SUCCESS);

    audioProcessInServerRet.isInited_ = true;
    audioProcessInServerRet.needCheckBackground_ = false;
    ret = audioProcessInServerRet.Release(isSwitchStream);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_023
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_023, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_STOPPING);
    bool isSwitchStream = false;

    auto ret = audioProcessInServerRet.Release(isSwitchStream);
    EXPECT_NE(ret, SUCCESS);

    audioProcessInServerRet.isInited_ = true;
    audioProcessInServerRet.needCheckBackground_ = true;
    ret = audioProcessInServerRet.Release(isSwitchStream);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_024
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_024, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::string dumpStringRet1;

    EXPECT_EQ(audioProcessInServerRet.streamStatus_, nullptr);
    audioProcessInServerRet.Dump(dumpStringRet1);

    audioProcessInServerRet.isInited_ = true;
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServerRet.ConfigProcessBuffer(totalSizeInFrame,
        spanSizeInFrame, g_audioStreamInfo, buffer);
    audioProcessInServerRet.streamStatus_->store(STREAM_STOPPING);
    std::string dumpStringRet2;

    EXPECT_NE(audioProcessInServerRet.streamStatus_, nullptr);
    audioProcessInServerRet.Dump(dumpStringRet2);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_025
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_025, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    std::string name = "unit_test";
    auto ret = audioProcessInServerRet.RegisterThreadPriority(0, name, METHOD_WRITE_OR_READ);
    EXPECT_EQ(ret, SUCCESS);
}
/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_026
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_026, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    configRet.originalSessionId = DEFAULT_STREAM_ID;
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_027
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_027, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    configRet.originalSessionId = MIN_STREAMID_2;
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_028
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_028, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    configRet.originalSessionId = MIN_STREAMID_3;
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_029
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_029, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);

    bool isStandby = true;
    int64_t enterStandbyTime = 0;

    audioProcessInServer->processBuffer_ = nullptr;
    auto ret = audioProcessInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_030
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_030, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);

    bool isStandby = true;
    int64_t enterStandbyTime = 0;

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServer->processBuffer_ = std::make_shared<OHAudioBufferBase>(
        bufferHolder, TOTAL_SIZE_IN_FRAME, byteSizePerFrame);
    EXPECT_NE(audioProcessInServer->processBuffer_, nullptr);

    audioProcessInServer->processBuffer_->basicBufferInfo_ = nullptr;
    auto ret = audioProcessInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_031
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_031, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);

    bool isStandby = true;
    int64_t enterStandbyTime = INTELL_VOICE_SERVICR_UID;

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServer->processBuffer_ = std::make_shared<OHAudioBufferBase>(
        bufferHolder, TOTAL_SIZE_IN_FRAME, byteSizePerFrame);
    EXPECT_NE(audioProcessInServer->processBuffer_, nullptr);

    BasicBufferInfo basicBufferInfo;
    audioProcessInServer->processBuffer_->basicBufferInfo_ = &basicBufferInfo;
    EXPECT_NE(audioProcessInServer->processBuffer_->basicBufferInfo_, nullptr);
    audioProcessInServer->processBuffer_->basicBufferInfo_->streamStatus = STREAM_STAND_BY;

    audioProcessInServer->enterStandbyTime_ = DEFAULT_STREAM_ID;

    auto ret = audioProcessInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(enterStandbyTime, DEFAULT_STREAM_ID);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_032
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_032, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();

    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);

    bool isStandby = true;
    int64_t enterStandbyTime = INTELL_VOICE_SERVICR_UID;

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = SPAN_SIZE_IN_FRAME;
    audioProcessInServer->processBuffer_ = std::make_shared<OHAudioBufferBase>(
        bufferHolder, TOTAL_SIZE_IN_FRAME, byteSizePerFrame);
    EXPECT_NE(audioProcessInServer->processBuffer_, nullptr);

    BasicBufferInfo basicBufferInfo;
    audioProcessInServer->processBuffer_->basicBufferInfo_ = &basicBufferInfo;
    EXPECT_NE(audioProcessInServer->processBuffer_->basicBufferInfo_, nullptr);
    audioProcessInServer->processBuffer_->basicBufferInfo_->streamStatus = STREAM_IDEL;

    audioProcessInServer->enterStandbyTime_ = DEFAULT_STREAM_ID;

    auto ret = audioProcessInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(enterStandbyTime, 0);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_033
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_033, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    int64_t duration = 0;

    int32_t ret = audioProcessInServer->SetSourceDuration(duration);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_034
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_034, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    uint32_t underrunCnt = 0;

    int32_t ret = audioProcessInServer->SetUnderrunCount(underrunCnt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_035
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_035, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);
    int64_t muteFrameCnt = 1;

    audioProcessInServer->lastWriteMuteFrame_ = 0;
    audioProcessInServer->AddMuteWriteFrameCnt(muteFrameCnt);
    EXPECT_EQ(audioProcessInServer->lastWriteMuteFrame_, 1);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_036
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_036, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    auto audioProcessInServer = std::make_shared<AudioProcessInServer>(configRet, releaseCallbackRet);

    audioProcessInServer->lastStopTime_ = 10;
    audioProcessInServer->lastStartTime_ = 100;
    int64_t ret = audioProcessInServer->GetLastAudioDuration();
    EXPECT_EQ(ret, -1);

    audioProcessInServer->lastStopTime_ = 100;
    audioProcessInServer->lastStartTime_ = 10;
    ret = audioProcessInServer->GetLastAudioDuration();
    EXPECT_EQ(ret, 90);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_037
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_037, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    int32_t syncId = 100;
    auto ret = audioProcessInServerRet.SetAudioHapticsSyncId(syncId);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioProcessInServerRet.audioHapticsSyncId_, syncId);
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInServer_038
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, AudioProcessInServer_038, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    int32_t syncId = 100;
    audioProcessInServerRet.audioHapticsSyncId_.store(syncId);
    auto ret = audioProcessInServerRet.GetAudioHapticsSyncId();
    EXPECT_EQ(ret, syncId);
}

/**
 * @tc.name  : Test TurnOnMicIndicator API
 * @tc.type  : FUNC
 * @tc.number: TurnOnMicIndicator_001
 * @tc.desc  : Test TurnOnMicIndicator interface.
 */
HWTEST(AudioProcessInServerUnitTest, TurnOnMicIndicator_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    CapturerState capturerState = CapturerState::CAPTURER_PREPARED;
    audioProcessInServerRet.isMicIndicatorOn_ = true;

    bool ret = audioProcessInServerRet.TurnOnMicIndicator(capturerState);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test TurnOnMicIndicator API
 * @tc.type  : FUNC
 * @tc.number: TurnOnMicIndicator_002
 * @tc.desc  : Test TurnOnMicIndicator interface.
 */
HWTEST(AudioProcessInServerUnitTest, TurnOnMicIndicator_002, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    CapturerState capturerState = CapturerState::CAPTURER_PREPARED;
    audioProcessInServerRet.isMicIndicatorOn_ = false;

    bool ret = audioProcessInServerRet.TurnOnMicIndicator(capturerState);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test TurnOffMicIndicator API
 * @tc.type  : FUNC
 * @tc.number: TurnOffMicIndicator_001
 * @tc.desc  : Test TurnOffMicIndicator interface.
 */
HWTEST(AudioProcessInServerUnitTest, TurnOffMicIndicator_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    CapturerState capturerState = CapturerState::CAPTURER_PREPARED;
    audioProcessInServerRet.isMicIndicatorOn_ = false;

    bool ret = audioProcessInServerRet.TurnOffMicIndicator(capturerState);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test TurnOffMicIndicator API
 * @tc.type  : FUNC
 * @tc.number: TurnOffMicIndicator_002
 * @tc.desc  : Test TurnOffMicIndicator interface.
 */
HWTEST(AudioProcessInServerUnitTest, TurnOffMicIndicator_002, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    CapturerState capturerState = CapturerState::CAPTURER_PREPARED;
    audioProcessInServerRet.isMicIndicatorOn_ = true;

    bool ret = audioProcessInServerRet.TurnOffMicIndicator(capturerState);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test GetInnerCapState API
 * @tc.type  : FUNC
 * @tc.number: GetInnerCapState_001
 * @tc.desc  : Test GetInnerCapState interface.
 */
HWTEST(AudioProcessInServerUnitTest, GetInnerCapState_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    CapturerState capturerState = CapturerState::CAPTURER_PREPARED;
    audioProcessInServerRet.isMicIndicatorOn_ = true;

    bool ret = audioProcessInServerRet.GetInnerCapState(capturerState);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test GetInnerCapState API
 * @tc.type  : FUNC
 * @tc.number: GetInnerCapState_002
 * @tc.desc  : Test GetInnerCapState interface.
 */
HWTEST(AudioProcessInServerUnitTest, GetInnerCapState_002, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    int32_t innerCapId = 1;
    audioProcessInServerRet.SetInnerCapState(true, innerCapId);

    bool ret = audioProcessInServerRet.GetInnerCapState(innerCapId);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CheckBGCapturer API
 * @tc.type  : FUNC
 * @tc.number: CheckBGCapturer_001
 * @tc.desc  : Test CheckBGCapturer interface.
 */
HWTEST(AudioProcessInServerUnitTest, CheckBGCapturer_001, TestSize.Level4)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    EXPECT_FALSE(audioProcessInServerRet.CheckBGCapturer());
}
} // namespace AudioStandard
} // namespace OHOS
