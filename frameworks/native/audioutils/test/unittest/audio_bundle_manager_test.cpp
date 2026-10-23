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

/**
 * @tc.name  : ExtractBundleNameAndIndex_WithSeparator
 * @tc.number: ExtractBundleNameAndIndex_001
 * @tc.desc  : Test ExtractBundleNameAndIndex parses bundleName and appIndex from string with '#'.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_WithSeparator, TestSize.Level1)
{
    std::string bundleNameWithIndex = "com.test.app#1";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "com.test.app");
    EXPECT_EQ(appIndex, 1);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_WithoutSeparator
 * @tc.number: ExtractBundleNameAndIndex_002
 * @tc.desc  : Test ExtractBundleNameAndIndex returns original name and appIndex 0 when no '#'.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_WithoutSeparator, TestSize.Level1)
{
    std::string bundleNameWithIndex = "com.test.app";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "com.test.app");
    EXPECT_EQ(appIndex, 0);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_EmptyString
 * @tc.number: ExtractBundleNameAndIndex_101
 * @tc.desc  : Test ExtractBundleNameAndIndex handles empty string correctly.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_EmptyString, TestSize.Level2)
{
    std::string bundleNameWithIndex = "";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "");
    EXPECT_EQ(appIndex, 0);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_SeparatorAtEnd
 * @tc.number: ExtractBundleNameAndIndex_102
 * @tc.desc  : Test ExtractBundleNameAndIndex when '#' is at the end of string.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_SeparatorAtEnd, TestSize.Level2)
{
    std::string bundleNameWithIndex = "com.test.app#";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "com.test.app");
    EXPECT_EQ(appIndex, 0);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_SeparatorAtBeginning
 * @tc.number: ExtractBundleNameAndIndex_103
 * @tc.desc  : Test ExtractBundleNameAndIndex when '#' is at the beginning of string.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_SeparatorAtBeginning, TestSize.Level2)
{
    std::string bundleNameWithIndex = "#1";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "");
    EXPECT_EQ(appIndex, 1);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_MultipleSeparators
 * @tc.number: ExtractBundleNameAndIndex_201
 * @tc.desc  : Test ExtractBundleNameAndIndex uses last '#' as separator.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_MultipleSeparators, TestSize.Level2)
{
    std::string bundleNameWithIndex = "com.test#app#1";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "com.test#app");
    EXPECT_EQ(appIndex, 1);
}

/**
 * @tc.name  : ExtractBundleNameAndIndex_ZeroAppIndex
 * @tc.number: ExtractBundleNameAndIndex_202
 * @tc.desc  : Test ExtractBundleNameAndIndex parses appIndex 0 from string.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_ZeroAppIndex, TestSize.Level2)
{
    std::string bundleNameWithIndex = "com.test.app#0";
    std::string bundleName;
    int32_t appIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);

    EXPECT_EQ(bundleName, "com.test.app");
    EXPECT_EQ(appIndex, 0);
}

/**
 * @tc.name  : ComposeBundleNameWithIndex_NonZeroAppIndex
 * @tc.number: ComposeBundleNameWithIndex_001
 * @tc.desc  : Test ComposeBundleNameWithIndex composes bundleName with appIndex.
 */
HWTEST(AudioBundleManagerTest, ComposeBundleNameWithIndex_NonZeroAppIndex, TestSize.Level1)
{
    std::string bundleName = "com.test.app";
    int32_t appIndex = 1;

    std::string result = AudioBundleManager::ComposeBundleNameWithIndex(bundleName, appIndex);

    EXPECT_EQ(result, "com.test.app#1");
}

/**
 * @tc.name  : ComposeBundleNameWithIndex_ZeroAppIndex
 * @tc.number: ComposeBundleNameWithIndex_002
 * @tc.desc  : Test ComposeBundleNameWithIndex returns bundleName only when appIndex is 0.
 */
HWTEST(AudioBundleManagerTest, ComposeBundleNameWithIndex_ZeroAppIndex, TestSize.Level1)
{
    std::string bundleName = "com.test.app";
    int32_t appIndex = 0;

    std::string result = AudioBundleManager::ComposeBundleNameWithIndex(bundleName, appIndex);

    EXPECT_EQ(result, "com.test.app");
}

/**
 * @tc.name  : ComposeExtractRoundtrip_Success
 * @tc.number: ComposeExtractRoundtrip_001
 * @tc.desc  : Test ComposeBundleNameWithIndex and ExtractBundleNameAndIndex roundtrip consistency.
 */
HWTEST(AudioBundleManagerTest, ComposeExtractRoundtrip_Success, TestSize.Level1)
{
    std::string originalName = "com.test.roundtrip";
    int32_t originalIndex = 2;

    std::string composed = AudioBundleManager::ComposeBundleNameWithIndex(originalName, originalIndex);
    std::string extractedName;
    int32_t extractedIndex = -1;

    AudioBundleManager::ExtractBundleNameAndIndex(composed, extractedName, extractedIndex);

    EXPECT_EQ(extractedName, originalName);
    EXPECT_EQ(extractedIndex, originalIndex);
}

/**
 * @tc.name  : GetBundleInfoV9_NoBundleMgrService
 * @tc.number: GetBundleInfoV9_001
 * @tc.desc  : Test GetBundleInfoV9 returns error when BundleMgrService is not available.
 *             In UT environment, SystemAbilityManager or BUNDLE_MGR_SERVICE_SYS_ABILITY_ID is not
 *             running, so GetBundleInfoV9 should return ERR_NULL_POINTER without crash.
 */
HWTEST(AudioBundleManagerTest, GetBundleInfoV9_NoBundleMgrService, TestSize.Level1)
{
    std::string bundleName = "com.test.app";
    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;
 
    int32_t ret = AudioBundleManager::GetBundleInfoV9(bundleName, userId, bundleInfo);
 
    EXPECT_NE(ret, 0);
}
 
/**
 * @tc.name  : GetBundleInfoV9_EmptyBundleName
 * @tc.number: GetBundleInfoV9_002
 * @tc.desc  : Test GetBundleInfoV9 with empty bundleName does not crash and returns error.
 */
HWTEST(AudioBundleManagerTest, GetBundleInfoV9_EmptyBundleName, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;
 
    int32_t ret = AudioBundleManager::GetBundleInfoV9(bundleName, userId, bundleInfo);
 
    EXPECT_NE(ret, 0);
}
 
/**
 * @tc.name  : GetBundleInfoV9_AncoStyleBundleName
 * @tc.number: GetBundleInfoV9_003
 * @tc.desc  : Test GetBundleInfoV9 with anco-style bundleName (e.g. com.xs.fm.lite.next).
 *             Anco apps are queried via BMS extension, in UT environment BMS is not available
 *             so it should return error without crash.
 */
HWTEST(AudioBundleManagerTest, GetBundleInfoV9_AncoStyleBundleName, TestSize.Level1)
{
    std::string bundleName = "com.xs.fm.lite.next";
    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;
 
    int32_t ret = AudioBundleManager::GetBundleInfoV9(bundleName, userId, bundleInfo);
 
    EXPECT_NE(ret, 0);
}
 
/**
 * @tc.name  : GetBundleInfoV9_ValidUserId
 * @tc.number: GetBundleInfoV9_004
 * @tc.desc  : Test GetBundleInfoV9 with valid userId value. Without BMS running, should return
 *             error but not crash.
 */
HWTEST(AudioBundleManagerTest, GetBundleInfoV9_ValidUserId, TestSize.Level1)
{
    std::string bundleName = "com.netease.cloudmusic";
    int32_t userId = 100;
    AppExecFwk::BundleInfo bundleInfo;
 
    int32_t ret = AudioBundleManager::GetBundleInfoV9(bundleName, userId, bundleInfo);
 
    EXPECT_NE(ret, 0);
}
 
/**
 * @tc.name  : GetBundleInfoV9_MultipleCallsNoCrash
 * @tc.number: GetBundleInfoV9_005
 * @tc.desc  : Test GetBundleInfoV9 can be called multiple times without crash or resource leak.
 */
HWTEST(AudioBundleManagerTest, GetBundleInfoV9_MultipleCallsNoCrash, TestSize.Level1)
{
    std::string bundleName = "com.test.repeat";
    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;
 
    for (int i = 0; i < 5; i++) {
        int32_t ret = AudioBundleManager::GetBundleInfoV9(bundleName, userId, bundleInfo);
        EXPECT_NE(ret, 0);
    }
}
 
/**
 * @tc.name  : ExtractBundleNameAndIndex_AppIndexZeroBranch
 * @tc.number: ExtractBundleNameAndIndex_AppIndexZero_001
 * @tc.desc  : Test ExtractBundleNameAndIndex returns appIndex==0 for normal app (no '#').
 *             This covers the appIndex==0 branch in RestoreSystemAppVolumeLevel which uses
 *             GetBundleInfoV9 path.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_AppIndexZero_001, TestSize.Level1)
{
    std::string bundleNameWithIndex = "com.test.normalapp";
    std::string bundleName;
    int32_t appIndex = -1;
 
    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);
 
    EXPECT_EQ(appIndex, 0);
    EXPECT_EQ(bundleName, "com.test.normalapp");
}
 
/**
 * @tc.name  : ExtractBundleNameAndIndex_AppIndexNonZeroBranch
 * @tc.number: ExtractBundleNameAndIndex_AppIndexNonZero_001
 * @tc.desc  : Test ExtractBundleNameAndIndex returns appIndex>0 for clone app (with '#').
 *             This covers the appIndex>0 branch in RestoreSystemAppVolumeLevel which uses
 *             GetUidByBundleName path.
 */
HWTEST(AudioBundleManagerTest, ExtractBundleNameAndIndex_AppIndexNonZero_001, TestSize.Level1)
{
    std::string bundleNameWithIndex = "com.test.cloneapp#1";
    std::string bundleName;
    int32_t appIndex = -1;
 
    AudioBundleManager::ExtractBundleNameAndIndex(bundleNameWithIndex, bundleName, appIndex);
 
    EXPECT_GT(appIndex, 0);
    EXPECT_EQ(appIndex, 1);
    EXPECT_EQ(bundleName, "com.test.cloneapp");
}
} // namespace AudioStandard
} // namespace OHOS