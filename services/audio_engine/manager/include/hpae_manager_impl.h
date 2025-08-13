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
#ifndef HPAE_MANAGER_IMPL_H
#define HPAE_MANAGER_IMPL_H

#include <functional>
#include <any>
#include "audio_module_info.h"
#include "hpae_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeManagerImpl : public IHpaeManager {
public:
    HpaeManagerImpl();
    ~HpaeManagerImpl() = default;
    // sync interface
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t RegisterSerivceCallback(const std::weak_ptr<AudioServiceHpaeCallback> &callback) override;
    int32_t RegisterHpaeDumpCallback(const std::weak_ptr<AudioServiceHpaeDumpCallback> &callback) override;
    void DumpSinkInfo(std::string deviceName) override;
    void DumpSourceInfo(std::string deviceName) override;
    void DumpAllAvailableDevice(HpaeDeviceInfo &devicesInfo) override;
    void DumpSinkInputsInfo() override;
    void DumpSourceOutputsInfo() override;
    uint32_t OpenAudioPort(const AudioModuleInfo &audioModuleInfo) override;
    int32_t CloseAudioPort(int32_t audioHandleIndex) override;
    uint32_t ReloadAudioPort(const AudioModuleInfo &audioModuleInfo) override;
    int32_t GetSinkInfoByIdx(const int32_t &sinkIdx,
        std::function<void(const HpaeSinkInfo &sinkInfo, int32_t result)> callback) override;
    int32_t GetSourceInfoByIdx(const int32_t &sourceIdx,
        std::function<void(const HpaeSourceInfo &sourceInfo, int32_t result)> callback) override;

    int32_t GetAllSinkInputs() override;
    int32_t GetAllSourceOutputs() override;
    int32_t MoveSourceOutputByIndexOrName(
        uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName) override;
    int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) override;
    void HandleMsg() override;
    bool IsInit() override;
    bool IsRunning() override;
    bool IsMsgProcessing() override;
    // async interface
    int32_t SetDefaultSink(std::string name) override;
    int32_t SetDefaultSource(std::string name) override;
    int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) override;
    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) override;
    int32_t SetSourceOutputMute(int32_t uid, bool setMute) override;
    int32_t GetAllSinks() override;

    // play and record stream interface
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Start(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t StartWithSyncId(HpaeStreamClassType streamClassType, uint32_t sessionId, int32_t syncId) override;
    int32_t Pause(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Flush(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Drain(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Stop(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Release(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t RegisterStatusCallback(HpaeStreamClassType streamClassType, uint32_t sessionId,
        const std::weak_ptr<IStreamStatusCallback> &callback) override;
    // record stream interface
    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeStreamInfo &streamInfo) override;
    // play stream interface
    int32_t SetClientVolume(uint32_t sessionId, float volume) override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    int32_t SetRate(uint32_t sessionId, int32_t rate) override;
    int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) override;
    int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) override;
    int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) override;
    int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) override;
    int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) override;
    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;
    int32_t SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type) override;
    void SetSpeed(uint32_t sessionId, float speed) override;

    // interfaces for render effect
    void InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectChainManagerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList) override;
    void SetOutputDeviceSink(int32_t device, const std::string &sinkName) override;
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;
    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) override;
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;
    int32_t EffectRotationUpdate(const uint32_t rotationState) override;
    int32_t SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume) override;
    int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    void InitHdiState() override;
    void UpdateEffectBtOffloadSupported(const bool &isSupported) override;
    void UpdateParamExtra(const std::string &mainkey, const std::string &subkey, const std::string &value) override;
    // interfaces for capture effect
    void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList) override;
    int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice) override;
    int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol) override;
    int32_t SetMicrophoneMuteInfo(const bool &isMute) override;
    int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol) override;
    int32_t SetAudioEnhanceProperty(
        const AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(
        AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t SetAudioEnhanceProperty(
        const AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(
        AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    void UpdateExtraSceneType(
        const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType) override;
    void NotifySettingsDataReady() override;
    void NotifyAccountsChanged() override;
    bool IsAcousticEchoCancelerSupported(SourceType sourceType) override;
    bool SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params) override;
    bool GetEffectLiveParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    int32_t UpdateCollaborativeState(bool isCollaborationEnabled) override;
    void AddStreamVolumeToEffect(const std::string stringSessionID, const float streamVolume) override;
    void DeleteStreamVolumeToEffect(const std::string stringSessionID) override;
private:
    std::shared_ptr<HpaeManager> manager_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // HPAE_MANAGER_IMPL_H