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
#include <cmath>
#include "hpae_pcm_process.h"
#include "test_case_common.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr uint32_t NUM_TWO = 2;
constexpr int ALIGIN_FLOAT_SIZE = 4;

class HpaePcmProcessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaePcmProcessTest::SetUp()
{}

void HpaePcmProcessTest::TearDown()
{}

std::aligned_storage<sizeof(float), alignof(float)> aligned_memory;

HWTEST_F(HpaePcmProcessTest, constructHpaePcmProcess, TestSize.Level0)
{
    std::vector<float> testData(TEST_FREAME_LEN);
    HpaePcmProcess hpaePcmProcess(testData.data(), TEST_FREAME_LEN);
    EXPECT_EQ(hpaePcmProcess.Size(), TEST_FREAME_LEN);
    for (int i = 0; i < TEST_FREAME_LEN; i++) {
        testData[i] = i;
        EXPECT_EQ(hpaePcmProcess[i], i);
        const float testValue = hpaePcmProcess[i];
        EXPECT_EQ(hpaePcmProcess[i], testValue);
    }
    EXPECT_EQ(&testData[0], hpaePcmProcess.Begin());
    EXPECT_EQ(&testData[TEST_FREAME_LEN - 1], hpaePcmProcess.End() - 1);
}

HWTEST_F(HpaePcmProcessTest, assignHpaeProcessTest, TestSize.Level0)
{
    std::vector<float> testData(TEST_FREAME_LEN);
    for (int i = 0; i < TEST_FREAME_LEN; i++) {
        testData[i] = i;
    }
    std::vector<float> testData2(TEST_SUB_FREAME_LEN);
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        testData2[i] = i;
    }
    std::vector<float> tmpData(TEST_FREAME_LEN);
    for (int i = 0; i < TEST_FREAME_LEN; i++) {
        tmpData[i] = i;
    }
    HpaePcmProcess tmpPcmProcess(tmpData.data(), TEST_FREAME_LEN);
    std::vector<float> pcmData(TEST_SUB_FREAME_LEN);
    std::vector<float> pcmData2(TEST_FREAME_LEN);
    HpaePcmProcess hpaePcmProcessTest(pcmData.data(), TEST_SUB_FREAME_LEN);
    hpaePcmProcessTest = testData;
    // errcase
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest[i], 0);
    }
    hpaePcmProcessTest = testData2;
    // normalcase
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest[i], i);
    }
    HpaePcmProcess hpaePcmProcessTest2(pcmData2.data(), TEST_SUB_FREAME_LEN);
    hpaePcmProcessTest2 = tmpPcmProcess;
    // errcase
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], 0);
    }
    hpaePcmProcessTest2 = hpaePcmProcessTest;
    // normalcase
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], i);
    }
}

HWTEST_F(HpaePcmProcessTest, calHpaeProcessTest, TestSize.Level0)
{
    std::vector<float> testData(TEST_SUB_FREAME_LEN);
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        testData[i] = i;
    }
    HpaePcmProcess hpaePcmProcessTest(testData.data(), TEST_SUB_FREAME_LEN);
    std::vector<float> testData2(TEST_SUB_FREAME_LEN, NUM_TWO);
    HpaePcmProcess hpaePcmProcessTest2(testData2.data(), TEST_SUB_FREAME_LEN);
    hpaePcmProcessTest2 += hpaePcmProcessTest;
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], i + NUM_TWO);
    }
    hpaePcmProcessTest2 -= hpaePcmProcessTest;
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], NUM_TWO);
    }
    hpaePcmProcessTest2 *= hpaePcmProcessTest;
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], NUM_TWO * i);
    }
    hpaePcmProcessTest2.Reset();
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], 0);
    }
}

HWTEST_F(HpaePcmProcessTest, selfAssignHpaeProcessTest, TestSize.Level0)
{
    std::vector<float> testData(TEST_SUB_FREAME_LEN);
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        testData[i] = i + 1;
    }
    
    HpaePcmProcess hpaePcmProcessTest(testData.data(), TEST_SUB_FREAME_LEN);
    
    std::vector<float> originalData;
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        originalData.push_back(hpaePcmProcessTest[i]);
    }
    
    HpaePcmProcess& hpaePcmProcessTestRef = hpaePcmProcessTest;
    hpaePcmProcessTest = hpaePcmProcessTestRef;
    
    for (int i = 0; i < TEST_SUB_FREAME_LEN; i++) {
        EXPECT_EQ(hpaePcmProcessTest[i], originalData[i]);
        EXPECT_EQ(hpaePcmProcessTest[i], i + 1);
    }
    
    EXPECT_EQ(hpaePcmProcessTest.Begin(), testData.data());
    EXPECT_EQ(hpaePcmProcessTest.End(), testData.data() + TEST_SUB_FREAME_LEN);
}

HWTEST_F(HpaePcmProcessTest, smallArrayOperations, TestSize.Level0)
{
    // Only test lengths smaller than ALIGIN_FLOAT_SIZE (0,1,2,3)
    const size_t testLengths[] = {0, 1, 2};
    for (auto len : testLengths) {
        // ================== Addition Test ==================
        {
            // Initialize objects with fixed size data
            std::vector<float> leftData(len, 1.5f);
            std::vector<float> rightData(len, 2.5f);
            HpaePcmProcess leftObj(leftData.data(), len);
            HpaePcmProcess rightObj(rightData.data(), len);
            // Force operation on small arrays
            leftObj += rightObj;
            // Verify all elements
            for (size_t i = 0; i < len; i++) {
                EXPECT_FLOAT_EQ(leftObj[i], 4.0f);
            }
        }
        // ================== Subtraction Test ==================
        {
            std::vector<float> leftData(len, 5.0f);
            std::vector<float> rightData(len, 3.0f);
            HpaePcmProcess leftObj(leftData.data(), len);
            HpaePcmProcess rightObj(rightData.data(), len);
            leftObj -= rightObj;
            for (size_t i = 0; i < len; i++) {
                EXPECT_FLOAT_EQ(leftObj[i], 2.0f);
            }
        }
        // ================== Multiplication Test ==================
        {
            std::vector<float> leftData(len, 3.0f);
            std::vector<float> rightData(len, 1.5f);
            
            HpaePcmProcess leftObj(leftData.data(), len);
            HpaePcmProcess rightObj(rightData.data(), len);
            leftObj *= rightObj;
            for (size_t i = 0; i < len; i++) {
                EXPECT_FLOAT_EQ(leftObj[i], 4.5f);
            }
        }
    }
}

HWTEST_F(HpaePcmProcessTest, multiplicationExactAlignment, TestSize.Level0)
{
    // Test lengths that are multiples of ALIGIN_FLOAT_SIZE
    const size_t testLen = ALIGIN_FLOAT_SIZE * 4;  // 16 elements
    // Prepare data with unique values for each element
    std::vector<float> leftData(testLen);
    std::vector<float> rightData(testLen);
    std::vector<float> expected(testLen);
    for (size_t i = 0; i < testLen; i++) {
        leftData[i] = 1.0f + i * 0.1f;
        rightData[i] = 2.0f + i * 0.1f;
        expected[i] = leftData[i] * rightData[i];
    }
    HpaePcmProcess leftObj(leftData.data(), testLen);
    HpaePcmProcess rightObj(rightData.data(), testLen);
    // Execute multiplication operation
    leftObj *= rightObj;
    // Verify all elements
    for (size_t i = 0; i < testLen; i++) {
        EXPECT_FLOAT_EQ(leftObj[i], expected[i]);
    }
}

HWTEST_F(HpaePcmProcessTest, zeroRemainderEdgeCases, TestSize.Level0)
{
    // Test multiple exact alignment cases
    const size_t multipliers[] = {1, 2, 3};  // x4, x8, x12
    const float testValue = 0.0f;
    
    for (auto multiplier : multipliers) {
        const size_t testLen = ALIGIN_FLOAT_SIZE * multiplier;
        
        std::vector<float> leftVec(testLen, testValue);
        std::vector<float> rightVec(testLen, testValue);
        
        HpaePcmProcess leftObj(leftVec.data(), testLen);
        HpaePcmProcess rightObj(rightVec.data(), testLen);
        
        // Test all operations
        leftObj += rightObj;
        for (size_t i = 0; i < testLen; i++) {
            EXPECT_FLOAT_EQ(leftObj[i], testValue);
        }
        
        leftObj -= rightObj;
        for (size_t i = 0; i < testLen; i++) {
            EXPECT_FLOAT_EQ(leftObj[i], testValue);
        }
        
        leftObj *= rightObj;
        for (size_t i = 0; i < testLen; i++) {
            EXPECT_FLOAT_EQ(leftObj[i], 0.0f);
        }
    }
}

HWTEST_F(HpaePcmProcessTest, hpaeProcessTestShortLen, TestSize.Level0)
{
    std::vector<float> testData(TEST_LEN_LT_FOUR);
    for (int i = 0; i < TEST_LEN_LT_FOUR; i++) {
        testData[i] = i;
    }
    HpaePcmProcess hpaePcmProcessTest(testData.data(), TEST_LEN_LT_FOUR);
    std::vector<float> testData2(TEST_LEN_LT_FOUR, NUM_TWO);
    HpaePcmProcess hpaePcmProcessTest2(testData2.data(), TEST_LEN_LT_FOUR);
    hpaePcmProcessTest2 += hpaePcmProcessTest;
    for (int i = 0; i < TEST_LEN_LT_FOUR; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], i + NUM_TWO);
    }
    hpaePcmProcessTest2 -= hpaePcmProcessTest;
    for (int i = 0; i < TEST_LEN_LT_FOUR; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], NUM_TWO);
    }
    hpaePcmProcessTest2 *= hpaePcmProcessTest;
    for (int i = 0; i < TEST_LEN_LT_FOUR; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], NUM_TWO * i);
    }
    hpaePcmProcessTest2.Reset();
    for (int i = 0; i < TEST_LEN_LT_FOUR; i++) {
        EXPECT_EQ(hpaePcmProcessTest2[i], 0);
    }
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS