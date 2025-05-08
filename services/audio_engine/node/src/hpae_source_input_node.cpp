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

#include "hpae_source_input_node.h"
#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_errors.h"
#include "audio_engine_log.h"
#include "audio_utils.h"
#include "cinttypes"

#define BYTE_SIZE_SAMPLE_U8 1
#define BYTE_SIZE_SAMPLE_S16 2
#define BYTE_SIZE_SAMPLE_S24 3
#define BYTE_SIZE_SAMPLE_S32 4
#define FRAME_DURATION_DEFAULT 20
#define MILLISECOND_PER_SECOND 1000

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
    static std::string TransSourceBufferTypeToString(HpaeSourceBufferType &type)
{
    switch (type) {
        case HPAE_SOURCE_BUFFER_TYPE_EC:
            return "_EC.pcm";
        case HPAE_SOURCE_BUFFER_TYPE_MICREF:
            return "_MICREF.pcm";
        default:
            return ".pcm";
    }
}

HpaeSourceInputNode::HpaeSourceInputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), sourceInputNodeType_(nodeInfo.sourceInputNodeType)
{
    HpaeSourceBufferType sourceBufferType = nodeInfo.sourceBufferType;
    nodeInfoMap_.emplace(sourceBufferType, nodeInfo);
    pcmBufferInfoMap_.emplace(
        sourceBufferType, PcmBufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate));
    inputAudioBufferMap_.emplace(sourceBufferType, HpaePcmBuffer(pcmBufferInfoMap_.at(sourceBufferType)));
    inputAudioBufferMap_.at(sourceBufferType).SetSourceBufferType(sourceBufferType);
    frameByteSizeMap_.emplace(
        sourceBufferType, nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    capturerFrameDataMap_.emplace(sourceBufferType, frameByteSizeMap_.at(sourceBufferType));
    outputStreamMap_.emplace(sourceBufferType, this);

#ifdef ENABLE_HOOK_PCM
    inputPcmDumperMap_.emplace(sourceBufferType,
        std::make_unique<HpaePcmDumper>("HpaeSourceInputNode_id_"+ std::to_string(GetSessionId()) +
            "_ch_" +  std::to_string(GetChannelCount()) +
            "_rate_" + std::to_string(GetSampleRate()) +
            "_bit_"+ std::to_string(GetBitWidth()) + TransSourceBufferTypeToString(sourceBufferType)));
#endif
}

HpaeSourceInputNode::HpaeSourceInputNode(std::vector<HpaeNodeInfo> &nodeInfos)
    : HpaeNode(*nodeInfos.begin()), sourceInputNodeType_((*nodeInfos.begin()).sourceInputNodeType)
{
    for (auto nodeInfo : nodeInfos) {
        HpaeSourceBufferType sourceBufferType = nodeInfo.sourceBufferType;
        nodeInfoMap_.emplace(sourceBufferType, nodeInfo);
        pcmBufferInfoMap_.emplace(
            sourceBufferType, PcmBufferInfo(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate));
        inputAudioBufferMap_.emplace(sourceBufferType, HpaePcmBuffer(pcmBufferInfoMap_.at(sourceBufferType)));
        inputAudioBufferMap_.at(sourceBufferType).SetSourceBufferType(sourceBufferType);
        frameByteSizeMap_.emplace(
            sourceBufferType, nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
        capturerFrameDataMap_.emplace(sourceBufferType, frameByteSizeMap_.at(sourceBufferType));
        fdescMap_.emplace(sourceBufferType,
            FrameDesc{capturerFrameDataMap_.at(sourceBufferType).data(), frameByteSizeMap_.at(sourceBufferType)});
        outputStreamMap_.emplace(sourceBufferType, this);
#ifdef ENABLE_HOOK_PCM
        inputPcmDumperMap_.emplace(sourceBufferType,
            std::make_unique<HpaePcmDumper>("HpaeSourceInputNode_id_"+ std::to_string(GetSessionId()) +
                "_ch_" +  std::to_string(nodeInfo.channels) +
                "_rate_" + std::to_string(nodeInfo.samplingRate) +
                "_bit_"+ std::to_string(nodeInfo.format) + TransSourceBufferTypeToString(sourceBufferType)));
#endif
    }
}

std::string HpaeSourceInputNode::GetTraceInfo()
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]_";
    auto format = "bit[" + std::to_string(GetBitWidth()) + "]";
    return rate + ch + len + format;
}

void HpaeSourceInputNode::SetBufferValid(const HpaeSourceBufferType &bufferType, const uint64_t &replyBytes)
{
    CHECK_AND_RETURN_LOG(inputAudioBufferMap_.find(bufferType) != inputAudioBufferMap_.end(),
        "set buffer valid with error type");
    inputAudioBufferMap_.at(bufferType).SetBufferValid(true);
    if (frameByteSizeMap_.at(bufferType) != replyBytes) {
        AUDIO_WARNING_LOG("DoProcess(), request size[%{public}" PRIu64 "], reply size[%{public}" PRIu64 "]",
            frameByteSizeMap_.at(bufferType), replyBytes);
        AUDIO_WARNING_LOG("DoProcess(), if reply != request, just drop now");
        inputAudioBufferMap_.at(bufferType).SetBufferValid(false);
    }
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
        SetBufferValid(HPAE_SOURCE_BUFFER_TYPE_MIC, replyBytes);
#ifdef ENABLE_HOOK_PCM
        if (inputPcmDumperMap_.find(HPAE_SOURCE_BUFFER_TYPE_MIC) != inputPcmDumperMap_.end() &&
            inputPcmDumperMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC)) {
            inputPcmDumperMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC)->Dump(
                (int8_t *) capturerFrameDataMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC).data(), replyBytes);
        }
        if (inputPcmDumperMap_.find(HPAE_SOURCE_BUFFER_TYPE_EC) != inputPcmDumperMap_.end() &&
            inputPcmDumperMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC)) {
            inputPcmDumperMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC)->Dump(
                (int8_t *) capturerFrameDataMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).data(), replyBytesEc);
        }
#endif
        // todo: do not convert to float in SourceInputNode
        ConvertToFloat(GetBitWidth(), GetChannelCount() * GetFrameLen(),
            capturerFrameDataMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC).data(),
            inputAudioBufferMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC).GetPcmDataBuffer());
        ConvertToFloat(nodeInfoMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).format,
            nodeInfoMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).channels * nodeInfoMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).frameLen,
            capturerFrameDataMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).data(),
            inputAudioBufferMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).GetPcmDataBuffer());
        outputStreamMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC).WriteDataToOutput(
            &inputAudioBufferMap_.at(HPAE_SOURCE_BUFFER_TYPE_MIC));
        outputStreamMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC).WriteDataToOutput(
            &inputAudioBufferMap_.at(HPAE_SOURCE_BUFFER_TYPE_EC));
    } else {
        HpaeSourceBufferType sourceBufferType = nodeInfoMap_.begin()->second.sourceBufferType;
        audioCapturerSource_->CaptureFrame(capturerFrameDataMap_.at(sourceBufferType).data(),
                                           (uint64_t)frameByteSizeMap_.at(sourceBufferType), replyBytes);
        SetBufferValid(sourceBufferType, replyBytes);
#ifdef ENABLE_HOOK_PCM
        if (inputPcmDumperMap_.find(sourceBufferType) != inputPcmDumperMap_.end() &&
            inputPcmDumperMap_.at(sourceBufferType)) {
            inputPcmDumperMap_.at(sourceBufferType)->Dump(
                (int8_t *) capturerFrameDataMap_.at(sourceBufferType).data(), replyBytes);
        }
#endif
        // todo: do not convert to float in SourceInputNode
        ConvertToFloat(GetBitWidth(), GetChannelCount() * GetFrameLen(),
            capturerFrameDataMap_.at(sourceBufferType).data(),
            inputAudioBufferMap_.at(sourceBufferType).GetPcmDataBuffer());
        outputStreamMap_.at(sourceBufferType).WriteDataToOutput(&inputAudioBufferMap_.at(sourceBufferType));
    }
}

int32_t HpaeSourceInputNode::WriteCapturerData(char *data, int32_t dataSize)
{
    auto itCapturerFrameData = capturerFrameDataMap_.begin();
    auto itFrameByteSize = frameByteSizeMap_.begin();
    CHECK_AND_RETURN_RET_LOG(
        itCapturerFrameData != capturerFrameDataMap_.end() && itFrameByteSize != frameByteSizeMap_.end(),
        ERROR, "outStreamMap_ is empty.");
    int32_t ret = memcpy_s(itCapturerFrameData->second.data(), itFrameByteSize->second, data, dataSize);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "memcpy error when WriteCapturerData");
    return 0;
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
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }

    if (audioCapturerSource_->IsInited()) {
        return SUCCESS;
    }

    audioSourceAttr_ = attr;
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Init(attr) == SUCCESS, ERROR, "Source init fail");
    SetSourceState(STREAM_MANAGER_IDLE);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceDeInit()
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    audioCapturerSource_->DeInit();
    audioCapturerSource_ = nullptr;
    // todo: check where to release captureId_
    HdiAdapterManager::GetInstance().ReleaseId(captureId_);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceFlush(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    return audioCapturerSource_->Flush();
}

int32_t HpaeSourceInputNode::CapturerSourcePause(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Pause() == SUCCESS, ERROR, "Source pause fail");
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceReset(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    return audioCapturerSource_->Reset();
}

int32_t HpaeSourceInputNode::CapturerSourceResume(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Resume() == SUCCESS, ERROR, "Source resume fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceStart(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Start() == SUCCESS, ERROR, "Source start fail");
    SetSourceState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSourceInputNode::CapturerSourceStop(void)
{
    if (audioCapturerSource_ == nullptr) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSource_->Stop() == SUCCESS, ERROR, "Source stop fail");
    SetSourceState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

StreamManagerState HpaeSourceInputNode::GetSourceState(void)
{
    return state_;
}

int32_t HpaeSourceInputNode::SetSourceState(StreamManagerState sourceState)
{
    AUDIO_INFO_LOG("Source[%{public}s] state change:[%{public}s]-->[%{public}s]",
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
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS