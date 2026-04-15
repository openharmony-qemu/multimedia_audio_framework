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

using namespace testing::ext;
using namespace testing;

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

class StaticBufferEventCallbackTest : public StaticBufferEventCallback {
public:
    void OnStaticBufferEvent(StaticBufferEventId eventId) override {}
};

/**
 * @tc.name  : Test GetPredictNextHandleTime API
 * @tc.type  : FUNC
 * @tc.number: GetPredictNextHandleTime_001
 * @tc.desc  : Test GetPredictNextHandleTime
 */
HWTEST(AudioProcessInClientUnitTest, GetPredictNextHandleTime_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t posInFrame = 100;
    bool isIndependent = false;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    int64_t result = ptrAudioProcessInClientInner->GetPredictNextHandleTime(posInFrame, isIndependent);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetPredictNextHandleTime API
 * @tc.type  : FUNC
 * @tc.number: GetPredictNextHandleTime_002
 * @tc.desc  : Test GetPredictNextHandleTime
 */
HWTEST(AudioProcessInClientUnitTest, GetPredictNextHandleTime_002, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t posInFrame = 100;
    bool isIndependent = true;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    int64_t result = ptrAudioProcessInClientInner->GetPredictNextHandleTime(posInFrame, isIndependent);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetPredictNextHandleTime API
 * @tc.type  : FUNC
 * @tc.number: GetPredictNextHandleTime_003
 * @tc.desc  : Test GetPredictNextHandleTime
 */
HWTEST(AudioProcessInClientUnitTest, GetPredictNextHandleTime_003, TestSize.Level2)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t posInFrame = 0;
    bool isIndependent = true;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 0;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    int64_t result = ptrAudioProcessInClientInner->GetPredictNextHandleTime(posInFrame, isIndependent);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test GetPredictNextHandleTime API
 * @tc.type  : FUNC
 * @tc.number: GetPredictNextHandleTime_004
 * @tc.desc  : Test GetPredictNextHandleTime
 */
HWTEST(AudioProcessInClientUnitTest, GetPredictNextHandleTime_004, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t posInFrame = 100;
    bool isIndependent = true;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 0;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    int64_t result = ptrAudioProcessInClientInner->GetPredictNextHandleTime(posInFrame, isIndependent);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test GetPredictNextHandleTime API
 * @tc.type  : FUNC
 * @tc.number: GetPredictNextHandleTime_005
 * @tc.desc  : Test GetPredictNextHandleTime
 */
HWTEST(AudioProcessInClientUnitTest, GetPredictNextHandleTime_005, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);

    uint64_t posInFrame = 0;
    bool isIndependent = false;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 0;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    int64_t result = ptrAudioProcessInClientInner->GetPredictNextHandleTime(posInFrame, isIndependent);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test ReadFromProcessClient API
 * @tc.type  : FUNC
 * @tc.number: ReadFromProcessClient
 * @tc.desc  : Test AudioProcessInClientInner::ReadFromProcessClient
 */
HWTEST(AudioProcessInClientUnitTest, ReadFromProcessClient_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner -> spanSizeInByte_ = 0;

    auto ret = ptrAudioProcessInClientInner->ReadFromProcessClient();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test CopyWithVolume API
 * @tc.type  : FUNC
 * @tc.number: CopyWithVolume_001
 * @tc.desc  : Test AudioProcessInClientInner::CopyWithVolume
 */
HWTEST(AudioProcessInClientUnitTest, CopyWithVolume_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    BufferDesc srcDesc;
    BufferDesc dstDesc;
    srcDesc.bufLength = 1;
    dstDesc.bufLength = 1;
    ptrAudioProcessInClientInner->CopyWithVolume(srcDesc, dstDesc);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_001
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, CheckOperations_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrAudioProcessInClientInner->audioBuffer_ =
        OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrAudioProcessInClientInner->sendStaticRecreateFunc_ = nullptr;
    ptrAudioProcessInClientInner->CheckOperations();

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ptrAudioProcessInClientInner->sendStaticRecreateFunc_ = nullptr;
    ptrAudioProcessInClientInner->CheckOperations();

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrAudioProcessInClientInner->sendStaticRecreateFunc_ = [](){return;};
    ptrAudioProcessInClientInner->CheckOperations();

    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ptrAudioProcessInClientInner->sendStaticRecreateFunc_ = [](){return;};
    ptrAudioProcessInClientInner->CheckOperations();
    EXPECT_NE(ptrAudioProcessInClientInner, nullptr);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_002
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, CheckOperations_002, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrAudioProcessInClientInner->audioBuffer_ =
        OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrAudioProcessInClientInner->audioStaticBufferEventCallback_ = std::make_shared<StaticBufferEventCallbackTest>();
    ptrAudioProcessInClientInner->audioBuffer_->SetStaticMode(true);
    ptrAudioProcessInClientInner->audioBuffer_->IncreaseBufferEndCallbackSendTimes();
    ptrAudioProcessInClientInner->CheckOperations();
    EXPECT_EQ(ptrAudioProcessInClientInner->audioBuffer_->IsNeedSendBufferEndCallback(), false);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckOperations_003
 * @tc.desc  : Test CheckOperations with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, CheckOperations_003, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrAudioProcessInClientInner->audioBuffer_ =
        OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrAudioProcessInClientInner->audioStaticBufferEventCallback_ = std::make_shared<StaticBufferEventCallbackTest>();
    ptrAudioProcessInClientInner->audioBuffer_->SetStaticMode(true);
    ptrAudioProcessInClientInner->audioBuffer_->SetIsNeedSendLoopEndCallback(true);
    ptrAudioProcessInClientInner->audioBuffer_->SetIsFirstFrame(false);
    ptrAudioProcessInClientInner->CheckOperations();
    EXPECT_EQ(ptrAudioProcessInClientInner->audioBuffer_->IsNeedSendLoopEndCallback(), false);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetStaticBufferEventCallback_001
 * @tc.desc  : Test SetStaticBufferInfo with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, SetStaticBufferInfo_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    uint32_t totalSizeInFrame = 100;
    uint32_t byteSizePerFrame = 1;
    ptrAudioProcessInClientInner->audioBuffer_ =
        OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ptrAudioProcessInClientInner->audioBuffer_->basicBufferInfo_->restoreStatus.store(NO_NEED_FOR_RESTORE);
    ptrAudioProcessInClientInner->audioStaticBufferEventCallback_ = std::make_shared<StaticBufferEventCallbackTest>();
    ptrAudioProcessInClientInner->audioBuffer_->SetStaticMode(true);
    ptrAudioProcessInClientInner->audioBuffer_->SetIsNeedSendLoopEndCallback(true);
    ptrAudioProcessInClientInner->CheckOperations();
    EXPECT_EQ(ptrAudioProcessInClientInner->audioBuffer_->IsNeedSendLoopEndCallback(), false);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetStaticBufferEventCallback_001
 * @tc.desc  : Test SetStaticBufferEventCallback with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, SetStaticBufferEventCallback_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    auto callback = std::make_shared<StaticBufferEventCallbackTest>();
    EXPECT_EQ(ptrAudioProcessInClientInner->SetStaticBufferEventCallback(callback), SUCCESS);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetStaticTriggerRecreateCallback_001
 * @tc.desc  : Test SetStaticTriggerRecreateCallback with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, SetStaticTriggerRecreateCallback_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    EXPECT_EQ(ptrAudioProcessInClientInner->SetStaticTriggerRecreateCallback([](){return;}), SUCCESS);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetLoopTimes_001
 * @tc.desc  : Test SetLoopTimes with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, SetLoopTimes_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    EXPECT_EQ(ptrAudioProcessInClientInner->SetLoopTimes(99), SUCCESS);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: CheckStaticAndOperate_001
 * @tc.desc  : Test CheckStaticAndOperate with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, CheckStaticAndOperate_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    ptrAudioProcessInClientInner->audioBuffer_ = OHAudioBufferBase::CreateFromLocal(10, 10);
    ptrAudioProcessInClientInner->audioBuffer_->SetStaticMode(true);
    ptrAudioProcessInClientInner->audioBuffer_->SetIsFirstFrame(false);
    EXPECT_FALSE(ptrAudioProcessInClientInner->CheckStaticAndOperate());
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetStaticRenderRate_001
 * @tc.desc  : Test SetStaticRenderRate with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, SetStaticRenderRate_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    EXPECT_NE(ptrAudioProcessInClientInner->SetStaticRenderRate(RENDER_RATE_NORMAL), SUCCESS);
}

/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_001
 * @tc.desc  : Test GetLatencyWithFlag interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetLatencyWithFlag_001, TestSize.Level4)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
 
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetLatencyWithFlag_001 start");
    uint64_t latency = 0;
    LatencyFlag flag = LATENCY_FLAG_SHARED_BUFFER;
    int result = fastAudioStream->GetLatencyWithFlag(latency, flag);
    EXPECT_EQ(result, SUCCESS);
}
 
/**
 * @tc.name  : Test GetLatencyWithFlag API
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_002
 * @tc.desc  : Test GetLatencyWithFlag interface.
 */
HWTEST_F(FastSystemStreamUnitTest, GetLatencyWithFlag_002, TestSize.Level4)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
 
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetLatencyWithFlag_002 start");
    uint64_t latency = 0;
    LatencyFlag flag = LATENCY_FLAG_HARDWARE;
    int result= fastAudioStream->GetLatencyWithFlag(latency, flag);
    EXPECT_EQ(result, SUCCESS);
}
 
/**
 * @tc.name  : Test PauseAudioStream API
 * @tc.type  : FUNC
 * @tc.number: PauseAudioStream_001
 * @tc.desc  : Test PauseAudioStream interface.
 */
HWTEST_F(FastSystemStreamUnitTest, PauseAudioStream_001, TestSize.Level4)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
 
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest PauseAudioStream start");
    fastAudioStream->state_ = RUNNING;
    StateChangeCmdType cmdType = CMD_FROM_SYSTEM;
    int result= fastAudioStream->PauseAudioStream(cmdType);
    EXPECT_EQ(result, SUCCESS);
}

/ *
 * @tc.name  : Test GetSessionID API
 * @tc.type  : FUNC
 * @tc.number: GetSessionID_001
 * @tc.desc  : Test GetSessionID returns session ID
 */
HWTEST(AudioProcessInClientUnitTest, GetSessionID_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    uint32_t sessionId = 0;
    int32_t ret = ptrAudioProcessInClientInner->->GetSessionID(sessionId);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sessionId, ptrAudioProcessInClientInner->sessionId_);
}

/*
 * @tc.name  : Test GetBufferSize API
 * @tc.type  : FUNC
 * @tc.number: GetBufferSize_001
 * @tc.desc  : Test GetBufferSize returns buffer size
 */
HWTEST(AudioProcessInClientUnitTest, GetBufferSize_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    size_t bufferSize = 0;
    int32_t ret = ptrAudioProcessInClientInner->->GetBufferSize(bufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test GetFrameCount API
 * @tc.type. : FUNC
 * @tc.number: GetFrameCount_001
 * @tc.desc  : Test GetFrameCount returns frame count
 */
HWTEST(AudioProcessInClientUnitTest, GetFrameCount_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    uint32_t frameCount = 0;
    int32_t ret = ptrAudioProcessInClientInner->->GetFrameCount(frameCount);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency returns fixed latency value
 */
HWTEST(AudioProcessInClientUnitTest, GetLatency_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    uint64_t latency = 0;
    int32_t ret = ptrAudioProcessInClientInner->->GetLatency(latency);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 20);
}

/*
 * @tc.name  : Test SetVolume API with valid int32 volume
 * @tc.type  : FUNC
 * @tc.number: SetVolume_Int32_Valid_Valid_001
 * @tc.desc  : Test SetVolume with valid int32 volume
 */
HWTEST(AudioProcessInClientUnitTest, SetVolume_Int32_Valid_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    int32_t ret = ptrAudioProcessInClientInner->->SetVolume(32768);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test SetDuckVolume API with valid volume
 * @tc.type  : FUNC
 * @tc.number: SetDuckVolume_Valid_001
 * @tc.desc  : Test SetDuckVolume with valid volume
 */
HWTEST(AudioProcessInClientUnitTest, SetDuckVolume_Valid_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = OHAudioBufferBase::CreateFromLocal(100, 4);
    int32_t ret = ptrAudioProcessInClientInner->->SetDuckVolume(0.7f);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(ptrAudioProcessInClientInner->GetDuckVolume(), 0.7f);
}

/*
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: SetMute_001
 * @tc.desc  : Test SetMute with true
 */
HWTEST(AudioProcessInClientUnitTest, SetMute_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = OHAudioBufferBase::CreateFromLocal(100, 4);
    int32_t ret = ptrAudioProcessInClientInner->->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(ptrAudioProcessInClientInner->GetMute(), true);
}

/*
 * @tc.name  : Test SetMute API
 * @tc.type  : FUNC
 * @tc.number: SetMute_002
 * @tc.desc  : Test SetMute with false
 */
HWTEST(AudioProcessInClientUnitTest, SetMute_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = OHAudioBufferBase::CreateFromLocal(100, 4);
    int32_t ret = ptrAudioProcessInClientInner->->SetMute(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(ptrAudioProcessInClientInner->GetMute(), false);
}

/*
 * @tc.name  : Test SetUnderflowCount and GetUnderflowCount
 * @tc.type  : FUNC
 * @tc.number: SetUnderflowCount_001
 * @tc.desc  : Test SetUnderflowCount and GetUnderflowCount
 */
HWTEST(AudioProcessInClientUnitTest, SetUnderflowCount_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->underflowCount_ = 0;
    ptrAudioProcessInClientInner->SetUnderflowCount(1);
    uint32_t res = ptrAudioProcessInClientInner->GetUnderflowCount();
    EXPECT_EQ(res, 1);
}

/*
 * @tc.name  : Test SetOverflowCount and GetOverflowCount
 * @tc.type  : FUNC
 * @tc.number: SetOverflowCount_001
 * @tc.desc  : Test SetOverflowCount and GetOverflowCount
 */
HWTEST(AudioProcessInClientUnitTest, SetOverflowCount_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->overflowCount_ = 0;
    ptrAudioProcessInClientInner->SetOverflowCount(1);
    uint32_t res = ptrAudioProcessInClientInner->GetOverflowCount();
    EXPECT_EQ(res, 1);
}

/*
 * @tc.name  : Test SetSourceDuration API
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration_001
 * @tc.desc  : Test SetSourceDuration with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetSourceDuration_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    int32_t ret = ptrAudioProcessInClientInner->->SetSourceDuration(1000);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/*
 * @tc.name  : Test UpdateLatencyTimestamp API
 * @tc.type  : FUNC
 * @tc.number: UpdateLatencyTimestamp_001
 * @tc.desc  : Test UpdateLatencyTimestamp
 */
HWTEST(AudioProcessInClientUnitTest, UpdateLatencyTimestamp_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    std::string timestamp = "test_timestamp";
    ptrAudioProcessInClientInner->UpdateLatencyTimestamp(timestamp, true);
    ptrAudioProcessInClientInner->UpdateLatencyTimestamp(timestamp, false);
}

/*
 * @tc.name  : Test SetDefaultOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServiceServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    int32_t ret = ptrAudioProcessInClientInner->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, false);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/*
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    int32_t ret = ptrAudioProcessInClientInner->SetSilentModeAndMixWithOthers(true);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/*
 * @tc.name  : Test GetRestoreInfo API
 * @tc.type  : FUNC
 * @tc.number: GetRestoreInfo_001
 * @tc.desc  : Test GetRestoreInfo with null audioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, GetRestoreInfo_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = nullptr;
    RestoreInfo restoreInfo;
    ptrAudioProcessInClientInner->GetRestoreInfo(restoreInfo);
}

/*
 * @tc.name  : Test SetRestoreInfo API
 * @tc.type  : FUNC
 * @tc.number: SetRestoreInfo_001
 * @tc.desc  : Test SetRestoreInfo with null audioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, SetRestoreInfo_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = nullptr;
    RestoreInfo restoreInfo;
    ptrAudioProcessInClientInner->SetRestoreInfo(restoreInfo);
}

/*
 * @tc.name  : Test CheckRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: CheckRestoreStatus_001
 * @tc.desc  : Test CheckRestoreStatus with null audioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, CheckRestoreStatus_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = nullptr;
    RestoreStatus status = ptrAudioProcessInClientInner->CheckRestoreStatus();
    EXPECT_EQ(status, RESTORE_ERROR);
}

/*
 * @tc.name  : Test SetRestoreStatus API
 * @tc.type  : FUNC
 * @tc.number: SetRestoreStatus_001
 * @tc.desc  : Test SetRestoreStatus with null audioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, SetRestoreStatus_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = nullptr;
    RestoreStatus status = ptrAudioProcessInClientInner->SetRestoreStatus(NEED_RESTORE);
    EXPECT_EQ(status, RESTORE_ERROR);
}

/*
 * @tc.name  : Test RegisterThreadPriority API
 * @tc.type  : FUNC
 * @tc.number: RegisterThreadPriority_001
 * @tc.desc  : Test RegisterThreadPriority with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, RegisterThreadPriority_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    int32_t ret = ptrAudioProcessInClientInner->RegisterThreadPriority(getpid(), "test.bundle",
        METHOD_WRITE_OR_READ, THREAD_PRIORITY_QOS_7);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/*
 * @tc.name  : Test GetStopFlag API
 * @tc.type  : FUNC
 * @tc.number: GetStopFlag_001
 * @tc.desc  : Test GetStopFlag with null audioBuffer
 */
HWTEST(AudioProcessInClientUnitTest, GetStopFlag_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->audioBuffer_ = nullptr;
    bool stopFlag = ptrAudioProcessInClientInner->GetStopFlag();
    EXPECT_EQ(stopFlag, true);
}

/*
 * @tc.name  : Test SetRebuildFlag API
 * @tc.type  : FUNC
 * @tc.number: SetRebuildFlag_001
 * @tc.desc  : Test SetRebuildFlag with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetRebuildFlag_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    ptrAudioProcessInClientInner->SetRebuildFlag();
}

/*
 * @tc.name  : Test SetVoipNoPrivacyFlag API
 * @tc.type  : FUNC
 * @tc.number: SetVoipNoPrivacyFlag_001
 * @tc.desc  : Test SetVoipNoPrivacyFlag with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetVoipNoPrivacyFlag_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    ptrAudioProcessInClientInner->SetVoipNoPrivacyFlag(true);
}

/*
 * @tc.name  : Test SetVoipNoPrivacyFlag API
 * @tc.type  : FUNC
 * @tc.number: SetVoipNoPrivacyFlag_002
 * @tc.desc  : Test SetVoipNoPrivacyFlag with valid processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetVoipNoPrivacyFlag_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.streamType = STREAM_VOICE_COMMUNICATION;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);

    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);

    ptrAudioProcessInClientInner->processProxy_ = processStream;
    ptrAudioProcessInClientInner->SetVoipNoPrivacyFlag(true);
    EXPECT_TRUE(processStream->GetVoipNoPrivacyFlag());
}

/*
 * @tc.name  : Test GetKeepRunning API
 * @tc.type  : FUNC
 * @tc.number: GetKeepRunning_001
 * @tc.desc  : Test GetKeepRunning with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, GetKeepRunning_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    bool keepRunning = false;
    ptrAudioProcessInClientInner->GetKeepRunning(keepRunning);
}

/*
 * @tc.name  : Test SetAudioHapticsSyncId API
 * @tc.type  : FUNC
 * @tc.number: SetAudioHapticsSyncId_001
 * @tc.desc  : Test SetAudioHapticsSyncId with null processProxy
 */
HWTEST(AudioProcessInClientUnitTest, SetAudioHapticsSyncId_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->processProxy_ = nullptr;
    ptrAudioProcessInClientInner->SetAudioHapticsSyncId(100);
}

/*
 * @tc.name  : Test SetPreferredFrameSize API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_001
 * @tc.desc  : Test SetPreferredFrameSize with frameSize > spanSizeInFrame_
 */
HWTEST(AudioProcessInClientUnitTest, SetPreferredFrameSize_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 4;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(20);
}

/*
 * @tc.name  : Test SetPreferredFrameSize API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_002
 * @tc.desc  : Test SetPreferredFrameSize with frameSize >= MAX_TIMES * spanSizeInFrame_
 */
HWTEST(AudioProcessInClientUnitTest, SetPreferredFrameSize_002, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 4;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(100);
}

/*
 * @tc.name  : Test SetPreferredFrameSize API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_003
 * @tc.desc  : Test SetPreferredFrameSize with isRecreate parameter
 */
HWTEST(AudioProcessInClientUnitTest, SetPreferredFrameSize_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    
    ptrAudioProcessInClientInner->spanSizeInFrame_ = 10;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 4;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(15, true);
}

/*
 * @tc.name  : Test SetPreferredFrameSize API
 * @tc.type  : FUNC
 * @tc.number: SetPreferredFrameSize_004
 * @tc.desc  : Test SetPreferredFrameSize with isRecreate parameter
 */
HWTEST(AudioProcessInClientUnitTest, SetPreferredFrameSize_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = false;
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    int32_t frameSize = 5;
    int32_t originalSize = 20;
    ptrAudioProcessInClientInner->spanSizeInFrame_ = originalSize;
    ptrAudioProcessInClientInner->clientByteSizePerFrame_ = 0;
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.originalFlag = AUDIO_FLAG_ULTRA_FAST;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize, true);
    EXPECT_EQ(ptrAudioProcessInClientInner->clientSpanSizeInFrame_, frameSize);
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize, false);
    EXPECT_EQ(ptrAudioProcessInClientInner->clientSpanSizeInFrame_, originalSize);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.originalFlag = AUDIO_FLAG_NORMAL;
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize, true);
    EXPECT_EQ(ptrAudioProcessInClientInner->clientSpanSizeInFrame_, originalSize);
    ptrAudioProcessInClientInner->SetPreferredFrameSize(frameSize, false);
    EXPECT_EQ(ptrAudioProcessInClientInner->clientSpanSizeInFrame_, originalSize);
}

} // namespace AudioStandard
} // namespace OHOS
