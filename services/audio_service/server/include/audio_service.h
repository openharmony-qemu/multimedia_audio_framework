/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_process_in_server.h"
#include "audio_endpoint.h"
#include "ipc_stream_in_server.h"
#include "playback_capturer_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
enum InnerCapFilterPolicy : uint32_t {
    POLICY_INVALID = 0,
    POLICY_USAGES_ONLY,
    POLICY_USAGES_AND_PIDS
};
} // anonymous namespace

class AudioService : public ProcessReleaseCallback, public ICapturerFilterListener {
public:
    static AudioService *GetInstance();
    ~AudioService();

    // override for ICapturerFilterListener
    int32_t OnCapturerFilterChange(uint32_t sessionId, const AudioPlaybackCaptureConfig &newConfig) override;
    int32_t OnCapturerFilterRemove(uint32_t sessionId) override;

    sptr<IpcStreamInServer> GetIpcStream(const AudioProcessConfig &config, int32_t &ret);

    sptr<AudioProcessInServer> GetAudioProcess(const AudioProcessConfig &config);
    // override for ProcessReleaseCallback, do release process work.
    int32_t OnProcessRelease(IAudioProcessStream *process, bool isSwitchStream = false) override;
    void ReleaseProcess(const std::string endpointName, const int32_t delayTime);

    AudioDeviceDescriptor GetDeviceInfoForProcess(const AudioProcessConfig &config);
    std::shared_ptr<AudioEndpoint> GetAudioEndpointForDevice(AudioDeviceDescriptor &deviceInfo,
        const AudioProcessConfig &clientConfig, bool isVoipStream);
    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume);

    int32_t LinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    int32_t UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void Dump(std::string &dumpString);
    float GetMaxAmplitude(bool isOutputDevice);
    void ResetAudioEndpoint();

    void RemoveRenderer(uint32_t sessionId);
    void RemoveCapturer(uint32_t sessionId);
    int32_t EnableDualToneList(uint32_t sessionId);
    int32_t DisableDualToneList(uint32_t sessionId);
    std::shared_ptr<RendererInServer> GetRendererBySessionID(const uint32_t &session);
    void SetNonInterruptMute(const uint32_t SessionId, const bool muteFlag);
    void UpdateMuteControlSet(uint32_t sessionId, bool muteFlag);

    int32_t UpdateSourceType(SourceType sourceType);
    void SetIncMaxRendererStreamCnt(AudioMode audioMode);
    int32_t GetCurrentRendererStreamCnt();
    void SetDecMaxRendererStreamCnt();
    bool IsExceedingMaxStreamCntPerUid(int32_t callingUid, int32_t appUid, int32_t maxStreamCntPerUid);
    void GetCreatedAudioStreamMostUid(int32_t &mostAppUid, int32_t &mostAppNum);
    void CleanAppUseNumMap(int32_t appUid);

private:
    AudioService();
    void DelayCallReleaseEndpoint(std::string endpointName, int32_t delayInMs);

    void InsertRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void InsertCapturer(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer);
    // for inner-capturer
    void CheckInnerCapForRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void CheckInnerCapForProcess(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void FilterAllFastProcess();
    InnerCapFilterPolicy GetInnerCapFilterPolicy();
    bool ShouldBeInnerCap(const AudioProcessConfig &rendererConfig);
    bool ShouldBeDualTone(const AudioProcessConfig &config);
    int32_t OnInitInnerCapList(); // for first InnerCap filter take effect.
    int32_t OnUpdateInnerCapList(); // for some InnerCap filter has already take effect.
    bool IsEndpointTypeVoip(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo);
    void RemoveIdFromMuteControlSet(uint32_t sessionId);
    void CheckRenderSessionMuteState(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void CheckCaptureSessionMuteState(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer);
    void CheckFastSessionMuteState(uint32_t sessionId, sptr<AudioProcessInServer> process);
    int32_t GetReleaseDelayTime(std::shared_ptr<AudioEndpoint> endpoint, bool isSwitchStream);
    void ReLinkProcessToEndpoint();

private:
    std::mutex processListMutex_;
    std::vector<std::pair<sptr<AudioProcessInServer>, std::shared_ptr<AudioEndpoint>>> linkedPairedList_;

    std::mutex releaseEndpointMutex_;
    std::condition_variable releaseEndpointCV_;
    std::set<std::string> releasingEndpointSet_;
    std::map<std::string, std::shared_ptr<AudioEndpoint>> endpointList_;

    // for inner-capturer
    PlaybackCapturerManager *innerCapturerMgr_ = nullptr;
    uint32_t workingInnerCapId_ = 0; // invalid sessionId
    uint32_t workingDualToneId_ = 0; // invalid sessionId
    AudioPlaybackCaptureConfig workingConfig_;

    std::mutex rendererMapMutex_;
    std::mutex capturerMapMutex_;
    std::vector<std::weak_ptr<RendererInServer>> filteredRendererMap_ = {};
    std::map<uint32_t, std::weak_ptr<RendererInServer>> allRendererMap_ = {};
    std::map<uint32_t, std::weak_ptr<CapturerInServer>> allCapturerMap_ = {};

    std::vector<std::weak_ptr<RendererInServer>> filteredDualToneRendererMap_ = {};

    std::mutex mutedSessionsMutex_;
    std::set<uint32_t> mutedSessions_ = {};
    int32_t currentRendererStreamCnt_ = 0;
    std::mutex streamLifeCycleMutex_ {};
    std::map<int32_t, std::int32_t> appUseNumMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_H
