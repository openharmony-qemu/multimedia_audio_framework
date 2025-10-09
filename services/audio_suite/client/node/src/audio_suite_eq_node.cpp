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
#define LOG_TAG "AudioSuiteEqNode"
#endif

#include <vector>
#include <memory>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_eq_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuiteEqNode::AudioSuiteEqNode()
    : AudioSuiteProcessNode(
          NODE_TYPE_EQUALIZER, AudioFormat{{CH_LAYOUT_STEREO, ALGO_CHANNEL_NUM}, SAMPLE_S16LE, SAMPLE_RATE_48000}),
      outPcmBuffer_(SAMPLE_RATE_48000, ALGO_CHANNEL_NUM, CH_LAYOUT_STEREO)
{}

AudioSuiteEqNode::~AudioSuiteEqNode()
{
    if (IsEqNodeInit()) {
        DeInit();
    }
}

int32_t AudioSuiteEqNode::Init()
{
    if (IsEqNodeInit()) {
        AUDIO_ERR_LOG("AudioSuiteEqNode::Init failed, already inited");
        return ERROR;
    }
    eqAlgoInterfaceImpl_ = std::make_shared<AudioSuiteEqAlgoInterfaceImpl>();
    eqAlgoInterfaceImpl_->Init();
    isEqNodeInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEqNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEqNode::DeInit()
{
    tmpin_.resize(0);
    tmpout_.resize(0);
    if (eqAlgoInterfaceImpl_ != nullptr) {
        eqAlgoInterfaceImpl_->Deinit();
        eqAlgoInterfaceImpl_ = nullptr;
    }

    if (IsEqNodeInit()) {
        isEqNodeInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEqNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

bool AudioSuiteEqNode::IsEqNodeInit()
{
    if (isEqNodeInit_) {
        return true;
    }
    return false;
}

bool AudioSuiteEqNode::Reset()
{
    return true;
}

int32_t AudioSuiteEqNode::preProcess(AudioSuitePcmBuffer *inputPcmBuffer)
{
    uint32_t inChannelCount = inputPcmBuffer->GetChannelCount();
    uint32_t inSampleRate = inputPcmBuffer->GetSampleRate();

    int32_t ret;
    if (inChannelCount == ALGO_CHANNEL_NUM && inSampleRate == outPcmBuffer_.GetSampleRate()) {
        ret = CopyBuffer(inputPcmBuffer, &outPcmBuffer_);
    } else if (inChannelCount == ALGO_CHANNEL_NUM) {
        ret = DoResample(inputPcmBuffer, &outPcmBuffer_);
    } else {
        AUDIO_ERR_LOG("Don't support channel convert now");
        return ERROR;
    }
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    return SUCCESS;
}

int32_t AudioSuiteEqNode::CopyBuffer(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    float *inputData = (*inputPcmBuffer).GetPcmDataBuffer();
    uint32_t inFrameSize = (*inputPcmBuffer).GetFrameLen() * sizeof(float);
    float *outputData = (*outputPcmBuffer).GetPcmDataBuffer();
    uint32_t outFrameSize = (*outputPcmBuffer).GetFrameLen() * sizeof(float);
    int32_t ret = memcpy_s(outputData, outFrameSize, inputData, inFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CopyBuffer failed.");
    return SUCCESS;
}

int32_t AudioSuiteEqNode::DoResample(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    uint32_t inRate = (*inputPcmBuffer).GetSampleRate();
    uint32_t outRate = SAMPLE_RATE_48000;
    uint32_t channelCount = (*inputPcmBuffer).GetChannelCount();
    if (channelCount == 0) {
        AUDIO_ERR_LOG("InputPcmBuffer ChannelCount is zero!");
        return ERROR;
    }
    int32_t ret = SetUpResample(inRate, outRate, channelCount, RESAMPLE_QUALITY);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetUpResample failed with error code %{public}d", ret);

    float *inputData = (*inputPcmBuffer).GetPcmDataBuffer();
    uint32_t inFrameSize = (*inputPcmBuffer).GetFrameLen() / channelCount;
    float *outputData = (*outputPcmBuffer).GetPcmDataBuffer();
    uint32_t outFrameSize = (*outputPcmBuffer).GetFrameLen() / channelCount;
    ret = DoResampleProcess(inputData, inFrameSize, outputData, outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoResampleProcess failed with error code %{public}d", ret);
    AUDIO_DEBUG_LOG("AudioSuiteEqNode::DoReample finished");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteEqNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_ERR_LOG("AudioSuiteEqNode SignalProcess inputs is empty");
        return nullptr;
    }

    eqInputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    eqOutputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    int32_t ret = preProcess(inputs[0]);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioSuiteEqNode SignalProcess preProCess failed");
        return &outPcmBuffer_;
    }

    ConvertFromFloat(SAMPLE_S16LE,
        outPcmBuffer_.GetFrameLen(),
        outPcmBuffer_.GetPcmDataBuffer(),
        static_cast<void *>(eqInputDataBuffer_.data()));

    tmpin_.resize(1);
    tmpout_.resize(1);
    uint8_t *inputPointer = eqInputDataBuffer_.data();
    uint8_t *outputPointer = eqOutputDataBuffer_.data();

    tmpin_[0] = inputPointer;
    tmpout_[0] = outputPointer;
    eqAlgoInterfaceImpl_->Apply(tmpin_, tmpout_);
    ConvertToFloat(
        SAMPLE_S16LE, outPcmBuffer_.GetFrameLen(), eqOutputDataBuffer_.data(), outPcmBuffer_.GetPcmDataBuffer());
    return &outPcmBuffer_;
}

bool AudioSuiteEqNode::SetEqMode(EqualizerMode type)
{
    currentEqMode = type;
    switch (currentEqMode) {
        case DEFAULT_MODE:
            eqValue_ = EQUALIZER_DEFAULT_VALUE;
            AUDIO_INFO_LOG("Set EqMode to DEFAULT_MODE");
            break;
        case BALLADS_MODE:
            eqValue_ = EQUALIZER_BALLADS_VALUE;
            AUDIO_INFO_LOG("Set EqMode to BALLADS_MODE");
            break;
        case CHINESE_STYLE_MODE:
            eqValue_ = EQUALIZER_CHINESE_STYLE_VALUE;
            AUDIO_INFO_LOG("Set EqMode to CHINESE_STYLE_MODE");
            break;
        case CLASSICAL_MODE:
            eqValue_ = EQUALIZER_CLASSICAL_VALUE;
            AUDIO_INFO_LOG("Set EqMode to CLASSICAL_MODE");
            break;
        case DANCE_MUSIC_MODE:
            eqValue_ = EQUALIZER_DANCE_MUSIC_VALUE;
            AUDIO_INFO_LOG("Set EqMode to DANCE_MUSIC");
            break;
        case JAZZ_MODE:
            eqValue_ = EQUALIZER_JAZZ_VALUE;
            AUDIO_INFO_LOG("Set EqMode to JAZZ");
            break;
        case POP_MODE:
            eqValue_ = EQUALIZER_POP_VALUE;
            AUDIO_INFO_LOG("Set EqMode to POP");
            break;
        case RB_MODE:
            eqValue_ = EQUALIZER_RB_VALUE;
            AUDIO_INFO_LOG("Set EqMode to RB");
            break;
        case ROCK_MODE:
            eqValue_ = EQUALIZER_ROCK_VALUE;
            AUDIO_INFO_LOG("Set EqMode to ROCK");
            break;
    }
    return true;
}

EqualizerMode StringToEqualizerMode(const std::string &modStr)
{
    static const std::unordered_map<std::string, EqualizerMode> modeMap = {
        {"1", DEFAULT_MODE},
        {"2", BALLADS_MODE},
        {"3", CHINESE_STYLE_MODE},
        {"4", CLASSICAL_MODE},
        {"5", DANCE_MUSIC_MODE},
        {"6", JAZZ_MODE},
        {"7", POP_MODE},
        {"8", RB_MODE},
        {"9", ROCK_MODE},
    };

    auto it = modeMap.find(modStr);
    if (it != modeMap.end()) {
        return it->second;
    } else {
        return DEFAULT_MODE;
    }
}

int32_t AudioSuiteEqNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteEqNode::SetOptions Enter");
    if (name == "AudioEqualizerFrequencyBandGains") {
        eqAlgoInterfaceImpl_->SetParameter(value, value);
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else if (name == "EqualizerMode") {
        EqualizerMode eqMode = StringToEqualizerMode(value);
        if (SetEqMode(eqMode) && !eqValue_.empty()) {
            eqAlgoInterfaceImpl_->SetParameter(eqValue_, eqValue_);
        }
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions Unknow Type %{public}s", name.c_str());
        return ERROR;
    }
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS