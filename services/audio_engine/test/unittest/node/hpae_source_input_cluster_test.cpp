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
#include "hpae_source_input_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;

class HpaeSourceInputClusterTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceInputClusterTest::SetUp()
{}

void HpaeSourceInputClusterTest::TearDown()
{}

TEST_F(HpaeSourceInputClusterTest, constructHpaeSourceInputClusterNode)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.statusCallback = testStatuscallback;

    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceInputCluster->GetNodeId(), 0);
    EXPECT_EQ(hpaeSourceInputCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceInputCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceInputCluster->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->Connect(hpaeSourceInputCluster);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 0);

    nodeInfo.samplingRate = SAMPLE_RATE_16000;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode1 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode1->ConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 1);

    hpaeSourceOutputNode1->DisConnectWithInfo(hpaeSourceInputCluster, hpaeSourceOutputNode1->GetNodeInfo());
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceInputNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceInputCluster->GetConverterNodeCount(), 1); // no delete converter now
}

static int32_t TestCapturerSourceFrame(char *frame, uint64_t requestBytes, uint64_t *replyBytes)
{
    for (int32_t i = 0; i < requestBytes / SAMPLE_F32LE; i++) {
        *(float *)(frame + i * sizeof(float)) = i;
    }
    *replyBytes = requestBytes;
    return 0;
}

TEST_F(HpaeSourceInputClusterTest, testWriteDataToSourceInputDataCase)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    uint64_t requestBytes = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    std::vector<char> testData(requestBytes);
    uint64_t replyBytes = 0;
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputCluster->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.adapterName = NULL;
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.bufferSize = 0;
    attr.isBigEndian = false;
    attr.filePath = NULL;
    attr.deviceNetworkId = NULL;
    attr.deviceType = 0;
    attr.sourceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceInit(attr), ERROR);
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceState() == STREAM_MANAGER_RUNNING, true);
    EXPECT_EQ(hpaeSourceInputCluster->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputCluster->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
    TestCapturerSourceFrame(testData.data(), requestBytes, &replyBytes);
    hpaeSourceInputCluster->WriteCapturerData(testData.data(), requestBytes);
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSourceInputCluster->GetSourceInputNodeOutputPort();
    HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
    float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
    for (int32_t j = 0; j < nodeInfo.frameLen; j++) {
        for (int32_t k = 0; k < nodeInfo.channels; k++) {
            float diff = outputPcmData[(j * nodeInfo.channels + k)] - (j * nodeInfo.channels + k);
            EXPECT_EQ(fabs(diff) < TEST_VALUE_PRESION, true);
        }
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS