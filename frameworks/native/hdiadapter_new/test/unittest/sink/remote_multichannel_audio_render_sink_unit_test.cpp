/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "sink/remote_multichannel_audio_render_sink.h"
#include "../include/sink/mock_remote_i_audio_render.h"

using namespace testing::ext;
using namespace OHOS::HDI::DistributedAudio::Audio::V2_0;

namespace OHOS {
namespace AudioStandard {
constexpr float TEST_VOLUME_LEFT = 1.0f;
constexpr float TEST_VOLUME_RIGHT = 1.0f;
constexpr float TEST_VOLUME_ZERO = 0.0f;
constexpr size_t TEST_DATA_SIZE = 1024;
constexpr int64_t TEST_LAST_FRAME_START_TIME = 100;
constexpr uint32_t TEST_RENDER_FRAME_NUM = 5;
constexpr float TEST_MAX_AMPLITUDE = 0.5f;

class RemoteMultichannelAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

protected:
    std::shared_ptr<RemoteMultichannelAudioRenderSink> sink_;
};

void RemoteMultichannelAudioRenderSinkUnitTest::SetUpTestCase() {}

void RemoteMultichannelAudioRenderSinkUnitTest::TearDownTestCase() {}

void RemoteMultichannelAudioRenderSinkUnitTest::SetUp()
{
    sink_ = std::make_shared<RemoteMultichannelAudioRenderSink>("test");
    sink_->sinkInited_.store(true);
    sink_->renderInited_.store(true);
    sink_->validState_.store(true);
}

void RemoteMultichannelAudioRenderSinkUnitTest::TearDown()
{
    sink_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, SetVolume_LeftZero_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetVolume(TEST_VOLUME_RIGHT)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_ZERO, TEST_VOLUME_RIGHT);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_ZERO);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_RIGHT);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, SetVolume_RightZero_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetVolume(TEST_VOLUME_LEFT)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_LEFT, TEST_VOLUME_ZERO);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_LEFT);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_ZERO);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, SetVolume_BothNonZero_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    float expectedAvgVolume = 1.0f;
    EXPECT_CALL(*mockRender, SetVolume(expectedAvgVolume)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    int32_t ret = sink_->SetVolume(TEST_VOLUME_LEFT, TEST_VOLUME_RIGHT);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->leftVolume_, TEST_VOLUME_LEFT);
    EXPECT_EQ(sink_->rightVolume_, TEST_VOLUME_RIGHT);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, UpdateActiveDevice_SameDevice_001, TestSize.Level1)
{
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, UpdateActiveDevice_DifferentDevice_001, TestSize.Level1)
{
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_BLUETOOTH_A2DP };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, OnAudioParamChange_ParamKeyState_001, TestSize.Level1)
{
    sink_->sinkInited_.store(true);
    sink_->started_.store(true);
    sink_->OnAudioParamChange("adapter", AudioParamKey::PARAM_KEY_STATE, "condition", "value");
    EXPECT_FALSE(sink_->sinkInited_.load());
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, OnAudioParamChange_OtherKey_001, TestSize.Level1)
{
    sink_->sinkInited_.store(true);
    sink_->started_.store(true);
    sink_->OnAudioParamChange("adapter", AudioParamKey::NONE, "condition", "value");
    EXPECT_TRUE(sink_->sinkInited_.load());
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, RenderFrame_SwitchDeviceMute_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->switchDeviceMute_ = true;
    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, TEST_DATA_SIZE, writeLen);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(data[0], 0);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, RenderFrame_NoMute_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->switchDeviceMute_ = false;
    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, TEST_DATA_SIZE, writeLen);
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, RenderFrame_LogModeTrue_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->switchDeviceMute_ = false;
    sink_->logMode_ = 1;
    char data[TEST_DATA_SIZE];
    for (size_t i = 0; i < TEST_DATA_SIZE; i++) {
        data[i] = static_cast<char>(0xFF);
    }
    uint64_t writeLen = 0;
    int32_t ret = sink_->RenderFrame(*data, TEST_DATA_SIZE, writeLen);
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, Stop_AlreadyStopped_001, TestSize.Level1)
{
    sink_->started_.store(false);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, Stop_NormalStop_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Stop()).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->started_.store(true);
    sink_->validState_.store(true);
    int32_t ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckUpdateState_StartUpdateTrue_001, TestSize.Level1)
{
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->startUpdate_ = true;
    sink_->renderFrameNum_ = 0;
    sink_->lastGetMaxAmplitudeTime_ = 0;
    sink_->last10FrameStartTime_ = TEST_LAST_FRAME_START_TIME;
    char data[TEST_DATA_SIZE] = {0};
    sink_->CheckUpdateState(data, TEST_DATA_SIZE);
    EXPECT_TRUE(sink_->startUpdate_);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckUpdateState_FrameThreshold_001, TestSize.Level1)
{
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->startUpdate_ = true;
    sink_->renderFrameNum_ = RemoteMultichannelAudioRenderSink::GET_MAX_AMPLITUDE_FRAMES_THRESHOLD - 1;
    sink_->lastGetMaxAmplitudeTime_ = 0;
    sink_->last10FrameStartTime_ = TEST_LAST_FRAME_START_TIME;
    char data[TEST_DATA_SIZE] = {0};
    sink_->CheckUpdateState(data, TEST_DATA_SIZE);
    EXPECT_EQ(sink_->renderFrameNum_, 0);
    EXPECT_FALSE(sink_->startUpdate_);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckUpdateState_StartUpdateFalse_001, TestSize.Level1)
{
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->startUpdate_ = false;
    sink_->renderFrameNum_ = TEST_RENDER_FRAME_NUM;
    char data[TEST_DATA_SIZE] = {0};
    sink_->CheckUpdateState(data, TEST_DATA_SIZE);
    EXPECT_EQ(sink_->renderFrameNum_, TEST_RENDER_FRAME_NUM);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, ResetActiveDeviceForDisconnect_SameDevice_001, TestSize.Level1)
{
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    sink_->ResetActiveDeviceForDisconnect(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(sink_->currentActiveDevice_, DEVICE_TYPE_NONE);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, ResetActiveDeviceForDisconnect_DiffDevice_001, TestSize.Level1)
{
    sink_->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    sink_->ResetActiveDeviceForDisconnect(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(sink_->currentActiveDevice_, DEVICE_TYPE_SPEAKER);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, PcmFormatToBit_001, TestSize.Level1)
{
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_U8), PCM_8_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S16LE), PCM_16_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S24LE), PCM_24_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S32LE), PCM_32_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_F32LE), PCM_16_BIT);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, ConvertToHdiFormat_001, TestSize.Level1)
{
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_U8), AUDIO_FORMAT_TYPE_PCM_8_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S16LE), AUDIO_FORMAT_TYPE_PCM_16_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S24LE), AUDIO_FORMAT_TYPE_PCM_24_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S32LE), AUDIO_FORMAT_TYPE_PCM_32_BIT);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_F32LE), AUDIO_FORMAT_TYPE_PCM_16_BIT);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, ParseAudioFormat_001, TestSize.Level1)
{
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_16_BIT"), SAMPLE_S16LE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_24_BIT"), SAMPLE_S24LE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_24_BIT_PACKED"), SAMPLE_S24LE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_32_BIT"), SAMPLE_S32LE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::ParseAudioFormat("UNKNOWN"), SAMPLE_S16LE);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, GetAudioCategory_001, TestSize.Level1)
{
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_DEFAULT), AUDIO_IN_MEDIA);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_RINGING), AUDIO_IN_RINGTONE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_VOICE_RINGING), AUDIO_IN_RINGTONE);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CALL), AUDIO_IN_CALL);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CHAT), AUDIO_IN_COMMUNICATION);
    EXPECT_EQ(RemoteMultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_MAX), AUDIO_IN_MEDIA);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    sink_->maxAmplitude_ = TEST_MAX_AMPLITUDE;
    float amplitude = sink_->GetMaxAmplitude();
    EXPECT_EQ(amplitude, TEST_MAX_AMPLITUDE);
    EXPECT_TRUE(sink_->startUpdate_);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, Destructor_SinkInited_001, TestSize.Level1)
{
    auto tempSink = std::make_shared<RemoteMultichannelAudioRenderSink>("test");
    tempSink->sinkInited_.store(true);
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetExtraParams(testing::_)).WillRepeatedly(testing::Return(SUCCESS));
    tempSink->audioRender_ = mockRender;
    tempSink->sinkInited_.store(false);
    tempSink->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, Start_RenderInited_ValidState_001, TestSize.Level1)
{
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Start()).WillOnce(testing::Return(SUCCESS));
    sink_->audioRender_ = mockRender;
    sink_->renderInited_.store(true);
    sink_->started_.store(false);
    sink_->validState_.store(true);
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    sink_->audioRender_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, InitAudioSampleAttr_FrameSizeNonZero_001, TestSize.Level1)
{
    AudioSampleAttributes param = {};
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.sampleRate = RemoteMultichannelAudioRenderSink::AUDIO_SAMPLE_RATE_48K;
    sink_->InitAudioSampleAttr(param);
    EXPECT_NE(param.frameSize, 0);
    EXPECT_NE(param.startThreshold, 0);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, InitAudioSampleAttr_FrameSizeZero_001, TestSize.Level1)
{
    AudioSampleAttributes param = {};
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->attr_.channel = 0;
    sink_->attr_.sampleRate = RemoteMultichannelAudioRenderSink::AUDIO_SAMPLE_RATE_48K;
    sink_->InitAudioSampleAttr(param);
    EXPECT_EQ(param.frameSize, 0);
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckLatencySignal_TimeGEAndDetected_001, TestSize.Level1)
{
    sink_->attr_.sampleRate = SAMPLE_RATE_48000;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    sink_->signalDetectAgent_->channels_ = sink_->attr_.channel;
    sink_->signalDetectAgent_->sampleRate_ = sink_->attr_.sampleRate;
    sink_->signalDetectAgent_->sampleFormat_ = sink_->attr_.format;
    sink_->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(sink_->attr_.format);
    sink_->signalDetectAgent_->signalDetected_ = true;
    sink_->signalDetectAgent_->lastPeakBufferTime_ = "2025-01-01-00:00:00";
    sink_->signalDetectedTime_ = MILLISECOND_PER_SECOND;
    LatencyMonitor::GetInstance().UpdateDspTime("2025-01-01-00:00:00:0002025-01-01-00:00:00:000");
    std::vector<uint8_t> buffer(sink_->attr_.channel * GetFormatByteSize(sink_->attr_.format), 0);
    sink_->CheckLatencySignal(buffer.data(), buffer.size());
    EXPECT_FALSE(sink_->signalDetectAgent_->signalDetected_);
    LatencyMonitor::GetInstance().UpdateDspTime("");
    sink_->signalDetectAgent_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckLatencySignal_TimeGEAndNotDetected_001, TestSize.Level1)
{
    sink_->attr_.sampleRate = SAMPLE_RATE_48000;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    sink_->signalDetectAgent_->channels_ = sink_->attr_.channel;
    sink_->signalDetectAgent_->sampleRate_ = sink_->attr_.sampleRate;
    sink_->signalDetectAgent_->sampleFormat_ = sink_->attr_.format;
    sink_->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(sink_->attr_.format);
    sink_->signalDetectAgent_->signalDetected_ = false;
    sink_->signalDetectedTime_ = MILLISECOND_PER_SECOND;
    std::vector<uint8_t> buffer(sink_->attr_.channel * GetFormatByteSize(sink_->attr_.format), 0);
    sink_->CheckLatencySignal(buffer.data(), buffer.size());
    EXPECT_FALSE(sink_->signalDetectAgent_->signalDetected_);
    sink_->signalDetectAgent_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckLatencySignal_TimeLTAndDetected_001, TestSize.Level1)
{
    sink_->attr_.sampleRate = SAMPLE_RATE_48000;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    sink_->signalDetectAgent_->channels_ = sink_->attr_.channel;
    sink_->signalDetectAgent_->sampleRate_ = sink_->attr_.sampleRate;
    sink_->signalDetectAgent_->sampleFormat_ = sink_->attr_.format;
    sink_->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(sink_->attr_.format);
    sink_->signalDetectAgent_->signalDetected_ = true;
    sink_->signalDetectAgent_->lastPeakBufferTime_ = "2025-01-01-00:00:00";
    sink_->signalDetectedTime_ = 0;
    std::vector<uint8_t> buffer(sink_->attr_.channel * GetFormatByteSize(sink_->attr_.format), 0);
    sink_->CheckLatencySignal(buffer.data(), buffer.size());
    EXPECT_TRUE(sink_->signalDetectAgent_->signalDetected_);
    sink_->signalDetectAgent_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckLatencySignal_TimeLTAndNotDetected_001, TestSize.Level1)
{
    sink_->attr_.sampleRate = SAMPLE_RATE_48000;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    sink_->signalDetectAgent_->channels_ = sink_->attr_.channel;
    sink_->signalDetectAgent_->sampleRate_ = sink_->attr_.sampleRate;
    sink_->signalDetectAgent_->sampleFormat_ = sink_->attr_.format;
    sink_->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(sink_->attr_.format);
    sink_->signalDetectAgent_->signalDetected_ = false;
    sink_->signalDetectedTime_ = 0;
    std::vector<uint8_t> buffer(sink_->attr_.channel * GetFormatByteSize(sink_->attr_.format), 0);
    sink_->CheckLatencySignal(buffer.data(), buffer.size());
    EXPECT_FALSE(sink_->signalDetectAgent_->signalDetected_);
    sink_->signalDetectAgent_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, CheckLatencySignal_SignalDetectedTrue_001, TestSize.Level1)
{
    sink_->attr_.sampleRate = SAMPLE_RATE_48000;
    sink_->attr_.channel = RemoteMultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    sink_->attr_.format = SAMPLE_S16LE;
    sink_->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    sink_->signalDetectAgent_->channels_ = sink_->attr_.channel;
    sink_->signalDetectAgent_->sampleRate_ = sink_->attr_.sampleRate;
    sink_->signalDetectAgent_->sampleFormat_ = sink_->attr_.format;
    sink_->signalDetectAgent_->formatByteSize_ = GetFormatByteSize(sink_->attr_.format);
    std::vector<int16_t> firstFrame = {100, 100};
    sink_->CheckLatencySignal(reinterpret_cast<uint8_t*>(firstFrame.data()), firstFrame.size() * sizeof(int16_t));
    const size_t framesToDetect = 5000;
    std::vector<uint8_t> silentBuffer(framesToDetect * sink_->attr_.channel *
        sink_->signalDetectAgent_->formatByteSize_, 0);
    sink_->CheckLatencySignal(silentBuffer.data(), silentBuffer.size());
    EXPECT_TRUE(sink_->signalDetected_);
    EXPECT_EQ(sink_->signalDetectedTime_, 0u);
    sink_->signalDetectAgent_ = nullptr;
}

HWTEST_F(RemoteMultichannelAudioRenderSinkUnitTest, SetSinkMuteForSwitchDevice_001, TestSize.Level1)
{
    auto remoteMchSink = std::make_shared<RemoteMultichannelAudioRenderSink>("test");
    ASSERT_NE(remoteMchSink, nullptr);

    int32_t ret = remoteMchSink->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 1);
    EXPECT_TRUE(remoteMchSink->switchDeviceMute_.load());

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 2);
    EXPECT_TRUE(remoteMchSink->switchDeviceMute_.load());

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 1);
    EXPECT_TRUE(remoteMchSink->switchDeviceMute_.load());

    ret = remoteMchSink->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(remoteMchSink->muteCount_, 0);
    EXPECT_FALSE(remoteMchSink->switchDeviceMute_.load());
}

} // namespace AudioStandard
} // namespace OHOS