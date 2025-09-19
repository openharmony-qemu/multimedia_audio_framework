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
#include "audio_suite_output_node.h"
#include "audio_suite_input_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"

#include "audio_suite_input_node.h"
#include "audio_suite_mixer_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_limiter.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

std::string g_fileNameOne = "/data/44100_2_32f.pcm";
std::string g_fileNameTwo = "/data/96000_1_32f.pcm";
std::string g_outFilename = "/data/out.pcm";
std::string g_baseFilename = "/data/base_96000_2_32f.pcm";

AudioFormat audioFormat = {
    {
        CH_LAYOUT_STEREO,
        2,
    },
    SAMPLE_F32LE,
    SAMPLE_RATE_96000
};
AudioNodeType nodeType = NODE_TYPE_AUDIO_MIXER;
const uint32_t SAMPLE_RATE = 44100;
const uint32_t CHANNEL_COUNT = 2;
const AudioChannelLayout LAY_OUT = CH_LAYOUT_STEREO;

const uint32_t SAMPLE_RATE_TWO = 96000;
const uint32_t CHANNEL_COUNT_TWO = 1;
const AudioChannelLayout LAY_OUT_TWO = CH_LAYOUT_MONO;

size_t g_frameCount20Ms = (SAMPLE_RATE_44100 * STEREO * 20) / 1000; // 20ms of data
size_t g_frameCount20MsTwo = (SAMPLE_RATE_96000 * 20) / 1000; // 20ms of data

class AudioSuiteMixerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteMixerTest::SetUp()
{}

void AudioSuiteMixerTest::TearDown()
{}

namespace {
HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNode, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteMixerNode> audioSuiteMixerNode =std::make_shared<AudioSuiteMixerNode>(nodeType,
        audioFormat);
    EXPECT_EQ(audioSuiteMixerNode->GetSampleRate(), audioFormat.rate);
}

HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNodeReadFile, TestSize.Level0)
{
    AudioSuiteMixerNode mixer(nodeType, audioFormat);   //初始化一个节点

    std::ifstream file1(g_fileNameOne, std::ios::binary | std::ios::ate);
    std::ifstream file2(g_fileNameTwo, std::ios::binary | std::ios::ate);

    ASSERT_TRUE(file1.is_open()) << "Failed to open file1: " << g_fileNameOne;
    ASSERT_TRUE(file2.is_open()) << "Failed to open file2: " << g_fileNameTwo;

    file1.seekg(0, std::ios::beg);
    file2.seekg(0, std::ios::beg);

    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ofstream outProcessedFile(g_outFilename, std::ios::binary);

    while (true) {
        AudioSuitePcmBuffer *buffer1 = new AudioSuitePcmBuffer(SAMPLE_RATE, CHANNEL_COUNT, LAY_OUT);
        buffer1->ResizePcmBuffer(SAMPLE_RATE, CHANNEL_COUNT);
        buffer1->pcmDataBuffer_.resize(g_frameCount20Ms);

        AudioSuitePcmBuffer *buffer2 = new AudioSuitePcmBuffer(SAMPLE_RATE_TWO, CHANNEL_COUNT_TWO, LAY_OUT_TWO);
        buffer2->ResizePcmBuffer(SAMPLE_RATE_TWO, CHANNEL_COUNT_TWO);
        buffer2->pcmDataBuffer_.resize(g_frameCount20MsTwo);

        file1.read(reinterpret_cast<char *>(buffer1->GetPcmDataBuffer()), g_frameCount20Ms * sizeof(float));
        file2.read(reinterpret_cast<char *>(buffer2->GetPcmDataBuffer()), g_frameCount20MsTwo * sizeof(float));

        if (file1.eof() || file2.eof()) {
            delete buffer1;
            delete buffer2;
            break;
        }
        inputs.push_back(buffer1);
        inputs.push_back(buffer2);

        outProcessedFile.write(reinterpret_cast<const char *>((mixer.SignalProcess(inputs))->GetPcmDataBuffer()),
            g_frameCount20MsTwo * 2 * sizeof(float));

        for (auto &input : inputs) {
            delete input;
        }
        inputs.clear();
    }
    file1.close();
    file2.close();
    outProcessedFile.close();
}

HWTEST_F(AudioSuiteMixerTest, constructHpaeMixerNodeCompar, TestSize.Level0)
{
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseFilename, std::ios::binary);

    ASSERT_TRUE(outFile.is_open()) << "Failed to open out.pcm";
    ASSERT_TRUE(baseFile.is_open()) << "Failed to open base.pcm";

    std::vector<char> out_data;
    std::vector<char> base_data;

    outFile.seekg(0, std::ios::end);
    out_data.resize(outFile.tellg());
    outFile.seekg(0, std::ios::beg);
    outFile.read(out_data.data(), out_data.size());

    baseFile.seekg(0, std::ios::end);
    base_data.resize(baseFile.tellg());
    baseFile.seekg(0, std::ios::beg);
    baseFile.read(base_data.data(), base_data.size());

    outFile.close();
    baseFile.close();

    EXPECT_EQ(out_data, base_data) << "out.pcm and base.pcm are not identical";
    std::remove(g_outFilename.c_str());
}
}  // namespace