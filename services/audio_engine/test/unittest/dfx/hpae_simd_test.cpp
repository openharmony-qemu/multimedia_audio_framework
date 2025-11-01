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
#include <vector>
#include <cstdlib>
#include <cstring>

#include "simd_utils.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

#define ALIGIN_FLOAT_SIZE 4

class SimdPointByPointTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // 验证两个数组是否相等
    void verifyArraysEqual(const std::vector<float>& expected, const std::vector<float>& actual, float eps = 1e-6f)
    {
        ASSERT_EQ(expected.size(), actual.size());
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_NEAR(expected[i], actual[i], eps) << "at index " << i;
        }
    }
};

/**
 * @tc.name  : Test small vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_001
 * @tc.desc  : test small size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_001, TestSize.Level1)
{
    const size_t length = 3;
    std::vector<float> left = {1.0f, 2.0f, 3.0f};
    std::vector<float> right = {4.0f, 5.0f, 6.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {5.0f, 7.0f, 9.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test mid vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_002
 * @tc.desc  : test medium size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_002, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> right = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {6.0f, 8.0f, 10.0f, 12.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test big vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_003
 * @tc.desc  : test big size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_003, TestSize.Level1)
{
    const size_t length = 8;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test bound vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_004
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_004, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test bound vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_005
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_005, TestSize.Level1)
{
    const size_t length = 0;
    std::vector<float> left = {1.0f, 2.0f};
    std::vector<float> right = {3.0f, 4.0f};
    std::vector<float> output = {5.0f, 6.0f};
    
    // 应该不会修改output数组
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test bound vec
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_006
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_006, TestSize.Level1)
{
    const size_t length = 1;
    std::vector<float> left = {42.5f};
    std::vector<float> right = {17.3f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {59.8f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_007
 * @tc.desc  : test vector with critical value
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_007, TestSize.Level1)
{
    const size_t length = 6;
    std::vector<float> left = {0.0f, -1.0f, 1.0f, 100.0f, -100.0f, 0.5f};
    std::vector<float> right = {0.0f, 1.0f, -1.0f, -50.0f, 50.0f, 0.5f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, 0.0f, 0.0f, 50.0f, -50.0f, 1.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_008
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_008, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> right = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, nullptr, right.data(), output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_009
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_009, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, left.data(), nullptr, output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_010
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_010, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    
    // 应该不会崩溃
    SimdPointByPointAdd(length, left.data(), right.data(), nullptr);
    
    // 测试通过，没有崩溃
    SUCCEED();
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_011
 * @tc.desc  : test all vectors are nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_011, TestSize.Level1)
{
    const size_t length = 5;
    
    // 应该不会崩溃
    SimdPointByPointAdd(length, nullptr, nullptr, nullptr);
    
    // 测试通过，没有崩溃
    SUCCEED();
}

/**
 * @tc.name  : Test vec with sp val
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_012
 * @tc.desc  : test all vectors are nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_012, TestSize.Level1)
{
    const size_t length = 10;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f, 99.0f, 110.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_013
 * @tc.desc  : test small size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_013, TestSize.Level1)
{
    const size_t length = 3;
    std::vector<float> left = {10.0f, 8.0f, 6.0f};
    std::vector<float> right = {4.0f, 5.0f, 3.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {6.0f, 3.0f, 3.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_014
 * @tc.desc  : test mid size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_014, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {10.0f, 9.0f, 8.0f, 7.0f};
    std::vector<float> right = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {9.0f, 7.0f, 5.0f, 3.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_015
 * @tc.desc  : test big size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_015, TestSize.Level1)
{
    const size_t length = 8;
    std::vector<float> left = {100.0f, 90.0f, 80.0f, 70.0f, 60.0f, 50.0f, 40.0f, 30.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 10.0f, 20.0f, 10.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {90.0f, 70.0f, 50.0f, 30.0f, 10.0f, 40.0f, 20.0f, 20.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_016
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_016, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {50.0f, 40.0f, 30.0f, 20.0f, 10.0f};
    std::vector<float> right = {10.0f, 5.0f, 15.0f, 25.0f, 35.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {40.0f, 35.0f, 15.0f, -5.0f, -25.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_017
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_017, TestSize.Level1)
{
    const size_t length = 0;
    std::vector<float> left = {1.0f, 2.0f};
    std::vector<float> right = {3.0f, 4.0f};
    std::vector<float> output = {5.0f, 6.0f};
    
    // 应该不会修改output数组
    std::vector<float> originalOutput = output;
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_018
 * @tc.desc  : test boundary size vector
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_018, TestSize.Level1)
{
    const size_t length = 1;
    std::vector<float> left = {42.5f};
    std::vector<float> right = {17.3f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {25.2f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_019
 * @tc.desc  : test vector with critical values
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_019, TestSize.Level1)
{
    const size_t length = 6;
    std::vector<float> left = {0.0f, -1.0f, 1.0f, 100.0f, -100.0f, 0.5f};
    std::vector<float> right = {0.0f, 1.0f, -1.0f, 50.0f, -50.0f, 0.5f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, -2.0f, 2.0f, 50.0f, -50.0f, 0.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_020
 * @tc.desc  : test vector with critical values
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_020, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {5.0f, 3.0f, 1.0f, 0.0f};
    std::vector<float> right = {10.0f, 5.0f, 2.0f, 1.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {-5.0f, -2.0f, -1.0f, -1.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_021
 * @tc.desc  : test vector with critical values
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_021, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> right = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointSub(length, nullptr, right.data(), output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_022
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_022, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointSub(length, left.data(), nullptr, output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_023
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_023, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    
    // 应该不会崩溃
    SimdPointByPointSub(length, left.data(), right.data(), nullptr);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_024
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_024, TestSize.Level1)
{
    const size_t length = 5;
    
    // 应该不会崩溃
    SimdPointByPointSub(length, nullptr, nullptr, nullptr);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_025
 * @tc.desc  : test vector with big size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_025, TestSize.Level1)
{
    const size_t length = 10;
    std::vector<float> left = {100.0f, 90.0f, 80.0f, 70.0f, 60.0f, 50.0f, 40.0f, 30.0f, 20.0f, 10.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 10.0f, 20.0f, 10.0f, 5.0f, 2.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {90.0f, 70.0f, 50.0f, 30.0f, 10.0f, 40.0f, 20.0f, 20.0f, 15.0f, 8.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec sub
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_026
 * @tc.desc  : test vector with sp value
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_026, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {5.0f, 10.0f, 15.0f, 20.0f};
    std::vector<float> right = {5.0f, 10.0f, 15.0f, 20.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointSub(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, 0.0f, 0.0f, 0.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_027
 * @tc.desc  : test vector with small size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_027, TestSize.Level1)
{
    const size_t length = 3;
    std::vector<float> left = {2.0f, 3.0f, 4.0f};
    std::vector<float> right = {5.0f, 6.0f, 7.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {10.0f, 18.0f, 28.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_028
 * @tc.desc  : test vector with mid size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_028, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> right = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {5.0f, 12.0f, 21.0f, 32.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_029
 * @tc.desc  : test vector with big size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_029, TestSize.Level1)
{
    const size_t length = 8;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> right = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {2.0f, 6.0f, 12.0f, 20.0f, 30.0f, 42.0f, 56.0f, 72.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_030
 * @tc.desc  : test vector with big size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_030, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    std::vector<float> right = {7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {14.0f, 24.0f, 36.0f, 50.0f, 66.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_031
 * @tc.desc  : test vector with zero size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_031, TestSize.Level1)
{
    const size_t length = 0;
    std::vector<float> left = {1.0f, 2.0f};
    std::vector<float> right = {3.0f, 4.0f};
    std::vector<float> output = {5.0f, 6.0f};
    
    // 应该不会修改output数组
    std::vector<float> originalOutput = output;
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_032
 * @tc.desc  : test vector with critical size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_032, TestSize.Level1)
{
    const size_t length = 1;
    std::vector<float> left = {4.5f};
    std::vector<float> right = {2.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {9.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_033
 * @tc.desc  : test vector with critical size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_033, TestSize.Level1)
{
    const size_t length = 6;
    std::vector<float> left = {0.0f, -1.0f, 2.0f, -3.0f, 4.0f, -5.0f};
    std::vector<float> right = {5.0f, 2.0f, -3.0f, 4.0f, -1.0f, 0.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, -2.0f, -6.0f, -12.0f, -4.0f, 0.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_034
 * @tc.desc  : test vector with critical val
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_034, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {0.5f, 1.5f, 2.5f, 0.25f};
    std::vector<float> right = {2.0f, 3.0f, 4.0f, 8.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {1.0f, 4.5f, 10.0f, 2.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_035
 * @tc.desc  : test vector with critical val
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_035, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {5.0f, -3.0f, 7.0f, -9.0f};
    std::vector<float> right = {1.0f, -1.0f, 1.0f, -1.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {5.0f, 3.0f, 7.0f, 9.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_036
 * @tc.desc  : test vector with critical val
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_036, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> right = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointMul(length, nullptr, right.data(), output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_037
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_037, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointMul(length, left.data(), nullptr, output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_038
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_038, TestSize.Level1)
{
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    
    // 应该不会崩溃
    SimdPointByPointMul(length, left.data(), right.data(), nullptr);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_039
 * @tc.desc  : test vector with nullptr
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_039, TestSize.Level1)
{
    const size_t length = 5;
    
    // 应该不会崩溃
    SimdPointByPointMul(length, nullptr, nullptr, nullptr);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_040
 * @tc.desc  : test vector with big size
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_040, TestSize.Level1)
{
    const size_t length = 10;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    std::vector<float> right = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {2.0f, 6.0f, 12.0f, 20.0f, 30.0f, 42.0f, 56.0f, 72.0f, 90.0f, 110.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_041
 * @tc.desc  : test vector with critical val
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_041, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {5.0f, 10.0f, 15.0f, 20.0f};
    std::vector<float> right = {0.0f, 0.0f, 0.0f, 0.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, 0.0f, 0.0f, 0.0f};
    verifyArraysEqual(expected, output);
}

/**
 * @tc.name  : Test vec mul
 * @tc.type  : FUNC
 * @tc.number: SimdPointByPoint_041
 * @tc.desc  : test vector with critical val
 */
HWTEST_F(SimdPointByPointTest, SimdPointByPoint_042, TestSize.Level1)
{
    const size_t length = 4;
    std::vector<float> left = {-2.0f, -3.0f, -4.0f, -5.0f};
    std::vector<float> right = {-6.0f, 7.0f, -8.0f, 9.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointMul(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {12.0f, -21.0f, 32.0f, -45.0f};
    verifyArraysEqual(expected, output);
}

} // HPAE
} // AudioStandard
} // OHOS