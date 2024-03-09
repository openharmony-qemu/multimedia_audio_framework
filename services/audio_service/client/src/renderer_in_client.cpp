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
#ifndef FAST_AUDIO_STREAM_H
#define FAST_AUDIO_STREAM_H

#include "renderer_in_client.h"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "securec.h"
#include "safe_block_queue.h"

#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_log.h"
#include "audio_ring_cache.h"
#include "audio_channel_blend.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "volume_ramp.h"
#include "volume_tools.h"
#include "callback_handler.h"
#include "audio_speed.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const uint64_t OLD_BUF_DURATION_IN_USEC = 92880; // This value is used for compatibility purposes.
const uint64_t AUDIO_US_PER_MS = 1000;
const int64_t AUDIO_NS_PER_US = 1000;
const uint64_t AUDIO_US_PER_S = 1000000;
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
const uint64_t MAX_CBBUF_IN_USEC = 100000;
const uint64_t MIN_CBBUF_IN_USEC = 20000;
const uint64_t AUDIO_FIRST_FRAME_LATENCY = 230; //ms
const float AUDIO_VOLOMUE_EPSILON = 0.0001;
const float AUDIO_MAX_VOLUME = 1.0f;
static const size_t MAX_WRITE_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t CREATE_TIMEOUT_IN_SECOND = 8; // 8S
static const int32_t OPERATION_TIMEOUT_IN_MS = 500; // 500ms
static const int32_t OFFLOAD_OPERATION_TIMEOUT_IN_MS = 8000; // 8000ms for offload
static const int32_t WRITE_CACHE_TIMEOUT_IN_MS = 3000; // 3000ms
static const int32_t WRITE_BUFFER_TIMEOUT_IN_MS = 20; // ms
static const int32_t SHORT_TIMEOUT_IN_MS = 20; // ms
static constexpr int CB_QUEUE_CAPACITY = 3;
constexpr int32_t MAX_BUFFER_SIZE = 100000;
}
class RendererInClientInner : public RendererInClient, public IStreamListener, public IHandler,
    public std::enable_shared_from_this<RendererInClientInner> {
public:
    RendererInClientInner(AudioStreamType eStreamType, int32_t appUid);
    ~RendererInClientInner();

    // IStreamListener
    int32_t OnOperationHandled(Operation operation, int64_t result) override;

    // IAudioStream
    void SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId) override;

    void SetRendererInfo(const AudioRendererInfo &rendererInfo) override;
    void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) override;
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj) override;
    int32_t GetAudioStreamInfo(AudioStreamParams &info) override;
    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid, SourceType sourceType =
        SOURCE_TYPE_MIC) override;
    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;
    int32_t GetAudioSessionID(uint32_t &sessionID) override;
    State GetState() override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    int32_t GetBufferSize(size_t &bufferSize) override;
    int32_t GetFrameCount(uint32_t &frameCount) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetAudioStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) override;
    float GetVolume() override;
    int32_t SetRenderRate(AudioRendererRate renderRate) override;
    AudioRendererRate GetRenderRate() override;
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) override;
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void OnFirstFrameWriting() override;
    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    int32_t ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t []> &outBuffer,
        int32_t &outBufferSize) override;
    int32_t WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize) override;

    // callback mode api
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override;
    AudioCaptureMode GetCaptureMode() override;
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t GetBufQueueState(BufferQueueState &bufState) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear() override;

    int32_t SetLowPowerVolume(float volume) override;
    float GetLowPowerVolume() override;
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadMode() override;
    float GetSingleStreamVolume() override;
    AudioEffectMode GetAudioEffectMode() override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) override;
    int64_t GetFramesWritten() override;
    int64_t GetFramesRead() override;

    void SetInnerCapturerState(bool isInnerCapturer) override;
    void SetWakeupCapturerState(bool isWakeupCapturer) override;
    void SetCapturerSource(int capturerSource) override;
    void SetPrivacyType(AudioPrivacyType privacyType) override;

    // Common APIs
    bool StartAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool PauseAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool StopAudioStream() override;
    bool ReleaseAudioStream(bool releaseRunner = true) override;
    bool FlushAudioStream() override;

    // Playback related APIs
    bool DrainAudioStream() override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize) override;
    void SetPreferredFrameSize(int32_t frameSize) override;

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;

    uint32_t GetUnderflowCount() override;
    void SetRendererPositionCallback(int64_t markPosition, const std::shared_ptr<RendererPositionCallback> &callback)
        override;
    void UnsetRendererPositionCallback() override;
    void SetRendererPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    void SetCapturerPositionCallback(int64_t markPosition, const std::shared_ptr<CapturerPositionCallback> &callback)
        override;
    void UnsetCapturerPositionCallback() override;
    void SetCapturerPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) override;
    uint32_t GetRendererSamplingRate() override;
    int32_t SetBufferSizeInMsec(int32_t bufferSizeInMsec) override;
    void SetApplicationCachePath(const std::string cachePath) override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    void SetStreamTrackerState(bool trackerRegisteredState) override;
    void GetSwitchInfo(IAudioStream::SwitchInfo& info) override;

    IAudioStream::StreamClass GetStreamClass() override;

    static const sptr<IStandardAudioService> GetAudioServerProxy();
    static void AudioServerDied(pid_t pid);

    void OnHandle(uint32_t code, int64_t data) override;
    void InitCallbackHandler();
    void SafeSendCallbackEvent(uint32_t eventCode, int64_t data);

    int32_t StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType);
    int32_t ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType);

    void SendRenderMarkReachedEvent(int64_t rendererMarkPosition);
    void SendRenderPeriodReachedEvent(int64_t rendererPeriodSize);

    void HandleRendererPositionChanges(size_t bytesWritten);
    void HandleStateChangeEvent(int64_t data);
    void HandleRenderMarkReachedEvent(int64_t rendererMarkPosition);
    void HandleRenderPeriodReachedEvent(int64_t rendererPeriodNumber);

private:
    void RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj);
    void UpdateTracker(const std::string &updateCase);

    int32_t DeinitIpcStream();

    int32_t InitIpcStream();

    const AudioProcessConfig ConstructConfig();

    int32_t InitSharedBuffer();
    int32_t InitCacheBuffer(size_t targetSize);

    int32_t FlushRingCache();
    int32_t DrainRingCache();

    int32_t WriteCacheData();
    bool ProcessSpeed(BufferDesc &temp);

    void InitCallbackBuffer(uint64_t bufferDurationInUs);
    void WriteCallbackFunc();
    // for callback mode. Check status if not running, wait for start or release.
    bool WaitForRunning();

    int32_t WriteInner(uint8_t *buffer, size_t bufferSize);

