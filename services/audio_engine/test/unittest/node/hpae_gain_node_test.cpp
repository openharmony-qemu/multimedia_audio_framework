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
#include "test_case_common.h"
#include "audio_errors.h"

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
    EXPECT_EQ(hpaeSinkInputNode.use_count(), DEFAULT_NUM_TWO);
    EXPECT_EQ(hpaeGainNode.use_count(), DEFAULT_NUM_TWO);
    EXPECT_EQ(hpaeSinkOutputNode.use_count(), 1);
    g_testValue = 0;
    int32_t testValue = 100;
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, testValue);
    g_testValue = testValue;
    hpaeSinkInputNode->RegisterWriteCallback(writeFixedValueCb0);
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DoProcess();
    TestRendererRenderFrame(hpaeSinkOutputNode->GetRenderFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    hpaeSinkOutputNode->DisConnect(hpaeGainNode);
    EXPECT_EQ(hpaeGainNode.use_count(), 1);
    hpaeGainNode->DisConnect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), 1);
}
}  // namespace
