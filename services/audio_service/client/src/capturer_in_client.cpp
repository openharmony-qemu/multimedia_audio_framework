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
#ifndef FAST_AUDIO_STREAM_H
#define FAST_AUDIO_STREAM_H

#ifndef LOG_TAG
#define LOG_TAG "CapturerInClientInner"
#endif

#include "capturer_in_client.h"
#include "capturer_in_client_inner.h"

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

#include "iipc_stream.h"
#include "audio_capturer_log.h"
#include "audio_errors.h"
#include "volume_tools.h"
#include "audio_manager_base.h"
#include "audio_ring_cache.h"
#include "audio_utils.h"
#include "audio_policy_manager.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "audio_process_config.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "callback_handler.h"
#include "audio_safe_block_queue.h"
#include "istandard_audio_service.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const size_t MAX_CLIENT_READ_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t CREATE_TIMEOUT_IN_SECOND = 9; // 9S
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static const int32_t LOGLITMITTIMES = 20;
const uint64_t DEFAULT_BUF_DURATION_IN_USEC = 20000; // 20ms
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
const int64_t INVALID_FRAME_SIZE = -1;
static const int32_t SHORT_TIMEOUT_IN_MS = 20; // ms
static constexpr int CB_QUEUE_CAPACITY = 3;
constexpr int32_t RETRY_WAIT_TIME_MS = 500; // 500ms
constexpr int32_t MAX_RETRY_COUNT = 8;
const float ERROR_VOLUME = -0.1f;
}

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
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
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
        AUDIO_WARNING_LOG("recv overflow %{public}d", overflowCount_);
        // in plan next: do more to reduce overflow
        readDataCV_.notify_all();
        return SUCCESS;
    }

    if (operation == RESTORE_SESSION) {
        if (audioStreamTracker_ && audioStreamTracker_.get()) {
            audioStreamTracker_->FetchInputDeviceForTrack(sessionId_, state_, clientPid_, capturerInfo_);
        }
        return SUCCESS;
    }

    AUDIO_INFO_LOG("OnOperationHandled() recv operation:%{public}d result:%{public}" PRId64".", operation, result);
    std::unique_lock<std::mutex> lock(callServerMutex_);
    notifiedOperation_ = operation;
    notifiedResult_ = result;

    callServerCV_.notify_all();
    return SUCCESS;
}

void CapturerInClientInner::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId)
{
    AUDIO_INFO_LOG("PID:%{public}d UID:%{public}d.", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
    fullTokenId_ = fullTokenId;
    return;
}

int32_t CapturerInClientInner::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("client set %{public}s", ProcessConfig::DumpInnerCapConfig(config).c_str());
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "IpcStream is already nullptr");
    int32_t ret = ipcStream_->UpdatePlaybackCaptureConfig(config);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed: %{public}d", ret);

    filterConfig_ = config;
#endif
    return SUCCESS;
}

void CapturerInClientInner::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    AUDIO_WARNING_LOG("SetRendererInfo is not supported");
    return;
}

void CapturerInClientInner::GetRendererInfo(AudioRendererInfo &rendererInfo)
{
    AUDIO_WARNING_LOG("GetRendererInfo is not supported");
}

void CapturerInClientInner::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    capturerInfo_ = capturerInfo;
    capturerInfo_.samplingRate = static_cast<AudioSamplingRate>(streamParams_.samplingRate);
    capturerInfo_.encodingType = streamParams_.encoding;
    capturerInfo_.channelLayout = streamParams_.channelLayout;
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

        capturerInfo_.samplingRate = static_cast<AudioSamplingRate>(streamParams_.samplingRate);
        registerTrackerInfo.sessionId = sessionId_;
        registerTrackerInfo.clientPid = clientPid_;
        registerTrackerInfo.state = state_;
        registerTrackerInfo.rendererInfo = rendererInfo_;
        registerTrackerInfo.capturerInfo = capturerInfo_;
        registerTrackerInfo.appTokenId = appTokenId_;

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
    const std::shared_ptr<AudioClientTracker> &proxyObj,
    const AudioPlaybackCaptureConfig &config)
{
    AUDIO_INFO_LOG("AudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d, stream type:"
        " %{public}d, encoding type: %{public}d", info.samplingRate, info.channels, info.format, eStreamType_,
        info.encoding);
    AudioXCollie guard("CapturerInClientInner::SetAudioStreamInfo", CREATE_TIMEOUT_IN_SECOND,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    if (!IsFormatValid(info.format) || !IsEncodingTypeValid(info.encoding) || !IsSamplingRateValid(info.samplingRate)) {
        AUDIO_ERR_LOG("CapturerInClient: Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (!IsRecordChannelRelatedInfoValid(info.channels, info.channelLayout)) {
        AUDIO_ERR_LOG("Invalid sink channel %{public}d or channel layout %{public}" PRIu64, info.channels,
                info.channelLayout);
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
    int32_t initRet = InitIpcStream(config);
    CHECK_AND_RETURN_RET_LOG(initRet == SUCCESS, initRet, "Init stream failed: %{public}d", initRet);
    state_ = PREPARED;
    logUtilsTag_ = "[" + std::to_string(sessionId_) + "]NormalCapturer";

    proxyObj_ = proxyObj;
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
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) { AudioServerDied(pid, uid); });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("GetAudioServerProxy: failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_ServerProxy;
    return gasp;
}

void CapturerInClientInner::AudioServerDied(pid_t pid, pid_t uid)
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
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this(), "OS_AudioStateCB");
    }
}

// call this without lock, we should be able to call deinit in any case.
int32_t CapturerInClientInner::DeinitIpcStream()
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, SUCCESS, "IpcStream is already nullptr");
    ipcStream_->Release(false);
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
    config.appInfo.appFullTokenId = fullTokenId_;

    config.streamInfo.channels = static_cast<AudioChannel>(streamParams_.channels);
    config.streamInfo.encoding = static_cast<AudioEncodingType>(streamParams_.encoding);
    config.streamInfo.format = static_cast<AudioSampleFormat>(streamParams_.format);
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(streamParams_.samplingRate);
    config.streamInfo.channelLayout = static_cast<AudioChannelLayout>(streamParams_.channelLayout);
    config.originalSessionId = streamParams_.originalSessionId;

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
    config.innerCapId = innerCapId_;

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

int32_t CapturerInClientInner::InitIpcStream(const AudioPlaybackCaptureConfig &filterConfig)
{
    AUDIO_INFO_LOG("Init Ipc stream");
    AudioProcessConfig config = ConstructConfig();

    sptr<IStandardAudioService> gasp = CapturerInClientInner::GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_OPERATION_FAILED, "Create failed, can not get service.");
    int32_t errorCode = 0;
    sptr<IRemoteObject> ipcProxy = nullptr;
    gasp->CreateAudioProcess(config, errorCode, filterConfig, ipcProxy);
    for (int32_t retrycount = 0; (errorCode == ERR_RETRY_IN_CLIENT) && (retrycount < MAX_RETRY_COUNT); retrycount++) {
        AUDIO_WARNING_LOG("retry in client");
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_TIME_MS));
        AudioPlaybackCaptureConfig defalutConfig = {};
        gasp->CreateAudioProcess(config, errorCode, defalutConfig, ipcProxy);
    }
    CHECK_AND_RETURN_RET_LOG(errorCode == SUCCESS, errorCode, "failed with create audio stream fail.");
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, ERR_OPERATION_FAILED, "failed with null ipcProxy.");
    ipcStream_ = iface_cast<IIpcStream>(ipcProxy);
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

int32_t CapturerInClientInner::GetAudioSessionID(uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != NEW), ERR_ILLEGAL_STATE,
        "State error %{public}d", state_.load());
    sessionID = sessionId_;
    return SUCCESS;
}

void CapturerInClientInner::GetAudioPipeType(AudioPipeType &pipeType)
{
    pipeType = capturerInfo_.pipeType;
}

State CapturerInClientInner::GetState()
{
    return state_;
}

bool CapturerInClientInner::GetAudioTimeInner(
    Timestamp &timestamp, Timestamp::Timestampbase base, int64_t latency)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());
    uint64_t currentReadPos = totalBytesRead_ / sizePerFrameInByte_;
    timestamp.framePosition = currentReadPos;

    uint64_t writePos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");
    clientBuffer_->GetHandleInfo(writePos, handleTime);
    if (writePos == 0 || handleTime == 0) {
        AUDIO_WARNING_LOG("GetHandleInfo may failed");
    }

    int64_t deltaPos = writePos >= currentReadPos ? static_cast<int64_t>(writePos - currentReadPos) : 0;
    int64_t deltaTime = deltaPos * AUDIO_MS_PER_SECOND /
        static_cast<int64_t>(streamParams_.samplingRate) * AUDIO_US_PER_S;
    handleTime = handleTime + deltaTime + latency;

    timestamp.time.tv_sec = static_cast<time_t>(handleTime / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(handleTime % AUDIO_NS_PER_SECOND);

    return true;
}

bool CapturerInClientInner::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    int64_t tempLatency = 25000000; // 25000000 -> 25 ms
    return GetAudioTimeInner(timestamp, base, tempLatency);
}

bool CapturerInClientInner::GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");

    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        return GetAudioTimeInner(timestamp, base, 0);
    }

    uint64_t writePos = 0;
    uint64_t writeTimeStamp = 0;
    clientBuffer_->GetTimeStampInfo(writePos, writeTimeStamp);
    CHECK_AND_RETURN_RET_LOG(writeTimeStamp != 0, false, "writeTimeStamp is zero");
    AUDIO_DEBUG_LOG("pos:%{public}" PRIu64 " ts:%{public}" PRIu64, writePos, writeTimeStamp);

    timestamp.framePosition = writePos;
    timestamp.time.tv_sec = static_cast<time_t>(writeTimeStamp / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(writeTimeStamp % AUDIO_NS_PER_SECOND);

    return true;
}

bool CapturerInClientInner::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base);
}

void CapturerInClientInner::SetSwitchInfoTimestamp(
    std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePair,
    std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePairWithSpeed)
{
    (void)lastFramePosAndTimePair;
    (void)lastFramePosAndTimePairWithSpeed;
    AUDIO_INFO_LOG("capturer stream not support timestamp re-set when stream switching");
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

int32_t CapturerInClientInner::SetLoudnessGain(float loudnessGain)
{
    AUDIO_WARNING_LOG("SetLoudnessGain is only for renderer");
    return ERROR;
}

float CapturerInClientInner::GetLoudnessGain()
{
    AUDIO_WARNING_LOG("GetLoudnessGain is only for renderer");
    return 0.0;
}

int32_t CapturerInClientInner::SetMute(bool mute, StateChangeCmdType cmdType)
{
    AUDIO_WARNING_LOG("only for renderer");
    return ERROR;
}

bool CapturerInClientInner::GetMute()
{
    AUDIO_WARNING_LOG("only for renderer");
    return false;
}

int32_t CapturerInClientInner::SetDuckVolume(float volume)
{
    AUDIO_WARNING_LOG("only for renderer");
    return ERROR;
}

float CapturerInClientInner::GetDuckVolume()
{
    AUDIO_WARNING_LOG("only for renderer");
    return ERROR_VOLUME;
}

int32_t CapturerInClientInner::SetSpeed(float speed)
{
    AUDIO_ERR_LOG("SetSpeed is not supported");
    return ERROR;
}

int32_t CapturerInClientInner::SetPitch(float pitch)
{
    AUDIO_ERR_LOG("SetPitch is not supported");
    return ERROR;
}

float CapturerInClientInner::GetSpeed()
{
    AUDIO_ERR_LOG("GetSpeed is not supported");
    return 1.0;
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
    cbBufferQueue_.Clear();
    cbBufferQueue_.Push(temp);
}

void CapturerInClientInner::InitCallbackLoop()
{
    cbThreadReleased_ = false;
    auto weakRef = weak_from_this();

    // OS_AudioWriteCB
    ResetCallbackLoopTid();
    callbackLoop_ = std::thread([weakRef] {
        bool keepRunning = true;
        std::shared_ptr<CapturerInClientInner> strongRef = weakRef.lock();
        strongRef->SetCallbackLoopTid(gettid());
        if (strongRef != nullptr) {
            strongRef->cbThreadCv_.notify_one();
            AUDIO_INFO_LOG("Thread start, sessionID :%{public}d", strongRef->sessionId_);
        } else {
            AUDIO_WARNING_LOG("Strong ref is nullptr, could cause error");
        }
        strongRef = nullptr;
        // start loop
        while (keepRunning) {
            strongRef = weakRef.lock();
            if (strongRef == nullptr) {
                AUDIO_INFO_LOG("CapturerInClientInner destroyed");
                break;
            }
            keepRunning = strongRef->ReadCallbackFunc(); // Main operation in callback loop
        }
        if (strongRef != nullptr) {
            AUDIO_INFO_LOG("CBThread end sessionID :%{public}d", strongRef->sessionId_);
        }
    });
    pthread_setname_np(callbackLoop_.native_handle(), "OS_AudioReadCb");
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
    InitCallbackLoop();

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

bool CapturerInClientInner::ReadCallbackFunc()
{
    if (cbThreadReleased_) {
        return false;
    }
    Trace traceLoop("CapturerInClientInner::WriteCallbackFunc");
    if (!WaitForRunning()) {
        return true;
    }

    // If client didn't call GetBufferDesc/Enqueue in OnReadData, pop will block here.
    BufferDesc temp = cbBufferQueue_.Pop();
    if (temp.buffer == nullptr) {
        AUDIO_WARNING_LOG("Queue pop error: get nullptr.");
        return false;
    }

    std::unique_lock<std::mutex> lockBuffer(cbBufferMutex_);
    // call read here.
    int32_t result = Read(*temp.buffer, temp.bufLength, true); // blocking read
    if (result < 0 || result != static_cast<int32_t>(cbBufferSize_)) {
        AUDIO_WARNING_LOG("Call read error, ret:%{public}d, cbBufferSize_:%{public}zu", result, cbBufferSize_);
    }
    if (state_ != RUNNING) {
        return true;
    }
    lockBuffer.unlock();

    // call client read
    Trace traceCb("CapturerInClientInner::OnReadData");
    std::unique_lock<std::mutex> lockCb(readCbMutex_);
    if (readCb_ != nullptr) {
        readCb_->OnReadData(cbBufferSize_);
    }
    lockCb.unlock();
    traceCb.End();
    return true;
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
    uint64_t readFrameNumber = totalBytesRead_ / sizePerFrameInByte_;
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

bool CapturerInClientInner::StartAudioStream(StateChangeCmdType cmdType, AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("CapturerInClientInner::StartAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != PREPARED && state_ != STOPPED && state_ != PAUSED) {
        AUDIO_ERR_LOG("Start failed Illegal state: %{public}d", state_.load());
        return false;
    }

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchInputDeviceForTrack(sessionId_, RUNNING, clientPid_, capturerInfo_);
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
        if (cbBufferQueue_.IsEmpty()) {
            cbBufferQueue_.Push({cbBuffer_.get(), cbBufferSize_, cbBufferSize_});
        }
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
        readDataCV_.notify_all();
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

bool CapturerInClientInner::ReleaseAudioStream(bool releaseRunner, bool isSwitchStream)
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Already release, do nothing");
        return true;
    }
    state_ = RELEASED;
    statusLock.unlock();

    Trace trace("CapturerInClientInner::ReleaseAudioStream " + std::to_string(sessionId_));
    if (ipcStream_ != nullptr) {
        ipcStream_->Release(isSwitchStream);
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
    CHECK_AND_RETURN_RET_LOG(FlushRingCache() == SUCCESS, false, "Flush ringCache failed");
    CHECK_AND_RETURN_RET_LOG(FlushCbBuffer() == SUCCESS, false, "Flush cbBuffer failed");

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

int32_t CapturerInClientInner::FlushCbBuffer()
{
    Trace trace("CapturerInClientInner::FlushCbBuffer");
    if (cbBuffer_ != nullptr && capturerMode_ == CAPTURE_MODE_CALLBACK) {
        std::lock_guard<std::mutex> lock(cbBufferMutex_);
        int32_t ret = memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "Clear buffer fail, ret %{public}d.", ret);
        AUDIO_INFO_LOG("Flush cbBuffer_ for sessionId:%{public}d uid:%{public}d, ret:%{public}d",
            sessionId_, clientUid_, ret);
    }
    return SUCCESS;
}

bool CapturerInClientInner::DrainAudioStream(bool stopFlag)
{
    AUDIO_ERR_LOG("Drain is not supported");
    return false;
}

void CapturerInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    AUDIO_WARNING_LOG("Not Supported Yet");
}

void CapturerInClientInner::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    sptr<IStandardAudioService> gasp = CapturerInClientInner::GetAudioServerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("LatencyMeas failed to get AudioServerProxy");
        return;
    }
    gasp->UpdateLatencyTimestamp(timestamp, isRenderer);
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
    Trace trace("CapturerInClientInner::HandleCapturerRead " + std::to_string(userSize));
    while (readSize < userSize) {
        AUDIO_DEBUG_LOG("readSize %{public}zu < userSize %{public}zu", readSize, userSize);
        OptResult result = ringCache_->GetReadableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "GetReadableSize err %{public}d", result.ret);
        size_t readableSize = std::min(result.size, userSize - readSize);
        if (readSize + result.size >= userSize) { // If ringCache is sufficient
            result = ringCache_->Dequeue({&buffer + (readSize), readableSize});
            CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "DequeueCache err %{public}d", result.ret);
            readSize += readableSize;
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
            clientBuffer_->GetTimeStampInfo(currentOHBuffer_.position, currentOHBuffer_.timeStampInNs);
            clientBuffer_->GetReadbuffer(clientBuffer_->GetCurReadFrame(), currentOHBuffer_);
            BufferWrap bufferWrap = {currentOHBuffer_.buffer, clientSpanSizeInByte_};
            ringCache_->Enqueue(bufferWrap);
            memset_s(static_cast<void *>(bufferWrap.dataPtr), bufferWrap.dataSize, 0, bufferWrap.dataSize);
            clientBuffer_->SetCurReadFrame((clientBuffer_->GetCurReadFrame() + spanSizeInFrame_), false);
        } else {
            if (!isBlockingRead) {
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

    std::unique_lock<std::mutex> statusLock(statusMutex_); // status check
    if (state_ != RUNNING) {
        if (readLogTimes_ < LOGLITMITTIMES) {
            readLogTimes_.fetch_add(1);
            AUDIO_ERR_LOG("Illegal state:%{public}u", state_.load());
        } else {
            AUDIO_DEBUG_LOG("Illegal state:%{public}u", state_.load());
        }
        return ERR_ILLEGAL_STATE;
    } else {
        readLogTimes_ = 0;
    }

    statusLock.unlock();

    std::lock_guard<std::mutex> lock(readMutex_);
    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_) {
        CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERROR, "ipcStream_ is null");
        ipcStream_->RegisterThreadPriority(gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(clientConfig_.appInfo.appUid), METHOD_WRITE_OR_READ);
        needSetThreadPriority_ = false;
    }

    size_t readSize = 0;
    int32_t res = HandleCapturerRead(readSize, userSize, buffer, isBlockingRead);
    CHECK_AND_RETURN_RET_LOG(res >= 0, ERROR, "HandleCapturerRead err : %{public}d", res);
    BufferDesc tmpBuffer = {reinterpret_cast<uint8_t *>(&buffer), userSize, userSize};
    VolumeTools::DfxOperation(tmpBuffer, clientConfig_.streamInfo, logUtilsTag_, volumeDataCount_);
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
    uint64_t readFrameNumber = totalBytesRead_ / sizePerFrameInByte_;
    AUDIO_DEBUG_LOG("totalBytesRead_ %{public}" PRId64 ", frame size: %{public}zu", totalBytesRead_,
        sizePerFrameInByte_);
    {
        std::lock_guard<std::mutex> lock(markReachMutex_);
        if (!capturerMarkReached_) {
            AUDIO_DEBUG_LOG("Frame mark position: %{public}" PRId64 ", Total frames read: %{public}" PRId64,
                capturerMarkPosition_, static_cast<int64_t>(readFrameNumber));
            if (readFrameNumber >= static_cast<uint64_t>(capturerMarkPosition_)) {
                AUDIO_DEBUG_LOG("capturerInClient OnMarkReached");
                SendCapturerMarkReachedEvent(capturerMarkPosition_);
                capturerMarkReached_ = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(periodReachMutex_);
        capturerPeriodRead_ += static_cast<int64_t>(bytesRead / sizePerFrameInByte_);
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

uint32_t CapturerInClientInner::GetOverflowCount()
{
    return overflowCount_;
}

void CapturerInClientInner::SetUnderflowCount(uint32_t underflowCount)
{
    // not supported for capturer
    AUDIO_WARNING_LOG("No Underflow in Capturer");
    return;
}

void CapturerInClientInner::SetOverflowCount(uint32_t overflowCount)
{
    overflowCount_ = overflowCount;
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
        uint64_t bufferDurationInUs = static_cast<uint64_t>(bufferSizeInMsec_ * AUDIO_US_PER_MS);
        InitCallbackBuffer(bufferDurationInUs);
    }
    return SUCCESS;
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
    info.params = streamParams_;

    info.rendererInfo = rendererInfo_;
    info.capturerInfo = capturerInfo_;
    info.eStreamType = eStreamType_;
    info.state = state_;
    info.sessionId = sessionId_;
    info.streamTrackerRegistered = streamTrackerRegistered_;
    GetStreamSwitchInfo(info);
}

void CapturerInClientInner::GetStreamSwitchInfo(IAudioStream::SwitchInfo& info)
{
    info.overFlowCount = overflowCount_;
    info.clientPid = clientPid_;
    info.clientUid = clientUid_;

    info.frameMarkPosition = static_cast<uint64_t>(capturerMarkPosition_);
    info.capturePositionCb = capturerPositionCallback_;

    info.framePeriodNumber = static_cast<uint64_t>(capturerPeriodSize_);
    info.capturePeriodPositionCb = capturerPeriodPositionCallback_;

    info.capturerReadCallback = readCb_;
}

bool CapturerInClientInner::GetOffloadEnable()
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return false;
}

bool CapturerInClientInner::GetSpatializationEnabled()
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return false;
}

bool CapturerInClientInner::GetHighResolutionEnabled()
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return false;
}

IAudioStream::StreamClass CapturerInClientInner::GetStreamClass()
{
    return PA_STREAM;
}

void CapturerInClientInner::SetSilentModeAndMixWithOthers(bool on)
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return;
}

bool CapturerInClientInner::GetSilentModeAndMixWithOthers()
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return false;
}

bool CapturerInClientInner::RestoreAudioStream(bool needStoreState)
{
    CHECK_AND_RETURN_RET_LOG(proxyObj_ != nullptr, false, "proxyObj_ is null");
    CHECK_AND_RETURN_RET_LOG(state_ != NEW && state_ != INVALID && state_ != RELEASED, true,
        "state_ is %{public}d, no need for restore", state_.load());
    bool result = false;
    State oldState = state_;
    state_ = NEW;
    SetStreamTrackerState(false);

    int32_t ret = SetAudioStreamInfo(streamParams_, proxyObj_);
    if (ret != SUCCESS) {
        goto error;
    }
#ifdef HAS_FEATURE_INNERCAPTURER
    // for inner-capturer
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        ret = UpdatePlaybackCaptureConfig(filterConfig_);
        if (ret != SUCCESS) {
            goto error;
        }
    }
#endif
    switch (oldState) {
        case RUNNING:
            result = StartAudioStream();
            break;
        case PAUSED:
            result = StartAudioStream() && PauseAudioStream();
            break;
        case STOPPED:
        case STOPPING:
            result = StartAudioStream() && StopAudioStream();
            break;
        default:
            break;
    }
    if (!result) {
        goto error;
    }
    return result;

error:
    AUDIO_ERR_LOG("RestoreAudioStream failed");
    state_ = oldState;
    return false;
}

void CapturerInClientInner::JoinCallbackLoop()
{
    AUDIO_INFO_LOG("Not Support");
}

int32_t CapturerInClientInner::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce)
{
    (void)defaultOutputDevice;
    (void)skipForce;
    AUDIO_WARNING_LOG("not supported in capturer");
    return ERROR;
}

FastStatus CapturerInClientInner::GetFastStatus()
{
    return FASTSTATUS_NORMAL;
}

DeviceType CapturerInClientInner::GetDefaultOutputDevice()
{
    AUDIO_WARNING_LOG("not supported in capturer");
    return DEVICE_TYPE_NONE;
}

// diffrence from GetAudioPosition only when set speed
int32_t CapturerInClientInner::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base) ? SUCCESS : ERROR;
}

void CapturerInClientInner::SetSwitchingStatus(bool isSwitching)
{
    AUDIO_WARNING_LOG("not supported in capturer");
}

void CapturerInClientInner::GetRestoreInfo(RestoreInfo &restoreInfo)
{
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    clientBuffer_->GetRestoreInfo(restoreInfo);
    return;
}

void CapturerInClientInner::SetRestoreInfo(RestoreInfo &restoreInfo)
{
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    clientBuffer_->SetRestoreInfo(restoreInfo);
    return;
}