private:
    AudioStreamType eStreamType_;
    int32_t appUid_;
    uint32_t sessionId_ = 0;
    int32_t clientPid_ = -1;
    int32_t clientUid_ = -1;
    uint32_t appTokenId_ = 0;

    std::unique_ptr<AudioStreamTracker> audioStreamTracker_;

    AudioRendererInfo rendererInfo_;
    AudioCapturerInfo capturerInfo_; // not in use

    AudioPrivacyType privacyType_ = PRIVACY_TYPE_PUBLIC;
    bool streamTrackerRegistered_ = false;

    bool needSetThreadPriority_ = true;

    AudioStreamParams streamParams_; // in plan next: replace it with AudioRendererParams

    // for data process
    bool isBlendSet_ = false;
    AudioBlend audioBlend_;
    VolumeRamp volumeRamp_;

    // callbacks
    std::mutex streamCbMutex_;
    std::weak_ptr<AudioStreamCallback> streamCallback_;

    size_t cacheSizeInByte_ = 0;
    uint32_t spanSizeInFrame_ = 0;
    size_t clientSpanSizeInByte_ = 0;
    size_t sizePerFrameInByte_ = 4; // 16bit 2ch as default

    int32_t bufferSizeInMsec_ = 20; // 20ms
    std::string cachePath_;
    std::string dumpOutFile_;
    FILE *dumpOutFd_ = nullptr;

    std::shared_ptr<AudioRendererFirstFrameWritingCallback> firstFrameWritingCb_ = nullptr;
    bool hasFirstFrameWrited_ = false;

    // callback mode releated
    AudioRenderMode renderMode_ = RENDER_MODE_NORMAL;
    std::thread callbackLoop_; // thread for callback to client and write.
    std::atomic<bool> cbThreadReleased_ = true;
    std::mutex writeCbMutex_;
    std::condition_variable cbThreadCv_;
    std::shared_ptr<AudioRendererWriteCallback> writeCb_ = nullptr;
    std::mutex cbBufferMutex_;
    std::condition_variable cbBufferCV_;
    std::unique_ptr<uint8_t[]> cbBuffer_ {nullptr};
    size_t cbBufferSize_ = 0;
    SafeBlockQueue<BufferDesc> cbBufferQueue_; // only one cbBuffer_

    std::atomic<State> state_ = INVALID;
    // using this lock when change status_
    std::mutex statusMutex_;
    // for status operation wait and notify
    std::mutex callServerMutex_;
    std::condition_variable callServerCV_;

    Operation notifiedOperation_ = MAX_OPERATION_CODE;
    int64_t notifiedResult_ = 0;

    // write data
    std::mutex writeDataMutex_;
    std::condition_variable writeDataCV_;

    float clientVolume_ = 1.0;
    float clientOldVolume_ = 1.0;

    uint64_t clientWrittenBytes_ = 0;
    uint32_t underrunCount_ = 0;
    // ipc stream related
    AudioProcessConfig clientConfig_;
    sptr<IpcStreamListenerImpl> listener_ = nullptr;
    sptr<IpcStream> ipcStream_ = nullptr;
    std::shared_ptr<OHAudioBuffer> clientBuffer_ = nullptr;

    // buffer handle
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    std::mutex writeMutex_; // used for prevent multi thread call write

    // Mark reach and period reach callback
    int64_t totalBytesWritten_ = 0;
    std::mutex markReachMutex_;
    bool rendererMarkReached_ = false;
    int64_t rendererMarkPosition_ = 0;
    std::shared_ptr<RendererPositionCallback> rendererPositionCallback_ = nullptr;

    std::mutex periodReachMutex_;
    int64_t rendererPeriodSize_ = 0;
    int64_t rendererPeriodWritten_ = 0;
    std::shared_ptr<RendererPeriodPositionCallback> rendererPeriodPositionCallback_ = nullptr;

    // Event handler
    bool runnerReleased_ = false;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;

    bool paramsIsSet_ = false;
    AudioRendererRate rendererRate_ = RENDER_RATE_NORMAL;
    AudioEffectMode effectMode_ = EFFECT_DEFAULT;

    float speed_ = 1.0;
    std::unique_ptr<uint8_t[]> speedBuffer_ {nullptr};
    size_t bufferSize_ = 0;
    std::unique_ptr<AudioSpeed> audioSpeed_ = nullptr;

    bool offloadEnable_ = false;
    uint64_t offloadStartReadPos_ = 0;
    int64_t offloadStartHandleTime_ = 0;

    enum {
        STATE_CHANGE_EVENT = 0,
        RENDERER_MARK_REACHED_EVENT,
        RENDERER_PERIOD_REACHED_EVENT,
        CAPTURER_PERIOD_REACHED_EVENT,
        CAPTURER_MARK_REACHED_EVENT,
    };

    // note that the starting elements should remain the same as the enum State
    enum : int64_t {
        HANDLER_PARAM_INVALID = -1,
        HANDLER_PARAM_NEW = 0,
        HANDLER_PARAM_PREPARED,
        HANDLER_PARAM_RUNNING,
        HANDLER_PARAM_STOPPED,
        HANDLER_PARAM_RELEASED,
        HANDLER_PARAM_PAUSED,
        HANDLER_PARAM_STOPPING,
        HANDLER_PARAM_RUNNING_FROM_SYSTEM,
        HANDLER_PARAM_PAUSED_FROM_SYSTEM,
    };
};

std::shared_ptr<RendererInClient> RendererInClient::GetInstance(AudioStreamType eStreamType, int32_t appUid)
{
    return std::make_shared<RendererInClientInner>(eStreamType, appUid);
}

RendererInClientInner::RendererInClientInner(AudioStreamType eStreamType, int32_t appUid)
    : eStreamType_(eStreamType), appUid_(appUid), cbBufferQueue_(CB_QUEUE_CAPACITY)
{
    AUDIO_INFO_LOG("Create with StreamType:%{public}d appUid:%{public}d ", eStreamType_, appUid_);
    audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AUDIO_MODE_PLAYBACK, appUid);
    state_ = NEW;
}

RendererInClientInner::~RendererInClientInner()
{
    AUDIO_INFO_LOG("~RendererInClientInner()");
    DumpFileUtil::CloseDumpFile(&dumpOutFd_);
    RendererInClientInner::ReleaseAudioStream(true);
}

int32_t RendererInClientInner::OnOperationHandled(Operation operation, int64_t result)
{
    if (operation == SET_OFFLOAD_ENABLE) {
        AUDIO_INFO_LOG("SET_OFFLOAD_ENABLE result:%{public}" PRId64".", result);
        if (!offloadEnable_ && static_cast<bool>(result)) {
            offloadStartReadPos_ = 0;
        }
        offloadEnable_ = static_cast<bool>(result);
        return SUCCESS;
    }
    // read/write operation may print many log, use debug.
    if (operation == UPDATE_STREAM) {
        AUDIO_DEBUG_LOG("OnOperationHandled() UPDATE_STREAM result:%{public}" PRId64".", result);
        // notify write if blocked
        writeDataCV_.notify_all();
        return SUCCESS;
    }
    if (operation == BUFFER_UNDERRUN) {
        underrunCount_++;
        AUDIO_WARNING_LOG("recv underrun %{public}d", underrunCount_);
        // in plan next: do more to reduce underrun
        writeDataCV_.notify_all();
        return SUCCESS;
    }
    AUDIO_INFO_LOG("OnOperationHandled() recv operation:%{public}d result:%{public}" PRId64".", operation, result);
    std::unique_lock<std::mutex> lock(callServerMutex_);
    notifiedOperation_ = operation;
    notifiedResult_ = result;
    callServerCV_.notify_all();
    return SUCCESS;
}

void RendererInClientInner::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId)
{
    AUDIO_INFO_LOG("Set client PID: %{public}d, UID: %{public}d", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
}

void RendererInClientInner::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    rendererInfo_ = rendererInfo;
    if (rendererInfo_.streamUsage == STREAM_USAGE_SYSTEM ||
        rendererInfo_.streamUsage == STREAM_USAGE_DTMF ||
        rendererInfo_.streamUsage == STREAM_USAGE_ENFORCED_TONE ||
        rendererInfo_.streamUsage == STREAM_USAGE_ULTRASONIC ||
        rendererInfo_.streamUsage == STREAM_USAGE_NAVIGATION ||
        rendererInfo_.streamUsage == STREAM_USAGE_NOTIFICATION) {
        effectMode_ = EFFECT_NONE;
    }
    AUDIO_INFO_LOG("SetRendererInfo with flag %{public}d", rendererInfo_.rendererFlags);
}

void RendererInClientInner::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    AUDIO_WARNING_LOG("SetCapturerInfo is not supported");
    return;
}

void RendererInClientInner::RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    if (audioStreamTracker_ && audioStreamTracker_.get() && !streamTrackerRegistered_) {
        // make sure sessionId_ is valid.
        AUDIO_INFO_LOG("Calling register tracker, sessionid is %{public}d", sessionId_);
        AudioRegisterTrackerInfo registerTrackerInfo;

        registerTrackerInfo.sessionId = sessionId_;
        registerTrackerInfo.clientPid = clientPid_;
        registerTrackerInfo.state = state_;
        registerTrackerInfo.rendererInfo = rendererInfo_;
        registerTrackerInfo.capturerInfo = capturerInfo_;
        registerTrackerInfo.channelCount = streamParams_.channels;

        audioStreamTracker_->RegisterTracker(registerTrackerInfo, proxyObj);
        streamTrackerRegistered_ = true;
    }
}

void RendererInClientInner::UpdateTracker(const std::string &updateCase)
{
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("Renderer:Calling Update tracker for %{public}s", updateCase.c_str());
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }
}

int32_t RendererInClientInner::SetAudioStreamInfo(const AudioStreamParams info,
    const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    // In plan: If paramsIsSet_ is true, and new info is same as old info, return
    AUDIO_INFO_LOG("AudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d,"
        " stream type: %{public}d, encoding type: %{public}d", info.samplingRate, info.channels, info.format,
        eStreamType_, info.encoding);

    AudioXCollie guard("RendererInClientInner::SetAudioStreamInfo", CREATE_TIMEOUT_IN_SECOND);
    if (!IsFormatValid(info.format) || !IsSamplingRateValid(info.samplingRate) || !IsEncodingTypeValid(info.encoding)) {
        AUDIO_ERR_LOG("Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (!IsPlaybackChannelRelatedInfoValid(info.channels, info.channelLayout)) {
        return ERR_NOT_SUPPORTED;
    }

    CHECK_AND_RETURN_RET_LOG(IAudioStream::GetByteSizePerFrame(info, sizePerFrameInByte_) == SUCCESS,
        ERROR_INVALID_PARAM, "GetByteSizePerFrame failed with invalid params");

    if (state_ != NEW) {
        AUDIO_ERR_LOG("State is not new, release existing stream and recreate, state %{public}d", state_.load());
        int32_t ret = DeinitIpcStream();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "release existing stream failed.");
    }

    streamParams_ = info; // keep it for later use
    paramsIsSet_ = true;
    int32_t initRet = InitIpcStream();
    CHECK_AND_RETURN_RET_LOG(initRet == SUCCESS, initRet, "Init stream failed: %{public}d", initRet);
    state_ = PREPARED;

    // eg: 48000_2_1_out.pcm
    dumpOutFile_ = std::to_string(streamParams_.samplingRate) + "_" + std::to_string(streamParams_.channels) +
        "_" + std::to_string(streamParams_.format) + "_out.pcm";

    DumpFileUtil::OpenDumpFile(DUMP_CLIENT_PARA, dumpOutFile_, &dumpOutFd_);

    RegisterTracker(proxyObj);
    return SUCCESS;
}

std::mutex g_serverProxyMutex;
sptr<IStandardAudioService> gServerProxy_ = nullptr;
const sptr<IStandardAudioService> RendererInClientInner::GetAudioServerProxy()
{
    std::lock_guard<std::mutex> lock(g_serverProxyMutex);
    if (gServerProxy_ == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            AUDIO_ERR_LOG("GetAudioServerProxy: get sa manager failed");
            return nullptr;
        }
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        if (object == nullptr) {
            AUDIO_ERR_LOG("GetAudioServerProxy: get audio service remote object failed");
            return nullptr;
        }
        gServerProxy_ = iface_cast<IStandardAudioService>(object);
        if (gServerProxy_ == nullptr) {
            AUDIO_ERR_LOG("GetAudioServerProxy: get audio service proxy failed");
            return nullptr;
        }

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient = new(std::nothrow) AudioServerDeathRecipient(getpid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb(std::bind(&RendererInClientInner::AudioServerDied,
                std::placeholders::_1));
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("GetAudioServerProxy: failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = gServerProxy_;
    return gasp;
}

void RendererInClientInner::AudioServerDied(pid_t pid)
{
    AUDIO_INFO_LOG("audio server died clear proxy, will restore proxy in next call");
    std::lock_guard<std::mutex> lock(g_serverProxyMutex);
    gServerProxy_ = nullptr;
}

void RendererInClientInner::OnHandle(uint32_t code, int64_t data)
{
    AUDIO_DEBUG_LOG("On handle event, event code: %{public}d, data: %{public}" PRIu64 "", code, data);
    switch (code) {
        case STATE_CHANGE_EVENT:
            HandleStateChangeEvent(data);
            break;
        case RENDERER_MARK_REACHED_EVENT:
            HandleRenderMarkReachedEvent(data);
            break;
        case RENDERER_PERIOD_REACHED_EVENT:
            HandleRenderPeriodReachedEvent(data);
            break;
        default:
            break;
    }
}

void RendererInClientInner::HandleStateChangeEvent(int64_t data)
{
    State state = INVALID;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    ParamsToStateCmdType(data, state, cmdType);
    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        state = state != STOPPING ? state : STOPPED; // client only need STOPPED
        streamCb->OnStateChange(state, cmdType);
    }
}

void RendererInClientInner::HandleRenderMarkReachedEvent(int64_t rendererMarkPosition)
{
    AUDIO_DEBUG_LOG("Start HandleRenderMarkReachedEvent");
    std::unique_lock<std::mutex> lock(markReachMutex_);
    if (rendererPositionCallback_) {
        rendererPositionCallback_->OnMarkReached(rendererMarkPosition);
    }
}

void RendererInClientInner::HandleRenderPeriodReachedEvent(int64_t rendererPeriodNumber)
{
    AUDIO_DEBUG_LOG("Start HandleRenderPeriodReachedEvent");
    std::unique_lock<std::mutex> lock(periodReachMutex_);
    if (rendererPeriodPositionCallback_) {
        rendererPeriodPositionCallback_->OnPeriodReached(rendererPeriodNumber);
    }
}

void RendererInClientInner::SafeSendCallbackEvent(uint32_t eventCode, int64_t data)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    AUDIO_INFO_LOG("Send callback event, code: %{public}u, data: %{public}" PRId64 "", eventCode, data);
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr && runnerReleased_ == false, "Runner is Released");
    callbackHandler_->SendCallbackEvent(eventCode, data);
}

void RendererInClientInner::InitCallbackHandler()
{
    if (callbackHandler_ == nullptr) {
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this());
    }
}

// call this without lock, we should be able to call deinit in any case.
int32_t RendererInClientInner::DeinitIpcStream()
{
    ipcStream_->Release();
    // in plan:
    ipcStream_ = nullptr;
    ringCache_->ResetBuffer();
    return SUCCESS;
}

const AudioProcessConfig RendererInClientInner::ConstructConfig()
{
    AudioProcessConfig config = {};

    config.appInfo.appPid = clientPid_;
    config.appInfo.appUid = clientUid_;
    config.appInfo.appTokenId = appTokenId_;

    config.streamInfo.channels = static_cast<AudioChannel>(streamParams_.channels);
    config.streamInfo.encoding = static_cast<AudioEncodingType>(streamParams_.encoding);
    config.streamInfo.format = static_cast<AudioSampleFormat>(streamParams_.format);
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(streamParams_.samplingRate);

    config.audioMode = AUDIO_MODE_PLAYBACK;

    if (rendererInfo_.rendererFlags != 0) {
        AUDIO_WARNING_LOG("ConstructConfig find renderer flag invalid:%{public}d", rendererInfo_.rendererFlags);
        rendererInfo_.rendererFlags = 0;
    }
    config.rendererInfo = rendererInfo_;

    config.capturerInfo = {};

    config.streamType = eStreamType_;

    config.privacyType = privacyType_;

    clientConfig_ = config;

    return config;
}

int32_t RendererInClientInner::InitSharedBuffer()
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "InitSharedBuffer failed, null ipcStream_.");
    int32_t ret = ipcStream_->ResolveBuffer(clientBuffer_);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && clientBuffer_ != nullptr, ret, "ResolveBuffer failed:%{public}d", ret);

    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ret = clientBuffer_->GetSizeParameter(totalSizeInFrame, spanSizeInFrame_, byteSizePerFrame);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && byteSizePerFrame == sizePerFrameInByte_, ret, "GetSizeParameter failed"
        ":%{public}d, byteSizePerFrame:%{public}u, sizePerFrameInByte_:%{public}zu", ret, byteSizePerFrame,
        sizePerFrameInByte_);

    clientSpanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame;

    AUDIO_INFO_LOG("totalSizeInFrame_[%{public}u] spanSizeInFrame[%{public}u] sizePerFrameInByte_[%{public}zu]"
        "clientSpanSizeInByte_[%{public}zu]", totalSizeInFrame, spanSizeInFrame_, sizePerFrameInByte_,
        clientSpanSizeInByte_);

    return SUCCESS;
}

// InitCacheBuffer should be able to modify the cache size between clientSpanSizeInByte_ and 4 * clientSpanSizeInByte_
int32_t RendererInClientInner::InitCacheBuffer(size_t targetSize)
{
    CHECK_AND_RETURN_RET_LOG(clientSpanSizeInByte_ != 0, ERR_OPERATION_FAILED, "clientSpanSizeInByte_ invalid");

    AUDIO_INFO_LOG("old size:%{public}zu, new size:%{public}zu", cacheSizeInByte_, targetSize);
    cacheSizeInByte_ = targetSize;

    if (ringCache_ == nullptr) {
        ringCache_ = AudioRingCache::Create(cacheSizeInByte_);
    } else {
        OptResult result = ringCache_->ReConfig(cacheSizeInByte_, false); // false --> clear buffer
        if (result.ret != OPERATION_SUCCESS) {
            AUDIO_ERR_LOG("ReConfig AudioRingCache to size %{public}u failed:ret%{public}zu", result.ret, targetSize);
            return ERR_OPERATION_FAILED;
        }
    }

    return SUCCESS;
}

int32_t RendererInClientInner::InitIpcStream()
{
    AudioProcessConfig config = ConstructConfig();
    sptr<IStandardAudioService> gasp = RendererInClientInner::GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_OPERATION_FAILED, "Create failed, can not get service.");
    sptr<IRemoteObject> ipcProxy = gasp->CreateAudioProcess(config); // in plan next: add ret
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, ERR_OPERATION_FAILED, "failed with null ipcProxy.");
    ipcStream_ = iface_cast<IpcStream>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "failed when iface_cast.");

    // in plan next: old listener_ is destoried here, will server receive dieth notify?
    listener_ = sptr<IpcStreamListenerImpl>::MakeSptr(shared_from_this());
    int32_t ret = ipcStream_->RegisterStreamListener(listener_->AsObject());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RegisterStreamListener failed:%{public}d", ret);

    ret = InitSharedBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitSharedBuffer failed:%{public}d", ret);

    ret = InitCacheBuffer(clientSpanSizeInByte_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitCacheBuffer failed:%{public}d", ret);

    ret = ipcStream_->GetAudioSessionID(sessionId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioSessionID failed:%{public}d", ret);
    InitCallbackHandler();
    return SUCCESS;
}

int32_t RendererInClientInner::GetAudioStreamInfo(AudioStreamParams &info)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, ERR_OPERATION_FAILED, "Params is not set");
    info = streamParams_;
    return SUCCESS;
}

bool RendererInClientInner::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    AUDIO_WARNING_LOG("CheckRecordingCreate is not supported");
    return false;
}

bool RendererInClientInner::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    AUDIO_WARNING_LOG("CheckRecordingCreate is not supported");
    return false;
}

int32_t RendererInClientInner::GetAudioSessionID(uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != NEW), ERR_ILLEGAL_STATE,
        "State error %{public}d", state_.load());
    sessionID = sessionId_;
    return SUCCESS;
}

State RendererInClientInner::GetState()
{
    return state_;
}

bool RendererInClientInner::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());
    int64_t currentWritePos = totalBytesWritten_ / sizePerFrameInByte_;
    timestamp.framePosition = currentWritePos;

    uint64_t readPos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");
    clientBuffer_->GetHandleInfo(readPos, handleTime);
    if (readPos == 0 || handleTime == 0) {
        AUDIO_WARNING_LOG("GetHandleInfo may failed");
    }

    int64_t deltaPos = static_cast<uint64_t>(currentWritePos) >= readPos ?  currentWritePos - readPos : 0;
    int64_t tempLatency = 0;
    if (GetState() == RUNNING) {
        tempLatency = 45000000; // 45000000 -> 45 ms
    }
    int64_t deltaTime = deltaPos * AUDIO_MS_PER_SECOND / streamParams_.samplingRate * AUDIO_US_PER_S;

    int64_t audioTimeResult = handleTime + deltaTime + tempLatency;

    if (offloadEnable_) {
        uint64_t timeStamp = 0;
        uint64_t paWriteIndex = 0;
        uint64_t cacheTimeDsp = 0;
        uint64_t cacheTimePa = 0;
        ipcStream_->GetOffloadApproximatelyCacheTime(timeStamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
        int64_t cacheTime = static_cast<int64_t>(cacheTimeDsp + cacheTimePa) * AUDIO_NS_PER_US;
        int64_t timeNow = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        int64_t deltaTimeStamp = (static_cast<int64_t>(timeNow) - static_cast<int64_t>(timeStamp)) * AUDIO_NS_PER_US;
        int64_t paWriteIndexNs = paWriteIndex * AUDIO_NS_PER_US;
        uint64_t readPosNs = readPos * AUDIO_MS_PER_SECOND / streamParams_.samplingRate * AUDIO_US_PER_S;

        int64_t deltaPaWriteIndexNs = static_cast<int64_t>(readPosNs) - static_cast<int64_t>(paWriteIndexNs);
        int64_t cacheTimeNow = cacheTime - deltaTimeStamp + deltaPaWriteIndexNs;
        if (offloadStartReadPos_ == 0) {
            offloadStartReadPos_ = readPosNs;
            offloadStartHandleTime_ = handleTime;
        }
        int64_t offloadDelta = 0;
        if (offloadStartReadPos_ != 0) {
            offloadDelta = (static_cast<int64_t>(readPosNs) - static_cast<int64_t>(offloadStartReadPos_)) -
                           (handleTime - offloadStartHandleTime_) - cacheTimeNow;
        }
        audioTimeResult += offloadDelta;
    }

    timestamp.time.tv_sec = static_cast<time_t>(audioTimeResult / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(audioTimeResult % AUDIO_NS_PER_SECOND);

    return true;
}

bool RendererInClientInner::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base);
}

int32_t RendererInClientInner::GetBufferSize(size_t &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Capturer stream is released");
    bufferSize = clientSpanSizeInByte_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        bufferSize = cbBufferSize_;
    }
    AUDIO_INFO_LOG("Buffer size is %{public}zu, mode is %{public}s", bufferSize, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetFrameCount(uint32_t &frameCount)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Capturer stream is released");
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ != 0, ERR_ILLEGAL_STATE, "sizePerFrameInByte_ is 0!");
    frameCount = spanSizeInFrame_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        frameCount = cbBufferSize_ / sizePerFrameInByte_;
    }
    AUDIO_INFO_LOG("Frame count is %{public}u, mode is %{public}s", frameCount, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetLatency(uint64_t &latency)
{
    // in plan: 150000 for debug
    latency = 150000; // unit is us, 150000 is 150ms
    return SUCCESS;
}

int32_t RendererInClientInner::SetAudioStreamType(AudioStreamType audioStreamType)
{
    AUDIO_ERR_LOG("Change stream type %{public}d to %{public}d is not supported", eStreamType_, audioStreamType);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolume(float volume)
{
    Trace trace("RendererInClientInner::SetVolume:" + std::to_string(volume));
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    if (volumeRamp_.IsActive()) {
        volumeRamp_.Terminate();
    }
    clientOldVolume_ = clientVolume_;
    clientVolume_ = volume;
    return SUCCESS;
}

float RendererInClientInner::GetVolume()
{
    Trace trace("RendererInClientInner::GetVolume:" + std::to_string(clientVolume_));
    return clientVolume_;
}

int32_t RendererInClientInner::SetRenderRate(AudioRendererRate renderRate)
{
    if (rendererRate_ == renderRate) {
        AUDIO_INFO_LOG("Set same rate");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    rendererRate_ = renderRate;
    return ipcStream_->SetRate(renderRate);
}

int32_t RendererInClientInner::SetSpeed(float speed)
{
    if (audioSpeed_ == nullptr) {
        audioSpeed_ = std::make_unique<AudioSpeed>(streamParams_.samplingRate,
            streamParams_.format, streamParams_.channels);
        GetBufferSize(bufferSize_);
        speedBuffer_ = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    }
    audioSpeed_->SetSpeed(speed);
    speed_ = speed;
    return SUCCESS;
}

float RendererInClientInner::GetSpeed()
{
    return speed_;
}

int32_t RendererInClientInner::ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t []> &outBuffer,
    int32_t &outBufferSize)
{
    return audioSpeed_->ChangeSpeedFunc(buffer, bufferSize, outBuffer, outBufferSize);
}

int32_t RendererInClientInner::WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize)
{
    int32_t writeIndex = 0;
    int32_t writeSize = bufferSize_;
    while (speedBufferSize > 0) {
        if (speedBufferSize < bufferSize_) {
            writeSize = speedBufferSize;
        }
        int32_t writtenSize = Write(speedBuffer + writeIndex, writeSize);
        if (writtenSize <= 0) {
            return writtenSize;
        }
        writeIndex += writtenSize;
        speedBufferSize -= writtenSize;
    }

    return bufferSize;
}

AudioRendererRate RendererInClientInner::GetRenderRate()
{
    AUDIO_INFO_LOG("Get RenderRate %{public}d", rendererRate_);
    return rendererRate_;
}

int32_t RendererInClientInner::SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetStreamCallback failed. callback == nullptr");
        return ERR_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(streamCbMutex_);
    streamCallback_ = callback;
    lock.unlock();

    if (state_ != PREPARED) {
        return SUCCESS;
    }
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, PREPARED);
    return SUCCESS;
}

int32_t RendererInClientInner::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    AUDIO_INFO_LOG("SetRendererFirstFrameWritingCallback in.");
    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "callback is nullptr");
    firstFrameWritingCb_ = callback;
    return SUCCESS;
}

void RendererInClientInner::OnFirstFrameWriting()
{
    hasFirstFrameWrited_ = true;
    CHECK_AND_RETURN_LOG(firstFrameWritingCb_!= nullptr, "firstFrameWritingCb_ is null.");
    uint64_t latency = AUDIO_FIRST_FRAME_LATENCY;
    AUDIO_DEBUG_LOG("OnFirstFrameWriting: latency %{public}" PRIu64 "", latency);
    firstFrameWritingCb_->OnFirstFrameWriting(latency);
}

void RendererInClientInner::InitCallbackBuffer(uint64_t bufferDurationInUs)
{
    if (bufferDurationInUs > MAX_BUF_DURATION_IN_USEC) {
        AUDIO_ERR_LOG("InitCallbackBuffer with invalid duration %{public}" PRIu64", use default instead.",
            bufferDurationInUs);
        bufferDurationInUs = OLD_BUF_DURATION_IN_USEC;
    }
    // Calculate buffer size based on duration.

    cbBufferSize_ = static_cast<size_t>(bufferDurationInUs * streamParams_.samplingRate / AUDIO_US_PER_S) *
        sizePerFrameInByte_;
    AUDIO_INFO_LOG("InitCallbackBuffer with duration %{public}" PRIu64", size: %{public}zu", bufferDurationInUs,
        cbBufferSize_);
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    cbBuffer_ = std::make_unique<uint8_t[]>(cbBufferSize_);
}

int32_t RendererInClientInner::SetRenderMode(AudioRenderMode renderMode)
{
    AUDIO_INFO_LOG("SetRenderMode to %{public}s", renderMode == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" :
        "RENDER_MODE_CALLBACK");
    if (renderMode_ == renderMode) {
        return SUCCESS;
    }

    // renderMode_ is inited as RENDER_MODE_NORMAL, can only be set to RENDER_MODE_CALLBACK.
    if (renderMode_ == RENDER_MODE_CALLBACK && renderMode == RENDER_MODE_NORMAL) {
        AUDIO_ERR_LOG("SetRenderMode from callback to normal is not supported.");
        return ERR_INCORRECT_MODE;
    }

    // state check
    if (state_ != PREPARED && state_ != NEW) {
        AUDIO_ERR_LOG("SetRenderMode failed. invalid state:%{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    renderMode_ = renderMode;

    // init callbackLoop_
    callbackLoop_ = std::thread(&RendererInClientInner::WriteCallbackFunc, this);
    pthread_setname_np(callbackLoop_.native_handle(), "OS_AudioWriteCB");

    std::unique_lock<std::mutex> threadStartlock(statusMutex_);
    bool stopWaiting = cbThreadCv_.wait_for(threadStartlock, std::chrono::milliseconds(SHORT_TIMEOUT_IN_MS), [this] {
        return cbThreadReleased_ == false; // When thread is started, cbThreadReleased_ will be false. So stop waiting.
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("Init OS_AudioWriteCB thread time out");
    }

    InitCallbackBuffer(OLD_BUF_DURATION_IN_USEC);
    return SUCCESS;
}

AudioRenderMode RendererInClientInner::GetRenderMode()
{
    AUDIO_INFO_LOG("Render mode is %{public}s", renderMode_ == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" :
        "RENDER_MODE_CALLBACK");
    return renderMode_;
}

int32_t RendererInClientInner::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Invalid null callback");
    CHECK_AND_RETURN_RET_LOG(renderMode_ == RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE, "incorrect render mode");
    std::lock_guard<std::mutex> lock(writeCbMutex_);
    writeCb_ = callback;
    return SUCCESS;
}

// Sleep or wait in WaitForRunning to avoid dead looping.
bool RendererInClientInner::WaitForRunning()
{
    Trace trace("RendererInClientInner::WaitForRunning");
    // check renderer state_: call client write only in running else wait on statusMutex_
    std::unique_lock<std::mutex> stateLock(statusMutex_);
    if (state_ != RUNNING) {
        bool stopWaiting = cbThreadCv_.wait_for(stateLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
            return state_ == RUNNING || cbThreadReleased_;
        });
        if (cbThreadReleased_) {
            AUDIO_INFO_LOG("CBThread end in non-running status, sessionID :%{public}d", sessionId_);
            return false;
        }
        if (!stopWaiting) {
            AUDIO_DEBUG_LOG("Wait timeout, current state_ is %{public}d", state_.load()); // wait 0.5s
            return false;
        }
    }
    return true;
}

bool RendererInClientInner::ProcessSpeed(BufferDesc &temp)
{
    int32_t speedBufferSize = 0;
    int32_t ret = audioSpeed_->ChangeSpeedFunc(temp.buffer, temp.bufLength, speedBuffer_, speedBufferSize);
    if (ret == 0 || speedBufferSize == 0) {
        // Continue writing when the sonic is not full
        return false;
    }
    temp.buffer = speedBuffer_.get();
    temp.bufLength = speedBufferSize;
    return true;
}

void RendererInClientInner::WriteCallbackFunc()
{
    AUDIO_INFO_LOG("WriteCallbackFunc start, sessionID :%{public}d", sessionId_);
    cbThreadReleased_ = false;

    // Modify thread priority is not need as first call write will do these work.
    cbThreadCv_.notify_one();

    // start loop
    while (!cbThreadReleased_) {
        Trace traceLoop("RendererInClientInner::WriteCallbackFunc");
        if (!WaitForRunning()) {
            continue;
        }
        if (!cbBufferQueue_.IsEmpty()) {
            Trace traceQueuePop("RendererInClientInner::QueueWaitPop");
            // If client didn't call Enqueue in OnWriteData, pop will block here.
            BufferDesc temp = cbBufferQueue_.Pop();
            if (temp.buffer == nullptr) {
                AUDIO_WARNING_LOG("Queue pop error: get nullptr.");
                break;
            }
            if (state_ != RUNNING) { continue; }
            traceQueuePop.End();
            if (!isEqual(speed_, 1.0f) && !ProcessSpeed(temp)) {
                continue;
            }
            // call write here.
            int32_t result = WriteInner(temp.buffer, temp.bufLength);
            if (result < 0 || result != static_cast<int32_t>(temp.bufLength)) {
                AUDIO_WARNING_LOG("Call write fail, result:%{public}d, bufLength:%{public}zu", result, temp.bufLength);
            }
        }
        if (state_ != RUNNING) { continue; }
        // call client write
        std::unique_lock<std::mutex> lockCb(writeCbMutex_);
        if (writeCb_ != nullptr) {
            Trace traceCb("RendererInClientInner::OnWriteData");
            writeCb_->OnWriteData(cbBufferSize_);
        }
        lockCb.unlock();

        Trace traceQueuePush("RendererInClientInner::QueueWaitPush");
        std::unique_lock<std::mutex> lockBuffer(cbBufferMutex_);
        cbBufferCV_.wait_for(lockBuffer, std::chrono::milliseconds(WRITE_BUFFER_TIMEOUT_IN_MS), [this] {
            return cbBufferQueue_.IsEmpty() == false; // will be false when got notified.
        });
        if (cbBufferQueue_.IsEmpty()) {
            AUDIO_WARNING_LOG("cbBufferQueue_ is empty");
        }
    }
    AUDIO_INFO_LOG("CBThread end sessionID :%{public}d", sessionId_);
}

int32_t RendererInClientInner::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_ERR_LOG("SetCaptureMode is not supported");
    return ERROR;
}

AudioCaptureMode RendererInClientInner::GetCaptureMode()
{
    AUDIO_ERR_LOG("GetCaptureMode is not supported");
    return CAPTURE_MODE_NORMAL; // not supported
}

int32_t RendererInClientInner::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerReadCallback is not supported");
    return ERROR;
}

