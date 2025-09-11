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
#define LOG_TAG "HpaePolicyManager"
#endif

#include "hpae_policy_manager.h"
#include <string>
#include "audio_errors.h"
#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaePolicyManager &HpaePolicyManager::GetInstance()
{
    static HpaePolicyManager hpaePolicyManager;
    return hpaePolicyManager;
}

HpaePolicyManager::HpaePolicyManager()
{
    AUDIO_DEBUG_LOG("created");
}

HpaePolicyManager::~HpaePolicyManager()
{
    AUDIO_WARNING_LOG("destroyed");
}

void HpaePolicyManager::InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectChainManagerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(effectChains,
        effectChainManagerParam, effectLibraryList);
}

void HpaePolicyManager::SetOutputDeviceSink(int32_t device, const std::string &sinkName)
{
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
}

int32_t HpaePolicyManager::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    return AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
}

int32_t HpaePolicyManager::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    return AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(spatialDeviceType);
}

int32_t HpaePolicyManager::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    return AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
}

int32_t HpaePolicyManager::EffectRotationUpdate(const uint32_t rotationState)
{
    return AudioEffectChainManager::GetInstance()->EffectRotationUpdate(rotationState);
}

int32_t HpaePolicyManager::SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume)
{
    return AudioEffectChainManager::GetInstance()->SetEffectSystemVolume(systemVolumeType, systemVolume);
}

int32_t HpaePolicyManager::SetAbsVolumeStateToEffect(const bool absVolumeState)
{
    return AudioEffectChainManager::GetInstance()->SetAbsVolumeStateToEffect(absVolumeState);
}

int32_t HpaePolicyManager::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(propertyArray);
}

int32_t HpaePolicyManager::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    return AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(propertyArray);
}

int32_t HpaePolicyManager::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    return AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(propertyArray);
}

int32_t HpaePolicyManager::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    return AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(propertyArray);
}

void HpaePolicyManager::InitHdiState()
{
    AudioEffectChainManager::GetInstance()->InitHdiState();
}

void HpaePolicyManager::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(isSupported);
}

void HpaePolicyManager::UpdateParamExtra(const std::string &mainkey, const std::string &subkey,
    const std::string &value)
{
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
}

void HpaePolicyManager::InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    AudioEnhanceChainManager::GetInstance()->InitAudioEnhanceChainManager(enhanceChains, managerParam,
        enhanceLibraryList);
}

int32_t HpaePolicyManager::SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice,
    const std::string &deviceName)
{
    return AudioEnhanceChainManager::GetInstance()->SetInputDevice(captureId, inputDevice, deviceName);
}

int32_t HpaePolicyManager::SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice)
{
    return AudioEnhanceChainManager::GetInstance()->SetOutputDevice(renderId, outputDevice);
}

int32_t HpaePolicyManager::SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol)
{
    return AudioEnhanceChainManager::GetInstance()->SetVolumeInfo(volumeType, systemVol);
}

int32_t HpaePolicyManager::SetMicrophoneMuteInfo(const bool &isMute)
{
    return AudioEnhanceChainManager::GetInstance()->SetMicrophoneMuteInfo(isMute);
}

int32_t HpaePolicyManager::SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol)
{
    return AudioEnhanceChainManager::GetInstance()->SetStreamVolumeInfo(sessionId, streamVol);
}

int32_t HpaePolicyManager::SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    return AudioEnhanceChainManager::GetInstance()->SetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t HpaePolicyManager::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
    DeviceType deviceType)
{
    return AudioEnhanceChainManager::GetInstance()->GetAudioEnhanceProperty(propertyArray, deviceType);
}

int32_t HpaePolicyManager::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    return AudioEnhanceChainManager::GetInstance()->SetAudioEnhanceProperty(propertyArray, deviceType);
}

// todo: change to callback mode
int32_t HpaePolicyManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    return AudioEnhanceChainManager::GetInstance()->GetAudioEnhanceProperty(propertyArray, deviceType);
}

void HpaePolicyManager::UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    return AudioEnhanceChainManager::GetInstance()->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
}

void HpaePolicyManager::LoadEffectProperties()
{
    return AudioEffectChainManager::GetInstance()->LoadEffectProperties();
}

std::string HpaePolicyManager::GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, "false", "local device manager is nullptr");
    return deviceManager->GetAudioParameter(adapterName, key, condition);
}

void HpaePolicyManager::SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "local device manager is nullptr");
    deviceManager->SetAudioParameter(adapterName, key, condition, value);
}

void HpaePolicyManager::SendInitCommandToAlgo()
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEnhanceChainManager != nullptr, "audioEnhanceChainManager is null");
    audioEnhanceChainManager->SendInitCommand();
}

void HpaePolicyManager::AddStreamVolumeToEffect(const std::string stringSessionId, const float streamVolume)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->StreamVolumeUpdate(stringSessionId, streamVolume);
}

void HpaePolicyManager::DeleteStreamVolumeToEffect(const std::string stringSessionId)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->DeleteStreamVolume(stringSessionId);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
