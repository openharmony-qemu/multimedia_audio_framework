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

#include <algorithm>
#include <cstdio>
#include <limits>
#include "hpae_no_lock_queue.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t INVALID_REQUEST_ID = std::numeric_limits<uint32_t>::max();
constexpr uint64_t SHIFT_32_OFFSET = 32;

HpaeNoLockQueue::HpaeNoLockQueue(size_t maxRequestCount)
{
    if (maxRequestCount != CURRENT_REQUEST_COUNT) {
        AUDIO_WARNING_LOG("maxRequestCount %{public}zu is not equal current request count", maxRequestCount);
        maxRequestCount = CURRENT_REQUEST_COUNT;
    }
    InitQueue(maxRequestCount);
}

HpaeNoLockQueue::~HpaeNoLockQueue()
{
    AUDIO_INFO_LOG(" destroyed");
}
void HpaeNoLockQueue::InitQueue(size_t maxRequestCount)
{
    CHECK_AND_RETURN_LOG(maxRequestCount > 0, "maxRequestCount = 0");
    requestCount_ = maxRequestCount;
    requestQueue_[0] = std::make_unique<RequestNode[]>(maxRequestCount);
    tempRequestQueue_.reserve(maxRequestCount);

    freeRequestHeadIndex_ = 0;
    for (size_t i = 0; i < maxRequestCount - 1; ++i) {
        requestQueue_[0][i].nextRequestIndex = i + 1;
    }
    requestQueue_[0][maxRequestCount - 1].nextRequestIndex = INVALID_REQUEST_ID;
    requestHeadIndex_ = INVALID_REQUEST_ID;
    AUDIO_INFO_LOG("size is %{public}zu", maxRequestCount);
}
void HpaeNoLockQueue::PushRequest(Request &&request)
{
    uint64_t freeRequestIndex = GetRequestNode(&freeRequestHeadIndex_);
    if (GetRequsetIndex(freeRequestIndex) == INVALID_REQUEST_ID) {
        if (TryExpandQueue(true)) {
            freeRequestIndex = GetRequestNode(&freeRequestHeadIndex_);
        }
        if (GetRequsetIndex(freeRequestIndex) == INVALID_REQUEST_ID) {
            HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_NO_LOCK_QUEUE_TYPE, __func__,
                "reached Queue Capacity");
            AUDIO_WARNING_LOG("reached Queue Capacity: drop this request");
            return;
        }
    }
    RequestNode *requestNode = GetRequestNodePtr(freeRequestIndex);
    CHECK_AND_RETURN_LOG(requestNode != nullptr, "requestNode is nullptr");
    requestNode->request = std::move(request);
    PushRequestNode(&requestHeadIndex_, freeRequestIndex);
    if (usedRequestCount_.fetch_add(1) + 1 >= GetExpandThreshold(requestCount_.load())) {
        TryExpandQueue();
    }
}

void HpaeNoLockQueue::HandleRequests()
{
    uint64_t oldRequestFlag;
    uint64_t requestHeadindex;
    do {
        requestHeadindex = requestHeadIndex_.load();
        oldRequestFlag = (GetRequsetFlag(requestHeadindex) << SHIFT_32_OFFSET) + INVALID_REQUEST_ID;
    } while (!std::atomic_compare_exchange_strong(&requestHeadIndex_, &requestHeadindex, oldRequestFlag));
    ProcessRequests(requestHeadindex, true);
}

void HpaeNoLockQueue::Reset()
{
    const uint64_t oldRequestFlag = (GetRequsetFlag(requestHeadIndex_.load()) << SHIFT_32_OFFSET) + INVALID_REQUEST_ID;
    const uint64_t oldRequestHeadindex = requestHeadIndex_.exchange(oldRequestFlag);
    ProcessRequests(oldRequestHeadindex, false);
}

size_t HpaeNoLockQueue::GetExpandThreshold(size_t requestCount) const
{
    return requestCount * EXPAND_THRESHOLD_PERCENT / PERCENTAGE_BASE;
}

bool HpaeNoLockQueue::TryExpandQueue(bool forceExpand)
{
    size_t requestCount = requestCount_.load();
    if (requestCount >= MAX_REQUEST_COUNT) {
        return false;
    }
    if (!forceExpand && usedRequestCount_.load() < GetExpandThreshold(requestCount)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(expandMutex_);
    requestCount = requestCount_.load();
    if (requestCount >= MAX_REQUEST_COUNT) {
        return false;
    }
    if (forceExpand && GetRequsetIndex(freeRequestHeadIndex_.load()) != INVALID_REQUEST_ID) {
        return true;
    }
    if (!forceExpand && usedRequestCount_.load() < GetExpandThreshold(requestCount)) {
        return false;
    }

    size_t newRequestCount = std::min(requestCount + EXPAND_REQUEST_COUNT, MAX_REQUEST_COUNT);
    size_t appendCount = newRequestCount - requestCount;
    size_t requestBlockIndex = requestCount / EXPAND_REQUEST_COUNT;
    CHECK_AND_RETURN_RET_LOG(requestBlockIndex < requestQueue_.size(), false, "requestBlockIndex is invalid");
    requestQueue_[requestBlockIndex] = std::make_unique<RequestNode[]>(appendCount);
    for (size_t i = 0; i < appendCount; ++i) {
        size_t requestIndex = requestCount + i;
        requestQueue_[requestBlockIndex][i].request = nullptr;
        requestQueue_[requestBlockIndex][i].nextRequestIndex = (requestIndex + 1 < newRequestCount) ?
            (requestIndex + 1) : INVALID_REQUEST_ID;
    }
    requestCount_ = newRequestCount;
    PushRequestNodeChain(&freeRequestHeadIndex_, requestCount, newRequestCount - 1);
    AUDIO_INFO_LOG("expand queue from %{public}zu to %{public}zu", requestCount, newRequestCount);
    return true;
}

uint64_t HpaeNoLockQueue::IncRequsetIndex(uint64_t requestIndex)
{
    return requestIndex + (static_cast<uint64_t>(1) << SHIFT_32_OFFSET);
}

uint64_t HpaeNoLockQueue::GetRequsetIndex(uint64_t requestIndex)
{
    return requestIndex & std::numeric_limits<uint32_t>::max();
}

uint64_t HpaeNoLockQueue::GetRequsetFlag(uint64_t requestFlag)
{
    return requestFlag >> SHIFT_32_OFFSET;
}

RequestNode *HpaeNoLockQueue::GetRequestNodePtr(uint64_t requestIndex)
{
    uint64_t index = GetRequsetIndex(requestIndex);
    if (index == INVALID_REQUEST_ID || index >= requestCount_.load()) {
        return nullptr;
    }
    if (index < CURRENT_REQUEST_COUNT) {
        return &requestQueue_[0][index];
    }
    size_t requestBlockIndex = index / EXPAND_REQUEST_COUNT;
    size_t requestBlockOffset = index % EXPAND_REQUEST_COUNT;
    CHECK_AND_RETURN_RET_LOG(requestBlockIndex < requestQueue_.size() && requestQueue_[requestBlockIndex] != nullptr,
        nullptr, "requestBlockIndex is invalid");
    return &requestQueue_[requestBlockIndex][requestBlockOffset];
}

void HpaeNoLockQueue::PushRequestNode(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t index)
{
    if (pRequestHeadIndex == nullptr) {
        return;
    }
    RequestNode *requestNode = GetRequestNodePtr(index);
    if (requestNode == nullptr) {
        return;
    }
    uint64_t requestHeadIndex;
    do {
        requestHeadIndex = pRequestHeadIndex->load();
        requestNode->nextRequestIndex = requestHeadIndex;
    } while (!std::atomic_compare_exchange_strong(pRequestHeadIndex, &requestHeadIndex, index));
}

void HpaeNoLockQueue::PushRequestNodeChain(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t firstIndex,
    uint64_t lastIndex)
{
    if (pRequestHeadIndex == nullptr) {
        return;
    }
    RequestNode *lastRequestNode = GetRequestNodePtr(lastIndex);
    if (lastRequestNode == nullptr) {
        return;
    }
    uint64_t requestHeadIndex;
    do {
        requestHeadIndex = pRequestHeadIndex->load();
        lastRequestNode->nextRequestIndex = requestHeadIndex;
    } while (!std::atomic_compare_exchange_strong(pRequestHeadIndex, &requestHeadIndex, firstIndex));
}

uint64_t HpaeNoLockQueue::GetRequestNode(std::atomic<uint64_t> *pRequestHeadIndex)
{
    if (pRequestHeadIndex == nullptr) {
        return std::numeric_limits<uint64_t>::max();
    }
    uint64_t requestHeadIndex;
    uint64_t nextRequestIndex;
    do {
        requestHeadIndex = pRequestHeadIndex->load();
        if (GetRequsetIndex(requestHeadIndex) == INVALID_REQUEST_ID) {
            return INVALID_REQUEST_ID;
        }
        RequestNode *requestNode = GetRequestNodePtr(requestHeadIndex);
        if (requestNode == nullptr) {
            return INVALID_REQUEST_ID;
        }
        nextRequestIndex = requestNode->nextRequestIndex;
    } while (!std::atomic_compare_exchange_strong(pRequestHeadIndex, &requestHeadIndex, nextRequestIndex));
    return IncRequsetIndex(requestHeadIndex);
}

void HpaeNoLockQueue::ProcessRequests(uint64_t requestHeadIndex, bool isProcess)
{
    if (tempRequestQueue_.capacity() < requestCount_.load()) {
        tempRequestQueue_.reserve(requestCount_.load());
    }
    uint64_t tempIndex = requestHeadIndex;
    size_t requestCount = 0;
    while (GetRequsetIndex(tempIndex) != INVALID_REQUEST_ID) {
        RequestNode *tempRequest = GetRequestNodePtr(tempIndex);
        CHECK_AND_BREAK_LOG(tempRequest != nullptr, "tempRequest is nullptr");
        uint64_t nextRequest = tempRequest->nextRequestIndex;
        tempRequestQueue_.emplace_back(std::move(tempRequest->request));
        tempRequest->request = nullptr;
        PushRequestNode(&freeRequestHeadIndex_, tempIndex);
        tempIndex = nextRequest;
        requestCount++;
    }
    if (requestCount > 0) {
        usedRequestCount_.fetch_sub(requestCount);
    }

    if (isProcess) {
        for (std::vector<Request>::reverse_iterator requestIter = tempRequestQueue_.rbegin();
             requestIter != tempRequestQueue_.rend();
             ++requestIter) {
            if (*requestIter != nullptr) {
                (*requestIter)();
            }
        }
    }
    tempRequestQueue_.clear();
}

bool HpaeNoLockQueue::IsFinishProcess()
{
    if (GetRequsetIndex(requestHeadIndex_) == INVALID_REQUEST_ID) {
        return true;
    } else {
        return false;
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
