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
#include "oh_audio_buffer.h"
#include <gmock/gmock.h>

using namespace testing::ext;
using namespace testing;

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

/**
 * @tc.name  : Test GetByteSizePerFrame API
 * @tc.type  : FUNC
 * @tc.number: GetByteSizePerFrame_001
 * @tc.desc  : Test GetByteSizePerFrame interface.
 */
HWTEST(AudioProcessInServerUnitTest, GetByteSizePerFrame_001, TestSize.Level4)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.byteSizePerFrame_ = 10;

    uint32_t ret = audioProcessInServerRet.GetByteSizePerFrame();
    EXPECT_EQ(ret, audioProcessInServerRet.byteSizePerFrame_);
}

/**
 * @tc.name  : Test RequestHandleInfoAsync API
 * @tc.type  : FUNC
 * @tc.number: RequestHandleInfoAsync_001
 * @tc.desc  : Test RequestHandleInfoAsync interface.
 */
HWTEST(AudioProcessInServerUnitTest, RequestHandleInfoAsync_001, TestSize.Level4)
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
    auto ret = audioProcessInServerRet.RequestHandleInfoAsync();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamInfo_001
 * @tc.desc  : Test UpdateStreamInfo interface.
 */
HWTEST(AudioProcessInServerUnitTest, UpdateStreamInfo_001, TestSize.Level1)
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

    audioProcessInServerRet.UpdateStreamInfo();
    EXPECT_GT(audioProcessInServerRet.checkCount_, 0);

    audioProcessInServerRet.UpdateStreamInfo();

    auto ret = audioProcessInServerRet.Release(isSwitchStream);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSpanSizeInFrame API
 * @tc.type  : FUNC
 * @tc.number: GetSpanSizeInFrame_001
 * @tc.desc  : Test GetSpanSizeInFrame interface.
 */
HWTEST(AudioProcessInServerUnitTest, GetSpanSizeInFrame_001, TestSize.Level4)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    audioProcessInServerRet.spanSizeInframe_ = 10;

    uint32_t ret = audioProcessInServerRet.GetSpanSizeInFrame();
    EXPECT_EQ(ret, audioProcessInServerRet.spanSizeInframe_);
}

/*
 * @tc.name  : Test NeedUseTempBuffer API
 * @tc.type  : FUNC
 * @tc.number: NeedUseTempBuffer_01
 * @tc.desc  : Test AudioEndpointInner::NeedUseTempBuffer()
 */
HWTEST_F(AudioProcessInServerUnitTest, NeedUseTempBuffer_01, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    std::vector<uint8_t> buffer1(1, 0);
    std::vector<uint8_t> buffer2(1, 0);
    RingBufferWrapper ringBuffer = {
        {{
            {.buffer = buffer1.data(), .bufLength = 1},
            {.buffer = buffer2.data(), .bufLength = 1},
        }},
        // 1 + 1 = 2
        .dataLength = 2
    };
    auto ret = audioProcessInServerRet.NeedUseTempBuffer(ringBuffer, 1);
    EXPECT_EQ(ret, true);

    ringBuffer.dataLength = 1;
    ret = audioProcessInServerRet.NeedUseTempBuffer(ringBuffer, 1);
    EXPECT_EQ(ret, false);

    // 2 > 1
    ret = audioProcessInServerRet.NeedUseTempBuffer(ringBuffer, 2);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name  : Test PrepareStreamDataBuffer API
 * @tc.type  : FUNC
 * @tc.number: PrepareStreamDataBuffer_01
 * @tc.desc  : Test AudioEndpointInner::PrepareStreamDataBuffer()
 */
HWTEST_F(AudioProcessInServerUnitTest, PrepareStreamDataBuffer_01, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    std::vector<uint8_t> buffer1(1, 0);
    RingBufferWrapper ringBuffer = {
        {{
            {.buffer = buffer1.data(), .bufLength = 1},
            {.buffer = nullptr, .bufLength = 0},
        }},
        .dataLength = 1
    };
    AudioStreamData streamData;
    audioProcessInServerRet.PrepareStreamDataBuffer(1, ringBuffer, streamData);
    // spansizeinframe == 2; spansizeinframe > datalenth
    audioProcessInServerRet.PrepareStreamDataBuffer(2, ringBuffer, streamData);

    // processTmpBufferList[i] == spansizeinframe
    EXPECT_EQ(audioProcessInServerRet.processTmpBuffer_.size(), 2);
}

class MockProResampler : public HPAE::ProResampler {
public:
    MockProResampler() : ProResampler(
        44100,  // 44100 is inRateSample
        48000,  // 48000 is outRateSample
        2,      // 2 is channels
        1       // 1 is quality
    ) {}
    
    MOCK_METHOD(int32_t, Process, (const float*, uint32_t, float*, uint32_t), ());
};

/**
 * @tc.name  : Test WriteToSpecialProcBuf API
 * @tc.type  : FUNC
 * @tc.number: WriteToSpecialProcBuf_001
 * @tc.desc  : Test WriteToSpecialProcBuf with null process buffer.
 */
HWTEST(AudioProcessInServerUnitTest, WriteToSpecialProcBuf_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService* service = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServer(config, service);
    BufferDesc readBuf;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    audioProcessInServer.processBuffer_ = nullptr; // Set null process buffer

    auto result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test WriteToSpecialProcBuf API
 * @tc.type  : FUNC
 * @tc.number: WriteToSpecialProcBuf_002
 * @tc.desc  : Test WriteToSpecialProcBuf with insufficient writable size.
 */
HWTEST(AudioProcessInServerUnitTest, WriteToSpecialProcBuf_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService* service = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServer(config, service);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    EXPECT_NE(procBuf, nullptr);
    EXPECT_EQ(procBuf->Init(-1, -1, 0), SUCCESS);
    audioProcessInServer.processBuffer_ = procBuf;
    audioProcessInServer.spanSizeInframe_ = 100; // Request more frames than available

    auto result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    procBuf->totalSizeInFrame_ = 0;
    EXPECT_EQ(result, ERR_OPERATION_FAILED);

    procBuf->basicBufferInfo_->curWriteFrame.store(8);
    procBuf->basicBufferInfo_->curReadFrame.store(12);
    result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);

    procBuf->totalSizeInFrame_ = 100;
    procBuf->basicBufferInfo_->curWriteFrame.store(0);
    procBuf->basicBufferInfo_->curReadFrame.store(0);
    result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test WriteToSpecialProcBuf API
 * @tc.type  : FUNC
 * @tc.number: WriteToSpecialProcBuf_003
 * @tc.desc  : Test WriteToSpecialProcBuf with GetAllWritableBufferFromPosFrame failure.
 */
HWTEST(AudioProcessInServerUnitTest, WriteToSpecialProcBuf_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService* service = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServer(config, service);
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    EXPECT_NE(procBuf, nullptr);
    EXPECT_EQ(procBuf->Init(-1, -1, 0), SUCCESS);
    procBuf->basicBufferInfo_->curWriteFrame.store(180);
    procBuf->basicBufferInfo_->curReadFrame.store(101);

    audioProcessInServer.processBuffer_ = procBuf;
    audioProcessInServer.spanSizeInframe_ = 20;

    auto result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, ERR_WRITE_FAILED);
}

/**
 * @tc.name  : Test WriteToSpecialProcBuf API
 * @tc.type  : FUNC
 * @tc.number: WriteToSpecialProcBuf_004
 * @tc.desc  : Test WriteToSpecialProcBuf with zero byteSizePerFrame.
 */
HWTEST(AudioProcessInServerUnitTest, WriteToSpecialProcBuf_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService* service = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServer(config, service);
    audioProcessInServer.muteFlag_.store(true);
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    EXPECT_NE(procBuf, nullptr);
    EXPECT_EQ(procBuf->Init(-1, -1, 0), SUCCESS);
    procBuf->basicBufferInfo_->curWriteFrame.store(180);
    procBuf->basicBufferInfo_->curReadFrame.store(101);

    audioProcessInServer.processBuffer_ = procBuf;
    audioProcessInServer.spanSizeInframe_ = 20;
    
    auto result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test WriteToSpecialProcBuf API
 * @tc.type  : FUNC
 * @tc.number: WriteToSpecialProcBuf_005
 * @tc.desc  : Test WriteToSpecialProcBuf with mute state enabled, should set buffer to zero.
 */
HWTEST(AudioProcessInServerUnitTest, WriteToSpecialProcBuf_005, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService* service = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServer(config, service);
    audioProcessInServer.muteFlag_.store(true);
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    EXPECT_NE(procBuf, nullptr);
    EXPECT_EQ(procBuf->Init(-1, -1, 0), SUCCESS);
    audioProcessInServer.processBuffer_ = procBuf;
    audioProcessInServer.spanSizeInframe_ = 50;

    auto result = audioProcessInServer.WriteToSpecialProcBuf(procParams);
    EXPECT_EQ(result, SUCCESS);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test SetCaptureStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureStreamInfo_001
 * @tc.desc  : Test SetCaptureStreamInfo with isConvertReadFormat true, should set format to SAMPLE_F32LE.
 */
HWTEST(AudioProcessInServerUnitTest, SetCaptureStreamInfo_001, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    AudioStreamInfo srcInfo = {};
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.isConvertReadFormat_ = true;
    procParams.srcSamplingRate = SAMPLE_RATE_48000;

    audioProcessInServerRet.SetCaptureStreamInfo(srcInfo, procParams);

    EXPECT_EQ(srcInfo.channels, STEREO);
    EXPECT_EQ(srcInfo.format, SAMPLE_F32LE);
    EXPECT_EQ(srcInfo.samplingRate, procParams.srcSamplingRate);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test SetCaptureStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: SetCaptureStreamInfo_002
 * @tc.desc  : Test SetCaptureStreamInfo with isConvertReadFormat false, should set format to SAMPLE_S16LE.
 */
HWTEST(AudioProcessInServerUnitTest, SetCaptureStreamInfo_002, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    AudioStreamInfo srcInfo = {};
    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.isConvertReadFormat_ = false;
    procParams.srcSamplingRate = SAMPLE_RATE_44100;

    audioProcessInServerRet.SetCaptureStreamInfo(srcInfo, procParams);

    EXPECT_EQ(srcInfo.channels, STEREO);
    EXPECT_EQ(srcInfo.format, SAMPLE_S16LE);
    EXPECT_EQ(srcInfo.samplingRate, procParams.srcSamplingRate);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test CaptureDataResampleProcess API
 * @tc.type  : FUNC
 * @tc.number: CaptureDataResampleProcess_001
 * @tc.desc  : Test CaptureDataResampleProcess with same sample rate and no format conversion.
 */
HWTEST(AudioProcessInServerUnitTest, CaptureDataResampleProcess_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.samplingRate = SAMPLE_RATE_48000;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    size_t bufLen = 1024;
    BufferDesc outBuf = {};
    AudioStreamInfo srcInfo;
    srcInfo.samplingRate = SAMPLE_RATE_48000;
    srcInfo.format = SAMPLE_S16LE;
    srcInfo.channels = STEREO;

    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.isConvertReadFormat_ = false;
    procParams.captureConvBuffer_.resize(bufLen);

    int32_t ret = audioProcessInServerRet.CaptureDataResampleProcess(bufLen, outBuf, srcInfo, procParams);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(outBuf.buffer, nullptr); // No change when no resampling needed
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test CaptureDataResampleProcess API
 * @tc.type  : FUNC
 * @tc.number: CaptureDataResampleProcess_002
 * @tc.desc  : Test CaptureDataResampleProcess with same sample rate and format conversion needed.
 */
HWTEST(AudioProcessInServerUnitTest, CaptureDataResampleProcess_002, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.samplingRate = SAMPLE_RATE_48000;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    size_t bufLen = 1024;
    BufferDesc outBuf;
    outBuf.buffer = new uint8_t[512];
    outBuf.bufLength = 512;
    AudioStreamInfo srcInfo;
    srcInfo.samplingRate = SAMPLE_RATE_48000;
    srcInfo.format = SAMPLE_S16LE;
    srcInfo.channels = STEREO;

    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.isConvertReadFormat_ = true;
    procParams.captureConvBuffer_.resize(bufLen);

    int32_t ret = audioProcessInServerRet.CaptureDataResampleProcess(bufLen, outBuf, srcInfo, procParams);

    EXPECT_EQ(ret, SUCCESS);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test CaptureDataResampleProcess API
 * @tc.type  : FUNC
 * @tc.number: CaptureDataResampleProcess_003
 * @tc.desc  : Test CaptureDataResampleProcess with different sample rates and F32 format.
 */
HWTEST(AudioProcessInServerUnitTest, CaptureDataResampleProcess_003, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.samplingRate = SAMPLE_RATE_44100;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    auto resample = std::make_unique<MockProResampler>();
    EXPECT_CALL(*resample, Process(_, _, _, _))
        .Times(1)
        .WillOnce(Return(SUCCESS));
    audioProcessInServerRet.resampler_ = std::move(resample);
    size_t bufLen = 1024;
    BufferDesc outBuf;
    outBuf.buffer = new uint8_t[512];
    outBuf.bufLength = 512;
    AudioStreamInfo srcInfo;
    srcInfo.samplingRate = SAMPLE_RATE_48000;
    srcInfo.format = SAMPLE_F32LE;
    srcInfo.channels = STEREO;

    BufferDesc readBuf;
    readBuf.buffer = new uint8_t[512];
    readBuf.bufLength = 512;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo = {};
    procBuf->basicBufferInfo_ = &basicBufferInfo;
    std::vector<uint8_t> dataBase;
    dataBase.assign(2048, 0);
    procBuf->dataBase_ = dataBase.data();
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.isConvertReadFormat_ = false;
    procParams.captureConvBuffer_.resize(bufLen);
    procParams.rendererConvBuffer_.resize(bufLen);

    int32_t ret = audioProcessInServerRet.CaptureDataResampleProcess(bufLen, outBuf, srcInfo, procParams);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(outBuf.buffer, nullptr);
    delete readBuf.buffer;
}

/**
 * @tc.name  : Test CapturerDataFormatAndChnConv API
 * @tc.type  : FUNC
 * @tc.number: CapturerDataFormatAndChnConv_001
 * @tc.desc  : Test CapturerDataFormatAndChnConv with supported format conversion.
 */
HWTEST(AudioProcessInServerUnitTest, CapturerDataFormatAndChnConv_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S16LE;
    configRet.streamInfo.channels = STEREO;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    uint8_t* bufA = new uint8_t[2048];
    uint8_t* bufB = new uint8_t[2048];

    RingBufferWrapper writeBuf = {
        .basicBufferDescs = {{
            {bufA, 2048},
            {bufB, 2048}
        }},
        .dataLength = 3000
    };
    BufferDesc resampleOutBuf;
    resampleOutBuf.bufLength = 1024;
    resampleOutBuf.buffer = new uint8_t[resampleOutBuf.bufLength];

    AudioStreamInfo srcInfo;
    srcInfo.format = SAMPLE_F32LE;
    srcInfo.channels = STEREO;

    AudioStreamInfo dstInfo = configRet.streamInfo;

    // Mock format handler
    FormatHandler mockHandler = [](const BufferDesc&, const BufferDesc&, bool& isDoConvert) {
        isDoConvert = true;
        return SUCCESS;
    };
    auto mockHandleOri = FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_F32LE, STEREO, SAMPLE_S16LE}];
    FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_F32LE, STEREO, SAMPLE_S16LE}] = mockHandler;

    int32_t ret = audioProcessInServerRet.CapturerDataFormatAndChnConv(writeBuf, resampleOutBuf, srcInfo, dstInfo);

    EXPECT_EQ(ret, ERR_WRITE_FAILED);
    FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_F32LE, STEREO, SAMPLE_S16LE}] = mockHandleOri;
    delete[] resampleOutBuf.buffer;
    delete[] bufA;
    delete[] bufB;
}

/**
 * @tc.name  : Test CapturerDataFormatAndChnConv API
 * @tc.type  : FUNC
 * @tc.number: CapturerDataFormatAndChnConv_002
 * @tc.desc  : Test CapturerDataFormatAndChnConv with no conversion needed.
 */
HWTEST(AudioProcessInServerUnitTest, CapturerDataFormatAndChnConv_002, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S16LE;
    configRet.streamInfo.channels = STEREO;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    uint8_t* bufA = new uint8_t[2048];
    uint8_t* bufB = new uint8_t[2048];

    RingBufferWrapper writeBuf = {
        .basicBufferDescs = {{
            {bufA, 2048},
            {bufB, 2048}
        }},
        .dataLength = 3000
    };
    BufferDesc resampleOutBuf;
    resampleOutBuf.bufLength = 1024;
    resampleOutBuf.buffer = new uint8_t[resampleOutBuf.bufLength];

    AudioStreamInfo srcInfo;
    srcInfo.format = SAMPLE_S16LE;
    srcInfo.channels = STEREO;

    AudioStreamInfo dstInfo = configRet.streamInfo;

    // Mock format handler that doesn't require conversion
    FormatHandler mockHandler = [](const BufferDesc&, const BufferDesc&, bool& isDoConvert) {
        isDoConvert = false;
        return SUCCESS;
    };
    auto mockHandleOri = FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_S16LE, STEREO, SAMPLE_S16LE}];
    FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_S16LE, STEREO, SAMPLE_S16LE}] = mockHandler;

    int32_t ret = audioProcessInServerRet.CapturerDataFormatAndChnConv(writeBuf, resampleOutBuf, srcInfo, dstInfo);
    EXPECT_EQ(ret, SUCCESS);

    FormatConverter::formatHandlers[FormatKey{STEREO, SAMPLE_S16LE, STEREO, SAMPLE_S16LE}] = mockHandleOri;
    delete[] resampleOutBuf.buffer;
    delete[] bufA;
    delete[] bufB;
}

/**
 * @tc.name  : Test CapturerDataFormatAndChnConv API
 * @tc.type  : FUNC
 * @tc.number: CapturerDataFormatAndChnConv_003
 * @tc.desc  : Test CapturerDataFormatAndChnConv with unsupported format conversion.
 */
HWTEST(AudioProcessInServerUnitTest, CapturerDataFormatAndChnConv_003, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S16LE;
    configRet.streamInfo.channels = STEREO;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);

    uint8_t* bufA = new uint8_t[2048];
    uint8_t* bufB = new uint8_t[2048];

    RingBufferWrapper writeBuf = {
        .basicBufferDescs = {{
            {bufA, 2048},
            {bufB, 2048}
        }},
        .dataLength = 3000
    };
    BufferDesc resampleOutBuf;
    resampleOutBuf.bufLength = 1024;
    resampleOutBuf.buffer = new uint8_t[resampleOutBuf.bufLength];

    AudioStreamInfo srcInfo;
    srcInfo.format = SAMPLE_S24LE;
    srcInfo.channels = STEREO;

    AudioStreamInfo dstInfo = configRet.streamInfo;

    FormatHandlerMap formatHandlersOri = FormatConverter::GetFormatHandlers();
    FormatConverter::GetFormatHandlers().clear();
    int32_t ret = audioProcessInServerRet.CapturerDataFormatAndChnConv(writeBuf, resampleOutBuf, srcInfo, dstInfo);

    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    FormatConverter::GetFormatHandlers() = formatHandlersOri;
    delete[] bufA;
    delete[] bufB;
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_001
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, RebuildCaptureInjector_001, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_002
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, RebuildCaptureInjector_002, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_001
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, ReleaseCaptureInjector_001, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_RECORD;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_002
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, ReleaseCaptureInjector_002, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_003
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, ReleaseCaptureInjector_003, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_RECORD;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_004
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, ReleaseCaptureInjector_004, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test HandleCapturerDataParams API
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_003
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, RebuildCaptureInjector_003, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test AudioProcessInServer API
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_004
 * @tc.desc  : Test AudioProcessInServer interface.
 */
HWTEST(AudioProcessInServerUnitTest, RebuildCaptureInjector_004, TestSize.Level1)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.audioMode = AUDIO_MODE_PLAYBACK;
    configRet.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    EXPECT_NE(audioProcessInServer, nullptr);
    audioProcessInServer->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test HandleCapturerDataParams API
 * @tc.type  : FUNC
 * @tc.number: HandleCapturerDataParams_001
 * @tc.desc  : Test HandleCapturerDataParams with successful processing.
 */
HWTEST(AudioProcessInServerUnitTest, HandleCapturerDataParams_001, TestSize.Level2)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.streamInfo.format = SAMPLE_S16LE;
    configRet.streamInfo.channels = STEREO;
    configRet.streamInfo.samplingRate = SAMPLE_RATE_44100;
    AudioService* releaseCallbackRet = AudioService::GetInstance();
    AudioProcessInServer audioProcessInServerRet(configRet, releaseCallbackRet);
    uint8_t* bufA = new uint8_t[2048];
    uint8_t* bufB = new uint8_t[2048];

    RingBufferWrapper writeBuf = {
        .basicBufferDescs = {{
            {bufA, 2048},
            {bufB, 2048}
        }},
        .dataLength = 3000
    };
    BufferDesc readBuf;
    readBuf.bufLength = 512;
    readBuf.buffer = new uint8_t[readBuf.bufLength];
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t byteSizePerFrame = 100;
    uint32_t totalSizeInFrame = 100;
    auto procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    std::vector<uint8_t> captureConvBuffer;
    std::vector<uint8_t> rendererConvBuffer;
    AudioCaptureDataProcParams procParams(
        readBuf,
        captureConvBuffer,
        rendererConvBuffer
    );
    procParams.srcSamplingRate = SAMPLE_RATE_48000;
    procParams.isConvertReadFormat_ = false;
    procParams.captureConvBuffer_.resize(1024);

    int32_t ret = audioProcessInServerRet.HandleCapturerDataParams(writeBuf, procParams);

    EXPECT_NE(ret, SUCCESS);

    delete[] procParams.readBuf_.buffer;
    delete[] bufA;
    delete[] bufB;
}
} // namespace AudioStandard
} // namespace OHOS
