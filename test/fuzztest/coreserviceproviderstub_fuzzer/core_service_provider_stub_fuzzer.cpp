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
#include "audio_log.h"
#include "audio_info.h"
#include "audio_volume.h"
#include "i_core_service_provider.h"
#include "core_service_provider_stub.h"
#include "audio_core_service.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

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

void CoreServiceProviderWrapperFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
}

void UpdateSessionOperationFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t operation = GetData<uint32_t>();
    uint32_t opMsg = GetData<uint32_t>();
    coreServiceProviderWrapper.UpdateSessionOperation(sessionId, operation, opMsg);
}

void ReloadCaptureSessionFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t operation = GetData<uint32_t>();
    coreServiceProviderWrapper.ReloadCaptureSession(sessionId, operation);
}

void SetDefaultOutputDeviceFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    int32_t defaultOutputDevice = GetData<int32_t>();
    uint32_t sessionID = GetData<uint32_t>();
    int32_t streamUsage = GetData<int32_t>();
    bool isRunning = GetData<bool>();
    coreServiceProviderWrapper.SetDefaultOutputDevice(defaultOutputDevice, sessionID, streamUsage, isRunning);
}

void GetAdapterNameBySessionIdFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    uint32_t sessionID = GetData<uint32_t>();
    std::string name = "abc";
    coreServiceProviderWrapper.GetAdapterNameBySessionId(sessionID, name);
}

void GetProcessDeviceInfoBySessionIdFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    uint32_t sessionId = GetData<uint32_t>();
    AudioDeviceDescriptor deviceInfo;
    bool reload = GetData<bool>();
    AudioStreamInfo info;
    coreServiceProviderWrapper.GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, reload);
}

void GenerateSessionIdFuzzTest()
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = new AudioCoreService::EventEntry(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker));
    uint32_t sessionId = GetData<uint32_t>();
    coreServiceProviderWrapper.GenerateSessionId(sessionId);
}

TestFuncs g_testFuncs[] = {
    CoreServiceProviderWrapperFuzzTest,
    UpdateSessionOperationFuzzTest,
    ReloadCaptureSessionFuzzTest,
    SetDefaultOutputDeviceFuzzTest,
    GetAdapterNameBySessionIdFuzzTest,
    GetProcessDeviceInfoBySessionIdFuzzTest,
    GenerateSessionIdFuzzTest,
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
