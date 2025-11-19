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
#include "audio_suite_eq_node.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class AudioSuiteEqNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteEqNodeTest::SetUp()
{}

void AudioSuiteEqNodeTest::TearDown()
{}

namespace {
static std::string g_inputfile001 = "/data/audiosuite/eqnode/48000_2_16.pcm";
static std::string g_outputfile001 = "/data/audiosuite/eqnode/eq_48000_2_16Out.pcm";
static std::string g_targetfile001 = "/data/audiosuite/eqnode/eq_48000_2_16_target.pcm";

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess_001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteEqNode>();
    ASSERT_EQ(node->Init(), 0);

    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string eqValue = "5:2:1:-1:-5:-5:-2:1:2:4";

    EXPECT_EQ(node->SetOptions(name, eqValue), 0);

    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), 0);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSetOptions, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();

    std::string value = "";
    EXPECT_EQ(eq.SetOptions("AudioEqualizerFrequencyBandGains", "8:8:8:8:8:8:8:0:10:-10"), 0);
    EXPECT_EQ(eq.GetOptions("AudioEqualizerFrequencyBandGains", value), 0);
    EXPECT_EQ(value, "8:8:8:8:8:8:8:0:10:-10");

    EXPECT_NE(eq.SetOptions("-------------", "0:0:0:0:0:0:0:0:0:0"), 0);
    EXPECT_NE(eq.GetOptions("-------------", value), 0);
    EXPECT_EQ(value, "8:8:8:8:8:8:8:0:10:-10");
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeDeInit, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    EXPECT_EQ(eq.Init(), 0);
    EXPECT_NE(eq.Init(), 0);
    EXPECT_EQ(eq.DeInit(), 0);
}
}  // namespace