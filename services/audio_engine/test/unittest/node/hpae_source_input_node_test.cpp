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
#include <vector>
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_format_convert.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";

class HpaeSourceInputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceInputNodeTest::SetUp()
{}

void HpaeSourceInputNodeTest::TearDown()
{}

TEST_F(HpaeSourceInputNodeTest, constructHpaeSourceInputNode)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSoruceInputNode->GetNodeId(), nodeInfo.nodeId);
    EXPECT_EQ(hpaeSoruceInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSoruceInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSoruceInputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSoruceInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.nodeId, nodeInfo.nodeId);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

TEST_F(HpaeSourceInputNodeTest, testSourceInputOutputCase)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
    {
        std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSoruceInputNode;
        EXPECT_EQ(hpaeSoruceInputNode.use_count(), 2);  // 2 for test
        std::shared_ptr<HpaeNode> hpaeNode = outputNode->GetSharedInstance();
        EXPECT_EQ(hpaeSoruceInputNode.use_count(), 3);  // 3 for test
        EXPECT_EQ(hpaeNode->GetSampleRate(), nodeInfo.samplingRate);
        EXPECT_EQ(hpaeNode->GetNodeId(), nodeInfo.nodeId);
        EXPECT_EQ(hpaeNode->GetFrameLen(), nodeInfo.frameLen);
        EXPECT_EQ(hpaeNode->GetChannelCount(), nodeInfo.channels);
        EXPECT_EQ(hpaeNode->GetBitWidth(), nodeInfo.format);
    }
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    hpaeSourceOutputNode->Connect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 2);  // 2 for test
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSoruceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 1);
    hpaeSourceOutputNode->DisConnect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
    outputPort = hpaeSoruceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 0);
}

TEST_F(HpaeSourceInputNodeTest, testWriteDataToSourceInputDataCase)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint64_t requestBytes = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    std::vector<char> testData(requestBytes);
    uint64_t replyBytes = 0;
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSoruceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.adapterName = "";
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.bufferSize = 0;
    attr.isBigEndian = false;
    attr.filePath = g_rootCapturerPath.c_str();
    attr.deviceNetworkId = NULL;
    attr.deviceType = 0;
    attr.sourceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_RUNNING, true);
    TestCapturerSourceFrame(testData.data(), requestBytes, &replyBytes);
    std::vector<float> testDataFloat(requestBytes / SAMPLE_F32LE);
    ConvertToFloat(nodeInfo.format, nodeInfo.channels * nodeInfo.frameLen, testData.data(), testDataFloat.data());
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSoruceInputNode->GetOutputPort();
    HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
    float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
    for (int32_t i = 0; i < requestBytes / SAMPLE_F32LE; i++) {
        float diff = outputPcmData[i] - testDataFloat[i];
        EXPECT_EQ(fabs(diff) < TEST_VALUE_PRESION, true);
    }
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS