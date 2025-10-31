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

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

#define ALIGIN_FLOAT_SIZE 4

class SimdPointByPointAddTest : public ::testing::Test {
protected:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    // 生成随机浮点数数组
    std::vector<float> generateRandomArray(size_t size, float min = -100.0f, float max = 100.0f) {
        std::vector<float> arr(size);
        for (size_t i = 0; i < size; i++) {
            arr[i] = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(max-min)));
        }
        return arr;
    }

    // 验证两个数组是否相等
    void verifyArraysEqual(const std::vector<float>& expected, const std::vector<float>& actual, float epsilon = 1e-6f) {
        ASSERT_EQ(expected.size(), actual.size());
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_NEAR(expected[i], actual[i], epsilon) << "at index " << i;
        }
    }

    // 参考实现，用于验证正确性
    std::vector<float> referenceAdd(const std::vector<float>& left, const std::vector<float>& right) {
        std::vector<float> result(left.size());
        for (size_t i = 0; i < left.size(); i++) {
            result[i] = left[i] + right[i];
        }
        return result;
    }
};

TEST_F(SimdPointByPointAddTest, SmallArray) {
    const size_t length = 3; // 小于ALIGIN_FLOAT_SIZE
    std::vector<float> left = {1.0f, 2.0f, 3.0f};
    std::vector<float> right = {4.0f, 5.0f, 6.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {5.0f, 7.0f, 9.0f};
    verifyArraysEqual(expected, output);
}

// 测试用例2: 基本功能测试 - 中等数组（等于ALIGIN_FLOAT_SIZE）
TEST_F(SimdPointByPointAddTest, MediumArrayExactAlignment) {
    const size_t length = 4; // 等于ALIGIN_FLOAT_SIZE
    std::vector<float> left = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> right = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> output(length, 0.0f);
    
    SimdPointByPointAdd(length, left.data(), right.data(), output.data());
    
    std::vector<float> expected = {6.0f, 8.0f, 10.0f, 12.0f};
    verifyArraysEqual(expected, output);
}


} // HPAE
} // AudioStandard
} // OHOS