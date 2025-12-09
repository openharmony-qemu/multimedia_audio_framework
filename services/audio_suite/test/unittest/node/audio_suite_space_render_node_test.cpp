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
 
#include <gmock/gmock.h>
#include "audio_suite_unittest_tools.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
 
#include "audio_suite_input_node.h"
#include "audio_suite_space_render_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_pcm_buffer.h"
 
using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
 
namespace {
static constexpr AudioSamplingRate SPACE_RENDER_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat SPACE_RENDER_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel SPACE_RENDER_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout SPACE_RENDER_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;

std::string g_fileNameOne = "/data/audiosuite/sr/48000_2_16.pcm";
std::string g_outFilename = "/data/audiosuite/sr/out.pcm";
std::string g_outFilename2 = "/data/audiosuite/sr/out2.pcm";
std::string g_outFilename3 = "/data/audiosuite/sr/out3.pcm";
std::string g_basePositionFilename = "/data/audiosuite/sr/base_position.pcm";
std::string g_baseRotationFilename = "/data/audiosuite/sr/base_rotation.pcm";
std::string g_baseExtensionFilename = "/data/audiosuite/sr/base_extension.pcm";
 
class AudioSuiteSpaceRenderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteSpaceRenderTest::SetUp()
{}

void AudioSuiteSpaceRenderTest::TearDown()
{}

void DoSignalProcess(std::string inputFile, std::string outputFile,
    std::string name, std::string value)
{
    auto node = std::make_shared<AudioSuiteSpaceRenderNode>();
    std::ifstream file1(inputFile, std::ios::binary | std::ios::ate);

    ASSERT_TRUE(file1.is_open()) << "Failed to open file1: " << inputFile;

    file1.seekg(0, std::ios::beg);

    std::ofstream outProcessedFile(outputFile, std::ios::binary);

    node->Init();
    node->SetOptions(name, value);
    AudioSuitePcmBuffer buffer(PcmBufferFormat{
        SPACE_RENDER_ALGO_SAMPLE_RATE, SPACE_RENDER_ALGO_CHANNEL_COUNT,
          SPACE_RENDER_ALGO_CHANNEL_LAYOUT, SPACE_RENDER_ALGO_SAMPLE_FORMAT});
    uint32_t dataSize = buffer.GetDataSize();

    std::vector<AudioSuitePcmBuffer *> inputs;
    bool exitLoop = false;
    while (!exitLoop) {
        file1.read(reinterpret_cast<char *>(buffer.GetPcmData()), dataSize);
        if (file1.eof()) {
            exitLoop = true;
        } else {
            inputs = {&buffer};
            outProcessedFile.write(reinterpret_cast<const char *>((node->SignalProcess(inputs))->GetPcmData()),
                dataSize);
            inputs.clear();
        }
    }
    file1.close();
    outProcessedFile.close();
}

HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderPositionParams001, TestSize.Level0)
{
    DoSignalProcess(g_fileNameOne, g_outFilename, "AudioSpaceRenderPositionParams", "1,1,1");
 
    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_basePositionFilename, std::ios::binary);

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
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderRotationParams001, TestSize.Level0)
{
    DoSignalProcess(g_fileNameOne, g_outFilename, "AudioSpaceRenderRotationParams", "1,1,1,2,0");

    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseRotationFilename, std::ios::binary);

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
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderExtensionParams001, TestSize.Level0)
{
    DoSignalProcess(g_fileNameOne, g_outFilename, "AudioSpaceRenderExtensionParams", "2,90");

    std::ifstream outFile(g_outFilename, std::ios::binary);
    std::ifstream baseFile(g_baseExtensionFilename, std::ios::binary);

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
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderSetParameterParams001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteSpaceRenderNode>();
    node->Init();
    int32_t ret = node->SetOptions("test", "1,1,1");
    EXPECT_EQ(ret, ERROR);
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderGetParameterParams001, TestSize.Level0)
{
    std::string paramValue;
    int32_t ret;
 
    auto node = std::make_shared<AudioSuiteSpaceRenderNode>();
 
    ret = node->Init();
    EXPECT_EQ(SUCCESS, ret);
 
    ret = node->Init();
    EXPECT_EQ(ERROR, ret);
 
    node->SetOptions("AudioSpaceRenderPositionParams", "1,1,1");
    ret = node->GetOptions("AudioSpaceRenderPositionParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('1', paramValue[0]);
 
    node->SetOptions("AudioSpaceRenderRotationParams", "1,1,1,2,0");
    ret = node->GetOptions("AudioSpaceRenderRotationParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('1', paramValue[0]);
 
    node->SetOptions("AudioSpaceRenderExtensionParams", "2,90");
    ret = node->GetOptions("AudioSpaceRenderExtensionParams", paramValue);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ('2', paramValue[0]);
 
    ret = node->GetOptions("test", paramValue);
    EXPECT_EQ(ret, ERROR);
 
    ret = node->DeInit();
    EXPECT_EQ(SUCCESS, ret);
 
    ret = node->DeInit();
    EXPECT_EQ(ERROR, ret);
}
 
HWTEST_F(AudioSuiteSpaceRenderTest, SpaceRenderDoProcess001, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteSpaceRenderNode> node = std::make_shared<AudioSuiteSpaceRenderNode>();
 
    node->SetAudioNodeDataFinishedFlag(true);
    int32_t ret = node->DoProcess();
    EXPECT_EQ(SUCCESS, ret);
 
    node->SetAudioNodeDataFinishedFlag(false);
    node->outputStream_ = nullptr;
    ret = node->DoProcess();
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}
}  // namespace