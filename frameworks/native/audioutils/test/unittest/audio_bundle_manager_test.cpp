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

#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "audio_bundle_manager.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS {
namespace AudioStandard {

class AudioBundleManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioBundleManagerTest::SetUpTestCase(void) {}

void AudioBundleManagerTest::TearDownTestCase(void) {}

void AudioBundleManagerTest::SetUp(void) {}

void AudioBundleManagerTest::TearDown(void) {}

/**
 * @tc.name  : Test GetBundleNameFromUidCached with empty cache
 * @tc.type  : FUNC
 * @tc.number: GetBundleNameFromUidCached_001
 * @tc.desc  : Test GetBundleNameFromUidCached returns empty string when cache is empty.
 */
HWTEST(AudioBundleManagerTest, GetBundleNameFromUidCached_001, TestSize.Level1)
{
    int32_t uid = 9999;
    std::string result = AudioBundleManager::GetBundleNameFromUidCached(uid);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test FindBundleNameFromMap with empty cache
 * @tc.type  : FUNC
 * @tc.number: FindBundleNameFromMap_001
 * @tc.desc  : Test FindBundleNameFromMap returns empty string when uid not found.
 */
HWTEST(AudioBundleManagerTest, FindBundleNameFromMap_001, TestSize.Level1)
{
    int32_t uid = 9998;
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test InsertBundleNameToMap and FindBundleNameFromMap
 * @tc.type  : FUNC
 * @tc.number: InsertBundleNameToMap_001
 * @tc.desc  : Test InsertBundleNameToMap inserts correctly and FindBundleNameFromMap finds it.
 */
HWTEST(AudioBundleManagerTest, InsertBundleNameToMap_001, TestSize.Level1)
{
    int32_t uid = 1001;
    std::string bundleName = "com.test.app1";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, bundleName);
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

/**
 * @tc.name  : Test EraseBundleNameFromMap
 * @tc.type  : FUNC
 * @tc.number: EraseBundleNameFromMap_001
 * @tc.desc  : Test EraseBundleNameFromMap removes entry correctly.
 */
HWTEST(AudioBundleManagerTest, EraseBundleNameFromMap_001, TestSize.Level1)
{
    int32_t uid = 1002;
    std::string bundleName = "com.test.app2";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
    
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test GetBundleNameFromUidCached after insert
 * @tc.type  : FUNC
 * @tc.number: GetBundleNameFromUidCached_002
 * @tc.desc  : Test GetBundleNameFromUidCached returns correct value after insert.
 */
HWTEST(AudioBundleManagerTest, GetBundleNameFromUidCached_002, TestSize.Level1)
{
    int32_t uid = 1003;
    std::string bundleName = "com.test.app3";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::string result = AudioBundleManager::GetBundleNameFromUidCached(uid);
    EXPECT_EQ(result, bundleName);
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

/**
 * @tc.name  : Test concurrent FindBundleNameFromMap
 * @tc.type  : FUNC
 * @tc.number: FindBundleNameFromMap_Concurrent_001
 * @tc.desc  : Test concurrent read operations on FindBundleNameFromMap.
 */
HWTEST(AudioBundleManagerTest, FindBundleNameFromMap_Concurrent_001, TestSize.Level1)
{
    const int32_t uid = 2001;
    const std::string bundleName = "com.test.concurrent";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::vector<std::thread> threads;
    std::vector<std::string> results(10);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&results, i, uid]() {
            results[i] = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(results[i], bundleName);
    }
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

/**
 * @tc.name  : Test concurrent GetBundleNameFromUidCached
 * @tc.type  : FUNC
 * @tc.number: GetBundleNameFromUidCached_Concurrent_001
 * @tc.desc  : Test concurrent read operations on GetBundleNameFromUidCached.
 */
HWTEST(AudioBundleManagerTest, GetBundleNameFromUidCached_Concurrent_001, TestSize.Level1)
{
    const int32_t uid = 2002;
    const std::string bundleName = "com.test.concurrent2";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::vector<std::thread> threads;
    std::vector<std::string> results(10);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&results, i, uid]() {
            results[i] = AudioBundleManager::GetBundleNameFromUidCached(uid);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(results[i], bundleName);
    }
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

/**
 * @tc.name  : Test RemoveBundleNameByUid
 * @tc.type  : FUNC
 * @tc.number: RemoveBundleNameByUid_001
 * @tc.desc  : Test RemoveBundleNameByUid removes entry and subsequent cached query returns empty.
 */
HWTEST(AudioBundleManagerTest, RemoveBundleNameByUid_001, TestSize.Level1)
{
    int32_t uid = 3001;
    std::string bundleName = "com.test.remove";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::string result = AudioBundleManager::GetBundleNameFromUidCached(uid);
    EXPECT_EQ(result, bundleName);
    
    AudioBundleManager::RemoveBundleNameByUid(uid);
    
    result = AudioBundleManager::GetBundleNameFromUidCached(uid);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test multiple inserts and finds
 * @tc.type  : FUNC
 * @tc.number: MultipleInserts_001
 * @tc.desc  : Test inserting multiple entries and finding each correctly.
 */
HWTEST(AudioBundleManagerTest, MultipleInserts_001, TestSize.Level1)
{
    std::vector<int32_t> uids = {4001, 4002, 4003};
    std::vector<std::string> bundleNames = {"com.test.app4", "com.test.app5", "com.test.app6"};
    
    for (size_t i = 0; i < uids.size(); i++) {
        AudioBundleManager::GetInstance().InsertBundleNameToMap(uids[i], bundleNames[i]);
    }
    
    for (size_t i = 0; i < uids.size(); i++) {
        std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uids[i]);
        EXPECT_EQ(result, bundleNames[i]);
    }
    
    for (int32_t uid : uids) {
        AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
    }
}

/**
 * @tc.name  : Test insert overwrite
 * @tc.type  : FUNC
 * @tc.number: InsertOverwrite_001
 * @tc.desc  : Test inserting same uid with different bundleName overwrites previous value.
 */
HWTEST(AudioBundleManagerTest, InsertOverwrite_001, TestSize.Level1)
{
    int32_t uid = 5001;
    std::string bundleName1 = "com.test.old";
    std::string bundleName2 = "com.test.new";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName1);
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, bundleName1);
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName2);
    result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, bundleName2);
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

/**
 * @tc.name  : Test erase non-existent uid
 * @tc.type  : FUNC
 * @tc.number: EraseNonExistent_001
 * @tc.desc  : Test EraseBundleNameFromMap on non-existent uid does not cause error.
 */
HWTEST(AudioBundleManagerTest, EraseNonExistent_001, TestSize.Level1)
{
    int32_t uid = 6001;
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
    
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : Test empty bundle name insert
 * @tc.type  : FUNC
 * @tc.number: EmptyBundleName_001
 * @tc.desc  : Test inserting empty bundle name.
 */
HWTEST(AudioBundleManagerTest, EmptyBundleName_001, TestSize.Level1)
{
    int32_t uid = 7001;
    std::string bundleName = "";
    
    AudioBundleManager::GetInstance().InsertBundleNameToMap(uid, bundleName);
    
    std::string result = AudioBundleManager::GetInstance().FindBundleNameFromMap(uid);
    EXPECT_EQ(result, "");
    
    AudioBundleManager::GetInstance().EraseBundleNameFromMap(uid);
}

} // namespace AudioStandard
} // namespace OHOS