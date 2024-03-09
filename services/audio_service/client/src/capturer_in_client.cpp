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

#include "capturer_in_client.h"

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

#include "ipc_stream.h"
#include "audio_log.h"
#include "audio_errors.h"

#include "audio_manager_base.h"
#include "audio_ring_cache.h"
#include "audio_utils.h"
#include "audio_policy_manager.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "callback_handler.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const size_t MAX_CLIENT_READ_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t CREATE_TIMEOUT_IN_SECOND = 5; // 5S
static const int32_t OPERATION_TIMEOUT_IN_MS = 500; // 500ms
static const int32_t LOGLITMITTIMES = 20;
const uint64_t AUDIO_US_PER_MS = 1000;
const uint64_t AUDIO_US_PER_S = 1000000;
const uint64_t DEFAULT_BUF_DURATION_IN_USEC = 20000; // 20ms
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
const int64_t INVALID_FRAME_SIZE = -1;
static const int32_t SHORT_TIMEOUT_IN_MS = 20; // ms
static constexpr int CB_QUEUE_CAPACITY = 3;
}
class CapturerInClientInner : public CapturerInClient, public IStreamListener, public IHandler,
    public std::enable_shared_from_this<CapturerInClientInner> {
public:
    CapturerInClientInner(AudioStreamType eStreamType, int32_t appUid);
    ~CapturerInClientInner();

    // IStreamListener
    int32_t OnOperationHandled(Operation operation, int64_t result) override;

    void SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId) override;

    void SetRendererInfo(const AudioRendererInfo &rendererInfo) override;
    void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) override;
    int32_t GetAudioStreamInfo(AudioStreamParams &info) override;
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj) override;
    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid, SourceType sourceType =
        SOURCE_TYPE_MIC) override;
    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;
    State GetState() override;
    int32_t GetAudioSessionID(uint32_t &sessionID) override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    int32_t GetBufferSize(size_t &bufferSize) override;
    int32_t GetFrameCount(uint32_t &frameCount) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetAudioStreamType(AudioStreamType audioStreamType) override;
    float GetVolume() override;
    int32_t SetVolume(float volume) override;
    int32_t SetRenderRate(AudioRendererRate renderRate) override;
    AudioRendererRate GetRenderRate() override;
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) override;
    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    int32_t ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t []> &outBuffer,
        int32_t &outBufferSize) override;
    int32_t WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize) override;

    // callback mode api
    AudioRenderMode GetRenderMode() override;
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override;
    AudioCaptureMode GetCaptureMode() override;
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t Clear() override;
    int32_t GetBufQueueState(BufferQueueState &bufState) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;

    int32_t SetLowPowerVolume(float volume) override;
    float GetLowPowerVolume() override;
    int32_t UnsetOffloadMode() override;
    int32_t SetOffloadMode(int32_t state, bool isAppBack) override;
    float GetSingleStreamVolume() override;
    AudioEffectMode GetAudioEffectMode() override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) override;
    int64_t GetFramesRead() override;
    int64_t GetFramesWritten() override;

    void SetInnerCapturerState(bool isInnerCapturer) override;
    void SetWakeupCapturerState(bool isWakeupCapturer) override;
    void SetPrivacyType(AudioPrivacyType privacyType) override;
    void SetCapturerSource(int capturerSource) override;

    // Common APIs
    bool StartAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool PauseAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool StopAudioStream() override;
    bool FlushAudioStream() override;
    bool ReleaseAudioStream(bool releaseRunner = true) override;

    // Playback related APIs
    bool DrainAudioStream() override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize) override;
    void SetPreferredFrameSize(int32_t frameSize) override;
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void OnFirstFrameWriting() override;

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;

    // Position and period callbacks
    void SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) override;
    void UnsetCapturerPositionCallback() override;
    void SetCapturerPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    void SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) override;
    void UnsetRendererPositionCallback() override;
    void SetRendererPeriodPositionCallback(int64_t periodPosition,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;

    uint32_t GetUnderflowCount() override;
    uint32_t GetRendererSamplingRate() override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) override;
    int32_t SetBufferSizeInMsec(int32_t bufferSizeInMsec) override;
    void SetApplicationCachePath(const std::string cachePath) override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    void SetStreamTrackerState(bool trackerRegisteredState) override;
    void GetSwitchInfo(IAudioStream::SwitchInfo& info) override;

    IAudioStream::StreamClass GetStreamClass() override;

    static void AudioServerDied(pid_t pid);

    void OnHandle(uint32_t code, int64_t data) override;
    void InitCallbackHandler();
    void SafeSendCallbackEvent(uint32_t eventCode, int64_t data);

    void SendCapturerMarkReachedEvent(int64_t capturerMarkPosition);
    void SendCapturerPeriodReachedEvent(int64_t capturerPeriodSize);

    void HandleCapturerPositionChanges(size_t bytesRead);
    void HandleStateChangeEvent(int64_t data);
    void HandleCapturerMarkReachedEvent(int64_t capturerMarkPosition);
    void HandleCapturerPeriodReachedEvent(int64_t capturerPeriodNumber);

    static const sptr<IStandardAudioService> GetAudioServerProxy();
private:
    void RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj);
    void UpdateTracker(const std::string &updateCase);

    int32_t DeinitIpcStream();

    int32_t InitIpcStream();

    const AudioProcessConfig ConstructConfig();

    int32_t InitCacheBuffer(size_t targetSize);
    int32_t InitSharedBuffer();
    int32_t FlushRingCache();

    int32_t StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType);
    int32_t ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType);

    void InitCallbackBuffer(uint64_t bufferDurationInUs);
    void ReadCallbackFunc();
    // for callback mode. Check status if not running, wait for start or release.
    bool WaitForRunning();

    int32_t HandleCapturerRead(size_t &readSize, size_t &userSize, uint8_t &buffer, bool isBlockingRead);
private:
    AudioStreamType eStreamType_;
    int32_t appUid_;
    uint32_t sessionId_ = 0;
    int32_t clientUid_ = -1;
    int32_t clientPid_ = -1;
    uint32_t appTokenId_ = 0;

    uint32_t readLogTimes_ = 0;

    std::unique_ptr<AudioStreamTracker> audioStreamTracker_;
    bool streamTrackerRegistered_ = false;

    AudioRendererInfo rendererInfo_; // not in use
    AudioCapturerInfo capturerInfo_;

    int32_t bufferSizeInMsec_ = 20; // 20ms
    std::string cachePath_;

    // callback mode
    AudioCaptureMode capturerMode_ = CAPTURE_MODE_NORMAL;
    std::thread callbackLoop_; // thread for callback to client and write.
    std::atomic<bool> cbThreadReleased_ = true;
    std::mutex readCbMutex_; // lock for change or use callback
    std::condition_variable cbThreadCv_;
    std::shared_ptr<AudioCapturerReadCallback> readCb_ = nullptr;
    std::mutex cbBufferMutex_;
    std::unique_ptr<uint8_t[]> cbBuffer_ {nullptr};
    size_t cbBufferSize_ = 0;
    SafeBlockQueue<BufferDesc> cbBufferQueue_; // only one cbBuffer_

    bool isInnerCapturer_ = false;
    bool isWakeupCapturer_ = false;

    bool needSetThreadPriority_ = true;

    AudioStreamParams streamParams_; // in plan: replace it with AudioCapturerParams

    // callbacks
    std::mutex streamCbMutex_;
    std::weak_ptr<AudioStreamCallback> streamCallback_;

    size_t cacheSizeInByte_ = 0;
    uint32_t spanSizeInFrame_ = 0;
    size_t clientSpanSizeInByte_ = 0;
    size_t sizePerFrameInByte_ = 4; // 16bit 2ch as default

    // using this lock when change status_
    std::mutex statusMutex_;
    std::atomic<State> state_ = INVALID;
    // for status operation wait and notify
    std::mutex callServerMutex_;
    std::condition_variable callServerCV_;

    Operation notifiedOperation_ = MAX_OPERATION_CODE;
    int64_t notifiedResult_ = 0;

    // read data
    std::mutex readDataMutex_;
    std::condition_variable readDataCV_;

    uint32_t overflowCount_ = 0;
    // ipc stream related
    AudioProcessConfig clientConfig_;
    sptr<IpcStreamListenerImpl> listener_ = nullptr;
    sptr<IpcStream> ipcStream_ = nullptr;
    std::shared_ptr<OHAudioBuffer> clientBuffer_ = nullptr;

    // buffer handle
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    std::mutex readMutex_; // used for prevent multi thread call read

    // Mark reach and period reach callback
    int64_t totalBytesRead_ = 0;
    std::mutex markReachMutex_;
    bool capturerMarkReached_ = false;
    int64_t capturerMarkPosition_ = 0;
    std::shared_ptr<CapturerPositionCallback> capturerPositionCallback_ = nullptr;

    std::mutex periodReachMutex_;
    int64_t capturerPeriodSize_ = 0;
    int64_t capturerPeriodRead_ = 0;
    std::shared_ptr<CapturerPeriodPositionCallback> capturerPeriodPositionCallback_ = nullptr;

    // Event handler
    bool runnerReleased_ = false;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;

    bool paramsIsSet_ = false;

    enum {
        STATE_CHANGE_EVENT = 0,
        RENDERER_MARK_REACHED_EVENT,
        RENDERER_PERIOD_REACHED_EVENT,
        CAPTURER_PERIOD_REACHED_EVENT,
        CAPTURER_MARK_REACHED_EVENT,
    };

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

std::shared_ptr<CapturerInClient> CapturerInClient::GetInstance(AudioStreamType eStreamType, int32_t appUid)
{
    return std::make_shared<CapturerInClientInner>(eStreamType, appUid);
}

CapturerInClientInner::CapturerInClientInner(AudioStreamType eStreamType, int32_t appUid) : eStreamType_(eStreamType),
    appUid_(appUid), cbBufferQueue_(CB_QUEUE_CAPACITY)
{
    AUDIO_INFO_LOG("Create with StreamType:%{public}d appUid:%{public}d ", eStreamType_, appUid_);
    audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AUDIO_MODE_RECORD, appUid);
    state_ = NEW;
}

CapturerInClientInner::~CapturerInClientInner()
{
    AUDIO_INFO_LOG("~CapturerInClientInner()");
    CapturerInClientInner::ReleaseAudioStream(true);
}

int32_t CapturerInClientInner::OnOperationHandled(Operation operation, int64_t result)
{
    // read/write operation may print many log, use debug.
    if (operation == UPDATE_STREAM) {
        AUDIO_DEBUG_LOG("OnOperationHandled() UPDATE_STREAM result:%{public}" PRId64".", result);
        // notify write if blocked
        readDataCV_.notify_all();
        return SUCCESS;
    }

    if (operation == BUFFER_OVERFLOW) {
        overflowCount_++;
        AUDIO_WARNING_LOG("recv overflow %{public}d", overflowCount_);
        // in plan next: do more to reduce overflow
        readDataCV_.notify_all();
        return SUCCESS;
    }

    AUDIO_INFO_LOG("OnOperationHandled() recv operation:%{public}d result:%{public}" PRId64".", operation, result);
    std::unique_lock<std::mutex> lock(callServerMutex_);
    notifiedOperation_ = operation;
    notifiedResult_ = result;
    callServerCV_.notify_all();
    return SUCCESS;
}

void CapturerInClientInner::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId)
{
    AUDIO_INFO_LOG("Capturer set client PID: %{public}d, UID: %{public}d", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
    return;
}

void CapturerInClientInner::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    AUDIO_WARNING_LOG("SetRendererInfo is not supported");
    return;
}

void CapturerInClientInner::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    capturerInfo_ = capturerInfo;
    AUDIO_INFO_LOG("SetCapturerInfo with SourceType %{public}d flag %{public}d", capturerInfo_.sourceType,
        capturerInfo_.capturerFlags);
    return;
}

void CapturerInClientInner::RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    if (audioStreamTracker_ && audioStreamTracker_.get() && !streamTrackerRegistered_) {
        // make sure sessionId_ is set before.
        AUDIO_INFO_LOG("Calling register tracker, sessionid = %{public}d", sessionId_);
        AudioRegisterTrackerInfo registerTrackerInfo;

        registerTrackerInfo.sessionId = sessionId_;
        registerTrackerInfo.clientPid = clientPid_;
        registerTrackerInfo.state = state_;
        registerTrackerInfo.rendererInfo = rendererInfo_;
        registerTrackerInfo.capturerInfo = capturerInfo_;

        audioStreamTracker_->RegisterTracker(registerTrackerInfo, proxyObj);
        streamTrackerRegistered_ = true;
    }
}

void CapturerInClientInner::UpdateTracker(const std::string &updateCase)
{
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("Capturer:Calling Update tracker for %{public}s", updateCase.c_str());
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }
}

int32_t CapturerInClientInner::SetAudioStreamInfo(const AudioStreamParams info,
    const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    AUDIO_INFO_LOG("AudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d, stream type:"
        " %{public}d, encoding type: %{public}d", info.samplingRate, info.channels, info.format, eStreamType_,
        info.encoding);
    AudioXCollie guard("CapturerInClientInner::SetAudioStreamInfo", CREATE_TIMEOUT_IN_SECOND);
    if (!IsFormatValid(info.format) || !IsCapturerChannelValid(info.channels) || !IsEncodingTypeValid(info.encoding) ||
        !IsSamplingRateValid(info.samplingRate)) {
        AUDIO_ERR_LOG("CapturerInClient: Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }

    CHECK_AND_RETURN_RET_LOG(IAudioStream::GetByteSizePerFrame(info, sizePerFrameInByte_) == SUCCESS,
        ERROR_INVALID_PARAM, "GetByteSizePerFrame failed with invalid params");

    if (state_ != NEW) {
        AUDIO_INFO_LOG("State is %{public}d, not new, release existing stream and recreate.", state_.load());
        int32_t ret = DeinitIpcStream();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "release existing stream failed.");
    }

    streamParams_ = info; // keep it for later use
    paramsIsSet_ = true;
    int32_t initRet = InitIpcStream();
    CHECK_AND_RETURN_RET_LOG(initRet == SUCCESS, initRet, "Init stream failed: %{public}d", initRet);
    state_ = PREPARED;

    RegisterTracker(proxyObj);
    return SUCCESS;
}

std::mutex g_serverMutex;
sptr<IStandardAudioService> g_ServerProxy = nullptr;
const sptr<IStandardAudioService> CapturerInClientInner::GetAudioServerProxy()
{
    std::lock_guard<std::mutex> lock(g_serverMutex);
    if (g_ServerProxy == nullptr) {
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
        g_ServerProxy = iface_cast<IStandardAudioService>(object);
        if (g_ServerProxy == nullptr) {
            AUDIO_ERR_LOG("GetAudioServerProxy: get audio service proxy failed");
            return nullptr;
        }

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient = new(std::nothrow) AudioServerDeathRecipient(getpid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb(std::bind(&CapturerInClientInner::AudioServerDied,
                std::placeholders::_1));
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("GetAudioServerProxy: failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_ServerProxy;
    return gasp;
}

void CapturerInClientInner::AudioServerDied(pid_t pid)
{
    AUDIO_INFO_LOG("audio server died clear proxy, will restore proxy in next call");
    std::lock_guard<std::mutex> lock(g_serverMutex);
    g_ServerProxy = nullptr;
}

void CapturerInClientInner::OnHandle(uint32_t code, int64_t data)
{
    AUDIO_DEBUG_LOG("On handle event, event code: %{public}d, data: %{public}" PRIu64 "", code, data);
    switch (code) {
        case STATE_CHANGE_EVENT:
            HandleStateChangeEvent(data);
            break;
        case RENDERER_MARK_REACHED_EVENT:
            HandleCapturerMarkReachedEvent(data);
            break;
        case RENDERER_PERIOD_REACHED_EVENT:
            HandleCapturerPeriodReachedEvent(data);
            break;
        default:
            break;
    }
}

void CapturerInClientInner::HandleStateChangeEvent(int64_t data)
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

void CapturerInClientInner::HandleCapturerMarkReachedEvent(int64_t capturerMarkPosition)
{
    AUDIO_DEBUG_LOG("Start HandleCapturerMarkReachedEvent");
    std::unique_lock<std::mutex> lock(markReachMutex_);
    if (capturerPositionCallback_) {
        capturerPositionCallback_->OnMarkReached(capturerMarkPosition);
    }
}

void CapturerInClientInner::HandleCapturerPeriodReachedEvent(int64_t capturerPeriodNumber)
{
    AUDIO_DEBUG_LOG("Start HandleCapturerPeriodReachedEvent");
    std::unique_lock<std::mutex> lock(periodReachMutex_);
    if (capturerPeriodPositionCallback_) {
        capturerPeriodPositionCallback_->OnPeriodReached(capturerPeriodNumber);
    }
}

// OnCapturerMarkReach by eventHandler
void CapturerInClientInner::SendCapturerMarkReachedEvent(int64_t capturerMarkPosition)
{
    SafeSendCallbackEvent(RENDERER_MARK_REACHED_EVENT, capturerMarkPosition);
}

// OnCapturerPeriodReach by eventHandler
void CapturerInClientInner::SendCapturerPeriodReachedEvent(int64_t capturerPeriodSize)
{
    SafeSendCallbackEvent(RENDERER_PERIOD_REACHED_EVENT, capturerPeriodSize);
}

int32_t CapturerInClientInner::ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType)
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

int32_t CapturerInClientInner::StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType)
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

void CapturerInClientInner::SafeSendCallbackEvent(uint32_t eventCode, int64_t data)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    AUDIO_INFO_LOG("Send callback event, code: %{public}u, data: %{public}" PRId64 "", eventCode, data);
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr && runnerReleased_ == false, "Runner is Released");
    callbackHandler_->SendCallbackEvent(eventCode, data);
}

void CapturerInClientInner::InitCallbackHandler()
{
    if (callbackHandler_ == nullptr) {
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this());
    }
}

// call this without lock, we should be able to call deinit in any case.
int32_t CapturerInClientInner::DeinitIpcStream()
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, SUCCESS, "IpcStream is already nullptr");
    ipcStream_->Release();
    // in plan:
    ipcStream_ = nullptr;
    ringCache_->ResetBuffer();
    return SUCCESS;
}

const AudioProcessConfig CapturerInClientInner::ConstructConfig()
{
    AudioProcessConfig config = {};
    // in plan: get token id
    config.appInfo.appPid = clientPid_;
    config.appInfo.appUid = clientUid_;
    config.appInfo.appTokenId = appTokenId_;

    config.streamInfo.channels = static_cast<AudioChannel>(streamParams_.channels);
    config.streamInfo.encoding = static_cast<AudioEncodingType>(streamParams_.encoding);
    config.streamInfo.format = static_cast<AudioSampleFormat>(streamParams_.format);
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(streamParams_.samplingRate);

    config.audioMode = AUDIO_MODE_RECORD;

    if (capturerInfo_.capturerFlags != 0) {
        AUDIO_WARNING_LOG("ConstructConfig find Capturer flag invalid:%{public}d", capturerInfo_.capturerFlags);
        capturerInfo_.capturerFlags = 0;
    }
    config.capturerInfo = capturerInfo_;

    config.rendererInfo = {};

    config.streamType = eStreamType_;

    config.isInnerCapturer = isInnerCapturer_;
    config.isWakeupCapturer = isWakeupCapturer_;

    clientConfig_ = config;
    return config;
}

int32_t CapturerInClientInner::InitSharedBuffer()
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "InitSharedBuffer failed, null ipcStream_.");
    int32_t ret = ipcStream_->ResolveBuffer(clientBuffer_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && clientBuffer_ != nullptr, ret, "ResolveBuffer failed:%{public}d", ret);

    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ret = clientBuffer_->GetSizeParameter(totalSizeInFrame, spanSizeInFrame_, byteSizePerFrame);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && byteSizePerFrame == sizePerFrameInByte_, ret, "ResolveBuffer failed"
        ":%{public}d", ret);

    clientSpanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame;

    AUDIO_INFO_LOG("totalSizeInFrame_[%{public}u] spanSizeInFrame_[%{public}u] sizePerFrameInByte_["
        "%{public}zu] clientSpanSizeInByte_[%{public}zu]", totalSizeInFrame, spanSizeInFrame_, sizePerFrameInByte_,
        clientSpanSizeInByte_);

    return SUCCESS;
}

// InitCacheBuffer should be able to modify the cache size between clientSpanSizeInByte_ and 4 * clientSpanSizeInByte_
int32_t CapturerInClientInner::InitCacheBuffer(size_t targetSize)
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

int32_t CapturerInClientInner::InitIpcStream()
{
    AUDIO_INFO_LOG("Init Ipc stream");
    AudioProcessConfig config = ConstructConfig();

    sptr<IStandardAudioService> gasp = CapturerInClientInner::GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_OPERATION_FAILED, "Create failed, can not get service.");
    sptr<IRemoteObject> ipcProxy = gasp->CreateAudioProcess(config); // in plan: add ret
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, ERR_OPERATION_FAILED, "failed with null ipcProxy.");
    ipcStream_ = iface_cast<IpcStream>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "failed when iface_cast.");

    // in plan: old listener_ is destoried here, will server receive dieth notify?
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

int32_t CapturerInClientInner::GetAudioStreamInfo(AudioStreamParams &info)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, ERR_OPERATION_FAILED, "Params is not set");
    info = streamParams_;
    return SUCCESS;
}

bool CapturerInClientInner::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    return AudioPolicyManager::GetInstance().CheckRecordingCreate(appTokenId, appFullTokenId, appUid, sourceType);
}

bool CapturerInClientInner::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    return AudioPolicyManager::GetInstance().CheckRecordingStateChange(appTokenId, appFullTokenId, appUid, state);
}

int32_t CapturerInClientInner::GetAudioSessionID(uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != NEW), ERR_ILLEGAL_STATE,
        "State error %{public}d", state_.load());
    sessionID = sessionId_;
    return SUCCESS;
}

State CapturerInClientInner::GetState()
{
    return state_;
}

bool CapturerInClientInner::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());
    int64_t currentReadPos = totalBytesRead_ / sizePerFrameInByte_;
    timestamp.framePosition = currentReadPos;

    uint64_t writePos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");
    clientBuffer_->GetHandleInfo(writePos, handleTime);
    if (writePos == 0 || handleTime == 0) {
        AUDIO_WARNING_LOG("GetHandleInfo may failed");
    }

    int64_t deltaPos = writePos >= static_cast<uint64_t>(currentReadPos) ? writePos - currentReadPos : 0;
    int64_t tempLatency = 25000000; // 25000000 -> 25 ms
    int64_t deltaTime = deltaPos * AUDIO_MS_PER_SECOND / streamParams_.samplingRate * AUDIO_US_PER_S;

    handleTime = handleTime + deltaTime + tempLatency;
    timestamp.time.tv_sec = static_cast<time_t>(handleTime / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(handleTime % AUDIO_NS_PER_SECOND);

    return true;
}

bool CapturerInClientInner::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base);
}

int32_t CapturerInClientInner::GetBufferSize(size_t &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Capturer stream is released");
    bufferSize = clientSpanSizeInByte_;
    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
        bufferSize = cbBufferSize_;
    }
    AUDIO_DEBUG_LOG("Buffer size is %{public}zu, mode is %{public}s", bufferSize, capturerMode_ == CAPTURE_MODE_NORMAL ?
        "CAPTURE_MODE_NORMAL" : "CAPTURE_MODE_CALLBACK");
    return SUCCESS;
}

int32_t CapturerInClientInner::GetFrameCount(uint32_t &frameCount)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Capturer stream is released");
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ != 0, ERR_ILLEGAL_STATE, "sizePerFrameInByte_ is 0!");
    frameCount = spanSizeInFrame_;
    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
        frameCount = cbBufferSize_ / sizePerFrameInByte_;
    }
    AUDIO_INFO_LOG("Frame count is %{public}u, mode is %{public}s", frameCount, capturerMode_ == CAPTURE_MODE_NORMAL ?
        "CAPTURE_MODE_NORMAL" : "CAPTURE_MODE_CALLBACK");
    return SUCCESS;
}

int32_t CapturerInClientInner::GetLatency(uint64_t &latency)
{
    // GetLatency is never called in audio_capturer.cpp
    latency = 150000; // unit is us, 150000 is 150ms
    return ERROR;
}

int32_t CapturerInClientInner::SetAudioStreamType(AudioStreamType audioStreamType)
{
    AUDIO_ERR_LOG("Change stream type %{public}d to %{public}d is not supported", eStreamType_, audioStreamType);
    return ERROR;
}

int32_t CapturerInClientInner::SetVolume(float volume)
{
    AUDIO_WARNING_LOG("SetVolume is only for renderer");
    return ERROR;
}

float CapturerInClientInner::GetVolume()
{
    AUDIO_WARNING_LOG("GetVolume is only for renderer");
    return 0.0;
}

int32_t CapturerInClientInner::SetSpeed(float speed)
{
    AUDIO_ERR_LOG("SetSpeed is not supported");
    return ERROR;
}

float CapturerInClientInner::GetSpeed()
{
    AUDIO_ERR_LOG("GetSpeed is not supported");
    return 1.0;
}

int32_t CapturerInClientInner::ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t []> &outBuffer,
    int32_t &outBufferSize)
{
    AUDIO_ERR_LOG("ChangeSpeed is not supported");
    return ERROR;
}

int32_t CapturerInClientInner::WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize)
{
    AUDIO_ERR_LOG("Speed is not supported");
    return ERROR;
}


int32_t CapturerInClientInner::SetRenderRate(AudioRendererRate renderRate)
{
    AUDIO_WARNING_LOG("SetRenderRate is only for renderer");
    return ERROR;
}

AudioRendererRate CapturerInClientInner::GetRenderRate()
{
    AUDIO_WARNING_LOG("GetRenderRate is only for renderer");
    return RENDER_RATE_NORMAL; // not supported
}

int32_t CapturerInClientInner::SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback)
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

int32_t CapturerInClientInner::SetRenderMode(AudioRenderMode renderMode)
{
    AUDIO_WARNING_LOG("SetRenderMode is only for renderer");
    return ERROR;
}

AudioRenderMode CapturerInClientInner::GetRenderMode()
{
    AUDIO_WARNING_LOG("GetRenderMode is only for renderer");
    return RENDER_MODE_NORMAL; // not supported
}

int32_t CapturerInClientInner::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    AUDIO_WARNING_LOG("SetRendererWriteCallback is only for renderer");
    return ERROR;
}

void CapturerInClientInner::InitCallbackBuffer(uint64_t bufferDurationInUs)
{
    if (bufferDurationInUs > MAX_BUF_DURATION_IN_USEC) {
        AUDIO_ERR_LOG("InitCallbackBuffer with invalid duration %{public}" PRIu64", use default instead.",
            bufferDurationInUs);
        bufferDurationInUs = DEFAULT_BUF_DURATION_IN_USEC;
    }
    // Calculate buffer size based on duration.
    cbBufferSize_ = static_cast<size_t>(bufferDurationInUs * streamParams_.samplingRate / AUDIO_US_PER_S) *
        sizePerFrameInByte_;
    AUDIO_INFO_LOG("InitCallbackBuffer with duration %{public}" PRIu64", size: %{public}zu", bufferDurationInUs,
        cbBufferSize_);
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    cbBuffer_ = std::make_unique<uint8_t[]>(cbBufferSize_);
    BufferDesc temp = {cbBuffer_.get(), cbBufferSize_, cbBufferSize_};
    cbBufferQueue_.Push(temp);
}

int32_t CapturerInClientInner::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_INFO_LOG("Set mode to %{public}s", captureMode == CAPTURE_MODE_NORMAL ? "CAPTURE_MODE_NORMAL" :
        "CAPTURE_MODE_CALLBACK");
    if (capturerMode_ == captureMode) {
        return SUCCESS;
    }

    // capturerMode_ is inited as CAPTURE_MODE_NORMAL, can only be set to CAPTURE_MODE_CALLBACK.
    if (capturerMode_ == CAPTURE_MODE_CALLBACK && captureMode == CAPTURE_MODE_NORMAL) {
        AUDIO_ERR_LOG("Set capturer mode from callback to normal is not supported.");
        return ERR_INCORRECT_MODE;
    }

    // state check
    if (state_ != PREPARED && state_ != NEW) {
        AUDIO_ERR_LOG("Set capturer mode failed. invalid state:%{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    capturerMode_ = captureMode;

    // init callbackLoop_
    callbackLoop_ = std::thread(&CapturerInClientInner::ReadCallbackFunc, this);
    pthread_setname_np(callbackLoop_.native_handle(), "OS_AudioReadCB");

    std::unique_lock<std::mutex> threadStartlock(statusMutex_);
    bool stopWaiting = cbThreadCv_.wait_for(threadStartlock, std::chrono::milliseconds(SHORT_TIMEOUT_IN_MS), [this] {
        return cbThreadReleased_ == false; // When thread is started, cbThreadReleased_ will be false. So stop waiting.
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("Init OS_AudioReadCB thread time out");
    }

    CHECK_AND_RETURN_RET_LOG(streamParams_.samplingRate != 0, ERR_ILLEGAL_STATE, "invalid sample rate");

    uint64_t bufferDurationInUs = spanSizeInFrame_ * AUDIO_US_PER_S / streamParams_.samplingRate;
    InitCallbackBuffer(bufferDurationInUs);
    return SUCCESS;
}

AudioCaptureMode CapturerInClientInner::GetCaptureMode()
{
    AUDIO_INFO_LOG("capturer mode is %{public}s", capturerMode_ == CAPTURE_MODE_NORMAL ? "CAPTURE_MODE_NORMAL" :
        "CAPTURE_MODE_CALLBACK");
    return capturerMode_;
}

int32_t CapturerInClientInner::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Invalid null callback");
    CHECK_AND_RETURN_RET_LOG(capturerMode_ == CAPTURE_MODE_CALLBACK, ERR_INCORRECT_MODE, "incorrect capturer mode");
    std::lock_guard<std::mutex> lock(readCbMutex_);
    readCb_ = callback;
    return SUCCESS;
}

bool CapturerInClientInner::WaitForRunning()
{
    Trace trace("CapturerInClientInner::WaitForRunning");
    // check capturer state_: call client write only in running else wait on statusMutex_
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
            AUDIO_INFO_LOG("Wait timeout, current state_ is %{public}d", state_.load()); // wait 0.5s
            return false;
        }
    }
    return true;
}

void CapturerInClientInner::ReadCallbackFunc()
{
    AUDIO_INFO_LOG("Thread start, sessionID :%{public}d", sessionId_);
    cbThreadReleased_ = false;

    // Modify thread priority is not need as first call read will do these work.
    cbThreadCv_.notify_one();

    // start loop
    while (!cbThreadReleased_) {
        Trace traceLoop("CapturerInClientInner::WriteCallbackFunc");
        if (!WaitForRunning()) {
            continue;
        }

        // If client didn't call GetBufferDesc/Enqueue in OnReadData, pop will block here.
        BufferDesc temp = cbBufferQueue_.Pop();
        if (temp.buffer == nullptr) {
            AUDIO_WARNING_LOG("Queue pop error: get nullptr.");
            break;
        }

        std::unique_lock<std::mutex> lockBuffer(cbBufferMutex_);
        // call read here.
        int32_t result = Read(*temp.buffer, temp.bufLength, true); // blocking read
        if (result < 0 || result != static_cast<int32_t>(cbBufferSize_)) {
            AUDIO_WARNING_LOG("Call read error, ret:%{public}d, cbBufferSize_:%{public}zu", result, cbBufferSize_);
        }
        if (state_ != RUNNING) { continue; }
        lockBuffer.unlock();

        // call client read
        Trace traceCb("CapturerInClientInner::OnReadData");
        std::unique_lock<std::mutex> lockCb(readCbMutex_);
        if (readCb_ != nullptr) {
            readCb_->OnReadData(cbBufferSize_);
        }
        lockCb.unlock();
        traceCb.End();
    }
    AUDIO_INFO_LOG("CBThread end sessionID :%{public}d", sessionId_);
}


int32_t CapturerInClientInner::GetBufferDesc(BufferDesc &bufDesc)
{
    Trace trace("CapturerInClientInner::GetBufferDesc");
    if (capturerMode_ != CAPTURE_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Not supported. mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    bufDesc.buffer = cbBuffer_.get();
    bufDesc.bufLength = cbBufferSize_;
    bufDesc.dataLength = cbBufferSize_;
    return SUCCESS;
}

int32_t CapturerInClientInner::GetBufQueueState(BufferQueueState &bufState)
{
    Trace trace("CapturerInClientInner::GetBufQueueState");
    if (capturerMode_ != CAPTURE_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Not supported, mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    // only one buffer in queue.
    bufState.numBuffers = 1;
    bufState.currentIndex = 0;
    return SUCCESS;
}

int32_t CapturerInClientInner::Enqueue(const BufferDesc &bufDesc)
{
    Trace trace("CapturerInClientInner::Enqueue");
    if (capturerMode_ != CAPTURE_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Not supported, mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::lock_guard<std::mutex> lock(cbBufferMutex_);

    if (bufDesc.bufLength != cbBufferSize_ || bufDesc.dataLength != cbBufferSize_) {
        AUDIO_ERR_LOG("Enqueue invalid bufLength:%{public}zu or dataLength:%{public}zu, should be %{public}zu",
            bufDesc.bufLength, bufDesc.dataLength, cbBufferSize_);
        return ERR_INVALID_INDEX;
    }
    if (bufDesc.buffer != cbBuffer_.get()) {
        AUDIO_WARNING_LOG("Enqueue buffer is not from us.");
    }

    // if Enqueue is not called in OnReadData, loop thread will block on pop, wait for the Push call here.
    BufferDesc temp = {cbBuffer_.get(), cbBufferSize_, cbBufferSize_};
    cbBufferQueue_.Push(temp);
    // Call read may block, so put it in loop callbackLoop_
    return SUCCESS;
}

int32_t CapturerInClientInner::Clear()
{
    Trace trace("CapturerInClientInner::Clear");
    if (capturerMode_ != CAPTURE_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Not supported, mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    int32_t ret = memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "Clear buffer fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t CapturerInClientInner::SetLowPowerVolume(float volume)
{
    AUDIO_WARNING_LOG("SetLowPowerVolume is only for renderer");
    return ERROR;
}

float CapturerInClientInner::GetLowPowerVolume()
{
    AUDIO_WARNING_LOG("GetLowPowerVolume is only for renderer");
    return 0.0;
}

int32_t CapturerInClientInner::SetOffloadMode(int32_t state, bool isAppBack)
{
    AUDIO_WARNING_LOG("SetOffloadMode is only for renderer");
    return ERROR;
}

int32_t CapturerInClientInner::UnsetOffloadMode()
{
    AUDIO_WARNING_LOG("UnsetOffloadMode is only for renderer");
    return ERROR;
}

float CapturerInClientInner::GetSingleStreamVolume()
{
    AUDIO_WARNING_LOG("GetSingleStreamVolume is only for renderer");
    return 0.0;
}

AudioEffectMode CapturerInClientInner::GetAudioEffectMode()
{
    AUDIO_WARNING_LOG("GetAudioEffectMode is only for renderer");
    return EFFECT_NONE;
}

int32_t CapturerInClientInner::SetAudioEffectMode(AudioEffectMode effectMode)
{
    AUDIO_WARNING_LOG("SetAudioEffectMode is only for renderer");
    return ERROR;
}

int64_t CapturerInClientInner::GetFramesWritten()
{
    AUDIO_WARNING_LOG("GetFramesWritten is only for renderer");
    return -1;
}

int64_t CapturerInClientInner::GetFramesRead()
{
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ != 0, INVALID_FRAME_SIZE, "sizePerFrameInByte_ is 0!");
    int64_t readFrameNumber = totalBytesRead_ / sizePerFrameInByte_;
    return readFrameNumber;
}

// Will only take effect before SetAudioStreaminfo
void CapturerInClientInner::SetInnerCapturerState(bool isInnerCapturer)
{
    isInnerCapturer_ = isInnerCapturer;
    AUDIO_INFO_LOG("SetInnerCapturerState %{public}s", (isInnerCapturer_ ? "true" : "false"));
    return;
}

// Will only take effect before SetAudioStreaminfo
void CapturerInClientInner::SetWakeupCapturerState(bool isWakeupCapturer)
{
    isWakeupCapturer_ = isWakeupCapturer;
    AUDIO_INFO_LOG("SetWakeupCapturerState %{public}s", (isWakeupCapturer_ ? "true" : "false"));
    return;
}

void CapturerInClientInner::SetCapturerSource(int capturerSource)
{
    // capturerSource is kept in capturerInfo_, no need to be set again.
    (void)capturerSource;
    return;
}

void CapturerInClientInner::SetPrivacyType(AudioPrivacyType privacyType)
{
    AUDIO_WARNING_LOG("SetPrivacyType is only for renderer");
    return;
}

bool CapturerInClientInner::StartAudioStream(StateChangeCmdType cmdType)
{
    Trace trace("CapturerInClientInner::StartAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != PREPARED && state_ != STOPPED && state_ != PAUSED) {
        AUDIO_ERR_LOG("Start failed Illegal state: %{public}d", state_.load());
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Start();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Start call server failed: %{public}u", ret);
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
    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
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

bool CapturerInClientInner::PauseAudioStream(StateChangeCmdType cmdType)
{
    Trace trace("CapturerInClientInner::PauseAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("Pause State is not RUNNING. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Pause();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Pause call server failed:%{public}u", ret);
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

    // waiting for review: use send event to clent with cmdType | call OnStateChange | call HiSysEventWrite
    int64_t param = -1;
    StateCmdTypeToParams(param, state_, cmdType);
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, param);

    AUDIO_INFO_LOG("Pause SUCCESS, sessionId: %{public}d, uid: %{public}d, mode %{public}s", sessionId_, clientUid_,
        capturerMode_ == CAPTURE_MODE_NORMAL ? "CAPTURE_MODE_NORMAL" : "CAPTURE_MODE_CALLBACK");
    UpdateTracker("PAUSED");
    return true;
}

bool CapturerInClientInner::StopAudioStream()
{
    Trace trace("CapturerInClientInner::StopAudioStream " + std::to_string(sessionId_));
    AUDIO_INFO_LOG("Stop begin for sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ == STOPPED) {
        AUDIO_INFO_LOG("Capturer in client is already stopped");
        return true;
    }
    if ((state_ != RUNNING) && (state_ != PAUSED)) {
        AUDIO_ERR_LOG("Stop failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Stop();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Stop call server failed:%{public}u", ret);
        return false;
    }

    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
        state_ = STOPPING;
        AUDIO_INFO_LOG("Stop begin in callback mode sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
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

    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);

    AUDIO_INFO_LOG("Stop SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("STOPPED");
    return true;
}

bool CapturerInClientInner::ReleaseAudioStream(bool releaseRunner)
{
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Already release, do nothing");
        return true;
    }
    state_ = RELEASED;
    Trace trace("CapturerInClientInner::ReleaseAudioStream " + std::to_string(sessionId_));
    if (ipcStream_ != nullptr) {
        ipcStream_->Release();
        ipcStream_ = nullptr;
    } else {
        AUDIO_WARNING_LOG("Release while ipcStream is null");
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
    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
        cbThreadReleased_ = true; // stop loop
        if (cbBufferQueue_.IsEmpty()) {
            cbBufferQueue_.PushNoWait({nullptr, 0, 0});
        }
        cbThreadCv_.notify_all();
        readDataCV_.notify_all();
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
    return true;
}

bool CapturerInClientInner::FlushAudioStream()
{
    Trace trace("CapturerInClientInner::FlushAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
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

int32_t CapturerInClientInner::FlushRingCache()
{
    ringCache_->ResetBuffer();
    return SUCCESS;
}

bool CapturerInClientInner::DrainAudioStream()
{
    AUDIO_ERR_LOG("Drain is not supported");
    return false;
}

void CapturerInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    AUDIO_WARNING_LOG("Not Supported Yet");
}

int32_t CapturerInClientInner::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    AUDIO_ERR_LOG("SetRendererFirstFrameWritingCallback is not supported for capturer");
    return ERR_INVALID_OPERATION;
}

void CapturerInClientInner::OnFirstFrameWriting()
{
    AUDIO_ERR_LOG("OnFirstFrameWriting is not supported for capturer");
}

int32_t CapturerInClientInner::Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer,
    size_t metaBufferSize)
{
    AUDIO_ERR_LOG("Write is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t CapturerInClientInner::Write(uint8_t *buffer, size_t bufferSize)
{
    AUDIO_ERR_LOG("Write is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t CapturerInClientInner::HandleCapturerRead(size_t &readSize, size_t &userSize, uint8_t &buffer,
    bool isBlockingRead)
{
    while (readSize < userSize) {
        AUDIO_DEBUG_LOG("readSize %{public}zu < userSize %{public}zu", readSize, userSize);
        OptResult result = ringCache_->GetReadableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "GetReadableSize err %{public}d", result.ret);
        size_t readableSize = std::min(result.size, userSize - readSize);
        if (readSize + result.size >= userSize) { // If ringCache is sufficient
            result = ringCache_->Dequeue({&buffer + (readSize), readableSize});
            CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "DequeueCache err %{public}d", result.ret);
            readSize += readableSize;
            HandleCapturerPositionChanges(readSize);
            return readSize; // data size
        }
        if (result.size != 0) {
            result = ringCache_->Dequeue({&buffer + readSize, result.size});
            CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "Dequeue failed %{public}d", result.ret);
            readSize += result.size;
        }
        uint64_t availableSizeInFrame = clientBuffer_->GetCurWriteFrame() - clientBuffer_->GetCurReadFrame();
        AUDIO_DEBUG_LOG("availableSizeInFrame %{public}" PRId64 "", availableSizeInFrame);
        if (availableSizeInFrame > 0) { // If OHAudioBuffer has data
            BufferDesc currentOHBuffer_ = {};
            clientBuffer_->GetReadbuffer(clientBuffer_->GetCurReadFrame(), currentOHBuffer_);
            BufferWrap bufferWrap = {currentOHBuffer_.buffer, clientSpanSizeInByte_};
            ringCache_->Enqueue(bufferWrap);
            clientBuffer_->SetCurReadFrame(clientBuffer_->GetCurReadFrame() + spanSizeInFrame_);
        } else {
            if (!isBlockingRead) {
                HandleCapturerPositionChanges(readSize);
                return readSize; // Return buffer immediately
            }
            // wait for server read some data
            std::unique_lock<std::mutex> readLock(readDataMutex_);
            bool isTimeout = !readDataCV_.wait_for(readLock,
                std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
                    return clientBuffer_->GetCurWriteFrame() > clientBuffer_->GetCurReadFrame() || state_ != RUNNING;
            });
            CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "State is not running");
            CHECK_AND_RETURN_RET_LOG(isTimeout == false, ERROR, "Wait timeout");
        }
    }
    return readSize;
}

int32_t CapturerInClientInner::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    Trace trace("CapturerInClientInner::Read " + std::to_string(userSize));

    CHECK_AND_RETURN_RET_LOG(userSize < MAX_CLIENT_READ_SIZE && userSize > 0,
        ERR_INVALID_PARAM, "invalid size %{public}zu", userSize);

    std::lock_guard<std::mutex> lock(readMutex_);

    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_) {
        AudioSystemManager::GetInstance()->RequestThreadPriority(gettid());
        needSetThreadPriority_ = false;
    }

    std::unique_lock<std::mutex> statusLock(statusMutex_); // status check
    if (state_ != RUNNING) {
        if (readLogTimes_ < LOGLITMITTIMES) {
            readLogTimes_++;
            AUDIO_ERR_LOG("Illegal state:%{public}u", state_.load());
        } else {
            AUDIO_DEBUG_LOG("Illegal state:%{public}u", state_.load());
        }
        return ERR_ILLEGAL_STATE;
    } else {
        readLogTimes_ = 0;
    }

    statusLock.unlock();
    size_t readSize = 0;
    int32_t res = HandleCapturerRead(readSize, userSize, buffer, isBlockingRead);
    CHECK_AND_RETURN_RET_LOG(res >= 0, ERROR, "HandleCapturerRead err : %{public}d", res);
    HandleCapturerPositionChanges(readSize);
    return readSize;
}

void CapturerInClientInner::HandleCapturerPositionChanges(size_t bytesRead)
{
    totalBytesRead_ += bytesRead;
    if (sizePerFrameInByte_ == 0) {
        AUDIO_ERR_LOG("HandleCapturerPositionChanges: sizePerFrameInByte_ is 0");
        return;
    }
    int64_t readFrameNumber = totalBytesRead_ / sizePerFrameInByte_;
    AUDIO_DEBUG_LOG("totalBytesRead_ %{public}" PRId64 ", frame size: %{public}zu", totalBytesRead_,
        sizePerFrameInByte_);
    {
        std::lock_guard<std::mutex> lock(markReachMutex_);
        if (!capturerMarkReached_) {
            AUDIO_DEBUG_LOG("Frame mark position: %{public}" PRId64 ", Total frames read: %{public}" PRId64,
                static_cast<int64_t>(capturerMarkPosition_), static_cast<int64_t>(readFrameNumber));
            if (readFrameNumber >= capturerMarkPosition_) {
                AUDIO_DEBUG_LOG("capturerInClient OnMarkReached");
                SendCapturerMarkReachedEvent(capturerMarkPosition_);
                capturerMarkReached_ = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(periodReachMutex_);
        capturerPeriodRead_ += (totalBytesRead_ / sizePerFrameInByte_);
        AUDIO_DEBUG_LOG("Frame period number: %{public}" PRId64 ", Total frames written: %{public}" PRId64,
            static_cast<int64_t>(capturerPeriodRead_), static_cast<int64_t>(totalBytesRead_));
        if (capturerPeriodRead_ >= capturerPeriodSize_ && capturerPeriodSize_ > 0) {
            capturerPeriodRead_ %= capturerPeriodSize_;
            AUDIO_DEBUG_LOG("OnPeriodReached, remaining frames: %{public}" PRId64,
                static_cast<int64_t>(capturerPeriodRead_));
            SendCapturerPeriodReachedEvent(capturerPeriodSize_);
        }
    }
}

uint32_t CapturerInClientInner::GetUnderflowCount()
{
    // not supported for capturer
    AUDIO_WARNING_LOG("No Underflow in Capturer");
    return 0;
}

void CapturerInClientInner::SetCapturerPositionCallback(int64_t markPosition, const
    std::shared_ptr<CapturerPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(markReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "CapturerPositionCallback is nullptr");
    capturerPositionCallback_ = callback;
    capturerMarkPosition_ = markPosition;
    capturerMarkReached_ = false;
}

void CapturerInClientInner::UnsetCapturerPositionCallback()
{
    std::lock_guard<std::mutex> lock(markReachMutex_);
    capturerPositionCallback_ = nullptr;
    capturerMarkPosition_ = 0;
    capturerMarkReached_ = false;
}

void CapturerInClientInner::SetCapturerPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "CapturerPeriodPositionCallback is nullptr");
    capturerPeriodPositionCallback_ = callback;
    capturerPeriodSize_ = periodPosition;
    totalBytesRead_ = 0;
    capturerPeriodRead_ = 0;
}

void CapturerInClientInner::UnsetCapturerPeriodPositionCallback()
{
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    capturerPeriodPositionCallback_ = nullptr;
    capturerPeriodSize_ = 0;
    totalBytesRead_ = 0;
    capturerPeriodRead_ = 0;
}

void CapturerInClientInner::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetRendererPositionCallback is not supported");
    return;
}

void CapturerInClientInner::UnsetRendererPositionCallback()
{
    AUDIO_ERR_LOG("UnsetRendererPositionCallback is not supported");
    return;
}

void CapturerInClientInner::SetRendererPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetRendererPeriodPositionCallback is not supported");
    return;
}

