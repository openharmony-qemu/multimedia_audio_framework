/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "OffloadAudioRenderSink"
#endif

#include "sink/offload_audio_render_sink.h"
#include <climits>
#include <future>
#include <pthread.h>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_stream_enum.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

OffloadAudioRenderSink::~OffloadAudioRenderSink()
{
    if (sinkInited_) {
        DeInit();
    }
    if (ctrlThread_.joinable()) {
        // dtor may run on the ctrl thread itself (last self-ref released there): never join self
        ctrlThread_.detach();
    }
    AUDIO_INFO_LOG("volumeDataCount: %{public}" PRId64, volumeDataCount_);
}

int32_t OffloadAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("OffloadAudioRenderSink::Init");
    attr_ = attr;
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");

    InitLatencyMeasurement();
    sinkInited_ = true;
    InitPipeInfo(hdiRenderId_, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_OUTPUT_FLAG_LOWPOWER);
    StartCtrlThread();
    return SUCCESS;
}

void OffloadAudioRenderSink::DeInit(void)
{
    AUDIO_WARNING_LOG("in");
    Trace trace("OffloadAudioRenderSink::DeInit");
    if (!ctrlRunning_.exchange(false)) {
        return; // not inited or already deinited
    }
    // reap the executor first: once joined nobody can still be inside a control call
    CtrlRequest request;
    request.cmd = CtrlCmd::CMD_SHUTDOWN;
    request.done = std::make_shared<std::promise<int32_t>>();
    auto future = request.done->get_future();
    {
        std::lock_guard<std::mutex> lock(ctrlQueueMutex_);
        ctrlQueue_.push(std::move(request));
    }
    ctrlQueueCv_.notify_one();
    if (future.wait_for(std::chrono::milliseconds(CTRL_SHUTDOWN_JOIN_MS)) != std::future_status::ready) {
        ctrlAbandoned_.store(true);
        if (ctrlThread_.joinable()) {
            ctrlThread_.detach();
        }
        // zombie executor may still be inside an HDI call (including a blocking DrainBuffer,
        // which is deliberately not joined -- see the constraint note in ExecDrain): skip
        // teardown to avoid UAF
        AUDIO_ERR_LOG("ctrl thread shutdown timeout, abandon teardown");
        return;
    }
    if (ctrlThread_.joinable()) {
        ctrlThread_.join(); // thread already finished or is about to, join is immediate
    }
    std::lock_guard<std::mutex> lock(sinkMutex_);
    std::lock_guard<std::mutex> switchDeviceLock(switchDeviceMutex_);
    sinkInited_ = false;
    started_.store(false);
    ctrlState_.store(CtrlState::CTRL_IDLE);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
    audioRender_ = nullptr;
    hdiCallback_ = {};
    muteCount_ = 0;
    switchDeviceMute_ = false;
    DeinitPipeInfo();
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool OffloadAudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

int32_t OffloadAudioRenderSink::Start(void)
{
    return PostCtrlCmd(CtrlCmd::CMD_START, 0, CTRL_WAIT_MS);
}

int32_t OffloadAudioRenderSink::ExecStart()
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("OffloadAudioRenderSink::Start");

    if (started_.load()) {
        return SUCCESS; // already running; the FLUSHING case is resolved by the state machine
    }
    if (currentActiveDevice_ != DEVICE_TYPE_NONE) {
        // no forceSetRouteFlag_ re-arm here: upstream 02e4da9df4 removed it so that Start
        // does not bypass the routing gate and overwrite the primary sink's route
        std::vector<DeviceType> devices = { currentActiveDevice_ };
        UpdateActiveDevice(devices);
    }
    AudioXCollie audioXCollie("OffloadAudioRenderSink::Start", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    UpdateSinkState(true);
    ChangePipeStatus(PIPE_STATUS_RUNNING);
    firstFrameRendered_.store(false);
    dumpFileName_ = "offload_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    started_.store(true);
    renderPos_ = 0;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Stop(void)
{
    return PostCtrlCmd(CtrlCmd::CMD_STOP, 0, CTRL_WAIT_MS);
}

int32_t OffloadAudioRenderSink::ExecStop(bool flushFirst)
{
    AUDIO_WARNING_LOG("in");
    Trace trace("OffloadAudioRenderSink::Stop");
    if (!started_.load()) {
        UnLockOffloadRunningLock();
        ctrlState_.store(CtrlState::CTRL_IDLE);
        return SUCCESS;
    }
    if (flushFirst) {
        // legacy stop flushes first; keep the gate closed through the whole stop:
        // ExecFlush publishes no state here, ExecStop owns the terminal transition
        ExecFlush(false, false);
    }
    AudioXCollie audioXCollie("OffloadAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::lock_guard<std::mutex> lock(sinkMutex_);
    if (audioRender_ == nullptr) {
        AUDIO_ERR_LOG("render is nullptr");
        ctrlState_.store(CtrlState::CTRL_STARTED); // legacy contract: started_ stays true on failure
        return ERR_INVALID_HANDLE;
    }
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    ChangePipeStatus(PIPE_STATUS_STANDBY);
    firstFrameRendered_.store(false);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("stop fail, ret: %{public}d", ret);
        ctrlState_.store(CtrlState::CTRL_STARTED); // legacy contract: started_ stays true on failure
        return ERR_NOT_STARTED;
    }
    started_.store(false);
    ctrlState_.store(CtrlState::CTRL_IDLE);
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Resume(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRenderSink::Pause(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRenderSink::PostCtrlCmd(CtrlCmd cmd, int32_t arg, int32_t waitMs)
{
    if (ctrlAbandoned_.load()) {
        AUDIO_ERR_LOG("ctrl thread abandoned, reject cmd: %{public}u", static_cast<uint32_t>(cmd));
        return ERR_ILLEGAL_STATE;
    }
    if (!ctrlRunning_.load()) {
        // sink not inited (or already deinited): run the IDLE-column handlers inline,
        // which returns exactly what the old direct-call path returned
        return HandleCtrlCmd(cmd, arg);
    }
    CtrlRequest request;
    request.cmd = cmd;
    request.arg = arg;
    if (waitMs > 0) {
        request.done = std::make_shared<std::promise<int32_t>>();
    }
    auto future = request.done == nullptr ? std::future<int32_t>() : request.done->get_future();
    {
        std::lock_guard<std::mutex> lock(ctrlQueueMutex_);
        ctrlQueue_.push(std::move(request));
    }
    ctrlQueueCv_.notify_one();
    if (waitMs <= 0) {
        return SUCCESS; // fire and forget, same contract as the old async flush
    }
    if (future.wait_for(std::chrono::milliseconds(waitMs)) != std::future_status::ready) {
        // the command stays queued and still runs; only this caller gives up waiting
        AUDIO_ERR_LOG("ctrl cmd: %{public}u timeout of %{public}d ms", static_cast<uint32_t>(cmd), waitMs);
        return ERR_OPERATION_FAILED;
    }
    return future.get();
}

void OffloadAudioRenderSink::StartCtrlThread()
{
    if (ctrlThread_.joinable()) {
        // double Init without DeInit: never move-assign over a joinable thread (terminate)
        AUDIO_WARNING_LOG("ctrl thread already running, skip restart");
        return;
    }
    ctrlRunning_.store(true);
    auto self = shared_from_this();
    ctrlThread_ = std::thread([self] { self->CtrlThreadMain(); });
    pthread_setname_np(ctrlThread_.native_handle(), "OffldSinkCtl");
}

void OffloadAudioRenderSink::CtrlThreadMain()
{
    std::unique_lock<std::mutex> lock(ctrlQueueMutex_);
    bool shutdown = false;
    while (!shutdown) {
        ctrlQueueCv_.wait(lock, [this] { return !ctrlQueue_.empty(); });
        CtrlRequest request = std::move(ctrlQueue_.front());
        ctrlQueue_.pop();
        lock.unlock();
        int32_t ret = SUCCESS;
        if (request.cmd != CtrlCmd::CMD_SHUTDOWN) {
            ret = HandleCtrlCmd(request.cmd, request.arg);
        }
        if (request.done != nullptr) {
            request.done->set_value(ret);
        }
        if (request.cmd == CtrlCmd::CMD_SHUTDOWN) {
            // exit via the loop condition with the lock already dropped (unique_lock
            // dtor is a no-op on the unlocked state), same path the nested return took
            shutdown = true;
        } else {
            lock.lock();
        }
    }
}

int32_t OffloadAudioRenderSink::HandleCtrlCmd(CtrlCmd cmd, int32_t arg)
{
    switch (cmd) {
        case CtrlCmd::CMD_START: {
            if (ctrlState_.load() == CtrlState::CTRL_FLUSHING) {
                // gate closed = a flush is executing or queued as the carrier: run this
                // start when that flush resolves (last writer wins)
                pendingAfterFlush_ = PendingAfterFlush::PENDING_START;
                return SUCCESS;
            }
            int32_t ret = ExecStart();
            ctrlState_.store(started_.load() ? CtrlState::CTRL_STARTED : CtrlState::CTRL_IDLE);
            return ret;
        }
        case CtrlCmd::CMD_STOP:
            if (ctrlState_.load() == CtrlState::CTRL_FLUSHING) {
                pendingAfterFlush_ = PendingAfterFlush::PENDING_STOP;
                return SUCCESS;
            }
            return ExecStop();
        case CtrlCmd::CMD_FLUSH: {
            if (ctrlState_.load() == CtrlState::CTRL_IDLE) {
                AUDIO_ERR_LOG("not start, invalid state");
                return ERR_OPERATION_FAILED;
            }
            // FLUSHING here means the caller pre-closed the gate and this command is the
            // flush carrier itself, so always execute; a repeat flush is idempotent at HDI
            ExecFlush(false);
            return SUCCESS;
        }
        case CtrlCmd::CMD_RESET: {
            if (ctrlState_.load() == CtrlState::CTRL_IDLE) {
                AUDIO_ERR_LOG("not start, invalid state");
                return ERR_OPERATION_FAILED;
            }
            // reset = flush then restart; never absorb into pending: this command is its
            // own flush carrier, and any earlier FLUSH in the queue has already completed
            ExecFlush(true);
            return SUCCESS;
        }
        case CtrlCmd::CMD_DRAIN:
            return ExecDrain(arg);
        default:
            return ERR_ILLEGAL_STATE;
    }
}

void OffloadAudioRenderSink::ExecFlush(bool restartAfter, bool publishStarted)
{
    Trace trace("OffloadAudioRenderSink::FlushInner");
    // publish FLUSHING before the HDI call so the lock-free RenderFrame gate closes first
    ctrlState_.store(CtrlState::CTRL_FLUSHING);
    int32_t ret = ERR_INVALID_HANDLE;
    {
        std::lock_guard<std::mutex> lock(sinkMutex_);
        if (audioRender_ == nullptr) {
            AUDIO_ERR_LOG("render is nullptr");
        } else {
            ret = audioRender_->Flush(audioRender_);
        }
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("flush fail, ret: %{public}d", ret);
    }
    renderPos_ = 0;
    ResolvePendingAfterFlush(restartAfter, publishStarted);
}

void OffloadAudioRenderSink::ResolvePendingAfterFlush(bool restartAfter, bool publishStarted)
{
    PendingAfterFlush pending = pendingAfterFlush_;
    pendingAfterFlush_ = PendingAfterFlush::PENDING_NONE;
    if (restartAfter) {
        pending = PendingAfterFlush::PENDING_START; // RESET always restarts after the flush
    }
    switch (pending) {
        case PendingAfterFlush::PENDING_START: {
            int32_t ret = ExecStart();
            ctrlState_.store(started_.load() ? CtrlState::CTRL_STARTED : CtrlState::CTRL_IDLE);
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("deferred start fail, ret: %{public}d", ret);
            }
            break;
        }
        case PendingAfterFlush::PENDING_STOP: {
            int32_t ret = ExecStop(false); // the flush above already ran, no second flush
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("deferred stop fail, ret: %{public}d", ret);
            }
            break;
        }
        default:
            if (publishStarted) {
                ctrlState_.store(CtrlState::CTRL_STARTED); // flush done, sink is still started
            }
            // else: ExecStop context keeps FLUSHING until it publishes its terminal state
            break;
    }
}

int32_t OffloadAudioRenderSink::ExecDrain(int32_t drainType)
{
    Trace trace("OffloadAudioRenderSink::Drain");
    // no sinkMutex_ here (legacy contract): DrainBuffer can block for seconds and must
    // not stall DeInit teardown; ctrl-queue serialization against flush/stop still holds.
    // constraint: if DeInit abandons the ctrl thread (shutdown timeout), a drain in flight
    // is never joined -- the zombie finishes on its own against the shared_ptr self-ref;
    // teardown paths must never add a join/wait for it (see the abandon branch in DeInit)
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->DrainBuffer(audioRender_, (AudioDrainNotifyType *)&drainType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "drain fail");
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Flush(void)
{
    // close the write gate before returning (legacy isFlushing_ contract): upstream
    // StopStream computes rewindTime from the HDI cache right after Flush() returns
    if (ctrlState_.load() == CtrlState::CTRL_STARTED) {
        ctrlState_.store(CtrlState::CTRL_FLUSHING);
    }
    return PostCtrlCmd(CtrlCmd::CMD_FLUSH, 0, 0);
}

int32_t OffloadAudioRenderSink::Reset(void)
{
    // same synchronous gate closure as Flush(): legacy Reset set isFlushing_ inline
    if (ctrlState_.load() == CtrlState::CTRL_STARTED) {
        ctrlState_.store(CtrlState::CTRL_FLUSHING);
    }
    return PostCtrlCmd(CtrlCmd::CMD_RESET, 0, CTRL_WAIT_MS);
}

int32_t OffloadAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CtrlState ctrlState = ctrlState_.load();
    CHECK_AND_RETURN_RET_LOG(ctrlState != CtrlState::CTRL_FLUSHING, ERR_OPERATION_FAILED, "during flushing");
    CHECK_AND_RETURN_RET_LOG(ctrlState == CtrlState::CTRL_STARTED, ERR_OPERATION_FAILED, "not start, invalid state");

    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }
    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }
    Trace trace("OffloadAudioRenderSink::RenderFrame");
    CheckLatencySignal(reinterpret_cast<uint8_t *>(&data), len);
    int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t *>(&data), static_cast<uint32_t>(len),
        &writeLen);
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_) {
        runningLock_->UpdateAppsUidToPowerMgr();
    }
