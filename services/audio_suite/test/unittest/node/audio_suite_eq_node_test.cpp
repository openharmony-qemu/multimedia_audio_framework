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

#include "audio_suite_eq_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
#include "audio_suite_eq_algo_interface_impl.h"

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

namespace {
HWTEST_F(AudioSuiteEqNodeTest, testAudioSuiteEqNodeSignalProcess, TestSize.Level0)
{
    AudioSuiteEqAlgoInterfaceImpl algo;
    AudioSuiteEqNode eq;
    eq.Init();
    std::string eqValue = "7";
    std::string name = "EqualizerMode";
    eq.SetOptions(name, eqValue);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::string filename = "/data/48000_2_16f.pcm";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    file.seekg(0, std::ios::beg);
    const uint32_t sampleRate = 48000;
    const uint32_t channelCount = 2;
    const AudioChannelLayout layout = CH_LAYOUT_STEREO;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(sampleRate, channelCount, layout);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/48000_2_16fOut.pcm", std::ios::binary | std::ios::out);
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
}  // namespace