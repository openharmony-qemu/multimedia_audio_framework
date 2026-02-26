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
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_stream_descriptor.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"
#include "audio_injector_policy.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static size_t g_dataSize = 0;
static size_t g_pos;

typedef void (*TestFuncs)();

vector<VoipType> VoipTypeVec = {
    NO_VOIP,
    NORMAL_VOIP,
    FAST_VOIP,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
;
    }
    if (memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize) != EOK) {
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

void InitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
}

void DeInitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().DeInit();
}

void UpdateAudioInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioModuleInfo info;
    info.name = "test_module";
    info.lib = "libtest.so";
    info.format = "s16le";
    info.channels = "2";
    info.rate = "48000";
    AudioInjectorPolicy::GetInstance().UpdateAudioInfo(info);
}

void AddStreamDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
}

void RemoveStreamDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
    AudioInjectorPolicy::GetInstance().RemoveStreamDescriptor(renderId);
}

void IsContainStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
    bool result = AudioInjectorPolicy::GetInstance().IsContainStream(renderId);
}

void GetAdapterNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::string name = AudioInjectorPolicy::GetInstance().GetAdapterName();
}

void GetRendererStreamCountFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    int32_t count = AudioInjectorPolicy::GetInstance().GetRendererStreamCount();
}

void SetCapturePortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(idx);
}

void GetCapturePortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(idx);
    uint32_t result = AudioInjectorPolicy::GetInstance().GetCapturePortIdx();
}

void SetRendererPortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetRendererPortIdx(idx);
}

void GetRendererPortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetRendererPortIdx(idx);
    uint32_t result = AudioInjectorPolicy::GetInstance().GetRendererPortIdx();
}

void GetAudioModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioModuleInfo& info = AudioInjectorPolicy::GetInstance().GetAudioModuleInfo();
}

void GetIsConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    bool result = AudioInjectorPolicy::GetInstance().GetIsConnected();
}

void SetVoipTypeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t voipTypeCount = GetData<uint32_t>() % VoipTypeVec.size();
    VoipType type = VoipTypeVec[voipTypeCount];
    AudioInjectorPolicy::GetInstance().SetVoipType(type);
}

void AddCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().AddCaptureInjector();
}

void AddCaptureInjectorInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().AddCaptureInjectorInner();
}

void RemoveCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    bool noCapturer = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().RemoveCaptureInjector(noCapturer);
}

void RemoveCaptureInjectorInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    bool noCapturer = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().RemoveCaptureInjectorInner(noCapturer);
}

void ReleaseCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().ReleaseCaptureInjector();
}

void RebuildCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().RebuildCaptureInjector(streamId);
}

void FindCaptureVoipPipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    uint32_t streamId = GetData<uint32_t>();
    std::shared_ptr<AudioPipeInfo> result = AudioInjectorPolicy::GetInstance().FindCaptureVoipPipe(pipeInfos, streamId);
}

void FindPipeByStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    VoipType type = NO_VOIP;
    uint32_t streamId = GetData<uint32_t>();
    std::shared_ptr<AudioPipeInfo> result =
        AudioInjectorPolicy::GetInstance().FindPipeByStreamId(pipeInfos, type, streamId);
}

void FetchCapDeviceInjectPreProcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    bool removeFlag = GetData<uint32_t>() % NUM_2;
;
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().FetchCapDeviceInjectPreProc(pipeInfos, removeFlag, streamId);
}

void FetchCapDeviceInjectPostProcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    bool removeFlag = GetData<uint32_t>() % NUM_2;
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().FetchCapDeviceInjectPostProc(pipeInfos, removeFlag, streamId);
}

void HasRunningVoipStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec;
    bool result = AudioInjectorPolicy::GetInstance().HasRunningVoipStream(streamVec);
}

void AddInjectorStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
}

void DeleteInjectorStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
    AudioInjectorPolicy::GetInstance().DeleteInjectorStreamId(streamId);
}

void IsActivateInterruptStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
    bool result = AudioInjectorPolicy::GetInstance().IsActivateInterruptStreamId(streamId);
}

void SendInterruptEventToInjectorStreamsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::shared_ptr<AudioPolicyServerHandler> handler = nullptr;
    AudioInjectorPolicy::GetInstance().SendInterruptEventToInjectorStreams(handler);
}

void SetInjectStreamsMuteForInjectionFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetInjectStreamsMuteForInjection(streamId);
}

void SetInjectStreamsMuteForPlaybackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetInjectStreamsMuteForPlayback(streamId);
}

void SetInjectorStreamsMuteFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    bool newMicrophoneMute = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().SetInjectorStreamsMute(newMicrophoneMute);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    InitFuzzTest,
    DeInitFuzzTest,
    UpdateAudioInfoFuzzTest,
    AddStreamDescriptorFuzzTest,
    RemoveStreamDescriptorFuzzTest,
    IsContainStreamFuzzTest,
    GetAdapterNameFuzzTest,
    GetRendererStreamCountFuzzTest,
    SetCapturePortIdxFuzzTest,
    GetCapturePortIdxFuzzTest,
    SetRendererPortIdxFuzzTest,
    GetRendererPortIdxFuzzTest,
    GetAudioModuleInfoFuzzTest,
    GetIsConnectedFuzzTest,
    SetVoipTypeFuzzTest,
    AddCaptureInjectorFuzzTest,
    AddCaptureInjectorInnerFuzzTest,
    RemoveCaptureInjectorFuzzTest,
    RemoveCaptureInjectorInnerFuzzTest,
    ReleaseCaptureInjectorFuzzTest,
    RebuildCaptureInjectorFuzzTest,
    FindCaptureVoipPipeFuzzTest,
    FindPipeByStreamIdFuzzTest,
    FetchCapDeviceInjectPreProcFuzzTest,
    FetchCapDeviceInjectPostProcFuzzTest,
    HasRunningVoipStreamFuzzTest,
    AddInjectorStreamIdFuzzTest,
    DeleteInjectorStreamIdFuzzTest,
    IsActivateInterruptStreamIdFuzzTest,
    SendInterruptEventToInjectorStreamsFuzzTest,
    SetInjectStreamsMuteForInjectionFuzzTest,
    SetInjectStreamsMuteForPlaybackFuzzTest,
    SetInjectorStreamsMuteFuzzTest,
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

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
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

extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}