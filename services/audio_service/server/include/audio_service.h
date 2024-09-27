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
    int32_t OnProcessRelease(IAudioProcessStream *process) override;

    DeviceInfo GetDeviceInfoForProcess(const AudioProcessConfig &config);
    std::shared_ptr<AudioEndpoint> GetAudioEndpointForDevice(DeviceInfo &deviceInfo,
        const AudioProcessConfig &clientConfig, bool isVoipStream);
    int32_t NotifyStreamVolumeChanged(AudioStreamType streamType, float volume);

    int32_t LinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    int32_t UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void Dump(std::string &dumpString);
    float GetMaxAmplitude(bool isOutputDevice);
    void ResetAudioEndpoint();

    void RemoveRenderer(uint32_t sessionId);
    int32_t EnableDualToneList(uint32_t sessionId);
    int32_t DisableDualToneList(uint32_t sessionId);
    int32_t UpdateSourceType(SourceType sourceType);

private:
    AudioService();
    void DelayCallReleaseEndpoint(std::string endpointName, int32_t delayInMs);

    void InsertRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    // for inner-capturer
    void CheckInnerCapForRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer);
    void CheckInnerCapForProcess(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint);
    void FilterAllFastProcess();
    InnerCapFilterPolicy GetInnerCapFilterPolicy();
    bool ShouldBeInnerCap(const AudioProcessConfig &rendererConfig);
    bool ShouldBeDualTone(const AudioProcessConfig &config);
    int32_t OnInitInnerCapList(); // for first InnerCap filter take effect.
    int32_t OnUpdateInnerCapList(); // for some InnerCap filter has already take effect.
    bool IsEndpointTypeVoip(const AudioProcessConfig &config, DeviceInfo &deviceInfo);

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
    std::vector<std::weak_ptr<RendererInServer>> filteredRendererMap_ = {};
    std::map<uint32_t, std::weak_ptr<RendererInServer>> allRendererMap_ = {};

    std::vector<std::weak_ptr<RendererInServer>> filteredDualToneRendererMap_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_H
