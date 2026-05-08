/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_pcm_process.h"
#include "audio_stream_info.h"
#include "audio_suite_log.h"
#include <gtest/gtest.h>
#include <vector>
#include <cstring>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioPcmProcessImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioPcmProcessImplTest::SetUpTestCase(void) {}
void AudioPcmProcessImplTest::TearDownTestCase(void) {}
void AudioPcmProcessImplTest::SetUp(void) {}
void AudioPcmProcessImplTest::TearDown(void) {}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_001, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {96000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);
    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_002, TestSize.Level1)
{
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(nullptr, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_003, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, nullptr);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_004, TestSize.Level1)
{
    AudioFormatConfig inCfg = {99999, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_005, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {99999, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_006, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, static_cast<CAudioChannel>(3), AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_007, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, static_cast<CAudioChannel>(3), AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_008, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, static_cast<AudioBitDepth>(99)};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_009, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, static_cast<AudioBitDepth>(99)};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterProcess_001, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> inputData(1024, 0);
    CAudioConvertResult result = AudioConverterProcess(handle, inputData.data(), 512);
    EXPECT_EQ(result.errCode, 0);

    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterProcess_002, TestSize.Level1)
{
    std::vector<uint8_t> inputData(1024, 0);
    CAudioConvertResult result = AudioConverterProcess(nullptr, inputData.data(), 512);
    EXPECT_NE(result.errCode, 0);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterProcess_003, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);

    CAudioConvertResult result = AudioConverterProcess(handle, nullptr, 512);
    EXPECT_NE(result.errCode, 0);

    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterProcess_004, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> inputData(1024, 0);
    CAudioConvertResult result = AudioConverterProcess(handle, inputData.data(), 0);
    EXPECT_NE(result.errCode, 0);

    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterProcess_005, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> inputData(1024, 0);
    CAudioConvertResult result = AudioConverterProcess(handle, inputData.data(), -1);
    EXPECT_NE(result.errCode, 0);

    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterDestroy_001, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);
    AudioConverterDestroy(handle);
    AudioConverterDestroy(nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_001, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_002, TestSize.Level1)
{
    AudioMixHandle handle = AudioMixerCreate(nullptr);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_003, TestSize.Level1)
{
    AudioFormatConfig cfg = {99999, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_004, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, static_cast<CAudioChannel>(3), AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_005, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, static_cast<AudioBitDepth>(99)};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_EQ(handle, nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_001, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0x10);
    std::vector<uint8_t> data2(1024, 0x20);

    CAudioMixStream streams[2];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;
    streams[1].data = data2.data();
    streams[1].sampleCount = 512;
    streams[1].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 2);
    EXPECT_EQ(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_002, TestSize.Level1)
{
    std::vector<uint8_t> data1(1024, 0);
    CAudioMixStream streams[1];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(nullptr, streams, 1);
    EXPECT_NE(result.errCode, 0);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_003, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    AudioMixResult result = AudioMixerProcess(handle, nullptr, 1);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_004, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0);
    CAudioMixStream streams[1];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 0);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_005, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    CAudioMixStream streams[1];
    streams[0].data = nullptr;
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 1);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_006, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0);
    std::vector<uint8_t> data2(1024, 0);

    CAudioMixStream streams[2];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;
    streams[1].data = data2.data();
    streams[1].sampleCount = 256;
    streams[1].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 2);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_007, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0);

    CAudioMixStream streams[1];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = -0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 1);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_008, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0);

    CAudioMixStream streams[1];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 1.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 1);
    EXPECT_NE(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_009, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0x10);
    CAudioMixStream streams[1];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;

    AudioMixResult result1 = AudioMixerProcess(handle, streams, 1);
    EXPECT_EQ(result1.errCode, 0);

    streams[0].sampleCount = 256;
    AudioMixResult result2 = AudioMixerProcess(handle, streams, 1);
    EXPECT_NE(result2.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerDestroy_001, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
    AudioMixerDestroy(nullptr);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_010, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_MONO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(512, 0x10);
    std::vector<uint8_t> data2(512, 0x20);

    CAudioMixStream streams[2];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.5f;
    streams[1].data = data2.data();
    streams[1].sampleCount = 512;
    streams[1].volume = 0.5f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 2);
    EXPECT_EQ(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_010, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_MONO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);
    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_011, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT24_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT32_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);
    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_012, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT32_FLOAT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);
    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_006, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_MONO, AUDIO_BIT24_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_007, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT32_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_008, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT32_FLOAT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_009, TestSize.Level1)
{
    AudioFormatConfig cfg = {8000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_010, TestSize.Level1)
{
    AudioFormatConfig cfg = {44100, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerCreate_011, TestSize.Level1)
{
    AudioFormatConfig cfg = {96000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);
    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_011, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0x10);
    std::vector<uint8_t> data2(1024, 0x20);
    std::vector<uint8_t> data3(1024, 0x30);

    CAudioMixStream streams[3];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 0.33f;
    streams[1].data = data2.data();
    streams[1].sampleCount = 512;
    streams[1].volume = 0.33f;
    streams[2].data = data3.data();
    streams[2].sampleCount = 512;
    streams[2].volume = 0.34f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 3);
    EXPECT_EQ(result.errCode, 0);

    AudioMixerDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioConverterCreate_013, TestSize.Level1)
{
    AudioFormatConfig inCfg = {48000, AUDIO_CH_MONO, AUDIO_BIT16_INT};
    AudioFormatConfig outCfg = {48000, AUDIO_CH_MONO, AUDIO_BIT16_INT};
    AudioConverterHandle handle = AudioConverterCreate(&inCfg, &outCfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> inputData(512, 0x55);
    CAudioConvertResult result = AudioConverterProcess(handle, inputData.data(), 256);
    EXPECT_EQ(result.errCode, 0);

    AudioConverterDestroy(handle);
}

HWTEST_F(AudioPcmProcessImplTest, AudioMixerProcess_012, TestSize.Level1)
{
    AudioFormatConfig cfg = {48000, AUDIO_CH_STEREO, AUDIO_BIT16_INT};
    AudioMixHandle handle = AudioMixerCreate(&cfg);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> data1(1024, 0x10);
    std::vector<uint8_t> data2(1024, 0x20);

    CAudioMixStream streams[2];
    streams[0].data = data1.data();
    streams[0].sampleCount = 512;
    streams[0].volume = 1.0f;
    streams[1].data = data2.data();
    streams[1].sampleCount = 512;
    streams[1].volume = 0.0f;

    AudioMixResult result = AudioMixerProcess(handle, streams, 2);
    EXPECT_EQ(result.errCode, 0);

    AudioMixerDestroy(handle);
}

}
}