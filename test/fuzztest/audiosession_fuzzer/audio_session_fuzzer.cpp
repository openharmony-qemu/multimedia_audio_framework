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
#include "../fuzz_utils.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

std::shared_ptr<AudioSession> CreateAudioSession()
{
    AudioSessionStrategy strategy;
    auto &audioSessionService = OHOS::Singleton<AudioSessionService>::GetInstance();
    return std::make_shared<AudioSession>(g_fuzzUtils.GetData<int32_t>(), strategy, audioSessionService);
}

void SetAudioSessionSceneFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->SetAudioSessionScene(g_fuzzUtils.GetData<AudioSessionScene>());
}

void GetStreamsFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetStreams();
}

void GetFakeStreamTypeFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->SetAudioSessionScene(g_fuzzUtils.GetData<AudioSessionScene>());
    audioSession->GetFakeStreamType();
}

void AddStreamInfoFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.isAudioSessionInterrupt = g_fuzzUtils.GetData<bool>();
    audioSession->IsActivated();
    audioSession->IsSceneParameterSet();
    incomingInterrupt.audioFocusType.streamType = g_fuzzUtils.GetData<AudioStreamType>();
    incomingInterrupt.audioFocusType.sourceType = g_fuzzUtils.GetData<SourceType>();
    audioSession->AddStreamInfo(incomingInterrupt);
}

void RemoveStreamInfoFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    uint32_t streamId = g_fuzzUtils.GetData<uint32_t>();
    audioSession->RemoveStreamInfo(streamId);
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->RemoveStreamInfo(streamId);
    audioSession->ClearStreamInfo();
}

void ClearStreamInfoFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->ClearStreamInfo();
}

void GetFakeStreamIdFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetFakeStreamId();
}

void SaveFakeStreamIdFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->SaveFakeStreamId(g_fuzzUtils.GetData<uint32_t>());
}

void DumpFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    std::string dumpString = "dumpString";
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = g_fuzzUtils.GetData<uint32_t>();
    audioInterrupt.audioFocusType.streamType = g_fuzzUtils.GetData<AudioStreamType>();
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->Dump(dumpString);
    audioSession->ClearStreamInfo();
}

void UpdateSingleVoipStreamDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    interrupt.streamId = g_fuzzUtils.GetData<uint32_t>();
    audioSession->UpdateSingleVoipStreamDefaultOutputDevice(interrupt);
}

void UpdateVoipStreamsDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt audioInterrupt;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->UpdateVoipStreamsDefaultOutputDevice();
    audioSession->ClearStreamInfo();
}

void DeactivateFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsSessionDefaultDeviceEnabled();
    audioSession->Deactivate();
}

void IsOutputDeviceConfigurableByStreamUsageFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsOutputDeviceConfigurableByStreamUsage(g_fuzzUtils.GetData<StreamUsage>());
}

void CanCurrentStreamSetDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    interrupt.streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    audioSession->CanCurrentStreamSetDefaultOutputDevice(interrupt);
}

void EnableSingleVoipStreamDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    interrupt.streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    interrupt.streamId = g_fuzzUtils.GetData<uint32_t>();
    audioSession->EnableSingleVoipStreamDefaultOutputDevice(interrupt);
}

void EnableVoipStreamsDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt interrupt;
    interrupt.streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    interrupt.streamId = g_fuzzUtils.GetData<uint32_t>();
    audioSession->AddStreamInfo(interrupt);
    audioSession->EnableVoipStreamsDefaultOutputDevice();
    audioSession->ClearStreamInfo();
}

void EnableDefaultDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsActivated();
    audioSession->SetSessionDefaultOutputDevice(g_fuzzUtils.GetData<DeviceType>());
    audioSession->SetAudioSessionScene(g_fuzzUtils.GetData<AudioSessionScene>());
    audioSession->EnableDefaultDevice();
}

void GetStreamUsageInnerFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->SetAudioSessionScene(g_fuzzUtils.GetData<AudioSessionScene>());
    audioSession->GetStreamUsageInner();
}

void GetSessionStrategyFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetSessionStrategy();
}

void IsAudioRendererEmptyFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.streamType = g_fuzzUtils.GetData<AudioStreamType>();
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->IsAudioRendererEmpty();
    audioSession->ClearStreamInfo();
}

void GetSessionDefaultOutputDeviceFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    audioSession->GetSessionDefaultOutputDevice(deviceType);
}

void IsRecommendToStopAudioFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsRecommendToStopAudio(
        AudioStreamDeviceChangeReason::OVERRODE, std::make_shared<AudioDeviceDescriptor>());
}

void IsSessionOutputDeviceChangedFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsSessionOutputDeviceChanged(std::make_shared<AudioDeviceDescriptor>());
}

void GetSessionStreamUsageFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->GetSessionStreamUsage();
}

void IsBackGroundAppFuzzTest()
{
    auto audioSession = CreateAudioSession();
    CHECK_AND_RETURN(audioSession != nullptr);
    audioSession->IsBackGroundApp();
}

vector<TestFuncs> g_testFuncs = {
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
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}