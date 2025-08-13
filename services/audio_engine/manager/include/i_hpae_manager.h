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
#ifndef IHPAE_MANAGER_H
#define IHPAE_MANAGER_H
#include "audio_module_info.h"
#include "hpae_info.h"
#include "i_capturer_stream.h"
#include "i_renderer_stream.h"
#include "audio_service_hpae_callback.h"
#include "audio_service_hpae_dump_callback.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class IHpaeManager {
public:
    virtual ~IHpaeManager() = default;

    static IHpaeManager& GetHpaeManager();

    virtual int32_t Init() = 0;
    virtual int32_t DeInit() = 0;
    virtual int32_t RegisterSerivceCallback(const std::weak_ptr<AudioServiceHpaeCallback> &callback) = 0;
    virtual int32_t RegisterHpaeDumpCallback(const std::weak_ptr<AudioServiceHpaeDumpCallback> &callback) = 0;
    virtual void DumpSinkInfo(std::string deviceName) = 0;
    virtual void DumpSourceInfo(std::string deviceName) = 0;
    virtual void DumpAllAvailableDevice(HpaeDeviceInfo &devicesInfo) = 0;
    virtual void DumpSinkInputsInfo() = 0;
    virtual void DumpSourceOutputsInfo() = 0;
    virtual uint32_t OpenAudioPort(const AudioModuleInfo &audioModuleInfo) = 0;
    virtual int32_t CloseAudioPort(int32_t audioHandleIndex) = 0;
    virtual uint32_t ReloadAudioPort(const AudioModuleInfo &audioModuleInfo) = 0;
    virtual int32_t GetSinkInfoByIdx(const int32_t &sinkIdx,
        std::function<void(const HpaeSinkInfo &sinkInfo, int32_t result)> callback) = 0;
    virtual int32_t GetSourceInfoByIdx(const int32_t &sourceIdx,
        std::function<void(const HpaeSourceInfo &sourceInfo, int32_t result)> callback) = 0;

    virtual int32_t SetDefaultSink(std::string name) = 0;
    virtual int32_t SetDefaultSource(std::string name) = 0;
    virtual int32_t GetAllSinkInputs() = 0;
    virtual int32_t GetAllSourceOutputs() = 0;
    virtual int32_t MoveSourceOutputByIndexOrName(
        uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName) = 0;
    virtual int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) = 0;
    virtual void HandleMsg() = 0;
    virtual bool IsInit() = 0;
    virtual bool IsRunning() = 0;
    virtual bool IsMsgProcessing() = 0;
    virtual int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) = 0;
    virtual bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) = 0;
    virtual int32_t SetSourceOutputMute(int32_t uid, bool setMute) = 0;
    virtual int32_t GetAllSinks() = 0;

    virtual int32_t CreateStream(const HpaeStreamInfo &streamInfo) = 0;
    virtual int32_t DestroyStream(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t Start(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t StartWithSyncId(HpaeStreamClassType streamClassType, uint32_t sessionId, int32_t syncId) = 0;
    virtual int32_t Pause(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t Flush(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t Drain(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t Stop(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t Release(HpaeStreamClassType streamClassType, uint32_t sessionId) = 0;
    virtual int32_t RegisterStatusCallback(HpaeStreamClassType streamClassType, uint32_t sessionId,
        const std::weak_ptr<IStreamStatusCallback> &callback) = 0;

    virtual int32_t RegisterReadCallback(uint32_t sessionId,
        const std::weak_ptr<ICapturerStreamCallback> &callback) = 0;
    virtual int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeStreamInfo &streamInfo) = 0;

    virtual int32_t SetClientVolume(uint32_t sessionId, float volume) = 0;
    virtual int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) = 0;
    virtual int32_t SetRate(uint32_t sessionId, int32_t rate) = 0;
    virtual int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) = 0;
    virtual int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) = 0;
    virtual int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) = 0;
    virtual int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) = 0;
    virtual int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) = 0;

    virtual int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) = 0;
    virtual size_t GetWritableSize(uint32_t sessionId) = 0;
    virtual int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) = 0;
    virtual int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) = 0;
    virtual int32_t SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type) = 0;
    virtual void SetSpeed(uint32_t sessionId, float speed) = 0;

    // interfaces for render effect
    virtual void InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectChainManagerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList) = 0;
    virtual void SetOutputDeviceSink(int32_t device, const std::string &sinkName) = 0;
    virtual int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) = 0;
    virtual int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) = 0;
    virtual int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) = 0;
    virtual int32_t EffectRotationUpdate(const uint32_t rotationState) = 0;
    virtual int32_t SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume) = 0;
    virtual int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) = 0;
    virtual int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray) = 0;
    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) = 0;
    virtual int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) = 0;
    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) = 0;
    virtual void InitHdiState() = 0;
    virtual void UpdateEffectBtOffloadSupported(const bool &isSupported) = 0;
    virtual void UpdateParamExtra(const std::string &mainkey, const std::string &subkey, const std::string &value) = 0;
    // interfaces for capture effect
    virtual void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList) = 0;
    virtual int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice) = 0;
    virtual int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol) = 0;
    virtual int32_t SetMicrophoneMuteInfo(const bool &isMute) = 0;
    virtual int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol) = 0;
    virtual int32_t SetAudioEnhanceProperty(
        const AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;
    virtual int32_t GetAudioEnhanceProperty(
        AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;
    virtual int32_t SetAudioEnhanceProperty(
        const AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;
    virtual int32_t GetAudioEnhanceProperty(
        AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;
    virtual void UpdateExtraSceneType(
        const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType) = 0;
    virtual void NotifySettingsDataReady() = 0;
    virtual void NotifyAccountsChanged() = 0;
    virtual bool IsAcousticEchoCancelerSupported(SourceType sourceType) = 0;
    virtual bool SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params) = 0;
    virtual bool GetEffectLiveParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) = 0;
    virtual int32_t UpdateCollaborativeState(bool isCollaborationEnabled) = 0;
    virtual void AddStreamVolumeToEffect(const std::string stringSessionID, const float streamVolume) = 0;
    virtual void DeleteStreamVolumeToEffect(const std::string stringSessionID) = 0;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // IHPAE_MANAGER_H