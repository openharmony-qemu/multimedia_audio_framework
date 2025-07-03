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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 5;
const int32_t NUM_2 = 2;
const int32_t TEST_MAX_REQUEST = 7680;

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

void SetLimiterConfigFuzzTest()
{
    AudioLmtManager *limiterManager = AudioLmtManager::GetInstance();
    int32_t sinkIndex = GetData<uint32_t>();
    limiterManager->CreateLimiter(sinkIndex);
    int32_t audioChannel = static_cast<int32_t>(AudioChannel::CHANNEL_16) + 1;
    int32_t channels = static_cast<AudioChannel>(GetData<uint8_t>() % audioChannel);
    limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, channels);
}

void ProcessLimiterFuzzTest()
{
    AudioLmtManager *limiterManager = AudioLmtManager::GetInstance();
    int32_t sinkIndex = GetData<uint32_t>();
    limiterManager->CreateLimiter(sinkIndex);
    limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    int32_t frameLength = GetData<uint32_t>() % NUM_2 == 0 ? frameLen : 0;
    limiterManager->ProcessLimiter(sinkIndex, frameLength, inBuffer, outBuffer);
}

void ReleaseLimiterFuzzTest()
{
    AudioLmtManager *limiterManager = AudioLmtManager::GetInstance();
    int32_t sinkIndex = GetData<uint32_t>();
    limiterManager->CreateLimiter(sinkIndex);
    limiterManager->ReleaseLimiter(sinkIndex);
    limiterManager->ReleaseLimiter(sinkIndex);
}

void GetLatencyFuzzTest()
{
    AudioLmtManager *limiterManager = AudioLmtManager::GetInstance();
    int32_t sinkIndex = GetData<uint32_t>();
    limiterManager->CreateLimiter(sinkIndex);
    limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    limiterManager->GetLatency(sinkIndex);
}

void CreateLimiterFuzzTest()
{
    AudioLmtManager *limiterManager = AudioLmtManager::GetInstance();
    int32_t sinkIndex = GetData<int32_t>();
    limiterManager->CreateLimiter(sinkIndex);
    limiterManager->CreateLimiter(sinkIndex);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    SetLimiterConfigFuzzTest,
    ProcessLimiterFuzzTest,
    ReleaseLimiterFuzzTest,
    GetLatencyFuzzTest,
    CreateLimiterFuzzTest,
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
