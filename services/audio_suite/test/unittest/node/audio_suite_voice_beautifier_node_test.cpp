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
#include "audio_suite_voice_beautifier_node.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
using namespace std;

namespace {
static std::string g_inputfile001 = "/data/audiosuite/vb/48000_2_16.pcm";
static std::string g_outputfile001 = "/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm";
static std::string g_targetfile001 = "/data/audiosuite/vb/vb_48000_2_S16LE_target.pcm";
static constexpr uint32_t needDataLength = 20;

class AudioSuiteVoiceBeautifierNodeTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    }
    void TearDown() override
    {
    }
};

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestInitAndDeinit, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    // init
    ASSERT_EQ(node->Init(), SUCCESS);
    
    // reset
    EXPECT_EQ(node->DeInit(), SUCCESS);
    EXPECT_EQ(node->Init(), SUCCESS);

    // muti init
    EXPECT_EQ(node->Init(), SUCCESS);
    EXPECT_EQ(node->Init(), SUCCESS);

    // deinit
    EXPECT_EQ(node->DeInit(), SUCCESS);
}


HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestSetAndGetOptions, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};

    std::string name = "VoiceBeautifierType";
    std::string setValue = "";
    std::string getValue = "";

    // Uninitialized, invalid
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions(name, setValue), ERROR);
    EXPECT_EQ(node->GetOptions(name, getValue), ERROR);

    // Initialization
    ASSERT_EQ(node->Init(), SUCCESS);

    // Clear mode
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // Theatre mode
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // CD mode
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // Studio mode
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // Invalid values
    EXPECT_EQ(node->SetOptions(name, "9"), ERROR);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestSignalProcessNormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE}, needDataLength);
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    ASSERT_EQ(node->Init(), SUCCESS);
    int32_t ret = node->InitCacheLength(needDataLength);
    EXPECT_EQ(ret, SUCCESS);

    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions("VoiceBeautifierType", setValue), SUCCESS);
    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}


HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestSignalProcessAbnormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE}, needDataLength);
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};

    // not init return nullptr
    std::vector<AudioSuitePcmBuffer *>  ret = node->SignalProcess(inputs);
    EXPECT_EQ(ret[0] == nullptr, true);

    // inputs is empty return nullptr
    ASSERT_EQ(node->Init(), SUCCESS);
    int32_t retInit = node->InitCacheLength(needDataLength);
    EXPECT_EQ(retInit, SUCCESS);

    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions("VoiceBeautifierType", setValue), SUCCESS);

    std::vector<AudioSuitePcmBuffer *> emptyInputs(0);
    ret = node->SignalProcess(emptyInputs);
    EXPECT_EQ(ret[0] == nullptr, true);
    
    // input is normal & nullptr return nullptr
    ret = node->SignalProcess(inputs);
    EXPECT_EQ(ret[0] != nullptr, true);
    inputs = {nullptr};
    ret = node->SignalProcess(inputs);
    EXPECT_EQ(ret[0] == nullptr, true);

    // input is wrong format return nullptr
    AudioSuitePcmBuffer wrongPcmBuffer({SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE}, needDataLength);
    inputs = {&wrongPcmBuffer};
    ret = node->SignalProcess(inputs);
    EXPECT_EQ(ret[0] == nullptr, true);

    // after deinit return nullptr
    inputs = {&inputBuffer};
    EXPECT_EQ(node->DeInit(), SUCCESS);
    ret = node->SignalProcess(inputs);
    EXPECT_EQ(ret[0] == nullptr, true);
}
}  // namespace