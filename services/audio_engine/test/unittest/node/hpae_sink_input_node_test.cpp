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
#include "test_case_common.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeSinkInputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSinkInputNodeTest::SetUp()
{}

void HpaeSinkInputNodeTest::TearDown()
{}

namespace {
constexpr int32_t NORMAL_FRAME_LEN = 960;
constexpr int32_t NORMAL_ID = 1243;
constexpr float LOUDNESS_GAIN = 1.0f;
HWTEST_F(HpaeSinkInputNodeTest, constructHpaeSinkInputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSinkInputNode> hpaeSinkInputNode =  std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkInputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSinkInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

HWTEST_F(HpaeSinkInputNodeTest, testSinkInputOutputCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    {
        std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSinkInputNode;
        EXPECT_EQ(hpaeSinkInputNode.use_count(), 1 + 1); // add 1 count because outputNode
        std::shared_ptr<HpaeNode> hpaeNode = outputNode->GetSharedInstance();
        EXPECT_EQ(hpaeSinkInputNode.use_count(), 1 + 1 + 1); // add 1 count because hpaeNode
        EXPECT_EQ(hpaeNode->GetSampleRate(), nodeInfo.samplingRate);
        EXPECT_EQ(hpaeNode->GetFrameLen(), nodeInfo.frameLen);
        EXPECT_EQ(hpaeNode->GetChannelCount(), nodeInfo.channels);
        EXPECT_EQ(hpaeNode->GetBitWidth(), nodeInfo.format);
    }
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkOutputNode.use_count(), 1);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkOutputNode.use_count(), 1);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1 + 1);
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSinkInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 1);
    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
    outputPort = hpaeSinkInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 0);
}

HWTEST_F(HpaeSinkInputNodeTest, testWriteDataToSinkInputDataCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    int32_t testNum = 10;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<WriteFixedDataCb> writeFixedDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedDataCb);
    for (int32_t i = 0; i < testNum; i++) {
        OutputPort<HpaePcmBuffer *> *outputPort = hpaeSinkInputNode->GetOutputPort();
        HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
        float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
        for (int32_t j = 0; j < nodeInfo.frameLen; j++) {
            for (int32_t k = 0; k < nodeInfo.channels; k++) {
                float diff = outputPcmData[j * nodeInfo.channels + k] - i;
                EXPECT_EQ(fabs(diff) < TEST_VALUE_PRESION, true);
            }
        }
    }
}

HWTEST_F(HpaeSinkInputNodeTest, testWriteDataToSinkInputAndSinkOutputDataCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    int32_t testNum = 10;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<WriteFixedDataCb> writeFixedDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedDataCb);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1 + 1);
    for (int32_t i = 0; i < testNum; i++) {
        OutputPort<HpaePcmBuffer *> *outputPort = hpaeSinkInputNode->GetOutputPort();
        HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
        float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
        for (int32_t j = 0; j < nodeInfo.frameLen; j++) {
            for (int32_t k = 0; k < nodeInfo.channels; k++) {
                float diff = outputPcmData[j * nodeInfo.channels + k] - i;
                EXPECT_EQ(fabs(diff) < TEST_VALUE_PRESION, true);
            }
        }
    }

    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
}

HWTEST_F(HpaeSinkInputNodeTest, testLoudnessGain, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetLoudnessGain(LOUDNESS_GAIN);

    EXPECT_FLOAT_EQ(sinkInputNode->GetLoudnessGain(), LOUDNESS_GAIN);
}

HWTEST_F(HpaeSinkInputNodeTest, testReadToAudioBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = NORMAL_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    nodeInfo.deviceClass = "offload";
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->offloadEnable_ = true;
    int32_t ret = 0;
    bool funcRet = sinkInputNode->ReadToAudioBuffer(ret);
    EXPECT_EQ(funcRet, true);

    sinkInputNode->offloadEnable_ = false;
    funcRet = sinkInputNode->ReadToAudioBuffer(ret);
    EXPECT_EQ(funcRet, true);

    nodeInfo.deviceClass = "remote_offload";
    sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->offloadEnable_ = true;
    funcRet = sinkInputNode->ReadToAudioBuffer(ret);
    EXPECT_EQ(funcRet, true);

    sinkInputNode->offloadEnable_ = false;
    funcRet = sinkInputNode->ReadToAudioBuffer(ret);
    EXPECT_EQ(funcRet, true);
}
}