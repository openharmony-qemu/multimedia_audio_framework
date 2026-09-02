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

#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_gain_node.h"
#include "hpae_pcm_buffer.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "audio_volume.h"
#include "audio_info.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeGainNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeGainNodeTest::SetUp()
{}

void HpaeGainNodeTest::TearDown()
{}

static int32_t g_testValue = 0;

namespace {

constexpr uint32_t DEFAULT_NODE_ID = 1234;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_NUM_TWO = 2;

HWTEST_F(HpaeGainNodeTest, constructHpaeGainNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeGainNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeGainNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeGainNode->GetBitWidth(), nodeInfo.format);
    std::cout << "HpaeGainNodeTest::GetNodeInfo" << std::endl;
    HpaeNodeInfo &retNi = hpaeGainNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    std::cout << "samplingRate: " << retNi.samplingRate << std::endl;
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    std::cout << "frameLen: " << retNi.frameLen << std::endl;
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    std::cout << "channels: " << retNi.channels << std::endl;
    EXPECT_EQ(retNi.format, nodeInfo.format);
    std::cout << "format: " << retNi.format << std::endl;
    EXPECT_EQ(retNi.deviceClass, nodeInfo.deviceClass);
    std::cout << "deviceClass: " << retNi.deviceClass << std::endl;
    std::cout << "HpaeGainNodeTest::GetNodeInfo end" << std::endl;
}
static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    uint64_t frameLen = len / (SAMPLE_F32LE * STEREO);
    const float *tempData = reinterpret_cast<const float *>(data);
    for (int32_t i = 0; i < frameLen; i++) {
        const float left = tempData[DEFAULT_NUM_TWO * i];
        const float right = tempData[DEFAULT_NUM_TWO * i + 1];
        EXPECT_EQ(left, g_testValue);
        EXPECT_EQ(right, g_testValue);
    }
    return 0;
}

HWTEST_F(HpaeGainNodeTest, testHpaeGainTestNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    hpaeGainNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeGainNode);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    g_testValue = 0;
    int32_t testValue = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    g_testValue = testValue;
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb0);
    hpaeSinkOutputNode->DoProcess();
    float gain = 1.0;
    EXPECT_EQ(hpaeGainNode->SetClientVolume(gain), true);
    hpaeGainNode->GetClientVolume();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DisConnect(hpaeGainNode);
    hpaeGainNode->DisConnect(hpaeSinkInputNode);
}

HWTEST_F(HpaeGainNodeTest, SetCollFadeState, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    hpaeGainNode->SetCollFadeState(OPERATION_STARTED);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, true);
    hpaeGainNode->SetCollFadeState(OPERATION_PAUSED);
    EXPECT_EQ(hpaeGainNode->collFadeOutState_, FadeOutState::DO_FADEOUT);
}

HWTEST_F(HpaeGainNodeTest, testDoCollFade, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    hpaeGainNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeGainNode);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);
    g_testValue = 0;
    int32_t testValue = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    g_testValue = testValue;
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb0);
    hpaeGainNode->SetCollFadeState(OPERATION_STARTED);
    hpaeSinkOutputNode->DoProcess();
    hpaeGainNode->SetCollFadeState(OPERATION_STARTED);
    hpaeSinkInputNode->inputAudioBuffer_.SetBufferValid(false);
    hpaeSinkOutputNode->DoProcess();
    
    hpaeGainNode->SetCollFadeState(OPERATION_PAUSED);
    hpaeSinkOutputNode->DoProcess();
    hpaeSinkOutputNode->DoProcess();
    EXPECT_EQ(hpaeGainNode->collFadeInState_, false);
    EXPECT_EQ(hpaeGainNode->collFadeOutState_, FadeOutState::DONE_FADEOUT);
    hpaeSinkOutputNode->DisConnect(hpaeGainNode);
    hpaeGainNode->DisConnect(hpaeSinkInputNode);
}

// --- Tests for ResetVolume and DoGain with cached streamVolume_/pipeVolume_ ---

static constexpr uint32_t UT_SESSION_ID = 9999;
static constexpr uint32_t UT_PIPE_ID = 0;
static constexpr int32_t UT_UID = 1000;
static constexpr int32_t UT_PID = 1000;
static constexpr int32_t UT_VOLUME_MODE = 0;
static constexpr int32_t UT_VOLUME_LEVEL = 100;
static constexpr float UT_UNIT_GAIN = 1.0f;
static constexpr float UT_HALF_GAIN = 0.5f;
static constexpr float UT_PRECISION = 0.001f;
static constexpr float UT_GAIN_PRECISION = 0.01f;
static const std::string UT_DEVICE_CLASS_PRIMARY = "primary";
static const std::string UT_DEVICE_CLASS_INNER_CAPTURER = "libmodule-inner-capturer-sink.z.so";

static void SetupStreamAndPipeVolume(uint32_t sessionId, float streamVol, float pipeVol)
{
    auto *audioVolume = AudioVolume::GetInstance();
    StreamVolumeParams params;
    params.sessionId = sessionId;
    params.streamType = STREAM_MUSIC;
    params.streamUsage = STREAM_USAGE_MUSIC;
    params.uid = UT_UID;
    params.pid = UT_PID;
    params.isSystemApp = false;
    params.mode = UT_VOLUME_MODE;
    params.isVKB = VKB_INVALID;
    audioVolume->AddStreamVolume(params);
    audioVolume->SetStreamVolume(sessionId, streamVol);
    PipeVolume pipeVolume(UT_PIPE_ID, STREAM_MUSIC, pipeVol, UT_VOLUME_LEVEL, false);
    audioVolume->SetPipeVolume(pipeVolume);
}

static void SetupStreamVolumeOnly(uint32_t sessionId, float streamVol)
{
    auto *audioVolume = AudioVolume::GetInstance();
    StreamVolumeParams params;
    params.sessionId = sessionId;
    params.streamType = STREAM_MUSIC;
    params.streamUsage = STREAM_USAGE_MUSIC;
    params.uid = UT_UID;
    params.pid = UT_PID;
    params.isSystemApp = false;
    params.mode = UT_VOLUME_MODE;
    params.isVKB = VKB_INVALID;
    audioVolume->AddStreamVolume(params);
    audioVolume->SetStreamVolume(sessionId, streamVol);
}

static void CleanupStreamAndPipeVolume(uint32_t sessionId)
{
    auto *audioVolume = AudioVolume::GetInstance();
    audioVolume->RemoveStreamVolume(sessionId);
    audioVolume->RemovePipeVolume(UT_PIPE_ID);
}

static void FillBuffer(HpaePcmBuffer &buf, float value)
{
    float *data = buf.GetPcmDataBuffer();
    size_t totalSamples = buf.GetFrameLen() * buf.GetChannelCount();
    for (size_t i = 0; i < totalSamples; i++) {
        data[i] = value;
    }
}

HWTEST_F(HpaeGainNodeTest, ResetVolume_CacheStreamAndPipeVolume_Normal, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = UT_SESSION_ID;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    SetupStreamAndPipeVolume(UT_SESSION_ID, UT_HALF_GAIN, 0.8f);
    hpaeGainNode->ResetVolume();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_NE(hpaeGainNode->pipeVolume_, nullptr);

    CleanupStreamAndPipeVolume(UT_SESSION_ID);
}

HWTEST_F(HpaeGainNodeTest, DoGain_NormalUsesCachedVolume, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 2;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    SetupStreamAndPipeVolume(sessionId, UT_UNIT_GAIN, UT_UNIT_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);
    ASSERT_NE(hpaeGainNode->pipeVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_UNIT_GAIN, UT_PRECISION);
    }

    CleanupStreamAndPipeVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_NormalAppliesGain, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 3;
    constexpr float expectedGain = UT_HALF_GAIN * UT_HALF_GAIN; // pipe * stream
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    SetupStreamAndPipeVolume(sessionId, UT_HALF_GAIN, UT_HALF_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);
    ASSERT_NE(hpaeGainNode->pipeVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // History volume starts at 0, ramp from 0 to expectedGain, last sample close to expectedGain
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[totalSamples - 1], expectedGain, UT_GAIN_PRECISION);

    CleanupStreamAndPipeVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_NullPtrDefaultsToUnitGain, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 5;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Do NOT call ResetVolume, streamVolume_ and pipeVolume_ remain nullptr
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // With nullptr, gain defaults to 1.0f, data should remain unchanged
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_UNIT_GAIN, UT_PRECISION);
    }
}

HWTEST_F(HpaeGainNodeTest, ResetVolume_Normal_PipeVolumeNull, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 7;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Only set stream volume, no pipe volume → pipeVolume_ stays nullptr
    SetupStreamVolumeOnly(sessionId, UT_HALF_GAIN);
    hpaeGainNode->ResetVolume();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, nullptr);
    // Stream-only fallback: ResetVolume records the stream total volume as history (was 1.0f)
    EXPECT_NEAR(AudioVolume::GetInstance()->GetHistoryVolume(sessionId), UT_HALF_GAIN, UT_GAIN_PRECISION);

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_Normal_PipeVolumeNull, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 11;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Only set stream volume, no pipe → pipeVolume_=nullptr after ResetVolume
    SetupStreamVolumeOnly(sessionId, UT_HALF_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);
    ASSERT_EQ(hpaeGainNode->pipeVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // Stream-only fallback: pipeVolume_ is nullptr → stream volume (UT_HALF_GAIN) applied alone.
    // After ResetVolume sets history=0.5, DoGain yields a flat 0.5 gain on the unit-amplitude buffer.
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[totalSamples - 1], UT_HALF_GAIN, UT_GAIN_PRECISION);

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

// --- Tests for ApplyFadeIn multi-frame fade-in branch coverage ---

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_DefaultFade_CompleteInOneFrame, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);

    hpaeGainNode->DoFading(&inputBuf);

    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
    EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[DEFAULT_FRAME_LEN * STEREO - 1],
        UT_UNIT_GAIN, UT_GAIN_PRECISION);
}

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_DefaultFade_MultiFrameComplete, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    // First half silent, second half non-zero → fade starts from middle
    float *data = inputBuf.GetPcmDataBuffer();
    size_t halfSamples = DEFAULT_FRAME_LEN * STEREO / DEFAULT_NUM_TWO;
    for (size_t i = 0; i < halfSamples; i++) {
        data[i] = 0.0f;
    }
    for (size_t i = halfSamples; i < DEFAULT_FRAME_LEN * STEREO; i++) {
        data[i] = UT_UNIT_GAIN;
    }

    // Frame 1: partial fade (half frame), fadeInState_ stays true
    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    hpaeGainNode->DoFading(&inputBuf);

    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_GT(hpaeGainNode->fadeProcessedBytes_, 0);
    EXPECT_LT(hpaeGainNode->fadeProcessedBytes_, hpaeGainNode->totalFadeBytes_);

    // Frame 2: full non-zero buffer → continuation completes the fade
    HpaePcmBuffer inputBuf2(bufInfo);
    FillBuffer(inputBuf2, UT_UNIT_GAIN);
    hpaeGainNode->DoFading(&inputBuf2);

    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
}

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_ContinuationSilent_Aborts, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    // Half silent, half non-zero → partial fade
    float *data = inputBuf.GetPcmDataBuffer();
    size_t halfSamples = DEFAULT_FRAME_LEN * STEREO / DEFAULT_NUM_TWO;
    for (size_t i = 0; i < halfSamples; i++) {
        data[i] = 0.0f;
    }
    for (size_t i = halfSamples; i < DEFAULT_FRAME_LEN * STEREO; i++) {
        data[i] = UT_UNIT_GAIN;
    }

    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    hpaeGainNode->DoFading(&inputBuf);
    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_GT(hpaeGainNode->fadeProcessedBytes_, 0);

    // Frame 2: silent → abort (processedBytes > 0 branch)
    HpaePcmBuffer silentBuf(bufInfo);
    FillBuffer(silentBuf, 0.0f);
    hpaeGainNode->DoFading(&silentBuf);

    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
}

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_ShortFade_CompleteInOneFrame, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = SHORT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    hpaeGainNode->DoFading(&inputBuf);

    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
}

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_SilentFirstFrame_NoFade, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, 0.0f);

    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    hpaeGainNode->DoFading(&inputBuf);

    // Silent first frame: no fade applied, waiting for non-silent frame
    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
}

HWTEST_F(HpaeGainNodeTest, SetFadeState_ResetsProcessedBytes, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    hpaeGainNode->fadeInState_ = true;
    hpaeGainNode->fadeProcessedBytes_ = 100;
    hpaeGainNode->totalFadeBytes_ = 7680;

    hpaeGainNode->SetFadeState(OPERATION_STARTED);
    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 100);
}

HWTEST_F(HpaeGainNodeTest, SetCollFadeState_ResetsCollProcessedBytes, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    hpaeGainNode->collFadeInState_ = true;
    hpaeGainNode->collFadeProcessedBytes_ = 200;
    hpaeGainNode->collTotalFadeBytes_ = 7680;

    hpaeGainNode->SetCollFadeState(OPERATION_STARTED);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, true);
    EXPECT_EQ(hpaeGainNode->collFadeProcessedBytes_, 0);
}

HWTEST_F(HpaeGainNodeTest, ApplyFadeIn_CollFadeIn_MultiFrameComplete, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    float *data = inputBuf.GetPcmDataBuffer();
    size_t halfSamples = DEFAULT_FRAME_LEN * STEREO / DEFAULT_NUM_TWO;
    for (size_t i = 0; i < halfSamples; i++) {
        data[i] = 0.0f;
    }
    for (size_t i = halfSamples; i < DEFAULT_FRAME_LEN * STEREO; i++) {
        data[i] = UT_UNIT_GAIN;
    }

    // Frame 1: coll fade-in partial
    hpaeGainNode->SetCollFadeState(OPERATION_STARTED);
    hpaeGainNode->DoCollFading(&inputBuf);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, true);

    // Frame 2: complete via DoCollFading continuation path
    HpaePcmBuffer inputBuf2(bufInfo);
    FillBuffer(inputBuf2, UT_UNIT_GAIN);
    hpaeGainNode->DoCollFading(&inputBuf2);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, false);
    EXPECT_EQ(hpaeGainNode->collFadeProcessedBytes_, 0);
}

// --- Tests for EnsureVolumeAvailable ---

HWTEST_F(HpaeGainNodeTest, EnsureVolumeAvailable_NormalBothNull_Retrieved, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 12;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, false);

    // Both null initially
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, nullptr);

    SetupStreamAndPipeVolume(sessionId, UT_HALF_GAIN, 0.8f);
    hpaeGainNode->EnsureVolumeAvailable();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_NE(hpaeGainNode->pipeVolume_, nullptr);

    CleanupStreamAndPipeVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, EnsureVolumeAvailable_InjectorStreamNull_Retrieved, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 13;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = VIRTUAL_INJECTOR;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    // streamVolume_ is null for injector capture stream.
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);

    SetupStreamVolumeOnly(sessionId, UT_HALF_GAIN);
    hpaeGainNode->EnsureVolumeAvailable();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, EnsureVolumeAvailable_StreamExistsPipeNull_Retrieved, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 14;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Set up stream only first, then call ResetVolume to cache streamVolume_ only
    SetupStreamVolumeOnly(sessionId, UT_HALF_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);
    ASSERT_EQ(hpaeGainNode->pipeVolume_, nullptr);

    // Now add pipe volume and call EnsureVolumeAvailable to retry pipeVolume_
    PipeVolume pipeVolume(UT_PIPE_ID, STREAM_MUSIC, 0.8f, UT_VOLUME_LEVEL, false);
    AudioVolume::GetInstance()->SetPipeVolume(pipeVolume);
    hpaeGainNode->EnsureVolumeAvailable();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_NE(hpaeGainNode->pipeVolume_, nullptr);

    CleanupStreamAndPipeVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, EnsureVolumeAvailable_BothCached_NoRetry, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 15;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    SetupStreamAndPipeVolume(sessionId, UT_UNIT_GAIN, UT_UNIT_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);
    ASSERT_NE(hpaeGainNode->pipeVolume_, nullptr);

    // Save pointers before retry to verify they are unchanged
    auto savedStream = hpaeGainNode->streamVolume_;
    auto savedPipe = hpaeGainNode->pipeVolume_;

    hpaeGainNode->EnsureVolumeAvailable();

    // Both already cached, no retry should happen, pointers unchanged
    EXPECT_EQ(hpaeGainNode->streamVolume_, savedStream);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, savedPipe);

    CleanupStreamAndPipeVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, EnsureVolumeAvailable_RetryFails_StillNull, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 16;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // No volume registered at all
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, nullptr);

    hpaeGainNode->EnsureVolumeAvailable();

    // Retry failed, still null
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);
    EXPECT_EQ(hpaeGainNode->pipeVolume_, nullptr);
}

// --- Tests for ResetFadeState (clear stale fade state when a node is reused/reconnected) ---

HWTEST_F(HpaeGainNodeTest, ResetFadeState_ClearsStaleDoneFadeout, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Simulate a node left dormant with stale fade state from a previous lifecycle
    // (e.g. DONE_FADEOUT left by a Pause before the node was reused on a device switch).
    hpaeGainNode->fadeOutState_ = FadeOutState::DONE_FADEOUT;
    hpaeGainNode->collFadeOutState_ = FadeOutState::DONE_FADEOUT;
    hpaeGainNode->fadeInState_ = false;
    hpaeGainNode->collFadeInState_ = true;
    hpaeGainNode->fadeProcessedBytes_ = 100;
    hpaeGainNode->totalFadeBytes_ = 7680;
    hpaeGainNode->collFadeProcessedBytes_ = 200;
    hpaeGainNode->collTotalFadeBytes_ = 7680;

    hpaeGainNode->ResetFadeState();

    EXPECT_EQ(hpaeGainNode->fadeOutState_, FadeOutState::NO_FADEOUT);
    EXPECT_EQ(hpaeGainNode->collFadeOutState_, FadeOutState::NO_FADEOUT);
    EXPECT_EQ(hpaeGainNode->fadeInState_, true);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, false);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
    EXPECT_EQ(hpaeGainNode->totalFadeBytes_, 0);
    EXPECT_EQ(hpaeGainNode->collFadeProcessedBytes_, 0);
    EXPECT_EQ(hpaeGainNode->collTotalFadeBytes_, 0);
}

