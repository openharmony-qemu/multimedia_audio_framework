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
#include "audio_suite_eq_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class AudioSuiteEqNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteEqNodeTest::SetUp()
{}

void AudioSuiteEqNodeTest::TearDown()
{}

const AudioChannelLayout layout = CH_LAYOUT_STEREO;
namespace {
HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    std::string eqValue = "7";
    std::string name = "EqualizerMode";
    eq.SetOptions(name, eqValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/48000_2_16.pcm", std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        eq.DeInit();
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_48000, ALGO_CHANNEL_NUM, layout);
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
        eq.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = eq.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        eq.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
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

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSetOptions, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    EXPECT_EQ(eq.SetOptions("AudioEqualizerFrequencyBandGains", "8:8:8:8:8:8:8:0:10:-10"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "1"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "2"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "3"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "4"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "5"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "6"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "7"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "8"), 0);
    EXPECT_EQ(eq.SetOptions("EqualizerMode", "9"), 0);
    EXPECT_NE(eq.SetOptions("-------------", "9"), 0);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeDeInit, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    EXPECT_EQ(eq.Init(), 0);
    EXPECT_NE(eq.Init(), 0);
    EXPECT_EQ(eq.DeInit(), 0);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodepreProcess, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    AudioSuitePcmBuffer *inputPcmbuffer1 =
        new AudioSuitePcmBuffer(SAMPLE_RATE_44100, ALGO_CHANNEL_NUM, CH_LAYOUT_STEREO);
    AudioSuitePcmBuffer *inputPcmbuffer2 = new AudioSuitePcmBuffer(SAMPLE_RATE_44100, 1, CH_LAYOUT_STEREO);
    std::vector<AudioSuitePcmBuffer *> inputs;
    eq.SignalProcess(inputs);
    EXPECT_EQ(eq.preProcess(inputPcmbuffer1), 0);
    EXPECT_NE(eq.preProcess(inputPcmbuffer2), 0);
    delete inputPcmbuffer1;
    delete inputPcmbuffer2;
}
}  // namespace