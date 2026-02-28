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

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "audio_suite_tempo_pitch_node.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_log.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

static std::string g_inputfile001 = "/data/audiosuite/tempo_pitch/in_48000_1_s16le.pcm";
static std::string g_outfile001 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_0.8_0.8.pcm";
static std::string g_outfile002 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_1.0_0.8.pcm";
static std::string g_outfile003 = "/data/audiosuite/tempo_pitch/out_48000_1_s16le_0.8_1.0.pcm";
static std::string g_targetfile001 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_0.8_0.8.pcm";
static std::string g_targetfile002 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_1.0_0.8.pcm";
static std::string g_targetfile003 = "/data/audiosuite/tempo_pitch/target_48000_1_s16le_0.8_1.0.pcm";

static constexpr uint32_t NEED_DATA_LENGTH = 20;
static constexpr int32_t MAX_FRAMES = 2000;
static constexpr uint32_t MAX_OUTPUT_BYTE = 8192;

/**
 * TempoPitch node UT needs to directly pull output data (old version via OutputPort).
 * In the new scheme, PullOutputData is protected in AudioNode/subclasses, so create a test wrapper.
 */
class TestableTempoPitchNode : public AudioSuiteTempoPitchNode {
public:
    TestableTempoPitchNode() = default;
    ~TestableTempoPitchNode() override = default;

    std::vector<AudioSuitePcmBuffer *> PullForTest(PcmBufferFormat outFormat, bool needConvert, uint32_t needLen)
    {
        return PullOutputData(outFormat, needConvert, needLen);
    }
};

/**
 * Fake pre-node:
 * - When DoProcess is pulled by downstream, write the current frame buffer to outputData_
 * - DoProcess uses gmock to inject "advance one frame on each pull" logic
 */
class FakePreNode : public AudioNode {
public:
    FakePreNode() : AudioNode(NODE_TYPE_EQUALIZER) {}
    ~FakePreNode() override = default;

    MOCK_METHOD(int32_t, DoProcess, (uint32_t needDataLength), (override));

    int32_t Flush() override
    {
        return SUCCESS;
    }

    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        AddPreNode(preNode);
        preNode->AddNextNode(GetSharedInstance());
        return SUCCESS;
    }

    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override
    {
        if (!preNode) {
            return ERR_INVALID_PARAM;
        }
        RemovePreNode(preNode);
        preNode->RemoveNextNode(GetSharedInstance());
        return SUCCESS;
    }

    void SetOutBuffer(AudioSuitePcmBuffer *buf)
    {
        outBuf_ = buf;
    }

    int32_t PushCurrentFrame(uint32_t needLen)
    {
        (void)needLen;
        if (outBuf_ != nullptr) {
            WriteOutputData(outBuf_);
        }
        return SUCCESS;
    }

private:
    AudioSuitePcmBuffer *outBuf_ {nullptr};
};

class AudioSuiteTempoPitchNodeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        std::filesystem::remove(g_outfile001);
        std::filesystem::remove(g_outfile002);
        std::filesystem::remove(g_outfile003);
    }
    void TearDown() override {}

    int32_t DoprocessTest(float speed, float pitch, const std::string &inputFile, const std::string &outputFile);
    std::vector<uint8_t> ReadInputFile(const std::string &inputFile, size_t frameSizeInput);

    PcmBufferFormat outFormat_ = {SAMPLE_RATE_48000, MONO, CH_LAYOUT_MONO, SAMPLE_S16LE};
    std::unique_ptr<AudioSuitePcmBuffer> buffer = std::make_unique<AudioSuitePcmBuffer>(outFormat_, NEED_DATA_LENGTH);
};

std::vector<uint8_t> AudioSuiteTempoPitchNodeTest::ReadInputFile(const std::string &inputFile, size_t frameSizeInput)
{
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET(ifs.is_open(), std::vector<uint8_t>());

    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = static_cast<size_t>(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    CHECK_AND_RETURN_RET(frameSizeInput > 0, std::vector<uint8_t>());

    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t inputFileBufferSize = inputFileSize + zeroPaddingSize;

    std::vector<uint8_t> inputfileBuffer(inputFileBufferSize, 0);  // PCM data padding 0
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();
    return inputfileBuffer;
}

int32_t AudioSuiteTempoPitchNodeTest::DoprocessTest(
    float speed, float pitch, const std::string &inputFile, const std::string &outputFile)
{
    auto node = std::make_shared<TestableTempoPitchNode>();
    node->Init();

    auto preNode = std::make_shared<FakePreNode>();
    preNode->InitFormatConverters();
    preNode->SetOutBuffer(buffer.get());

    std::string optionValue = std::to_string(speed) + "," + std::to_string(pitch);
    EXPECT_EQ(node->SetOptions("speedAndPitch", optionValue), SUCCESS);

    // node <- preNode
    node->Connect(preNode);

    size_t frameSizeInput = buffer->GetDataSize();
    CHECK_AND_RETURN_RET(frameSizeInput > 0, ERROR);

    std::vector<uint8_t> inputfileBuffer = ReadInputFile(inputFile, frameSizeInput);
    CHECK_AND_RETURN_RET(!inputfileBuffer.empty(), ERROR);

    std::ofstream outFile(outputFile, std::ios::binary | std::ios::out);

    uint8_t *readPtr = inputfileBuffer.data();
    int32_t frames = static_cast<int32_t>(inputfileBuffer.size() / frameSizeInput);
    int32_t frameIndex = 0;
    uint32_t loopCount = 0;

    EXPECT_CALL(*preNode, DoProcess(NEED_DATA_LENGTH))
        .WillRepeatedly(::testing::Invoke([this, &frameIndex, &frames, frameSizeInput, &readPtr, preNode]() {
            if (frameIndex >= frames) {
                buffer->SetIsFinished(true);
                std::vector<uint8_t> zeros(frameSizeInput, 0);
                memcpy_s(buffer->GetPcmData(), frameSizeInput, zeros.data(), frameSizeInput);
                return preNode->PushCurrentFrame(NEED_DATA_LENGTH);
            }

            if (frameIndex == frames - 1) {
                buffer->SetIsFinished(true);
            }

            memcpy_s(buffer->GetPcmData(), frameSizeInput, readPtr, frameSizeInput);

            frameIndex++;
            readPtr += frameSizeInput;
            return preNode->PushCurrentFrame(NEED_DATA_LENGTH);
        }));

    while (loopCount < MAX_FRAMES) {
        std::vector<AudioSuitePcmBuffer *> result = node->PullForTest(outFormat_, false, NEED_DATA_LENGTH);
        CHECK_AND_RETURN_RET(result.size() == 1, ERROR);

        outFile.write(reinterpret_cast<const char *>(result[0]->GetPcmData()), frameSizeInput);

        if (result[0]->GetIsFinished() || ++loopCount >= MAX_FRAMES) {
            break;
        }
    }

    outFile.close();

    node->DisConnect(preNode);

    buffer->SetIsFinished(false);
    node->Flush();
    Mock::VerifyAndClearExpectations(preNode.get());
    preNode.reset();
    return SUCCESS;
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DoProcessTest, TestSize.Level0)
{
    float speed = 0.8f;
    float pitch = 0.8f;
    int ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile001);
    EXPECT_EQ(SUCCESS, ret);
    bool isFileEqual = IsFilesEqual(g_outfile001, g_targetfile001);
    EXPECT_EQ(true, isFileEqual);

    speed = 1.0f;
    pitch = 0.8f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile002);
    EXPECT_EQ(SUCCESS, ret);
    isFileEqual = IsFilesEqual(g_outfile002, g_targetfile002);
    EXPECT_EQ(true, isFileEqual);

    speed = 0.8f;
    pitch = 1.0f;
    ret = DoprocessTest(speed, pitch, g_inputfile001, g_outfile003);
    EXPECT_EQ(SUCCESS, ret);
    isFileEqual = IsFilesEqual(g_outfile003, g_targetfile003);
    EXPECT_EQ(true, isFileEqual);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, InitTest, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteTempoPitchNode>();
    int ret = node->Init();
    EXPECT_EQ(SUCCESS, ret);

    ret = node->Init();
    EXPECT_EQ(ERROR, ret);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, DeInitTest, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteTempoPitchNode>();
    int ret = node->DeInit();
    EXPECT_EQ(ERROR, ret);
}

HWTEST_F(AudioSuiteTempoPitchNodeTest, CalculationNeedBytesTest001, TestSize.Level0)
{
    auto node = std::make_shared<AudioSuiteTempoPitchNode>();
    node->Init();

    int32_t ret = node->CalculationNeedBytes(NEED_DATA_LENGTH);
    EXPECT_EQ(ret, MAX_OUTPUT_BYTE);
}

}  // namespace