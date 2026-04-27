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
}