/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <thread>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"
#include "audio_scope_exit.h"
#include "audio_safe_block_queue.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {

constexpr unsigned int QUEUE_SLOTS = 10;
constexpr unsigned int THREAD_NUM = QUEUE_SLOTS + 1;

class MockExe {
public:
    MOCK_METHOD(void, Exe, ());
};

class AudioUtilsUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioUtilsUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioUtilsUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioUtilsUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioUtilsUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test ClockTime API
* @tc.type  : FUNC
* @tc.number: ClockTime_001
* @tc.desc  : Test ClockTime interface.
*/
HWTEST(AudioUtilsUnitTest, ClockTime_001, TestSize.Level1)
{
    const int64_t CLOCK_TIME = 0;
    int32_t ret = -1;
    ret = ClockTime::AbsoluteSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);

    int64_t nanoTime = 1000;
    ret = ClockTime::AbsoluteSleep(nanoTime);
    EXPECT_EQ(SUCCESS, ret);

    ret = ClockTime::RelativeSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);

    ret = ClockTime::RelativeSleep(nanoTime);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Trace API
* @tc.type  : FUNC
* @tc.number: Trace_001
* @tc.desc  : Test Trace interface.
*/
HWTEST(AudioUtilsUnitTest, Trace_001, TestSize.Level1)
{
    std::string value = "Test";
    std::shared_ptr<Trace> trace = std::make_shared<Trace>(value);
    trace->End();
    int64_t count = 1;
    Trace::Count(value, count);
}

/**
* @tc.name  : Test PermissionUtil API
* @tc.type  : FUNC
* @tc.number: PermissionUtil_001
* @tc.desc  : Test PermissionUtil interface.
*/
HWTEST(AudioUtilsUnitTest, PermissionUtil_001, TestSize.Level1)
{
    bool ret1 = PermissionUtil::VerifyIsSystemApp();
    EXPECT_EQ(true, ret1);
    bool ret2 = PermissionUtil::VerifySelfPermission();
    EXPECT_EQ(true, ret2);
    bool ret3 = PermissionUtil::VerifySystemPermission();
    EXPECT_EQ(true, ret3);
}

/**
* @tc.name  : Test AdjustStereoToMonoForPCM API
* @tc.type  : FUNC
* @tc.number: AdjustStereoToMonoForPCM_001
* @tc.desc  : Test AdjustStereoToMonoForPCM interface.
*/
HWTEST(AudioUtilsUnitTest, AdjustStereoToMonoForPCM_001, TestSize.Level1)
{
    uint64_t len = 2;

    const int8_t BitRET = 1;
    int8_t arr1[2] = {1, 2};
    int8_t *data1 = &arr1[0];
    AdjustStereoToMonoForPCM8Bit(data1, len);
    EXPECT_EQ(BitRET, data1[0]);
    EXPECT_EQ(BitRET, data1[1]);

    len = 4;
    const int16_t Bit16RET = 1;
    int16_t arr2[2] = {1, 2};
    int16_t *data2 = &arr2[0];
    AdjustStereoToMonoForPCM16Bit(data2, len);
    EXPECT_EQ(Bit16RET, data2[0]);
    EXPECT_EQ(Bit16RET, data2[1]);

    len = 8;
    const int32_t Bit32RET = 1;
    int32_t arr4[2] = {1, 2};
    int32_t *data4 = &arr4[0];
    AdjustStereoToMonoForPCM32Bit(data4, len);
    EXPECT_EQ(Bit32RET, data4[0]);
    EXPECT_EQ(Bit32RET, data4[1]);
}

/**
* @tc.name  : Test AdjustAudioBalanceForPCM API
* @tc.type  : FUNC
* @tc.number: AdjustAudioBalanceForPCM_001
* @tc.desc  : Test AdjustAudioBalanceForPCM interface.
*/
HWTEST(AudioUtilsUnitTest, AdjustAudioBalanceForPCM_001, TestSize.Level1)
{
    float left = 2.0;
    float right = 2.0;
    uint64_t len = 2;

    const int8_t Bit8RET1 = 2;
    const int8_t Bit8RET2 = 4;
    int8_t arr1[2] = {1, 2};
    int8_t *data1 = &arr1[0];
    AdjustAudioBalanceForPCM8Bit(data1, len, left, right);
    EXPECT_EQ(Bit8RET1, data1[0]);
    EXPECT_EQ(Bit8RET2, data1[1]);

    len = 4;
    const int16_t Bit16RET1 = 2;
    const int16_t Bit16RET2 = 4;
    int16_t arr2[2] = {1, 2};
    int16_t *data2 = &arr2[0];
    AdjustAudioBalanceForPCM16Bit(data2, len, left, right);
    EXPECT_EQ(Bit16RET1, data2[0]);
    EXPECT_EQ(Bit16RET2, data2[1]);

    len = 8;
    const int32_t Bit32RET = 2;
    int32_t arr4[2] = {1, 2};
    int32_t *data4 = &arr4[0];
    AdjustAudioBalanceForPCM32Bit(data4, len, left, right);
    EXPECT_EQ(Bit32RET, data4[0]);
    EXPECT_EQ(Bit32RET * 2, data4[1]);
}

/**
* @tc.name  : Test GetSysPara API
* @tc.type  : FUNC
* @tc.number: GetSysPara_001
* @tc.desc  : Test GetSysPara interface.
*/
HWTEST(AudioUtilsUnitTest, GetSysPara_001, TestSize.Level1)
{
    const char *invaildKey = nullptr;
    int32_t value32 = 2;
    bool result = GetSysPara(invaildKey, value32);
    EXPECT_EQ(false, result);
    const char *key = "debug.audio_service.testmodeon";
    bool result1 = GetSysPara(key, value32);
    EXPECT_EQ(true, result1);
    uint32_t valueU32 = 3;
    bool result2 = GetSysPara(key, valueU32);
    EXPECT_EQ(true, result2);
    int64_t value64 = 0;
    bool result3 = GetSysPara(key, value64);
    EXPECT_EQ(true, result3);
    std::string strValue = "100";
    bool result4 = GetSysPara(key, strValue);
    EXPECT_EQ(true, result4);
}

/**
* @tc.name  : Test UpdateMaxAmplitude API
* @tc.type  : FUNC
* @tc.number: UpdateMaxAmplitude_001
* @tc.desc  : Test UpdateMaxAmplitude interface  when adapterFormat is SAMPLE_U8_C.
*/
HWTEST(AudioUtilsUnitTest, UpdateMaxAmplitude_001, TestSize.Level0)
{
    ConvertHdiFormat adapterFormat = SAMPLE_U8_C;
    char frame[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);
    EXPECT_NEAR(result, 0.071, 0.001);
}

/**
* @tc.name  : Test UpdateMaxAmplitude API
* @tc.type  : FUNC
* @tc.number: UpdateMaxAmplitude_002
* @tc.desc  : Test UpdateMaxAmplitude interface  when adapterFormat is SAMPLE_S16_C.
*/
HWTEST(AudioUtilsUnitTest, UpdateMaxAmplitude_002, TestSize.Level0)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S16_C;
    char frame[20] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);
    EXPECT_NEAR(result, 0.032, 0.001);
}

/**
* @tc.name  : Test UpdateMaxAmplitude API
* @tc.type  : FUNC
* @tc.number: UpdateMaxAmplitude_003
* @tc.desc  : Test UpdateMaxAmplitude interface  when adapterFormat is SAMPLE_S24_C.
*/
HWTEST(AudioUtilsUnitTest, UpdateMaxAmplitude_003, TestSize.Level0)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S24_C;
    char frame[30] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,
                     5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);
    EXPECT_NEAR(result, 0.016, 0.001);
}

/**
* @tc.name  : Test UpdateMaxAmplitude API
* @tc.type  : FUNC
* @tc.number: UpdateMaxAmplitude_004
* @tc.desc  : Test UpdateMaxAmplitude interface  when adapterFormat is SAMPLE_S32_C.
*/
HWTEST(AudioUtilsUnitTest, UpdateMaxAmplitude_004, TestSize.Level0)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S32_C;
    char frame[40] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
                    5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);
    EXPECT_NEAR(result, 0, 0.1);
}

/**
* @tc.name  : Test UpdateMaxAmplitude API
* @tc.type  : FUNC
* @tc.number: UpdateMaxAmplitude_005
* @tc.desc  : Test UpdateMaxAmplitude interface when adapterFormat is not supported then AUDIO_INFO_LOG is called
*/
HWTEST(AudioUtilsUnitTest, UpdateMaxAmplitude_005, TestSize.Level0)
{
    ConvertHdiFormat adapterFormat = INVALID_WIDTH_C;
    char frame[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM8Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM8Bit_001
* @tc.desc  : Test CalculateMaxAmplitudeForPCM8Bit interface Return 0 when frame is null arry
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM8Bit_001, TestSize.Level0)
{
    int8_t frame[0];
    uint64_t nSamples = 0;
    float result = CalculateMaxAmplitudeForPCM8Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM8Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM8Bit_002
* @tc.desc  : Test CalculateMaxAmplitudeForPCM8Bit interface Return 0 when nSamples is 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM8Bit_002, TestSize.Level0)
{
    int8_t frame1[1] = {1};
    uint64_t nSamples1 = 0;
    float result = CalculateMaxAmplitudeForPCM8Bit(frame1, nSamples1);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM8Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM8Bit_003
* @tc.desc  : Test CalculateMaxAmplitudeForPCM8Bit interface
*             Return MaxAmplitude when frame is not null arry and nSamples is not 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM8Bit_003, TestSize.Level0)
{
    int8_t frame2[3] = {1, -2, 3};
    uint64_t nSamples2 = 3;
    float result = CalculateMaxAmplitudeForPCM8Bit(frame2, nSamples2);
    EXPECT_FLOAT_EQ(result, 3.0f/ SCHAR_MAX);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM16Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM16Bit_001
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit interface .Return 0 when  nSamples is 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM16Bit_001, TestSize.Level0)
{
    int16_t frame[1] = {100};
    uint64_t nSamples = 0;
    float result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM16Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM16Bit_002
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit interface .Return 1.0 when value < 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM16Bit_002, TestSize.Level0)
{
    int16_t frame[5] = {-6554, -8192, -10923, -16384, -32767};;
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
    EXPECT_NEAR(result, 1.0, 0.1);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM16Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM16Bit_003
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit interface .update curMaxAmplitude when curMaxAmplitude < value
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM16Bit_003, TestSize.Level0)
{
    int16_t frame[5] = {6554, 8192, 10923, 16384, 32767};;
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
    EXPECT_NEAR(result, 1.0, 0.001);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM16Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM16Bit_004
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit interface
*               when value isn't bigger than curMaxAmplitude and smaller than 0.
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM16Bit_004, TestSize.Level0)
{
    int16_t frame[5] = {0};;
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM16Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM16Bit_005
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit interface when value contains  negative and positive numbers.
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM16Bit_005, TestSize.Level0)
{
    int16_t frame[5] = {-6554, -8192, 0, 10923, 16384};
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);
    EXPECT_NEAR(result, 0.5, 0.1);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM24Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM24Bit_001
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit interface .Return 0 when  nSamples is 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM24Bit_001, TestSize.Level0)
{
    char frame[1] = {100};
    uint64_t nSamples = 0;
    float result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM24Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM24Bit_002
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit interface .Return 1.0 when value < 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM24Bit_002, TestSize.Level0)
{
    char frame[15] = {0xC0, 0x00, 0x00, 0xE0, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xF4, 0x00, 0x00};

    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
    EXPECT_NEAR(result, 3.0e-05, 0.1e-05);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM24Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM24Bit_003
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit interface .update curMaxAmplitude when curMaxAmplitude < value
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM24Bit_003, TestSize.Level0)
{
    char frame[15] = {0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00};
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
    EXPECT_NEAR(result, 0.76e-05, 0.1e-05);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM24Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM24Bit_004
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit interface
*               when value isn't bigger than curMaxAmplitude and smaller than 0.
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM24Bit_004, TestSize.Level0)
{
    char frame[3] = {0x00, 0x00, 0x00};
    uint64_t nSamples = 1;
    float result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
    EXPECT_NEAR(result, 0, 0.1);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM24Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM24Bit_005
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit interface when value contains  negative and positive numbers.
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM24Bit_005, TestSize.Level0)
{
    char frame[15] = {0xC0, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00};
    uint64_t nSamples = 5;
    float result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);
    EXPECT_NEAR(result,  2.7e-05, 0.1e-05);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM32Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM32Bit_001
* @tc.desc  : Test CalculateMaxAmplitudeForPCM32Bit interface .Return 0 when  nSamples is 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM32Bit_001, TestSize.Level0)
{
    int32_t frame[1] = {100};
    uint64_t nSamples = 0;
    float result = CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM32Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM32Bit_002
* @tc.desc  : Test CalculateMaxAmplitudeForPCM32Bit interface .Return 1.0 when value < 0
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM32Bit_002, TestSize.Level0)
{
    int32_t frame[4] = {-1, 1, 0, 8};
    uint64_t nSamples = 4;
    float result = CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);
    EXPECT_NEAR(result, 0, 0.0001);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM32Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM32Bit_003
* @tc.desc  : Test CalculateMaxAmplitudeForPCM32Bit interface .update curMaxAmplitude when curMaxAmplitude < value
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM32Bit_003, TestSize.Level0)
{
    int32_t frame[4] = {0, 1, -1, 8};
    uint64_t nSamples = 4;
    float result = CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);
    EXPECT_NEAR(result, 0, 0.0001);
}

/**
* @tc.name  : Test CalculateMaxAmplitudeForPCM32Bit API
* @tc.type  : FUNC
* @tc.number: CalculateMaxAmplitudeForPCM32Bit_004
* @tc.desc  : Test CalculateMaxAmplitudeForPCM32Bit interface
*               when value isn't bigger than curMaxAmplitude and smaller than 0.
*/
HWTEST(AudioUtilsUnitTest, CalculateMaxAmplitudeForPCM32Bit_004, TestSize.Level0)
{
    int32_t frame[6] = {0};
    uint64_t nSamples = 6;
    float result = CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test GetFormatByteSize API
* @tc.type  : FUNC
* @tc.number: GetFormatByteSize_001
* @tc.desc  : Test GetFormatByteSize interface Return 2 when format is  SAMPLE_S16LE
*/
HWTEST(AudioUtilsUnitTest, GetFormatByteSize_001, TestSize.Level0)
{
    int32_t format = 100;
    int32_t formatByteSize = GetFormatByteSize(format);
    EXPECT_EQ(formatByteSize, 2);
}

/**
* @tc.name  : Test GetFormatByteSize API
* @tc.type  : FUNC
* @tc.number: GetFormatByteSize_002
* @tc.desc  : Test GetFormatByteSize interface Return 2 when  when format is other int32_t
*/
HWTEST(AudioUtilsUnitTest, GetFormatByteSize_002, TestSize.Level0)
{
    int32_t format = SAMPLE_S16LE;
    int32_t formatByteSize = GetFormatByteSize(format);
    EXPECT_EQ(formatByteSize, 2);
}

/**
* @tc.name  : Test GetFormatByteSize API
* @tc.type  : FUNC
* @tc.number: GetFormatByteSize_003
* @tc.desc  : Test GetFormatByteSize interface Return 3 when format is  SAMPLE_S24LE
*/
HWTEST(AudioUtilsUnitTest, GetFormatByteSize_003, TestSize.Level0)
{
    int32_t format = SAMPLE_S24LE;
    int32_t formatByteSize = GetFormatByteSize(format);
    EXPECT_EQ(formatByteSize, 3);
}

/**
* @tc.name  : Test GetFormatByteSize API
* @tc.type  : FUNC
* @tc.number: GetFormatByteSize_004
* @tc.desc  : Test GetFormatByteSize interface Return 4 when format is  SAMPLE_S32LE
*/
HWTEST(AudioUtilsUnitTest, GetFormatByteSize_004, TestSize.Level0)
{
    int32_t format = SAMPLE_S32LE;
    int32_t formatByteSize = GetFormatByteSize(format);
    EXPECT_EQ(formatByteSize, 4);
}

/**
* @tc.name  : Test SignalDetectAgent::DetectSignalData API
* @tc.type  : FUNC
* @tc.number: SignalDetectAgent_DetectSignalData_001
* @tc.desc  : Test DetectSignalData interface Return  false when bufferLen is 0
*/
HWTEST(AudioUtilsUnitTest, SignalDetectAgent_DetectSignalData_001, TestSize.Level0)
{
    int32_t buffer[10] = {0};
    size_t bufferLen = 0;
    struct SignalDetectAgent signalDetectAgent;
    bool ret = signalDetectAgent.DetectSignalData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}
/**
* @tc.name  : Test SignalDetectAgent::DetectSignalData API
* @tc.type  : FUNC
* @tc.number: SignalDetectAgent_DetectSignalData_002
* @tc.desc  : Test DetectSignalData interface Return  false when NearZero(tempMax) is true and NearZero(tempMin) is true
*/
HWTEST(AudioUtilsUnitTest, SignalDetectAgent_DetectSignalData_002, TestSize.Level0)
{
    int32_t buffer[10] = {2, 3, 2, 3, 2, 3, 2, 3, 2, 3};
    size_t bufferLen = 10 * sizeof(int32_t);

    struct SignalDetectAgent signalDetectAgent;
    bool ret = signalDetectAgent.DetectSignalData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SignalDetectAgent::DetectSignalData API
* @tc.type  : FUNC
* @tc.number: SignalDetectAgent_DetectSignalData_003
* @tc.desc  : Test DetectSignalData interface Return  true when currentPeakIndex is -1
*/
HWTEST(AudioUtilsUnitTest, SignalDetectAgent_DetectSignalData_003, TestSize.Level0)
{
    int32_t buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t bufferLen = 10 * sizeof(int32_t);

    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.ResetDetectResult();
    signalDetectAgent.channels_ = 1;
    bool ret = signalDetectAgent.DetectSignalData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SignalDetectAgent::DetectSignalData API
* @tc.type  : FUNC
* @tc.number: SignalDetectAgent_DetectSignalData_004
* @tc.desc  : Test DetectSignalData interface Return  true when blankPeriod_ < thresholdBlankPeriod
*/
HWTEST(AudioUtilsUnitTest, SignalDetectAgent_DetectSignalData_004, TestSize.Level0)
{
    int32_t buffer[10] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
    size_t bufferLen = 10 * sizeof(int32_t);

    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.ResetDetectResult();
    signalDetectAgent.channels_ = 1 ;
    signalDetectAgent.sampleRate_ = 100;
    bool ret = signalDetectAgent.DetectSignalData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test SignalDetectAgent::ResetDetectResult API
* @tc.type  : FUNC
* @tc.number: SignalDetectAgent_ResetDetectResult_001
* @tc.desc  : Test ResetDetectResult interface.
*/
HWTEST(AudioUtilsUnitTest, SignalDetectAgent_ResetDetectResult_001, TestSize.Level0)
{
    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.ResetDetectResult();
    EXPECT_EQ(signalDetectAgent.blankHaveOutput_, true);
    EXPECT_EQ(signalDetectAgent.hasFirstNoneZero_, false);
    EXPECT_EQ(signalDetectAgent.lastPeakSignal_, SHRT_MIN);
    EXPECT_EQ(signalDetectAgent.signalDetected_, true);
    EXPECT_EQ(signalDetectAgent.dspTimestampGot_, false);
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_001
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return VOICE_ASSISTANT
*             when streamType is STREAM_VOICE_ASSISTANT
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_001, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_VOICE_ASSISTANT;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "VOICE_ASSISTANT");
}
/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_002
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return VOICE_CALL
*             when streamType is STREAM_VOICE_CALL
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_002, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_VOICE_CALL;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "VOICE_CALL");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_003
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return VOICE_CALL
*             when streamType is STREAM_VOICE_COMMUNICATION
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_003, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_VOICE_COMMUNICATION;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "VOICE_COMMUNICATION");
}
/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_004
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return SYSTEM
*             when streamType is STREAM_SYSTEM
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_004, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_SYSTEM;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "SYSTEM");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_005
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return RING
*             when streamType is STREAM_RING
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_005, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_RING;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "RING");
}
/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_006
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return MUSIC
*             when streamType is STREAM_MUSIC
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_006, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_MUSIC;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "MUSIC");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_007
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return ALARM
*             when streamType is STREAM_ALARM
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_007, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_ALARM;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "ALARM");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_008
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return NOTIFICATION
*             when streamType is STREAM_NOTIFICATION
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_008, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_NOTIFICATION;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "NOTIFICATION");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_009
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return BLUETOOTH_SCO
*             when streamType is STREAM_BLUETOOTH_SCO
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_009, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_BLUETOOTH_SCO;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "BLUETOOTH_SCO");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_010
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return DTMF
*             when streamType is STREAM_DTMF
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_010, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_DTMF;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "DTMF");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_011
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return TTS
*             when streamType is STREAM_TTS
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_011, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_TTS;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "TTS");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_013
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return ULTRASONIC
*             when streamType is STREAM_ULTRASONIC
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_013, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_ULTRASONIC;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "ULTRASONIC");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_014
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return WAKEUP
*             when streamType is STREAM_WAKEUP
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_014, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_WAKEUP;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "WAKEUP");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_015
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API,Return UNKNOWN
*             when streamType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_015, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_DEFAULT;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "UNKNOWN");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_001
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return EARPIECE
*             when deviceType is DEVICE_TYPE_EARPIECE
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_001, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "EARPIECE");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_002
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return SPEAKER
*             when deviceType is DEVICE_TYPE_SPEAKER
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_002, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "SPEAKER");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_003
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return WIRED_HEADSET
*             when deviceType is DEVICE_TYPE_WIRED_HEADSET
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_003, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "WIRED_HEADSET");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_004
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return SPEAKER
*             when deviceType is DEVICE_TYPE_WIRED_HEADPHONES
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_004, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "WIRED_HEADPHONES");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_005
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return BLUETOOTH_SCO
*             when deviceType is DEVICE_TYPE_BLUETOOTH_SCO
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_005, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "BLUETOOTH_SCO");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_006
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return BLUETOOTH_A2DP
*             when deviceType is DEVICE_TYPE_BLUETOOTH_A2DP
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_006, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "BLUETOOTH_A2DP");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_007
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return MIC
*             when deviceType is DEVICE_TYPE_MIC
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_007, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_MIC;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "MIC");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_008
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return WAKEUP
*             when deviceType is DEVICE_TYPE_WAKEUP
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_008, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_WAKEUP;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "WAKEUP");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_009
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return NONE
*             when deviceType is DEVICE_TYPE_NONE
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_009, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_NONE;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "NONE");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_010
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return INVALID
*             when deviceType is DEVICE_TYPE_INVALID
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_010, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "INVALID");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_011
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return UNKNOWN
*             when deviceType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_011, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_DEFAULT;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "UNKNOWN");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_012
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return HDMI
*             when deviceType is DEVICE_TYPE_HDMI
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_012, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_HDMI;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "HDMI");
}

/**
 * @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
 * @tc.type  : FUNC
 * @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_013
 * @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return NEARLINK
 *             when deviceType is DEVICE_TYPE_NEARLINK
 */
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_013, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_NEARLINK;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "NEARLINK");
}

/**
 * @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
 * @tc.type  : FUNC
 * @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_014
 * @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return REMOTE_CAST
 *             when deviceType is DEVICE_TYPE_REMOTE_CAST
 */
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_014, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_REMOTE_CAST;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "REMOTE_CAST");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceTypeName_015
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceTypeName API,Return HEARING_AID
*             when deviceType is DEVICE_TYPE_HEARING_AID
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceTypeName_015, TestSize.Level0)
{
    DeviceType deviceType = DEVICE_TYPE_HEARING_AID;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "HEARING_AID");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetConnectTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetConnectTypeName_001
* @tc.desc  : Test AudioInfoDumpUtils GetConnectTypeName API,Return LOCAL
*             when connectType is CONNECT_TYPE_LOCAL
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetConnectTypeName_001, TestSize.Level0)
{
    ConnectType connectType = OHOS::AudioStandard::CONNECT_TYPE_LOCAL;
    const std::string connectTypeName = AudioInfoDumpUtils::GetConnectTypeName(connectType);
    EXPECT_EQ(connectTypeName, "LOCAL");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetConnectTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetConnectTypeName_002
* @tc.desc  : Test AudioInfoDumpUtils GetConnectTypeName API,Return REMOTE
*             when connectType is CONNECT_TYPE_DISTRIBUTED
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetConnectTypeName_002, TestSize.Level0)
{
    ConnectType connectType = AudioStandard::CONNECT_TYPE_DISTRIBUTED;
    const std::string connectTypeName = AudioInfoDumpUtils::GetConnectTypeName(connectType);
    EXPECT_EQ(connectTypeName, "REMOTE");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetConnectTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetConnectTypeName_003
* @tc.desc  : Test AudioInfoDumpUtils GetConnectTypeName API,Return UNKNOWN
*             when connectType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetConnectTypeName_003, TestSize.Level0)
{
    ConnectType connectType =  static_cast<ConnectType>(-1);
    const std::string connectTypeName = AudioInfoDumpUtils::GetConnectTypeName(connectType);
    EXPECT_EQ(connectTypeName, "UNKNOWN");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_001
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return INVALID
*             when sourceType is SOURCE_TYPE_INVALID
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_001, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_INVALID;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "INVALID");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_002
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return MIC
*             when sourceType is SOURCE_TYPE_MIC
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_002, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_MIC;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "MIC");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_003
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return VOICE_RECOGNITION
*             when sourceType is SOURCE_TYPE_VOICE_RECOGNITION
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_003, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "VOICE_RECOGNITION");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_004
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return ULTRASONIC
*             when sourceType is SOURCE_TYPE_ULTRASONIC
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_004, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_ULTRASONIC;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "ULTRASONIC");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_005
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return VOICE_COMMUNICATION
*             when sourceType is SOURCE_TYPE_VOICE_COMMUNICATION
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_005, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "VOICE_COMMUNICATION");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_006
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return WAKEUP
*             when sourceType is SOURCE_TYPE_WAKEUP
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_006, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_WAKEUP;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "WAKEUP");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetSourceName_007
* @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return SOURCE_TYPE_LIVE
*             when sourceType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_007, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_MAX;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "UNKNOWN");
}

/**
 * @tc.name  : Test AudioInfoDumpUtils::GetSourceName  API
 * @tc.type  : FUNC
 * @tc.number: AudioInfoDumpUtils_GetSourceName_008
 * @tc.desc  : Test AudioInfoDumpUtils GetSourceName API,Return SOURCE_TYPE_UNPROCESSED
 *             when sourceType is SOURCE_TYPE_UNPROCESSED
 */
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetSourceName_008, TestSize.Level0)
{
    SourceType sourceType = SOURCE_TYPE_UNPROCESSED;
    const std::string sourceName = AudioInfoDumpUtils::GetSourceName(sourceType);
    EXPECT_EQ(sourceName, "SOURCE_TYPE_UNPROCESSED");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceVolumeTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceVolumeTypeName_001
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceVolumeTypeName API,Return EARPIECE
*             when deviceType is EARPIECE_VOLUME_TYPE
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceVolumeTypeName_001, TestSize.Level0)
{
    DeviceVolumeType deviceType = EARPIECE_VOLUME_TYPE;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceVolumeTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "EARPIECE");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceVolumeTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceVolumeTypeName_002
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceVolumeTypeName API,Return SPEAKER
*             when deviceType is SPEAKER_VOLUME_TYPE
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceVolumeTypeName_002, TestSize.Level0)
{
    DeviceVolumeType deviceType = SPEAKER_VOLUME_TYPE;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceVolumeTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "SPEAKER");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceVolumeTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceVolumeTypeName_003
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceVolumeTypeName API,Return HEADSET
*             when deviceType is HEADSET_VOLUME_TYPE
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceVolumeTypeName_003, TestSize.Level0)
{
    DeviceVolumeType deviceType = HEADSET_VOLUME_TYPE;
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceVolumeTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "HEADSET");
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetDeviceVolumeTypeName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetDeviceVolumeTypeName_004
* @tc.desc  : Test AudioInfoDumpUtils GetDeviceVolumeTypeName API,Return UNKNOWN
*             when deviceType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetDeviceVolumeTypeName_004, TestSize.Level0)
{
    DeviceVolumeType deviceType = static_cast<DeviceVolumeType>(-1);
    const std::string deviceTypeName = AudioInfoDumpUtils::GetDeviceVolumeTypeName(deviceType);
    EXPECT_EQ(deviceTypeName, "UNKNOWN");
}


/**
* @tc.name  : Test GetEncryptStr  API
* @tc.type  : FUNC
* @tc.number: GetEncryptStr_001
* @tc.desc  : Test GetEncryptStr API,Return empty when src is empty
*/
HWTEST(AudioUtilsUnitTest, GetEncryptStr_001, TestSize.Level0)
{
    const std::string src = "";
    std::string dst = GetEncryptStr(src);
    EXPECT_EQ(dst, "");
}

/**
* @tc.name  : Test GetEncryptStr  API
* @tc.type  : FUNC
* @tc.number: GetEncryptStr_002
* @tc.desc  : Test GetEncryptStr APIwhen length of src less than MIN_LEN
*/
HWTEST(AudioUtilsUnitTest, GetEncryptStr_002, TestSize.Level0)
{
    const std::string src = "abcdef";
    std::string dst = GetEncryptStr(src);
    EXPECT_EQ(dst, "*bcdef");
}

/**
* @tc.name  : Test Hide API
* @tc.type  : FUNC
* @tc.number: Hide_001
* @tc.desc  : Test Hide API
*/
HWTEST(AudioUtilsUnitTest, Hide_001, TestSize.Level0)
{
    string str{"12345"};
    EXPECT_EQ(Hide(str), "*");
    str = "123456";
    EXPECT_EQ(Hide(str), "123*456");
    str = "13682363247";
    EXPECT_EQ(Hide(str), "136*247");
}

class DemoThreadData {
public:
    DemoThreadData()
    {
        putStatus = false;
        getStatus = false;
    }
    bool putStatus;
    bool getStatus;
    static AudioSafeBlockQueue<int> shareQueue;

    void Put(int j)
    {
        shareQueue.Push(j);
        putStatus = true;
    }

    void Get()
    {
        shareQueue.Pop();
        getStatus = true;
    }
};

AudioSafeBlockQueue<int> DemoThreadData::shareQueue(QUEUE_SLOTS);

void PutHandleThreadData(DemoThreadData& q, int i)
{
    q.Put(i);
}

void GetThreadDatePushedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
                               unsigned int& unpushedIn)
{
    pushedIn = 0;
    unpushedIn = 0;
    for (auto& t : demoDatas) {
        if (t.putStatus) {
            pushedIn++;
        } else {
            unpushedIn++;
        }
    }
}

void GetThreadDateGetedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas,
    unsigned int& getedOut, unsigned int& ungetedOut)
{
    getedOut = 0;
    ungetedOut = 0;
    for (auto& t : demoDatas) {
        if (t.getStatus) {
            getedOut++;
        } else {
            ungetedOut++;
        }
    }
}

void PutHandleThreadDataTime(DemoThreadData& q, int i, std::chrono::system_clock::time_point absTime)
{
    cout << "thread-" << std::this_thread::get_id() << " run time: "
        << std::chrono::system_clock::to_time_t(absTime) << endl;
    std::this_thread::sleep_until(absTime);

    q.Put(i);
}

void GetHandleThreadDataTime(DemoThreadData& q, int i, std::chrono::system_clock::time_point absTime)
{
    cout << "thread-" << std::this_thread::get_id() << " run time: "
        << std::chrono::system_clock::to_time_t(absTime) << endl;
    std::this_thread::sleep_until(absTime);

    q.Get();
}

/*
 * @tc.name: testPut001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
HWTEST(AudioUtilsUnitTest, testPut001, TestSize.Level0)
{
    AudioSafeBlockQueue<int> qi(10);
    int i = 1;
    qi.Push(i);
    EXPECT_EQ(static_cast<unsigned>(1), qi.Size());
}

/*
 * @tc.name: testGet001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
HWTEST(AudioUtilsUnitTest, testGet001, TestSize.Level0)
{
    AudioSafeBlockQueue<int> qi(10);
    for (int i = 0; i < 3; i++) {
        qi.Push(i);
    }
    EXPECT_EQ(static_cast<unsigned>(3), qi.Size());
    int t = qi.Pop();
    ASSERT_EQ(t, 0);
}

static void ThreadsJoin(std::thread (&threads)[THREAD_NUM])
{
    for (auto& t : threads) {
        t.join();
    }
}

static void CheckFullQueueStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
    unsigned int& unpushedIn, unsigned int& getedOut, unsigned int& ungetedOut)
{
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(pushedIn, THREAD_NUM);
    ASSERT_EQ(getedOut, THREAD_NUM - QUEUE_SLOTS);
}

/*
 * @tc.name: testMutilthreadPutAndBlock001
 * @tc.desc: Multiple threads put until blocking runs, one thread gets, all threads finish running normally
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadPutAndBlock001, TestSize.Level0)
{
    std::thread threads[THREAD_NUM];

    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadData, std::ref(demoDatas[i]), i);
    }

    // 1. queue is full and some threads is blocked
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    unsigned int pushedIn = 0, unpushedIn = 0, getedOut = 0, ungetedOut = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, QUEUE_SLOTS);
    ASSERT_EQ(unpushedIn, THREAD_NUM - QUEUE_SLOTS);

    // 2. get one out  and wait some put in
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Get();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    CheckFullQueueStatus(demoDatas, pushedIn, unpushedIn, getedOut, ungetedOut);
    ThreadsJoin(threads);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }

    // here means all thread end ok or if some operation blocked and the testcase blocked
}

static std::time_t GetTimeAddTwoSeconds()
{
    using std::chrono::system_clock;
    std::time_t timeT = system_clock::to_time_t(system_clock::now());
    cout << "start time: " << timeT << endl;
    const int twoSec = 2;
    timeT += twoSec;
    return timeT;
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInblankqueue001
 * @tc.desc: Multi-threaded put() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    // 2. start thread
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    // 1. queue is full and some threads is blocked
    std::this_thread::sleep_for(std::chrono::seconds(4));
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, QUEUE_SLOTS);
    ASSERT_EQ(unpushedIn, THREAD_NUM - QUEUE_SLOTS);

    // 2. get one out  and wait some put in
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Get();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    CheckFullQueueStatus(demoDatas, pushedIn, unpushedIn, getedOut, ungetedOut);
    ThreadsJoin(threads);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
    // here means all thread end ok or if some operation blocked and the testcase blocked
}

static void QueuePushInfull()
{
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

static void QueuePushInnotfull(const unsigned int remain)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS - remain; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInfullqueue001
 * @tc.desc: Multi-threaded put() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInfull();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    // 2. start thread put in full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    // 3. now thread is running and all is blocked
    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    ASSERT_EQ(pushedIn, static_cast<unsigned int>(0));
    ASSERT_EQ(unpushedIn, THREAD_NUM);
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        cout << "get out one and then the queue is full" << endl;
        DemoThreadData::shareQueue.Pop();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        ASSERT_EQ(pushedIn, i + 1);
        ASSERT_EQ(unpushedIn, THREAD_NUM - (i + 1));
    }

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInblankqueue001
 * @tc.desc: Multi-threaded get() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    // 2. start thread put in empty queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 3. now thread is running and all is blocked
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, static_cast<unsigned int>(0));
    ASSERT_EQ(ungetedOut, THREAD_NUM);
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    int value = 1;

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        cout << "put in one and then the queue is empty" << endl;
        DemoThreadData::shareQueue.Push(value);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        ASSERT_EQ(getedOut, i + 1);
        ASSERT_EQ(ungetedOut, THREAD_NUM - (i + 1));
    }

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}
/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInfullqueue001
 * @tc.desc: Multi-threaded get() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    int t = 1;
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(t);
    }
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    // 2. start thread put in full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(4));
    // 3. start judge
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);

    ASSERT_EQ(getedOut, QUEUE_SLOTS);
    ASSERT_EQ(ungetedOut, THREAD_NUM - QUEUE_SLOTS);

    // 2.  put one in  and wait some get out
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Put(t);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, THREAD_NUM);
    ASSERT_EQ(ungetedOut, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded get() on the notfull queue. When n threads are waiting to reach a certain
 * time-point, everyone get concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInnotfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    const unsigned int REMAIN_SLOTS = 5;
    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInnotfull(REMAIN_SLOTS);

    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, QUEUE_SLOTS - REMAIN_SLOTS);
    ASSERT_EQ(ungetedOut, THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS));

    // 3. put ungetedOut
    for (unsigned int i = 0; i < ungetedOut; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, THREAD_NUM);
    ASSERT_EQ(ungetedOut, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded put() on the not full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInnotfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    const unsigned int REMAIN_SLOTS = 5;
    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInnotfull(REMAIN_SLOTS);

    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    unsigned int putedin = 0;
    unsigned int unputedin = 0;
    GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
    ASSERT_EQ(putedin, REMAIN_SLOTS);
    ASSERT_EQ(unputedin, THREAD_NUM - REMAIN_SLOTS);

    // 3. put ungetedOut
    for (unsigned int i = 0; i < unputedin; i++) {
        DemoThreadData::shareQueue.Pop();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
    ASSERT_EQ(putedin, THREAD_NUM);
    ASSERT_EQ(unputedin, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

static void CheckQueueStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas)
{
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, THREAD_NUM);
    ASSERT_EQ(getedOut, THREAD_NUM);
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInblankqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the empty queue. When all threads are waiting to reach
 * a certain time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndPopInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    std::thread threadsin[THREAD_NUM];

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    // 2. start thread put and get in empty queue

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsout[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsin[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    CheckQueueStatus(demoDatas);

    ThreadsJoin(threadsout);
    ThreadsJoin(threadsin);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInfullqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the full queue.
 * When all threads are waiting to reach a certain
 * time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndPopInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    std::thread threadsin[THREAD_NUM];

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    int t = 1;
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(t);
    }
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsin[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsout[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    CheckQueueStatus(demoDatas);

    ThreadsJoin(threadsout);
    ThreadsJoin(threadsin);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/**
 * @tc.name  : Test AudioBlend  API
 * @tc.type  : FUNC
 * @tc.number: AudioBlend_001
 * @tc.desc  : Test AudioBlend Process API
 */
HWTEST(AudioUtilsUnitTest, AudioBlend_001, TestSize.Level1)
{
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_DEFAULT, SAMPLE_U8, MONO);
    EXPECT_EQ(audioBlend->blendMode_, MODE_DEFAULT);

    audioBlend->SetParams(MODE_BLEND_LR, SAMPLE_U8, MONO);
    EXPECT_EQ(audioBlend->blendMode_, MODE_BLEND_LR);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_001
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is channel 0
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_001, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = 0;
    uint8_t channels = 0;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_DEFAULT, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_002
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,channel 8
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_002, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_U8;
    uint8_t channels = CHANNEL_8;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[7], 15);
    EXPECT_EQ(b[5], 11);
    EXPECT_EQ(b[1], 3);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_003
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,channel 6
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_003, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_U8;
    uint8_t channels = CHANNEL_6;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[5], 11);
    EXPECT_EQ(b[1], 3);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_004
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,channel 4
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_004, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = CHANNEL_4;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[3], 6);
    EXPECT_EQ(b[1], 6);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_005
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,STEREO
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_005, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S32LE;
    uint8_t channels = STEREO;
    ChannelBlendMode blendMode = MODE_BLEND_LR;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 4);
    EXPECT_EQ(b[1], 4);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_006
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_LEFT,channel 8
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_006, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_U8;
    uint8_t channels = CHANNEL_8;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[7], 14);
    EXPECT_EQ(b[5], 10);
    EXPECT_EQ(b[1], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_007
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_LEFT,channel 6
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_007, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_U8;
    uint8_t channels = CHANNEL_6;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[5], 10);
    EXPECT_EQ(b[1], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_008
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_LEFT,channel 4
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_008, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = CHANNEL_4;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[3], 4);
    EXPECT_EQ(b[1], 4);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_009
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_LEFT,STEREO
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_009, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S32LE;
    uint8_t channels = STEREO;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[1], 4);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_010
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_RIGHT,channel 8
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_010, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S16LE;
    uint8_t channels = CHANNEL_8;
    ChannelBlendMode blendMode = MODE_ALL_LEFT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[6], 14);
}

/**
 * @tc.name  : Test audio_channel_blend  API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_011
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_RIGHT,channel 6
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_011, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S24LE;
    uint8_t channels = CHANNEL_6;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_012
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_RIGHT,STEREO
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_012, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S32LE;
    uint8_t channels = CHANNEL_4;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[2], 6);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test audio_channel_blend  API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_013
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_ALL_RIGHT,STEREO
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_013, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_S24LE;
    uint8_t channels = CHANNEL_3;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_014
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is 0 or blendMode is MODE_DEFAULT
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_014, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_DEFAULT, SAMPLE_S24LE, 0);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 2);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 2);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 2);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 2);

    audioBlend->SetParams(MODE_DEFAULT, SAMPLE_S24LE, 1);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 2);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 2);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 2);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_015
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is 1, blendMode is MODE_BLEND_LR, MODE_ALL_LEFT or MODE_ALL_RIGHT.
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_015, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_BLEND_LR, SAMPLE_U8, 1);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 2);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 2);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 2);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 2);

    audioBlend->SetParams(MODE_ALL_LEFT, SAMPLE_U8, 1);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 2);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 2);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 2);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 2);

    audioBlend->SetParams(MODE_ALL_RIGHT, SAMPLE_U8, 1);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 2);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 2);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 2);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_016
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is STEREO, blendMode is MODE_BLEND_LR, MODE_ALL_LEFT or MODE_ALL_RIGHT.
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_016, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_BLEND_LR, 3, STEREO);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_LEFT, 3, STEREO);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_RIGHT, 3, STEREO);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_017
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is CHANNEL_5, blendMode is MODE_BLEND_LR, MODE_ALL_LEFT or MODE_ALL_RIGHT.
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_017, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_BLEND_LR, 0, CHANNEL_5);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[2], 7);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[5], 7);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[8], 7);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[11], 7);

    audioBlend->SetParams(MODE_ALL_LEFT, 0, CHANNEL_5);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[2], 7);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[5], 7);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[8], 7);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[11], 7);

    audioBlend->SetParams(MODE_ALL_RIGHT, 0, CHANNEL_5);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[2], 7);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[5], 7);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[8], 7);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[11], 7);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_017
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is CHANNEL_7, blendMode is MODE_BLEND_LR, MODE_ALL_LEFT or MODE_ALL_RIGHT.
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_018, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_BLEND_LR, 0, CHANNEL_7);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_LEFT, 0, CHANNEL_7);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_RIGHT, 0, CHANNEL_7);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_017
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when channel is CHANNEL_8, blendMode is MODE_BLEND_LR, MODE_ALL_LEFT or MODE_ALL_RIGHT.
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_019, TestSize.Level1)
{
    uint8_t buffer1[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t buffer2[16] = {0, 2, 0, 4, 0, 6, 0, 8, 0, 10, 0, 12, 0, 14, 0, 16};
    uint8_t buffer3[24] = {0, 0, 2, 0, 0, 4, 0, 0, 6, 0, 0, 8, 0, 0, 10, 0, 0, 12, 0, 0, 14, 0, 0, 16};
    uint8_t buffer4[32] =
        {0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 12, 0, 0, 0, 14, 0, 0, 0, 16};
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(MODE_BLEND_LR, 0, CHANNEL_8);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_LEFT, 0, CHANNEL_8);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);

    audioBlend->SetParams(MODE_ALL_RIGHT, 0, CHANNEL_8);
    audioBlend->ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer1), 8);
    EXPECT_EQ(buffer1[0], 3);

    audioBlend->ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer2), 8);
    EXPECT_EQ(buffer2[1], 3);

    audioBlend->ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer3), 8);
    EXPECT_EQ(buffer3[2], 3);

    audioBlend->ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer4), 8);
    EXPECT_EQ(buffer4[3], 3);
}

/**
* @tc.name  : Test SetVolumeRampConfig  API
* @tc.type  : FUNC
* @tc.number: SetVolumeRampConfig_001
* @tc.desc  : Test SetVolumeRampConfig API,
*             when rampDirection_ is RAMP_UP
*/
HWTEST(AudioUtilsUnitTest, SetVolumeRampConfig_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->SetVolumeRampConfig(9.9f, 10.1f, 4);
    EXPECT_EQ(volumeRamp->rampDirection_, RAMP_DOWN);
}

/**
* @tc.name  : Test GetRampVolume  API
* @tc.type  : FUNC
* @tc.number: GetRampVolume_001
* @tc.desc  : Test GetRampVolume API,
*             when ret is 0.0f
*/
HWTEST(AudioUtilsUnitTest, GetRampVolume_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->initTime_ = -1;
    float ret = volumeRamp->GetRampVolume();
    EXPECT_EQ(ret, 0.0f);
}

/**
* @tc.name  : Test GetRampVolume  API
* @tc.type  : FUNC
* @tc.number: GetRampVolume_002
* @tc.desc  : Test GetRampVolume API,
*             when ret is 0.0f
*/
HWTEST(AudioUtilsUnitTest, GetRampVolume_002, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    float ret = volumeRamp->GetRampVolume();
    EXPECT_EQ(ret, 1.0f);
}

/**
* @tc.name  : Test GetScaledTime  API
* @tc.type  : FUNC
* @tc.number: GetScaledTime_001
* @tc.desc  : Test GetScaledTime API,return offset,
*             when offset is 0.0f
*/
HWTEST(AudioUtilsUnitTest, GetScaledTime_001, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->scale_ = 10.0f;
    volumeRamp->initTime_ = 1;
    float ret = volumeRamp->GetScaledTime(1000);
    EXPECT_EQ(ret, 1.0f);
}

/**
* @tc.name  : Test GetScaledTime  API
* @tc.type  : FUNC
* @tc.number: GetScaledTime_002
* @tc.desc  : Test GetScaledTime API,return offset,
*             when offset is 1.0f
*/
HWTEST(AudioUtilsUnitTest, GetScaledTime_002, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->scale_ = -10.0f;
    volumeRamp->initTime_ = 1;
    float ret = volumeRamp->GetScaledTime(1000);
    EXPECT_EQ(ret, 0.0f);
}

/**
* @tc.name  : Test GetScaledTime  API
* @tc.type  : FUNC
* @tc.number: GetScaledTime_003
* @tc.desc  : Test GetScaledTime API,return offset,
*             when offset is 0.0f
*/
HWTEST(AudioUtilsUnitTest, GetScaledTime_003, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->rampDirection_ = RAMP_DOWN;
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->scale_ = 10.0f;
    volumeRamp->initTime_ = 1;
    float ret = volumeRamp->GetScaledTime(1000);
    EXPECT_EQ(ret, 0.0f);
}

