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
#ifndef LOG_TAG
#define LOG_TAG "AudioSinkLatencyFetcher"
#endif

#include "audio_sink_latency_fetcher.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {
SinkLatencyFetcherManager &SinkLatencyFetcherManager::GetInstance()
{
    static SinkLatencyFetcherManager instance;
    return instance;
}

SinkLatencyFetcherManager::SinkLatencyFetcher::SinkLatencyFetcher(
    uint32_t renderId, FetchLatencyFunc provider, std::shared_ptr<LatencyCache> cache)
    : renderId_(renderId), provider_(std::move(provider)),
      cacheEntry_(std::move(cache))
{
}

int32_t SinkLatencyFetcherManager::SinkLatencyFetcher::operator()(uint32_t &latency)
{
    if (cacheEntry_->cached.load()) {
        latency = cacheEntry_->value.load();
        return SUCCESS;
    }
    uint32_t sinkLatency = 0;
    CHECK_AND_RETURN_RET_LOG(provider_, ERR_INVALID_PARAM, "provider is null for renderId %{public}u", renderId_);
    int32_t ret = provider_(renderId_, sinkLatency);
    if (ret != SUCCESS) {
        if (ret == ERR_LATENCY_DEFAULT_VALUE) {
            AUDIO_ERR_LOG("Latency provider returns default value for renderId %{public}u", renderId_);
            latency = sinkLatency;
            return SUCCESS;
        }
        return ret;
    }
    cacheEntry_->value.store(sinkLatency);
    cacheEntry_->cached.store(true);
    latency = sinkLatency;
    return SUCCESS;
}

std::function<int32_t (uint32_t &)> SinkLatencyFetcherManager::CreateFetcher(
    uint32_t renderId, const FetchLatencyFunc &provider, std::shared_ptr<LatencyCache> &cacheOut)
{
    cacheOut = std::make_shared<LatencyCache>();
    return SinkLatencyFetcher(renderId, provider, cacheOut);
}

void SinkLatencyFetcherManager::RegisterProvider(uint32_t renderId, const FetchLatencyFunc &provider,
    uint32_t warmupLatency)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<LatencyCache> cache;
    auto fetcher = CreateFetcher(renderId, provider, cache);
    fetcherMap_[renderId] = FetcherEntry{std::move(fetcher), std::move(cache), warmupLatency};
}

std::function<int32_t (uint32_t &)> SinkLatencyFetcherManager::EnsureFetcher(uint32_t renderId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = fetcherMap_.find(renderId);
    CHECK_AND_RETURN_RET_LOG(it != fetcherMap_.end(), nullptr, "fetcher not found for renderId %{public}u", renderId);
    return it->second.fetcher;
}

void SinkLatencyFetcherManager::RemoveFetcherById(uint32_t renderId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    fetcherMap_.erase(renderId);
}

bool SinkLatencyFetcherManager::TryGetCachedLatency(uint32_t renderId, uint32_t &latency) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = fetcherMap_.find(renderId);
    if (it == fetcherMap_.end()) {
        return false;
    }
    if (!it->second.cache->cached.load()) {
        return false;
    }
    latency = it->second.cache->value.load();
    return true;
}

uint32_t SinkLatencyFetcherManager::GetWarmupLatency(uint32_t renderId) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = fetcherMap_.find(renderId);
    if (it == fetcherMap_.end()) {
        return 0;
    }
    return it->second.warmupLatency;
}
} // namespace AudioStandard
} // namespace OHOS
