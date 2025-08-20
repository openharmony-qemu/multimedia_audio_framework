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

#include "audio_errors.h"
#include "format_converter.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
const size_t BUFFER_LENGTH_FOUR = 4;
const size_t BUFFER_LENGTH_EIGHT = 8;
}
class FormatConverterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormatConverterUnitTest::SetUpTestCase(void)
{
}

void FormatConverterUnitTest::TearDownTestCase(void)
{
}

void FormatConverterUnitTest::SetUp(void)
{
}

void FormatConverterUnitTest::TearDown(void)
{
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16StereoToF32Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16StereoToF32Stereo_001, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[4] = {0};
    uint8_t dstBuffer[8] = {0};

    srcDesc.bufLength = 4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 2;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    dstDesc.bufLength = 8;

    ret = FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16StereoToF32Mono_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16StereoToF32Mono_001, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[4] = {0};
    uint8_t dstBuffer[4] = {0};

    srcDesc.bufLength = 4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 2;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    dstDesc.bufLength = 4;

    ret = FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32MonoToS16Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32MonoToS16Stereo_001, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[4] = {0};
    uint8_t dstBuffer[4] = {0};

    srcDesc.bufLength = 4;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 2;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    dstDesc.bufLength = 4;

    ret = FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32StereoToS16Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Stereo_001, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[8] = {0};
    uint8_t dstBuffer[4] = {0};

    srcDesc.bufLength = 8;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 2;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    dstDesc.bufLength = 4;

    ret = FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16MonoToS16Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16MonoToS16Stereo_001, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[8] = {0};
    uint8_t dstBuffer[8] = {0};

    srcDesc.bufLength = 2;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 8;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16MonoToS16Stereo_002
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16MonoToS16Stereo_002, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t dstBuffer[8] = {0};

    srcDesc.bufLength = 2;
    srcDesc.buffer = nullptr;
    dstDesc.bufLength = 8;
    dstDesc.buffer = dstBuffer;

    ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16MonoToS16Stereo_003
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16MonoToS16Stereo_003, TestSize.Level1)
{
    BufferDesc srcDesc;
    BufferDesc dstDesc;
    int32_t ret = -1;
    uint8_t srcBuffer[8] = {0};

    srcDesc.bufLength = 2;
    srcDesc.buffer = srcBuffer;
    dstDesc.bufLength = 8;
    dstDesc.buffer = nullptr;

    ret = FormatConverter::S16MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32StereoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::S16StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);

    ret = FormatConverter::F32MonoToS16Stereo(srcDesc, dstDesc);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: DataAccumulationFromVolume_001
 * @tc.desc  : Test FormatConverter interface: format not equal
 */
HWTEST_F(FormatConverterUnitTest, DataAccumulationFromVolume_001, TestSize.Level0)
{
    uint8_t srcBuffer[BUFFER_LENGTH_EIGHT] = {0};
    BufferDesc srcDesc = {srcBuffer, BUFFER_LENGTH_EIGHT, BUFFER_LENGTH_EIGHT};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};

    uint8_t dstBuffer[BUFFER_LENGTH_FOUR] = {0};
    BufferDesc dstDesc = {dstBuffer, BUFFER_LENGTH_FOUR, BUFFER_LENGTH_FOUR};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    dstData.bufferDesc = dstDesc;

    bool ret = FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: DataAccumulationFromVolume_002
 * @tc.desc  : Test FormatConverter interface: mix s32
 */
HWTEST_F(FormatConverterUnitTest, DataAccumulationFromVolume_002, TestSize.Level0)
{
    uint8_t srcBuffer[BUFFER_LENGTH_EIGHT] = {0};
    BufferDesc srcDesc = {srcBuffer, BUFFER_LENGTH_EIGHT, BUFFER_LENGTH_EIGHT};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};

    uint8_t dstBuffer[BUFFER_LENGTH_EIGHT] = {0};
    BufferDesc dstDesc = {dstBuffer, BUFFER_LENGTH_EIGHT, BUFFER_LENGTH_EIGHT};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, STEREO};
    dstData.bufferDesc = dstDesc;

    bool ret = FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: DataAccumulationFromVolume_003
 * @tc.desc  : Test FormatConverter interface: mix s32
 */
HWTEST_F(FormatConverterUnitTest, DataAccumulationFromVolume_003, TestSize.Level0)
{
    uint8_t srcBuffer[BUFFER_LENGTH_FOUR] = {0};
    BufferDesc srcDesc = {srcBuffer, BUFFER_LENGTH_FOUR, BUFFER_LENGTH_FOUR};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};

    uint8_t dstBuffer[BUFFER_LENGTH_FOUR] = {0};
    BufferDesc dstDesc = {dstBuffer, BUFFER_LENGTH_FOUR, BUFFER_LENGTH_FOUR};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    dstData.bufferDesc = dstDesc;

    bool ret = FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: DataAccumulationFromVolume_004
 * @tc.desc  : Test FormatConverter interface: not support f32
 */
HWTEST_F(FormatConverterUnitTest, DataAccumulationFromVolume_004, TestSize.Level0)
{
    uint8_t srcBuffer[BUFFER_LENGTH_EIGHT] = {0};
    BufferDesc srcDesc = {srcBuffer, BUFFER_LENGTH_EIGHT, BUFFER_LENGTH_EIGHT};
    AudioStreamData srcData;
    srcData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};
    srcData.bufferDesc = srcDesc;
    std::vector<AudioStreamData> srcDataList = {srcData};

    uint8_t dstBuffer[BUFFER_LENGTH_EIGHT] = {0};
    BufferDesc dstDesc = {dstBuffer, BUFFER_LENGTH_EIGHT, BUFFER_LENGTH_EIGHT};
    AudioStreamData dstData;
    dstData.streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_F32LE, STEREO};
    dstData.bufferDesc = dstDesc;

    bool ret = FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);

    EXPECT_EQ(ret, false);
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
