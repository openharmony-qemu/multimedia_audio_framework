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
#include "audio_service.h"
#include "audio_errors.h"
#include "audio_process_in_client.h"
#include "audio_process_in_client.cpp"
#include "fast_audio_stream.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioProcessInClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

constexpr int32_t DEFAULT_STREAM_ID = 10;
constexpr size_t NUMBER1 = 1;
constexpr size_t NUMBER4 = 4;
constexpr size_t NUMBER6 = 6;

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
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
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_001
 * @tc.desc  : Test AudioProcessInClientInner::SetPreferredFrameSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int32_t frameSize = 0;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_002
 * @tc.desc  : Test AudioProcessInClientInner::SetPreferredFrameSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int32_t frameSize = 10;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 1;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 10;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_003
 * @tc.desc  : Test AudioProcessInClientInner::SetPreferredFrameSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int32_t frameSize = 10;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 5;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 10;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_004
 * @tc.desc  : Test static GetFormatSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_004, TestSize.Level1)
{
    AudioStreamInfo info;
    info.format = AudioSampleFormat::SAMPLE_U8;
    info.channels = AudioChannel::MONO;
    auto ret = GetFormatSize(info);

    EXPECT_EQ(ret, NUMBER1);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_005
 * @tc.desc  : Test static GetFormatSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_005, TestSize.Level1)
{
    AudioStreamInfo info;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.channels = AudioChannel::STEREO;
    auto ret = GetFormatSize(info);

    EXPECT_EQ(ret, NUMBER4);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_006
 * @tc.desc  : Test static GetFormatSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_006, TestSize.Level1)
{
    AudioStreamInfo info;
    info.format = AudioSampleFormat::SAMPLE_S24LE;
    info.channels = AudioChannel::CHANNEL_3;
    auto ret = GetFormatSize(info);

    EXPECT_EQ(ret, NUMBER6);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_007
 * @tc.desc  : Test static GetFormatSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_007, TestSize.Level1)
{
    AudioStreamInfo info;
    info.format = AudioSampleFormat::SAMPLE_S32LE;
    info.channels = AudioChannel::MONO;
    auto ret = GetFormatSize(info);

    EXPECT_EQ(ret, NUMBER4);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_008
 * @tc.desc  : Test static GetFormatSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_008, TestSize.Level1)
{
    AudioStreamInfo info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.channels = AudioChannel::MONO;
    auto ret = GetFormatSize(info);

    EXPECT_EQ(ret, NUMBER4);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_009
 * @tc.desc  : Test inline S32MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_009, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 2;
    auto ret = S32MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_010
 * @tc.desc  : Test inline S32MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_010, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    srcDesc.buffer = nullptr;
    auto ret = S32MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_011
 * @tc.desc  : Test inline S32MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_011, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = nullptr;
    auto ret = S32MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_012
 * @tc.desc  : Test inline S32MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_012, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S32MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_013
 * @tc.desc  : Test inline S32MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_013, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S32MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_014
 * @tc.desc  : Test inline S32StereoS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_014, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S32StereoS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_015
 * @tc.desc  : Test inline S32StereoS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_015, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 2;
    srcDesc.buffer = nullptr;
    auto ret = S32StereoS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_016
 * @tc.desc  : Test inline S32StereoS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_016, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 2;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = nullptr;
    auto ret = S32StereoS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_017
 * @tc.desc  : Test inline S32StereoS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_017, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 2;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S32StereoS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_018
 * @tc.desc  : Test inline S32StereoS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_018, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 6;
    dstDesc.bufLength = 3;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S32StereoS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_019
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_019, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_IDEL;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_IDEL");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_020
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_020, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_STARTING;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_STARTING");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_021
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_021, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_RUNNING;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_RUNNING");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_022
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_022, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_PAUSING;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_PAUSING");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_023
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_023, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_PAUSED;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_PAUSED");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_024
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_024, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_STOPPING;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_STOPPING");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_025
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_025, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_STOPPED;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_STOPPED");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_026
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_026, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_RELEASED;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_RELEASED");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_027
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_027, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_INVALID;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "STREAM_INVALID");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_028
 * @tc.desc  : Test AudioProcessInClientInner::GetStatusInfo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_028, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    StreamStatus status = StreamStatus::STREAM_STAND_BY;
    auto ret = ptrAudioProcessInClientInner->GetStatusInfo(status);
    EXPECT_EQ(ret, "NO_SUCH_STATUS");
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_029
 * @tc.desc  : Test AudioProcessInClientInner::KeepLoopRunning
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_029, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    auto ret = ptrAudioProcessInClientInner->KeepLoopRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_030
 * @tc.desc  : Test AudioProcessInClientInner::KeepLoopRunning
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_030, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_STAND_BY);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    auto ret = ptrAudioProcessInClientInner->KeepLoopRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_033
 * @tc.desc  : Test AudioProcessInClientInner::KeepLoopRunning
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_033, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_PAUSING);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    ptrAudioProcessInClientInner->startFadeout_.store(true);
    auto ret = ptrAudioProcessInClientInner->KeepLoopRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_035
 * @tc.desc  : Test AudioProcessInClientInner::KeepLoopRunning
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_035, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_STOPPING);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    ptrAudioProcessInClientInner->startFadeout_.store(true);
    auto ret = ptrAudioProcessInClientInner->KeepLoopRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_036
 * @tc.desc  : Test AudioProcessInClientInner::KeepLoopRunning
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_036, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_STOPPED);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    ptrAudioProcessInClientInner->startFadeout_.store(true);
    auto ret = ptrAudioProcessInClientInner->KeepLoopRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_038
 * @tc.desc  : Test AudioProcessInClientInner::DoFadeInOut
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_038, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t curWritePos = 0;
    ptrAudioProcessInClientInner->startFadein_.store(true);
    ptrAudioProcessInClientInner->startFadeout_.store(true);

    ptrAudioProcessInClientInner->DoFadeInOut(curWritePos);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_039
 * @tc.desc  : Test AudioProcessInClientInner::DoFadeInOut
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_039, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t curWritePos = 0;
    ptrAudioProcessInClientInner->startFadein_.store(false);
    ptrAudioProcessInClientInner->startFadeout_.store(true);

    ptrAudioProcessInClientInner->DoFadeInOut(curWritePos);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_040
 * @tc.desc  : Test AudioProcessInClientInner::DoFadeInOut
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_040, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t curWritePos = 0;
    ptrAudioProcessInClientInner->startFadein_.store(false);
    ptrAudioProcessInClientInner->startFadeout_.store(false);

    ptrAudioProcessInClientInner->DoFadeInOut(curWritePos);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_044
 * @tc.desc  : Test AudioProcessInClientInner::CheckIfWakeUpTooLate
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_044, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int64_t curTime = 1000100;
    int64_t wakeUpTime = 0;

    ptrAudioProcessInClientInner->CheckIfWakeUpTooLate(curTime, wakeUpTime);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_045
 * @tc.desc  : Test AudioProcessInClientInner::CheckIfWakeUpTooLate
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_045, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int64_t curTime = 1000;
    int64_t wakeUpTime = 0;

    ptrAudioProcessInClientInner->CheckIfWakeUpTooLate(curTime, wakeUpTime);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_046
 * @tc.desc  : Test AudioProcessInClientInner::CheckIfWakeUpTooLate
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_046, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int64_t curTime = 1000100;
    int64_t wakeUpTime = 0;
    int64_t clientWriteCost = 0;

    ptrAudioProcessInClientInner->CheckIfWakeUpTooLate(curTime, wakeUpTime, clientWriteCost);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_047
 * @tc.desc  : Test AudioProcessInClientInner::CheckIfWakeUpTooLate
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_047, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    int64_t curTime = 100;
    int64_t wakeUpTime = 0;
    int64_t clientWriteCost = 1000100;

    ptrAudioProcessInClientInner->CheckIfWakeUpTooLate(curTime, wakeUpTime, clientWriteCost);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_048
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_048, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->KeepLoopRunningIndependent());
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_049
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_049, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_IDEL);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->KeepLoopRunningIndependent());
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_050
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_050, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_PAUSED);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->KeepLoopRunningIndependent());
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_051
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_051, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_INVALID);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->KeepLoopRunningIndependent());
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_052
 * @tc.desc  : Test inline S16MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_052, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 2;
    auto ret = S16MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_053
 * @tc.desc  : Test inline S16MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_053, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    srcDesc.buffer = nullptr;
    auto ret = S16MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_054
 * @tc.desc  : Test inline S16MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_054, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = nullptr;
    auto ret = S16MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_055
 * @tc.desc  : Test inline S16MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_055, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S16MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_056
 * @tc.desc  : Test inline S16MonoToS16Stereo
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_056, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 4;
    dstDesc.bufLength = 1;
    uint8_t buffer = 0;
    srcDesc.buffer = &buffer;
    dstDesc.buffer = &buffer;
    auto ret = S16MonoToS16Stereo(srcDesc, dstDesc);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_057
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_057, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_058
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_058, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_059
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_059, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.streamInfo.samplingRate = SAMPLE_RATE_8000;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_060
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_060, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_061
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_061, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.streamInfo.format = SAMPLE_S24LE;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_062
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_062, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.streamInfo.channels = CHANNEL_3;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_063
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_063, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioProcConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    audioProcConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    audioProcConfig.streamInfo.encoding = ENCODING_PCM;
    audioProcConfig.streamInfo.format = SAMPLE_S16LE;
    audioProcConfig.streamInfo.channels = MONO;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_064
 * @tc.desc  : Test SetMute, SetDuckVolume, SetUnderflowCount, GetUnderflowCount, SetOverflowCount, GetOverflowCount
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_064, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    int32_t ret = ptrAudioProcessInClientInner->SetMute(true);
    EXPECT_EQ(0, ret);
    ret = ptrAudioProcessInClientInner->SetDuckVolume(0.5f);
    EXPECT_EQ(0, ret);
    ptrAudioProcessInClientInner->underflowCount_ = 0;
    ptrAudioProcessInClientInner->SetUnderflowCount(1);
    uint32_t res = ptrAudioProcessInClientInner->GetUnderflowCount();
    EXPECT_EQ(1, res);
    ptrAudioProcessInClientInner->overflowCount_ = 0;
    ptrAudioProcessInClientInner->SetOverflowCount(1);
    res = ptrAudioProcessInClientInner->GetOverflowCount();
    EXPECT_EQ(1, res);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_065
 * @tc.desc  : Test GetFramesWritten, GetFramesRead, UpdateLatencyTimestamp
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_065, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    ptrAudioProcessInClientInner->processConfig_.audioMode = AUDIO_MODE_PLAYBACK;
    int64_t res = 0;
    res = ptrAudioProcessInClientInner->GetFramesWritten();
    EXPECT_EQ(res, -1);
    ptrAudioProcessInClientInner->processConfig_.audioMode = AUDIO_MODE_RECORD;
    res = ptrAudioProcessInClientInner->GetFramesRead();
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_066
 * @tc.desc  : Test SaveUnderrunCallback
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_066, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    std::shared_ptr<ClientUnderrunCallBack> underrunCallback = nullptr;
    ptrAudioProcessInClientInner->isInited_ = false;
    int32_t ret = ptrAudioProcessInClientInner->SaveUnderrunCallback(underrunCallback);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_067
 * @tc.desc  : Test ChannelFormatConvert
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_067, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.samplingRate = SAMPLE_RATE_16000;
    dstData.streamInfo.samplingRate = SAMPLE_RATE_48000;
    bool ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    dstData.streamInfo.samplingRate = SAMPLE_RATE_16000;
    srcData.streamInfo.format = SAMPLE_S16LE;
    srcData.streamInfo.channels = STEREO;
    dstData.streamInfo.encoding = ENCODING_AUDIOVIVID;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    dstData.streamInfo.encoding = ENCODING_PCM;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(true, ret);
    srcData.streamInfo.channels = MONO;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.channels = CHANNEL_3;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.format = SAMPLE_S32LE;
    srcData.streamInfo.channels = MONO;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.channels = CHANNEL_3;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.channels = STEREO;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.format = INVALID_WIDTH;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.format = SAMPLE_F32LE;
    srcData.streamInfo.channels = MONO;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(true, ret);
    srcData.streamInfo.channels = STEREO;
    ret = ptrAudioProcessInClientInner->ChannelFormatConvert(srcData, dstData);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_068
 * @tc.desc  : Test Pause, Resume
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_068, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    int32_t ret = ptrAudioProcessInClientInner->Pause(true);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = ptrAudioProcessInClientInner->Pause(true);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = ptrAudioProcessInClientInner->Resume();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_069
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_069, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig;
    audioProcConfig.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioProcConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    audioProcConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    audioProcConfig.streamInfo.encoding = ENCODING_AUDIOVIVID;
    audioProcConfig.streamInfo.channels = MONO;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
    audioProcConfig.streamInfo.encoding = ENCODING_PCM;
    audioProcConfig.streamInfo.format = SAMPLE_S24LE;
    EXPECT_EQ(false, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
    audioProcConfig.streamInfo.format = SAMPLE_S32LE;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_070
 * @tc.desc  : Test CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_070, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = false;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    bool ret = ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);
    EXPECT_EQ(ret, false);
}
} // namespace AudioStandard
} // namespace OHOS