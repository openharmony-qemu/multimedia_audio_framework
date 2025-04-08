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
#include "audio_ring_cache.h"
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

using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
namespace {
const uint64_t OLD_BUF_DURATION_IN_USEC = 92880; // This value is used for compatibility purposes.
const uint64_t AUDIO_US_PER_S = 1000000;
const uint64_t MAX_BUF_DURATION_IN_USEC = 2000000; // 2S
static const size_t MAX_WRITE_SIZE = 20 * 1024 * 1024; // 20M
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static const int32_t OFFLOAD_OPERATION_TIMEOUT_IN_MS = 8000; // 8000ms for offload
static const int32_t WRITE_CACHE_TIMEOUT_IN_MS = 1500; // 1500ms
static const int32_t WRITE_BUFFER_TIMEOUT_IN_MS = 20; // ms
static const uint32_t WAIT_FOR_NEXT_CB = 5000; // 5ms
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
    for (int32_t retrycount = 0; (errorCode == ERR_RETRY_IN_CLIENT) && (retrycount < MAX_RETRY_COUNT); retrycount++) {
        AUDIO_WARNING_LOG("retry in client");
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_TIME_MS));
        ipcProxy = gasp->CreateAudioProcess(config, errorCode);
    }
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
    AUDIO_PRERELEASE_LOGI("SetClientVolume success, volume: %{public}f", volume);
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

bool RendererInClientInner::WriteCallbackFunc()
{
    if (cbThreadReleased_) {
        AUDIO_INFO_LOG("Callback thread released");
        return false;
    }
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
        // only run in pause scene
        if (result > 0 && static_cast<size_t>(result) < temp.dataLength) {
            BufferDesc tmp = {temp.buffer + static_cast<size_t>(result),
                temp.bufLength - static_cast<size_t>(result), temp.dataLength - static_cast<size_t>(result)};
            cbBufferQueue_.Push(tmp);
            AUDIO_INFO_LOG("Repush %{public}zu bytes in queue", temp.dataLength - static_cast<size_t>(result));
            break;
        }
    }
    if (state_ != RUNNING) {
        return true;
    }
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
    if (ipcStream_ == nullptr) {
        AUDIO_ERR_LOG("Error: ipcStream_ is not initialized!");
        return;
    }

    // if first call, call set thread priority. if thread tid change recall set thread priority
    if (needSetThreadPriority_.exchange(false)) {
        ipcStream_->RegisterThreadPriority(gettid(),
            AudioSystemManager::GetInstance()->GetSelfBundleName(clientConfig_.appInfo.appUid));
    }

    if (!hasFirstFrameWrited_.exchange(true)) { OnFirstFrameWriting(); }
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

int32_t RendererInClientInner::DrainIncompleteFrame(OptResult result, bool stopFlag,
    size_t targetSize, BufferDesc *desc, bool &dropIncompleteFrame)
{
    if (result.size < clientSpanSizeInByte_ && stopFlag) {
        result = ringCache_->Dequeue({desc->buffer, targetSize});
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
            "ringCache Dequeue failed %{public}d", result.ret);
        int32_t ret = memset_s(desc->buffer, targetSize, 0, targetSize);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "DrainIncompleteFrame memset output failed");
        AUDIO_WARNING_LOG("incomplete frame is set to 0");
        dropIncompleteFrame = true;
    }
    return SUCCESS;
}


int32_t RendererInClientInner::WriteCacheData(bool isDrain, bool stopFlag)
{
    Trace traceCache(isDrain ? "RendererInClientInner::DrainCacheData" : "RendererInClientInner::WriteCacheData");

    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERR_OPERATION_FAILED, "ring cache unreadable");
    if (result.size == 0) {
        AUDIO_WARNING_LOG("Readable size is already zero");
        return SUCCESS;
    }
    size_t targetSize = isDrain ? std::min(result.size, clientSpanSizeInByte_) : clientSpanSizeInByte_;

    int32_t sizeInFrame = clientBuffer_->GetAvailableDataFrames();
    CHECK_AND_RETURN_RET_LOG(sizeInFrame >= 0, ERROR, "GetAvailableDataFrames invalid, %{public}d", sizeInFrame);

    FutexCode futexRes = FUTEX_OPERATION_FAILED;
    if (static_cast<uint32_t>(sizeInFrame) < spanSizeInFrame_) {
        int32_t timeout = offloadEnable_ ? OFFLOAD_OPERATION_TIMEOUT_IN_MS : WRITE_CACHE_TIMEOUT_IN_MS;
        futexRes = FutexTool::FutexWait(clientBuffer_->GetFutex(), static_cast<int64_t>(timeout) * AUDIO_US_PER_SECOND,
            [this] () {
                return (state_ != RUNNING) ||
                    (static_cast<uint32_t>(clientBuffer_->GetAvailableDataFrames()) >= spanSizeInFrame_);
            });
        CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "failed with state:%{public}d", state_.load());
        CHECK_AND_RETURN_RET_LOG(futexRes != FUTEX_TIMEOUT, ERROR,
            "write data time out, mode is %{public}s", (offloadEnable_ ? "offload" : "normal"));
        sizeInFrame = clientBuffer_->GetAvailableDataFrames();
    }

    if (sizeInFrame < 0 || static_cast<uint32_t>(clientBuffer_->GetAvailableDataFrames()) < spanSizeInFrame_) {
        AUDIO_ERR_LOG("failed: sizeInFrame is:%{public}d, futexRes:%{public}d", sizeInFrame, futexRes);
        return ERROR;
    }
    BufferDesc desc = {};
    uint64_t curWriteIndex = clientBuffer_->GetCurWriteFrame();
    int32_t ret = clientBuffer_->GetWriteBuffer(curWriteIndex, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetWriteBuffer failed %{public}d", ret);
    bool dropIncompleteFrame = false;
    CHECK_AND_RETURN_RET_LOG(DrainIncompleteFrame(result, stopFlag, targetSize, &desc, dropIncompleteFrame) == SUCCESS,
        ERROR, "DrainIncompleteFrame failed");
    if (dropIncompleteFrame) {
        return SUCCESS;
    }
    result = ringCache_->Dequeue({desc.buffer, targetSize});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "ringCache Dequeue failed %{public}d", result.ret);
    if (isDrain && targetSize < clientSpanSizeInByte_ && clientConfig_.streamInfo.format == SAMPLE_U8) {
        size_t leftSize = clientSpanSizeInByte_ - targetSize;
        int32_t ret = memset_s(desc.buffer + targetSize, leftSize, 0X7F, leftSize);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "left buffer memset output failed");
    }
    if (!ProcessVolume()) {
        return ERR_OPERATION_FAILED;
    }

    DumpFileUtil::WriteDumpFile(dumpOutFd_, static_cast<void *>(desc.buffer), desc.bufLength);
    VolumeTools::DfxOperation(desc, clientConfig_.streamInfo, traceTag_, volumeDataCount_);
    clientBuffer_->SetCurWriteFrame(curWriteIndex + spanSizeInFrame_);

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "WriteCacheData failed, null ipcStream_.");
    ipcStream_->UpdatePosition(); // notiify server update position
    HandleRendererPositionChanges(desc.bufLength);
    return SUCCESS;
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
    CHECK_AND_RETURN_RET_LOG(WriteCacheData(true, stopFlag) == SUCCESS, false, "Drain cache failed");

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
} // namespace AudioStandard
} // namespace OHOS