#endif
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    firstFrameRendered_.store(true);
    if (writeLen != 0) {
        BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
        AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
            static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
        VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
        if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
            DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), writeLen);
            AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), writeLen);
        }
        CheckUpdateState(&data, len);
    }
    renderPos_ += writeLen;
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThreshold = 50; // 50ms
    if (stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::GetVolumeDataCount(int64_t &volumeData)
{
    volumeData = volumeDataCount_;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::SetVolume(float left, float right)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    Trace trace("OffloadAudioRenderSink::SetVolume");

    leftVolume_ = left;
    rightVolume_ = right;

    if (switchDeviceMute_) {
        AUDIO_WARNING_LOG("mute for switch device, store volume, left: %{public}f, right: %{public}f", left, right);
        return SUCCESS;
    }
    return SetVolumeInner(left, right);
}

int32_t OffloadAudioRenderSink::SetVolumeWithRamp(float left, float right, uint32_t durationMs)
{
    AUDIO_INFO_LOG("OffloadAudioRenderSink::SetVolumeWithRamp::set offload vol "\
        "left: %{public}f, right: %{public}f, durationMs: %{public}u", left, right, durationMs);
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    Trace trace("OffloadAudioRenderSink::SetVolume");

    leftVolume_ = left;
    rightVolume_ = right;

    if (switchDeviceMute_) {
        AUDIO_WARNING_LOG("mute for switch device, store volume, left: %{public}f, right: %{public}f", left, right);
        return SUCCESS;
    }
    return SetVolumeInner(left, right, durationMs);
}

int32_t OffloadAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::GetLatency(uint32_t &latency)
{
    Trace trace("OffloadAudioRenderSink::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    // 4: bytewidth
    uint64_t hdiLatency = renderPos_ * SECOND_TO_MICROSECOND / (AUDIO_SAMPLE_RATE_48K * 4 * STEREO_CHANNEL_COUNT);
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = GetPresentationPosition(frames, timeSec, timeNanoSec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get frames fail");
    latency = hdiLatency > frames ? (hdiLatency - frames) / MICROSECOND_PER_MILLISECOND : 0;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    Trace trace("OffloadAudioRenderSink::GetPresentationPosition");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(ctrlState_.load() != CtrlState::CTRL_FLUSHING, ERR_OPERATION_FAILED,
        "during flushing");

    uint64_t tmpFrames;
    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->GetRenderPosition(audioRender_, &tmpFrames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get render position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    frames = tmpFrames * SECOND_TO_MICROSECOND / attr_.sampleRate;
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;

    // check hdi timestamp out of range 40 * 1000 * 1000 ns
    struct timespec curStamp;
    if (clock_gettime(CLOCK_MONOTONIC, &curStamp) >= 0) {
        int64_t curNs = curStamp.tv_sec * AUDIO_NS_PER_SECOND + curStamp.tv_nsec;
        int64_t hdiNs = stamp.tvSec * AUDIO_NS_PER_SECOND + stamp.tvNSec;
        int64_t outNs = 40 * 1000 * 1000; // 40 * 1000 * 1000 ns
        if (curNs <= hdiNs || curNs > hdiNs + outNs) {
            AUDIO_PRERELEASE_LOGW("HDI time is not in the range, hdi: %{public}" PRId64 ", cur: %{public}" PRId64,
                hdiNs, curNs);
            timeSec = curStamp.tv_sec;
            timeNanoSec = curStamp.tv_nsec;
        }
    }
    return ret;
}

float OffloadAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void OffloadAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void OffloadAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    // reset the balance coefficient value firstly
    leftBalanceCoef_ = 1.0f;
    rightBalanceCoef_ = 1.0f;

    if (std::abs(audioBalance - 0.0f) <= std::numeric_limits<float>::epsilon()) {
        // audioBalance is equal to 0.0f
        audioBalanceState_ = false;
    } else {
        // audioBalance is not equal to 0.0f
        audioBalanceState_ = true;
        // calculate the balance coefficient
        if (audioBalance > 0.0f) {
            leftBalanceCoef_ -= audioBalance;
        } else if (audioBalance < 0.0f) {
            rightBalanceCoef_ += audioBalance;
        }
    }
}

int32_t OffloadAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set offload mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("offload already muted");
            return SUCCESS;
        }
        int32_t ret = SetVolumeInner(0.0f, 0.0f);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "set offload mute fail");
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("offload not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
        SetVolumeInner(leftVolume_, rightVolume_);
    }

    return SUCCESS;
}

int32_t OffloadAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
#endif
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Drain(AudioDrainType type)
{
    return PostCtrlCmd(CtrlCmd::CMD_DRAIN, static_cast<int32_t>(type), CTRL_WAIT_MS);
}

void OffloadAudioRenderSink::RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback)
{
    AUDIO_INFO_LOG("in");

    hdiCallback_ = {
        .callback_.RenderCallback = &OffloadAudioRenderSink::OffloadRenderCallback,
        .serviceCallback_ = callback,
        .sink_ = this,
    };
    int32_t ret = audioRender_->RegCallback(audioRender_, &hdiCallback_.callback_, (int8_t)0);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("fail, error code: %{public}d", ret);
    }
}

int32_t OffloadAudioRenderSink::SetBufferSize(uint32_t sizeMs)
{
    Trace trace("OffloadAudioRenderSink::SetBufferSize");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(ctrlState_.load() != CtrlState::CTRL_FLUSHING, ERR_OPERATION_FAILED,
        "during flushing");

    // 4: bytewidth
    uint32_t size = (uint64_t) sizeMs * AUDIO_SAMPLE_RATE_48K * 4 * STEREO_CHANNEL_COUNT / SECOND_TO_MILLISECOND;
    AUDIO_INFO_LOG("SetBufferSize, size:%{public}u, sizeMs:%{public}u", size, sizeMs);
    int32_t ret = audioRender_->SetBufferSize(audioRender_, size);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set buffer size fail");
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::LockOffloadRunningLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        guard.CheckCurrTimeout();
    }
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, ERR_OPERATION_FAILED,
        "running lock is null, playback can not work well");
    CHECK_AND_RETURN_RET(!runningLocked_, SUCCESS);
    AUDIO_INFO_LOG("in");
    runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    runningLocked_ = true;
#endif
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::UnLockOffloadRunningLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, ERR_OPERATION_FAILED,
        "running lock is null, playback can not work well");
    CHECK_AND_RETURN_RET(runningLocked_, SUCCESS);
    AUDIO_INFO_LOG("in");
    runningLock_->UnLock();
    runningLocked_ = false;
#endif
    return SUCCESS;
}

void OffloadAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: OffloadSink\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

uint32_t OffloadAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    AudioFormat hdiFormat = ConvertToHdiFormat(format);
    switch (hdiFormat) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat OffloadAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        case SAMPLE_F32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_FLOAT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

int32_t OffloadAudioRenderSink::OffloadRenderCallback(struct IAudioCallback *self, enum AudioCallbackType type,
    int8_t *reserved, int8_t *cookie)
{
    (void)reserved;
    (void)cookie;
    auto *impl = reinterpret_cast<struct OffloadHdiCallback *>(self);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, ERR_OPERATION_FAILED, "impl is nullptr");
    auto *sink = reinterpret_cast<OffloadAudioRenderSink *>(impl->sink_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_OPERATION_FAILED, "sink is nullptr");
    if (sink->ctrlState_.load() != CtrlState::CTRL_STARTED) {
        AUDIO_DEBUG_LOG("invalid call, ctrl state: %{public}u",
            static_cast<uint32_t>(sink->ctrlState_.load()));
        return SUCCESS;
    }

    impl->serviceCallback_(static_cast<RenderCallbackType>(type));
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::SetOffloadRenderCallbackType(RenderCallbackType type)
{
    AUDIO_INFO_LOG("SetOffloadRenderCallbackType type: %{public}d", type);
    if (hdiCallback_.serviceCallback_ == nullptr) {
        AUDIO_ERR_LOG("SetOffloadRenderCallbackType invalid, callback is nullptr");
        return ERR_ILLEGAL_STATE;
    }

    if (ctrlState_.load() != CtrlState::CTRL_STARTED) {
        AUDIO_INFO_LOG("SetOffloadRenderCallbackType invalid, ctrl state: %{public}u",
            static_cast<uint32_t>(ctrlState_.load()));
        return ERR_ILLEGAL_STATE;
    }

    hdiCallback_.serviceCallback_(type);
    return SUCCESS;
}

void OffloadAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_OFFLOAD));
    param.type = AUDIO_OFFLOAD;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    // offload attr
    param.offloadInfo.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.offloadInfo.channelCount = AUDIO_CHANNELCOUNT;
    param.offloadInfo.bitRate = AUDIO_SAMPLE_RATE_48K * BIT_IN_BYTE;
    param.offloadInfo.bitWidth = PCM_32_BIT;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    if (param.channelCount == MONO) {
        param.channelLayout = CH_LAYOUT_MONO;
    } else if (param.channelCount == STEREO) {
        param.channelLayout = CH_LAYOUT_STEREO;
    }
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }

    param.offloadInfo.format = ConvertToHdiFormat(attr_.format);
}

void OffloadAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = const_cast<char *>("");
}

int32_t OffloadAudioRenderSink::CreateRender(void)
{
    Trace trace("AudioRenderSink::CreateRender");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);
    AudioXCollie audioXCollie("OffloadAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(attr_.adapterName, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);

    return SUCCESS;
}

void OffloadAudioRenderSink::InitLatencyMeasurement(void)
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    AUDIO_INFO_LOG("in");
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    signalDetected_ = false;
}

void OffloadAudioRenderSink::CheckLatencySignal(uint8_t *data, size_t len)
{
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    uint32_t byteSize = static_cast<uint32_t>(GetFormatByteSize(attr_.format));
    size_t newlyCheckedTime = len / (attr_.sampleRate / MILLISECOND_PER_SECOND) /
        (byteSize * sizeof(uint8_t) * attr_.channel);
    signalDetectedTime_ += newlyCheckedTime;
    if (signalDetectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN(deviceManager != nullptr);
        std::string value = deviceManager->GetAudioParameter(attr_.adapterName, key, condition);

        LatencyMonitor::GetInstance().UpdateDspTime(value.c_str());
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true, signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().ShowTimestamp(true);
        signalDetectAgent_->dspTimestampGot_ = true;
        signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("signal detected");
        signalDetectedTime_ = 0;
    }
}

void OffloadAudioRenderSink::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        case SAMPLE_S16LE:
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        case SAMPLE_S24LE:
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<uint8_t *>(data), len);
            break;
        case SAMPLE_S32LE:
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void OffloadAudioRenderSink::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S16LE:
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S24LE:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S32LE:
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void OffloadAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), data, len);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
            }
        }
    }
}

int32_t OffloadAudioRenderSink::SetVolumeInner(float left, float right, uint32_t durationMs)
{
    AudioXCollie audioXCollie("OffloadAudioRenderSink::SetVolumeInner", TIMEOUT_SECONDS_10, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    AUDIO_INFO_LOG("set offload vol, left: %{public}f, right: %{public}f, durationMs: %{public}u",
        left, right, durationMs);

    CHECK_AND_RETURN_RET_LOG(ctrlState_.load() != CtrlState::CTRL_FLUSHING, ERR_OPERATION_FAILED,
        "during flushing");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE,
        "render is nullptr, because set volume on device which offload is not available");

    float volume;
    if ((left == 0) && (right != 0)) {
        volume = right;
    } else if ((left != 0) && (right == 0)) {
        volume = left;
    } else {
        volume = (left + right) / HALF_FACTOR;
    }

    int32_t ret = SUCCESS;
    if (durationMs == 0) {
        CHECK_AND_RETURN_RET_LOG(NeedToSetOffloadVolume(volume), ret, "No need to set volume. Return SUCCESS.");
        ret = audioRender_->SetVolume(audioRender_, volume);
    } else {
        ret = audioRender_->SetVolumeWithRamp(audioRender_, volume, durationMs);
    }
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }
    offloadVolume_ = volume;
    durationMs_ = durationMs;
    setVolumeTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    return ret;
}

bool OffloadAudioRenderSink::NeedToSetOffloadVolume(const float newVolume)
{
    if (!FLOAT_COMPARE_EQ(newVolume, offloadVolume_)) {
        AUDIO_INFO_LOG("newVolume: %{public}f, oldVolume: %{public}f. Not equal.", newVolume, offloadVolume_);
        return true;
    }
    if (durationMs_ == 0) {
        AUDIO_INFO_LOG("durationMs_ is zero.");
        return true;
    }
    std::chrono::milliseconds currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    if (currentTime - setVolumeTime_ > std::chrono::milliseconds(durationMs_)) {
        AUDIO_INFO_LOG("The volume fade time has ended.");
        return true;
    }
    AUDIO_INFO_LOG("The volume fade time has not yet ended, and the volume has not changed.");
    return false;
}

// must be called with sinkMutex_ held
void OffloadAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_OFFLOAD), started);
}

void OffloadAudioRenderSink::SetSpeed(float speed)
{
    CHECK_AND_RETURN_LOG(speed <= 3.0f && speed >= 0.5f, "speed is invalid"); // 3.0 upper, 0.5 lower
    uint32_t hdiSpeed = static_cast<uint32_t>(speed * AUDIO_SPEED_BASE);
    AUDIO_INFO_LOG("speed: %{public}f, hdi speed: %{public}u", speed, hdiSpeed);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is nullptr");
    std::string parameters = "pcm_offload_play_speed=" + std::to_string(hdiSpeed) + ";";
    deviceManager->SetAudioParameter(attr_.adapterName, NONE, "pcm_offload_play_speed", parameters);
}

int32_t OffloadAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() == 1, ERR_INVALID_PARAM, "invalid device");
    bool forceSet = forceSetRouteFlag_;
    if (currentActiveDevice_ == outputDevices[0] && !forceSet) {
        return SUCCESS;
    }
    forceSetRouteFlag_ = false;
    if (currentActiveDevice_ != outputDevices[0]) {
        AUDIO_INFO_LOG("set output device from %{public}d to %{public}d", currentActiveDevice_, outputDevices[0]);
    }
    currentActiveDevice_ = outputDevices[0];
    if (!Util::IsEnhancedRoutingSupported() && !forceSet) {
        return SUCCESS;
    }
    ChangePipeDevice(outputDevices);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t ret = deviceManager->SetOutputRoute(attr_.adapterName, outputDevices,
        GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_OFFLOAD));
    return ret;
}

DeviceType OffloadAudioRenderSink::GetCurrentOutputDevice()
{
    return currentActiveDevice_;
}

bool OffloadAudioRenderSink::IsInA2dpOffload()
{
    return currentActiveDevice_ == DEVICE_TYPE_BLUETOOTH_A2DP;
}
} // namespace AudioStandard
} // namespace OHOS
