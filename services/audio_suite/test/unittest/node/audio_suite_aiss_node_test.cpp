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
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include "securec.h"
#include "audio_suite_aiss_node.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_algo_interface.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;

class MockSuiteNodeReadTapDataCallback : public SuiteNodeReadTapDataCallback {
    MOCK_METHOD(void, OnReadTapDataCallback, (void*, int32_t), (override));
};

class AudioSuiteAissNodeTest : public testing::Test {
public:
    void SetUp()
    {
        impl = std::make_shared<AudioSuiteAissNode>();
    };
    void TearDown()
    {
        impl = nullptr;
    };
    std::shared_ptr<AudioSuiteAissNode> impl = nullptr;
};

namespace {
    const std::string INPUT_PATH = "/data/aiss_48000_2_S32LE.pcm";
    constexpr uint32_t FRAME_LEN_MS = 20;
    constexpr uint32_t DEFAULT_SAMPLING_RATE = 48000;
    constexpr uint32_t DEFAULT_CHANNELS_IN = 2;
    constexpr uint32_t BYTES_PER_SAMPLE = 4;
    const AudioChannelLayout LAY_OUT = CH_LAYOUT_STEREO;
     
    HWTEST_F(AudioSuiteAissNodeTest, ProcessTest, TestSize.Level0)
    {
        EXPECT_NE(impl->DoProcess(), SUCCESS);
        std::ifstream inputFile(INPUT_PATH, std::ios::binary | std::ios::ate);
        ASSERT_TRUE(inputFile.is_open());
        AudioSuitePcmBuffer inputBuffer(PcmBufferFormat(SAMPLE_RATE_48000, DEFAULT_CHANNELS_IN, LAY_OUT, SAMPLE_F32LE));
        const uint32_t byteSizePerFrameIn = DEFAULT_SAMPLING_RATE * FRAME_LEN_MS /
            1000 * DEFAULT_CHANNELS_IN * BYTES_PER_SAMPLE;
        inputFile.read(reinterpret_cast<char *>(inputBuffer.GetPcmData()), byteSizePerFrameIn);
        ASSERT_FALSE(inputFile.fail() && !inputFile.eof());
        std::vector<AudioSuitePcmBuffer*> inputs;
        inputs.emplace_back(&inputBuffer);
        AudioSuitePcmBuffer *outputBuffer = impl->SignalProcess(inputs);
        EXPECT_NE(outputBuffer, nullptr);
        inputFile.close();
    }

    HWTEST_F(AudioSuiteAissNodeTest, DoProcessTest001, TestSize.Level0)
    {
        std::shared_ptr<AudioSuiteAissNode> impl = std::make_shared<AudioSuiteAissNode>();
        int32_t ret = impl->DoProcess();
        EXPECT_EQ(ret, ERROR);
    }
 
    HWTEST_F(AudioSuiteAissNodeTest, DoProcessTest002, TestSize.Level0)
    {
        std::shared_ptr<AudioSuiteAissNode> impl = std::make_shared<AudioSuiteAissNode>();
        int32_t ret = impl->DeInit();
        EXPECT_EQ(ret, SUCCESS);
    }
}