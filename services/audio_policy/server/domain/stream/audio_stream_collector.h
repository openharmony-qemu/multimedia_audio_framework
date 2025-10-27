/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_STREAM_COLLECTOR_H
#define AUDIO_STREAM_COLLECTOR_H

#include "iaudio_policy_client.h"
#include "audio_system_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_concurrency_service.h"
#include "audio_ability_manager.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioStreamCollector {
public:
    static AudioStreamCollector& GetAudioStreamCollector()
    {
        static AudioStreamCollector audioStreamCollector;
        return audioStreamCollector;
    }

    AudioStreamCollector();
    ~AudioStreamCollector();

    void ReduceAudioPolicyClientProxyMap(pid_t clientPid);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateTracker(const AudioMode &mode, AudioDeviceDescriptor &deviceInfo);
    int32_t UpdateTrackerInternal(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
    int32_t UpdateRendererDeviceInfo(int32_t clientUID, int32_t sessionId, AudioDeviceDescriptor &outputDeviceInfo);
    int32_t UpdateRendererDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> outputDeviceInfo);
    int32_t UpdateRendererPipeInfo(const int32_t sessionId, const AudioPipeType pipeType);
    int32_t UpdateCapturerDeviceInfo(int32_t clientUID, int32_t sessionId, AudioDeviceDescriptor &inputDeviceInfo);
    int32_t UpdateCapturerDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> inputDeviceInfo);
    int32_t GetCurrentRendererChangeInfos(std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos);
    int32_t GetCurrentCapturerChangeInfos(std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos);
    int32_t GetRunningRendererInfos(std::vector<std::shared_ptr<AudioRendererChangeInfo>> &infos);
    void RegisteredTrackerClientDied(int32_t uid, int32_t pid);
    int32_t UpdateStreamState(int32_t clientUid, StreamSetStateEventInternal &streamSetStateEventInternal);
    void HandleAppStateChange(int32_t uid, int32_t pid, bool mute, bool &notifyMute, bool hasBackTask);
    void HandleKaraokeAppToBack(int32_t uid, int32_t pid);
    void HandleForegroundUnmute(int32_t uid, int32_t pid);
    void HandleFreezeStateChange(int32_t pid, bool mute, bool hasSession);
    void HandleBackTaskStateChange(int32_t uid, bool hasSession);
    void HandleStartStreamMuteState(int32_t uid, int32_t pid, bool mute, bool skipMedia);
    bool IsStreamActive(AudioStreamType volumeType);
    bool CheckVoiceCallActive(int32_t sessionId);
    bool IsVoiceCallActive();
    int32_t GetRunningStream(AudioStreamType certainType = STREAM_DEFAULT, int32_t certainChannelCount = 0);
    int32_t SetLowPowerVolume(int32_t streamId, float volume);
    float GetLowPowerVolume(int32_t streamId);
    int32_t SetOffloadMode(int32_t streamId, int32_t state, bool isAppBack);
    int32_t UnsetOffloadMode(int32_t streamId);
    bool IsOffloadAllowed(const int32_t sessionId);
    float GetSingleStreamVolume(int32_t streamId);
    bool GetAndCompareStreamType(StreamUsage targetUsage, AudioRendererInfo rendererInfo);
    int32_t UpdateCapturerInfoMuteStatus(int32_t uid, bool muteStatus);
    AudioStreamType GetStreamType(int32_t sessionId);
    int32_t GetChannelCount(int32_t sessionId);
    int32_t GetUid(int32_t sessionId);
    void GetRendererStreamInfo(AudioStreamChangeInfo &streamChangeInfo, AudioRendererChangeInfo &rendererInfo);
    void GetCapturerStreamInfo(AudioStreamChangeInfo &streamChangeInfo, AudioCapturerChangeInfo &capturerInfo);
    int32_t GetPipeType(const int32_t sessionId, AudioPipeType &pipeType);
    bool ExistStreamForPipe(AudioPipeType pipeType);
    int32_t GetRendererDeviceInfo(const int32_t sessionId, AudioDeviceDescriptor &outputDeviceInfo);

    ConcurrencyAction GetConcurrencyAction(const AudioPipeType existingPipe, const AudioPipeType commingPipe);
    void ResetRendererStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc);
    void ResetCapturerStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc);
    StreamUsage GetRunningStreamUsageNoUltrasonic();
    SourceType GetRunningSourceTypeNoUltrasonic();
    StreamUsage GetLastestRunningCallStreamUsage();
    std::vector<uint32_t> GetAllRendererSessionIDForUID(int32_t uid);
    std::vector<uint32_t> GetAllCapturerSessionIDForUID(int32_t uid);
    std::vector<int32_t> GetPlayingMediaSessionIdList();
    int32_t ResumeStreamState();
    bool HasVoipRendererStream(bool isFirstCreate = true);
    bool ChangeVoipCapturerStreamToNormal();
    bool IsCallStreamUsage(StreamUsage usage);
    std::set<int32_t> GetSessionIdsOnRemoteDeviceByStreamUsage(StreamUsage streamUsage);
    std::set<int32_t> GetSessionIdsOnRemoteDeviceBySourceType(SourceType sourceType);
    std::set<int32_t> GetSessionIdsOnRemoteDeviceByDeviceType(DeviceType deviceType);
    int32_t GetSessionIdsPauseOnRemoteDeviceByRemote(InterruptHint hintType);
    bool HasRunningRendererStream();
    bool HasRunningRecognitionCapturerStream();
    bool HasRunningNormalCapturerStream(DeviceType type);
    bool HasRunningCapturerStreamByUid(int32_t uid = INVALID_UID);
    bool IsMediaPlaying();
    bool IsVoipStreamActive();
    bool IsStreamRunning(StreamUsage streamUsage);
    void UpdateAppVolume(int32_t appUid, int32_t volume);

private:
    std::mutex streamsInfoMutex_;
    std::map<std::pair<int32_t, int32_t>, int32_t> rendererStatequeue_;
    std::map<std::pair<int32_t, int32_t>, int32_t> capturerStatequeue_;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos_;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos_;
    std::unordered_map<int32_t, std::shared_ptr<AudioClientTracker>> clientTracker_;
    static const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_;
    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();
    int32_t AddRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t AddCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t CheckRendererUpdataState(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateRendererDeviceInfo(AudioDeviceDescriptor &outputDeviceInfo);
    int32_t UpdateCapturerDeviceInfo(AudioDeviceDescriptor &inputDeviceInfo);
    int32_t UpdateRendererStreamInternal(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateCapturerStreamInternal(AudioStreamChangeInfo &streamChangeInfo);
    AudioStreamType GetVolumeTypeFromContentUsage(ContentType contentType, StreamUsage streamUsage);
    AudioStreamType GetStreamTypeFromSourceType(SourceType sourceType);
    void WriterStreamChangeSysEvent(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void WriterRenderStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo);
    void WriterCaptureStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo);
    void WriteRenderStreamReleaseSysEvent(const std::shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfo);
    void WriteCaptureStreamReleaseSysEvent(const std::shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfo);
    void SetRendererStreamParam(AudioStreamChangeInfo &streamChangeInfo,
        std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);
    void SetCapturerStreamParam(AudioStreamChangeInfo &streamChangeInfo,
        std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo);
    void RegisteredRendererTrackerClientDied(const int32_t uid, const int32_t pid);
    void RegisteredCapturerTrackerClientDied(const int32_t uid);
    void SendCapturerInfoEvent(const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    bool CheckRendererStateInfoChanged(AudioStreamChangeInfo &streamChangeInfo);
    bool CheckRendererInfoChanged(AudioStreamChangeInfo &streamChangeInfo);
    bool IsTransparentCapture(const uint32_t clientUid);
    void ResetRingerModeMute(RendererState rendererState, StreamUsage streamUsage);
    void PostReclaimMemoryTask();
    void ReclaimMem();
    bool CheckAudioStateIdle();
    std::atomic_bool isActivatedMemReclaiTask_ = false;
    std::mutex clearMemoryMutex_;
    AudioAbilityManager *audioAbilityMgr_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    std::shared_ptr<AudioConcurrencyService> audioConcurrencyService_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
