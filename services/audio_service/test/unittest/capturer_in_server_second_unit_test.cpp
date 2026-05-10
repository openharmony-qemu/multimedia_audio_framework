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
#include "capturer_in_server.h"
#include "record_privacy_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class CapturerInServerSecondUnitTest : public testing::Test {
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

void CapturerInServerSecondUnitTest::SetUpTestCase() {}
void CapturerInServerSecondUnitTest::TearDownTestCase() {}
void CapturerInServerSecondUnitTest::SetUp() {}
void CapturerInServerSecondUnitTest::TearDown() {}

class ConcreteIStreamListener : public IStreamListener {
    int32_t OnOperationHandled(Operation operation, int64_t result) { return SUCCESS; }
};

class CapturerSpanSizeListener : public IStreamListener {
public:
    int32_t OnOperationHandled(Operation operation, int64_t result) override
    {
        operation_ = operation;
        result_ = result;
        notifyCount_++;
        return SUCCESS;
    }

    Operation operation_ = UPDATE_SPANSIZE;
    int64_t result_ = FASTSTATUS_INVALID;
    int32_t notifyCount_ = 0;
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
    void AbortCallback(int32_t abortTimes) override { return; }
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
    void AbortCallback(int32_t abortTimes) override { return; }
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

class ICapturerStreamRouteFlagTest : public ICapturerStreamTest2 {
public:
    explicit ICapturerStreamRouteFlagTest(uint32_t routeFlag) : routeFlag_(routeFlag) {}

    uint32_t GetRouteFlag() const noexcept override
    {
        return routeFlag_;
    }

private:
    uint32_t routeFlag_ = 0;
};

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerSecondUnitTest_004.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_004, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_005.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_005, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_006.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_006, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_007.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_007, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_019.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_019, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_020.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_020, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, UpdatePlaybackCaptureConfigInLegacy_001, TestSize.Level3)
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
 * @tc.number: CapturerInServerSecondUnitTest_021.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_021, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_022.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_022, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_023.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_023, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_024.
 * @tc.desc  : Test UpdatePlaybackCaptureConfig interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_024, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_025.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_025, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_026.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_026, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_027.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_027, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_028.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_028, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_029.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_029, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_030.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_030, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, DrainAudioBuffer_001, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, ResolveBuffer_001, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, OnReadData_001, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, HandleOperationFlushed_001, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, GetLastAudioDuration_001, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_033.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_033, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_034
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_034, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_035
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_035, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_037.
 * @tc.desc  : Test TurnOnMicIndicator interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_037, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    CapturerState capturerState = CAPTURER_NEW;
    capturerInServer_->processConfig_.appInfo.appFullTokenId = (static_cast<uint64_t>(1) << 32);
    auto ret = capturerInServer_->CheckBgRecordPermission(capturerState);
    EXPECT_EQ(ret, true);

    capturerInServer_->isMicIndicatorOn_ = true;
    ret = capturerInServer_->TurnOnMicIndicator();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerSecondUnitTest_038.
 * @tc.desc  : Test TurnOffMicIndicator interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_038, TestSize.Level1)
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
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerSecondUnitTest_040.
 * @tc.desc  : Test Release interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_040, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_041.
 * @tc.desc  : Test InitCacheBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_041, TestSize.Level1)
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
 * @tc.number: CapturerInServerSecondUnitTest_042.
 * @tc.desc  : Test GetAudioTime interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_042, TestSize.Level1)
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

HWTEST_F(CapturerInServerSecondUnitTest, StartInner_TurnOnMicIndicatorFail_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    processConfig.capturerInfo.isLoopback = false;
    processConfig.appInfo.appTokenId = 0;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);

    auto &manager = RecordPrivacyManager::GetInstance();
    manager.tokenIdRecordMap_.clear();
    capturerInServer_->status_ = I_STATUS_IDLE;
    capturerInServer_->streamIndex_ = 202;
    capturerInServer_->stream_ = std::make_shared<ICapturerStreamTest1>();
    SwitchStreamInfo info = {
        capturerInServer_->streamIndex_,
        processConfig.callerUid,
        processConfig.appInfo.appUid,
        processConfig.appInfo.appPid,
        processConfig.appInfo.appTokenId,
        CAPTURER_RUNNING,
    };

    EXPECT_TRUE(SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_WAITING));
    info.nextState = CAPTURER_PREPARED;
    EXPECT_TRUE(SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_CREATED));
    auto ret = capturerInServer_->StartInner();
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    EXPECT_FALSE(capturerInServer_->isMicIndicatorOn_);
    manager.tokenIdRecordMap_.clear();
}

/**
 * @tc.name  : Test ConfigServerBuffer.
 * @tc.type  : FUNC
 * @tc.number: ConfigServerBuffer_001.
 * @tc.desc  : Test ConfigServerBuffer interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, ConfigServerBuffer_001, TestSize.Level1)
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
 * @tc.name  : Test RebuildCaptureInjector.
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_001.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, RebuildCaptureInjector_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, capturerInServer_);
    capturerInServer_->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test RebuildCaptureInjector.
 * @tc.type  : FUNC
 * @tc.number: RebuildCaptureInjector_002.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, RebuildCaptureInjector_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    EXPECT_NE(nullptr, capturerInServer_);
    capturerInServer_->RebuildCaptureInjector();
}

/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_001.
 * @tc.desc  : Test OnStatusUpdate interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, OnStatusUpdate_001, TestSize.Level1)
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
HWTEST_F(CapturerInServerSecondUnitTest, OnStatusUpdate_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    capturerInServer_->status_ = I_STATUS_STARTED;

    capturerInServer_->OnStatusUpdate(static_cast<IOperation>(999));

    EXPECT_NE(capturerInServer_->status_, I_STATUS_INVALID);
}

/**
 * @tc.name  : Test OnStatusUpdate.
 * @tc.type  : FUNC
 * @tc.number: StopSession_001
 * @tc.desc  : Test StopSession interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, StopSession_001, TestSize.Level3)
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
HWTEST_F(CapturerInServerSecondUnitTest, ResolveBufferBaseAndGetServerSpanSize_001, TestSize.Level3)
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

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_001
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 12);
}

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_002
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 6);
}

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_003
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_003, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 4);
}

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_004
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_004, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::SAMPLE_S32LE;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 3);
}

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_005
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_005, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::SAMPLE_F32LE;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 3);
}

/**
 * @tc.name  : Test UpdateBufferTimeStamp.
 * @tc.type  : FUNC
 * @tc.number: UpdateBufferTimeStamp_006
 * @tc.desc  : Test UpdateBufferTimeStamp interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, UpdateBufferTimeStamp_006, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    processConfig.streamInfo.format = AudioSampleFormat::INVALID_WIDTH;
    processConfig.streamInfo.channels = static_cast<AudioChannel>(1);
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_TRUE(capturerInServer_ != nullptr);
    capturerInServer_->curProcessPos_ = 0;
    uint32_t totalSizeInFrame = 10;
    uint32_t spanSizeInFrame = 10;
    uint32_t byteSizePerFrame = 10;
    capturerInServer_->audioServerBuffer_ = std::make_shared<OHAudioBuffer>(AudioBufferHolder::AUDIO_CLIENT,
        totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    int32_t result = capturerInServer_->ConfigServerBuffer();
    EXPECT_EQ(result, SUCCESS);

    capturerInServer_->UpdateBufferTimeStamp(12);
    EXPECT_EQ(capturerInServer_->lastPosInc_, 6);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: OnReadData_002.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, OnReadData_002, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    const size_t testDataSize = 0;
    auto testData = std::make_unique<int8_t[]>(testDataSize);
 
    int32_t result = capturerInServer_->OnReadData(testData.get(), testDataSize);
    EXPECT_EQ(result, ERR_READ_FAILED);
}
 
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: OnReadData_003.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, OnReadData_003, TestSize.Level4)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    const size_t testDataSize = 1000;
    auto testData = std::make_unique<int8_t[]>(testDataSize);
 
    int32_t result = capturerInServer_->OnReadData(testData.get(), testDataSize);
    EXPECT_EQ(result, ERR_READ_FAILED);
}
 
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: OnReadData_004.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, OnReadData_004, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    size_t length = 0;
    bool currentStatus = false;
    int32_t result = capturerInServer_->OnReadData(length);
    capturerInServer_->RecordOverflowStatus(currentStatus);
    EXPECT_EQ(result, SUCCESS);
}
 
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: OnReadData_005.
 * @tc.desc  : Test OnReadData interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, OnReadData_005, TestSize.Level4)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    size_t length = 0;
    bool currentStatus = true;
    int32_t result = capturerInServer_->OnReadData(length);
    capturerInServer_->RecordOverflowStatus(currentStatus);
    EXPECT_EQ(result, SUCCESS);
}
 
/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: RequestUserPrivacyAuthority_001.
 * @tc.desc  : Test RequestUserPrivacyAuthority interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, RequestUserPrivacyAuthority_001, TestSize.Level4)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    int32_t result = capturerInServer_->RequestUserPrivacyAuthority(1);
    EXPECT_EQ(result, ERR_OPERATION_FAILED);
}

/*
 * @tc.name  : Test SetInMainThreadState.
 * @tc.type  : FUNC
 * @tc.number: SetInMainThreadState_001
 * @tc.desc  : Test SetInMainThreadState interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, SetInMainThreadState_001, TestSize.Level4)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    auto result = capturerInServer_->SetInMainThreadState(false);
    EXPECT_EQ(capturerInServer_->isInMainThread_, false);
}

/**
 * @tc.name  : Test CapturerInServer.
 * @tc.type  : FUNC
 * @tc.number: SetInMainThreadStateAndReportOverflow_001.
 * @tc.desc  : Test SetInMainThreadState and ReportOverflowEvent combination.
 */
HWTEST_F(CapturerInServerSecondUnitTest, SetInMainThreadStateAndReportOverflow_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);

    int32_t result1 = capturerInServer_->SetInMainThreadState(true);
    EXPECT_EQ(result1, SUCCESS);

    capturerInServer_->overFlowLogFlag_ = 15;
    capturerInServer_->ReportOverflowEvent();

    int32_t result2 = capturerInServer_->SetInMainThreadState(false);
    EXPECT_EQ(result2, SUCCESS);

    capturerInServer_->overFlowLogFlag_ = 30;
    capturerInServer_->ReportOverflowEvent();
}

/*
 * @tc.name  : Test StartPrivacyConfirmDialog.
 * @tc.type  : FUNC
 * @tc.number: StartPrivacyConfirmDialog_001
 * @tc.desc  : Test StartPrivacyConfirmDialog interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, StartPrivacyConfirmDialog_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    auto result = capturerInServer_->StartPrivacyConfirmDialog(1);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test GetCapturerFlagsByRoute.
 * @tc.type  : FUNC
 * @tc.number: GetCapturerFlagsByRoute_001.
 * @tc.desc  : Cover normal, fast, and voip fast route mapping.
 */
HWTEST_F(CapturerInServerSecondUnitTest, GetCapturerFlagsByRoute_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::weak_ptr<IStreamListener> streamListener;
    auto capturerInServer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_NE(capturerInServer, nullptr);

    EXPECT_EQ(capturerInServer->GetCapturerFlagsByRoute(0), AUDIO_FLAG_NORMAL);
    EXPECT_EQ(capturerInServer->GetCapturerFlagsByRoute(AUDIO_INPUT_FLAG_FAST), AUDIO_FLAG_MMAP);
    EXPECT_EQ(capturerInServer->GetCapturerFlagsByRoute(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP),
        AUDIO_FLAG_VOIP_FAST);
}

/**
 * @tc.name  : Test RefreshServerBufferForSpanSizeUpdate.
 * @tc.type  : FUNC
 * @tc.number: RefreshServerBufferForSpanSizeUpdate_001.
 * @tc.desc  : Cover fast status notification when capturer flags change.
 */
HWTEST_F(CapturerInServerSecondUnitTest, RefreshServerBufferForSpanSizeUpdate_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.capturerInfo.capturerFlags = AUDIO_FLAG_NORMAL;
    auto stateListener = std::make_shared<CapturerSpanSizeListener>();
    std::weak_ptr<IStreamListener> streamListener = stateListener;
    auto capturerInServer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_NE(capturerInServer, nullptr);
    capturerInServer->stream_ = std::make_shared<ICapturerStreamRouteFlagTest>(AUDIO_INPUT_FLAG_FAST);

    capturerInServer->RefreshServerBufferForSpanSizeUpdate(stateListener);

    EXPECT_EQ(capturerInServer->processConfig_.capturerInfo.capturerFlags, AUDIO_FLAG_MMAP);
    EXPECT_EQ(stateListener->notifyCount_, 1);
    EXPECT_EQ(stateListener->operation_, UPDATE_SPANSIZE);
    EXPECT_EQ(stateListener->result_, FASTSTATUS_FAST);
}

/**
 * @tc.name  : Test RefreshServerBufferForSpanSizeUpdate.
 * @tc.type  : FUNC
 * @tc.number: RefreshServerBufferForSpanSizeUpdate_002.
 * @tc.desc  : Cover normal status notification and unchanged branch.
 */
HWTEST_F(CapturerInServerSecondUnitTest, RefreshServerBufferForSpanSizeUpdate_002, TestSize.Level1)
{
    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.capturerInfo.capturerFlags = AUDIO_FLAG_MMAP;
    auto stateListener = std::make_shared<CapturerSpanSizeListener>();
    std::weak_ptr<IStreamListener> streamListener = stateListener;
    auto capturerInServer = std::make_shared<CapturerInServer>(processConfig, streamListener);
    ASSERT_NE(capturerInServer, nullptr);
    capturerInServer->stream_ = std::make_shared<ICapturerStreamRouteFlagTest>(0);

    capturerInServer->RefreshServerBufferForSpanSizeUpdate(stateListener);

    EXPECT_EQ(capturerInServer->processConfig_.capturerInfo.capturerFlags, AUDIO_FLAG_NORMAL);
    EXPECT_EQ(stateListener->notifyCount_, 1);
    EXPECT_EQ(stateListener->result_, FASTSTATUS_NORMAL);

    capturerInServer->RefreshServerBufferForSpanSizeUpdate(stateListener);
    EXPECT_EQ(stateListener->notifyCount_, 1);
}
 
/*
 * @tc.name  : Test ReportPlaybackCaptureUserChoice.
 * @tc.type  : FUNC
 * @tc.number: ReportPlaybackCaptureUserChoice_001
 * @tc.desc  : Test ReportPlaybackCaptureUserChoice interface.
 */
HWTEST_F(CapturerInServerSecondUnitTest, ReportPlaybackCaptureUserChoice_001, TestSize.Level1)
{
    AudioProcessConfig processConfig;
    std::shared_ptr<IStreamListener> iStreamListener_ = std::make_shared<ConcreteIStreamListener>();
    std::weak_ptr<IStreamListener> streamListener = iStreamListener_;
    auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
    auto result = capturerInServer_->ReportPlaybackCaptureUserChoice(1, true);
    capturerInServer_->OnPrivacyAuthorityResult(true);
    capturerInServer_->OnPrivacyAuthorityResult(false);
    EXPECT_NE(result, SUCCESS);
}

/*
 * @tc.name  : Test CapturerInServer destructor loopback effect cleanup branch.
 * @tc.type  : FUNC
 * @tc.number: CapturerInServerSecondUnitTest_Destructor_LoopBackEffectEnabled_001
 * @tc.desc  : Cover false and true branches of loopback effect cleanup in destructor.
 */
HWTEST_F(CapturerInServerSecondUnitTest, CapturerInServerSecondUnitTest_Destructor_LoopBackEffectEnabled_001,
    TestSize.Level1)
{
    {
        AudioProcessConfig processConfig;
        processConfig.capturerInfo.loopBackEffectEnabled = false;
        std::weak_ptr<IStreamListener> streamListener;
        auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
        ASSERT_NE(capturerInServer_, nullptr);
        capturerInServer_->status_ = I_STATUS_RELEASED;
        capturerInServer_ = nullptr;
    }

    {
        AudioProcessConfig processConfig;
        processConfig.capturerInfo.loopBackEffectEnabled = true;
        std::weak_ptr<IStreamListener> streamListener;
        auto capturerInServer_ = std::make_shared<CapturerInServer>(processConfig, streamListener);
        ASSERT_NE(capturerInServer_, nullptr);
        capturerInServer_->status_ = I_STATUS_RELEASED;
        capturerInServer_ = nullptr;
    }
}
} // namespace AudioStandard
} // namespace OHOS
