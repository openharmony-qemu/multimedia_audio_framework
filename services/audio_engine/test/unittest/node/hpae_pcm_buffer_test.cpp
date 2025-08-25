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
#include <cmath>
#include "hpae_pcm_buffer.h"
#include "test_case_common.h"
#include <vector>

namespace {
constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
constexpr uint32_t DEFAULT_FRAME_LEN = 480;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_FRAME_NUM = 2;
constexpr uint32_t DEFAULT_FRAME_SIZE = DEFAULT_CHANNEL_COUNT * DEFAULT_FRAME_LEN;
constexpr uint32_t NUM_TWO = 2;
constexpr uint32_t NUM_THREE = 3;

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaePcmBufferTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

    PcmBufferInfo CreateBufferInfo(uint32_t frames, bool multiFrames = false)
    {
        PcmBufferInfo info;
        info.ch = DEFAULT_CHANNEL_COUNT;
        info.frameLen = DEFAULT_FRAME_LEN;
        info.rate = DEFAULT_SAMPLE_RATE;
        info.frames = frames;
        info.isMultiFrames = multiFrames;
        return info;
    }
    
    std::vector<float> CreateTestVector(float value = 1.0f)
    {
        return std::vector<float>(DEFAULT_FRAME_SIZE, value);
    }
    
    std::vector<std::vector<float>> CreateTestMatrix(float value = 1.0f, size_t frames = 1)
    {
        std::vector<std::vector<float>> matrix;
        for (size_t i = 0; i < frames; i++) {
            matrix.push_back(CreateTestVector(value));
        }
        return matrix;
    }
};

void HpaePcmBufferTest::SetUp()
{}

void HpaePcmBufferTest::TearDown()
{}

HWTEST_F(HpaePcmBufferTest, constructHpaePcmBufferTest, TestSize.Level0)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(hpaePcmBuffer.GetChannelCount(), pcmBufferInfo.ch);
    EXPECT_EQ(hpaePcmBuffer.GetFrameLen(), pcmBufferInfo.frameLen);
    EXPECT_EQ(hpaePcmBuffer.GetSampleRate(), pcmBufferInfo.rate);
    EXPECT_EQ(hpaePcmBuffer.GetFrames(), pcmBufferInfo.frames);
    EXPECT_EQ(hpaePcmBuffer.GetReadPos(), 0);
    EXPECT_EQ(hpaePcmBuffer.GetWritePos(), 0);
    EXPECT_EQ(hpaePcmBuffer.GetCurFrames(), 0);
    EXPECT_EQ(hpaePcmBuffer.IsMultiFrames(), false);
    size_t addBytes = MEMORY_ALIGN_BYTE_NUM -
        (pcmBufferInfo.frameLen * sizeof(float) * pcmBufferInfo.ch) % MEMORY_ALIGN_BYTE_NUM;
    size_t frameByteSize = pcmBufferInfo.frameLen * sizeof(float) * pcmBufferInfo.ch + addBytes;
    size_t bufferSize = frameByteSize * pcmBufferInfo.frames;
    EXPECT_EQ(hpaePcmBuffer.Size(), bufferSize);
}

HWTEST_F(HpaePcmBufferTest, assignHpaePcmBufferTest, TestSize.Level0)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    size_t tempFrameLen = pcmBufferInfo.frameLen * pcmBufferInfo.ch;
    std::vector<std::vector<float>> testVec;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        testVec.push_back(std::vector<float>(tempFrameLen, 3.14f));
    }
    hpaePcmBuffer = testVec;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        for (size_t j = 0; j < tempFrameLen; j++) {
            EXPECT_EQ(fabs(hpaePcmBuffer[i][j] - 3.14f) < TEST_VALUE_PRESION, true);
        }
    }
}

HWTEST_F(HpaePcmBufferTest, calHpaePcmBufferTest, TestSize.Level0)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    size_t tempFrameLen = pcmBufferInfo.frameLen * pcmBufferInfo.ch;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        for (size_t j = 0; j < tempFrameLen; j++) {
            hpaePcmBuffer[i][j] = 3.14f;
        }
    }
    std::vector<std::vector<float>> testVec;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        testVec.push_back(std::vector<float>(tempFrameLen, 3.14f));
    }
    HpaePcmBuffer hpaePcmBuffer2(pcmBufferInfo);
    hpaePcmBuffer2 = testVec;
    hpaePcmBuffer += hpaePcmBuffer2;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        for (size_t j = 0; j < tempFrameLen; j++) {
            EXPECT_EQ(fabs(hpaePcmBuffer[i][j] - 6.28f) < TEST_VALUE_PRESION, true);
        }
    }
    hpaePcmBuffer -= hpaePcmBuffer2;
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        for (size_t j = 0; j < tempFrameLen; j++) {
            EXPECT_EQ(fabs(hpaePcmBuffer[i][j] - 3.14f) < TEST_VALUE_PRESION, true);
        }
    }
    hpaePcmBuffer.Reset();
    for (size_t i = 0; i < pcmBufferInfo.frames; i++) {
        for (size_t j = 0; j < tempFrameLen; j++) {
            EXPECT_EQ(fabs(hpaePcmBuffer[i][j] - 0.0f) < TEST_VALUE_PRESION, true);
        }
    }
}

HWTEST_F(HpaePcmBufferTest, calHpaePcmBufferMultiFrameTest, TestSize.Level0)
{
    PcmBufferInfo pcmBufferInfo;
    size_t inputFrames = 4; // 4: input frame numbers
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = inputFrames;
    pcmBufferInfo.isMultiFrames = true;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(hpaePcmBuffer.IsMultiFrames(), true);
    
    size_t tempFrameLen = pcmBufferInfo.frameLen * pcmBufferInfo.ch;
    EXPECT_EQ(hpaePcmBuffer.GetFrameSample(), tempFrameLen);
    for (size_t i = 0; i < inputFrames; i++) {
        std::vector<float> testVec(tempFrameLen, 3.14f);
        hpaePcmBuffer = testVec;
    }
    std::cout << "tempFrameLen is: " << tempFrameLen << std::endl;
   
    EXPECT_EQ(hpaePcmBuffer.GetCurFrames(), inputFrames);
    EXPECT_EQ(hpaePcmBuffer.GetWritePos(), 0);

    std::vector<float> testVec2(tempFrameLen, 0.0f);
    EXPECT_EQ(hpaePcmBuffer.GetFrameSample(), tempFrameLen);
    EXPECT_EQ(hpaePcmBuffer.PushFrameData(testVec2), false);
    pcmBufferInfo.frames = 1;
    pcmBufferInfo.isMultiFrames = false;
    HpaePcmBuffer testHpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(hpaePcmBuffer.PushFrameData(testHpaePcmBuffer), false);
    size_t curFrames = inputFrames;
    std::cout << "inputFrames is: " << inputFrames << std::endl;
    for (size_t i = 0; i < inputFrames; i++) {
        EXPECT_EQ(hpaePcmBuffer.GetFrameData(testHpaePcmBuffer), true);
        curFrames--;
        EXPECT_EQ(hpaePcmBuffer.GetCurFrames(), curFrames);
        for (size_t j = 0; j < tempFrameLen; j++) {
            EXPECT_EQ(testHpaePcmBuffer[0][j], 3.14f);
        }
    }
    EXPECT_EQ(hpaePcmBuffer.GetCurFrames(), 0);
    EXPECT_EQ(hpaePcmBuffer.GetReadPos(), 0);
    EXPECT_EQ(hpaePcmBuffer.GetFrameData(testHpaePcmBuffer), false);
    EXPECT_EQ(hpaePcmBuffer.GetFrameData(testVec2), false);
}

HWTEST_F(HpaePcmBufferTest, ConstructorInitialization, TestSize.Level0)
{
    PcmBufferInfo info = CreateBufferInfo(NUM_TWO, true);
    HpaePcmBuffer buffer(info);

    EXPECT_EQ(buffer.GetChannelCount(), DEFAULT_CHANNEL_COUNT);
    EXPECT_EQ(buffer.GetFrameLen(), DEFAULT_FRAME_LEN);
    EXPECT_EQ(buffer.GetFrames(), NUM_TWO);
    EXPECT_EQ(buffer.GetCurFrames(), 0);
    EXPECT_EQ(buffer.GetReadPos(), 0);
    EXPECT_EQ(buffer.GetWritePos(), 0);
    EXPECT_TRUE(buffer.IsMultiFrames());
}

HWTEST_F(HpaePcmBufferTest, arithmeticOperations, TestSize.Level0)
{
    PcmBufferInfo info = CreateBufferInfo(1);
    HpaePcmBuffer bufferA(info);
    HpaePcmBuffer bufferB(info);
    bufferA = CreateTestVector(2.0f);
    bufferB = CreateTestVector(3.0f);
    bufferA += bufferB;
    for (size_t i = 0; i < info.frameLen; ++i) {
        EXPECT_FLOAT_EQ(bufferA[0][i], 2.0f + 3.0f);
    }
    bufferA -= bufferB;
    for (size_t i = 0; i < info.frameLen; ++i) {
        EXPECT_FLOAT_EQ(bufferA[0][i], 2.0f);
    }
    bufferA *= bufferB;
    for (size_t i = 0; i < info.frameLen; ++i) {
        EXPECT_FLOAT_EQ(bufferA[0][i], 2.0f * 3.0f);
    }
}

HWTEST_F(HpaePcmBufferTest, bufferManagement, TestSize.Level0)
{
    PcmBufferInfo info = CreateBufferInfo(NUM_THREE, true);
    HpaePcmBuffer buffer(info);

    buffer = CreateTestMatrix(1.0f, NUM_TWO);
    ASSERT_EQ(buffer.GetCurFrames(), NUM_TWO);

    size_t rewound = buffer.RewindBuffer(NUM_TWO);
    EXPECT_EQ(rewound, 1);
    EXPECT_EQ(buffer.GetCurFrames(), NUM_THREE);
    EXPECT_EQ(buffer.GetReadPos(), NUM_TWO); // (0 - 1 + 3) % 3 = 2

    buffer.UpdateReadPos(1);
    buffer.UpdateWritePos(1);
    EXPECT_EQ(buffer.GetReadPos(), 1);
    EXPECT_EQ(buffer.GetWritePos(), 1);

    buffer.Reset();
    for (size_t i = 0; i < buffer.GetFrames(); i++) {
        for (size_t j = 0; j < DEFAULT_FRAME_SIZE; j++) {
            EXPECT_FLOAT_EQ(buffer[i][j], 0.0f);
        }
    }
}

HWTEST_F(HpaePcmBufferTest, edgeCases, TestSize.Level0)
{
    PcmBufferInfo multiInfo = CreateBufferInfo(NUM_TWO, true);
    HpaePcmBuffer multiBuffer(multiInfo);
    multiBuffer = std::vector<std::vector<float>>();
    EXPECT_EQ(multiBuffer.GetCurFrames(), 0);

    std::vector<float> outputVector(DEFAULT_FRAME_SIZE, 1.0f);
    EXPECT_FALSE(multiBuffer.GetFrameData(outputVector));
    for (float sample : outputVector) {
        EXPECT_FLOAT_EQ(sample, 1.0f);
    }

    std::vector<float> testFrame = CreateTestVector();
    for (int i = 0; i < NUM_TWO; i++) {
        EXPECT_TRUE(multiBuffer.PushFrameData(testFrame));
    }
    EXPECT_FALSE(multiBuffer.PushFrameData(testFrame));

    size_t rewound = multiBuffer.RewindBuffer(NUM_THREE);
    EXPECT_EQ(rewound, 0);
}

HWTEST_F(HpaePcmBufferTest, invalidArguments, TestSize.Level0)
{
    PcmBufferInfo multiInfo = CreateBufferInfo(NUM_TWO, true);
    HpaePcmBuffer multiBuffer(multiInfo);
    
    PcmBufferInfo singleInfo = CreateBufferInfo(1);
    HpaePcmBuffer singleBuffer(singleInfo);

    EXPECT_FALSE(multiBuffer.GetFrameData(multiBuffer));
    EXPECT_FALSE(singleBuffer.GetFrameData(multiBuffer));

    std::vector<float> testFrame = CreateTestVector();
    EXPECT_FALSE(singleBuffer.PushFrameData(testFrame));

    EXPECT_FALSE(multiBuffer.StoreFrameData(multiBuffer));
    EXPECT_FALSE(singleBuffer.StoreFrameData(multiBuffer));

    EXPECT_EQ(singleBuffer.RewindBuffer(1), 0);
}

HWTEST_F(HpaePcmBufferTest, positionWrapping, TestSize.Level0)
{
    PcmBufferInfo info = CreateBufferInfo(NUM_TWO, true);
    HpaePcmBuffer buffer(info);

    buffer.UpdateWritePos(1);
    buffer.UpdateReadPos(1);

    std::vector<float> testFrame = CreateTestVector();
    EXPECT_TRUE(buffer.PushFrameData(testFrame));

    EXPECT_EQ(buffer.GetWritePos(), 0);
    EXPECT_EQ(buffer.GetReadPos(), 1);
    EXPECT_EQ(buffer.GetCurFrames(), 1);

    PcmBufferInfo outputInfo = CreateBufferInfo(1);
    HpaePcmBuffer outputBuffer(outputInfo);
    EXPECT_TRUE(buffer.StoreFrameData(outputBuffer));

    EXPECT_EQ(buffer.GetWritePos(), 1);
    EXPECT_EQ(buffer.GetReadPos(), 0); // (1 + 1) % 2 = 0
}
}