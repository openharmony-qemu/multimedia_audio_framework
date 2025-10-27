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
#include "audio_suite_eq_node.h"
#include "audio_suite_unittest_tools.h"

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
HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess_001, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    std::string eqValue = "5:2:1:-1:-5:-5:-2:1:2:4";
    std::string name = "AudioEqualizerFrequencyBandGains";
    eq.SetOptions(name, eqValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/eqnode/audiosuite/eqnode/eq_48000_2_16.pcm", std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_48000, ALGO_CHANNEL_NUM, layout);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/eqnode/eq_48000_2_16Out.pcm", std::ios::binary | std::ios::out);
    if (!outFile || !file.is_open()) {
        eq.DeInit();
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
    EXPECT_EQ(IsFilesEqual(
                  "/data/audiosuite/eqnode/eq_48000_2_16Out.pcm", "/data/audiosuite/eqnode/eq_48000_2_16_target.pcm"),
        true);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess_002, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    std::string eqValue = "2:0:2:3:6:5:-1:3:4:4";
    std::string name = "AudioEqualizerFrequencyBandGains";
    eq.SetOptions(name, eqValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/eqnode/eq_44100_2_32f.pcm", std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_44100, ALGO_CHANNEL_NUM, layout);
    const size_t frameBytes = 44100 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/eqnode/eq_44100_2_32fOut.pcm", std::ios::binary | std::ios::out);
    if (!outFile || !file.is_open()) {
        eq.DeInit();
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
    EXPECT_EQ(IsFilesEqual(
                  "/data/audiosuite/eqnode/eq_44100_2_32fOut.pcm", "/data/audiosuite/eqnode/eq_44100_2_32f_target.pcm"),
        true);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess_003, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();
    std::string eqValue = "4:3:2:-3:0:0:5:4:2:0";
    std::string name = "AudioEqualizerFrequencyBandGains";
    eq.SetOptions(name, eqValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/eqnode/eq_44100_1_32f.pcm", std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_44100, 1, layout);
    const size_t frameBytes = 44100 * 0.02 * 2;
    std::ofstream outFile("/data/audiosuite/eqnode/eq_44100_1_32fOut.pcm", std::ios::binary | std::ios::out);
    if (!outFile || !file.is_open()) {
        eq.DeInit();
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
    EXPECT_EQ(IsFilesEqual(
                  "/data/audiosuite/eqnode/eq_44100_1_32fOut.pcm", "/data/audiosuite/eqnode/eq_44100_1_32f_target.pcm"),
        true);
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSetOptions, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    eq.Init();

    std::string value = "";
    EXPECT_EQ(eq.SetOptions("AudioEqualizerFrequencyBandGains", "8:8:8:8:8:8:8:0:10:-10"), 0);
    EXPECT_EQ(eq.GetOptions("AudioEqualizerFrequencyBandGains", value), 0);
    EXPECT_EQ(value, "8:8:8:8:8:8:8:0:10:-10");

    EXPECT_NE(eq.SetOptions("-------------", "0:0:0:0:0:0:0:0:0:0"), 0);
    EXPECT_NE(eq.GetOptions("-------------", value), 0);
    EXPECT_EQ(value, "8:8:8:8:8:8:8:0:10:-10");
}

HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeDeInit, TestSize.Level0)
{
    AudioSuiteEqNode eq;
    EXPECT_EQ(eq.Init(), 0);
    EXPECT_NE(eq.Init(), 0);
    EXPECT_EQ(eq.DeInit(), 0);
}
}  // namespace