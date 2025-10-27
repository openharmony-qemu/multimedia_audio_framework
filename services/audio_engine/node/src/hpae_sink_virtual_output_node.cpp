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
#define LOG_TAG "HpaeSinkVirtualOutputNode"
#endif

#include "hpae_sink_virtual_output_node.h"
#include "audio_common_utils.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t DEFAULT_RING_BUFFER_NUM = 1;
static constexpr uint32_t MS_PER_SECOND = 1000;

HpaeSinkVirtualOutputNode::HpaeSinkVirtualOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), outputStream_(this),
      renderFrameData_(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format)),
      pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
      outputAudioBuffer_(pcmBufferInfo_)
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeSinkVirtualOutputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    ringCache_ = AudioRingCache::Create(GetRingCacheSize());
    if (ringCache_ == nullptr) {
        AUDIO_ERR_LOG("ringCache create fail");
    }
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeSinkVirtualOutputNode_id_" + std::to_string(GetSessionId()) + "_nodeId_" + std::to_string(GetNodeId()) +
        "_ch_" + std::to_string(GetChannelCount()) +
        "_rate_" + std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");

#endif
}

HpaeSinkVirtualOutputNode::~HpaeSinkVirtualOutputNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeSinkVirtualOutputNode::DoRenderProcess()
{
    Trace trace("HpaeSinkVirtualOutputNode::DoRenderProcess " + GetTraceInfo());
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    CHECK_AND_RETURN(!outputVec.empty());
    HpaePcmBuffer *outputData = outputVec.front();

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->CheckAndReopenHandle();
        outputPcmDumper_->Dump((int8_t *)outputData->GetPcmDataBuffer(),
            outputData->GetFrameLen() * sizeof(float) * outputData->GetChannelCount());
    }
#endif

    OptResult result = ringCache_->Enqueue(
        {reinterpret_cast<uint8_t *>(outputData->GetPcmDataBuffer()), outputData->DataSize()});
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "ringCache enqueue fail");
}

void HpaeSinkVirtualOutputNode::DoProcess()
{
    Trace trace("HpaeSinkVirtualOutputNode::DoProcess " + GetTraceInfo());
    std::lock_guard<std::mutex> lock(mutex_);
    DoProcessInner();
}

void HpaeSinkVirtualOutputNode::DoProcessInner()
{
    Trace trace("HpaeSinkVirtualOutputNode::DoProcessInner " + GetTraceInfo());
    OptResult result = ringCache_->Dequeue(
        {reinterpret_cast<uint8_t *>(outputAudioBuffer_.GetPcmDataBuffer()), outputAudioBuffer_.DataSize()});
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "ringCache dequeue fail");
    outputStream_.WriteDataToOutput(&outputAudioBuffer_);
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyQueue();
    }
}

int32_t HpaeSinkVirtualOutputNode::PeekAudioData(uint8_t *buffer, const size_t &bufferSize,
    AudioStreamInfo &streamInfo)
{
    Trace trace("HpaeSinkVirtualOutputNode::PeekAudioData " + GetTraceInfo());
    std::lock_guard<std::mutex> lock(mutex_);
    DoProcessInner();
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERROR_INVALID_PARAM, "Invalid nullptr buffer provided");
    memset_s(buffer, bufferSize, 0, bufferSize);
    size_t outputSizeInt = outputAudioBuffer_.DataSize() * GetSizeFromFormat(GetBitWidth()) / sizeof(float);
    if (bufferSize > outputSizeInt) {
        AUDIO_WARNING_LOG("peek buffersize[%{public}zu] > sinnVirtualOutputNode bufferSize[%{public}zu]!",
            bufferSize, outputSizeInt);
    } else if (bufferSize < outputSizeInt) {
        AUDIO_WARNING_LOG("peek buffersize[%{public}zu] < sinnVirtualOutputNode bufferSize[%{public}zu]!",
            bufferSize, outputSizeInt);
    }
    uint64_t length = bufferSize / GetBitWidth();
    ConvertFromFloat(GetBitWidth(), std::min(static_cast<uint64_t>(GetChannelCount() * GetFrameLen()), length),
        outputAudioBuffer_.GetPcmDataBuffer(), buffer);

    streamInfo.format = GetBitWidth();
    streamInfo.samplingRate = GetSampleRate();
    streamInfo.channels = GetChannelCount();
    return SUCCESS;
}

bool HpaeSinkVirtualOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeSinkVirtualOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

std::shared_ptr<HpaeNode> HpaeSinkVirtualOutputNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<HpaePcmBuffer *> *HpaeSinkVirtualOutputNode::GetOutputPort()
{
    return &outputStream_;
}

void HpaeSinkVirtualOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, GetNodeId(), preNode->GetSharedInstance()->GetNodeId());
    }
#endif
}

void HpaeSinkVirtualOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        auto preNodeReal = preNode->GetSharedInstance();
        callback->OnNotifyDfxNodeInfo(false, GetNodeId(), preNodeReal->GetNodeId());
    }
#endif
}

StreamManagerState HpaeSinkVirtualOutputNode::GetState()
{
    return state_;
}

int32_t HpaeSinkVirtualOutputNode::RenderSinkInit()
{
    CHECK_AND_RETURN_RET_LOG(ringCache_ != nullptr, ERR_INVALID_OPERATION, "init fail, ringcache is null");
    SetSinkState(STREAM_MANAGER_IDLE);
    return SUCCESS;
}

int32_t HpaeSinkVirtualOutputNode::RenderSinkDeInit()
{
    SetSinkState(STREAM_MANAGER_RELEASED);
    return SUCCESS;
}

int32_t HpaeSinkVirtualOutputNode::RenderSinkStart(void)
{
    SetSinkState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSinkVirtualOutputNode::RenderSinkStop(void)
{
    if (ringCache_ != nullptr) {
        ringCache_->ResetBuffer();
    }
    SilenceData();
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

void HpaeSinkVirtualOutputNode::SilenceData()
{
    void *data = outputAudioBuffer_.GetPcmDataBuffer();
    CHECK_AND_RETURN_LOG(data != nullptr, "outputAudioBuffer_ data is null");
    if (GetNodeInfo().format == INVALID_WIDTH) {
        AUDIO_WARNING_LOG("HpaePcmBuffer.SetDataSilence: invalid format");
    } else if (GetNodeInfo().format == SAMPLE_U8) {
        // set silence data for all the frames
        memset_s(data, outputAudioBuffer_.Size(), 0x80, outputAudioBuffer_.Size());
    } else {
        memset_s(data, outputAudioBuffer_.Size(), 0, outputAudioBuffer_.Size());
    }
}

size_t HpaeSinkVirtualOutputNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

int32_t HpaeSinkVirtualOutputNode::SetSinkState(StreamManagerState sinkState)
{
    AUDIO_INFO_LOG("Sink[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sinkState).c_str());
    state_ = sinkState;
    return SUCCESS;
}

uint32_t HpaeSinkVirtualOutputNode::GetLatency()
{
    return 0;
}

bool HpaeSinkVirtualOutputNode::GetIsReadFinished()
{
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, false,
        "ringCache get writable invalid size : %{public}zu", result.size);
    return result.size != 0;
}

int32_t HpaeSinkVirtualOutputNode::ReloadNode(HpaeNodeInfo nodeInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    nodeInfo.nodeId = GetNodeId(); // not change nodeId
    SetNodeInfo(nodeInfo);
    pcmBufferInfo_ = PcmBufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate);
    outputAudioBuffer_.ReConfig(pcmBufferInfo_);
    size_t size = GetRingCacheSize();
    if (ringCache_ == nullptr) {
        ringCache_ = AudioRingCache::Create(size);
    } else {
        ringCache_->ReConfig(size, false);
    }
    return SUCCESS;
}

size_t HpaeSinkVirtualOutputNode::GetRingCacheSize()
{
    size_t frameBytes = static_cast<size_t>(GetSizeFromFormat(SAMPLE_F32LE)) * GetSampleRate() *
        FRAME_LEN_20MS / MS_PER_SECOND * GetChannelCount();
    return DEFAULT_RING_BUFFER_NUM * frameBytes;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
