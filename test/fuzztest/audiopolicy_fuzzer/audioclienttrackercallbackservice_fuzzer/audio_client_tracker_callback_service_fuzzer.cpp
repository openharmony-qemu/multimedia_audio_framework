/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <securec.h>
#include "audio_info.h"
#include "audio_client_tracker_callback_service.h"
#include "audio_stream_manager.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;

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

void SetClientTrackerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    std::shared_ptr<AudioClientTracker> tracker = std::make_shared<AudioClientTracker>();
    std::weak_ptr<AudioClientTracker> weakTracker = tracker;
    service.SetClientTrackerCallback(weakTracker);
}

void UnsetClientTrackerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    service.UnsetClientTrackerCallback();
}

void MuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal event;
    event.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % 20);
    event.clientUID = GetData<int32_t>();
    event.sessionID = GetData<int32_t>();
    event.volume = GetData<float>();
    service.MuteStreamImpl(event);
}

void UnmuteStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal event;
    event.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % 20);
    event.clientUID = GetData<int32_t>();
    event.sessionID = GetData<int32_t>();
    event.volume = GetData<float>();
    service.UnmuteStreamImpl(event);
}

void PausedStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal event;
    event.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % 20);
    event.clientUID = GetData<int32_t>();
    event.sessionID = GetData<int32_t>();
    event.volume = GetData<float>();
    service.PausedStreamImpl(event);
}

void ResumeStreamImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    StreamSetStateEventInternal event;
    event.streamType = static_cast<AudioStreamType>(GetData<int32_t>() % 20);
    event.clientUID = GetData<int32_t>();
    event.sessionID = GetData<int32_t>();
    event.volume = GetData<float>();
    service.ResumeStreamImpl(event);
}

void SetLowPowerVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    float volume = GetData<float>();
    service.SetLowPowerVolumeImpl(volume);
}

void GetLowPowerVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    float volume = 0.0f;
    service.GetLowPowerVolumeImpl(volume);
}

void SetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    int32_t state = GetData<int32_t>();
    bool isAppBack = GetData<uint8_t>() % 2 == 0;
    service.SetOffloadModeImpl(state, isAppBack);
}

void UnsetOffloadModeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    service.UnsetOffloadModeImpl();
}

void GetSingleStreamVolumeImplFuzzTest(FuzzedDataProvider& fdp)
{
    AudioClientTrackerCallbackService service;
    float volume = 0.0f;
    service.GetSingleStreamVolumeImpl(volume);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        SetClientTrackerCallbackFuzzTest,
        UnsetClientTrackerCallbackFuzzTest,
        MuteStreamImplFuzzTest,
        UnmuteStreamImplFuzzTest,
        PausedStreamImplFuzzTest,
        ResumeStreamImplFuzzTest,
        SetLowPowerVolumeImplFuzzTest,
        GetLowPowerVolumeImplFuzzTest,
        SetOffloadModeImplFuzzTest,
        UnsetOffloadModeImplFuzzTest,
        GetSingleStreamVolumeImplFuzzTest,
    });
    func(fdp);
}

void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}