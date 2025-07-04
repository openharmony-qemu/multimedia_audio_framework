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

}