/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AuxiliarySink"
#endif

#include "sink/auxiliary_sink.h"
#include <climits>
#include <memory>
#include <sys/mman.h>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_stream_enum.h"
#include "util/hdi_dfx_utils.h"

constexpr int32_t MAX_AUXILIARY_BUFFERSIZE = 32768;

namespace OHOS {
namespace AudioStandard {
AuxiliarySink::~AuxiliarySink()
{
    AUDIO_INFO_LOG("in");
    if (sinkInited_) {
        DeInit();
    }
}

int32_t AuxiliarySink::Init(const IAudioSinkAttr &attr)
{
    attr_ = attr;
    halName_ = attr_.adapterName == "bt_a2dp" ? "btAuxSink" : "usbAuxSink";
    logTag_ = attr.sinkName == "a2dp" ? "btAuxSink" : "usbAuxSink";
    AUDIO_INFO_LOG("%{public}s::Init with params:[%{public}d_%{public}d_%{public}d]",
        logTag_.c_str(), attr.sampleRate, attr.channel, attr.format);
    Trace trace(logTag_ + "::Init params:[" + std::to_string(attr.sampleRate) + "_" +
        std::to_string(attr.channel) + "_" + std::to_string(attr.format) + "]");

    dumpFileName_ = std::string(AUXILIARY_SINK_FILENAME) + "_" + GetTime() + "_" +
        std::to_string(attr.sampleRate) + "_" + std::to_string(attr.channel) +
        "_" + std::to_string(attr.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    deviceManager_ = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager_ != nullptr, ERR_INVALID_HANDLE);

    uint32_t ret = PrepareMmapBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "prepare mmap buffer fail");
    sinkInited_ = true;
    return SUCCESS;
}

int32_t AuxiliarySink::PrepareMmapBuffer(void)
{
    int32_t streamId = HDI_INVALID_ID;
    struct AudioMmapBufferDescriptor buffer;
    struct AudioSampleAttributes params;
    InitAudioSampleAttr(params);

    int32_t ret = deviceManager_->CreateCognitionStream("primary", &params, streamId, &buffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "create cogStream:%{public}d fail,"
        " ret:%{public}d", streamId, ret);
    AUDIO_INFO_LOG("sinkId:[%{public}d], bufferInfo:[%{private}p]_[%{public}d]_[%{public}d]_[%{public}d]_"
        "[%{public}d]_[%{public}d]", streamId, buffer.memoryAddress, buffer.memoryFd, buffer.totalBufferFrames,
        buffer.transferFrameSize, buffer.isShareable, buffer.offset);
    sinkId_ = streamId;
    buffer_ = buffer;
    bufferFd_ = buffer.memoryFd;
    int32_t periodFrameMaxSize = 1920000;
    CHECK_AND_RETURN_RET_LOG(buffer.totalBufferFrames >= 0 && buffer.transferFrameSize >= 0 &&
        buffer.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames:[%{public}d],transferFrameSize: [%{public}d]",
        buffer.totalBufferFrames, buffer.transferFrameSize);

    frameSizeInByte_ = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    totalBufferFrames_ = static_cast<uint32_t>(buffer.totalBufferFrames);
    eachSpanFrames_ = static_cast<uint32_t>(buffer.transferFrameSize);
    eachSpanFramesSize_ = eachSpanFrames_ * frameSizeInByte_;
    CHECK_AND_RETURN_RET_LOG(frameSizeInByte_ <= ULLONG_MAX / totalBufferFrames_, ERR_OPERATION_FAILED,
        "buffer size will overflow");
    if (buffer.syncInfoSize != 0) {
        AUDIO_INFO_LOG("syncInfo for auxiliarySink is enabled:%{public}d", buffer.syncInfoSize);
        syncInfoSize_ = buffer.syncInfoSize;
    } else {
        AUDIO_WARNING_LOG("syncInfo for auxiliarySink is not enabled");
    }

    bufferSize_ = totalBufferFrames_ * frameSizeInByte_;
    dupBufferFd_ = dup(bufferFd_);
    bufferAddress_ = (char *)mmap(nullptr, MAX_AUXILIARY_BUFFERSIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, dupBufferFd_, 0);
    CHECK_AND_RETURN_RET_LOG(bufferAddress_ != nullptr && bufferAddress_ != MAP_FAILED, ERR_OPERATION_FAILED,
        "mmap buffer fail");
    return SUCCESS;
}

int32_t AuxiliarySink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int64_t stamp = ClockTime::GetCurNano();
    writeLen = 0;
    CHECK_AND_RETURN_RET(IsInited(), ERROR_ILLEGAL_STATE);
    if (len > bufferSize_) {
        AUDIO_ERR_LOG("fail, too large, len: [%{public}" PRIu64 "]", len);
        return ERR_WRITE_FAILED;
    }

    CHECK_AND_RETURN_RET_LOG((curWritePos_ >= 0 && curWritePos_ <= bufferSize_), ERR_INVALID_PARAM,
        "invalid write pos");
    char *writePtr = bufferAddress_ + curWritePos_;
    uint64_t writeLenth = len >= eachSpanFramesSize_ ? eachSpanFramesSize_ : len;
    Trace trace(logTag_ + "::RenderFrame len:" + std::to_string(len) + "curPos:" +
        std::to_string(curWritePos_) + "writeLen:" + std::to_string(writeLenth));
    int32_t ret = memcpy_s(writePtr, eachSpanFramesSize_, static_cast<void *>(&data), writeLenth);
    if (ret != EOK) {
        AUDIO_DEBUG_LOG("CogStream:%{public}d copy data fail, ret:%{public}d", sinkId_, ret);
        return ERR_WRITE_FAILED;
    }
    writeLen = writeLenth;
    ret = deviceManager_->NotifyCognitionData("primary", sinkId_, writeLen, curWritePos_);
    if (ret != SUCCESS) {
        AUDIO_DEBUG_LOG("notify CogStream:%{public}d data fail, ret:%{public}d", sinkId_, ret);
        return ERR_OPERATION_FAILED;
    }
    curWritePos_ = (curWritePos_ == 0) ? eachSpanFramesSize_ : 0;

    HdiDfxUtils::PrintSinkVolInfo(static_cast<char *>(&data), writeLen, attr_, logTag_, volumeDataCount_);
    HdiDfxUtils::DumpData(static_cast<char *>(&data), writeLen, dumpFile_, dumpFileName_);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("len:[%{public}" PRIu64 "] writeLen:[%{public}" PRIu64 "] cost: [%{public}" PRId64
        "]ms curWritePos: [%{public}d]", len, writeLen, stamp, curWritePos_);
    return SUCCESS;
}

void AuxiliarySink::DeInit(void)
{
    Trace trace(logTag_ + "::DeInit");
    AUDIO_INFO_LOG("in");
    ReleaseMmapBuffer();
    sinkInited_ = false;
    deviceManager_ = nullptr;
}

bool AuxiliarySink::IsInited(void)
{
    return sinkInited_;
}

int32_t AuxiliarySink::Start(void)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t AuxiliarySink::Stop(void)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t AuxiliarySink::Resume(void)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t AuxiliarySink::Pause(void)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t AuxiliarySink::Flush(void)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t AuxiliarySink::Reset(void)
{
    AUDIO_WARNING_LOG("not supported");
    return SUCCESS;
}

void AuxiliarySink::ReleaseMmapBuffer(void)
{
    uint32_t ret = deviceManager_->DestroyCognitionStream("primary", sinkId_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "destroy cogStream:%{public}d fail, ret:%{public}d", sinkId_, ret);
    if (bufferAddress_ != nullptr) {
        munmap(bufferAddress_, MAX_AUXILIARY_BUFFERSIZE);
        bufferAddress_ = nullptr;
        bufferSize_ = 0;
        AUDIO_INFO_LOG("release mmap buffer succ");
    } else {
        AUDIO_WARNING_LOG("buffer is already nullptr");
    }
    if (dupBufferFd_ != INVALID_FD) {
        CloseFd(dupBufferFd_);
        dupBufferFd_ = INVALID_FD;
    }

    if (bufferFd_ != INVALID_FD) {
        CloseFd(bufferFd_);
        bufferFd_ = INVALID_FD;
    }
}

void AuxiliarySink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = 1;
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

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
}

uint32_t AuxiliarySink::PcmFormatToBit(AudioSampleFormat format)
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

AudioFormat AuxiliarySink::ConvertToHdiFormat(AudioSampleFormat format)
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

void AuxiliarySink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: AuxSink\tInitState: " + std::string(sinkInited_ ? "true" : "false") +
        "\nparams:[" + std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) +
        std::to_string(attr_.format) + "]";
}

int64_t AuxiliarySink::GetVolumeDataCount()
{
    AUDIO_WARNING_LOG("not supported");
    return 0;
}

int32_t AuxiliarySink::SetVolume(float left, float right)
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t AuxiliarySink::GetVolume(float &left, float &right)
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t AuxiliarySink::GetLatency(uint32_t &latency)
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t AuxiliarySink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t AuxiliarySink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

float AuxiliarySink::GetMaxAmplitude(void)
{
    AUDIO_WARNING_LOG("not supported");
    return 0.0;
}

void AuxiliarySink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void AuxiliarySink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t AuxiliarySink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    AUDIO_WARNING_LOG("not supported");
    return SUCCESS;
}

int32_t AuxiliarySink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported");
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
