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
#include "audio_sink_latency_fetcher.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class SinkLatencyFetcherTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        SinkLatencyFetcherManager::GetInstance().RemoveFetcherById(RENDER_ID);
    }
    void TearDown() override
    {
        SinkLatencyFetcherManager::GetInstance().RemoveFetcherById(RENDER_ID);
    }
private:
    static constexpr uint32_t RENDER_ID = 1001;
};

/**
 * @tc.name  : SinkLatencyFetcherManager ensures provider exists before fetcher creation.
 * @tc.desc  : Expect nullptr fetcher when provider not registered.
 * @tc.type  : FUNC
 * @tc.level : Level1
 */
HWTEST_F(SinkLatencyFetcherTest, EnsureFetcher_NoProvider_ReturnsNull, TestSize.Level1)
{
    auto fetcher = SinkLatencyFetcherManager::GetInstance().EnsureFetcher(RENDER_ID);
    EXPECT_EQ(fetcher, nullptr);
}

/**
 * @tc.name  : SinkLatencyFetcherManager caches latency after first fetch.
 * @tc.desc  : First call invokes provider, second call uses cached value.
 * @tc.type  : FUNC
 * @tc.level : Level1
 */
HWTEST_F(SinkLatencyFetcherTest, EnsureFetcher_WithProvider_CacheHit, TestSize.Level1)
{
    int32_t callCount = 0;
    SinkLatencyFetcherManager::GetInstance().RegisterProvider(RENDER_ID,
        [&callCount](uint32_t renderId, uint32_t &latency) -> int32_t {
            callCount++;
            latency = 15 + renderId % 5;
            return SUCCESS;
        });

    auto fetcher = SinkLatencyFetcherManager::GetInstance().EnsureFetcher(RENDER_ID);
    ASSERT_NE(fetcher, nullptr);

    uint32_t latency = 0;
    int32_t ret = fetcher(latency);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 15 + RENDER_ID % 5);
    EXPECT_EQ(callCount, 1);

    latency = 0;
    ret = fetcher(latency);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, 15 + RENDER_ID % 5);
    EXPECT_EQ(callCount, 1); // cached, provider not called again
}

/**
 * @tc.name  : SinkLatencyFetcherManager propagates provider failure.
 * @tc.desc  : Provider failure returns error and does not cache.
 * @tc.type  : FUNC
 * @tc.level : Level1
 */
HWTEST_F(SinkLatencyFetcherTest, EnsureFetcher_WithProvider_Failure, TestSize.Level1)
{
    int32_t callCount = 0;
    SinkLatencyFetcherManager::GetInstance().RegisterProvider(RENDER_ID,
        [&callCount](uint32_t renderId, uint32_t &latency) -> int32_t {
            callCount++;
            (void)renderId;
            (void)latency;
            return ERR_OPERATION_FAILED;
        });

    auto fetcher = SinkLatencyFetcherManager::GetInstance().EnsureFetcher(RENDER_ID);
    ASSERT_NE(fetcher, nullptr);

    uint32_t latency = 0;
    int32_t ret = fetcher(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_EQ(callCount, 1);

    // retry should invoke provider again because cache not set
    ret = fetcher(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_EQ(callCount, 2);
}

/**
 * @tc.name  : SinkLatencyFetcherManager accepts retry hint without caching.
 * @tc.desc  : Provider can return ERR_RETRY_IN_CLIENT with default value; fetcher returns SUCCESS but does not cache.
 * @tc.type  : FUNC
 * @tc.level : Level1
 */
HWTEST_F(SinkLatencyFetcherTest, EnsureFetcher_WithProvider_RetryInClient, TestSize.Level1)
{
    int32_t callCount = 0;
    const uint32_t defaultLatency = 42;
    SinkLatencyFetcherManager::GetInstance().RegisterProvider(RENDER_ID,
        [&callCount, defaultLatency](uint32_t renderId, uint32_t &latency) -> int32_t {
            callCount++;
            latency = defaultLatency + renderId % 3;
            return ERR_LATENCY_DEFAULT_VALUE;
        });

    auto fetcher = SinkLatencyFetcherManager::GetInstance().EnsureFetcher(RENDER_ID);
    ASSERT_NE(fetcher, nullptr);

    uint32_t latency = 0;
    int32_t ret = fetcher(latency);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, defaultLatency + RENDER_ID % 3);
    EXPECT_EQ(callCount, 1);

    latency = 0;
    ret = fetcher(latency);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(latency, defaultLatency + RENDER_ID % 3);
    EXPECT_EQ(callCount, 2); // not cached when provider returns ERR_LATENCY_DEFAULT_VALUE
}
} // namespace AudioStandard
} // namespace OHOS
