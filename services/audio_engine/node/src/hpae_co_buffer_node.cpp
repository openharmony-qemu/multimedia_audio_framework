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
HpaeCoBufferNode::HpaeCoBufferNode(HpaeNodeInfo& nodeInfo, int32_t& delay)
    : HpaeNode(nodeInfo), outputStream_(this)
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
}

void HpaeCoBufferNode::Enqueue(HpaePcmBuffer* buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
    size_t writeLen = buffer->GetFrameLen() * buffer->GetChannelCount() * sizeof(float);
    float *data = buffer->GetPcmDataBuffer();
    CHECK_AND_RETURN_LOG(result.size >= writeLen,
        "Get writable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, writeLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(data), writeLen};
    result = ringCache_->Enqueue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue data failed");
}

void HpaeCoBufferNode::DoProcess()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get readable size failed");
    size_t requesetDataLen = nodeInfo_.frameLen * nodeInfo_.channels * sizeof(float) * 1000 / delay_;
    float *data = nullptr;
    CHECK_AND_RETURN_LOG(result.size >= requesetDataLen,
        "Get readable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, requesetDataLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(data), requesetDataLen};
    result = ringCache_->Dequeue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Dequeue data failed");
    HpaePcmBuffer tempOut(pcmBufferInfo_);
    memcpy_s(tempOut.GetPcmDataBuffer(), requesetDataLen, bufferWrap.data, requesetDataLen);
    tempOut.GetPcmDataBuffer() = data;
    outputStream_.WriteDataToOutput(tempOut);
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

OutputPort<HpaePcmBuffer*>* HpaeCoBufferNode::GetOutputPort()
{
    return &outputStream_;
}

void HpaeCoBufferNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("HpaeCoBufferNode connect to preNode");
    HpaeNodeInfo &preNodeInfo = preNode->GetNodeInfo();
    nodeInfo_ = preNodeInfo;
    SetBufferSize(nodeInfo_.frameLen * nodeInfo_.channels * sizeof(float) * 1000 / delay_);
    inputStream_.Connect(GetSharedInstance(), preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
}

void HpaeCoBufferNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("HpaeCoBufferNode disconnect from preNode");
    inputStream_.DisConnect(preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
}

// todo delete
size_t HpaeCoBufferNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

size_t HpaeCoBufferNode::GetOutputPortNum()
{
    return outputStream_.GetInputNum();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS