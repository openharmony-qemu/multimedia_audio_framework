/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_running_lock_manager_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_running_lock_manager.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
}

void AudioRunningLockManagerUnitTest::SetUpTestCase(void) {}
void AudioRunningLockManagerUnitTest::TearDownTestCase(void) {}
void AudioRunningLockManagerUnitTest::SetUp(void) {}
void AudioRunningLockManagerUnitTest::TearDown(void) {}

class MockRunningLockInterface {
public:
    virtual int32_t Lock(const int32_t timeoutMs) = 0;
    virtual int32_t UnLock() = 0;
    virtual int32_t UpdateWorkSource(const std::vector<int32_t>&) = 0;
    virtual ~MockRunningLockInterface() = default;
};

class MockRunningLock : public MockRunningLockInterface {
public:
    MOCK_METHOD(int32_t, Lock, (const int32_t timeoutMs), (override));
    MOCK_METHOD(int32_t, UnLock, (), (override));
    MOCK_METHOD(int32_t, UpdateWorkSource, (const std::vector<int32_t>&), (override));
};

/**
 * @tc.name  : Test Template AudioRunningLockManager
 * @tc.number: Audio_Running_Lock_Manager_001
 * @tc.desc  : Test Template AudioRunningLockManager call Lock and Unlock interface check isLocked
 */
HWTEST(AudioRunningLockManagerUnitTest, AudioRunningLockManagerUnitTest_001, TestSize.Level1)
{
    auto sharedPtrMockRunningLock = std::make_shared<MockRunningLock>();
    AudioRunningLockManager<MockRunningLockInterface> lockManagerMock(sharedPtrMockRunningLock);

    InSequence seq;

    EXPECT_CALL((*sharedPtrMockRunningLock), Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING))
        .Times(1).WillOnce(Return(SUCCESS));
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(IsEmpty())).Times(1);
    EXPECT_CALL((*sharedPtrMockRunningLock), UnLock()).Times(1).WillOnce(Return(SUCCESS));

    lockManagerMock.Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING);
    EXPECT_EQ(lockManagerMock.isLocked_, true);

    lockManagerMock.UnLock();
    EXPECT_EQ(lockManagerMock.isLocked_, false);
}

/**
 * @tc.name  : Test Template AudioRunningLockManager
 * @tc.number: Audio_Running_Lock_Manager_002
 * @tc.desc  : Test Template AudioRunningLockManager call UpdateAppsUid and UpdateAppsUidToPowerMgr interface
 */
HWTEST(AudioRunningLockManagerUnitTest, AudioRunningLockManagerUnitTest_002, TestSize.Level1)
{
    auto sharedPtrMockRunningLock = std::make_shared<MockRunningLock>();
    AudioRunningLockManager<MockRunningLockInterface> lockManagerMock(sharedPtrMockRunningLock);

    InSequence seq;
    std::vector<int32_t> appsUid1 = {};
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(IsEmpty())).Times(0);
    lockManagerMock.UpdateAppsUid(appsUid1.begin(), appsUid1.end());
    lockManagerMock.UpdateAppsUidToPowerMgr();

    EXPECT_CALL((*sharedPtrMockRunningLock), Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING))
        .Times(1).WillOnce(Return(SUCCESS));
    lockManagerMock.Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING);
    EXPECT_EQ(lockManagerMock.isLocked_, true);

    std::vector<int32_t> appsUid2 = {1};
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(UnorderedElementsAreArray(
        std::unordered_set<int32_t>(appsUid2.begin(), appsUid2.end())))).Times(1);
    lockManagerMock.UpdateAppsUid(appsUid2.begin(), appsUid2.end());
    lockManagerMock.UpdateAppsUidToPowerMgr();

    std::vector<int32_t> appsUid3 = {1, 1};
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(UnorderedElementsAreArray(
        std::unordered_set<int32_t>(appsUid3.begin(), appsUid3.end())))).Times(0);
    lockManagerMock.UpdateAppsUid(appsUid3.begin(), appsUid3.end());
    lockManagerMock.UpdateAppsUidToPowerMgr();

    std::vector<int32_t> appsUid4 = {1, 2};
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(UnorderedElementsAreArray(
        std::unordered_set<int32_t>(appsUid4.begin(), appsUid4.end())))).Times(1);
    lockManagerMock.UpdateAppsUid(appsUid4.begin(), appsUid4.end());
    lockManagerMock.UpdateAppsUidToPowerMgr();

    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(IsEmpty())).Times(1);
    EXPECT_CALL((*sharedPtrMockRunningLock), UnLock()).Times(1).WillOnce(Return(SUCCESS));
    lockManagerMock.UnLock();
    EXPECT_EQ(lockManagerMock.isLocked_, false);

    std::vector<int32_t> appsUid5 = {1, 2, 3};
    EXPECT_CALL((*sharedPtrMockRunningLock), UpdateWorkSource(UnorderedElementsAreArray(
        std::unordered_set<int32_t>(appsUid5.begin(), appsUid5.end())))).Times(0);
    lockManagerMock.UpdateAppsUid(appsUid5.begin(), appsUid5.end());
    lockManagerMock.UpdateAppsUidToPowerMgr();
}
} // namespace AudioStandard
} // namespace OHOS
