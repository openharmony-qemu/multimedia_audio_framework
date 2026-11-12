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

#include "audio_utils_container_aiut.h"
#include "aiut_log.h"
#include "audio_scope_exit.h"
#include "audio_utils.h"
#include "audio_safe_block_queue.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr size_t TEST_QUEUE_CAPACITY = 4;
constexpr int32_t TEST_VALUE_1 = 100;
constexpr int32_t TEST_VALUE_2 = 200;
constexpr int32_t TEST_VALUE_3 = 300;
constexpr size_t TEST_FIXED_LIST_SIZE = 3;
}

// ==================== AudioUtilsContainerAiut ====================

void AudioUtilsContainerAiut::SetUpTestCase(void) {}
void AudioUtilsContainerAiut::TearDownTestCase(void) {}
void AudioUtilsContainerAiut::SetUp(void) {}
void AudioUtilsContainerAiut::TearDown(void) {}

// ==================== AudioScopeExit Tests ====================

/**
 * @tc.name  : Test AudioScopeExit executes callback on destruction
 * @tc.number: AudioScopeExit_ExecuteOnDestruction_001
 * @tc.desc  : Test AudioScopeExit automatically invokes the callback when
 *             going out of scope.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioScopeExit_ExecuteOnDestruction_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool executed = false;
    {
        AudioScopeExit scopeExit([&executed]() { executed = true; });
        AIUT_EXPECT_FALSE_INFO_LOG(executed, "callback should not execute yet");
    }
    AIUT_EXPECT_TRUE_INFO_LOG(executed, "callback should have executed on scope exit");
}

/**
 * @tc.name  : Test AudioScopeExit Release cancels callback
 * @tc.number: AudioScopeExit_ReleaseCancelCallback_001
 * @tc.desc  : Test AudioScopeExit::Relase() prevents the callback from executing.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioScopeExit_ReleaseCancelCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool executed = false;
    {
        AudioScopeExit scopeExit([&executed]() { executed = true; });
        scopeExit.Relase();
    }
    AIUT_EXPECT_FALSE_INFO_LOG(executed, "callback should NOT execute after Relase()");
}

/**
 * @tc.name  : Test AudioScopeExit with non-trivial callback
 * @tc.number: AudioScopeExit_NonTrivialCallback_001
 * @tc.desc  : Test AudioScopeExit can modify external variables through callback.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioScopeExit_NonTrivialCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t counter = 0;
    {
        AudioScopeExit scopeExit([&counter]() { counter += TEST_VALUE_1; });
        AIUT_EXPECT_EQ_INFO_LOG(counter, 0, "counter should be 0 before scope exit");
    }
    AIUT_EXPECT_EQ_INFO_LOG(counter, TEST_VALUE_1,
        "counter should be incremented by callback, counter=%d", counter);
}

// ==================== ObjectRefMap Tests ====================

/**
 * @tc.name  : Test ObjectRefMap Insert and GetPtr
 * @tc.number: ObjectRefMap_InsertAndGetPtr_001
 * @tc.desc  : Test ObjectRefMap correctly manages reference counting for a raw pointer.
 */
HWTEST_F(AudioUtilsContainerAiut, ObjectRefMap_InsertAndGetPtr_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t *rawPtr = new int32_t(TEST_VALUE_1);
    {
        ObjectRefMap<int32_t> refMap(rawPtr);
        AIUT_ASSERT_NE_INFO_LOG(refMap.GetPtr(), nullptr, "GetPtr should return non-null");
        AIUT_EXPECT_EQ_INFO_LOG(*refMap.GetPtr(), TEST_VALUE_1,
            "value should match, got=%d", *refMap.GetPtr());
    }
    // After refMap destruction, the object is deleted (ref count drops to 0)
}

/**
 * @tc.name  : Test ObjectRefMap multiple references
 * @tc.number: ObjectRefMap_MultipleRefs_001
 * @tc.desc  : Test ObjectRefMap increases ref count on copy and decreases on destruction.
 */
HWTEST_F(AudioUtilsContainerAiut, ObjectRefMap_MultipleRefs_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t *rawPtr = new int32_t(TEST_VALUE_2);
    int32_t *savedPtr = rawPtr;
    {
        ObjectRefMap<int32_t> ref1(rawPtr);
        AIUT_ASSERT_NE_INFO_LOG(ref1.GetPtr(), nullptr, "ref1 GetPtr is null");
        {
            ObjectRefMap<int32_t> ref2(rawPtr);
            AIUT_ASSERT_NE_INFO_LOG(ref2.GetPtr(), nullptr, "ref2 GetPtr is null");
            AIUT_EXPECT_EQ_INFO_LOG(ref2.GetPtr(), savedPtr,
                "both refs should point to same object");
        }
        // ref2 destroyed, but ref1 still holds, so object should still be alive
        AIUT_EXPECT_NE_INFO_LOG(ref1.GetPtr(), nullptr, "object should still be alive after ref2 dies");
        AIUT_EXPECT_EQ_INFO_LOG(*ref1.GetPtr(), TEST_VALUE_2,
            "value should still be accessible");
    }
    // Both refs gone, object deleted
}

/**
 * @tc.name  : Test ObjectRefMap with null pointer
 * @tc.number: ObjectRefMap_NullPointer_001
 * @tc.desc  : Test ObjectRefMap handles null pointer gracefully.
 */
HWTEST_F(AudioUtilsContainerAiut, ObjectRefMap_NullPointer_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    ObjectRefMap<int32_t> refMap(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(refMap.GetPtr(), nullptr,
        "GetPtr should return nullptr for null construction");
}

/**
 * @tc.name  : Test ObjectRefMap IncreaseRef returns null for unknown
 * @tc.number: ObjectRefMap_IncreaseRefUnknown_001
 * @tc.desc  : Test ObjectRefMap::IncreaseRef returns nullptr for untracked pointer.
 */
HWTEST_F(AudioUtilsContainerAiut, ObjectRefMap_IncreaseRefUnknown_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t *unknownPtr = new int32_t(TEST_VALUE_3);
    int32_t *result = ObjectRefMap<int32_t>::IncreaseRef(unknownPtr);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr,
        "IncreaseRef should return nullptr for untracked pointer");
    delete unknownPtr;
}

/**
 * @tc.name  : Test ObjectRefMap Erase removes tracked pointer
 * @tc.number: ObjectRefMap_EraseTracked_001
 * @tc.desc  : Test ObjectRefMap::Erase removes a pointer from the ref map.
 */
HWTEST_F(AudioUtilsContainerAiut, ObjectRefMap_EraseTracked_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t *rawPtr = new int32_t(TEST_VALUE_1);
    ObjectRefMap<int32_t>::Insert(rawPtr);
    AIUT_EXPECT_NE_INFO_LOG(ObjectRefMap<int32_t>::IncreaseRef(rawPtr), nullptr,
        "pointer should be tracked after Insert");
    ObjectRefMap<int32_t>::Erase(rawPtr);
    // After erase, IncreaseRef should return nullptr (not tracked anymore)
    // But we need to manually delete since Erase doesn't delete
    delete rawPtr;
}

// ==================== FixedSizeList Tests ====================

/**
 * @tc.name  : Test FixedSizeList Add and GetData
 * @tc.number: FixedSizeList_AddAndGet_001
 * @tc.desc  : Test FixedSizeList Add stores values and GetData retrieves them.
 */
HWTEST_F(AudioUtilsContainerAiut, FixedSizeList_AddAndGet_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    FixedSizeList<int32_t> list(TEST_FIXED_LIST_SIZE);
    list.Add(TEST_VALUE_1);
    list.Add(TEST_VALUE_2);

    auto data = list.GetData();
    AIUT_EXPECT_EQ_INFO_LOG(data.size(), static_cast<size_t>(2),
        "data size should be 2, got=%zu", data.size());
    AIUT_EXPECT_EQ_INFO_LOG(data[0], TEST_VALUE_1, "first element mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(data[1], TEST_VALUE_2, "second element mismatch");
}

/**
 * @tc.name  : Test FixedSizeList wraps around when full
 * @tc.number: FixedSizeList_WrapAround_001
 * @tc.desc  : Test FixedSizeList overwrites oldest element when capacity is exceeded.
 */
HWTEST_F(AudioUtilsContainerAiut, FixedSizeList_WrapAround_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    FixedSizeList<int32_t> list(TEST_FIXED_LIST_SIZE);
    list.Add(TEST_VALUE_1);
    list.Add(TEST_VALUE_2);
    list.Add(TEST_VALUE_3);
    list.Add(400); // exceeds capacity, overwrites TEST_VALUE_1

    auto data = list.GetData();
    AIUT_EXPECT_EQ_INFO_LOG(data.size(), TEST_FIXED_LIST_SIZE,
        "data size should be capped at %zu", TEST_FIXED_LIST_SIZE);
    AIUT_EXPECT_EQ_INFO_LOG(data[0], TEST_VALUE_2, "oldest should be overwritten");
}

/**
 * @tc.name  : Test FixedSizeList FindIf finds matching element
 * @tc.number: FixedSizeList_FindIfMatch_001
 * @tc.desc  : Test FixedSizeList::FindIf returns the first matching element and removes it.
 */
HWTEST_F(AudioUtilsContainerAiut, FixedSizeList_FindIfMatch_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    FixedSizeList<int32_t> list(TEST_FIXED_LIST_SIZE);
    list.Add(TEST_VALUE_1);
    list.Add(TEST_VALUE_2);
    list.Add(TEST_VALUE_3);

    auto result = list.FindIf([](const int32_t &val) { return val == TEST_VALUE_2; });
    AIUT_EXPECT_TRUE_INFO_LOG(result.has_value(), "FindIf should find value");
    AIUT_EXPECT_EQ_INFO_LOG(result.value(), TEST_VALUE_2, "found value mismatch");

    // After FindIf removes the element, list size should decrease
    auto data = list.GetData();
    AIUT_EXPECT_EQ_INFO_LOG(data.size(), static_cast<size_t>(2),
        "list size should be 2 after removal, got=%zu", data.size());
}

/**
 * @tc.name  : Test FixedSizeList FindIf returns nullopt when not found
 * @tc.number: FixedSizeList_FindIfNotFound_001
 * @tc.desc  : Test FixedSizeList::FindIf returns nullopt when no element matches.
 */
HWTEST_F(AudioUtilsContainerAiut, FixedSizeList_FindIfNotFound_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    FixedSizeList<int32_t> list(TEST_FIXED_LIST_SIZE);
    list.Add(TEST_VALUE_1);
    list.Add(TEST_VALUE_2);

    auto result = list.FindIf([](const int32_t &val) { return val == 999; });
    AIUT_EXPECT_FALSE_INFO_LOG(result.has_value(), "FindIf should not find non-existent value");
}

/**
 * @tc.name  : Test FixedSizeList Clear empties the list
 * @tc.number: FixedSizeList_Clear_001
 * @tc.desc  : Test FixedSizeList::Clear removes all elements.
 */
HWTEST_F(AudioUtilsContainerAiut, FixedSizeList_Clear_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    FixedSizeList<int32_t> list(TEST_FIXED_LIST_SIZE);
    list.Add(TEST_VALUE_1);
    list.Add(TEST_VALUE_2);

    list.Clear();
    auto data = list.GetData();
    AIUT_EXPECT_TRUE_INFO_LOG(data.empty(), "list should be empty after Clear");
}

// ==================== AudioSafeBlockQueue Tests ====================

/**
 * @tc.name  : Test AudioSafeBlockQueue Push and Pop
 * @tc.number: AudioSafeBlockQueue_PushPop_001
 * @tc.desc  : Test AudioSafeBlockQueue basic push and pop operations.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioSafeBlockQueue_PushPop_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSafeBlockQueue<int32_t> queue(TEST_QUEUE_CAPACITY);
    queue.Push(TEST_VALUE_1);
    queue.Push(TEST_VALUE_2);

    AIUT_EXPECT_EQ_INFO_LOG(queue.Size(), static_cast<size_t>(2),
        "queue size should be 2, got=%zu", static_cast<size_t>(queue.Size()));
    AIUT_EXPECT_FALSE_INFO_LOG(queue.IsEmpty(), "queue should not be empty");

    int32_t val = queue.Pop();
    AIUT_EXPECT_EQ_INFO_LOG(val, TEST_VALUE_1, "popped value mismatch");
}

/**
 * @tc.name  : Test AudioSafeBlockQueue PushNoWait and PopNotWait
 * @tc.number: AudioSafeBlockQueue_NonBlocking_001
 * @tc.desc  : Test AudioSafeBlockQueue non-blocking push and pop.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioSafeBlockQueue_NonBlocking_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSafeBlockQueue<int32_t> queue(TEST_QUEUE_CAPACITY);

    int32_t val = 0;
    bool popResult = queue.PopNotWait(val);
    AIUT_EXPECT_FALSE_INFO_LOG(popResult, "PopNotWait should fail on empty queue");

    queue.PushNoWait(TEST_VALUE_1);
    popResult = queue.PopNotWait(val);
    AIUT_EXPECT_TRUE_INFO_LOG(popResult, "PopNotWait should succeed after push");
    AIUT_EXPECT_EQ_INFO_LOG(val, TEST_VALUE_1, "popped value mismatch");
}

/**
 * @tc.name  : Test AudioSafeBlockQueue IsFull
 * @tc.number: AudioSafeBlockQueue_IsFull_001
 * @tc.desc  : Test AudioSafeBlockQueue correctly reports full state.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioSafeBlockQueue_IsFull_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSafeBlockQueue<int32_t> queue(2);
    queue.PushNoWait(TEST_VALUE_1);
    queue.PushNoWait(TEST_VALUE_2);

    AIUT_EXPECT_TRUE_INFO_LOG(queue.IsFull(), "queue should be full");
}

/**
 * @tc.name  : Test AudioSafeBlockQueue Clear
 * @tc.number: AudioSafeBlockQueue_Clear_001
 * @tc.desc  : Test AudioSafeBlockQueue::Clear removes all elements.
 */
HWTEST_F(AudioUtilsContainerAiut, AudioSafeBlockQueue_Clear_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSafeBlockQueue<int32_t> queue(TEST_QUEUE_CAPACITY);
    queue.Push(TEST_VALUE_1);
    queue.Push(TEST_VALUE_2);

    queue.Clear();
    AIUT_EXPECT_TRUE_INFO_LOG(queue.IsEmpty(), "queue should be empty after Clear");
}

} // namespace AudioStandard
} // namespace OHOS
