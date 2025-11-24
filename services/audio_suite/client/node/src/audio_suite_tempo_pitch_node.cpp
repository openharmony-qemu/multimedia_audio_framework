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
#define LOG_TAG "AudioSuiteTempoPitchNode"
#endif

#include "audio_suite_tempo_pitch_node.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr size_t TEMPO_PITCH_PCM_FRAME_BYTES = 1920;      // 0.02s * 480 samples * 1 channel * 2 bytes
static constexpr size_t RESIZE_EXPAND_BYTES = 512; // 256 frames
}

static constexpr AudioSamplingRate TEMPO_PITCH_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat TEMPO_PITCH_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel TEMPO_PITCH_ALGO_CHANNEL_COUNT = MONO;
static constexpr AudioChannelLayout TEMPO_PITCH_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_MONO;

AudioSuiteTempoPitchNode::AudioSuiteTempoPitchNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_TEMPO_PITCH,
          AudioFormat{{TEMPO_PITCH_ALGO_CHANNEL_LAYOUT, TEMPO_PITCH_ALGO_CHANNEL_COUNT},
          TEMPO_PITCH_ALGO_SAMPLE_FORMAT, TEMPO_PITCH_ALGO_SAMPLE_RATE}),
    outPcmBuffer_(PcmBufferFormat{TEMPO_PITCH_ALGO_SAMPLE_RATE,
          TEMPO_PITCH_ALGO_CHANNEL_COUNT,
          TEMPO_PITCH_ALGO_CHANNEL_LAYOUT,
          TEMPO_PITCH_ALGO_SAMPLE_FORMAT})
{}

AudioSuiteTempoPitchNode::~AudioSuiteTempoPitchNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuiteTempoPitchNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuiteTempoPitchNode::Init failed, already inited");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioSuiteTempoPitchNode::Init enter");
    algoInterface_ = std::make_shared<AudioSuiteTempoPitchAlgoInterfaceImpl>(nodeCapability);
    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteTempoPitchAlgoInterfaceImpl Init failed");

    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteTempoPitchNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::DeInit()
{
    tmpin_.resize(0);
    tmpout_.resize(0);
    outBuffer_.resize(0);
    currentDataBuffer_.resize(0);
    while (!readyDataBuffer_.empty()) {
        readyDataBuffer_.pop();
    }
    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteTempoPitchNode::DeInit end");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("AudioSuiteTempoPitchNode DeInit failed, must be initialized first.");
    return ERROR;
}

float ParseStringToSpeedRate(const std::string &str, char delimiter)
{
    std::string token;
    std::istringstream iss(str);

    if (std::getline(iss, token, delimiter) && !token.empty()) {
        return std::stof(token);
    }

    return 0.0f;
}

int32_t AudioSuiteTempoPitchNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("Tempo and Pitch node SetOptions [%{public}s]: %{public}s", name.c_str(), value.c_str());
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");
    CHECK_AND_RETURN_RET_LOG(name == "speedAndPitch", ERROR, "SetOptions Unknow Type %{public}s", name.c_str());

    paraName_ = name;
    paraValue_ = value;
    int32_t ret = algoInterface_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "TempoPitchNode SetOptions ERROR");

    float speedRate = ParseStringToSpeedRate(value, ',');
    CHECK_AND_RETURN_RET_LOG(speedRate > 0.0f, ERROR, "TempoPitchNode ParseStringToSpeedRate ERROR");
    // Add 512 bytes of expansion
    size_t outBufferSize =
        static_cast<size_t>(std::ceil(TEMPO_PITCH_PCM_FRAME_BYTES / speedRate)) + RESIZE_EXPAND_BYTES;
    outBuffer_.resize(outBufferSize);
    currentDataBuffer_.resize(TEMPO_PITCH_PCM_FRAME_BYTES);
    bufferRemainSize_ = TEMPO_PITCH_PCM_FRAME_BYTES;
    AUDIO_INFO_LOG("TempoPitchNode SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("AudioSuiteTempoPitchNode::GetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == "speedAndPitch", ERROR, "GetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "Tempo and pitch value is empty");

    value = paraValue_;
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::PadBufferToPcmBuffer(AudioSuitePcmBuffer &pcmBuffer)
{
    auto writePtr = currentDataBuffer_.begin() + TEMPO_PITCH_PCM_FRAME_BYTES - bufferRemainSize_;
    currentDataBuffer_.insert(writePtr, bufferRemainSize_, 0);

    int copyRet = memcpy_s(pcmBuffer.GetPcmData(), TEMPO_PITCH_PCM_FRAME_BYTES, 
        currentDataBuffer_.data(), TEMPO_PITCH_PCM_FRAME_BYTES);
    CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "pcmBuffer copy not enough");

    bufferRemainSize_ = TEMPO_PITCH_PCM_FRAME_BYTES;
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::DoProcessPreOutputs(AudioSuitePcmBuffer** tempOut)
{
    CHECK_AND_RETURN_RET_LOG(tempOut != nullptr, ERROR, "DoProcessPreOutputs input para is nullptr");
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadProcessNodePreOutputData();
    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());
        Trace trace("wangrubin SignalProcess start");
        if (SignalProcess(preOutputs) == nullptr) {
            AUDIO_ERR_LOG("node %{public}d do SignalProcess failed, return a nullptr", GetNodeType());
            return ERR_OPERATION_FAILED;
        }
        trace.End();
        if (!readyDataBuffer_.empty()) {
            int32_t copyRet = memcpy_s(outPcmBuffer_.GetPcmData(), TEMPO_PITCH_PCM_FRAME_BYTES,
                readyDataBuffer_.front().data(), TEMPO_PITCH_PCM_FRAME_BYTES);
            CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "outPcmBuffer not enough");
            readyDataBuffer_.pop();
            *tempOut = &outPcmBuffer_;
        }
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
        *tempOut = preOutputs[0];
        if (*tempOut == nullptr) {
            AUDIO_ERR_LOG("node %{public}d get a null pcmbuffer from prenode", GetNodeType());
            return ERR_INVALID_READ;
        }
    } else {
        AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    // read data finished, but remain buffer
    if (GetAudioNodeDataFinishedFlag() && !readFinishedFlag_) {
        readFinishedFlag_ = true;
        if (!readyDataBuffer_.empty() || bufferRemainSize_ < static_cast<int32_t>(TEMPO_PITCH_PCM_FRAME_BYTES)) {
            SetAudioNodeDataFinishedFlag(false);
        }
    }
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::DoProcess()
{
    CHECK_AND_RETURN_RET_LOG(!GetAudioNodeDataFinishedFlag(), SUCCESS,
        "Current node type = %{public}d does not have more data to process.", GetNodeType());
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }
    CHECK_AND_RETURN_RET_LOG(inputStream_, ERR_INVALID_PARAM,
        "node type = %{public}d inputstream is null!", GetNodeType());
    Trace trace("wangrubin DoProcess start");
    AudioSuitePcmBuffer* tempOut = nullptr;
    int32_t ret = -1;
    // readyDataBuffer_ has data
    if (!readyDataBuffer_.empty()) {
        int32_t copyRet = memcpy_s(outPcmBuffer_.GetPcmData(), TEMPO_PITCH_PCM_FRAME_BYTES,
            readyDataBuffer_.front().data(), TEMPO_PITCH_PCM_FRAME_BYTES);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "outPcmBuffer not enough");
        readyDataBuffer_.pop();
        tempOut = &outPcmBuffer_;
    } else if (readFinishedFlag_) {
        ret = PadBufferToPcmBuffer(outPcmBuffer_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "PadBufferToPcmBuffer ERROR");
        tempOut = &outPcmBuffer_;
    } else {    // read data from preNode
        ret = DoProcessPreOutputs(&tempOut);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoProcessPreOutputs ERROR");
    }
    if (tempOut == nullptr) {
        AUDIO_INFO_LOG("readyDataBuffer_ is not enough, wait next frame");
        return SUCCESS;
    }

    if (readFinishedFlag_ && readyDataBuffer_.empty() && bufferRemainSize_ == TEMPO_PITCH_PCM_FRAME_BYTES) {
        SetAudioNodeDataFinishedFlag(true);
    }
    tempOut->SetIsFinished(GetAudioNodeDataFinishedFlag());
    outputStream_->WriteDataToOutput(tempOut);
    trace.End();
    AUDIO_DEBUG_LOG("node type = %{public}d set "
        "pcmbuffer IsFinished: %{public}d.", GetNodeType(), GetAudioNodeDataFinishedFlag());
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::SplitDataToQueue(uint8_t* outBuffer, int32_t outFrameBytes)
{
    CHECK_AND_RETURN_RET_LOG(outBuffer != nullptr, ERROR, "SplitDataToQueue input para is nullptr");
    int32_t copyRet = -1;
    uint8_t *readPtr = outBuffer;
    uint8_t *writePtr = currentDataBuffer_.data() + TEMPO_PITCH_PCM_FRAME_BYTES - bufferRemainSize_;
    while (outFrameBytes > 0) {
        int32_t needCopyBytes = bufferRemainSize_ > outFrameBytes ? outFrameBytes : bufferRemainSize_;
        copyRet = memcpy_s(writePtr, bufferRemainSize_, readPtr, needCopyBytes);
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "currentDataBuffer_ not enough");
        bufferRemainSize_ -= needCopyBytes;
        outFrameBytes -= needCopyBytes;

        if (bufferRemainSize_ == 0) {
            std::vector<uint8_t> tempOutput(TEMPO_PITCH_PCM_FRAME_BYTES);
            copyRet = memcpy_s(tempOutput.data(), TEMPO_PITCH_PCM_FRAME_BYTES, 
                currentDataBuffer_.data(), TEMPO_PITCH_PCM_FRAME_BYTES);
            CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "tempOutput copy not enough");
 
            readyDataBuffer_.push(tempOutput);
            bufferRemainSize_ = TEMPO_PITCH_PCM_FRAME_BYTES;
        }
        readPtr += needCopyBytes;
        writePtr = currentDataBuffer_.data() + TEMPO_PITCH_PCM_FRAME_BYTES - bufferRemainSize_;
    }
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteTempoPitchNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuiteTempoPitchNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr,
        "AudioSuiteTempoPitchNode SignalProcess inputs[0] is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid inputs format");

    tmpin_.resize(1);
    tmpout_.resize(1);
    tmpin_[0] = inputs[0]->GetPcmData();
    tmpout_[0] = outBuffer_.data();
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "tempoPitchAlgoInterfaceImpl_ is nullptr");
    int32_t outFrameBytes = algoInterface_->Apply(tmpin_, tmpout_) * sizeof(int16_t);
    CHECK_AND_RETURN_RET_LOG(outFrameBytes >= 0, nullptr, "AudioSuiteTempoPitchNode SignalProcess Apply failed");

    int32_t ret = SplitDataToQueue(outBuffer_.data(), outFrameBytes);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteTempoPitchNode SplitDataToQueue failed");

    return &outPcmBuffer_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS