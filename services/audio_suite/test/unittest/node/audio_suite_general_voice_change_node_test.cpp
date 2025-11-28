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
    AudioGeneralVoiceChangeType gnerralVoiceType;
};

static std::string g_outputNodeTestDir = "/data/audiosuite/vb/";

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

class AudioSuiteGeneralVoiceChangeNodeTest : public testing::Test {
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

static bool RunGeneralVoiceChangeTest(
    const GeneralVoiceChangeInfo &info, const std::string &inputFilePath, const std::string &outputFilePath)
{
    AudioSuiteGeneralVoiceChangeNode vb;
    vb.Init();
    std::string value = std::to_string(static_cast<int32_t>(info.gnerralVoiceType));
    std::string name = "AudioGeneralVoiceChangeType";
    int32_t ret = vb.SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);

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
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        EXPECT_TRUE(outPcmbuffer != nullptr);
        uint8_t *data = outPcmbuffer->GetPcmData();
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(data), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
    }
    ret = vb.SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);

    file.close();
    outFile.close();
    delete buffer;
    vb.DeInit();
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
    AudioSuiteGeneralVoiceChangeNode vb;
    vb.algoInterface_ = nullptr;
    int32_t ret = vb.DeInit();
    EXPECT_EQ(ret, SUCCESS);
}

}  // namespace