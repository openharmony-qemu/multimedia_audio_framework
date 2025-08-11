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
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";

class HpaeSourceOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSourceOutputNodeTest::SetUp()
{}

void HpaeSourceOutputNodeTest::TearDown()
{}

class TestReadDataCb : public ICapturerStreamCallback, public std::enable_shared_from_this<TestReadDataCb> {
public:
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override
    {
        return SUCCESS;
    }
    TestReadDataCb()
    {}
    virtual ~TestReadDataCb()
    {}
};

HWTEST_F(HpaeSourceOutputNodeTest, constructHpaeSourceOutputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceOutputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSoruceOutputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSoruceOutputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSoruceOutputNode->GetBitWidth(), nodeInfo.format);
    HpaeNodeInfo &retNi = hpaeSoruceOutputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
}

static void GetTestAudioSourceAttr(IAudioSourceAttr &attr)
{
    attr.adapterName = "";
    attr.openMicSpeaker = 0;
    attr.format = AudioSampleFormat::INVALID_WIDTH;
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
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

HWTEST_F(HpaeSourceOutputNodeTest, connectHpaeSourceInputAndOutputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
    std::shared_ptr<HpaeSourceInputNode> hpaeSoruceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    EXPECT_EQ(hpaeSoruceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName), 0);
    IAudioSourceAttr attr;
    GetTestAudioSourceAttr(attr);
    std::shared_ptr<HpaeSourceOutputNode> hpaeSoruceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceInit(attr), SUCCESS);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStart(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_RUNNING, true);

    hpaeSoruceOutputNode->Connect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 2);  // 2 for test
    hpaeSoruceOutputNode->DoProcess();

    std::shared_ptr<TestReadDataCb> testReadDataCb = std::make_shared<TestReadDataCb>();
    hpaeSoruceOutputNode->RegisterReadCallback(testReadDataCb);
    hpaeSoruceOutputNode->DoProcess();

    hpaeSoruceOutputNode->DisConnect(hpaeSoruceInputNode);
    EXPECT_EQ(hpaeSoruceInputNode.use_count(), 1);
    EXPECT_EQ(hpaeSoruceInputNode->CapturerSourceStop(), 0);
    EXPECT_EQ(hpaeSoruceInputNode->GetSourceState() == STREAM_MANAGER_SUSPENDED, true);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS