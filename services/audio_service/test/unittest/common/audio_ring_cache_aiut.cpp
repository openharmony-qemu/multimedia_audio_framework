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

#include "audio_ring_cache.h"
#include "aiut_log.h"
#include "audio_service_log.h"

#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr size_t TEST_CACHE_SIZE = 64;
constexpr size_t TEST_DATA_SIZE_HALF = 32;
constexpr size_t TEST_DATA_SIZE_40 = 40;
constexpr size_t TEST_DATA_SIZE_8 = 8;
constexpr size_t TEST_DATA_SIZE_16 = 16;
constexpr size_t MAX_CACHE_SIZE_LIMIT = 16 * 1024 * 1024;
constexpr size_t BASE_INDEX_FENCE = SIZE_MAX - 2 * MAX_CACHE_SIZE_LIMIT;
}  // namespace

class AudioRingCacheAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

static std::unique_ptr<AudioRingCache> CreateTestCache()
{
    return AudioRingCache::Create(TEST_CACHE_SIZE);
}

// ==================== Create Tests ====================

/**
 * @tc.name  : Test Create with valid cache size
 * @tc.number: AudioRingCache_Create_Valid
 * @tc.desc  : Test Create returns non-null with valid cache size.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Create_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = AudioRingCache::Create(TEST_CACHE_SIZE);
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "Create returned null");
    AIUT_EXPECT_EQ_INFO_LOG(cache->GetCahceSize(), TEST_CACHE_SIZE,
        "cacheSize should be 64");
}

// ==================== GetCahceSize / ResetBuffer Tests ====================

/**
 * @tc.name  : Test GetCahceSize after init
 * @tc.number: AudioRingCache_GetCahceSize_Init
 * @tc.desc  : Test GetCahceSize returns correct value after init.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_GetCahceSize_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    AIUT_EXPECT_EQ_INFO_LOG(cache->GetCahceSize(), TEST_CACHE_SIZE,
        "cacheSize should be 64");
}

/**
 * @tc.name  : Test ResetBuffer after enqueue
 * @tc.number: AudioRingCache_ResetBuffer_AfterEnqueue
 * @tc.desc  : Test ResetBuffer clears data and resets indices.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ResetBuffer_AfterEnqueue, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer);
    auto readable = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readable.size, TEST_DATA_SIZE_HALF,
        "readable should be 32");
    cache->ResetBuffer();
    auto readableAfter = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readableAfter.size, 0U, "readable should be 0 after reset");
}

// ==================== GetWritableSize / GetReadableSize Tests ====================

/**
 * @tc.name  : Test GetWritableSize after init
 * @tc.number: AudioRingCache_GetWritableSize_Init
 * @tc.desc  : Test GetWritableSize returns cacheTotalSize after init.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_GetWritableSize_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    auto result = cache->GetWritableSize();
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "GetWritableSize failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, TEST_CACHE_SIZE,
        "writable should be 64");
}

/**
 * @tc.name  : Test GetReadableSize after init
 * @tc.number: AudioRingCache_GetReadableSize_Init
 * @tc.desc  : Test GetReadableSize returns 0 after init.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_GetReadableSize_Init, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    auto result = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "GetReadableSize failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, 0U, "readable should be 0");
}

/**
 * @tc.name  : Test GetWritableSize with invalid status
 * @tc.number: AudioRingCache_GetWritableSize_InvalidStatus
 * @tc.desc  : Test GetWritableSize returns INVALID_STATUS when writeIndex < readIndex.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_GetWritableSize_InvalidStatus, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    cache->writeIndex_ = 4;
    cache->readIndex_ = 8;
    auto result = cache->GetWritableSize();
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_STATUS,
        "GetWritableSize should return INVALID_STATUS");
    AIUT_EXPECT_EQ_INFO_LOG(result.size, 0U, "size should be 0");
}

/**
 * @tc.name  : Test GetReadableSize with invalid status
 * @tc.number: AudioRingCache_GetReadableSize_InvalidStatus
 * @tc.desc  : Test GetReadableSize returns INVALID_STATUS when write-read > cacheTotalSize.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_GetReadableSize_InvalidStatus, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    cache->writeIndex_ = 100;
    cache->readIndex_ = 0;
    cache->cacheTotalSize_ = 64;
    auto result = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_STATUS,
        "GetReadableSize should return INVALID_STATUS");
}

// ==================== Enqueue Tests ====================

/**
 * @tc.name  : Test Enqueue with valid data
 * @tc.number: AudioRingCache_Enqueue_Valid
 * @tc.desc  : Test Enqueue returns OPERATION_SUCCESS with valid data.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    auto result = cache->Enqueue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Enqueue failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, TEST_DATA_SIZE_HALF,
        "size should be 32");
}

/**
 * @tc.name  : Test Enqueue with null data pointer
 * @tc.number: AudioRingCache_Enqueue_NullPtr
 * @tc.desc  : Test Enqueue returns INVALID_PARAMS with null data pointer.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_NullPtr, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    BufferWrap buffer = {nullptr, TEST_DATA_SIZE_HALF};
    auto result = cache->Enqueue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_PARAMS,
        "Enqueue null ptr should return INVALID_PARAMS");
}

/**
 * @tc.name  : Test Enqueue with zero data size
 * @tc.number: AudioRingCache_Enqueue_ZeroSize
 * @tc.desc  : Test Enqueue returns INVALID_PARAMS with zero data size.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_ZeroSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[1] = {0};
    BufferWrap buffer = {data, 0};
    auto result = cache->Enqueue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_PARAMS,
        "Enqueue zero size should return INVALID_PARAMS");
}

/**
 * @tc.name  : Test Enqueue with data larger than writable
 * @tc.number: AudioRingCache_Enqueue_TooLarge
 * @tc.desc  : Test Enqueue returns INDEX_OUT_OF_RANGE when data > writable.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_CACHE_SIZE + 1] = {0};
    BufferWrap buffer = {data, TEST_CACHE_SIZE + 1};
    auto result = cache->Enqueue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INDEX_OUT_OF_RANGE,
        "Enqueue too large should return INDEX_OUT_OF_RANGE");
}

/**
 * @tc.name  : Test Enqueue with cross-boundary data
 * @tc.number: AudioRingCache_Enqueue_CrossBoundary
 * @tc.desc  : Test Enqueue handles data that wraps around the buffer end.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_CrossBoundary, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data1[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer1 = {data1, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer1);
    uint8_t out0[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap outBuf0 = {out0, TEST_DATA_SIZE_HALF};
    cache->Dequeue(outBuf0);
    uint8_t data2[TEST_DATA_SIZE_40] = {0};
    BufferWrap buffer2 = {data2, TEST_DATA_SIZE_40};
    auto result = cache->Enqueue(buffer2);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Enqueue cross-boundary failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, TEST_DATA_SIZE_40,
        "size should be 40");
    auto readable = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readable.size, TEST_DATA_SIZE_40,
        "readable should be 40");
}

/**
 * @tc.name  : Test Enqueue after wrap-around (not crossing)
 * @tc.number: AudioRingCache_Enqueue_AfterWrap
 * @tc.desc  : Test Enqueue writes at wrapped position without crossing.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Enqueue_AfterWrap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data1[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer1 = {data1, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer1);
    uint8_t out0[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap outBuf0 = {out0, TEST_DATA_SIZE_HALF};
    cache->Dequeue(outBuf0);
    uint8_t data2[TEST_DATA_SIZE_40] = {0};
    BufferWrap buffer2 = {data2, TEST_DATA_SIZE_40};
    cache->Enqueue(buffer2);
    uint8_t data3[TEST_DATA_SIZE_8] = {0};
    BufferWrap buffer3 = {data3, TEST_DATA_SIZE_8};
    auto result = cache->Enqueue(buffer3);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Enqueue after wrap failed, ret=%d", result.ret);
}

// ==================== Dequeue Tests ====================

/**
 * @tc.name  : Test Dequeue with valid data
 * @tc.number: AudioRingCache_Dequeue_Valid
 * @tc.desc  : Test Dequeue returns OPERATION_SUCCESS with valid data.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Dequeue_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t inData[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap inBuffer = {inData, TEST_DATA_SIZE_HALF};
    cache->Enqueue(inBuffer);
    uint8_t outData[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap outBuffer = {outData, TEST_DATA_SIZE_HALF};
    auto result = cache->Dequeue(outBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Dequeue failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, TEST_DATA_SIZE_HALF,
        "size should be 32");
    auto readable = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readable.size, 0U, "readable should be 0 after dequeue");
}

/**
 * @tc.name  : Test Dequeue with null data pointer
 * @tc.number: AudioRingCache_Dequeue_NullPtr
 * @tc.desc  : Test Dequeue returns INVALID_PARAMS with null data pointer.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Dequeue_NullPtr, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    BufferWrap buffer = {nullptr, TEST_DATA_SIZE_HALF};
    auto result = cache->Dequeue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_PARAMS,
        "Dequeue null ptr should return INVALID_PARAMS");
}

/**
 * @tc.name  : Test Dequeue with data larger than readable
 * @tc.number: AudioRingCache_Dequeue_TooLarge
 * @tc.desc  : Test Dequeue returns INVALID_OPERATION when data > readable.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Dequeue_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    auto result = cache->Dequeue(buffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INVALID_OPERATION,
        "Dequeue too large should return INVALID_OPERATION");
}

/**
 * @tc.name  : Test Dequeue with cross-boundary data
 * @tc.number: AudioRingCache_Dequeue_CrossBoundary
 * @tc.desc  : Test Dequeue handles data that wraps around the buffer end.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Dequeue_CrossBoundary, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data1[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer1 = {data1, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer1);
    uint8_t out0[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap outBuf0 = {out0, TEST_DATA_SIZE_HALF};
    cache->Dequeue(outBuf0);
    uint8_t data2[TEST_DATA_SIZE_40] = {0};
    BufferWrap buffer2 = {data2, TEST_DATA_SIZE_40};
    cache->Enqueue(buffer2);
    uint8_t out2[TEST_DATA_SIZE_40] = {0};
    BufferWrap outBuf2 = {out2, TEST_DATA_SIZE_40};
    auto result = cache->Dequeue(outBuf2);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Dequeue cross-boundary failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(result.size, TEST_DATA_SIZE_40,
        "size should be 40");
}

/**
 * @tc.name  : Test Dequeue triggers baseIndex move
 * @tc.number: AudioRingCache_Dequeue_BaseIndexMove
 * @tc.desc  : Test Dequeue moves baseIndex when readIndex reaches buffer end.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_Dequeue_BaseIndexMove, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_CACHE_SIZE] = {0};
    BufferWrap buffer = {data, TEST_CACHE_SIZE};
    cache->Enqueue(buffer);
    uint8_t out[TEST_CACHE_SIZE] = {0};
    BufferWrap outBuf = {out, TEST_CACHE_SIZE};
    auto result = cache->Dequeue(outBuf);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Dequeue full buffer failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(cache->baseIndex_, TEST_CACHE_SIZE,
        "baseIndex should be 64");
}

// ==================== ReConfig Tests ====================

/**
 * @tc.name  : Test ReConfig with too large size
 * @tc.number: AudioRingCache_ReConfig_TooLarge
 * @tc.desc  : Test ReConfig returns INDEX_OUT_OF_RANGE with size > MAX_CACHE_SIZE.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ReConfig_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    auto result = cache->ReConfig(MAX_CACHE_SIZE_LIMIT + 1);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, INDEX_OUT_OF_RANGE,
        "ReConfig too large should return INDEX_OUT_OF_RANGE");
}

/**
 * @tc.name  : Test ReConfig without copying remained data
 * @tc.number: AudioRingCache_ReConfig_NoCopy
 * @tc.desc  : Test ReConfig with copyRemained=false resets buffer to new size.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ReConfig_NoCopy, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer);
    auto result = cache->ReConfig(128, false);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "ReConfig no copy failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(cache->GetCahceSize(), 128U,
        "cacheSize should be 128");
    auto readable = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readable.size, 0U,
        "readable should be 0 after ReConfig no copy");
}

/**
 * @tc.name  : Test ReConfig with copying remained data
 * @tc.number: AudioRingCache_ReConfig_CopyRemained
 * @tc.desc  : Test ReConfig with copyRemained=true preserves data.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ReConfig_CopyRemained, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer);
    auto result = cache->ReConfig(128, true);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "ReConfig copy failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(cache->GetCahceSize(), 128U,
        "cacheSize should be 128");
    auto readable = cache->GetReadableSize();
    AIUT_EXPECT_EQ_INFO_LOG(readable.size, TEST_DATA_SIZE_HALF,
        "readable should be 32 after copy");
}

/**
 * @tc.name  : Test ReConfig with copyRemained when data > new size
 * @tc.number: AudioRingCache_ReConfig_CopyTooLarge
 * @tc.desc  : Test ReConfig with copyRemained=true when readable > new cache size.
 *             Source returns OPERATION_SUCCESS (result from GetReadableSize) without reconfiguring.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ReConfig_CopyTooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_HALF] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_HALF};
    cache->Enqueue(buffer);
    auto result = cache->ReConfig(TEST_DATA_SIZE_16, true);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "ReConfig copy too large returns OPERATION_SUCCESS, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(cache->GetCahceSize(), TEST_CACHE_SIZE,
        "cacheSize should remain unchanged at 64");
}

// ==================== ReIndex Tests ====================

/**
 * @tc.name  : Test ReIndex triggered by baseIndex reaching fence
 * @tc.number: AudioRingCache_ReIndex_Triggered
 * @tc.desc  : Test ReIndex is triggered when baseIndex >= BASE_INDEX_FENCE.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_ReIndex_Triggered, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    uint8_t data[TEST_DATA_SIZE_8] = {0};
    BufferWrap buffer = {data, TEST_DATA_SIZE_8};
    cache->Enqueue(buffer);
    cache->baseIndex_ = BASE_INDEX_FENCE;
    cache->writeIndex_ = BASE_INDEX_FENCE + TEST_DATA_SIZE_8;
    cache->readIndex_ = BASE_INDEX_FENCE;
    uint8_t out[TEST_DATA_SIZE_8] = {0};
    BufferWrap outBuf = {out, TEST_DATA_SIZE_8};
    auto result = cache->Dequeue(outBuf);
    AIUT_EXPECT_EQ_INFO_LOG(result.ret, OPERATION_SUCCESS,
        "Dequeue with ReIndex failed, ret=%d", result.ret);
    AIUT_EXPECT_EQ_INFO_LOG(cache->baseIndex_, 0U,
        "baseIndex should be 0 after ReIndex");
}

// ==================== Full Cycle Tests ====================

/**
 * @tc.name  : Test full enqueue-dequeue cycle
 * @tc.number: AudioRingCache_FullCycle
 * @tc.desc  : Test multiple enqueue-dequeue operations in a cycle.
 * @tc.owner : audio_route_common
 */
HWTEST_F(AudioRingCacheAiut, AudioRingCache_FullCycle, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto cache = CreateTestCache();
    AIUT_ASSERT_NE_INFO_LOG(cache, nullptr, "cache is null");
    for (int32_t i = 0; i < 4; i++) {
        uint8_t data[TEST_DATA_SIZE_16] = {0};
        BufferWrap buffer = {data, TEST_DATA_SIZE_16};
        auto enqResult = cache->Enqueue(buffer);
        AIUT_EXPECT_EQ_INFO_LOG(enqResult.ret, OPERATION_SUCCESS,
            "Enqueue cycle %d failed, ret=%d", i, enqResult.ret);
        uint8_t out[TEST_DATA_SIZE_16] = {0};
        BufferWrap outBuf = {out, TEST_DATA_SIZE_16};
        auto deqResult = cache->Dequeue(outBuf);
        AIUT_EXPECT_EQ_INFO_LOG(deqResult.ret, OPERATION_SUCCESS,
            "Dequeue cycle %d failed, ret=%d", i, deqResult.ret);
    }
    auto writable = cache->GetWritableSize();
    AIUT_EXPECT_EQ_INFO_LOG(writable.size, TEST_CACHE_SIZE,
        "writable should be 64 after full cycle");
}

}  // namespace AudioStandard
}  // namespace OHOS
