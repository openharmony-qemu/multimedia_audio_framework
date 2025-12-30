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
#include <gmock/gmock.h>
#include <vector>
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_node.h"
#include "audio_suite_pure_voice_change_node.h"
#include "audio_suite_process_node.h"
#include "audio_errors.h"
#include "audio_suite_unittest_tools.h"
#include "audio_voicemorphing_api.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;
using namespace std;
namespace {
struct PureVoiceChangeInfo {
    std::string inputFileName;
    std::string outputFileName;
    std::string compareFileName;
    AudioPureVoiceChangeType pureVoiceType;
    AudioPureVoiceChangeGenderOption valueSexType;
    float pitch;
};

struct PureVoiceParameter {
    std::string value;
    std::string name;
};

static std::string g_outputNodeTestDir = "/data/audiosuite/pure/";

static PureVoiceChangeInfo g_info[] = {
    {"voice_morph_trad_input_stereo.pcm",
        "out1.pcm",
        "voice_morph_trad_stereo_male_uncle_02.pcm",
        PURE_VOICE_CHANGE_TYPE_SEASONED,
        PURE_VOICE_CHANGE_MALE,
        3.0f},
};

static PureVoiceParameter g_setparameter[] = {
    {std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_MALE)) + "," +
            std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED)) + "," +
            std::to_string(static_cast<float>(3.0f)),
        "AudioGeneralVoiceChangeOption"},
    {std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED)) + "," +
            std::to_string(static_cast<float>(0.3f)),
        "AudioPureVoiceChangeOption"},
    {std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_MALE)) + "," +
            std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED) +
                           static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED)) +
            "," + std::to_string(static_cast<float>(0)),
        "AudioPureVoiceChangeOption"},
    {std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_MALE)) + "," +
            std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED)) + "," +
            std::to_string(static_cast<float>(0.1f)),
        "AudioPureVoiceChangeOption"},
};
 
static std::string g_outfile002 = "/data/audiosuite/pure/out2.pcm";
static int32_t g_expectedGetOutputPortCalls = 2;      // Times of GetOutputPort called in DoProcess

class MockInputNode : public AudioNode {
public:
    MockInputNode() : AudioNode(NODE_TYPE_EQUALIZER)
    {}
    ~MockInputNode() {}
    MOCK_METHOD(int32_t, DoProcess, (), ());
    MOCK_METHOD(OutputPort<AudioSuitePcmBuffer*>*, GetOutputPort, ());
    MOCK_METHOD(int32_t, Flush, (), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type), ());
    MOCK_METHOD(int32_t, Connect, (const std::shared_ptr<AudioNode> &preNode), ());
    MOCK_METHOD(int32_t, DisConnect, (const std::shared_ptr<AudioNode> &preNode), ());
};

class AudioSuitePureVoiceChangeNodeTest : public testing::Test {
protected:
    void SetUp() override
    {}
    void TearDown() override
    {}
    int32_t DoprocessTest(AudioPureVoiceChangeType changeType, AudioPureVoiceChangeGenderOption sexType, float pitch,
        std::string inputFile, std::string outputFile);
    std::vector<uint8_t> ReadInputFile(std::string inputFile, size_t frameSizeInput);

    PcmBufferFormat outFormat_ = {SAMPLE_RATE_16000, STEREO, CH_LAYOUT_MONO, SAMPLE_S16LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer =
        std::make_unique<AudioSuitePcmBuffer>(outFormat_);
};

std::vector<uint8_t> AudioSuitePureVoiceChangeNodeTest::ReadInputFile(std::string inputFile, size_t frameSizeInput)
{
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET(ifs.is_open(), std::vector<uint8_t>());
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to apply
    CHECK_AND_RETURN_RET(frameSizeInput > 0, std::vector<uint8_t>());
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t inputFileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<uint8_t> inputfileBuffer(inputFileBufferSize, 0);  // PCM data padding 0
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();
    return inputfileBuffer;
}

int32_t AudioSuitePureVoiceChangeNodeTest::DoprocessTest(AudioPureVoiceChangeType changeType,
    AudioPureVoiceChangeGenderOption sexType, float pitch, std::string inputFile, std::string outputFile)
{
    std::shared_ptr<AudioSuitePureVoiceChangeNode> node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->SetOutputPort(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(inputNodeOutputPort.get()));
    std::string optionValue = std::to_string(static_cast<int32_t>(sexType)) + "," +
                        std::to_string(static_cast<int32_t>(changeType)) + "," +
                        std::to_string(static_cast<float>(pitch));
    int32_t ret = node->SetOptions("AudioPureVoiceChangeOption", optionValue);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    node->Connect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort->GetInputNum() == 1, ERROR);
    OutputPort<AudioSuitePcmBuffer*>* nodeOutputPort = node->GetOutputPort();
    size_t frameSizeInput = buffer->GetDataSize();
    CHECK_AND_RETURN_RET(frameSizeInput > 0, ERROR);
    std::vector<uint8_t> inputfileBuffer = ReadInputFile(inputFile, frameSizeInput);
    CHECK_AND_RETURN_RET(inputfileBuffer.empty() == false, ERROR);
    std::ofstream outFile(outputFile, std::ios::binary | std::ios::out | std::ios::app);
    uint8_t *readPtr = inputfileBuffer.data();
    int32_t frames = inputfileBuffer.size() / frameSizeInput;
    int32_t frameIndex = 0;
    while (!node->GetAudioNodeDataFinishedFlag()) {
        EXPECT_CALL(*mockInputNode_, DoProcess())
            .WillRepeatedly(::testing::Invoke([&]() {
            if (frameIndex == frames - 1) {
                buffer->SetIsFinished(true);
            }
            memcpy_s(buffer->GetPcmData(), frameSizeInput, readPtr, frameSizeInput);
            inputNodeOutputPort->WriteDataToOutput(buffer.get());
            frameIndex++;
            readPtr += frameSizeInput;
            return SUCCESS;
        }));
        std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
        CHECK_AND_RETURN_RET(result.size() == 1, ERROR);
        outFile.write(reinterpret_cast<const char *>(result[0]->GetPcmData()), frameSizeInput);
    }
    outFile.close();
    node->DisConnect(mockInputNode_);
    CHECK_AND_RETURN_RET(inputNodeOutputPort->GetInputNum() == 0, ERROR);
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    buffer->SetIsFinished(false);
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
    return SUCCESS;
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, DoProcessTest, TestSize.Level0)
{
    std::string inputFilePath = g_outputNodeTestDir + "voice_morph_trad_input_stereo.pcm";
    std::string outputFilePath = g_outfile002;
    AudioPureVoiceChangeType changeType = PURE_VOICE_CHANGE_TYPE_SEASONED;
    AudioPureVoiceChangeGenderOption SexType = PURE_VOICE_CHANGE_MALE;
    int ret = DoprocessTest(changeType, SexType, 2.0, inputFilePath, outputFilePath);
    EXPECT_EQ(SUCCESS, ret);
}

static bool RunPureVoiceChangeTest(
    const PureVoiceChangeInfo &info, const std::string &inputFilePath, const std::string &outputFilePath)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    std::string value = std::to_string(static_cast<int32_t>(info.valueSexType)) + "," +
                        std::to_string(static_cast<int32_t>(info.pureVoiceType)) + "," +
                        std::to_string(static_cast<float>(info.pitch));
    std::string name = "AudioPureVoiceChangeOption";
    int32_t ret = node->SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file(inputFilePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_16000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE), PCM_DATA_DURATION_40_MS);
    const size_t frameBytes = buffer->GetDataSize();
    std::ofstream outFile(outputFilePath, std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return false;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        std::copy(rawBuffer.begin(), rawBuffer.end(), buffer->GetPcmData());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = node->SignalProcess(inputs);
        EXPECT_TRUE(outPcmbuffer != nullptr);
        uint8_t *data = outPcmbuffer->GetPcmData();
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(data), outPcmbuffer->GetDataSize());
            if (outFile.fail()) {
                break;
            }
        }
    }
    ret = node->GetOptions(name, value);
    file.close();
    outFile.close();
    delete buffer;
    node->DeInit();
    return true;
}

static bool RunSplitDataInHalfTest(
    const PureVoiceChangeInfo &info, const std::string &inputFilePath, const std::string &outputFilePath)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    std::string value = std::to_string(static_cast<int32_t>(info.valueSexType)) + "," +
                        std::to_string(static_cast<int32_t>(info.pureVoiceType)) + "," +
                        std::to_string(static_cast<float>(info.pitch));
    std::string name = "AudioPureVoiceChangeOption";
    int32_t ret = node->SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);
    std::vector<AudioSuitePcmBuffer *> inputs;
    std::ifstream file(inputFilePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    AudioSuitePcmBuffer *buffer = new AudioSuitePcmBuffer(
        PcmBufferFormat(SAMPLE_RATE_16000, STEREO, CH_LAYOUT_STEREO, SAMPLE_S16LE), PCM_DATA_DURATION_40_MS);
    const size_t frameBytes = buffer->GetDataSize();
    std::ofstream outFile(outputFilePath, std::ios::binary | std::ios::out);
    if (!outFile) {
        delete buffer;
        file.close();
        return false;
    }
    vector<char> rawBuffer(frameBytes);
    while (file.read(rawBuffer.data(), frameBytes).gcount() > 0) {
        if (file.gcount() != rawBuffer.size()) {
            rawBuffer.resize(file.gcount());
        }
        std::copy(rawBuffer.begin(), rawBuffer.end(), buffer->GetPcmData());
        inputs.clear();
        inputs.push_back(buffer);
        AudioSuitePcmBuffer *outPcmbuffer = nullptr;
        outPcmbuffer = node->splitDataInHalf(inputs);
        EXPECT_TRUE(outPcmbuffer != nullptr);
        uint8_t *data = outPcmbuffer->GetPcmData();
        if (data != nullptr) {
            outFile.write(reinterpret_cast<const char *>(data), outPcmbuffer->GetDataSize());
            if (outFile.fail()) {
                break;
            }
        }
    }
    ret = node->GetOptions(name, value);
    file.close();
    outFile.close();
    delete buffer;
    node->DeInit();
    return true;
}

