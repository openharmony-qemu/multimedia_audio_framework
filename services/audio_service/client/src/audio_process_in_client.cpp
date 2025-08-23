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
#ifndef LOG_TAG
#define LOG_TAG "AudioProcessInClientInner"
#endif

#include "audio_process_in_client.h"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>
#include <algorithm>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "audio_system_manager.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#include "securec.h"

#include "audio_manager_base.h"
#include "audio_server_death_recipient.h"
#include "fast_audio_stream.h"
#include "linear_pos_time_model.h"
#include "volume_tools.h"
#include "format_converter.h"
#include "ring_buffer_wrapper.h"
#include "iaudio_process.h"
#include "process_cb_stub.h"
#include "istandard_audio_service.h"

namespace OHOS {
namespace AudioStandard {

namespace {
static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
static const int64_t DELAY_RESYNC_TIME = 10000000000; // 10s
constexpr int32_t RETRY_WAIT_TIME_MS = 500; // 500ms
constexpr int32_t MAX_RETRY_COUNT = 8;
static constexpr int64_t FAST_WRITE_CACHE_TIMEOUT_IN_MS = 40; // 40ms
static const uint32_t FAST_WAIT_FOR_NEXT_CB_US = 2500; // 2.5ms
static const uint32_t VOIP_WAIT_FOR_NEXT_CB_US = 10000; // 10ms
}

class ProcessCbImpl;
class AudioProcessInClientInner : public AudioProcessInClient,
    public std::enable_shared_from_this<AudioProcessInClientInner> {
public:
    AudioProcessInClientInner(const sptr<IAudioProcess> &ipcProxy, bool isVoipMmap, AudioStreamInfo targetStreamInfo);
    ~AudioProcessInClientInner();

    int32_t SaveDataCallback(const std::shared_ptr<AudioDataCallback> &dataCallback) override;

    int32_t SaveUnderrunCallback(const std::shared_ptr<ClientUnderrunCallBack> &underrunCallback) override;

    int32_t GetBufferDesc(BufferDesc &bufDesc) const override;

    int32_t Enqueue(const BufferDesc &bufDesc) override;

    int32_t SetVolume(int32_t vol) override;

    int32_t Start() override;

    int32_t Pause(bool isFlush) override;

    int32_t Resume() override;

    int32_t Stop(AudioProcessStage stage = AUDIO_PROC_STAGE_STOP) override;

    int32_t Release(bool isSwitchStream = false) override;

    // methods for support IAudioStream
    int32_t GetSessionID(uint32_t &sessionID) override;

    bool GetAudioTime(uint32_t &framePos, int64_t &sec, int64_t &nanoSec) override;

    int32_t GetBufferSize(size_t &bufferSize) override;

    int32_t GetFrameCount(uint32_t &frameCount) override;

    int32_t GetLatency(uint64_t &latency) override;

    int32_t SetVolume(float vol) override;

    float GetVolume() override;

    int32_t SetDuckVolume(float vol) override;

    float GetDuckVolume() override;

    int32_t SetMute(bool mute) override;

    bool GetMute() override;

    int32_t SetSourceDuration(int64_t duration) override;

    uint32_t GetUnderflowCount() override;

    uint32_t GetOverflowCount() override;

    void SetUnderflowCount(uint32_t underflowCount) override;

    void SetOverflowCount(uint32_t overflowCount) override;

    int64_t GetFramesWritten() override;

    int64_t GetFramesRead() override;

    void SetPreferredFrameSize(int32_t frameSize) override;

    void UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer) override;
    
    bool Init(const AudioProcessConfig &config, std::weak_ptr<FastAudioStream> weakStream);

    int32_t SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce = false) override;

    int32_t SetSilentModeAndMixWithOthers(bool on) override;

    void GetRestoreInfo(RestoreInfo &restoreInfo) override;

    void SetRestoreInfo(RestoreInfo &restoreInfo) override;

    RestoreStatus CheckRestoreStatus() override;

    RestoreStatus SetRestoreStatus(RestoreStatus restoreStatus) override;

    void SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string adjustTime, uint32_t code) override;

    int32_t RegisterThreadPriority(pid_t tid, const std::string &bundleName, BoostTriggerMethod method) override;

    bool GetStopFlag() const override;

    void JoinCallbackLoop() override;

    void SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId) override;

    static const sptr<IStandardAudioService> GetAudioServerProxy();
    static void AudioServerDied(pid_t pid, pid_t uid);

private:
    static bool ChannelFormatS16Convert(const AudioStreamData &srcData, const AudioStreamData &dstData);
    static bool ChannelFormatS32Convert(const AudioStreamData &srcData, const AudioStreamData &dstData);

    bool InitAudioBuffer();

    void CallClientHandleCurrent();
    int32_t ReadFromProcessClient() const;
    int32_t RecordReSyncServicePos();
    int32_t RecordFinishHandleCurrent(uint64_t &curReadPos, int64_t &clientReadCost);

    void UpdateHandleInfo(bool isAysnc = true, bool resetReadWritePos = false);
    int64_t GetPredictNextHandleTime(uint64_t posInFrame, bool isIndependent = false);

    std::string GetStatusInfo(StreamStatus status);
    bool KeepLoopRunning();
    bool KeepLoopRunningIndependent();

    void CallExitStandBy();

    bool ProcessCallbackFuc(uint64_t &curWritePos);
    bool RecordProcessCallbackFuc(uint64_t &curReadPos, int64_t clientReadCost);
    void InitPlaybackThread(std::weak_ptr<FastAudioStream> weakStream);
    void InitRecordThread(std::weak_ptr<FastAudioStream> weakStream);
    void CopyWithVolume(const BufferDesc &srcDesc, const BufferDesc &dstDesc) const;
    void ProcessVolume(const AudioStreamData &targetData) const;
    int32_t ProcessData(const BufferDesc &srcDesc, const BufferDesc &dstDesc) const;
    int32_t ProcessData(const BufferDesc &srcDesc, const RingBufferWrapper &dstDesc);
    void CheckIfWakeUpTooLate(int64_t &curTime, int64_t &wakeUpTime);
    void CheckIfWakeUpTooLate(int64_t &curTime, int64_t &wakeUpTime, int64_t clientWriteCost);

    void DoFadeInOut(const BufferDesc &buffDesc);

    bool CheckAndWaitBufferReadyForPlayback();

    bool CheckAndWaitBufferReadyForRecord();

    void WaitForWritableSpace();

    int32_t WriteDataChunk(const BufferDesc &bufDesc, size_t clientRemainSizeInFrame);

    bool WaitIfBufferEmpty(const BufferDesc &bufDesc);

    void ExitStandByIfNeed();

    bool IsRestoreNeeded();
private:
    static constexpr int64_t MILLISECOND_PER_SECOND = 1000; // 1000ms
    static constexpr int64_t ONE_MILLISECOND_DURATION = 1000000; // 1ms
    static constexpr int64_t TWO_MILLISECOND_DURATION = 2000000; // 2ms
    static constexpr int64_t VOIP_MILLISECOND_DURATION = 20000000; // 20ms
    static constexpr int64_t MAX_WRITE_COST_DURATION_NANO = 5000000; // 5ms
    static constexpr int64_t MAX_READ_COST_DURATION_NANO = 5000000; // 5ms
    static constexpr int64_t WRITE_BEFORE_DURATION_NANO = 2000000; // 2ms
    static constexpr int64_t RECORD_RESYNC_SLEEP_NANO = 2000000; // 2ms
    static constexpr int64_t RECORD_HANDLE_DELAY_NANO = 3000000; // 3ms
    static constexpr size_t MAX_TIMES = 4; // 4 times spanSizeInFrame_
    static constexpr size_t DIV = 2; // halt of span
    static constexpr int64_t MAX_STOP_FADING_DURATION_NANO = 10000000; // 10ms
    static constexpr int64_t WAKE_UP_LATE_COUNT = 20; // late for 20 times
    enum ThreadStatus : uint32_t {
        WAITTING = 0,
        SLEEPING,
        INRUNNING,
        INVALID
    };
    AudioProcessConfig processConfig_;
    bool needConvert_ = false;
    size_t clientByteSizePerFrame_ = 0;
    size_t clientSpanSizeInByte_ = 0;
    size_t clientSpanSizeInFrame_ = 240;
    sptr<IAudioProcess> processProxy_ = nullptr;
    std::shared_ptr<OHAudioBufferBase> audioBuffer_ = nullptr;
    uint32_t sessionId_ = 0;
    bool isVoipMmap_ = false;

    AudioStreamInfo targetStreamInfo_;
    uint32_t totalSizeInFrame_ = 0;
    uint32_t spanSizeInFrame_ = 0;
    uint32_t byteSizePerFrame_ = 0;
    uint32_t spanSizeInMs_ = 0;
    size_t spanSizeInByte_ = 0;
    std::weak_ptr<AudioDataCallback> audioDataCallback_;
    std::weak_ptr<ClientUnderrunCallBack> underrunCallback_;

    std::unique_ptr<uint8_t[]> callbackBuffer_ = nullptr;

    std::mutex statusSwitchLock_;
    std::atomic<StreamStatus> *streamStatus_ = nullptr;
    bool isInited_ = false;
    bool needReSyncPosition_ = true;
    int64_t lastPausedTime_ = INT64_MAX;

    float volumeInFloat_ = 1.0f;
    float duckVolumeInFloat_ = 1.0f;
    float muteVolumeInFloat_ = 1.0f;
    int32_t processVolume_ = PROCESS_VOLUME_MAX; // 0 ~ 65536
    LinearPosTimeModel handleTimeModel_;

    std::thread callbackLoop_; // thread for callback to client and write.
    std::mutex loopMutex_;
    bool isCallbackLoopEnd_ = false;
    std::atomic<ThreadStatus> threadStatus_ = INVALID;
    std::mutex loopThreadLock_;
    std::condition_variable threadStatusCV_;

    std::atomic<uint32_t> underflowCount_ = 0;
    std::atomic<uint32_t> overflowCount_ = 0;

    FILE *dumpFile_ = nullptr;
    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";

    std::atomic<bool> startFadein_ = false; // true-fade  in  when start or resume stream
    std::atomic<bool> startFadeout_ = false; // true-fade out when pause or stop stream

    sptr<ProcessCbImpl> processCbImpl_ = nullptr;

    std::vector<uint8_t> tmpBuffer_ = {};
    std::mutex tmpBufferMutex_;

    struct HandleInfo {
        uint64_t serverHandlePos = 0;
        int64_t serverHandleTime = 0;
    };

    std::atomic<HandleInfo> lastHandleInfo_;
};

// ProcessCbImpl --> sptr | AudioProcessInClientInner --> shared_ptr
class ProcessCbImpl : public ProcessCbStub {
public:
    explicit ProcessCbImpl(std::shared_ptr<AudioProcessInClientInner> processInClientInner);
    virtual ~ProcessCbImpl() = default;

    int32_t OnEndpointChange(int32_t status) override;

private:
    std::weak_ptr<AudioProcessInClientInner> processInClientInner_;
};

ProcessCbImpl::ProcessCbImpl(std::shared_ptr<AudioProcessInClientInner> processInClientInner)
{
    if (processInClientInner == nullptr) {
        AUDIO_ERR_LOG("ProcessCbImpl() find null processInClientInner");
    }
    processInClientInner_ = processInClientInner;
}

int32_t ProcessCbImpl::OnEndpointChange(int32_t status)
{
    AUDIO_INFO_LOG("OnEndpointChange: %{public}d", status);
    return SUCCESS;
}

std::mutex g_audioServerProxyMutex;
sptr<IStandardAudioService> gAudioServerProxy = nullptr;

AudioProcessInClientInner::AudioProcessInClientInner(const sptr<IAudioProcess> &ipcProxy, bool isVoipMmap,
    AudioStreamInfo targetStreamInfo) : processProxy_(ipcProxy), isVoipMmap_(isVoipMmap),
    targetStreamInfo_(targetStreamInfo)
{
    processProxy_->GetSessionId(sessionId_);
    AUDIO_INFO_LOG("Construct with sessionId: %{public}d", sessionId_);
}

const sptr<IStandardAudioService> AudioProcessInClientInner::GetAudioServerProxy()
{
    std::lock_guard<std::mutex> lock(g_audioServerProxyMutex);
    if (gAudioServerProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        gAudioServerProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(gAudioServerProxy != nullptr, nullptr, "get audio service proxy failed");

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) { AudioServerDied(pid, uid); });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_WARNING_LOG("failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = gAudioServerProxy;
    return gasp;
}

/**
 * When AudioServer died, all stream in client should be notified. As they were proxy stream ,the stub stream
 * has been destoried in server.
*/
void AudioProcessInClientInner::AudioServerDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("audio server died, will restore proxy in next call");
    std::lock_guard<std::mutex> lock(g_audioServerProxyMutex);
    gAudioServerProxy = nullptr;
}

std::shared_ptr<AudioProcessInClient> AudioProcessInClient::Create(const AudioProcessConfig &config,
    std::weak_ptr<FastAudioStream> weakStream)
{
    AUDIO_INFO_LOG("Create with config: render flag %{public}d, capturer flag %{public}d, streamType %{public}d.",
        config.rendererInfo.rendererFlags, config.capturerInfo.capturerFlags, config.streamType);
    bool ret = AudioProcessInClient::CheckIfSupport(config);
    CHECK_AND_RETURN_RET_LOG(config.audioMode != AUDIO_MODE_PLAYBACK || ret, nullptr,
        "CheckIfSupport failed!");
    AudioStreamInfo targetStreamInfo = AudioPolicyManager::GetInstance().GetFastStreamInfo();
    sptr<IStandardAudioService> gasp = AudioProcessInClientInner::GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, nullptr, "Create failed, can not get service.");
    AudioProcessConfig resetConfig = config;
    bool isVoipMmap = false;
    if (config.rendererInfo.streamUsage != STREAM_USAGE_VOICE_COMMUNICATION &&
        config.rendererInfo.streamUsage != STREAM_USAGE_VIDEO_COMMUNICATION &&
        config.capturerInfo.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION) {
        resetConfig.streamInfo = targetStreamInfo;
        if (config.audioMode == AUDIO_MODE_RECORD) {
            resetConfig.streamInfo.format = config.streamInfo.format;
            resetConfig.streamInfo.channels = config.streamInfo.channels;
        }
    } else {
        isVoipMmap = true;
    }

    int32_t errorCode = 0;
    sptr<IRemoteObject> ipcProxy = nullptr;
    AudioPlaybackCaptureConfig filterConfig = {};
    gasp->CreateAudioProcess(resetConfig, errorCode, filterConfig, ipcProxy);
    for (int32_t retrycount = 0; (errorCode == ERR_RETRY_IN_CLIENT) && (retrycount < MAX_RETRY_COUNT); retrycount++) {
        AUDIO_WARNING_LOG("retry in client");
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_TIME_MS));
        gasp->CreateAudioProcess(config, errorCode, filterConfig, ipcProxy);
    }
    CHECK_AND_RETURN_RET_LOG(errorCode == SUCCESS, nullptr, "failed with create audio stream fail.");
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, nullptr, "Create failed with null ipcProxy.");
    sptr<IAudioProcess> iProcessProxy = iface_cast<IAudioProcess>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(iProcessProxy != nullptr, nullptr, "Create failed when iface_cast.");
    std::shared_ptr<AudioProcessInClientInner> process =
        std::make_shared<AudioProcessInClientInner>(iProcessProxy, isVoipMmap, targetStreamInfo);
    if (!process->Init(config, weakStream)) {
        AUDIO_ERR_LOG("Init failed!");
        process = nullptr;
    }

    return process;
}

AudioProcessInClientInner::~AudioProcessInClientInner()
{
    AUDIO_INFO_LOG("AudioProcessInClient deconstruct.");

    JoinCallbackLoop();
    if (isInited_) {
        AudioProcessInClientInner::Release();
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
}

int32_t AudioProcessInClientInner::GetSessionID(uint32_t &sessionID)
{
    sessionID = sessionId_;
    return SUCCESS;
}

bool AudioProcessInClientInner::GetAudioTime(uint32_t &framePos, int64_t &sec, int64_t &nanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, false, "buffer is null, maybe not inited.");

    uint64_t serverHandlePos = 0;
    int64_t serverHandleTime = 0;
    bool ret = audioBuffer_->GetHandleInfo(serverHandlePos, serverHandleTime);
    CHECK_AND_RETURN_RET_LOG(ret, false, "GetHandleInfo failed");

    if (serverHandlePos > UINT32_MAX) {
        framePos = serverHandlePos % UINT32_MAX;
    } else {
        framePos = static_cast<uint32_t>(serverHandlePos);
    }

    auto lastHandleInfo = lastHandleInfo_.load();
    if (lastHandleInfo.serverHandlePos < framePos || lastHandleInfo.serverHandleTime == 0) {
        lastHandleInfo_.compare_exchange_strong(lastHandleInfo, {framePos, serverHandleTime});
    } else {
        framePos = lastHandleInfo.serverHandlePos;
        serverHandleTime = lastHandleInfo.serverHandleTime;
    }

    sec = serverHandleTime / AUDIO_NS_PER_SECOND;
    nanoSec = serverHandleTime % AUDIO_NS_PER_SECOND;
    return true;
}

int32_t AudioProcessInClientInner::GetBufferSize(size_t &bufferSize)
{
    bufferSize = clientSpanSizeInByte_;
    return SUCCESS;
}

int32_t AudioProcessInClientInner::GetFrameCount(uint32_t &frameCount)
{
    frameCount = static_cast<uint32_t>(clientSpanSizeInFrame_);
    AUDIO_INFO_LOG ("GetFrameCount successfully, FrameCount: %{public}u", frameCount);
    return SUCCESS;
}

int32_t AudioProcessInClientInner::GetLatency(uint64_t &latency)
{
    latency = 20; // 20ms for debug
    return SUCCESS;
}

int32_t AudioProcessInClientInner::SetVolume(float vol)
{
    float minVol = 0.0f;
    float maxVol = 1.0f;
    CHECK_AND_RETURN_RET_LOG(vol >= minVol && vol <= maxVol, ERR_INVALID_PARAM,
        "SetVolume failed to with invalid volume:%{public}f", vol);
    int32_t volumeInt = static_cast<int32_t>(vol * PROCESS_VOLUME_MAX);
    int32_t ret = SetVolume(volumeInt);
    if (ret == SUCCESS) {
        SaveAdjustStreamVolumeInfo(vol, sessionId_, GetTime(),
            static_cast<uint32_t>(AdjustStreamVolume::STREAM_VOLUME_INFO));
        volumeInFloat_ = vol;

        CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, ret, "audiobuffer_ is null");
        audioBuffer_->SetStreamVolume(vol);
    }
    return ret;
}

