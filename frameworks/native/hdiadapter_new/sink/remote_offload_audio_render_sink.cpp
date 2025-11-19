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
#define LOG_TAG "RemoteOffloadAudioRenderSink"
#endif

#include "sink/remote_offload_audio_render_sink.h"
#include <climits>
#include <future>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
RemoteOffloadHdiCallbackImpl::RemoteOffloadHdiCallbackImpl(RemoteOffloadAudioRenderSink *sink)
    : sink_(sink)
{
}

int32_t RemoteOffloadHdiCallbackImpl::RenderCallback(AudioCallbackType type, int8_t &reserved, int8_t &cookie)
{
    (void)reserved;
    (void)cookie;
    CHECK_AND_RETURN_RET_LOG(sink_ != nullptr, ERR_OPERATION_FAILED, "sink is nullptr");
    if (!sink_->started_.load() || sink_->isFlushing_.load()) {
        AUDIO_DEBUG_LOG("invalid call, started: %{public}d, isFlushing: %{public}d", sink_->started_.load(),
            sink_->isFlushing_.load());
        return SUCCESS;
    }
    sink_->hdiCallback_.serviceCallback_(static_cast<RenderCallbackType>(type));
    return SUCCESS;
}

int32_t RemoteOffloadHdiCallbackImpl::ParamCallback(AudioExtParamKey key, const std::string &condition,
    const std::string &value, int8_t &reserved, int8_t cookie)
{
    (void)key;
    (void)condition;
    (void)value;
    (void)reserved;
    (void)cookie;
    return SUCCESS;
}

RemoteOffloadAudioRenderSink::RemoteOffloadAudioRenderSink(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_DEBUG_LOG("construction");
}

RemoteOffloadAudioRenderSink::~RemoteOffloadAudioRenderSink()
{
    if (sinkInited_.load()) {
        DeInit();
    }
    CheckFlushThread();
    AUDIO_INFO_LOG("volumeDataCount: %{public}" PRId64, volumeDataCount_);
}

int32_t RemoteOffloadAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteOffloadAudioRenderSink::Init");
    attr_ = attr;
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");

    renderInited_.store(true);
    sinkInited_.store(true);
    return SUCCESS;
}

void RemoteOffloadAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteOffloadAudioRenderSink::DeInit");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    std::lock_guard<std::mutex> switchDeviceLock(switchDeviceMutex_);
    sinkInited_.store(false);
    renderInited_.store(false);
    started_.store(false);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyRender(deviceNetworkId_, hdiRenderId_);
    deviceManager->UnRegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_);
    audioRender_.ForceSetRefPtr(nullptr);
    hdiCallback_ = {};
    muteCount_ = 0;
    switchDeviceMute_ = false;
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool RemoteOffloadAudioRenderSink::IsInited(void)
{
    return sinkInited_.load();
}

int32_t RemoteOffloadAudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteOffloadAudioRenderSink::Start");
    if (!renderInited_.load()) {
        int32_t ret = CreateRender();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
        renderInited_.store(true);
    }
    InitLatencyMeasurement();

    if (started_.load()) {
        if (isFlushing_.load()) {
            isNeedRestart_ = true;
            AUDIO_ERR_LOG("start fail, will restart after flush");
            return ERR_OPERATION_FAILED;
        }
        return SUCCESS;
    }
    AudioXCollie audioXCollie("RemoteOffloadAudioRenderSink::Start", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    UpdateSinkState(true);
    dumpFileName_ = "remote_offload_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    started_.store(true);
    renderPos_ = 0;
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteOffloadAudioRenderSink::Stop");
    DeInitLatencyMeasurement();

    if (!started_.load()) {
        UnLockOffloadRunningLock();
        return SUCCESS;
    }
    int32_t ret = FlushInner();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    AudioXCollie audioXCollie("RemoteOffloadAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    ret = audioRender_->Stop();
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_.store(false);

    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::Resume(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("already resumed");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "resume fail, ret: %{public}d", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (paused_.load()) {
        AUDIO_INFO_LOG("already paused");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "pause fail, ret: %{public}d", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::FlushInner(void)
{
    Trace trace("RemoteOffloadAudioRenderSink::FlushInner");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_.load(), ERR_OPERATION_FAILED, "duplicate flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");

    CheckFlushThread();
    isFlushing_.store(true);
    flushThread_ = std::make_shared<std::thread>([&] {
        auto future = async(std::launch::async, [&] {
            std::lock_guard<std::mutex> lock(sinkMutex_);
            CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
            return audioRender_->Flush();
        });
        if (future.wait_for(std::chrono::milliseconds(250)) == std::future_status::timeout) { // 250: max wait 250ms
            AUDIO_ERR_LOG("flush fail, timeout of 250ms");
        } else {
            int32_t ret = future.get();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("flush fail, ret: %{public}d", ret);
            }
        }
        isFlushing_.store(false);
        if (isNeedRestart_) {
            isNeedRestart_ = false;
            Start();
        }
    });
    FlushResetPosition();
    renderPos_ = 0;
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::Flush(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteOffloadAudioRenderSink::Flush");
    return FlushInner();
}

void RemoteOffloadAudioRenderSink::CheckFlushThread()
{
    if (flushThread_ != nullptr && flushThread_->joinable()) {
        flushThread_->join();
    }
    flushThread_.reset();
}

int32_t RemoteOffloadAudioRenderSink::Reset(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteOffloadAudioRenderSink::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");

    isNeedRestart_ = true;
    int32_t ret = FlushInner();
    if (ret != SUCCESS) {
        isNeedRestart_ = false;
        AUDIO_ERR_LOG("reset fail");
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_.load(), ERR_OPERATION_FAILED, "during flushing");

    Trace trace("RemoteOffloadAudioRenderSink::RenderFrame");
    CheckLatencySignal(reinterpret_cast<uint8_t *>(&data), len);
    std::vector<int8_t> bufferVec(len);
    int32_t ret = memcpy_s(bufferVec.data(), len, &data, len);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "copy fail, error code: %{public}d", ret);

    ret = audioRender_->RenderFrame(bufferVec, writeLen);
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_) {
        runningLock_->UpdateAppsUidToPowerMgr();
    }
#endif
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    if (writeLen != 0) {
        BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
        AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
            static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
        VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_, OFFLOAD_DFX_SPLIT);
        if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
            DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), writeLen);
            AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), writeLen);
        }
        CheckUpdateState(&data, len);
    }
    renderPos_ += writeLen;
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThresholdMs = 50; // 50ms
    if (stamp >= stampThresholdMs) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

int64_t RemoteOffloadAudioRenderSink::GetVolumeDataCount(void)
{
    return volumeDataCount_;
}

int32_t RemoteOffloadAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void RemoteOffloadAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    int32_t ret = audioRender_->SetExtraParams(value.c_str());
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "set parameter fail, error code: %{public}d", ret);
}

std::string RemoteOffloadAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t RemoteOffloadAudioRenderSink::SetVolume(float left, float right)
{
    std::lock_guard<std::mutex> sinkLock(sinkMutex_);
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    Trace trace("RemoteOffloadAudioRenderSink::SetVolume");

    leftVolume_ = left;
    rightVolume_ = right;

    if (switchDeviceMute_) {
        AUDIO_WARNING_LOG("mute for switch device, store volume, left: %{public}f, right: %{public}f", left, right);
        return SUCCESS;
    }
    return SetVolumeInner(left, right);
}

int32_t RemoteOffloadAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::GetHdiLatency(uint32_t &latency)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteOffloadAudioRenderSink::GetLatency");

    if (hdiLatencyUS_ == 0) {
        int32_t result = GetLatencyInner();
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "GetLatencyInner failed");
    }

    latency = hdiLatencyUS_;
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::GetHdiLatency hdiLatencyUS_ %{public}" PRIu64, hdiLatencyUS_);
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::GetLatencyInner()
{
    Trace trace("RemoteOffloadAudioRenderSink::GetLatencyInner");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    uint32_t hdiLatencyMS;
    int32_t ret = audioRender_->GetLatency(hdiLatencyMS);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail, ret: %{public}d", ret);

    hdiLatencyUS_ = hdiLatencyMS * MICROSECOND_PER_MILLISECOND;
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::GetLatency(uint32_t &latency)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);

    // calc sample count (after scaling to one times speed) first
    int32_t ret = SUCCESS;

    ret = lastHdiOriginFramesUS_ > 0 ? EstimateRenderPosition() : GetRenderPositionInner();
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::GetOriginLatency lastHdiOriginFramesUS_: %{public}" PRIu64,
        lastHdiOriginFramesUS_);

    // then get origin latency, because when calc sample count, record latency Deque
    if (hdiLatencyUS_ == 0) {
        int32_t result = GetLatencyInner();
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "GetLatencyInner failed");
    }

    uint32_t realLatencyUS = 0;
    uint32_t originLatencyUS = 0;
    for (const auto &rl : realLatencyDeque_) {
        if (realLatencyUS + rl.first <= hdiLatencyUS_) {
            realLatencyUS += rl.first;
            originLatencyUS += rl.first * rl.second;
        } else {
            uint32_t remainingLength = hdiLatencyUS_ - realLatencyUS;
            realLatencyUS += remainingLength;
            originLatencyUS += remainingLength * rl.second;
            break;
        }
    }
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::GetLatency originLatencyUS: %{public}u", originLatencyUS);

    uint64_t positionUS = lastHdiOriginFramesUS_ > originLatencyUS ? lastHdiOriginFramesUS_ - originLatencyUS : 0;
    uint64_t renderFrameUS = renderPos_ * SECOND_TO_MICROSECOND /
        (attr_.sampleRate * static_cast<uint32_t>(GetFormatByteSize(attr_.format)) * attr_.channel);
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::GetOriginLatency renderFrameUS: %{public}" PRIu64, renderFrameUS);
    latency = renderFrameUS > positionUS ? (renderFrameUS - positionUS) / MICROSECOND_PER_MILLISECOND : 0;
    return ret;
}

int32_t RemoteOffloadAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::GetHdiPresentationPosition(uint64_t &frames, int64_t &timeSec,
    int64_t &timeNanoSec)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);

    // If the sample data is not zero, it means that after obtaining the sample count,
    // the latest sample data can be inferred.
    int32_t ret = lastHdiFramesUS_ > 0 ? EstimateRenderPosition() : GetRenderPositionInner();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "RemoteOffloadAudioRenderSink::GetHdiPresentationPosition fail, ret: %{public}d", ret);
    frames = lastHdiFramesUS_;
    timeSec = lastHdiTimeSec_;
    timeNanoSec = lastHdiTimeNanoSec_;
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::ForceRefreshPresentationPosition(uint64_t &frames, uint64_t &hdiFrames,
    int64_t &timeSec, int64_t &timeNanoSec)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);

    int32_t ret = GetRenderPositionInner();
    AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::ForceRefreshPresentationPosition ret: %{public}d", ret);
    frames = lastHdiOriginFramesUS_;
    hdiFrames = lastHdiFramesUS_;
    timeSec = lastHdiTimeSec_;
    timeNanoSec = lastHdiTimeNanoSec_;
    return ret;
}

int32_t RemoteOffloadAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::EstimateRenderPosition()
{
    // The sample count can be inferred based on the time interval
    // between the current time and the last occurrence.
    int64_t now = ClockTime::GetCurNano();
    int64_t durationNS = now > lastSystemTimeNS_ ? now - lastSystemTimeNS_ : 0;
    uint64_t durationUS = static_cast<uint64_t>(durationNS) / NANOSECOND_TO_MICROSECOND;

    // The underlying time recorded must be calculated because this time increments at a fixed rate
    // regardless of whether it is played or not, or whether there is buffering or not
    lastHdiTimeNS_ += durationNS;
    lastHdiTimeSec_ = lastHdiTimeNS_ / AUDIO_NS_PER_SECOND;
    lastHdiTimeNanoSec_ = lastHdiTimeNS_ % AUDIO_NS_PER_SECOND;
    // The system time also needs to be updated because it is fixed as well
    lastSystemTimeNS_ = now;

    uint64_t renderFrameUS = renderPos_ * SECOND_TO_MICROSECOND /
        (attr_.sampleRate * static_cast<uint32_t>(GetFormatByteSize(attr_.format)) * attr_.channel);
    if (renderFrameUS <= lastHdiOriginFramesUS_) {
        AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::EstimateRenderPosition renderFrameUS: %{public}" PRIu64
            ", lastHdiFramesUS_: %{public}" PRIu64 " no need to estimate", renderFrameUS, lastHdiOriginFramesUS_);
        return SUCCESS;
    }
    
    uint64_t originFrameUS = lastHdiOriginFramesUS_ + durationUS * speed_;
    if (originFrameUS > renderFrameUS) {
        AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::EstimateRenderPosition renderFrameUS: %{public}" PRIu64
            ", originFrameUS: %{public}" PRIu64 " set to renderFrameUS", renderFrameUS, originFrameUS);
        auto excess = (renderFrameUS - lastHdiOriginFramesUS_) / speed_;
        lastHdiOriginFramesUS_ = renderFrameUS;
        lastHdiFramesUS_ += excess;

        AddHdiLatency(excess);
        return SUCCESS;
    }

    lastHdiFramesUS_ += durationUS;
    lastHdiOriginFramesUS_ += durationUS * speed_;

    AddHdiLatency(durationUS);
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::GetRenderPositionInner()
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_.load(), ERR_OPERATION_FAILED, "during flushing");

    uint64_t tmpFrames;
    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->GetRenderPosition(tmpFrames, stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get render position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    CHECK_AND_RETURN_RET_LOG(tmpFrames <= UINT64_MAX / SECOND_TO_MICROSECOND, ERR_OPERATION_FAILED,
        "frames overflow, tmpFrames: %{public}" PRIu64, tmpFrames);
    CHECK_AND_RETURN_RET_LOG(attr_.sampleRate != 0, ERR_OPERATION_FAILED, "invalid sample rate");
    uint64_t frames = tmpFrames * SECOND_TO_MICROSECOND / attr_.sampleRate;
    int64_t timeSec = stamp.tvSec;
    int64_t timeNanoSec = stamp.tvNSec;
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::GetRenderPositionInner framesUS: [%{public}" PRIu64
        "],[tmpFrames: %{public}" PRIu64 "]", frames, tmpFrames);

    CheckHdiTime(timeSec, timeNanoSec);
    CalcHdiPosition(frames, timeSec, timeNanoSec);
    return ret;
}

void RemoteOffloadAudioRenderSink::CheckHdiTime(int64_t &timeSec, int64_t &timeNanoSec)
{
    // check hdi timestamp out of range 40 * 1000 * 1000 ns
    struct timespec curStamp;
    if (clock_gettime(CLOCK_MONOTONIC, &curStamp) >= 0) {
        int64_t curNs = curStamp.tv_sec * AUDIO_NS_PER_SECOND + curStamp.tv_nsec;
        int64_t hdiNs = timeSec * AUDIO_NS_PER_SECOND + timeNanoSec;
        int64_t outNs = 40 * 1000 * 1000; // 40 * 1000 * 1000 ns
        if (curNs <= hdiNs || curNs > hdiNs + outNs) {
            AUDIO_PRERELEASE_LOGW("HDI time is not in the range, hdi: %{public}" PRId64 ", cur: %{public}" PRId64,
                hdiNs, curNs);
            timeSec = curStamp.tv_sec;
            timeNanoSec = curStamp.tv_nsec;
        }
    }
}

void RemoteOffloadAudioRenderSink::FlushResetPosition()
{
    // If the previous sampling time is null, it indicates that a reset has already been performed; return directly.
    if (lastHdiOriginFramesUS_ == 0) {
        return;
    }

    // Calculate the latest sampling time during flush operations
    // += previous sampling time + (current system time - previous system sampling time) * rate / 1000
    // after scaling to one times speed
    lastHdiOriginFlushFramesUS_ += lastHdiOriginFramesUS_ +
        (ClockTime::GetCurNano() - lastSystemTimeNS_) * speed_ / NANOSECOND_TO_MICROSECOND;
    // before scaling to one times speed
    lastHdiFlushFramesUS_ += lastHdiFramesUS_ +
        (ClockTime::GetCurNano() - lastSystemTimeNS_) / NANOSECOND_TO_MICROSECOND;
    AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::FlushResetPosition lastHdiOriginFlushFramesUS_: [%{public}" PRIu64
        "], lastHdiFlushFramesUS_: [%{public}" PRIu64 "]", lastHdiOriginFlushFramesUS_, lastHdiFlushFramesUS_);

    lastHdiFramesUS_ = 0;
    lastHdiOriginFramesUS_ = 0;
    realLatencyDeque_.clear();
    realLatencyTotalUS_ = 0;
}

void RemoteOffloadAudioRenderSink::CalcHdiPosition(uint64_t frames, int64_t timeSec, int64_t timeNanoSec)
{
    // If the current sampling time is earlier than the previous inferred sampling time.
    // It indicates that the inference has an error and needs correction.
    if (frames < lastHdiOriginFramesUS_) {
        auto duration = (lastHdiOriginFramesUS_ - frames) / speed_;
        lastHdiFramesUS_ = lastHdiFramesUS_ > duration ? lastHdiFramesUS_ - duration : 0;
        RemoveHdiLatency(duration);
    } else {
        auto duration = (frames - lastHdiOriginFramesUS_) / speed_;
        lastHdiFramesUS_ += duration;
        AddHdiLatency(duration);
    }

    lastHdiOriginFramesUS_ = frames;
    lastSystemTimeNS_ = ClockTime::GetCurNano();
    lastHdiTimeNS_ = timeNanoSec + timeSec * AUDIO_NS_PER_SECOND;
    lastHdiTimeSec_ = timeSec;
    lastHdiTimeNanoSec_ = timeNanoSec;
}

void RemoteOffloadAudioRenderSink::RemoveHdiLatency(uint64_t duration)
{
    AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::RemoveHdiLatency duration: %{public}" PRId64, duration);
    if (realLatencyTotalUS_ <= duration) {
        realLatencyTotalUS_ = 0;
        realLatencyDeque_.clear();
        return;
    }

    while (duration > 0) {
        if (duration >= realLatencyDeque_.front().first) {
            duration -= realLatencyDeque_.front().first;
            realLatencyTotalUS_ -= realLatencyDeque_.front().first;
            realLatencyDeque_.pop_front();
        } else {
            realLatencyDeque_.front().first -= duration;
            realLatencyTotalUS_ -= duration;
            duration = 0;
        }
    }
    AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::RemoveHdiLatency realLatencyTotalUS_: %{public}" PRId64,
        realLatencyTotalUS_);
}

void RemoteOffloadAudioRenderSink::AddHdiLatency(uint64_t duration)
{
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::AddHdiLatency duration: %{public}" PRId64
        ", speed: %{public}f", duration, speed_);
    realLatencyTotalUS_ += duration;
    realLatencyDeque_.push_front({duration, speed_});

    uint64_t maxDequeLengthUS = 1000000;
    // If the total length exceeds 1,000,000 microseconds.
    // clear the end of the queue to ensure the maximum length remains at 1,000,000 microseconds.
    while (realLatencyTotalUS_ > maxDequeLengthUS) {
        if (realLatencyTotalUS_ - realLatencyDeque_.back().first >= maxDequeLengthUS) {
            realLatencyTotalUS_ -= realLatencyDeque_.back().first;
            realLatencyDeque_.pop_back();
        } else {
            uint64_t excess = realLatencyTotalUS_ - maxDequeLengthUS;
            realLatencyDeque_.back().first -= excess;
            realLatencyTotalUS_ -= excess;
        }
    }
    AUDIO_DEBUG_LOG("RemoteOffloadAudioRenderSink::AddHdiLatency realLatencyTotalUS_: %{public}" PRId64,
        realLatencyTotalUS_);
}

float RemoteOffloadAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void RemoteOffloadAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteOffloadAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteOffloadAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> sinkLock(sinkMutex_);
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set offload mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("offload already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
        SetVolumeInner(0.0f, 0.0f);
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

void RemoteOffloadAudioRenderSink::SetSpeed(float speed)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");

    AUDIO_INFO_LOG("speed: %{public}f", speed);
    int32_t ret = audioRender_->SetRenderSpeed(speed);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "set speed fail, ret: %{public}d", ret);

    // Every time the speed is set, the sampling count needs to be refreshed for two reasons:
    // 1. For easier calculation, avoiding the need to consider multiple segments with different speeds
    // 2. When setting the speed, the underlying system will wake up, eliminating concerns about power consumption
    ret = GetRenderPositionInner();
    AUDIO_INFO_LOG("RemoteOffloadAudioRenderSink::SetSpeed ret: %{public}d", ret);

    speed_ = speed;
}

int32_t RemoteOffloadAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void RemoteOffloadAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void RemoteOffloadAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteOffloadAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteOffloadAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
#endif
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::Drain(AudioDrainType type)
{
    Trace trace("RemoteOffloadAudioRenderSink::Drain");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    auto drainType = static_cast<AudioDrainNotifyType>(type);
    int32_t ret = audioRender_->DrainBuffer(drainType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "drain fail");
    return SUCCESS;
}

void RemoteOffloadAudioRenderSink::RegistOffloadHdiCallback(std::function<void(const RenderCallbackType type)> callback)
{
    CHECK_AND_RETURN_LOG(hdiCallback_.serviceCallback_ == nullptr, "already registed");
    AUDIO_INFO_LOG("in");

    hdiCallback_ = {
        .callback_ = new RemoteOffloadHdiCallbackImpl(this),
        .serviceCallback_ = callback,
    };
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    int32_t ret = audioRender_->RegCallback(hdiCallback_.callback_, (int8_t)0);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("fail, error code: %{public}d", ret);
    }
}

int32_t RemoteOffloadAudioRenderSink::SetBufferSize(uint32_t sizeMs)
{
    Trace trace("RemoteOffloadAudioRenderSink::SetBufferSize");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_.load(), ERR_OPERATION_FAILED, "during flushing");

    // 4: bytewidth
    uint32_t size = (uint64_t) sizeMs * attr_.sampleRate * 4 * STEREO_CHANNEL_COUNT / SECOND_TO_MILLISECOND;
    AUDIO_INFO_LOG("size: %{public}u, sizeMs: %{public}u", size, sizeMs);
    AudioMmapBufferDescriptor desc;
    int32_t ret = audioRender_->ReqMmapBuffer(size, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set buffer size fail");
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::SetOffloadRenderCallbackType(RenderCallbackType type)
{
    AUDIO_INFO_LOG("type: %{public}d", type);
    CHECK_AND_RETURN_RET_LOG(hdiCallback_.serviceCallback_ != nullptr, ERR_ILLEGAL_STATE, "callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_.load() && !isFlushing_.load(), ERR_ILLEGAL_STATE,
        "not start or flushing, invalid state");

    hdiCallback_.serviceCallback_(type);
    return SUCCESS;
}

int32_t RemoteOffloadAudioRenderSink::LockOffloadRunningLock(void)
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

int32_t RemoteOffloadAudioRenderSink::UnLockOffloadRunningLock(void)
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

void RemoteOffloadAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteOffloadSink\tstarted: " + std::string(started_.load() ? "true" : "false") + "\n";
}

void RemoteOffloadAudioRenderSink::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnRenderSinkParamChange(adapterName, key, condition, value);
}

void RemoteOffloadAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

uint32_t RemoteOffloadAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
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

AudioFormat RemoteOffloadAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

void RemoteOffloadAudioRenderSink::InitAudioSampleAttr(AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = 0;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_OFFLOAD));
    param.type = AUDIO_OFFLOAD;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void RemoteOffloadAudioRenderSink::InitDeviceDesc(AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = const_cast<char *>("");
}

int32_t RemoteOffloadAudioRenderSink::CreateRender(void)
{
    Trace trace("RemoteOffloadAudioRenderSink::CreateRender");

    AudioSampleAttributes param;
    AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);
    AudioXCollie audioXCollie("RemoteOffloadAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(deviceNetworkId_, &param, &deviceDesc, hdiRenderId_);
    audioRender_.ForceSetRefPtr(static_cast<IAudioRender *>(render));
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    std::stringstream val;
    val << "offloadParams=" << attr_.sampleRate << "," << std::to_string(attr_.format) << ","
        << attr_.channel << "," << attr_.channelLayout;
    SetAudioParameter(AudioParamKey::NONE, "", val.str());
    deviceManager->RegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_, this);
    return SUCCESS;
}

void RemoteOffloadAudioRenderSink::InitLatencyMeasurement(void)
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

void RemoteOffloadAudioRenderSink::DeInitLatencyMeasurement(void)
{
    signalDetectAgent_ = nullptr;
}

void RemoteOffloadAudioRenderSink::CheckLatencySignal(uint8_t *data, size_t len)
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
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
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

void RemoteOffloadAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
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

int32_t RemoteOffloadAudioRenderSink::SetVolumeInner(float left, float right)
{
    AudioXCollie audioXCollie("RemoteOffloadAudioRenderSink::SetVolumeInner", TIMEOUT_SECONDS_10, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    AUDIO_INFO_LOG("set offload vol, left: %{public}f, right: %{public}f", left, right);

    CHECK_AND_RETURN_RET_LOG(!isFlushing_.load(), ERR_OPERATION_FAILED, "during flushing");
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

    int32_t ret = audioRender_->SetVolume(volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

// must be called with sinkMutex_ held
void RemoteOffloadAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_OFFLOAD),
        started);
}

} // namespace AudioStandard
} // namespace OHOS