HWTEST_F(HpaeGainNodeTest, ResetFadeState_PreventsSilenceOnReusedNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    nodeInfo.fadeType = DEFAULT_FADE;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);

    // Stale DONE_FADEOUT silences every frame via SignalProcess (the bug).
    hpaeGainNode->fadeOutState_ = FadeOutState::DONE_FADEOUT;
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);
    std::vector<HpaePcmBuffer *> inputs = {&inputBuf};
    hpaeGainNode->SignalProcess(inputs);
    EXPECT_EQ(inputBuf.GetPcmDataBuffer()[DEFAULT_FRAME_LEN * STEREO - 1], 0.0f); // silenced

    // After ResetFadeState the reused node no longer silences data.
    hpaeGainNode->ResetFadeState();
    HpaePcmBuffer inputBuf2(bufInfo);
    FillBuffer(inputBuf2, UT_UNIT_GAIN);
    std::vector<HpaePcmBuffer *> inputs2 = {&inputBuf2};
    hpaeGainNode->SignalProcess(inputs2);
    EXPECT_NE(inputBuf2.GetPcmDataBuffer()[DEFAULT_FRAME_LEN * STEREO - 1], 0.0f); // not silenced
}

// --- Tests for ClearFadeState / HasPendingFade (conditional fade arming on reconnect) ---

HWTEST_F(HpaeGainNodeTest, ClearFadeState_DoesNotArmFadeIn, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Leftover counters from a finished fade must not resurrect a fade-in either.
    hpaeGainNode->fadeProcessedBytes_ = 100;
    hpaeGainNode->totalFadeBytes_ = 7680;

    hpaeGainNode->ClearFadeState();

    // Fade-in is NOT armed: a hot re-plumb of a cleanly playing session keeps gain continuity.
    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
    EXPECT_EQ(hpaeGainNode->fadeOutState_, FadeOutState::NO_FADEOUT);
    EXPECT_EQ(hpaeGainNode->fadeProcessedBytes_, 0);
    EXPECT_EQ(hpaeGainNode->totalFadeBytes_, 0);
    EXPECT_EQ(hpaeGainNode->collFadeInState_, false);
    EXPECT_EQ(hpaeGainNode->collFadeOutState_, FadeOutState::NO_FADEOUT);
}

HWTEST_F(HpaeGainNodeTest, ClearFadeState_ClearsStaleDoneFadeoutWithoutArming, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // Stale DONE_FADEOUT silences every frame via SignalProcess (the reused-node mute bug).
    hpaeGainNode->fadeOutState_ = FadeOutState::DONE_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);

    // ClearFadeState still owns the anti-mute safety net: the gate must be cleared...
    hpaeGainNode->ClearFadeState();
    EXPECT_EQ(hpaeGainNode->fadeOutState_, FadeOutState::NO_FADEOUT);
    // ...but unlike ResetFadeState it must not arm a fade-in for this state.
    EXPECT_EQ(hpaeGainNode->fadeInState_, false);
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), false);
}

HWTEST_F(HpaeGainNodeTest, HasPendingFade_CoversAllFadeLifecycleStates, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_PRIMARY;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);

    // A cleanly playing session: no fade axis pending -> reconnect must keep continuity.
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), false);

    hpaeGainNode->fadeInState_ = true;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->fadeInState_ = false;

    hpaeGainNode->fadeOutState_ = FadeOutState::DO_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->fadeOutState_ = FadeOutState::DONE_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->fadeOutState_ = FadeOutState::NO_FADEOUT;

    hpaeGainNode->collFadeInState_ = true;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->collFadeInState_ = false;

    hpaeGainNode->collFadeOutState_ = FadeOutState::DO_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->collFadeOutState_ = FadeOutState::DONE_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), true);
    hpaeGainNode->collFadeOutState_ = FadeOutState::NO_FADEOUT;
    EXPECT_EQ(hpaeGainNode->HasPendingFade(), false);
}
}  // namespace
