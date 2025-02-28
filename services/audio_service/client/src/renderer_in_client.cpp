/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "RendererInClientInner"
#endif

#include "futex_tool.h"
#include "renderer_in_client.h"
#include "renderer_in_client_private.h"

#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_manager_base.h"
#include "audio_renderer_log.h"
#include "audio_ring_cache.h"
#include "audio_channel_blend.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "futex_tool.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "volume_ramp.h"
#include "callback_handler.h"
#include "audio_speed.h"
#include "audio_spatial_channel_converter.h"
#include "audio_policy_manager.h"
#include "audio_spatialization_manager.h"
#include "policy_handler.h"
#include "audio_log_utils.h"

#include "media_monitor_manager.h"
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
namespace {
const uint64_t OLD_BUF_DURATION_IN_USEC = 92880; // This value is used for compatibility purposes.
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
const uint64_t AUDIO_FIRST_FRAME_LATENCY = 120; //ms
static const size_t MAX_WRITE_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t CREATE_TIMEOUT_IN_SECOND = 9; // 9S
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static const int32_t OFFLOAD_OPERATION_TIMEOUT_IN_MS = 8000; // 8000ms for offload
static const int32_t WRITE_CACHE_TIMEOUT_IN_MS = 1500; // 1500ms
static const int32_t WRITE_BUFFER_TIMEOUT_IN_MS = 20; // ms
static const int32_t SHORT_TIMEOUT_IN_MS = 20; // ms
static const uint32_t WAIT_FOR_NEXT_CB = 5000; // 5ms
static const int32_t HALF_FACTOR = 2;
static const int32_t DATA_CONNECTION_TIMEOUT_IN_MS = 1000; // ms
static constexpr int CB_QUEUE_CAPACITY = 3;
constexpr int32_t MAX_BUFFER_SIZE = 100000;
static constexpr int32_t ONE_MINUTE = 60;
static constexpr int32_t MEDIA_SERVICE_UID = 1013;
static const int32_t MAX_WRITE_INTERVAL_MS = 40;
} // namespace

static AppExecFwk::BundleInfo gBundleInfo_;

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
    std::lock_guard<std::mutex> runnerlock(runnerMutex_);
    if (!runnerReleased_ && callbackHandler_ != nullptr) {
        AUDIO_INFO_LOG("runner remove");
        callbackHandler_->ReleaseEventRunner();
        runnerReleased_ = true;
        callbackHandler_ = nullptr;
    }
    UnregisterSpatializationStateEventListener(spatializationRegisteredSessionID_);
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
}

int32_t RendererInClientInner::OnOperationHandled(Operation operation, int64_t result)
{
    Trace trace(traceTag_ + " OnOperationHandled:" + std::to_string(operation));
    AUDIO_INFO_LOG("sessionId %{public}d recv operation:%{public}d result:%{public}" PRId64".", sessionId_, operation,
        result);
    if (operation == SET_OFFLOAD_ENABLE) {
        AUDIO_INFO_LOG("SET_OFFLOAD_ENABLE result:%{public}" PRId64".", result);
        if (!offloadEnable_ && static_cast<bool>(result)) {
            offloadStartReadPos_ = 0;
        }
        offloadEnable_ = static_cast<bool>(result);
        rendererInfo_.pipeType = offloadEnable_ ? PIPE_TYPE_OFFLOAD : PIPE_TYPE_NORMAL_OUT;
        return SUCCESS;
    }

    if (operation == DATA_LINK_CONNECTING || operation == DATA_LINK_CONNECTED) {
        if (operation == DATA_LINK_CONNECTING) {
            isDataLinkConnected_ = false;
        } else {
            isDataLinkConnected_ = true;
            dataConnectionCV_.notify_all();
        }
        return SUCCESS;
    }

    std::unique_lock<std::mutex> lock(callServerMutex_);
    notifiedOperation_ = operation;
    notifiedResult_ = result;

    if (notifiedResult_ == SUCCESS) {
        switch (operation) {
            case START_STREAM :
                state_ = RUNNING;
                break;
            case PAUSE_STREAM :
                state_ = PAUSED;
                break;
            case STOP_STREAM :
                state_ = STOPPED;
                break;
            default :
                break;
        }
    } else {
        AUDIO_ERR_LOG("operation %{public}d failed, result: %{public}" PRId64 "", operation, result);
    }

    callServerCV_.notify_all();
    return SUCCESS;
}

void RendererInClientInner::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId)
{
    AUDIO_INFO_LOG("PID:%{public}d UID:%{public}d.", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
    fullTokenId_ = fullTokenId;
}

int32_t RendererInClientInner::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    AUDIO_ERR_LOG("Unsupported operation!");
    return ERR_NOT_SUPPORTED;
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
    rendererInfo_.sceneType = GetEffectSceneName(rendererInfo_.streamUsage);
    AUDIO_PRERELEASE_LOGI("SetRendererInfo with flag %{public}d, sceneType %{public}s", rendererInfo_.rendererFlags,
        rendererInfo_.sceneType.c_str());
    AudioSpatializationState spatializationState =
        AudioPolicyManager::GetInstance().GetSpatializationState(rendererInfo_.streamUsage);
    rendererInfo_.spatializationEnabled = spatializationState.spatializationEnabled;
    rendererInfo_.headTrackingEnabled = spatializationState.headTrackingEnabled;
    rendererInfo_.encodingType = curStreamParams_.encoding;
    rendererInfo_.channelLayout = curStreamParams_.channelLayout;
    UpdateTracker("UPDATE");
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

        rendererInfo_.samplingRate = static_cast<AudioSamplingRate>(curStreamParams_.samplingRate);
        rendererInfo_.format = static_cast<AudioSampleFormat>(curStreamParams_.format);
        registerTrackerInfo.sessionId = sessionId_;
        registerTrackerInfo.clientPid = clientPid_;
        registerTrackerInfo.state = state_;
        registerTrackerInfo.rendererInfo = rendererInfo_;
        registerTrackerInfo.capturerInfo = capturerInfo_;
        registerTrackerInfo.channelCount = curStreamParams_.channels;

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

bool RendererInClientInner::IsHightResolution() const noexcept
{
    return eStreamType_ == STREAM_MUSIC && curStreamParams_.samplingRate >= SAMPLE_RATE_48000 &&
           curStreamParams_.format >= SAMPLE_S24LE;
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

    streamParams_ = curStreamParams_ = info; // keep it for later use
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        ConverterConfig cfg = AudioPolicyManager::GetInstance().GetConverterConfig();
        converter_ = std::make_unique<AudioSpatialChannelConverter>();
        if (converter_ == nullptr || !converter_->Init(curStreamParams_, cfg) || !converter_->AllocateMem()) {
            AUDIO_ERR_LOG("AudioStream: converter construct error");
            return ERR_NOT_SUPPORTED;
        }
        converter_->ConverterChannels(curStreamParams_.channels, curStreamParams_.channelLayout);
    }

    if (!IsPlaybackChannelRelatedInfoValid(curStreamParams_.channels, curStreamParams_.channelLayout)) {
        return ERR_NOT_SUPPORTED;
    }

    CHECK_AND_RETURN_RET_LOG(IAudioStream::GetByteSizePerFrame(curStreamParams_, sizePerFrameInByte_) == SUCCESS,
        ERROR_INVALID_PARAM, "GetByteSizePerFrame failed with invalid params");

    if (state_ != NEW) {
        AUDIO_ERR_LOG("State is not new, release existing stream and recreate, state %{public}d", state_.load());
        int32_t ret = DeinitIpcStream();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "release existing stream failed.");
    }
    paramsIsSet_ = true;
    int32_t initRet = InitIpcStream();
    CHECK_AND_RETURN_RET_LOG(initRet == SUCCESS, initRet, "Init stream failed: %{public}d", initRet);
    state_ = PREPARED;

    // eg: 100005_44100_2_1_client_out.pcm
    dumpOutFile_ = std::to_string(sessionId_) + "_" + std::to_string(curStreamParams_.samplingRate) + "_" +
        std::to_string(curStreamParams_.channels) + "_" + std::to_string(curStreamParams_.format) + "_client_out.pcm";

    DumpFileUtil::OpenDumpFile(DUMP_CLIENT_PARA, dumpOutFile_, &dumpOutFd_);
    logUtilsTag_ = "[" + std::to_string(sessionId_) + "]NormalRenderer";
    InitDirectPipeType();

    proxyObj_ = proxyObj;
    RegisterTracker(proxyObj);
    RegisterSpatializationStateEventListener();
    return SUCCESS;
}

void RendererInClientInner::InitDirectPipeType()
{
    if (rendererInfo_.rendererFlags == AUDIO_FLAG_VOIP_DIRECT || IsHightResolution()) {
        AudioPipeType originType = rendererInfo_.pipeType;
        int32_t type = ipcStream_->GetStreamManagerType();
        if (type == AUDIO_DIRECT_MANAGER_TYPE) {
            rendererInfo_.pipeType = (rendererInfo_.rendererFlags == AUDIO_FLAG_VOIP_DIRECT) ?
                PIPE_TYPE_CALL_OUT : PIPE_TYPE_DIRECT_MUSIC;
        } else if (originType == PIPE_TYPE_DIRECT_MUSIC) {
            rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
        }
    }
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
            asDeathRecipient->SetNotifyCb([] (pid_t pid) { AudioServerDied(pid); });
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
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ == nullptr) {
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this(), "OS_AudioStateCB");
    }
}

// call this without lock, we should be able to call deinit in any case.
int32_t RendererInClientInner::DeinitIpcStream()
{
    Trace trace("RendererInClientInner::DeinitIpcStream");
    ipcStream_->Release();
    ringCache_->ResetBuffer();
    return SUCCESS;
}

const AudioProcessConfig RendererInClientInner::ConstructConfig()
{
    AudioProcessConfig config = {};

    config.appInfo.appPid = clientPid_;
    config.appInfo.appUid = clientUid_;
    config.appInfo.appTokenId = appTokenId_;
    config.appInfo.appFullTokenId = fullTokenId_;

    config.streamInfo.channels = static_cast<AudioChannel>(curStreamParams_.channels);
    config.streamInfo.encoding = static_cast<AudioEncodingType>(curStreamParams_.encoding);
    config.streamInfo.format = static_cast<AudioSampleFormat>(curStreamParams_.format);
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(curStreamParams_.samplingRate);
    config.streamInfo.channelLayout = static_cast<AudioChannelLayout>(curStreamParams_.channelLayout);
    config.originalSessionId = curStreamParams_.originalSessionId;

    config.audioMode = AUDIO_MODE_PLAYBACK;

    if (rendererInfo_.rendererFlags != AUDIO_FLAG_NORMAL && rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_DIRECT) {
        AUDIO_WARNING_LOG("ConstructConfig find renderer flag invalid:%{public}d", rendererInfo_.rendererFlags);
        rendererInfo_.rendererFlags = 0;
    }
    config.rendererInfo = rendererInfo_;

    config.capturerInfo = {};

    config.streamType = eStreamType_;

    config.deviceType = AudioPolicyManager::GetInstance().GetActiveOutputDevice();

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
    Trace trace("RendererInClientInner::InitIpcStream");
    AudioProcessConfig config = ConstructConfig();
    bool resetSilentMode = (gServerProxy_ == nullptr) ? true : false;
    sptr<IStandardAudioService> gasp = RendererInClientInner::GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_OPERATION_FAILED, "Create failed, can not get service.");
    int32_t errorCode = 0;
    sptr<IRemoteObject> ipcProxy = gasp->CreateAudioProcess(config, errorCode);
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, ERR_OPERATION_FAILED, "failed with null ipcProxy.");
    ipcStream_ = iface_cast<IpcStream>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "failed when iface_cast.");

    // in plan next: old listener_ is destoried here, will server receive dieth notify?
    listener_ = sptr<IpcStreamListenerImpl>::MakeSptr(shared_from_this());
    int32_t ret = ipcStream_->RegisterStreamListener(listener_->AsObject());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RegisterStreamListener failed:%{public}d", ret);

    if (resetSilentMode && gServerProxy_ != nullptr && silentModeAndMixWithOthers_) {
        ipcStream_->SetSilentModeAndMixWithOthers(silentModeAndMixWithOthers_);
    }

    ret = InitSharedBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitSharedBuffer failed:%{public}d", ret);

    ret = InitCacheBuffer(clientSpanSizeInByte_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitCacheBuffer failed:%{public}d", ret);

    ret = ipcStream_->GetAudioSessionID(sessionId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioSessionID failed:%{public}d", ret);
    traceTag_ = "[" + std::to_string(sessionId_) + "]RendererInClient"; // [100001]RendererInClient
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

void RendererInClientInner::GetAudioPipeType(AudioPipeType &pipeType)
{
    pipeType = rendererInfo_.pipeType;
}

State RendererInClientInner::GetState()
{
    std::lock_guard lock(switchingMutex_);
    if (switchingInfo_.isSwitching_) {
        AUDIO_INFO_LOG("switching, return state in switchingInfo");
        return switchingInfo_.state_;
    }
    return state_;
}

bool RendererInClientInner::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());

    uint64_t readPos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");
    clientBuffer_->GetHandleInfo(readPos, handleTime);
    if (readPos == 0 || handleTime == 0) {
        AUDIO_WARNING_LOG("GetHandleInfo may failed");
    }

    timestamp.framePosition = readPos;
    int64_t audioTimeResult = handleTime;

    if (offloadEnable_) {
        uint64_t timestampHdi = 0;
        uint64_t paWriteIndex = 0;
        uint64_t cacheTimeDsp = 0;
        uint64_t cacheTimePa = 0;
        ipcStream_->GetOffloadApproximatelyCacheTime(timestampHdi, paWriteIndex, cacheTimeDsp, cacheTimePa);
        int64_t cacheTime = static_cast<int64_t>(cacheTimeDsp + cacheTimePa) * AUDIO_NS_PER_US;
        int64_t timeNow = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        int64_t deltaTimeStamp = (static_cast<int64_t>(timeNow) - static_cast<int64_t>(timestampHdi)) * AUDIO_NS_PER_US;
        uint64_t paWriteIndexNs = paWriteIndex * AUDIO_NS_PER_US;
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
    AUDIO_DEBUG_LOG("audioTimeResult: %{public}" PRIi64, audioTimeResult);
    return true;
}

bool RendererInClientInner::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false, "Renderer stream state is not RUNNING");
    uint64_t readIndex = 0;
    uint64_t timestampVal = 0;
    uint64_t latency = 0;
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->GetAudioPosition(readIndex, timestampVal, latency);

    uint64_t framePosition = readIndex > lastFlushReadIndex_ ? readIndex - lastFlushReadIndex_ : 0;
    framePosition = framePosition > latency ? framePosition - latency : 0;

    // add MCR latency
    uint32_t mcrLatency = 0;
    if (converter_ != nullptr) {
        mcrLatency = converter_->GetLatency();
        framePosition = framePosition - (mcrLatency * curStreamParams_.samplingRate / AUDIO_MS_PER_S);
    }

    if (lastFramePosition_ < framePosition) {
        lastFramePosition_ = framePosition;
        lastFrameTimestamp_ = timestampVal;
    } else {
        AUDIO_DEBUG_LOG("The frame position should be continuously increasing");
        framePosition = lastFramePosition_;
        timestampVal = lastFrameTimestamp_;
    }
    AUDIO_DEBUG_LOG("[CLIENT]Latency info: framePosition: %{public}" PRIu64 ", lastFlushReadIndex_ %{public}" PRIu64
        ", timestamp %{public}" PRIu64 ", mcrLatency %{public}u, Sinklatency %{public}" PRIu64, framePosition,
        lastFlushReadIndex_, timestampVal, mcrLatency, latency);

    timestamp.framePosition = framePosition;
    timestamp.time.tv_sec = static_cast<time_t>(timestampVal / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(timestampVal % AUDIO_NS_PER_SECOND);
    return ret == SUCCESS;
}

int32_t RendererInClientInner::GetBufferSize(size_t &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Renderer stream is released");
    bufferSize = clientSpanSizeInByte_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        bufferSize = cbBufferSize_;
    }

    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET(converter_ != nullptr && converter_->GetInputBufferSize(bufferSize), ERR_OPERATION_FAILED);
    }

    AUDIO_INFO_LOG("Buffer size is %{public}zu, mode is %{public}s", bufferSize, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetFrameCount(uint32_t &frameCount)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Renderer stream is released");
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ != 0, ERR_ILLEGAL_STATE, "sizePerFrameInByte_ is 0!");
    frameCount = spanSizeInFrame_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        frameCount = cbBufferSize_ / sizePerFrameInByte_;
        if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
            frameCount = frameCount * curStreamParams_.channels / streamParams_.channels;
        }
    }
    AUDIO_INFO_LOG("Frame count is %{public}u, mode is %{public}s", frameCount, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetLatency(uint64_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    return ipcStream_->GetLatency(latency);
}

int32_t RendererInClientInner::SetAudioStreamType(AudioStreamType audioStreamType)
{
    AUDIO_ERR_LOG("Change stream type %{public}d to %{public}d is not supported", eStreamType_, audioStreamType);
    return SUCCESS;
}

int32_t RendererInClientInner::SetInnerVolume(float volume)
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, ERR_OPERATION_FAILED, "buffer is not inited");
    clientBuffer_->SetStreamVolume(volume);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetClientVolume();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set Client Volume failed:%{public}u", ret);
        return -1;
    }
    AUDIO_PRERELEASE_LOGI("SetClientVolume success, volume: %{public}f", volume);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolume(float volume)
{
    Trace trace("RendererInClientInner::SetVolume:" + std::to_string(volume));
    AUDIO_INFO_LOG("[%{public}s]sessionId:%{public}d volume:%{public}f", (offloadEnable_ ? "offload" : "normal"),
        sessionId_, volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    if (volumeRamp_.IsActive()) {
        volumeRamp_.Terminate();
    }
    clientVolume_ = volume;

    return SetInnerVolume(volume);
}

float RendererInClientInner::GetVolume()
{
    Trace trace("RendererInClientInner::GetVolume:" + std::to_string(clientVolume_));
    return clientVolume_;
}

int32_t RendererInClientInner::SetDuckVolume(float volume)
{
    Trace trace("RendererInClientInner::SetDuckVolume:" + std::to_string(volume));
    AUDIO_INFO_LOG("sessionId:%{public}d SetDuck:%{public}f", sessionId_, volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetDuckVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    duckVolume_ = volume;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, ERR_OPERATION_FAILED, "buffer is not inited");
    clientBuffer_->SetDuckFactor(volume);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetDuckFactor(volume);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set Duck failed:%{public}u", ret);
        return ERROR;
    }
    return SUCCESS;
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
        audioSpeed_ = std::make_unique<AudioSpeed>(curStreamParams_.samplingRate, curStreamParams_.format,
            curStreamParams_.channels);
        GetBufferSize(bufferSize_);
        speedBuffer_ = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    }
    audioSpeed_->SetSpeed(speed);
    speed_ = speed;
    AUDIO_DEBUG_LOG("SetSpeed %{public}f, OffloadEnable %{public}d", speed_, offloadEnable_);
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

    size_t metaSize = 0;
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_LOG(converter_ != nullptr, "converter is not inited");
        metaSize = converter_->GetMetaSize();
        converter_->GetInputBufferSize(cbBufferSize_);
    } else {
        cbBufferSize_ = static_cast<size_t>(bufferDurationInUs * curStreamParams_.samplingRate / AUDIO_US_PER_S) *
            sizePerFrameInByte_;
    }
    AUDIO_INFO_LOG("duration %{public}" PRIu64 ", ecodingType: %{public}d, size: %{public}zu, metaSize: %{public}zu",
        bufferDurationInUs, curStreamParams_.encoding, cbBufferSize_, metaSize);
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    cbBuffer_ = std::make_unique<uint8_t[]>(cbBufferSize_ + metaSize);
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
    callbackLoop_ = std::thread([this] { this->WriteCallbackFunc(); });
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

int32_t RendererInClientInner::ProcessWriteInner(BufferDesc &bufferDesc)
{
    int32_t result = 0; // Ensure result with default value.
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        result = WriteInner(bufferDesc.buffer, bufferDesc.bufLength, bufferDesc.metaBuffer, bufferDesc.metaLength);
    }
    if (curStreamParams_.encoding == ENCODING_PCM) {
        if (bufferDesc.dataLength != 0) {
            result = WriteInner(bufferDesc.buffer, bufferDesc.bufLength);
            sleepCount_ = LOG_COUNT_LIMIT;
        } else {
            if (sleepCount_++ == LOG_COUNT_LIMIT) {
                sleepCount_ = 0;
                AUDIO_WARNING_LOG("OnWriteData Process 1st or 500 times INVALID buffer");
            }
            usleep(WAIT_FOR_NEXT_CB);
        }
    }
    if (result < 0) {
        AUDIO_WARNING_LOG("Call write fail, result:%{public}d, bufLength:%{public}zu", result, bufferDesc.bufLength);
    }
    return result;
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
        if (cbBufferQueue_.Size() > 1) { // One callback, one enqueue, queue size should always be 1.
            AUDIO_WARNING_LOG("The queue is too long, reducing data through loops");
        }
        BufferDesc temp;
        while (cbBufferQueue_.PopNotWait(temp)) {
            Trace traceQueuePop("RendererInClientInner::QueueWaitPop");
            if (state_ != RUNNING) {
                cbBufferQueue_.Push(temp);
                AUDIO_INFO_LOG("Repush left buffer in queue");
                break;
            }
            traceQueuePop.End();
            // call write here.
            int32_t result = ProcessWriteInner(temp);
            // only run in pause scene
            if (result > 0 && static_cast<size_t>(result) < temp.dataLength) {
                BufferDesc tmp = {temp.buffer + static_cast<size_t>(result),
                    temp.bufLength - static_cast<size_t>(result), temp.dataLength - static_cast<size_t>(result)};
                cbBufferQueue_.Push(tmp);
                AUDIO_INFO_LOG("Repush %{public}zu bytes in queue", temp.dataLength - static_cast<size_t>(result));
                break;
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
        cbBufferQueue_.WaitNotEmptyFor(std::chrono::milliseconds(WRITE_BUFFER_TIMEOUT_IN_MS));
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
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET_LOG(converter_ != nullptr, ERR_INVALID_OPERATION, "converter is not inited");
        bufDesc.metaBuffer = bufDesc.buffer + cbBufferSize_;
        bufDesc.metaLength = converter_->GetMetaSize();
    }
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
    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr && bufDesc.bufLength != 0, ERR_INVALID_PARAM, "Invalid buffer");
    CHECK_AND_RETURN_RET_LOG(curStreamParams_.encoding != ENCODING_AUDIOVIVID ||
            converter_ != nullptr && converter_->CheckInputValid(bufDesc),
        ERR_INVALID_PARAM, "Invalid buffer desc");
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
    AUDIO_INFO_LOG("Volume number: %{public}f", volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("Invalid param: %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    lowPowerVolume_ = volume;

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->SetLowPowerVolume(lowPowerVolume_);
}

float RendererInClientInner::GetLowPowerVolume()
{
    return lowPowerVolume_;
}

int32_t RendererInClientInner::SetOffloadMode(int32_t state, bool isAppBack)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->SetOffloadMode(state, isAppBack);
}

int32_t RendererInClientInner::UnsetOffloadMode()
{
    rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->UnsetOffloadMode();
}

float RendererInClientInner::GetSingleStreamVolume()
{
    // in plan. For now, keep it consistent with fast_audio_stream
    return 1.0f;
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
    return totalBytesWritten_ / static_cast<int64_t>(sizePerFrameInByte_);
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

bool RendererInClientInner::StartAudioStream(StateChangeCmdType cmdType,
    AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("RendererInClientInner::StartAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != PREPARED && state_ != STOPPED && state_ != PAUSED) {
        AUDIO_ERR_LOG("Start failed Illegal state:%{public}d", state_.load());
        return false;
    }

    hasFirstFrameWrited_ = false;
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchOutputDeviceForTrack(sessionId_, RUNNING, clientPid_, rendererInfo_, reason);
    }
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Start();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Start call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return state_ == RUNNING; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Start failed: timeout");
        ipcStream_->Stop();
        return false;
    }

    waitLock.unlock();

    AUDIO_INFO_LOG("Start SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("RUNNING");

    std::unique_lock<std::mutex> dataConnectionWaitLock(dataConnectionMutex_);
    if (!isDataLinkConnected_) {
        AUDIO_INFO_LOG("data-connection blocking starts.");
        stopWaiting = dataConnectionCV_.wait_for(
            dataConnectionWaitLock, std::chrono::milliseconds(DATA_CONNECTION_TIMEOUT_IN_MS), [this] {
                return isDataLinkConnected_;
            });
        AUDIO_INFO_LOG("data-connection blocking ends.");
    }
    dataConnectionWaitLock.unlock();

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
    preWriteEndTime_ = 0;
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
        return state_ == PAUSED; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Pause failed: timeout");
        return false;
    }

    waitLock.unlock();

    FutexTool::FutexWake(clientBuffer_->GetFutex());
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
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (!offloadEnable_) {
        DrainAudioStreamInner(true);
    }

    if (state_ == STOPPED) {
        AUDIO_INFO_LOG("Renderer in client is already stopped");
        return true;
    }
    if ((state_ != RUNNING) && (state_ != PAUSED)) {
        AUDIO_ERR_LOG("Stop failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    std::unique_lock<std::mutex> waitLock(callServerMutex_);
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

    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return state_ == STOPPED; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Stop failed: timeout");
        state_ = INVALID;
        return false;
    }

    waitLock.unlock();

    FutexTool::FutexWake(clientBuffer_->GetFutex());
    statusLock.unlock();

    // in plan: call HiSysEventWrite
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);

    AUDIO_INFO_LOG("Stop SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("STOPPED");
    return true;
}

