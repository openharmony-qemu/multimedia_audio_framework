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
#define LOG_TAG "AudioSuitePorcessNode"
#endif

#include <vector>
#include <memory>
#include "audio_suite_process_node.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"
#include "media_monitor_info.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuiteProcessNode::AudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat)
    : AudioNode(nodeType, audioFormat)
{
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    CHECK_AND_RETURN_LOG((audioSuiteCapabilities.GetNodeCapability(nodeType, nodeCapability) == SUCCESS),
        "node: %{public}d GetNodeCapability failed.", nodeType);
}

int32_t AudioSuiteProcessNode::DoProcess()
{
    if (GetAudioNodeDataFinishedFlag()) {
        AUDIO_DEBUG_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
        return SUCCESS;
    }
    AudioSuitePcmBuffer* tempOut = nullptr;
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadProcessNodePreOutputData();
    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());

        // for dfx
        auto startTime = std::chrono::steady_clock::now();

        Trace trace("AudioSuiteProcessNode::SignalProcess Start");
        tempOut = SignalProcess(preOutputs);
        trace.End();
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("node %{public}d do SignalProcess failed, return a nullptr", GetNodeType());
            return ERR_OPERATION_FAILED;
        }

        // for dfx
        auto endTime = std::chrono::steady_clock::now();
        auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        CheckEffectNodeProcessTime(tempOut->GetDataDuration(), static_cast<uint64_t>(processDuration));
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
        tempOut = preOutputs[0];
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("node %{public}d get a null pcmbuffer from prenode", GetNodeType());
            return ERR_INVALID_READ;
        }
    } else {
        AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    AUDIO_DEBUG_LOG("node type = %{public}d set "
        "pcmbuffer IsFinished: %{public}d.", GetNodeType(), GetAudioNodeDataFinishedFlag());
    tempOut->SetIsFinished(GetAudioNodeDataFinishedFlag());
    outputStream_.WriteDataToOutput(tempOut);
    return SUCCESS;
}

std::vector<AudioSuitePcmBuffer*>& AudioSuiteProcessNode::ReadProcessNodePreOutputData()
{
    bool isFinished = true;
    auto& preOutputs = inputStream_.getInputDataRef();
    preOutputs.clear();
    auto& preOutputMap = inputStream_.GetPreOutputMap();

    for (auto& o : preOutputMap) {
        if (o.first == nullptr || !o.second) {
            AUDIO_ERR_LOG("node %{public}d has a invalid connection with prenode, "
                "node connection error.", GetNodeType());
            continue;
        }
        if (finishedPrenodeSet.find(o.second) != finishedPrenodeSet.end()) {
            AUDIO_DEBUG_LOG("current node type is %{public}d, it's prenode type = %{public}d is "
                "finished, skip this outputport.", GetNodeType(), o.second->GetNodeType());
            continue;
        }
        std::vector<AudioSuitePcmBuffer *> outputData = o.first->PullOutputData(
            GetAudioNodeInPcmFormat(), !GetNodeBypassStatus());
        if (!outputData.empty() && (outputData[0] != nullptr)) {
            if (outputData[0]->GetIsFinished()) {
                finishedPrenodeSet.insert(o.second);
            }
            isFinished = isFinished && outputData[0]->GetIsFinished();
            preOutputs.insert(preOutputs.end(), outputData.begin(), outputData.end());
        }
    }
    AUDIO_DEBUG_LOG("set node type = %{public}d isFinished status: %{public}d.", GetNodeType(), isFinished);
    SetAudioNodeDataFinishedFlag(isFinished);
    return preOutputs;
}

