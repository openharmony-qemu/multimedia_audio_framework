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
}  // namespace OHOS::AudioStandard
}  // namespace OHOS
