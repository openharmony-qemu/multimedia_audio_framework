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
#include <vector>
#include <atomic>
#include <functional>
#include <cstdint>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const size_t CURRENT_REQUEST_COUNT = 10000;
using Request = std::function<void()>;
struct RequestNode {
    RequestNode() = default;
    RequestNode(const RequestNode &requestNode) : nextRequestIndex()
    {}
    RequestNode& operator=(const RequestNode& requestNode) = delete;
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
    uint64_t IncRequsetIndex(uint64_t requestIndex);
    uint64_t GetRequsetIndex(uint64_t requestIndex);
    uint64_t GetRequsetFlag(uint64_t requestFlag);

    void PushRequestNode(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t index);
    uint64_t GetRequestNode(std::atomic<uint64_t> *pRequestHeadIndex);
    void ProcessRequests(uint64_t requestHeadIndex, bool isProcess);

private:
    std::atomic<uint64_t> freeRequestHeadIndex_;
    std::atomic<uint64_t> requestHeadIndex_;
    std::vector<RequestNode> requestQueue_;
    std::vector<Request> tempRequestQueue_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
