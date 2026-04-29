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
#include <thread>
#include <vector>
#include <mutex>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_pitch_processor.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioPitchProcessorUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioPitchProcessorUnitTest::SetUpTestCase(void)
{
}

void AudioPitchProcessorUnitTest::TearDownTestCase(void)
{
}

void AudioPitchProcessorUnitTest::SetUp()
{
}

void AudioPitchProcessorUnitTest::TearDown()
{
}

static AudioStreamInfo CreateTestStreamInfo(uint32_t sampleRate = 48000, uint32_t channels = 2)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = static_cast<AudioSamplingRate>(sampleRate);
    streamInfo.channels = static_cast<AudioChannel>(channels);
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    streamInfo.channelLayout = static_cast<AudioChannelLayout>(0);
    return streamInfo;
}

HWTEST(AudioPitchProcessorUnitTest, CreateInstance_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo();
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
}

HWTEST(AudioPitchProcessorUnitTest, PitchAlgoInit_SingleInstance_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, PitchAlgoInit_MultiInstance_001, TestSize.Level2)
{
    AudioStreamInfo streamInfo1 = CreateTestStreamInfo(48000, 2);
    AudioStreamInfo streamInfo2 = CreateTestStreamInfo(48000, 2);
    
    auto processor1 = AudioPitchProcessor::CreateInstance(streamInfo1);
    auto processor2 = AudioPitchProcessor::CreateInstance(streamInfo2);
    EXPECT_NE(processor1, nullptr);
    EXPECT_NE(processor2, nullptr);
    
    int32_t ret1 = processor1->PitchAlgoInit();
    EXPECT_EQ(ret1, SUCCESS);
    
    int32_t ret2 = processor2->PitchAlgoInit();
    EXPECT_EQ(ret2, SUCCESS);
    
    processor1->PitchAlgoRelease();
    processor2->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, PitchAlgoRelease_Order_001, TestSize.Level2)
{
    AudioStreamInfo streamInfo1 = CreateTestStreamInfo(48000, 2);
    AudioStreamInfo streamInfo2 = CreateTestStreamInfo(48000, 2);
    
    auto processor1 = AudioPitchProcessor::CreateInstance(streamInfo1);
    auto processor2 = AudioPitchProcessor::CreateInstance(streamInfo2);
    EXPECT_NE(processor1, nullptr);
    EXPECT_NE(processor2, nullptr);
    
    int32_t ret1 = processor1->PitchAlgoInit();
    EXPECT_EQ(ret1, SUCCESS);
    
    int32_t ret2 = processor2->PitchAlgoInit();
    EXPECT_EQ(ret2, SUCCESS);
    
    processor1->PitchAlgoRelease();
    processor2->PitchAlgoRelease();
    
    processor1->PitchAlgoRelease();
    processor2->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, SetPitch_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    ret = processor->SetPitch(1.0f);
    EXPECT_EQ(ret, SUCCESS);
    
    ret = processor->SetPitch(0.5f);
    EXPECT_EQ(ret, SUCCESS);
    
    ret = processor->SetPitch(2.0f);
    EXPECT_EQ(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, SetPitch_InvalidRange_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    ret = processor->SetPitch(0.1f);
    EXPECT_NE(ret, SUCCESS);
    
    ret = processor->SetPitch(5.0f);
    EXPECT_NE(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, UnsupportedChannel_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 4);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST(AudioPitchProcessorUnitTest, MultiThread_Create_001, TestSize.Level3)
{
    const int32_t threadCount = 5;
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<AudioPitchProcessor>> processors(threadCount);
    std::vector<int32_t> results(threadCount, ERR_OPERATION_FAILED);
    std::mutex resultsMutex;
    
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    
    for (int32_t i = 0; i < threadCount; i++) {
        threads.emplace_back([&, i]() {
            auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
            if (processor != nullptr) {
                int32_t ret = processor->PitchAlgoInit();
                {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    processors[i] = processor;
                    results[i] = ret;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    for (int32_t i = 0; i < threadCount; i++) {
        EXPECT_EQ(results[i], SUCCESS);
        if (processors[i] != nullptr) {
            processors[i]->PitchAlgoRelease();
        }
    }
}

HWTEST(AudioPitchProcessorUnitTest, MultiThread_CreateRelease_001, TestSize.Level3)
{
    const int32_t createCount = 3;
    const int32_t releaseCount = 2;
    std::vector<std::shared_ptr<AudioPitchProcessor>> processors(createCount);
    std::vector<int32_t> initResults(createCount, ERR_OPERATION_FAILED);
    std::mutex processorsMutex;
    
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    
    std::vector<std::thread> createThreads;
    for (int32_t i = 0; i < createCount; i++) {
        createThreads.emplace_back([&, i]() {
            auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
            if (processor != nullptr) {
                int32_t ret = processor->PitchAlgoInit();
                {
                    std::lock_guard<std::mutex> lock(processorsMutex);
                    processors[i] = processor;
                    initResults[i] = ret;
                }
            }
        });
    }
    
    for (auto& thread : createThreads) {
        thread.join();
    }
    
    for (int32_t i = 0; i < createCount; i++) {
        EXPECT_EQ(initResults[i], SUCCESS);
    }
    
    std::vector<std::thread> releaseThreads;
    for (int32_t i = 0; i < releaseCount; i++) {
        releaseThreads.emplace_back([&, i]() {
            if (processors[i] != nullptr) {
                processors[i]->PitchAlgoRelease();
            }
        });
    }
    
    for (auto& thread : releaseThreads) {
        thread.join();
    }
    
    for (int32_t i = releaseCount; i < createCount; i++) {
        if (processors[i] != nullptr) {
            processors[i]->PitchAlgoRelease();
        }
    }
}

HWTEST(AudioPitchProcessorUnitTest, ReinitAfterRelease_001, TestSize.Level2)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
    
    ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, DoubleRelease_001, TestSize.Level2)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    processor->PitchAlgoRelease();
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, ProcessBufferPitch_S16LE_001, TestSize.Level2)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    ret = processor->SetPitch(1.5f);
    EXPECT_EQ(ret, SUCCESS);
    
    const size_t inputSize = 1024;
    std::vector<int8_t> inputBuffer(inputSize);
    std::vector<int8_t> outputBuffer(inputSize * 3);
    size_t outputSize = 0;
    
    for (size_t i = 0; i < inputSize; i++) {
        inputBuffer[i] = static_cast<int8_t>(i);
    }
    
    ret = processor->ProcessBufferPitch(inputBuffer.data(), inputSize,
        outputBuffer.data(), outputSize);
    EXPECT_EQ(ret, SUCCESS);
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, ProcessBufferPitch_NullBuffer_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, SUCCESS);
    
    size_t outputSize = 0;
    ret = processor->ProcessBufferPitch(nullptr, 1024, nullptr, outputSize);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    
    processor->PitchAlgoRelease();
}

HWTEST(AudioPitchProcessorUnitTest, SetPitchWithoutInit_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
    
    int32_t ret = processor->SetPitch(1.5f);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

} // namespace AudioStandard
} // namespace OHOS