/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "audio_info.h"
#include "audio_policy_client.h"
#include "audio_system_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_concurrency_service.h"

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

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb);
    void ReduceAudioPolicyClientProxyMap(pid_t clientPid);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateTracker(const AudioMode &mode, DeviceInfo &deviceInfo);
    int32_t UpdateTrackerInternal(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
    int32_t UpdateRendererDeviceInfo(int32_t clientUID, int32_t sessionId, DeviceInfo &outputDeviceInfo);
    int32_t UpdateRendererPipeInfo(const int32_t sessionId, const AudioPipeType pipeType);
    int32_t UpdateCapturerDeviceInfo(int32_t clientUID, int32_t sessionId, DeviceInfo &inputDeviceInfo);
    int32_t GetCurrentRendererChangeInfos(std::vector<std::unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos);
    int32_t GetCurrentCapturerChangeInfos(std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos);
    void RegisteredTrackerClientDied(int32_t uid);
    int32_t UpdateStreamState(int32_t clientUid, StreamSetStateEventInternal &streamSetStateEventInternal);
    bool IsStreamActive(AudioStreamType volumeType);
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
    int32_t GetRendererDeviceInfo(const int32_t sessionId, DeviceInfo &outputDeviceInfo);

    int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object);
    int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID);
    int32_t ActivateAudioConcurrency(const AudioPipeType &pipeType);
    void ResetRendererStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc);
    void ResetCapturerStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc);
    StreamUsage GetLastestRunningCallStreamUsage();
    std::vector<uint32_t> GetAllRendererSessionIDForUID(int32_t uid);
private:
    std::mutex streamsInfoMutex_;
    std::map<std::pair<int32_t, int32_t>, int32_t> rendererStatequeue_;
    std::map<std::pair<int32_t, int32_t>, int32_t> capturerStatequeue_;
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos_;
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos_;
    std::unordered_map<int32_t, std::shared_ptr<AudioClientTracker>> clientTracker_;
    static const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_;
    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();
    int32_t AddRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t AddCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t CheckRendererUpdataState(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateRendererDeviceInfo(DeviceInfo &outputDeviceInfo);
    int32_t UpdateCapturerDeviceInfo(DeviceInfo &inputDeviceInfo);
    int32_t UpdateRendererStreamInternal(AudioStreamChangeInfo &streamChangeInfo);
    AudioStreamType GetVolumeTypeFromContentUsage(ContentType contentType, StreamUsage streamUsage);
    AudioStreamType GetStreamTypeFromSourceType(SourceType sourceType);
    void WriterStreamChangeSysEvent(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void WriterRenderStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo);
    void WriterCaptureStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo);
    void WriteRenderStreamReleaseSysEvent(const std::unique_ptr<AudioRendererChangeInfo> &audioRendererChangeInfo);
    void WriteCaptureStreamReleaseSysEvent(const std::unique_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfo);
    void SetRendererStreamParam(AudioStreamChangeInfo &streamChangeInfo,
        std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);
    void SetCapturerStreamParam(AudioStreamChangeInfo &streamChangeInfo,
        std::unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo);
    void RegisteredRendererTrackerClientDied(const int32_t uid);
    void RegisteredCapturerTrackerClientDied(const int32_t uid);
    bool CheckRendererStateInfoChanged(AudioStreamChangeInfo &streamChangeInfo);
    bool CheckRendererInfoChanged(AudioStreamChangeInfo &streamChangeInfo);
    bool IsCallStreamUsage(StreamUsage usage);
    AudioSystemManager *audioSystemMgr_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    std::shared_ptr<AudioConcurrencyService> audioConcurrencyService_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
