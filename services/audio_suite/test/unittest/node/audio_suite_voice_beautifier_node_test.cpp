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
#include "audio_suite_voice_beautifier_node.h"
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
static std::string g_inputPcmFilePath001 = "/data/audiosuite/nr/ainr_input_48000_2_F32LE.pcm";

class AudioSuiteVoiceBeautifierNodeTest : public testing::Test {
public:
    void SetUp() override
    {
        std::filesystem::remove("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm");
        std::filesystem::remove("/data/audiosuite/vb/vb_output_48000_2_F32LE_out.pcm");
        vb.Init();
    }
    void TearDown() override
    {
        vb.DeInit();
    }

private:
    AudioSuiteVoiceBeautifierNode vb;
};

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess001, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/vb/vb_input_48000_2_S16LE.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", std::ios::binary | std::ios::out);
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
        vb.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        vb.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/vb_output_48000_2_S16LE_target01.pcm"),
        true);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess002, TestSize.Level0)
{
    std::shared_ptr<AudioSuiteVoiceBeautifierNode> nrNode = std::make_shared<AudioSuiteVoiceBeautifierNode>();
    EXPECT_NE(nrNode, nullptr);

    EXPECT_EQ(nrNode->Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    std::string name = "VoiceBeautifierType";
    nrNode->SetOptions(name, value);
    AudioSuitePcmBuffer pcmBufferInput(SAMPLE_RATE_96000, MONO, CH_LAYOUT_MONO);
    std::vector<AudioSuitePcmBuffer *> inputs(1);
    inputs[0] = &pcmBufferInput;

    size_t frameSizeInput = pcmBufferInput.GetFrameLen() * sizeof(float);
    size_t frameSizeOutput = nrNode->pcmBufferOutput_.GetFrameLen() * sizeof(float);
    float *inputData = pcmBufferInput.GetPcmDataBuffer();

    // 处理输入文件
    std::ifstream ifs("/data/audiosuite/vb/vb_input_96000_1_F32LE.pcm", std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // pcm文件长度可能不是帧长的整数倍，补0后再传给算法处理
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t fileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<float> inputfileBuffer(fileBufferSize / sizeof(float), 0.0f);  // 32 float PCM data
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *readPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *writePtr = outputfileBuffer.data();
    size_t outputFileSize = 0;
    for (int32_t i = 0; i + frameSizeInput <= fileBufferSize; i += frameSizeInput) {
        // 从inputfileBuffer拷贝一帧数据到pcmBufferInput
        memcpy_s(reinterpret_cast<char *>(inputData), frameSizeInput, readPtr, frameSizeInput);

        AudioSuitePcmBuffer *pcmBufferOutputPtr = nrNode->SignalProcess(inputs);

        // 处理后数据拷贝到outputfileBuffer
        uint8_t *outputData = reinterpret_cast<uint8_t *>(pcmBufferOutputPtr->GetPcmDataBuffer());
        memcpy_s(writePtr, frameSizeOutput, outputData, frameSizeOutput);

        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
        outputFileSize += frameSizeOutput;
    }

    // 输出pcm数据写入文件
    ASSERT_EQ(CreateOutputPcmFile("/data/audiosuite/vb/vb_output_48000_2_F32LE_out.pcm"), true);
    bool isWriteFileSucc =
        WritePcmFile("/data/audiosuite/vb/vb_output_48000_2_F32LE_out.pcm", outputfileBuffer.data(), outputFileSize);
    ASSERT_EQ(isWriteFileSucc, true);

    // 和归档结果比对
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_F32LE_out.pcm",
                  "/data/audiosuite/vb/vb_output_48000_2_F32LE_target02.pcm"),
        true);

    EXPECT_EQ(nrNode->DeInit(), 0);
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess003, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/vb/voice_morph_input.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", std::ios::binary | std::ios::out);
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
        vb.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        vb.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/voice_morph_pc_output_cd.pcm"),
        true);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess004, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/vb/voice_morph_input.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", std::ios::binary | std::ios::out);
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
        vb.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        vb.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/voice_morph_pc_output_clear.pcm"),
        true);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess005, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/vb/voice_morph_input.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", std::ios::binary | std::ios::out);
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
        vb.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        vb.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/voice_morph_pc_output_recording_studio.pcm"),
        true);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess006, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file("/data/audiosuite/vb/voice_morph_input.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        return;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(48000, 2, CH_LAYOUT_STEREO);
    const size_t frameBytes = 48000 * 0.02 * 2 * 2;
    std::ofstream outFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", std::ios::binary | std::ios::out);
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
        vb.ConvertToFloat(SAMPLE_S16LE, inputSamples, rawBuffer.data(), buffer->GetPcmDataBuffer());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = vb.SignalProcess(inputs);
        float *data = outPcmbuffer->GetPcmDataBuffer();
        int16_t *outData = new int16_t[inputSamples];
        vb.ConvertFromFloat(SAMPLE_S16LE, inputSamples, data, outData);
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(outData), actualBytesRead);
            if (outFile.fail()) {
                break;
            }
        }
        delete[] outData;
    }
    file.close();
    outFile.close();
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/voice_morph_pc_output_theatre.pcm"),
        true);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess007, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_96000, MONO, CH_LAYOUT_MONO);
    inputs.push_back(buffer);
    AudioSuitePcmBuffer *outPcmbuffer = nullptr;
    outPcmbuffer = vb.SignalProcess(inputs);
    float *data = outPcmbuffer->GetPcmDataBuffer();
    bool res = true;
    for (size_t i = 0; i < outPcmbuffer->GetFrameLen(); ++i) {
        if (data[i] != 0.0f) {
            res = false;
        }
    }
    EXPECT_TRUE(res);
    delete buffer;
}

HWTEST_F(AudioSuiteVoiceBeautifierNodeTest, testAudioSuiteVoiceBeautifierNodeSignalProcess008, TestSize.Level0)
{
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_THEATRE));
    std::string name = "VoiceBeautifierType";
    vb.SetOptions(name, value);
    std::vector<AudioSuitePcmBuffer *> inputs;
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(SAMPLE_RATE_96000, MONO, CH_LAYOUT_MONO);
    float *bufferData = buffer->GetPcmDataBuffer();
    for (size_t i = 0; i < buffer->GetFrameLen(); ++i) {
        bufferData[i] = 1.0f;
    }
    inputs.push_back(buffer);
    AudioSuitePcmBuffer *outPcmbuffer = nullptr;
    outPcmbuffer = vb.SignalProcess(inputs);
    float *data = outPcmbuffer->GetPcmDataBuffer();
    EXPECT_TRUE(data[outPcmbuffer->GetFrameLen() - 1] != 0);
    delete buffer;
}

}  // namespace