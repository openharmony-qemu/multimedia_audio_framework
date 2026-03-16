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

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
constexpr uint32_t DEFAULT_NODE_ID = 1234;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_CHANNELS = STEREO;
constexpr uint32_t TEST_BUFFER_SIZE = 1920;
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

HWTEST_F(HpaeLimiterNodeTest, constructHpaeLimiterNode, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, setupAudioLimiter_Success, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, setupAudioLimiter_MultipleSetup, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, getLatency_WithoutLimiter, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, getLatency_WithLimiter, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithoutLimiterInit, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    inputs.push_back(&hpaeSinkInputNode->GetOutputBuffer());

    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);
    EXPECT_NE(output, nullptr);
    EXPECT_FALSE(hpaeLimiterNode->IsLimiterInited());
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithLimiterInit, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, signalProcess_MultipleInputs, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, signalProcess_BufferStatePropagation, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;

    std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode = std::make_shared<HpaeLimiterNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
        std::make_shared<HpaeSinkInputNode>(nodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    inputs.push_back(&hpaeSinkInputNode->GetOutputBuffer());

    hpaeSinkInputNode->GetOutputBuffer().SetBufferState(PCM_BUFFER_STATE_INVALID);

    HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);
    EXPECT_NE(output, nullptr);
    EXPECT_EQ(output->GetBufferState(), PCM_BUFFER_STATE_INVALID);
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_MonoChannel, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = MONO;
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

HWTEST_F(HpaeLimiterNodeTest, signalProcess_DifferentSampleRates, TestSize.Level0)
{
    std::vector<uint32_t> sampleRates = {
        SAMPLE_RATE_8000, SAMPLE_RATE_16000, SAMPLE_RATE_44100,
        SAMPLE_RATE_48000, SAMPLE_RATE_96000
    };

    for (auto sampleRate : sampleRates) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = DEFAULT_FRAME_LEN;
        nodeInfo.samplingRate = sampleRate;
        nodeInfo.channels = DEFAULT_CHANNELS;
        nodeInfo.format = SAMPLE_F32LE;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);
        std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode =
            std::make_shared<HpaeSinkInputNode>(nodeInfo);

        std::vector<HpaePcmBuffer *> inputs;
        inputs.push_back(&hpaeSinkInputNode->GetOutputBuffer());

        HpaePcmBuffer *output = hpaeLimiterNode->SignalProcess(inputs);
        EXPECT_NE(output, nullptr);
    }
}

HWTEST_F(HpaeLimiterNodeTest, signalProcess_EmptyInput, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, signalProcess_WithSilence, TestSize.Level0)
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

HWTEST_F(HpaeLimiterNodeTest, initAudioLimiter_VariousConfigs, TestSize.Level0)
{
    struct TestConfig {
        uint32_t frameLen;
        uint32_t channels;
        uint32_t sampleRate;
    };

    std::vector<TestConfig> configs = {
        {480, MONO, SAMPLE_RATE_48000},
        {960, STEREO, SAMPLE_RATE_48000},
        {1920, STEREO, SAMPLE_RATE_44100},
        {2048, MONO, SAMPLE_RATE_16000}
    };

    for (auto &config : configs) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = DEFAULT_NODE_ID;
        nodeInfo.frameLen = config.frameLen;
        nodeInfo.samplingRate = config.sampleRate;
        nodeInfo.channels = config.channels;
        nodeInfo.format = SAMPLE_F32LE;

        std::shared_ptr<HpaeLimiterNode> hpaeLimiterNode =
            std::make_shared<HpaeLimiterNode>(nodeInfo);

        int32_t ret = hpaeLimiterNode->InitAudioLimiter();
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_TRUE(hpaeLimiterNode->IsLimiterInited());
    }
}