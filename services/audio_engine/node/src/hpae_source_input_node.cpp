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
#define LOG_TAG "HpaeSourceInputNode"
#endif

#include <cinttypes>
#include <thread>
#include "hpae_source_input_node.h"
#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "capturer_clock_manager.h"
#include "audio_engine_log.h"

#define BYTE_SIZE_SAMPLE_U8 1
#define BYTE_SIZE_SAMPLE_S16 2
#define BYTE_SIZE_SAMPLE_S24 3
#define BYTE_SIZE_SAMPLE_S32 4
#define FRAME_DURATION_DEFAULT 20
#define MILLISECOND_PER_SECOND 1000

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static std::string TransSourceBufferTypeToString(const HpaeSourceBufferType &type)
{
    if (type == HPAE_SOURCE_BUFFER_TYPE_MIC) {
        return "MIC";
    } else if (type == HPAE_SOURCE_BUFFER_TYPE_EC) {
        return "EC";
    } else if (type == HPAE_SOURCE_BUFFER_TYPE_MICREF) {
        return "MICREF";
    }
    return "DEFAULT";
}

HpaeSourceInputNode::HpaeSourceInputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), sourceInputNodeType_(nodeInfo.sourceInputNodeType)
{
    HpaeSourceBufferType sourceBufferType = nodeInfo.sourceBufferType;
    nodeInfoMap_.emplace(sourceBufferType, nodeInfo);
    frameByteSizeMap_.emplace(
        sourceBufferType, nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    nodeInfoMap_[sourceBufferType].frameLen = FRAME_DURATION_DEFAULT * nodeInfo.samplingRate / MILLISECOND_PER_SECOND;
    pcmBufferInfoMap_.emplace(
        sourceBufferType, PcmBufferInfo(nodeInfoMap_[sourceBufferType].channels,
        nodeInfoMap_[sourceBufferType].frameLen, nodeInfoMap_[sourceBufferType].samplingRate));
    inputAudioBufferMap_.emplace(sourceBufferType, HpaePcmBuffer(pcmBufferInfoMap_.at(sourceBufferType)));
    inputAudioBufferMap_.at(sourceBufferType).SetSourceBufferType(sourceBufferType);
    capturerFrameDataMap_.emplace(sourceBufferType, frameByteSizeMap_.at(sourceBufferType));
    outputStreamMap_.emplace(sourceBufferType, this);
    historyDataMap_.emplace(sourceBufferType, 0);
    historyRemainSizeMap_.emplace(sourceBufferType, 0);
    if (sourceInputNodeType_ == HPAE_SOURCE_EC) {
        fdescMap_.emplace(sourceBufferType,
            FrameDesc{capturerFrameDataMap_.at(sourceBufferType).data(), frameByteSizeMap_.at(sourceBufferType)});
        fdescMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_DEFAULT, FrameDesc{nullptr, 0});
    }
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeSourceInputNode[" + TransSourceBufferTypeToString(nodeInfo.sourceBufferType) + "]");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeSourceInputNode::HpaeSourceInputNode(std::vector<HpaeNodeInfo> &nodeInfos)
    : HpaeNode(*nodeInfos.begin()), sourceInputNodeType_((*nodeInfos.begin()).sourceInputNodeType)
{
    for (auto nodeInfo : nodeInfos) {
        HpaeSourceBufferType sourceBufferType = nodeInfo.sourceBufferType;
        nodeInfoMap_.emplace(sourceBufferType, nodeInfo);
        frameByteSizeMap_.emplace(
            sourceBufferType, nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
        nodeInfoMap_[sourceBufferType].frameLen =
            FRAME_DURATION_DEFAULT * nodeInfo.samplingRate / MILLISECOND_PER_SECOND;
        pcmBufferInfoMap_.emplace(
            sourceBufferType, PcmBufferInfo(nodeInfoMap_[sourceBufferType].channels,
            nodeInfoMap_[sourceBufferType].frameLen, nodeInfoMap_[sourceBufferType].samplingRate));
        inputAudioBufferMap_.emplace(sourceBufferType, HpaePcmBuffer(pcmBufferInfoMap_.at(sourceBufferType)));
        inputAudioBufferMap_.at(sourceBufferType).SetSourceBufferType(sourceBufferType);
        capturerFrameDataMap_.emplace(sourceBufferType, frameByteSizeMap_.at(sourceBufferType));
        fdescMap_.emplace(sourceBufferType,
            FrameDesc{capturerFrameDataMap_.at(sourceBufferType).data(), frameByteSizeMap_.at(sourceBufferType)});
        outputStreamMap_.emplace(sourceBufferType, this);
        historyDataMap_.emplace(sourceBufferType, 0);
        historyRemainSizeMap_.emplace(sourceBufferType, 0);
    }
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeSourceInputNode[MIC_EC]");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeSourceInputNode::~HpaeSourceInputNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeSourceInputNode::SetBufferValid(const HpaeSourceBufferType &bufferType, const uint64_t &replyBytes)
{
    CHECK_AND_RETURN_LOG(inputAudioBufferMap_.find(bufferType) != inputAudioBufferMap_.end(),
        "set buffer valid with error type");
    inputAudioBufferMap_.at(bufferType).SetBufferValid(true);
    uint32_t byteSize = nodeInfoMap_.at(bufferType).channels * nodeInfoMap_.at(bufferType).frameLen *
        static_cast<uint32_t>(GetSizeFromFormat(nodeInfoMap_.at(bufferType).format));
    if (replyBytes != byteSize) {
        AUDIO_WARNING_LOG("DoProcess(), request size[%{public}zu][%{public}u], reply size[%{public}" PRIu64 "]",
            frameByteSizeMap_.at(bufferType), byteSize, replyBytes);
        AUDIO_WARNING_LOG("DoProcess(), if reply != request, just drop now");
        inputAudioBufferMap_.at(bufferType).SetBufferValid(false);
    }
}

void HpaeSourceInputNode::DoProcessInner(const HpaeSourceBufferType &bufferType, const uint64_t &replyBytes)
{
    AUDIO_DEBUG_LOG("DoProcessInner, replyBytes: %{public}" PRIu64, replyBytes);
    // todo: do not convert to float in SourceInputNode
    ConvertToFloat(nodeInfoMap_.at(bufferType).format,
        nodeInfoMap_.at(bufferType).channels * nodeInfoMap_.at(bufferType).frameLen,
        capturerFrameDataMap_.at(bufferType).data(),
        inputAudioBufferMap_.at(bufferType).GetPcmDataBuffer());
    if (inputAudioBufferMap_.at(bufferType).IsValid()) {
        outputStreamMap_.at(bufferType).WriteDataToOutput(&inputAudioBufferMap_.at(bufferType));
    }
}

void HpaeSourceInputNode::DoProcessMicInner(const HpaeSourceBufferType &bufferType, const uint64_t &replyBytes)
{
    AUDIO_DEBUG_LOG("DoProcessMicInner, replyBytes: %{public}" PRIu64, replyBytes);
    if (isInjecting_ && replyBytes == 0 && audioCapturerSource_->GetArmUsbDeviceStatus() != 1) {
        AUDIO_WARNING_LOG("HpaeSourceInputNode::DoProcessMicInner injecting need sleep");
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DURATION_DEFAULT));
        return;
    }
    auto &historyData = historyDataMap_.at(bufferType);
    uint32_t byteSize = nodeInfoMap_.at(bufferType).channels * nodeInfoMap_.at(bufferType).frameLen *
        static_cast<uint32_t>(GetSizeFromFormat(nodeInfoMap_.at(bufferType).format));
    if (historyRemainSizeMap_.at(bufferType) < byteSize) {
        historyData.insert(historyData.end(), byteSize - historyRemainSizeMap_.at(bufferType), 0);
        historyRemainSizeMap_.at(bufferType) = byteSize;
    }

    // todo: do not convert to float in SourceInputNode
    ConvertToFloat(nodeInfoMap_.at(bufferType).format,
        nodeInfoMap_.at(bufferType).channels * nodeInfoMap_.at(bufferType).frameLen,
        historyData.data(), inputAudioBufferMap_.at(bufferType).GetPcmDataBuffer());
    historyRemainSizeMap_[bufferType] -= byteSize;
    // drop data has been written
    historyData.erase(historyData.begin(), historyData.begin() + byteSize);
    outputStreamMap_.at(bufferType).WriteDataToOutput(&inputAudioBufferMap_.at(bufferType));
}

static bool CheckEcAndMicRefReplyValid(const uint64_t &requestBytes, const uint64_t replyBytes)
{
    return replyBytes != 0 && requestBytes == replyBytes;
}

void HpaeSourceInputNode::DoProcess()
{
    Trace trace("[" + std::to_string(GetNodeId()) + "]HpaeSourceInputNode::DoProcess " + GetTraceInfo());
    CHECK_AND_RETURN_LOG(audioCapturerSource_ != nullptr,
        "audioCapturerSource_ is nullptr NodeId: %{public}u", GetNodeId());
    uint64_t replyBytes = 0;
    if (sourceInputNodeType_ == HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC) {
        uint64_t replyBytesEc = 0;
        audioCapturerSource_->CaptureFrameWithEc(&fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC), replyBytes,
                                                 &fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC), replyBytesEc);
        PushDataToBuffer(HPAE_SOURCE_BUFFER_TYPE_MIC, replyBytes);
        DoProcessMicInner(HPAE_SOURCE_BUFFER_TYPE_MIC, replyBytes);
        CHECK_AND_RETURN_LOG(
            CheckEcAndMicRefReplyValid(frameByteSizeMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC), replyBytesEc),
            "same ec request != reply");
        DoProcessInner(HPAE_SOURCE_BUFFER_TYPE_EC, replyBytesEc);
    } else if (sourceInputNodeType_ == HpaeSourceInputNodeType::HPAE_SOURCE_OFFLOAD) {
        uint64_t replyBytesEc = 0;
        audioCapturerSource_->CaptureFrameWithEc(&fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC), replyBytes,
                                                 &fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC), replyBytesEc);
        CHECK_AND_RETURN_LOG(
            CheckEcAndMicRefReplyValid(frameByteSizeMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC), replyBytesEc),
            "same offload ec request != reply");
        DoProcessInner(HPAE_SOURCE_BUFFER_TYPE_EC, replyBytesEc);
    } else {
        HpaeSourceBufferType sourceBufferType = nodeInfoMap_.begin()->second.sourceBufferType;
        if (sourceInputNodeType_ == HPAE_SOURCE_EC) {
            uint64_t replyBytesUnused = 0;
            audioCapturerSource_->CaptureFrameWithEc(&fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_DEFAULT), replyBytesUnused,
                                                     &fdescMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC), replyBytes);
        } else {
            ReadDataFromSource(sourceBufferType, replyBytes);
        }
        
        if (sourceInputNodeType_ == HPAE_SOURCE_MIC) {
            DoProcessMicInner(sourceBufferType, replyBytes);
        } else {
            CHECK_AND_RETURN_LOG(CheckEcAndMicRefReplyValid(frameByteSizeMap_.at(sourceBufferType), replyBytes),
                "request != reply");
            DoProcessMicInner(sourceBufferType, replyBytes);
        }
    }
}

bool HpaeSourceInputNode::Reset()
{
    return true;
}

bool HpaeSourceInputNode::ResetAll()
{
    return true;
}

std::shared_ptr<HpaeNode> HpaeSourceInputNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<HpaePcmBuffer *> *HpaeSourceInputNode::GetOutputPort()
{
    std::unordered_map<HpaeSourceBufferType, OutputPort<HpaePcmBuffer *>>::iterator it;
    if (sourceInputNodeType_ != HPAE_SOURCE_MIC_EC) {
        it = outputStreamMap_.begin();
    } else {
        it = outputStreamMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    }
    CHECK_AND_RETURN_RET_LOG(it != outputStreamMap_.end(), nullptr, "outStreamMap_ is empty.");
    return &(it->second);
}

OutputPort<HpaePcmBuffer *> *HpaeSourceInputNode::GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect)
{
    auto it = outputStreamMap_.find(nodeInfo.sourceBufferType);
    CHECK_AND_RETURN_RET_LOG(it != outputStreamMap_.end(), nullptr,
        "can't find nodeKey in outStreamMap_, sourceBufferType = %{public}d.\n",
        nodeInfo.sourceBufferType);
    return &(it->second);
}

HpaeSourceBufferType HpaeSourceInputNode::GetOutputPortBufferType(HpaeNodeInfo &nodeInfo)
{
    auto it = outputStreamMap_.find(nodeInfo.sourceBufferType);
    CHECK_AND_RETURN_RET_LOG(it != outputStreamMap_.end(), HPAE_SOURCE_BUFFER_TYPE_DEFAULT,
        "can't find nodeKey in outStreamMap_, sourceBufferType = %{public}d.\n", nodeInfo.sourceBufferType);
    // todo: rewrite this function
    if (sourceInputNodeType_ == HpaeSourceInputNodeType::HPAE_SOURCE_MIC_EC) {
        if (nodeInfo.sourceBufferType == HPAE_SOURCE_BUFFER_TYPE_MIC) {
            return HPAE_SOURCE_BUFFER_TYPE_MIC;
        } else {
            return HPAE_SOURCE_BUFFER_TYPE_EC;
        }
    } else {
        return inputAudioBufferMap_.at(nodeInfo.sourceBufferType).GetSourceBufferType();
    }
}

int32_t HpaeSourceInputNode::GetCapturerSourceAdapter(
    const std::string &deviceClass, const SourceType &sourceType, const std::string &info)
{
    captureId_ = HDI_INVALID_ID;
    if (info.empty()) {
        captureId_ = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(
            deviceClass, sourceType, HDI_ID_INFO_DEFAULT, true);
    } else {
        captureId_ = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(
            deviceClass, sourceType, info, true);
    }
    audioCapturerSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(captureId_, true);
    if (audioCapturerSource_ == nullptr) {
        AUDIO_ERR_LOG("get source fail, deviceClass: %{public}s, info: %{public}s, captureId_: %{public}u",
            deviceClass.c_str(), info.c_str(), captureId_);
        HdiAdapterManager::GetInstance().ReleaseId(captureId_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HpaeSourceInputNode::GetCapturerSourceInstance(const std::string &deviceClass, const std::string &deviceNetId,
    const SourceType &sourceType, const std::string &sourceName)
{
    if (sourceType == SOURCE_TYPE_WAKEUP || sourceName == HDI_ID_INFO_EC || sourceName == HDI_ID_INFO_MIC_REF) {
        return GetCapturerSourceAdapter(deviceClass, sourceType, sourceName);
    }
    return GetCapturerSourceAdapter(deviceClass, sourceType, deviceNetId);
}

int32_t HpaeSourceInputNode::CapturerSourceInit(IAudioSourceAttr &attr)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    Trace trace("HpaeSourceInputNode::CapturerSourceInit");
    if (audioCapturerSource_->IsInited()) {
        SetSourceState(STREAM_MANAGER_IDLE);
#ifdef IS_EMULATOR
        AUDIO_INFO_LOG("do start and stop");
        if (sourceInputNodeType_ == HPAE_SOURCE_MIC || sourceInputNodeType_ == HPAE_SOURCE_MIC_EC) {
            audioCapturerSource_->Start();
            audioCapturerSource_->Stop();
        }
#endif
        return SUCCESS;
    }

    audioSourceAttr_ = attr;
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Init(attr) == SUCCESS, ERROR, "Source init fail");
    SetSourceState(STREAM_MANAGER_IDLE);
#ifdef IS_EMULATOR
    // Due to the peculiar implementation of the emulator's HDI,
    // an initial start and stop sequence is required to circumvent protential issues and ensure proper functionality.
    AUDIO_INFO_LOG("do start and stop");
    if (sourceInputNodeType_ == HPAE_SOURCE_MIC || sourceInputNodeType_ == HPAE_SOURCE_MIC_EC) {
        audioCapturerSource_->Start();
        audioCapturerSource_->Stop();
    }
#endif
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceDeInit()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    Trace trace("HpaeSourceInputNode::CapturerSourceDeInit");
    audioCapturerSource_->DeInit();
    audioCapturerSource_ = nullptr;
    HdiAdapterManager::GetInstance().ReleaseId(captureId_);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceFlush(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    return audioCapturerSource_->Flush();
}

int32_t HpaeSourceInputNode::CapturerSourcePause(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Pause() == SUCCESS, ERROR, "Source pause fail");
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceReset(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    return audioCapturerSource_->Reset();
}

int32_t HpaeSourceInputNode::CapturerSourceResume(void)
{
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Resume() == SUCCESS, ERROR, "Source resume fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceStart(void)
{
    Trace trace("HpaeSourceInputNode::CapturerSourceStart");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Start() == SUCCESS, ERROR, "Source start fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceStop(void)
{
    Trace trace("HpaeSourceInputNode::CapturerSourceStop");
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        ERROR, "invalid audioCapturerSource");
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->IsInited(), ERROR, "invalid source state");
    if (audioCapturerSource_->Stop() != SUCCESS) {
        AUDIO_ERR_LOG("stop error, sourceInputNode[%{public}u]", sourceInputNodeType_);
    }
    return SUCCESS;
}

StreamManagerState HpaeSourceInputNode::GetSourceState(void)
{
    return state_;
}

int32_t HpaeSourceInputNode::SetSourceState(StreamManagerState sourceState)
{
    HILOG_COMM_INFO("Source[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sourceState).c_str());
    state_ = sourceState;
    return SUCCESS;
}

size_t HpaeSourceInputNode::GetOutputPortNum()
{
    std::unordered_map<HpaeSourceBufferType, OutputPort<HpaePcmBuffer *>>::iterator it;
    if (sourceInputNodeType_ != HPAE_SOURCE_MIC_EC) {
        it = outputStreamMap_.begin();
    } else {
        it = outputStreamMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC);
    }
    CHECK_AND_RETURN_RET_LOG(it != outputStreamMap_.end(), 0, "outStreamMap_ is empty.");
    return it->second.GetInputNum();
}

size_t HpaeSourceInputNode::GetOutputPortNum(HpaeNodeInfo &nodeInfo)
{
    auto it = outputStreamMap_.find(nodeInfo.sourceBufferType);
    CHECK_AND_RETURN_RET_LOG(it != outputStreamMap_.end(), 0, "can't find nodeKey in outStreamMap_.");
    return it->second.GetInputNum();
}

HpaeSourceInputNodeType HpaeSourceInputNode::GetSourceInputNodeType()
{
    return sourceInputNodeType_;
}

void HpaeSourceInputNode::SetSourceInputNodeType(HpaeSourceInputNodeType type)
{
    sourceInputNodeType_ = type;
}

HpaeNodeInfo &HpaeSourceInputNode::GetNodeInfoWithInfo(HpaeSourceBufferType &type)
{
    auto it = nodeInfoMap_.find(type);
    CHECK_AND_RETURN_RET_LOG(it != nodeInfoMap_.end(), nodeInfoMap_.begin()->second,
        "can't find nodeKey in nodeInfoMap_.");
    return it->second;
}

void HpaeSourceInputNode::UpdateAppsUidAndSessionId(std::vector<int32_t> &appsUid, std::vector<int32_t> &sessionsId)
{
    CHECK_AND_RETURN_LOG(audioCapturerSource_ != nullptr && captureId_ != HDI_INVALID_ID,
        "audioCapturerSource_ is nullptr");
    CHECK_AND_RETURN_LOG(audioCapturerSource_->IsInited(), "invalid source state");
    audioCapturerSource_->UpdateAppsUid(appsUid);
    std::shared_ptr<AudioSourceClock> clock =
        CapturerClockManager::GetInstance().GetAudioSourceClock(captureId_);
    if (clock != nullptr) {
        clock->UpdateSessionId(sessionsId);
    }
}

uint32_t HpaeSourceInputNode::GetCaptureId() const
{
    return captureId_;
}

void HpaeSourceInputNode::ReadDataFromSource(const HpaeSourceBufferType &bufferType, uint64_t &replyBytes)
{
    uint32_t byteSize = nodeInfoMap_.at(bufferType).channels * nodeInfoMap_.at(bufferType).frameLen *
        static_cast<uint32_t>(GetSizeFromFormat(nodeInfoMap_.at(bufferType).format));
    while (historyRemainSizeMap_[bufferType] < byteSize) {
        int32_t ret = audioCapturerSource_->CaptureFrame(capturerFrameDataMap_.at(bufferType).data(),
            (uint64_t)frameByteSizeMap_.at(bufferType), replyBytes);
        if (sourceInputNodeType_ == HPAE_SOURCE_MIC) { // micref not sleep
            backoffController_.HandleResult(ret == SUCCESS);
        }
        CHECK_AND_RETURN_LOG(replyBytes != 0, "replyBytes is 0");
        PushDataToBuffer(bufferType, replyBytes);
    }
}

void HpaeSourceInputNode::PushDataToBuffer(const HpaeSourceBufferType &bufferType, const uint64_t &replyBytes)
{
    auto &historyData = historyDataMap_.at(bufferType);
    auto newData = capturerFrameDataMap_.at(bufferType).data();
    historyData.insert(historyData.end(), newData, newData + replyBytes);
    historyRemainSizeMap_[bufferType] += replyBytes;
}

void HpaeSourceInputNode::SetInjectState(bool isInjecting)
{
    isInjecting_ = isInjecting;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS