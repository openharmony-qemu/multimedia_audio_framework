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
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuiteProcessNode::AudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat)
    : AudioNode(nodeType, audioFormat)
{
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    CHECK_AND_RETURN_LOG((audioSuiteCapabilities.GetNodeParameter(nodeType, nodeParameter) == SUCCESS),
        "node: %{public}d GetNodeParameter failed.", nodeType);
}

AudioSuiteProcessNode::AudioSuiteProcessNode(AudioNodeType nodeType)
    : AudioNode(nodeType)
{
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    CHECK_AND_RETURN_LOG((audioSuiteCapabilities.GetNodeParameter(nodeType, nodeParameter) == SUCCESS),
        "node: %{public}d GetNodeParameter failed.", nodeType);
    resultNumber = 1;
}

int32_t AudioSuiteProcessNode::CalculationNeedBytes(uint32_t frameLengthMs)
{
    uint32_t dataBytes = 0;
    PcmBufferFormat pcmFormat = GetAudioNodeInPcmFormat();
    dataBytes = (frameLengthMs * pcmFormat.sampleRate / SECONDS_TO_MS) * pcmFormat.channelCount *
                AudioSuiteUtil::GetSampleSize(pcmFormat.sampleFormat);
    return dataBytes;
}

std::vector<AudioSuitePcmBuffer *> AudioSuiteProcessNode::SignalProcess(
    const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    std::vector<AudioSuitePcmBuffer *> retError{ nullptr };
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, retError, "algoInterface_ is nullptr, need Init first");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), retError, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, retError, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), retError, "Invalid input format");

    algorithmInput_[0] = inputs[0]->GetPcmData();
    int32_t ret =
        algoInterface_->Apply(algorithmInput_, algorithmOutput_);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, retError, "Node SignalProcess Apply failed");

    return retPcmBuffer;
}

int32_t AudioSuiteProcessNode::InitCacheLength(uint32_t needDataLength)
{
    frameOutBytes = CalculationNeedBytes(pcmDurationMs_);
    algoOutPcmBuffer_.resize(resultNumber);
    outputPcmBuffer.resize(resultNumber);
    cachedBuffer.resize(resultNumber);

    for (size_t idx = 0; idx < outputPcmBuffer.size(); ++idx) {
        int32_t ret = outputPcmBuffer[idx].ResizePcmBuffer(
            GetAudioNodeInPcmFormat(), needDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Target buffer allocation failed.");
        CHECK_AND_RETURN_RET_LOG(
            outputPcmBuffer[idx].GetPcmData() != nullptr, ERROR, "The target buffer pointer is null.");

        uint32_t needByteLength = 
            outputPcmBuffer[idx].GetDataSize() >= frameOutBytes ? outputPcmBuffer[idx].GetDataSize() : frameOutBytes;
        ret = cachedBuffer[idx].ResizeBuffer(needByteLength * 2);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Circular buffer allocation failed.");

        ret = algoOutPcmBuffer_[idx].ResizePcmBuffer(GetAudioNodeInPcmFormat(), pcmDurationMs_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Algorithm result buffer allocation failed.");
        CHECK_AND_RETURN_RET_LOG(
            algoOutPcmBuffer_[idx].GetPcmData() != nullptr, ERROR, "The algorithm result buffer pointer is empty.");

        algoOutPcmBuffer_[idx].ResizePcmBuffer(frameOutBytes);
        CHECK_AND_RETURN_RET_LOG(algoOutPcmBuffer_[idx].GetDataSize() == CalculationNeedBytes(pcmDurationMs_),
            ERROR,
            "Target buffer size error.");

        algorithmOutput_.emplace_back(algoOutPcmBuffer_[idx].GetPcmData());
        retPcmBuffer.emplace_back(&algoOutPcmBuffer_[idx]);
        needCache = (!algoOutPcmBuffer_[idx].IsSameLength(outputPcmBuffer[idx].GetDataSize()));
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ObtainProcessedData()
{
    preOutputs = ReadProcessNodePreOutputData();

    if (preOutputs.empty()) {
         AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
         return ERROR;
    }

    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
         AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());

         // for dfx
         auto startTime = std::chrono::steady_clock::now();

         Trace trace("AudioSuiteProcessNode::SignalProcess Start");
         algoRetPcmBuffer = SignalProcess(preOutputs);
         trace.End();

         // for dfx
         auto endTime = std::chrono::steady_clock::now();
         auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
         CheckEffectNodeProcessTime(algoRetPcmBuffer[0]->GetDataDuration(), static_cast<uint64_t>(processDuration));
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ProcessedDataToNextNode()
{
    if (needCache) {
         while (outputPcmBuffer[0].GetDataSize() > cachedBuffer[0].GetSize() && !GetAudioNodeDataFinishedFlag()) {
             int32_t ret = ObtainProcessedData();
             CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to retrieve data from the preceding node.");
             for (size_t idx = 0; idx < outputPcmBuffer.size(); ++idx) {
                 CHECK_AND_RETURN_RET_LOG(
                     algoRetPcmBuffer[idx] != nullptr && algoRetPcmBuffer[idx]->GetPcmData() != nullptr,
                     ERR_OPERATION_FAILED,
                     "node %{public}d do SignalProcess failed, return a nullptr.",
                     GetNodeType());

                 cachedBuffer[idx].PushData(algoRetPcmBuffer[idx]->GetPcmData(), frameOutBytes);
             }
         }
         for (size_t idx = 0; idx < outputPcmBuffer.size(); ++idx) {
            outputPcmBuffer[idx].Reset();
             uint32_t CopyDataLength = cachedBuffer[idx].GetSize() <= outputPcmBuffer[idx].GetDataSize()
                                           ? cachedBuffer[idx].GetSize()
                                           : outputPcmBuffer[idx].GetDataSize();
             int32_t ret = cachedBuffer[idx].GetData(outputPcmBuffer[idx].GetPcmData(),
                 CopyDataLength);
             CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get data from cachedBuffer fail");
             outputPcmBuffer[idx].SetIsFinished(GetAudioNodeDataFinishedFlag() && cachedBuffer[idx].GetSize() == 0);
             outputStream_.WriteDataToOutput(&outputPcmBuffer[idx]);
         }
    } else {
         int32_t ret = ObtainProcessedData();
         CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to retrieve data from the preceding node.");

         for (size_t idx = 0; idx < outputPcmBuffer.size(); ++idx) {
            outputPcmBuffer[idx].Reset();
            CHECK_AND_RETURN_RET_LOG(
                     algoRetPcmBuffer[idx] != nullptr && algoRetPcmBuffer[idx]->GetPcmData() != nullptr,
                     ERR_OPERATION_FAILED,
                     "node %{public}d do SignalProcess failed, return a nullptr.",
                     GetNodeType());
             CHECK_AND_RETURN_RET_LOG(algoRetPcmBuffer[idx]->GetDataSize() <= outputPcmBuffer[idx].GetDataSize(),
                 ERROR,
                 "Insufficient target buffer size.");
             int32_t ret = memcpy_s(outputPcmBuffer[idx].GetPcmData(),
                 outputPcmBuffer[idx].GetDataSize(),  // Copy the first frame 20ms data
                 algoRetPcmBuffer[idx]->GetPcmData(),
                 algoRetPcmBuffer[idx]->GetDataSize());
             CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memcpy failed, ret is %{public}d.", ret);
             outputPcmBuffer[idx].SetIsFinished(GetAudioNodeDataFinishedFlag());
             outputStream_.WriteDataToOutput(&outputPcmBuffer[idx]);
         }
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::DoProcess(uint32_t needDataLength)
{
    CHECK_AND_RETURN_RET_LOG(needDataLength <= maxRequestLength, ERROR, "Request data length error.");
    if (GetAudioNodeDataFinishedFlag() && cachedBuffer[0].GetSize() == 0) {
        AUDIO_DEBUG_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
        return SUCCESS;
    }

    if (!secondCall) {
        int32_t ret = InitCacheLength(needDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to initialize buffer.");
        secondCall = true;
    }

    if ((GetNodeBypassStatus() == true)) {
        pcmDurationMs_ = needDataLength;
         preOutputs = ReadProcessNodePreOutputData();
         if (!preOutputs.empty()) {
             AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
             for (size_t idx = 0; idx < outputPcmBuffer.size(); ++idx) {
                 preOutputs[0]->SetIsFinished(GetAudioNodeDataFinishedFlag());
                 outputStream_.WriteDataToOutput(preOutputs[0]);
             }
         } else {
            AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
            return ERROR;
         }
         return SUCCESS;
    }

    int32_t ret = ProcessedDataToNextNode();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to process node data.");

    AUDIO_DEBUG_LOG("node type = %{public}d set "
                    "pcmbuffer IsFinished: %{public}d.",
        GetNodeType(),
        GetAudioNodeDataFinishedFlag());
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
            GetAudioNodeInPcmFormat(), !GetNodeBypassStatus(), pcmDurationMs_);
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

int32_t AudioSuiteProcessNode::SetOptions(std::string name, std::string value)
{
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterfaceImpl_ is nullptr");

    paraName_ = name;
    paraValue_ = value;

    int32_t ret = algoInterface_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed %{public}d", ret);
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::GetOptions(std::string name, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "paraValue_ is empty");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");
    value = paraValue_;

    int32_t ret = algoInterface_->GetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetParameter failed");
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::Flush()
{
    // for dfx
    CheckEffectNodeOvertimeCount();

    secondCall = false;
    needCache = false;
    pcmDurationMs_ = 20;
    frameOutBytes = 0;
    resultNumber = 1;
    cachedBuffer.resize(0);
    outputPcmBuffer.resize(0);
    algoRetPcmBuffer.resize(0);
    retPcmBuffer.resize(0);
    preOutputs.resize(0);
    nextNeedDataLength = 0;
    algorithmInput_.resize(1);
    algorithmOutput_.resize(0);
    algoInterface_ = nullptr ;

    algoOutPcmBuffer_.resize(0);

    CHECK_AND_RETURN_RET_LOG(DeInit() == SUCCESS, ERROR, "DeInit failed");
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "Init failed");
    if (!paraName_.empty() && !paraValue_.empty()) {
        SetOptions(paraName_, paraValue_);
    }
    finishedPrenodeSet.clear();
    outputStream_.ResetResampleCfg();
    AUDIO_INFO_LOG("Flush SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::InitOutputStream()
{
    CHECK_AND_RETURN_RET_LOG(GetSharedInstance() != nullptr, ERROR, "GetSharedInstance returns a nullptr");
    int32_t ret = outputStream_.SetOutputPort(GetSharedInstance());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetOutputPort failed.");
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
        uint64_t thresholdValue = dataDurationUS * nodeParameter.realtimeFactor * RTF_OVERTIME_THRESHOLDS[i];
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