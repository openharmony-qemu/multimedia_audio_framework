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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
static int32_t NUM_5 = 5;

typedef void (*TestFuncs)();

void LoadInnerCapturerSinkFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::string moduleName = "moduleName";
    AudioStreamInfo streamInfo;
    session.LoadInnerCapturerSink(moduleName, streamInfo);
}

void UnloadInnerCapturerSinkFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::string moduleName = "moduleName";
    session.UnloadInnerCapturerSink(moduleName);
}

void HandleRemoteCastDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    bool isConnected = g_fuzzUtils.GetData<bool>();
    session.HandleRemoteCastDevice(isConnected, streamInfo);
}

void FindRunningNormalSessionFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamDescriptor runningSessionInfo;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    session.FindRunningNormalSession(sessionId, runningSessionInfo);
}

void ConstructWakeupAudioModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    session.ConstructWakeupAudioModuleInfo(streamInfo, audioModuleInfo);
}

void SetWakeUpAudioCapturerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    InternalAudioCapturerOptions options;
    session.SetWakeUpAudioCapturer(options);
}

void SetWakeUpAudioCapturerFromAudioServerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioProcessConfig config;
    session.SetWakeUpAudioCapturerFromAudioServer(config);
}

void CloseWakeUpAudioCapturerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    session.CloseWakeUpAudioCapturer();
}

void FillWakeupStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    session.FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
}

void IsVoipDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    session.IsVoipDeviceChanged(inputDevice, outputDevice);
}

void SetInputDeviceTypeForReloadFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioDeviceDescriptor inputDevice;
    session.SetInputDeviceTypeForReload(inputDevice);
}

void GetInputDeviceTypeForReloadFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    session.GetInputDeviceTypeForReload();
}

void GetEnhancePropByNameV3FuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEffectPropertyArrayV3 propertyArray;
    std::string propName = "propName";
    session.GetEnhancePropByNameV3(propertyArray, propName);
}

void ReloadSourceForEffectFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyArrayV3 newPropertyArray;
    session.ReloadSourceForEffect(propertyArray, newPropertyArray);
}

void GetEnhancePropByNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEnhancePropertyArray propertyArray;
    std::string propName = "propName";
    session.GetEnhancePropByName(propertyArray, propName);
}

void ReloadSourceForEffectDifferentArgsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    session.ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

void GetTargetSessionForEcFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipe = std::make_shared<AudioPipeInfo>();
    if (pipe == nullptr) {
        return;
    }
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    session.IsInvalidPipeRole(pipe);
    session.IsIndependentPipe(pipe);
    session.GetTargetSessionForEc();
}

void HandleIndependentInputpipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::shared_ptr<AudioPipeInfo> pipe = std::make_shared<AudioPipeInfo>();
    if (pipe == nullptr) {
        return;
    }
    pipe->pipeRole_ = g_fuzzUtils.GetData<AudioPipeRole>();
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = {pipe};
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = g_fuzzUtils.GetData<bool>();
    session.HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    session.HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
}

void IsStreamValidFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    session.IsStreamValid(stream);
}

void FindRemainingNormalSessionFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    bool findRunningSessionRet = g_fuzzUtils.GetData<bool>();
    uint32_t runningSessionId = g_fuzzUtils.GetData<uint32_t>();
    uint32_t targetSessionId = g_fuzzUtils.GetData<uint32_t>();
    session.FindRemainingNormalSession(sessionId, findRunningSessionRet, runningSessionId, targetSessionId);
}

void SetHearingAidReloadFlagFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    bool hearingAidReloadFlag = g_fuzzUtils.GetData<bool>();
    session.SetHearingAidReloadFlag(hearingAidReloadFlag);
}

void ReloadCaptureSoftLinkFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    AudioModuleInfo moduleInfo;
    session.ReloadCaptureSoftLink(pipeInfo, moduleInfo);
    session.ReloadCaptureSessionSoftLink();
}

void ReloadCapturerSessionForInputPipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    SessionOperation operation = g_fuzzUtils.GetData<SessionOperation>();
    session.ReloadCapturerSessionForInputPipe(sessionId, operation);
}

void GetTargetSessionIdForInputPipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.sourceType = g_fuzzUtils.GetData<SourceType>();
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(stream);

    uint32_t originSessionId = g_fuzzUtils.GetData<uint32_t>();
    uint32_t targetSessionId = g_fuzzUtils.GetData<uint32_t>();
    SessionOperation operation = g_fuzzUtils.GetData<SessionOperation>();
    session.GetTargetSessionIdForInputPipe(pipeInfo, originSessionId, targetSessionId, operation);
}

void GetMaxPriorityForInputPipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    stream->streamStatus_ = g_fuzzUtils.GetData<AudioStreamStatus>();
    stream->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    pipeInfo->streamDescriptors_.push_back(stream);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    AudioStreamDescriptor maxRunningDesc;
    AudioStreamDescriptor maxRemainingDesc;

    AudioSourceStrategyType strategyType;
    strategyType.audioFlag = stream->audioFlag_;
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->insert({stream->capturerInfo_.sourceType, strategyType});
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    session.GetMaxPriorityForInputPipe(pipeInfo, sessionId, maxRunningDesc, maxRemainingDesc);
}

void IsVirtualAudioRecognitionSessionFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    SessionInfo sessionInfo;
    session.sessionWithNormalSourceType_.insert({sessionId, sessionInfo});
    session.IsVirtualAudioRecognitionSession(sessionId);
}

void IsPipeInSourceStrategyMapFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint64_t sessionId = g_fuzzUtils.GetData<uint64_t>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = sessionId;
    pipeInfo->streamDescriptors_.push_back(stream);
    session.IsPipeInSourceStrategyMap(pipeInfo, sessionId);
}

void IsRemainingSourceIndependentFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    session.sessionWithNormalSourceType_.insert({sessionId, sessionInfo});

    AudioSourceStrategyType strategyType;
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->insert({sessionInfo.sourceType, strategyType});
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    session.IsRemainingSourceIndependent();
}

void IsSourceTypeValidForEcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    session.IsSourceTypeValidForEc(sourceType);
}

void IsSessionIdValidForEcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerSession& session = AudioCapturerSession::GetInstance();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    session.IsSessionIdValidForEc(sessionId);
}
void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
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
    GetTargetSessionForEcFuzzTest,
    HandleIndependentInputpipeFuzzTest,
    IsStreamValidFuzzTest,
    FindRemainingNormalSessionFuzzTest,
    SetHearingAidReloadFlagFuzzTest,
    ReloadCaptureSoftLinkFuzzTest,
    ReloadCapturerSessionForInputPipeFuzzTest,
    GetTargetSessionIdForInputPipeFuzzTest,
    GetMaxPriorityForInputPipeFuzzTest,
    IsVirtualAudioRecognitionSessionFuzzTest,
    IsPipeInSourceStrategyMapFuzzTest,
    IsRemainingSourceIndependentFuzzTest,
    IsSourceTypeValidForEcFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}