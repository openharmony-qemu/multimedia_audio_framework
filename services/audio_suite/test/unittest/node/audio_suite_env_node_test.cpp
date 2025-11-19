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
#include "audio_suite_env_node.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class AudioSuiteEnvNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteEnvNodeTest::SetUp()
{}

void AudioSuiteEnvNodeTest::TearDown()
{}

namespace {
static std::string g_inputfile001 = "/data/audiosuite/envnode/48000_2_16.pcm";
static std::string g_outputfile001 = "/data/audiosuite/envnode/env_48000_2_16Out.pcm";
static std::string g_targetfile001 = "/data/audiosuite/envnode/env_48000_2_16_target.pcm";

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvDeInit, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    EXPECT_EQ(env.Init(), 0);
    EXPECT_NE(env.Init(), 0);
    EXPECT_EQ(env.DeInit(), 0);
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvSetOptions, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    std::string typeValue = "";
    EXPECT_EQ(env.SetOptions("EnvironmentType", "0"), 0);
    EXPECT_EQ(env.GetOptions("EnvironmentType", typeValue), 0);
    EXPECT_EQ(typeValue, "1");

    EXPECT_EQ(env.SetOptions("EnvironmentType", "1"), 0);
    EXPECT_EQ(env.GetOptions("EnvironmentType", typeValue), 0);
    EXPECT_EQ(typeValue, "1");

    EXPECT_EQ(env.SetOptions("EnvironmentType", "2"), 0);
    EXPECT_EQ(env.GetOptions("EnvironmentType", typeValue), 0);
    EXPECT_EQ(typeValue, "2");

    EXPECT_EQ(env.SetOptions("EnvironmentType", "3"), 0);
    EXPECT_EQ(env.GetOptions("EnvironmentType", typeValue), 0);
    EXPECT_EQ(typeValue, "3");

    EXPECT_EQ(env.SetOptions("EnvironmentType", "4"), 0);
    EXPECT_EQ(env.GetOptions("EnvironmentType", typeValue), 0);
    EXPECT_EQ(typeValue, "4");

    EXPECT_NE(env.SetOptions("---------------", "0"), 0);
    EXPECT_NE(env.GetOptions("---------------", typeValue), 0);
    EXPECT_EQ(typeValue, "4");
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodeSignalProcess_001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteEnvNode>();
    AudioSuitePcmBuffer pcmBufferInput({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&pcmBufferInput};
    ASSERT_EQ(node->Init(), 0);

    std::string envValue = "1";
    std::string name = "EnvironmentType";
    EXPECT_EQ(node->SetOptions(name, envValue), 0);

    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), 0);
}
}  // namespace