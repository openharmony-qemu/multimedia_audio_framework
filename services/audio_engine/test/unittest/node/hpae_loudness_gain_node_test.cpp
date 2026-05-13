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
constexpr float EPSILON_TOLERANCE = 1e-4f;
constexpr uint32_t TEST_FRAMELEN_96K = 1920;
constexpr uint32_t TEST_FRAMELEN_192K = 3840;

static void FillBufferWithValue(HpaePcmBuffer& buffer, float value)
{
    float* data = buffer.GetPcmDataBuffer();
    uint32_t totalSamples = buffer.GetFrameLen() * buffer.GetChannelCount();
    for (uint32_t i = 0; i < totalSamples; i++) {
        data[i] = value;
    }
}

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

/**
* @tc.name  : signalProcess96kHz_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess at 96000Hz with linear gain fallback
*/
HWTEST_F(HpaeLoudnessGainNodeTest, signalProcess96kHz_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN_96K;
    nodeInfo.samplingRate = SAMPLE_RATE_96000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN_96K, SAMPLE_RATE_96000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(6.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float expectedGain = pow(10, 6.0f / 20);
    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples && i < 16; i++) {
        EXPECT_LT(fabs(outData[i] - expectedGain), EPSILON_TOLERANCE);
    }
}

/**
* @tc.name  : signalProcess192kHz_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess at 192000Hz with linear gain fallback
*/
HWTEST_F(HpaeLoudnessGainNodeTest, signalProcess192kHz_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN_192K;
    nodeInfo.samplingRate = SAMPLE_RATE_192000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN_192K, SAMPLE_RATE_192000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(6.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float expectedGain = pow(10, 6.0f / 20);
    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples && i < 16; i++) {
        EXPECT_LT(fabs(outData[i] - expectedGain), EPSILON_TOLERANCE);
    }
}

/**
* @tc.name  : dynamicRateSwitch48kTo96k_001
* @tc.type  : FUNC
* @tc.desc  : Test dynamic sample rate switch from 48kHz to 96kHz updates internal pcmBufferInfo_
*/
HWTEST_F(HpaeLoudnessGainNodeTest, dynamicRateSwitch48kTo96k_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    // Process at 48kHz first
    PcmBufferInfo pcmBufferInfo48k(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer buffer48k(pcmBufferInfo48k);
    FillBufferWithValue(buffer48k, 1.0f);
    std::vector<HpaePcmBuffer*> inputs48k = {&buffer48k};
    node->SignalProcess(inputs48k);

    node->SetLoudnessGain(3.0f);

    // Process at 96kHz
    PcmBufferInfo pcmBufferInfo96k(STEREO, TEST_FRAMELEN_96K, SAMPLE_RATE_96000);
    HpaePcmBuffer buffer96k(pcmBufferInfo96k);
    FillBufferWithValue(buffer96k, 1.0f);
    std::vector<HpaePcmBuffer*> inputs96k = {&buffer96k};
    node->SignalProcess(inputs96k);

    EXPECT_EQ(node->pcmBufferInfo_.rate, SAMPLE_RATE_96000);
    EXPECT_FLOAT_EQ(node->GetLoudnessGain(), 3.0f);
}

/**
* @tc.name  : gainZeroPassthrough_001
* @tc.type  : FUNC
* @tc.desc  : Test SignalProcess returns input directly when gain is 0.0 dB
*/
HWTEST_F(HpaeLoudnessGainNodeTest, gainZeroPassthrough_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 0.75f);

    // Default gain is 0.0f
    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* result = node->SignalProcess(inputs);

    EXPECT_EQ(result, inputs[0]);
}

/**
* @tc.name  : linearGainFallbackValue_001
* @tc.type  : FUNC
* @tc.desc  : Test linear gain fallback applies correct +20dB conversion
*/
HWTEST_F(HpaeLoudnessGainNodeTest, linearGainFallbackValue_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(20.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples; i++) {
        EXPECT_LT(fabs(outData[i] - 10.0f), EPSILON_TOLERANCE);
    }
}

/**
* @tc.name  : linearGainFallbackNeg3dB_001
* @tc.type  : FUNC
* @tc.desc  : Test linear gain fallback applies correct -3dB conversion
*/
HWTEST_F(HpaeLoudnessGainNodeTest, linearGainFallbackNeg3dB_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    PcmBufferInfo pcmBufferInfo(STEREO, TEST_FRAMELEN, SAMPLE_RATE_48000);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    FillBufferWithValue(hpaePcmBuffer, 1.0f);

    node->SetLoudnessGain(-3.0f);

    std::vector<HpaePcmBuffer*> inputs = {&hpaePcmBuffer};
    HpaePcmBuffer* output = node->SignalProcess(inputs);

    float expectedGain = pow(10, -3.0f / 20);
    float* outData = output->GetPcmDataBuffer();
    uint32_t totalSamples = output->GetFrameLen() * output->GetChannelCount();
    for (uint32_t i = 0; i < totalSamples; i++) {
        EXPECT_LT(fabs(outData[i] - expectedGain), EPSILON_TOLERANCE);
    }
}

/**
* @tc.name  : isLoudnessAlgoOn_001
* @tc.type  : FUNC
* @tc.desc  : Test IsLoudnessAlgoOn returns false when DSP library is not loaded
*/
HWTEST_F(HpaeLoudnessGainNodeTest, isLoudnessAlgoOn_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_FALSE(node->IsLoudnessAlgoOn());

    node->SetLoudnessGain(6.0f);
    EXPECT_FALSE(node->IsLoudnessAlgoOn());
}

/**
* @tc.name  : getLatency_001
* @tc.type  : FUNC
* @tc.desc  : Test GetLatency returns 0 when handle_ is null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, getLatency_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_EQ(node->GetLatency(), 0);
}

/**
* @tc.name  : getLatencyWithSessionId_001
* @tc.type  : FUNC
* @tc.desc  : Test GetLatency with sessionId returns 0 when handle_ is null
*/
HWTEST_F(HpaeLoudnessGainNodeTest, getLatencyWithSessionId_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    EXPECT_EQ(node->GetLatency(12345), 0);
}

/**
* @tc.name  : setLoudnessGainUpdatesLinearGain_001
* @tc.type  : FUNC
* @tc.desc  : Test SetLoudnessGain correctly updates internal linearGain_ member
*/
HWTEST_F(HpaeLoudnessGainNodeTest, setLoudnessGainUpdatesLinearGain_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = TEST_FRAMELEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);

    node->SetLoudnessGain(6.0f);
    EXPECT_LT(fabs(node->linearGain_ - pow(10, 6.0f / 20)), EPSILON_TOLERANCE);

    node->SetLoudnessGain(0.0f);
    EXPECT_LT(fabs(node->linearGain_ - 1.0f), EPSILON_TOLERANCE);
}
}