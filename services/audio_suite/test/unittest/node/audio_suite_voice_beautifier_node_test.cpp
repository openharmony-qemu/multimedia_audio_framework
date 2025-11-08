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
static std::string g_inputfile001 = "/data/audiosuite/vb/48000_2_S16LE.pcm";
static std::string g_outputfile001 = "/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm";
static std::string g_targetfile001 = "/data/audiosuite/vb/vb_48000_2_S16LE_target.pcm";

class AudioSuiteVoiceBeautifierNodeTest : public testing::Test {
public:
    void SetUp() override
    {
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

    // 未初始化 无效
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions(name, setValue), ERROR);
    EXPECT_EQ(node->GetOptions(name, getValue), ERROR);

    // 初始化
    ASSERT_EQ(node->Init(), SUCCESS);

    // 清澈
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // 剧场
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // CD音效
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // 录音棚
    setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO));
    EXPECT_EQ(node->SetOptions(name, setValue), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, getValue), SUCCESS);
    EXPECT_EQ(getValue == setValue, true);

    // 无效值
    EXPECT_EQ(node->GetOptions("unKnownType", getValue), ERROR);
    EXPECT_EQ(node->SetOptions("unKnownType", setValue), ERROR);
    EXPECT_EQ(node->SetOptions(name, "9"), ERROR);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestSignalProcessNormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    ASSERT_EQ(node->Init(), SUCCESS);

    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions("VoiceBeautifierType", setValue), SUCCESS);
    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}


HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, TestSignalProcessAbnormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};

    // not init return nullptr
    EXPECT_EQ(node->SignalProcess(inputs) == nullptr, true);

    // inputs is empty return nullptr
    ASSERT_EQ(node->Init(), SUCCESS);
    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    EXPECT_EQ(node->SetOptions("VoiceBeautifierType", setValue), SUCCESS);

    std::vector<AudioSuitePcmBuffer *> emptyInputs(0);
    EXPECT_EQ(node->SignalProcess(emptyInputs) == nullptr, true);
    
    // input is normal & nullptr return nullptr
    EXPECT_EQ(node->SignalProcess(inputs) != nullptr, true);
    inputs = {nullptr};
    EXPECT_EQ(node->SignalProcess(inputs) == nullptr, true);

    // input is wrong format return nullptr
    AudioSuitePcmBuffer wrongPcmBuffer({SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE});
    inputs = {&wrongPcmBuffer};
    EXPECT_EQ(node->SignalProcess(inputs) == nullptr, true);

    // after deinit return nullptr
    inputs = {&inputBuffer};
    EXPECT_EQ(node->DeInit(), SUCCESS);
    EXPECT_EQ(node->SignalProcess(inputs) == nullptr, true);
}
}  // namespace