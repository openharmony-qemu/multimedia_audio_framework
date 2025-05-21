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

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr int32_t MAX_CACHE_SIZE = 500;
static constexpr int32_t DEFAULT_FRAME_LEN_MS = 20;
static constexpr int32_t MS_PER_SECOND = 1000;

HpaeCoBufferNode::HpaeCoBufferNode(HpaeNodeInfo& nodeInfo, int32_t& delay)
    : HpaeNode(nodeInfo), outputStream_(this),
      pcmBufferInfo_(STEREO, 960, SAMPLE_RATE_48000), coBufferOut_(pcmBufferInfo_)
{
    AUDIO_INFO_LOG("HpaeCoBufferNode created, delay: %{public}d", delay);
    delay_ = delay;
}

// according to latency
void HpaeCoBufferNode::SetBufferSize(size_t size)
{
    if (size == 0) {
        return;
    }
    if (ringCache_ == nullptr) {
        AUDIO_INFO_LOG("Create ring cache, size: %{public}zu", size);
        ringCache_ = AudioRingCache::Create(size);
        CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "Create ring cache failed");
    } else {
        OptResult result = ringCache_->ReConfig(size);
        AUDIO_INFO_LOG("ReConfig ring cache, size: %{public}zu", size);
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "ReConfig ring cache failed");
    }
    // todo set latency
    int32_t offset;
    while (offset < delay_) {
        OptResult result = ringCache_->GetWritableSize();
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
        size_t writeLen = buffer->GetFrameLen() * buffer->GetChannelCount() *
            sizeof(float) * DEFAULT_FRAME_LEN_MS / MS_PER_SECOND;
        memset_s(coBufferOut_.GetPcmDataBuffer(), writeLen, 0, writeLen);
        CHECK_AND_RETURN_LOG(result.size >= writeLen,
            "Get writable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
            result.size, writeLen);
        BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(coBufferOut_.GetPcmDataBuffer()), writeLen};
        result = ringCache_->Enqueue(bufferWrap);
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue data failed");
        offset += 20;
    }
}

void HpaeCoBufferNode::Enqueue(HpaePcmBuffer* buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
    size_t writeLen = buffer->GetFrameLen() * buffer->GetChannelCount() * sizeof(float);
    CHECK_AND_RETURN_LOG(result.size >= writeLen,
        "Get writable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, writeLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(buffer->GetPcmDataBuffer()), writeLen};
    result = ringCache_->Enqueue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue data failed");
}

void HpaeCoBufferNode::DoProcess()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get readable size failed");
    size_t requesetDataLen = SAMPLE_RATE_48000 * static_cast<int32_t>(STEREO) *
        sizeof(float) * DEFAULT_FRAME_LEN_MS / MS_PER_SECOND;
    CHECK_AND_RETURN_LOG(result.size >= requesetDataLen,
        "Get readable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, requesetDataLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(coBufferOut_.GetPcmDataBuffer()), requesetDataLen};
    result = ringCache_->Dequeue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Dequeue data failed");
    outputStream_.WriteDataToOutput(&coBufferOut_);
}

bool HpaeCoBufferNode::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeCoBufferNode::ResetAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
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
    std::lock_guard<std::mutex> lock(mutex_);
    return shared_from_this();
}

OutputPort<HpaePcmBuffer*>* HpaeCoBufferNode::GetOutputPort()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return &outputStream_;
}

void HpaeCoBufferNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("HpaeCoBufferNode connect to preNode");
    HpaeNodeInfo &preNodeInfo = preNode->GetNodeInfo();
    HpaeNodeInfo nodeInfo = preNodeInfo;
    nodeInfo.nodeName = "HpaeCoBufferNode";
    SetNodeInfo(nodeInfo);
    SetBufferSize(nodeInfo.samplingRate * nodeInfo.channels * sizeof(float) * MAX_CACHE_SIZE / MS_PER_SECOND);
    inputStream_.Connect(GetSharedInstance(), preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
}

void HpaeCoBufferNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("HpaeCoBufferNode disconnect from preNode");
    inputStream_.DisConnect(preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
}

// todo delete
size_t HpaeCoBufferNode::GetPreOutNum()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return inputStream_.GetPreOutputNum();
}

size_t HpaeCoBufferNode::GetOutputPortNum()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return outputStream_.GetInputNum();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS