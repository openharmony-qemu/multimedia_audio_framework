/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/remote_audio_render_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class RemoteAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
};

uint32_t RemoteAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> RemoteAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr RemoteAudioRenderSinkUnitTest::attr_ = {};

void RemoteAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "test", true);
}

void RemoteAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void RemoteAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
    attr_.channel = 2; // 2: channel
    sink_->Init(attr_);
}

void RemoteAudioRenderSinkUnitTest::TearDown()
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_001
 * @tc.desc   : Test remote sink create
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_002
 * @tc.desc   : Test remote sink init
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    sink_->DeInit();
    int32_t ret = sink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(sink_->IsInited());
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_003
 * @tc.desc   : Test remote sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Pause();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Flush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Reset();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : RemoteSinkUnitTest_004
 * @tc.desc   : Test remote sink set/get volume
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_005
 * @tc.desc   : Test remote sink set audio scene
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_006
 * @tc.desc   : Test remote sink update active device
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test RemoteSink API
 * @tc.number : RemoteSinkUnitTest_007
 * @tc.desc   : Test remote sink update app uid
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    std::vector<int32_t> appsUid = {};
    int32_t ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    appsUid.push_back(20000001);
    appsUid.push_back(20000002);
    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    ret = sink_->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    appsUid.clear();
    ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);

    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test RemoteSink CheckLatencySignal
 * @tc.number : RemoteSinkUnitTest_008
 * @tc.desc   : Verify latency monitor update branch when signal detected flag is set
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_008, TestSize.Level1)
{
    auto remoteSink = std::static_pointer_cast<RemoteAudioRenderSink>(sink_);
    ASSERT_NE(remoteSink, nullptr);

    IAudioSinkAttr originalAttr = remoteSink->attr_;
    auto originalAgent = remoteSink->signalDetectAgent_;
    size_t originalDetectedTime = remoteSink->signalDetectedTime_;
    remoteSink->attr_.sampleRate = SAMPLE_RATE_48000;
    remoteSink->attr_.channel = STEREO;
    remoteSink->attr_.format = AudioSampleFormat::SAMPLE_S16LE;
    std::vector<uint8_t> buffer(remoteSink->attr_.channel * GetFormatByteSize(remoteSink->attr_.format), 0);
    remoteSink->CheckLatencySignal(buffer.data(), buffer.size());

    remoteSink->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    ASSERT_NE(remoteSink->signalDetectAgent_, nullptr);
    remoteSink->signalDetectAgent_->channels_ = remoteSink->attr_.channel;
    remoteSink->signalDetectAgent_->sampleRate_ = remoteSink->attr_.sampleRate;
    remoteSink->signalDetectAgent_->sampleFormat_ = remoteSink->attr_.format;
    remoteSink->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(remoteSink->attr_.format);
    remoteSink->signalDetectAgent_->signalDetected_ = true;
    remoteSink->signalDetectAgent_->lastPeakBufferTime_ = "2025-01-01-00:00:00";

    // Prepare dsp time to avoid out_of_range in ShowTimestamp
    LatencyMonitor::GetInstance().UpdateDspTime("2025-01-01-00:00:00:0002025-01-01-00:00:00:000");

    remoteSink->signalDetectedTime_ = MILLISECOND_PER_SECOND;
    remoteSink->CheckLatencySignal(buffer.data(), buffer.size());

    EXPECT_FALSE(remoteSink->signalDetectAgent_->signalDetected_);
    EXPECT_GE(remoteSink->signalDetectedTime_, static_cast<size_t>(MILLISECOND_PER_SECOND));

    LatencyMonitor::GetInstance().UpdateDspTime("");
    remoteSink->signalDetectAgent_ = originalAgent;
    remoteSink->attr_ = originalAttr;
    remoteSink->signalDetectedTime_ = originalDetectedTime;
}

/**
 * @tc.name   : Test RemoteSink CheckLatencySignal
 * @tc.number : RemoteSinkUnitTest_009
 * @tc.desc   : Verify latency signal detection resets timer
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_009, TestSize.Level1)
{
    auto remoteSink = std::static_pointer_cast<RemoteAudioRenderSink>(sink_);
    ASSERT_NE(remoteSink, nullptr);

    IAudioSinkAttr originalAttr = remoteSink->attr_;
    auto originalAgent = remoteSink->signalDetectAgent_;
    bool originSignalDetected = remoteSink->signalDetected_;
    size_t originSignalDetectedTime = remoteSink->signalDetectedTime_;

    remoteSink->attr_.sampleRate = SAMPLE_RATE_48000;
    remoteSink->attr_.channel = STEREO;
    remoteSink->attr_.format = AudioSampleFormat::SAMPLE_S16LE;

    remoteSink->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    ASSERT_NE(remoteSink->signalDetectAgent_, nullptr);
    remoteSink->signalDetectAgent_->channels_ = remoteSink->attr_.channel;
    remoteSink->signalDetectAgent_->sampleRate_ = remoteSink->attr_.sampleRate;
    remoteSink->signalDetectAgent_->sampleFormat_ = remoteSink->attr_.format;
    remoteSink->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(remoteSink->attr_.format);

    std::vector<int16_t> firstFrame = {1, 1};
    remoteSink->CheckLatencySignal(reinterpret_cast<uint8_t *>(firstFrame.data()),
        firstFrame.size() * sizeof(int16_t));

    const size_t framesToDetect = 5000;
    std::vector<uint8_t> silentBuffer(framesToDetect * remoteSink->attr_.channel *
        remoteSink->signalDetectAgent_->formatByteSize_, 0);
    remoteSink->CheckLatencySignal(silentBuffer.data(), silentBuffer.size());

    EXPECT_TRUE(remoteSink->signalDetected_);
    EXPECT_EQ(remoteSink->signalDetectedTime_, 0u);
    EXPECT_TRUE(remoteSink->signalDetectAgent_->signalDetected_);

    remoteSink->signalDetectAgent_ = originalAgent;
    remoteSink->attr_ = originalAttr;
    remoteSink->signalDetected_ = originSignalDetected;
    remoteSink->signalDetectedTime_ = originSignalDetectedTime;
}

/**
 * @tc.name   : Test RemoteSink CheckLatencySignal
 * @tc.number : RemoteSinkUnitTest_010
 * @tc.desc   : Verify latency signal detection resets timer
 */
HWTEST_F(RemoteAudioRenderSinkUnitTest, RemoteSinkUnitTest_010, TestSize.Level1)
{
    auto remoteSink = std::static_pointer_cast<RemoteAudioRenderSink>(sink_);
    ASSERT_NE(remoteSink, nullptr);

    IAudioSinkAttr originalAttr = remoteSink->attr_;
    auto originalAgent = remoteSink->signalDetectAgent_;
    bool originSignalDetected = remoteSink->signalDetected_;
    size_t originSignalDetectedTime = remoteSink->signalDetectedTime_;

    remoteSink->attr_.sampleRate = SAMPLE_RATE_48000;
    remoteSink->attr_.channel = STEREO;
    remoteSink->attr_.format = AudioSampleFormat::SAMPLE_S16LE;

    remoteSink->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    ASSERT_NE(remoteSink->signalDetectAgent_, nullptr);
    remoteSink->signalDetectAgent_->channels_ = remoteSink->attr_.channel;
    remoteSink->signalDetectAgent_->sampleRate_ = remoteSink->attr_.sampleRate;
    remoteSink->signalDetectAgent_->sampleFormat_ = remoteSink->attr_.format;
    remoteSink->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(remoteSink->attr_.format);

    // Use values above DETECTED_ZERO_THRESHOLD to ensure non-zero path
    std::vector<int16_t> firstFrame = {100, 100};
    remoteSink->CheckLatencySignal(reinterpret_cast<uint8_t *>(firstFrame.data()),
        firstFrame.size() * sizeof(int16_t));

    const size_t framesToDetect = 5000;
    std::vector<uint8_t> silentBuffer(framesToDetect * remoteSink->attr_.channel *
        remoteSink->signalDetectAgent_->formatByteSize_, 0);
    remoteSink->CheckLatencySignal(silentBuffer.data(), silentBuffer.size());

    EXPECT_TRUE(remoteSink->signalDetected_);
    EXPECT_EQ(remoteSink->signalDetectedTime_, 0u);
    EXPECT_TRUE(remoteSink->signalDetectAgent_->signalDetected_);

    remoteSink->signalDetectAgent_ = originalAgent;
    remoteSink->attr_ = originalAttr;
    remoteSink->signalDetected_ = originSignalDetected;
    remoteSink->signalDetectedTime_ = originSignalDetectedTime;
}
} // namespace AudioStandard
} // namespace OHOS
