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

#ifndef AUDIO_SINK_LATENCY_FETCHER_H
#define AUDIO_SINK_LATENCY_FETCHER_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
class SinkLatencyFetcherManager {
public:
    using FetchLatencyFunc = std::function<int32_t (uint32_t renderId, uint32_t &latency)>;

    static SinkLatencyFetcherManager &GetInstance();

    void RegisterProvider(uint32_t renderId, const FetchLatencyFunc &provider);

    // Called when route switches; stream retrieves or reuses the latency fetcher for given renderId through this
    // interface.
    std::function<int32_t (uint32_t &)> EnsureFetcher(uint32_t renderId);
    void RemoveFetcherById(uint32_t renderId);

private:
    class SinkLatencyFetcher {
    public:
        SinkLatencyFetcher(uint32_t renderId, FetchLatencyFunc provider);
        int32_t operator()(uint32_t &latency);

    private:
        struct SinkLatencyCacheEntry {
            std::atomic<bool> cached {false};
            std::atomic<uint32_t> value {0};
        };

        const uint32_t renderId_;
        // Action: provider that fetches latency from sink side.
        FetchLatencyFunc provider_;
        // Data: shared cache entry reused by fetch calls for the same renderId.
        const std::shared_ptr<SinkLatencyCacheEntry> cacheEntry_;
    };

    SinkLatencyFetcherManager() = default;
    ~SinkLatencyFetcherManager() = default;
    SinkLatencyFetcherManager(const SinkLatencyFetcherManager &) = delete;
    SinkLatencyFetcherManager &operator=(const SinkLatencyFetcherManager &) = delete;
    SinkLatencyFetcherManager(SinkLatencyFetcherManager &&) = delete;
    SinkLatencyFetcherManager &operator=(SinkLatencyFetcherManager &&) = delete;

    std::function<int32_t (uint32_t &)> CreateFetcher(uint32_t renderId, const FetchLatencyFunc &provider);

    std::mutex mutex_;
    std::unordered_map<uint32_t, std::function<int32_t (uint32_t &)>> fetcherMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SINK_LATENCY_FETCHER_H
