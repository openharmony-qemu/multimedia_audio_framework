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
#define LOG_TAG "AudioSuiteAissNode"
#endif

#include "audio_suite_aiss_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint16_t DEFAULT_CHANNEL_COUNT = 2;
constexpr uint16_t DEFAULT_CHANNEL_COUNT_OUT = 4;
constexpr int RESAMPLE_QUALITY = 5;

AudioSuiteAissNode::AudioSuiteAissNode()
    : AudioSuiteProcessNode(NODE_TYPE_AUDIO_SEPARATION, AudioFormat{{CH_LAYOUT_STEREO,
        DEFAULT_CHANNEL_COUNT}, SAMPLE_F32LE, SAMPLE_RATE_48000}),
    tmpInput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_STEREO),
    channelOutput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_STEREO),
    rateOutput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_STEREO),
    tmpOutput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT_OUT, CH_LAYOUT_QUAD),
    tmpHumanSoundOutput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_QUAD),
    tmpBkgSoundOutput_(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_QUAD)
{
    AUDIO_INFO_LOG("AudioSuiteAissNode create success");
}

bool AudioSuiteAissNode::Reset()
{
    if (aissAlgo_ == nullptr) {
        AUDIO_ERR_LOG("aissAlgo_ is nullptr");
        return false;
    }
    if (aissAlgo_->Init() != SUCCESS) {
        AUDIO_ERR_LOG("InitAlgorithm failed");
        return false;
    }
    if (Flush() != SUCCESS) {
        AUDIO_ERR_LOG("Flush failed");
        return false;
    }
    return true;
}

int32_t AudioSuiteAissNode::DoProcess()
{
    CHECK_AND_RETURN_RET_LOG(GetAudioNodeDataFinishedFlag() != true, SUCCESS, "AudioSuiteProcessNode"
        "DoProcess:Current node type = %{public}d does not have more data to process.", GetNodeType());
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "AudioSuiteAissNode init failed");
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_INVALID_PARAM,
        "node type = %{public}d inputstream is null!", GetNodeType());
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }

    AudioSuitePcmBuffer* tempOut = nullptr;
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadProcessNodePreOutputData();
    if ((GetNodeEnableStatus() == NODE_ENABLE) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("AudioSuiteProcessNode::DoProcess: node type = %{public}d need "
            "do SignalProcess.", GetNodeType());
        tempOut = SignalProcess(preOutputs);
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d do SignalProcess failed, "
                "return a nullptr", GetNodeType());
            return ERR_OPERATION_FAILED;
        }
        tmpHumanSoundOutput_.SetIsFinished(GetAudioNodeDataFinishedFlag());
        tmpBkgSoundOutput_.SetIsFinished(GetAudioNodeDataFinishedFlag());
        outputStream_->WriteDataToOutput(&tmpHumanSoundOutput_);
        outputStream_->WriteDataToOutput(&tmpBkgSoundOutput_);
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("AudioSuiteProcessNode::DoProcess: node type = %{public}d signalProcess "
            "is not enabled.", GetNodeType());
        tempOut = preOutputs[0];
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d get a null pcmbuffer "
                "from prenode", GetNodeType());
            return ERR_INVALID_READ;
        }
        tempOut->SetIsFinished(GetAudioNodeDataFinishedFlag());
        tmpHumanSoundOutput_ = *tempOut;
        tmpBkgSoundOutput_ = *tempOut;
        outputStream_->WriteDataToOutput(tempOut);
        outputStream_->WriteDataToOutput(tempOut);
    } else {
        AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d can't get "
            "pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    HandleTapCallback(&tmpOutput_);
    return SUCCESS;
}

int32_t AudioSuiteAissNode::Flush()
{
    // inputStream_ need flush
    // outputStream_ need flush
    // bkgOutputStream_ need flush
    finishedPrenodeSet.clear();
    tmpInput_.Reset();
    channelOutput_.Reset();
    rateOutput_.Reset();
    tmpOutput_.Reset();
    tmpHumanSoundOutput_.Reset();
    tmpBkgSoundOutput_.Reset();
    tmpin_.clear();
    tmpout_.clear();
    return SUCCESS;
}

int32_t AudioSuiteAissNode::Init()
{
    if (isInit_ == true) {
        AUDIO_DEBUG_LOG("AudioSuiteAissNode has inited");
        return SUCCESS;
    }
    if (!aissAlgo_) {
        aissAlgo_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION);
    }
    if (Flush() != SUCCESS) {
        AUDIO_ERR_LOG("Flush failed");
        return ERROR;
    }
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }

    if (aissAlgo_->Init() != SUCCESS) {
        AUDIO_ERR_LOG("InitAlgorithm failed");
        return ERROR;
    }
    isInit_ = true;
    AUDIO_DEBUG_LOG("AudioSuiteAissNode Init success");
    return SUCCESS;
}

int32_t AudioSuiteAissNode::DeInit()
{
    isInit_ = false;
    if (aissAlgo_ != nullptr) {
        aissAlgo_->Deinit();
    }
    aissAlgo_ = nullptr;
    if (Flush() != SUCCESS) {
        AUDIO_ERR_LOG("Flush failed");
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissNode DeInit success");
    return SUCCESS;
}

int32_t AudioSuiteAissNode::InstallTap(AudioNodePortType portType,
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
{
    if (portType == AudioNodePortType::AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE) {
        humanTap_.SetAudioNodePortType(portType);
        humanTap_.SetOnReadTapDataCallback(callback);
        AUDIO_DEBUG_LOG("AudioSuiteAissNode InstallTap humanTap_");
        return SUCCESS;
    } else if (portType == AudioNodePortType::AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE) {
        bkgTap_.SetAudioNodePortType(portType);
        bkgTap_.SetOnReadTapDataCallback(callback);
        AUDIO_DEBUG_LOG("AudioSuiteAissNode InstallTap bkgTap_");
        return SUCCESS;
    }
    AUDIO_ERR_LOG("Invalid port type: %{public}d", (uint32_t)portType);
    return ERROR;
}

int32_t AudioSuiteAissNode::RemoveTap(AudioNodePortType portType)
{
    if (portType == AudioNodePortType::AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE) {
        humanTap_.SetOnReadTapDataCallback(nullptr);
        AUDIO_DEBUG_LOG("AudioSuiteAissNode RemoveTap humanTap_");
        return SUCCESS;
    } else if (portType == AudioNodePortType::AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE) {
        bkgTap_.SetOnReadTapDataCallback(nullptr);
        AUDIO_DEBUG_LOG("AudioSuiteAissNode RemoveTap bkgTap_");
        return SUCCESS;
    }
    AUDIO_ERR_LOG("Invalid port type: %{public}d", (uint32_t)portType);
    return ERROR;
}

AudioSuitePcmBuffer* AudioSuiteAissNode::SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs)
{
    if (aissAlgo_ == nullptr) {
        AUDIO_ERR_LOG("aissAlgo_ is nullptr");
        return nullptr;
    }
    if (inputs.empty() || inputs[0] == nullptr) {
        AUDIO_ERR_LOG("inputs error");
        return nullptr;
    }
    tmpInput_ = preProcess(*inputs[0]);
    tmpin_.clear();
    tmpout_.clear();
    tmpin_.emplace_back(reinterpret_cast<uint8_t *>(tmpInput_.GetPcmDataBuffer()));
    tmpout_.emplace_back(reinterpret_cast<uint8_t *>(tmpOutput_.GetPcmDataBuffer()));
    tmpout_.emplace_back(reinterpret_cast<uint8_t *>(tmpHumanSoundOutput_.GetPcmDataBuffer()));
    tmpout_.emplace_back(reinterpret_cast<uint8_t *>(tmpBkgSoundOutput_.GetPcmDataBuffer()));
    int32_t ret = aissAlgo_->Apply(tmpin_, tmpout_);
    if (ret != SUCCESS) {
        return nullptr;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissNode SignalProcess success");
    return &tmpOutput_;
}

void AudioSuiteAissNode::HandleTapCallback(AudioSuitePcmBuffer* pcmBuffer)
{
    AUDIO_DEBUG_LOG("Enter AudioSuiteAissNode HandleTapCallback");
    std::shared_ptr<SuiteNodeReadTapDataCallback> humanSoundCallback = humanTap_.GetOnReadTapDataCallback();
    if (humanSoundCallback != nullptr) {
        AudioNodePortType tapType = humanTap_.GetAudioNodePortType();
        CHECK_AND_RETURN_LOG(tapType == AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE,
            "tap type error, taptype:%{public}d", tapType);
        humanSoundCallback->OnReadTapDataCallback(static_cast<void*>(tmpHumanSoundOutput_.GetPcmDataBuffer()),
            tmpHumanSoundOutput_.GetFrameLen() * sizeof(float));
        AUDIO_DEBUG_LOG("AudioSuiteAissNode handle humanSoundCallback success");
    }
    std::shared_ptr<SuiteNodeReadTapDataCallback> bkgSoundCallback = bkgTap_.GetOnReadTapDataCallback();
    if (bkgSoundCallback != nullptr) {
        AudioNodePortType tapType = bkgTap_.GetAudioNodePortType();
        CHECK_AND_RETURN_LOG(tapType == AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE,
            "tap type error, taptype:%{public}d", tapType);
        bkgSoundCallback->OnReadTapDataCallback(static_cast<void*>(tmpBkgSoundOutput_.GetPcmDataBuffer()),
            tmpBkgSoundOutput_.GetFrameLen() * sizeof(float));
        AUDIO_DEBUG_LOG("AudioSuiteAissNode handle bkgSoundCallback success");
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissNode HandleTapCallback success");
}

AudioSuitePcmBuffer AudioSuiteAissNode::preProcess(AudioSuitePcmBuffer& input)
{
    tmpInput_ = input;
    if (input.GetChannelCount() != DEFAULT_CHANNEL_COUNT) {
        rateOutput_ = rateConvert(tmpInput_, input.GetSampleRate(), DEFAULT_CHANNEL_COUNT);
        tmpInput_ = rateOutput_;
    }
    if (input.GetSampleRate() != SAMPLE_RATE_48000) {
        channelOutput_ = channelConvert(tmpInput_, SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT);
        tmpInput_ = channelOutput_;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissNode preProcess success");
    return tmpInput_;
}

AudioSuitePcmBuffer AudioSuiteAissNode::rateConvert(AudioSuitePcmBuffer input,
    uint32_t sampleRate, uint32_t channelCount)
{
    rateOutput_.ResizePcmBuffer(sampleRate, channelCount);
    int32_t ret = SetUpResample(input.GetSampleRate(), sampleRate,
        input.GetChannelCount(), RESAMPLE_QUALITY);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, rateOutput_,
        "setup resample failed with error code %{public}d", ret);
    ret = DoResampleProcess(input.GetPcmDataBuffer(),
        input.GetFrameLen() / input.GetChannelCount(),
        rateOutput_.GetPcmDataBuffer(), rateOutput_.GetFrameLen() / rateOutput_.GetChannelCount());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, rateOutput_,
        "Do resample process failed with error code %{public}d", ret);
    AUDIO_DEBUG_LOG("AudioSuiteAissNode rateConvert success");
    return rateOutput_;
}

AudioSuitePcmBuffer AudioSuiteAissNode::channelConvert(AudioSuitePcmBuffer input,
    uint32_t sampleRate, uint32_t channelCount)
{
    channelOutput_.ResizePcmBuffer(sampleRate, channelCount);
    AudioChannelInfo inChannelInfo = {input.GetChannelLayout(), input.GetChannelCount()};
    AudioChannelInfo outChannelInfo = {CH_LAYOUT_STEREO, channelCount};
    uint32_t inputBuffSize = input.GetFrameLen() * SAMPLE_F32LE;
    uint32_t outputBuffSize = channelOutput_.GetFrameLen() * SAMPLE_F32LE;
    int32_t ret = SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, channelOutput_,
        "Set Channel convert processParam failed with error code %{public}d", ret);
    ret = ChannelConvertProcess(input.GetFrameLen() / input.GetChannelCount(),
        input.GetPcmDataBuffer(), inputBuffSize, channelOutput_.GetPcmDataBuffer(), outputBuffSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, channelOutput_,
        "Channel convert process failed with error code %{public}d", ret);
    AUDIO_DEBUG_LOG("AudioSuiteAissNode channelConvert success");
    return channelOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS