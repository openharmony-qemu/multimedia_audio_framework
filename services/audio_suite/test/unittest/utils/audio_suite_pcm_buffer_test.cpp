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
#include "audio_suite_pcm_buffer.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {
class AudioSuitePcmBufferTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(AudioSuitePcmBufferTest, AudioSuitePcmBufferConstructor_001, TestSize.Level0)
{
    AudioSuitePcmBuffer* testBuffer = new(std::nothrow)
                            AudioSuitePcmBuffer(PcmBufferFormat(SAMPLE_RATE_48000, 2, CH_LAYOUT_STEREO, SAMPLE_S16LE));
    EXPECT_NE(testBuffer, nullptr);
}

HWTEST_F(AudioSuitePcmBufferTest, AudioSuitePcmBufferInitPcmProcess_001, TestSize.Level0)
{
    AudioSuitePcmBuffer* testBuffer = new(std::nothrow)
                            AudioSuitePcmBuffer(PcmBufferFormat(SAMPLE_RATE_44100, 2, CH_LAYOUT_STEREO, SAMPLE_S16LE));
    EXPECT_NE(testBuffer, nullptr);

    testBuffer->InitPcmProcess();
    EXPECT_EQ(testBuffer->frames_, 1);
}

HWTEST_F(AudioSuitePcmBufferTest, AudioSuitePcmBufferResizePcmBuffer_001, TestSize.Level0)
{
    AudioSuitePcmBuffer* testBuffer = new(std::nothrow)
                            AudioSuitePcmBuffer(PcmBufferFormat(SAMPLE_RATE_44100, 2, CH_LAYOUT_STEREO, SAMPLE_S16LE));
    EXPECT_NE(testBuffer, nullptr);

    testBuffer->ResizePcmBuffer(PcmBufferFormat(SAMPLE_RATE_48000, 2, CH_LAYOUT_STEREO, SAMPLE_S16LE));
    EXPECT_EQ(testBuffer->frames_, 1);
}
}
