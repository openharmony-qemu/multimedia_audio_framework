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
#include <cmath>
#include <memory>
#include <gtest/gtest.h>
#include "hpae_sink_input_node.h"
#include "hpae_sink_virtual_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t DEFAULT_RING_BUFFER_NUM = 1;
static constexpr uint32_t DEFAULT_FRAME_LEN_MS = 20;
static constexpr uint32_t MS_PER_SECOND = 1000;

class HpaeSinkVirtualOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown() {}

    HpaeNodeInfo nodeInfo_;
};

void HpaeSinkVirtualOutputNodeTest::SetUp()
{
    nodeInfo_.nodeId = 1;
    nodeInfo_.samplingRate = SAMPLE_RATE_48000;
    nodeInfo_.frameLen = 960; // 960 for framelen
    nodeInfo_.channels = STEREO;
    nodeInfo_.format = SAMPLE_F32LE;
    nodeInfo_.sessionId = 1001; // 1001 for sessionid
    nodeInfo_.statusCallback.reset();
    nodeInfo_.historyFrameCount = 0;
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, ConstructHpaeSinkVirtualOutputNode, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetSampleRate(), nodeInfo_.samplingRate);
    EXPECT_EQ(node->GetFrameLen(), nodeInfo_.frameLen);
    EXPECT_EQ(node->GetChannelCount(), nodeInfo_.channels);
    EXPECT_EQ(node->GetBitWidth(), nodeInfo_.format);
    EXPECT_EQ(node->GetSessionId(), nodeInfo_.sessionId);
    EXPECT_EQ(node->ringCache_ != nullptr, true);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, PeekAudioDataInvalidBufferSize, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    uint8_t* buffer = nullptr;
    AudioStreamInfo audioStreamInfo;
    
    int32_t result = node->PeekAudioData(buffer, 1, audioStreamInfo);
    EXPECT_EQ(result, ERROR_INVALID_PARAM);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, PeekAudioDataSuccess, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    size_t validSize = nodeInfo_.frameLen * nodeInfo_.channels * GetSizeFromFormat(nodeInfo_.format);
    std::vector<uint8_t> buffer(validSize, 1);
    AudioStreamInfo audioStreamInfo;
    
    EXPECT_EQ(node->PeekAudioData(buffer.data(), validSize, audioStreamInfo), SUCCESS);
    EXPECT_EQ(buffer[0], 0);
    EXPECT_EQ(audioStreamInfo.format, nodeInfo_.format);
    EXPECT_EQ(audioStreamInfo.samplingRate, nodeInfo_.samplingRate);
    EXPECT_EQ(audioStreamInfo.channels, nodeInfo_.channels);

    std::fill(buffer.begin(), buffer.end(), 1);
    EXPECT_EQ(node->PeekAudioData(buffer.data(), validSize - 1, audioStreamInfo), SUCCESS);
    EXPECT_EQ(buffer[0], 0);
    std::fill(buffer.begin(), buffer.end(), 1);
    EXPECT_EQ(node->PeekAudioData(buffer.data(), validSize + 1, audioStreamInfo), SUCCESS);
    EXPECT_EQ(buffer[0], 0);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, ResetNode, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    std::shared_ptr<HpaeSinkInputNode> inputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo_);
    std::shared_ptr<HpaeSinkInputNode> inputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo_);
    node->Connect(inputNode);
    node->Connect(inputNode2);
    EXPECT_EQ(inputNode.use_count(), 2); // 2 for use_count
    node->DisConnect(inputNode);
    EXPECT_EQ(inputNode.use_count(), 1);
    node->DoProcess();
    EXPECT_TRUE(node->Reset());
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, TestGetReadFinishAndResetAll, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    EXPECT_EQ(node->GetIsReadFinished(), true);
    EXPECT_TRUE(node->ResetAll());
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, GetSharedInstance, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    
    std::shared_ptr<HpaeNode> sharedInstance = node->GetSharedInstance();
    EXPECT_EQ(sharedInstance, node);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, GetOutputPort, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    
    OutputPort<HpaePcmBuffer*>* port = node->GetOutputPort();
    EXPECT_NE(port, nullptr);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, RenderSinkInitNullRingCache, TestSize.Level1)
{
    HpaeNodeInfo info = nodeInfo_;
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(info);
    node->ringCache_.reset();
    EXPECT_NE(node->RenderSinkInit(), SUCCESS);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, RenderSinkInitSuccess, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    
    int32_t result = node->RenderSinkInit();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_IDLE);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, RenderSinkDeInit, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    
    int32_t result = node->RenderSinkDeInit();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_RELEASED);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, RenderSinkStart, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    int32_t result = node->RenderSinkStart();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_RUNNING);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, RenderSinkStop, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    EXPECT_EQ(node->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_SUSPENDED);

    node->ringCache_.reset();
    EXPECT_EQ(node->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_SUSPENDED);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, GetPreOutNum, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    size_t num = node->GetPreOutNum();
    EXPECT_GE(num, 0);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, SetSinkState, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    int32_t result = node->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(node->GetState(), STREAM_MANAGER_RUNNING);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, GetLatency, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    uint32_t latency = node->GetLatency();
    EXPECT_EQ(latency, 0);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, ReloadNode, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    
    HpaeNodeInfo newInfo = nodeInfo_;
    newInfo.samplingRate = SAMPLE_RATE_44100;
    newInfo.frameLen = 882; // 20ms at 44.1kHz
    newInfo.channels = MONO;
    newInfo.format = SAMPLE_S16LE;
    
    int32_t result = node->ReloadNode(newInfo);
    EXPECT_EQ(result, SUCCESS);
    
    EXPECT_EQ(node->GetSampleRate(), newInfo.samplingRate);
    EXPECT_EQ(node->GetFrameLen(), newInfo.frameLen);
    EXPECT_EQ(node->GetChannelCount(), newInfo.channels);
    EXPECT_EQ(node->GetBitWidth(), newInfo.format);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, GetRingCacheSize, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    size_t expectedSize = nodeInfo_.channels * nodeInfo_.samplingRate * DEFAULT_FRAME_LEN_MS *
        DEFAULT_RING_BUFFER_NUM * static_cast<size_t>(GetSizeFromFormat(nodeInfo_.format)) / MS_PER_SECOND;
    
    size_t actualSize = node->GetRingCacheSize();
    EXPECT_EQ(actualSize, expectedSize);
}

HWTEST_F(HpaeSinkVirtualOutputNodeTest, HpaeSinkVirtualOutputNodeTestReload, TestSize.Level1)
{
    std::shared_ptr<HpaeSinkVirtualOutputNode> node = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo_);
    node->ringCache_ = nullptr;
    EXPECT_EQ(node->ringCache_ == nullptr, true);
    node->ReloadNode(nodeInfo_);
    EXPECT_EQ(node->ringCache_ != nullptr, true);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
