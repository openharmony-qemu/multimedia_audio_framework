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
#include "capturer_clock.h"
#include "capturer_clock_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 5;
static int32_t NUM_2 = 2;
constexpr uint64_t MOCK_POSITION_INC = 960;
constexpr uint32_t MOCK_SAMPLE_RATE = 48000;
constexpr uint32_t MOCK_SAMPLE_RATE_2 = 96000;
constexpr uint64_t MOCK_TIMESTAMP_1 = 1000000000;
constexpr uint64_t MOCK_TIMESTAMP_2 = 1020000000;
constexpr uint64_t MOCK_TIMESTAMP_4 = 1100000000;
constexpr uint64_t MOCK_TIMESTAMP_5 = 1120000000;
constexpr uint64_t MOCK_POSITION_1 = 0;
constexpr uint64_t MOCK_POSITION_2 = 960;
constexpr uint64_t MOCK_POSITION_3 = 1920;
constexpr uint64_t MOCK_POSITION_4 = 2880;
constexpr uint64_t MOCK_POSITION_5 = 3840;

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

void GetMediaRenderDeviceFuzzTest()
{
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    CapturerClockManager::GetInstance().GetCapturerClock(1);
    CapturerClockManager::GetInstance().GetCapturerClock(0);
    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
}

void GetRecordCaptureDeviceFuzzTest()
{
    shared_ptr<AudioSourceClock> srcClock = make_shared<AudioSourceClock>();
    CapturerClockManager::GetInstance().audioSrcClockPool_.size();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock);
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(1, srcClock);
    CapturerClockManager::GetInstance().audioSrcClockPool_.size();

    CapturerClockManager::GetInstance().GetAudioSourceClock(1);
    CapturerClockManager::GetInstance().GetAudioSourceClock(0);
    CapturerClockManager::GetInstance().audioSrcClockPool_.size();

    CapturerClockManager::GetInstance().DeleteAudioSourceClock(1);
    CapturerClockManager::GetInstance().audioSrcClockPool_.size();
}

void CaptureClockStartAndStopFuzzTest()
{
    uint32_t capturerSampleRate = GetData<uint32_t>();
    CapturerClock clock(capturerSampleRate);
    clock.Start();
    uint64_t time = GetData<uint64_t>();
    uint32_t srcSampleRate = GetData<uint32_t>();
    uint64_t posIncSize = GetData<uint64_t>();

    clock.SetTimeStampByPosition(time, srcSampleRate, posIncSize);
    clock.Stop();
}

void GetTimeStampByPositionNormalFuzzTest()
{
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    std::shared_ptr<CapturerClock> capturerClock_ = CapturerClockManager::GetInstance().GetCapturerClock(1);
    if (capturerClock_ == nullptr) {
        return;
    }
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_1, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);

    capturerClock_->Start();
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_1, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_2, MOCK_SAMPLE_RATE, MOCK_POSITION_INC);
    uint64_t timestamp;
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_1, timestamp);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_2, timestamp);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_3, timestamp);
    capturerClock_->Stop();
    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
}

void GetTimeStampByPositionDifferentFuzzTest()
{
    CapturerClockManager::GetInstance().CreateCapturerClock(1, MOCK_SAMPLE_RATE);
    std::shared_ptr<CapturerClock> capturerClock_ = CapturerClockManager::GetInstance().GetCapturerClock(1);
    if (capturerClock_ == nullptr) {
        return;
    }
    capturerClock_->Start();

    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_4, MOCK_SAMPLE_RATE_2, MOCK_POSITION_INC * NUM_2);
    capturerClock_->SetTimeStampByPosition(MOCK_TIMESTAMP_5, MOCK_SAMPLE_RATE_2, MOCK_POSITION_INC * NUM_2);

    uint64_t timestamp;
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_4, timestamp);
    capturerClock_->GetTimeStampByPosition(MOCK_POSITION_5, timestamp);
    CapturerClockManager::GetInstance().DeleteCapturerClock(1);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    GetMediaRenderDeviceFuzzTest,
    GetRecordCaptureDeviceFuzzTest,
    CaptureClockStartAndStopFuzzTest,
    GetTimeStampByPositionNormalFuzzTest,
    GetTimeStampByPositionDifferentFuzzTest,
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
