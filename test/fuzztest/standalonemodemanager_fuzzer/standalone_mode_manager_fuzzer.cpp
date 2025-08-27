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

#include "standalone_mode_manager.h"
#include "audio_interrupt_service.h"
#include "audio_session_info.h"
#include "audio_bundle_manager.h"
#include "audio_volume.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

void StandaloneModeManagerInItFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
}

void StandaloneModeManagerCheckAndRecordStandaloneAppFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    int32_t appUid = GetData<int32_t>();
    int32_t sessionId = GetData<int32_t>();
    bool isOnlyRecordUid = GetData<bool>();
    StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(appUid,
        isOnlyRecordUid, sessionId);
}

void StandaloneModeManagerSetAppSilentOnDisplayFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    int32_t ownerPid = GetData<int32_t>();
    int32_t displayId = GetData<int32_t>();
    StandaloneModeManager::GetInstance().SetAppSilentOnDisplay(ownerPid, displayId);
}

void StandaloneModeManagerSetAppConcurrencyModeFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    int32_t ownerPid = GetData<int32_t>();
    int32_t appUid = GetData<int32_t>();
    int32_t mode = GetData<int32_t>();
    StandaloneModeManager::GetInstance().SetAppConcurrencyMode(ownerPid, appUid, mode);
}

void StandaloneModeManagerEraseDeactivateAudioStreamFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    int32_t appUid = GetData<int32_t>();
    int32_t sessionId = GetData<int32_t>();
    StandaloneModeManager::GetInstance().EraseDeactivateAudioStream(appUid, sessionId);
}

void StandaloneModeManagerResumeAllStandaloneAppFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService =
        std::make_shared<AudioInterruptService>();
    StandaloneModeManager::GetInstance().Init(interruptService);
    int32_t appUid = GetData<int32_t>();
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(appUid);
}

TestPtr g_testPtrs[] = {
    StandaloneModeManagerInItFuzzTest,
    StandaloneModeManagerCheckAndRecordStandaloneAppFuzzTest,
    StandaloneModeManagerSetAppSilentOnDisplayFuzzTest,
    StandaloneModeManagerSetAppConcurrencyModeFuzzTest,
    StandaloneModeManagerEraseDeactivateAudioStreamFuzzTest,
    StandaloneModeManagerResumeAllStandaloneAppFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
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