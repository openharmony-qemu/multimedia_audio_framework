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

#include "audio_log.h"
#include "audio_session.h"
#include "audio_session_service.h"
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;
namespace OHOS {
namespace AudioStandard {

const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

std::shared_ptr<AudioSession> CreateAudioSession()
{
    AudioSessionStrategy strategy;
    int32_t callerPid = 1;
    auto &audioSessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    return std::make_shared<AudioSession>(callerPid, strategy, audioSessionService);
}

void SetAudioSessionSceneFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    int32_t sceneValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->SetAudioSessionScene(static_cast<AudioSessionScene>(sceneValue));
}

void GetStreamsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetStreams();
}

void GetFakeStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    int32_t sceneValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->SetAudioSessionScene(static_cast<AudioSessionScene>(sceneValue));
    audioSession->GetFakeStreamType();
}

void AddStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.isAudioSessionInterrupt = fdp.ConsumeBool();
    audioSession->IsActivated();
    audioSession->IsSceneParameterSet();
    int32_t streamTypeValue = fdp.ConsumeIntegral<int32_t>();
    incomingInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(streamTypeValue);
    int32_t sourceTypeValue = fdp.ConsumeIntegral<int32_t>();
    incomingInterrupt.audioFocusType.sourceType = static_cast<SourceType>(sourceTypeValue);
    audioSession->AddStreamInfo({incomingInterrupt, ACTIVE});
}

void RemoveStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    uint32_t streamId = fdp.ConsumeIntegral<int32_t>();
    audioSession->RemoveStreamInfo(streamId);
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->RemoveStreamInfo(streamId);
    audioSession->ClearStreamInfo();
}

void ClearStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->ClearStreamInfo();
}

void GetFakeStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetFakeStreamId();
}

void SaveFakeStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->SaveFakeStreamId(fdp.ConsumeIntegral<uint32_t>());
}

void DumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    std::string dumpString = "dumpString";
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = fdp.ConsumeIntegral<uint32_t>();
    int32_t streamTypeValue = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(streamTypeValue);
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->Dump(dumpString);
    audioSession->ClearStreamInfo();
}

void UpdateSingleVoipStreamDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    interrupt.streamId = fdp.ConsumeIntegral<uint32_t>();
    audioSession->UpdateSingleVoipStreamDefaultOutputDevice(interrupt);
}

void UpdateVoipStreamsDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt audioInterrupt;
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->UpdateVoipStreamsDefaultOutputDevice();
    audioSession->ClearStreamInfo();
}

void DeactivateFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsSessionDefaultDeviceEnabled();
    audioSession->Deactivate();
}

void IsOutputDeviceConfigurableByStreamUsageFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    int32_t streamUsageValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->IsOutputDeviceConfigurableByStreamUsage(static_cast<StreamUsage>(streamUsageValue));
}

void CanCurrentStreamSetDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    int32_t streamUsageValue = fdp.ConsumeIntegral<int32_t>();
    interrupt.streamUsage = static_cast<StreamUsage>(streamUsageValue);
    audioSession->CanCurrentStreamSetDefaultOutputDevice(interrupt);
}

void EnableSingleVoipStreamDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    int32_t streamUsageValue = fdp.ConsumeIntegral<int32_t>();
    interrupt.streamUsage = static_cast<StreamUsage>(streamUsageValue);
    interrupt.streamId = fdp.ConsumeIntegral<int32_t>();
    audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
}

void EnableVoipStreamsDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    int32_t streamUsageValue = fdp.ConsumeIntegral<int32_t>();
    interrupt.streamUsage = static_cast<StreamUsage>(streamUsageValue);
    interrupt.streamId = fdp.ConsumeIntegral<int32_t>();
    audioSession->AddStreamInfo({interrupt, ACTIVE});
    audioSession->EnableVoipStreamsDefaultOutputDevice();
    audioSession->ClearStreamInfo();
}

void EnableDefaultDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsActivated();
    int32_t deviceTypeValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->SetSessionDefaultOutputDevice(static_cast<DeviceType>(deviceTypeValue));
    int32_t sceneValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->SetAudioSessionScene(static_cast<AudioSessionScene>(sceneValue));
    audioSession->EnableDefaultDevice();
}

void GetStreamUsageInnerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    int32_t sceneValue = fdp.ConsumeIntegral<int32_t>();
    audioSession->SetAudioSessionScene(static_cast<AudioSessionScene>(sceneValue));
    audioSession->GetStreamUsageInner();
}

void GetSessionStrategyFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetSessionStrategy();
}

void IsAudioRendererEmptyFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt audioInterrupt;
    int32_t streamTypeValue = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(streamTypeValue);
    audioSession->AddStreamInfo({audioInterrupt, ACTIVE});
    audioSession->IsAudioRendererEmpty();
    audioSession->ClearStreamInfo();
}

void GetSessionDefaultOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    int32_t deviceTypeValue = fdp.ConsumeIntegral<int32_t>();
    DeviceType deviceType = static_cast<DeviceType>(deviceTypeValue);
    audioSession->GetSessionDefaultOutputDevice(deviceType);
}

void IsRecommendToStopAudioFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsRecommendToStopAudio(
        AudioStreamDeviceChangeReason::OVERRODE, std::make_shared<AudioDeviceDescriptor>());
}

void IsSessionOutputDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDeviceDescriptors;
    audioSession->IsSessionOutputDeviceChanged(
        std::make_shared<AudioDeviceDescriptor>(), oldDeviceDescriptors);
}

void GetSessionStreamUsageFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetSessionStreamUsage();
}

void IsBackGroundAppFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsBackGroundApp();
}

void GetAudioSessionStreamUsageForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetAudioSessionStreamUsageForDevice();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    SetAudioSessionSceneFuzzTest,
    GetStreamsFuzzTest,
    GetFakeStreamTypeFuzzTest,
    AddStreamInfoFuzzTest,
    RemoveStreamInfoFuzzTest,
    ClearStreamInfoFuzzTest,
    GetFakeStreamIdFuzzTest,
    SaveFakeStreamIdFuzzTest,
    DumpFuzzTest,
    UpdateSingleVoipStreamDefaultOutputDeviceFuzzTest,
    UpdateVoipStreamsDefaultOutputDeviceFuzzTest,
    DeactivateFuzzTest,
    IsOutputDeviceConfigurableByStreamUsageFuzzTest,
    CanCurrentStreamSetDefaultOutputDeviceFuzzTest,
    EnableSingleVoipStreamDefaultOutputDeviceFuzzTest,
    EnableVoipStreamsDefaultOutputDeviceFuzzTest,
    EnableDefaultDeviceFuzzTest,
    GetStreamUsageInnerFuzzTest,
    GetSessionStrategyFuzzTest,
    IsAudioRendererEmptyFuzzTest,
    GetSessionDefaultOutputDeviceFuzzTest,
    IsRecommendToStopAudioFuzzTest,
    IsSessionOutputDeviceChangedFuzzTest,
    GetSessionStreamUsageFuzzTest,
    IsBackGroundAppFuzzTest,
    GetAudioSessionStreamUsageForDeviceFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}