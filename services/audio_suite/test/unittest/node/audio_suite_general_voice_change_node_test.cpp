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
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_general_voice_change_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
using namespace std;

namespace {

struct GeneralVoiceChangeInfo {
    std::string inputFileName;
    std::string outputFileName;
    std::string compareFileName;
    AudioGeneralVoiceChangeType generalVoiceType;
};

struct GeneralVoiceParameter {
    std::string value;
    std::string name;
};

static std::string g_outputNodeTestDir = "/data/audiosuite/vb/";
static constexpr uint32_t NEED_DATA_LENGTH = 20;

static GeneralVoiceChangeInfo g_info[] = {
    {"vb_input_48000_2_S16LE.pcm", "out1.pcm", "voice_morph_output_cute_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_CUTE},
    {"vb_input_48000_2_S16LE.pcm",
        "out2.pcm",
        "voice_morph_output_cyberpunk_sframe.pcm",
        GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK},
    {"vb_input_48000_2_S16LE.pcm",
        "out3.pcm",
        "voice_morph_output_female_sframe.pcm",
        GENERAL_VOICE_CHANGE_TYPE_FEMALE},
    {"vb_input_48000_2_S16LE.pcm", "out4.pcm", "voice_morph_output_male_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_MALE},
    {"vb_input_48000_2_S16LE.pcm", "out5.pcm", "voice_morph_output_mix_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_MIX},
    {"vb_input_48000_2_S16LE.pcm",
        "out6.pcm",
        "voice_morph_output_monster_sframe.pcm",
        GENERAL_VOICE_CHANGE_TYPE_MONSTER},
    {"vb_input_48000_2_S16LE.pcm",
        "out7.pcm",
        "voice_morph_output_uncle_sframe.pcm",
        GENERAL_VOICE_CHANGE_TYPE_SEASONED},
    {"vb_input_48000_2_S16LE.pcm", "out8.pcm", "voice_morph_output_synth_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_SYNTH},
    {"vb_input_48000_2_S16LE.pcm", "out9.pcm", "voice_morph_output_trill_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_TRILL},
    {"vb_input_48000_2_S16LE.pcm", "out10.pcm", "voice_morph_output_war_sframe.pcm", GENERAL_VOICE_CHANGE_TYPE_WAR},
};

static GeneralVoiceParameter g_setparameter[] = {
    {std::to_string(static_cast<int32_t>(GENERAL_VOICE_CHANGE_TYPE_SEASONED)), "AudioPureVoiceChangeOption"},
    {std::to_string(static_cast<int32_t>(GENERAL_VOICE_CHANGE_TYPE_SEASONED) +
                    static_cast<int32_t>(GENERAL_VOICE_CHANGE_TYPE_SEASONED)),
        "AudioGeneralVoiceChangeType"},
};

class AudioSuiteGeneralVoiceChangeNodeTest : public testing::Test {
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

static bool RunGeneralVoiceChangeTest(
    const GeneralVoiceChangeInfo &info, const std::string &inputFilePath, const std::string &outputFilePath)
{
    auto node = std::make_shared<AudioSuiteGeneralVoiceChangeNode>();
    node->Init();
    std::string value = std::to_string(static_cast<int32_t>(info.generalVoiceType));
    std::string name = "AudioGeneralVoiceChangeType";
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);
    EXPECT_EQ(node->InitCacheLength(NEED_DATA_LENGTH), SUCCESS);

    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file(inputFilePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer =
        new AudioSuitePcmBuffer(PcmBufferFormat(SAMPLE_RATE_48000, 2, CH_LAYOUT_STEREO, SAMPLE_S16LE));
    const size_t frameBytes = buffer->GetDataSize();
    std::ofstream outFile(outputFilePath, std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return false;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        size_t actualBytesRead = file.gcount();
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        std::copy(rawBuffer.begin(), rawBuffer.end(), buffer->GetPcmData());
        inputs.clear();
        inputs.push_back(buffer);
        std::vector<AudioSuitePcmBuffer *> outPcmbuffer = node->SignalProcess(inputs);

        EXPECT_TRUE(outPcmbuffer[0] != nullptr);
        uint8_t *data = outPcmbuffer[0]->GetPcmData();
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(data), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
    }
    EXPECT_EQ(node->SetOptions(name, value), SUCCESS);

    file.close();
    outFile.close();
    delete buffer;
    node->DeInit();
    return true;
}

static bool RunAllTestCases(const GeneralVoiceChangeInfo* testCases, size_t count)
{
    for (size_t idx = 0; idx < count; idx++) {
        const GeneralVoiceChangeInfo& info = testCases[idx];
        std::string inputFilePath = g_outputNodeTestDir + info.inputFileName;
        std::string outputFilePath = g_outputNodeTestDir + info.outputFileName;
        std::string compareFilePath = g_outputNodeTestDir + info.compareFileName;
        std::cout << testCases[idx].inputFileName << std::endl;
        std::cout << testCases[idx].outputFileName << std::endl;
        std::cout << testCases[idx].compareFileName << std::endl;

        if (!RunGeneralVoiceChangeTest(info, inputFilePath, outputFilePath)) {
            continue;
        }
        EXPECT_TRUE(IsFilesEqual(outputFilePath, compareFilePath));
    }
    return true;
}

HWTEST_F(AudioSuiteGeneralVoiceChangeNodeTest, testAudioSuiteGeneralVoiceChangeNodeSignalProcess001, TestSize.Level0)
{
    EXPECT_TRUE(RunAllTestCases(g_info, sizeof(g_info) / sizeof(g_info[0])));
}

HWTEST_F(AudioSuiteGeneralVoiceChangeNodeTest, testAudioSuiteGeneralVoiceChangeNodeDeInit001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteGeneralVoiceChangeNode>();
    node->algoInterface_ = nullptr;
    int32_t ret = node->DeInit();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteGeneralVoiceChangeNodeTest, AudioSuiteGeneralVoiceChangeNodeSetOptionTest001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteGeneralVoiceChangeNode>();
    node->Init();
    int32_t ret;
 
    size_t count = sizeof(g_setparameter) / sizeof(g_setparameter[0]);
    for (size_t idx = 0; idx < count; idx++) {
        ret = node->SetOptions(g_setparameter[idx].name, g_setparameter[idx].value);
        EXPECT_EQ(ret, ERROR);
    }
}
 
HWTEST_F(AudioSuiteGeneralVoiceChangeNodeTest, AudioSuiteGeneralVoiceChangeNodeGetOptionTest001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteGeneralVoiceChangeNode>();
    node->Init();
    std::string value = std::to_string(static_cast<int32_t>(GENERAL_VOICE_CHANGE_TYPE_SEASONED));
    std::string name = "AudioGeneralVoiceChangeType";
    int32_t ret = node->GetOptions(name, value);
    EXPECT_EQ(ret, ERROR);
 
    ret = node->SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);
 
    std::string getValue;
    ret = node->GetOptions(name, getValue);
    EXPECT_EQ(getValue, value);
}

}  // namespace