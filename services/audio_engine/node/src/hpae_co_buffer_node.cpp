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
#define LOG_TAG "HpaeCoBufferNode"
#endif

#include "hpae_co_buffer_node.h"
#include "hpae_define.h"
#include "audio_effect_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr int32_t DEFAULT_FRAME_LEN = 960;
static constexpr int32_t MAX_CACHE_SIZE = 500;
static constexpr int32_t MS_PER_SECOND = 1000;
static constexpr int32_t TEST_LATENCY = 280;
static constexpr int32_t ENQUEUE_DONE_FRAME = 10;

HpaeCoBufferNode::HpaeCoBufferNode()
    : HpaeNode(),
      outputStream_(this),
      pcmBufferInfo_(STEREO, DEFAULT_FRAME_LEN, SAMPLE_RATE_48000),
      coBufferOut_(pcmBufferInfo_),
      silenceData_(pcmBufferInfo_)
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeCoBufferNode");
#endif
    const size_t size = static_cast<size_t>(SAMPLE_RATE_48000) *
                        static_cast<size_t>(STEREO) *
                        sizeof(float) *
                        static_cast<size_t>(MAX_CACHE_SIZE) /
                        static_cast<size_t>(MS_PER_SECOND);
    AUDIO_INFO_LOG("Created ring cache, size: %{public}zu", size);
    ringCache_ = AudioRingCache::Create(size);
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "Create ring cache failed");
}

HpaeCoBufferNode::~HpaeCoBufferNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeCoBufferNode::Enqueue(HpaePcmBuffer* buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
#ifdef ENABLE_HOOK_PCM
    if (inputPcmDumper_ && buffer) {
        const size_t dumpSize = buffer->GetFrameLen() * sizeof(float) * buffer->GetChannelCount();
        inputPcmDumper_->Dump(reinterpret_cast<int8_t*>(buffer->GetPcmDataBuffer()), dumpSize);
    }
#endif
    // process input buffer
    ProcessInputFrameInner(buffer);
    
    // process enqueue flag
    if (enqueueCount_ < ENQUEUE_DONE_FRAME) {
        enqueueCount_++;
    } else if (enqueueCount_ == ENQUEUE_DONE_FRAME) {
        enqueueCount_++;
        enqueueRunning_ = true;
        // fill silence frames for latency adjustment
        HILOG_COMM_INFO("Filling silence frames for latency adjustment");
        ringCache_->ResetBuffer();
        FillSilenceFramesInner(TEST_LATENCY);
    }
}

void HpaeCoBufferNode::DoProcess()
{
    std::unique_lock<std::mutex> lock(mutex_);
    
    // write silence data if enqueue is not running
    if (!enqueueRunning_) {
        outputStream_.WriteDataToOutput(&silenceData_);
        return;
    }
    
    // process output buffer
    ProcessOutputFrameInner();
    
#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_) {
        const size_t dumpSize = coBufferOut_.GetFrameLen() * sizeof(float) * coBufferOut_.GetChannelCount();
        outputPcmDumper_->Dump(reinterpret_cast<int8_t*>(coBufferOut_.GetPcmDataBuffer()), dumpSize);
    }
#endif
}

bool HpaeCoBufferNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeCoBufferNode::ResetAll()
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

std::shared_ptr<HpaeNode> HpaeCoBufferNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<HpaePcmBuffer *> *HpaeCoBufferNode::GetOutputPort()
{
    return &outputStream_;
}

void HpaeCoBufferNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo nodeInfo = preNode->GetNodeInfo();
    if (connectedProcessCluster_.find(nodeInfo.sceneType) == connectedProcessCluster_.end()) {
        connectedProcessCluster_.insert(nodeInfo.sceneType);
        nodeInfo.nodeId = GetNodeId();
        nodeInfo.nodeName = GetNodeName();
        SetNodeInfo(nodeInfo);
        inputStream_.Connect(shared_from_this(), preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
        HILOG_COMM_INFO("HpaeCoBufferNode connect to preNode");
    }

    // reset status flag
    enqueueCount_ = 1;
    enqueueRunning_ = false;
#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeCoBufferNodeInput_id_" + std::to_string(GetNodeId()) + ".pcm");
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeCoBufferNodeOutput_id_" + std::to_string(GetNodeId()) + ".pcm");
#endif
}

void HpaeCoBufferNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    HpaeNodeInfo nodeInfo = preNode->GetNodeInfo();
    if (connectedProcessCluster_.find(nodeInfo.sceneType) != connectedProcessCluster_.end()) {
        connectedProcessCluster_.erase(nodeInfo.sceneType);
        inputStream_.DisConnect(preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
        HILOG_COMM_INFO("HpaeCoBufferNode disconnected from prenode, scenetype %{public}u", nodeInfo.sceneType);
    }
}

void HpaeCoBufferNode::SetLatency(uint32_t latency)
{
    latency_ = latency;
    AUDIO_INFO_LOG("latency is %{public}d", latency);
}

void HpaeCoBufferNode::FillSilenceFramesInner(uint32_t latencyMs)
{
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "Ring cache is null");
    
    uint32_t offset = 0;
    const size_t frameSize = silenceData_.GetFrameLen() * silenceData_.GetChannelCount() * sizeof(float);
    
    while (offset < latencyMs) {
        // check writable size
        OptResult result = ringCache_->GetWritableSize();
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
        if (result.size < frameSize) {
            AUDIO_WARNING_LOG("Insufficient space for silence frame: %{public}zu < %{public}zu",
                result.size, frameSize);
            break;
        }
        
        // create silence frame
        BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(silenceData_.GetPcmDataBuffer()), frameSize};
        result = ringCache_->Enqueue(bufferWrap);
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue silence frame failed");
        offset += FRAME_LEN_20MS;
    }
    AUDIO_INFO_LOG("Filled %{public}u ms of silence frames", offset);
}

void HpaeCoBufferNode::ProcessInputFrameInner(HpaePcmBuffer* buffer)
{
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr && buffer != nullptr,
        "Ring cache or buffer is null");
    
    const size_t writeLen = buffer->GetFrameLen() * buffer->GetChannelCount() * sizeof(float);
    
    // check writable size
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
    CHECK_AND_RETURN_LOG(result.size >= writeLen,
        "Insufficient cache space: %{public}zu < %{public}zu", result.size, writeLen);
    
    // enqueue buffer
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t*>(buffer->GetPcmDataBuffer()), writeLen};
    result = ringCache_->Enqueue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue data failed");
}

void HpaeCoBufferNode::ProcessOutputFrameInner()
{
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "Ring cache is null");
    
    const size_t requestDataLen = static_cast<size_t>(SAMPLE_RATE_48000) *
                                  static_cast<size_t>(STEREO) *
                                  sizeof(float) *
                                  static_cast<size_t>(FRAME_LEN_20MS) /
                                  static_cast<size_t>(MS_PER_SECOND);
    
    // check readable size
    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get readable size failed");
    
    if (result.size < requestDataLen) {
        AUDIO_WARNING_LOG("Insufficient data: %{public}zu < %{public}zu, outputting silence",
            result.size, requestDataLen);
        outputStream_.WriteDataToOutput(&silenceData_);
    } else {
        // read buffer
        BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(coBufferOut_.GetPcmDataBuffer()), requestDataLen};
        result = ringCache_->Dequeue(bufferWrap);
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Dequeue data failed");
        if (result.ret != OPERATION_SUCCESS) {
            outputStream_.WriteDataToOutput(&silenceData_);
        } else {
            outputStream_.WriteDataToOutput(&coBufferOut_);
        }
    }
}

void HpaeCoBufferNode::SetOutputClusterConnected(bool isConnect)
{
    isOutputClusterConnected_ = isConnect;
    HILOG_COMM_INFO("HpaeCoBufferNode output cluster connected status: %{public}d", isConnect);
}

bool HpaeCoBufferNode::IsOutputClusterConnected()
{
    return isOutputClusterConnected_;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS