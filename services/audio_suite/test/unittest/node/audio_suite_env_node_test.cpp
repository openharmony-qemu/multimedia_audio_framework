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
    EXPECT_EQ(env.SetOptions("EnvironmentType", "0"), 0);
    EXPECT_EQ(env.SetOptions("EnvironmentType", "2"), 0);
    EXPECT_EQ(env.SetOptions("EnvironmentType", "3"), 0);
    EXPECT_EQ(env.SetOptions("EnvironmentType", "4"), 0);
    EXPECT_NE(env.SetOptions("---------------", "0"), 0);
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodeSignalProcess_001, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    std::string envValue = "1";
    std::string name = "EnvironmentType";
    env.SetOptions(name, envValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::string filename = "/data/48000_2_16.pcm";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    const uint32_t sampleRate = 48000;
    const uint32_t channelCount = 2;
    const AudioChannelLayout layout = CH_LAYOUT_STEREO;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, layout);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/48000_2_16Out.pcm", std::ios::binary | std::ios::out);
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
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodeSignalProcess_002, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    std::string envValue = "1";
    std::string name = "EnvironmentType";
    env.SetOptions(name, envValue);
    std::vector<AudioSuitePcmBuffer *> inputs;

    const uint32_t sampleRate = 48000;
    const uint32_t channelCount = 2;
    const AudioChannelLayout layout = CH_LAYOUT_STEREO;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, layout);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;

    vector<char> rawBuffer(frameBytes);
    std::fill(rawBuffer.begin(), rawBuffer.end(), 0);

    int inputSamples = frameBytes / 2;
    env.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
    inputs.push_back(buffer);
    AudioSuitePcmBuffer *outPcmbuffer = nullptr;
    outPcmbuffer = env.SignalProcess(inputs);
    float *data = outPcmbuffer->GetPcmDataBuffer();
    int16_t *outData = new int16_t[inputSamples];
    env.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
    delete[] outData;
    delete buffer;
}

HWTEST_F(AudioSuiteEnvNodeTest, testAudioSuiteEnvNodepreProcess, TestSize.Level0)
{
    AudioSuiteEnvNode env;
    env.Init();
    AudioSuitePcmBuffer *inputPcmbuffer1 =
        new AudioSuitePcmBuffer(SAMPLE_RATE_44100, ALGO_CHANNEL_NUM, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer *inputPcmbuffer2 = new AudioSuitePcmBuffer(SAMPLE_RATE_44100, 1, CH_LAYOUT_STEREO);
    std::vector<AudioSuitePcmBuffer *> inputs;
    env.SignalProcess(inputs);
    EXPECT_EQ(env.preProcess(inputPcmbuffer1), 0);
    EXPECT_NE(env.preProcess(inputPcmbuffer2), 0);
    delete inputPcmbuffer1;
    delete inputPcmbuffer2;
}

}  // namespace