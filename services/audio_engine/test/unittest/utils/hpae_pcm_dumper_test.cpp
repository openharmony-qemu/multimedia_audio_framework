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

#include "gtest/gtest.h"
#include "hpae_pcm_dumper.h"
#include "audio_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static const std::string TEST_DUMP_FILENAME = "/data/test_audio_dump.pcm";
constexpr int32_t TEST_BUFFER_SIZE = 1024;

class HpaePcmDumperTest : public ::testing::Test {
protected:
    void SetUp();
    void TearDown();
};

void HpaePcmDumperTest::SetUp()
{}

void HpaePcmDumperTest::TearDown()
{}

/**
 * @tc.name    : Construct_001
 * @tc.type    : FUNC
 * @tc.number  : Construct_001
 * @tc.desc    : Test HpaePcmDumper construction with valid filename.
 */
HWTEST_F(HpaePcmDumperTest, Construct_001, TestSize.Level0)
{
    HpaePcmDumper dumper(TEST_DUMP_FILENAME);
    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : Construct_002
 * @tc.type    : FUNC
 * @tc.number  : Construct_002
 * @tc.desc    : Test HpaePcmDumper construction with empty filename.
 */
HWTEST_F(HpaePcmDumperTest, Construct_002, TestSize.Level0)
{
    HpaePcmDumper dumper("");
    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : Dump_001
 * @tc.type    : FUNC
 * @tc.number  : Dump_001
 * @tc.desc    : Test Dump with valid buffer and length, returns SUCCESS.
 */
HWTEST_F(HpaePcmDumperTest, Dump_001, TestSize.Level0)
{
    HpaePcmDumper dumper(TEST_DUMP_FILENAME);
    int8_t buffer[TEST_BUFFER_SIZE] = {0};
    int32_t result = dumper.Dump(buffer, TEST_BUFFER_SIZE);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name    : Dump_002
 * @tc.type    : FUNC
 * @tc.number  : Dump_002
 * @tc.desc    : Test Dump with null buffer, returns SUCCESS without crash.
 */
HWTEST_F(HpaePcmDumperTest, Dump_002, TestSize.Level0)
{
    HpaePcmDumper dumper(TEST_DUMP_FILENAME);
    int32_t result = dumper.Dump(nullptr, TEST_BUFFER_SIZE);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name    : Dump_003
 * @tc.type    : FUNC
 * @tc.number  : Dump_003
 * @tc.desc    : Test Dump with zero length, returns SUCCESS without crash.
 */
HWTEST_F(HpaePcmDumperTest, Dump_003, TestSize.Level0)
{
    HpaePcmDumper dumper(TEST_DUMP_FILENAME);
    int8_t buffer[TEST_BUFFER_SIZE] = {0};
    int32_t result = dumper.Dump(buffer, 0);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name    : CheckAndReopenHandle_001
 * @tc.type    : FUNC
 * @tc.number  : CheckAndReopenHandle_001
 * @tc.desc    : Test CheckAndReopenHandle is callable and returns bool.
 */
HWTEST_F(HpaePcmDumperTest, CheckAndReopenHandle_001, TestSize.Level0)
{
    HpaePcmDumper dumper(TEST_DUMP_FILENAME);
    bool result = dumper.CheckAndReopenHandle();
    // dumpFile_ is nullptr after construction, so CheckAndReopenHandle returns false
    // after attempting (and failing) to reopen in the test environment.
    EXPECT_FALSE(result);
}

/**
 * @tc.name    : Destruct_001
 * @tc.type    : FUNC
 * @tc.number  : Destruct_001
 * @tc.desc    : Test scope-based destruction does not crash.
 */
HWTEST_F(HpaePcmDumperTest, Destruct_001, TestSize.Level0)
{
    {
        HpaePcmDumper dumper(TEST_DUMP_FILENAME);
        int8_t buffer[TEST_BUFFER_SIZE] = {0};
        dumper.Dump(buffer, TEST_BUFFER_SIZE);
    }
    // Dumper destroyed when scope exited; no crash means success
    EXPECT_TRUE(true);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
