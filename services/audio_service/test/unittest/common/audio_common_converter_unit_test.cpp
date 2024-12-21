/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_common_converter.h"
#include "audio_common_converter.cpp"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
AudioCommonConverter audioCommonConverter;
std::vector<float> floatBuffer;
BufferBaseInfo bufferInfo;
std::vector<char> dstBuffer;

class AudioCommonConverterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number: ConvertBufferTo32Bit_001
 * @tc.desc  : Test ConvertBufferTo32Bit with AUDIO_SAMPLE_FORMAT_8BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo32Bit_001, TestSize.Level1)
{
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_8BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int32_t));
    audioCommonConverter.ConvertBufferTo32Bit(bufferInfo, dstBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);
    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int32_t sampleValue = static_cast<int32_t>(((static_cast<int32_t>(bufferInfo.buffer[i]) - 0x80) <<
            AUDIO_SAMPLE_24BIT_LENGTH) * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number: ConvertBufferTo32Bit_002
 * @tc.desc  : Test ConvertBufferTo32Bit with AUDIO_SAMPLE_FORMAT_16BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo32Bit_002, TestSize.Level1)
{
    std::unique_ptr<int16_t[]> buffer = std::make_unique<int16_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int16_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_16BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int32_t));

    audioCommonConverter.ConvertBufferTo32Bit(bufferInfo, dstBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);
    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int32_t sample = static_cast<int32_t>((static_cast<int32_t>(buffer[i]) << AUDIO_SAMPLE_16BIT_LENGTH) * vol);
        EXPECT_EQ(dst[i], sample);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number: ConvertBufferTo32Bit_003
 * @tc.desc  : Test ConvertBufferTo32Bit with AUDIO_SAMPLE_FORMAT_24BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo32Bit_003, TestSize.Level1)
{
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(10 * 3);
    for (size_t i = 0; i < 10 * 3; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_24BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int32_t));

    audioCommonConverter.ConvertBufferTo32Bit(bufferInfo, dstBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);
    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int32_t sample = static_cast<int32_t>(((static_cast<int32_t>(buffer[i * 3 + 2]) << AUDIO_SAMPLE_24BIT_LENGTH) |
        (static_cast<int32_t>(buffer[i * 3 + 1]) << AUDIO_SAMPLE_16BIT_LENGTH) |
        (static_cast<int32_t>(buffer[i * 3]) << BYTES_ALIGNMENT_SIZE)) * vol);
        EXPECT_EQ(dst[i], sample);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number: ConvertBufferTo32Bit_004
 * @tc.desc  : Test ConvertBufferTo32Bit with AUDIO_SAMPLE_FORMAT_32BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo32Bit_004, TestSize.Level1)
{
    std::unique_ptr<int32_t[]> buffer = std::make_unique<int32_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int32_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_32BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int32_t));

    audioCommonConverter.ConvertBufferTo32Bit(bufferInfo, dstBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int32_t sampleValue = static_cast<int32_t>(buffer[i] * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number: ConvertBufferTo32Bit_005
 * @tc.desc  : Test ConvertBufferTo32Bit with AUDIO_SAMPLE_FORMAT_32F_BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo32Bit_005, TestSize.Level1)
{
    std::unique_ptr<float[]> buffer = std::make_unique<float[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<float>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_32F_BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int32_t));

    audioCommonConverter.ConvertBufferTo32Bit(bufferInfo, dstBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);
    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int32_t sampleValue = static_cast<int32_t>(buffer[i] * AUDIO_SAMPLE_32BIT_VALUE * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number:  ConvertBufferTo16Bit_001
 * @tc.desc  : Test ConvertBufferTo16Bit with AUDIO_SAMPLE_FORMAT_8BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo16Bit_001, TestSize.Level1)
{
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_8BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int16_t));

    audioCommonConverter.ConvertBufferTo16Bit(bufferInfo, dstBuffer);
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int16_t sampleValue = static_cast<int16_t>(((static_cast<int16_t>(bufferInfo.buffer[i]) - 0x80) <<
            BYTES_ALIGNMENT_SIZE) * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number:  ConvertBufferTo16Bit_002
 * @tc.desc  : Test ConvertBufferTo16Bit with AUDIO_SAMPLE_FORMAT_16BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo16Bit_002, TestSize.Level1)
{
    std::unique_ptr<int16_t[]> buffer = std::make_unique<int16_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int16_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_16BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int16_t));

    audioCommonConverter.ConvertBufferTo16Bit(bufferInfo, dstBuffer);
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int16_t sampleValue = buffer[i] * vol;
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number:  ConvertBufferTo16Bit_003
 * @tc.desc  : Test ConvertBufferTo16Bit with AUDIO_SAMPLE_FORMAT_24BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo16Bit_003, TestSize.Level1)
{
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(10 * 3);
    for (size_t i = 0; i < 10 * 3; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_24BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int16_t));

    audioCommonConverter.ConvertBufferTo16Bit(bufferInfo, dstBuffer);
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int16_t sampleValue = static_cast<int16_t>(((static_cast<int16_t>(buffer[i * 3 + 2]) << BYTES_ALIGNMENT_SIZE) |
            (static_cast<int16_t>(buffer[i * 3 + 1])))* vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number:  ConvertBufferTo16Bit_004
 * @tc.desc  : Test ConvertBufferTo16Bit with AUDIO_SAMPLE_FORMAT_32BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo16Bit_004, TestSize.Level1)
{
    std::unique_ptr<int32_t[]> buffer = std::make_unique<int32_t[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int32_t>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_32BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int16_t));

    audioCommonConverter.ConvertBufferTo16Bit(bufferInfo, dstBuffer);
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int16_t sampleValue = static_cast<int16_t>((buffer[i] >> AUDIO_SAMPLE_16BIT_LENGTH) * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}

/**
 * @tc.name  : Test AudioCommonConverter API
 * @tc.type  : FUNC
 * @tc.number:  ConvertBufferTo16Bit_005
 * @tc.desc  : Test ConvertBufferTo16Bit with AUDIO_SAMPLE_FORMAT_32BIT
 */
HWTEST(AudioCommonConverterUnitTest, ConvertBufferTo16Bit_005, TestSize.Level1)
{
    std::unique_ptr<float[]> buffer = std::make_unique<float[]>(10);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<float>(i);
    }
    bufferInfo.buffer = reinterpret_cast<uint8_t *>(buffer.get());
    bufferInfo.format = AUDIO_SAMPLE_FORMAT_32F_BIT;
    bufferInfo.frameSize = 10;
    bufferInfo.channelCount = 1;
    bufferInfo.volumeBg = 0.0f;
    bufferInfo.volumeEd = 1.0f;
    dstBuffer.resize(10 * sizeof(int16_t));

    audioCommonConverter.ConvertBufferTo16Bit(bufferInfo, dstBuffer);
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    float volumeStep = GetVolumeStep(bufferInfo);

    for (size_t i = 0; i < 10; i++) {
        float vol = GetVolume(volumeStep, i + 1, bufferInfo.volumeBg);
        int16_t sampleValue = static_cast<int16_t>(buffer[i]  * SCALE * vol);
        EXPECT_EQ(dst[i], sampleValue);
    }
}
} // namespace AudioStandard
} // namespace OHOS
