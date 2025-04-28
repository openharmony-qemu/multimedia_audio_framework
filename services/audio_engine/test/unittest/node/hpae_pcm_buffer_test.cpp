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

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;

class HpaePcmBufferTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaePcmBufferTest::SetUp()
{}

void HpaePcmBufferTest::TearDown()
{}

namespace {

constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
constexpr uint32_t DEFAULT_FRAME_LEN = 480;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_FRAME_NUM = 2;


TEST_F(HpaePcmBufferTest, constructHpaePcmBufferTest)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(hpaePcmBuffer.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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

TEST_F(HpaePcmBufferTest, assignHpaePcmBufferTest)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(hpaePcmBuffer.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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

TEST_F(HpaePcmBufferTest, calHpaePcmBufferTest)
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = DEFAULT_FRAME_NUM;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(hpaePcmBuffer.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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
    EXPECT_EQ(reinterpret_cast<uintptr_t>(hpaePcmBuffer2.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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

TEST_F(HpaePcmBufferTest, calHpaePcmBufferMultiFrameTest)
{
    PcmBufferInfo pcmBufferInfo;
    size_t inputFrames = 4;
    pcmBufferInfo.ch = DEFAULT_CHANNEL_COUNT;
    pcmBufferInfo.frameLen = DEFAULT_FRAME_LEN;
    pcmBufferInfo.rate = DEFAULT_SAMPLE_RATE;
    pcmBufferInfo.frames = inputFrames;
    pcmBufferInfo.isMultiFrames = true;
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(hpaePcmBuffer.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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
    EXPECT_EQ(reinterpret_cast<uintptr_t>(testHpaePcmBuffer.GetPcmDataBuffer()) % MEMORY_ALIGN_BYTE_NUM, 0);
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
}