void CapturerInClientInner::UnsetRendererPeriodPositionCallback()
{
    AUDIO_ERR_LOG("UnsetRendererPeriodPositionCallback is not supported");
    return;
}

int32_t CapturerInClientInner::SetRendererSamplingRate(uint32_t sampleRate)
{
    // in plan
    return ERROR;
}

uint32_t CapturerInClientInner::GetRendererSamplingRate()
{
    // in plan
    return 0; // not supported
}

int32_t CapturerInClientInner::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    // bufferSizeInMsec is checked between 5ms and 20ms.
    bufferSizeInMsec_ = bufferSizeInMsec;
    AUDIO_INFO_LOG("SetBufferSizeInMsec to %{public}d", bufferSizeInMsec_);
    if (capturerMode_ == CAPTURE_MODE_CALLBACK) {
        uint64_t bufferDurationInUs = bufferSizeInMsec_ * AUDIO_US_PER_MS;
        InitCallbackBuffer(bufferDurationInUs);
    }
    return SUCCESS;
}

void CapturerInClientInner::SetApplicationCachePath(const std::string cachePath)
{
    cachePath_ = cachePath;
    AUDIO_INFO_LOG("SetApplicationCachePath to %{publid}s", cachePath_.c_str());
}

int32_t CapturerInClientInner::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return ERROR;
}

int32_t CapturerInClientInner::SetVolumeWithRamp(float volume, int32_t duration)
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return ERROR;
}

void CapturerInClientInner::SetStreamTrackerState(bool trackerRegisteredState)
{
    streamTrackerRegistered_ = trackerRegisteredState;
}

void CapturerInClientInner::GetSwitchInfo(IAudioStream::SwitchInfo& info)
{
    // in plan
}

IAudioStream::StreamClass CapturerInClientInner::GetStreamClass()
{
    return PA_STREAM;
}
} // namespace AudioStandard
} // namespace OHOS
#endif // FAST_AUDIO_STREAM_H
