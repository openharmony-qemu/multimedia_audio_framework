/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "audio_capturer_session.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

typedef void (*TestFuncs)();

void LoadInnerCapturerSinkFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::string moduleName = "moduleName";
    AudioStreamInfo streamInfo;
    session.LoadInnerCapturerSink(moduleName, streamInfo);
}

void UnloadInnerCapturerSinkFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::string moduleName = "moduleName";
    session.UnloadInnerCapturerSink(moduleName);
}

void HandleRemoteCastDeviceFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    bool isConnected = g_fuzzUtils.GetData<bool>();
    session.HandleRemoteCastDevice(isConnected, streamInfo);
}

void FindRunningNormalSessionFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamDescriptor runningSessionInfo;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    session.FindRunningNormalSession(sessionId, runningSessionInfo);
}

void ConstructWakeupAudioModuleInfoFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    session.ConstructWakeupAudioModuleInfo(streamInfo, audioModuleInfo);
}

void SetWakeUpAudioCapturerFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    InternalAudioCapturerOptions options;
    session.SetWakeUpAudioCapturer(options);
}

void SetWakeUpAudioCapturerFromAudioServerFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioProcessConfig config;
    session.SetWakeUpAudioCapturerFromAudioServer(config);
}

void CloseWakeUpAudioCapturerFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    session.CloseWakeUpAudioCapturer();
}

void FillWakeupStreamPropInfoFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    session.FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
}

void IsVoipDeviceChangedFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    session.IsVoipDeviceChanged(inputDevice, outputDevice);
}

void SetInputDeviceTypeForReloadFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioDeviceDescriptor inputDevice;
    session.SetInputDeviceTypeForReload(inputDevice);
}

void GetInputDeviceTypeForReloadFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    session.GetInputDeviceTypeForReload();
}

void GetEnhancePropByNameV3FuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEffectPropertyArrayV3 propertyArray;
    std::string propName = "propName";
    session.GetEnhancePropByNameV3(propertyArray, propName);
}

void ReloadSourceForEffectFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyArrayV3 newPropertyArray;
    session.ReloadSourceForEffect(propertyArray, newPropertyArray);
}

void GetEnhancePropByNameFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEnhancePropertyArray propertyArray;
    std::string propName = "propName";
    session.GetEnhancePropByName(propertyArray, propName);
}

void ReloadSourceForEffectDifferentArgsFuzzTest()
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    session.ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

vector<TestFuncs> g_testFuncs = {
    LoadInnerCapturerSinkFuzzTest,
    UnloadInnerCapturerSinkFuzzTest,
    HandleRemoteCastDeviceFuzzTest,
    FindRunningNormalSessionFuzzTest,
    ConstructWakeupAudioModuleInfoFuzzTest,
    SetWakeUpAudioCapturerFuzzTest,
    SetWakeUpAudioCapturerFromAudioServerFuzzTest,
    CloseWakeUpAudioCapturerFuzzTest,
    FillWakeupStreamPropInfoFuzzTest,
    IsVoipDeviceChangedFuzzTest,
    SetInputDeviceTypeForReloadFuzzTest,
    GetInputDeviceTypeForReloadFuzzTest,
    GetEnhancePropByNameV3FuzzTest,
    ReloadSourceForEffectFuzzTest,
    GetEnhancePropByNameFuzzTest,
    ReloadSourceForEffectDifferentArgsFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}