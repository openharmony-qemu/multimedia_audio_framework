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
#ifndef NOLOCK_REQUEST_QUEUE_H
#define NOLOCK_REQUEST_QUEUE_H
#include <array>
#include <vector>
#include <atomic>
#include <functional>
#include <cstdint>
#include <memory>
#include <mutex>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const size_t CURRENT_REQUEST_COUNT = 1000;
const size_t MAX_REQUEST_COUNT = 5000;
const size_t EXPAND_REQUEST_COUNT = CURRENT_REQUEST_COUNT;
const size_t REQUEST_BLOCK_COUNT = MAX_REQUEST_COUNT / EXPAND_REQUEST_COUNT;
const size_t EXPAND_THRESHOLD_PERCENT = 90;
const size_t PERCENTAGE_BASE = 100;
using Request = std::function<void()>;
struct RequestNode {
    RequestNode() = default;
    RequestNode(const RequestNode &requestNode) : request(requestNode.request), nextRequestIndex()
    {}
    RequestNode &operator=(const RequestNode &requestNode) = delete;
    Request request;
    std::atomic<uint64_t> nextRequestIndex;
};

class HpaeNoLockQueue {
public:
    explicit HpaeNoLockQueue(size_t maxRequestCount);
    ~HpaeNoLockQueue();

    void PushRequest(Request &&request);
    void HandleRequests();
    void Reset();
    bool IsFinishProcess();

private:
    void InitQueue(size_t maxRequestCount);
    bool TryExpandQueue(bool forceExpand = false);
    size_t GetExpandThreshold(size_t requestCount) const;
    uint64_t IncRequsetIndex(uint64_t requestIndex);
    uint64_t GetRequsetIndex(uint64_t requestIndex);
    uint64_t GetRequsetFlag(uint64_t requestFlag);
    RequestNode *GetRequestNodePtr(uint64_t requestIndex);

    void PushRequestNode(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t index);
    void PushRequestNodeChain(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t firstIndex, uint64_t lastIndex);
    uint64_t GetRequestNode(std::atomic<uint64_t> *pRequestHeadIndex);
    void ProcessRequests(uint64_t requestHeadIndex, bool isProcess);

private:
    std::atomic<uint64_t> freeRequestHeadIndex_ = 0;
    std::atomic<uint64_t> requestHeadIndex_ = 0;
    std::atomic<size_t> requestCount_ = 0;
    std::atomic<size_t> usedRequestCount_ = 0;
    std::array<std::unique_ptr<RequestNode[]>, REQUEST_BLOCK_COUNT> requestQueue_;
    std::vector<Request> tempRequestQueue_;
    std::mutex expandMutex_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
