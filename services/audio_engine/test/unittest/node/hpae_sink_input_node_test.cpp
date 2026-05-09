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
    int32_t offset = writeFixedDataCb->writeNum_;
    for (int32_t i = 0; i < testNum; i++) {
        OutputPort<HpaePcmBuffer *> *outputPort = hpaeSinkInputNode->GetOutputPort();
        HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
        float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
        for (int32_t j = 0; j < nodeInfo.frameLen; j++) {
            for (int32_t k = 0; k < nodeInfo.channels; k++) {
                float diff = outputPcmData[j * nodeInfo.channels + k] - offset - i;
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
    int32_t offset = writeFixedDataCb->writeNum_;
    for (int32_t i = 0; i < testNum; i++) {
        OutputPort<HpaePcmBuffer *> *outputPort = hpaeSinkInputNode->GetOutputPort();
        HpaePcmBuffer* outPcmBuffer = outputPort->PullOutputData();
        float* outputPcmData = outPcmBuffer->GetPcmDataBuffer();
        for (int32_t j = 0; j < nodeInfo.frameLen; j++) {
            for (int32_t k = 0; k < nodeInfo.channels; k++) {
                float diff = outputPcmData[j * nodeInfo.channels + k] - offset - i;
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

HWTEST_F(HpaeSinkInputNodeTest, SetOffloadType_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NORMAL_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(sinkInputNode->GetOffloadType(), OFFLOAD_DEFAULT);

    sinkInputNode->SetOffloadType(OFFLOAD_ACTIVE_FOREGROUND);
    EXPECT_EQ(sinkInputNode->GetOffloadType(), OFFLOAD_ACTIVE_FOREGROUND);

    sinkInputNode->SetOffloadType(OFFLOAD_INACTIVE_BACKGROUND);
    EXPECT_EQ(sinkInputNode->GetOffloadType(), OFFLOAD_INACTIVE_BACKGROUND);

    sinkInputNode->SetOffloadType(OFFLOAD_DEFAULT);
    EXPECT_EQ(sinkInputNode->GetOffloadType(), OFFLOAD_DEFAULT);
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

/**
 * @tc.name  : FaultCode_OnStreamInfoChange_WriteCallbackNull
 * @tc.type  : FUNC
 * @tc.desc  : Test OnStreamInfoChange when writeCallback is null,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code and return ERROR.
 */
HWTEST_F(HpaeSinkInputNodeTest, FaultCode_OnStreamInfoChange_WriteCallbackNull, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    // writeCallback_ is default (empty weak_ptr), not set
    int32_t result = sinkInputNode->OnStreamInfoChange(true);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_QueryUnderrun_WriteCallbackNull
 * @tc.type  : FUNC
 * @tc.desc  : Test QueryUnderrun when writeCallback is null,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code and return false.
 */
HWTEST_F(HpaeSinkInputNodeTest, FaultCode_QueryUnderrun_WriteCallbackNull, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    // writeCallback_ is default (empty weak_ptr)
    bool result = sinkInputNode->QueryUnderrun();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : FaultCode_OnStreamInfoChange_WithValidCallback
 * @tc.type  : FUNC
 * @tc.desc  : Test OnStreamInfoChange with valid writeCallback succeeds
 *             without PLAY_QUERY_INSTANCE_NULL fault code.
 */
HWTEST_F(HpaeSinkInputNodeTest, FaultCode_OnStreamInfoChange_WithValidCallback, TestSize.Level1)
{
    EXPECT_CALL(*mockNodeCallback_, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(0));
    EXPECT_CALL(*mockStreamCallback_, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            return SUCCESS;
        });
    int32_t result = node_->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : testSetAndGetState
 * @tc.type  : FUNC
 * @tc.desc  : Test SetState and GetState functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetAndGetState, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->GetState(), HPAE_SESSION_NEW);
    
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(sinkInputNode->GetState(), HPAE_SESSION_RUNNING);
    
    sinkInputNode->SetState(HPAE_SESSION_PAUSED);
    EXPECT_EQ(sinkInputNode->GetState(), HPAE_SESSION_PAUSED);
}

/**
 * @tc.name  : testSetAndGetAppUid
 * @tc.type  : FUNC
 * @tc.desc  : Test SetAppUid and GetAppUid functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetAndGetAppUid, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->GetAppUid(), -1);
    
    sinkInputNode->SetAppUid(1000);
    EXPECT_EQ(sinkInputNode->GetAppUid(), 1000);
}

/**
 * @tc.name  : testSetAndGetIsLowLatency
 * @tc.type  : FUNC
 * @tc.desc  : Test SetIsLowLatency and GetIsLowLatency functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetAndGetIsLowLatency, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_FALSE(sinkInputNode->GetIsLowLatency());
    
    sinkInputNode->SetIsLowLatency(true);
    EXPECT_TRUE(sinkInputNode->GetIsLowLatency());
    
    sinkInputNode->SetIsLowLatency(false);
    EXPECT_FALSE(sinkInputNode->GetIsLowLatency());
}

/**
 * @tc.name  : testSetAndGetSpeed
 * @tc.type  : FUNC
 * @tc.desc  : Test SetSpeed and GetSpeed functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetAndGetSpeed, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_FLOAT_EQ(sinkInputNode->GetSpeed(), 1.0f);
    
    sinkInputNode->SetSpeed(2.0f);
    EXPECT_FLOAT_EQ(sinkInputNode->GetSpeed(), 2.0f);
    
    sinkInputNode->SetSpeed(0.5f);
    EXPECT_FLOAT_EQ(sinkInputNode->GetSpeed(), 0.5f);
}

/**
 * @tc.name  : testSetAndGetDirect
 * @tc.type  : FUNC
 * @tc.desc  : Test SetDirect and GetDirect functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetAndGetDirect, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_FALSE(sinkInputNode->GetDirect());
    
    sinkInputNode->SetDirect(true);
    EXPECT_TRUE(sinkInputNode->GetDirect());
    
    sinkInputNode->SetDirect(false);
    EXPECT_FALSE(sinkInputNode->GetDirect());
}

/**
 * @tc.name  : testDrainAndIsDrain
 * @tc.type  : FUNC
 * @tc.desc  : Test Drain and IsDrain functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testDrainAndIsDrain, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_FALSE(sinkInputNode->IsDrain());
    
    sinkInputNode->Drain();
    EXPECT_TRUE(sinkInputNode->IsDrain());
}

/**
 * @tc.name  : testResetAndResetAll
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset and ResetAll functions
 */
HWTEST_F(HpaeSinkInputNodeTest, testResetAndResetAll, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_TRUE(sinkInputNode->Reset());
    EXPECT_TRUE(sinkInputNode->ResetAll());
}

/**
 * @tc.name  : testGetKeepRunning
 * @tc.type  : FUNC
 * @tc.desc  : Test GetKeepRunning function
 */
HWTEST_F(HpaeSinkInputNodeTest, testGetKeepRunning, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.keepRunning = true;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_TRUE(sinkInputNode->GetKeepRunning());
    
    nodeInfo.keepRunning = false;
    sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    EXPECT_FALSE(sinkInputNode->GetKeepRunning());
}

/**
 * @tc.name  : testSetNodeInfoFrameLenChanged
 * @tc.type  : FUNC
 * @tc.desc  : Test SetNodeInfo when frameLen changed
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetNodeInfoFrameLenChanged, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    uint32_t oldFrameLen = sinkInputNode->GetFrameLen();
    EXPECT_EQ(oldFrameLen, FRAME_LENGTH_960);
    
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.frameLen = 1920;
    newNodeInfo.historyFrameCount = 5;
    sinkInputNode->SetNodeInfo(newNodeInfo);
    
    EXPECT_EQ(sinkInputNode->GetFrameLen(), 1920);
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testSetNodeInfoFrameLenChangedNoHistory
 * @tc.type  : FUNC
 * @tc.desc  : Test SetNodeInfo when frameLen changed with historyFrameCount = 0
 */
HWTEST_F(HpaeSinkInputNodeTest, testSetNodeInfoFrameLenChangedNoHistory, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 0;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
    
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.frameLen = 1920;
    newNodeInfo.historyFrameCount = 0;
    sinkInputNode->SetNodeInfo(newNodeInfo);
    
    EXPECT_EQ(sinkInputNode->GetFrameLen(), 1920);
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testFlushWithHistoryBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test Flush when historyBuffer exists with historyFrameCount > 0
 */
HWTEST_F(HpaeSinkInputNodeTest, testFlushWithHistoryBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 5;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
    
    sinkInputNode->Flush();
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testFlushNoHistoryBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test Flush when historyFrameCount = 0
 */
HWTEST_F(HpaeSinkInputNodeTest, testFlushNoHistoryBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 0;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
    
    sinkInputNode->Flush();
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testGetLatencyNoHistoryBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when historyBuffer is null
 */
HWTEST_F(HpaeSinkInputNodeTest, testGetLatencyNoHistoryBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 0;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
    uint64_t latency = sinkInputNode->GetLatency();
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : testGetLatencyWithHistoryBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test GetLatency when historyBuffer exists but has no frames
 */
HWTEST_F(HpaeSinkInputNodeTest, testGetLatencyWithHistoryBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 5;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
    uint64_t latency = sinkInputNode->GetLatency();
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : testCheckAndDestroyHistoryBuffer_CreateBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test CheckAndDestroyHistoryBuffer when historyFrameCount > 0 and historyBuffer is null
 */
HWTEST_F(HpaeSinkInputNodeTest, testCheckAndDestroyHistoryBuffer_CreateBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 0;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
    
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.historyFrameCount = 5;
    sinkInputNode->SetNodeInfo(newNodeInfo);
    sinkInputNode->CheckAndDestroyHistoryBuffer();
    
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testCheckAndDestroyHistoryBuffer_DestroyBuffer
 * @tc.type  : FUNC
 * @tc.desc  : Test CheckAndDestroyHistoryBuffer when historyFrameCount = 0 and historyBuffer exists
 */
HWTEST_F(HpaeSinkInputNodeTest, testCheckAndDestroyHistoryBuffer_DestroyBuffer, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 5;
    auto sinkInputNode = std::make_unique<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_NE(sinkInputNode->historyBuffer_, nullptr);
    
    HpaeNodeInfo newNodeInfo = nodeInfo;
    newNodeInfo.historyFrameCount = 0;
    sinkInputNode->SetNodeInfo(newNodeInfo);
    sinkInputNode->CheckAndDestroyHistoryBuffer();
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
}

/**
 * @tc.name  : testOnStreamInfoChange_ForceDataFalse_DirectEnabled
 * @tc.type  : FUNC
 * @tc.desc  : Test OnStreamInfoChange when isDirect is true, forceData should be false
 */
HWTEST_F(HpaeSinkInputNodeTest, testOnStreamInfoChange_ForceDataFalse_DirectEnabled, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto mockNodeCallback = std::make_shared<MockNodeCallback>();
    auto mockStreamCallback = std::make_shared<MockStreamCallback>();
    sinkInputNode->nodeInfo_.statusCallback = mockNodeCallback;
    sinkInputNode->writeCallback_ = mockStreamCallback;
    sinkInputNode->isDirect_ = true;
    sinkInputNode->historyBuffer_ = nullptr;
    
    EXPECT_CALL(*mockNodeCallback, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(0));
    EXPECT_CALL(*mockStreamCallback, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_TRUE(info.needData);
            EXPECT_FALSE(info.forceData);
            return SUCCESS;
        });
    
    int32_t result = sinkInputNode->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : testOnStreamInfoChange_IsLowLatency
 * @tc.type  : FUNC
 * @tc.desc  : Test OnStreamInfoChange when isLowLatency is true
 */
HWTEST_F(HpaeSinkInputNodeTest, testOnStreamInfoChange_IsLowLatency, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto mockNodeCallback = std::make_shared<MockNodeCallback>();
    auto mockStreamCallback = std::make_shared<MockStreamCallback>();
    sinkInputNode->nodeInfo_.statusCallback = mockNodeCallback;
    sinkInputNode->writeCallback_ = mockStreamCallback;
    sinkInputNode->isLowLatency_ = true;
    sinkInputNode->historyBuffer_ = nullptr;
    
    EXPECT_CALL(*mockNodeCallback, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(0));
    EXPECT_CALL(*mockStreamCallback, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_TRUE(info.isLowLatency);
            return SUCCESS;
        });
    
    int32_t result = sinkInputNode->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : testOnStreamInfoChange_GetOffloadCallbackData
 * @tc.type  : FUNC
 * @tc.desc  : Test OnStreamInfoChange when nodeCallback provides OffloadCallbackData
 */
HWTEST_F(HpaeSinkInputNodeTest, testOnStreamInfoChange_GetOffloadCallbackData, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto mockNodeCallback = std::make_shared<MockNodeCallback>();
    auto mockStreamCallback = std::make_shared<MockStreamCallback>();
    sinkInputNode->nodeInfo_.statusCallback = mockNodeCallback;
    sinkInputNode->writeCallback_ = mockStreamCallback;
    sinkInputNode->historyBuffer_ = nullptr;
    
    OffloadCallbackData callbackData;
    callbackData.writePos_ = 1000;
    callbackData.isFlush_ = true;
    
    EXPECT_CALL(*mockNodeCallback, OnRequestLatency(_, _)).WillOnce(SetArgReferee<1>(0));
    EXPECT_CALL(*mockNodeCallback, GetOffloadCallbackData()).WillOnce(Return(callbackData));
    EXPECT_CALL(*mockStreamCallback, OnStreamData(_))
        .WillOnce([&](AudioCallBackStreamInfo& info) {
            EXPECT_EQ(info.writePos_, 1000);
            EXPECT_TRUE(info.isWriteFirst_);
            return SUCCESS;
        });
    
    int32_t result = sinkInputNode->OnStreamInfoChange(true);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : testRewindHistoryBufferNull
 * @tc.type  : FUNC
 * @tc.desc  : Test RewindHistoryBuffer when historyBuffer is null
 */
HWTEST_F(HpaeSinkInputNodeTest, testRewindHistoryBufferNull, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.historyFrameCount = 0;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    
    EXPECT_EQ(sinkInputNode->historyBuffer_, nullptr);
    
    sinkInputNode->RewindHistoryBuffer(10000, 100);
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS