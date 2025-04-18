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
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"
using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;

class HpaeSourceOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceOutputNodeTest::SetUp()
{}

void HpaeSourceOutputNodeTest::TearDown()
{}

class TestReadDataCb : public IReadCallback, public std::enable_shared_from_this<TestReadDataCb> {
public:
    int32_t OnReadData(std::vector<char> &inputData, size_t requestDataLen) override;
    int32_t OnReadData(size_t length) override
    {
        return SUCCESS;
    }
    TestReadDataCb()
    {}
    virtual ~TestReadDataCb()
    {}
};

int32_t TestReadDataCb::OnReadData(std::vector<char> &inputData, size_t requestDataLen)
{
    for (int32_t i = 0; i < requestDataLen / SAMPLE_F32LE; i++) {
        EXPECT_EQ(*(float *)(inputData.data() + i * sizeof(float)), i);
    }
    return 0;
}

static int32_t TestCapturerSourceFrame(char *frame, uint64_t requestBytes, uint64_t *replyBytes)
{
    for (int32_t i = 0; i < requestBytes / SAMPLE_F32LE; i++) {
        *(float *)(frame + i * sizeof(float)) = i;
    }
    *replyBytes = requestBytes;
    return 0;
}

TEST_F(HpaeSourceOutputNodeTest, constructHpaeSourceOutputNode)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceOutputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSoruceOutputNode->GetNodeId(), nodeInfo.nodeId);
    EXPECT_EQ(hpaeSoruceOutputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSoruceOutputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSoruceOutputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSoruceOutputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.nodeId, nodeInfo.nodeId);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

TEST_F(HpaeSourceOutputNodeTest, connectHpaeSourceInputAndOutputNode)
{
   HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint64_t requestBytes = nodeInfo.frameLen * nodeInfo.channels * GET_SIZE_FROM_FORMAT(nodeInfo.format);
    std::vector<char> testData(requestBytes);
    uint64_t replyBytes = 0;
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSoruceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
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
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceInit(attr), ERROR);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStart(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == CAPTURER_RUNNING, true);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStop(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == CAPTURER_STOPPED, true);
    TestCapturerSourceFrame(testData.data(), requestBytes, &replyBytes);
    hpaeSoruceInputNode->WriteCapturerData(testData.data(), requestBytes);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    std::shared_ptr<TestReadDataCb> testReadDataCb = std::make_shared<TestReadDataCb>();
    hpaeSoruceOutputNode->RegisterReadCallback(testReadDataCb);
    hpaeSoruceOutputNode->Connect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 2);  // 2 for test
    hpaeSoruceOutputNode->DoProcess();
    hpaeSoruceOutputNode->DisConnect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS