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

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr uint32_t NUM_TWO = 2;

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

TEST_F(HpaePcmProcessTest, constructHpaePcmProcess)
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
    EXPECT_EQ(&testData[0], hpaePcmProcess.begin());
    EXPECT_EQ(&testData[TEST_FREAME_LEN - 1], hpaePcmProcess.end() - 1);
}

TEST_F(HpaePcmProcessTest, assignHpaeProcessTest)
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

TEST_F(HpaePcmProcessTest, calHpaeProcessTest)
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
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS