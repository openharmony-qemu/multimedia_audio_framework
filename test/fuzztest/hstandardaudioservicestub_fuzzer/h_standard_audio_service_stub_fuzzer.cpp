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
#include <unordered_map>
#include <vector>
#include "audio_server.h"
#include "../fuzz_utils.h"
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {
using namespace std;
using FuzzFuncPtr = void(*)(FuzzedDataProvider&);
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
void FuzzTestOne(FuzzedDataProvider &provider);
void FuzzTestTwo(FuzzedDataProvider &provider);

void GetAudioParameter(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string key = provider.ConsumeRandomLengthString();
    std::string value = provider.ConsumeRandomLengthString();
    audioServer->GetAudioParameter(key, value);
}

void SetAudioParameter(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string key = provider.ConsumeRandomLengthString();
    std::string value = provider.ConsumeRandomLengthString();
    audioServer->SetAudioParameter(key, value);
}

void GetExtraParameters(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string mainKey = provider.ConsumeRandomLengthString();
    std::vector<std::string> subKeys;
    std::vector<StringPair> parameters;
    int maxScore = 2;
    for (int i = 0 ; i < maxScore; i++) {
        std::string str =  provider.ConsumeRandomLengthString();
        subKeys.push_back(str);
    }
    audioServer->GetExtraParameters(mainKey, subKeys, parameters);
}

void SetExtraParameters(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string key = provider.ConsumeRandomLengthString();
    std::vector<StringPair> parameters;
    audioServer->SetExtraParameters(key, parameters);
}

void SetMicrophoneMute(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool isMute = provider.ConsumeBool();
    audioServer->SetMicrophoneMute(isMute);
}

void SetAudioScene(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t audioScene = provider.ConsumeIntegral<int32_t>();
    int32_t a2dpOffloadFlag = provider.ConsumeIntegral<int32_t>();
    bool scoExcludeFlag = provider.ConsumeBool();
    audioServer->SetAudioScene(audioScene, a2dpOffloadFlag, scoExcludeFlag);
}
 
void UpdateActiveDeviceRoute(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t type = provider.ConsumeIntegral<int32_t>();
    int32_t flag = provider.ConsumeIntegral<int32_t>();
    int32_t a2dpOffloadFlag = provider.ConsumeIntegral<int32_t>();
    audioServer->UpdateActiveDeviceRoute(type, flag, a2dpOffloadFlag);
}

void UpdateActiveDevicesRoute(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<IntPair> activeDevices;
    int32_t a2dpOffloadFlag = provider.ConsumeIntegral<int32_t>();
    std::string deviceName =  provider.ConsumeRandomLengthString();
    audioServer->UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag, deviceName);
}


void UpdateDualToneState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool enable = provider.ConsumeBool();
    int32_t sessionId = provider.ConsumeIntegral<int32_t>();
    std::string dupSinkName =  provider.ConsumeRandomLengthString();
    audioServer->UpdateDualToneState(enable, sessionId, dupSinkName);
}

void GetTransactionId(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    int32_t deviceRole = provider.ConsumeIntegral<int32_t>();
    uint64_t transactionId = provider.ConsumeIntegral<uint64_t>();
    audioServer->GetTransactionId(deviceType, deviceRole, transactionId);
}

void SetParameterCallback(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<IRemoteObject> iRemoteObject = nullptr;
    audioServer->SetParameterCallback(iRemoteObject);
}

void GetAudioMoreParameters(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string networkId = provider.ConsumeRandomLengthString();
    int32_t key = provider.ConsumeIntegral<int32_t>();
    std::string condition = provider.ConsumeRandomLengthString();
    std::string value = provider.ConsumeRandomLengthString();
    audioServer->GetAudioParameter(networkId, key, condition, value);
}

void SetAudioMoreParameter(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string networkId = provider.ConsumeRandomLengthString();
    int32_t key = provider.ConsumeIntegral<int32_t>();
    std::string condition = provider.ConsumeRandomLengthString();
    std::string value = provider.ConsumeRandomLengthString();
    audioServer->SetAudioParameter(networkId, key, condition, value);
}

void NotifyDeviceInfo(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string networkId = provider.ConsumeRandomLengthString();
    int32_t connected = provider.ConsumeIntegral<int32_t>();
    audioServer->NotifyDeviceInfo(networkId, connected);
}

void CheckRemoteDeviceState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string networkId = provider.ConsumeRandomLengthString();
    int32_t deviceRole = provider.ConsumeIntegral<int32_t>();
    bool isStartDevice = provider.ConsumeBool();
    audioServer->CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
}

void SetVoiceVolume(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    float volume = provider.ConsumeFloatingPoint<float>();
    audioServer->SetVoiceVolume(volume);
}

void SetAudioMonoState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool audioMono = provider.ConsumeBool();
    audioServer->SetAudioMonoState(audioMono);
}

void SetAudioBalanceValue(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    float audioBalance = provider.ConsumeFloatingPoint<float>();
    audioServer->SetAudioBalanceValue(audioBalance);
}

void CreateAudioProcess(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioProcessConfig config;
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    AudioPlaybackCaptureConfig filterConfig;
    sptr<IRemoteObject> client = nullptr;
    audioServer->CreateAudioProcess(config, errorCode, filterConfig, client);
}

void SetOutputDeviceSink(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string sinkName = provider.ConsumeRandomLengthString();
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    audioServer->SetOutputDeviceSink(deviceType, sinkName);
}

void SetActiveOutputDevice(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    audioServer->SetActiveOutputDevice(deviceType);
}

void CreatePlaybackCapturerManager(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool isSuccess = provider.ConsumeBool();
    audioServer->CreatePlaybackCapturerManager(isSuccess);
}

void RegiestPolicyProvider(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<IRemoteObject> object = nullptr;
    audioServer->RegiestPolicyProvider(object);
}

void SetWakeupSourceCallback(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<IRemoteObject> object = nullptr;
    audioServer->SetWakeupSourceCallback(object);
}

void OffloadSetVolume(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    float volume = provider.ConsumeFloatingPoint<float>();
    std::string deviceClass = provider.ConsumeRandomLengthString();
    std::string networkId = provider.ConsumeRandomLengthString();
    audioServer->OffloadSetVolume(volume, deviceClass, networkId);
}

void NotifyStreamVolumeChanged(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    float volume = provider.ConsumeFloatingPoint<float>();
    int32_t streamType = provider.ConsumeIntegral<int32_t>();
    audioServer->NotifyStreamVolumeChanged(streamType, volume);
}

void GetMaxAmplitude(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool isOutputDevice = provider.ConsumeBool();
    float maxAmplitude = provider.ConsumeFloatingPoint<float>();
    int32_t sourceType = provider.ConsumeIntegral<int32_t>();
    std::string deviceClass = provider.ConsumeRandomLengthString();
    audioServer->GetMaxAmplitude(isOutputDevice, deviceClass, sourceType, maxAmplitude);
}

void ResetRouteForDisconnect(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t streamType = provider.ConsumeIntegral<int32_t>();
    audioServer->ResetRouteForDisconnect(streamType);
}

void UpdateLatencyTimestamp(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string timestamp = provider.ConsumeRandomLengthString();
    bool isRenderer = provider.ConsumeBool();
    audioServer->UpdateLatencyTimestamp(timestamp, isRenderer);
}

void SetAsrAecMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrAecMode = provider.ConsumeIntegral<int32_t>();
    audioServer->SetAsrAecMode(asrAecMode);
}

void GetAsrAecMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrAecMode = provider.ConsumeIntegral<int32_t>();
    audioServer->GetAsrAecMode(asrAecMode);
}

void SetAsrNoiseSuppressionMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrNoiseSuppressionMode = provider.ConsumeIntegral<int32_t>();
    audioServer->SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
}

void SetOffloadMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    int32_t state = provider.ConsumeIntegral<int32_t>();
    bool isAppBack = provider.ConsumeBool();
    audioServer->SetOffloadMode(sessionId, state, isAppBack);
}

void UnsetOffloadMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    audioServer->UnsetOffloadMode(sessionId);
}

void CheckHibernateState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool hibernate = provider.ConsumeBool();
    audioServer->CheckHibernateState(hibernate);
}

void GetAsrNoiseSuppressionMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrNoiseSuppressionMode = provider.ConsumeIntegral<int32_t>();
    audioServer->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
}

void SetAsrWhisperDetectionMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrWhisperDetectionMode = provider.ConsumeIntegral<int32_t>();
    audioServer->SetAsrWhisperDetectionMode(asrWhisperDetectionMode);
}

void GetAsrWhisperDetectionMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrWhisperDetectionMode = provider.ConsumeIntegral<int32_t>();
    audioServer->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
}

void SetAsrVoiceControlMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrVoiceControlMode = provider.ConsumeIntegral<int32_t>();
    bool on = provider.ConsumeBool();
    audioServer->SetAsrVoiceControlMode(asrVoiceControlMode, on);
}

void SetAsrVoiceMuteMode(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t asrVoiceControlMode = provider.ConsumeIntegral<int32_t>();
    bool on = provider.ConsumeBool();
    audioServer->SetAsrVoiceMuteMode(asrVoiceControlMode, on);
}

void IsWhispering(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t whisperRes = provider.ConsumeIntegral<int32_t>();
    audioServer->IsWhispering(whisperRes);
}

void SuspendRenderSink(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string sinkName = provider.ConsumeRandomLengthString();
    audioServer->SuspendRenderSink(sinkName);
}

void RestoreRenderSink(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string sinkName = provider.ConsumeRandomLengthString();
    audioServer->RestoreRenderSink(sinkName);
}

void SetSinkMuteForSwitchDevice(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string devceClass = provider.ConsumeRandomLengthString();
    int32_t durationUs = provider.ConsumeIntegral<int32_t>();
    bool mute = provider.ConsumeBool();
    audioServer->SetSinkMuteForSwitchDevice(devceClass, durationUs, mute);
}

void UpdateSessionConnectionState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t sessionId = provider.ConsumeIntegral<int32_t>();
    int32_t state = provider.ConsumeIntegral<int32_t>();
    audioServer->UpdateSessionConnectionState(sessionId, state);
}

void SetNonInterruptMute(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    bool muteFlag = provider.ConsumeBool();
    audioServer->SetNonInterruptMute(sessionId, muteFlag);
}

void RestoreSession(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionID = provider.ConsumeIntegral<uint32_t>();
    RestoreInfoIpc restoreInfoIn;
    audioServer->RestoreSession(sessionID, restoreInfoIn);
}

void GetStandbyStatus(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    bool isStandby = provider.ConsumeBool();
    int64_t enterStandbyTime = provider.ConsumeIntegral<int64_t>();
    audioServer->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
}

void GenerateSessionId(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    audioServer->GenerateSessionId(sessionId);
}

void GetAllSinkInputs(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<SinkInput> sinkInputs;
    audioServer->GetAllSinkInputs(sinkInputs);
}

void NotifyAccountsChanged(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServer->NotifyAccountsChanged();
}

void NotifyAudioPolicyReady(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServer->NotifyAudioPolicyReady();
}

void SetInnerCapLimit(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t innerCapLimit = provider.ConsumeIntegral<uint32_t>();
    audioServer->SetInnerCapLimit(innerCapLimit);
}

void UnloadHdiAdapter(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t devMgrType = provider.ConsumeIntegral<uint32_t>();
    const std::string adapterName = provider.ConsumeRandomLengthString();
    bool force = provider.ConsumeBool();
    audioServer->UnloadHdiAdapter(devMgrType, adapterName, force);
}

void CheckCaptureLimit(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioPlaybackCaptureConfig config;
    int32_t innerCapId = provider.ConsumeIntegral<int32_t>();
    audioServer->CheckCaptureLimit(config, innerCapId);
}

void ReleaseCaptureLimit(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t innerCapId = provider.ConsumeIntegral<int32_t>();
    audioServer->ReleaseCaptureLimit(innerCapId);
}

void CreateHdiSinkPort(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string deviceClass = provider.ConsumeRandomLengthString();
    std::string idInfo = provider.ConsumeRandomLengthString();
    uint32_t renderId = provider.ConsumeIntegral<uint32_t>();
    IAudioSinkAttr attr;
    audioServer->CreateHdiSinkPort(deviceClass, idInfo, attr, renderId);
}

void CreateSinkPort(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t idBase = provider.ConsumeIntegral<uint32_t>();
    uint32_t idType = provider.ConsumeIntegral<uint32_t>();
    std::string deviceClass = provider.ConsumeRandomLengthString();
    std::string idInfo = provider.ConsumeRandomLengthString();
    uint32_t renderId = provider.ConsumeIntegral<uint32_t>();
    IAudioSinkAttr attr;
    audioServer->CreateSinkPort(idBase, idType, idInfo, attr, renderId);
}

void CreateHdiSourcePort(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string deviceClass = provider.ConsumeRandomLengthString();
    std::string idInfo = provider.ConsumeRandomLengthString();
    uint32_t captureId = provider.ConsumeIntegral<uint32_t>();
    IAudioSourceAttr attr;
    audioServer->CreateHdiSourcePort(deviceClass, idInfo, attr, captureId);
}

void CreateSourcePort(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t idBase = provider.ConsumeIntegral<uint32_t>();
    uint32_t idType = provider.ConsumeIntegral<uint32_t>();
    std::string idInfo = provider.ConsumeRandomLengthString();
    uint32_t captureId = provider.ConsumeIntegral<uint32_t>();
    IAudioSourceAttr attr;
    audioServer->CreateSourcePort(idBase, idType, idInfo, attr, captureId);
}

void DestroyHdiPort(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t id = provider.ConsumeIntegral<uint32_t>();
    audioServer->DestroyHdiPort(id);
}

void SetDeviceConnectedFlag(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool flag = provider.ConsumeBool();
    audioServer->SetDeviceConnectedFlag(flag);
}

void SetDmDeviceType(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint16_t dmDeviceType = provider.ConsumeIntegral<uint16_t>();
    int32_t deviceTypeIn = provider.ConsumeIntegral<int32_t>();
    audioServer->SetDmDeviceType(dmDeviceType, deviceTypeIn);
}

void RegisterDataTransferMonitorParam(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t callbackId = provider.ConsumeIntegral<int32_t>();
    DataTransferMonitorParam param;
    audioServer->RegisterDataTransferMonitorParam(callbackId, param);
}

void UnregisterDataTransferMonitorParam(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t callbackId = provider.ConsumeIntegral<int32_t>();
    audioServer->UnregisterDataTransferMonitorParam(callbackId);
}

void RegisterDataTransferCallback(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<IRemoteObject> object = nullptr;
    audioServer->RegisterDataTransferCallback(object);
}


void NotifySettingsDataReady(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServer->NotifySettingsDataReady();
}

void IsAcousticEchoCancelerSupported(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t sourceType = provider.ConsumeIntegral<int32_t>();
    bool isSupported = provider.ConsumeBool();
    audioServer->IsAcousticEchoCancelerSupported(sourceType, isSupported);
}

void SetSessionMuteState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    bool insert = provider.ConsumeBool();
    bool muteFlag = provider.ConsumeBool();
    audioServer->SetSessionMuteState(sessionId, insert, muteFlag);
}

