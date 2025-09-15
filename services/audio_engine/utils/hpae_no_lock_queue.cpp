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

#include <cstdio>
#include "hpae_no_lock_queue.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MAX_REQUEST_COUNT = 10000000;
constexpr uint32_t INVALID_REQUEST_ID = std::numeric_limits<uint32_t>::max();
constexpr uint64_t SHIFT_32_OFFSET = 32;
HpaeNoLockQueue::HpaeNoLockQueue(size_t maxRequestCount)
{
    if (maxRequestCount > MAX_REQUEST_COUNT) {
        AUDIO_WARNING_LOG("maxRequestCount %{public}zu is beyound Max Count", maxRequestCount);
        maxRequestCount = MAX_REQUEST_COUNT;
    }
    InitQueue(maxRequestCount);
}

HpaeNoLockQueue::~HpaeNoLockQueue()
{
    AUDIO_INFO_LOG("destroyed");
}
void HpaeNoLockQueue::InitQueue(size_t maxRequestCount)
{
    CHECK_AND_RETURN_LOG(maxRequestCount > 0, "maxRequestCount = 0");
    requestQueue_.resize(maxRequestCount);
    tempRequestQueue_.reserve(maxRequestCount);

    freeRequestHeadIndex_ = 0;
    for (size_t i = 0; i < maxRequestCount - 1; ++i) {
        requestQueue_[i].nextRequestIndex = i + 1;
    }
    requestQueue_[maxRequestCount - 1].nextRequestIndex = INVALID_REQUEST_ID;
    requestHeadIndex_ = INVALID_REQUEST_ID;
    AUDIO_INFO_LOG("size is %{public}zu", maxRequestCount);
}
void HpaeNoLockQueue::PushRequest(Request &&request)
{
    const uint64_t freeRequestIndex = GetRequestNode(&freeRequestHeadIndex_);
    if (GetRequsetIndex(freeRequestIndex) == INVALID_REQUEST_ID) {
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_NO_LOCK_QUEUE_TYPE, __func__,
            "reached Queue Capacity");
        AUDIO_WARNING_LOG("reached Queue Capacity: drop this request");
        return;
    }
    requestQueue_[GetRequsetIndex(freeRequestIndex)].request = std::move(request);
    PushRequestNode(&requestHeadIndex_, freeRequestIndex);
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
    const uint64_t oldRequestFlag = (GetRequsetFlag(requestHeadIndex_) << SHIFT_32_OFFSET) + INVALID_REQUEST_ID;
    const uint64_t oldRequestHeadindex = requestHeadIndex_.exchange(oldRequestFlag);
    ProcessRequests(oldRequestHeadindex, false);
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

void HpaeNoLockQueue::PushRequestNode(std::atomic<uint64_t> *pRequestHeadIndex, uint64_t index)
{
    if (pRequestHeadIndex == nullptr) {
        return;
    }
    uint64_t requestHeadIndex;
    do {
        requestHeadIndex = pRequestHeadIndex->load();
        requestQueue_[GetRequsetIndex(index)].nextRequestIndex = requestHeadIndex;
    } while (!std::atomic_compare_exchange_strong(pRequestHeadIndex, &requestHeadIndex, index));
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
        nextRequestIndex = requestQueue_[GetRequsetIndex(requestHeadIndex)].nextRequestIndex;
    } while (!std::atomic_compare_exchange_strong(pRequestHeadIndex, &requestHeadIndex, nextRequestIndex));
    return IncRequsetIndex(requestHeadIndex);
}

void HpaeNoLockQueue::ProcessRequests(uint64_t requestHeadIndex, bool isProcess)
{
    uint64_t tempIndex = requestHeadIndex;
    while (GetRequsetIndex(tempIndex) != INVALID_REQUEST_ID) {
        RequestNode *tempRequest = &requestQueue_[GetRequsetIndex(tempIndex)];
        uint64_t nextRequest = tempRequest->nextRequestIndex;
        tempRequestQueue_.emplace_back(std::move(tempRequest->request));
        tempRequest->request = nullptr;
        PushRequestNode(&freeRequestHeadIndex_, tempIndex);
        tempIndex = nextRequest;
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
