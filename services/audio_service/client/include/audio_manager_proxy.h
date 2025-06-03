/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_MANAGER_PROXY_H
#define ST_AUDIO_MANAGER_PROXY_H

#include "iremote_proxy.h"
#include "audio_system_manager.h"
#include "audio_manager_base.h"
#include "audio_asr.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerProxy : public IRemoteProxy<IStandardAudioService> {
public:
    explicit AudioManagerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioManagerProxy() = default;

    int32_t SetMicrophoneMute(bool isMute) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t OffloadSetVolume(float volume) override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeOutputDevices,
        DeviceType activeInputDevice, BluetoothOffloadState a2dpOffloadFlag) override;
    const std::string GetAudioParameter(const std::string &key) override;
    const std::string GetAudioParameter(const std::string& networkId, const AudioParamKey key,
        const std::string& condition) override;
    int32_t GetExtraParameters(const std::string &mainKey, const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    int32_t SuspendRenderSink(const std::string &sinkName) override;
    int32_t RestoreRenderSink(const std::string &sinkName) override;
    void SetAudioParameter(const std::string &key, const std::string &value) override;
    void SetAudioParameter(const std::string& networkId, const AudioParamKey key, const std::string& condition,
        const std::string& value) override;
    int32_t SetExtraParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs) override;
    int32_t UpdateActiveDeviceRoute(DeviceType type, DeviceFlag flag, BluetoothOffloadState a2dpOffloadFlag) override;
    int32_t UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName = "") override;
    int32_t UpdateDualToneState(bool enable, int32_t sessionId) override;
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole) override;
    void NotifyDeviceInfo(std::string networkId, bool connected) override;
    int32_t CheckRemoteDeviceState(std::string networkId, DeviceRole deviceRole, bool isStartDevice) override;
    int32_t SetParameterCallback(const sptr<IRemoteObject>& object) override;
    int32_t SetWakeupSourceCallback(const sptr<IRemoteObject>& object) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    sptr<IRemoteObject> CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode,
        const AudioPlaybackCaptureConfig &filterConfig = AudioPlaybackCaptureConfig()) override;
    bool LoadAudioEffectLibraries(const std::vector<Library> libraries, const std::vector<Effect> effects,
        std::vector<Effect> &successEffects) override;
    bool CreateEffectChainManager(std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam) override;
    void SetOutputDeviceSink(int32_t deviceType, std::string &sinkName) override;
    void SetActiveOutputDevice(DeviceType deviceType) override;
    bool CreatePlaybackCapturerManager() override;
    int32_t RegiestPolicyProvider(const sptr<IRemoteObject> &object) override;
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;
    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) override;
    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume) override;
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;
    int32_t ResetRouteForDisconnect(DeviceType type) override;
    uint32_t GetEffectLatency(const std::string &sessionId) override;
    float GetMaxAmplitude(bool isOutputDevice, std::string deviceClass, SourceType sourceType) override;
    void ResetAudioEndpoint() override;
    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;
    int32_t SetAsrAecMode(AsrAecMode asrAecMode) override;
    int32_t GetAsrAecMode(AsrAecMode &asrAecMode) override;
    int32_t SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode asrNoiseSuppressionMode) override;
    int32_t GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode) override;
    int32_t SetAsrWhisperDetectionMode(AsrWhisperDetectionMode asrWhisperDetectionMode) override;
    int32_t GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode) override;
    int32_t SetAsrVoiceControlMode(AsrVoiceControlMode asrVoiceControlMode, bool on) override;
    int32_t SetAsrVoiceMuteMode(AsrVoiceMuteMode asrVoiceMuteMode, bool on) override;
    int32_t IsWhispering() override;
    bool GetEffectOffloadEnabled() override;
    void LoadHdiEffectModel() override;
    // for effect V3
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        const DeviceType& deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray,
        const DeviceType& deviceType = DEVICE_TYPE_NONE) override;
    // for effect
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    // for enhance
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;

    void UpdateEffectBtOffloadSupported(const bool &isSupported) override;
    int32_t SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute) override;
    void SetRotationToEffect(const uint32_t rotate) override;
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state) override;
    void SetNonInterruptMute(const uint32_t sessionId, const bool muteFlag) override;
    int32_t SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadMode(uint32_t sessionId) override;
    void CheckHibernateState(bool onHibernate) override;
    void RestoreSession(const uint32_t &sessionID, RestoreInfo restoreInfo) override;
    sptr<IRemoteObject> CreateIpcOfflineStream(int32_t &errorCode) override;
    int32_t GetOfflineAudioEffectChains(std::vector<std::string> &effectChains) override;
    int32_t GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime) override;
    int32_t GenerateSessionId(uint32_t &sessionId) override;
    void NotifyAccountsChanged() override;
    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) override;
    void SetDefaultAdapterEnable(bool isEnable) override;
    void NotifyAudioPolicyReady() override;
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t SetInnerCapLimit(uint32_t innerCapLimit) override;
    int32_t CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId) override;
    int32_t ReleaseCaptureLimit(int32_t innerCapId) override;
#endif
    int32_t LoadHdiAdapter(uint32_t devMgrType, const std::string &adapterName) override;
    void UnloadHdiAdapter(uint32_t devMgrType, const std::string &adapterName, bool force) override;
    void SetDeviceConnectedFlag(bool flag) override;
    void NotifySettingsDataReady() override;
private:
    static inline BrokerDelegator<AudioManagerProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_MANAGER_PROXY_H