RestoreStatus CapturerInClientInner::CheckRestoreStatus()
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return clientBuffer_->CheckRestoreStatus();
}

RestoreStatus CapturerInClientInner::SetRestoreStatus(RestoreStatus restoreStatus)
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return clientBuffer_->SetRestoreStatus(restoreStatus);
}

void CapturerInClientInner::FetchDeviceForSplitStream()
{
    AUDIO_INFO_LOG("Fetch input device for split stream %{public}u", sessionId_);
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchInputDeviceForTrack(sessionId_, state_, clientPid_, capturerInfo_);
    } else {
        AUDIO_WARNING_LOG("Tracker is nullptr, fail to split stream %{public}u", sessionId_);
    }
    SetRestoreStatus(NO_NEED_FOR_RESTORE);
}

void CapturerInClientInner::SetCallStartByUserTid(pid_t tid)
{
    AUDIO_WARNING_LOG("not supported in capturer");
}

void CapturerInClientInner::SetCallbackLoopTid(int32_t tid)
{
    AUDIO_INFO_LOG("Callback loop tid: %{public}d", tid);
    callbackLoopTid_ = tid;
    callbackLoopTidCv_.notify_all();
}

int32_t CapturerInClientInner::GetCallbackLoopTid()
{
    std::unique_lock<std::mutex> waitLock(callbackLoopTidMutex_);
    bool stopWaiting = callbackLoopTidCv_.wait_for(waitLock, std::chrono::seconds(1), [this] {
        return callbackLoopTid_ != -1; // callbackLoopTid_ will change when got notified.
    });

    if (!stopWaiting) {
        AUDIO_WARNING_LOG("Wait timeout");
        callbackLoopTid_ = 0; // set tid to prevent get operation from getting stuck
    }
    return callbackLoopTid_;
}

void CapturerInClientInner::ResetCallbackLoopTid()
{
    AUDIO_INFO_LOG("to -1");
    callbackLoopTid_ = -1;
}

bool CapturerInClientInner::GetStopFlag() const
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "Client OHAudioBuffer is nullptr");
    return clientBuffer_->GetStopFlag();
}
} // namespace AudioStandard
} // namespace OHOS
#endif // FAST_AUDIO_STREAM_H
