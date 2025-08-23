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
using namespace testing::ext;
using namespace testing;

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

HWTEST_F(HpaeSourceInputNodeTest, constructHpaeSourceInputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceInputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSourceInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

HWTEST_F(HpaeSourceInputNodeTest, testSourceInputOutputCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    {
        std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSourceInputNode;
        EXPECT_EQ(hpaeSourceInputNode.use_count(), 2);  // 2 for test
        std::shared_ptr<HpaeNode> hpaeNode = outputNode->GetSharedInstance();
        EXPECT_EQ(hpaeSourceInputNode.use_count(), 3);  // 3 for test
        EXPECT_EQ(hpaeNode->GetSampleRate(), nodeInfo.samplingRate);
        EXPECT_EQ(hpaeNode->GetFrameLen(), nodeInfo.frameLen);
        EXPECT_EQ(hpaeNode->GetChannelCount(), nodeInfo.channels);
        EXPECT_EQ(hpaeNode->GetBitWidth(), nodeInfo.format);
    }
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    hpaeSourceOutputNode->Connect(hpaeSourceInputNode);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 2);  // 2 for test
    OutputPort<HpaePcmBuffer *> *outputPort = hpaeSourceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 1);
    hpaeSourceOutputNode->DisConnect(hpaeSourceInputNode);
    EXPECT_EQ(hpaeSourceInputNode.use_count(), 1);
    outputPort = hpaeSourceInputNode->GetOutputPort();
    EXPECT_EQ(outputPort->GetInputNum(), 0);
}

static void InitAudioSourceAttr(IAudioSourceAttr &attr, const HpaeNodeInfo &nodeInfo)
{
    attr.adapterName = "";
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;
    attr.volume = 0.0f;
    attr.bufferSize = 0;
    attr.isBigEndian = false;
    attr.filePath = g_rootCapturerPath;
    attr.deviceNetworkId = "";
    attr.deviceType = 0;
    attr.sourceType = 0;
    attr.channelLayout = 0;
    attr.audioStreamFlag = 0;
}

HWTEST_F(HpaeSourceInputNodeTest, testWriteDataToSourceInputDataCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);

    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    InitAudioSourceAttr(attr, nodeInfo);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState() == STREAM_MANAGER_RUNNING, true);
    hpaeSourceInputNode->DoProcess();
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);
}

HWTEST_F(HpaeSourceInputNodeTest, testInterfaces_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_MICREF;
    std::vector<HpaeNodeInfo> nodeVector;
    nodeVector.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeVector.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_EC;
    nodeVector.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeVector);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSourceOutputNode.use_count(), 1);
    hpaeSourceOutputNode->Connect(hpaeSourceInputNode);

    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_WAKEUP, ""), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_MIC, "mic"), 0);
    hpaeSourceInputNode->SetSourceInputNodeType(HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_EC);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_MIC);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPort(), nullptr);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPort(nodeInfo, false), nullptr);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(), 0);
    EXPECT_NE(hpaeSourceInputNode->GetCaptureId(), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(nodeInfo), 0);
    hpaeSourceInputNode->GetNodeInfoWithInfo(nodeInfo.sourceBufferType);
    hpaeSourceInputNode->DoProcess();
    std::vector<int32_t> appsUid;
    std::vector<int32_t> sessionsId;
    constexpr int32_t testUid = 55;
    constexpr int32_t testSessionId = 66;
    appsUid.push_back(testUid);
    sessionsId.push_back(testSessionId);
    hpaeSourceInputNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
    EXPECT_EQ(hpaeSourceInputNode->GetSourceInputNodeType(), HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC);
    hpaeSourceInputNode->SetSourceInputNodeType(HpaeSourceInputNodeType::HPAE_SOURCE_MIC);
    EXPECT_NE(hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo), HPAE_SOURCE_BUFFER_TYPE_EC);
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance("file_io", "LocalDevice", SOURCE_TYPE_MIC, "mic"), 0);
    EXPECT_EQ(hpaeSourceInputNode->GetOutputPortNum(), 1);
    EXPECT_EQ(hpaeSourceInputNode->Reset(), true);
    EXPECT_EQ(hpaeSourceInputNode->ResetAll(), true);
}

HWTEST_F(HpaeSourceInputNodeTest, testInterfaces_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint64_t requestBytes = nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format);
    std::vector<char> testData(requestBytes);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);

    EXPECT_NE(hpaeSourceInputNode->CapturerSourcePause(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceFlush(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceResume(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceReset(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);

    attr.filePath = "";
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStart(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceStop(), SUCCESS);
    EXPECT_NE(hpaeSourceInputNode->CapturerSourceDeInit(), SUCCESS);
}

HWTEST_F(HpaeSourceInputNodeTest, testDoprocess_001, TestSize.Level0)
{
    std::vector<HpaeNodeInfo> vec;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC_EC;
    vec.push_back(nodeInfo);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    vec.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(vec);
        std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    hpaeSourceInputNode->DoProcess();
    EXPECT_NE(hpaeSourceInputNode, nullptr);
}

HWTEST_F(HpaeSourceInputNodeTest, testDoprocess_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_EC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
        std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    attr.filePath = g_rootCapturerPath;
    EXPECT_EQ(hpaeSourceInputNode->CapturerSourceInit(attr), SUCCESS);
    hpaeSourceInputNode->DoProcess();
    EXPECT_NE(hpaeSourceInputNode, nullptr);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS