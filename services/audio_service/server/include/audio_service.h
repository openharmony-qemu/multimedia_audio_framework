/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

#include <condition_variable>
#include <sstream>
#include <set>
#include <map>
#include <mutex>
#include <vector>

#ifdef SUPPORT_LOW_LATENCY
#include "audio_process_in_server.h"
#include "audio_endpoint.h"
#endif

#include "i_audio_process_stream.h"
#include "audio_info.h"
#include "audio_device_descriptor.h"
#include "ipc_stream_in_server.h"
#include "playback_capturer_filter_listener.h"

namespace OHOS {
namespace AudioStandard {
namespace {
enum InnerCapFilterPolicy : uint32_t {
    POLICY_INVALID = 0,
    POLICY_USAGES_ONLY,
    POLICY_USAGES_AND_PIDS
};

enum ReuseEndpointType : uint32_t {
    CREATE_ENDPOINT = 0,
    RECREATE_ENDPOINT,
    REUSE_ENDPOINT,
};
} // anonymous namespace

using MuteStateChangeCallbck = std::function<void(bool)>;

#ifdef SUPPORT_LOW_LATENCY
class AudioService : public ProcessReleaseCallback, public ICapturerFilterListener
#else
class AudioService : public ICapturerFilterListener
#endif
{
public:
    static AudioService *GetInstance();
    ~AudioService();

    // override for ICapturerFilterListener
    int32_t OnCapturerFilterChange(uint32_t sessionId, const AudioPlaybackCaptureConfig &newConfig,
        int32_t innerCapId) override;
    int32_t OnCapturerFilterRemove(uint32_t sessionId, int32_t innerCapId) override;
    void InitAllDupBuffer(int32_t innerCapId) override;

    void SaveForegroundList(std::vector<std::string> list);
    // if match, keep uid for speed up, used in create process.
    bool MatchForegroundList(const std::string &bundleName, uint32_t uid);
    // used in start process.
    bool InForegroundList(uint32_t uid);
    bool UpdateForegroundState(uint32_t appTokenId, bool isActive);
    void DumpForegroundList(std::string &dumpString);
    void SaveRenderWhitelist(std::vector<std::string> list);
    bool InRenderWhitelist(const std::string bundleName);

    int32_t GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime);
    sptr<IpcStreamInServer> GetIpcStream(const AudioProcessConfig &config, int32_t &ret);
    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume);

#ifdef SUPPORT_LOW_LATENCY
    sptr<AudioProcessInServer> GetAudioProcess(const AudioProcessConfig &config);
    // override for ProcessReleaseCallback, do release process work.
    int32_t OnProcessRelease(IAudioProcessStream *process, bool isSwitchStream = false) override;
    void ReleaseProcess(const std::string endpointName, const int32_t delayTime);

    void CheckBeforeRecordEndpointCreate(bool isRecord);
    AudioDeviceDescriptor GetDeviceInfoForProcess(const AudioProcessConfig &config,
        AudioStreamInfo &streamInfo, bool isReloadProcess = false);
    std::shared_ptr<AudioEndpoint> GetAudioEndpointForDevice(AudioDeviceDescriptor &deviceInfo,
        const AudioProcessConfig &clientConfig, AudioStreamInfo &streamInfo, bool isVoipStream);

    int32_t LinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    int32_t UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    std::shared_ptr<AudioEndpoint> GetEndPointByType(AudioEndpoint::EndpointType type);
#endif

    void Dump(std::string &dumpString);
    float GetMaxAmplitude(bool isOutputDevice);

    void RemoveRenderer(uint32_t sessionId, bool isSwitchStream = false);
    void RemoveCapturer(uint32_t sessionId, bool isSwitchStream = false);
    int32_t EnableDualStream(const uint32_t sessionId, const std::string &dupSinkName);
    int32_t DisableDualStream(const uint32_t sessionId);
    int32_t SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack);
    int32_t UnsetOffloadMode(uint32_t sessionId);
    void UpdateAudioSinkState(uint32_t sinkId, bool started);
    void CheckHibernateState(bool onHibernate);
    bool GetHibernateState();
    std::shared_ptr<RendererInServer> GetRendererBySessionID(const uint32_t &session);
    std::shared_ptr<CapturerInServer> GetCapturerBySessionID(const uint32_t &session);
    void SetNonInterruptMute(const uint32_t SessionId, const bool muteFlag);
    void SetNonInterruptMuteForProcess(const uint32_t SessionId, const bool muteFlag);
    void UpdateMuteControlSet(uint32_t sessionId, bool muteFlag);
    int32_t UpdateSourceType(SourceType sourceType);
    void SetIncMaxRendererStreamCnt(AudioMode audioMode);
    int32_t GetCurrentRendererStreamCnt();
    void SetDecMaxRendererStreamCnt();
    int32_t GetCurrentLoopbackStreamCnt(AudioMode audioMode);
    void SetIncMaxLoopbackStreamCnt(AudioMode audioMode);
    void SetDecMaxLoopbackStreamCnt(AudioMode audioMode);
    void DisableLoopback();
    bool IsExceedingMaxStreamCntPerUid(int32_t callingUid, int32_t appUid, int32_t maxStreamCntPerUid);
    void GetCreatedAudioStreamMostUid(int32_t &mostAppUid, int32_t &mostAppNum);
    void CleanAppUseNumMap(int32_t appUid);
    bool HasBluetoothEndpoint();
    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs);
    void SetDefaultAdapterEnable(bool isEnable);
    bool GetDefaultAdapterEnable();
    RestoreStatus RestoreSession(uint32_t sessionId, RestoreInfo restoreInfo);
    int32_t ForceStopAudioStream(StopAudioType audioType);
    void SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string adjustTime, uint32_t code);
    void RegisterMuteStateChangeCallback(uint32_t sessionId, const MuteStateChangeCallbck &callback);
    void SetSessionMuteState(const uint32_t sessionId, const bool insert, const bool muteFlag);
    void SetLatestMuteState(const uint32_t sessionId, const bool muteFlag);
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t UnloadModernInnerCapSink(int32_t innerCapId);
    int32_t UnloadModernOffloadCapSource();
#endif
    void RenderersCheckForAudioWorkgroup(int32_t pid);
    int32_t GetPrivacyType(const uint32_t sessionId, AudioPrivacyType &privacyType);
private:
    AudioService();
    void DelayCallReleaseEndpoint(std::string endpointName);
    bool IsSameAudioStreamInfoNotIncludeSample(AudioStreamInfo &newStreamInfo, AudioStreamInfo &oldStreamInfo);
    ReuseEndpointType GetReuseEndpointType(AudioDeviceDescriptor &deviceInfo,
        const std::string &deviceKey, AudioStreamInfo &streamInfo, int32_t endpointFlag);
    void InsertRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void InsertCapturer(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer);
#ifdef HAS_FEATURE_INNERCAPTURER
    // for inner-capturer
    void CheckInnerCapForRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
#ifdef SUPPORT_LOW_LATENCY
    void CheckInnerCapForProcess(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void FilterAllFastProcess();
    int32_t CheckDisableFastInner(std::shared_ptr<AudioEndpoint> endpoint);
    int32_t HandleFastCapture(std::set<int32_t> captureIds, sptr<AudioProcessInServer> audioProcessInServer,
        std::shared_ptr<AudioEndpoint> audioEndpoint);

    void CheckFastSessionMuteState(uint32_t sessionId, sptr<AudioProcessInServer> process);
    int32_t GetReleaseDelayTime(std::shared_ptr<AudioEndpoint> endpoint, bool isSwitchStream, bool isRecord);
#endif
    InnerCapFilterPolicy GetInnerCapFilterPolicy(int32_t innerCapId);
    bool ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, int32_t innerCapId);
    bool ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, std::set<int32_t> &beCapIds);
    bool CheckShouldCap(const AudioProcessConfig &rendererConfig, int32_t innerCapId);
#endif
    bool ShouldBeDualTone(const AudioProcessConfig &config, const std::string &dupSinkName);
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t OnInitInnerCapList(int32_t innerCapId); // for first InnerCap filter take effect.
    int32_t OnUpdateInnerCapList(int32_t innerCapId); // for some InnerCap filter has already take effect.
#endif
    bool IsEndpointTypeVoip(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo);
    void RemoveIdFromMuteControlSet(uint32_t sessionId);
    void CheckRenderSessionMuteState(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void CheckCaptureSessionMuteState(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer);
    void AddFilteredRender(int32_t innerCapId, std::shared_ptr<RendererInServer> renderer);
    bool IsMuteSwitchStream(uint32_t sessionId);
    float GetSystemVolume();
    bool IsStreamTypeFitWorkgroup(AudioStreamType streamType);
    void UpdateSystemVolume(AudioStreamType streamType, float volume);
    void UpdateSessionMuteStatus(const uint32_t sessionId, const bool muteFlag);
    std::shared_ptr<RendererInServer> GetRendererInServerBySessionId(const uint32_t sessionId);
    int32_t GetPrivacyTypeForNormalStream(const uint32_t sessionId, AudioPrivacyType &privacyType);

#ifdef SUPPORT_LOW_LATENCY
    sptr<AudioProcessInServer> GetProcessInServerBySessionId(const uint32_t sessionId);
    int32_t GetPrivacyTypeForFastStream(const uint32_t sessionId, AudioPrivacyType &privacyType);
    int32_t EnableDualStreamForFastStream(const uint32_t sessionId, const std::string &dupSinkName);
    int32_t DisableDualStreamForFastStream(const uint32_t sessionId);
    std::vector<std::pair<sptr<AudioProcessInServer>, std::shared_ptr<AudioEndpoint>>> GetLinkedPairInner(
        const uint32_t sessionId);
    void HandleProcessInserverDualStreamDisableInner(AudioEndpoint &endpoint);
    void HandleProcessInserverDualStreamEnableInner(AudioEndpoint &endpoint, const std::string &dupSinkName);
#endif
    int32_t EnableDualStreamForNormalStream(const uint32_t sessionId, const std::string &dupSinkName);
    int32_t DisableDualStreamForNormalStream(const uint32_t sessionId);
private:
    std::mutex foregroundSetMutex_;
    std::set<std::string> foregroundSet_;
    std::set<uint32_t> foregroundUidSet_;
    std::mutex processListMutex_;
    std::mutex releaseEndpointMutex_;
    std::condition_variable releaseEndpointCV_;
    std::set<std::string> releasingEndpointSet_;
    std::mutex renderWhitelistMutex_;
    std::set<std::string> renderWhitelist_;

#ifdef SUPPORT_LOW_LATENCY
    std::vector<std::pair<sptr<AudioProcessInServer>, std::shared_ptr<AudioEndpoint>>> linkedPairedList_;
    std::map<std::string, std::shared_ptr<AudioEndpoint>> endpointList_;
    std::unordered_map<uint32_t, wptr<AudioProcessInServer>> allProcessInServer_;
#endif

    // for inner-capturer
    bool isRegisterCapturerFilterListened_ = false;
    bool isDefaultAdapterEnable_ = false;
    AudioPlaybackCaptureConfig workingConfig_;
    std::unordered_map<int32_t, AudioPlaybackCaptureConfig> workingConfigs_;

    std::mutex rendererMapMutex_;
    std::mutex capturerMapMutex_;
    std::mutex muteSwitchStreamSetMutex_;
    std::mutex workingConfigsMutex_;
    std::unordered_map<int32_t, std::vector<std::weak_ptr<RendererInServer>>> filteredRendererMap_ = {};
    std::map<uint32_t, std::weak_ptr<RendererInServer>> allRendererMap_ = {};
    std::map<uint32_t, std::weak_ptr<CapturerInServer>> allCapturerMap_ = {};

    std::mutex mutedSessionsMutex_;
    std::set<uint32_t> mutedSessions_ = {};
    int32_t currentRendererStreamCnt_ = 0;
    int32_t currentLoopbackRendererStreamCnt_ = 0;
    int32_t currentLoopbackCapturerStreamCnt_ = 0;
    std::mutex streamLifeCycleMutex_ {};
    std::map<int32_t, std::int32_t> appUseNumMap_;
    std::mutex allRunningSinksMutex_;
    std::condition_variable allRunningSinksCV_;
    std::set<uint32_t> allRunningSinks_;
    bool onHibernate_ = false;
    std::set<uint32_t> muteSwitchStreams_ = {};
    std::map<uint32_t, MuteStateChangeCallbck> muteStateCallbacks_{};
    std::mutex muteStateMapMutex_;
    std::map<uint32_t, bool> muteStateMap_{};
    std::mutex audioWorkGroupSystemVolumeMutex_;
    float audioWorkGroupSystemVolume_ = 0.0f;

    std::mutex dualStreamMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_H
