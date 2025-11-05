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
#include "audio_suite_unittest_tools.h"
#include "audio_suite_soundfield_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
static std::string g_inputfile001 = "/data/audiosuite/48000_2_16.pcm";
static std::string g_outputfile001 = "/data/audiosuite/soundfield_output_48000_2_S16LE.pcm";
static std::string g_targetfile001 = "/data/audiosuite/soundfieldnode_target_48000_2_S16LE.pcm";

class AudioSuiteSoundFieldNodeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void){};
    void TearDown(void){};
};

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestInitAndDeinit, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();
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

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSetAndGetOptions, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();

    std::string name = "SoundFieldType";
    std::string value = "";
    std::string newValue = "";

    // 未初始化 无效
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_FRONT_FACING));
    EXPECT_EQ(node->SetOptions(name, value), ERROR);
    EXPECT_EQ(node->GetOptions(name, newValue), ERROR);

    // 初始化
    ASSERT_EQ(node->Init(), SUCCESS);

    // 前置
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_FRONT_FACING));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // 宏大
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_GRAND));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // 聆听
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_NEAR));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // 宽广
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_WIDE));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // 无效值
    EXPECT_EQ(node->GetOptions("unKnownType", newValue), ERROR);
    EXPECT_EQ(node->SetOptions("unKnownType", value), ERROR);
    EXPECT_EQ(node->SetOptions(name, "9"), ERROR);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcessNormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    ASSERT_EQ(node->Init(), SUCCESS);
    
    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_WIDE));
    EXPECT_EQ(node->SetOptions("SoundFieldType", setValue), SUCCESS);

    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}


HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcessAbnormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE});
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};

    // not init return nullptr
    EXPECT_EQ(node->SignalProcess(inputs) == nullptr, true);

    // inputs is empty return nullptr
    ASSERT_EQ(node->Init(), SUCCESS);
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