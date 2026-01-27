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

class ICapturerStreamTest1 : public ICapturerStream {
public:
    int32_t GetStreamFramesRead(uint64_t &framesRead) override { return 0; }
    int32_t GetCurrentTimeStamp(uint64_t &timestamp) override { return 0; }
    int32_t GetLatency(uint64_t &latency) override { return 0; }
    void RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback) override { return; }
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override { return 0; }
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override { return; }
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override { spanSizeInFrame = 0; }
    int32_t DropBuffer() override { return 0; }
    void SetStreamIndex(uint32_t index) override { return; }
    uint32_t GetStreamIndex() override { return 0; }
    int32_t Start() override { return 0; }
    int32_t Pause(bool isStandby = false) override { return 0; }
    int32_t Flush() override { return 0; }
    int32_t Drain(bool stopFlag = false) override { return 0; }
    int32_t Stop() override { return 0; }
    int32_t Release() override { return 0; }
    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override { return; }
    BufferDesc DequeueBuffer(size_t length) override
    {
        BufferDesc bufferDesc;
        return bufferDesc;
    }
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override { return 0; }
};

class ICapturerStreamTest2 : public ICapturerStream {
public:
    int32_t GetStreamFramesRead(uint64_t &framesRead) override { return 0; }
    int32_t GetCurrentTimeStamp(uint64_t &timestamp) override { return 0; }
    int32_t GetLatency(uint64_t &latency) override { return 0; }
    void RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback) override { return; }
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override { return 0; }
    void GetByteSizePerFrame(size_t &byteSizePerFrame) const override { return; }
    void GetSpanSizePerFrame(size_t &spanSizeInFrame) const override { spanSizeInFrame = 1; }
    int32_t DropBuffer() override { return 0; }
    void SetStreamIndex(uint32_t index) override { return; }
    uint32_t GetStreamIndex() override { return 0; }
    int32_t Start() override { return 0; }
    int32_t Pause(bool isStandby = false) override { return 0; }
    int32_t Flush() override { return 0; }
    int32_t Drain(bool stopFlag = false) override { return 0; }
    int32_t Stop() override { return 0; }
    int32_t Release() override { return 0; }
    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override { return; }
    BufferDesc DequeueBuffer(size_t length) override
    {
        BufferDesc bufferDesc;
        return bufferDesc;
    }
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override { return 0; }
};

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

    capturerInServer_->totalSizeInFrame_ = 0;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->totalSizeInFrame_ = 10;
    capturerInServer_->spanSizeInFrame_ = 0;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->totalSizeInFrame_ = 10;
    capturerInServer_->spanSizeInFrame_ = 5;
    capturerInServer_->ConfigServerBuffer();
    EXPECT_NE(capturerInServer_, nullptr);

    capturerInServer_->totalSizeInFrame_ = 10;
    capturerInServer_->spanSizeInFrame_ = 3;
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
    capturerInServer_->audioServerBuffer_->spanBasicInfo_.spanConut_ = 5;
    capturerInServer_->InitBufferStatus();
    EXPECT_NE(capturerInServer_, nullptr);
}

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
    ASSERT_NE(nullptr, capturerInServer_);

    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();

    capturerInServer_->spanSizeInFrame_ = 5;
    auto ret = capturerInServer_->IsReadDataOverFlow(length, currentWriteFrame, stateListener);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_009.
 * @tc.desc  : Test ReadData interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_009, TestSize.Level1)
{
    size_t length = 10;
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    std::weak_ptr<IStreamListener> streamListener = std::make_shared<ConcreteIStreamListener>();
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream, processConfig, mainloop);
    ASSERT_NE(nullptr, capturerInServer_);

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
    int32_t udimsdpsa = 6699;
    processConfig.callerUid = udimsdpsa;
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
    int32_t udimsdpsa = 6699;
    processConfig.callerUid = udimsdpsa;
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
    capturerInServer_->recorderDfx_ = std::make_unique<RecorderDfxWriter>(processConfig.appInfo, 0);
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

    capturerInServer_->status_ = I_STATUS_STARTED;
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
 * @tc.number: pdatePlaybackCaptureConfigInLegacy_021.
 * @tc.desc  : Test pdatePlaybackCaptureConfigInLegacy interface.
 */
HWTEST_F(CapturerInServerUnitTest, UpdatePlaybackCaptureConfigInLegacy_001, TestSize.Level3)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    AudioPlaybackCaptureConfig config;
    processConfig.innerCapMode = MODERN_INNER_CAP;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->UpdatePlaybackCaptureConfigInLegacy(config);
    EXPECT_EQ(result, SUCCESS);
}

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

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: HandleOperationFlushed_001.
 * @tc.desc  : Test HandleOperationFlushed interface.
 */
HWTEST_F(CapturerInServerUnitTest, HandleOperationFlushed_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);

    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    capturerInServer_->HandleOperationFlushed();
    EXPECT_EQ(capturerInServer_->status_, I_STATUS_STARTED);

    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    capturerInServer_->HandleOperationFlushed();
    EXPECT_EQ(capturerInServer_->status_, I_STATUS_PAUSED);

    capturerInServer_->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    capturerInServer_->HandleOperationFlushed();
    EXPECT_EQ(capturerInServer_->status_, I_STATUS_STOPPED);

    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->HandleOperationFlushed();
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: GetLastAudioDuration_001.
 * @tc.desc  : Test GetLastAudioDuration interface.
 */
HWTEST_F(CapturerInServerUnitTest, GetLastAudioDuration_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);

    capturerInServer_->lastStopTime_ = 1;
    capturerInServer_->lastStartTime_ = 2;
    int64_t result = capturerInServer_->GetLastAudioDuration();
    EXPECT_EQ(result, -1);

    capturerInServer_->lastStopTime_ = 3;
    result = capturerInServer_->GetLastAudioDuration();
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_031.
 * @tc.desc  : Test Flush interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_031, TestSize.Level1)
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
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->curWriteFrame.store(10);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->curReadFrame.store(20);
    auto ret = capturerInServer_->Flush();
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_032.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_032, TestSize.Level1)
{
    uint64_t framePos;
    uint64_t timestamp;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_STARTING;

    pa_stream *paStream = nullptr;
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(paStream,
        processConfig, mainloop);

    capturerInServer_->resetTime_ = false;
    int32_t result = capturerInServer_->GetAudioTime(framePos, timestamp);
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->resetTime_ = true;
    result = capturerInServer_->GetAudioTime(framePos, timestamp);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_033.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_033, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    RestoreInfo restoreInfo;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = nullptr;
    capturerInServer_->RestoreSession(restoreInfo);
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->RestoreSession(restoreInfo);
    capturerInServer_->status_.store(I_STATUS_INVALID);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    auto ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
    capturerInServer_->status_.store(I_STATUS_FLUSHING_WHEN_STARTED);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
    capturerInServer_->status_.store(I_STATUS_FLUSHING_WHEN_PAUSED);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
    capturerInServer_->status_.store(I_STATUS_FLUSHING_WHEN_STOPPED);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
    capturerInServer_->status_.store(I_STATUS_RELEASED);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
    capturerInServer_->status_.store(I_STATUS_IDLE);
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_034
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_034, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;

    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer->audioServerBuffer_ = nullptr;
    capturerInServer->stream_ = std::make_shared<ICapturerStreamTest1>();
    ASSERT_NE(capturerInServer->stream_, nullptr);

    auto ret = capturerInServer->ConfigServerBuffer();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_035
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_035, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;

    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer->audioServerBuffer_ = nullptr;
    capturerInServer->stream_ = std::make_shared<ICapturerStreamTest2>();
    ASSERT_NE(capturerInServer->stream_, nullptr);

    auto ret = capturerInServer->ConfigServerBuffer();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_036.
 * @tc.desc  : Test Init interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_036, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    uint32_t sessionId = 123456;
    pa_stream *paStream = adapterManager->InitPaStream(processConfig, sessionId, false);
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(paStream,
        processConfig, mainloop);
    EXPECT_NE(capturerInServer_, nullptr);

    auto ret = capturerInServer_->Init();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_037.
 * @tc.desc  : Test TurnOnMicIndicator interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_037, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    CapturerState capturerState = CAPTURER_NEW;
    capturerInServer_->processConfig_.appInfo.appFullTokenId = (static_cast<uint64_t>(1) << 32);
    auto ret = capturerInServer_->TurnOnMicIndicator(capturerState);
    EXPECT_EQ(ret, false);

    capturerInServer_->isMicIndicatorOn_ = true;
    ret = capturerInServer_->TurnOnMicIndicator(capturerState);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_038.
 * @tc.desc  : Test TurnOffMicIndicator interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_038, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    CapturerState capturerState = CAPTURER_NEW;
    auto ret = capturerInServer_->TurnOffMicIndicator(capturerState);
    EXPECT_EQ(ret, true);

    capturerInServer_->isMicIndicatorOn_ = true;
    ret = capturerInServer_->TurnOffMicIndicator(capturerState);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_039.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_039, TestSize.Level1)
{
    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    pa_threaded_mainloop *mainloop = pa_threaded_mainloop_new();
    AudioProcessConfig processConfig = GetInnerCapConfig();
    uint32_t sessionId = 123456;
    pa_stream *stream = adapterManager->InitPaStream(processConfig, sessionId, false);
    auto streamListener = std::make_shared<ConcreteIStreamListener>();
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->stream_ = std::make_shared<PaCapturerStreamImpl>(stream, processConfig, mainloop);
    ASSERT_NE(nullptr, capturerInServer_);

    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    size_t cacheSize = 1024;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->status_.store(I_STATUS_STARTED);
    capturerInServer_->ringCache_ = std::make_unique<AudioRingCache>(cacheSize);
    int8_t outputData[10] = {1, 2, 3, 4};
    auto ret = capturerInServer_->OnReadData(outputData, 10);
    EXPECT_EQ(SUCCESS, ret);
    
    capturerInServer_->spanSizeInBytes_ = 10;
    ret = capturerInServer_->OnReadData(outputData, 10);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_040.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_040, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    processConfig.innerCapMode = INVALID_CAP_MODE;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_RELEASING;
    capturerInServer_->needCheckBackground_ = true;
    auto result = capturerInServer_->Release();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_041.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_041, TestSize.Level1)
{
    size_t targetSize = 17 * 1024 * 1024;
    size_t cacheSize = 960;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->ringCache_ = std::make_unique<AudioRingCache>(cacheSize);
    capturerInServer_->spanSizeInBytes_ = 1;
    int32_t result = capturerInServer_->InitCacheBuffer(targetSize);
    EXPECT_EQ(ERR_OPERATION_FAILED, result);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerUnitTest_042.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerUnitTest, CapturerInServerUnitTest_042, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    RestoreInfo restoreInfo;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    auto bufferInfo = std::make_shared<BasicBufferInfo>();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_ = bufferInfo.get();
    capturerInServer_->audioServerBuffer_->ohAudioBufferBase_.basicBufferInfo_->restoreStatus.store(NEED_RESTORE);
    auto ret = capturerInServer_->RestoreSession(restoreInfo);
    EXPECT_EQ(NEED_RESTORE, ret);
}

/**
 * @tc.name  : Test ConfigServerBuffer.
 * @tc.type  : FUNC
 * @tc.number: ConfigServerBuffer_001.
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerUnitTest, ConfigServerBuffer_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_001.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, OnStatusUpdate_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_RELEASED;

    capturerInServer_->OnStatusUpdate(OPERATION_STARTED);

    EXPECT_EQ(capturerInServer_->status_, I_STATUS_RELEASED);
}

/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_002
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerUnitTest, OnStatusUpdate_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_STARTED;

    capturerInServer_->OnStatusUpdate(static_cast<IOperation>(999));

    EXPECT_NE(capturerInServer_->status_, I_STATUS_INVALID);
}

/**
 * @tc.name  : Test ReleaseCaptureInjector.
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_001.
 * @tc.desc  : Test ReleaseCaptureInjector interface.
 */
HWTEST_F(CapturerInServerUnitTest, ReleaseCaptureInjector_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    EXPECT_NE(capturerInServer_, nullptr);
    capturerInServer_->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test ReleaseCaptureInjector.
 * @tc.type  : FUNC
 * @tc.number: ReleaseCaptureInjector_002.
 * @tc.desc  : Test ReleaseCaptureInjector interface.
 */
HWTEST_F(CapturerInServerUnitTest, ReleaseCaptureInjector_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    EXPECT_NE(capturerInServer_, nullptr);
    capturerInServer_->ReleaseCaptureInjector();
}

/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: StopSession_001
 * @tc.desc  : Test StopSession interface.
 */
HWTEST_F(CapturerInServerUnitTest, StopSession_001, TestSize.Level3)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
 
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    auto result = capturerInServer_->StopSession();
 
    EXPECT_EQ(result, SUCCESS);
}
 
/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: ResolveBufferBaseAndGetServerSpanSize_001
 * @tc.desc  : Test ResolveBufferBaseAndGetServerSpanSize interface.
 */
HWTEST_F(CapturerInServerUnitTest, ResolveBufferBaseAndGetServerSpanSize_001, TestSize.Level3)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<OHAudioBufferBase> buffer;
    uint32_t spanSizeInFrame = 1;
    uint64_t engineTotalSizeInFrame = 1;
 
    auto result = capturerInServer_->ResolveBufferBaseAndGetServerSpanSize(
                    buffer, spanSizeInFrame, engineTotalSizeInFrame);
 
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}
} // namespace AudioStandard
} // namespace OHOS
