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
#define LOG_TAG "HpaeFastSinkOutputNode"
#endif

#include "hpae_fast_sink_output_node.h"
#include "audio_errors.h"
#include <algorithm>
#include <cinttypes>
#include <limits>
#include <numeric>
#include <unistd.h>

#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "audio_utils.h"
#include "volume_tools.h"
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
    constexpr uint32_t TIME_MS_PER_SEC = 1000;
    constexpr size_t MAX_TRANS_BUFFER_SIZE = 1 * 1024 * 1024; // 1M
    constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    constexpr int64_t AUDIO_NS_PER_US = 1000;
    constexpr uint32_t WAIT_GET_POS_TIMEOUT_MS = 200;
    const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 40;
    constexpr int64_t ULTRA_FAST_MINIMUM_AHEAD_TIME_NS = 3000000; // 3ms
}

HpaeFastSinkOutputNode::HpaeFastSinkOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo)
{
    frameLenMs_ = nodeInfo.samplingRate ? nodeInfo.frameLen * TIME_MS_PER_SEC / nodeInfo.samplingRate : 0;
    uint32_t sampleRate = nodeInfo.customSampleRate == 0 ? nodeInfo.samplingRate : nodeInfo.customSampleRate;
    readTimeModel_.ConfigSampleRate(sampleRate);
    writeTimeModel_.ConfigSampleRate(sampleRate);
    logUtilsTag_ = "HpaeFastSinkOutputNode::" +
        (nodeInfo.deviceClass.empty() ? std::string("Fast") : nodeInfo.deviceClass);
    isUltraFast_ = (nodeInfo.deviceClass == "ultra_fast");
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeFastSinkOutputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeFastSinkOutputNode::~HpaeFastSinkOutputNode()
{
    StopUpdateThread();
    DumpFileUtil::CloseDumpFile(&dumpHdi_);
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeFastSinkOutputNode::DoProcess()
{
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_LOG(dstAudioBuffer_ != nullptr, "dstAudioBuffer_ is nullptr");
    CHECK_AND_RETURN(!CheckIfSuspend());

    int64_t curTime = ClockTime::GetCurNano();
    Trace loopTrace("HpaeFastSinkOutputNode::loop_trace predictWakeT:" + std::to_string(lastPredictWakeUpTime_) +
        " actualWakeT:" + std::to_string(curTime));
    int64_t wakeUpTime = curTime;
    uint64_t curWritePos = dstAudioBuffer_->GetCurWriteFrame();

    if (needReSyncPosition_) {
        Trace reSyncTrace("HpaeFastSinkOutputNode::ReSyncPosition");
        ReSyncPosition();
        reSyncTrace.End();
        needReSyncPosition_ = false;
        curTime = ClockTime::GetCurNano();
    }

    CheckTimeAndBufferReady(curWritePos, wakeUpTime, curTime);
    HpaePcmBuffer *pcmBuffer = nullptr;
    CHECK_AND_RETURN_LOG(GetRenderFrameDataInner(pcmBuffer), "GetRenderFrameDataInner failed");
    Trace writeTrace("HpaeFastSinkOutputNode::WriteDstBuffer=>" + std::to_string(curWritePos));
    CHECK_AND_RETURN_LOG(WriteToDeviceBuffer(pcmBuffer, curWritePos) == SUCCESS, "WriteToDeviceBuffer failed");
    writeTrace.End();
    CheckJank(curWritePos);
    CHECK_AND_RETURN_LOG(PrepareNextLoop(curWritePos, wakeUpTime), "PrepareNextLoop failed");
    lastPredictWakeUpTime_ = wakeUpTime;
    // Wake client before sleep so client can prepare next buffer during sleep
    if (clientWakeCallback_) {
        clientWakeCallback_();
    }
    loopTrace.End();
    CheckWakeUpTime(wakeUpTime);
    ClockTime::AbsoluteSleep(wakeUpTime);
}

const char *HpaeFastSinkOutputNode::GetRenderFrameData()
{
    return renderFrameData_.empty() ? nullptr : renderFrameData_.data();
}

bool HpaeFastSinkOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        inputStream_.DisConnect(preOutput.first);
    }
    return true;
}

bool HpaeFastSinkOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode != nullptr && hpaeNode->ResetAll()) {
            inputStream_.DisConnect(preOutput.first);
        }
    }
    return true;
}

void HpaeFastSinkOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, GetNodeId(), preNode->GetSharedInstance()->GetNodeId());
    }
#endif
}

void HpaeFastSinkOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, GetNodeId(), preNode->GetSharedInstance()->GetNodeId());
    }
#endif
}

