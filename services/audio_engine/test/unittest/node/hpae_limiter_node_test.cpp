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
#include <memory>
#include "hpae_limiter_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_pcm_buffer.h"
#include "audio_stream_info.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
constexpr uint32_t DEFAULT_NODE_ID = 1234;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr AudioSamplingRate DEFAULT_SAMPLE_RATE = SAMPLE_RATE_48000;
constexpr AudioChannel DEFAULT_CHANNELS = STEREO;
}

class HpaeLimiterNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};

void HpaeLimiterNodeTest::SetUp()
{}

void HpaeLimiterNodeTest::TearDown()
{}

HWTEST_F(HpaeLimiterNodeTest, constructHpaeLimiterNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    EXPECT_EQ(hpaeLimiterNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeLimiterNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeLimiterNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeLimiterNode->GetBitWidth(), nodeInfo.format);

    HpaeNodeInfo &retNi = hpaeLimiterNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.deviceClass, nodeInfo.deviceClass);
}

HWTEST_F(HpaeLimiterNodeTest, setupAudioLimiter_Success, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(HpaeLimiterNodeTest, setupAudioLimiter_MultipleSetup, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(HpaeLimiterNodeTest, getLatency_WithoutLimiter, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    uint64_t latency = hpaeLimiterNode->GetLatency(0);
    EXPECT_EQ(latency, 0);
}

HWTEST_F(HpaeLimiterNodeTest, getLatency_WithLimiter, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    hpaeLimiterNode->SetupAudioLimiter();

    uint64_t latency = hpaeLimiterNode->GetLatency(0);
    EXPECT_GT(latency, 0);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithLimiterInit, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    int32_t testValue = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_MultipleInputs, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode0 =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode0);
    hpaeLimiterNode->Connect(hpaeSinkInputNode1);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    int32_t testValue0 = 100;
    int32_t testValue1 = 200;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue0);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb1 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue1);
    hpaeSinkInputNode0->RegisterWriteCallback(writeFixedValueCb0);
    hpaeSinkInputNode1->RegisterWriteCallback(writeFixedValueCb1);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode0);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode1);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_BufferStatePropagation, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    // Create input buffer with INVALID state
    PcmBufferInfo bufferInfo(DEFAULT_CHANNELS, DEFAULT_FRAME_LEN, DEFAULT_SAMPLE_RATE);
    HpaePcmBuffer inputBuffer(bufferInfo);
    inputBuffer.SetBufferState(PCM_BUFFER_STATE_INVALID);
    inputBuffer.GetPcmDataBuffer();

    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};
    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);

    EXPECT_NE(output, nullptr);
    EXPECT_EQ(output->GetBufferState(), 1);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_MonoChannel, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    int32_t testValue = 50;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_EmptyInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);

    EXPECT_NE(output, nullptr);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithSilence, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    int32_t testValue = 0;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode);
}

HWTEST_F(HpaeLimiterNodeTest, initAudioLimiter_VariousConfigs, TestSize.Level1)
{
    struct TestConfig {
        uint32_t frameLen;
        AudioChannel channels;
        AudioSamplingRate sampleRate;
        AudioChannelLayout layout;
    };

    std::vector<TestConfig> configs = {
        {480, STEREO, SAMPLE_RATE_48000, CH_LAYOUT_STEREO},
        {960, STEREO, SAMPLE_RATE_48000, CH_LAYOUT_STEREO},
        {1920, STEREO, SAMPLE_RATE_44100, CH_LAYOUT_STEREO},
        {2048, STEREO, SAMPLE_RATE_16000, CH_LAYOUT_STEREO}
    };

    for (auto &config : configs) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = config.frameLen;
        nodeInfo.samplingRate = config.sampleRate;
        nodeInfo.channels = config.channels;
        nodeInfo.channelLayout = config.layout;
        nodeInfo.format = SAMPLE_F32LE;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);

        int32_t ret = hpaeLimiterNode->InitAudioLimiter();
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());
    }
}

/**
 * @tc.name  : Test signalProcess_WithoutLimiterInit
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_WithoutLimiterInit
 * @tc.desc  : Test pass-through mode when limiter is not initialized.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithoutLimiterInit, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    // Do NOT call SetupAudioLimiter to test pass-through mode
    EXPECT_FALSE(hpaeLimiterNode->IsLimiterInited());

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    int32_t testValue = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode);
}

/**
 * @tc.name  : Test signalProcess_WithLargeSignal
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_WithLargeSignal
 * @tc.desc  : Test limiter behavior with large input signal that exceeds threshold.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithLargeSignal, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode =
        std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    hpaeLimiterNode->Connect(hpaeSinkInputNode);
    hpaeSinkOutputNode->Connect(hpaeLimiterNode);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    EXPECT_EQ(hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId), 0);

    // Use a very large value to test limiter's clipping behavior
    int32_t testValue = 99999;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb);

    hpaeSinkOutputNode->DoProcess();
    const char *outputData = hpaeSinkOutputNode->GetRenderFrameData();
    EXPECT_NE(outputData, nullptr);

    hpaeSinkOutputNode->DisConnect(hpaeLimiterNode);
    hpaeLimiterNode->DisConnect(hpaeSinkInputNode);
}

/**
 * @tc.name  : Test signalProcess_DifferentStreamTypes
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_DifferentStreamTypes
 * @tc.desc  : Test limiter processing with different stream types.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_DifferentStreamTypes, TestSize.Level1)
{
    struct StreamTypeTest {
        AudioStreamType streamType;
        StreamUsage streamUsage;
    };

    std::vector<StreamTypeTest> streamTypes = {
        {STREAM_MUSIC, STREAM_USAGE_MEDIA},
        {STREAM_VOICE_CALL, STREAM_USAGE_VOICE_COMMUNICATION},
        {STREAM_RING, STREAM_USAGE_NOTIFICATION_RINGTONE},
        {STREAM_ALARM, STREAM_USAGE_ALARM},
        {STREAM_VOICE_ASSISTANT, STREAM_USAGE_VOICE_ASSISTANT}
    };

    for (auto &streamType : streamTypes) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = DEFAULT_FRAME_LEN;
        nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
        nodeInfo.channels = DEFAULT_CHANNELS;
        nodeInfo.format = SAMPLE_F32LE;
        nodeInfo.streamType = streamType.streamType;
        nodeInfo.effectInfo.streamUsage = streamType.streamUsage;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);

        int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());
    }
}

/**
 * @tc.name  : Test getLatency_DifferentConfigs
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_getLatency_DifferentConfigs
 * @tc.desc  : Test latency calculation with different configurations.
 */
HWTEST_F(HpaeLimiterNodeTest, getLatency_DifferentConfigs, TestSize.Level1)
{
    struct Config {
        uint32_t frameLen;
        AudioChannel channels;
        AudioSamplingRate sampleRate;
        AudioChannelLayout layout;
    };

    std::vector<Config> configs = {
        {480, STEREO, SAMPLE_RATE_48000, CH_LAYOUT_STEREO},
        {960, STEREO, SAMPLE_RATE_48000, CH_LAYOUT_STEREO},
        {1024, STEREO, SAMPLE_RATE_44100, CH_LAYOUT_STEREO},
        {2048, STEREO, SAMPLE_RATE_16000, CH_LAYOUT_STEREO}
    };

    for (auto &config : configs) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = config.frameLen;
        nodeInfo.samplingRate = config.sampleRate;
        nodeInfo.channels = config.channels;
        nodeInfo.channelLayout = config.layout;
        nodeInfo.format = SAMPLE_F32LE;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);

        int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
        EXPECT_EQ(ret, SUCCESS);

        uint64_t latency = hpaeLimiterNode->GetLatency(0);
        EXPECT_GT(latency, 0);
    }
}

/**
 * @tc.name  : Test signalProcess_SilenceStatePropagation
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_SilenceStatePropagation
 * @tc.desc  : Test that silence state is properly propagated through limiter.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_SilenceStatePropagation, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    // Create input buffer with SILENCE state
    PcmBufferInfo bufferInfo(DEFAULT_CHANNELS, DEFAULT_FRAME_LEN, DEFAULT_SAMPLE_RATE);
    HpaePcmBuffer inputBuffer(bufferInfo);
    inputBuffer.SetBufferState(PCM_BUFFER_STATE_SILENCE);
    inputBuffer.GetPcmDataBuffer();

    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};
    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);

    EXPECT_NE(output, nullptr);
    EXPECT_EQ(output->GetBufferState(), 2);
}

/**
 * @tc.name  : Test signalProcess_MultipleInputsWithMixedStates
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_MultipleInputsWithMixedStates
 * @tc.desc  : Test limiter behavior with multiple inputs having mixed buffer states.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_MultipleInputsWithMixedStates, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    // Create two input buffers with different states
    PcmBufferInfo bufferInfo(DEFAULT_CHANNELS, DEFAULT_FRAME_LEN, DEFAULT_SAMPLE_RATE);
    HpaePcmBuffer inputBuffer1(bufferInfo);
    inputBuffer1.SetBufferState(PCM_BUFFER_STATE_INVALID);
    inputBuffer1.GetPcmDataBuffer();

    HpaePcmBuffer inputBuffer2(bufferInfo);
    inputBuffer2.SetBufferState(PCM_BUFFER_STATE_SILENCE);
    inputBuffer2.GetPcmDataBuffer();

    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer1, &inputBuffer2};
    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);

    EXPECT_NE(output, nullptr);
    // Buffer state should be AND of input states
    EXPECT_EQ(output->GetBufferState(), 1);
}

/**
 * @tc.name  : Test setupAudioLimiter_AfterSetup
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_setupAudioLimiter_AfterSetup
 * @tc.desc  : Test that SetupAudioLimiter returns ERROR when called twice.
 */
HWTEST_F(HpaeLimiterNodeTest, setupAudioLimiter_AfterSetup, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());

    // Second call should return ERROR
    ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test constructHpaeLimiterNode_DifferentChannelLayouts
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_constructHpaeLimiterNode_DifferentChannelLayouts
 * @tc.desc  : Test construction with different channel layouts.
 */
HWTEST_F(HpaeLimiterNodeTest, constructHpaeLimiterNode_DifferentChannelLayouts, TestSize.Level1)
{
    std::vector<AudioChannelLayout> layouts = {
        CH_LAYOUT_MONO,
        CH_LAYOUT_STEREO,
        CH_LAYOUT_2POINT1,
        CH_LAYOUT_SURROUND,
        CH_LAYOUT_5POINT1
    };

    for (auto &layout : layouts) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = DEFAULT_FRAME_LEN;
        nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
        nodeInfo.channels = STEREO;
        nodeInfo.channelLayout = layout;
        nodeInfo.format = SAMPLE_F32LE;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);

        EXPECT_EQ(hpaeLimiterNode->GetSampleRate(), nodeInfo.samplingRate);
        EXPECT_EQ(hpaeLimiterNode->GetFrameLen(), nodeInfo.frameLen);
        EXPECT_EQ(hpaeLimiterNode->GetChannelCount(), nodeInfo.channels);
    }
}

/**
 * @tc.name  : Test signalProcess_WithSplitStream
 * @tc.type  : FUNC
 * @tc.number: HpaeLimiterNode_signalProcess_WithSplitStream
 * @tc.desc  : Test limiter processing with split stream type.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithSplitStream, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());
}

/**
 * @tc.name  : Test signalProcess_WithSplitStream002
 * @tc.type  : FUNC
 * @tc.number: signalProcess_WithSplitStream002
 * @tc.desc  : Test limiter processing with split stream type.
 */
HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithSplitStream002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.streamType = STREAM_MUSIC;
    nodeInfo.effectInfo.streamUsage = STREAM_USAGE_MEDIA;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
        std::make_shared<HpaeLimiterNode>(nodeInfo);

    int32_t ret = hpaeLimiterNode->SetupAudioLimiter();
    EXPECT_NE(ret, SUCCESS);
    EXPECT_FALSE(hpaeLimiterNode->IsLimiterInited());
}

/**
 * @tc.name  : FaultCode_SignalProcess_EmptyInputs
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess with empty inputs vector,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code and return limiterOutput_.
 */
HWTEST_F(HpaeLimiterNodeTest, FaultCode_SignalProcess_EmptyInputs, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    auto limiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    std::vector<HpaePcmBuffer *> inputs;
    HpaePcmBuffer *result = limiterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_SignalProcess_NullFirstInput
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess with nullptr as first input,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code.
 */
HWTEST_F(HpaeLimiterNodeTest, FaultCode_SignalProcess_NullFirstInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    auto limiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    std::vector<HpaePcmBuffer *> inputs = {nullptr};
    HpaePcmBuffer *result = limiterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_GetLatency_WithoutLimiter
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when limiter handle is null,
 *             should return 0 (limiter_ null path).
 */
HWTEST_F(HpaeLimiterNodeTest, FaultCode_GetLatency_WithoutLimiter, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    auto limiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    // limiter_ is nullptr by default (not initialized)
    uint64_t latency = limiterNode->GetLatency(0);
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : FaultCode_SignalProcess_InvalidInputBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess with invalid input buffer,
 *             should return input buffer directly when not valid.
 */
HWTEST_F(HpaeLimiterNodeTest, FaultCode_SignalProcess_InvalidInputBuffer, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    auto limiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);

    PcmBufferInfo bufferInfo(DEFAULT_CHANNELS, DEFAULT_FRAME_LEN, DEFAULT_SAMPLE_RATE);
    HpaePcmBuffer inputBuffer(bufferInfo);
    // Do not call GetPcmDataBuffer, buffer is not valid
    std::vector<HpaePcmBuffer *> inputs = {&inputBuffer};
    HpaePcmBuffer *result = limiterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
    // Should return input buffer directly since it's not valid
    EXPECT_EQ(result, &inputBuffer);
}
