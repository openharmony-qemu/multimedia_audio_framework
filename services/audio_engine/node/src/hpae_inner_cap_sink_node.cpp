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
#define LOG_TAG "HpaeInnerCapSinkNode"
#endif

#include <ctime>
#include "hpae_inner_cap_sink_node.h"
#include "hpae_format_convert.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr auto DEFAULT_NANO_SECONDS = std::chrono::nanoseconds(20000000); // 20000000ns = 20ms

HpaeInnerCapSinkNode::HpaeInnerCapSinkNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), outputStream_(this),
      pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate), silenceData_(pcmBufferInfo_)
{
    silenceData_.Reset();
    historyTime_ = std::chrono::high_resolution_clock::now();
    sleepTime_ = std::chrono::nanoseconds(0);
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeInnerCapSinkNode_bit_" +
                       std::to_string(SAMPLE_F32LE) + "_ch_" + std::to_string(GetChannelCount()) +
                       "_rate_" + std::to_string(GetSampleRate()) + ".pcm");
#endif
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeInnerCapSinkNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, 0, GetNodeInfo());
    }
#endif
}

HpaeInnerCapSinkNode::~HpaeInnerCapSinkNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeInnerCapSinkNode::DoProcess()
{
    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeInnerCapSinkNode::DoProcess " + GetTraceInfo());
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    if (outputVec.empty() || isMute_ == true) {
        outputStream_.WriteDataToOutput(&silenceData_);
#ifdef ENABLE_HOOK_PCM
        if (outputPcmDumper_) {
            outputPcmDumper_->Dump((int8_t *)silenceData_.GetPcmDataBuffer(), GetChannelCount() *
                GetFrameLen() * GetSizeFromFormat(SAMPLE_F32LE));
        }
#endif
    } else {
        HpaePcmBuffer *outputData = outputVec.front();
#ifdef ENABLE_HOOK_PCM
        if (outputPcmDumper_) {
            outputPcmDumper_->Dump((int8_t *)outputData->GetPcmDataBuffer(), GetChannelCount() *
                GetFrameLen() * GetSizeFromFormat(SAMPLE_F32LE));
        }
#endif
    // no need convert
        outputStream_.WriteDataToOutput(outputVec[0]);
    }
    // sleep
    endTime_ = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::nanoseconds>(DEFAULT_NANO_SECONDS -
        (endTime_ - historyTime_ - sleepTime_)));
    AUDIO_DEBUG_LOG("sleeptime : %{public} " PRIi64"", static_cast<int64_t>(std::chrono::duration_cast
        <std::chrono::nanoseconds>(DEFAULT_NANO_SECONDS - (endTime_ - historyTime_ - sleepTime_)).count()));
    if (static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(DEFAULT_NANO_SECONDS -
        (endTime_ - historyTime_ - sleepTime_)).count()) <= 0) {
        sleepTime_ = std::chrono::nanoseconds(0);
    } else {
        sleepTime_ = DEFAULT_NANO_SECONDS - (endTime_ - historyTime_ - sleepTime_);
    }
    historyTime_ = endTime_;
}

bool HpaeInnerCapSinkNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeInnerCapSinkNode::ResetAll()
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

// todo
std::shared_ptr<HpaeNode> HpaeInnerCapSinkNode::GetSharedInstance()
{
    return shared_from_this();
}

// todo
OutputPort<HpaePcmBuffer *> *HpaeInnerCapSinkNode::GetOutputPort()
{
    return &outputStream_;
}

void HpaeInnerCapSinkNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("Connect");
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, GetNodeId(), preNode->GetSharedInstance()->GetNodeInfo());
    }
#endif
}

void HpaeInnerCapSinkNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("DisConnect");
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        auto preNodeReal = preNode->GetSharedInstance();
        callback->OnNotifyDfxNodeInfo(false, preNodeReal->GetNodeId(), preNodeReal->GetNodeInfo());
    }
#endif
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkInit()
{
    AUDIO_INFO_LOG("Init");
    SetSinkState(STREAM_MANAGER_IDLE);
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkDeInit()
{
    AUDIO_INFO_LOG("DeInit");
    SetSinkState(STREAM_MANAGER_RELEASED);
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkFlush()
{
    AUDIO_INFO_LOG("Flush");
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkPause()
{
    AUDIO_INFO_LOG("Pause");
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkReset()
{
    AUDIO_INFO_LOG("Reset");
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkResume()
{
    AUDIO_INFO_LOG("Resume");
    SetSinkState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkStart()
{
    AUDIO_INFO_LOG("Start");
    SetSinkState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeInnerCapSinkNode::InnerCapturerSinkStop()
{
    AUDIO_INFO_LOG("Stop");
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

StreamManagerState HpaeInnerCapSinkNode::GetSinkState(void)
{
    return state_;
}

int32_t HpaeInnerCapSinkNode::SetSinkState(StreamManagerState sinkState)
{
    AUDIO_INFO_LOG("Sink[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sinkState).c_str());
    state_ = sinkState;
    return SUCCESS;
}

void HpaeInnerCapSinkNode::SetMute(bool isMute)
{
    if (isMute_ != isMute) {
        isMute_ = isMute;
        AUDIO_INFO_LOG("SetMute: %{public}d", isMute);
    }
}

size_t HpaeInnerCapSinkNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

size_t HpaeInnerCapSinkNode::GetOutputPortNum()
{
    return outputStream_.GetInputNum();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS