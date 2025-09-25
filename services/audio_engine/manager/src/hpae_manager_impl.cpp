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
#ifndef LOG_TAG
#define LOG_TAG "HpaeManagerImpl"
#endif

#include "audio_errors.h"
#include "hpae_manager_impl.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeManagerImpl::HpaeManagerImpl()
{
    manager_ = std::make_shared<HpaeManager>();
}

int32_t HpaeManagerImpl::Init()
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->Init();
}

int32_t HpaeManagerImpl::DeInit()
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->DeInit();
}

int32_t HpaeManagerImpl::RegisterSerivceCallback(const std::weak_ptr<AudioServiceHpaeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->RegisterSerivceCallback(callback);
}

int32_t HpaeManagerImpl::RegisterHpaeDumpCallback(const std::weak_ptr<AudioServiceHpaeDumpCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->RegisterHpaeDumpCallback(callback);
}

void HpaeManagerImpl::DumpSinkInfo(std::string deviceName)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DumpSinkInfo(std::move(deviceName));
}

void HpaeManagerImpl::DumpSourceInfo(std::string deviceName)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DumpSourceInfo(std::move(deviceName));
}

void HpaeManagerImpl::DumpAllAvailableDevice(HpaeDeviceInfo &devicesInfo)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DumpAllAvailableDevice(devicesInfo);
}

void HpaeManagerImpl::DumpSinkInputsInfo()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DumpSinkInputsInfo();
}

void HpaeManagerImpl::DumpSourceOutputsInfo()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DumpSourceOutputsInfo();
}

uint32_t HpaeManagerImpl::OpenAudioPort(const AudioModuleInfo &audioModuleInfo)
{
    CHECK_AND_RETURN_RET_LOG(manager_, 0,
        "manager is nullptr");
    return manager_->OpenAudioPort(audioModuleInfo);
}

uint32_t HpaeManagerImpl::ReloadAudioPort(const AudioModuleInfo &audioModuleInfo)
{
    CHECK_AND_RETURN_RET_LOG(manager_, -1, "manager is nullptr");
    return manager_->ReloadAudioPort(audioModuleInfo);
}

int32_t HpaeManagerImpl::CloseAudioPort(int32_t audioHandleIndex)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->CloseAudioPort(audioHandleIndex);
}

int32_t HpaeManagerImpl::GetSinkInfoByIdx(const int32_t &sinkIdx,
    std::function<void(const HpaeSinkInfo &sinkInfo, int32_t result)> callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetSinkInfoByIdx(sinkIdx, callback);
}

int32_t HpaeManagerImpl::GetSourceInfoByIdx(const int32_t &sourceIdx,
    std::function<void(const HpaeSourceInfo &sourceInfo, int32_t result)> callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetSourceInfoByIdx(sourceIdx, callback);
}

int32_t HpaeManagerImpl::GetAllSinkInputs()
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->GetAllSinkInputs();
}

int32_t HpaeManagerImpl::GetAllSourceOutputs()
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->GetAllSourceOutputs();
}

int32_t HpaeManagerImpl::MoveSourceOutputByIndexOrName(
    uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->MoveSourceOutputByIndexOrName(sourceOutputId,
        sourceIndex, std::move(sourceName));
}

int32_t HpaeManagerImpl::MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->MoveSinkInputByIndexOrName(sinkInputId,
        sinkIndex, std::move(sinkName));
}

void HpaeManagerImpl::HandleMsg()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->HandleMsg();
}

bool HpaeManagerImpl::IsInit()
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->IsInit();
}

bool HpaeManagerImpl::IsRunning()
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->IsRunning();
}

bool HpaeManagerImpl::IsMsgProcessing()
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->IsMsgProcessing();
}

// async interface
int32_t HpaeManagerImpl::SetDefaultSink(std::string name)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->SetDefaultSink(std::move(name));
}

int32_t HpaeManagerImpl::SetDefaultSource(std::string name)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->SetDefaultSource(std::move(name));
}

int32_t HpaeManagerImpl::SuspendAudioDevice(std::string &audioPortName, bool isSuspend)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->SuspendAudioDevice(audioPortName, isSuspend);
}

bool HpaeManagerImpl::SetSinkMute(const std::string &sinkName, bool isMute, bool isSync)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false,
        "manager is nullptr");
    return manager_->SetSinkMute(sinkName, isMute, isSync);
}

int32_t HpaeManagerImpl::SetSourceOutputMute(int32_t uid, bool setMute)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->SetSourceOutputMute(uid, setMute);
}

int32_t HpaeManagerImpl::GetAllSinks()
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE,
        "manager is nullptr");
    return manager_->GetAllSinks();
}

// play and record stream interface
int32_t HpaeManagerImpl::CreateStream(const HpaeStreamInfo &streamInfo)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->CreateStream(streamInfo);
}

int32_t HpaeManagerImpl::DestroyStream(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->DestroyStream(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::Start(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Start(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::StartWithSyncId(HpaeStreamClassType streamClassType, uint32_t sessionId, int32_t syncId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->StartWithSyncId(streamClassType, sessionId, syncId);
}

int32_t HpaeManagerImpl::Pause(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Pause(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::Flush(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Flush(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::Drain(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Drain(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::Stop(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Stop(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::Release(HpaeStreamClassType streamClassType, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->Release(streamClassType, sessionId);
}

int32_t HpaeManagerImpl::RegisterStatusCallback(HpaeStreamClassType streamClassType, uint32_t sessionId,
    const std::weak_ptr<IStreamStatusCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->RegisterStatusCallback(streamClassType, sessionId, callback);
}

// record stream interface
int32_t HpaeManagerImpl::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->RegisterReadCallback(sessionId, callback);
}

int32_t HpaeManagerImpl::GetSourceOutputInfo(uint32_t sessionId, HpaeStreamInfo &streamInfo)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetSourceOutputInfo(sessionId, streamInfo);
}

// play stream interface
int32_t HpaeManagerImpl::SetClientVolume(uint32_t sessionId, float volume)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetClientVolume(sessionId, volume);
}

int32_t HpaeManagerImpl::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetLoudnessGain(sessionId, loudnessGain);
}

int32_t HpaeManagerImpl::SetRate(uint32_t sessionId, int32_t rate)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetRate(sessionId, rate);
}

int32_t HpaeManagerImpl::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAudioEffectMode(sessionId, effectMode);
}

int32_t HpaeManagerImpl::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetAudioEffectMode(sessionId, effectMode);
}

int32_t HpaeManagerImpl::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetPrivacyType(sessionId, privacyType);
}

int32_t HpaeManagerImpl::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetPrivacyType(sessionId, privacyType);
}

int32_t HpaeManagerImpl::RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->RegisterWriteCallback(sessionId, callback);
}

int32_t HpaeManagerImpl::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetOffloadPolicy(sessionId, state);
}

size_t HpaeManagerImpl::GetWritableSize(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(manager_, 0, "manager is nullptr");
    return manager_->GetWritableSize(sessionId);
}

int32_t HpaeManagerImpl::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

int32_t HpaeManagerImpl::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->UpdateMaxLength(sessionId, maxLength);
}

int32_t HpaeManagerImpl::SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetOffloadRenderCallbackType(sessionId, type);
}

void HpaeManagerImpl::SetSpeed(uint32_t sessionId, float speed)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->SetSpeed(sessionId, speed);
}

// interfaces for render effect
void HpaeManagerImpl::InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectChainManagerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->InitAudioEffectChainManager(effectChains, effectChainManagerParam, effectLibraryList);
}

void HpaeManagerImpl::SetOutputDeviceSink(int32_t device, const std::string &sinkName)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->SetOutputDeviceSink(device, sinkName);
}

int32_t HpaeManagerImpl::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->UpdateSpatializationState(spatializationState);
}

int32_t HpaeManagerImpl::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->UpdateSpatialDeviceType(spatialDeviceType);
}

int32_t HpaeManagerImpl::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetSpatializationSceneType(spatializationSceneType);
}

int32_t HpaeManagerImpl::EffectRotationUpdate(const uint32_t rotationState)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->EffectRotationUpdate(rotationState);
}

int32_t HpaeManagerImpl::SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetEffectSystemVolume(systemVolumeType, systemVolume);
}

int32_t HpaeManagerImpl::SetAbsVolumeStateToEffect(const bool absVolumeState)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAbsVolumeStateToEffect(absVolumeState);
}
int32_t HpaeManagerImpl::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAudioEffectProperty(propertyArray);
}