int32_t RendererInClientInner::GetBufferDesc(BufferDesc &bufDesc)
{
    Trace trace("RendererInClientInner::GetBufferDesc");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("GetBufferDesc is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    bufDesc.buffer = cbBuffer_.get();
    bufDesc.bufLength = cbBufferSize_;
    bufDesc.dataLength = cbBufferSize_;
    return SUCCESS;
}

int32_t RendererInClientInner::GetBufQueueState(BufferQueueState &bufState)
{
    Trace trace("RendererInClientInner::GetBufQueueState");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("GetBufQueueState is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    // only one buffer in queue.
    bufState.numBuffers = 1;
    bufState.currentIndex = 0;
    return SUCCESS;
}

int32_t RendererInClientInner::Enqueue(const BufferDesc &bufDesc)
{
    Trace trace("RendererInClientInner::Enqueue " + std::to_string(bufDesc.bufLength));
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Enqueue is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr, ERR_INVALID_PARAM, "Invalid buffer");
    if (bufDesc.bufLength > cbBufferSize_ || bufDesc.dataLength > cbBufferSize_) {
        AUDIO_WARNING_LOG("Invalid bufLength:%{public}zu or dataLength:%{public}zu, should be %{public}zu",
            bufDesc.bufLength, bufDesc.dataLength, cbBufferSize_);
    }

    BufferDesc temp = bufDesc;

    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Invalid state: %{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    // Call write here may block, so put it in loop callbackLoop_
    cbBufferQueue_.Push(temp);
    return SUCCESS;
}

int32_t RendererInClientInner::Clear()
{
    Trace trace("RendererInClientInner::Clear");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Clear is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::unique_lock<std::mutex> lock(cbBufferMutex_);
    int32_t ret = memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "Clear buffer fail, ret %{public}d.", ret);
    lock.unlock();
    FlushAudioStream();
    return SUCCESS;
}

int32_t RendererInClientInner::SetLowPowerVolume(float volume)
{
    // in plan
    return ERROR;
}

float RendererInClientInner::GetLowPowerVolume()
{
    // in plan
    return 0.0;
}

int32_t RendererInClientInner::SetOffloadMode(int32_t state, bool isAppBack)
{
    return ipcStream_->SetOffloadMode(state, isAppBack);
}

int32_t RendererInClientInner::UnsetOffloadMode()
{
    return ipcStream_->UnsetOffloadMode();
}

float RendererInClientInner::GetSingleStreamVolume()
{
    // in plan
    return 0.0;
}

AudioEffectMode RendererInClientInner::GetAudioEffectMode()
{
    AUDIO_DEBUG_LOG("Current audio effect mode is %{public}d", effectMode_);
    return effectMode_;
}

int32_t RendererInClientInner::SetAudioEffectMode(AudioEffectMode effectMode)
{
    if (effectMode_ == effectMode) {
        AUDIO_INFO_LOG("Set same effect mode");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetAudioEffectMode(effectMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Set audio effect mode failed");
    effectMode_ = effectMode;
    return SUCCESS;
}

int64_t RendererInClientInner::GetFramesWritten()
{
    return totalBytesWritten_ / sizePerFrameInByte_;
}

int64_t RendererInClientInner::GetFramesRead()
{
    AUDIO_ERR_LOG("not supported");
    return -1;
}


void RendererInClientInner::SetInnerCapturerState(bool isInnerCapturer)
{
    AUDIO_ERR_LOG("SetInnerCapturerState is not supported");
    return;
}

void RendererInClientInner::SetWakeupCapturerState(bool isWakeupCapturer)
{
    AUDIO_ERR_LOG("SetWakeupCapturerState is not supported");
    return;
}

void RendererInClientInner::SetCapturerSource(int capturerSource)
{
    AUDIO_ERR_LOG("SetCapturerSource is not supported");
    return;
}

void RendererInClientInner::SetPrivacyType(AudioPrivacyType privacyType)
{
    if (privacyType_ == privacyType) {
        AUDIO_INFO_LOG("Set same privacy type");
        return;
    }
    privacyType_ = privacyType;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetPrivacyType(privacyType);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Set privacy type failed");
}

bool RendererInClientInner::StartAudioStream(StateChangeCmdType cmdType)
{
    Trace trace("RendererInClientInner::StartAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != PREPARED && state_ != STOPPED && state_ != PAUSED) {
        AUDIO_ERR_LOG("Start failed Illegal state:%{public}d", state_.load());
        return false;
    }

    hasFirstFrameWrited_ = false;
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Start();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Start call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == START_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != START_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Start failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        return false;
    }
    waitLock.unlock();

    state_ = RUNNING; // change state_ to RUNNING, then notify cbThread
    offloadStartReadPos_ = 0;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        // start the callback-write thread
        cbThreadCv_.notify_all();
    }
    statusLock.unlock();
    // in plan: call HiSysEventWrite
    int64_t param = -1;
    StateCmdTypeToParams(param, state_, cmdType);
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, param);

    AUDIO_INFO_LOG("Start SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("RUNNING");
    return true;
}

bool RendererInClientInner::PauseAudioStream(StateChangeCmdType cmdType)
{
    Trace trace("RendererInClientInner::PauseAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("State is not RUNNING. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Pause();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == PAUSE_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != PAUSE_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Pause failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        return false;
    }
    waitLock.unlock();

    state_ = PAUSED;
    statusLock.unlock();

    // in plan: call HiSysEventWrite
    int64_t param = -1;
    StateCmdTypeToParams(param, state_, cmdType);
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, param);

    AUDIO_INFO_LOG("Pause SUCCESS, sessionId %{public}d, uid %{public}d, mode %{public}s", sessionId_,
        clientUid_, renderMode_ == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    UpdateTracker("PAUSED");
    return true;
}

bool RendererInClientInner::StopAudioStream()
{
    Trace trace("RendererInClientInner::StopAudioStream " + std::to_string(sessionId_));
    AUDIO_INFO_LOG("Stop begin for sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
    if (!offloadEnable_) {
        DrainAudioStream();
    }
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ == STOPPED) {
        AUDIO_INFO_LOG("Renderer in client is already stopped");
        return true;
    }
    if ((state_ != RUNNING) && (state_ != PAUSED)) {
        AUDIO_ERR_LOG("Stop failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        state_ = STOPPING;
        AUDIO_INFO_LOG("Stop begin in callback mode sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Stop();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Stop call server failed:%{public}u", ret);
        return false;
    }

    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == STOP_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != STOP_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Stop failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        state_ = INVALID;
        return false;
    }
    waitLock.unlock();

    state_ = STOPPED;
    statusLock.unlock();

    // in plan: call HiSysEventWrite
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);

    AUDIO_INFO_LOG("Stop SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("STOPPED");
    return true;
}

bool RendererInClientInner::ReleaseAudioStream(bool releaseRunner)
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Already released, do nothing");
        return true;
    }
    state_ = RELEASED;
    statusLock.unlock();

    Trace trace("RendererInClientInner::ReleaseAudioStream " + std::to_string(sessionId_));
    if (ipcStream_ != nullptr) {
        ipcStream_->Release();
        ipcStream_ = nullptr;
    } else {
        AUDIO_WARNING_LOG("release while ipcStream is null");
    }

    // no lock, call release in any case, include blocked case.
    {
        std::lock_guard<std::mutex> runnerlock(runnerMutex_);
        if (releaseRunner && callbackHandler_ != nullptr) {
            AUDIO_INFO_LOG("runner remove");
            callbackHandler_->ReleaseEventRunner();
            runnerReleased_ = true;
            callbackHandler_ = nullptr;
        }
    }

    // clear write callback
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        cbThreadReleased_ = true; // stop loop
        if (cbBufferQueue_.IsEmpty()) {
            std::lock_guard<std::mutex> lockWriteCb(writeCbMutex_);
            cbBufferQueue_.PushNoWait({nullptr, 0, 0});
        }
        cbThreadCv_.notify_all();
        writeDataCV_.notify_all();
        if (callbackLoop_.joinable()) {
            callbackLoop_.join();
        }
    }
    paramsIsSet_ = false;

    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        AUDIO_INFO_LOG("Notify client the state is released");
        streamCb->OnStateChange(RELEASED, CMD_FROM_CLIENT);
    }
    lock.unlock();

    UpdateTracker("RELEASED");
    AUDIO_INFO_LOG("Release end, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);

    audioSpeed_.reset();
    audioSpeed_ = nullptr;
    return true;
}

bool RendererInClientInner::FlushAudioStream()
{
    Trace trace("RendererInClientInner::FlushAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    std::lock_guard<std::mutex>lock(writeMutex_);
    if ((state_ != RUNNING) && (state_ != PAUSED) && (state_ != STOPPED)) {
        AUDIO_ERR_LOG("Flush failed. Illegal state:%{public}u", state_.load());
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(FlushRingCache() == SUCCESS, false, "Flush cache failed");

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Flush();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Flush call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == FLUSH_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != FLUSH_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Flush failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        notifiedOperation_ = MAX_OPERATION_CODE;
        return false;
    }
    notifiedOperation_ = MAX_OPERATION_CODE;
    waitLock.unlock();
    AUDIO_INFO_LOG("Flush stream SUCCESS, sessionId: %{public}d", sessionId_);
    return true;
}

int32_t RendererInClientInner::FlushRingCache()
{
    ringCache_->ResetBuffer();
    return SUCCESS;
}

int32_t RendererInClientInner::DrainRingCache()
{
    // send all data in ringCache_ to server even if GetReadableSize() < clientSpanSizeInByte_.

    Trace trace("RendererInClientInner::DrainRingCache " + std::to_string(sessionId_));
    std::lock_guard<std::mutex> lock(writeMutex_);
    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERR_OPERATION_FAILED, "ring cache unreadable");
    size_t readableSize = result.size;
    if (readableSize == 0) {
        AUDIO_WARNING_LOG("Readable size is already zero");
        return SUCCESS;
    }

    BufferDesc desc = {};
    uint64_t curWriteIndex = clientBuffer_->GetCurWriteFrame();
    int32_t ret = clientBuffer_->GetWriteBuffer(curWriteIndex, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetWriteBuffer failed %{public}d", ret);

    // if readableSize < clientSpanSizeInByte_, server will recv a data with some empty data.
    // it looks like this: |*******_____|
    size_t minSize = std::min(readableSize, clientSpanSizeInByte_);
    result = ringCache_->Dequeue({desc.buffer, minSize});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringCache Dequeue failed %{public}d", result.ret);
    clientBuffer_->SetCurWriteFrame(curWriteIndex + spanSizeInFrame_);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is nullptr");
    ipcStream_->UpdatePosition(); // notiify server update position
    HandleRendererPositionChanges(minSize);
    return SUCCESS;
}

bool RendererInClientInner::DrainAudioStream()
{
    Trace trace("RendererInClientInner::DrainAudioStream " + std::to_string(sessionId_));
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("Drain failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(DrainRingCache() == SUCCESS, false, "Drain cache failed");

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Drain();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Drain call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == DRAIN_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != DRAIN_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Drain failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        notifiedOperation_ = MAX_OPERATION_CODE;
        return false;
    }
    notifiedOperation_ = MAX_OPERATION_CODE;
    waitLock.unlock();
    AUDIO_INFO_LOG("Drain stream SUCCESS, sessionId: %{public}d", sessionId_);
    return true;
}

void RendererInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    size_t maxCbBufferSize = static_cast<size_t>(MAX_CBBUF_IN_USEC * streamParams_.samplingRate / AUDIO_US_PER_S) *
        sizePerFrameInByte_;
    size_t minCbBufferSize = static_cast<size_t>(MIN_CBBUF_IN_USEC * streamParams_.samplingRate / AUDIO_US_PER_S) *
        sizePerFrameInByte_;
    size_t preferredCbBufferSize = static_cast<size_t>(frameSize) * sizePerFrameInByte_;
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    cbBufferSize_ = (preferredCbBufferSize > maxCbBufferSize || preferredCbBufferSize < minCbBufferSize) ?
        (preferredCbBufferSize > maxCbBufferSize ? maxCbBufferSize : minCbBufferSize) : preferredCbBufferSize;
    AUDIO_INFO_LOG("Set CallbackBuffer with byte size: %{public}zu", cbBufferSize_);
    cbBuffer_ = std::make_unique<uint8_t[]>(cbBufferSize_);
    return;
}

int32_t RendererInClientInner::Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer,
    size_t metaBufferSize)
{
    AUDIO_ERR_LOG("Write with metaBuffer is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t RendererInClientInner::Write(uint8_t *buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write with callback is not supported");
    return WriteInner(buffer, bufferSize);
}

int32_t RendererInClientInner::WriteInner(uint8_t *buffer, size_t bufferSize)
{
    Trace trace("RendererInClient::Write " + std::to_string(bufferSize));
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr && bufferSize < MAX_WRITE_SIZE && bufferSize > 0, ERR_INVALID_PARAM,
        "invalid size is %{public}zu", bufferSize);

    std::lock_guard<std::mutex> lock(writeMutex_);

    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_) {
        AudioSystemManager::GetInstance()->RequestThreadPriority(gettid());
        needSetThreadPriority_ = false;
    }

    if (!hasFirstFrameWrited_) { OnFirstFrameWriting(); }

    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "Write: Illegal state:%{public}u", state_.load());

    // hold lock
    if (isBlendSet_) {
        audioBlend_.Process(buffer, bufferSize);
    }

    size_t targetSize = bufferSize;
    size_t offset = 0;
    while (targetSize > sizePerFrameInByte_) {
        // 1. write data into ring cache
        OptResult result = ringCache_->GetWritableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, bufferSize - targetSize, "RingCache write status "
            "invalid size is:%{public}zu", result.size);

        size_t writableSize = result.size;
        Trace::Count("RendererInClient::CacheBuffer->writableSize", writableSize);

        size_t writeSize = std::min(writableSize, targetSize);
        BufferWrap bufferWrap = {buffer + offset, writeSize};

        if (writeSize > 0) {
            result = ringCache_->Enqueue(bufferWrap);
            if (result.ret != OPERATION_SUCCESS) {
                // in plan: recall enqueue in some cases
                AUDIO_ERR_LOG("RingCache Enqueue failed ret:%{public}d size:%{public}zu", result.ret, result.size);
                break;
            }
            offset += writeSize;
            targetSize -= writeSize;
            clientWrittenBytes_ += writeSize;
        }

        // 2. copy data from cache to OHAudioBuffer
        result = ringCache_->GetReadableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, bufferSize - targetSize, "RingCache read status "
            "invalid size is:%{public}zu", result.size);
        size_t readableSize = result.size;
        Trace::Count("RendererInClient::CacheBuffer->readableSize", readableSize);

        if (readableSize < clientSpanSizeInByte_) {
            continue;
        }
        // if readable size is enough, we will call write data to server
        int32_t ret = WriteCacheData();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "WriteCacheData failed %{public}d", ret);
    }

    return bufferSize - targetSize;
}

int32_t RendererInClientInner::WriteCacheData()
{
    Trace traceCache("RendererInClientInner::WriteCacheData");
    int32_t sizeInFrame = clientBuffer_->GetAvailableDataFrames();
    CHECK_AND_RETURN_RET_LOG(sizeInFrame >= 0, ERROR, "GetAvailableDataFrames invalid, %{public}d", sizeInFrame);
    while (sizeInFrame * sizePerFrameInByte_ < clientSpanSizeInByte_) {
        // wait for server read some data
        std::unique_lock<std::mutex> lock(writeDataMutex_);
        int32_t timeout = offloadEnable_ ? OFFLOAD_OPERATION_TIMEOUT_IN_MS : WRITE_CACHE_TIMEOUT_IN_MS;
        std::cv_status stat = writeDataCV_.wait_for(lock, std::chrono::milliseconds(timeout));
        CHECK_AND_RETURN_RET_LOG(stat == std::cv_status::no_timeout, ERROR, "write data time out");
        if (state_ != RUNNING) { return ERR_ILLEGAL_STATE; }
        sizeInFrame = clientBuffer_->GetAvailableDataFrames();
    }
    BufferDesc desc = {};
    uint64_t curWriteIndex = clientBuffer_->GetCurWriteFrame();
    int32_t ret = clientBuffer_->GetWriteBuffer(curWriteIndex, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetWriteBuffer failed %{public}d", ret);
    OptResult result = ringCache_->Dequeue({desc.buffer, desc.bufLength});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringCache Dequeue failed %{public}d", result.ret);
    AUDIO_DEBUG_LOG("RendererInClientInner::WriteCacheData() curWriteIndex[%{public}" PRIu64 "], spanSizeInFrame_ "
        "%{public}u", curWriteIndex, spanSizeInFrame_);

    // volume process in client
    if (volumeRamp_.IsActive()) {
        // do not call SetVolume here.
        clientOldVolume_ = clientVolume_;
        clientVolume_ = volumeRamp_.GetRampVolume();
    }
    if (!IsVolumeSame(AUDIO_MAX_VOLUME, clientVolume_, AUDIO_VOLOMUE_EPSILON)) {
        Trace traceVol("RendererInClientInner::VolumeTools::Process " + std::to_string(clientVolume_));
        AudioChannel channel = clientConfig_.streamInfo.channels;
        ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(channel, clientVolume_, clientVolume_);
        int32_t volRet = VolumeTools::Process(desc, clientConfig_.streamInfo.format, mapVols);
        if (volRet != SUCCESS) {
            AUDIO_INFO_LOG("VolumeTools::Process error: %{public}d", volRet);
        }
    }
    DumpFileUtil::WriteDumpFile(dumpOutFd_, static_cast<void *>(desc.buffer), desc.bufLength);
    clientBuffer_->SetCurWriteFrame(curWriteIndex + spanSizeInFrame_);

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "WriteCacheData failed, null ipcStream_.");
    ipcStream_->UpdatePosition(); // notiify server update position
    HandleRendererPositionChanges(desc.bufLength);
    return SUCCESS;
}

void RendererInClientInner::HandleRendererPositionChanges(size_t bytesWritten)
{
    totalBytesWritten_ += bytesWritten;
    if (sizePerFrameInByte_ == 0) {
        AUDIO_ERR_LOG("HandleRendererPositionChanges: sizePerFrameInByte_ is 0");
        return;
    }
    int64_t writtenFrameNumber = totalBytesWritten_ / sizePerFrameInByte_;
    AUDIO_DEBUG_LOG("frame size: %{public}zu", sizePerFrameInByte_);

    {
        std::lock_guard<std::mutex> lock(markReachMutex_);
        if (!rendererMarkReached_) {
            AUDIO_DEBUG_LOG("Frame mark position: %{public}" PRId64", Total frames written: %{public}" PRId64,
                static_cast<int64_t>(rendererMarkPosition_), static_cast<int64_t>(writtenFrameNumber));
            if (writtenFrameNumber >= rendererMarkPosition_) {
                AUDIO_DEBUG_LOG("OnMarkReached %{public}" PRId64".", rendererMarkPosition_);
                SendRenderMarkReachedEvent(rendererMarkPosition_);
                rendererMarkReached_ = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(periodReachMutex_);
        rendererPeriodWritten_ += (totalBytesWritten_ / sizePerFrameInByte_);
        AUDIO_DEBUG_LOG("Frame period number: %{public}" PRId64", Total frames written: %{public}" PRId64,
            static_cast<int64_t>(rendererPeriodWritten_), static_cast<int64_t>(totalBytesWritten_));
        if (rendererPeriodWritten_ >= rendererPeriodSize_ && rendererPeriodSize_ > 0) {
            rendererPeriodWritten_ %= rendererPeriodSize_;
            AUDIO_DEBUG_LOG("OnPeriodReached, remaining frames: %{public}" PRId64,
                static_cast<int64_t>(rendererPeriodWritten_));
            SendRenderPeriodReachedEvent(rendererPeriodSize_);
        }
    }
}

// OnRenderMarkReach by eventHandler
void RendererInClientInner::SendRenderMarkReachedEvent(int64_t rendererMarkPosition)
{
    SafeSendCallbackEvent(RENDERER_MARK_REACHED_EVENT, rendererMarkPosition);
}

// OnRenderPeriodReach by eventHandler
void RendererInClientInner::SendRenderPeriodReachedEvent(int64_t rendererPeriodSize)
{
    SafeSendCallbackEvent(RENDERER_PERIOD_REACHED_EVENT, rendererPeriodSize);
}

int32_t RendererInClientInner::ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType)
{
    cmdType = CMD_FROM_CLIENT;
    switch (params) {
        case HANDLER_PARAM_NEW:
            state = NEW;
            break;
        case HANDLER_PARAM_PREPARED:
            state = PREPARED;
            break;
        case HANDLER_PARAM_RUNNING:
            state = RUNNING;
            break;
        case HANDLER_PARAM_STOPPED:
            state = STOPPED;
            break;
        case HANDLER_PARAM_RELEASED:
            state = RELEASED;
            break;
        case HANDLER_PARAM_PAUSED:
            state = PAUSED;
            break;
        case HANDLER_PARAM_STOPPING:
            state = STOPPING;
            break;
        case HANDLER_PARAM_RUNNING_FROM_SYSTEM:
            state = RUNNING;
            cmdType = CMD_FROM_SYSTEM;
            break;
        case HANDLER_PARAM_PAUSED_FROM_SYSTEM:
            state = PAUSED;
            cmdType = CMD_FROM_SYSTEM;
            break;
        default:
            state = INVALID;
            break;
    }
    return SUCCESS;
}

int32_t RendererInClientInner::StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType)
{
    if (cmdType == CMD_FROM_CLIENT) {
        params = static_cast<int64_t>(state);
        return SUCCESS;
    }
    switch (state) {
        case RUNNING:
            params = HANDLER_PARAM_RUNNING_FROM_SYSTEM;
            break;
        case PAUSED:
            params = HANDLER_PARAM_PAUSED_FROM_SYSTEM;
            break;
        default:
            params = HANDLER_PARAM_INVALID;
            break;
    }
    return SUCCESS;
}

int32_t RendererInClientInner::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    AUDIO_ERR_LOG("Read is not supported");
    return ERROR;
}


uint32_t RendererInClientInner::GetUnderflowCount()
{
    return underrunCount_;
}

void RendererInClientInner::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    // waiting for review
    std::lock_guard<std::mutex> lock(markReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "RendererPositionCallback is nullptr");
    rendererPositionCallback_ = callback;
    rendererMarkPosition_ = markPosition;
    rendererMarkReached_ = false;
}

void RendererInClientInner::UnsetRendererPositionCallback()
{
    // waiting for review
    std::lock_guard<std::mutex> lock(markReachMutex_);
    rendererPositionCallback_ = nullptr;
    rendererMarkPosition_ = 0;
    rendererMarkReached_ = false;
}

void RendererInClientInner::SetRendererPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    // waiting for review
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "RendererPeriodPositionCallback is nullptr");
    rendererPeriodPositionCallback_ = callback;
    rendererPeriodSize_ = periodPosition;
    totalBytesWritten_ = 0;
    rendererPeriodWritten_ = 0;
}

void RendererInClientInner::UnsetRendererPeriodPositionCallback()
{
    // waiting for review
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    rendererPeriodPositionCallback_ = nullptr;
    rendererPeriodSize_ = 0;
    totalBytesWritten_ = 0;
    rendererPeriodWritten_ = 0;
}

void RendererInClientInner::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::UnsetCapturerPositionCallback()
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::SetCapturerPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::UnsetCapturerPeriodPositionCallback()
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

int32_t RendererInClientInner::SetRendererSamplingRate(uint32_t sampleRate)
{
    AUDIO_ERR_LOG("SetRendererSamplingRate to %{publid}d is not supported", sampleRate);
    return ERROR;
}

uint32_t RendererInClientInner::GetRendererSamplingRate()
{
    return streamParams_.samplingRate;
}

int32_t RendererInClientInner::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    // bufferSizeInMsec is checked between 5ms and 20ms.
    bufferSizeInMsec_ = bufferSizeInMsec;
    AUDIO_INFO_LOG("SetBufferSizeInMsec to %{public}d", bufferSizeInMsec_);
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        uint64_t bufferDurationInUs = bufferSizeInMsec_ * AUDIO_US_PER_MS;
        InitCallbackBuffer(bufferDurationInUs);
    }
    return SUCCESS;
}

void RendererInClientInner::SetApplicationCachePath(const std::string cachePath)
{
    cachePath_ = cachePath;
    AUDIO_INFO_LOG("SetApplicationCachePath to %{public}s", cachePath_.c_str());
}

int32_t RendererInClientInner::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    if ((state_ != PREPARED) && (state_ != NEW)) {
        AUDIO_ERR_LOG("SetChannelBlendMode in invalid status:%{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    isBlendSet_ = true;
    audioBlend_.SetParams(blendMode, streamParams_.format, streamParams_.channels);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolumeWithRamp(float volume, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != INVALID) && (state_ != STOPPED),
        ERR_ILLEGAL_STATE, "Illegal state %{public}d", state_.load());

    if (FLOAT_COMPARE_EQ(clientVolume_, volume)) {
        AUDIO_INFO_LOG("set same volume %{publid}f", volume);
        return SUCCESS;
    }

    volumeRamp_.SetVolumeRampConfig(volume, clientVolume_, duration);
    return SUCCESS;
}

void RendererInClientInner::SetStreamTrackerState(bool trackerRegisteredState)
{
    streamTrackerRegistered_ = trackerRegisteredState;
}

void RendererInClientInner::GetSwitchInfo(IAudioStream::SwitchInfo& info)
{
    // in plan
}

IAudioStream::StreamClass RendererInClientInner::GetStreamClass()
{
    return PA_STREAM;
}
} // namespace AudioStandard
} // namespace OHOS
#endif // FAST_AUDIO_STREAM_H
