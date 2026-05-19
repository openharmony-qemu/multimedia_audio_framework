/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_INTERRUPT_SERVICE_H
#define ST_AUDIO_INTERRUPT_SERVICE_H

#include <mutex>
#include <list>
#include <set>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <future>
#include "iremote_object.h"

#include "i_audio_interrupt_event_dispatcher.h"
#include "audio_interrupt_info.h"
#include "audio_policy_server_handler.h"
#include "audio_session_service.h"
#include "client_type_manager.h"
#include "audio_interrupt_dfx_collector.h"
#include "audio_zone_info.h"
#include "audio_interrupt_zone.h"
#include "audio_info.h"
#include "istandard_audio_service.h"
#include "async_action_handler.h"
#include "audio_interrupt_custom.h"
#include "audio_stream_collector.h"
#include "audio_interrupt_dfx.h"
#include "audio_interrupt_strategy.h"
#include "audio_debug_info.h"

namespace OHOS {
namespace AudioStandard {

struct CachedFocusInfo {
    int32_t userId;
    uint32_t sessionId;
    InterruptEventInternal interruptEvent;
    AudioInterrupt interrupt;
};

class AudioPolicyServerHandler;
class AudioPolicyServer;

class AudioInterruptService : public std::enable_shared_from_this<AudioInterruptService>,
                              public IAudioInterruptEventDispatcher {
public:
    AudioInterruptService();
    virtual ~AudioInterruptService();

    const sptr<IStandardAudioService> GetAudioServerProxy();

    // callback run in handler thread
    void DispatchInterruptEventWithStreamId(
        uint32_t streamId, InterruptEventInternal &interruptEvent) override;

    void Init(sptr<AudioPolicyServer> server);
    void AddDumpInfo(std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>> &audioInterruptZonesMapDump);
    void SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler);
    void SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> &handler);

    // interfaces of SessionTimeOutCallback
    void OnSessionTimeout(const int32_t pid);

    // interfaces for AudioSessionService
    AudioInterruptResult ActivateAudioSession(const int32_t zoneId, const int32_t callerPid,
        const AudioSessionStrategy &strategy, const bool isStandalone = false,
        const bool stateChangeCallbackFlag = false, int32_t callerUid = INVALID_UID);
    int32_t DeactivateAudioSession(const int32_t zoneId, const int32_t callerPid,
        std::function<void()> onFakeInterruptDeactivate = nullptr);
    bool IsAudioSessionActivated(const int32_t callerPid);

    bool IsOtherMediaPlaying();

    void SetRegisterInterruptCallbackFlag(const uint32_t streamId, const bool interruptCallbackFlag);

    void SetRegisterStateChangeCallbackFlag(int32_t callerPid, const bool stateChangeCallbackFlag);

    bool CheckPlaying(const AudioInterrupt &audioInterrupt);

    // deprecated interrupt interfaces
    int32_t SetAudioManagerInterruptCallback(const sptr<IRemoteObject> &object);
    int32_t UnsetAudioManagerInterruptCallback();
    int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);
    int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    // modern interrupt interfaces
    int32_t SetAudioInterruptCallback(const int32_t zoneId, const uint32_t streamId,
        const sptr<IRemoteObject> &object, uint32_t uid);
    int32_t UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t streamId);
    bool AudioInterruptIsActiveInFocusList(const int32_t zoneId, const uint32_t incomingStreamId);
    bool AudioInterruptIsStandbyInFocusList(const int32_t zoneId, const uint32_t incomingStreamId);
    AudioInterruptResult ActivateAudioInterrupt(
        const int32_t zoneId, const AudioInterrupt &audioInterrupt, const bool isUpdatedAudioStrategy = false);
    AudioInterruptResult DeactivateAudioInterrupt(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
        bool isRemoveFocusHis = false);
    bool IsCapturerFocusAvailable(int32_t zoneId, const AudioCapturerInfo &capturerInfo);
    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID);

    // preempt audio focus interfaces
    int32_t ActivatePreemptMode();
    int32_t DeactivatePreemptMode();

    // zone debug interfaces
    int32_t FindZoneByPid(int32_t pid);
    int32_t CreateAudioInterruptZone(const int32_t zoneId, const AudioZoneContext &context);
    int32_t ReleaseAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func);
    int32_t MigrateAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func);
    int32_t InjectInterruptToAudioZone(const int32_t zoneId, const AudioFocusList &interrupts);
    int32_t InjectInterruptToAudioZone(const int32_t zoneId, const std::string &deviceTag,
        const AudioFocusList &interrupts);
    int32_t GetAudioFocusInfoList(const int32_t zoneId, AudioFocusList &focusInfoList);
    int32_t GetAudioFocusInfoList(const int32_t zoneId, const std::string &deviceTag,
        AudioFocusList &focusInfoList);
    void UpdateContextForAudioZone(const int32_t zoneId, const AudioZoneContext &context);

    int32_t SetAudioFocusInfoCallback(const int32_t zoneId, const sptr<IRemoteObject> &object);
    int32_t GetStreamTypePriority(AudioStreamType streamType);
    AudioStreamType GetStreamInFocus(const int32_t zoneId);
    AudioStreamType GetStreamInFocusByUid(const int32_t uid, const int32_t zoneId);
    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneId);
    int32_t NotifyStreamStandby(int32_t pid, uint32_t streamId);
    int32_t NotifyStreamStandbyResume(int32_t pid, uint32_t streamId);
    int32_t UpdateStreamFocusStateToStandby(int32_t pid, uint32_t streamId);
    int32_t UpdateStreamFocusStateToActive(int32_t pid, uint32_t streamId);

private:
    int32_t FindAndUpdateFocusState(uint32_t streamId, AudioFocuState focuState);

public:
    void ClearAudioFocusInfoListOnAccountsChanged(const int32_t &id, const int32_t &oldId);
    int32_t ClearAudioFocusInfoList();
    void AudioInterruptZoneDump(std::string &dumpString);
    AudioScene GetHighestPriorityAudioScene(const int32_t zoneId) const;
    // for audiosessionv2
    int32_t SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene, int32_t callerUid = INVALID_UID);
    std::set<int32_t> GetStreamIdsForAudioSessionByStreamUsage(
        const int32_t zoneId, const std::set<StreamUsage> &streamUsageSet);
    std::set<int32_t> GetStreamIdsForAudioSessionByDeviceType(const int32_t zoneId, DeviceType deviceType);
    std::vector<int32_t> GetAudioSessionUidList(int32_t zoneId);
    StreamUsage GetAudioSessionStreamUsage(int32_t callerPid);
    int32_t EnableMuteSuggestionWhenMixWithOthers(int32_t callerPid, bool enable);

    void ProcessRemoteInterrupt(std::set<int32_t> streamIds, InterruptEventInternal interruptEvent);
    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object);
    void RegisterDefaultVolumeTypeListener();

    void RemoveExistingFocus(
        const int32_t appUid, std::unordered_set<int32_t> &uidActivedSessions);
    void ResumeFocusByStreamId(
        const int32_t streamId, const InterruptEventInternal interruptEventResume);
    void OnUserUnlocked();
    void SetUserId(const int32_t newId, const int32_t oldId);
    bool NotifyStreamSilentChange(uint32_t streamId);
    int32_t GetActiveAudioInterruptZone(int32_t &zoneId, AudioStreamType &streamType);
    std::future<void> stopFuture_;

    AudioScene GetHighestPriorityAudioSceneFromAllZones();
    int32_t FindAudioZoneByStreamId(const uint32_t streamId);
    bool IsAudioStreamActive(const AudioFocuState &state);
    int32_t SetAudioSessionBehavior(const int32_t callerPid, const uint32_t behavior,
        int32_t callerUid = INVALID_UID);
    void RefreshAudioSceneAfterDelay(const int32_t zoneId);
    void GetFocusDebugInfoByStreamId(uint32_t streamId, AudioDebugInfo &audioDebugInfo);
    int32_t GetSessionDebugInfo(int32_t pid, AudioSessionDebugInfo &sessionDebugInfo);
    std::vector<int32_t> FindAudioZonesByPid(int32_t pid);

private:
    static constexpr int32_t ZONEID_DEFAULT = 0;
    static constexpr int32_t ZONEID_INVALID = -1;
    static constexpr float DUCK_FACTOR = 0.2f;
    static constexpr int32_t DEFAULT_APP_PID = -1;
    static constexpr int32_t RSS_UID = 1096;
    static constexpr int32_t SET_SCENE_DELAY = 120;

    using InterruptIterator = std::list<std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator>;
    std::unordered_map<int32_t, std::vector<CachedFocusInfo>> cachedFocusMap_;
    std::mutex cachedFocusMutex_;
    int32_t oldUserId_ = 0;
    int32_t newUserId_ = 0;
    bool isGetFocusForLog_ = false;

    void CacheFocusAndCallback(const uint32_t &sessionId, const InterruptEventInternal &interruptEvent,
        const AudioInterrupt &audioInterrupt);
    void GameRecogSetParam(ClientType clientType, SourceType sourceType, bool switchOn);

    // Inner class for death handler
    class AudioInterruptDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioInterruptDeathRecipient(
            const std::shared_ptr<AudioInterruptService> &service,
            uint32_t streamId);
        virtual ~AudioInterruptDeathRecipient() = default;

        DISALLOW_COPY_AND_MOVE(AudioInterruptDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote);

    private:
        const std::weak_ptr<AudioInterruptService> service_;
        const uint32_t streamId_;
    };

    // Inner class for callback
    class AudioInterruptClient {
    public:
        explicit AudioInterruptClient(
            const std::shared_ptr<AudioInterruptCallback> &callback,
            const sptr<IRemoteObject> &object,
            const sptr<AudioInterruptDeathRecipient> &deathRecipient);
        virtual ~AudioInterruptClient();

        DISALLOW_COPY_AND_MOVE(AudioInterruptClient);

        void OnInterrupt(const InterruptEventInternal &interruptEvent);

        void SetCallingUid(uint32_t uid);
        uint32_t GetCallingUid();

    private:
        const std::shared_ptr<AudioInterruptCallback> callback_;
        const sptr<IRemoteObject> object_;
        sptr<AudioInterruptDeathRecipient> deathRecipient_;
        uint32_t callingUid_ = 0;
    };

    // deprecated interrupt interfaces
    void NotifyFocusGranted(const int32_t clientId, const AudioInterrupt &audioInterrupt);
    int32_t NotifyFocusAbandoned(const int32_t clientId, const AudioInterrupt &audioInterrupt);
    int32_t AbandonAudioFocusInternal(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    // modern interrupt interfaces
    int32_t ActivateAudioInterruptInternal(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
        const bool isUpdatedAudioStrategy, bool &updateScene);
    int32_t ActivateAudioInterruptCoreProcedure(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
        const bool isUpdatedAudioStrategy, bool &updateScene);
    void ProcessAudioScene(const AudioInterrupt &audioInterrupt, const uint32_t &incomingStreamId,
        const int32_t &zoneId, bool &shouldReturnSuccess);
    bool IsAudioSourceConcurrency(const SourceType &existSourceType, const SourceType &incomingSourceType,
        const std::vector<SourceType> &existConcurrentSources,
        const std::vector<SourceType> &incomingConcurrentSources);
    void UpdateAudioFocusStrategy(const AudioInterrupt &currentInterrupt, const AudioInterrupt &incomingInterrupt,
        AudioFocusEntry &focusEntry);
    bool FocusEntryContinue(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
        AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt);
    int32_t ProcessFocusEntry(const int32_t zoneId, const AudioInterrupt &incomingInterrupt);
    void SendInterruptEventToIncomingStream(InterruptEventInternal &interruptEvent,
        const AudioInterrupt &incomingInterrupt);
    void AddToAudioFocusInfoList(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
        const int32_t &zoneId, const AudioInterrupt &incomingInterrupt, const AudioFocuState &incomingState);
    void HandleIncomingState(const int32_t &zoneId, const AudioFocuState &incomingState,
        InterruptEventInternal &interruptEvent, const AudioInterrupt &incomingInterrupt);
    void ProcessExistInterrupt(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator
        &iterActive, AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt,
        bool &removeFocusInfo, InterruptEventInternal &interruptEvent);
    void UpdateAudioSceneForResult(AudioInterruptResult &result);
    void ProcessActiveInterrupt(const int32_t zoneId, const AudioInterrupt &incomingInterrupt);
    bool ProcessActiveInterruptContinue(const AudioFocusEntry &focusEntry,
        const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        AudioFocuState activeState);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioFocusInfoList(const int32_t zoneId);
    void ResumeAudioFocusList(const int32_t zoneId, bool isSessionTimeout = false);
    bool EvaluateWhetherContinue(const AudioInterrupt &incoming, const AudioInterrupt
        &inprocessing, AudioFocusEntry &focusEntry, bool bConcurrency);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> SimulateFocusEntry(const int32_t zoneId);
    void SendActiveInterruptEvent(const uint32_t activeStreamId, const InterruptEventInternal &interruptEvent,
        const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt);
    void DeactivateAudioInterruptInternal(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
        bool isSessionTimeout = false);
    bool CheckNeedPlaceHolder(const AudioInterrupt &audioInterrupt,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList);
    void DeactivateAudioInterruptInternalContinue(const int32_t zoneId, bool isSessionTimeout = false);
    void SendInterruptEvent(AudioFocuState oldState, AudioFocuState newState,
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive, bool &removeFocusInfo);
    void SendInterruptEventCallback(const InterruptEventInternal &interruptEvent,
        const uint32_t &streamId, const AudioInterrupt &audioInterrupt);
    bool IsSameAppInShareMode(const AudioInterrupt incomingInterrupt, const AudioInterrupt activeInterrupt);
    bool UpdateAudioSceneFromInterrupt(const AudioScene audioScene, AudioInterruptChangeType changeType,
        int32_t zoneId = ZONEID_DEFAULT, bool notFromInterrupt = true);
    void SendFocusChangeEvent(const int32_t zoneId, int32_t callbackCategory, const AudioInterrupt &audioInterrupt);
    void SendActiveVolumeTypeChangeEvent(const int32_t zoneId);
    void RemoveClient(uint32_t streamId);
    void RemoveFocusInfo(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &tmpFocusInfoList,
        std::shared_ptr<AudioInterruptZone> &zoneInfo,
        std::list<int32_t> &removeFocusInfoPidList);
    void PrintLogsOfFocusStrategyBaseMusic(const AudioInterrupt &audioInterrupt);

    // zone debug interfaces
    void WriteFocusMigrateEvent(const int32_t &toZoneId);
    void WriteServiceStartupError();
    // systemapp debug interfaces
    void WriteCallSessionEvent(int32_t strategyValue);
    void ActivateAudioSessionErrorEvent(const int32_t zoneId, const int32_t callerPid);
    void DeactivateAudioSessionErrorEvent(const std::vector<AudioInterrupt> &streamsInSession,
        const int32_t callerPid);
    void AddInterruptErrorEvent(const int32_t zoneId, const AudioInterrupt &audioInterrupt);

    // interfaces about audio session.
    void AddActiveInterruptToSession(const int32_t callerPid);
    void RemovePlaceholderInterruptForSession(const int32_t callerPid, bool isSessionTimeout = false);
    bool CanMixForSession(const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        const AudioFocusEntry &focusEntry);
    bool CanMixForIncomingSession(const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        const AudioFocusEntry &focusEntry);
    bool CanMixForActiveSession(const AudioInterrupt &incomingInterrupt, const AudioInterrupt &activeInterrupt,
        const AudioFocusEntry &focusEntry);
    bool CapturerMixForSessionBehavior(const AudioInterrupt &incomingInterrupt,
        const AudioInterrupt &activeInterrupt);
    bool IsIncomingStreamLowPriority(const AudioFocusEntry &focusEntry);
    bool IsActiveStreamLowPriority(const AudioFocusEntry &focusEntry);
    void UpdateHintTypeForExistingSession(const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry);
    void HandleSessionTimeOutEvent(const int32_t pid);
    int32_t GetAudioSessionZoneidByPid(const int32_t pid);
    bool HandleLowPriorityEvent(const int32_t pid, const uint32_t streamId);
    void SendSessionTimeOutStopEvent(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList);
    void SetNonInterruptMute(int32_t streamId, bool muteFlag);
    bool ShouldCallbackToClient(uint32_t uid, int32_t streamId, InterruptEventInternal &interruptEvent);

    AudioFocuState GetNewIncomingState(InterruptHint hintType, AudioFocuState oldState);
    void RemoveAllPlaceholderInterrupt(std::list<int32_t> &removeFocusInfoPidList);
    bool IsLowestPriorityRecording(const AudioInterrupt &audioInterrupt);
    bool IsRecordingInterruption(const AudioInterrupt &audioInterrupt);
    void SetLatestMuteState(const AudioInterrupt &audioInterrupt, bool muteFlag);

    void CheckIncommingFoucsValidity(AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt,
        std::vector<SourceType> incomingConcurrentSources);
    bool IsCanMixInterrupt(const AudioInterrupt &incomingInterrupt,
        const AudioInterrupt &activeInterrupt);
    bool HadVoipStatus(const AudioInterrupt &audioInterrupt, const std::list<std::pair<AudioInterrupt, AudioFocuState>>
        &audioFocusInfoList);
    void HandleMuteInterruptEvent(const AudioInterrupt &audioInterrupt, bool muteFlag);

    AudioStreamType GetStreamInFocusInternal(const int32_t uid, const int32_t zoneId, bool returnDefault = false);

    bool SwitchHintType(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
        InterruptEventInternal &interruptEvent, std::list<std::pair<AudioInterrupt, AudioFocuState>> &tmpFocusInfoList);

    bool IsHandleIter(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
        AudioFocuState oldState, std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterNew,
        bool isExistActiveInjectFocus = false);
    void WriteStartDfxMsg(InterruptDfxBuilder &dfxBuilder, const AudioInterrupt &audioInterrupt);
    void WriteStopDfxMsg(const AudioInterrupt &audioInterrupt);
    void WriteSessionTimeoutDfxEvent(const int32_t pid);

    bool AudioFocusInfoListRemovalCondition(const AudioInterrupt &audioInterrupt,
        const std::pair<AudioInterrupt, AudioFocuState> &audioFocus);
    AudioScene RefreshAudioSceneFromAudioInterrupt(const AudioInterrupt &audioInterrupt,
        AudioScene &highestPriorityAudioScene);
    void PostRefreshAudioSceneAction(const int32_t zoneId);
    bool ProcessFocusListIteration(const int32_t zoneId, bool isSessionTimeout,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &newAudioFocuInfoList,
        bool isExistActiveInjectFocus);

    void HandleAppStreamType(const int32_t zoneId, AudioInterrupt &audioInterrupt);
    bool IsGameAvoidCallbackCase(const AudioInterrupt &audioInterrupt);
    void ResetNonInterruptControl(AudioInterrupt audioInterrupt);
    ClientType GetClientTypeByStreamId(int32_t streamId);
    // for audiosessionv2
    int32_t ProcessFocusEntryForAudioSession(const int32_t zoneId, const int32_t callerPid, bool &updateScene);
    bool ShouldBypassAudioSessionFocus(const int32_t zoneId, const AudioInterrupt &incomingInterrupt);
    void DeactivateAudioSessionFakeInterrupt(const int32_t zoneId, const int32_t callerPid);
    void DeactivateAudioSessionFakeInterruptInternal(
        const int32_t zoneId, const int32_t callerPid, bool isSessionTimeout = false);
    void DispatchInterruptEventForAudioSession(
        InterruptEventInternal &interruptEvent, const AudioInterrupt &audioInterrupt) override;
    void DeactivateAudioSessionInFakeFocusMode(const int32_t pid, InterruptHint hintType);
    void SendAudioSessionInterruptEventCallback(
        const InterruptEventInternal &interruptEvent, const AudioInterrupt &audioInterrupt);
    void TryHandleStreamCallbackInSession(const int32_t zoneId, const AudioInterrupt &incomingInterrupt);
    bool HasAudioSessionFakeInterrupt(const int32_t zoneId, const int32_t callerPid);
    int32_t HandleExistStreamsForSession(const int32_t zoneId, const int32_t callerPid, bool &updateScene);
    void HandleOtherZoneExistStreams(const int32_t targetZoneId, const int32_t callerPid, bool &updateScene);
    void ReactivateAudioInterrupts(const int32_t zoneId, const int32_t callerPid, bool &updateScene);
    AudioScene GetHighestPriorityAudioSceneFromAudioSession(
        const AudioInterrupt &audioInterrupt, const AudioScene &audioScene) const;
    void DelayToDeactivateStreamsInAudioSession(
        const int32_t zoneId, const int32_t callerPid, const std::vector<AudioInterrupt> &streamsInSession,
        std::function<void()> onFakeInterruptDeactivate = nullptr);
    int32_t DeactivateStreamsInAudioSession(
        const int32_t zoneId, const int32_t callerPid, const std::vector<AudioInterrupt> &streamsInSession);
    int32_t ProcessActiveStreamFocus(std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList,
        const AudioInterrupt &incomingInterrupt, AudioFocuState &incomingState,
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &activeInterrupt);
    void ReportRecordGetFocusFail(const AudioInterrupt &incomingInterrupt,
        const AudioInterrupt &activeInterrupt, int32_t reason);
    void PublishCtrlCmdEvent(int32_t hintType, int32_t uid, int32_t streamId);
    void RemoveStreamIdSuggestionRecord(int32_t streamId);
    void RemovePidSuggestionRecord(int32_t pid);
    bool HasMuteSuggestionRecord(uint32_t streamId);
    void SendUnMuteSuggestionInterruptEvent(uint32_t streamId);
    void DelayRemoveMuteSuggestionRecord(uint32_t streamId);
    void UpdateMuteSuggestionRecords(uint32_t currentpid);
    void RemoveMuteSuggestionRecord();
    void AddMuteSuggestionRecord(const AudioFocusEntry &focusEntry, const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt);
    void SuggestionProcessWhenMixWithOthers(const AudioFocusEntry &focusEntry, const AudioInterrupt &currentInterrupt,
        const AudioInterrupt &incomingInterrupt);
    void RemoveInterruptFocusInfoList(const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo,
        std::list<int32_t> &removeFocusInfoPidList);
    void MuteCheckFocusStrategy(AudioFocusEntry &focusEntry,
        const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo, const AudioInterrupt &incomingInterrupt,
        bool &removeFocusInfo, InterruptEventInternal &interruptEvent);
    void ResetInterruptMuteState(const AudioInterrupt &audioInterrupt);
    bool IsActiveInjectFocusExist(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList);
    void AudioSessionStateChangeCallbackEvent(const int32_t zoneId, const int32_t callerPid,
        const InterruptErrorCode interruptErrorCode);
    int32_t FindAudioInterruptByPid(const int32_t zoneId, const int32_t callerPid, AudioInterrupt &audioInterrupt);
    void MirrorInterPhoneInterrupt(AudioInterrupt &audioInterrupt);
    void RecordFocusHistory(const AudioInterrupt &currentInterrupt, const AudioInterrupt &incomingInterrupt,
        const InterruptEventInternal &interruptEvent);
    void AddFocusHistory(uint32_t streamId, const AudioFocusBehavior &focusBehavior);
    bool UpdateFocusStateByHintType(const AudioFocusEntry &focusEntry, AudioFocuState &activeState,
        AudioFocuState &incomingState);

    // interrupt members
    sptr<AudioPolicyServer> policyServer_;
    std::shared_ptr<AudioPolicyServerHandler> handler_;
    AudioSessionService &sessionService_;
    friend class AudioInterruptZoneManager;
    AudioInterruptZoneManager zoneManager_;
    std::shared_ptr<AsyncActionHandler> asyncHandler_ = nullptr;

    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> focusCfgMap_ = {};
    std::map<int32_t, std::shared_ptr<AudioInterruptZone>> zonesMap_;

    std::map<int32_t, std::shared_ptr<AudioInterruptClient>> interruptClients_;

    std::map<uint32_t, std::shared_ptr<AudioInterrupt>> suggestionInterrupts_;
    std::map<uint32_t, std::unordered_set<int32_t>> suggestionStreamIdRecords_;
    std::map<uint32_t, std::unordered_set<int32_t>> suggestionPidRecords_;

    std::map<uint32_t, std::list<std::pair<AudioInterrupt, AudioFocuState>>> muteAudioFocus_;

    // deprecated interrupt members
    std::unique_ptr<AudioInterrupt> focussedAudioInterruptInfo_;
    int32_t clientOnFocus_ = 0;

    // preempt audio focus mode flag
    bool isPreemptMode_ = false;

    std::mutex mutex_;
    mutable std::atomic<int32_t> formerUid_ = -1;
    mutable int32_t ownerPid_ = 0;
    mutable int32_t ownerUid_ = 0;
    std::unique_ptr<AudioInterruptDfxCollector> dfxCollector_;
    AudioStreamType activeStreamType_ = STREAM_MUSIC;

    // settingsdata members
    AudioStreamType defaultVolumeType_ = STREAM_MUSIC;

    std::mutex audioServerProxyMutex_;
    std::unordered_set<uint32_t> mutedGameSessionId_;

    std::unique_ptr<AudioInterruptCustom> interruptCustom_;
    std::shared_ptr<AudioInterruptDfx> interruptDfx_;
    std::unique_ptr<AudioInterruptStrategy> interruptStrategy_;

    AudioStreamCollector& streamCollector_;
    std::map<uint32_t, std::vector<AudioFocusBehavior>> audioFocusHisMap_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_SERVICE_H
