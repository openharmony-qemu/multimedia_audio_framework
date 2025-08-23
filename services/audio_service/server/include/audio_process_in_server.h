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

#ifndef AUDIO_PROCESS_IN_SERVER_H
#define AUDIO_PROCESS_IN_SERVER_H

#include <mutex>
#include <sstream>

#include "audio_process_stub.h"
#include "i_audio_process_stream.h"
#include "i_process_status_listener.h"
#include "player_dfx_writer.h"
#include "recorder_dfx_writer.h"
#include "audio_schedule_guard.h"
#include "audio_stream_monitor.h"
#include "audio_stream_checker.h"

namespace OHOS {
namespace AudioStandard {
class ProcessReleaseCallback {
public:
    virtual ~ProcessReleaseCallback() = default;

    virtual int32_t OnProcessRelease(IAudioProcessStream *process, bool isSwitchStream = false) = 0;
};
class AudioProcessInServer;
class ProcessDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    ProcessDeathRecipient(AudioProcessInServer *processInServer, ProcessReleaseCallback *processHolder);
    virtual ~ProcessDeathRecipient() = default;
    // overridde for DeathRecipient
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    ProcessReleaseCallback *processHolder_ = nullptr;
    AudioProcessInServer *processInServer_ = nullptr;
    int64_t createTime_ = 0;
};

class AudioProcessInServer : public AudioProcessStub, public IAudioProcessStream {
public:
    static sptr<AudioProcessInServer> Create(const AudioProcessConfig &processConfig,
        ProcessReleaseCallback *releaseCallback);
    virtual ~AudioProcessInServer();

    // override for AudioProcess
    int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
        uint32_t &spanSizeInFrame) override;

    int32_t GetSessionId(uint32_t &sessionId) override;

    int32_t Start() override;

    int32_t Pause(bool isFlush) override;

    int32_t Resume() override;

    int32_t Stop(int32_t stage) override;

    int32_t RequestHandleInfo() override;

    int32_t RequestHandleInfoAsync() override;

    int32_t Release(bool isSwitchStream) override;

    int32_t SetDefaultOutputDevice(int32_t defaultOutputDevice, bool skipForce = false) override;

    int32_t SetSilentModeAndMixWithOthers(bool on) override;

    int32_t SetSourceDuration(int64_t duration) override;

    int32_t SetUnderrunCount(uint32_t underrunCnt) override;

    int32_t SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, const std::string& adjustTime,
        uint32_t code) override;

    int32_t RegisterProcessCb(const sptr<IRemoteObject>& object) override;

    int32_t RegisterThreadPriority(int32_t tid, const std::string &bundleName,
        uint32_t method) override;
    
    int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override;
    int32_t GetAudioHapticsSyncId() override;

    // override for IAudioProcessStream, used in endpoint
    std::shared_ptr<OHAudioBufferBase> GetStreamBuffer() override;
    AudioStreamInfo GetStreamInfo() override;
    uint32_t GetAudioSessionId() override;
    AudioStreamType GetAudioStreamType() override;
    AudioProcessConfig GetAudioProcessConfig() override;
    void EnableStandby() override;

    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void Dump(std::string &dumpString);

    int32_t ConfigProcessBuffer(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        AudioStreamInfo &serverStreamInfo, const std::shared_ptr<OHAudioBufferBase> &endpoint = nullptr);

    int32_t AddProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);
    int32_t RemoveProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);

    void SetNonInterruptMute(const bool muteFlag);
    bool GetMuteState() override;
    uint32_t GetSessionId();
    int32_t GetStandbyStatus(bool &isStandby, int64_t &enterStandbyTime);

    // for inner-cap
    void SetInnerCapState(bool isInnerCapped, int32_t innerCapId) override;
    bool GetInnerCapState(int32_t innerCapId) override;
    std::unordered_map<int32_t, bool> GetInnerCapState() override;

    AppInfo GetAppInfo() override final;
    BufferDesc &GetConvertedBuffer() override;

    void WriteDumpFile(void *buffer, size_t bufferSize) override final;

    std::time_t GetStartMuteTime() override;
    void SetStartMuteTime(std::time_t time) override;
 
    bool GetSilentState() override;
    void SetSilentState(bool state) override;
    void AddMuteWriteFrameCnt(int64_t muteFrameCnt) override;
    void AddMuteFrameSize(int64_t muteFrameCnt) override;
    void AddNormalFrameSize() override;
    void AddNoDataFrameSize() override;
    StreamStatus GetStreamStatus() override;
    RestoreStatus RestoreSession(RestoreInfo restoreInfo);
    int32_t StopSession();
    
    bool TurnOnMicIndicator(CapturerState capturerState);
    bool TurnOffMicIndicator(CapturerState capturerState);

    uint32_t GetSpanSizeInFrame() override;
    uint32_t GetByteSizePerFrame() override;
public:
    const AudioProcessConfig processConfig_;

private:
    int32_t StartInner();
    int64_t GetLastAudioDuration();
    AudioProcessInServer(const AudioProcessConfig &processConfig, ProcessReleaseCallback *releaseCallback);
    int32_t InitBufferStatus();
    bool CheckBGCapturer();
    void WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby);
    void ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type);

private:
    std::atomic<bool> muteFlag_ = false;
    std::atomic<bool> silentModeAndMixWithOthers_ = false;
    std::mutex innerCapStateMutex_;
    std::unordered_map<int32_t, bool> innerCapStates_;
    ProcessReleaseCallback *releaseCallback_ = nullptr;
    sptr<IRemoteObject> object_ = nullptr;
    sptr<ProcessDeathRecipient> deathRecipient_ = nullptr;

    bool needCheckBackground_ = false;
    bool isMicIndicatorOn_ = false;

    uint32_t sessionId_ = 0;
    bool isInited_ = false;
    std::atomic<StreamStatus> *streamStatus_ = nullptr;
    std::mutex statusLock_;

    uint32_t clientTid_ = 0;
    std::string clientBundleName_;

    uint32_t totalSizeInframe_ = 0;
    uint32_t spanSizeInframe_ = 0;
    uint32_t byteSizePerFrame_ = 0;
    bool isBufferConfiged_ = false;
    std::shared_ptr<OHAudioBufferBase> processBuffer_ = nullptr;
    std::mutex listenerListLock_;
    std::vector<std::shared_ptr<IProcessStatusListener>> listenerList_;
    BufferDesc convertedBuffer_ = {};
    std::string dumpFileName_;
    FILE *dumpFile_ = nullptr;
    int64_t enterStandbyTime_ = 0;
    std::time_t startMuteTime_ = 0;
    bool isInSilentState_ = false;

    int64_t lastStartTime_{};
    int64_t lastStopTime_{};
    int64_t lastWriteFrame_{};
    int64_t lastWriteMuteFrame_{};
    std::atomic<uint32_t> underrunCount_ = 0;
    int64_t sourceDuration_ = -1;
    std::unique_ptr<PlayerDfxWriter> playerDfx_;
    std::unique_ptr<RecorderDfxWriter> recorderDfx_;

    std::array<std::shared_ptr<SharedAudioScheduleGuard>, METHOD_MAX> scheduleGuards_ = {};
    std::mutex scheduleGuardsMutex_;
    std::shared_ptr<AudioStreamChecker> audioStreamChecker_ = nullptr;
    
    std::atomic<int32_t> audioHapticsSyncId_ = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_IN_SERVER_H
