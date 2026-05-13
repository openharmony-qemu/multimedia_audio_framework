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
    float curGain = 0.0f;
    float targetGain = 1.0f;
    float stepGain = targetGain - curGain;
    uint64_t frameLen = len / (SAMPLE_F32LE * STEREO);
    stepGain = stepGain / frameLen;
    const float *tempData = reinterpret_cast<const float *>(data);
    for (int32_t i = 0; i < frameLen; i++) {
        const float left = tempData[DEFAULT_NUM_TWO * i];
        const float right = tempData[DEFAULT_NUM_TWO * i + 1];
        const float expectedValue = g_testValue * (curGain + i * stepGain);
        EXPECT_EQ(left, expectedValue);
        EXPECT_EQ(right, expectedValue);
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
    params.isVKB = false;
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
    params.isVKB = false;
    audioVolume->AddStreamVolume(params);
    audioVolume->SetStreamVolume(sessionId, streamVol);
}

static void CleanupStreamAndPipeVolume(uint32_t sessionId)
{
    auto *audioVolume = AudioVolume::GetInstance();
    audioVolume->RemoveStreamVolume(sessionId);
    audioVolume->RemovePipeVolume(UT_PIPE_ID, STREAM_MUSIC);
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

HWTEST_F(HpaeGainNodeTest, ResetVolume_CacheStreamOnly_InnerCapturer, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 1;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    SetupStreamVolumeOnly(sessionId, 0.6f);
    hpaeGainNode->ResetVolume();

    EXPECT_NE(hpaeGainNode->streamVolume_, nullptr);

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
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

HWTEST_F(HpaeGainNodeTest, DoGain_InnerCapturerUsesStreamVolumeOnly, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 4;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    SetupStreamVolumeOnly(sessionId, UT_UNIT_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // Inner capturer reads GetTotalVolume() directly (stream only, no pipe factor)
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_UNIT_GAIN, UT_PRECISION);
    }

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
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

HWTEST_F(HpaeGainNodeTest, ResetVolume_InnerCapturer_StreamVolumeNull, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 6;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    // Do NOT set up any stream volume → GetStreamAndPipeVolume fails, streamVolume_ stays nullptr
    hpaeGainNode->ResetVolume();

    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);
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

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_InnerCapturer_AppliesGain, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 8;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    SetupStreamVolumeOnly(sessionId, UT_HALF_GAIN);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // ResetVolume set history=0.5, DoGain reads cur=0.5 pre=0.5, no ramp, scale by 0.5
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_HALF_GAIN, UT_PRECISION);
    }

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_InnerCapturer_ZeroVolumeSilence, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 9;
    constexpr float zeroVolume = 0.0f;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    SetupStreamVolumeOnly(sessionId, zeroVolume);
    hpaeGainNode->ResetVolume();
    ASSERT_NE(hpaeGainNode->streamVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // curGain=0 and preGain=0 → SilenceData path, buffer should be zeroed
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], zeroVolume, UT_PRECISION);
    }
    EXPECT_EQ(inputBuf.IsSilence(), true);

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}

HWTEST_F(HpaeGainNodeTest, DoGain_InnerCapturer_StreamVolumeNull, TestSize.Level0)
{
    constexpr uint32_t sessionId = UT_SESSION_ID + 10;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.deviceClass = UT_DEVICE_CLASS_INNER_CAPTURER;
    nodeInfo.sessionId = sessionId;
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    EXPECT_EQ(hpaeGainNode->isInnerCapturerOrInjector_, true);

    // No stream volume set → ResetVolume leaves streamVolume_=nullptr
    hpaeGainNode->ResetVolume();
    EXPECT_EQ(hpaeGainNode->streamVolume_, nullptr);

    PcmBufferInfo bufInfo(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000);
    HpaePcmBuffer inputBuf(bufInfo);
    FillBuffer(inputBuf, UT_UNIT_GAIN);

    hpaeGainNode->DoGain(&inputBuf, DEFAULT_FRAME_LEN, STEREO);

    // streamVolume_ is nullptr → defaults to 1.0f, data unchanged
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_UNIT_GAIN, UT_PRECISION);
    }
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

    // pipeVolume_ is nullptr → !(streamVolume_ && pipeVolume_) → defaults to 1.0f
    size_t totalSamples = DEFAULT_FRAME_LEN * STEREO;
    for (size_t i = 0; i < totalSamples; i++) {
        EXPECT_NEAR(inputBuf.GetPcmDataBuffer()[i], UT_UNIT_GAIN, UT_PRECISION);
    }

    AudioVolume::GetInstance()->RemoveStreamVolume(sessionId);
}
}  // namespace
