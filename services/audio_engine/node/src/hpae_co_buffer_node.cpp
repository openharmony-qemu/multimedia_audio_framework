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
#include "hpae_cobuffer_node.h"
#include "hpae_define.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeCoBufferNode::HpaeCoBufferNode(HpaeNodeInfo& nodeInfo)
    : HpaeNode(nodeInfo), outputStream_(this),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate), silenceData_(pcmBufferInfo_)
      
{
    silenceData_.Reset();
}

// according latency
void HpaeCoBufferNode::SetBufferSize(size_t size)
{
    if (size == 0) {
        return;
    }
    if (ringCache_ == nullptr) {
        ringCache_ = AudioRingCache::Create(size);
        CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "Create ring cache failed");
    } else {
        OptResult = ringCache_->ReConfig(size);
        CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "ReConfig ring cache failed");
    }
}

void HpaeCoBufferNode::Enqueue(const std::vector<HpaePcmBuffer *> &inputs)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 1. 获取ringcache的可写大小
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get writable size failed");
    // 2. 从ringcache中写入数据
    size_t requesetDataLen = inputs[0]->GetFrameLen() * inputs[0]->GetChannelCount() * sizeof(float);
    float *data = inputs[0]->GetPcmDataBuffer();
    CHECK_AND_RETURN_LOG(result.size >= requesetDataLen,
        "Get writable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, requesetDataLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(data), requesetDataLen};
    result = ringCache_->Enqueue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Enqueue data failed");
}

void HpaeCoBufferNode::DoProcess()
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 查询ringcache的可读大小
    CHECK_AND_RETURN_LOG(ringCache_ != nullptr, "ring cache is null");
    OptResult result = ringCache_->GetReadableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Get readable size failed");
    // 3. 从ringcache中读取数据
    size_t requesetDataLen = 20ms;
    float *data = nullptr;
    CHECK_AND_RETURN_LOG(result.size >= requesetDataLen,
        "Get readable size is not enough, size is %{public}zu, requestDataLen is %{public}zu",
        result.size, requesetDataLen);
    BufferWrap bufferWrap = {reinterpret_cast<uint8_t *>(data), requesetDataLen};
    result = ringCache_->Dequeue(bufferWrap);
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "Dequeue data failed");
    PcmBuffer tempOut(pcmBufferInfo_);
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
    inputStream_.Connect(GetSharedInstance(), preNode->GetOutputPort(), HPAE_BUFFER_TYPE_COBUFFER);
}

void HpaeCoBufferNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
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