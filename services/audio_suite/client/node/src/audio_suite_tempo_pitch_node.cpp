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
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr size_t TEMPO_PITCH_PCM_FRAME_BYTES = 1920;      // 0.02s * 480 samples * 1 channel * 2 bytes
static constexpr size_t RESIZE_EXPAND_BYTES = 512; // 256 frames
static constexpr int32_t RESIZE_EXPAND_RATE = 2;
}

static constexpr AudioChannelLayout TEMPO_PITCH_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_MONO;

AudioSuiteTempoPitchNode::AudioSuiteTempoPitchNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_TEMPO_PITCH)
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
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_TEMPO_PITCH, nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create algoInterface");
    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteTempoPitchAlgoInterfaceImpl Init failed");

    SetAudioNodeFormat(AudioFormat{{TEMPO_PITCH_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});

    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;
    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteTempoPitchNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchNode::DeInit()
{
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
    float value;
    std::string paramValue;
    std::istringstream iss(str);

    if (std::getline(iss, paramValue, delimiter) && !paramValue.empty()) {
        CHECK_AND_RETURN_RET_LOG(StringConverterFloat(paramValue, value), 0.0f,
            "Pure voice change convert string to float value error, invalid data is %{public}s", paramValue.c_str());
        return value;
    }

    return 0.0f;
}

uint32_t AudioSuiteTempoPitchNode::CalculationNeedBytes(uint32_t frameLengthMs)
{
    uint32_t dataBytes = 0;
    dataBytes = static_cast<size_t>(std::ceil(TEMPO_PITCH_PCM_FRAME_BYTES / speedRate)) * RESIZE_EXPAND_RATE +
                        RESIZE_EXPAND_BYTES;
    return dataBytes;
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
    speedRate = ParseStringToSpeedRate(value, ',');
    if (FLOAT_COMPARE_EQ(speedRate, 0.0f)) {
        AUDIO_ERR_LOG("TempoPitchNode ParseStringToSpeedRate ERROR");
        return ERROR;
    }
    AUDIO_INFO_LOG("TempoPitchNode SetOptions SUCCESS");
    return SUCCESS;
}

std::vector<AudioSuitePcmBuffer *> AudioSuiteTempoPitchNode::SignalProcess(
    const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, retPcmBuffer, "algoInterface_ is nullptr, need Init first");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), retPcmBuffer, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, retPcmBuffer, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), retPcmBuffer, "Invalid input format");

    algorithmInput_[0] = inputs[0]->GetPcmData();

    int32_t ret = algoInterface_->Apply(algorithmInput_, algorithmOutput_);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, retPcmBuffer, "Node SignalProcess Apply failed");
    frameOutBytes = static_cast<uint32_t>(ret * sizeof(int16_t));
    
    return retPcmBuffer;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS