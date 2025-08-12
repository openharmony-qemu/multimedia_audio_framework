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
#define LOG_TAG "RendererInClientInner"
#endif

#include "renderer_in_client.h"
#include "renderer_in_client_private.h"

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
#include "hisysevent.h"

#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_manager_base.h"
#include "audio_renderer_log.h"
#include "audio_channel_blend.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
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
#include "volume_tools.h"

#include "media_monitor_manager.h"
#include "istandard_audio_service.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
namespace {
const uint64_t OLD_BUF_DURATION_IN_USEC = 92880; // This value is used for compatibility purposes.
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
const int64_t MUTE_PLAY_MIN_DURAION = 3000000000; // 3S
const int64_t MUTE_PLAY_MAX_DURAION = 30000000000; // 30S
static const size_t MAX_WRITE_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static const int32_t OFFLOAD_OPERATION_TIMEOUT_IN_MS = 8000; // 8000ms for offload
static const int32_t WRITE_CACHE_TIMEOUT_IN_MS = 1500; // 1500ms
static const int32_t WRITE_BUFFER_TIMEOUT_IN_MS = 20; // ms
static const uint32_t WAIT_FOR_NEXT_CB = 10000; // 10ms
static constexpr int32_t ONE_MINUTE = 60;
static const int32_t MAX_WRITE_INTERVAL_MS = 40;
constexpr int32_t RETRY_WAIT_TIME_MS = 500; // 500ms
constexpr int32_t MAX_RETRY_COUNT = 8;
} // namespace

static AppExecFwk::BundleInfo gBundleInfo_;
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
    sptr<IStandardAudioService> gasp = gServerProxy_;
    return gasp;
}

void RendererInClientInner::AudioServerDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("audio server died clear proxy, will restore proxy in next call");
    std::lock_guard<std::mutex> lock(g_serverProxyMutex);
    gServerProxy_ = nullptr;
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

bool RendererInClientInner::IsHighResolution() const noexcept
{
    return eStreamType_ == STREAM_MUSIC && curStreamParams_.samplingRate >= SAMPLE_RATE_48000 &&
           curStreamParams_.format >= SAMPLE_S24LE;
}

void RendererInClientInner::InitDirectPipeType()
{
    if (rendererInfo_.rendererFlags == AUDIO_FLAG_VOIP_DIRECT || IsHighResolution()) {
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

// call this without lock, we should be able to call deinit in any case.
int32_t RendererInClientInner::DeinitIpcStream()
{
    Trace trace("RendererInClientInner::DeinitIpcStream");
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERROR,
        "ipcStream_ is nullptr");
    ipcStream_->Release(false);
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

    if (rendererInfo_.rendererFlags != AUDIO_FLAG_NORMAL && rendererInfo_.rendererFlags != AUDIO_FLAG_VOIP_DIRECT &&
        rendererInfo_.rendererFlags != AUDIO_FLAG_DIRECT) {
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
    int32_t ret = ipcStream_->ResolveBufferBaseAndGetServerSpanSize(clientBuffer_, spanSizeInFrame_,
        engineTotalSizeInFrame_);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && clientBuffer_ != nullptr, ret, "ResolveBuffer failed:%{public}d", ret);

    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ret = clientBuffer_->GetSizeParameter(totalSizeInFrame, byteSizePerFrame);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && byteSizePerFrame == sizePerFrameInByte_, ret, "GetSizeParameter failed"
        ":%{public}d, byteSizePerFrame:%{public}u, sizePerFrameInByte_:%{public}zu", ret, byteSizePerFrame,
        sizePerFrameInByte_);

    clientSpanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame;

    AUDIO_INFO_LOG("totalSizeInFrame_[%{public}u] spanSizeInFrame[%{public}u] sizePerFrameInByte_[%{public}zu]"
        "clientSpanSizeInByte_[%{public}zu]", totalSizeInFrame, spanSizeInFrame_, sizePerFrameInByte_,
        clientSpanSizeInByte_);

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
    sptr<IRemoteObject> ipcProxy = nullptr;
    AudioPlaybackCaptureConfig playbackConfig = {};
    gasp->CreateAudioProcess(config, errorCode, playbackConfig, ipcProxy);
    for (int32_t retrycount = 0; (errorCode == ERR_RETRY_IN_CLIENT) && (retrycount < MAX_RETRY_COUNT); retrycount++) {
        AUDIO_WARNING_LOG("retry in client");
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_TIME_MS));
        gasp->CreateAudioProcess(config, errorCode, playbackConfig, ipcProxy);
    }
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, ERR_OPERATION_FAILED, "failed with null ipcProxy.");
    ipcStream_ = iface_cast<IIpcStream>(ipcProxy);
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

    ret = ipcStream_->GetAudioSessionID(sessionId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioSessionID failed:%{public}d", ret);
    traceTag_ = "[" + std::to_string(sessionId_) + "]RendererInClient"; // [100001]RendererInClient
    InitCallbackHandler();
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
        return ERROR;
    }
    AUDIO_PRERELEASE_LOGI("volume: %{public}f", volume);
    return SUCCESS;
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
        if (bufferDesc.dataLength != 0) {
            result = WriteInner(bufferDesc.buffer, bufferDesc.bufLength, bufferDesc.metaBuffer, bufferDesc.metaLength);
        } else {
            AUDIO_WARNING_LOG("INVALID AudioVivid buffer");
            usleep(WAIT_FOR_NEXT_CB);
        }
    }
    if (curStreamParams_.encoding == ENCODING_PCM) {
        if (bufferDesc.dataLength != 0) {
            result = WriteInner(bufferDesc.buffer, bufferDesc.bufLength);
            sleepCount_ = LOG_COUNT_LIMIT;
        } else {
            int32_t readableSizeInFrames = clientBuffer_->GetReadableDataFrames();
            bool flagTryPrintLog = ((readableSizeInFrames >= 0) && (readableSizeInFrames < spanSizeInFrame_));
            if (flagTryPrintLog && (sleepCount_++ == LOG_COUNT_LIMIT)) {
                sleepCount_ = 0;
                AUDIO_WARNING_LOG("1st or 200 times INVALID buffer");
            }
            usleep(WAIT_FOR_NEXT_CB);
        }
    }
    if (result < 0) {
        AUDIO_WARNING_LOG("Call write fail, result:%{public}d, bufLength:%{public}zu", result, bufferDesc.bufLength);
    }
    return result;
}

bool RendererInClientInner::CheckBufferNeedWrite()
{
    uint32_t totalSizeInFrame = clientBuffer_->GetTotalSizeInFrame();
    size_t totalSizeInByte = totalSizeInFrame * sizePerFrameInByte_;
    int32_t writableInFrame = clientBuffer_ -> GetWritableDataFrames();
    size_t writableSizeInByte = writableInFrame * sizePerFrameInByte_;

    if (writableInFrame <= 0) {
        return false;
    }

    if (cbBufferSize_ > totalSizeInByte) {
        return false;
    }

    // readable >= engineTotalSizeInFrame_
    if (static_cast<uint64_t>(writableInFrame) <
        (static_cast<uint64_t>(totalSizeInFrame) - engineTotalSizeInFrame_)) {
        return false;
    }

    if (writableSizeInByte < cbBufferSize_) {
        return false;
    }

    return true;
}

bool RendererInClientInner::IsRestoreNeeded()
{
    RestoreStatus restoreStatus = clientBuffer_->GetRestoreStatus();
    if (restoreStatus == NEED_RESTORE) {
        return true;
    }

    if (restoreStatus == NEED_RESTORE_TO_NORMAL) {
        return true;
    }

    return false;
}

void RendererInClientInner::WaitForBufferNeedWrite()
{
    int32_t timeout = offloadEnable_ ? OFFLOAD_OPERATION_TIMEOUT_IN_MS : WRITE_CACHE_TIMEOUT_IN_MS;
    FutexCode futexRes = clientBuffer_->WaitFor(
        static_cast<int64_t>(timeout) * AUDIO_US_PER_SECOND,
        [this] () {
            if (state_ != RUNNING) {
                return true;
            }

            if (IsRestoreNeeded()) {
                return true;
            }

            return CheckBufferNeedWrite();
        });
    if (futexRes != SUCCESS) {
        AUDIO_ERR_LOG("futex err: %{public}d", futexRes);
    }
}

bool RendererInClientInner::WriteCallbackFunc()
{
    CHECK_AND_RETURN_RET_LOG(!cbThreadReleased_, false, "Callback thread released");
    Trace traceLoop("RendererInClientInner::WriteCallbackFunc");
    if (!WaitForRunning()) {
        return true;
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
        // only run in pause scene, do not repush audiovivid buffer cause metadata error
        if (result > 0 && static_cast<size_t>(result) < temp.dataLength &&
            curStreamParams_.encoding == ENCODING_PCM) {
            BufferDesc tmp = {temp.buffer + static_cast<size_t>(result),
                temp.bufLength - static_cast<size_t>(result), temp.dataLength - static_cast<size_t>(result)};
            cbBufferQueue_.Push(tmp);
            AUDIO_INFO_LOG("Repush %{public}zu bytes in queue", temp.dataLength - static_cast<size_t>(result));
            break;
        }
    }

    WaitForBufferNeedWrite();

    if (state_ != RUNNING) {
        return true;
    }
    // call client write
    std::shared_ptr<AudioRendererWriteCallback> cb = nullptr;
    {
        std::unique_lock<std::mutex> lockCb(writeCbMutex_);
        cb = writeCb_;
    }
    if (cb != nullptr) {
        Trace traceCb("RendererInClientInner::OnWriteData");
        cb->OnWriteData(cbBufferSize_);
    }

    Trace traceQueuePush("RendererInClientInner::QueueWaitPush");
    std::unique_lock<std::mutex> lockBuffer(cbBufferMutex_);
    cbBufferQueue_.WaitNotEmptyFor(std::chrono::milliseconds(WRITE_BUFFER_TIMEOUT_IN_MS));
    return true;
}

bool RendererInClientInner::ProcessSpeed(uint8_t *&buffer, size_t &bufferSize, bool &speedCached)
{
    speedCached = false;
#ifdef SONIC_ENABLE
    std::lock_guard lockSpeed(speedMutex_);
    if (speedEnable_.load()) {
        Trace trace(traceTag_ + " ProcessSpeed" + std::to_string(speed_));
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
    if (ipcStream_ == nullptr) {
        AUDIO_ERR_LOG("Error: ipcStream_ is not initialized!");
        return;
    }

    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_.exchange(false)) {
        ipcStream_->RegisterThreadPriority(gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(clientConfig_.appInfo.appUid), METHOD_WRITE_OR_READ);
    }

    if (!hasFirstFrameWrited_.exchange(true)) { OnFirstFrameWriting(); }
}

int32_t RendererInClientInner::WriteCacheData(uint8_t *buffer, size_t bufferSize, bool speedCached,
    size_t oriBufferSize)
{
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ > 0, ERROR, "sizePerFrameInByte :%{public}zu", sizePerFrameInByte_);
    size_t remainSize = (bufferSize / sizePerFrameInByte_) * sizePerFrameInByte_;

    RingBufferWrapper inBuffer = {
        .basicBufferDescs = {{
            {.buffer = buffer, .bufLength = remainSize},
            {.buffer = nullptr, .bufLength = 0}
        }},
        .dataLength = 0
    };

    while (remainSize >= sizePerFrameInByte_) {
        FutexCode futexRes = FUTEX_OPERATION_FAILED;
        int32_t timeout = offloadEnable_ ? OFFLOAD_OPERATION_TIMEOUT_IN_MS : WRITE_CACHE_TIMEOUT_IN_MS;
        futexRes = clientBuffer_->WaitFor(static_cast<int64_t>(timeout) * AUDIO_US_PER_SECOND,
            [this] () {
                return (state_ != RUNNING) ||
                    (static_cast<uint32_t>(clientBuffer_->GetWritableDataFrames()) > 0);
            });
        CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "failed with state:%{public}d", state_.load());
        CHECK_AND_RETURN_RET_LOG(futexRes != FUTEX_TIMEOUT, ERROR,
            "write data time out, mode is %{public}s", (offloadEnable_ ? "offload" : "normal"));

        uint64_t writePos = clientBuffer_->GetCurWriteFrame();
        uint64_t readPos = clientBuffer_->GetCurReadFrame();
        CHECK_AND_RETURN_RET_LOG(writePos >= readPos, ERROR,
            "writePos: %{public}" PRIu64 " readPos: %{public}" PRIu64 "",
            writePos, readPos);
        RingBufferWrapper ringBuffer;
        int32_t ret = clientBuffer_->GetAllWritableBufferFromPosFrame(writePos, ringBuffer);
        CHECK_AND_RETURN_RET(ret == SUCCESS && (ringBuffer.dataLength > 0), ERROR);
        auto copySize = std::min(remainSize, ringBuffer.dataLength);
        inBuffer.dataLength = copySize;
        ret = ringBuffer.CopyInputBufferValueToCurBuffer(inBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "errcode: %{public}d", ret);
        clientBuffer_->SetCurWriteFrame((writePos + (copySize / sizePerFrameInByte_)), false);
        inBuffer.SeekFromStart(copySize);
        remainSize -= copySize;
    }
    size_t writtenSize = bufferSize - remainSize;

    preWriteEndTime_ = ClockTime::GetCurNano() / AUDIO_US_PER_SECOND;

    if (!ProcessVolume()) {
        return ERR_OPERATION_FAILED;
    }
    DumpFileUtil::WriteDumpFile(dumpOutFd_, static_cast<void *>(buffer), writtenSize);
    VolumeTools::DfxOperation({.buffer = buffer, .bufLength = writtenSize, .dataLength = writtenSize},
        clientConfig_.streamInfo, traceTag_, volumeDataCount_);

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "WriteCacheData failed, null ipcStream_.");
    ipcStream_->UpdatePosition(); // notiify server update position
    HandleRendererPositionChanges(writtenSize);

    return speedCached ? oriBufferSize : writtenSize;
}

int32_t RendererInClientInner::WriteInner(uint8_t *buffer, size_t bufferSize)
{
    // eg: RendererInClient::sessionId:100001 WriteSize:3840
    DfxWriteInterval();
    Trace trace(traceTag_+ " WriteSize:" + std::to_string(bufferSize));
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr && bufferSize < MAX_WRITE_SIZE && bufferSize > 0, ERR_INVALID_PARAM,
        "invalid size is %{public}zu", bufferSize);

    // Bugfix. Callback threadloop would go into infinite loop, consuming too much data from app
    // but fail to play them due to audio server's death. Block and exit callback threadloop when server died.
    if (gServerProxy_ == nullptr) {
        cbThreadReleased_ = true;
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

    FirstFrameProcess();

    std::lock_guard<std::mutex> lock(writeMutex_);

    size_t oriBufferSize = bufferSize;
    bool speedCached = false;

    unprocessedFramesBytes_.fetch_add(bufferSize / sizePerFrameInByte_);
    if (!ProcessSpeed(buffer, bufferSize, speedCached)) {
        return bufferSize;
    }

    WriteMuteDataSysEvent(buffer, bufferSize);

    CHECK_AND_RETURN_RET_PRELOG(state_ == RUNNING, ERR_ILLEGAL_STATE,
        "Write: Illegal state:%{public}u sessionid: %{public}u", state_.load(), sessionId_);

    // hold lock
    if (isBlendSet_) {
        audioBlend_.Process(buffer, bufferSize);
    }
    totalBytesWrittenAfterFlush_.fetch_add(bufferSize / sizePerFrameInByte_);
    int32_t result = WriteCacheData(buffer, bufferSize, speedCached, oriBufferSize);
    MonitorMutePlay(false);
    return result;
}

void RendererInClientInner::ResetFramePosition()
{
    Trace trace("RendererInClientInner::ResetFramePosition");
    uint64_t timestampval = 0;
    uint64_t latency = 0;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "ipcStream is not inited!");
    int32_t ret = ipcStream_->GetAudioPosition(lastFlushReadIndex_, timestampval, latency,
        Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_PRELOG(ret == SUCCESS, "Get position failed: %{public}d", ret);
    ret = ipcStream_->GetSpeedPosition(lastSpeedFlushReadIndex_, timestampval, latency,
        Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_PRELOG(ret == SUCCESS, "Get speed position failed: %{public}d", ret);
    // no need to reset timestamp, only reset frameposition
    for (int32_t base = 0; base < Timestamp::Timestampbase::BASESIZE; base++) {
        lastFramePosAndTimePair_[base].first = 0;
        lastFramePosAndTimePairWithSpeed_[base].first = 0;
        lastSwitchPosition_[base] = 0;
    }
    unprocessedFramesBytes_ = 0;
    totalBytesWrittenAfterFlush_ = 0;
    writtenAtSpeedChange_.store(WrittenFramesWithSpeed{0, speed_});
}

bool RendererInClientInner::IsMutePlaying()
{
    // this is updated in DfxOperation
    if (volumeDataCount_ < 0) {
        return true;
    }

    return mutePlaying_;
}

void RendererInClientInner::MonitorMutePlay(bool isPlayEnd)
{
    int64_t cur = ClockTime::GetRealNano();
    // judge if write mute
    bool isMutePlay = isPlayEnd ? false : IsMutePlaying();
    // not write mute or play end
    if (!isMutePlay) {
        if (mutePlayStartTime_ == 0) {
            return;
        }
        if (cur - mutePlayStartTime_ > MUTE_PLAY_MIN_DURAION) {
            ReportWriteMuteEvent(cur - mutePlayStartTime_);
            return;
        }
        mutePlayStartTime_ = 0;
        return;
    }

    // write mute
    if (mutePlayStartTime_ == 0) {
        // record first mute play
        mutePlayStartTime_ = cur;
        return;
    }
    if (cur - mutePlayStartTime_ > MUTE_PLAY_MAX_DURAION) {
        ReportWriteMuteEvent(cur - mutePlayStartTime_);
    }
}

void RendererInClientInner::ReportWriteMuteEvent(int64_t mutePlayDuration)
{
    mutePlayDuration /= AUDIO_US_PER_SECOND; // ns -> ms
    bool isMute = GetMute();
    bool isClientMute = muteCmd_ == CMD_FROM_CLIENT;
    uint8_t muteState = (isClientMute ? 0x0 : 0x4) | (isMute ? 0x1 : 0x0);

    AUDIO_WARNING_LOG("[%{public}d]MutePlaying for %{public}" PRId64" ms, muteState:%{public}d", sessionId_,
        mutePlayDuration, muteState);
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::APP_WRITE_MUTE, Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("UID", appUid_); // for APP_BUNDLE_NAME
    bean->Add("STREAM_TYPE", clientConfig_.rendererInfo.streamUsage);
    bean->Add("SESSION_ID", static_cast<int32_t>(sessionId_));
    bean->Add("STREAM_VOLUME", clientVolume_);
    bean->Add("MUTE_STATE", static_cast<int32_t>(muteState));
    bean->Add("APP_BACKGROUND_STATE", 0);
    bean->Add("MUTE_PLAY_START_TIME", static_cast<uint64_t>(mutePlayStartTime_ / AUDIO_US_PER_SECOND));
    bean->Add("MUTE_PLAY_DURATION", static_cast<int32_t>(mutePlayDuration));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    mutePlayStartTime_ = 0; // reset it to 0 for next record
}

void RendererInClientInner::WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize)
{
    if (silentModeAndMixWithOthers_) {
        return;
    }
    if (IsInvalidBuffer(buffer, bufferSize)) {
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

bool RendererInClientInner::IsInvalidBuffer(uint8_t *buffer, size_t bufferSize)
{
    bool isInvalid = false;
    uint8_t ui8Data = 0;
    int16_t i16Data = 0;
    switch (clientConfig_.streamInfo.format) {
        case SAMPLE_U8:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 0, false, "buffer size is too small");
            ui8Data = *buffer;
            isInvalid = ui8Data == 0;
            break;
        case SAMPLE_S16LE:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 1, false, "buffer size is too small");
            i16Data = *(reinterpret_cast<const int16_t*>(buffer));
            isInvalid = i16Data == 0;
            break;
        default:
            break;
    }
    return isInvalid;
}

bool RendererInClientInner::ProcessVolume()
{
    // volume process in client
    if (volumeRamp_.IsActive()) {
        // do not call SetVolume here.
        clientVolume_ = volumeRamp_.GetRampVolume();
        AUDIO_INFO_LOG("clientVolume_:%{public}f", clientVolume_);
        Trace traceVolume("RendererInClientInner::WriteCacheData:Ramp:clientVolume_:" + std::to_string(clientVolume_));
        SetInnerVolume(clientVolume_);
    }
    return true;
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
    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("Drain failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    AUDIO_INFO_LOG("stopFlag:%{public}d", stopFlag);
    int32_t ret = ipcStream_->Drain(stopFlag);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Drain call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == DRAIN_STREAM; // will be false when got notified.
    });

    // clear cbBufferQueue
    if (renderMode_ == RENDER_MODE_CALLBACK && stopFlag) {
        cbBufferQueue_.Clear();
        if (memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_) != EOK) {
            AUDIO_ERR_LOG("memset_s buffer failed");
        };
    }

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

void RendererInClientInner::RegisterThreadPriorityOnStart(StateChangeCmdType cmdType)
{
    pid_t tid;
    switch (rendererInfo_.playerType) {
        case PLAYER_TYPE_ARKTS_AUDIO_RENDERER:
            // main thread
            tid = getpid();
            break;
        case PLAYER_TYPE_OH_AUDIO_RENDERER:
            tid = gettid();
            break;
        default:
            return;
    }

    if (cmdType == CMD_FROM_CLIENT) {
        std::lock_guard lock(lastCallStartByUserTidMutex_);
        lastCallStartByUserTid_ = tid;
    } else if (cmdType == CMD_FROM_SYSTEM) {
        std::lock_guard lock(lastCallStartByUserTidMutex_);
        CHECK_AND_RETURN_LOG(lastCallStartByUserTid_.has_value(), "has not value");
        tid = lastCallStartByUserTid_.value();
    } else {
        AUDIO_ERR_LOG("illegal param");
        return;
    }

    ipcStream_->RegisterThreadPriority(tid,
        AudioSystemManager::GetInstance()->GetSelfBundleName(clientConfig_.appInfo.appUid), METHOD_START);
}

void RendererInClientInner::ResetCallbackLoopTid()
{
    AUDIO_INFO_LOG("to -1");
    callbackLoopTid_ = -1;
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

void RendererInClientInner::FlushSpeedBuffer()
{
    std::lock_guard lock(speedMutex_);

    if (audioSpeed_ != nullptr) {
        audioSpeed_->Flush();
    }
}

int32_t RendererInClientInner::SetSpeedInner(float speed)
{
    // set the speed to 1.0 and the speed has never been turned on, no actual sonic stream is created.
    if (isEqual(speed, SPEED_NORMAL) && !speedEnable_) {
        speed_ = speed;
        return SUCCESS;
    }

    if (audioSpeed_ == nullptr) {
        audioSpeed_ = std::make_unique<AudioSpeed>(curStreamParams_.samplingRate, curStreamParams_.format,
            curStreamParams_.channels);
        GetBufferSize(bufferSize_);
        speedBuffer_ = std::make_unique<uint8_t[]>(MAX_SPEED_BUFFER_SIZE);
    }
    audioSpeed_->SetSpeed(speed);
    writtenAtSpeedChange_.store(WrittenFramesWithSpeed{totalBytesWrittenAfterFlush_.load(), speed_});
    speed_ = speed;
    speedEnable_ = true;
    AUDIO_DEBUG_LOG("SetSpeed %{public}f, OffloadEnable %{public}d", speed_, offloadEnable_);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
