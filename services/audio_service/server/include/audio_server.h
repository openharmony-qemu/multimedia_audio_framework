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

#ifndef ST_AUDIO_SERVER_H
#define ST_AUDIO_SERVER_H

#include <mutex>
#include <pthread.h>
#include <unordered_map>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "system_ability.h"

#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#include "audio_server_dump.h"
#include "audio_system_manager.h"
#include "audio_inner_call.h"
#include "i_audio_renderer_sink.h"
#include "i_audio_capturer_source.h"
#include "audio_effect_server.h"
#include "audio_asr.h"

namespace OHOS {
namespace AudioStandard {
class AudioServer : public SystemAbility, public AudioManagerStub, public IAudioSinkCallback, IAudioSourceCallback,
    public IAudioServerInnerCall {
    DECLARE_SYSTEM_ABILITY(AudioServer);
public:
    DISALLOW_COPY_AND_MOVE(AudioServer);
    explicit AudioServer(int32_t systemAbilityId, bool runOnCreate = true);
    virtual ~AudioServer() = default;
    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    bool LoadAudioEffectLibraries(std::vector<Library> libraries, std::vector<Effect> effects,
        std::vector<Effect>& successEffectList) override;
    bool CreatePlaybackCapturerManager() override;
    bool CreateEffectChainManager(std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam) override;
    void SetOutputDeviceSink(int32_t deviceType, std::string &sinkName) override;
    int32_t SetMicrophoneMute(bool isMute) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t OffloadSetVolume(float volume) override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeOutputDevices,
        DeviceType activeInputDevice, BluetoothOffloadState a2dpOffloadFlag) override;
    static void *paDaemonThread(void *arg);
    int32_t SetExtraParameters(const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& kvpairs) override;
    void SetAudioParameter(const std::string& key, const std::string& value) override;
    void SetAudioParameter(const std::string& networkId, const AudioParamKey key, const std::string& condition,
        const std::string& value) override;
    bool CheckAndPrintStacktrace(const std::string &key);
    int32_t GetExtraParameters(const std::string &mainKey, const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    const std::string GetAudioParameter(const std::string &key) override;
    const std::string GetAudioParameter(const std::string& networkId, const AudioParamKey key,
        const std::string& condition) override;
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole) override;
    int32_t UpdateActiveDeviceRoute(DeviceType type, DeviceFlag flag, BluetoothOffloadState a2dpOffloadFlag) override;
    int32_t UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        BluetoothOffloadState a2dpOffloadFlag) override;
    int32_t UpdateDualToneState(bool enable, int32_t sessionId) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t SuspendRenderSink(const std::string &sinkName) override;
    int32_t RestoreRenderSink(const std::string &sinkName) override;

    int32_t SetAsrAecMode(AsrAecMode asrAecMode) override;
    int32_t GetAsrAecMode(AsrAecMode &asrAecMode) override;
    int32_t SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode asrNoiseSuppressionMode) override;
    int32_t GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode) override;
    int32_t SetAsrWhisperDetectionMode(AsrWhisperDetectionMode asrWhisperDetectionMode) override;
    int32_t GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode) override;
    int32_t SetAsrVoiceControlMode(AsrVoiceControlMode asrVoiceControlMode, bool on) override;
    int32_t SetAsrVoiceMuteMode(AsrVoiceMuteMode asrVoiceMuteMode, bool on) override;
    int32_t IsWhispering() override;

    void NotifyDeviceInfo(std::string networkId, bool connected) override;

    int32_t CheckRemoteDeviceState(std::string networkId, DeviceRole deviceRole, bool isStartDevice) override;

    sptr<IRemoteObject> CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode) override;

    // ISinkParameterCallback
    void OnAudioSinkParamChange(const std::string &netWorkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    // IAudioSourceCallback
    void OnWakeupClose() override;
    void OnAudioSourceParamChange(const std::string &netWorkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    int32_t SetParameterCallback(const sptr<IRemoteObject>& object) override;

    int32_t RegiestPolicyProvider(const sptr<IRemoteObject> &object) override;

    int32_t SetWakeupSourceCallback(const sptr<IRemoteObject>& object) override;

    void RequestThreadPriority(uint32_t tid, std::string bundleName) override;

    int32_t SetSupportStreamUsage(std::vector<int32_t> usage) override;

    int32_t SetCaptureSilentState(bool state) override;

    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;

    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) override;

    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume) override;

    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;

    int32_t ResetRouteForDisconnect(DeviceType type) override;

    uint32_t GetEffectLatency(const std::string &sessionId) override;

    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;

    float GetMaxAmplitude(bool isOutputDevice, int32_t deviceType) override;

    void OnCapturerState(bool isActive, int32_t num);

    void ResetAudioEndpoint() override;

    bool GetEffectOffloadEnabled() override;

    // IAudioServerInnerCall
    int32_t SetSinkRenderEmpty(const std::string &devceClass, int32_t durationUs) final;

    int32_t SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute) override;

    void LoadHdiEffectModel() override;

    void UpdateEffectBtOffloadSupported(const bool &isSupported) override;

    void SetRotationToEffect(const uint32_t rotate) override;

    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state) override;

    void SetNonInterruptMute(const uint32_t sessionId, const bool muteFlag) override;

    sptr<IRemoteObject> CreateIpcOfflineStream(int32_t &errorCode) override;

    int32_t GetOfflineAudioEffectChains(std::vector<std::string> &effectChains) override;
protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    bool VerifyClientPermission(const std::string &permissionName,
        Security::AccessToken::AccessTokenID tokenId = Security::AccessToken::INVALID_TOKENID);
    bool PermissionChecker(const AudioProcessConfig &config);
    bool CheckPlaybackPermission(const AudioProcessConfig &config);
    bool CheckRecorderPermission(const AudioProcessConfig &config);
    bool CheckVoiceCallRecorderPermission(Security::AccessToken::AccessTokenID tokenId);

    void ResetRecordConfig(AudioProcessConfig &config);
    AudioProcessConfig ResetProcessConfig(const AudioProcessConfig &config);
    bool CheckStreamInfoFormat(const AudioProcessConfig &config);
    bool CheckRendererFormat(const AudioProcessConfig &config);
    bool CheckRecorderFormat(const AudioProcessConfig &config);
    bool CheckConfigFormat(const AudioProcessConfig &config);
    int32_t GetHapBuildApiVersion(int32_t callerUid);

    void AudioServerDied(pid_t pid);
    void RegisterPolicyServerDeathRecipient();
    void RegisterAudioCapturerSourceCallback();
    int32_t SetIORoutes(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        BluetoothOffloadState a2dpOffloadFlag);
    int32_t SetIORoutes(DeviceType type, DeviceFlag flag, std::vector<DeviceType> deviceTypes,
        BluetoothOffloadState a2dpOffloadFlag);
    const std::string GetDPParameter(const std::string &condition);
    const std::string GetUsbParameter();
    void WriteServiceStartupError();
    bool IsNormalIpcStream(const AudioProcessConfig &config) const;
    void RecognizeAudioEffectType(const std::string &mainkey, const std::string &subkey,
        const std::string &extraSceneType);
    int32_t SetSystemVolumeToEffect(const AudioStreamType streamType, float volume);
    const std::string GetBundleNameFromUid(int32_t uid);
    bool IsFastBlocked(int32_t uid);
    void InitMaxRendererStreamCntPerUid();
    int32_t CheckParam(const AudioProcessConfig &config);
    void SendRendererCreateErrorInfo(const StreamUsage &sreamUsage,
        const int32_t &errorCode);
    int32_t CheckMaxRendererInstances();
    sptr<IRemoteObject> CreateAudioStream(const AudioProcessConfig &config, int32_t callingUid);
private:
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t VASSISTANT_UID = 3001;
    static constexpr int32_t MAX_VOLUME = 15;
    static constexpr int32_t MIN_VOLUME = 0;
    static uint32_t paDaemonTid_;
    static std::unordered_map<int, float> AudioStreamVolumeMap;
    static std::map<std::string, std::string> audioParameters;
    static std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys;

    pthread_t m_paDaemonThread;
    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;

    // Capturer status flags: each capturer is represented by a single bit.
    // 0 indicates the capturer has stopped; 1 indicates the capturer has started.
    std::atomic<uint64_t> capturerStateFlag_ = 0;

    std::shared_ptr<AudioParameterCallback> audioParamCb_;
    std::mutex onCapturerStateCbMutex_;
    std::shared_ptr<WakeUpSourceCallback> wakeupCallback_;
    std::mutex audioParamCbMtx_;
    std::mutex setWakeupCloseCallbackMutex_;
    std::mutex audioParameterMutex_;
    std::mutex audioSceneMutex_;
    std::unique_ptr<AudioEffectServer> audioEffectServer_;
    bool isFastControlled_ = false;
    int32_t maxRendererStreamCntPerUid_ = 0;
    std::mutex streamLifeCycleMutex_ {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SERVER_H