static bool RunAllTestCases(const PureVoiceChangeInfo* testCases, size_t count)
{
    for (size_t idx = 0; idx < count; idx++) {
        const PureVoiceChangeInfo& info = testCases[idx];
        std::string inputFilePath = g_outputNodeTestDir + info.inputFileName;
        std::string outputFilePath = g_outputNodeTestDir + info.outputFileName;
        std::string compareFilePath = g_outputNodeTestDir + info.compareFileName;
        std::cout << testCases[idx].inputFileName << std::endl;
        std::cout << testCases[idx].outputFileName << std::endl;
        std::cout << testCases[idx].compareFileName << std::endl;
        EXPECT_TRUE(RunPureVoiceChangeTest(info, inputFilePath, outputFilePath));
        EXPECT_TRUE(RunSplitDataInHalfTest(info, inputFilePath, outputFilePath));
    }
    return true;
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, testAudioSuitePurelVoiceChangeNodeSignalProcess001, TestSize.Level0)
{
    EXPECT_TRUE(RunAllTestCases(g_info, sizeof(g_info) / sizeof(g_info[0])));
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, AudioSuitePurelVoiceChangeNodeInitTest, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->isInit_ = true;
    int32_t ret = node->Init();
    EXPECT_EQ(ret, ERROR);

    node->isInit_ = false;
    ret = node->Init();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, AudioSuitePurelVoiceChangeNodeDeInitTest, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->isInit_ = false;
    node->algoInterfaceImpl_ = nullptr;
    int32_t ret = node->DeInit();
    EXPECT_EQ(ret, ERROR);

    node->isInit_ = true;
    ret = node->DeInit();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, DoProcessTest_002, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePureVoiceChangeNode> node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    node->SetBypassEffectNode(true);
    std::shared_ptr<MockInputNode> mockInputNode_ = std::make_shared<MockInputNode>();
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> inputNodeOutputPort =
        std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(mockInputNode_);
    inputNodeOutputPort->SetOutputPort(mockInputNode_);
    EXPECT_CALL(*mockInputNode_, GetOutputPort())
        .Times(g_expectedGetOutputPortCalls).WillRepeatedly(::testing::Return(inputNodeOutputPort.get()));

    node->Connect(mockInputNode_);
    EXPECT_EQ(1, inputNodeOutputPort->GetInputNum());
    OutputPort<AudioSuitePcmBuffer*>* nodeOutputPort = node->GetOutputPort();
    EXPECT_CALL(*mockInputNode_, DoProcess())
            .WillRepeatedly(::testing::Invoke([&]() {
            std::vector<uint8_t> tempData(buffer->GetDataSize(), 0);
            memcpy_s(buffer->GetPcmData(), buffer->GetDataSize(), tempData.data(), buffer->GetDataSize());
            inputNodeOutputPort->WriteDataToOutput(buffer.get());
            return SUCCESS;
        }));
    std::vector<AudioSuitePcmBuffer *> result = nodeOutputPort->PullOutputData(outFormat_, true);
    EXPECT_EQ(1, result.size());

    node->DisConnect(mockInputNode_);
    EXPECT_EQ(0, inputNodeOutputPort->GetInputNum());
    testing::Mock::VerifyAndClearExpectations(mockInputNode_.get());
    node->Flush();
    mockInputNode_.reset();
    inputNodeOutputPort.reset();
}

HWTEST_F(AudioSuitePureVoiceChangeNodeTest, AudioSuitePurelVoiceChangeNodeSetOptionTest001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    int32_t ret;
 
    size_t count = sizeof(g_setparameter) / sizeof(g_setparameter[0]);
    for (size_t idx = 0; idx < count; idx++) {
        ret = node->SetOptions(g_setparameter[idx].name, g_setparameter[idx].value);
        EXPECT_EQ(ret, ERROR);
    }
}
 
HWTEST_F(AudioSuitePureVoiceChangeNodeTest, AudioSuitePurelVoiceChangeNodeGetOptionTest001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuitePureVoiceChangeNode>();
    node->Init();
    std::string value = std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_MALE)) + "," +
                        std::to_string(static_cast<int32_t>(PURE_VOICE_CHANGE_TYPE_SEASONED)) + "," +
                        std::to_string(static_cast<float>(0));
    std::string name = "AudioPureVoiceChangeOption";
    int32_t ret = node->GetOptions(name, value);
    EXPECT_EQ(ret, ERROR);
 
    ret = node->SetOptions(name, value);
    EXPECT_EQ(ret, SUCCESS);
 
    std::string getValue;
    ret = node->GetOptions(name, getValue);
    EXPECT_EQ(getValue, value);
 
    std::string getName = "AudioGeneralVoiceChangeOption";
    ret = node->GetOptions(getName, getValue);
    EXPECT_EQ(ret, ERROR);
}
 
}  // namespace