/**
* @tc.name  : Test GetScaledTime  API
* @tc.type  : FUNC
* @tc.number: GetScaledTime_004
* @tc.desc  : Test GetScaledTime API,return offset,
*             when offset is 1.0f
*/
HWTEST(AudioUtilsUnitTest, GetScaledTime_004, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->initTime_ = 1;
    volumeRamp->rampDirection_ = RAMP_DOWN;
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->scale_ = -10.0f;
    float ret = volumeRamp->GetScaledTime(1000);
    EXPECT_EQ(ret, 1.0f);
}

/**
* @tc.name  : Test GetScaledTime  API
* @tc.type  : FUNC
* @tc.number: GetScaledTime_004
* @tc.desc  : Test GetScaledTime API,return offset,
*             when offset is 1.0f
*/
HWTEST(AudioUtilsUnitTest, GetScaledTime_005, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->rampDirection_ = RAMP_DOWN;
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->scale_ = -10.0f;
    volumeRamp->initTime_ = 1;
    float ret = volumeRamp->GetScaledTime(1000);
    EXPECT_EQ(ret, 1.0f);
}

/**
* @tc.name  : Test LoadChangeSpeedFunc API
* @tc.type  : FUNC
* @tc.number: LoadChangeSpeedFunc_001
* @tc.desc  : Test LoadChangeSpeedFunc API
*/
HWTEST(AudioUtilsUnitTest, LoadChangeSpeedFunc_001, TestSize.Level1)
{
    shared_ptr<AudioSpeed> audioSpeed = std::make_shared<AudioSpeed>(1, 1, 1);
    audioSpeed->format_ = 0;
    int32_t result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(0, result);
}

/**
* @tc.name  : Test LoadChangeSpeedFunc API
* @tc.type  : FUNC
* @tc.number: LoadChangeSpeedFunc_002
* @tc.desc  : Test LoadChangeSpeedFunc API
*/
HWTEST(AudioUtilsUnitTest, LoadChangeSpeedFunc_002, TestSize.Level1)
{
    shared_ptr<AudioSpeed> audioSpeed = std::make_shared<AudioSpeed>(1, 1, 1);
    audioSpeed->format_ = 1;
    int32_t result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(0, result);
}

/**
* @tc.name  : Test LoadChangeSpeedFunc  API
* @tc.type  : FUNC
* @tc.number: LoadChangeSpeedFunc_003
* @tc.desc  : Test LoadChangeSpeedFunc API
*/
HWTEST(AudioUtilsUnitTest, LoadChangeSpeedFunc_003, TestSize.Level1)
{
    shared_ptr<AudioSpeed> audioSpeed = std::make_shared<AudioSpeed>(1, 1, 1);
    audioSpeed->format_ = -1;
    int32_t result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(0, result);
}

/**
* @tc.name  : Test AbsoluteSleep  API
* @tc.type  : FUNC
* @tc.number: AbsoluteSleep_001
* @tc.desc  : Test AbsoluteSleep API
*/
HWTEST(AudioUtilsUnitTest, AbsoluteSleep_001, TestSize.Level1)
{
    const int64_t CLOCK_TIME = 0;
    int32_t ret = ClockTime::AbsoluteSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);
}

/**
* @tc.name  : Test RelativeSleep  API
* @tc.type  : FUNC
* @tc.number: RelativeSleep_001
* @tc.desc  : Test RelativeSleep API
*/
HWTEST(AudioUtilsUnitTest, RelativeSleep_001, TestSize.Level1)
{
    const int64_t CLOCK_TIME = 0;
    int32_t ret = ClockTime::RelativeSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);
}

/**
* @tc.name  : Test AudioInfoDumpUtils::GetStreamName  API
* @tc.type  : FUNC
* @tc.number: AudioInfoDumpUtils_GetStreamName_016
* @tc.desc  : Test AudioInfoDumpUtils GetStreamName API, Return UNKNOWN
*             when streamType is others
*/
HWTEST(AudioUtilsUnitTest, AudioInfoDumpUtils_GetStreamName_016, TestSize.Level0)
{
    AudioStreamType streamType = STREAM_ACCESSIBILITY;
    const std::string streamName = AudioInfoDumpUtils::GetStreamName(streamType);
    EXPECT_EQ(streamName, "ACCESSIBILITY");
}

/**
* @tc.name  : Test MockPcmData  API
* @tc.type  : FUNC
* @tc.number: MockPcmData_001
* @tc.desc  : Test MockPcmData API
*/
HWTEST(AudioUtilsUnitTest, MockPcmData_001, TestSize.Level1)
{
    uint8_t buffer[0] = {};
    size_t bufferLen = 0;
    int32_t sampleRate = 44100;
    int32_t channelCount = 2;
    int32_t sampleFormat = 16;
    std::string appName = "com.example.null";
    uint32_t sessionId = 0;

    std::shared_ptr<AudioLatencyMeasurement> audioLatencyMeasurement =
        std::make_shared<AudioLatencyMeasurement>(sampleRate, channelCount, sampleFormat, appName, sessionId);

    bool ret = audioLatencyMeasurement->MockPcmData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test MockPcmData  API
* @tc.type  : FUNC
* @tc.number: MockPcmData_002
* @tc.desc  : Test MockPcmData API
*/
HWTEST(AudioUtilsUnitTest, MockPcmData_002, TestSize.Level1)
{
    uint8_t buffer[0] = {};
    size_t bufferLen = 0;
    int32_t sampleRate = 44100;
    int32_t channelCount = 2;
    int32_t sampleFormat = 16;
    std::string appName = "com.example.test";
    uint32_t sessionId = 0;

    std::shared_ptr<AudioLatencyMeasurement> audioLatencyMeasurement =
        std::make_shared<AudioLatencyMeasurement>(sampleRate, channelCount, sampleFormat, appName, sessionId);

    audioLatencyMeasurement->mockedTime_ = 2000;
    bool ret = audioLatencyMeasurement->MockPcmData(buffer, bufferLen);
    EXPECT_EQ(ret, false);

    audioLatencyMeasurement->mockedTime_ = 2000;
    audioLatencyMeasurement->format_ = SAMPLE_S32LE;
    ret = audioLatencyMeasurement->MockPcmData(buffer, bufferLen);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test UpdateClientTime  API
* @tc.type  : FUNC
* @tc.number: UpdateClientTime_001
* @tc.desc  : Test UpdateClientTime API
*/
HWTEST(AudioUtilsUnitTest, UpdateClientTime_001, TestSize.Level1)
{
    bool isRenderer = true;
    std::string Timestamp = "abcdef";
    LatencyMonitor::GetInstance().UpdateClientTime(isRenderer, Timestamp);
}

/**
* @tc.name  : Test UpdateClientTime  API
* @tc.type  : FUNC
* @tc.number: UpdateClientTime_002
* @tc.desc  : Test UpdateClientTime API
*/
HWTEST(AudioUtilsUnitTest, UpdateClientTime_002, TestSize.Level1)
{
    bool isRenderer = false;
    std::string Timestamp = "abcdef";
    LatencyMonitor::GetInstance().UpdateClientTime(isRenderer, Timestamp);
}

/**
* @tc.name  : Test UpdateSinkOrSourceTime  API
* @tc.type  : FUNC
* @tc.number: UpdateSinkOrSourceTime_001
* @tc.desc  : Test UpdateSinkOrSourceTime API
*/
HWTEST(AudioUtilsUnitTest, UpdateSinkOrSourceTime_001, TestSize.Level1)
{
    bool isRenderer = true;
    std::string Timestamp = "abcdef";
    LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(isRenderer, Timestamp);
}

/**
* @tc.name  : Test UpdateSinkOrSourceTime  API
* @tc.type  : FUNC
* @tc.number: UpdateSinkOrSourceTime_002
* @tc.desc  : Test UpdateSinkOrSourceTime API
*/
HWTEST(AudioUtilsUnitTest, UpdateSinkOrSourceTime_002, TestSize.Level1)
{
    bool isRenderer = false;
    std::string Timestamp = "abcdef";
    LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(isRenderer, Timestamp);
}

/**
* @tc.name  : Test ShowTimestamp  API
* @tc.type  : FUNC
* @tc.number: ShowTimestamp_001
* @tc.desc  : Test ShowTimestamp API
*/
HWTEST(AudioUtilsUnitTest, ShowTimestamp_001, TestSize.Level1)
{
    bool isRenderer = false;
    LatencyMonitor::GetInstance().ShowTimestamp(isRenderer);
}

/**
* @tc.name  : Test ShowTimestamp  API
* @tc.type  : FUNC
* @tc.number: ShowTimestamp_002
* @tc.desc  : Test ShowTimestamp API
*/
HWTEST(AudioUtilsUnitTest, ShowTimestamp_002, TestSize.Level1)
{
    bool isRenderer = false;
    LatencyMonitor latencyMonitor = LatencyMonitor::GetInstance();
    latencyMonitor.extraStrLen_ = 1;
    latencyMonitor.UpdateDspTime("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    latencyMonitor.ShowTimestamp(isRenderer);
}

/**
* @tc.name  : Test ShowTimestamp  API
* @tc.type  : FUNC
* @tc.number: ShowTimestamp_003
* @tc.desc  : Test ShowTimestamp API
*/
HWTEST(AudioUtilsUnitTest, ShowTimestamp_003, TestSize.Level1)
{
    bool isRenderer = true;
    LatencyMonitor latencyMonitor = LatencyMonitor::GetInstance();
    latencyMonitor.extraStrLen_ = 1;
    latencyMonitor.ShowTimestamp(isRenderer);
}

/**
* @tc.name  : Test ShowTimestamp  API
* @tc.type  : FUNC
* @tc.number: ShowTimestamp_004
* @tc.desc  : Test ShowTimestamp API
*/
HWTEST(AudioUtilsUnitTest, ShowTimestamp_004, TestSize.Level1)
{
    bool isRenderer = true;
    LatencyMonitor latencyMonitor = LatencyMonitor::GetInstance();
    latencyMonitor.extraStrLen_ = 1;
    latencyMonitor.UpdateDspTime("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    latencyMonitor.ShowTimestamp(isRenderer);
}

/**
* @tc.name  : Test GetVolumeTypeFromStreamType  API
* @tc.type  : FUNC
* @tc.number: GetVolumeTypeFromStreamType_001
* @tc.desc  : Test GetVolumeTypeFromStreamType API
*/
HWTEST(AudioUtilsUnitTest, GetVolumeTypeFromStreamType_001, TestSize.Level1)
{
    AudioStreamType ret = VolumeUtils::GetVolumeTypeFromStreamType(STREAM_BLUETOOTH_SCO);
    EXPECT_EQ(STREAM_MUSIC, ret);
}

/**
* @tc.name  : Test GetEncryptStr  API
* @tc.type  : FUNC
* @tc.number: GetEncryptStr_003
* @tc.desc  : Test GetEncryptStr API
*/
HWTEST(AudioUtilsUnitTest, GetEncryptStr_003, TestSize.Level1)
{
    const std::string src = "abcdefgh";
    std::string dst = GetEncryptStr(src);
    EXPECT_EQ(dst, "ab*defgh");
}

/**
* @tc.name  : Test ConvertNetworkId  API
* @tc.type  : FUNC
* @tc.number: ConvertNetworkId_001
* @tc.desc  : Test ConvertNetworkId API
*/
HWTEST(AudioUtilsUnitTest, ConvertNetworkId_001, TestSize.Level1)
{
    const std::string src = "abcdefgh";
    std::string dst = ConvertNetworkId(src);
    EXPECT_EQ(dst, REMOTE_NETWORK_ID);
}

/**
* @tc.name  : Test ConvertNetworkId  API
* @tc.type  : FUNC
* @tc.number: ConvertNetworkId_002
* @tc.desc  : Test ConvertNetworkId API
*/
HWTEST(AudioUtilsUnitTest, ConvertNetworkId_002, TestSize.Level1)
{
    const std::string src;
    std::string dst = ConvertNetworkId(src);
    EXPECT_EQ(dst, src);
}

/**
* @tc.name  : Test ConvertNetworkId  API
* @tc.type  : FUNC
* @tc.number: ConvertNetworkId_003
* @tc.desc  : Test ConvertNetworkId API
*/
HWTEST(AudioUtilsUnitTest, ConvertNetworkId_003, TestSize.Level1)
{
    const std::string src = LOCAL_NETWORK_ID;
    std::string dst = ConvertNetworkId(src);
    EXPECT_EQ(dst, LOCAL_NETWORK_ID);
}

/**
* @tc.name  : Test CheckCallingUidPermission  API
* @tc.type  : FUNC
* @tc.number: CheckCallingUidPermission_001
* @tc.desc  : Test CheckCallingUidPermission API
*/
HWTEST(AudioUtilsUnitTest, CheckCallingUidPermission_001, TestSize.Level1)
{
    const std::vector<uid_t> allowedUids = {};
    bool result = PermissionUtil::CheckCallingUidPermission(allowedUids);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test CheckCallingUidPermission  API
* @tc.type  : FUNC
* @tc.number: CheckCallingUidPermission_002
* @tc.desc  : Test CheckCallingUidPermission API
*/
HWTEST(AudioUtilsUnitTest, CheckCallingUidPermission_002, TestSize.Level1)
{
    const std::vector<uid_t> allowedUids = {3001, 3003};
    bool result = PermissionUtil::CheckCallingUidPermission(allowedUids);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test UpdateBGSet  API
* @tc.type  : FUNC
* @tc.number: UpdateBGSet_001
* @tc.desc  : Test UpdateBGSet API
*/
HWTEST(AudioUtilsUnitTest, UpdateBGSet_001, TestSize.Level1)
{
    PermissionUtil::UpdateBGSet();

    char ch = '0';
    bool result = PermissionUtil::IsFoldAble(ch);
    EXPECT_EQ(result, false);

    ch = '2';
    result = PermissionUtil::IsFoldAble(ch);
    EXPECT_EQ(result, true);

    ch = '4';
    result = PermissionUtil::IsFoldAble(ch);
    EXPECT_EQ(result, true);

    ch = '5';
    result = PermissionUtil::IsFoldAble(ch);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test NeedVerifyBackgroundCapture  API
* @tc.type  : FUNC
* @tc.number: NeedVerifyBackgroundCapture_001
* @tc.desc  : Test NeedVerifyBackgroundCapture API
*/
HWTEST(AudioUtilsUnitTest, NeedVerifyBackgroundCapture_001, TestSize.Level1)
{
    int32_t callingUid = 6699;
    SourceType sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    bool result = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test NeedVerifyBackgroundCapture  API
* @tc.type  : FUNC
* @tc.number: NeedVerifyBackgroundCapture_002
* @tc.desc  : Test NeedVerifyBackgroundCapture API
*/
HWTEST(AudioUtilsUnitTest, NeedVerifyBackgroundCapture_002, TestSize.Level1)
{
    int32_t callingUid = 7000;
    SourceType sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    bool result = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test NeedVerifyBackgroundCapture  API
* @tc.type  : FUNC
* @tc.number: NeedVerifyBackgroundCapture_003
* @tc.desc  : Test NeedVerifyBackgroundCapture API
*/
HWTEST(AudioUtilsUnitTest, NeedVerifyBackgroundCapture_003, TestSize.Level1)
{
    int32_t callingUid = 7000;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    bool result = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test WriteDumpFile  API
* @tc.type  : FUNC
* @tc.number: WriteDumpFile_001
* @tc.desc  : Test WriteDumpFile API
*/
HWTEST(AudioUtilsUnitTest, WriteDumpFile_001, TestSize.Level1)
{
    uint32_t buffer[10] = {0};
    size_t buffersize = 10;
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::WriteDumpFile(tempFile, buffer, buffersize);
    fclose(tempFile);
}

/**
* @tc.name  : Test ChangeDumpFileState  API
* @tc.type  : FUNC
* @tc.number: ChangeDumpFileState_001
* @tc.desc  : Test ChangeDumpFileState API
*/
HWTEST(AudioUtilsUnitTest, ChangeDumpFileState_001, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_SERVER_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "w";
    SetParameter(key, value);
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::ChangeDumpFileState(para, &tempFile, fileName);
    fclose(tempFile);
}

/**
* @tc.name  : Test ChangeDumpFileState  API
* @tc.type  : FUNC
* @tc.number: ChangeDumpFileState_002
* @tc.desc  : Test ChangeDumpFileState API
*/
HWTEST(AudioUtilsUnitTest, ChangeDumpFileState_002, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_CLIENT_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "a";
    SetParameter(key, value);
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::ChangeDumpFileState(para, &tempFile, fileName);
    fclose(tempFile);
}

/**
* @tc.name  : Test ChangeDumpFileState  API
* @tc.type  : FUNC
* @tc.number: ChangeDumpFileState_003
* @tc.desc  : Test ChangeDumpFileState API
*/
HWTEST(AudioUtilsUnitTest, ChangeDumpFileState_003, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_CLIENT_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "a";
    SetParameter(key, value);
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::ChangeDumpFileState(para, &tempFile, fileName);
    fclose(tempFile);
}

/**
* @tc.name  : Test OpenDumpFile  API
* @tc.type  : FUNC
* @tc.number: OpenDumpFile_001
* @tc.desc  : Test OpenDumpFile API
*/
HWTEST(AudioUtilsUnitTest, OpenDumpFile_001, TestSize.Level1)
{
    std::string para = "w";
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::OpenDumpFile(para, fileName, &tempFile);
    fclose(tempFile);
}

/**
* @tc.name  : Test OpenDumpFile  API
* @tc.type  : FUNC
* @tc.number: OpenDumpFile_002
* @tc.desc  : Test OpenDumpFile API
*/
HWTEST(AudioUtilsUnitTest, OpenDumpFile_002, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_CLIENT_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    FILE* tempFile = std::tmpfile();
    DumpFileUtil::OpenDumpFile(para, fileName, &tempFile);
    fclose(tempFile);
}

/**
* @tc.name  : Test OpenDumpFileInner  API
* @tc.type  : FUNC
* @tc.number: OpenDumpFileInner_001
* @tc.desc  : Test OpenDumpFileInner API
*/
HWTEST(AudioUtilsUnitTest, OpenDumpFileInner_001, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_SERVER_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    FILE* tempFile = std::tmpfile();
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "w";
    SetParameter(key, value);
    AudioDumpFileType fileType = static_cast<AudioDumpFileType>(3);
    FILE *ret = DumpFileUtil::OpenDumpFileInner(para, fileName, fileType);
    ASSERT_TRUE(ret == nullptr);
    fclose(tempFile);
}

/**
* @tc.name  : Test OpenDumpFileInner  API
* @tc.type  : FUNC
* @tc.number: OpenDumpFileInner_002
* @tc.desc  : Test OpenDumpFileInner API
*/
HWTEST(AudioUtilsUnitTest, OpenDumpFileInner_002, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_SERVER_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    FILE* tempFile = std::tmpfile();
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "w";
    SetParameter(key, value);
    DumpFileUtil::OpenDumpFile(para, fileName, &tempFile);
    fclose(tempFile);
}

/**
* @tc.name  : Test OpenDumpFileInner  API
* @tc.type  : FUNC
* @tc.number: OpenDumpFileInner_003
* @tc.desc  : Test OpenDumpFileInner API
*/
HWTEST(AudioUtilsUnitTest, OpenDumpFileInner_003, TestSize.Level1)
{
    std::string para = DumpFileUtil::DUMP_SERVER_PARA;
    std::string fileName = "dump_bluetooth_audiosink.pcm";
    FILE* tempFile = std::tmpfile();
    const char *key = "sys.audio.dump.writeserver.enable";
    const char *value = "a";
    SetParameter(key, value);
    DumpFileUtil::OpenDumpFile(para, fileName, &tempFile);
    fclose(tempFile);
}

/**
* @tc.name  : Test CheckAudioData  API
* @tc.type  : FUNC
* @tc.number: CheckAudioData_001
* @tc.desc  : Test CheckAudioData API
*/
HWTEST(AudioUtilsUnitTest, CheckAudioData_001, TestSize.Level1)
{
    uint8_t buffer[10] = {0};
    size_t bufferLen = 0;
    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.formatByteSize_ = 1;
    signalDetectAgent.sampleFormat_= SAMPLE_S32LE;
    bool ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test CheckAudioData  API
* @tc.type  : FUNC
* @tc.number: CheckAudioData_002
* @tc.desc  : Test CheckAudioData API
*/
HWTEST(AudioUtilsUnitTest, CheckAudioData_002, TestSize.Level1)
{
    uint8_t buffer[10] = {2, 3, 2, 3, 2, 3, 2, 3, 2, 3};
    size_t bufferLen = 10 * sizeof(int32_t);
    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.formatByteSize_ = 1;
    signalDetectAgent.sampleFormat_= SAMPLE_S24LE;
    bool ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test CheckAudioData  API
* @tc.type  : FUNC
* @tc.number: CheckAudioData_003
* @tc.desc  : Test CheckAudioData API
*/
HWTEST(AudioUtilsUnitTest, CheckAudioData_003, TestSize.Level1)
{
    uint8_t buffer[10] = {2, 3, 2, 3, 2, 3, 2, 3, 2, 3};
    size_t bufferLen = 10 * sizeof(int32_t);
    struct SignalDetectAgent signalDetectAgent;
    signalDetectAgent.formatByteSize_ = 1;
    bool ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);

    signalDetectAgent.sampleFormat_ = SAMPLE_S32LE;
    ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);

    signalDetectAgent.sampleFormat_ = SAMPLE_F32LE;
    ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);

    signalDetectAgent.sampleFormat_ = SAMPLE_S24LE;
    ret = signalDetectAgent.CheckAudioData(buffer, bufferLen);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioScopeExit  API
* @tc.type  : FUNC
* @tc.number: AudioScopeExit_001
* @tc.desc  : Test AudioScopeExit API
*/
HWTEST(AudioUtilsUnitTest, AudioScopeExit_001, TestSize.Level1)
{
    MockExe mock;
    InSequence seq;
    EXPECT_CALL(mock, Exe()).Times(1);
    AudioScopeExit scopeExit([&mock] () {
        mock.Exe();
    });
}

/**
* @tc.name  : Test AudioScopeExit  API
* @tc.type  : FUNC
* @tc.number: AudioScopeExit_002
* @tc.desc  : Test AudioScopeExit API
*/
HWTEST(AudioUtilsUnitTest, AudioScopeExit_002, TestSize.Level1)
{
    MockExe mock;
    InSequence seq;
    EXPECT_CALL(mock, Exe()).Times(0);
    AudioScopeExit scopeExit([&mock] () {
        mock.Exe();
    });
    scopeExit.Relase();
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_001
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_001, TestSize.Level1)
{
    AudioSampleFormat format = SAMPLE_U8;

    uint32_t ret = Util::GetSamplePerFrame(format);
    EXPECT_EQ(ret, 1);

    format = SAMPLE_S16LE;
    ret = Util::GetSamplePerFrame(format);
    EXPECT_EQ(ret, 2);

    format = SAMPLE_S24LE;
    ret = Util::GetSamplePerFrame(format);
    EXPECT_EQ(ret, 3);

    format = SAMPLE_S32LE;
    ret = Util::GetSamplePerFrame(format);
    EXPECT_EQ(ret, 4);

    format = INVALID_WIDTH;
    ret = Util::GetSamplePerFrame(format);
    EXPECT_EQ(ret, 2);
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_002
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_002, TestSize.Level1)
{
    std::string funcName = "AudioUtilsUnitTest_002";
    WatchTimeout watchTimeout(funcName);

        watchTimeout.CheckCurrTimeout();
        EXPECT_EQ(watchTimeout.isChecked_, true);

        watchTimeout.timeoutNs_ = 0;
        watchTimeout.CheckCurrTimeout();
        EXPECT_EQ(watchTimeout.isChecked_, true);
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_003
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_003, TestSize.Level1)
{
    int32_t uid = 1003;
    bool ret = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
    EXPECT_EQ(ret, true);

    uid = 1;
    ret = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_004
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_004, TestSize.Level1)
{
    int64_t nanoTime = 0;
    std::string ret = ClockTime::NanoTimeToString(nanoTime);
    EXPECT_EQ(ret, "08:00:00");
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_005
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_005, TestSize.Level1)
{
    std::string tag = "";
    uint32_t timeoutSeconds = 0;
    AudioXCollie audioXCollie(tag, timeoutSeconds, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    audioXCollie.isCanceled_ = false;
    audioXCollie.CancelXCollieTimer();
    EXPECT_EQ(audioXCollie.isCanceled_, true);

    audioXCollie.CancelXCollieTimer();
    EXPECT_EQ(audioXCollie.isCanceled_, true);
}

/**
* @tc.name  : Test AudioUtils API
* @tc.type  : FUNC
* @tc.number: AudioUtilsUnitTest_006
* @tc.desc  : Test AudioUtils API
*/
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_006, TestSize.Level1)
{
    bool ret = PermissionUtil::VerifyIsShell();
    EXPECT_EQ(ret, true);

    ret = PermissionUtil::VerifyIsAudio();
    EXPECT_EQ(ret, true);

    int32_t callingUid = 6699;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    ret = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(ret, false);

    callingUid = 1000;
    sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    ret = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(ret, false);

    callingUid = 1000;
    sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    ret = PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
    EXPECT_EQ(ret, true);

    uint32_t tokenId = 1;
    uint64_t fullTokenId = 1;
    ret = PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioUtils API
 * @tc.type  : FUNC
 * @tc.number: AudioUtilsUnitTest_007
 * @tc.desc  : Test AudioUtils API
 */
HWTEST(AudioUtilsUnitTest, AudioUtilsUnitTest_007, TestSize.Level1)
{
    std::string str1 = "-0";
    uint64_t result64 = 1;
    EXPECT_TRUE(StringConverter(str1, result64));
    EXPECT_EQ(result64, 0);

    uint32_t result32 = 1;
    EXPECT_TRUE(StringConverter(str1, result32));
    EXPECT_EQ(result32, 0);

    uint16_t result16 = 1;
    EXPECT_TRUE(StringConverter(str1, result16));
    EXPECT_EQ(result16, 0);

    uint8_t result8 = 1;
    EXPECT_TRUE(StringConverter(str1, result8));
    EXPECT_EQ(result8, 0);

    int32_t result32Signed = 1;
    EXPECT_TRUE(StringConverter(str1, result32Signed));
    EXPECT_EQ(result32Signed, 0);

    int8_t result8Signed = 1;
    EXPECT_TRUE(StringConverter(str1, result8Signed));
    EXPECT_EQ(result8Signed, 0);

    std::string str2 = "10";
    EXPECT_TRUE(StringConverter(str2, result64));
    EXPECT_EQ(result64, 10);

    EXPECT_TRUE(StringConverter(str2, result32));
    EXPECT_EQ(result32, 10);

    EXPECT_TRUE(StringConverter(str2, result16));
    EXPECT_EQ(result16, 10);

    EXPECT_TRUE(StringConverter(str2, result8));
    EXPECT_EQ(result8, 10);

    EXPECT_TRUE(StringConverter(str2, result32Signed));
    EXPECT_EQ(result32Signed, 10);

    EXPECT_TRUE(StringConverter(str2, result8Signed));
    EXPECT_EQ(result8Signed, 10);
}

/**
* @tc.name  : Test GetSupportedAudioVolumeTypes  API
* @tc.type  : FUNC
* @tc.number: GetSupportedAudioVolumeTypes_001
* @tc.desc  : Test GetSupportedAudioVolumeTypes API
*/
HWTEST(AudioUtilsUnitTest, GetSupportedAudioVolumeTypes_001, TestSize.Level1)
{
    auto ret = VolumeUtils::GetSupportedAudioVolumeTypes();
    EXPECT_GE(ret.size(), 0);
}

/**
* @tc.name  : Test GetStreamUsagesByVolumeType  API
* @tc.type  : FUNC
* @tc.number: GetStreamUsagesByVolumeType_001
* @tc.desc  : Test GetStreamUsagesByVolumeType API
*/
HWTEST(AudioUtilsUnitTest, GetStreamUsagesByVolumeType_001, TestSize.Level1)
{
    auto ret = VolumeUtils::GetStreamUsagesByVolumeType(STREAM_MUSIC);
    EXPECT_GE(ret.size(), 0);
}
} // namespace AudioStandard
} // namespace OHOS