void SetLatestMuteState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    bool muteFlag = provider.ConsumeBool();
    audioServer->SetLatestMuteState(sessionId, muteFlag);
}

void ForceStopAudioStream(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t audioType = provider.ConsumeIntegral<int32_t>();
    audioServer->ForceStopAudioStream(audioType);
}

void CreateAudioWorkgroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<IRemoteObject> object = nullptr;
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    audioServer->CreateAudioWorkgroup(object, workgroupId);
}

void ReleaseAudioWorkgroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    audioServer->ReleaseAudioWorkgroup(workgroupId);
}

void AddThreadToGroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    int32_t tokenId = provider.ConsumeIntegral<int32_t>();
    audioServer->AddThreadToGroup(workgroupId, tokenId);
}

void RemoveThreadFromGroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    int32_t tokenId = provider.ConsumeIntegral<int32_t>();
    audioServer->RemoveThreadFromGroup(workgroupId, tokenId);
}

void StartGroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    uint64_t startTime = provider.ConsumeIntegral<uint64_t>();
    uint64_t deadlineTime = provider.ConsumeIntegral<uint64_t>();
    audioServer->StartGroup(workgroupId, startTime, deadlineTime);
}

void StopGroup(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    audioServer->StopGroup(workgroupId);
}

void SetBtHdiInvalidState(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServer->SetBtHdiInvalidState();
}

void SetKaraokeParameters(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string parameters = provider.ConsumeRandomLengthString();
    bool ret = provider.ConsumeBool();
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    audioServer->SetKaraokeParameters(deviceType, parameters, ret);
}

void IsAudioLoopbackSupported(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t mode = provider.ConsumeIntegral<int32_t>();
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    bool isSupported = provider.ConsumeBool();
    audioServer->IsAudioLoopbackSupported(mode, deviceType, isSupported);
}

void SetRenderWhitelist(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    int32_t mode = provider.ConsumeIntegral<int32_t>();
    bool isSupported = provider.ConsumeBool();
    std::vector<std::string> list;
    int maxScope = 2;
    for (int i = 0 ; i < maxScope ; i++) {
        std::string str =  provider.ConsumeRandomLengthString();
        list.push_back(str);
    }
    audioServer->SetRenderWhitelist(list);
}

void ImproveAudioWorkgroupPrio(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::unordered_map<int32_t, bool> threads;
    int maxScope = 2;
    for (int i = 0 ; i < maxScope ; i++) {
        int32_t key = provider.ConsumeIntegral<int32_t>();
        bool value = provider.ConsumeBool();
        threads.insert(std::make_pair(key, value));
    }
    audioServer->ImproveAudioWorkgroupPrio(threads);
}

void RestoreAudioWorkgroupPrio(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::unordered_map<int32_t, int32_t> threads;
    int maxScope = 2;
    for (int i = 0 ; i < maxScope ; i++) {
        int32_t key = provider.ConsumeIntegral<int32_t>();
        int32_t value = provider.ConsumeIntegral<int32_t>();
        threads.insert(std::make_pair(key, value));
    }
    audioServer->RestoreAudioWorkgroupPrio(threads);
}

void AddCaptureInjector(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sinkPortidx = provider.ConsumeIntegral<uint32_t>();
    std::string rate = provider.ConsumeRandomLengthString();
    std::string format = provider.ConsumeRandomLengthString();
    std::string channels = provider.ConsumeRandomLengthString();
    std::string bufferSiz = provider.ConsumeRandomLengthString();
    audioServer->AddCaptureInjector(sinkPortidx, rate, format, channels, bufferSiz);
}

void RemoveCaptureInjector(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sinkPortidx = provider.ConsumeIntegral<uint32_t>();
    audioServer->RemoveCaptureInjector(sinkPortidx);
}

void SetForegroundList(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<std::string> list;
    int maxScope = 2;
    for (int i = 0 ; i < maxScope ; i++) {
        std::string str =  provider.ConsumeRandomLengthString();
        list.push_back(str);
    }
    audioServer->SetForegroundList(list);
}

void GetVolumeDataCount(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::string sinkName =  provider.ConsumeRandomLengthString();
    int64_t volumeData = provider.ConsumeIntegral<int64_t>();
    audioServer->GetVolumeDataCount(sinkName, volumeData);
}

void GetPrivacyTypeAudioServer(FuzzedDataProvider &provider)
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    int32_t privacyType = provider.ConsumeIntegral<int32_t>();
    int32_t ret = provider.ConsumeIntegral<int32_t>();
    audioServer->GetPrivacyTypeAudioServer(sessionId, privacyType, ret);
}

FuzzFuncPtr g_fuzzFuncArray[] = {
    GetAudioParameter,
    SetAudioParameter,
    GetExtraParameters,
    SetExtraParameters,
    SetMicrophoneMute,
    SetAudioScene,
    UpdateActiveDeviceRoute,
    UpdateActiveDevicesRoute,
    UpdateDualToneState,
    GetTransactionId,
    SetParameterCallback,
    GetAudioMoreParameters,
    SetAudioMoreParameter,
    NotifyDeviceInfo,
    CheckRemoteDeviceState,
    SetVoiceVolume,
    SetAudioMonoState,
    SetAudioBalanceValue,
    CreateAudioProcess,
    SetOutputDeviceSink,
    SetActiveOutputDevice,
    CreatePlaybackCapturerManager,
    RegiestPolicyProvider,
    SetWakeupSourceCallback,
    OffloadSetVolume,
    NotifyStreamVolumeChanged,
    GetMaxAmplitude,
    ResetRouteForDisconnect,
    UpdateLatencyTimestamp,
    SetAsrAecMode,
    GetAsrAecMode,
    SetAsrNoiseSuppressionMode,
    SetOffloadMode,
    UnsetOffloadMode,
    CheckHibernateState,
    GetAsrNoiseSuppressionMode,
    SetAsrWhisperDetectionMode,
    GetAsrWhisperDetectionMode,
    SetAsrVoiceControlMode,
    SetAsrVoiceMuteMode,
    IsWhispering,
    SuspendRenderSink,
    RestoreRenderSink,
    SetSinkMuteForSwitchDevice,
    UpdateSessionConnectionState,
    SetNonInterruptMute,
    RestoreSession,
    GetStandbyStatus,
    GenerateSessionId,
    GetAllSinkInputs,
    NotifyAccountsChanged,
    NotifyAudioPolicyReady,
    SetInnerCapLimit,
    UnloadHdiAdapter,
    CheckCaptureLimit,
    ReleaseCaptureLimit,
    CreateHdiSinkPort,
    CreateSinkPort,
    CreateHdiSourcePort,
    CreateSourcePort,
    DestroyHdiPort,
    SetDeviceConnectedFlag,
    SetDmDeviceType,
    RegisterDataTransferMonitorParam,
    UnregisterDataTransferMonitorParam,
    RegisterDataTransferCallback,
    NotifySettingsDataReady,
    IsAcousticEchoCancelerSupported,
    SetSessionMuteState,
    SetLatestMuteState,
    ForceStopAudioStream,
    CreateAudioWorkgroup,
    ReleaseAudioWorkgroup,
    AddThreadToGroup,
    RemoveThreadFromGroup,
    StartGroup,
    StopGroup,
    SetBtHdiInvalidState,
    SetKaraokeParameters,
    IsAudioLoopbackSupported,
    SetRenderWhitelist,
    ImproveAudioWorkgroupPrio,
    RestoreAudioWorkgroupPrio,
    AddCaptureInjector,
    RemoveCaptureInjector,
    SetForegroundList,
    GetVolumeDataCount,
    GetPrivacyTypeAudioServer,
};

void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray(g_fuzzFuncArray);
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
