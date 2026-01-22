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
    EXPECT_FALSE(ptrAudioProcessInClientInner->CheckStaticAndOperate());
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: GetStaticBufferInfo_001
 * @tc.desc  : Test GetStaticBufferInfo with static renderer info
 */
HWTEST(AudioProcessInClientUnitTest, GetStaticBufferInfo_001, TestSize.Level4)
{
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool isVoipMmap = true;
    AudioStreamInfo info = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    auto ptrAudioProcessInClientInner = std::make_shared<AudioProcessInClientInner>(processStream, isVoipMmap);
    ASSERT_TRUE(ptrAudioProcessInClientInner != nullptr);
    ptrAudioProcessInClientInner->processConfig_.rendererInfo.isStatic = true;
    StaticBufferInfo staticBufferInfo{};
    EXPECT_NE(ptrAudioProcessInClientInner->GetStaticBufferInfo(staticBufferInfo), SUCCESS);
}

/**
 * @tc.name  : Test CheckOperations API with static renderer
 * @tc.type  : FUNC
 * @tc.number: SetStaticRenderRate_001
 * @tc.desc  : Test GetStaticBufferInfo with static renderer info
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

} // namespace AudioStandard
} // namespace OHOSs