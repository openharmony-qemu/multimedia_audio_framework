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

class SimdPointByPointAddTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // 验证两个数组是否相等
    void verifyArraysEqual(const std::vector<float>& expected, const std::vector<float>& actual, float epsilon = 1e-6f) {
        ASSERT_EQ(expected.size(), actual.size());
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_NEAR(expected[i], actual[i], epsilon) << "at index " << i;
        }
    }
};

// 测试用例1: 基本功能测试 - 小数组（小于ALIGIN_FLOAT_SIZE）
TEST_F(SimdPointByPointAddTest, SmallArray) {
    const size_t length = 3;
    std::vector<float> left = {1.0f, 2.0f, 3.0f};
    std::vector<float> right = {4.0f, 5.0f, 6.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {5.0f, 7.0f, 9.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例2: 基本功能测试 - 中等数组（等于ALIGIN_FLOAT_SIZE）
TEST_F(SimdPointByPointAddTest, MediumArrayExactAlignment) {
    const size_t length = 4;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> right = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {6.0f, 8.0f, 10.0f, 12.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例3: 基本功能测试 - 大数组（ALIGIN_FLOAT_SIZE的倍数）
TEST_F(SimdPointByPointAddTest, LargeArrayAligned) {
    const size_t length = 8;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例4: 边界情况 - 长度不是ALIGIN_FLOAT_SIZE的倍数
TEST_F(SimdPointByPointAddTest, LargeArrayUnaligned) {
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例5: 边界情况 - 长度为0
TEST_F(SimdPointByPointAddTest, ZeroLength) {
    const size_t length = 0;
    std::vector<float> left = {1.0f, 2.0f};
    std::vector<float> right = {3.0f, 4.0f};
    std::vector<float> output = {5.0f, 6.0f};
    
    // 应该不会修改output数组
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    verifyArraysEqual(originalOutput, output);
}

// 测试用例6: 边界情况 - 长度为1
TEST_F(SimdPointByPointAddTest, SingleElement) {
    const size_t length = 1;
    std::vector<float> left = {42.5f};
    std::vector<float> right = {17.3f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {59.8f};
    verifyArraysEqual(expected, output);
}

// 测试用例7: 特殊值测试 - 包含零和负数
TEST_F(SimdPointByPointAddTest, SpecialValues) {
    const size_t length = 6;
    std::vector<float> left = {0.0f, -1.0f, 1.0f, 100.0f, -100.0f, 0.5f};
    std::vector<float> right = {0.0f, 1.0f, -1.0f, -50.0f, 50.0f, 0.5f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {0.0f, 0.0f, 0.0f, 50.0f, -50.0f, 1.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例8: 空指针测试 - inputLeft为nullptr
TEST_F(SimdPointByPointAddTest, NullInputLeft) {
    const size_t length = 5;
    std::vector<float> right = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, nullptr, right.data(), output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

// 测试用例9: 空指针测试 - inputRight为nullptr
TEST_F(SimdPointByPointAddTest, NullInputRight) {
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> output(length, 0.0f);
    
    // 应该不会崩溃，也不会修改output
    std::vector<float> originalOutput = output;
    
    SimdPointByPointAdd(length, left.data(), nullptr, output.data());
    
    // 验证output没有被修改
    verifyArraysEqual(originalOutput, output);
}

// 测试用例10: 空指针测试 - output为nullptr
TEST_F(SimdPointByPointAddTest, NullOutput) {
    const size_t length = 5;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<float> right = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    
    // 应该不会崩溃
    SimdPointByPointAdd(length, left.data(), right.data(), nullptr);
    
    // 测试通过，没有崩溃
    SUCCEED();
}

// 测试用例11: 所有指针都为nullptr
TEST_F(SimdPointByPointAddTest, AllNullPointers) {
    const size_t length = 5;
    
    // 应该不会崩溃
    SimdPointByPointAdd(length, nullptr, nullptr, nullptr);
    
    // 测试通过，没有崩溃
    SUCCEED();
}

// 测试用例12: 简单大数组测试
TEST_F(SimdPointByPointAddTest, SimpleLargeArray) {
    const size_t length = 10;
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    std::vector<float> right = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {11.0f, 22.0f, 33.0f, 44.0f, 55.0f, 66.0f, 77.0f, 88.0f, 99.0f, 110.0f};
    verifyArraysEqual(expected, output);
}


} // HPAE
} // AudioStandard
} // OHOS