int32_t HpaeManagerImpl::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetAudioEffectProperty(propertyArray);
}

int32_t HpaeManagerImpl::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAudioEffectProperty(propertyArray);
}

int32_t HpaeManagerImpl::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetAudioEffectProperty(propertyArray);
}

void HpaeManagerImpl::InitHdiState()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->InitHdiState();
}

void HpaeManagerImpl::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->UpdateEffectBtOffloadSupported(isSupported);
}

void HpaeManagerImpl::UpdateParamExtra(const std::string &mainkey, const std::string &subkey, const std::string &value)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->UpdateParamExtra(mainkey, subkey, value);
}

// interfaces for capture effect
void HpaeManagerImpl::InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList);
}

int32_t HpaeManagerImpl::SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetOutputDevice(renderId, outputDevice);
}

int32_t HpaeManagerImpl::SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetVolumeInfo(volumeType, systemVol);
}

int32_t HpaeManagerImpl::SetMicrophoneMuteInfo(const bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetMicrophoneMuteInfo(isMute);
}

int32_t HpaeManagerImpl::SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetStreamVolumeInfo(sessionId, streamVol);
}

int32_t HpaeManagerImpl::SetAudioEnhanceProperty(
    const AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t HpaeManagerImpl::GetAudioEnhanceProperty(
    AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t HpaeManagerImpl::SetAudioEnhanceProperty(
    const AudioEnhancePropertyArray &propertyArray, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->SetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t HpaeManagerImpl::GetAudioEnhanceProperty(
    AudioEnhancePropertyArray &propertyArray, DeviceType deviceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->GetAudioEnhanceProperty(propertyArray, deviceType);
}

void HpaeManagerImpl::UpdateExtraSceneType(
    const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
}

void HpaeManagerImpl::NotifySettingsDataReady()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    return manager_->NotifySettingsDataReady();
}

void HpaeManagerImpl::NotifyAccountsChanged()
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    return manager_->NotifyAccountsChanged();
}

bool HpaeManagerImpl::IsAcousticEchoCancelerSupported(SourceType sourceType)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->IsAcousticEchoCancelerSupported(sourceType);
}

bool HpaeManagerImpl::SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->SetEffectLiveParameter(params);
}

bool HpaeManagerImpl::GetEffectLiveParameter(const std::vector<std::string> &subKeys,
    std::vector<std::pair<std::string, std::string>> &result)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->GetEffectLiveParameter(subKeys, result);
}

int32_t HpaeManagerImpl::UpdateCollaborativeState(bool isCollaborationEnabled)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->UpdateCollaborativeState(isCollaborationEnabled);
}

void HpaeManagerImpl::AddStreamVolumeToEffect(const std::string stringSessionID, const float streamVolume)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->AddStreamVolumeToEffect(stringSessionID, streamVolume);
}

void HpaeManagerImpl::DeleteStreamVolumeToEffect(const std::string stringSessionID)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->DeleteStreamVolumeToEffect(stringSessionID);
}

// interfaces for injector
void HpaeManagerImpl::UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->UpdateAudioPortInfo(sinkPortIndex, audioPortInfo);
}

void HpaeManagerImpl::AddCaptureInjector(
    const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex, const SourceType &sourceType)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->AddCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

void HpaeManagerImpl::RemoveCaptureInjector(
    const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex, const SourceType &sourceType)
{
    CHECK_AND_RETURN_LOG(manager_, "manager is nullptr");
    manager_->RemoveCaptureInjector(sinkPortIndex, sourcePortIndex, sourceType);
}

int32_t HpaeManagerImpl::PeekAudioData(
    const uint32_t &sinkPortIndex, uint8_t *buffer, size_t bufferSize, AudioStreamInfo &streamInfo)
{
    CHECK_AND_RETURN_RET_LOG(manager_, ERR_ILLEGAL_STATE, "manager is nullptr");
    return manager_->PeekAudioData(sinkPortIndex, buffer, bufferSize, streamInfo);
}

bool HpaeManagerImpl::IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout)
{
    CHECK_AND_RETURN_RET_LOG(manager_, false, "manager is nullptr");
    return manager_->IsChannelLayoutSupportedForDspEffect(channelLayout);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
