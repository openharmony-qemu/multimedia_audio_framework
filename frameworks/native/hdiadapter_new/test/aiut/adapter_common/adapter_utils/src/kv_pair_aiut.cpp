/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "kv_pair_aiut.h"
#include "aiut_log.h"
#include "util/kv_pair.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void KvPairAiut::SetUpTestCase(void) {}
void KvPairAiut::TearDownTestCase(void) {}
void KvPairAiut::SetUp(void) {}
void KvPairAiut::TearDown(void) {}

// ==================== Set and Get Tests ====================

/**
 * @tc.name  : Test KvPair Set and Get with string
 * @tc.number: KvPair_SetGet_001
 * @tc.desc  : Test KvPair Set/Get with string value.
 */
HWTEST_F(KvPairAiut, KvPair_SetGet_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    kv.Set<std::string>("key1", "value1");
    std::string result;
    int32_t ret = kv.Get<std::string>("key1", result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Get should succeed");
    AIUT_EXPECT_EQ_INFO_LOG(result, "value1", "Get should return correct value");
}

/**
 * @tc.name  : Test KvPair Set and Get with int32_t
 * @tc.number: KvPair_SetGet_002
 * @tc.desc  : Test KvPair Set/Get with int32_t value.
 */
HWTEST_F(KvPairAiut, KvPair_SetGet_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    kv.Set<int32_t>("key_int", 42);
    int32_t result = 0;
    int32_t ret = kv.Get<int32_t>("key_int", result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Get should succeed");
    AIUT_EXPECT_EQ_INFO_LOG(result, 42, "Get should return correct int value");
}

/**
 * @tc.name  : Test KvPair Get non-existent key
 * @tc.number: KvPair_SetGet_003
 * @tc.desc  : Test KvPair Get returns error for non-existent key.
 */
HWTEST_F(KvPairAiut, KvPair_SetGet_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    std::string result;
    int32_t ret = kv.Get<std::string>("nonexistent", result);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Get should fail for non-existent key");
}

// ==================== Erase Tests ====================

/**
 * @tc.name  : Test KvPair Erase
 * @tc.number: KvPair_Erase_001
 * @tc.desc  : Test KvPair Erase removes key and Get fails.
 */
HWTEST_F(KvPairAiut, KvPair_Erase_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    kv.Set<std::string>("key1", "value1");
    kv.Erase("key1");
    std::string result;
    int32_t ret = kv.Get<std::string>("key1", result);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "Get should fail after Erase");
}

/**
 * @tc.name  : Test KvPair Erase non-existent key
 * @tc.number: KvPair_Erase_002
 * @tc.desc  : Test KvPair Erase non-existent key does not crash.
 */
HWTEST_F(KvPairAiut, KvPair_Erase_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    kv.Erase("nonexistent");
}

// ==================== Overwrite Tests ====================

/**
 * @tc.name  : Test KvPair overwrite value
 * @tc.number: KvPair_Overwrite_001
 * @tc.desc  : Test KvPair Set overwrites existing value.
 */
HWTEST_F(KvPairAiut, KvPair_Overwrite_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    KvPair<std::string> kv;
    kv.Set<std::string>("key1", "value1");
    kv.Set<std::string>("key1", "value2");
    std::string result;
    int32_t ret = kv.Get<std::string>("key1", result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Get should succeed");
    AIUT_EXPECT_EQ_INFO_LOG(result, "value2", "Get should return overwritten value");
}

} // namespace AudioStandard
} // namespace OHOS
