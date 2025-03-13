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

#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "pa_capturer_stream_impl.h"
#include "pa_adapter_manager.h"
#include "capturer_in_server.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class CapturerInServerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

void CapturerInServerUnitTest::SetUpTestCase() {}
void CapturerInServerUnitTest::TearDownTestCase() {}
void CapturerInServerUnitTest::SetUp() {}
void CapturerInServerUnitTest::TearDown() {}

class ConcreteIStreamListener : public IStreamListener {
    int32_t OnOperationHandled(Operation operation, int64_t result) { return SUCCESS; }
};

const int32_t CAPTURER_FLAG = 10;
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
    return config;
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_001.
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    pa_stream *paStream = nullptr;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(paStream,
        processConfig, mainloop);
    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->audioServerBuffer_ = nullptr;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->spanSizeInFrame_ = 10;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_002.
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_002, TestSize.Level1)
{
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    pa_stream *paStream = nullptr;
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(paStream,
        processConfig, mainloop);
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->spanSizeInFrame_ = 10;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_003.
 * @tc.desc  : Test InitBufferStatus interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_003, TestSize.Level1)
{
    AudioBufferHolder bufferHolder;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->InitBufferStatus();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->audioServerBuffer_->spanConut_ = 5;
    capturerInServer_->InitBufferStatus();
    EXPECT_NE(capturerInServer_, nullptr);
}
#ifdef CAPTURER_IN_SERVER_UNIT_TEST_DIFF
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_004.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_004, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_RELEASED;
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_DRAINED);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_UNDERFLOW);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_STARTED);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_PAUSED);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_STOPPED);
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_005.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_005, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_FLUSHED);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_FLUSHED);
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_006.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_006, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_FLUSHED);
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_007.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_007, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    capturerInServer_->OnStatusUpdate(IOperation::OPERATION_FLUSHED);
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_008.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_008, TestSize.Level1)
{
    size_t length = 10;
    uint64_t currentWriteFrame = 10;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    std::shared_ptr<IStreamListener> stateListener = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->audioServerBuffer_->basicBufferInfo_ = new BasicBufferInfo();
    capturerInServer_->spanSizeInFrame_ = 1000;

    capturerInServer_->IsReadDataOverFlow(length, currentWriteFrame, stateListener);
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->overFlowLogFlag_ = 1;
    capturerInServer_->IsReadDataOverFlow(length, currentWriteFrame, stateListener);
    EXPECT_NE(capturerInServer_, nullptr);
    delete capturerInServer_->audioServerBuffer_->basicBufferInfo_;
}
#endif
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_009.
 * @tc.desc  : Test ReadData interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_009, TestSize.Level1)
{
    size_t length = 10;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener = std::make_shared<ConcreteIStreamListener>();
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->spanSizeInFrame_ = -100;
    capturerInServer_->muteFlag_.store(true);
    AudioDump::GetInstance().GetVersionType() = DumpFileUtil::BETA_VERSION;
    capturerInServer_->ReadData(length);
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_010.
 * @tc.desc  : Test Start/Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_010, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    int32_t result = capturerInServer_->Start();
    EXPECT_NE(result, SUCCESS);

    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_PAUSED;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_STOPPED;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_011.
 * @tc.desc  : Test Start/Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_011, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    int32_t UID_MSDP_SA = 6699;
    processConfig.callerUid = UID_MSDP_SA;
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_012.
 * @tc.desc  : Test Start/Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_012, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    int32_t TIME_OUT_SECONDS = 10;
    processConfig.callerUid = TIME_OUT_SECONDS;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_EC;
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_013.
 * @tc.desc  : Test Start/Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_013, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    int32_t UID_MSDP_SA = 6699;
    processConfig.callerUid = UID_MSDP_SA;
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->needCheckBackground_ = true;
    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_014.
 * @tc.desc  : Test Start/Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_014, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    int32_t TIME_OUT_SECONDS = 10;
    processConfig.callerUid = TIME_OUT_SECONDS;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_EC;
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->needCheckBackground_ = true;
    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->Start();
    capturerInServer_->Stop();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_015.
 * @tc.desc  : Test Pause interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_015, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->status_ = I_STATUS_STARTING;
    int32_t result = capturerInServer_->Pause();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    capturerInServer_->status_ = I_STATUS_STARTED;
    result = capturerInServer_->Pause();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_STARTING;
    capturerInServer_->needCheckBackground_ = true;
    result = capturerInServer_->Pause();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_016.
 * @tc.desc  : Test Flush interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_016, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    std::weak_ptr<IStreamListener> streamListener;

    AudioProcessConfig processConfig = GetInnerCapConfig();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    int32_t result = capturerInServer_->Flush();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    capturerInServer_->status_ = I_STATUS_STARTED;
    capturerInServer_->Flush();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_017.
 * @tc.desc  : Test Flush interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_017, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    std::weak_ptr<IStreamListener> streamListener;

    AudioProcessConfig processConfig = GetInnerCapConfig();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->status_ = I_STATUS_PAUSED;
    capturerInServer_->Flush();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->status_ = I_STATUS_STOPPED;
    capturerInServer_->Flush();
    EXPECT_NE(capturerInServer_, nullptr);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_018.
 * @tc.desc  : Test Stop interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_018, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);

    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream,
        processConfig, mainloop);
    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    int32_t result = capturerInServer_->Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    capturerInServer_->status_ = I_STATUS_PAUSED;
    result = capturerInServer_->Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    capturerInServer_->status_ = I_STATUS_PAUSED;
    capturerInServer_->needCheckBackground_ = true;
    result = capturerInServer_->Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_019.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_019, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_RELEASED;
    int result = capturerInServer_->Release();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_020.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_020, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    processConfig.innerCapMode = INVALID_CAP_MODE;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_RELEASING;
    int result = capturerInServer_->Release();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->needCheckBackground_ = true;
    result = capturerInServer_->Release();
    EXPECT_EQ(result, SUCCESS);
}

#ifdef HAS_FEATURE_INNERCAPTURER
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_021.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_021, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    AudioPlaybackCaptureConfig config;
    processConfig.innerCapMode = MODERN_INNER_CAP;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_022.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_022, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    AudioPlaybackCaptureConfig config;
    processConfig.innerCapMode = LEGACY_INNER_CAP;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_023.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_023, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    AudioPlaybackCaptureConfig config;
    processConfig.innerCapMode = MODERN_INNER_CAP;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_UNKNOWN);
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_024.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_024, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    AudioPlaybackCaptureConfig config;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    config.filterOptions.usages.push_back(StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION);
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}
#endif

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_025.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_025, TestSize.Level1)
{
    uint64_t framePos;
    uint64_t timestamp;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_STOPPED;
    int32_t result = capturerInServer_->GetAudioTime(framePos, timestamp);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_026.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_026, TestSize.Level1)
{
    size_t targetSize = 0;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    processConfig.innerCapMode =LEGACY_MUTE_CAP;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_027.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_027, TestSize.Level1)
{
    size_t targetSize = 0;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    processConfig.innerCapMode =LEGACY_INNER_CAP;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_028.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_028, TestSize.Level1)
{
    size_t targetSize = 0;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    processConfig.innerCapMode =LEGACY_MUTE_CAP;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_029.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_029, TestSize.Level1)
{
    size_t targetSize = 0;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    processConfig.innerCapMode =LEGACY_INNER_CAP;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_030.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_030, TestSize.Level1)
{
    size_t cacheSize = 960;
    size_t targetSize = 0;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->ringCache_ = std::make_unique<AudioRingCache>(cacheSize);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: DrainAudioBuffer_001.
 * @tc.desc  : Test DrainAudioBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, DrainAudioBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->DrainAudioBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: ResolveBuffer_001.
 * @tc.desc  : Test ResolveBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, ResolveBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<OHAudioBuffer> buffer;
    int32_t result = capturerInServer_->ResolveBuffer(buffer);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: OnReadData_001.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerUnitTest, OnReadData_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    size_t length = 0;
    int32_t result = capturerInServer_->OnReadData(length);
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
