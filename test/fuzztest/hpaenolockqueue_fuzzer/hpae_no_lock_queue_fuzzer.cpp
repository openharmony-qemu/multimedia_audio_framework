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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "hpae_no_lock_queue.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const size_t TESTQUEUESIZE = 5;
static constexpr size_t TESTQUEUESIZETHREE = 3;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void RequestNodeFuzzTest()
{
    HPAE::RequestNode requestNode1;
    HPAE::RequestNode requestNode2(requestNode1);
}

void HpaeNoLockQueueFuzzTest()
{
    vector<uint32_t> requestCountList = {-1, 0, 1, 10000000, 10000001};
    size_t maxRequestCount = GetData<uint32_t>() % requestCountList.size();
    HPAE::HpaeNoLockQueue large_queue(maxRequestCount);
    large_queue.IsFinishProcess();
}

void PushRequestFuzzTest()
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    HPAE::HpaeNoLockQueue queue(TESTQUEUESIZETHREE);
    for (int i = 0; i < TESTQUEUESIZETHREE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.PushRequest(countingRequest);
    queue.HandleRequests();
}

void ResetFuzzTest()
{
    std::unique_ptr<HPAE::HpaeNoLockQueue> queue_ = std::make_unique<HPAE::HpaeNoLockQueue>(TESTQUEUESIZE);
    std::atomic<int> processed_count_ = 0;
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };

    HPAE::HpaeNoLockQueue queue(TESTQUEUESIZE);
    for (int i = 0; i < TESTQUEUESIZE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.Reset();
    queue.HandleRequests();
}

void GetRequsetFlagFuzzTest()
{
    std::unique_ptr<HPAE::HpaeNoLockQueue> queue_ = std::make_unique<HPAE::HpaeNoLockQueue>(GetData<size_t>());
    uint64_t requestFlag = GetData<uint64_t>();
    queue_->GetRequsetFlag(requestFlag);
}

void PushRequestNodeFuzzTest()
{
    std::unique_ptr<HPAE::HpaeNoLockQueue> queue_ = std::make_unique<HPAE::HpaeNoLockQueue>(GetData<size_t>());
    uint64_t index = GetData<uint64_t>();
    queue_->PushRequestNode(nullptr, index);
}

TestFuncs g_testFuncs[] = {
    RequestNodeFuzzTest,
    HpaeNoLockQueueFuzzTest,
    PushRequestFuzzTest,
    ResetFuzzTest,
    GetRequsetFlagFuzzTest,
    PushRequestNodeFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