bool RendererInClientInner::ReleaseAudioStream(bool releaseRunner, bool isSwitchStream)
{
    (void)isSwitchStream;
    AUDIO_PRERELEASE_LOGI("Enter");
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
    } else {
        AUDIO_WARNING_LOG("release while ipcStream is null");
    }

    // no lock, call release in any case, include blocked case.
    std::unique_lock<std::mutex> runnerlock(runnerMutex_);
    if (releaseRunner && callbackHandler_ != nullptr) {
        callbackHandler_->ReleaseEventRunner();
        runnerReleased_ = true;
        callbackHandler_ = nullptr;
    }
    runnerlock.unlock();

    // clear write callback
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        cbThreadReleased_ = true; // stop loop
        cbThreadCv_.notify_all();
        FutexTool::FutexWake(clientBuffer_->GetFutex(), IS_PRE_EXIT);
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

    // clear cbBufferQueue
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        cbBufferQueue_.Clear();
        if (memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_) != EOK) {
            AUDIO_ERR_LOG("memset_s buffer failed");
        };
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
    ResetFramePosition();
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

bool RendererInClientInner::DrainAudioStream(bool stopFlag)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    std::lock_guard<std::mutex> lock(writeMutex_);
    bool ret = DrainAudioStreamInner(stopFlag);
    return ret;
}

void RendererInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    std::lock_guard<std::mutex> lockSetPreferredFrameSize(setPreferredFrameSizeMutex_);
    userSettedPreferredFrameSize_ = frameSize;
    CHECK_AND_RETURN_LOG(curStreamParams_.encoding != ENCODING_AUDIOVIVID,
        "playing audiovivid, frameSize is always 1024.");
    size_t maxCbBufferSize =
        static_cast<size_t>(MAX_CBBUF_IN_USEC * curStreamParams_.samplingRate / AUDIO_US_PER_S) * sizePerFrameInByte_;
    size_t minCbBufferSize =
        static_cast<size_t>(MIN_CBBUF_IN_USEC * curStreamParams_.samplingRate / AUDIO_US_PER_S) * sizePerFrameInByte_;
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
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write with callback is not supported");
    int32_t ret = WriteInner(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    return ret <= 0 ? ret : static_cast<int32_t>(pcmBufferSize);
}

int32_t RendererInClientInner::Write(uint8_t *buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write with callback is not supported");
    return WriteInner(buffer, bufferSize);
}

bool RendererInClientInner::ProcessSpeed(uint8_t *&buffer, size_t &bufferSize, bool &speedCached)
{
    speedCached = false;
#ifdef SONIC_ENABLE
    if (!isEqual(speed_, 1.0f)) {
        Trace trace(traceTag_ + " ProcessSpeed");
        if (audioSpeed_ == nullptr) {
            AUDIO_ERR_LOG("audioSpeed_ is nullptr, use speed default 1.0");
            return true;
        }
        int32_t outBufferSize = 0;
        if (audioSpeed_->ChangeSpeedFunc(buffer, bufferSize, speedBuffer_, outBufferSize) == 0) {
            bufferSize = 0;
            AUDIO_ERR_LOG("process speed error");
            return false;
        }
        if (outBufferSize == 0) {
            AUDIO_DEBUG_LOG("speed buffer is not full");
            return false;
        }
        buffer = speedBuffer_.get();
        bufferSize = static_cast<size_t>(outBufferSize);
        speedCached = true;
    }
#endif
    return true;
}

void RendererInClientInner::DfxWriteInterval()
{
    if (preWriteEndTime_ != 0 &&
        ((ClockTime::GetCurNano() / AUDIO_US_PER_SECOND) - preWriteEndTime_) > MAX_WRITE_INTERVAL_MS) {
        AUDIO_WARNING_LOG("[%{public}s] write interval too long cost %{public}" PRId64,
            logUtilsTag_.c_str(), (ClockTime::GetCurNano() / AUDIO_US_PER_SECOND) - preWriteEndTime_);
    }
}
int32_t RendererInClientInner::WriteInner(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer,
    size_t metaBufferSize)
{
    Trace trace("RendererInClient::Write with meta " + std::to_string(pcmBufferSize));
    CHECK_AND_RETURN_RET_LOG(curStreamParams_.encoding == ENCODING_AUDIOVIVID, ERR_NOT_SUPPORTED,
        "Write: Write not supported. encoding doesnot match.");
    BufferDesc bufDesc = {pcmBuffer, pcmBufferSize, pcmBufferSize, metaBuffer, metaBufferSize};
    CHECK_AND_RETURN_RET_LOG(converter_ != nullptr, ERR_WRITE_FAILED, "Write: converter isn't init.");
    CHECK_AND_RETURN_RET_LOG(converter_->CheckInputValid(bufDesc), ERR_INVALID_PARAM, "Write: Invalid input.");

    WriteMuteDataSysEvent(pcmBuffer, pcmBufferSize);

    converter_->Process(bufDesc);
    uint8_t *buffer;
    uint32_t bufferSize;
    converter_->GetOutputBufferStream(buffer, bufferSize);
    return WriteInner(buffer, bufferSize);
}

void RendererInClientInner::FirstFrameProcess()
{
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "ipcStream is not inited!");
    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_) {
        ipcStream_->RegisterThreadPriority(gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(clientConfig_.appInfo.appUid));
        needSetThreadPriority_ = false;
    }

    if (!hasFirstFrameWrited_) { OnFirstFrameWriting(); }
}

int32_t RendererInClientInner::WriteRingCache(uint8_t *buffer, size_t bufferSize, bool speedCached,
    size_t oriBufferSize)
{
    size_t targetSize = bufferSize;
    size_t offset = 0;
    while (targetSize >= sizePerFrameInByte_) {
        // 1. write data into ring cache
        OptResult result = ringCache_->GetWritableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, speedCached ? oriBufferSize : bufferSize - targetSize,
            "RingCache write status invalid size is:%{public}zu", result.size);

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
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, speedCached ? oriBufferSize : bufferSize - targetSize,
            "RingCache read status invalid size is:%{public}zu", result.size);
        size_t readableSize = result.size;
        Trace::Count("RendererInClient::CacheBuffer->readableSize", readableSize);

        if (readableSize < clientSpanSizeInByte_) { continue; }
        // if readable size is enough, we will call write data to server
        int32_t ret = WriteCacheData();
        CHECK_AND_RETURN_RET_LOG(ret != ERR_ILLEGAL_STATE, speedCached ? oriBufferSize : bufferSize - targetSize,
            "Status changed while write");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "WriteCacheData failed %{public}d", ret);
    }
    preWriteEndTime_ = ClockTime::GetCurNano() / AUDIO_US_PER_SECOND;
    return speedCached ? oriBufferSize : bufferSize - targetSize;
}

int32_t RendererInClientInner::WriteInner(uint8_t *buffer, size_t bufferSize)
{
    // eg: RendererInClient::sessionId:100001 WriteSize:3840
    DfxWriteInterval();
    Trace trace(traceTag_+ " WriteSize:" + std::to_string(bufferSize));
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr && bufferSize < MAX_WRITE_SIZE && bufferSize > 0, ERR_INVALID_PARAM,
        "invalid size is %{public}zu", bufferSize);
    Trace::CountVolume(traceTag_, *buffer);
    if (gServerProxy_ == nullptr && getuid() == MEDIA_SERVICE_UID) {
        uint32_t samplingRate = clientConfig_.streamInfo.samplingRate;
        uint32_t channels = clientConfig_.streamInfo.channels;
        uint32_t samplePerFrame = Util::GetSamplePerFrame(clientConfig_.streamInfo.format);
        // calculate wait time by buffer size, 10e6 is converting seconds to microseconds
        uint32_t waitTimeUs = bufferSize * 10e6 / (samplingRate * channels * samplePerFrame);
        AUDIO_ERR_LOG("server is died! wait %{public}d us", waitTimeUs);
        usleep(waitTimeUs);
        return ERR_WRITE_BUFFER;
    }

    CHECK_AND_RETURN_RET_LOG(gServerProxy_ != nullptr, ERROR, "server is died");
    if (clientBuffer_->GetStreamStatus() == nullptr) {
        AUDIO_ERR_LOG("The stream status is null!");
        return ERR_INVALID_PARAM;
    }
    
    if (clientBuffer_->GetStreamStatus()->load() == STREAM_STAND_BY) {
        Trace trace2(traceTag_+ " call start to exit stand-by");
        CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERROR, "ipcStream is not inited!");
        int32_t ret = ipcStream_->Start();
        AUDIO_INFO_LOG("%{public}u call start to exit stand-by ret %{public}u", sessionId_, ret);
    }
    std::lock_guard<std::mutex> lock(writeMutex_);

    size_t oriBufferSize = bufferSize;
    bool speedCached = false;
    if (!ProcessSpeed(buffer, bufferSize, speedCached)) {
        return bufferSize;
    }

    WriteMuteDataSysEvent(buffer, bufferSize);

    FirstFrameProcess();

    CHECK_AND_RETURN_RET_PRELOG(state_ == RUNNING, ERR_ILLEGAL_STATE,
        "Write: Illegal state:%{public}u sessionid: %{public}u", state_.load(), sessionId_);

    // hold lock
    if (isBlendSet_) {
        audioBlend_.Process(buffer, bufferSize);
    }

    return WriteRingCache(buffer, bufferSize, speedCached, oriBufferSize);
}

void RendererInClientInner::ResetFramePosition()
{
    Trace trace("RendererInClientInner::ResetFramePosition");
    uint64_t timestampVal = 0;
    uint64_t latency = 0;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "ipcStream is not inited!");
    int32_t ret = ipcStream_->GetAudioPosition(lastFlushReadIndex_, timestampVal, latency);
    if (ret != SUCCESS) {
        AUDIO_PRERELEASE_LOGE("Get position failed: %{public}u", ret);
        return;
    }
    lastFramePosition_ = 0;
    lastReadIdx_ = 0;
    lastLatency_ = latency;
    lastLatencyPosition_ = latency * speed_;
}

void RendererInClientInner::WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize)
{
    if (silentModeAndMixWithOthers_) {
        return;
    }
    if (buffer[0] == 0) {
        if (startMuteTime_ == 0) {
            startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if ((currentTime - startMuteTime_ >= ONE_MINUTE) && !isUpEvent_) {
            AUDIO_WARNING_LOG("write silent data for some time");
            isUpEvent_ = true;
            std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
                Media::MediaMonitor::AUDIO, Media::MediaMonitor::BACKGROUND_SILENT_PLAYBACK,
                Media::MediaMonitor::FREQUENCY_AGGREGATION_EVENT);
            bean->Add("CLIENT_UID", appUid_);
            Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        }
    } else if (buffer[0] != 0 && startMuteTime_ != 0) {
        startMuteTime_ = 0;
    }
}

int32_t RendererInClientInner::DrainIncompleteFrame(OptResult result, bool stopFlag,
    size_t targetSize, BufferDesc *desc, bool &dropFlag)
{
    if (result.size < clientSpanSizeInByte_ && stopFlag) {
        result = ringCache_->Dequeue({desc->buffer, targetSize});
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
            "ringCache Dequeue failed %{public}d", result.ret);
        int32_t ret = memset_s(desc->buffer, targetSize, 0, targetSize);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "DrainIncompleteFrame memset output failed");
        AUDIO_WARNING_LOG("incomplete frame is set to 0");
        dropFlag = true;
    }
    return SUCCESS;
}


int32_t RendererInClientInner::WriteCacheData(bool isDrain, bool stopFlag)
{
    Trace traceCache(isDrain ? "RendererInClientInner::DrainCacheData" : "RendererInClientInner::WriteCacheData");

    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERR_OPERATION_FAILED, "ring cache unreadable");
    CHECK_AND_RETURN_RET_LOG(result.size != 0, SUCCESS, "Readable size is already zero");

    size_t targetSize = isDrain ? std::min(result.size, clientSpanSizeInByte_) : clientSpanSizeInByte_;

    int32_t sizeInFrame = clientBuffer_->GetAvailableDataFrames();
    CHECK_AND_RETURN_RET_LOG(sizeInFrame >= 0, ERROR, "GetAvailableDataFrames invalid, %{public}d", sizeInFrame);

    int32_t tryCount = 2; // try futex wait for 2 times.
    FutexCode futexRes = FUTEX_OPERATION_FAILED;
    while (static_cast<uint32_t>(sizeInFrame) < spanSizeInFrame_ && tryCount > 0) {
        tryCount--;
        int32_t timeout = offloadEnable_ ? OFFLOAD_OPERATION_TIMEOUT_IN_MS : WRITE_CACHE_TIMEOUT_IN_MS;
        futexRes = FutexTool::FutexWait(clientBuffer_->GetFutex(), static_cast<int64_t>(timeout) * AUDIO_US_PER_SECOND);
        CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "failed with state:%{public}d", state_.load());
        CHECK_AND_RETURN_RET_LOG(futexRes != FUTEX_TIMEOUT, ERROR,
            "write data time out, mode is %{public}s", (offloadEnable_ ? "offload" : "normal"));
        sizeInFrame = clientBuffer_->GetAvailableDataFrames();
        if (futexRes == FUTEX_SUCCESS && sizeInFrame > 0) { break; }
    }

    if (sizeInFrame < 0 || static_cast<uint32_t>(clientBuffer_->GetAvailableDataFrames()) < spanSizeInFrame_) {
        AUDIO_ERR_LOG("failed: sizeInFrame is:%{public}d, futexRes:%{public}d", sizeInFrame, futexRes);
        return ERROR;
    }
    BufferDesc desc = {};
    uint64_t curWriteIndex = clientBuffer_->GetCurWriteFrame();
    int32_t ret = clientBuffer_->GetWriteBuffer(curWriteIndex, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetWriteBuffer failed %{public}d", ret);
    bool dropFlag = false;
    CHECK_AND_RETURN_RET(DrainIncompleteFrame(result, stopFlag, targetSize, &desc, dropFlag) == SUCCESS, ERROR);
    if (dropFlag) {
        return SUCCESS;
    }
    result = ringCache_->Dequeue({desc.buffer, targetSize});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringCache Dequeue failed %{public}d", result.ret);

    // volume process in client
    if (volumeRamp_.IsActive()) {
        // do not call SetVolume here.
        clientVolume_ = volumeRamp_.GetRampVolume();
        AUDIO_INFO_LOG("clientVolume_:%{public}f", clientVolume_);
        Trace traceVolume("RendererInClientInner::WriteCacheData:Ramp:clientVolume_:" + std::to_string(clientVolume_));
        SetInnerVolume(clientVolume_);
    }

    DumpFileUtil::WriteDumpFile(dumpOutFd_, static_cast<void *>(desc.buffer), desc.bufLength);
    DfxOperation(desc, clientConfig_.streamInfo.format, clientConfig_.streamInfo.channels);
    clientBuffer_->SetCurWriteFrame(curWriteIndex + spanSizeInFrame_);

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "WriteCacheData failed, null ipcStream_.");
    ipcStream_->UpdatePosition(); // notiify server update position
    HandleRendererPositionChanges(desc.bufLength);
    return SUCCESS;
}

void RendererInClientInner::DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const
{
    ChannelVolumes vols = VolumeTools::CountVolumeLevel(buffer, format, channel);
    if (channel == MONO) {
        Trace::Count(logUtilsTag_, vols.volStart[0]);
    } else {
        Trace::Count(logUtilsTag_, (vols.volStart[0] + vols.volStart[1]) / HALF_FACTOR);
    }
    AudioLogUtils::ProcessVolumeData(logUtilsTag_, vols, volumeDataCount_);
}

void RendererInClientInner::HandleRendererPositionChanges(size_t bytesWritten)
{
    totalBytesWritten_ += static_cast<int64_t>(bytesWritten);
    if (sizePerFrameInByte_ == 0) {
        AUDIO_ERR_LOG("HandleRendererPositionChanges: sizePerFrameInByte_ is 0");
        return;
    }
    int64_t writtenFrameNumber = totalBytesWritten_ / static_cast<int64_t>(sizePerFrameInByte_);
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
        rendererPeriodWritten_ += static_cast<int64_t>((bytesWritten / sizePerFrameInByte_));
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
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, 0, "buffer is not inited");

    return clientBuffer_->GetUnderrunCount();
}

uint32_t RendererInClientInner::GetOverflowCount()
{
    AUDIO_WARNING_LOG("No Overflow in renderer");
    return 0;
}

void RendererInClientInner::SetUnderflowCount(uint32_t underflowCount)
{
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "buffer is not inited");
    clientBuffer_->SetUnderrunCount(underflowCount);
}

void RendererInClientInner::SetOverflowCount(uint32_t overflowCount)
{
    // not support for renderer
    AUDIO_WARNING_LOG("No Overflow in renderer");
    return;
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
    AUDIO_ERR_LOG("SetRendererSamplingRate to %{public}d is not supported", sampleRate);
    return ERROR;
}

uint32_t RendererInClientInner::GetRendererSamplingRate()
{
    return curStreamParams_.samplingRate;
}

int32_t RendererInClientInner::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    // bufferSizeInMsec is checked between 5ms and 20ms.
    bufferSizeInMsec_ = static_cast<uint32_t>(bufferSizeInMsec);
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
    audioBlend_.SetParams(blendMode, curStreamParams_.format, curStreamParams_.channels);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolumeWithRamp(float volume, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != INVALID) && (state_ != STOPPED),
        ERR_ILLEGAL_STATE, "Illegal state state %{public}d", state_.load());

    if (FLOAT_COMPARE_EQ(clientVolume_, volume)) {
        AUDIO_INFO_LOG("set same volume %{public}f", volume);
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
    info.params = streamParams_;

    info.rendererInfo = rendererInfo_;
    info.capturerInfo = capturerInfo_;
    info.eStreamType = eStreamType_;
    info.renderMode = renderMode_;
    info.state = state_;
    info.sessionId = sessionId_;
    info.streamTrackerRegistered = streamTrackerRegistered_;
    GetStreamSwitchInfo(info);

    {
        std::lock_guard<std::mutex> lock(setPreferredFrameSizeMutex_);
        info.userSettedPreferredFrameSize = userSettedPreferredFrameSize_;
    }
}

void RendererInClientInner::GetStreamSwitchInfo(IAudioStream::SwitchInfo& info)
{
    info.cachePath = cachePath_;
    info.underFlowCount = GetUnderflowCount();
    info.effectMode = effectMode_;
    info.renderRate = rendererRate_;
    info.clientPid = clientPid_;
    info.clientUid = clientUid_;
    info.volume = clientVolume_;
    info.silentModeAndMixWithOthers = silentModeAndMixWithOthers_;

    info.frameMarkPosition = static_cast<uint64_t>(rendererMarkPosition_);
    info.renderPositionCb = rendererPositionCallback_;

    info.framePeriodNumber = static_cast<uint64_t>(rendererPeriodSize_);
    info.renderPeriodPositionCb = rendererPeriodPositionCallback_;

    info.rendererWriteCallback = writeCb_;
}

IAudioStream::StreamClass RendererInClientInner::GetStreamClass()
{
    return PA_STREAM;
}

void RendererInClientInner::OnSpatializationStateChange(const AudioSpatializationState &spatializationState)
{
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "Object ipcStream is nullptr");
    CHECK_AND_RETURN_LOG(ipcStream_->UpdateSpatializationState(spatializationState.spatializationEnabled,
        spatializationState.headTrackingEnabled) == SUCCESS, "Update spatialization state failed");
}

bool RendererInClientInner::GetOffloadEnable()
{
    return offloadEnable_;
}

bool RendererInClientInner::GetSpatializationEnabled()
{
    return rendererInfo_.spatializationEnabled;
}

bool RendererInClientInner::GetHighResolutionEnabled()
{
    return AudioPolicyManager::GetInstance().IsHighResolutionExist();
}

int32_t RendererInClientInner::RegisterSpatializationStateEventListener()
{
    if (firstSpatializationRegistered_) {
        firstSpatializationRegistered_ = false;
    } else {
        UnregisterSpatializationStateEventListener(spatializationRegisteredSessionID_);
    }

    if (!spatializationStateChangeCallback_) {
        spatializationStateChangeCallback_ = std::make_shared<SpatializationStateChangeCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(spatializationStateChangeCallback_, ERROR, "Memory Allocation Failed !!");
    }
    spatializationStateChangeCallback_->SetRendererInClientPtr(shared_from_this());

    int32_t ret = AudioPolicyManager::GetInstance().RegisterSpatializationStateEventListener(
        sessionId_, rendererInfo_.streamUsage, spatializationStateChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "RegisterSpatializationStateEventListener failed");
    spatializationRegisteredSessionID_ = sessionId_;

    return SUCCESS;
}

