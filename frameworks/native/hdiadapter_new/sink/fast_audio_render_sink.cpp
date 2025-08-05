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
#define LOG_TAG "FastAudioRenderSink"
#endif

#include "sink/fast_audio_render_sink.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
FastAudioRenderSink::~FastAudioRenderSink()
{
    AUDIO_INFO_LOG("in");
    if (sinkInited_) {
        DeInit();
    }
}

int32_t FastAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("init with format:%{public}d", attr.format);
    attr_ = attr;
    halName_ = attr_.audioStreamFlag == AUDIO_FLAG_MMAP ? "primary" : "voip";
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    ret = PrepareMmapBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "prepare mmap buffer fail");

    sinkInited_ = true;
    return SUCCESS;
}

void FastAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif

    sinkInited_ = false;
    started_ = false;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
    audioRender_ = nullptr;
    ReleaseMmapBuffer();
}

bool FastAudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

int32_t FastAudioRenderSink::Start(void)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(startMutex_);
    Trace trace("FastAudioRenderSink::Start");
    AudioXCollie audioXCollie("FastAudioRenderSink::Start", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    int64_t stamp = ClockTime::GetCurNano();
    if (started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    UpdateSinkState(true);
    ret = CheckPositionTime();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "check position time fail");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
#endif
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_FAST, INIT_LASTWRITTEN_TIME);
    started_ = true;
    AUDIO_DEBUG_LOG("cost: [%{public}" PRId64 "]ms", (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND);
    return SUCCESS;
}

int32_t FastAudioRenderSink::Stop(void)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(startMutex_);
    Trace trace("FastAudioRenderSink::Stop");
    AudioXCollie audioXCollie("FastAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    if (!started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail, ret: %{public}d", ret);
    started_ = false;
    return SUCCESS;
}

int32_t FastAudioRenderSink::Resume(void)
{
    Trace trace("FastAudioRenderSink::Resume");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Resume(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_FAST, INIT_LASTWRITTEN_TIME);
    paused_ = false;
    return SUCCESS;
}

int32_t FastAudioRenderSink::Pause(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Pause(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t FastAudioRenderSink::Flush(void)
{
    Trace trace("FastAudioRenderSink::Flush");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t FastAudioRenderSink::Reset(void)
{
    Trace trace("FastAudioRenderSink::Reset");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t FastAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
#ifdef DEBUG_DIRECT_USE_HDI
    int64_t stamp = ClockTime::GetCurNano();
    if (len > (bufferSize_ - eachReadFrameSize_ * frameSizeInByte_ * writeAheadPeriod_)) {
        writeLen = 0;
        AUDIO_ERR_LOG("fail, too large, len: [%{public}" PRIu64 "]", len);
        return ERR_WRITE_FAILED;
    }
    if (isFirstWrite_) {
        PreparePosition();
    }
    CHECK_AND_RETURN_RET_LOG((curWritePos_ >= 0 && curWritePos_ < bufferSize_), ERR_INVALID_PARAM, "invalid write pos");
    char *writePtr = bufferAddresss_ + curWritePos_;
    uint64_t dataBefore = *(uint64_t *)writePtr;
    uint64_t dataAfter = 0;
    uint64_t tempPos = curWritePos_ + len;
    if (tempPos <= bufferSize_) {
        int32_t ret = memcpy_s(writePtr, (bufferSize_ - curWritePos_), static_cast<void *>(&data), len);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "copy fail");
        dataAfter = *(uint64_t *)writePtr;
        curWritePos_ = (tempPos == bufferSize_ ? 0 : tempPos);
    } else {
        AUDIO_DEBUG_LOG("curWritePos + len is %{public}" PRIu64 ", more than bufferSize", tempPos);
        size_t writeableSize = bufferSize_ - curWritePos_;
        if (memcpy_s(writePtr, writeableSize, static_cast<void *>(&data), writeableSize) ||
            memcpy_s(bufferAddresss_, bufferSize_, static_cast<void *>((char *)&data + writeableSize),
            (len - writeableSize))) {
            AUDIO_ERR_LOG("copy fail");
            return ERR_WRITE_FAILED;
        }
        curWritePos_ = len - writeableSize;
    }
    writeLen = len;

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms, curWritePos: [%{public}d], dataBefore: "
        "[%{public}" PRIu64 "], dataAfter: [%{public}" PRIu64 "]", len, stamp, curWritePos_, dataBefore, dataAfter);
    return SUCCESS;
#else
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
#endif
}

int64_t FastAudioRenderSink::GetVolumeDataCount()
{
    AUDIO_WARNING_LOG("not supported");
    return 0;
}

int32_t FastAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t FastAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void FastAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    int32_t ret = audioRender_->SetExtraParams(audioRender_, value.c_str());
    AUDIO_INFO_LOG("SetExtraParams ret: %{public}d", ret);
}

std::string FastAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t FastAudioRenderSink::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    leftVolume_ = left;
    rightVolume_ = right;
    CHECK_AND_RETURN_RET_LOG(!(halName_ == "voip" && switchDeviceMute_ && (abs(left) > FLOAT_EPS ||
        abs(right) > FLOAT_EPS)), ERR_ILLEGAL_STATE, "mute for switch device at voip scene, not support set volume");
    float volume;
    if ((abs(leftVolume_) < FLOAT_EPS) && (abs(rightVolume_) > FLOAT_EPS)) {
        volume = rightVolume_;
    } else if ((abs(leftVolume_) > FLOAT_EPS) && (abs(rightVolume_) < FLOAT_EPS)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set volume fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t FastAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t FastAudioRenderSink::GetLatency(uint32_t &latency)
{
    Trace trace("FastAudioRenderSink::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    uint32_t hdiLatency = 0;
    int32_t ret = audioRender_->GetLatency(audioRender_, &hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail, ret: %{public}d", ret);
    latency = hdiLatency;
    return SUCCESS;
}

int32_t FastAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    transactionId = 6; // 6: mmap device
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float FastAudioRenderSink::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

void FastAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void FastAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t FastAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set fast_%{public}s mute %{public}d", halName_.c_str(), mute);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("fast_%{public}s already muted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = true;
        if (halName_ == "voip") {
            audioRender_->SetVolume(audioRender_, 0.0f);
        }
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("fast_%{public}s not all unmuted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
        if (halName_ == "voip") {
            SetVolume(leftVolume_, rightVolume_);
        }
    }

    return SUCCESS;
}

int32_t FastAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t FastAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void FastAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void FastAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t FastAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    return SUCCESS;
}

int32_t FastAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void FastAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: FastSink\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

void FastAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

int32_t FastAudioRenderSink::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame, uint32_t &syncInfoSize)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE, "buffer fd has been released");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    syncInfoSize = syncInfoSize_;
    return SUCCESS;
}

int32_t FastAudioRenderSink::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->GetMmapPosition(audioRender_, &frames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get mmap position fail, ret: %{public}d", ret);
#ifdef DEBUG_DIRECT_USE_HDI
    curReadPos_ = frameSizeInByte_ * (frames - bufferTotalFrameSize_ * (frames / bufferTotalFrameSize_));
    CHECK_AND_RETURN_RET_LOG(curReadPos_ >= 0 && curReadPos_ < bufferSize_, ERR_INVALID_PARAM, "invalid pos");
    AUDIO_DEBUG_LOG("frames: [%{public}" PRIu64 "], tvSec: %{public}" PRId64 ", tvNSec: %{public}" PRId64
        ", alreadyReadFrames: %{public}" PRId64 ", curReadPos: [%{public}d]", frames, stamp.tvSec, stamp.tvNSec,
        frames, curReadPos_);
#endif
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

uint32_t FastAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return PCM_8_BIT;
        case SAMPLE_S16LE:
            return PCM_16_BIT;
        case SAMPLE_S24LE:
            return PCM_24_BIT;
        case SAMPLE_S32LE:
            return PCM_32_BIT;
        case SAMPLE_F32LE:
            return PCM_32_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat FastAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

void FastAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.interleaved = true;
    param.streamId = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ?
        static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_VOIP_FAST)) :
        static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_FAST));
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AUDIO_MMAP_VOIP : AUDIO_MMAP_NOIRQ;
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
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize); // not passed in hdi
    }
}

void FastAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    switch (static_cast<DeviceType>(attr_.deviceType)) {
        case DEVICE_TYPE_EARPIECE:
            deviceDesc.pins = PIN_OUT_EARPIECE;
            break;
        case DEVICE_TYPE_SPEAKER:
            deviceDesc.pins = PIN_OUT_SPEAKER;
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            deviceDesc.pins = PIN_OUT_HEADSET;
            break;
        case DEVICE_TYPE_USB_HEADSET:
            deviceDesc.pins = PIN_OUT_USB_EXT;
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            deviceDesc.pins = PIN_OUT_BLUETOOTH_SCO;
            break;
        default:
            AUDIO_WARNING_LOG("unsupport, use default, deviceType: %{public}d", attr_.deviceType);
            deviceDesc.pins = PIN_OUT_SPEAKER;
            break;
    }
    deviceDesc.desc = const_cast<char *>("");
}

int32_t FastAudioRenderSink::CreateRender(void)
{
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, type: %{public}d, rate: %{public}u, channel: %{public}u, format: %{public}u, "
        "device: %{public}u", param.type, param.sampleRate, param.channelCount, param.format, attr_.deviceType);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(attr_.adapterName, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    return SUCCESS;
}

// must be called with sinkMutex_ held
void FastAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_FAST), started);
}

int32_t FastAudioRenderSink::PrepareMmapBuffer(void)
{
    uint32_t totalBufferInMs = 40; // 40: 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / SECOND_TO_MILLISECOND);
    struct AudioMmapBufferDescriptor desc;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    int32_t ret = audioRender_->ReqMmapBuffer(audioRender_, reqBufferFrameSize, &desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "request mmap buffer fail, ret: %{public}d", ret);
    AUDIO_INFO_LOG("memoryAddress: [%{private}p], memoryFd: [%{public}d], totalBufferFrames: [%{public}d], "
        "transferFrameSize: [%{public}d], isShareable: [%{public}d], offset: [%{public}d]", desc.memoryAddress,
        desc.memoryFd, desc.totalBufferFrames, desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030, 3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d]", desc.totalBufferFrames,
        desc.transferFrameSize);

    frameSizeInByte_ = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames); // 1440 ~ 3840
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize); // 240
    CHECK_AND_RETURN_RET_LOG(frameSizeInByte_ <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "buffer size will overflow");
    if (desc.syncInfoSize != 0) {
        AUDIO_INFO_LOG("syncInfo for fast is enabled: %{public}d", desc.syncInfoSize);
        syncInfoSize_ = desc.syncInfoSize;
    } else {
        AUDIO_WARNING_LOG("syncInfo for fast is not enabled");
    }
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte_;
#ifdef DEBUG_DIRECT_USE_HDI
    privBufferFd_ = dup(bufferFd_);
    bufferAddress_ = (char *)mmap(nullptr, bufferSize_, PROT_READ | PROT_WRITE, MAP_SHARED, privBufferFd_, 0);
    CHECK_AND_RETURN_RET_LOG(bufferAddress_ != nullptr && bufferAddress_ != MAP_FAILED, ERR_OPERATION_FAILED,
        "mmap buffer fail");
#endif
    return SUCCESS;
}

void FastAudioRenderSink::ReleaseMmapBuffer(void)
{
#ifdef DEBUG_DIRECT_USE_HDI
    if (bufferAddress_ != nullptr) {
        munmap(bufferAddress_, bufferSize_);
        bufferAddress_ = nullptr;
        bufferSize_ = 0;
        AUDIO_INFO_LOG("release mmap buffer succ");
    } else {
        AUDIO_WARNING_LOG("buffer is already nullptr");
    }
    if (privBufferFd_ != INVALID_FD) {
        CloseFd(privBufferFd_);
        privBufferFd_ = INVALID_FD;
    }
#endif
    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }
}

int32_t FastAudioRenderSink::CheckPositionTime(void)
{
    int32_t tryCount = MAX_GET_POSITION_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int64_t maxHandleTime = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? VOIP_MAX_GET_POSITION_HANDLE_TIME :
        GENERAL_MAX_GET_POSITION_HANDLE_TIME;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        AUDIO_WARNING_LOG("sec: %{public}" PRId64 ", nanoSec: %{public}" PRId64 ", time cost: %{public}" PRId64,
            timeSec, timeNanoSec, ClockTime::GetCurNano() - curTime);
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > maxHandleTime) {
            AUDIO_WARNING_LOG("tryCount: %{public}d, ret: %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("check succ");
            return SUCCESS;
        }
    }
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    AUDIO_ERR_LOG("fail, stop render");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "stop fail, ret: %{public}d", ret);
    return ERR_OPERATION_FAILED;
}

void FastAudioRenderSink::PreparePosition(void)
{
#ifdef DEBUG_DIRECT_USE_HDI
    isFirstWrite_ = false;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    GetMmapHandlePosition(frames, timeSec, timeNanoSec); // get first start position
    int32_t periodByteSize = eachReadFrameSize_ * frameSizeInByte_;
    CHECK_AND_RETURN_LOG(periodByteSize * writeAheadPeriod_ <= ULLONG_MAX - curReadPos_, "pos will overflow");
    size_t tempPos = curReadPos_ + periodByteSize * writeAheadPeriod_; // 1 period ahead
    curWritePos_ = (tempPos < bufferSize_ ? tempPos : tempPos - bufferSize_);
    AUDIO_INFO_LOG("first render frame start, curReadPos: [%{public}d], curWritePos: [%{public}d]", curReadPos_,
        curWritePos_);
#endif
}

} // namespace AudioStandard
} // namespace OHOS
