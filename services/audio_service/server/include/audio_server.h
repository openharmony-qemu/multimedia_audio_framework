/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <condition_variable>
#include <pthread.h>
#include <unordered_map>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "system_ability.h"

#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#ifdef SUPPORT_OLD_ENGINE
#include "audio_server_dump.h"
#endif
#include "i_audio_server_hpae_dump.h"
#include "audio_system_manager.h"
#include "audio_inner_call.h"
#include "common/hdi_adapter_info.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "audio_effect_server.h"
#include "audio_asr.h"
#include "policy_handler.h"
#include "audio_resource_service.h"
#include "audio_stream_monitor.h"
#include "standard_audio_service_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioServer;
class ProxyDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    ProxyDeathRecipient(int32_t pid, AudioServer *audioServer) : pid_(pid), audioServer_(audioServer) {};
    virtual ~ProxyDeathRecipient() = default;
    // overridde for DeathRecipient
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    int32_t pid_ = 0;
    AudioServer *audioServer_ = nullptr;
};

class PipeInfoGuard {
public:
    PipeInfoGuard(uint32_t sessionId);
    ~PipeInfoGuard(); // Checks the flag and calls ReleaseClient if needed
    void SetReleaseFlag(bool flag);
private:
    bool releaseFlag_ = true; // Determines whether to release pipe info in policy
    uint32_t sessionId_ = 0;
};

class AudioServer : public SystemAbility, public StandardAudioServiceStub,
    public IAudioSinkCallback, IAudioSourceCallback,
    public IAudioServerInnerCall, public DataTransferStateChangeCallbackForMonitor {
    DECLARE_SYSTEM_ABILITY(AudioServer);
public:
    DISALLOW_COPY_AND_MOVE(AudioServer);
    explicit AudioServer(int32_t systemAbilityId, bool runOnCreate = true);
    virtual ~AudioServer() = default;
    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t LoadAudioEffectLibraries(const std::vector<Library>& libraries, const std::vector<Effect>& effects,
        std::vector<Effect>& successEffectList, bool& hasEffectsLoaded) override;
    int32_t CreatePlaybackCapturerManager(bool& isSuccess) override;
    int32_t CreateEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam) override;
    int32_t SetOutputDeviceSink(int32_t deviceType, const std::string &sinkName) override;
    int32_t SetActiveOutputDevice(int32_t deviceType) override;
    int32_t SetMicrophoneMute(bool isMute) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t OffloadSetVolume(float volume, const std::string &deviceClass, const std::string &networkId) override;
    int32_t SetAudioScene(int32_t audioScene, int32_t a2dpOffloadFlag, bool scoExcludeFlag) override;
    static void *paDaemonThread(void *arg);
    int32_t SetExtraParameters(const std::string& key,
        const std::vector<StringPair>& kvpairs) override;
    int32_t SetAudioParameter(const std::string& key, const std::string& value) override;
    int32_t SetAudioParameter(const std::string& networkId, int32_t key, const std::string& condition,
        const std::string& value) override;
    int32_t GetExtraParameters(const std::string &mainKey, const std::vector<std::string> &subKeys,
        std::vector<StringPair> &result) override;
    int32_t GetAudioParameter(const std::string &key, std::string& value) override;
    int32_t GetAudioParameter(const std::string& networkId, int32_t key,
        const std::string& condition, std::string& value) override;
    int32_t GetTransactionId(int32_t deviceType, int32_t deviceRole, uint64_t& transactionId) override;
    int32_t UpdateActiveDeviceRoute(int32_t type, int32_t flag, int32_t a2dpOffloadFlag) override;
    int32_t UpdateActiveDevicesRoute(const std::vector<IntPair> &activeDevices,
        int32_t a2dpOffloadFlag, const std::string &deviceName) override;
    int32_t SetDmDeviceType(uint16_t dmDeviceType, int32_t deviceType) override;
    int32_t UpdateDualToneState(bool enable, int32_t sessionId, const std::string &dupSinkName) override;
    int32_t SetAudioMonoState(bool audioMono) override;
    int32_t SetAudioBalanceValue(float audioBalance) override;
    int32_t SuspendRenderSink(const std::string &sinkName) override;
    int32_t RestoreRenderSink(const std::string &sinkName) override;

    int32_t SetAsrAecMode(int32_t asrAecMode) override;
    int32_t GetAsrAecMode(int32_t &asrAecMode) override;
    int32_t SetAsrNoiseSuppressionMode(int32_t asrNoiseSuppressionMode) override;
    int32_t GetAsrNoiseSuppressionMode(int32_t &asrNoiseSuppressionMode) override;
    int32_t SetAsrWhisperDetectionMode(int32_t asrWhisperDetectionMode) override;
    int32_t GetAsrWhisperDetectionMode(int32_t &asrWhisperDetectionMode) override;
    int32_t SetAsrVoiceControlMode(int32_t asrVoiceControlMode, bool on) override;
    int32_t SetAsrVoiceMuteMode(int32_t asrVoiceMuteMode, bool on) override;
    int32_t IsWhispering(int32_t& whisperRes) override;
    // for effect V3
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        int32_t deviceType) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray,
        int32_t deviceType) override;
    // for effect
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    // for enhance
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        int32_t deviceType) override;
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        int32_t deviceType) override;

    int32_t NotifyDeviceInfo(const std::string &networkId, bool connected) override;

    int32_t CheckRemoteDeviceState(const std::string& networkId, int32_t deviceRole, bool isStartDevice) override;

    int32_t CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode,
        const AudioPlaybackCaptureConfig &filterConfig, sptr<IRemoteObject>& client) override;

    // ISinkParameterCallback
    void OnRenderSinkParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    // IAudioSourceCallback
    void OnWakeupClose() override;
    void OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override;

    int32_t SetParameterCallback(const sptr<IRemoteObject>& object) override;

    int32_t RegiestPolicyProvider(const sptr<IRemoteObject> &object) override;

    int32_t RegistCoreServiceProvider(const sptr<IRemoteObject> &object) override;

    int32_t SetWakeupSourceCallback(const sptr<IRemoteObject>& object) override;

    int32_t UpdateSpatializationState(const AudioSpatializationState& spatializationState) override;

    int32_t UpdateSpatialDeviceType(int32_t spatialDeviceType) override;

    int32_t NotifyStreamVolumeChanged(int32_t streamType, float volume) override;

    int32_t SetSpatializationSceneType(int32_t spatializationSceneType) override;

    int32_t ResetRouteForDisconnect(int32_t type) override;

    int32_t GetEffectLatency(const std::string &sessionId, uint32_t& latency) override;

    int32_t GetMaxAmplitude(bool isOutputDevice, const std::string& deviceClass, int32_t sourceType,
        float& maxAmplitude) override;

    int32_t GetVolumeDataCount(const std::string &sinkName, int64_t &volumeDataCount) override;

    int32_t UpdateLatencyTimestamp(const std::string &timestamp, bool isRenderer) override;

    int32_t GetEffectOffloadEnabled(bool& isEffectOffloadEnabled) override;

    void OnCapturerState(bool isActive, size_t preNum, size_t curNum);

    // IAudioServerInnerCall
    int32_t SetSinkRenderEmpty(const std::string &devceClass, int32_t durationUs) final;

    int32_t SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute) override;

    int32_t LoadHdiEffectModel() override;

    int32_t UpdateEffectBtOffloadSupported(bool isSupported) override;

    int32_t RestoreSession(uint32_t sessionID, const RestoreInfoIpc& restoreInfoIpc) override;

    int32_t SetRotationToEffect(uint32_t rotate) override;

    int32_t UpdateSessionConnectionState(int32_t sessionID, int32_t state) override;

    int32_t SetNonInterruptMute(uint32_t sessionId, bool muteFlag) override;

    int32_t SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack) override;

    int32_t UnsetOffloadMode(uint32_t sessionId) override;

    void OnRenderSinkStateChange(uint32_t sinkId, bool started) override;

    int32_t CheckHibernateState(bool hibernate) override;

    int32_t CreateIpcOfflineStream(int32_t &errorCode, sptr<IRemoteObject>& client) override;

    int32_t GetOfflineAudioEffectChains(std::vector<std::string> &effectChains) override;

    int32_t SetForegroundList(const std::vector<std::string>& list) override;

    int32_t SetRenderWhitelist(const std::vector<std::string>& list) override;

    int32_t GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime) override;

    int32_t GenerateSessionId(uint32_t &sessionId) override;

    int32_t NotifyAccountsChanged() override;

    int32_t NotifySettingsDataReady() override;

    int32_t GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) override;

    int32_t SetDefaultAdapterEnable(bool isEnable) override;

    int32_t NotifyAudioPolicyReady() override;

    int32_t SetInnerCapLimit(uint32_t innerCapLimit) override;
    int32_t CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId) override;
    int32_t ReleaseCaptureLimit(int32_t innerCapId) override;

    int32_t LoadHdiAdapter(uint32_t devMgrType, const std::string &adapterName) override;
    int32_t UnloadHdiAdapter(uint32_t devMgrType, const std::string &adapterName, bool force) override;
    int32_t CreateHdiSinkPort(const std::string &deviceClass, const std::string &idInfo,
        const IAudioSinkAttr &attr, uint32_t& renderId) override;
    int32_t CreateSinkPort(uint32_t idBase, uint32_t idType, const std::string &idInfo,
        const IAudioSinkAttr &attr, uint32_t& renderId) override;
    int32_t CreateHdiSourcePort(const std::string &deviceClass, const std::string &idInfo,
        const IAudioSourceAttr &attr, uint32_t& captureId) override;
    int32_t CreateSourcePort(uint32_t idBase, uint32_t idType, const std::string &idInfo,
        const IAudioSourceAttr &attr, uint32_t& captureId) override;
    int32_t DestroyHdiPort(uint32_t id) override;
    int32_t SetDeviceConnectedFlag(bool flag) override;
    int32_t IsAcousticEchoCancelerSupported(int32_t sourceType, bool& isSupported) override;
    int32_t SetKaraokeParameters(const std::string &parameters, bool &ret) override;
    int32_t IsAudioLoopbackSupported(int32_t mode, bool &isSupported) override;
    int32_t SetSessionMuteState(uint32_t sessionId, bool insert, bool muteFlag) override;
    int32_t SetLatestMuteState(uint32_t sessionId, bool muteFlag) override;
    int32_t ForceStopAudioStream(int32_t audioType) override;

    void RemoveRendererDataTransferCallback(const int32_t &pid);
    int32_t RegisterDataTransferCallback(const sptr<IRemoteObject> &object) override;
    int32_t RegisterDataTransferMonitorParam(int32_t callbackId,
        const DataTransferMonitorParam &param) override;
    int32_t UnregisterDataTransferMonitorParam(int32_t callbackId) override;
    void OnDataTransferStateChange(const int32_t &pid, const int32_t &callbackId,
        const AudioRendererDataTransferStateChangeInfo &info) override;
    void OnMuteStateChange(const int32_t &pid, const int32_t &callbackId,
        const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override;
    int32_t SetBtHdiInvalidState() override;
    int32_t GetVolumeBySessionId(uint32_t sessionId, float &volume) override;

    int32_t AddCaptureInjector(uint32_t sinkPortidx, std::string &rate, std::string &format,
        std::string &channels, std::string &bufferSize) override;
    int32_t RemoveCaptureInjector(uint32_t sinkPortidx) override;
protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
#ifdef HAS_FEATURE_INNERCAPTURER
    bool HandleCheckCaptureLimit(AudioProcessConfig &resetConfig,
        const AudioPlaybackCaptureConfig &filterConfig);
    int32_t InnerCheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId);
#endif
    int32_t GetAudioEnhancePropertyArray(AudioEffectPropertyArrayV3 &propertyArray,
        const DeviceType& deviceType);
    int32_t GetAudioEffectPropertyArray(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEffectChainProperty(const AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEnhanceChainProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        const DeviceType& deviceType);

    void SetA2dpAudioParameter(const std::string &renderValue);
    bool VerifyClientPermission(const std::string &permissionName,
        Security::AccessToken::AccessTokenID tokenId = Security::AccessToken::INVALID_TOKENID);
    bool PermissionChecker(const AudioProcessConfig &config);
    bool CheckPlaybackPermission(const AudioProcessConfig &config);
    int32_t CheckInnerRecorderPermission(const AudioProcessConfig &config);
    bool CheckRecorderPermission(const AudioProcessConfig &config);
    bool HandleCheckRecorderBackgroundCapture(const AudioProcessConfig &config);
    bool CheckVoiceCallRecorderPermission(Security::AccessToken::AccessTokenID tokenId);

    void ResetRecordConfig(AudioProcessConfig &config);
    AudioProcessConfig ResetProcessConfig(const AudioProcessConfig &config);
    bool CheckStreamInfoFormat(const AudioProcessConfig &config);
    bool CheckRendererFormat(const AudioProcessConfig &config);
    bool CheckRecorderFormat(const AudioProcessConfig &config);
    bool CheckConfigFormat(const AudioProcessConfig &config);
    int32_t GetHapBuildApiVersion(int32_t callerUid);

    void AudioServerDied(pid_t pid, pid_t uid);
    void RegisterPolicyServerDeathRecipient();
    void RegisterAudioCapturerSourceCallback();
    void RegisterAudioRendererSinkCallback();
    void RegisterDataTransferStateChangeCallback();

    int32_t SetIORoutes(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName = "");
    int32_t SetIORoutes(DeviceType type, DeviceFlag flag, std::vector<DeviceType> deviceTypes,
        BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName = "");
    const std::string GetDPParameter(const std::string &condition);
    const std::string GetUsbParameter(const std::string &condition);
    void WriteServiceStartupError();
    void ParseAudioParameter();
    bool CacheExtraParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs);
    bool IsNormalIpcStream(const AudioProcessConfig &config) const;
    void RecognizeAudioEffectType(const std::string &mainkey, const std::string &subkey,
        const std::string &extraSceneType);
    bool IsFastBlocked(int32_t uid, PlayerType playerType);
    int32_t SetVolumeInfoForEnhanceChain(const AudioStreamType &streamType);
    int32_t SetMicrophoneMuteForEnhanceChain(const bool &isMute);
    void InitMaxRendererStreamCntPerUid();
    bool IsSatellite(const AudioProcessConfig &config, int32_t callingUid);
    void SendCreateErrorInfo(const AudioProcessConfig &config, int32_t errorCode);
    int32_t CheckMaxRendererInstances();
    int32_t CheckMaxLoopbackInstances(AudioMode audioMode);
    bool SetPcmDumpParameter(const std::vector<std::pair<std::string, std::string>> &params);
    bool GetPcmDumpParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result);
    sptr<IRemoteObject> CreateAudioStream(const AudioProcessConfig &config, int32_t callingUid,
        std::shared_ptr<PipeInfoGuard> &pipeInfoGuard);
    int32_t SetAsrVoiceSuppressionControlMode(const AudioParamKey paramKey, AsrVoiceControlMode asrVoiceControlMode,
        bool on, int32_t modifyVolume);
    int32_t CheckAndWaitAudioPolicyReady();
    void NotifyProcessStatus();
    bool ProcessKeyValuePairs(const std::string &key, const std::vector<std::pair<std::string, std::string>> &kvpairs,
        const std::unordered_map<std::string, std::set<std::string>> &subKeyMap, std::string &value);
    bool SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params);
    bool GetEffectLiveParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result);
    int32_t CreateAudioWorkgroup(const sptr<IRemoteObject> &object, int32_t &workgroupId) override;
    int32_t ReleaseAudioWorkgroup(int32_t workgroupId) override;
    int32_t AddThreadToGroup(int32_t workgroupId, int32_t tokenId) override;
    int32_t RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId) override;
    int32_t StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime) override;
    int32_t StopGroup(int32_t workgroupId) override;

    const std::string GetAudioParameterInner(const std::string &key);
    const std::string GetAudioParameterInner(const std::string& networkId, const AudioParamKey key,
        const std::string& condition);
    bool UpdateAudioParameterInfo(const std::string &key, const std::string &value,
        AudioParamKey &parmKey, std::string &valueNew, std::string &halName);
    const std::string GetVAParameter(const std::string &key);
    int32_t SetAudioSceneInner(AudioScene audioScene, BluetoothOffloadState a2dpOffloadFlag, bool scoExcludeFlag);
    sptr<IRemoteObject> CreateAudioProcessInner(const AudioProcessConfig &config, int32_t &errorCode,
        const AudioPlaybackCaptureConfig &filterConfig);
    int32_t GetExtraParametersInner(const std::string &mainKey,
        const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result);
    int32_t ImproveAudioWorkgroupPrio(const std::unordered_map<int32_t, bool> &threads) override;
    int32_t RestoreAudioWorkgroupPrio(const std::unordered_map<int32_t, int32_t> &threads) override;
    int32_t GetPrivacyTypeAudioServer(uint32_t sessionId, int32_t &privacyType, int32_t &ret) override;
private:
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t VASSISTANT_UID = 3001;
    static constexpr int32_t MAX_VOLUME = 15;
    static constexpr int32_t MIN_VOLUME = 0;
    static constexpr int32_t ROOT_UID = 0;
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

    std::atomic<bool> isAudioParameterParsed_ = false;
    std::mutex audioParameterCacheMutex_;
    std::vector<std::pair<std::string,
        std::vector<std::pair<std::string, std::string>>>> audioExtraParameterCacheVector_;

    bool isFastControlled_ = true;
    int32_t maxRendererStreamCntPerUid_ = 0;
    std::mutex streamLifeCycleMutex_ {};
    // Temporary resolution to avoid pcm driver problem
    std::map<std::string, std::string> usbInfoMap_;
    std::mutex mtxGetUsbParameter_;

    std::atomic<bool> isAudioPolicyReady_ = false;
    std::mutex isAudioPolicyReadyMutex_;
    std::condition_variable isAudioPolicyReadyCv_;

    int32_t waitCreateStreamInServerCount_ = 0;
    std::mutex hpaeDumpMutex_;
    std::shared_ptr<IAudioServerHpaeDump> hpaeDumpObj_ = nullptr;

    std::mutex audioDataTransferMutex_;
    std::map<int32_t, std::shared_ptr<DataTransferStateChangeCallbackInner>> audioDataTransferCbMap_;

    std::mutex setA2dpParamMutex_;
};

class DataTransferStateChangeCallbackInnerImpl : public DataTransferStateChangeCallbackInner {
public:
    DataTransferStateChangeCallbackInnerImpl() = default;
    virtual ~DataTransferStateChangeCallbackInnerImpl() = default;
    void OnDataTransferStateChange(const int32_t &callbackId,
        const AudioRendererDataTransferStateChangeInfo &info) override;
    void OnMuteStateChange(const int32_t &callbackId, const int32_t &uid,
        const uint32_t &sessionId, const bool &isMuted) override {}
    void SetDataTransferMonitorParam(const DataTransferMonitorParam &param);
private:
    void ReportEvent(const AudioRendererDataTransferStateChangeInfo &info);

    DataTransferMonitorParam param_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SERVER_H
