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
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_service.h"
#include "audio_errors.h"
#include "audio_process_in_client.h"
#include "audio_process_in_client.cpp"
#include "fast_audio_stream.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
class MockIAudioProcess : public IAudioProcess {
public:
    MOCK_METHOD(int32_t, ResolveBufferBaseAndGetServerSpanSize,
        (std::shared_ptr<OHAudioBufferBase> buffer, uint32_t &spanSizeInFrame), (override));

    MOCK_METHOD(int32_t, GetSessionId, (uint32_t &sessionId), (override));
    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Pause, (bool isFlush), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));

    MOCK_METHOD(int32_t, Stop, (int32_t stage), (override));

    MOCK_METHOD(int32_t, RequestHandleInfo, (), (override));

    MOCK_METHOD(int32_t, RequestHandleInfoAsync, (), (override));

    MOCK_METHOD(int32_t, Release, (bool isSwitchStream), (override));

    MOCK_METHOD(int32_t, RegisterProcessCb, (const &sptr<object>), (override));

    MOCK_METHOD(int32_t, RegisterThreadPriority,
        (int32_t tid, const std::string &bundleName, uint32_t method), (override));

    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (int32_t defaultOutputDevice, bool skipForce), (override));
    MOCK_METHOD(int32_t, SetSilentModeAndMixWithOthers, (bool on), (override));
    MOCK_METHOD(int32_t, SetSourceDuration, (int64_t duration), (override));
    MOCK_METHOD(int32_t, SetUnderrunCount, (uint32_t underrunCnt), (override));

    MOCK_METHOD(int32_t, SaveAdjustStreamVolumeInfo,
        (float volume, uint32_t sessionId, const std::string &adjustTime, uint32_t code), (override));

    MOCK_METHOD(int32_t, SetAudioHapticsSyncId, (int32_t audioHapticsSyncId), (override));

    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
};

class AudioProcessInClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

constexpr int32_t DEFAULT_STREAM_ID = 10;
constexpr size_t NUMBER1 = 1;
constexpr size_t NUMBER2 = 2;
constexpr size_t NUMBER4 = 4;
constexpr size_t NUMBER6 = 6;
constexpr size_t NUMBER8 = 8;

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

class ClientUnderrunCallBackTest : public ClientUnderrunCallBack {
    virtual ~ClientUnderrunCallBackTest() = default;

    /**
     * Callback function when underrun occurs.
     *
     * @param posInFrames Indicates the postion when client handle underrun in frames.
     */
    virtual void OnUnderrun(size_t posInFrames) {}
};

class AudioDataCallbackTest : public AudioDataCallback {
public:
    virtual ~AudioDataCallbackTest() = default;

    /**
     * Called when request handle data.
     *
     * @param length Indicates requested buffer length.
     */
    virtual void OnHandleData(size_t length) {}
};

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    auto ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc) == 0;

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    auto ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    auto ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc) == 0;

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioProcessConfig audioProcConfig = {0};
    audioProcConfig.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    EXPECT_EQ(true, ptrAudioProcessInClientInner->CheckIfSupport(audioProcConfig));
    audioProcConfig.rendererInfo.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
 * @tc.desc  : Test ChannelFormatS16Convert
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_067, TestSize.Level1)
{
    sptr<AudioProcessInServer> stream = AudioProcessInServer::Create(InitProcessConfig(), AudioService::GetInstance());

    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(stream, true, info);

    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.samplingRate = SAMPLE_RATE_16000;
    dstData.streamInfo.samplingRate = SAMPLE_RATE_48000;
    bool ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    dstData.streamInfo.samplingRate = SAMPLE_RATE_16000;
    srcData.streamInfo.format = SAMPLE_S16LE;
    srcData.streamInfo.channels = STEREO;
    dstData.streamInfo.encoding = ENCODING_AUDIOVIVID;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    dstData.streamInfo.encoding = ENCODING_PCM;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(true, ret);
    srcData.streamInfo.channels = MONO;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);

    srcData.streamInfo.format = SAMPLE_S32LE;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.channels = CHANNEL_3;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.channels = STEREO;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.format = INVALID_WIDTH;
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
    srcData.streamInfo.format = SAMPLE_F32LE;
    srcData.streamInfo.channels = MONO;
    uint8_t src1[NUMBER4] = {0};
    srcData.bufferDesc = {src1, NUMBER4, NUMBER4};
    uint8_t dst[NUMBER4] = {0};
    dstData.bufferDesc = {dst, NUMBER4, NUMBER4};
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(true, ret);
    srcData.streamInfo.channels = STEREO;
    uint8_t src2[NUMBER8] = {0};
    srcData.bufferDesc = {src2, NUMBER8, NUMBER8};
    ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

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
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    bool ret = ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_072
 * @tc.desc  : Test AudioProcessInClientInner::GetAudioTime
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_072, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->curReadFrame.store(UINT32_MAX + 1);
    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);

    uint32_t framePos = 0;
    int64_t sec = 0;
    int64_t nanoSec = 0;
    auto ret = ptrAudioProcessInClientInner->GetAudioTime(framePos, sec, nanoSec);
    delete ptrBufferInfo;
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_073
 * @tc.desc  : Test AudioProcessInClientInner::GetFramesWritten
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_073, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);
    ptrAudioProcessInClientInner->processConfig_.audioMode = AUDIO_MODE_PLAYBACK;
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->curWriteFrame.store(0);

    auto ret = ptrAudioProcessInClientInner->GetFramesWritten();
    delete ptrBufferInfo;
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_074
 * @tc.desc  : Test AudioProcessInClientInner::GetFramesRead
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_074, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->curReadFrame.store(0);

    auto ret = ptrAudioProcessInClientInner->GetFramesRead();
    delete ptrBufferInfo;
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_075
 * @tc.desc  : Test AudioProcessInClientInner::UpdateLatencyTimestamp
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_075, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    std::string timestamp = "";
    bool isRenderer = true;
    ptrAudioProcessInClientInner->UpdateLatencyTimestamp(timestamp, isRenderer);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_076
 * @tc.desc  : Test AudioProcessInClientInner::Init
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_076, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioProcessConfig audioProcessConfig;
    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    auto ret = ptrAudioProcessInClientInner->Init(audioProcessConfig, ptrFastAudioStream);
    EXPECT_NE(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_077
 * @tc.desc  : Test AudioProcessInClientInner::SaveUnderrunCallback
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_077, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->isInited_ = true;
    std::shared_ptr<ClientUnderrunCallBack> underrunCallback;
    underrunCallback = std::make_shared<ClientUnderrunCallBackTest>();
    ASSERT_TRUE(underrunCallback != nullptr);

    auto ret = ptrAudioProcessInClientInner->SaveUnderrunCallback(underrunCallback);
    ptrAudioProcessInClientInner->isInited_ = false;
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_078
 * @tc.desc  : Test AudioProcessInClientInner::CheckIfSupport
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_078, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.appInfo.appPid = getpid();
    audioProcessConfig.appInfo.appUid = getuid();
    audioProcessConfig.audioMode = AUDIO_MODE_RECORD;
    audioProcessConfig.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    audioProcessConfig.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    audioProcessConfig.streamInfo.channels = STEREO;
    audioProcessConfig.streamInfo.encoding = ENCODING_PCM;
    audioProcessConfig.streamInfo.format = SAMPLE_S16LE;
    audioProcessConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;

    auto ret = ptrAudioProcessInClientInner->CheckIfSupport(audioProcessConfig);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_079
 * @tc.desc  : Test AudioProcessInClientInner::ChannelFormatS16Convert
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_079, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioStreamData dstData;
    AudioStreamData srcData;
    srcData.streamInfo.format = SAMPLE_F32LE;
    srcData.streamInfo.channels = MONO;
    dstData.streamInfo.format = SAMPLE_S16LE;
    dstData.streamInfo.channels = STEREO;

    uint8_t src1[NUMBER4] = {0};
    srcData.bufferDesc = {src1, NUMBER4, NUMBER4};
    uint8_t dst[NUMBER4] = {0};
    dstData.bufferDesc = {dst, NUMBER4, NUMBER4};
    auto ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_080
 * @tc.desc  : Test AudioProcessInClientInner::ChannelFormatS16Convert
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_080, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioStreamData dstData;
    AudioStreamData srcData;
    srcData.streamInfo.format = SAMPLE_F32LE;
    srcData.streamInfo.channels = STEREO;
    dstData.streamInfo.format = SAMPLE_S16LE;
    dstData.streamInfo.channels = STEREO;

    uint8_t src1[NUMBER8] = {0};
    srcData.bufferDesc = {src1, NUMBER8, NUMBER8};
    uint8_t dst[NUMBER4] = {0};
    dstData.bufferDesc = {dst, NUMBER4, NUMBER4};
    auto ret = ptrAudioProcessInClientInner->ChannelFormatS16Convert(srcData, dstData);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_081
 * @tc.desc  : Test AudioProcessInClientInner::CopyWithVolume
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_081, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc srcDesc;
    BufferDesc dstDesc;
    ptrAudioProcessInClientInner->CopyWithVolume(srcDesc, dstDesc);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_082
 * @tc.desc  : Test AudioProcessInClientInner::ProcessData
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_082, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_INDEPENDENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    BufferDesc srcDesc;
    BufferDesc dstDesc;
    auto ret = ptrAudioProcessInClientInner->ProcessData(srcDesc, dstDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_083
 * @tc.desc  : Test AudioProcessInClientInner::ProcessData
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_083, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    BufferDesc srcDesc;
    BufferDesc dstDesc;
    auto ret = ptrAudioProcessInClientInner->ProcessData(srcDesc, dstDesc);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_084
 * @tc.desc  : Test AudioProcessInClientInner::Pause
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_084, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->isInited_ = true;
    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_PAUSED);
    auto ptrStreamStatus = ptrAudioProcessInClientInner->streamStatus_;
    ASSERT_TRUE(ptrStreamStatus != nullptr);

    bool isFlush = true;
    auto ret = ptrAudioProcessInClientInner->Pause(isFlush);
    EXPECT_EQ(ret, SUCCESS);

    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_STAND_BY);
    ret = ptrAudioProcessInClientInner->Pause(isFlush);
    EXPECT_NE(ret, SUCCESS);

    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_RUNNING);
    ret = ptrAudioProcessInClientInner->Pause(isFlush);
    ptrAudioProcessInClientInner->isInited_ = false;
    delete ptrStreamStatus;
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_085
 * @tc.desc  : Test AudioProcessInClientInner::Resume
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_085, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->isInited_ = true;
    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_RUNNING);
    auto ptrStreamStatus = ptrAudioProcessInClientInner->streamStatus_;
    ASSERT_TRUE(ptrStreamStatus != nullptr);

    auto ret = ptrAudioProcessInClientInner->Resume();
    ptrAudioProcessInClientInner->isInited_ = false;
    delete ptrStreamStatus;
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_087
 * @tc.desc  : Test AudioProcessInClientInner::PrepareCurrent
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_087, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_088
 * @tc.desc  : Test AudioProcessInClientInner::PrepareCurrentLoop
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_088, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_089
 * @tc.desc  : Test AudioProcessInClientInner::PrepareCurrentLoop
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_089, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);

    ptrAudioProcessInClientInner->spanSizeInFrame_ = 1;
    ptrAudioProcessInClientInner->clientSpanSizeInFrame_ = 0;
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_090
 * @tc.desc  : Test AudioProcessInClientInner::FinishHandleCurrent
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_090, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_091
 * @tc.desc  : Test AudioProcessInClientInner::FinishHandleCurrentLoop
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_091, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_092
 * @tc.desc  : Test AudioProcessInClientInner::FinishHandleCurrentLoop
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_092, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->basePosInFrame.store(0);

    ptrAudioProcessInClientInner->spanSizeInFrame_ = 1;
    ptrAudioProcessInClientInner->clientSpanSizeInFrame_ = 0;
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_093
 * @tc.desc  : Test AudioProcessInClientInner::ProcessCallbackFuc
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_093, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    uint64_t curWritePos = 0;

    ptrAudioProcessInClientInner->isCallbackLoopEnd_ = true;
    ptrAudioProcessInClientInner->startFadeout_.store(false);
    auto ret = ptrAudioProcessInClientInner->ProcessCallbackFuc(curWritePos);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_094
 * @tc.desc  : Test AudioProcessInClientInner::ProcessCallbackFuc
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_094, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    ptrBufferInfo->curWriteFrame.store(0);

    ptrAudioProcessInClientInner->isCallbackLoopEnd_ = false;
    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_RUNNING);
    auto ptrStreamStatus = ptrAudioProcessInClientInner->streamStatus_;
    ASSERT_TRUE(ptrStreamStatus != nullptr);

    uint64_t curWritePos = 0;
    auto ret = ptrAudioProcessInClientInner->ProcessCallbackFuc(curWritePos);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_095
 * @tc.desc  : Test AudioProcessInClientInner::SetSilentModeAndMixWithOthers
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_095, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    bool on = true;
    auto ret = ptrAudioProcessInClientInner->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(ret, SUCCESS);

    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    ret = ptrAudioProcessInClientInner->SetSilentModeAndMixWithOthers(on);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_096
 * @tc.desc  : Test AudioProcessInClientInner::SetDefaultOutputDevice
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_096, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    DeviceType defaultOutputDevice = DEVICE_TYPE_NONE;
    auto ret = ptrAudioProcessInClientInner->SetSilentModeAndMixWithOthers(defaultOutputDevice);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_097
 * @tc.desc  : Test AudioProcessInClientInner::SaveDataCallback
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_097, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto audioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    EXPECT_NE(audioProcessInClientInner, nullptr);

    std::shared_ptr<AudioDataCallback> audioDataCallbackTest = nullptr;
    audioDataCallbackTest = std::make_shared<AudioDataCallbackTest>();

    audioProcessInClientInner->isInited_ = false;
    auto ret = audioProcessInClientInner->SaveDataCallback(audioDataCallbackTest);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_098
 * @tc.desc  : Test AudioProcessInClientInner::GetBufferDesc_001
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_098, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto audioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    EXPECT_NE(audioProcessInClientInner, nullptr);

    BufferDesc bufDesc;

    audioProcessInClientInner->isInited_ = false;
    auto ret = audioProcessInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_099
 * @tc.desc  : Test AudioProcessInClientInner::GetBufferDesc_002
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_099, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto audioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    EXPECT_NE(audioProcessInClientInner, nullptr);

    BufferDesc bufDesc;
    audioProcessInClientInner->clientSpanSizeInByte_ = 1024;
    audioProcessInClientInner->callbackBuffer_ =
        std::make_unique<uint8_t[]>(audioProcessInClientInner->clientSpanSizeInByte_);
    audioProcessInClientInner->processConfig_.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    auto ret = audioProcessInClientInner->GetBufferDesc(bufDesc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_100
 * @tc.desc  : Test AudioProcessInClientInner::Enqueue
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_100, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer = new uint8_t[ptrAudioProcessInClientInner->clientSpanSizeInByte_];
    bufDesc.bufLength = ptrAudioProcessInClientInner->clientSpanSizeInByte_;
    bufDesc.dataLength = ptrAudioProcessInClientInner->clientSpanSizeInByte_;

    auto ret = ptrAudioProcessInClientInner->Enqueue(bufDesc);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_101
 * @tc.desc  : Test AudioProcessInClientInner::SetVolume
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_101, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    float volume = 0.5f;
    auto ret = ptrAudioProcessInClientInner->SetVolume(volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_102
 * @tc.desc  : Test AudioProcessInClientInner::Start
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_102, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    auto ret = ptrAudioProcessInClientInner->Start();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_103
 * @tc.desc  : Test AudioProcessInClientInner::Stop
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_103, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->isInited_ = false;
    auto ret = ptrAudioProcessInClientInner->Stop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ptrAudioProcessInClientInner->isInited_ = true;
    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_STOPPED);
    ret = ptrAudioProcessInClientInner->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_RUNNING);
    ret = ptrAudioProcessInClientInner->Stop();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_104
 * @tc.desc  : Test AudioProcessInClientInner::Release
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_104, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->isInited_ = false;
    auto ret = ptrAudioProcessInClientInner->Release(false);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ptrAudioProcessInClientInner->isInited_ = true;
    ptrAudioProcessInClientInner->streamStatus_ = new std::atomic<StreamStatus>(StreamStatus::STREAM_RUNNING);
    ret = ptrAudioProcessInClientInner->Release(false);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_105
 * @tc.desc  : Test AudioProcessInClientInner::GetSessionID
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_105, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    uint32_t sessionId = 0;
    auto ret = ptrAudioProcessInClientInner->GetSessionID(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_106
 * @tc.desc  : Test AudioProcessInClientInner::GetBufferSize
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_106, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    uint32_t bufferSize = 0;
    auto ret = ptrAudioProcessInClientInner->GetBufferSize(bufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_107
 * @tc.desc  : Test AudioProcessInClientInner::GetFrameCount
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_107, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    uint32_t frameCount = 0;
    auto ret = ptrAudioProcessInClientInner->GetFrameCount(frameCount);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_108
 * @tc.desc  : Test AudioProcessInClientInner::GetAudioServerProxy
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_108, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    auto ret = AudioProcessInClientInner::GetAudioServerProxy();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_109
 * @tc.desc  : Test AudioProcessInClientInner::InitAudioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_109, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    auto ret = ptrAudioProcessInClientInner->InitAudioBuffer();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_110
 * @tc.desc  : Test AudioProcessInClientInner::ReadFromProcessClient
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_110, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    auto ret = ptrAudioProcessInClientInner->ReadFromProcessClient();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_001
 * @tc.desc  : Test Convert success
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_001, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};
    uint8_t srcArray[NUMBER8] = {0};
    srcData.bufferDesc = {srcArray, NUMBER8, NUMBER8};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_002
 * @tc.desc  : Test sample rate or encoding different
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_002, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t srcArray[NUMBER8] = {0};
    srcData.bufferDesc = {srcArray, NUMBER8, NUMBER8};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_16000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    // samplingRate diff
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret);

    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_AUDIOVIVID, SAMPLE_S32LE, STEREO};
    // encoding diff
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_003
 * @tc.desc  : Test convert SAMPLE_S16LE STEREO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_003, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_004
 * @tc.desc  : Test convert SAMPLE_S16LE MONO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_004, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO};
    uint8_t srcArray[NUMBER2] = {0};
    srcData.bufferDesc = {srcArray, NUMBER2, NUMBER2};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_005
 * @tc.desc  : Test convert SAMPLE_S32LE MONO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_005, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, MONO};
    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_006
 * @tc.desc  : Test convert SAMPLE_S32LE STEREO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_006, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t srcArray[NUMBER8] = {0};
    srcData.bufferDesc = {srcArray, NUMBER8, NUMBER8};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_007
 * @tc.desc  : Test convert SAMPLE_F32LE STEREO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_007, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, MONO};
    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    uint8_t dstArray[NUMBER8] = {0};
    dstData.bufferDesc = {dstArray, NUMBER8, NUMBER8};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(true, ret) << "convert failed, check format";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_008
 * @tc.desc  : Test convert SAMPLE_F32LE STEREO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_008, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER8;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength not equel fail";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer nullptr fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_009
 * @tc.desc  : Test convert SAMPLE_F32LE STEREO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_009, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    dstData.bufferDesc.bufLength = NUMBER8;

    uint8_t srcArray[NUMBER8] = {0};
    srcData.bufferDesc = {srcArray, NUMBER8, NUMBER8};
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer nullptr fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_010
 * @tc.desc  : Test convert SAMPLE_S16LE STEREO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_010, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER4;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength failed";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer is nullptr should fail";

    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer is nullptr should fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_011
 * @tc.desc  : Test convert SAMPLE_S16LE MONO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_011, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, MONO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER2;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength failed";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer is nullptr should fail";

    uint8_t srcArray[NUMBER2] = {0};
    srcData.bufferDesc = {srcArray, NUMBER2, NUMBER2};
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer is nullptr should fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_012
 * @tc.desc  : Test convert SAMPLE_S32LE MONO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_012, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, MONO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER4;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength failed";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer is nullptr should fail";

    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer is nullptr should fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_013
 * @tc.desc  : Test convert SAMPLE_S32LE CHANNEL_3 failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_013, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, CHANNEL_3};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "CHANNEL_3 not supported";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_014
 * @tc.desc  : Test convert SAMPLE_F32LE MONO failed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_014, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, MONO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER4;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength failed";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer is nullptr should fail";

    uint8_t srcArray[NUMBER4] = {0};
    srcData.bufferDesc = {srcArray, NUMBER4, NUMBER4};
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer is nullptr should fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ConvertS32_015
 * @tc.desc  : Test convert SAMPLE_F32LE STEREO
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ConvertS32_015, TestSize.Level1)
{
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};

    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};

    srcData.bufferDesc.bufLength = NUMBER8;
    bool ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "bufLength failed";

    dstData.bufferDesc.bufLength = NUMBER8;
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "srcDesc.buffer is nullptr should fail";

    uint8_t srcArray[NUMBER8] = {0};
    srcData.bufferDesc = {srcArray, NUMBER8, NUMBER8};
    ret = AudioProcessInClientInner::ChannelFormatS32Convert(srcData, dstData);
    EXPECT_EQ(false, ret) << "dstDesc.buffer is nullptr should fail";
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_WaitIfBufferEmpty_001
 * @tc.desc  : Test AudioProcessInClientInner::WaitIfBufferEmpty
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_WaitIfBufferEmpty_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer;
    bufDesc.dataLength = 0;

    auto ret = ptrAudioProcessInClientInner->WaitIfBufferEmpty(bufDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_WaitIfBufferEmpty_002
 * @tc.desc  : Test AudioProcessInClientInner::WaitIfBufferEmpty
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_WaitIfBufferEmpty_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = false;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer;
    bufDesc.dataLength = 0;

    auto ret = ptrAudioProcessInClientInner->WaitIfBufferEmpty(bufDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_WaitIfBufferEmpty_003
 * @tc.desc  : Test AudioProcessInClientInner::WaitIfBufferEmpty
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_WaitIfBufferEmpty_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer;
    // datalenth > 0
    bufDesc.dataLength = 1;

    auto ret = ptrAudioProcessInClientInner->WaitIfBufferEmpty(bufDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_WaitIfBufferEmpty_004
 * @tc.desc  : Test AudioProcessInClientInner::WaitIfBufferEmpty
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_WaitIfBufferEmpty_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = false;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer;
    // datalenth > 0
    bufDesc.dataLength = 1;

    auto ret = ptrAudioProcessInClientInner->WaitIfBufferEmpty(bufDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_GetAudioTime_002
 * @tc.desc  : Test AudioProcessInClientInner::GetAudioTime
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_GetAudioTime_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);

    ptrBufferInfo->handlePos.store(0);
    ptrBufferInfo->handleTime.store(0);

    uint32_t framePos = 0;
    int64_t sec = 0;
    int64_t nanoSec = 0;
    auto ret = ptrAudioProcessInClientInner->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(nanoSec, 0);

    ptrBufferInfo->handleTime++;
    ptrAudioProcessInClientInner->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(nanoSec, 1);

    ptrBufferInfo->handlePos++;
    ptrAudioProcessInClientInner->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(nanoSec, 1);

    ptrBufferInfo->handleTime++;
    ptrAudioProcessInClientInner->GetAudioTime(framePos, sec, nanoSec);
    EXPECT_EQ(nanoSec, 1);

    delete ptrBufferInfo;
}

/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_ExitStandByIfNeed_001
 * @tc.desc  : Test AudioProcessInClientInner::ExitStandByIfNeed
 */
HWTEST(AudioProcessInClientUnitTest, AudioProcessInClientInner_ExitStandByIfNeed_001, TestSize.Level0)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptrAudioProcessInClientInner->audioBuffer_ = std::make_shared<OHAudioBufferBase>(bufferHolder, totalSizeInFrame,
        byteSizePerFrame);
    ASSERT_TRUE(ptrAudioProcessInClientInner->audioBuffer_ != nullptr);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    auto ptrBufferInfo = ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_;
    ASSERT_TRUE(ptrBufferInfo != nullptr);

    auto ptrFastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    ptrAudioProcessInClientInner->Init(config, ptrFastAudioStream);

    ptrBufferInfo->handlePos.store(0);
    ptrBufferInfo->handleTime.store(0);

    auto mockIAudioProcess = sptr<MockIAudioProcess>::MakeSptr();
    ptrAudioProcessInClientInner->processProxy_ = mockIAudioProcess;
    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_STAND_BY);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;

    EXPECT_CALL(*mockIAudioProcess, Start()).Times(1).WillOnce(Return(0));
    ptrAudioProcessInClientInner->ExitStandByIfNeed();

    delete ptrBufferInfo;
}

/**
 * @tc.name  : Test CallClientHandleCurrent API
 * @tc.type  : FUNC
 * @tc.number: CallClientHandleCurrent_001
 * @tc.desc  : Test CallClientHandleCurrent
 */
HWTEST(AudioProcessInClientUnitTest, CallClientHandleCurrent_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap, info);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
    ptrAudioProcessInClientInner->CallClientHandleCurrent();
}

/**
 * @tc.name  : Test IsRestoreNeeded API
 * @tc.type  : FUNC
 * @tc.number: IsRestoreNeeded_001
 * @tc.desc  : Test IsRestoreNeeded
 */
HWTEST(AudioProcessInClientUnitTest, IsRestoreNeeded_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, true, info);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    std::atomic<StreamStatus> streamStatus;
    streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptrAudioProcessInClientInner->streamStatus_ = &streamStatus;

    // totalsize is 100, byteSizePerFrame is 1
    ptrAudioProcessInClientInner->audioBuffer_ = OHAudioBufferBase::CreateFromLocal(100, 1);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    EXPECT_EQ(ptrAudioProcessInClientInner->IsRestoreNeeded(), false);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    EXPECT_EQ(ptrAudioProcessInClientInner->CheckAndWaitBufferReadyForRecord(), true);
    EXPECT_EQ(ptrAudioProcessInClientInner->CheckAndWaitBufferReadyForPlayback(), true);
    EXPECT_EQ(ptrAudioProcessInClientInner->IsRestoreNeeded(), true);

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE_TO_NORMAL);
    EXPECT_EQ(ptrAudioProcessInClientInner->CheckAndWaitBufferReadyForRecord(), true);
    EXPECT_EQ(ptrAudioProcessInClientInner->CheckAndWaitBufferReadyForPlayback(), true);
    EXPECT_EQ(ptrAudioProcessInClientInner->IsRestoreNeeded(), true);
}
} // namespace AudioStandard
} // namespace OHOS
