/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_bluetooth_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const char *SINK_ADAPTER_NAME = "primary";
const uint64_t COMMON_UINT64_NUM = 2;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static uint32_t g_renderId = HDI_INVALID_ID;
const size_t THRESHOLD = 10;

void GetRenderId()
{
    g_renderId = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP,
        true);
}

void ReleaseRenderId()
{
    HdiAdapterManager::GetInstance().ReleaseId(g_renderId);
}

std::shared_ptr<IAudioRenderSink> GetAdaptorBlueToothSink()
{
    return HdiAdapterManager::GetInstance().GetRenderSink(g_renderId, true);
}

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
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

void IsInitedFuzzTest()
{
    GetAdaptorBlueToothSink()->IsInited();
}

void SetAudioSceneFuzzTest()
{
    AudioScene audioScene = GetData<AudioScene>();
    DeviceType deviceType = GetData<DeviceType>();
    std::vector<DeviceType> activeDevices = {deviceType};
    GetAdaptorBlueToothSink()->SetAudioScene(audioScene, activeDevices);
}

void SetOutputRoutesFuzzTest()
{
    DeviceType deviceType = GetData<DeviceType>();
    std::vector<DeviceType> outputDevices = {deviceType};
    GetAdaptorBlueToothSink()->UpdateActiveDevice(outputDevices);
}

void SetAudioParameterFuzzTest()
{
    AudioParamKey key = GetData<AudioParamKey>();
    std::string condition = "123456";
    std::string value = "123456";
    GetAdaptorBlueToothSink()->SetAudioParameter(key, condition, value);
}

void GetAudioParameterFuzzTest()
{
    AudioParamKey key = GetData<AudioParamKey>();
    std::string condition = "123456";
    GetAdaptorBlueToothSink()->GetAudioParameter(key, condition);
}

void RegisterParameterCallbackFuzzTest()
{
    IAudioSinkCallback *callback_ = nullptr;
    GetAdaptorBlueToothSink()->RegistCallback(HDI_CB_RENDER_STATE, callback_);
}

void InitFuzzTest()
{
    IAudioSinkAttr attr = {};
    attr.adapterName = SINK_ADAPTER_NAME;
    attr.sampleRate = GetData<uint32_t>();
    attr.channel = GetData<uint32_t>();
    attr.format = GetData<AudioSampleFormat>();
    attr.channelLayout = COMMON_UINT64_NUM;
    attr.deviceType = GetData<DeviceType>();
    attr.volume = GetData<float>();
    attr.openMicSpeaker = GetData<uint32_t>();

    GetAdaptorBlueToothSink()->Init(attr);
}

void RenderFrameFuzzTest()
{
    char data = GetData<char>();
    uint64_t len = GetData<uint64_t>();
    uint64_t writeLen = GetData<uint64_t>();
    GetAdaptorBlueToothSink()->RenderFrame(data, len, writeLen);
}

void StartFuzzTest()
{
    GetAdaptorBlueToothSink()->Start();
}

void SetVolumeFuzzTest()
{
    float left = GetData<float>();
    float right = GetData<float>();

    GetAdaptorBlueToothSink()->SetVolume(left, right);
}

void GetVolumeFuzzTest()
{
    float left, right;
    if (g_dataSize >= sizeof(left)) {
        left = GetData<float>();
    } else {
        return;
    }
    if (g_dataSize >= sizeof(right)) {
        right = GetData<float>();
    } else {
        return;
    }
    GetAdaptorBlueToothSink()->GetVolume(left, right);
}

void GetTransactionIdFuzzTest()
{
    uint64_t transactionId = GetData<uint64_t>();
    GetAdaptorBlueToothSink()->GetTransactionId(transactionId);
}

void StopFuzzTest()
{
    GetAdaptorBlueToothSink()->Stop();
}

void PauseFuzzTest()
{
    GetAdaptorBlueToothSink()->Pause();
}

void ResumeFuzzTest()
{
    GetAdaptorBlueToothSink()->Resume();
}

void ResetFuzzTest()
{
    GetAdaptorBlueToothSink()->Reset();
}

void FlushFuzzTest()
{
    GetAdaptorBlueToothSink()->Flush();
}

void SuspendRenderSinkFuzzTest()
{
    GetAdaptorBlueToothSink()->SuspendRenderSink();
}

void RestoreRenderSinkFuzzTest()
{
    GetAdaptorBlueToothSink()->RestoreRenderSink();
}

void GetPresentationPositionFuzzTest()
{
    uint64_t frames = GetData<uint64_t>();
    int64_t timeSec = GetData<int64_t>();
    int64_t timeNanoSec = GetData<int64_t>();
    GetAdaptorBlueToothSink()->GetPresentationPosition(frames, timeSec, timeNanoSec);
}

void ResetOutputRouteForDisconnectFuzzTest()
{
    DeviceType deviceType = GetData<DeviceType>();
    GetAdaptorBlueToothSink()->ResetActiveDeviceForDisconnect(deviceType);
}

void SetPaPowerFuzzTest()
{
    int32_t flag = GetData<int32_t>();
    GetAdaptorBlueToothSink()->SetPaPower(flag);
}

void ReleaseRenderIdFuzzTest()
{
    g_renderId = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP,
        true);
}

typedef void (*TestFuncs[22])();

TestFuncs g_testFuncs = {
    InitFuzzTest,
    RenderFrameFuzzTest,
    StartFuzzTest,
    IsInitedFuzzTest,
    SetAudioSceneFuzzTest,
    SetOutputRoutesFuzzTest,
    SetAudioParameterFuzzTest,
    GetAudioParameterFuzzTest,
    RegisterParameterCallbackFuzzTest,
    SetVolumeFuzzTest,
    GetVolumeFuzzTest,
    GetTransactionIdFuzzTest,
    SuspendRenderSinkFuzzTest,
    RestoreRenderSinkFuzzTest,
    GetPresentationPositionFuzzTest,
    ResetOutputRouteForDisconnectFuzzTest,
    SetPaPowerFuzzTest,
    PauseFuzzTest,
    ResumeFuzzTest,
    ResetFuzzTest,
    FlushFuzzTest,
    StopFuzzTest,
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
    GetRenderId();

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    // release data
    ReleaseRenderId();
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
