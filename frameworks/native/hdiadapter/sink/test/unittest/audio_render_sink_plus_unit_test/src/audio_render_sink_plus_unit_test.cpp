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

#include "audio_render_sink_plus_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static int32_t NUM_1 = 1;
static int32_t NUM_2 = 2;
static int32_t NUM_3 = 3;
static int32_t NUM_4 = 4;
static int32_t NUM_5 = 5;

void AudioRenderSinkPlusUnitTest::SetUpTestCase(void) {}
void AudioRenderSinkPlusUnitTest::TearDownTestCase(void) {}
void AudioRenderSinkPlusUnitTest::SetUp(void) {}
void AudioRenderSinkPlusUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioRendererSink
 * @tc.number: AudioRenderSinkPlusUnitTest_001
 * @tc.desc  : Test ConvertByteToAudioFormat()
 */
HWTEST(AudioRenderSinkPlusUnitTest, AudioRenderSinkPlusUnitTest_001, TestSize.Level1)
{
    int32_t format = NUM_1;
    auto ret = ConvertByteToAudioFormat(format);

    EXPECT_EQ(ret, SAMPLE_U8);
}

/**
 * @tc.name  : Test AudioRendererSink
 * @tc.number: AudioRenderSinkPlusUnitTest_002
 * @tc.desc  : Test ConvertByteToAudioFormat()
 */
HWTEST(AudioRenderSinkPlusUnitTest, AudioRenderSinkPlusUnitTest_002, TestSize.Level1)
{
    int32_t format = NUM_2;
    auto ret = ConvertByteToAudioFormat(format);

    EXPECT_EQ(ret, SAMPLE_S16LE);
}

/**
 * @tc.name  : Test AudioRendererSink
 * @tc.number: AudioRenderSinkPlusUnitTest_003
 * @tc.desc  : Test ConvertByteToAudioFormat()
 */
HWTEST(AudioRenderSinkPlusUnitTest, AudioRenderSinkPlusUnitTest_003, TestSize.Level1)
{
    int32_t format = NUM_3;
    auto ret = ConvertByteToAudioFormat(format);

    EXPECT_EQ(ret, SAMPLE_S24LE);
}

/**
 * @tc.name  : Test AudioRendererSink
 * @tc.number: AudioRenderSinkPlusUnitTest_004
 * @tc.desc  : Test ConvertByteToAudioFormat()
 */
HWTEST(AudioRenderSinkPlusUnitTest, AudioRenderSinkPlusUnitTest_004, TestSize.Level1)
{
    int32_t format = NUM_4;
    auto ret = ConvertByteToAudioFormat(format);

    EXPECT_EQ(ret, SAMPLE_S32LE);
}

/**
 * @tc.name  : Test AudioRendererSink
 * @tc.number: AudioRenderSinkPlusUnitTest_005
 * @tc.desc  : Test ConvertByteToAudioFormat()
 */
HWTEST(AudioRenderSinkPlusUnitTest, AudioRenderSinkPlusUnitTest_005, TestSize.Level1)
{
    int32_t format = NUM_5;
    auto ret = ConvertByteToAudioFormat(format);

    EXPECT_EQ(ret, SAMPLE_S16LE);
}
} // namespace AudioStandard
} // namespace OHOS