int32_t RendererInClientInner::UnregisterSpatializationStateEventListener(uint32_t sessionID)
{
    int32_t ret = AudioPolicyManager::GetInstance().UnregisterSpatializationStateEventListener(sessionID);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "UnregisterSpatializationStateEventListener failed");
    return SUCCESS;
}

bool RendererInClientInner::DrainAudioStreamInner(bool stopFlag)
{
    Trace trace("RendererInClientInner::DrainAudioStreamInner " + std::to_string(sessionId_));
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false, "Drain failed. Illegal state:%{public}u", state_.load());
    CHECK_AND_RETURN_RET_LOG(WriteCacheData(true, stopFlag) == SUCCESS, false, "Drain cache failed");

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    AUDIO_INFO_LOG("stopFlag:%{public}d", stopFlag);
    int32_t ret = ipcStream_->Drain(stopFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Drain call server failed:%{public}u", ret);
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

void RendererInClientInner::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    sptr<IStandardAudioService> gasp = RendererInClientInner::GetAudioServerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("LatencyMeas failed to get AudioServerProxy");
        return;
    }
    gasp->UpdateLatencyTimestamp(timestamp, isRenderer);
}

void RendererInClientInner::SetSilentModeAndMixWithOthers(bool on)
{
    AUDIO_PRERELEASE_LOGI("SetSilentModeAndMixWithOthers %{public}d", on);
    silentModeAndMixWithOthers_ = on;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "Object ipcStream is nullptr");
    ipcStream_->SetSilentModeAndMixWithOthers(on);
    return;
}

bool RendererInClientInner::GetSilentModeAndMixWithOthers()
{
    return silentModeAndMixWithOthers_;
}

SpatializationStateChangeCallbackImpl::SpatializationStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("Instance create");
}

SpatializationStateChangeCallbackImpl::~SpatializationStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("Instance destory");
}

void SpatializationStateChangeCallbackImpl::SetRendererInClientPtr(
    std::shared_ptr<RendererInClientInner> rendererInClientPtr)
{
    rendererInClientPtr_ = rendererInClientPtr;
}

void SpatializationStateChangeCallbackImpl::OnSpatializationStateChange(
    const AudioSpatializationState &spatializationState)
{
    std::shared_ptr<RendererInClientInner> rendererInClient = rendererInClientPtr_.lock();
    if (rendererInClient != nullptr) {
        rendererInClient->OnSpatializationStateChange(spatializationState);
    }
}

bool RendererInClientInner::RestoreAudioStream(bool needStoreState)
{
    CHECK_AND_RETURN_RET_LOG(proxyObj_ != nullptr, false, "proxyObj_ is null");
    CHECK_AND_RETURN_RET_LOG(state_ != NEW && state_ != INVALID && state_ != RELEASED, true,
        "state_ is %{public}d, no need for restore", state_.load());
    bool result = false;
    State oldState = state_;
    state_ = NEW;
    SetStreamTrackerState(false);
    // If pipe type is offload, need reset to normal.
    // Otherwise, unable to enter offload mode.
    if (rendererInfo_.pipeType == PIPE_TYPE_OFFLOAD) {
        rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    }
    int32_t ret = SetAudioStreamInfo(streamParams_, proxyObj_);
    if (ret != SUCCESS) {
        goto error;
    }
    if (!needStoreState) {
        AUDIO_INFO_LOG("telephony scene, return directly");
        return ret == SUCCESS;
    }
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

int32_t RendererInClientInner::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "Renderer stream state is not RUNNING");
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    uint64_t readIdx = 0;
    uint64_t timestampVal = 0;
    uint64_t latency = 0;
    int32_t ret = ipcStream_->GetAudioPosition(readIdx, timestampVal, latency);
    readIdx = readIdx > lastFlushReadIndex_ ? readIdx - lastFlushReadIndex_ : 0;
    uint64_t framePosition = lastFramePosition_;
    if (readIdx >= latency + lastReadIdx_) { // happen when last speed latency consumed
        framePosition += lastLatencyPosition_ + (readIdx - lastReadIdx_ - latency) * speed_;
        lastLatency_ = latency;
        lastLatencyPosition_ = latency * speed_;
        lastReadIdx_ = readIdx;
    } else { // happen when last speed latency not consumed
        if (lastLatency_ + readIdx > latency + lastReadIdx_) {
            framePosition += lastLatencyPosition_ * (lastLatency_ + readIdx - latency - lastReadIdx_) / lastLatency_;
            lastLatencyPosition_ = lastLatencyPosition_ * (latency + lastReadIdx_ - readIdx) / lastLatency_;
            lastLatency_ = latency + lastReadIdx_ - readIdx;
        }
    }
    // add MCR latency
    uint32_t mcrLatency = 0;
    if (converter_ != nullptr) {
        mcrLatency = converter_->GetLatency() * curStreamParams_.samplingRate / AUDIO_MS_PER_S;
        framePosition = framePosition > mcrLatency ? framePosition - mcrLatency : 0;
    }
 
    if (lastFramePosition_ < framePosition) {
        lastFramePosition_ = framePosition;
        lastFrameTimestamp_ = timestampVal;
    } else {
        AUDIO_DEBUG_LOG("The frame position should be continuously increasing");
        framePosition = lastFramePosition_;
        timestampVal = lastFrameTimestamp_;
    }
    AUDIO_DEBUG_LOG("[CLIENT]Latency info: framePosition: %{public}" PRIu64 ", lastFlushReadIndex_ %{public}" PRIu64
        ", timestamp %{public}" PRIu64 ", lastLatencyPosition_ %{public}" PRIu64 ", totlatency %{public}" PRIu64,
        framePosition, lastFlushReadIndex_, timestampVal, lastLatencyPosition_, latency + mcrLatency);
 
    timestamp.framePosition = framePosition;
    timestamp.time.tv_sec = static_cast<time_t>(timestampVal / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(timestampVal % AUDIO_NS_PER_SECOND);
    return ret;
}

void RendererInClientInner::SetSwitchingStatus(bool isSwitching)
{
    std::lock_guard lock(switchingMutex_);
    if (isSwitching) {
        switchingInfo_ = {true, state_};
    } else {
        switchingInfo_ = {false, INVALID};
    }
}
} // namespace AudioStandard
} // namespace OHOS
#endif // FAST_AUDIO_STREAM_H
