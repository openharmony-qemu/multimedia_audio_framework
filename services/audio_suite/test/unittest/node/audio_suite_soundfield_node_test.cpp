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
static constexpr uint32_t needDataLength = 10;
class AudioSuiteSoundFieldNodeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void)
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    };
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

    // Uninitialized, invalid
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_FRONT_FACING));
    EXPECT_EQ(node->SetOptions(name, value), ERROR);
    EXPECT_EQ(node->GetOptions(name, newValue), ERROR);

    // Initialization
    ASSERT_EQ(node->Init(), SUCCESS);

    // Front facing mode
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_FRONT_FACING));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // Grand mode
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_GRAND));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // Near (listening) mode
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_NEAR));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // Wide mode
    value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_WIDE));
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->GetOptions(name, newValue), SUCCESS);
    EXPECT_EQ(newValue == value, true);

    // Invalid values
    // EXPECT_EQ(node->GetOptions("unKnownType", newValue), ERROR);
    // EXPECT_EQ(node->SetOptions("unKnownType", value), ERROR);
    EXPECT_EQ(node->SetOptions(name, "9"), ERROR);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}

HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcessNormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE}, needDataLength);
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    ASSERT_EQ(node->Init(), SUCCESS);
    int32_t ret = node->InitCacheLength(needDataLength);
    EXPECT_EQ(ret, SUCCESS);
    
    std::string setValue = std::to_string(static_cast<int32_t>(AUDIO_SUITE_SOUND_FIELD_WIDE));
    EXPECT_EQ(node->SetOptions("SoundFieldType", setValue), SUCCESS);

    EXPECT_EQ(TestEffectNodeSignalProcess(node, inputs, g_inputfile001, g_outputfile001, g_targetfile001), SUCCESS);

    EXPECT_EQ(node->DeInit(), SUCCESS);
}


HWTEST_F(AudioSuiteSoundFieldNodeUnitTest, TestSignalProcessAbnormal, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSoundFieldNode>();
    AudioSuitePcmBuffer inputBuffer({SAMPLE_RATE_48000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE}, needDataLength);
    std::vector<AudioSuitePcmBuffer *> inputs = {&inputBuffer};
    
    std::vector<AudioSuitePcmBuffer *> outPcmbuffer;
    // not init return nullptr
    outPcmbuffer = node->SignalProcess(inputs);
    EXPECT_EQ(outPcmbuffer[0] == nullptr, true);

    // inputs is empty return nullptr
    ASSERT_EQ(node->Init(), SUCCESS);
    int32_t ret = node->InitCacheLength(needDataLength);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<AudioSuitePcmBuffer *> emptyInputs(0);
    outPcmbuffer = node->SignalProcess(emptyInputs);
    EXPECT_EQ(outPcmbuffer[0] == nullptr, true);
    
    // input is normal & nullptr return nullptr
    outPcmbuffer = node->SignalProcess(inputs);
    EXPECT_EQ(outPcmbuffer[0] != nullptr, true);
    inputs = {nullptr};
    outPcmbuffer = node->SignalProcess(inputs);
    EXPECT_EQ(outPcmbuffer[0] == nullptr, true);

    // input is wrong format return nullptr
    AudioSuitePcmBuffer wrongPcmBuffer({SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE}, needDataLength);
    inputs = {&wrongPcmBuffer};
    outPcmbuffer = node->SignalProcess(inputs);
    EXPECT_EQ(outPcmbuffer[0] == nullptr, true);

    // after deinit return nullptr
    inputs = {&inputBuffer};
    EXPECT_EQ(node->DeInit(), SUCCESS);
    outPcmbuffer = node->SignalProcess(inputs);
    EXPECT_EQ(outPcmbuffer[0] == nullptr, true);

    node->DeInit();
}

}  // namespace