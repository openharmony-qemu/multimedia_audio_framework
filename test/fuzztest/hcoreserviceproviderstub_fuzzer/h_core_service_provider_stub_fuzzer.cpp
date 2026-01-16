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
#include "token_setproc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "../fuzz_utils.h"
#include "audio_volume.h"
#include "audio_info.h"
#include "i_core_service_provider.h"
#include "core_service_provider_stub.h"
#include "audio_core_service.h"
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {

void UpdateSessionOperation(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    uint32_t operation = provider.ConsumeIntegral<uint32_t>();
    uint32_t opMsg = provider.ConsumeIntegralInRange<uint32_t>(0, 1);
    coreServiceProviderWrapper.UpdateSessionOperation(sessionId, operation, opMsg);
}

void ReloadCaptureSession(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    uint32_t operation = provider.ConsumeIntegralInRange<uint32_t>(0, 3);
    coreServiceProviderWrapper.ReloadCaptureSession(sessionId, operation);
}

void GetAdapterNameBySessionId(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    std::string name = provider.ConsumeRandomLengthString();
    coreServiceProviderWrapper.GetAdapterNameBySessionId(sessionId, name);
}

void GetProcessDeviceInfoBySessionId(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    AudioDeviceDescriptor deviceInfo;
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    int32_t pin = provider.ConsumeIntegral<int32_t>();
    bool isReloadProcess = provider.ConsumeBool();
    AudioStreamInfo info;
    coreServiceProviderWrapper.GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, pin, isReloadProcess);
}

void GenerateSessionId(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    coreServiceProviderWrapper.GenerateSessionId(sessionId);
}

void SetWakeUpAudioCapturerFromAudioServer(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    AudioProcessConfig config;
    int32_t ret = provider.ConsumeIntegral<int32_t>();
    coreServiceProviderWrapper.SetWakeUpAudioCapturerFromAudioServer(config, ret);
}

void GetPaIndexByPortName(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    std::string portName = provider.ConsumeRandomLengthString();
    uint32_t ret = provider.ConsumeIntegral<uint32_t>();
    coreServiceProviderWrapper.GetPaIndexByPortName(portName, ret);
}

void A2dpOffloadGetRenderPosition(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t delayValue = provider.ConsumeIntegral<uint32_t>();
    uint64_t sendDataSize = provider.ConsumeIntegral<uint64_t>();
    uint32_t timeStamp = provider.ConsumeIntegral<uint32_t>();
    int32_t ret = provider.ConsumeIntegral<int32_t>();
    coreServiceProviderWrapper.A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp, ret);
}

void SetRendererTarget(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t target = provider.ConsumeIntegralInRange<uint32_t>(0, 1);
    uint32_t lastTarget = provider.ConsumeIntegralInRange<uint32_t>(0, 1);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    int32_t ret = provider.ConsumeIntegral<int32_t>();
    coreServiceProviderWrapper.SetRendererTarget(target, lastTarget, sessionId, ret);
}

void StartInjection(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t streamId = provider.ConsumeIntegral<uint32_t>();
    int32_t ret = provider.ConsumeIntegral<int32_t>();
    coreServiceProviderWrapper.StartInjection(streamId, ret);
}

void RemoveIdForInjector(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t streamId = provider.ConsumeIntegral<uint32_t>();
    coreServiceProviderWrapper.RemoveIdForInjector(streamId);
}

void ReleaseCaptureInjector(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    coreServiceProviderWrapper.ReleaseCaptureInjector();
}

void RebuildCaptureInjector(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t streamId = provider.ConsumeIntegral<uint32_t>();
    coreServiceProviderWrapper.RebuildCaptureInjector(streamId);
}

void OnCheckActiveMusicTime(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    std::string reason = provider.ConsumeRandomLengthString();
    coreServiceProviderWrapper.OnCheckActiveMusicTime(reason);
}

void CaptureConcurrentCheck(FuzzedDataProvider &provider)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    auto coreServiceWorker = std::make_unique<AudioCoreService::EventEntry>(audioCoreService);
    CoreServiceProviderWrapper coreServiceProviderWrapper(static_cast<ICoreServiceProvider*>(coreServiceWorker.get()));
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    coreServiceProviderWrapper.CaptureConcurrentCheck(sessionId);
}

void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray({
        UpdateSessionOperation,
        ReloadCaptureSession,
        GetAdapterNameBySessionId,
        GetProcessDeviceInfoBySessionId,
        GenerateSessionId,
        SetWakeUpAudioCapturerFromAudioServer,
        GetPaIndexByPortName,
        A2dpOffloadGetRenderPosition,
        SetRendererTarget,
        StartInjection,
        RemoveIdForInjector,
        ReleaseCaptureInjector,
        RebuildCaptureInjector,
        OnCheckActiveMusicTime,
        CaptureConcurrentCheck,
    });
    func(provider);
}
} // namespace AudioStandard
} // namesapce OHOS
extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (SetSelfTokenID(718336240uLL | (1uLL << NUM_32)) < 0) {
        return -1;
    }
    return 0;
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
