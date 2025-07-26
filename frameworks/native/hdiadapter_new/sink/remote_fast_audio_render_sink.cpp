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
#define LOG_TAG "RemoteFastAudioRenderSink"
#endif

#include "sink/remote_fast_audio_render_sink.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
RemoteFastAudioRenderSink::RemoteFastAudioRenderSink(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_DEBUG_LOG("construction");
}

RemoteFastAudioRenderSink::~RemoteFastAudioRenderSink()
{
    if (sinkInited_.load()) {
        DeInit();
    }
    AUDIO_DEBUG_LOG("destruction");
}

int32_t RemoteFastAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("in");
    attr_ = attr;

    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");

    renderInited_.store(true);
    sinkInited_.store(true);
    return SUCCESS;
}

void RemoteFastAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    sinkInited_.store(false);
    renderInited_.store(false);
    started_.store(false);
    paused_.store(false);

#ifdef DEBUG_DIRECT_USE_HDI
    if (ashmemSink_ != nullptr) {
        ashmemSink_->UnmapAshmem();
        ashmemSink_->CloseAshmem();
        ashmemSink_ = nullptr;
        AUDIO_INFO_LOG("deinit ashmem sink succ");
    }
#endif

    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->DestroyRender(deviceNetworkId_, hdiRenderId_);
    deviceManager->UnRegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_);
    audioRender_.ForceSetRefPtr(nullptr);
    AUDIO_INFO_LOG("end");
}

bool RemoteFastAudioRenderSink::IsInited(void)
{
    return sinkInited_.load();
}

int32_t RemoteFastAudioRenderSink::Start(void)
{
    AUDIO_INFO_LOG("in");
    if (!renderInited_.load()) {
        int32_t ret = CreateRender();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
        renderInited_.store(true);
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("already started");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    ret = CheckPositionTime();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "check position time fail, ret: %{public}d", ret);
    started_.store(true);
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::Stop(void)
{
    AUDIO_INFO_LOG("in");
    if (!started_.load()) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail, ret: %{public}d", ret);
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::Resume(void)
{
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

int32_t RemoteFastAudioRenderSink::Pause(void)
{
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

int32_t RemoteFastAudioRenderSink::Flush(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "flush fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::Reset(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "reset fail, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int64_t RemoteFastAudioRenderSink::GetVolumeDataCount()
{
    AUDIO_WARNING_LOG("not supported");
    return 0;
}

int32_t RemoteFastAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void RemoteFastAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string RemoteFastAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t RemoteFastAudioRenderSink::SetVolume(float left, float right)
{
    leftVolume_ = left;
    rightVolume_ = right;
    float volume;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->SetVolume(volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set volume fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::GetLatency(uint32_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    uint32_t hdiLatency;
    int32_t ret = audioRender_->GetLatency(hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail, ret: %{public}d", ret);
    latency = hdiLatency;
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float RemoteFastAudioRenderSink::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

void RemoteFastAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteFastAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteFastAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void RemoteFastAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

void RemoteFastAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteFastAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteFastAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return ERR_NOT_SUPPORTED;
}

void RemoteFastAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteFastSink\tstarted: " + std::string(started_.load() ? "true" : "false") +
        "\tdeviceNetworkId: " + deviceNetworkId_ + "\n";
}

void RemoteFastAudioRenderSink::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnRenderSinkParamChange(adapterName, key, condition, value);
}

void RemoteFastAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteFastAudioRenderSink::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
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

int32_t RemoteFastAudioRenderSink::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->GetMmapPosition(frames, stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get mmap position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

uint32_t RemoteFastAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
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

AudioFormat RemoteFastAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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

void RemoteFastAudioRenderSink::InitAudioSampleAttr(AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = 0;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_FAST));
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.type = attr_.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? AUDIO_MMAP_VOIP : AUDIO_MMAP_NOIRQ;
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void RemoteFastAudioRenderSink::InitDeviceDesc(AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = "";
}

int32_t RemoteFastAudioRenderSink::CreateRender(void)
{
    int64_t stamp = ClockTime::GetCurNano();
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, format: %{public}u", param.format);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(deviceNetworkId_, &param, &deviceDesc, hdiRenderId_);
    audioRender_.ForceSetRefPtr(static_cast<IAudioRender *>(render));
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    deviceManager->RegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_, this);
    if (param.type == AUDIO_MMAP_NOIRQ || param.type == AUDIO_MMAP_VOIP) {
        PrepareMmapBuffer();
    }

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("create render success, cost: [%{public}" PRId64 "]ms", stamp);
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::PrepareMmapBuffer(void)
{
    uint32_t totalBufferInMs = 40; // 40: 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / SECOND_TO_MILLISECOND);
    struct AudioMmapBufferDescriptor desc;

    int32_t ret = audioRender_->ReqMmapBuffer(reqBufferFrameSize, desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "request mmap buffer fail, ret: %{public}d", ret);
    AUDIO_INFO_LOG("memoryFd: [%{public}d], totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d], "
        "isShareable: [%{public}d], offset: [%{public}d]", desc.memoryFd, desc.totalBufferFrames,
        desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030, 3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d]", desc.totalBufferFrames,
        desc.transferFrameSize);

    uint32_t frameSizeInByte = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames); // 1440 ~ 3840
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize); // 240
    CHECK_AND_RETURN_RET_LOG(frameSizeInByte <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "buffer size will overflow");

#ifdef DEBUG_DIRECT_USE_HDI
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte;
    ashmemSink_ = new Ashmem(bufferFd_, bufferSize_);
    AUDIO_INFO_LOG("create ashmem sink succ, ashmemLen: %{public}zu", bufferSize_);
    bool tmp = ashmemSink_->MapReadAndWriteAshmem();
    CHECK_AND_RETURN_RET_LOG(tmp, ERR_OPERATION_FAILED, "map ashmem sink fail");
#endif
    return SUCCESS;
}

int32_t RemoteFastAudioRenderSink::CheckPositionTime(void)
{
    int32_t tryCount = MAX_GET_POSITION_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > MAX_GET_POSITION_HANDLE_TIME) {
            AUDIO_WARNING_LOG("tryCount: %{public}d, ret: %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("check succ");
            return SUCCESS;
        }
    }
    return ERR_OPERATION_FAILED;
}

} // namespace AudioStandard
} // namespace OHOS
