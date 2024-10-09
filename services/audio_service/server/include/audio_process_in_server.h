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

namespace OHOS {
namespace AudioStandard {
class ProcessReleaseCallback {
public:
    virtual ~ProcessReleaseCallback() = default;

    virtual int32_t OnProcessRelease(IAudioProcessStream *process) = 0;
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
};

class AudioProcessInServer : public AudioProcessStub, public IAudioProcessStream {
public:
    static sptr<AudioProcessInServer> Create(const AudioProcessConfig &processConfig,
        ProcessReleaseCallback *releaseCallback);
    virtual ~AudioProcessInServer();

    // override for AudioProcess
    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override;

    int32_t GetSessionId(uint32_t &sessionId) override;

    int32_t Start() override;

    int32_t Pause(bool isFlush) override;

    int32_t Resume() override;

    int32_t Stop() override;

    int32_t RequestHandleInfo(bool isAsync) override;

    int32_t Release() override;

    int32_t RegisterProcessCb(sptr<IRemoteObject> object) override;

    // override for IAudioProcessStream, used in endpoint
    std::shared_ptr<OHAudioBuffer> GetStreamBuffer() override;
    AudioStreamInfo GetStreamInfo() override;
    uint32_t GetAudioSessionId() override;
    AudioStreamType GetAudioStreamType() override;

    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void Dump(std::string &dumpString);

    int32_t ConfigProcessBuffer(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        DeviceStreamInfo &serverStreamInfo, const std::shared_ptr<OHAudioBuffer> &endpoint = nullptr);

    int32_t AddProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);
    int32_t RemoveProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);

    void SetNonInterruptMute(const bool muteFlag);
    bool GetMuteFlag() override;
    uint32_t GetSessionId();

    // for inner-cap
    void SetInnerCapState(bool isInnerCapped) override;
    bool GetInnerCapState() override;

    AppInfo GetAppInfo() override final;
    BufferDesc &GetConvertedBuffer() override;
    int32_t RegisterThreadPriority(uint32_t tid, const std::string &bundleName) override;
public:
    const AudioProcessConfig processConfig_;

private:
    AudioProcessInServer(const AudioProcessConfig &processConfig, ProcessReleaseCallback *releaseCallback);
    int32_t InitBufferStatus();
    void WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby);

private:
    std::atomic<bool> muteFlag_ = false;
    bool isInnerCapped_ = false;
    ProcessReleaseCallback *releaseCallback_ = nullptr;

    bool needCheckBackground_ = false;

    uint32_t sessionId_ = 0;
    bool isInited_ = false;
    std::atomic<StreamStatus> *streamStatus_ = nullptr;
    std::mutex statusLock_;

    uint32_t clientTid_ = 0;
    std::string clientBundleName_;
    bool clientThreadPriorityRequested_ = false;

    uint32_t totalSizeInframe_ = 0;
    uint32_t spanSizeInframe_ = 0;
    uint32_t byteSizePerFrame_ = 0;
    bool isBufferConfiged_ = false;
    std::shared_ptr<OHAudioBuffer> processBuffer_ = nullptr;
    std::mutex listenerListLock_;
    std::vector<std::shared_ptr<IProcessStatusListener>> listenerList_;
    BufferDesc convertedBuffer_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_IN_SERVER_H
