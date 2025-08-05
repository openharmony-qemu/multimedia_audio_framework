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
#include "audio_utils.h"
#include "audio_log.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_performance_monitor.h"
#include "audio_performance_monitor_c.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 10;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
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

void RecordSilenceStateFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    bool isSilence = GetData<uint8_t>() % NUM_2;
    int32_t audioPipeTypeCount = static_cast<int32_t>(AudioPipeType::PIPE_TYPE_DIRECT_VOIP) + 1;
    AudioPipeType pipeType = static_cast<AudioPipeType>(GetData<uint8_t>() % audioPipeTypeCount);
    uint32_t uid = GetData<uint32_t>();
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, isSilence, pipeType, uid);
}

void DeleteSilenceMonitorFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    AudioPerformanceMonitor::GetInstance().DeleteSilenceMonitor(sessionId);
}

void ReportWriteSlowFuzzTest()
{
    int32_t adapterTypeCount = static_cast<int32_t>(AdapterType::ADAPTER_TYPE_MAX) + 1;
    AdapterType adapterType = static_cast<AdapterType>(GetData<uint8_t>() % adapterTypeCount);
    int32_t overtimeMs = GetData<int32_t>();
    AudioPerformanceMonitor::GetInstance().ReportWriteSlow(adapterType, overtimeMs);
}

void RecordTimeStampFuzzTest()
{
    int32_t adapterTypeCount = static_cast<int32_t>(AdapterType::ADAPTER_TYPE_MAX) + 1;
    AdapterType adapterType = static_cast<AdapterType>(GetData<uint8_t>() % adapterTypeCount);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, INIT_LASTWRITTEN_TIME);
    int64_t curTime = ClockTime::GetCurNano();
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, curTime);
    int64_t exeedTime = ClockTime::GetCurNano();
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, exeedTime);
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(adapterType);
}

void DeleteOvertimeMonitorFuzzTest()
{
    int32_t adapterTypeCount = static_cast<int32_t>(AdapterType::ADAPTER_TYPE_MAX) + 1;
    AdapterType adapterType = static_cast<AdapterType>(GetData<uint8_t>() % adapterTypeCount);
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(adapterType);
}

void DumpMonitorInfoFuzzTest()
{
    std::string dumpString = "abc";
    AudioPerformanceMonitor::GetInstance().DumpMonitorInfo(dumpString);
}

void JudgeNoiseFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    bool isSilence = GetData<uint32_t>() % NUM_2;
    uint32_t uid = GetData<uint32_t>();
    AudioPerformanceMonitor::GetInstance().JudgeNoise(sessionId, isSilence, uid);
}

void ReportEventFuzzTest()
{
    int32_t detectEventCount = static_cast<int32_t>(DetectEvent::SILENCE_EVENT) + 1;
    DetectEvent detectEvent = static_cast<DetectEvent>(GetData<uint8_t>() % detectEventCount);
    DetectEvent reasonCode = GetData<uint8_t>() % NUM_2 == 0 ? detectEvent : GetData<DetectEvent>();
    int32_t periodMs = GetData<int32_t>();
    int32_t audioPipeTypeCount = static_cast<int32_t>(AudioPipeType::PIPE_TYPE_DIRECT_VOIP) + 1;
    AudioPipeType pipeType = static_cast<AudioPipeType>(GetData<uint8_t>() % audioPipeTypeCount);
    int32_t adapterTypeCount = static_cast<int32_t>(AdapterType::ADAPTER_TYPE_MAX) + 1;
    AdapterType adapterType = static_cast<AdapterType>(GetData<uint8_t>() % adapterTypeCount);
    uint32_t uid = GetData<uint32_t>();
    AudioPerformanceMonitor::GetInstance().ReportEvent(reasonCode, periodMs, pipeType, adapterType, uid);
}

void RecordPaSilenceStateFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    bool isSilence = GetData<bool>();
    PA_PIPE_TYPE paPipeType = GetData<PA_PIPE_TYPE>();
    uint32_t uid = GetData<uint32_t>();
    RecordPaSilenceState(sessionId, isSilence, paPipeType, uid);
}

void StartSilenceMonitorFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t tokenId = GetData<uint32_t>();
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId, tokenId);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    RecordSilenceStateFuzzTest,
    DeleteSilenceMonitorFuzzTest,
    ReportWriteSlowFuzzTest,
    RecordTimeStampFuzzTest,
    DeleteOvertimeMonitorFuzzTest,
    DumpMonitorInfoFuzzTest,
    JudgeNoiseFuzzTest,
    ReportEventFuzzTest,
    RecordPaSilenceStateFuzzTest,
    StartSilenceMonitorFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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

    return;
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
