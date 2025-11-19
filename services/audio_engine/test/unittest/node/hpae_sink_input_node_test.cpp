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
#include "hpae_mocks.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t NORMAL_ID = 1243;
constexpr float LOUDNESS_GAIN = 1.0f;
constexpr uint32_t SAMPLE_RATE_16010 = 16010;
constexpr size_t DEFAULT_HISTROY_FRAME_COUNT = 5;

static void AddFrameToBuffer(std::unique_ptr<HpaePcmBuffer> &buffer)
{
    if (buffer == nullptr) {
        return;
    }
    PcmBufferInfo info = buffer->pcmBufferInfo_;
    info.isMultiFrames = false;
    info.frames = 1;
    HpaePcmBuffer d{info};
    buffer->PushFrameData(d);
}

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
    nodeInfo.deviceNetId = "local";
    nodeInfo.historyFrameCount = DEFAULT_HISTROY_FRAME_COUNT;
}

class HpaeSinkInputNodeTest : public testing::Test {
public:
    void SetUp() override
    {
        HpaeNodeInfo nodeInfo;
        PrepareNodeInfo(nodeInfo);
        node_ = std::make_unique<HpaeSinkInputNode>(nodeInfo);
        mockNodeCallback_ = std::make_shared<MockNodeCallback>();
        mockStreamCallback_ = std::make_shared<MockStreamCallback>();

        // Set up weak pointers for callbacks
        node_->nodeInfo_.statusCallback = mockNodeCallback_;
        node_->writeCallback_ = mockStreamCallback_;
    }

    void TearDown() override
    {
        node_.reset();
        mockNodeCallback_.reset();
        mockStreamCallback_.reset();
    }

    std::unique_ptr<HpaeSinkInputNode> node_;
    std::shared_ptr<MockNodeCallback> mockNodeCallback_;
    std::shared_ptr<MockStreamCallback> mockStreamCallback_;
};

HWTEST_F(HpaeSinkInputNodeTest, constructHpaeSinkInputNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSinkInputNode> hpaeSinkInputNode =  std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkInputNode->GetBitWidth(), nodeInfo.format);
    EXPECT_FALSE(hpaeSinkInputNode->pullDataFlag_);
    HpaeNodeInfo &retNi = hpaeSinkInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.customSampleRate, 0);
}

/**
 * @tc.name  : Test HpaeSinkInputNode construct
 * @tc.number: constructHpaeSinkInputNode_001
 * @tc.desc  : Test HpaeSinkInputNode the branch when samplingRate = 11025
 */
HWTEST_F(HpaeSinkInputNodeTest, constructHpaeSinkInputNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_11025;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSinkInputNode> hpaeSinkInputNode =  std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkInputNode->GetBitWidth(), nodeInfo.format);
    EXPECT_TRUE(hpaeSinkInputNode->pullDataFlag_);
    HpaeNodeInfo &retNi = hpaeSinkInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.customSampleRate, 0);
}

/**
 * @tc.name  : Test HpaeSinkInputNode construct
 * @tc.number: constructHpaeSinkInputNode_002
 * @tc.desc  : Test HpaeSinkInputNode the branch when customSampleRate = 16010
 */
HWTEST_F(HpaeSinkInputNodeTest, constructHpaeSinkInputNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.customSampleRate = SAMPLE_RATE_16010;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSinkInputNode> hpaeSinkInputNode =  std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkInputNode->GetBitWidth(), nodeInfo.format);
    EXPECT_FALSE(hpaeSinkInputNode->pullDataFlag_);
    HpaeNodeInfo &retNi = hpaeSinkInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.customSampleRate, nodeInfo.customSampleRate);
}

/**
 * @tc.name  : Test HpaeSinkInputNode construct
 * @tc.number: constructHpaeSinkInputNode_003
 * @tc.desc  : Test HpaeSinkInputNode the branch when customSampleRate = 11025
 */
HWTEST_F(HpaeSinkInputNodeTest, constructHpaeSinkInputNode_003, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.customSampleRate = SAMPLE_RATE_11025;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::unique_ptr<HpaeSinkInputNode> hpaeSinkInputNode =  std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeSinkInputNode->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSinkInputNode->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSinkInputNode->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSinkInputNode->GetBitWidth(), nodeInfo.format);
    EXPECT_TRUE(hpaeSinkInputNode->pullDataFlag_);
    HpaeNodeInfo &retNi = hpaeSinkInputNode->GetNodeInfo();
    EXPECT_EQ(retNi.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(retNi.frameLen, nodeInfo.frameLen);
    EXPECT_EQ(retNi.channels, nodeInfo.channels);
    EXPECT_EQ(retNi.format, nodeInfo.format);
    EXPECT_EQ(retNi.customSampleRate, nodeInfo.customSampleRate);
}

HWTEST_F(HpaeSinkInputNodeTest, testSinkInputOutputCase, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
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
    nodeInfo.frameLen = FRAME_LENGTH_960;
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
    nodeInfo.frameLen = FRAME_LENGTH_960;
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
    nodeInfo.frameLen = FRAME_LENGTH_960;
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
    nodeInfo.frameLen = FRAME_LENGTH_960;
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

// Test case when nodeCallback is null
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_NodeCallbackNull_LatencyZero, TestSize.Level0) {
    node_->nodeInfo_.statusCallback.reset();

    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_EQ(info.latency, 0); // no latency
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case when writeCallback is null
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_WriteCallbackNull_ReturnsError, TestSize.Level0) {
    node_->writeCallback_.reset();

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, ERROR);
}

// Test case when needData is true (historyBuffer is null and isPullData is true)
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_NeedDataTrue, TestSize.Level0) {
    node_->historyBuffer_ = nullptr;

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    // Verify that needData is true and forceData is true (offloadEnable_ is false by default)
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_TRUE(info.needData);
            EXPECT_TRUE(info.forceData);
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case when needData is false (historyBuffer has data)
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_NeedDataFalse, TestSize.Level0) {
    AddFrameToBuffer(node_->historyBuffer_);

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    // Verify that needData is false and forceData is true (offloadEnable_ is false by default)
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_FALSE(info.needData);
            EXPECT_TRUE(info.forceData);
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case when forceData is true (offloadEnable is false)
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_ForceDataTrue_OffloadDisabled, TestSize.Level0) {
    node_->historyBuffer_ = nullptr;
    node_->offloadEnable_ = false;

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    // Verify that needData is true and forceData is true
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_TRUE(info.needData);
            EXPECT_TRUE(info.forceData);
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case when forceData is false (offloadEnable is true)
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_ForceDataFalse_OffloadEnabled, TestSize.Level0) {
    node_->historyBuffer_ = nullptr;
    node_->offloadEnable_ = true;

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    // Verify that needData is true and forceData is false
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_TRUE(info.needData);
            EXPECT_FALSE(info.forceData);
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case to verify parameters passed to OnStreamData are correct
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_StreamDataParametersCorrect, TestSize.Level0) {
    node_->historyBuffer_ = nullptr;

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    AudioCallBackStreamInfo expectedInfo;
    expectedInfo.framePosition = node_->totalFrames_;
    expectedInfo.hdiFramePosition = 0; // Because of hdiFramePosition_.exchange(0)
    expectedInfo.framesWritten = node_->totalFrames_;
    expectedInfo.latency = 5; // OnRequestLatency returns 5 + GetLatency(0) returns 5
    expectedInfo.inputData = node_->interleveData_.data();
    expectedInfo.requestDataLen = node_->interleveData_.size();
    expectedInfo.deviceClass = "primary";
    expectedInfo.deviceNetId = "local";
    expectedInfo.needData = true;
    expectedInfo.forceData = true; // Because offloadEnable_ is false by default

    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_EQ(info.framePosition, expectedInfo.framePosition);
            EXPECT_EQ(info.hdiFramePosition, expectedInfo.hdiFramePosition);
            EXPECT_EQ(info.framesWritten, expectedInfo.framesWritten);
            EXPECT_EQ(info.latency, expectedInfo.latency);
            EXPECT_EQ(info.inputData, expectedInfo.inputData);
            EXPECT_EQ(info.requestDataLen, expectedInfo.requestDataLen);
            EXPECT_EQ(info.deviceClass, expectedInfo.deviceClass);
            EXPECT_EQ(info.deviceNetId, expectedInfo.deviceNetId);
            EXPECT_EQ(info.needData, expectedInfo.needData);
            EXPECT_EQ(info.forceData, expectedInfo.forceData);
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

// Test case when isPullData is false and historyBuffer is null
HWTEST_F(HpaeSinkInputNodeTest, OnStreamInfoChange_IsPullDataFalse, TestSize.Level0) {
    node_->historyBuffer_ = nullptr;

    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(5));

    // Verify that needData is false (because isPullData is false) and forceData is true
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_FALSE(info.needData);
            EXPECT_TRUE(info.forceData); // offloadEnable_ is false by default
            return SUCCESS;
        });

    int32_t result = node_->OnStreamInfoChange(false);
    EXPECT_EQ(result, SUCCESS);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS