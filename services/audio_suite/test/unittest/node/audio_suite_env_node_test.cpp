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
#include <algorithm>
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
    AudioSuiteEnvNode env;
    env.Init();
    std::string envValue = "1";
    std::string name = "EnvironmentType";
    env.SetOptions(name, envValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::string filename = "/data/audiosuite/envnode/env_48000_2_16.pcm";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    const uint32_t sampleRate = 48000;
    const uint32_t channelCount = 2;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/envnode/env_48000_2_16Out.pcm", std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        size_t actualBytesRead = file.gcount();
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        int inputSamples = actualBytesRead / 2;
        env.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = env.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        env.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    delete buffer;
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/envnode/env_48000_2_16Out.pcm",
                  "/data/audiosuite/envnode/env_48000_2_16_target.pcm"),
        true);
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodeSignalProcess_002, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    std::string envValue = "2";
    std::string name = "EnvironmentType";
    env.SetOptions(name, envValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::string filename = "/data/audiosuite/envnode/env_44100_2_32f.pcm";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    const uint32_t sampleRate = 44100;
    const uint32_t channelCount = 2;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, CH_LAYOUT_STEREO);
    const size_t frameBytes = 44100 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/envnode/env_44100_2_32fOut.pcm", std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        size_t actualBytesRead = file.gcount();
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        int inputSamples = actualBytesRead / 2;
        env.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = env.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        env.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    delete buffer;
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/envnode/env_44100_2_32fOut.pcm",
                  "/data/audiosuite/envnode/env_44100_2_32f_target.pcm"),
        true);
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodeSignalProcess_003, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    std::string envValue = "3";
    std::string name = "EnvironmentType";
    env.SetOptions(name, envValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::string filename = "/data/audiosuite/envnode/env_44100_1_32f.pcm";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    const uint32_t sampleRate = 44100;
    const uint32_t channelCount = 1;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, CH_LAYOUT_MONO);
    const size_t frameBytes = 44100 * 0.02 * 2;
    std::ofstream outFile("/data/audiosuite/envnode/env_44100_1_32fOut.pcm", std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        size_t actualBytesRead = file.gcount();
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        int inputSamples = actualBytesRead / 2;
        env.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = env.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        env.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    delete buffer;
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/envnode/env_44100_1_32fOut.pcm",
                  "/data/audiosuite/envnode/env_44100_1_32f_target.pcm"),
        true);
}

}  // namespace