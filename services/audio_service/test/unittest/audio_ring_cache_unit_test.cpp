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
#include "audio_ring_cache.h"
#include "audio_service_log.h"

#include "securec.h"

using namespace std;
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {

static const size_t MAX_CACHE_SIZE = 16 * 1024 * 1024; // 16M
static const size_t BASE_INDEX_FENCE = SIZE_MAX - 2 * MAX_CACHE_SIZE;

class Test : public ::testing::Test {
protected:
void SetUp() override {}
void TearDown() override {}
};

class AudioRingCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test AudioRingCacheTest API
 * @tc.type  : FUNC
 * @tc.number: AudioRingCache_001
 * @tc.desc  : Test AudioRingCacheTest interface.
 */
HWTEST(AudioRingCacheTest, AudioRingCache_001, TestSize.Level0)
{
    size_t cacheSize = 64;
    std::unique_ptr<AudioRingCache> audioringcache = std::make_unique<AudioRingCache>(cacheSize);

    EXPECT_EQ(nullptr, audioringcache->Create(MAX_CACHE_SIZE + 1));
}

/**
 * @tc.name  : Test AudioRingCacheTest API
 * @tc.type  : FUNC
 * @tc.number: AudioRingCache_002
 * @tc.desc  : Test AudioRingCacheTest interface.
 */

HWTEST(AudioRingCacheTest, AudioRingCache_002, TestSize.Level0)
{
    size_t cacheSize = 64;
    std::unique_ptr<AudioRingCache> audioringcache = std::make_unique<AudioRingCache>(cacheSize);

    audioringcache->writeIndex_ = 4;
    audioringcache->readIndex_ = 8;
    audioringcache->cacheTotalSize_ = 0;
    BufferWrap buffer;
    OptResult result = audioringcache->Enqueue(buffer);
    EXPECT_EQ(INVALID_PARAMS, result.ret);

    audioringcache->baseIndex_ = 2;
    audioringcache->writeIndex_ = 12;
    audioringcache->cacheTotalSize_ = 8;
    result = audioringcache->Enqueue(buffer);
    EXPECT_EQ(0, result.size);

    audioringcache->baseIndex_ = 64;
    result = audioringcache->Enqueue(buffer);
    EXPECT_EQ(8, audioringcache->readIndex_);
}

/**
 * @tc.name  : Test AudioRingCacheTest API
 * @tc.type  : FUNC
 * @tc.number: AudioRingCache_003
 * @tc.desc  : Test AudioRingCacheTest interface.
 */
HWTEST(AudioRingCacheTest, AudioRingCache_003, TestSize.Level0)
{
    size_t cacheSize = 64;
    std::unique_ptr<AudioRingCache> audioringcache = std::make_unique<AudioRingCache>(cacheSize);

    BufferWrap buffer;
    audioringcache->writeIndex_ = 4;
    audioringcache->readIndex_ = 8;
    audioringcache->cacheTotalSize_ = 0;
    audioringcache->baseIndex_ = BASE_INDEX_FENCE;
    audioringcache->GetWritableSizeNoLock();
    OptResult result = audioringcache->Dequeue(buffer);
    EXPECT_EQ(0, result.size);

    audioringcache->baseIndex_ = 10;
    audioringcache->GetWritableSizeNoLock();
    result = audioringcache->Dequeue(buffer);
    EXPECT_EQ(INVALID_PARAMS, result.ret);

    audioringcache->baseIndex_ = 2;
    audioringcache->writeIndex_ = 12;
    audioringcache->cacheTotalSize_ = 8;
    audioringcache->GetWritableSizeNoLock();
    result = audioringcache->Dequeue(buffer);
    EXPECT_EQ(INVALID_PARAMS, result.ret);
}
} // namespace AudioStandard
} // namespace OHOS
