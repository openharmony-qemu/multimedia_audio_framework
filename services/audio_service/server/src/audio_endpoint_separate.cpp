/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEndpointSeparate"
#endif

#include "audio_endpoint.h"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <thread>
#include <vector>
#include <mutex>

#include "securec.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "linear_pos_time_model.h"
#include "policy_handler.h"
#include "audio_service.h"
namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    static constexpr int64_t MAX_SPAN_DURATION_NS = 100000000; // 100ms
    static constexpr int64_t DELTA_TO_REAL_READ_START_TIME = 0; // 0ms
}

AudioEndpointSeparate::AudioEndpointSeparate(EndpointType type, uint64_t id,
    AudioStreamType streamType) : endpointType_(type), id_(id), streamType_(streamType)
{
    AUDIO_INFO_LOG("AudioEndpoint type:%{public}d", endpointType_);
}

std::string AudioEndpointSeparate::GetEndpointName()
{
    // temp method to get device key, should be same with AudioService::GetAudioEndpointForDevice.
    return deviceInfo_.networkId_ + std::to_string(deviceInfo_.deviceId_) + "_" + std::to_string(id_);
}

bool AudioEndpointSeparate::ShouldInnerCap(int32_t innerCapId)
{
    AUDIO_WARNING_LOG("AudioEndpointSeparate is not supported");
    return false;
}

int32_t AudioEndpointSeparate::EnableFastInnerCap(int32_t innerCapId)
{
    AUDIO_WARNING_LOG("AudioEndpointSeparate is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t AudioEndpointSeparate::DisableFastInnerCap()
{
    AUDIO_WARNING_LOG("AudioEndpointSeparate is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t AudioEndpointSeparate::DisableFastInnerCap(int32_t innerCapId)
{
    AUDIO_WARNING_LOG("AudioEndpointSeparate is not supported");
    return ERR_INVALID_OPERATION;
}

int32_t AudioEndpointSeparate::SetVolume(AudioStreamType streamType, float volume)
{
    if (streamType_ == streamType) {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        CHECK_AND_RETURN_RET(sink != nullptr, ERR_INVALID_HANDLE);
        return sink->SetVolume(volume, volume);
    }
    return SUCCESS;
}

int32_t AudioEndpointSeparate::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    if (!isInited_.load()) {
        AUDIO_ERR_LOG("ResolveBuffer failed, buffer is not configured.");
        return ERR_ILLEGAL_STATE;
    }
    buffer = dstAudioBuffer_;

    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_ILLEGAL_STATE, "ResolveBuffer failed, processBuffer_ is null.");

    return SUCCESS;
}

std::shared_ptr<OHAudioBuffer> AudioEndpointSeparate::GetBuffer()
{
    return dstAudioBuffer_;
}

AudioEndpoint::EndpointStatus AudioEndpointSeparate::GetStatus()
{
    AUDIO_INFO_LOG("AudioEndpoint get status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    return endpointStatus_.load();
}

void AudioEndpointSeparate::Release()
{
    // Wait for thread end and then clear other data to avoid using any cleared data in thread.
    AUDIO_INFO_LOG("%{public}s enter.", __func__);
    if (!isInited_.load()) {
        AUDIO_WARNING_LOG("already released");
        return;
    }

    isInited_.store(false);
    workThreadCV_.notify_all();

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    if (sink != nullptr) {
        sink->DeInit();
    }
    HdiAdapterManager::GetInstance().ReleaseId(fastRenderId_);

    endpointStatus_.store(INVALID);

    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("Set device buffer null");
        dstAudioBuffer_ = nullptr;
    }
}

AudioEndpointSeparate::~AudioEndpointSeparate()
{
    if (isInited_.load()) {
        AudioEndpointSeparate::Release();
    }
    AUDIO_INFO_LOG("~AudioEndpoint()");
}

void AudioEndpointSeparate::Dump(std::string &dumpString)
{
    // dump endpoint stream info
    dumpString += "Endpoint stream info:\n";
    AppendFormat(dumpString, "  - samplingRate: %d\n", dstStreamInfo_.samplingRate);
    AppendFormat(dumpString, "  - channels: %u\n", dstStreamInfo_.channels);
    AppendFormat(dumpString, "  - format: %u\n", dstStreamInfo_.format);

    // dump status info
    AppendFormat(dumpString, "  - Current endpoint status: %s\n", GetStatusStr(endpointStatus_).c_str());
    if (dstAudioBuffer_ != nullptr) {
        AppendFormat(dumpString, "  - Currend hdi read position: %u\n", dstAudioBuffer_->GetCurReadFrame());
        AppendFormat(dumpString, "  - Currend hdi write position: %u\n", dstAudioBuffer_->GetCurWriteFrame());
    }

    // dump linked process info
    std::lock_guard<std::mutex> lock(listLock_);
    AppendFormat(dumpString, "  - linked process:: %zu\n", processBufferList_.size());
    for (auto item : processBufferList_) {
        AppendFormat(dumpString, "  - process read position: %u\n", item->GetCurReadFrame());
        AppendFormat(dumpString, "  - process write position: %u\n", item->GetCurWriteFrame());
    }
    dumpString += "\n";
}

void AudioEndpointSeparate::InitSinkAttr(IAudioSinkAttr &attr, const AudioDeviceDescriptor &deviceInfo)
{
    bool isDefaultAdapterEnable = AudioService::GetInstance()->GetDefaultAdapterEnable();
    attr.adapterName = isDefaultAdapterEnable ? "dp" : "primary";
    attr.sampleRate = dstStreamInfo_.samplingRate; // 48000hz
    attr.channel = dstStreamInfo_.channels; // STEREO = 2
    attr.format = ConvertToHdiAdapterFormat(dstStreamInfo_.format); // SAMPLE_S16LE = 1
    attr.deviceNetworkId = deviceInfo.networkId_.c_str();
    attr.deviceType = static_cast<int32_t>(deviceInfo.deviceType_);
}

bool AudioEndpointSeparate::Config(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("%{public}s enter, deviceRole %{public}d.", __func__, deviceInfo.deviceRole_);
    if (deviceInfo.deviceRole_ == INPUT_DEVICE || deviceInfo.networkId_ != LOCAL_NETWORK_ID) {
        return false;
    }

    deviceInfo_ = deviceInfo;
    if (!deviceInfo_.audioStreamInfo_.CheckParams()) {
        AUDIO_ERR_LOG("%{public}s samplingRate or channels size is 0", __func__);
        return false;
    }
    dstStreamInfo_ = {
        *deviceInfo.audioStreamInfo_.samplingRate.rbegin(),
        deviceInfo.audioStreamInfo_.encoding,
        deviceInfo.audioStreamInfo_.format,
        *deviceInfo.audioStreamInfo_.channels.rbegin()
    };
    dstStreamInfo_.channelLayout = deviceInfo.audioStreamInfo_.channelLayout;

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    fastRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, "endpoint_sep_" + std::to_string(id_), true);
    std::shared_ptr<IAudioRenderSink> sink = manager.GetRenderSink(fastRenderId_, true);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("fast sink is nullptr");
        manager.ReleaseId(fastRenderId_);
        return false;
    }

    IAudioSinkAttr attr = {};
    InitSinkAttr(attr, deviceInfo);

    sink->Init(attr);
    if (!sink->IsInited()) {
        AUDIO_ERR_LOG("fastSinkInit failed");
        manager.ReleaseId(fastRenderId_);
        return false;
    }
    if (PrepareDeviceBuffer(deviceInfo) != SUCCESS) {
        sink->DeInit();
        manager.ReleaseId(fastRenderId_);
        return false;
    }

    Volume vol = {true, 1.0f, 0};
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType_);
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    PolicyHandler::GetInstance().GetSharedVolume(volumeType, deviceType, vol);
    sink->SetVolume(vol.volumeFloat, vol.volumeFloat);
    AUDIO_DEBUG_LOG("Init hdi volume to %{public}f", vol.volumeFloat);

    endpointStatus_ = UNLINKED;
    isInited_.store(true);
    return true;
}

int32_t AudioEndpointSeparate::GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo)
{
    int32_t ret = 0;
    AUDIO_INFO_LOG("%{public}s enter, deviceRole %{public}d.", __func__, deviceInfo.deviceRole_);

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "%{public}s fast sink is null.", __func__);
    ret = sink->GetMmapBufferInfo(dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
    if (ret != SUCCESS || dstBufferFd_ == -1 || dstTotalSizeInframe_ == 0 || dstSpanSizeInframe_ == 0 ||
        dstByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("%{public}s get mmap buffer info fail, ret %{public}d, dstBufferFd %{public}d, \
            dstTotalSizeInframe %{public}d, dstSpanSizeInframe %{public}d, dstByteSizePerFrame %{public}d.",
            __func__, ret, dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
        return ERR_ILLEGAL_STATE;
    }
    AUDIO_DEBUG_LOG("%{public}s end, fd %{public}d.", __func__, dstBufferFd_);
    return SUCCESS;
}

int32_t AudioEndpointSeparate::PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("%{public}s enter, deviceRole %{public}d.", __func__, deviceInfo.deviceRole_);
    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("%{public}s endpoint buffer is preapred, fd:%{public}d", __func__, dstBufferFd_);
        return SUCCESS;
    }

    int32_t ret = GetAdapterBufferInfo(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "%{public}s get adapter buffer Info fail, ret %{public}d.", __func__, ret);

    // spanDuration_ may be less than the correct time of dstSpanSizeInframe_.
    spanDuration_ = static_cast<int64_t>(dstSpanSizeInframe_ * AUDIO_NS_PER_SECOND / dstStreamInfo_.samplingRate);
    int64_t temp = spanDuration_ / 5 * 3; // 3/5 spanDuration
    serverAheadReadTime_ = temp < ONE_MILLISECOND_DURATION ? ONE_MILLISECOND_DURATION : temp; // at least 1ms ahead.
    AUDIO_DEBUG_LOG("%{public}s spanDuration %{public}" PRIu64" ns, serverAheadReadTime %{public}" PRIu64" ns.",
        __func__, spanDuration_, serverAheadReadTime_);

    if (spanDuration_ <= 0 || spanDuration_ >= MAX_SPAN_DURATION_NS) {
        AUDIO_ERR_LOG("%{public}s mmap span info error, spanDuration %{public}" PRIu64".", __func__, spanDuration_);
        return ERR_INVALID_PARAM;
    }
    dstAudioBuffer_ = OHAudioBuffer::CreateFromRemote(dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_,
        AUDIO_SERVER_INDEPENDENT, dstBufferFd_, OHAudioBuffer::INVALID_BUFFER_FD);
    CHECK_AND_RETURN_RET_LOG((dstAudioBuffer_ != nullptr && (dstAudioBuffer_->GetStreamStatus() != nullptr)),
        ERR_ILLEGAL_STATE, "%{public}s create buffer from remote fail.", __func__);
    dstAudioBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_IDEL);
    // clear data buffer
    ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0, dstAudioBuffer_->GetDataSize());
    if (ret != EOK) {
        AUDIO_WARNING_LOG("%{public}s memset buffer fail, ret %{public}d, fd %{public}d.", __func__, ret, dstBufferFd_);
    }
    InitAudiobuffer(true);

    AUDIO_DEBUG_LOG("%{public}s end, fd %{public}d.", __func__, dstBufferFd_);
    return SUCCESS;
}

void AudioEndpointSeparate::InitAudiobuffer(bool resetReadWritePos)
{
    CHECK_AND_RETURN_LOG((dstAudioBuffer_ != nullptr), "%{public}s: dst audio buffer is null.", __func__);
    if (resetReadWritePos) {
        dstAudioBuffer_->ResetCurReadWritePos(0, 0);
    }

    uint32_t spanCount = dstAudioBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = dstAudioBuffer_->GetSpanInfoByIndex(i);
        if (spanInfo == nullptr) {
            AUDIO_ERR_LOG("InitAudiobuffer failed.");
            return;
        }
        if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
            spanInfo->spanStatus = SPAN_WRITE_DONE;
        } else {
            spanInfo->spanStatus = SPAN_READ_DONE;
        }
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->writeStartTime = 0;
        spanInfo->writeDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    return;
}

int32_t AudioEndpointSeparate::GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe)
{
    totalSizeInframe = dstTotalSizeInframe_;
    spanSizeInframe = dstSpanSizeInframe_;
    return SUCCESS;
}

bool AudioEndpointSeparate::IsAnyProcessRunning()
{
    std::lock_guard<std::mutex> lock(listLock_);
    bool isRunning = false;
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        if (processBufferList_[i]->GetStreamStatus() == nullptr) {
            AUDIO_ERR_LOG("%{public}s process buffer %{public}zu has a null stream status.", __func__, i);
            continue;
        }
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() == STREAM_RUNNING) {
            isRunning = true;
            break;
        }
    }
    return isRunning;
}

void AudioEndpointSeparate::ResyncPosition()
{
    Trace loopTrace("AudioEndpoint::ResyncPosition");
    uint64_t curHdiReadPos = 0;
    int64_t readTime = 0;
    if (!GetDeviceHandleInfo(curHdiReadPos, readTime)) {
        AUDIO_ERR_LOG("ResyncPosition call GetDeviceHandleInfo failed.");
        return;
    }
    int64_t curTime = ClockTime::GetCurNano();
    int64_t temp = curTime - readTime;
    if (temp > spanDuration_) {
        AUDIO_ERR_LOG("GetDeviceHandleInfo may cost long time.");
    }

    dstAudioBuffer_->SetHandleInfo(curHdiReadPos, readTime);
}

bool AudioEndpointSeparate::StartDevice()
{
    AUDIO_INFO_LOG("%{public}s enter.", __func__);
    // how to modify the status while unlinked and started?
    if (endpointStatus_ != IDEL) {
        AUDIO_ERR_LOG("Endpoint status is not IDEL");
        return false;
    }
    endpointStatus_ = STARTING;

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    if (sink == nullptr || sink->Start() != SUCCESS) {
        AUDIO_ERR_LOG("Sink start failed.");
        return false;
    }

    std::unique_lock<std::mutex> lock(loopThreadLock_);
    needResyncPosition_ = true;
    endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    workThreadCV_.notify_all();
    AUDIO_DEBUG_LOG("StartDevice out, status is %{public}s", GetStatusStr(endpointStatus_).c_str());
    return true;
}

bool AudioEndpointSeparate::StopDevice()
{
    AUDIO_INFO_LOG("StopDevice with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    endpointStatus_ = STOPPING;
    // Clear data buffer to avoid noise in some case.
    if (dstAudioBuffer_ != nullptr) {
        int32_t ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0,
            dstAudioBuffer_->GetDataSize());
        if (ret != EOK) {
            AUDIO_WARNING_LOG("memset_s failed. ret:%{public}d", ret);
        }
    }

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    if (sink == nullptr || sink->Stop() != SUCCESS) {
        AUDIO_ERR_LOG("Sink stop failed.");
        return false;
    }

    endpointStatus_ = STOPPED;
    return true;
}

int32_t AudioEndpointSeparate::OnStart(IAudioProcessStream *processStream)
{
    AUDIO_INFO_LOG("OnStart endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        AUDIO_INFO_LOG("OnStart find endpoint already in RUNNING.");
        return SUCCESS;
    }
    if (endpointStatus_ == IDEL && !isDeviceRunningInIdel_) {
        // call sink start
        StartDevice();
        endpointStatus_ = RUNNING;
    }
    return SUCCESS;
}

int32_t AudioEndpointSeparate::OnPause(IAudioProcessStream *processStream)
{
    AUDIO_INFO_LOG("OnPause endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    }
    if (endpointStatus_ == IDEL && !isDeviceRunningInIdel_) {
        // call sink stop when no process running?
        AUDIO_INFO_LOG("OnPause status is IDEL, call stop");
    }
    return SUCCESS;
}

int32_t AudioEndpointSeparate::GetProcLastWriteDoneInfo(const std::shared_ptr<OHAudioBuffer> processBuffer,
    uint64_t curWriteFrame, uint64_t &proHandleFrame, int64_t &proHandleTime)
{
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_HANDLE, "Process found but buffer is null");
    uint64_t curReadFrame = processBuffer->GetCurReadFrame();
    SpanInfo *curWriteSpan = processBuffer->GetSpanInfo(curWriteFrame);
    CHECK_AND_RETURN_RET_LOG(curWriteSpan != nullptr, ERR_INVALID_HANDLE,
        "%{public}s curWriteSpan of curWriteFrame %{public}" PRIu64" is null", __func__, curWriteFrame);
    if (curWriteSpan->spanStatus == SpanStatus::SPAN_WRITE_DONE || curWriteFrame < dstSpanSizeInframe_ ||
        curWriteFrame < curReadFrame) {
        proHandleFrame = curWriteFrame;
        proHandleTime = curWriteSpan->writeDoneTime;
    } else {
        int32_t ret = GetProcLastWriteDoneInfo(processBuffer, curWriteFrame - dstSpanSizeInframe_,
            proHandleFrame, proHandleTime);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
            "%{public}s get process last write done info fail, ret %{public}d.", __func__, ret);
    }

    AUDIO_INFO_LOG("%{public}s end, curWriteFrame %{public}" PRIu64", proHandleFrame %{public}" PRIu64", "
        "proHandleTime %{public}" PRId64".", __func__, curWriteFrame, proHandleFrame, proHandleTime);
    return SUCCESS;
}

int32_t AudioEndpointSeparate::OnUpdateHandleInfo(IAudioProcessStream *processStream)
{
    Trace trace("separate AudioEndpoint::OnUpdateHandleInfo");
    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr != processStream) {
            processItr++;
            continue;
        }
        std::shared_ptr<OHAudioBuffer> processBuffer = (*processItr)->GetStreamBuffer();
        CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_OPERATION_FAILED, "Process found but buffer is null");

        ResyncPosition();
        isFind = true;
        break;
    }
    if (!isFind) {
        AUDIO_ERR_LOG("Can not find any process to UpdateHandleInfo");
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t AudioEndpointSeparate::LinkProcessStream(IAudioProcessStream *processStream, bool startWhenLinking)
{
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBuffer> processBuffer = processStream->GetStreamBuffer();
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");
    CHECK_AND_RETURN_RET_LOG(processBuffer->GetStreamStatus() != nullptr, ERR_INVALID_PARAM,
        "stream status is null");
    CHECK_AND_RETURN_RET_LOG(processList_.size() < MAX_LINKED_PROCESS, ERR_OPERATION_FAILED, "reach link limit.");

    AUDIO_INFO_LOG("LinkProcessStream endpoint status:%{public}s.", GetStatusStr(endpointStatus_).c_str());

    bool needEndpointRunning = processBuffer->GetStreamStatus()->load() == STREAM_RUNNING;

    if (endpointStatus_ == STARTING) {
        AUDIO_INFO_LOG("LinkProcessStream wait start begin.");
        std::unique_lock<std::mutex> lock(loopThreadLock_);
        workThreadCV_.wait(lock, [this] {
            return endpointStatus_ != STARTING;
        });
        AUDIO_DEBUG_LOG("LinkProcessStream wait start end.");
    }

    if (endpointStatus_ == RUNNING) {
        std::lock_guard<std::mutex> lock(listLock_);
        processList_.push_back(processStream);
        processBufferList_.push_back(processBuffer);
        AUDIO_DEBUG_LOG("LinkProcessStream success.");
        return SUCCESS;
    }

    if (endpointStatus_ == UNLINKED) {
        endpointStatus_ = IDEL; // handle push_back in IDEL
        if (isDeviceRunningInIdel_) {
            StartDevice();
        }
    }

    if (endpointStatus_ == IDEL) {
        {
            std::lock_guard<std::mutex> lock(listLock_);
            processList_.push_back(processStream);
            processBufferList_.push_back(processBuffer);
        }
        if (!needEndpointRunning) {
            AUDIO_DEBUG_LOG("LinkProcessStream success, process stream status is not running.");
            return SUCCESS;
        }
        // needEndpointRunning = true
        if (isDeviceRunningInIdel_) {
            endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
        } else {
            // KeepWorkloopRunning will wait on IDEL
            StartDevice();
        }
        AUDIO_DEBUG_LOG("LinkProcessStream success.");
        return SUCCESS;
    }

    return SUCCESS;
}

int32_t AudioEndpointSeparate::UnlinkProcessStream(IAudioProcessStream *processStream)
{
    AUDIO_INFO_LOG("UnlinkProcessStream in status:%{public}s.", GetStatusStr(endpointStatus_).c_str());
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBuffer> processBuffer = processStream->GetStreamBuffer();
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");

    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    auto bufferItr = processBufferList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr == processStream && *bufferItr == processBuffer) {
            processList_.erase(processItr);
            processBufferList_.erase(bufferItr);
            isFind = true;
            break;
        } else {
            processItr++;
            bufferItr++;
        }
    }
    if (processList_.size() == 0) {
        StopDevice();
        endpointStatus_ = UNLINKED;
    }

    AUDIO_DEBUG_LOG("UnlinkProcessStream end, %{public}s the process.", (isFind ? "find and remove" : "not find"));
    return SUCCESS;
}

void AudioEndpointSeparate::ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData)
{
    size_t srcListSize = srcDataList.size();

    for (size_t i = 0; i < srcListSize; i++) {
        if (srcDataList[i].streamInfo.format != SAMPLE_S16LE || srcDataList[i].streamInfo.channels != STEREO ||
            srcDataList[i].bufferDesc.bufLength != dstData.bufferDesc.bufLength ||
            srcDataList[i].bufferDesc.dataLength != dstData.bufferDesc.dataLength) {
            AUDIO_ERR_LOG("ProcessData failed, streamInfo are different");
            return;
        }
    }

    // Assum using the same format and same size
    if (dstData.streamInfo.format != SAMPLE_S16LE || dstData.streamInfo.channels != STEREO) {
        AUDIO_ERR_LOG("ProcessData failed, streamInfo are not support");
        return;
    }

    size_t dataLength = dstData.bufferDesc.dataLength;
    dataLength /= 2; // SAMPLE_S16LE--> 2 byte
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; dataLength > 0; dataLength--) {
        int32_t sum = 0;
        for (size_t i = 0; i < srcListSize; i++) {
            int32_t vol = srcDataList[i].volumeStart; // change to modify volume of each channel
            int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDataList[i].bufferDesc.buffer) + offset;
            sum += (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER; // 1/65536
        }
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
}

bool AudioEndpointSeparate::GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    Trace trace("AE::GetMmapHP");
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = 0;

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    if (sink == nullptr || !sink->IsInited()) {
        AUDIO_ERR_LOG("GetDeviceHandleInfo failed: sink is not inited.");
        return false;
    }
    // GetMmapHandlePosition will call using ipc.
    ret = sink->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Call adapter GetMmapHandlePosition failed: %{public}d", ret);
        return false;
    }
    trace.End();
    nanoTime = timeNanoSec + timeSec * AUDIO_NS_PER_SECOND;
    Trace infoTrace("AudioEndpoint::GetDeviceHandleInfo frames=>" + std::to_string(frames) + " " +
        std::to_string(nanoTime) + " at " + std::to_string(ClockTime::GetCurNano()));
    nanoTime += DELTA_TO_REAL_READ_START_TIME; // global delay in server
    return true;
}

std::string AudioEndpointSeparate::GetStatusStr(EndpointStatus status)
{
    switch (status) {
        case INVALID:
            return "INVALID";
        case UNLINKED:
            return "UNLINKED";
        case IDEL:
            return "IDEL";
        case STARTING:
            return "STARTING";
        case RUNNING:
            return "RUNNING";
        case STOPPING:
            return "STOPPING";
        case STOPPED:
            return "STOPPED";
        default:
            break;
    }
    return "NO_SUCH_STATUS";
}

int32_t AudioEndpointSeparate::WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBuffer> &procBuf,
    const BufferDesc &readBuf)
{
    CHECK_AND_RETURN_RET_LOG(procBuf != nullptr, ERR_INVALID_HANDLE, "%{public}s process buffer is null.", __func__);
    uint64_t curWritePos = procBuf->GetCurWriteFrame();
    Trace trace("AudioEndpoint::WriteProcessData-<" + std::to_string(curWritePos));
    SpanInfo *curWriteSpan = procBuf->GetSpanInfo(curWritePos);
    CHECK_AND_RETURN_RET_LOG(curWriteSpan != nullptr, ERR_INVALID_HANDLE,
        "%{public}s get write span info of procBuf fail.", __func__);

    AUDIO_DEBUG_LOG("%{public}s process buffer write start, curWritePos %{public}" PRIu64".", __func__, curWritePos);
    curWriteSpan->spanStatus.store(SpanStatus::SPAN_WRITTING);
    curWriteSpan->writeStartTime = ClockTime::GetCurNano();

    BufferDesc writeBuf;
    int32_t ret = procBuf->GetWriteBuffer(curWritePos, writeBuf);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s get write buffer fail, ret %{public}d.", __func__, ret);
    ret = memcpy_s(static_cast<void *>(writeBuf.buffer), writeBuf.bufLength,
        static_cast<void *>(readBuf.buffer), readBuf.bufLength);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "%{public}s memcpy data to process buffer fail, "
        "curWritePos %{public}" PRIu64", ret %{public}d.", __func__, curWritePos, ret);

    curWriteSpan->writeDoneTime = ClockTime::GetCurNano();
    procBuf->SetHandleInfo(curWritePos, curWriteSpan->writeDoneTime);
    ret = procBuf->SetCurWriteFrame(curWritePos + dstSpanSizeInframe_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s set procBuf next write frame fail, ret %{public}d.",
        __func__, ret);
    curWriteSpan->spanStatus.store(SpanStatus::SPAN_WRITE_DONE);
    return SUCCESS;
}

void AudioEndpointSeparate::WriteToProcessBuffers(const BufferDesc &readBuf)
{
    std::lock_guard<std::mutex> lock(listLock_);
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        if (processBufferList_[i] == nullptr) {
            AUDIO_ERR_LOG("%{public}s process buffer %{public}zu is null.", __func__, i);
            continue;
        }
        if (processBufferList_[i]->GetStreamStatus() == nullptr) {
            AUDIO_ERR_LOG("%{public}s process buffer %{public}zu has a null stream status.", __func__, i);
            continue;
        }
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() != STREAM_RUNNING) {
            AUDIO_WARNING_LOG("%{public}s process buffer %{public}zu not running, stream status %{public}d.",
                __func__, i, processBufferList_[i]->GetStreamStatus()->load());
            continue;
        }

        int32_t ret = WriteToSpecialProcBuf(processBufferList_[i], readBuf);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("%{public}s endpoint write to process buffer %{public}zu fail, ret %{public}d.",
                __func__, i, ret);
            continue;
        }
        AUDIO_DEBUG_LOG("%{public}s endpoint process buffer %{public}zu write success.", __func__, i);
    }
}

float AudioEndpointSeparate::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in audioEndpointSeparate not support");
    return 0;
}

uint32_t AudioEndpointSeparate::GetLinkedProcessCount()
{
    std::lock_guard<std::mutex> lock(listLock_);
    return processList_.size();
}

AudioMode AudioEndpointSeparate::GetAudioMode() const
{
    // AudioEndpointSeparate only support playback for now
    return AUDIO_MODE_PLAYBACK;
}
} // namespace AudioStandard
} // namespace OHOS