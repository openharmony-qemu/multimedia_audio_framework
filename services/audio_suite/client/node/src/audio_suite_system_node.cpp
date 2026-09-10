/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSuiteSystemNode"
#endif

#include <cinttypes>
#include "audio_suite_system_node.h"
#include "audio_suite_log.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr int32_t META_DATA_LEN_SIZE = 4;
static constexpr size_t META_DATA_MAX_PROCESS_LEN = 32 * 1024; // 32K

AudioSuiteSystemNode::AudioSuiteSystemNode(AudioNodeType nodeType)
    : AudioSuiteProcessNode(nodeType), hasInputFormat_(false), hasOutputFormat_(false)
{
}

AudioSuiteSystemNode::~AudioSuiteSystemNode()
{
    if (effectChain_ != nullptr) {
        effectChain_->Release();
        effectChain_ = nullptr;
    }
}

int32_t AudioSuiteSystemNode::Init()
{
    std::string chainName = GetEffectChainName();
    CHECK_AND_RETURN_RET_LOG(!chainName.empty(), ERROR, "GetEffectChainName failed");

    OfflineAudioEffectManager manager;
    std::vector<std::string> availableChains = manager.GetOfflineAudioEffectChains();
    
    bool isChainSupported = false;
    for (const auto& chain : availableChains) {
        AUDIO_INFO_LOG("chain name list:%{public}s", chain.c_str());
        if (chain == chainName) {
            isChainSupported = true;
            break;
        }
    }
    
    if (!isChainSupported) {
        AUDIO_ERR_LOG("AudioSuiteSystemNode::Init failed, chain '%{public}s' is not supported", chainName.c_str());
        return ERR_NOT_SUPPORTED;
    }

    effectChain_ = manager.CreateOfflineAudioEffectChain(chainName);
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR,
        "CreateOfflineAudioEffectChain failed for chain: %{public}s", chainName.c_str());
    
    // By default, all system nodes process data every 40 ms.
    nodeNeedDataDuration_ = PCM_DATA_DURATION_40_MS;

    if (!isOutputStreamInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputStreamInit_ = true;
    }
    AUDIO_INFO_LOG("AudioSuiteSystemNode::Init end, chain: %{public}s", chainName.c_str());
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::DeInit()
{
    if (effectChain_ != nullptr) {
        effectChain_->Release();
        effectChain_ = nullptr;
    }

    AUDIO_INFO_LOG("AudioSuiteSystemNode::DeInit end");
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::ConfigureEffectChain()
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");

    AudioStreamInfo inInfo;
    AudioStreamInfo outInfo;

    PcmBufferFormat inFormat = GetAudioNodeInPcmFormat();
    PcmBufferFormat outFormat = GetAudioNodeOutPcmFormat();

    inInfo.samplingRate = static_cast<AudioSamplingRate>(inFormat.sampleRate);
    inInfo.format = static_cast<AudioSampleFormat>(inFormat.sampleFormat);
    inInfo.channels = static_cast<AudioChannel>(inFormat.channelCount);

    outInfo.samplingRate = static_cast<AudioSamplingRate>(outFormat.sampleRate);
    outInfo.format = static_cast<AudioSampleFormat>(outFormat.sampleFormat);
    outInfo.channels = static_cast<AudioChannel>(outFormat.channelCount);

    int32_t ret = effectChain_->Configure(inInfo, outInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "effectChain_->Configure failed, ret=%{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::PrepareEffectChain()
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");

    int32_t ret = effectChain_->Prepare();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "effectChain_->Prepare failed, ret=%{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::ProcessWithEffectChain(
    uint8_t *inBuffer, int32_t inSize, uint8_t *outBuffer, int32_t outSize)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr && outBuffer != nullptr, ERROR,
        "buffer is nullptr");
    CHECK_AND_RETURN_RET_LOG(inSize > 0 && outSize > 0, ERROR,
        "buffer size invalid, inSize=%{public}d, outSize=%{public}d", inSize, outSize);

    int32_t ret = effectChain_->Process(inBuffer, inSize, outBuffer, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "effectChain_->Process failed, ret=%{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteSystemNode::SetSystemParam(std::vector<uint8_t> &param)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(!param.empty(), ERR_INVALID_PARAM, "param is empty");
    
    int32_t ret = effectChain_->SetParam(param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "effectChain_->SetParam failed, ret=%{public}d", ret);
    
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::GetSystemParam(std::vector<uint8_t> &param)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");
    
    int32_t ret = effectChain_->GetParam(param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "effectChain_->GetParam failed, ret=%{public}d", ret);
    
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::GetInOutSize(uint32_t &inSize, uint32_t &outSize)
{
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERROR, "effectChain_ is nullptr");
    int32_t ret = effectChain_->GetEffectBufferSize(inSize, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "effectChain_->GetEffectBufferSize failed, ret=%{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::PrintNodeInfo(std::string &nodeInfo)
{
    AudioNodeType nodeType = GetNodeType();
    std::string typeName = (nodeType == NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE) ?
        "SYSTEM_DIALOGUE_ENHANCE" : "SYSTEM_MUSIC_MASTER";
    
    nodeInfo = "    Node Type: " + typeName + "\n";
    nodeInfo += "    Buffer Size: in=" + std::to_string(inBufferSize_) +
                ", out=" + std::to_string(outBufferSize_) + "\n";
    return SUCCESS;
}

int32_t AudioSuiteSystemNode::Connect(const std::shared_ptr<AudioNode>& preNode)
{
    CHECK_AND_RETURN_RET_LOG(preNode != nullptr, ERR_INVALID_PARAM, "Connect failed, preNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(preNode->GetNodeType() == NODE_TYPE_INPUT, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT,
        "Connect failed, current node only connnet input node, noteType:%{public}d.", GetNodeType());

    int32_t ret = AudioSuiteProcessNode::Connect(preNode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Connect failed, AudioSuiteProcessNode::Connect failed.");

    AudioFormat preNodeFormat = preNode->GetAudioNodeFormat();
    SetInputFormat(preNodeFormat);
    return SUCCESS;
}

void AudioSuiteSystemNode::Prepare()
{
    CHECK_AND_RETURN_LOG(effectChain_ != nullptr,
        "Prepare failsed, currentNode not Init, nodeId:%{public}u", GetAudioNodeId());
    int32_t ret = effectChain_->Prepare();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "effectChain_->Prepare failed, ret=%{public}d", ret);
}

void AudioSuiteSystemNode::SetInputFormat(AudioFormat format)
{
    AUDIO_INFO_LOG("SetInputFormat, nodeType:%{public}d.", GetNodeType());
    inputFormat_ = format;
    hasInputFormat_ = true;
    TryConfigure();
}

void AudioSuiteSystemNode::SetOutputFormat(AudioFormat format)
{
    AUDIO_INFO_LOG("SetOutputFormat, nodeType:%{public}d.", GetNodeType());
    outputFormat_ = format;
    hasOutputFormat_ = true;
    TryConfigure();
}

void AudioSuiteSystemNode::UpdateOutputFormat(AudioFormat audioFormat)
{
    SetOutputFormat(audioFormat);
}

void AudioSuiteSystemNode::UpdateInputFormat(AudioFormat audioFormat)
{
    SetInputFormat(audioFormat);
}

void AudioSuiteSystemNode::TryConfigure()
{
    CHECK_AND_RETURN_LOG(hasInputFormat_ && hasOutputFormat_,
        "Current Node note set input and output format, nodeId:%{public}u", GetAudioNodeId());
    CHECK_AND_RETURN_LOG(effectChain_ != nullptr, "currentNode not Init, nodeId:%{public}u", GetAudioNodeId());

    AUDIO_INFO_LOG("%{public}d %{public}d %{public}d %{public}u %{public}" PRIu64 " input cfg",
        inputFormat_.rate, inputFormat_.encodingType, inputFormat_.format,
        inputFormat_.audioChannelInfo.numChannels, inputFormat_.audioChannelInfo.channelLayout);
    AUDIO_INFO_LOG("%{public}d %{public}d %{public}d %{public}u %{public}" PRIu64 " output set",
        inputFormat_.rate, inputFormat_.encodingType, inputFormat_.format,
        inputFormat_.audioChannelInfo.numChannels, inputFormat_.audioChannelInfo.channelLayout);

    AudioStreamInfo inInfo;
    AudioStreamInfo outInfo;
    inInfo.samplingRate = inputFormat_.rate;
    inInfo.format = static_cast<AudioSampleFormat>(inputFormat_.format);
    inInfo.channels = static_cast<AudioChannel>(inputFormat_.audioChannelInfo.numChannels);
    inInfo.channelLayout = inputFormat_.audioChannelInfo.channelLayout;
    inInfo.encoding = static_cast<AudioEncodingType>(inputFormat_.encodingType);

    outInfo.samplingRate = outputFormat_.rate;
    outInfo.format = static_cast<AudioSampleFormat>(outputFormat_.format);
    outInfo.channels = static_cast<AudioChannel>(outputFormat_.audioChannelInfo.numChannels);
    outInfo.channelLayout = outputFormat_.audioChannelInfo.channelLayout;
    outInfo.encoding = static_cast<AudioEncodingType>(outputFormat_.encodingType);

    int32_t ret = effectChain_->Configure(inInfo, outInfo);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "effectChain_->Configure failed, ret=%{public}d", ret);

    ret = effectChain_->Prepare();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "effectChain_->Prepare failed, ret=%{public}d", ret);

    ret = effectChain_->GetEffectBufferSize(inBufferSize_, outBufferSize_);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "effectChain_->GetEffectBufferSize failed, ret=%{public}d", ret);

    inMetaData_.resize(META_DATA_MAX_PROCESS_LEN);
    inAudioData_.resize(inBufferSize_);
    inData_.resize(inBufferSize_);
    outData_.resize(outBufferSize_);
    metaFrame_.audioData = inAudioData_.data();
    metaFrame_.audioDataSize = static_cast<int32_t>(inAudioData_.size());
    metaFrame_.metaData = inMetaData_.data();
    metaFrame_.metaDataSize = static_cast<int32_t>(inMetaData_.size());
    AUDIO_INFO_LOG("init buffer inBufferSize_:%{public}u, outBufferSize_:%{public}u", inBufferSize_, outBufferSize_);

    uint32_t frameLen = static_cast<uint32_t>(PCM_DATA_DURATION_40_MS) * outputFormat_.rate / SECONDS_TO_MS;
    uint32_t sampleCount = frameLen * outputFormat_.audioChannelInfo.numChannels;
    outAudioDataSize_ = sampleCount * AudioSuiteUtil::GetSampleSize(outputFormat_.format);

    SetAudioNodeFormat(AudioFormat{
        {inputFormat_.audioChannelInfo.channelLayout, inputFormat_.audioChannelInfo.numChannels},
        inputFormat_.format, inputFormat_.rate,
        {outputFormat_.audioChannelInfo.channelLayout, outputFormat_.audioChannelInfo.numChannels},
        outputFormat_.format, outputFormat_.rate});
    AUDIO_INFO_LOG("Configure success, nodeId:%{public}u", GetAudioNodeId());
    return;
}

std::vector<AudioSuitePcmBuffer*> AudioSuiteSystemNode::SignalProcess(
    const std::vector<AudioSuitePcmBuffer*>& inputs)
{
    std::vector<AudioSuitePcmBuffer*> retError{ nullptr };
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), retError, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, retError, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), retError,
        "Invalid input format");

    uint8_t* inBuffer = inputs[0]->GetPcmData();
    int32_t inSize = static_cast<int32_t>(inputs[0]->GetDataSize());
    
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr && inSize > 0, retError,
        "Invalid input buffer, size=%{public}d", inSize);
    
    if (algoOutPcmBuffer_.empty()) {
        AUDIO_ERR_LOG("algoOutPcmBuffer_ is empty");
        return retError;
    }
    uint8_t* outBuffer = algoOutPcmBuffer_[0].GetPcmData();
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, retError, "effectChain_ is nullptr");
    int32_t ret = effectChain_->Process(inBuffer, inSize, outBuffer, algoOutPcmBuffer_[0].GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, retError, "effectChain_->Process failed, ret=%{public}d", ret);
    return intermediateResult_;
}

int32_t AudioSuiteSystemNode::DoProcess(AudioMetaFrame &metaFrame,
    int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag)
{
    CHECK_AND_RETURN_RET_LOG(responseAudioSize != nullptr && responseMetaSize != nullptr && finishedFlag != nullptr,
        ERR_INVALID_PARAM, "invalid DoProcess param.");
    CHECK_AND_RETURN_RET_LOG(!GetAudioNodeDataFinishedFlag(), ERR_NOT_SUPPORTED, "meta data output completed.");
    CHECK_AND_RETURN_RET_LOG(effectChain_ != nullptr, ERR_NOT_SUPPORTED, "effectChain_ is nullptr.");
    CHECK_AND_RETURN_RET_LOG(!preNodes_.empty(), ERR_NOT_SUPPORTED, "current system node not connect pre node.");
    auto preNode = preNodes_[0].lock();
    CHECK_AND_RETURN_RET_LOG(preNode != nullptr, ERR_NOT_SUPPORTED, "current system node connect pre node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(metaFrame.audioData != nullptr && metaFrame.metaData != nullptr,
        ERR_INVALID_PARAM, "invalid audio data");

    int32_t responseInAudioSize = 0;
    int32_t responseInMetaSize = 0;
    int32_t ret = preNode->DoProcess(metaFrame_, &responseInAudioSize, &responseInMetaSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get process meta audio data failed, ret:%{public}d", ret);

    int32_t inputDataSize = responseInMetaSize + META_DATA_LEN_SIZE + responseInAudioSize;
    CHECK_AND_RETURN_RET_LOG(static_cast<int32_t>(inData_.size()) >= inputDataSize,
        ERR_INVALID_PARAM, "get metaData size err, responseInAudioSize:%{public}d, responseInMetaSize:%{public}d.",
        responseInAudioSize, responseInMetaSize);
    uint8_t* ptr = inData_.data();
    *((int32_t *)ptr) = responseInMetaSize;
    AUDIO_DEBUG_LOG("inputDataSize:%{public}d, metaDataSize:%{public}d", responseInAudioSize, *((int32_t *)ptr));
    ptr += META_DATA_LEN_SIZE;
    errno_t err = memcpy_s(ptr, inData_.size() - META_DATA_LEN_SIZE, metaFrame_.metaData, responseInMetaSize);
    CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s meta data failed, ret=%{public}d, size=%{public}zu, "
        "count=%{public}d", err, inData_.size() - META_DATA_LEN_SIZE, responseInMetaSize);
    ptr += responseInMetaSize;
    err = memcpy_s(ptr, inData_.size() - META_DATA_LEN_SIZE - responseInMetaSize,
        metaFrame_.audioData, responseInAudioSize);
    CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s audio data failed, ret=%{public}d, size=%{public}zu, "
        "count=%{public}d", err, inData_.size() - META_DATA_LEN_SIZE - responseInMetaSize, responseInAudioSize);
    
    ret = effectChain_->Process(inData_.data(), inputDataSize, outData_.data(), outData_.size());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "effectChain_->Process failed, ret=%{public}d", ret);
    ptr = outData_.data();
    *responseMetaSize = *((int32_t *)ptr);
    ptr += META_DATA_LEN_SIZE;
    *responseAudioSize = static_cast<int32_t>(outAudioDataSize_);
    AUDIO_DEBUG_LOG("output outsize:%{public}zu, audio data:%{public}d, metaDataSize:%{public}d",
        outData_.size(), *responseAudioSize, *responseMetaSize);
    CHECK_AND_RETURN_RET_LOG(static_cast<int>(outData_.size()) >= *responseMetaSize +
        META_DATA_LEN_SIZE + *responseAudioSize, ERR_INVALID_PARAM, "responseMetaSize error");
    err = memcpy_s(metaFrame.metaData, metaFrame.metaDataSize, ptr, *responseMetaSize);
    CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s out meta data failed, ret=%{public}d, size=%{public}d, "
        "count=%{public}d", err, metaFrame.metaDataSize, *responseMetaSize);
    ptr += *responseMetaSize;

    err = memcpy_s(metaFrame.audioData, metaFrame.audioDataSize, ptr, *responseAudioSize);
    CHECK_AND_RETURN_RET_LOG(err == EOK, ERROR, "memcpy_s out audio data failed, ret=%{public}d, size=%{public}d, "
        "count=%{public}d", err, metaFrame.audioDataSize, *responseAudioSize);
    SetAudioNodeDataFinishedFlag(*finishedFlag);
    return SUCCESS;
}

}
}
}