int32_t HpaeFastSinkOutputNode::GetRenderSinkInstance(const std::string &deviceClass, const std::string &deviceNetId)
{
    Trace trace("HpaeFastSinkOutputNode::GetRenderSinkInstance " + deviceClass + "_" + deviceNetId);
    std::string idInfo = deviceNetId.empty() ? HDI_ID_INFO_DEFAULT : deviceNetId;
    renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, idInfo, true);
    audioRendererSink_ = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (audioRendererSink_ == nullptr) {
        AUDIO_ERR_LOG("GetRenderSinkInstance failed, deviceClass:%{public}s deviceNetId:%{public}s renderId:%{public}u",
            deviceClass.c_str(), deviceNetId.c_str(), renderId_);
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        renderId_ = HDI_INVALID_ID;
        return ERROR;
    }
    AUDIO_INFO_LOG("GetRenderSinkInstance success, deviceClass:%{public}s deviceNetId:%{public}s renderId:%{public}u",
        deviceClass.c_str(), deviceNetId.c_str(), renderId_);
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::RenderSinkInit(IAudioSinkAttr &attr)
{
    Trace trace("HpaeFastSinkOutputNode::RenderSinkInit " + GetTraceInfo());
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());

    sinkOutAttr_ = attr;
    currentOutputDevice_ = static_cast<DeviceType>(sinkOutAttr_.deviceType);
    AUDIO_INFO_LOG("RenderSinkInit begin, sessionId:%{public}u flag:%{public}u deviceType:%{public}d rate:%{public}u "
        "channel:%{public}u format:%{public}u deviceClass:%{public}s",
        GetSessionId(), attr.audioStreamFlag, attr.deviceType, attr.sampleRate, attr.channel, attr.format,
        GetDeviceClass().c_str());
    int32_t ret = -1;
    if (!audioRendererSink_->IsInited()) {
        Trace hdiInitTrace("HDI::Init sessionId[" + std::to_string(GetSessionId()) + "]");
        ret = audioRendererSink_->Init(attr);
        hdiInitTrace.End();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
            "audioRendererSink_ init failed, errCode is %{public}d", ret);
    }
    adapterType_ = attr.audioStreamFlag == AUDIO_FLAG_VOIP_FAST ? ADAPTER_TYPE_VOIP_FAST :
        (isUltraFast_ ? ADAPTER_TYPE_ULTRA_FAST : ADAPTER_TYPE_FAST);
    SetSinkState(STREAM_MANAGER_IDLE);
    ret = PrepareDeviceBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "PrepareDeviceBuffer failed, ret:%{public}d", ret);
    SyncCurrentOutputDevice();
    audioRendererSink_->RegisterCurrentDeviceCallback([this](bool) {
        SyncCurrentOutputDevice();
    });
    InitSinkVolume();
    InitDumpFile();
    AUDIO_INFO_LOG("RenderSinkInit success, sessionId:%{public}u currentOutputDevice:%{public}d span:%{public}u "
        "byteSize:%{public}u", GetSessionId(), currentOutputDevice_, dstSpanSizeInframe_, dstByteSizePerFrame_);
    return ret;
}

int32_t HpaeFastSinkOutputNode::RenderSinkStart()
{
    Trace trace("HpaeFastSinkOutputNode::RenderSinkStart " + GetTraceInfo());
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());

    Trace hdiStartTrace("HDI::Start sessionId[" + std::to_string(GetSessionId()) + "]");
    int32_t ret = audioRendererSink_->Start();
    hdiStartTrace.End();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ start failed, errCode is %{public}d", ret);
    SetSinkState(STREAM_MANAGER_RUNNING);
    isStarted_ = true;
    ResetZeroVolumeState();
    timeoutStopCount_ = 0;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType_, INIT_LASTWRITTEN_TIME);
    lastWriteTime_ = ClockTime::GetCurNano();
    stopUpdateThread_.store(false);
    InitLatencyMeasurement();
    AsyncGetPosTime();
    needReSyncPosition_ = true;
    AUDIO_INFO_LOG("RenderSinkStart success, sessionId:%{public}u device:%{public}d adapterType:%{public}d",
        GetSessionId(), currentOutputDevice_, adapterType_);
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::RenderSinkStop()
{
    Trace trace("HpaeFastSinkOutputNode::RenderSinkStop " + GetTraceInfo());
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    isStarted_ = false;
    lastPredictWakeUpTime_ = 0;
    StopUpdateThread();
    Trace hdiStopTrace("HDI::Stop sessionId[" + std::to_string(GetSessionId()) + "]");
    int32_t ret = audioRendererSink_->Stop();
    hdiStopTrace.End();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ stop failed, errCode is %{public}d", ret);
    AUDIO_INFO_LOG("RenderSinkStop success, sessionId:%{public}u device:%{public}d", GetSessionId(),
        currentOutputDevice_);
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::RenderSinkDeInit()
{
    Trace trace("HpaeFastSinkOutputNode::RenderSinkDeInit " + GetTraceInfo());
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    StopUpdateThread();
    SetSinkState(STREAM_MANAGER_RELEASED);
    isStarted_ = false;
    DeinitLatencyMeasurement();
    ResetZeroVolumeState();

    // Release mmap buffer resources
    dstAudioBuffer_ = nullptr;
    dstBufferFd_ = -1;
    dstTotalSizeInframe_ = 0;
    dstSpanSizeInframe_ = 0;
    dstByteSizePerFrame_ = 0;
    syncInfoSize_ = 0;
    dstSpanSizeInByte_ = 0;
    curWritePos_ = 0;
    DumpFileUtil::CloseDumpFile(&dumpHdi_);

    audioRendererSink_->DeInit();
    audioRendererSink_ = nullptr;
    if (renderId_ != HDI_INVALID_ID) {
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        renderId_ = HDI_INVALID_ID;
    }
    AUDIO_INFO_LOG("RenderSinkDeInit success, sessionId:%{public}u", GetSessionId());
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::RenderSinkFlush()
{
    Trace trace("HpaeFastSinkOutputNode::RenderSinkFlush " + GetTraceInfo());
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERR_ILLEGAL_STATE, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_ILLEGAL_STATE, "dstAudioBuffer_ is nullptr");
    int32_t ret = audioRendererSink_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Flush failed");
    dstAudioBuffer_->ResetCurReadWritePos(0, 0);
    needReSyncPosition_ = true;
    AUDIO_INFO_LOG("RenderSinkFlush success, sessionId:%{public}u", GetSessionId());
    return SUCCESS;
}

size_t HpaeFastSinkOutputNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

StreamManagerState HpaeFastSinkOutputNode::GetSinkState() const
{
    return state_;
}

int32_t HpaeFastSinkOutputNode::SetSinkState(StreamManagerState sinkState)
{
    HILOG_COMM_INFO("[SetSinkState]Sink[%{public}s] state change:"
        "[%{public}s]-->[%{public}s]", GetDeviceClass().c_str(),
        ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sinkState).c_str());
    state_ = sinkState;
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::SetTimeoutStopThd(int64_t timeoutMs)
{
    CHECK_AND_RETURN_RET_LOG(timeoutMs >= 0, ERR_INVALID_PARAM,
        "timeoutMs must be non-negative, got %{public}" PRId64, timeoutMs);
    timeoutThdFrames_ = frameLenMs_ == 0 ? TIME_OUT_STOP_THD_DEFAULT_FRAME : (timeoutMs / frameLenMs_);
    if (timeoutThdFrames_ == 0) {
        timeoutThdFrames_ = 1;
    }
    AUDIO_INFO_LOG("timeoutThdFrames_:%{public}u, timeoutMs:%{public}" PRId64, timeoutThdFrames_, timeoutMs);
    return SUCCESS;
}

int32_t HpaeFastSinkOutputNode::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERROR, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_->IsInited(), ERR_ILLEGAL_STATE, "audioRendererSink_ not init");
    return audioRendererSink_->UpdateAppsUid(appsUid);
}

void HpaeFastSinkOutputNode::SetNeedCheckZeroVolume(bool needCheckZeroVolume)
{
    if (needCheckZeroVolume_ != needCheckZeroVolume) {
        AUDIO_INFO_LOG("SetNeedCheckZeroVolume change, sessionId:%{public}u from:%{public}d to:%{public}d",
            GetSessionId(), needCheckZeroVolume_, needCheckZeroVolume);
    }
    needCheckZeroVolume_ = needCheckZeroVolume;
}

int32_t HpaeFastSinkOutputNode::RefreshSpanSize(bool &isUpdated)
{
    isUpdated = false;
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERR_ILLEGAL_STATE, "audioRendererSink_ is nullptr");

    int oldFd = dstBufferFd_;
    uint32_t oldTotal = dstTotalSizeInframe_;
    uint32_t oldSpan = dstSpanSizeInframe_;
    uint32_t oldByte = dstByteSizePerFrame_;
    uint32_t oldSync = syncInfoSize_;

    int32_t ret = GetAdapterBufferInfo();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAdapterBufferInfo failed");

    isUpdated = (oldFd != dstBufferFd_) || (oldTotal != dstTotalSizeInframe_) ||
        (oldSpan != dstSpanSizeInframe_) || (oldByte != dstByteSizePerFrame_) || (oldSync != syncInfoSize_);
    if (!isUpdated) {
        AUDIO_DEBUG_LOG("RefreshSpanSize unchanged, sessionId:%{public}u span:%{public}u byte:%{public}u",
            GetSessionId(), dstSpanSizeInframe_, dstByteSizePerFrame_);
        return SUCCESS;
    }

    AUDIO_INFO_LOG("RefreshSpanSize change, sessionId:%{public}u old(fd:%{public}d total:%{public}u span:%{public}u "
        "byte:%{public}u sync:%{public}u) -> new(fd:%{public}d total:%{public}u span:%{public}u byte:%{public}u "
        "sync:%{public}u)", GetSessionId(), oldFd, oldTotal, oldSpan, oldByte, oldSync, dstBufferFd_,
        dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_, syncInfoSize_);

    GetNodeInfo().frameLen = dstSpanSizeInframe_;
    readTimeModel_.SetSpanCount(dstSpanSizeInframe_);
    writeTimeModel_.SetSpanCount(dstSpanSizeInframe_);

    AudioBufferHolder holder = syncInfoSize_ != 0 ? AUDIO_SERVER_ONLY_WITH_SYNC : AUDIO_SERVER_ONLY;
    dstAudioBuffer_ = OHAudioBuffer::CreateFromRemote(dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_,
        holder, dstBufferFd_, INVALID_BUFFER_FD);
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_ILLEGAL_STATE, "CreateFromRemote fail");
    InitAudiobuffer(true);
    InitTransBuffer();
    curWritePos_ = 0;
    needReSyncPosition_ = true;
    AUDIO_INFO_LOG("RefreshSpanSize success, span:%{public}u byte:%{public}u", dstSpanSizeInframe_,
        dstByteSizePerFrame_);
    return SUCCESS;
}

uint32_t HpaeFastSinkOutputNode::GetSpanSizeInFrame() const
{
    return dstSpanSizeInframe_;
}

void HpaeFastSinkOutputNode::NotifyStreamChangeToSink(StreamChangeType change,
    uint32_t sessionId, StreamUsage usage, RendererState state, uint32_t appUid)
{
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_LOG(audioRendererSink_->IsInited(), "audioRendererSink_ not init");
    audioRendererSink_->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}

uint64_t HpaeFastSinkOutputNode::GetLatency()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, 0, "audioRendererSink_ is nullptr");
    uint32_t latency = 0;
    Trace hdiTrace("HDI::GetLatency sessionId[" + std::to_string(GetSessionId()) + "]");
    if (audioRendererSink_->GetLatency(latency) != SUCCESS) {
        hdiTrace.End();
        return 0;
    }
    hdiTrace.End();
    return latency;
}

