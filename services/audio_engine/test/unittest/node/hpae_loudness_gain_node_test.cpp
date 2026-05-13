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
#include "hpae_loudness_gain_node.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeLoudnessGainNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeLoudnessGainNodeTest::SetUp()
{}

void HpaeLoudnessGainNodeTest::TearDown()
{}

namespace {

constexpr uint32_t TEST_ID = 1234;
constexpr uint32_t TEST_FRAMELEN = 960;
constexpr int TIMES = 5;
constexpr float LOUDNESS_GAIN_VALUE = 10.0f;

HWTEST_F(HpaeLoudnessGainNodeTest, testLoudnessGainNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeLoudnessGainNode> hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    std::vector<HpaePcmBuffer*> inputs;
    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), 0.0f);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), LOUDNESS_GAIN_VALUE);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), 0.0f);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs);
    }
    std::vector<HpaePcmBuffer*> inputs1;
    PcmBufferInfo pcmBufferInfo1(CHANNEL_6, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer1(pcmBufferInfo1);
    inputs1.emplace_back(&hpaePcmBuffer1);
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE), SUCCESS);
    for (int32_t i = 0; i < TIMES; i++) {
        hpaeLoudnessGainNode->SignalProcess(inputs1);
    }
    EXPECT_FLOAT_EQ(hpaeLoudnessGainNode->GetLoudnessGain(), LOUDNESS_GAIN_VALUE);
}

/**
 * @tc.name  : FaultCode_ReleaseHandle_NullHandle
 * @tc.type  : FUNC
 * @tc.desc  : Test ReleaseHandle returns ERROR when handle_ is nullptr,
 *             covering PLAY_CREATE_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_ReleaseHandle_NullHandle, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // handle_ is nullptr by default (no dl handle loaded in UT)
    int32_t result = hpaeLoudnessGainNode->ReleaseHandle(0.0f);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_SetLoudnessGain_SameGain
 * @tc.type  : FUNC
 * @tc.desc  : Test SetLoudnessGain returns SUCCESS when setting same gain value,
 *             covering PLAY_CREATE_INVALID_PARAM fault code path for same-value skip.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SetLoudnessGain_SameGain, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Initial gain is 0.0f, setting same value should return SUCCESS
    EXPECT_EQ(hpaeLoudnessGainNode->SetLoudnessGain(0.0f), SUCCESS);
}

/**
 * @tc.name  : FaultCode_SignalProcess_EmptyInputs
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess returns &silenceData_ when inputs is empty,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SignalProcess_EmptyInputs, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Set gain to non-zero to avoid early return at gain==0 check
    hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE);

    // Empty inputs vector should return &silenceData_
    std::vector<HpaePcmBuffer*> emptyInputs;
    HpaePcmBuffer* result = hpaeLoudnessGainNode->SignalProcess(emptyInputs);
    EXPECT_EQ(result, &hpaeLoudnessGainNode->silenceData_);
}

/**
 * @tc.name  : FaultCode_SignalProcess_NullFirstInput
 * @tc.type  : FUNC
 * @tc.desc  : Test SignalProcess returns &silenceData_ when first input is nullptr,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeLoudnessGainNodeTest, FaultCode_SignalProcess_NullFirstInput, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeLoudnessGainNode = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Set gain to non-zero
    hpaeLoudnessGainNode->SetLoudnessGain(LOUDNESS_GAIN_VALUE);

    // First input is nullptr
    std::vector<HpaePcmBuffer*> inputs = {nullptr};
    HpaePcmBuffer* result = hpaeLoudnessGainNode->SignalProcess(inputs);
    EXPECT_EQ(result, &hpaeLoudnessGainNode->silenceData_);
}

// ==================== LoudnessGainNode tests ====================

static HpaeStreamInfo MakeVoipStreamInfo(uint32_t sessionId)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = sessionId;
    streamInfo.streamType = STREAM_VOICE_COMMUNICATION;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

/**
* @tc.name  : loudnessGainNodeExistsForDirectMusic_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_145
* @tc.desc  : Test loudnessGainNode_ is created for Direct Music pathway
*/
HWTEST_F(HpaeDirectRendererManagerTest, loudnessGainNodeExistsForDirectMusic_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_NE(directManager_->gainNode_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
* @tc.name  : loudnessGainNodeNullForVoip_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_146
* @tc.desc  : Test loudnessGainNode_ is nullptr for Direct VoIP pathway
*/
HWTEST_F(HpaeDirectRendererManagerTest, loudnessGainNodeNullForVoip_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetVoipDirectSinkInfo();
    auto voipManager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->CreateStream(MakeVoipStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->loudnessGainNode_, nullptr);
    EXPECT_NE(voipManager->volumeSyncNode_, nullptr);

    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(voipManager);
}

/**
* @tc.name  : setLoudnessGainInitialized_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_147
* @tc.desc  : Test SetLoudnessGain propagates gain to loudnessGainNode_ for running Direct Music stream
*/
HWTEST_F(HpaeDirectRendererManagerTest, setLoudnessGainInitialized_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->SetLoudnessGain(TEST_STREAM_SESSION_ID, 5.0f), SUCCESS);
    WaitForMsgProcessing(directManager_);

    ASSERT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_FLOAT_EQ(directManager_->loudnessGainNode_->GetLoudnessGain(), 5.0f);
    EXPECT_FLOAT_EQ(directManager_->curNode_->GetLoudnessGain(), 5.0f);

    EXPECT_EQ(directManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
* @tc.name  : setLoudnessGainNonCurrentSession_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_148
* @tc.desc  : Test SetLoudnessGain for non-current session stores on SinkInputNode but does not propagate
*/
HWTEST_F(HpaeDirectRendererManagerTest, setLoudnessGainNonCurrentSession_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(500001)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(500002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->SetLoudnessGain(500002, 7.5f), SUCCESS);
    WaitForMsgProcessing(directManager_);

    ASSERT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_FLOAT_EQ(directManager_->loudnessGainNode_->GetLoudnessGain(), 0.0f);

    auto node2 = directManager_->sinkInputNodeMap_[500002];
    ASSERT_NE(node2, nullptr);
    EXPECT_FLOAT_EQ(node2->GetLoudnessGain(), 7.5f);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
* @tc.name  : setLoudnessGainVoip_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_149
* @tc.desc  : Test SetLoudnessGain on VoIP manager has no side effects (loudnessGainNode_ always null)
*/
HWTEST_F(HpaeDirectRendererManagerTest, setLoudnessGainVoip_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipDirectSinkInfo();
    auto voipManager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->CreateStream(MakeVoipStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(voipManager);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(voipManager->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(voipManager->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->SetLoudnessGain(TEST_STREAM_SESSION_ID, 10.0f), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->loudnessGainNode_, nullptr);

    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(voipManager);
}

/**
* @tc.name  : loudnessGainPersistenceAfterStopStart_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_150
* @tc.desc  : Test loudness gain value persists across stream Stop then Start cycle
*/
HWTEST_F(HpaeDirectRendererManagerTest, loudnessGainPersistenceAfterStopStart_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->SetLoudnessGain(TEST_STREAM_SESSION_ID, 8.0f), SUCCESS);
    WaitForMsgProcessing(directManager_);

    ASSERT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_FLOAT_EQ(directManager_->loudnessGainNode_->GetLoudnessGain(), 8.0f);

    EXPECT_EQ(directManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    ASSERT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_FLOAT_EQ(directManager_->loudnessGainNode_->GetLoudnessGain(), 8.0f);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
* @tc.name  : loudnessGainNodeDestroyedOnDeInit_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_151
* @tc.desc  : Test loudnessGainNode_ is null after stream destruction
*/
HWTEST_F(HpaeDirectRendererManagerTest, loudnessGainNodeDestroyedOnDeInit_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->loudnessGainNode_, nullptr);

    EXPECT_EQ(directManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    EXPECT_EQ(directManager_->loudnessGainNode_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
* @tc.name  : onRequestLatencyIncludesLoudnessGainNode_001
* @tc.type  : FUNC
* @tc.number: HpaeDirectRendererManagerTest_152
* @tc.desc  : Test OnRequestLatency exercises loudnessGainNode_ branch without crash
*/
HWTEST_F(HpaeDirectRendererManagerTest, onRequestLatencyIncludesLoudnessGainNode_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    uint64_t latency = 0;
    directManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);

    EXPECT_NE(directManager_->loudnessGainNode_, nullptr);
    EXPECT_EQ(latency, 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}
}