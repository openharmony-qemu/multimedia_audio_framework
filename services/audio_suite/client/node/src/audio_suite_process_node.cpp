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
    CHECK_AND_RETURN_LOG((audioSuiteCapabilities.GetNodeParameter(nodeType, nodeParameter_) == SUCCESS),
        "node: %{public}d GetNodeParameter failed.", nodeType);
}

AudioSuiteProcessNode::AudioSuiteProcessNode(AudioNodeType nodeType)
    : AudioNode(nodeType)
{
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    CHECK_AND_RETURN_LOG((audioSuiteCapabilities.GetNodeParameter(nodeType, nodeParameter_) == SUCCESS),
        "node: %{public}d GetNodeParameter failed.", nodeType);
    resultNumber_ = 1;
}

uint32_t AudioSuiteProcessNode::CalculationNeedBytes(uint32_t frameLengthMs)
{
    uint32_t dataBytes = 0;
    PcmBufferFormat pcmFormat = GetAudioNodeOutPcmFormat();
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

    algoInput_[0] = inputs[0]->GetPcmData();
    int32_t ret = algoInterface_->Apply(algoInput_, algoOutput_);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, retError, "Node SignalProcess Apply failed");

    return intermediateResult_;
}

int32_t AudioSuiteProcessNode::InitCacheLength(uint32_t needDataLength)
{
    requestPreNodeDuration_ = nodeNeedDataDuration_;
    frameOutBytes_ = CalculationNeedBytes(requestPreNodeDuration_);
    algoOutPcmBuffer_.resize(resultNumber_);
    downStreamData_.resize(resultNumber_);
    cachedBuffer_.resize(resultNumber_);
    // The buffer size is twice the maximum length of the data in the current node and the next node.
    uint32_t doubleLength = 2;

    for (size_t idx = 0; idx < downStreamData_.size(); ++idx) {
        int32_t ret = downStreamData_[idx].ResizePcmBuffer(
            GetAudioNodeOutPcmFormat(), needDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Target buffer allocation failed.");
        CHECK_AND_RETURN_RET_LOG(
            downStreamData_[idx].GetPcmData() != nullptr, ERROR, "The target buffer pointer is null.");
        
        uint32_t needByteLength =
            downStreamData_[idx].GetDataSize() >= frameOutBytes_ ? downStreamData_[idx].GetDataSize() : frameOutBytes_;
        ret = cachedBuffer_[idx].ResizeBuffer(needByteLength * doubleLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Circular buffer allocation failed.");

        ret = algoOutPcmBuffer_[idx].ResizePcmBuffer(GetAudioNodeOutPcmFormat(), requestPreNodeDuration_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Algorithm result buffer allocation failed.");
        CHECK_AND_RETURN_RET_LOG(
            algoOutPcmBuffer_[idx].GetPcmData() != nullptr, ERROR, "The algorithm result buffer pointer is empty.");

        ret = algoOutPcmBuffer_[idx].ResizePcmBuffer(frameOutBytes_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Resetting byte count failed.");
        CHECK_AND_RETURN_RET_LOG(algoOutPcmBuffer_[idx].GetDataSize() == CalculationNeedBytes(requestPreNodeDuration_),
            ERROR,
            "Target buffer size error.");

        algoOutput_.emplace_back(algoOutPcmBuffer_[idx].GetPcmData());
        intermediateResult_.emplace_back(&algoOutPcmBuffer_[idx]);
        needCache_ = (!algoOutPcmBuffer_[idx].IsSameLength(downStreamData_[idx].GetDataSize()));
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ObtainProcessedData()
{
    ReadProcessNodePreOutputData();
    
    std::vector<AudioSuitePcmBuffer *> preOutputs;
    for (auto* buffer : preNodeResult_) {
        preOutputs.push_back(buffer);
    }
    
    if (preOutputs.empty()) {
        AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    if (GetNodeBypassStatus() == false) {
        AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());
        frameOutBytes_ = CalculationNeedBytes(requestPreNodeDuration_);
        // for dfx
        auto startTime = std::chrono::steady_clock::now();

        Trace trace("AudioSuiteProcessNode::SignalProcess Start");
        algoProcessedResult_ = SignalProcess(preOutputs);
        trace.End();

        // for dfx
        auto endTime = std::chrono::steady_clock::now();
        auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        CheckEffectNodeProcessTime(algoProcessedResult_[0]->GetDataDuration(), static_cast<uint64_t>(processDuration));
    } else {
        algoProcessedResult_.clear();
        AudioSuitePcmBuffer *convertData =
            convert_.Process(preOutputs[0], const_cast<PcmBufferFormat &>(GetAudioNodeInPcmFormat()));
        CHECK_AND_RETURN_RET_LOG(convertData != nullptr, ERROR, "convertData is nullptr.");
        frameOutBytes_ = convertData->GetDataSize();
        algoProcessedResult_.emplace_back(convertData);
    }

    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ProcessWithCache()
{
    requestPreNodeDuration_ = nodeNeedDataDuration_;
    while (downStreamData_[0].GetDataSize() > cachedBuffer_[0].GetSize() && !GetAudioNodeDataFinishedFlag()) {
        int32_t ret = ObtainProcessedData();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to retrieve data from the preceding node.");
        for (size_t idx = 0; idx < algoProcessedResult_.size(); ++idx) {  // 用algoProcessedResult_的size
            CHECK_AND_RETURN_RET_LOG(
                algoProcessedResult_[idx] != nullptr && algoProcessedResult_[idx]->GetPcmData() != nullptr,
                ERR_OPERATION_FAILED, "node %{public}d do SignalProcess failed, return a nullptr.", GetNodeType());

            cachedBuffer_[idx].PushData(algoProcessedResult_[idx]->GetPcmData(), frameOutBytes_);
        }
    }
    for (size_t idx = 0; idx < downStreamData_.size(); ++idx) {
        downStreamData_[idx].Reset();
        uint32_t CopyDataLength = cachedBuffer_[idx].GetSize() <= downStreamData_[idx].GetDataSize()
                                      ? cachedBuffer_[idx].GetSize()
                                      : downStreamData_[idx].GetDataSize();
        int32_t ret = cachedBuffer_[idx].GetData(downStreamData_[idx].GetPcmData(), CopyDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get data from cachedBuffer_ fail");
        downStreamData_[idx].SetIsFinished(GetAudioNodeDataFinishedFlag() && cachedBuffer_[idx].GetSize() == 0);
        WriteOutputData(&downStreamData_[idx]);
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ProcessDirectly()
{
    requestPreNodeDuration_ = nodeNeedDataDuration_;
    int32_t ret = ObtainProcessedData();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to retrieve data from the preceding node.");

    for (size_t idx = 0; idx < downStreamData_.size(); ++idx) {
        downStreamData_[idx].Reset();
        CHECK_AND_RETURN_RET_LOG(
            algoProcessedResult_[idx] != nullptr && algoProcessedResult_[idx]->GetPcmData() != nullptr,
            ERR_OPERATION_FAILED, "node %{public}d do SignalProcess failed, return a nullptr.", GetNodeType());
        CHECK_AND_RETURN_RET_LOG(algoProcessedResult_[idx]->GetDataSize() <= downStreamData_[idx].GetDataSize(),
            ERROR, "Insufficient target buffer size.");
        int32_t ret = memcpy_s(downStreamData_[idx].GetPcmData(),
            downStreamData_[idx].GetDataSize(),  // Copy the first frame 20ms data
            algoProcessedResult_[idx]->GetPcmData(),
            algoProcessedResult_[idx]->GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memcpy failed, ret is %{public}d.", ret);
        downStreamData_[idx].SetIsFinished(GetAudioNodeDataFinishedFlag());
        WriteOutputData(&downStreamData_[idx]);
    }
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::ProcessBypassMode(uint32_t needDataLength)
{
    requestPreNodeDuration_ = needDataLength;
    ReadProcessNodePreOutputData();
    
    std::vector<AudioSuitePcmBuffer *> preOutputs;
    for (auto* buffer : preNodeResult_) {
        preOutputs.push_back(buffer);
    }
    
    CHECK_AND_RETURN_RET_LOG(
        !preOutputs.empty(), ERROR, "node %{public}d can't get pcmbuffer from prenodes.", GetNodeType());
    AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
    for (size_t idx = 0; idx < downStreamData_.size(); ++idx) {
        preOutputs[0]->SetIsFinished(GetAudioNodeDataFinishedFlag());
        WriteOutputData(preOutputs[0]);
    }

    return SUCCESS;
}

int32_t AudioSuiteProcessNode::DoProcess(uint32_t needDataLength)
{
    CHECK_AND_RETURN_RET_LOG(needDataLength <= maxRequestLength_, ERROR, "Request data length error.");
    if (GetAudioNodeDataFinishedFlag() && cachedBuffer_[0].GetSize() == 0) {
        AUDIO_DEBUG_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
        return ERROR;
    }

    if (nextNeedDataLength_ != needDataLength) {
        int32_t ret = InitCacheLength(needDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to initialize buffer.");
        nextNeedDataLength_ = needDataLength;
    }
    int32_t ret = 0;
    if ((GetNodeBypassStatus() == true) && !needCache_) {
        ret = ProcessBypassMode(needDataLength);
    } else if (!needCache_) {
        ret = ProcessDirectly();
    } else {
        ret = ProcessWithCache();
    }
    
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to process node data.");

    AUDIO_DEBUG_LOG("node type = %{public}d set "
                    "pcmbuffer IsFinished: %{public}d.", GetNodeType(), GetAudioNodeDataFinishedFlag());
    return SUCCESS;
}

void AudioSuiteProcessNode::ReadProcessNodePreOutputData()
{
    bool isFinished = true;
    ReadPreNodeData(GetAudioNodeInPcmFormat(), !GetNodeBypassStatus(), requestPreNodeDuration_);

    // Track finished pre-nodes (based on output data)
    for (auto* buffer : preNodeResult_) {
        isFinished = isFinished && buffer->GetIsFinished();
    }
    AUDIO_DEBUG_LOG("set node type = %{public}d isFinished status: %{public}d.", GetNodeType(), isFinished);
    SetAudioNodeDataFinishedFlag(isFinished);
}

int32_t AudioSuiteProcessNode::SetOptions(std::string name, std::string value)
{
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterfaceImpl_ is nullptr");

    paraName_ = name;
    paraValue_ = value;

    int32_t ret = algoInterface_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed %{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::GetOptions(std::string name, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "paraValue_ is empty");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");
    value = paraValue_;

    int32_t ret = algoInterface_->GetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetParameter failed");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::Flush()
{
    // for dfx
    CheckEffectNodeOvertimeCount();
    secondCall_ = false;
    needCache_ = false;
    nodeNeedDataDuration_ = 0;
    requestPreNodeDuration_ = 0;
    frameOutBytes_ = 0;
    downStreamData_.resize(0);
    algoProcessedResult_.resize(0);
    intermediateResult_.resize(0);
    nextNeedDataLength_ = 0;
    algoInput_.resize(1);
    algoOutput_.resize(0);
    algoInterface_ = nullptr ;

    algoOutPcmBuffer_.resize(0);
    for (uint32_t i = 0; i < cachedBuffer_.size(); i++) {
        cachedBuffer_[i].ClearBuffer();
    }

    CHECK_AND_RETURN_RET_LOG(DeInit() == SUCCESS, ERROR, "DeInit failed");
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "Init failed");
    if (!paraName_.empty() && !paraValue_.empty()) {
        SetOptions(paraName_, paraValue_);
    }
    finishedPrenodeSet.clear();
    
    AUDIO_INFO_LOG("Flush SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::InitOutputStream()
{
    CHECK_AND_RETURN_RET_LOG(GetSharedInstance() != nullptr, ERROR, "GetSharedInstance returns a nullptr");
    int32_t ret = InitFormatConverters();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "InitFormatConverters failed.");
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::Connect(const std::shared_ptr<AudioNode>& preNode)
{
    if (!preNode) {
        AUDIO_ERR_LOG("node type = %{public}d preNode is null!", GetNodeType());
        return ERR_INVALID_PARAM;
    }
    // Add upstream node to preNodes_
    AddPreNode(preNode);
    // Add this node to upstream's nextNodes_
    preNode->AddNextNode(shared_from_this());
    return SUCCESS;
}

int32_t AudioSuiteProcessNode::DisConnect(const std::shared_ptr<AudioNode>& preNode)
{
    if (!preNode) {
        AUDIO_ERR_LOG("node type = %{public}d preNode is null!", GetNodeType());
        return ERR_INVALID_PARAM;
    }
    // Remove from preNodes_
    RemovePreNode(preNode);
    // Remove this node from upstream's nextNodes_
    preNode->RemoveNextNode(shared_from_this());
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
        uint64_t thresholdValue = dataDurationUS * nodeParameter_.realtimeFactor * RTF_OVERTIME_THRESHOLDS[i];
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

// Format converter initialization for ProcessNode
int32_t AudioSuiteProcessNode::InitFormatConverters()
{
    constexpr uint32_t AUDIO_SEPARATION_OUTPUT_COUNT = 2;
    
    formatConverters_.clear();
    formatConversionBuffers_.clear();
    formatConverters_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
    formatConversionBuffers_.resize(1);

    // Audio separation node needs an additional converter
    if (GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) {
        formatConverters_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
        formatConversionBuffers_.resize(AUDIO_SEPARATION_OUTPUT_COUNT);
    }

    return SUCCESS;
}
}
}
}