float HpaeFastSinkOutputNode::GetMaxAmplitude() const
{
    const_cast<HpaeFastSinkOutputNode *>(this)->lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    const_cast<HpaeFastSinkOutputNode *>(this)->startUpdate_ = true;
    return maxAmplitude_;
}

int32_t HpaeFastSinkOutputNode::GetAdapterBufferInfo()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr,
        ERR_INVALID_HANDLE, "sink is null");

    Trace hdiTrace("HDI::GetMmapBufferInfo sessionId[" + std::to_string(GetSessionId()) + "]");
    int32_t ret = audioRendererSink_->GetMmapBufferInfo(
        dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_,
        dstByteSizePerFrame_, syncInfoSize_);
    hdiTrace.End();

    if (ret != SUCCESS || dstBufferFd_ == -1 || dstTotalSizeInframe_ == 0
        || dstSpanSizeInframe_ == 0 || dstByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("GetMmapBufferInfo fail, ret:%{public}d, fd:%{public}d, "
            "totalFrame:%{public}u, spanFrame:%{public}u, bytePerFrame:%{public}u",
            ret, dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
        return ERR_ILLEGAL_STATE;
    }
    AUDIO_INFO_LOG("mmap buffer info: totalFrame:%{public}u, spanFrame:%{public}u, bytePerFrame:%{public}u",
        dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
    return SUCCESS;
}

void HpaeFastSinkOutputNode::InitTransBuffer() //todo
{
    dstSpanSizeInByte_ = static_cast<size_t>(dstSpanSizeInframe_) * dstByteSizePerFrame_;
    if (dstSpanSizeInByte_ == 0 || dstSpanSizeInByte_ > MAX_TRANS_BUFFER_SIZE) {
        AUDIO_WARNING_LOG("dstSpanSizeInByte is too large:%{public}zu, use default value.", dstSpanSizeInByte_);
        dstSpanSizeInByte_ = MAX_TRANS_BUFFER_SIZE;
    }
    renderFrameData_.resize(dstSpanSizeInByte_);
}

int64_t HpaeFastSinkOutputNode::CalcServerAheadReadTime(uint32_t sampleRate)
{
    spanDuration_ = static_cast<int64_t>(dstSpanSizeInframe_) *
        AUDIO_NS_PER_SECOND / static_cast<int64_t>(sampleRate);
    int64_t temp = spanDuration_ / 5 * 3; // 3/5 spanDuration
    if (isUltraFast_ && temp < ULTRA_FAST_MINIMUM_AHEAD_TIME_NS) {
        temp = ULTRA_FAST_MINIMUM_AHEAD_TIME_NS;
    }
    int64_t setTime = -1;
    int64_t maxSetTime = static_cast<int64_t>(dstTotalSizeInframe_ - dstSpanSizeInframe_) *
        AUDIO_NS_PER_SECOND / static_cast<int64_t>(sampleRate);
    GetSysPara("persist.multimedia.serveraheadreadtime", setTime);
    temp = setTime > 0 && setTime < maxSetTime ? setTime : temp;
    return temp < ONE_MILLISECOND_DURATION_NS ? ONE_MILLISECOND_DURATION_NS : temp;
}

int32_t HpaeFastSinkOutputNode::PrepareDeviceBuffer()
{
    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("mmap buffer already prepared, fd:%{public}d", dstBufferFd_);
        return SUCCESS;
    }

    // Step A: Get mmap buffer info from HDI adapter
    int32_t ret = GetAdapterBufferInfo();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "GetAdapterBufferInfo fail, ret:%{public}d", ret);
    GetNodeInfo().frameLen = dstSpanSizeInframe_;
    readTimeModel_.SetSpanCount(dstSpanSizeInframe_);
    writeTimeModel_.SetSpanCount(dstSpanSizeInframe_);
    uint32_t sampleRate = sinkOutAttr_.sampleRate == 0 ? GetNodeInfo().samplingRate : sinkOutAttr_.sampleRate;
    CHECK_AND_RETURN_RET_LOG(sampleRate > 0, ERR_INVALID_PARAM, "sampleRate is invalid");
    serverAheadReadTime_ = CalcServerAheadReadTime(sampleRate);
    AUDIO_INFO_LOG("PrepareDeviceBuffer spanDuration:%{public}" PRId64
        " aheadReadTime:%{public}" PRId64
        " totalFrame:%{public}u spanFrame:%{public}u sampleRate:%{public}u",
        spanDuration_, serverAheadReadTime_, dstTotalSizeInframe_, dstSpanSizeInframe_, sampleRate);
    // Step B: Create OHAudioBuffer from the remote mmap buffer
    AudioBufferHolder holder = syncInfoSize_ != 0 ?
        AUDIO_SERVER_ONLY_WITH_SYNC : AUDIO_SERVER_ONLY;
    dstAudioBuffer_ = OHAudioBuffer::CreateFromRemote(
        dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_,
        holder, dstBufferFd_, INVALID_BUFFER_FD);

    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_ILLEGAL_STATE,
        "CreateFromRemote fail");
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_->GetBufferHolder() == holder, ERR_ILLEGAL_STATE,
        "Buffer holder mismatch");
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_->GetStreamStatus() != nullptr, ERR_INVALID_PARAM,
        "Stream status is null");

    // Step C: Initialize stream status and clear data buffer
    dstAudioBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_IDEL);

    ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(),
        0, dstAudioBuffer_->GetDataSize());
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret:%{public}d, fd:%{public}d", ret, dstBufferFd_);
    }

    InitAudiobuffer(true);
    InitTransBuffer();
    curWritePos_ = 0;

    AUDIO_INFO_LOG("PrepareDeviceBuffer success, fd:%{public}d, spanSizeInByte:%{public}zu",
        dstBufferFd_, dstSpanSizeInByte_);
    return SUCCESS;
}

void HpaeFastSinkOutputNode::InitAudiobuffer(bool resetReadWritePos)
{
    CHECK_AND_RETURN_LOG((dstAudioBuffer_ != nullptr), "dst audio buffer is null.");
    if (resetReadWritePos) {
        dstAudioBuffer_->ResetCurReadWritePos(0, 0, false);
    }

    uint32_t spanCount = dstAudioBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = dstAudioBuffer_->GetSpanInfoByIndex(i);
        CHECK_AND_RETURN_LOG(spanInfo != nullptr, "InitAudiobuffer failed.");
        spanInfo->spanStatus = SPAN_READ_DONE;
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

void HpaeFastSinkOutputNode::AsyncGetPosTime()
{
    if (updatePosTimeThread_.joinable()) {
        return;
    }
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

void HpaeFastSinkOutputNode::StopUpdateThread()
{
    stopUpdateThread_.store(true);
    updateThreadCV_.notify_all();
    if (updatePosTimeThread_.joinable()) {
        updatePosTimeThread_.join();
    }
}

bool HpaeFastSinkOutputNode::TriggerPrepareNextLoop()
{
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, false, "dstAudioBuffer_ is nullptr");
    uint64_t curWritePos = dstAudioBuffer_->GetCurWriteFrame();
    int64_t wakeUpTime = ClockTime::GetCurNano();
    return PrepareNextLoop(curWritePos, wakeUpTime);
}

bool HpaeFastSinkOutputNode::TriggerPrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime)
{
    return PrepareNextLoop(curWritePos, wakeUpTime);
}

bool HpaeFastSinkOutputNode::IsCheckingSuspend() const
{
    return isCheckingSuspend_;
}

bool HpaeFastSinkOutputNode::CheckIfSuspend()
{
    if (GetPreOutNum() == 0) {
        isCheckingSuspend_ = true;
        timeoutStopCount_++;
        if (timeoutStopCount_ > timeoutThdFrames_ && GetSinkState() == STREAM_MANAGER_RUNNING) {
            AUDIO_INFO_LOG("timeout stop sink");
            RenderSinkStop();
        }
        return true;
    }
    timeoutStopCount_ = 0;
    isCheckingSuspend_ = false;
    return false;
}

void HpaeFastSinkOutputNode::ReSyncPosition()
{
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    if (!GetDeviceHandleInfo(frame, nanoTime)) {
        return;
    }
    readTimeModel_.ResetFrameStamp(frame, nanoTime);
    writeTimeModel_.ResetFrameStamp(frame, nanoTime);
    posInFrame_.store(frame);
    timeInNano_.store(nanoTime);
    lastPredictWakeUpTime_ = 0;
    if (dstAudioBuffer_ != nullptr) {
        dstAudioBuffer_->SetHandleInfo(frame, nanoTime);
        uint64_t nextWritePos = frame + dstSpanSizeInframe_;
        InitAudiobuffer(false);
        int32_t ret = dstAudioBuffer_->ResetCurReadWritePos(nextWritePos, nextWritePos, false);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed.");
        SpanInfo *nextWriteSpan = dstAudioBuffer_->GetSpanInfo(nextWritePos);
        CHECK_AND_RETURN_LOG(nextWriteSpan != nullptr, "GetSpanInfo failed.");
        nextWriteSpan->offsetInFrame = nextWritePos;
        nextWriteSpan->spanStatus = SpanStatus::SPAN_READ_DONE;
        curWritePos_ = nextWritePos;
    }
}

bool HpaeFastSinkOutputNode::GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    Trace trace("HpaeFastSinkOutputNode::GetMmapHandlePosition");
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, false, "audioRendererSink_ is nullptr");
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = audioRendererSink_->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "GetMmapHandlePosition failed");
    trace.End();
    nanoTime = timeSec * AUDIO_NS_PER_SECOND + timeNanoSec;
    Trace infoTrace("HpaeFastSinkOutputNode::GetDeviceHandleInfo frames=>" + std::to_string(frames) + " " +
        std::to_string(nanoTime) + " at " + std::to_string(ClockTime::GetCurNano()));
    auto result = readTimeModel_.UpdataFrameStamp(frames, nanoTime);
    if (result == NEED_MODIFY) {
        readTimeModel_.ResetFrameStamp(frames, nanoTime);
    }
    posInFrame_.store(frames);
    timeInNano_.store(nanoTime);
    if (dstAudioBuffer_ != nullptr) {
        dstAudioBuffer_->SetHandleInfo(frames, nanoTime);
    }
    return true;
}

bool HpaeFastSinkOutputNode::CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos)
{
    if (dstAudioBuffer_ == nullptr) {
        return false;
    }
    int64_t nextHdiReadTime = GetPredictNextReadTime(curWritePos);
    int64_t predictWakeupTime = nextHdiReadTime - serverAheadReadTime_;
    return predictWakeupTime <= checkTime;
}

void HpaeFastSinkOutputNode::CheckTimeAndBufferReady(uint64_t &curWritePos, int64_t &wakeUpTime, int64_t &curTime)
{
    if (curTime - wakeUpTime > THREE_MILLISECOND_DURATION_NS) {
        AUDIO_WARNING_LOG("Wake up cost %{public}" PRId64 " ms", (curTime - wakeUpTime) / AUDIO_NS_PER_US);
    }
    if (!CheckAllBufferReady(wakeUpTime, curWritePos)) {
        curTime = ClockTime::GetCurNano();
    }
}

bool HpaeFastSinkOutputNode::PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime)
{
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, false, "dstAudioBuffer_ is nullptr");
    CheckSyncInfo(curWritePos);
    uint64_t nextHandlePos = curWritePos + dstSpanSizeInframe_;
    Trace prepareTrace("HpaeFastSinkOutputNode::PrepareNextLoop " + std::to_string(nextHandlePos));
    Trace predictTrace("HpaeFastSinkOutputNode::GetPredictNextRead");
    int64_t nextHdiReadTime = GetPredictNextReadTime(nextHandlePos);
    predictTrace.End();
    wakeUpTime = nextHdiReadTime - serverAheadReadTime_;
    int32_t ret1 = dstAudioBuffer_->SetCurWriteFrame(nextHandlePos, false);
    int32_t ret2 = dstAudioBuffer_->SetCurReadFrame(nextHandlePos, false);
    CHECK_AND_RETURN_RET_LOG(ret1 == SUCCESS && ret2 == SUCCESS, false,
        "SetCurWriteFrame or SetCurReadFrame failed, ret1:%{public}d ret2:%{public}d", ret1, ret2);
    CheckWakeUpTime(wakeUpTime);
    return true;
}

int64_t HpaeFastSinkOutputNode::GetPredictNextReadTime(uint64_t posInFrame)
{
    int64_t predictWakeupTime = readTimeModel_.GetTimeOfPos(posInFrame);
    if (predictWakeupTime <= 0) {
        return ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION_NS;
    }
    return predictWakeupTime;
}

void HpaeFastSinkOutputNode::CheckWakeUpTime(int64_t &wakeUpTime)
{
    int64_t curTime = ClockTime::GetCurNano();
    if (wakeUpTime - curTime > MAX_WAKEUP_TIME_NS) {
        wakeUpTime = curTime + RELATIVE_SLEEP_TIME_NS;
    }
}

void HpaeFastSinkOutputNode::CheckJank(uint64_t curWritePos)
{
    if (GetSinkState() != STREAM_MANAGER_RUNNING || !isStarted_) {
        return;
    }
    if (syncInfoSize_ != 0) {
        CheckSyncInfo(curWritePos);
        lastWriteTime_ = ClockTime::GetCurNano();
    }
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType_, ClockTime::GetCurNano());
}

void HpaeFastSinkOutputNode::CheckSyncInfo(uint64_t curWritePos)
{
    CHECK_AND_RETURN(dstAudioBuffer_ != nullptr);
    CHECK_AND_RETURN(dstSpanSizeInframe_ != 0);

    uint32_t curWriteFrame = curWritePos / dstSpanSizeInframe_;
    dstAudioBuffer_->SetSyncWriteFrame(curWriteFrame);
    uint32_t curReadFrame = dstAudioBuffer_->GetSyncReadFrame();
    Trace trace("Sync: writeIndex:" + std::to_string(curWriteFrame) + " readIndex:" + std::to_string(curReadFrame));

    if (curWriteFrame >= curReadFrame) {
        return;
    }
    AUDIO_WARNING_LOG("write %{public}d is slower than read %{public}d ", curWriteFrame, curReadFrame);
    int64_t cost = (ClockTime::GetCurNano() - lastWriteTime_) / AUDIO_US_PER_SECOND;
    AudioPerformanceMonitor::GetInstance().ReportWriteSlow(adapterType_, cost);
}

bool HpaeFastSinkOutputNode::GetRenderFrameDataInner(HpaePcmBuffer *&pcmBuffer)
{
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    CHECK_AND_RETURN_RET(!outputVec.empty(), false);
    pcmBuffer = outputVec.front();
    CHECK_AND_RETURN_RET_LOG(pcmBuffer != nullptr, false, "pcmBuffer is nullptr");
    size_t convertSize = std::min(renderFrameData_.size(), static_cast<size_t>(
        pcmBuffer->GetChannelCount() * pcmBuffer->GetFrameLen() * GetSizeFromFormat(GetBitWidth())));
    CHECK_AND_RETURN_RET(convertSize > 0, false);
    std::fill(renderFrameData_.begin(), renderFrameData_.end(), 0);
    validRenderDataLen_ = convertSize;
    uint32_t convertSampleCount = static_cast<uint32_t>(convertSize / GetSizeFromFormat(GetBitWidth()));
    CHECK_AND_RETURN_RET(convertSampleCount > 0, false);
    ConvertFromFloat(GetBitWidth(), convertSampleCount, pcmBuffer->GetPcmDataBuffer(), renderFrameData_.data());
    return true;
}

int32_t HpaeFastSinkOutputNode::WriteToDeviceBuffer(HpaePcmBuffer *pcmBuffer, uint64_t curWritePos)
{
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_ILLEGAL_STATE, "dstAudioBuffer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(dstSpanSizeInByte_ > 0, ERR_INVALID_PARAM, "dstSpanSizeInByte_ is invalid");
    CHECK_AND_RETURN_RET_LOG(pcmBuffer != nullptr, ERR_INVALID_PARAM, "pcmBuffer is nullptr");

    BufferDesc writeBuf = {};
    int32_t ret = dstAudioBuffer_->GetWriteBuffer(curWritePos, writeBuf);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetWriteBuffer failed");

    size_t writeSize = std::min(validRenderDataLen_, writeBuf.bufLength);
    if (writeSize == 0) {
        return ERR_INVALID_PARAM;
    }
    if (switchDevicesMute_) {
        ret = memset_s(writeBuf.buffer, writeBuf.bufLength, 0, writeBuf.bufLength);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memset_s for switch mute failed");
    } else {
        ret = memcpy_s(writeBuf.buffer, writeBuf.bufLength, renderFrameData_.data(), writeSize);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memcpy_s failed");
        if (writeSize < writeBuf.bufLength) {
            size_t remainingSize = writeBuf.bufLength - writeSize;
            ret = memset_s(writeBuf.buffer + writeSize, remainingSize, 0, remainingSize);
            CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memset_s failed");
            AUDIO_WARNING_LOG("WriteToDeviceBuffer short frame, "
                "sessionId:%{public}u valid:%{public}zu span:%{public}zu",
                GetSessionId(), writeSize, writeBuf.bufLength);
        }
    }
    writeBuf.dataLength = writeBuf.bufLength;
    CheckPlaySignal(writeBuf.buffer, writeBuf.dataLength);
    VolumeTools::DfxOperation(writeBuf, GetDfxStreamInfo(), logUtilsTag_, volumeDataCount_);
    ChannelVolumes channelVolumes = VolumeTools::CountVolumeLevel(writeBuf, GetDfxStreamInfo().format,
        GetDfxStreamInfo().channels);
    int32_t vol = channelVolumes.channel == 0 ? 0 : std::accumulate(channelVolumes.volStart,
        channelVolumes.volStart + channelVolumes.channel, static_cast<int64_t>(0)) / channelVolumes.channel;
    vol = isExistLoopback_ ? 1 : vol;
    ZeroVolumeCheck(vol);
    UpdateSilentState(writeBuf.buffer, writeBuf.dataLength);
    UpdateAmplitudeIfNeeded(writeBuf);
    DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(writeBuf.buffer), writeBuf.dataLength);

    lastWriteTime_ = ClockTime::GetCurNano();
    return SUCCESS;
}

AudioStreamInfo HpaeFastSinkOutputNode::GetDfxStreamInfo()
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

void HpaeFastSinkOutputNode::InitDumpFile()
{
    DumpFileUtil::CloseDumpFile(&dumpHdi_);
    dumpHdiName_ = "hpae_fast_sink_hdi_audio_" + std::to_string(sinkOutAttr_.deviceType) + "_" +
        GetDeviceClass() + "_" + GetTime() + "_" + std::to_string(sinkOutAttr_.sampleRate) + "_" +
        std::to_string(sinkOutAttr_.channel) + "_" + std::to_string(sinkOutAttr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
}

void HpaeFastSinkOutputNode::InitSinkVolume()
{
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    audioRendererSink_->SetVolume(1.0f, 1.0f);
    AUDIO_INFO_LOG("Init fast sink default volume 1.0 with device %{public}d", currentOutputDevice_);
}

void HpaeFastSinkOutputNode::SyncCurrentOutputDevice()
{
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    DeviceType outputDevice = audioRendererSink_->GetCurrentOutputDevice();
    if (outputDevice == DEVICE_TYPE_INVALID || outputDevice == DEVICE_TYPE_NONE) {
        return;
    }
    if (currentOutputDevice_ != outputDevice) {
        AUDIO_INFO_LOG("SyncCurrentOutputDevice change, sessionId:%{public}u from:%{public}d to:%{public}d",
            GetSessionId(), currentOutputDevice_, outputDevice);
    }
    currentOutputDevice_ = outputDevice;
}

void HpaeFastSinkOutputNode::InitLatencyMeasurement()
{
    if (latencyMeasEnabled_ || !AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleRate_ = static_cast<int32_t>(sinkOutAttr_.sampleRate);
    signalDetectAgent_->channels_ = sinkOutAttr_.channel;
    signalDetectAgent_->sampleFormat_ = sinkOutAttr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(static_cast<AudioSampleFormat>(sinkOutAttr_.format));
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
    detectedTime_ = 0;
}

void HpaeFastSinkOutputNode::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
    signalDetected_ = false;
    detectedTime_ = 0;
}

void HpaeFastSinkOutputNode::CheckPlaySignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    size_t byteSize = static_cast<size_t>(GetFormatByteSize(static_cast<AudioSampleFormat>(sinkOutAttr_.format)));
    size_t denominator = (sinkOutAttr_.sampleRate / MILLISECOND_PER_SECOND) * byteSize * sinkOutAttr_.channel;
    CHECK_AND_RETURN(denominator != 0);
    detectedTime_ += bufferSize / denominator;
    if (detectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        std::string dspTime = audioRendererSink_->GetAudioParameter(key, condition);
        LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true, signalDetectAgent_->lastPeakBufferTime_);
        AUDIO_INFO_LOG("LatencyMeas hpae fast sink signal detected");
        LatencyMonitor::GetInstance().ShowTimestamp(true);
        signalDetectAgent_->dspTimestampGot_ = true;
        signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AUDIO_INFO_LOG("LatencyMeas hpae fast sink signal detected");
        detectedTime_ = 0;
    }
}

bool HpaeFastSinkOutputNode::IsInvalidBuffer(uint8_t *buffer, size_t bufferSize) const
{
    bool isInvalid = false;
    uint8_t ui8Data = 0;
    int16_t i16Data = 0;
    switch (sinkOutAttr_.format) {
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

void HpaeFastSinkOutputNode::UpdateSilentState(uint8_t *buffer, size_t bufferSize)
{
    if (IsInvalidBuffer(buffer, bufferSize)) {
        if (startMuteTime_ == 0) {
            startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if ((currentTime - startMuteTime_ >= ONE_MINUTE) && !isInSilentState_) {
            isInSilentState_ = true;
            AUDIO_WARNING_LOG("final hpae fast sink output remains silent for some time, sessionId:%{public}u",
                GetSessionId());
        }
        return;
    }
    if (startMuteTime_ != 0) {
        startMuteTime_ = 0;
    }
    if (isInSilentState_) {
        AUDIO_WARNING_LOG("final hpae fast sink output becomes non-silent, sessionId:%{public}u", GetSessionId());
        isInSilentState_ = false;
    }
}

void HpaeFastSinkOutputNode::UpdateAmplitudeIfNeeded(BufferDesc &writeBuf)
{
    if (!startUpdate_) {
        return;
    }
    if (renderFrameNum_ == 0) {
        last10FrameStartTime_ = ClockTime::GetCurNano();
    }
    renderFrameNum_++;
    maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(sinkOutAttr_.format),
        reinterpret_cast<char *>(writeBuf.buffer), writeBuf.dataLength);
    if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
        renderFrameNum_ = 0;
        if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
            startUpdate_ = false;
        }
    }
}

void HpaeFastSinkOutputNode::ZeroVolumeCheck(int32_t vol)
{
    CHECK_AND_RETURN(needCheckZeroVolume_);
    if (currentOutputDevice_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        return;
    }
    if (std::abs(vol - 0) <= std::numeric_limits<float>::epsilon()) {
        if (currentOutputDevice_ == DEVICE_TYPE_NEARLINK) {
            return;
        }
        if (zeroVolumeState_ == INACTIVE) {
            zeroVolumeStartTime_ = ClockTime::GetCurNano();
            zeroVolumeState_ = IN_TIMING;
            AUDIO_INFO_LOG("ZeroVolumeCheck enter timing, sessionId:%{public}u device:%{public}d", GetSessionId(),
                currentOutputDevice_);
            return;
        }
        if (zeroVolumeState_ == IN_TIMING &&
            ClockTime::GetCurNano() - zeroVolumeStartTime_ > DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS) {
            zeroVolumeState_ = ACTIVE;
            HandleZeroVolumeStopEvent();
            AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(adapterType_);
        }
        return;
    }
    if (zeroVolumeState_ == INACTIVE) {
        return;
    }
    if (zeroVolumeState_ == ACTIVE) {
        HandleZeroVolumeStartEvent();
    }
    ResetZeroVolumeState();
}

void HpaeFastSinkOutputNode::HandleZeroVolumeStartEvent()
{
    Trace trace("HpaeFastSinkOutputNode::HandleZeroVolumeStartEvent " + std::to_string(GetSessionId()));
    if (isStarted_) {
        AUDIO_INFO_LOG("fast sink already started");
        return;
    }
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    int32_t ret = audioRendererSink_->Start();
    if (ret != SUCCESS) {
        AUDIO_INFO_LOG("Volume from zero to none-zero, start hpae fast sink failed");
        isStarted_ = false;
        return;
    }
    AUDIO_INFO_LOG("Volume from zero to none-zero, start hpae fast sink success");
    stopUpdateThread_.store(false);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType_, INIT_LASTWRITTEN_TIME);
    lastWriteTime_ = ClockTime::GetCurNano();
    AsyncGetPosTime();
    isStarted_ = true;
    needReSyncPosition_ = true;
}

void HpaeFastSinkOutputNode::HandleZeroVolumeStopEvent()
{
    Trace trace("HpaeFastSinkOutputNode::HandleZeroVolumeStopEvent " + std::to_string(GetSessionId()));
    if (!isStarted_) {
        AUDIO_INFO_LOG("fast sink already stopped");
        return;
    }
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    StopUpdateThread();
    int32_t ret = audioRendererSink_->Stop();
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop hpae fast sink success");
        isStarted_ = false;
        return;
    }
    AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop hpae fast sink failed");
    isStarted_ = true;
}

void HpaeFastSinkOutputNode::ResetZeroVolumeState()
{
    zeroVolumeStartTime_ = INT64_MAX;
    zeroVolumeState_ = INACTIVE;
}

void HpaeFastSinkOutputNode::SetLoopbackState(bool isExistLoopback)
{
    isExistLoopback_ = isExistLoopback;
    AUDIO_INFO_LOG("SetLoopbackState:%{public}d sessionId:%{public}u", isExistLoopback, GetSessionId());
}

void HpaeFastSinkOutputNode::SetMuteForSwitchDevice(bool mute)
{
    if (mute == switchDevicesMute_) {
        return;
    }
    AUDIO_INFO_LOG("SetMuteForSwitchDevice mute:%{public}d sessionId:%{public}u", mute, GetSessionId());
    switchDevicesMute_ = mute;
    Trace trace("HpaeFastSinkOutputNode::SwitchDeviceMute sessionId:" + std::to_string(GetSessionId()) +
        " mute:" + (mute ? "start" : "end"));
    trace.End();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
