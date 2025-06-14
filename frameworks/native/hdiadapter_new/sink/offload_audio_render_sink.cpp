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
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
OffloadAudioRenderSink::~OffloadAudioRenderSink()
{
    AUDIO_INFO_LOG("volumeDataCount: %{public}" PRId64, volumeDataCount_);
}

int32_t OffloadAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("OffloadAudioRenderSink::Init");
    attr_ = attr;
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");

    sinkInited_ = true;
    return SUCCESS;
}

void OffloadAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("OffloadAudioRenderSink::DeInit");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    std::lock_guard<std::mutex> switchDeviceLock(switchDeviceMutex_);
    sinkInited_ = false;
    started_ = false;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
    audioRender_ = nullptr;
    hdiCallback_ = {};
    muteCount_ = 0;
    switchDeviceMute_ = false;
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool OffloadAudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

int32_t OffloadAudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("OffloadAudioRenderSink::Start");
    InitLatencyMeasurement();

    if (started_) {
        if (isFlushing_) {
            isNeedRestart_ = true;
            AUDIO_ERR_LOG("start fail, will restart after flush");
            return ERR_OPERATION_FAILED;
        }
        return SUCCESS;
    }
    AudioXCollie audioXCollie("OffloadAudioRenderSink::Start", TIMEOUT_SECONDS_10);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    UpdateSinkState(true);
    dumpFileName_ = "offload_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    started_ = true;
    renderPos_ = 0;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("OffloadAudioRenderSink::Stop");
    DeInitLatencyMeasurement();
    if (!started_) {
        UnLockOffloadRunningLock();
        return SUCCESS;
    }
    int32_t ret = Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    AudioXCollie audioXCollie("OffloadAudioRenderSink::Stop", TIMEOUT_SECONDS_10);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_ = false;

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

int32_t OffloadAudioRenderSink::Flush(void)
{
    Trace trace("OffloadAudioRenderSink::Flush");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "duplicate flush");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    isFlushing_ = true;
    std::thread([&] {
        auto future = async(std::launch::async, [&] {
            std::lock_guard<std::mutex> lock(sinkMutex_);
            CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
            return audioRender_->Flush(audioRender_);
        });
        if (future.wait_for(std::chrono::milliseconds(250)) == std::future_status::timeout) { // 250: max wait 250ms
            AUDIO_ERR_LOG("flush fail, timeout of 250ms");
        } else {
            int32_t ret = future.get();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("flush fail, ret: %{public}d", ret);
            }
        }
        isFlushing_ = false;
        if (isNeedRestart_) {
            isNeedRestart_ = false;
            Start();
        }
    }).detach();
    renderPos_ = 0;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Reset(void)
{
    Trace trace("OffloadAudioRenderSink::Reset");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    isNeedRestart_ = true;
    int32_t ret = Flush();
    if (ret != SUCCESS) {
        isNeedRestart_ = false;
        AUDIO_ERR_LOG("reset fail");
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "during flushing");

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
    int64_t stampThreshold = 50; // 50ms
    if (stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void OffloadAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string OffloadAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
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
    latency = hdiLatency > frames ? (hdiLatency - frames) / MICROSECOND_TO_MILLISECOND : 0;
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    Trace trace("OffloadAudioRenderSink::GetPresentationPosition");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "during flushing");

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

int32_t OffloadAudioRenderSink::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void OffloadAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void OffloadAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t OffloadAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t OffloadAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
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
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::Drain(AudioDrainType type)
{
    Trace trace("OffloadAudioRenderSink::Drain");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->DrainBuffer(audioRender_, (AudioDrainNotifyType*)&type);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "drain fail");
    return SUCCESS;
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
    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "during flushing");

    // 4: bytewidth
    uint32_t size = (uint64_t) sizeMs * AUDIO_SAMPLE_RATE_48K * 4 * STEREO_CHANNEL_COUNT / SECOND_TO_MILLISECOND;
    int32_t ret = audioRender_->SetBufferSize(audioRender_, size);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set buffer size fail");
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::LockOffloadRunningLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    AUDIO_INFO_LOG("in");
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        guard.CheckCurrTimeout();
    }
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, ERR_OPERATION_FAILED,
        "running lock is null, playback can not work well");
    CHECK_AND_RETURN_RET(!runningLocked_, SUCCESS);
    runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    runningLocked_ = true;
#endif
    return SUCCESS;
}

int32_t OffloadAudioRenderSink::UnLockOffloadRunningLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(runningLock_ != nullptr, ERR_OPERATION_FAILED,
        "running lock is null, playback can not work well");
    CHECK_AND_RETURN_RET(runningLocked_, SUCCESS);
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
    if (!sink->started_ || sink->isFlushing_) {
        AUDIO_DEBUG_LOG("invalid call, started: %{public}d, isFlushing: %{public}d", sink->started_, sink->isFlushing_);
        return SUCCESS;
    }

    impl->serviceCallback_(static_cast<RenderCallbackType>(type));
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
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);

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
    AudioXCollie audioXCollie("OffloadAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10);
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

void OffloadAudioRenderSink::DeInitLatencyMeasurement(void)
{
    signalDetectAgent_ = nullptr;
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
                maxAmplitude_ = 0;
            }
        }
    }
}

int32_t OffloadAudioRenderSink::SetVolumeInner(float left, float right)
{
    AudioXCollie audioXCollie("OffloadAudioRenderSink::SetVolumeInner", TIMEOUT_SECONDS_10, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    AUDIO_INFO_LOG("set offload vol, left: %{public}f, right: %{public}f", left, right);

    CHECK_AND_RETURN_RET_LOG(!isFlushing_, ERR_OPERATION_FAILED, "during flushing");
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

    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

// must be called with sinkMutex_ held
void OffloadAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_OFFLOAD), started);
}

} // namespace AudioStandard
} // namespace OHOS
