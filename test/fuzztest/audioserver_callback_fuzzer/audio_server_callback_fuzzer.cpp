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

#include <cinttypes>
#include <fuzzer/FuzzedDataProvider.h>

#include "audio_server.h"

namespace OHOS {
namespace AudioStandard {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static size_t g_count = 0;
typedef void (*TestPtr)(const uint8_t *, size_t);

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void FuzzTestRegisterCallbackHandle()
{
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    // Can not make fuzz remote object, so only test nullptr case now
    sptr<IRemoteObject> testClient = nullptr;
    AudioServerPtr->RegisterCallbackHandle(testClient);
}

void FuzzTestSetCallbackHandleEnable()
{
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t fuzzCallbackId = provider.ConsumeIntegral<uint32_t>();
    bool fuzzEnable = provider.ConsumeBool();

    AudioServerPtr->SetCallbackHandleEnable(fuzzCallbackId, fuzzEnable);
}

TestFuncs g_testFuncs[] = {
    FuzzTestRegisterCallbackHandle,
    FuzzTestSetCallbackHandleEnable,
};

void FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t len = sizeof(g_testFuncs) / sizeof(g_testFuncs[0]);
    if (len > 0) {
        g_testFuncs[g_count % len]();
        g_count++;
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    g_count = g_count == len ? 0 : g_count;

    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}