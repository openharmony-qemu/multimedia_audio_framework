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
#include <cstdio>
#include "hpae_sink_input_node.h"
#include "hpae_resample_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_source_input_node.h"
#include <fstream>
#include <streambuf>
#include <string>
#include "test_case_common.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

static constexpr uint32_t TEST_ID = 1243;
static constexpr uint32_t TEST_ID2 = 1246;
static constexpr uint32_t TEST_FRAMELEN1 = 960;
static constexpr uint32_t TEST_FRAMELEN2 = 640;
namespace {
class HpaeResampleNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeResampleNodeTest::SetUp()
{}

void HpaeResampleNodeTest::TearDown()
{}

HWTEST_F(HpaeResampleNodeTest, constructHpaeResampleNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID2;
    dstNodeInfo.frameLen = TEST_FRAMELEN1;
    dstNodeInfo.samplingRate = SAMPLE_RATE_44100;
    dstNodeInfo.channels = CHANNEL_4;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode = std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo);
    EXPECT_EQ(hpaeResampleNode->GetSampleRate(), dstNodeInfo.samplingRate);
    EXPECT_EQ(hpaeResampleNode->GetFrameLen(), dstNodeInfo.frameLen);
    EXPECT_EQ(hpaeResampleNode->GetChannelCount(), dstNodeInfo.channels);
    EXPECT_EQ(hpaeResampleNode->GetBitWidth(), dstNodeInfo.format);
    HpaeNodeInfo &retNi = hpaeResampleNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, dstNodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, dstNodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, dstNodeInfo.channels);
    EXPECT_EQ(retNi.format, dstNodeInfo.format);
}

HWTEST_F(HpaeResampleNodeTest, testReset, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID2;
    dstNodeInfo.frameLen = TEST_FRAMELEN1;
    dstNodeInfo.samplingRate = SAMPLE_RATE_44100;
    dstNodeInfo.channels = CHANNEL_4;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode =
        std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo, ResamplerType::PRORESAMPLER);
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode2 =
        std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo, (ResamplerType)0xff);
    EXPECT_EQ(hpaeResampleNode->GetSampleRate(), dstNodeInfo.samplingRate);
    EXPECT_EQ(hpaeResampleNode->GetFrameLen(), dstNodeInfo.frameLen);
    EXPECT_EQ(hpaeResampleNode->GetChannelCount(), dstNodeInfo.channels);
    EXPECT_EQ(hpaeResampleNode->GetBitWidth(), dstNodeInfo.format);
    HpaeNodeInfo &retNi = hpaeResampleNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, dstNodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, dstNodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, dstNodeInfo.channels);
    EXPECT_EQ(retNi.format, dstNodeInfo.format);
    EXPECT_EQ(hpaeResampleNode->Reset(), true);
    EXPECT_EQ(hpaeResampleNode2->Reset(), false);
}

HWTEST_F(HpaeResampleNodeTest, testHpaeReampleNodeProcess, TestSize.Level0)
{
    HpaeNodeInfo srcNodeInfo;
    srcNodeInfo.nodeId = TEST_ID;
    srcNodeInfo.frameLen = TEST_FRAMELEN1;
    srcNodeInfo.samplingRate = SAMPLE_RATE_48000;
    srcNodeInfo.channels = STEREO;
    srcNodeInfo.format = SAMPLE_S32LE;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(srcNodeInfo);
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID;
    dstNodeInfo.frameLen = TEST_FRAMELEN2;
    dstNodeInfo.samplingRate = SAMPLE_RATE_32000;
    dstNodeInfo.channels = CHANNEL_4;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode = std::make_shared<HpaeResampleNode>(srcNodeInfo, dstNodeInfo);
}

HWTEST_F(HpaeResampleNodeTest, testSignalProcess_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID2;
    dstNodeInfo.frameLen = TEST_FRAMELEN1;
    dstNodeInfo.samplingRate = SAMPLE_RATE_44100;
    dstNodeInfo.channels = MONO;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode = std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo);

    std::vector<HpaePcmBuffer *> inputs;
    EXPECT_EQ(hpaeResampleNode->SignalProcess(inputs), nullptr);
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeResampleNode->SignalProcess(inputs), nullptr);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeResampleNode->SignalProcess(inputs), nullptr);
}

HWTEST_F(HpaeResampleNodeTest, testSignalProcess_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_F32LE;
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID2;
    dstNodeInfo.frameLen = TEST_FRAMELEN1;
    dstNodeInfo.samplingRate = SAMPLE_RATE_44100;
    dstNodeInfo.channels = MONO;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode =
        std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo, (ResamplerType)0xff);

    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(MONO, TEST_FRAMELEN1, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    EXPECT_NE(hpaeResampleNode->SignalProcess(inputs), nullptr);

    std::shared_ptr<HpaeResampleNode> hpaeResampleNode2 =
        std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo);
    EXPECT_NE(hpaeResampleNode2->SignalProcess(inputs), nullptr);
}

HWTEST_F(HpaeResampleNodeTest, testConnectWithInfo, TestSize.Level0)
{
    HpaeNodeInfo srcNodeInfo;
    srcNodeInfo.nodeId = TEST_ID;
    srcNodeInfo.frameLen = TEST_FRAMELEN1;
    srcNodeInfo.samplingRate = SAMPLE_RATE_48000;
    srcNodeInfo.channels = STEREO;
    srcNodeInfo.format = SAMPLE_S32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeInputNode = std::make_shared<HpaeSourceInputNode>(srcNodeInfo);
    HpaeNodeInfo dstNodeInfo;
    dstNodeInfo.nodeId = TEST_ID;
    dstNodeInfo.frameLen = TEST_FRAMELEN2;
    dstNodeInfo.samplingRate = SAMPLE_RATE_32000;
    dstNodeInfo.channels = CHANNEL_4;
    dstNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeResampleNode> hpaeResampleNode = std::make_shared<HpaeResampleNode>(srcNodeInfo, dstNodeInfo);
    EXPECT_EQ(hpaeResampleNode->GetSampleRate(), dstNodeInfo.samplingRate);
    EXPECT_EQ(hpaeResampleNode->GetFrameLen(), dstNodeInfo.frameLen);
    EXPECT_EQ(hpaeResampleNode->GetChannelCount(), dstNodeInfo.channels);
    EXPECT_EQ(hpaeResampleNode->GetBitWidth(), dstNodeInfo.format);

    hpaeResampleNode->ConnectWithInfo(hpaeInputNode, srcNodeInfo);
    hpaeResampleNode->DisConnectWithInfo(hpaeInputNode, srcNodeInfo);
}
} // namespace
