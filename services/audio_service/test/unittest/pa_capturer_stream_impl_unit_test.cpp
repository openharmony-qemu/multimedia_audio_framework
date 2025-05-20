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
#include "pa_capturer_stream_impl.h"
#include "audio_errors.h"
#include "pa_adapter_manager.h"
#include <pulse/pulseaudio.h>
#include "pulse/stream.h"
#include "audio_system_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static std::shared_ptr<PaAdapterManager> adapterManager;

class PaCapturerStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<PaCapturerStreamImpl> CreatePaCapturerStreamImpl();
};

void PaCapturerStreamUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void PaCapturerStreamUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void PaCapturerStreamUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void PaCapturerStreamUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

#ifdef HAS_FEATURE_INNERCAPTURER
void LoadPaPort()
{
    AudioPlaybackCaptureConfig checkConfig;
    int32_t checkInnerCapId = 0;
    AudioSystemManager::GetInstance()->CheckCaptureLimit(checkConfig, checkInnerCapId);
}

void ReleasePaPort()
{
    AudioSystemManager::GetInstance()->ReleaseCaptureLimit(1);
}
#endif

static AudioProcessConfig GetInnerCapConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    config.innerCapId = 1;
    return config;
}

std::shared_ptr<PaCapturerStreamImpl> PaCapturerStreamUnitTest::CreatePaCapturerStreamImpl()
{
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(processConfig, stream);
    std::shared_ptr<PaCapturerStreamImpl> capturerStreamImpl =
        std::static_pointer_cast<PaCapturerStreamImpl>(capturerStream);
    return capturerStreamImpl;
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_001
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_001, TestSize.Level1)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    LoadPaPort();
#endif
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    uint64_t framesReadRet = 0;
    capturerStreamImplRet->byteSizePerFrame_ = 0;
    auto ret = capturerStreamImplRet->GetStreamFramesRead(framesReadRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    capturerStreamImplRet->byteSizePerFrame_ = 1;
    ret = capturerStreamImplRet->GetStreamFramesRead(framesReadRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_002
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_002, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    uint64_t timestampRet = 0;
    capturerStreamImplRet->paStream_ = nullptr;

    auto ret = capturerStreamImplRet->GetCurrentTimeStamp(timestampRet);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_003
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_003, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    uint64_t timestampRet = 0;

    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);
    pa_stream_terminate(capturerStreamImplRet->paStream_);
    auto ret = capturerStreamImplRet->GetCurrentTimeStamp(timestampRet);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_004
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_004, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    uint64_t latencyRet;
    bool isStandbyRet = false;
    capturerStreamImplRet->paStream_ = nullptr;

    auto ret = capturerStreamImplRet->GetLatency(latencyRet);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_005
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_005, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    bool isStandbyRet = false;
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_006
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_006, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    capturerStreamImplRet->paStream_ = nullptr;
    capturerStreamImplRet->state_ = RUNNING;

    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_007
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_007, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    capturerStreamImplRet->statusCallback_ = std::weak_ptr<IStatusCallback>();
    capturerStreamImplRet->state_ = STOPPED;

    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_008
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_008, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);

    size_t lengthRet = 10;
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *streamRet = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, streamRet);

    void *userdataRet = nullptr;
    capturerStreamImplRet->PAStreamReadCb(streamRet, lengthRet, userdataRet);
    EXPECT_NE(capturerStreamImplRet->paStream_, nullptr);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_009
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_009, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);

    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *streamRet = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, streamRet);

    void *userdataRet = nullptr;
    capturerStreamImplRet->PAStreamMovedCb(streamRet, userdataRet);
    capturerStreamImplRet->PAStreamMovedCb(streamRet, (void *)1);
    EXPECT_NE(capturerStreamImplRet->paStream_, nullptr);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_010
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_010, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);
    
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, stream);

    capturerStreamImplRet->paStream_ = stream;
    auto ret = capturerStreamImplRet->InitParams();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_011
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_011, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);

    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *streamRet = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, streamRet);

    void *userdataRet = nullptr;
    capturerStreamImplRet->PAStreamUnderFlowCb(streamRet, userdataRet);
    EXPECT_NE(capturerStreamImplRet->paStream_, nullptr);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_012
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_012, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);

    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *streamRet = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, streamRet);

    void *userdataRet = nullptr;
    capturerStreamImplRet->PAStreamSetStartedCb(streamRet, userdataRet);
    capturerStreamImplRet->PAStreamSetStartedCb(streamRet, (void *)1);
    EXPECT_NE(capturerStreamImplRet->paStream_, nullptr);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_013
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_013, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    ASSERT_NE(nullptr, capturerStreamImplRet);

    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *streamRet = adapterManager->InitPaStream(processConfig, sessionId, false);
    ASSERT_NE(nullptr, streamRet);
    void *userdataRet = nullptr;
    int32_t successRet = 0;

    capturerStreamImplRet->PAStreamStartSuccessCb(streamRet, successRet, userdataRet);
    capturerStreamImplRet->PAStreamPauseSuccessCb(streamRet, successRet, userdataRet);
    capturerStreamImplRet->PAStreamFlushSuccessCb(streamRet, successRet, userdataRet);
    capturerStreamImplRet->PAStreamStopSuccessCb(streamRet, successRet, userdataRet);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_014
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_014, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);
    pa_stream_terminate(capturerStreamImplRet->paStream_);
    int32_t ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_015
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_015, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    bool isStandbyRet = false;
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);
    pa_stream_terminate(capturerStreamImplRet->paStream_);
    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_016
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_016, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    bool isStandbyRet = false;
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    pa_stream_terminate(capturerStreamImplRet->paStream_);
    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_017
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_017, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    bool isStandbyRet = false;
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    EXPECT_EQ(pa_stream_get_state(capturerStreamImplRet->paStream_), PA_STREAM_READY);

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);

    pa_stream_terminate(capturerStreamImplRet->paStream_);
    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_018
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_018, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    capturerStreamImplRet->state_ = RUNNING;
    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test PaCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: PaCapturerStream_019
 * @tc.desc  : Test PaCapturerStreamImpl interface.
 */
HWTEST_F(PaCapturerStreamUnitTest, PaCapturerStream_019, TestSize.Level1)
{
    auto capturerStreamImplRet = CreatePaCapturerStreamImpl();
    adapterManager = std::make_shared<PaAdapterManager>(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    AudioProcessConfig processConfig = GetInnerCapConfig();
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    capturerStreamImplRet->paStream_ = stream;
    capturerStreamImplRet->capturerServerDumpFile_ = fopen("/data/data/.pulse_dir/capturer_impl.pcm", "wb+");
    size_t length = 1;
    capturerStreamImplRet->DequeueBuffer(length);
    EXPECT_EQ(capturerStreamImplRet != nullptr, true);
#ifdef HAS_FEATURE_INNERCAPTURER
    ReleasePaPort();
#endif
}
}
}