int32_t AudioSuiteProcessNode::Flush()
{
    // for dfx
    CheckEffectNodeOvertimeCount();

    CHECK_AND_RETURN_RET_LOG(DeInit() == SUCCESS, ERROR, "DeInit failed");
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "Init failed");
    if (!paraName_.empty() && !paraValue_.empty()) {
        SetOptions(paraName_, paraValue_);
    }
    finishedPrenodeSet.clear();
    outputStream_.resetResampleCfg();
    AUDIO_INFO_LOG("Flush SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::InitOutputStream()
{
    outputStream_ = OutputPort<AudioSuitePcmBuffer*>(GetSharedInstance());
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::Connect(const std::shared_ptr<AudioNode>& preNode)
{
    if (!preNode) {
        AUDIO_ERR_LOG("node type = %{public}d preNode is null!", GetNodeType());
        return ERR_INVALID_PARAM;
    }
    CHECK_AND_RETURN_RET_LOG(preNode->GetOutputPort() != nullptr, ERROR, "OutputPort is null");
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::DisConnect(const std::shared_ptr<AudioNode>& preNode)
{
    if (!preNode) {
        AUDIO_ERR_LOG("node type = %{public}d preNode is null!", GetNodeType());
        return ERR_INVALID_PARAM;
    }
    inputStream_.DisConnect(preNode);
    return SUCCESS;
}

void AudioSuiteProcessNode::CheckEffectNodeProcessTime(uint32_t dataDurationMS, uint64_t processDurationUS)
{
    if (dataDurationMS == 0) {
        AUDIO_WARNING_LOG("Invalid para, data duration is 0.");
        return;
    }

    signalProcessTotalCount_++;

    // for dfx, overtime counter add when realtime factor exceeds the threshold
    uint64_t dataDurationUS = static_cast<uint64_t>(dataDurationMS) * MILLISECONDS_TO_MICROSECONDS;
    for (size_t i = 0; i < RTF_OVERTIME_LEVELS; ++i) {
        uint64_t thresholdValue = dataDurationUS * nodeCapability.realtimeFactor * RTF_OVERTIME_THRESHOLDS[i];
        if (processDurationUS >= thresholdValue) {
            rtfOvertimeCounters_[i]++;
        }
    }

    // count for RTF of node exceeds 100%
    if (processDurationUS >= dataDurationUS) {
        rtfOver100Count_++;
    }
}

void AudioSuiteProcessNode::CheckEffectNodeOvertimeCount()
{
    std::string pipelineWorkMode = (GetAudioNodeWorkMode() == PIPELINE_REALTIME_MODE) ? "Realtime mode" : "Edit mode";
    AUDIO_INFO_LOG("[%{public}s] - [%{public}s] effect node realtimeFactor overtime counters(1.0, 1.1, 1.2, 100%%): "
                   "%{public}d, %{public}d, %{public}d, %{public}d, signalProcess total count: %{public}d.",
        pipelineWorkMode.c_str(),
        GetNodeTypeString().c_str(),
        rtfOvertimeCounters_[RtfOvertimeLevel::OVER_BASE],
        rtfOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE],
        rtfOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE],
        rtfOver100Count_,
        signalProcessTotalCount_);

    bool allOvertimeCounterZero = std::all_of(
        std::begin(rtfOvertimeCounters_),
        std::end(rtfOvertimeCounters_),
        [](int32_t count) { return count == 0; }
    );
    if (!allOvertimeCounterZero || rtfOver100Count_ != 0) {
        // report SuiteEngineUtilizationStats event
        std::shared_ptr<Media::MediaMonitor::EventBean> bean =
            std::make_shared<Media::MediaMonitor::EventBean>(Media::MediaMonitor::ModuleId::AUDIO,
                Media::MediaMonitor::EventId::SUITE_ENGINE_UTILIZATION_STATS,
                Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);

        bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
        bean->Add("AUDIO_NODE_TYPE", GetNodeTypeString());
        if (GetAudioNodeWorkMode() == PIPELINE_REALTIME_MODE) {
            bean->Add("RT_MODE_RENDER_COUNT", signalProcessTotalCount_);
            bean->Add("RT_MODE_RTF_OVER_BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
            bean->Add("RT_MODE_RTF_OVER_110BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE]);
            bean->Add("RT_MODE_RTF_OVER_120BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE]);
            bean->Add("RT_MODE_RTF_OVER_100_COUNT", rtfOver100Count_);
        } else {
            bean->Add("EDIT_MODE_RENDER_COUNT", signalProcessTotalCount_);
            bean->Add("EDIT_MODE_RTF_OVER_BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_110BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_120BASE_COUNT", rtfOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_100_COUNT", rtfOver100Count_);
        }
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);

        AUDIO_WARNING_LOG(
            "effect node [%{public}s] run signalProcess overtime, report SuiteEngineUtilizationStats event.",
            GetNodeTypeString().c_str());
    }

    // reset counter
    signalProcessTotalCount_ = 0;
    rtfOver100Count_ = 0;
    std::fill(rtfOvertimeCounters_.begin(), rtfOvertimeCounters_.end(), 0);
}

}
}
}