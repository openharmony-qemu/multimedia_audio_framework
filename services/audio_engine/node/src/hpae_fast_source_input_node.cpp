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
#define LOG_TAG "HpaeFastSourceInputNode"
#endif

#include "hpae_fast_source_input_node.h"

#include <cinttypes>
#include <chrono>

#include "audio_engine_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "oh_audio_buffer.h"
#include "volume_tools.h"
#include "audio_stream_enum.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr int32_t VOLUME_SHIFT_NUMBER = 16;
constexpr uint32_t WAIT_GET_POS_TIMEOUT_MS = 200;
}

HpaeFastSourceInputNode::HpaeFastSourceInputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      outputStream_(this),
      pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
      inputAudioBuffer_(pcmBufferInfo_)
{
    writeTimeModel_.ConfigSampleRate(nodeInfo.customSampleRate == 0 ?
        nodeInfo.samplingRate : nodeInfo.customSampleRate);
    inputAudioBuffer_.SetSourceBufferType(nodeInfo.sourceBufferType);
    logUtilsTag_ = "HpaeFastSourceInputNode::" +
        (nodeInfo.deviceClass.empty() ? std::string("Fast") : nodeInfo.deviceClass);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeFastSourceInputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeFastSourceInputNode::~HpaeFastSourceInputNode()
{
    StopUpdateThread();
    DeinitLatencyMeasurement();
    (void)CapturerSourceDeInit();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.", GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeFastSourceInputNode::DoProcess()
{
    Trace trace("[" + std::to_string(GetNodeId()) + "]HpaeFastSourceInputNode::DoProcess " + GetTraceInfo());
    CHECK_AND_RETURN_LOG(audioCapturerSource_ != nullptr, "audioCapturerSource_ is nullptr");
    CHECK_AND_RETURN_LOG(srcAudioBuffer_ != nullptr, "srcAudioBuffer_ is nullptr");
    CHECK_AND_RETURN(GetSourceState() == STREAM_MANAGER_RUNNING);

    if (needReSyncPosition_) {
        Trace reSyncTrace("HpaeFastSourceInputNode::ReSyncPosition");
        CHECK_AND_RETURN_LOG(ResetReadPosition(), "ResetReadPosition failed");
        reSyncTrace.End();
        needReSyncPosition_ = false;
    }

    uint64_t handlePos = srcAudioBuffer_->GetCurWriteFrame();

    Trace readTrace("HpaeFastSourceInputNode::ReadSrcBuffer=<" + std::to_string(curReadPos_));
    while (!TryReadOneSpan(handlePos)) {
        int64_t wakeUpTime = ClockTime::GetCurNano();
        CHECK_AND_RETURN_LOG(PrepareNextLoop(wakeUpTime), "PrepareNextLoop failed");
        ClockTime::AbsoluteSleep(wakeUpTime);
        handlePos = srcAudioBuffer_->GetCurWriteFrame();
    }
    readTrace.End();
    inputAudioBuffer_.SetBufferValid(true);
    outputStream_.WriteDataToOutput(&inputAudioBuffer_);
}

bool HpaeFastSourceInputNode::Reset()
{
    return true;
}

bool HpaeFastSourceInputNode::ResetAll()
{
    return true;
}

std::shared_ptr<HpaeNode> HpaeFastSourceInputNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<HpaePcmBuffer *> *HpaeFastSourceInputNode::GetOutputPort()
{
    return &outputStream_;
}

OutputPort<HpaePcmBuffer *> *HpaeFastSourceInputNode::GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect)
{
    (void)nodeInfo;
    (void)isDisConnect;
    return &outputStream_;
}

HpaeSourceBufferType HpaeFastSourceInputNode::GetOutputPortBufferType(HpaeNodeInfo &nodeInfo)
{
    (void)nodeInfo;
    return inputAudioBuffer_.GetSourceBufferType();
}

int32_t HpaeFastSourceInputNode::GetCapturerSourceAdapter(
    const std::string &deviceClass, const SourceType &sourceType, const std::string &info)
{
    captureId_ = HDI_INVALID_ID;
    captureId_ = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(
        deviceClass, sourceType, info.empty() ? HDI_ID_INFO_DEFAULT : info, true, GetNodeInfo().routeFlag);
    audioCapturerSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(captureId_, true);
    if (audioCapturerSource_ == nullptr) {
        AUDIO_ERR_LOG("get source fail, deviceClass: %{public}s, info: %{public}s, captureId_: %{public}u",
            deviceClass.c_str(), info.c_str(), captureId_);
        HdiAdapterManager::GetInstance().ReleaseId(captureId_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::GetCapturerSourceInstance(const std::string &deviceClass,
    const std::string &deviceNetId, const SourceType &sourceType, const std::string &sourceName,
    const std::string &busAddress)
{
    if (!busAddress.empty()) {
        return GetCapturerSourceAdapter(deviceClass, sourceType, busAddress);
    }
    if (sourceType == SOURCE_TYPE_WAKEUP || sourceName == HDI_ID_INFO_EC || sourceName == HDI_ID_INFO_MIC_REF) {
        return GetCapturerSourceAdapter(deviceClass, sourceType, sourceName);
    }
    return GetCapturerSourceAdapter(deviceClass, sourceType, deviceNetId);
}

int32_t HpaeFastSourceInputNode::CapturerSourceInit(IAudioSourceAttr &attr)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    audioSourceAttr_ = attr;
    if (!audioCapturerSource_->IsInited()) {
        CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Init(attr) == SUCCESS, ERROR, "Source init fail");
    }
    int32_t ret = PrepareDeviceBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PrepareDeviceBuffer failed");
    InitDumpFile();
    InitLatencyMeasurement();
    AUDIO_INFO_LOG("Fast source DFX init, tag:%{public}s captureId:%{public}u deviceClass:%{public}s "
        "sampleRate:%{public}u channels:%{public}u format:%{public}u spanFrame:%{public}u spanBytes:%{public}u",
        logUtilsTag_.c_str(), captureId_, GetDeviceClass().c_str(),
        audioSourceAttr_.sampleRate, audioSourceAttr_.channel,
        audioSourceAttr_.format, srcSpanSizeInframe_, srcByteSizePerFrame_ * srcSpanSizeInframe_);
    SetSourceState(STREAM_MANAGER_IDLE);
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourceDeInit()
{
    StopUpdateThread();
    if (audioCapturerSource_ == nullptr || captureId_ == HDI_INVALID_ID) {
        return SUCCESS;
    }
    if (audioCapturerSource_->IsInited()) {
        audioCapturerSource_->DeInit();
    }
    audioCapturerSource_ = nullptr;
    HdiAdapterManager::GetInstance().ReleaseId(captureId_);
    captureId_ = HDI_INVALID_ID;
    srcAudioBuffer_ = nullptr;
    srcBufferFd_ = -1;
    srcTotalSizeInframe_ = 0;
    srcSpanSizeInframe_ = 0;
    srcByteSizePerFrame_ = 0;
    syncInfoSize_ = 0;
    curReadPos_ = 0;
    state_ = STREAM_MANAGER_RELEASED;
    DumpFileUtil::CloseDumpFile(&dumpHdi_);
    DeinitLatencyMeasurement();
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourceFlush(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    int32_t ret = audioCapturerSource_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Source flush fail");
    needReSyncPosition_ = true;
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourcePause(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Pause() == SUCCESS, ERROR, "Source pause fail");
    StopUpdateThread();
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourceReset(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    needReSyncPosition_ = true;
    return audioCapturerSource_->Reset();
}

int32_t HpaeFastSourceInputNode::CapturerSourceResume(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Resume() == SUCCESS, ERROR, "Source resume fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    stopUpdateThread_.store(false);
    AsyncGetPosTime();
    needReSyncPosition_ = true;
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourceStart(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Start() == SUCCESS, ERROR, "Source start fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    stopUpdateThread_.store(false);
    AsyncGetPosTime();
    needReSyncPosition_ = true;
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::CapturerSourceStop(void)
{
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    StopUpdateThread();
    if (audioCapturerSource_ == nullptr || captureId_ == HDI_INVALID_ID) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    needReSyncPosition_ = true;
    if (audioCapturerSource_->Stop() != SUCCESS) {
        AUDIO_ERR_LOG("stop error");
    }
    return SUCCESS;
}

StreamManagerState HpaeFastSourceInputNode::GetSourceState(void)
{
    return state_;
}

int32_t HpaeFastSourceInputNode::SetSourceState(StreamManagerState sourceState)
{
    HILOG_COMM_INFO("[SetSourceState]Source[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sourceState).c_str());
    state_ = sourceState;
    return SUCCESS;
}

size_t HpaeFastSourceInputNode::GetOutputPortNum()
{
    return outputStream_.GetInputNum();
}

uint32_t HpaeFastSourceInputNode::GetCaptureId() const
{
    return captureId_;
}

void HpaeFastSourceInputNode::UpdateAppsUidAndSessionId(std::vector<int32_t> &appsUid, std::vector<int32_t> &sessionsId)
{
    CHECK_AND_RETURN_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        "audioCapturerSource_ is nullptr");
    CHECK_AND_RETURN_LOG(audioCapturerSource_->IsInited(), "invalid source state");
    (void)audioCapturerSource_->UpdateAppsUid(appsUid);
    std::shared_ptr<AudioSourceClock> clock = CapturerClockManager::GetInstance().GetAudioSourceClock(captureId_);
    if (clock != nullptr) {
        clock->UpdateSessionId(sessionsId);
    }
}

void HpaeFastSourceInputNode::NotifyStreamChangeToSource(StreamChangeType change,
    uint32_t sessionId, SourceType source, CapturerState state, uint32_t appUid, bool mute)
{
    CHECK_AND_RETURN(audioCapturerSource_ != nullptr);
    audioCapturerSource_->NotifyStreamChangeToSource(change, sessionId, source, state, appUid, mute);
}

int32_t HpaeFastSourceInputNode::GetAdapterBufferInfo()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr, ERR_INVALID_HANDLE, "source is null");
    int32_t ret = audioCapturerSource_->GetMmapBufferInfo(
        srcBufferFd_, srcTotalSizeInframe_, srcSpanSizeInframe_, srcByteSizePerFrame_, syncInfoSize_);
    if (ret != SUCCESS || srcBufferFd_ == -1 || srcTotalSizeInframe_ == 0 ||
        srcSpanSizeInframe_ == 0 || srcByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("GetMmapBufferInfo fail, ret:%{public}d, fd:%{public}d,"
            " totalFrame:%{public}u, spanFrame:%{public}u",
            ret, srcBufferFd_, srcTotalSizeInframe_, srcSpanSizeInframe_);
        return ERR_ILLEGAL_STATE;
    }
    return SUCCESS;
}

int32_t HpaeFastSourceInputNode::PrepareDeviceBuffer()
{
    int32_t ret = GetAdapterBufferInfo();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAdapterBufferInfo failed");
    GetNodeInfo().frameLen = srcSpanSizeInframe_;
    writeTimeModel_.SetSpanCount(srcSpanSizeInframe_);
    uint32_t sampleRate = audioSourceAttr_.sampleRate == 0 ? GetNodeInfo().samplingRate : audioSourceAttr_.sampleRate;
    CHECK_AND_RETURN_RET_LOG(sampleRate > 0, ERR_INVALID_PARAM, "sampleRate is invalid");
    spanDuration_ = static_cast<int64_t>(srcSpanSizeInframe_) * AUDIO_NS_PER_SECOND / static_cast<int64_t>(sampleRate);
    AudioBufferHolder holder = syncInfoSize_ != 0 ? AUDIO_SERVER_ONLY_WITH_SYNC : AUDIO_SERVER_ONLY;
    srcAudioBuffer_ = OHAudioBuffer::CreateFromRemote(srcTotalSizeInframe_, srcSpanSizeInframe_, srcByteSizePerFrame_,
        holder, srcBufferFd_, INVALID_BUFFER_FD);
    CHECK_AND_RETURN_RET_LOG(srcAudioBuffer_ != nullptr, ERR_ILLEGAL_STATE, "CreateFromRemote failed");
    InitAudiobuffer(true);
    pcmBufferInfo_.frameLen = srcSpanSizeInframe_;
    inputAudioBuffer_.ReConfig(pcmBufferInfo_);
    needReSyncPosition_ = true;
    return SUCCESS;
}

void HpaeFastSourceInputNode::InitAudiobuffer(bool resetReadWritePos)
{
    CHECK_AND_RETURN_LOG(srcAudioBuffer_ != nullptr, "src audio buffer is null.");
    if (resetReadWritePos) {
        srcAudioBuffer_->ResetCurReadWritePos(0, 0, false);
    }

    uint32_t spanCount = srcAudioBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = srcAudioBuffer_->GetSpanInfoByIndex(i);
        CHECK_AND_RETURN_LOG(spanInfo != nullptr, "InitAudiobuffer failed.");
        spanInfo->spanStatus = SPAN_WRITE_DONE;
        spanInfo->offsetInFrame = 0;
        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;
        spanInfo->writeStartTime = 0;
        spanInfo->writeDoneTime = 0;
        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER;
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER;
        spanInfo->isMute = false;
    }
}

void HpaeFastSourceInputNode::AsyncGetPosTime()
{
    CHECK_AND_RETURN(!updatePosTimeThread_.joinable());
    updatePosTimeThread_ = std::thread([this]() {
        while (!stopUpdateThread_.load()) {
            uint64_t frames = 0;
            int64_t nanoTime = 0;
            (void)GetDeviceHandleInfo(frames, nanoTime);
            std::unique_lock<std::mutex> lock(updateThreadLock_);
            updateThreadCV_.wait_for(lock, std::chrono::milliseconds(WAIT_GET_POS_TIMEOUT_MS),
                [this]() { return stopUpdateThread_.load(); });
        }
    });
}

void HpaeFastSourceInputNode::StopUpdateThread()
{
    stopUpdateThread_.store(true);
    updateThreadCV_.notify_all();
    CHECK_AND_RETURN(updatePosTimeThread_.joinable());
    updatePosTimeThread_.join();
}

bool HpaeFastSourceInputNode::ResetReadPosition()
{
    CHECK_AND_RETURN_RET(srcAudioBuffer_ != nullptr, false);
    uint64_t handlePos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(GetDeviceHandleInfo(handlePos, handleTime), false, "GetDeviceHandleInfo failed");
    curReadPos_ = handlePos;
    srcAudioBuffer_->SetHandleInfo(handlePos, handleTime);
    writeTimeModel_.ResetFrameStamp(handlePos, handleTime);
    InitAudiobuffer(false);
    CHECK_AND_RETURN_RET_LOG(srcAudioBuffer_->ResetCurReadWritePos(handlePos, handlePos, false) == SUCCESS,
        false, "ResetCurReadWritePos failed");
    SpanInfo *nextReadSapn = srcAudioBuffer_->GetSpanInfo(handlePos);
    CHECK_AND_RETURN_RET_LOG(nextReadSapn != nullptr, false, "GetSpanInfo failed.");
    nextReadSapn->offsetInFrame = handlePos;
    nextReadSapn->spanStatus = SpanStatus::SPAN_WRITE_DONE;
    return true;
}

bool HpaeFastSourceInputNode::GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    Trace trace("HpaeFastSourceInputNode::GetMmapHandlePosition");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr, false, "audioCapturerSource_ is nullptr");
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = audioCapturerSource_->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "GetMmapHandlePosition failed");
    trace.End();
    nanoTime = timeSec * AUDIO_NS_PER_SECOND + timeNanoSec;
    Trace infoTrace("HpaeFastSourceInputNode::GetDeviceHandleInfo frames=>" + std::to_string(frames) + " " +
        std::to_string(nanoTime) + " at " + std::to_string(ClockTime::GetCurNano()));
    auto result = writeTimeModel_.UpdataFrameStamp(frames, nanoTime);
    if (result != CHECK_SUCCESS) {
        writeTimeModel_.ResetFrameStamp(frames, nanoTime);
    }
    if (srcAudioBuffer_ != nullptr) {
        srcAudioBuffer_->SetHandleInfo(frames, nanoTime);
    }
    posInFrame_.store(frames);
    timeInNano_.store(nanoTime);
    return true;
}

bool HpaeFastSourceInputNode::PrepareNextLoop(int64_t &wakeUpTime)
{
    CHECK_AND_RETURN_RET_LOG(srcAudioBuffer_ != nullptr, false, "srcAudioBuffer_ is nullptr");
    Trace prepareTrace("HpaeFastSourceInputNode::PrepareNextLoop curReadPos:" + std::to_string(curReadPos_));
    Trace predictTrace("HpaeFastSourceInputNode::GetPredictNextWrite");
    int64_t nextHdiWriteTime = GetPredictNextWriteTime(curReadPos_);
    predictTrace.End();
    wakeUpTime = nextHdiWriteTime + (IsVoipFast() ? RECORD_VOIP_DELAY_TIME_NS : RECORD_DELAY_TIME_NS);
    if (wakeUpTime <= ClockTime::GetCurNano()) {
        wakeUpTime = ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION_NS;
        AUDIO_WARNING_LOG("hdi send wrong position time, curReadPos:%{public}" PRIu64, curReadPos_);
    }
    int32_t ret1 = srcAudioBuffer_->SetCurWriteFrame(curReadPos_, false);
    int32_t ret2 = srcAudioBuffer_->SetCurReadFrame(curReadPos_, false);
    CHECK_AND_RETURN_RET_LOG(ret1 == SUCCESS && ret2 == SUCCESS, false,
        "SetCurWriteFrame or SetCurReadFrame failed, ret1:%{public}d ret2:%{public}d", ret1, ret2);
    CheckWakeUpTime(wakeUpTime);
    return true;
}

int64_t HpaeFastSourceInputNode::GetPredictNextWriteTime(uint64_t posInFrame)
{
    int64_t nextHdiWriteTime = writeTimeModel_.GetTimeOfPos(posInFrame);
    if (nextHdiWriteTime <= 0) {
        int64_t defaultSleepTime = spanDuration_ > ONE_MILLISECOND_DURATION_NS ?
            spanDuration_ : ONE_MILLISECOND_DURATION_NS;
        return ClockTime::GetCurNano() + defaultSleepTime;
    }
    return nextHdiWriteTime;
}

void HpaeFastSourceInputNode::CheckWakeUpTime(int64_t &wakeUpTime)
{
    int64_t curTime = ClockTime::GetCurNano();
    if (wakeUpTime - curTime > MAX_WAKEUP_TIME_NS) {
        wakeUpTime = curTime + RELATIVE_SLEEP_TIME_NS;
    }
}

void HpaeFastSourceInputNode::RecordCheckSyncInfo(uint64_t curReadPos)
{
    CHECK_AND_RETURN(srcAudioBuffer_ != nullptr);
    CHECK_AND_RETURN(srcSpanSizeInframe_ != 0);
    uint32_t curReadFrame = curReadPos / srcSpanSizeInframe_;
    (void)srcAudioBuffer_->SetSyncReadFrame(curReadFrame);
}

bool HpaeFastSourceInputNode::IsVoipFast()
{
    return (GetNodeInfo().routeFlag & AUDIO_INPUT_FLAG_VOIP_FAST) != 0 ||
        ((GetNodeInfo().routeFlag & AUDIO_INPUT_FLAG_FAST) != 0 &&
        (GetNodeInfo().routeFlag & AUDIO_INPUT_FLAG_VOIP) != 0);
}

bool HpaeFastSourceInputNode::TryReadOneSpan(uint64_t handlePos)
{
    CHECK_AND_RETURN_RET(srcAudioBuffer_ != nullptr, false);
    if (handlePos < curReadPos_) {
        return false;
    }
    BufferDesc readBuf = {};
    CHECK_AND_RETURN_RET_LOG(srcAudioBuffer_->GetReadbuffer(curReadPos_, readBuf) == SUCCESS, false,
        "GetReadbuffer failed, curReadPos:%{public}" PRIu64, curReadPos_);
    readBuf.dataLength = readBuf.bufLength;
    CheckRecordSignal(readBuf.buffer, readBuf.bufLength);
    VolumeTools::DfxOperation(readBuf, GetDfxStreamInfo(), logUtilsTag_, volumeDataCount_);
    DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(readBuf.buffer), readBuf.dataLength);
    uint32_t convertSampleCount = static_cast<uint32_t>(readBuf.dataLength / GetSizeFromFormat(GetBitWidth()));
    CHECK_AND_RETURN_RET_LOG(convertSampleCount > 0, false, "convertSampleCount is invalid");
    ConvertToFloat(GetBitWidth(), convertSampleCount, readBuf.buffer, inputAudioBuffer_.GetPcmDataBuffer());
    RecordCheckSyncInfo(curReadPos_);
    curReadPos_ += srcSpanSizeInframe_;
    return true;
}

AudioStreamInfo HpaeFastSourceInputNode::GetDfxStreamInfo()
{
    AudioStreamInfo streamInfo = {};
    const auto &nodeInfo = GetNodeInfo();
    streamInfo.samplingRate = nodeInfo.samplingRate;
    streamInfo.customSampleRate = nodeInfo.customSampleRate;
    streamInfo.format = nodeInfo.format;
    streamInfo.channels = nodeInfo.channels;
    streamInfo.channelLayout = nodeInfo.channelLayout;
    return streamInfo;
}

void HpaeFastSourceInputNode::InitDumpFile()
{
    DumpFileUtil::CloseDumpFile(&dumpHdi_);
    dumpHdiName_ = "hpae_fast_source_hdi_audio_" + std::to_string(audioSourceAttr_.deviceType) + "_" +
        GetDeviceClass() + "_" + GetTime() + "_" + std::to_string(audioSourceAttr_.sampleRate) + "_" +
        std::to_string(audioSourceAttr_.channel) + "_" + std::to_string(audioSourceAttr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
}

void HpaeFastSourceInputNode::InitLatencyMeasurement()
{
    if (latencyMeasEnabled_ || !AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleRate_ = static_cast<int32_t>(audioSourceAttr_.sampleRate);
    signalDetectAgent_->channels_ = audioSourceAttr_.channel;
    signalDetectAgent_->sampleFormat_ = audioSourceAttr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(static_cast<AudioSampleFormat>(audioSourceAttr_.format));
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void HpaeFastSourceInputNode::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
    signalDetected_ = false;
}

void HpaeFastSourceInputNode::CheckRecordSignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    CHECK_AND_RETURN_LOG(audioCapturerSource_ != nullptr, "audioCapturerSource_ is nullptr");
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        std::string dspTime = audioCapturerSource_->GetAudioParameter(key, condition);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false, signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
        AUDIO_INFO_LOG("LatencyMeas hpae fast source signal detected");
        signalDetected_ = false;
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
