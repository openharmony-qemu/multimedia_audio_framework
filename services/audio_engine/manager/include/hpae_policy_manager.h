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

#ifndef HPAE_POLICY_MANAGER_H
#define HPAE_POLICY_MANAGER_H

#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaePolicyManager {
public:
    static HpaePolicyManager &GetInstance();
    ~HpaePolicyManager();
    // interfaces for render effect
    void InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectChainManagerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList);
    void SetOutputDeviceSink(int32_t device, const std::string &sinkName);
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState);
    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType);
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType);
    int32_t EffectRotationUpdate(const uint32_t rotationState);
    int32_t SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume);
    int32_t SetAbsVolumeStateToEffect(const bool absVolumeState);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    void InitHdiState();
    void UpdateEffectBtOffloadSupported(const bool &isSupported);
    void UpdateParamExtra(const std::string &mainkey, const std::string &subkey, const std::string &value);

    // interfaces for capture effect
    void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList);
    int32_t SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice,
        const std::string &deviceName = "");
    int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice);
    int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol);
    int32_t SetMicrophoneMuteInfo(const bool &isMute);
    int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol);
    int32_t SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    void UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
        const std::string &extraSceneType);
    void LoadEffectProperties();
    std::string GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition);
    void SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition, const std::string &value);
    void SendInitCommandToAlgo();
    void AddStreamVolumeToEffect(const std::string stringSessionId, const float streamVolume);
    void DeleteStreamVolumeToEffect(const std::string stringSessionID);
private:
    HpaePolicyManager();
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_POLICY_MANAGER_H