float AudioProcessInClientInner::GetVolume()
{
    return volumeInFloat_;
}

int32_t AudioProcessInClientInner::SetMute(bool mute)
{
    muteVolumeInFloat_ = mute ? 0.0f : 1.0f;
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, SUCCESS, "audiobuffer_ is null");
    audioBuffer_->SetMuteFactor(muteVolumeInFloat_);
    return SUCCESS;
}

bool AudioProcessInClientInner::GetMute()
{
    return std::abs(muteVolumeInFloat_ - 0.0f) <= std::numeric_limits<float>::epsilon();
}

int32_t AudioProcessInClientInner::SetSourceDuration(int64_t duration)
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr, ERR_OPERATION_FAILED, "ipcProxy is null.");
    return processProxy_->SetSourceDuration(duration);
}

int32_t AudioProcessInClientInner::SetDuckVolume(float vol)
{
    float minVol = 0.0f;
    float maxVol = 1.0f;
    CHECK_AND_RETURN_RET_LOG(vol >= minVol && vol <= maxVol, ERR_INVALID_PARAM,
        "SetDuckVolume failed to with invalid volume:%{public}f", vol);
    SaveAdjustStreamVolumeInfo(vol, sessionId_, GetTime(),
        static_cast<uint32_t>(AdjustStreamVolume::DUCK_VOLUME_INFO));
    duckVolumeInFloat_ = vol;

    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, SUCCESS, "audiobuffer_ is null");
    audioBuffer_->SetDuckFactor(vol);

    return SUCCESS;
}

float AudioProcessInClientInner::GetDuckVolume()
{
    return duckVolumeInFloat_;
}

uint32_t AudioProcessInClientInner::GetUnderflowCount()
{
    return underflowCount_.load();
}

uint32_t AudioProcessInClientInner::GetOverflowCount()
{
    return overflowCount_.load();
}

void AudioProcessInClientInner::SetUnderflowCount(uint32_t underflowCount)
{
    underflowCount_ += underflowCount;
}

void AudioProcessInClientInner::SetOverflowCount(uint32_t overflowCount)
{
    overflowCount_ += overflowCount;
}

int64_t AudioProcessInClientInner::GetFramesWritten()
{
    CHECK_AND_RETURN_RET_LOG(processConfig_.audioMode == AUDIO_MODE_PLAYBACK, -1, "Playback not support.");
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, -1, "buffer is null, maybe not inited.");
    return audioBuffer_->GetCurWriteFrame();
}

int64_t AudioProcessInClientInner::GetFramesRead()
{
    CHECK_AND_RETURN_RET_LOG(processConfig_.audioMode == AUDIO_MODE_RECORD, -1, "Record not support.");
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, -1, "buffer is null, maybe not inited.");
    return audioBuffer_->GetCurReadFrame();
}

void AudioProcessInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    size_t originalSpanSizeInFrame = static_cast<size_t>(spanSizeInFrame_);
    size_t tmp = static_cast<size_t>(frameSize);
    size_t count = static_cast<size_t>(frameSize) / spanSizeInFrame_;
    size_t rest = static_cast<size_t>(frameSize) % spanSizeInFrame_;
    if (tmp <= originalSpanSizeInFrame) {
        clientSpanSizeInFrame_ = originalSpanSizeInFrame;
    } else if (tmp >= MAX_TIMES * originalSpanSizeInFrame) {
        clientSpanSizeInFrame_ = MAX_TIMES * originalSpanSizeInFrame;
    } else {
        if (rest <= originalSpanSizeInFrame / DIV) {
            clientSpanSizeInFrame_ = count * spanSizeInFrame_;
        } else {
            count++;
            clientSpanSizeInFrame_ = count * spanSizeInFrame_;
        }
    }
    if (clientByteSizePerFrame_ == 0) {
        clientSpanSizeInByte_ = count * spanSizeInByte_;
    } else {
        clientSpanSizeInByte_ = clientSpanSizeInFrame_ * clientByteSizePerFrame_;
    }
    callbackBuffer_ = std::make_unique<uint8_t[]>(clientSpanSizeInByte_);
    AUDIO_INFO_LOG("Set preferred callbackBuffer size:%{public}zu", clientSpanSizeInByte_);
    memset_s(callbackBuffer_.get(), clientSpanSizeInByte_, 0, clientSpanSizeInByte_);
}

void AudioProcessInClientInner::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    sptr<IStandardAudioService> gasp = AudioProcessInClientInner::GetAudioServerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("LatencyMeas failed to get AudioServerProxy");
        return;
    }
    gasp->UpdateLatencyTimestamp(timestamp, isRenderer);
}

bool AudioProcessInClientInner::InitAudioBuffer()
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr, false, "Init failed with null ipcProxy.");
    processCbImpl_ = sptr<ProcessCbImpl>::MakeSptr(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(processProxy_->RegisterProcessCb(processCbImpl_) == SUCCESS, false,
        "RegisterProcessCb failed.");

    int32_t ret = processProxy_->ResolveBufferBaseAndGetServerSpanSize(audioBuffer_, spanSizeInFrame_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioBuffer_ != nullptr, false,
        "Init failed to call ResolveBuffer");
    streamStatus_ = audioBuffer_->GetStreamStatus();
    CHECK_AND_RETURN_RET_LOG(streamStatus_ != nullptr, false, "Init failed, access buffer failed.");

    audioBuffer_->GetSizeParameter(totalSizeInFrame_, byteSizePerFrame_);
    spanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame_;
    spanSizeInMs_ = spanSizeInFrame_ * MILLISECOND_PER_SECOND / processConfig_.streamInfo.samplingRate;

    clientSpanSizeInByte_ = spanSizeInFrame_ * clientByteSizePerFrame_;
    clientSpanSizeInFrame_ = spanSizeInFrame_;
    if ((processConfig_.audioMode != AUDIO_MODE_PLAYBACK) && (!isVoipMmap_)) {
        clientSpanSizeInByte_ = spanSizeInByte_;
    }

    AUDIO_INFO_LOG("Using totalSizeInFrame_ %{public}d spanSizeInFrame_ %{public}d byteSizePerFrame_ %{public}d "
        "spanSizeInByte_ %{public}zu, spanSizeInMs_ %{public}u", totalSizeInFrame_, spanSizeInFrame_,
        byteSizePerFrame_, spanSizeInByte_, spanSizeInMs_);

    callbackBuffer_ = std::make_unique<uint8_t[]>(clientSpanSizeInByte_);
    CHECK_AND_RETURN_RET_LOG(callbackBuffer_ != nullptr, false, "Init callbackBuffer_ failed.");
    memset_s(callbackBuffer_.get(), clientSpanSizeInByte_, 0, clientSpanSizeInByte_);
    AUDIO_INFO_LOG("CallbackBufferSize is %{public}zu", clientSpanSizeInByte_);

    return true;
}

static size_t GetFormatSize(const AudioStreamInfo &info)
{
    size_t result = 0;
    size_t bitWidthSize = 0;
    switch (info.format) {
        case SAMPLE_U8:
            bitWidthSize = 1; // size is 1
            break;
        case SAMPLE_S16LE:
            bitWidthSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            bitWidthSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            bitWidthSize = 4; // size is 4
            break;
        case SAMPLE_F32LE:
            bitWidthSize = 4; // size is 4
            break;
        default:
            bitWidthSize = 2; // size is 2
            break;
    }

    size_t channelSize = 0;
    switch (info.channels) {
        case MONO:
            channelSize = 1; // size is 1
            break;
        case STEREO:
            channelSize = 2; // size is 2
            break;
        default:
            channelSize = 2; // size is 2
            break;
    }
    result = bitWidthSize * channelSize;
    return result;
}

void AudioProcessInClientInner::InitPlaybackThread(std::weak_ptr<FastAudioStream> weakStream)
{
    logUtilsTag_ = "ProcessPlay::" + std::to_string(sessionId_);
#ifdef SUPPORT_LOW_LATENCY
    std::shared_ptr<FastAudioStream> fastStream = weakStream.lock();
    CHECK_AND_RETURN_LOG(fastStream != nullptr, "fast stream is null");
    fastStream->ResetCallbackLoopTid();
#endif
    std::unique_lock<std::mutex> statusLock(loopMutex_);
    callbackLoop_ = std::thread([this, weakStream] {
        bool keepRunning = true;
        uint64_t curWritePos = 0;
        std::shared_ptr<FastAudioStream> strongStream = weakStream.lock();
        strongStream->SetCallbackLoopTid(gettid());
        AUDIO_INFO_LOG("Callback loop of session %{public}u start", sessionId_);
        processProxy_->RegisterThreadPriority(
            gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(processConfig_.appInfo.appUid),
            METHOD_WRITE_OR_READ);
        // Callback loop
        while (keepRunning) {
            strongStream = weakStream.lock();
            // Check if FastAudioStream or AudioProcessInClientInner is already destroyed to avoid use after free.
            CHECK_AND_BREAK_LOG(strongStream != nullptr, "FastAudioStream destroyed, exit AudioPlayCb");
            // Main operation in callback loop
            keepRunning = ProcessCallbackFuc(curWritePos);
        }
    });
    pthread_setname_np(callbackLoop_.native_handle(), "OS_AudioPlayCb");
}

void AudioProcessInClientInner::InitRecordThread(std::weak_ptr<FastAudioStream> weakStream)
{
    logUtilsTag_ = "ProcessRec::" + std::to_string(sessionId_);
#ifdef SUPPORT_LOW_LATENCY
    std::shared_ptr<FastAudioStream> fastStream = weakStream.lock();
    CHECK_AND_RETURN_LOG(fastStream != nullptr, "fast stream is null");
    fastStream->ResetCallbackLoopTid();
#endif
    callbackLoop_ = std::thread([this, weakStream] {
        bool keepRunning = true;
        uint64_t curReadPos = 0;
        int64_t clientReadCost = 0;
        std::shared_ptr<FastAudioStream> strongStream = weakStream.lock();
        strongStream->SetCallbackLoopTid(gettid());
        AUDIO_INFO_LOG("Callback loop of session %{public}u start", sessionId_);
        processProxy_->RegisterThreadPriority(
            gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(processConfig_.appInfo.appUid),
            METHOD_WRITE_OR_READ);
        // Callback loop
        while (keepRunning) {
            strongStream = weakStream.lock();
            // Check if FastAudioStream or AudioProcessInClientInner is already destroyed to avoid use after free.
            CHECK_AND_BREAK_LOG(strongStream != nullptr, "FastAudioStream destroyed, exit AudioPlayCb");
            // Main operation in callback loop
            keepRunning = RecordProcessCallbackFuc(curReadPos, clientReadCost);
        }
    });
    pthread_setname_np(callbackLoop_.native_handle(), "OS_AudioRecCb");
}

bool AudioProcessInClientInner::Init(const AudioProcessConfig &config, std::weak_ptr<FastAudioStream> weakStream)
{
    AUDIO_INFO_LOG("Call Init.");
    processConfig_ = config;
    if (!isVoipMmap_ && (config.streamInfo.format != targetStreamInfo_.format ||
        config.streamInfo.channels != targetStreamInfo_.channels)) {
        needConvert_ = true;
    }
    clientByteSizePerFrame_ = GetFormatSize(config.streamInfo);

    AUDIO_DEBUG_LOG("Using clientByteSizePerFrame_:%{public}zu", clientByteSizePerFrame_);
    bool isBufferInited = InitAudioBuffer();
    CHECK_AND_RETURN_RET_LOG(isBufferInited, isBufferInited, "%{public}s init audio buffer fail.", __func__);

    bool ret = handleTimeModel_.ConfigSampleRate(processConfig_.streamInfo.samplingRate);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "Init LinearPosTimeModel failed.");
    uint64_t handlePos = 0;
    int64_t handleTime = 0;
    audioBuffer_->GetHandleInfo(handlePos, handleTime);
    handleTimeModel_.ResetFrameStamp(handlePos, handleTime);

    streamStatus_->store(StreamStatus::STREAM_IDEL);

    if (config.audioMode == AUDIO_MODE_RECORD) {
        InitRecordThread(weakStream);
    } else {
        InitPlaybackThread(weakStream);
    }

    int waitThreadStartTime = 5; // wait for thread start.
    while (threadStatus_.load() == INVALID) {
        AUDIO_DEBUG_LOG("%{public}s wait %{public}d ms for %{public}s started...", __func__, waitThreadStartTime,
            config.audioMode == AUDIO_MODE_RECORD ? "RecordProcessCallbackFuc" : "ProcessCallbackFuc");
        ClockTime::RelativeSleep(ONE_MILLISECOND_DURATION * waitThreadStartTime);
    }

    isInited_ = true;
    return true;
}

int32_t AudioProcessInClientInner::SaveDataCallback(const std::shared_ptr<AudioDataCallback> &dataCallback)
{
    AUDIO_INFO_LOG("%{public}s enter.", __func__);
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    CHECK_AND_RETURN_RET_LOG(dataCallback != nullptr, ERR_INVALID_PARAM,
        "data callback is null.");
    audioDataCallback_ = dataCallback;
    return SUCCESS;
}

int32_t AudioProcessInClientInner::SaveUnderrunCallback(const std::shared_ptr<ClientUnderrunCallBack> &underrunCallback)
{
    AUDIO_INFO_LOG("%{public}s enter.", __func__);
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    CHECK_AND_RETURN_RET_LOG(underrunCallback != nullptr, ERR_INVALID_PARAM,
        "underrun callback is null.");
    underrunCallback_ = underrunCallback;
    return SUCCESS;
}

int32_t AudioProcessInClientInner::ReadFromProcessClient() const
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, ERR_INVALID_HANDLE,
        "%{public}s audio buffer is null.", __func__);
    uint64_t curReadPos = audioBuffer_->GetCurReadFrame();
    Trace trace("AudioProcessInClient::ReadProcessData-<" + std::to_string(curReadPos));
    RingBufferWrapper ringBuffer;
    int32_t ret = audioBuffer_->GetAllReadableBufferFromPosFrame(curReadPos, ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && (ringBuffer.dataLength >= spanSizeInByte_),
        ERR_OPERATION_FAILED, "get client mmap read buffer failed, ret %{public}d.", ret);
    ringBuffer.dataLength = spanSizeInByte_;

    ret = RingBufferWrapper{
        .basicBufferDescs = {{
            {.buffer = callbackBuffer_.get(), .bufLength = spanSizeInByte_},
            {.buffer = nullptr, .bufLength = 0}}},
        .dataLength = spanSizeInByte_}.CopyInputBufferValueToCurBuffer(ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "%{public}s memcpy fail, ret %{public}d,"
        " spanSizeInByte %{public}zu.", __func__, ret, spanSizeInByte_);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(callbackBuffer_.get()), spanSizeInByte_);
    VolumeTools::DfxOperation(BufferDesc{.buffer = callbackBuffer_.get(),
        .bufLength = spanSizeInByte_, .dataLength = spanSizeInByte_},
        processConfig_.streamInfo, logUtilsTag_, volumeDataCount_);

    ringBuffer.SetBuffersValueWithSpecifyDataLen(0);
    return SUCCESS;
}

// the buffer will be used by client
int32_t AudioProcessInClientInner::GetBufferDesc(BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "%{public}s not inited!", __func__);
    Trace trace("AudioProcessInClient::GetBufferDesc");

    if (processConfig_.audioMode == AUDIO_MODE_RECORD) {
        ReadFromProcessClient();
    }

    bufDesc.buffer = callbackBuffer_.get();
    bufDesc.dataLength = clientSpanSizeInByte_;
    bufDesc.bufLength = clientSpanSizeInByte_;
    return SUCCESS;
}

// only support convert to SAMPLE_S32LE STEREO
bool AudioProcessInClientInner::ChannelFormatS32Convert(const AudioStreamData &srcData, const AudioStreamData &dstData)
{
    Trace traceConvert("APIC::ChannelFormatS32Convert");
    if (srcData.streamInfo.samplingRate != dstData.streamInfo.samplingRate ||
        srcData.streamInfo.encoding != dstData.streamInfo.encoding) {
        return false;
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == STEREO) {
        return FormatConverter::S16StereoToS32Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::S16MonoToS32Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_S32LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::S32MonoToS32Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_S32LE && srcData.streamInfo.channels == STEREO) {
        return true; // no need convert, copy is done in NoFormatConvert:CopyData
    }
    if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::F32MonoToS32Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == STEREO) {
        return FormatConverter::F32StereoToS32Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }

    return false;
}

bool AudioProcessInClient::CheckIfSupport(const AudioProcessConfig &config)
{
    if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        config.rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        return true;
    }

    if (config.streamInfo.samplingRate != SAMPLE_RATE_48000) {
        return false;
    }

    if (config.streamInfo.encoding != ENCODING_PCM) {
        return false;
    }

    if (config.streamInfo.format != SAMPLE_S16LE && config.streamInfo.format != SAMPLE_S32LE &&
        config.streamInfo.format != SAMPLE_F32LE) {
        return false;
    }

    if (config.streamInfo.channels != MONO && config.streamInfo.channels != STEREO) {
        return false;
    }
    return true;
}

// only support convert to SAMPLE_S16LE STEREO
bool AudioProcessInClientInner::ChannelFormatS16Convert(const AudioStreamData &srcData, const AudioStreamData &dstData)
{
    if (srcData.streamInfo.samplingRate != dstData.streamInfo.samplingRate ||
        srcData.streamInfo.encoding != dstData.streamInfo.encoding) {
        return false;
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == STEREO) {
        return true; // no need convert, copy is done in NoFormatConvert:CopyData
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::S16MonoToS16Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_S32LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::S32MonoToS16Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_S32LE && srcData.streamInfo.channels == STEREO) {
        return FormatConverter::S32StereoToS16Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == MONO) {
        return FormatConverter::F32MonoToS16Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }
    if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == STEREO) {
        return FormatConverter::F32StereoToS16Stereo(srcData.bufferDesc, dstData.bufferDesc) == 0;
    }

    return false;
}

void AudioProcessInClientInner::CopyWithVolume(const BufferDesc &srcDesc, const BufferDesc &dstDesc) const
{
    size_t len = dstDesc.dataLength;
    len /= 2; // SAMPLE_S16LE--> 2 byte
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    for (size_t pos = 0; len > 0; len--) {
        int32_t sum = 0;
        int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer) + pos;
        sum += (*srcPtr * static_cast<int64_t>(processVolume_ * duckVolumeInFloat_ *
            muteVolumeInFloat_)) >> VOLUME_SHIFT_NUMBER; // 1/65536
        pos++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
}

void AudioProcessInClientInner::ProcessVolume(const AudioStreamData &targetData) const
{
    size_t half = 2;
    size_t len = targetData.bufferDesc.dataLength;
    len /= half;
    int16_t *dstPtr = reinterpret_cast<int16_t *>(targetData.bufferDesc.buffer);
    for (; len > 0; len--) {
        int32_t sum = 0;
        sum += (*dstPtr * static_cast<int64_t>(processVolume_ * duckVolumeInFloat_ *
            muteVolumeInFloat_)) >> VOLUME_SHIFT_NUMBER;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
}

int32_t AudioProcessInClientInner::ProcessData(const BufferDesc &srcDesc, const BufferDesc &dstDesc) const
{
    int32_t ret = 0;
    if (!needConvert_) {
        Trace traceNoConvert("APIC::NoFormatConvert:CopyData");
        AudioBufferHolder bufferHolder = audioBuffer_->GetBufferHolder();
        if (bufferHolder == AudioBufferHolder::AUDIO_SERVER_INDEPENDENT) {
            CopyWithVolume(srcDesc, dstDesc);
        } else {
            ret = memcpy_s(static_cast<void *>(dstDesc.buffer), dstDesc.dataLength,
                static_cast<void *>(srcDesc.buffer), srcDesc.dataLength);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Copy data failed!");
        }
        return SUCCESS;
    }

    // need convert
    Trace traceConvert("APIC::FormatConvert");
    AudioStreamData srcData = {processConfig_.streamInfo, srcDesc, 0, 0};
    AudioStreamData dstData = {targetStreamInfo_, dstDesc, 0, 0};
    bool succ = false;
    if (targetStreamInfo_.format == SAMPLE_S16LE) {
        succ = ChannelFormatS16Convert(srcData, dstData);
    } else if (targetStreamInfo_.format == SAMPLE_S32LE) {
        succ = ChannelFormatS32Convert(srcData, dstData);
    }
    CHECK_AND_RETURN_RET_LOG(succ, ERR_OPERATION_FAILED, "Convert data failed!");
    AudioBufferHolder bufferHolder = audioBuffer_->GetBufferHolder();
    if (bufferHolder == AudioBufferHolder::AUDIO_SERVER_INDEPENDENT) {
        ProcessVolume(dstData);
    }

    return SUCCESS;
}

int32_t AudioProcessInClientInner::ProcessData(const BufferDesc &srcDesc, const RingBufferWrapper &dstDesc)
{
    BufferDesc tmpDstDesc;
    if (dstDesc.dataLength <= dstDesc.basicBufferDescs[0].bufLength) {
        tmpDstDesc.buffer = dstDesc.basicBufferDescs[0].buffer;
        tmpDstDesc.dataLength = dstDesc.dataLength;
        tmpDstDesc.bufLength = dstDesc.dataLength;
        return ProcessData(srcDesc, tmpDstDesc);
    }

    std::lock_guard lock(tmpBufferMutex_);
    tmpBuffer_.resize(0);
    tmpBuffer_.resize(dstDesc.dataLength);
    tmpDstDesc.buffer = tmpBuffer_.data();
    tmpDstDesc.dataLength = dstDesc.dataLength;
    tmpDstDesc.bufLength = dstDesc.dataLength;
    int32_t ret = ProcessData(srcDesc, tmpDstDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ProcessData failed!");
    RingBufferWrapper ringBufferDescForCotinueData;
    ringBufferDescForCotinueData.dataLength = tmpDstDesc.dataLength;
    ringBufferDescForCotinueData.basicBufferDescs[0].buffer = tmpDstDesc.buffer;
    ringBufferDescForCotinueData.basicBufferDescs[0].bufLength = tmpDstDesc.dataLength;
    RingBufferWrapper(dstDesc).CopyInputBufferValueToCurBuffer(ringBufferDescForCotinueData);
    return SUCCESS;
}

void AudioProcessInClientInner::WaitForWritableSpace()
{
    FutexCode futexRes = FUTEX_OPERATION_FAILED;
    int64_t timeout = FAST_WRITE_CACHE_TIMEOUT_IN_MS;
    futexRes = audioBuffer_->WaitFor(timeout * AUDIO_US_PER_SECOND,
        [this] () {
            return (streamStatus_->load() != StreamStatus::STREAM_RUNNING) ||
                (static_cast<uint32_t>(audioBuffer_->GetWritableDataFrames()) > 0);
        });
    if (futexRes != SUCCESS) {
        AUDIO_ERR_LOG("futex err: %{public}d", futexRes);
    }
}

int32_t AudioProcessInClientInner::WriteDataChunk(const BufferDesc &bufDesc, size_t clientRemainSizeInFrame)
{
    RingBufferWrapper inBuffer = {
        .basicBufferDescs = {{
            {.buffer = bufDesc.buffer, .bufLength = clientRemainSizeInFrame * clientByteSizePerFrame_},
            {.buffer = nullptr, .bufLength = 0}
        }},
        .dataLength = clientRemainSizeInFrame * clientByteSizePerFrame_
    };

    while (clientRemainSizeInFrame > 0) {
        WaitForWritableSpace();

        uint64_t curWritePos = audioBuffer_->GetCurWriteFrame();
        Trace writeProcessDataTrace("AudioProcessInClient::WriteProcessData->" + std::to_string(curWritePos));
        RingBufferWrapper curWriteBuffer;
        int32_t ret = audioBuffer_->GetAllWritableBufferFromPosFrame(curWritePos, curWriteBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && curWriteBuffer.dataLength > 0,
            ERR_OPERATION_FAILED, "get write buffer fail, ret:%{public}d", ret);

        size_t copySizeInFrame = std::min(clientRemainSizeInFrame, (curWriteBuffer.dataLength / byteSizePerFrame_));
        CHECK_AND_RETURN_RET_LOG(copySizeInFrame > 0, ERR_OPERATION_FAILED, "copysize is 0");

        BufferDesc curCallbackBuffer = {nullptr, 0, 0};
        curCallbackBuffer.buffer = inBuffer.basicBufferDescs[0].buffer;
        curCallbackBuffer.bufLength = inBuffer.dataLength;
        curCallbackBuffer.dataLength = inBuffer.dataLength;

        curWriteBuffer.dataLength = copySizeInFrame * byteSizePerFrame_;
        ret = ProcessData(curCallbackBuffer, curWriteBuffer);
        audioBuffer_->SetCurWriteFrame((curWritePos + copySizeInFrame), false);
        if (ret != SUCCESS) {
            return ERR_OPERATION_FAILED;
        }
        writeProcessDataTrace.End();
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(curCallbackBuffer.buffer),
            curCallbackBuffer.dataLength);
        VolumeTools::DfxOperation(curCallbackBuffer, processConfig_.streamInfo, logUtilsTag_, volumeDataCount_);

        clientRemainSizeInFrame -= copySizeInFrame;
        inBuffer.SeekFromStart(copySizeInFrame * clientByteSizePerFrame_);
    }

    return SUCCESS;
}

bool AudioProcessInClientInner::WaitIfBufferEmpty(const BufferDesc &bufDesc)
{
    if (bufDesc.dataLength == 0) {
        const uint32_t sleepTimeUs = isVoipMmap_ ? VOIP_WAIT_FOR_NEXT_CB_US : FAST_WAIT_FOR_NEXT_CB_US;
        AUDIO_WARNING_LOG("%{public}u", sleepTimeUs);
        usleep(sleepTimeUs);
        return false;
    }
    return true;
}

int32_t AudioProcessInClientInner::Enqueue(const BufferDesc &bufDesc)
{
    Trace trace("AudioProcessInClient::Enqueue");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr && bufDesc.bufLength <= clientSpanSizeInByte_ &&
        bufDesc.dataLength <= bufDesc.bufLength, ERR_INVALID_PARAM,
        "bufDesc error, bufLen %{public}zu, dataLen %{public}zu, spanSize %{public}zu.",
        bufDesc.bufLength, bufDesc.dataLength, clientSpanSizeInByte_);
    // check if this buffer is form us.
    if (bufDesc.buffer != callbackBuffer_.get()) {
        AUDIO_WARNING_LOG("the buffer is not created by client.");
    }

    if (processConfig_.audioMode == AUDIO_MODE_RECORD) {
        if (memset_s(callbackBuffer_.get(), clientSpanSizeInByte_, 0, clientSpanSizeInByte_) != EOK) {
            AUDIO_WARNING_LOG("reset callback buffer fail.");
        }
        return SUCCESS;
    };

    CHECK_AND_RETURN_RET(WaitIfBufferEmpty(bufDesc), ERR_INVALID_PARAM);

    ExitStandByIfNeed();

    DoFadeInOut(bufDesc);

    CHECK_AND_RETURN_RET_LOG(clientByteSizePerFrame_ > 0 && byteSizePerFrame_ > 0, ERROR,
        "clientsizePerFrameInByte :%{public}zu byteSizePerFrame_ :%{public}u",
        clientByteSizePerFrame_, byteSizePerFrame_);
    size_t clientRemainSizeInFrame = bufDesc.dataLength / clientByteSizePerFrame_;

    int32_t ret = WriteDataChunk(bufDesc, clientRemainSizeInFrame);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "writedataChunk failed, err: %{public}d", ret);

    if (memset_s(callbackBuffer_.get(), clientSpanSizeInByte_, 0, clientSpanSizeInByte_) != EOK) {
        AUDIO_WARNING_LOG("reset callback buffer fail.");
    }

    return SUCCESS;
}

int32_t AudioProcessInClientInner::SetVolume(int32_t vol)
{
    AUDIO_INFO_LOG("proc client mode %{public}d to %{public}d.", processConfig_.audioMode, vol);
    Trace trace("AudioProcessInClient::SetVolume " + std::to_string(vol));
    CHECK_AND_RETURN_RET_LOG(vol >= 0 && vol <= PROCESS_VOLUME_MAX, ERR_INVALID_PARAM,
        "SetVolume failed, invalid volume:%{public}d", vol);
    processVolume_ = vol;
    return SUCCESS;
}

int32_t AudioProcessInClientInner::Start()
{
    Trace traceStart("AudioProcessInClient::Start");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    AudioSamplingRate samplingRate = processConfig_.streamInfo.samplingRate;
    AudioSampleFormat format = processConfig_.streamInfo.format;
    AudioChannel channels = processConfig_.streamInfo.channels;
    // eg: 100005_dump_process_client_audio_48000_2_1.pcm
    std::string dumpFileName = std::to_string(sessionId_) + "_dump_process_client_audio_" +
        std::to_string(samplingRate) + '_' + std::to_string(channels) + '_' + std::to_string(format) +
        ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);

    std::lock_guard<std::mutex> lock(statusSwitchLock_);
    if (streamStatus_->load() == StreamStatus::STREAM_RUNNING) {
        AUDIO_INFO_LOG("Start find already started");
        return SUCCESS;
    }

    startFadein_.store(true);
    StreamStatus targetStatus = StreamStatus::STREAM_IDEL;
    bool ret = streamStatus_->compare_exchange_strong(targetStatus, StreamStatus::STREAM_STARTING);
    if (!ret) {
        startFadein_.store(false);
    }
    CHECK_AND_RETURN_RET_LOG(
        ret, ERR_ILLEGAL_STATE, "Start failed, invalid status: %{public}s", GetStatusInfo(targetStatus).c_str());

    if (processProxy_->Start() != SUCCESS) {
        streamStatus_->store(StreamStatus::STREAM_IDEL);
        AUDIO_ERR_LOG("Start failed to call process proxy, reset status to IDEL.");
        startFadein_.store(false);
        threadStatusCV_.notify_all();
        return ERR_OPERATION_FAILED;
    }
    UpdateHandleInfo();
    streamStatus_->store(StreamStatus::STREAM_RUNNING);
    threadStatusCV_.notify_all();
    return SUCCESS;
}

int32_t AudioProcessInClientInner::Pause(bool isFlush)
{
    Trace tracePause("AudioProcessInClient::Pause");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    std::lock_guard<std::mutex> lock(statusSwitchLock_);
    if (streamStatus_->load() == StreamStatus::STREAM_PAUSED) {
        AUDIO_INFO_LOG("Pause find already paused");
        return SUCCESS;
    }
    startFadeout_.store(true);
    StreamStatus targetStatus = StreamStatus::STREAM_RUNNING;

    if (streamStatus_->load() == StreamStatus::STREAM_STAND_BY) {
        targetStatus = StreamStatus::STREAM_STAND_BY;
    }
    bool ret = streamStatus_->compare_exchange_strong(targetStatus, StreamStatus::STREAM_PAUSING);
    if (!ret) {
        startFadeout_.store(false);
    }
    CHECK_AND_RETURN_RET_LOG(
        ret, ERR_ILLEGAL_STATE, "Pause failed, invalid status : %{public}s", GetStatusInfo(targetStatus).c_str());
    ClockTime::RelativeSleep(MAX_STOP_FADING_DURATION_NANO);
    if (processProxy_->Pause(isFlush) != SUCCESS) {
        streamStatus_->store(StreamStatus::STREAM_RUNNING);
        AUDIO_ERR_LOG("Pause failed to call process proxy, reset status to RUNNING");
        startFadeout_.store(false);
        threadStatusCV_.notify_all(); // avoid thread blocking with status PAUSING
        return ERR_OPERATION_FAILED;
    }
    startFadeout_.store(false);
    streamStatus_->store(StreamStatus::STREAM_PAUSED);

    audioBuffer_->WakeFutex();

    lastPausedTime_ = ClockTime::GetCurNano();

    return SUCCESS;
}

int32_t AudioProcessInClientInner::Resume()
{
    Trace traceResume("AudioProcessInClient::Resume");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    std::lock_guard<std::mutex> lock(statusSwitchLock_);

    if (streamStatus_->load() == StreamStatus::STREAM_RUNNING) {
        AUDIO_INFO_LOG("Resume find already running");
        return SUCCESS;
    }

    startFadein_.store(true);

    StreamStatus pausedStatus = StreamStatus::STREAM_PAUSED;
    StreamStatus stoppedStatus = StreamStatus::STREAM_STOPPED;
    if (!streamStatus_->compare_exchange_strong(pausedStatus, StreamStatus::STREAM_STARTING) &&
        !streamStatus_->compare_exchange_strong(stoppedStatus, StreamStatus::STREAM_STARTING)) {
        startFadein_.store(false);
        AUDIO_ERR_LOG("Resume failed, invalid status : %{public}s", GetStatusInfo(stoppedStatus).c_str());
        return ERR_ILLEGAL_STATE;
    }

    if (processProxy_->Resume() != SUCCESS) {
        streamStatus_->store(StreamStatus::STREAM_PAUSED);
        AUDIO_ERR_LOG("Resume failed to call process proxy, reset status to PAUSED.");
        startFadein_.store(false);
        threadStatusCV_.notify_all();
        return ERR_OPERATION_FAILED;
    }

    if (ClockTime::GetCurNano() - lastPausedTime_ > DELAY_RESYNC_TIME) {
        UpdateHandleInfo(false, true);
        lastPausedTime_ = INT64_MAX;
    } else {
        UpdateHandleInfo();
    }

    streamStatus_->store(StreamStatus::STREAM_RUNNING);
    threadStatusCV_.notify_all();

    return SUCCESS;
}

int32_t AudioProcessInClientInner::Stop(AudioProcessStage stage)
{
    Trace traceStop("AudioProcessInClient::Stop");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    std::lock_guard<std::mutex> lock(statusSwitchLock_);
    if (streamStatus_->load() == StreamStatus::STREAM_STOPPED) {
        AUDIO_INFO_LOG("Stop find already stopped");
        return SUCCESS;
    }

    StreamStatus oldStatus = streamStatus_->load();
    CHECK_AND_RETURN_RET_LOG(oldStatus != STREAM_IDEL && oldStatus != STREAM_RELEASED && oldStatus != STREAM_INVALID,
        ERR_ILLEGAL_STATE, "Stop failed, invalid status : %{public}s", GetStatusInfo(oldStatus).c_str());
    if (oldStatus == STREAM_STARTING || oldStatus == STREAM_RUNNING) {
        startFadeout_.store(true);
    }
    streamStatus_->store(StreamStatus::STREAM_STOPPING);

    ClockTime::RelativeSleep(MAX_STOP_FADING_DURATION_NANO);

    processProxy_->SetUnderrunCount(underflowCount_);
    if (processProxy_->Stop(stage) != SUCCESS) {
        streamStatus_->store(oldStatus);
        AUDIO_ERR_LOG("Stop failed in server, reset status to %{public}s", GetStatusInfo(oldStatus).c_str());
        startFadeout_.store(false);
        threadStatusCV_.notify_all(); // avoid thread blocking with status RUNNING
        return ERR_OPERATION_FAILED;
    }
    startFadeout_.store(false);
    streamStatus_->store(StreamStatus::STREAM_STOPPED);

    audioBuffer_->WakeFutex();

    AUDIO_INFO_LOG("Success stop proc client mode %{public}d form %{public}s.",
        processConfig_.audioMode, GetStatusInfo(oldStatus).c_str());
    return SUCCESS;
}

void AudioProcessInClientInner::JoinCallbackLoop()
{
    std::unique_lock<std::mutex> statusLock(loopMutex_);
    if (callbackLoop_.joinable()) {
        std::unique_lock<std::mutex> lock(loopThreadLock_);
        isCallbackLoopEnd_ = true; // change it with lock to break the loop
        threadStatusCV_.notify_all();
        lock.unlock(); // should call unlock before join
        audioBuffer_->WakeFutex(IS_PRE_EXIT);
        callbackLoop_.join();
    }
}

int32_t AudioProcessInClientInner::Release(bool isSwitchStream)
{
    Trace traceRelease("AudioProcessInClient::Release");
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    AUDIO_INFO_LOG("AudioProcessInClientInner::Release()");
    // not lock as status is already released
    if (streamStatus_->load() == StreamStatus::STREAM_RELEASED) {
        AUDIO_INFO_LOG("Stream status is already released");
        return SUCCESS;
    }
    Stop(AudioProcessStage::AUDIO_PROC_STAGE_STOP_BY_RELEASE);
    isCallbackLoopEnd_ = true;
    threadStatusCV_.notify_all();
    std::lock_guard<std::mutex> lock(statusSwitchLock_);
    StreamStatus currentStatus = streamStatus_->load();
    if (currentStatus != STREAM_STOPPED) {
        AUDIO_WARNING_LOG("Release in currentStatus:%{public}s", GetStatusInfo(currentStatus).c_str());
    }

    if (processProxy_->Release(isSwitchStream) != SUCCESS) {
        AUDIO_ERR_LOG("Release may failed in server");
        threadStatusCV_.notify_all(); // avoid thread blocking with status RUNNING
        return ERR_OPERATION_FAILED;
    }

    streamStatus_->store(StreamStatus::STREAM_RELEASED);

    audioBuffer_->WakeFutex();

    AUDIO_INFO_LOG("Success release proc client mode %{public}d.", processConfig_.audioMode);
    isInited_ = false;

    return SUCCESS;
}

// client should call GetBufferDesc and Enqueue in OnHandleData
void AudioProcessInClientInner::CallClientHandleCurrent()
{
    Trace trace("AudioProcessInClient::CallClientHandleCurrent");
    std::shared_ptr<AudioDataCallback> cb = audioDataCallback_.lock();
    CHECK_AND_RETURN_LOG(cb != nullptr, "audio data callback is null.");

    int64_t stamp = ClockTime::GetCurNano();
    cb->OnHandleData(clientSpanSizeInByte_);
    stamp = ClockTime::GetCurNano() - stamp;
    if (stamp > MAX_WRITE_COST_DURATION_NANO) {
        if (processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
            underflowCount_++;
        } else {
            overflowCount_++;
        }
        // todo
        // handle write time out: send underrun msg to client, reset time model with latest server handle time.
    }

    int64_t limit = isVoipMmap_ ? VOIP_MILLISECOND_DURATION : MAX_WRITE_COST_DURATION_NANO;
    if (stamp + ONE_MILLISECOND_DURATION > limit) {
        AUDIO_WARNING_LOG("Client handle cb too slow, cost %{public}" PRId64"us", stamp / AUDIO_MS_PER_SECOND);
    }
}

void AudioProcessInClientInner::UpdateHandleInfo(bool isAysnc, bool resetReadWritePos)
{
    Trace traceSync("AudioProcessInClient::UpdateHandleInfo");
    uint64_t serverHandlePos = 0;
    int64_t serverHandleTime = 0;
    CHECK_AND_RETURN_LOG(processProxy_ != nullptr, "processProxy_ is nullptr");
    int32_t ret = isAysnc ? processProxy_->RequestHandleInfoAsync() : processProxy_->RequestHandleInfo();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RequestHandleInfo failed ret:%{public}d", ret);
    audioBuffer_->GetHandleInfo(serverHandlePos, serverHandleTime);

    CheckPosTimeRes res = handleTimeModel_.UpdataFrameStamp(serverHandlePos, serverHandleTime);
    if (res == CHECK_FAILED) {
        handleTimeModel_.ResetFrameStamp(serverHandlePos, serverHandleTime);
    }

    if (resetReadWritePos) {
        uint64_t nextWritePos = serverHandlePos + spanSizeInFrame_;
        ret = audioBuffer_->ResetCurReadWritePos(nextWritePos, nextWritePos, false);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed ret:%{public}d", ret);
    }
}

int64_t AudioProcessInClientInner::GetPredictNextHandleTime(uint64_t posInFrame, bool isIndependent)
{
    Trace trace("AudioProcessInClient::GetPredictNextRead");
    CHECK_AND_RETURN_RET_LOG(spanSizeInFrame_ != 0, 0, "spanSizeInFrame is 0.");
    uint64_t handleSpanCnt = 0;
    if (spanSizeInFrame_ != 0) {
        handleSpanCnt = posInFrame / spanSizeInFrame_;
    }
    uint32_t startPeriodCnt = 20; // sync each time when start
    uint32_t oneBigPeriodCnt = 40; // 200ms
    if (isIndependent) {
        if (handleSpanCnt % oneBigPeriodCnt == 0) {
            UpdateHandleInfo(true);
        }
    } else {
        if (handleSpanCnt < startPeriodCnt || handleSpanCnt % oneBigPeriodCnt == 0) {
            UpdateHandleInfo();
        }
    }

    int64_t nextHandleTime = handleTimeModel_.GetTimeOfPos(posInFrame);

    return nextHandleTime;
}

void AudioProcessInClientInner::CallExitStandBy()
{
    Trace trace("AudioProcessInClient::CallExitStandBy::" + std::to_string(sessionId_));
    int32_t result = processProxy_->Start();
    StreamStatus targetStatus = StreamStatus::STREAM_STARTING;
    bool ret = streamStatus_->compare_exchange_strong(targetStatus, StreamStatus::STREAM_RUNNING);
    AUDIO_INFO_LOG("Call start result:%{public}d  status change: %{public}s", result, ret ? "success" : "fail");
    UpdateHandleInfo();
}

std::string AudioProcessInClientInner::GetStatusInfo(StreamStatus status)
{
    switch (status) {
        case STREAM_IDEL:
            return "STREAM_IDEL";
        case STREAM_STARTING:
            return "STREAM_STARTING";
        case STREAM_RUNNING:
            return "STREAM_RUNNING";
        case STREAM_PAUSING:
            return "STREAM_PAUSING";
        case STREAM_PAUSED:
            return "STREAM_PAUSED";
        case STREAM_STOPPING:
            return "STREAM_STOPPING";
        case STREAM_STOPPED:
            return "STREAM_STOPPED";
        case STREAM_RELEASED:
            return "STREAM_RELEASED";
        case STREAM_INVALID:
            return "STREAM_INVALID";
        default:
            break;
    }
    return "NO_SUCH_STATUS";
}

void AudioProcessInClientInner::ExitStandByIfNeed()
{
    if (streamStatus_->load() == STREAM_STAND_BY) {
        AUDIO_INFO_LOG("Status is STAND_BY, let's call exit!");
        CallExitStandBy();
    }
}

bool AudioProcessInClientInner::KeepLoopRunning()
{
    StreamStatus targetStatus = STREAM_INVALID;

    switch (streamStatus_->load()) {
        case STREAM_RUNNING:
            return true;
        case STREAM_STAND_BY:
            return true;
        case STREAM_STARTING:
            targetStatus = STREAM_RUNNING;
            break;
        case STREAM_IDEL:
            targetStatus = STREAM_STARTING;
            break;
        case STREAM_PAUSING:
            targetStatus = STREAM_PAUSED;
            break;
        case STREAM_PAUSED:
            targetStatus = STREAM_STARTING;
            break;
        case STREAM_STOPPING:
            targetStatus = STREAM_STOPPED;
            break;
        case STREAM_STOPPED:
            targetStatus = STREAM_RELEASED;
            break;
        default:
            break;
    }

    if (startFadeout_.load() &&
        (targetStatus == STREAM_PAUSED || targetStatus == STREAM_STOPPED || targetStatus == STREAM_RELEASED)) {
        // do one more time to prepare fade out span buffer
        return true;
    }

    Trace trace("AudioProcessInClient::InWaitStatus");
    std::unique_lock<std::mutex> lock(loopThreadLock_);
    AUDIO_DEBUG_LOG("Process status is %{public}s now, wait for %{public}s...",
        GetStatusInfo(streamStatus_->load()).c_str(), GetStatusInfo(targetStatus).c_str());
    threadStatus_ = WAITTING;
    threadStatusCV_.wait(lock);
    AUDIO_DEBUG_LOG("Process wait end. Cur is %{public}s now, target is %{public}s...",
        GetStatusInfo(streamStatus_->load()).c_str(), GetStatusInfo(targetStatus).c_str());

    return false;
}

bool AudioProcessInClientInner::RecordProcessCallbackFuc(uint64_t &curReadPos, int64_t clientReadCost)
{
    if (isCallbackLoopEnd_ || audioBuffer_ == nullptr) {
        return false;
    }
    if (!KeepLoopRunning()) {
        return true;
    }
    threadStatus_ = INRUNNING;
    Trace traceLoop("AudioProcessInClient Record InRunning");
    if (needReSyncPosition_ && RecordReSyncServicePos() == SUCCESS) {
        needReSyncPosition_ = false;
        return true;
    }

    if (!CheckAndWaitBufferReadyForRecord()) {
        return true;
    }

    if (streamStatus_->load() != StreamStatus::STREAM_RUNNING) {
        return true;
    }

    CallClientHandleCurrent();

    curReadPos = audioBuffer_->GetCurReadFrame();
    int32_t recordFinish = RecordFinishHandleCurrent(curReadPos, clientReadCost);
    CHECK_AND_RETURN_RET_LOG(recordFinish == SUCCESS, true, "finish handle current fail.");

    threadStatus_ = SLEEPING;
    return true;
}

int32_t AudioProcessInClientInner::RecordReSyncServicePos()
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr && audioBuffer_ != nullptr, ERR_INVALID_HANDLE,
        "%{public}s process proxy or audio buffer is null.", __func__);
    uint64_t serverHandlePos = 0;
    int64_t serverHandleTime = 0;
    int32_t tryTimes = 3;
    int32_t ret = 0;
    while (tryTimes > 0) {
        ret = processProxy_->RequestHandleInfoAsync();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s request handle info fail, ret %{public}d.",
            __func__, ret);

        CHECK_AND_RETURN_RET_LOG(audioBuffer_->GetHandleInfo(serverHandlePos, serverHandleTime), ERR_OPERATION_FAILED,
            "%{public}s get handle info fail.", __func__);
        if (serverHandlePos > 0) {
            break;
        }
        ClockTime::RelativeSleep(MAX_READ_COST_DURATION_NANO);
        tryTimes--;
    }
    AUDIO_INFO_LOG("%{public}s get handle info OK, tryTimes %{public}d, serverHandlePos %{public}" PRIu64", "
        "serverHandleTime %{public}" PRId64".", __func__, tryTimes, serverHandlePos, serverHandleTime);
    ClockTime::AbsoluteSleep(serverHandleTime + RECORD_HANDLE_DELAY_NANO);

    ret = audioBuffer_->SetCurReadFrame(serverHandlePos, false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s set curReadPos fail, ret %{public}d.", __func__, ret);
    return SUCCESS;
}

int32_t AudioProcessInClientInner::RecordFinishHandleCurrent(uint64_t &curReadPos, int64_t &clientReadCost)
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, ERR_INVALID_HANDLE,
        "%{public}s audio buffer is null.", __func__);

    uint64_t nextWritePos = curReadPos + spanSizeInFrame_;
    int32_t ret = audioBuffer_->SetCurReadFrame(nextWritePos, false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s set next hand frame %{public}" PRIu64" fail, "
        "ret %{public}d.", __func__, nextWritePos, ret);
    curReadPos = nextWritePos;

    return SUCCESS;
}

void AudioProcessInClientInner::DoFadeInOut(const BufferDesc &buffDesc)
{
    if (startFadein_.load() || startFadeout_.load()) {
        CHECK_AND_RETURN_LOG(buffDesc.buffer != nullptr, "audioBuffer_ is null.");

        bool isFadeOut = startFadeout_.load();

        BufferDesc fadeBufferDesc = buffDesc;
        size_t targetFadeSize = clientByteSizePerFrame_ * spanSizeInFrame_;
        if (isFadeOut && (fadeBufferDesc.dataLength > targetFadeSize)) {
            fadeBufferDesc.buffer += (fadeBufferDesc.dataLength - targetFadeSize);
            fadeBufferDesc.dataLength = targetFadeSize;
        }

        if (!isFadeOut && (fadeBufferDesc.dataLength > targetFadeSize)) {
            fadeBufferDesc.dataLength = targetFadeSize;
        }

        AudioChannel channel = processConfig_.streamInfo.channels;
        ChannelVolumes mapVols = isFadeOut ? VolumeTools::GetChannelVolumes(channel, 1.0f, 0.0f) :
            VolumeTools::GetChannelVolumes(channel, 0.0f, 1.0f);
        int32_t ret = VolumeTools::Process(fadeBufferDesc, processConfig_.streamInfo.format, mapVols);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("VolumeTools::Process failed: %{public}d", ret);
        }

        if (isFadeOut) {
            startFadeout_.store(false);
        } else {
            startFadein_.store(false);
        }
    }
}

bool AudioProcessInClientInner::IsRestoreNeeded()
{
    RestoreStatus restoreStatus = audioBuffer_->GetRestoreStatus();
    if (restoreStatus == NEED_RESTORE) {
        return true;
    }

    if (restoreStatus == NEED_RESTORE_TO_NORMAL) {
        return true;
    }

    return false;
}

bool AudioProcessInClientInner::CheckAndWaitBufferReadyForPlayback()
{
    FutexCode ret = audioBuffer_->WaitFor(FAST_WRITE_CACHE_TIMEOUT_IN_MS * AUDIO_US_PER_SECOND, [this] () {
        if (streamStatus_->load() != StreamStatus::STREAM_RUNNING) {
            return true;
        }

        if (IsRestoreNeeded()) {
            return true;
        }

        int32_t writableSizeInFrame = audioBuffer_->GetWritableDataFrames();
        if ((writableSizeInFrame > 0) && ((totalSizeInFrame_ - writableSizeInFrame) < spanSizeInFrame_)) {
            return true;
        }
        return false;
    });

    return (ret == FUTEX_SUCCESS);
}

bool AudioProcessInClientInner::CheckAndWaitBufferReadyForRecord()
{
    FutexCode ret = audioBuffer_->WaitFor(FAST_WRITE_CACHE_TIMEOUT_IN_MS * AUDIO_US_PER_SECOND, [this] () {
        if (streamStatus_->load() != StreamStatus::STREAM_RUNNING) {
            return true;
        }

        if (IsRestoreNeeded()) {
            return true;
        }

        int32_t writableSizeInFrame = audioBuffer_->GetWritableDataFrames();
        if ((writableSizeInFrame > 0) && ((totalSizeInFrame_ - writableSizeInFrame) >= spanSizeInFrame_)) {
            return true;
        }
        return false;
    });

    return (ret == FUTEX_SUCCESS);
}

bool AudioProcessInClientInner::ProcessCallbackFuc(uint64_t &curWritePos)
{
    if (isCallbackLoopEnd_ && !startFadeout_.load()) {
        return false;
    }
    if (!KeepLoopRunning()) {
        return true;
    }
    threadStatus_ = INRUNNING;
    Trace traceLoop("AudioProcessInClient::InRunning");
    curWritePos = audioBuffer_->GetCurWriteFrame();
    if (!CheckAndWaitBufferReadyForPlayback()) {
        return true;
    }

    auto status = streamStatus_->load();
    if (status != StreamStatus::STREAM_RUNNING && status != StreamStatus::STREAM_STAND_BY) {
        return true;
    }
    // call client write
    CallClientHandleCurrent();
    // client write done, check if time out

    traceLoop.End();
    // start safe sleep
    threadStatus_ = SLEEPING;
    return true;
}

bool AudioProcessInClientInner::KeepLoopRunningIndependent()
{
    switch (streamStatus_->load()) {
        case STREAM_RUNNING:
            return true;
        case STREAM_IDEL:
            return true;
        case STREAM_PAUSED:
            return true;
        default:
            break;
    }

    return false;
}

void AudioProcessInClientInner::CheckIfWakeUpTooLate(int64_t &curTime, int64_t &wakeUpTime)
{
    curTime = ClockTime::GetCurNano();
    int64_t wakeupCost = curTime - wakeUpTime;
    if (wakeupCost > ONE_MILLISECOND_DURATION) {
        if (wakeupCost > TWO_MILLISECOND_DURATION) {
            AUDIO_WARNING_LOG("loop wake up too late, cost %{public}" PRId64"us", wakeupCost / AUDIO_MS_PER_SECOND);
        }
        wakeUpTime = curTime;
    }
}

void AudioProcessInClientInner::CheckIfWakeUpTooLate(int64_t &curTime, int64_t &wakeUpTime, int64_t clientWriteCost)
{
    curTime = ClockTime::GetCurNano();
    int64_t round = static_cast<int64_t>(spanSizeInFrame_ == 0 ? 1 : clientSpanSizeInFrame_ / spanSizeInFrame_);
    int64_t clientBufferDurationInMs = static_cast<int64_t>(spanSizeInMs_) * ONE_MILLISECOND_DURATION * round;
    if (wakeUpTime - curTime > clientBufferDurationInMs + clientWriteCost) {
        Trace trace("BigWakeUpTime curTime[" + std::to_string(curTime) + "] target[" + std::to_string(wakeUpTime) +
            "] delay " + std::to_string(wakeUpTime - curTime) + "ns");
        AUDIO_PRERELEASE_LOGW("wakeUpTime is too late...");
    }
}

int32_t AudioProcessInClientInner::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce)
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr, ERR_OPERATION_FAILED, "set failed with null ipcProxy.");
    return processProxy_->SetDefaultOutputDevice(defaultOutputDevice, skipForce);
}

int32_t AudioProcessInClientInner::SetSilentModeAndMixWithOthers(bool on)
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr, ERR_OPERATION_FAILED, "ipcProxy is null.");
    return processProxy_->SetSilentModeAndMixWithOthers(on);
}

void AudioProcessInClientInner::GetRestoreInfo(RestoreInfo &restoreInfo)
{
    CHECK_AND_RETURN_LOG(audioBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    audioBuffer_->GetRestoreInfo(restoreInfo);
    return;
}

void AudioProcessInClientInner::SetRestoreInfo(RestoreInfo &restoreInfo)
{
    CHECK_AND_RETURN_LOG(audioBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    audioBuffer_->SetRestoreInfo(restoreInfo);
    return;
}

RestoreStatus AudioProcessInClientInner::CheckRestoreStatus()
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return audioBuffer_->CheckRestoreStatus();
}

RestoreStatus AudioProcessInClientInner::SetRestoreStatus(RestoreStatus restoreStatus)
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return audioBuffer_->SetRestoreStatus(restoreStatus);
}

void AudioProcessInClientInner::SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string adjustTime,
    uint32_t code)
{
    processProxy_->SaveAdjustStreamVolumeInfo(volume, sessionId, adjustTime, code);
}

int32_t AudioProcessInClientInner::RegisterThreadPriority(pid_t tid, const std::string &bundleName,
    BoostTriggerMethod method)
{
    CHECK_AND_RETURN_RET_LOG(processProxy_ != nullptr, ERR_OPERATION_FAILED, "ipcProxy is null.");
    return processProxy_->RegisterThreadPriority(tid, bundleName, method);
}

bool AudioProcessInClientInner::GetStopFlag() const
{
    CHECK_AND_RETURN_RET_LOG(audioBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return audioBuffer_->GetStopFlag();
}

void AudioProcessInClientInner::SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId)
{
    CHECK_AND_RETURN_LOG(processProxy_ != nullptr, "SetAudioHapticsSyncId processProxy_ is nullptr");
    processProxy_->SetAudioHapticsSyncId(audioHapticsSyncId);
}
} // namespace AudioStandard
} // namespace OHOS
