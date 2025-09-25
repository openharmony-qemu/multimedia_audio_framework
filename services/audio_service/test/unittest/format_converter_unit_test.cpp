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

    // Helper function to create BufferDesc
    BufferDesc CreateBufferDesc(void* buffer, size_t bufLength)
    {
        BufferDesc desc;
        desc.buffer = static_cast<uint8_t*>(buffer);
        desc.bufLength = bufLength;
        return desc;
    }
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

BufferDesc srcDescTest;
BufferDesc dstDescTest;
uint8_t srcBufferTest[8] = {1, 2, 3, 4};
uint8_t dstBufferTest[8] = {0};

class BufferDescTest {
public:
    BufferDescTest(uint8_t *buffersrc, uint8_t *bufferdst, size_t bufLengthsrc, size_t bufLengthdst)
    {
        srcDescTest.buffer = buffersrc;
        dstDescTest.buffer = bufferdst;
        srcDescTest.bufLength = bufLengthsrc;
        dstDescTest.bufLength = bufLengthdst;
    }
};

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

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32MonoToS16Stereo_002
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32MonoToS16Stereo_002, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(nullptr, dstBufferTest, 4, 4);
    ret = FormatConverter::F32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, nullptr, 4, 4);
    ret = FormatConverter::F32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, dstBufferTest, 5, 5);
    ret = FormatConverter::F32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32MonoToS16Stereo_CapMax_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32MonoToS16Stereo_CapMax_001, TestSize.Level1)
{
    int32_t ret = -1;
    uint8_t srcBuffer[4] = {2, 3, 4, 5};
    uint8_t dstBuffer[4] = {0};
    BufferDescTest(srcBuffer, dstBuffer, 4, 4);
    ret = FormatConverter::F32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 0);
    EXPECT_EQ(*dstDescTest.buffer + 1, 1);
    EXPECT_EQ(*dstDescTest.buffer + 2, 2);
    EXPECT_EQ(*dstDescTest.buffer + 3, 3);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32MonoToS16Stereo_CapMax_002
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32MonoToS16Stereo_CapMax_002, TestSize.Level1)
{
    int32_t ret = -1;
    uint8_t srcBuffer[4] = {-2, 0, 1, 2};
    uint8_t dstBuffer[4] = {0};
    BufferDescTest(srcBuffer, dstBuffer, 4, 4);
    ret = FormatConverter::F32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 0);
    EXPECT_EQ(*dstDescTest.buffer + 1, 1);
    EXPECT_EQ(*dstDescTest.buffer + 2, 2);
    EXPECT_EQ(*dstDescTest.buffer + 3, 3);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: F32StereoToS16Stereo_002
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Stereo_002, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(nullptr, dstBufferTest, 4, 2);
    ret = FormatConverter::F32StereoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, nullptr, 4, 2);
    ret = FormatConverter::F32StereoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, dstBufferTest, 6, 3);
    ret = FormatConverter::F32StereoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16StereoToF32Mono_002
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16StereoToF32Mono_002, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(nullptr, dstBufferTest, 4, 4);
    ret = FormatConverter::S16StereoToF32Mono(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, nullptr, 4, 4);
    ret = FormatConverter::S16StereoToF32Mono(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16MonoToS32Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16MonoToS32Stereo_001, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(nullptr, dstBufferTest, 2, 8);
    ret = FormatConverter::S16MonoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, nullptr, 2, 8);
    ret = FormatConverter::S16MonoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, dstBufferTest, 4, 8);
    ret = FormatConverter::S16MonoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S32StereoToS16Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S32StereoToS16Stereo_001, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(nullptr, dstBufferTest, 8, 4);
    ret = FormatConverter::S32StereoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);

    BufferDescTest(srcBufferTest, nullptr, 8, 4);
    ret = FormatConverter::S32StereoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S32MonoToS16Stereo_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S32MonoToS16Stereo_001, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(srcBufferTest, dstBufferTest, 4, 4);
    ret = FormatConverter::S32MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 2);
    EXPECT_EQ(*dstDescTest.buffer + 1, 3);
    EXPECT_EQ(*dstDescTest.buffer + 2, 4);
    EXPECT_EQ(*dstDescTest.buffer + 3, 5);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16MonoToS16Stereo_004
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16MonoToS16Stereo_004, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(srcBufferTest, dstBufferTest, 4, 8);
    ret = FormatConverter::S16MonoToS16Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 1);
    EXPECT_EQ(*dstDescTest.buffer + 1, 2);
    EXPECT_EQ(*dstDescTest.buffer + 2, 3);
    EXPECT_EQ(*dstDescTest.buffer + 3, 4);

    ret = FormatConverter::S16StereoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 2);
    EXPECT_EQ(*dstDescTest.buffer + 1, 3);
    EXPECT_EQ(*dstDescTest.buffer + 2, 4);
    EXPECT_EQ(*dstDescTest.buffer + 3, 5);

    ret = FormatConverter::S32MonoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 1);
    EXPECT_EQ(*dstDescTest.buffer + 1, 2);
    EXPECT_EQ(*dstDescTest.buffer + 2, 3);
    EXPECT_EQ(*dstDescTest.buffer + 3, 4);

    ret = FormatConverter::F32MonoToS32Stereo(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 0);
    EXPECT_EQ(*dstDescTest.buffer + 1, 1);
    EXPECT_EQ(*dstDescTest.buffer + 2, 2);
    EXPECT_EQ(*dstDescTest.buffer + 3, 3);
}

/**
 * @tc.name  : Test FormatConverter API
 * @tc.type  : FUNC
 * @tc.number: S16StereoToS16Mono_001
 * @tc.desc  : Test FormatConverter interface.
 */
HWTEST_F(FormatConverterUnitTest, S16StereoToS16Mono_001, TestSize.Level1)
{
    int32_t ret = -1;
    BufferDescTest(srcBufferTest, dstBufferTest, 8, 4);
    ret = FormatConverter::S16StereoToS16Mono(srcDescTest, dstDescTest);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(*dstDescTest.buffer, 2);
    EXPECT_EQ(*dstDescTest.buffer + 1, 3);
    EXPECT_EQ(*dstDescTest.buffer + 2, 4);
    EXPECT_EQ(*dstDescTest.buffer + 3, 5);
}

// Test normal case: stereo to mono conversion
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_NormalCase, TestSize.Level1)
{
    FormatConverter converter;

    // Create test data: stereo (left and right channels interleaved)
    std::vector<float> stereoData = {
        1.0f, 2.0f,   // Frame 1: left=1.0, right=2.0 → mono=(1.0+2.0)/2=1.5
        3.0f, 4.0f,   // Frame 2: left=3.0, right=4.0 → mono=3.5
        5.0f, 6.0f    // Frame 3: left=5.0, right=6.0 → mono=5.5
    };

    std::vector<float> monoData(3); // Expected output: 3 mono samples

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_FLOAT_EQ(monoData[0], 1.5f);
    EXPECT_FLOAT_EQ(monoData[1], 3.5f);
    EXPECT_FLOAT_EQ(monoData[2], 5.5f);
}

// Test null source buffer
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_NullSrcBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<float> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(nullptr, stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test null destination buffer
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_NullDstBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<float> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(nullptr, monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test zero-length buffers
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_ZeroLengthBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<float> monoData(2);

    // Test zero-length source buffer
    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), 0);
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);

    // Test zero-length destination buffer
    srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    dstDesc = CreateBufferDesc(monoData.data(), 0);

    result = converter.F32StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test buffer length mismatch
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_BufferLengthMismatch, TestSize.Level1)
{
    FormatConverter converter;

    // Stereo data has 4 samples (2 frames), but destination buffer can only hold 1 mono sample
    std::vector<float> stereoData = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> monoData(1); // Too small, should need 2 mono samples

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test edge case: single frame conversion
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_SingleFrame, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData = {10.0f, 20.0f}; // One stereo frame
    std::vector<float> monoData(1);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_FLOAT_EQ(monoData[0], 15.0f); // (10+20)/2 = 15
}

// Test negative values
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_NegativeValues, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData = {-1.0f, 1.0f, -2.0f, 2.0f};
    std::vector<float> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_FLOAT_EQ(monoData[0], 0.0f);  // (-1+1)/2 = 0
    EXPECT_FLOAT_EQ(monoData[1], 0.0f);  // (-2+2)/2 = 0
}

// Test zero values
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_ZeroValues, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData = {0.0f, 0.0f, 0.0f, 0.0f};
    std::vector<float> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_FLOAT_EQ(monoData[0], 0.0f);
    EXPECT_FLOAT_EQ(monoData[1], 0.0f);
}

// Test large number of frames
HWTEST_F(FormatConverterUnitTest, F32StereoToF32Mono_MultipleFrames, TestSize.Level1)
{
    FormatConverter converter;

    const size_t frameCount = 10;
    std::vector<float> stereoData(frameCount * 2);
    std::vector<float> monoData(frameCount);

    // Fill stereo data with pattern: left = index, right = index + 0.5
    for (size_t i = 0; i < frameCount; i++) {
        stereoData[i * 2] = static_cast<float>(i);        // left channel
        stereoData[i * 2 + 1] = static_cast<float>(i) + 0.5f; // right channel
    }

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(float));

    int32_t result = converter.F32StereoToF32Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);

    // Verify each mono sample is the average of corresponding stereo frame
    for (size_t i = 0; i < frameCount; i++) {
        float expected = (static_cast<float>(i) + static_cast<float>(i) + 0.5f) / 2.0f;
        EXPECT_FLOAT_EQ(monoData[i], expected);
    }
}

// Test normal case: stereo float32 to mono int16 conversion
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_NormalCase, TestSize.Level1)
{
    FormatConverter converter;

    // Create test data: stereo float32 (left and right channels interleaved)
    std::vector<float> stereoData = {
        1.0f, -1.0f,   // Frame 1: left=1.0, right=-1.0 → mono=0.0
        0.5f, -0.5f,   // Frame 2: left=0.5, right=-0.5 → mono=0.0
        0.8f, 0.2f     // Frame 3: left=0.8, right=0.2 → mono=0.5
    };

    // Expected output: 3 mono int16 samples
    std::vector<int16_t> monoData(3);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);

    // Verify conversion with proper scaling to int16 range
    EXPECT_EQ(monoData[0], 0);                                  // (1.0 + -1.0)/2 = 0.0 → 0
    EXPECT_EQ(monoData[1], 0);                                  // (0.5 + -0.5)/2 = 0.0 → 0
    EXPECT_EQ(monoData[2], static_cast<int16_t>(16384));        // (0.8 + 0.2)/2 = 0.5 → 0.5 * 32767
}

// Test null source buffer
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_NullSrcBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<int16_t> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(nullptr, stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test null destination buffer
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_NullDstBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<int16_t> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(nullptr, monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test zero-length buffers
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_ZeroLengthBuffer, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData(4);
    std::vector<int16_t> monoData(2);

    // Test zero-length source buffer
    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), 0);
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);

    // Test zero-length destination buffer
    srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    dstDesc = CreateBufferDesc(monoData.data(), 0);

    result = converter.F32StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test buffer length mismatch
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_BufferLengthMismatch, TestSize.Level1)
{
    FormatConverter converter;

    // Stereo data has 4 float samples (2 frames), but destination buffer can only hold 1 int16 sample
    std::vector<float> stereoData = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<int16_t> monoData(1); // Too small, should need 2 int16 samples

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);
    EXPECT_EQ(result, -1);
}

// Test edge case: single frame conversion
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_SingleFrame, TestSize.Level1)
{
    FormatConverter converter;

    std::vector<float> stereoData = {1.0f, 0.0f}; // One stereo frame
    std::vector<int16_t> monoData(1);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(monoData[0], static_cast<int16_t>(16383)); // (1.0+0.0)/2 = 0.5 → 0.5 * 32767
}

// Test full scale values
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_FullScaleValues, TestSize.Level1)
{
    FormatConverter converter;

    // Test maximum positive and negative values
    std::vector<float> stereoData = {1.0f, 1.0f, -1.0f, -1.0f}; // Two stereo frames
    std::vector<int16_t> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(monoData[0], 32767);   // (1.0+1.0)/2 = 1.0 → 32767
    EXPECT_EQ(monoData[1], -32767);  // (-1.0+-1.0)/2 = -1.0 → -32767
}

// Test clipping behavior (values beyond ±1.0)
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_ClippingBehavior, TestSize.Level1)
{
    FormatConverter converter;

    // Test values that would exceed int16 range without clamping
    std::vector<float> stereoData = {2.0f, 2.0f, -2.0f, -2.0f}; // Values beyond ±1.0
    std::vector<int16_t> monoData(2);

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);
    // Assuming CapMax() clamps values to ±1.0 range
    EXPECT_EQ(monoData[0], 32767);   // Clamped to (1.0+1.0)/2 = 1.0 → 32767
    EXPECT_EQ(monoData[1], -32767);  // Clamped to (-1.0+-1.0)/2 = -1.0 → -32767
}

// Test multiple frames with various values
HWTEST_F(FormatConverterUnitTest, F32StereoToS16Mono_MultipleFrames, TestSize.Level1)
{
    FormatConverter converter;

    const size_t frameCount = 5;
    std::vector<float> stereoData(frameCount * 2);
    std::vector<int16_t> monoData(frameCount);

    // Fill with various test values
    stereoData = {
        0.25f, 0.75f,   // → 0.5f
        0.1f, 0.9f,     // → 0.5f
        -0.3f, 0.3f,    // → 0.0f
        0.9f, -0.9f,    // → 0.0f
        0.0f, 0.0f      // → 0.0f
    };

    BufferDesc srcDesc = CreateBufferDesc(stereoData.data(), stereoData.size() * sizeof(float));
    BufferDesc dstDesc = CreateBufferDesc(monoData.data(), monoData.size() * sizeof(int16_t));

    int32_t result = converter.F32StereoToS16Mono(srcDesc, dstDesc);

    EXPECT_EQ(result, 0);

    // Verify conversion results
    EXPECT_EQ(monoData[0], static_cast<int16_t>(16384));   // (0.25+0.75)/2 = 0.5
    EXPECT_EQ(monoData[1], static_cast<int16_t>(16384));   // (0.1+0.9)/2 = 0.5
    EXPECT_EQ(monoData[2], 0);                             // (-0.3+0.3)/2 = 0.0
    EXPECT_EQ(monoData[3], 0);                             // (0.9+-0.9)/2 = 0.0
    EXPECT_EQ(monoData[4], 0);                             // (0.0+0.0)/2 = 0.0
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
