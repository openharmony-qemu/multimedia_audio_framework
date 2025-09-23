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

#include <vector>
#include <memory>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_eq_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
static constexpr uint32_t RESAMPLE_QUALITY = 5;
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
        AUDIO_INFO_LOG("AudioSuiteEqNode::Init failed, already inited");
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
    tmpin.resize(0);
    tmpout.resize(0);
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
    uint32_t outChannelCount = 2;
    uint32_t inSampleRate = inputPcmBuffer->GetSampleRate();
    uint32_t outSampleRate = SAMPLE_RATE_48000;
    AUDIO_INFO_LOG("AudioSuiteEqNode::preProcess, inChannel");

    int32_t ret;
    if (inChannelCount == outChannelCount && inSampleRate == outSampleRate) {
        ret = CopyBuffer(inputPcmBuffer, &outPcmBuffer_);
    } else if (inChannelCount == outChannelCount) {
        ret = DoResample(inputPcmBuffer, &outPcmBuffer_);
    } else if (inSampleRate == outSampleRate) {
        ret = DoChannelConvert(inputPcmBuffer, &outPcmBuffer_);
    } else {
        AudioSuitePcmBuffer channelConvertPcmBuffer(inputPcmBuffer->GetSampleRate(), outChannelCount, CH_LAYOUT_STEREO);
        ret = DoChannelConvert(inputPcmBuffer, &outPcmBuffer_);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
        ret = DoResample(&channelConvertPcmBuffer, &outPcmBuffer_);
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

int32_t AudioSuiteEqNode::DoChannelConvert(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    AudioChannelInfo inChannelInfo = {(*inputPcmBuffer).GetChannelLayout(), (*inputPcmBuffer).GetChannelCount()};
    AudioChannelInfo outChannelInfo = {CH_LAYOUT_STEREO, 2};
    SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);

    uint32_t framesize = (*inputPcmBuffer).GetFrameLen() / (*inputPcmBuffer).GetChannelCount();
    float *inputData = (*inputPcmBuffer).GetPcmDataBuffer();
    uint32_t inLen = (*inputPcmBuffer).GetFrameLen() / (*inputPcmBuffer).GetChannelCount();
    float *outputData = (*outputPcmBuffer).GetPcmDataBuffer();
    uint32_t outLen = (*outputPcmBuffer).GetFrameLen() / (*outputPcmBuffer).GetChannelCount();
    AUDIO_INFO_LOG("AudioSuiteEqNode::DoChannelConvert, framesize:%{public}u, inlen:%{public}u, outLen:%{public}u",
        framesize,
        inLen,
        outLen);
    int ret = ChannelConvertProcess(framesize, inputData, inLen, outputData, outLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Eqnode ChannnelConvert failed with error code %{public}d", ret);
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
    AUDIO_INFO_LOG("AudioSuiteEqNode::DoReample finished");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteEqNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_INFO_LOG("AudioSuiteEqNode SignalProcess inputs is empty");
        return nullptr;
    } else {
        AUDIO_INFO_LOG("AudioSuiteEqNode SignalProcess inputs frameLen:%{public}d", inputs[0]->GetFrameLen());
    }

    std::vector<uint8_t> eqInputDataBuffer(inputs[0]->GetFrameLen() * ALGO_CHANNEL_NUM);
    std::vector<uint8_t> eqOutputDataBuffer(inputs[0]->GetFrameLen() * ALGO_CHANNEL_NUM);
    int32_t ret = preProcess(inputs[0]);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioSuiteEqNode SignalProcess preProCess failed");
        return &outPcmBuffer_;
    }

    ConvertFromFloat(SAMPLE_S16LE,
        outPcmBuffer_.GetFrameLen(),
        outPcmBuffer_.GetPcmDataBuffer(),
        static_cast<void *>(eqInputDataBuffer.data()));

    tmpin.resize(1);
    tmpout.resize(1);
    uint8_t *inputPointer = eqInputDataBuffer.data();
    uint8_t *outputPointer = eqInputDataBuffer.data();

    tmpin[0] = inputPointer;
    tmpout[0] = outputPointer;

    eqAlgoInterfaceImpl_->Apply(tmpin, tmpout);
    AudioSuiteProcessNode::ConvertToFloat(
        SAMPLE_S16LE, outPcmBuffer_.GetFrameLen(), eqOutputDataBuffer.data(), outPcmBuffer_.GetPcmDataBuffer());

    return &outPcmBuffer_;
}

bool AudioSuiteEqNode::SetEqMode(EqualizerMode type)
{
    currentEqMode = type;
    switch (currentEqMode) {
        case DEFAULT_MODE:
            eqValue = EQUALIZER_DEFAULT_VALUE;
            AUDIO_INFO_LOG("Set EqMode to DEFAULT_MODE");
            break;
        case BALLADS_MODE:
            eqValue = EQUALIZER_BALLADS_VALUE;
            AUDIO_INFO_LOG("Set EqMode to BALLADS_MODE");
            break;
        case CHINESE_STYLE_MODE:
            eqValue = EQUALIZER_CHINESE_STYLE_VALUE;
            AUDIO_INFO_LOG("Set EqMode to CHINESE_STYLE_MODE");
            break;
        case CLASSICAL_MODE:
            eqValue = EQUALIZER_CLASSICAL_VALUE;
            AUDIO_INFO_LOG("Set EqMode to CLASSICAL_MODE");
            break;
        case DANCE_MUSIC_MODE:
            eqValue = EQUALIZER_DANCE_MUSIC_VALUE;
            AUDIO_INFO_LOG("Set EqMode to DANCE_MUSIC");
            break;
        case JAZZ_MODE:
            eqValue = EQUALIZER_JAZZ_VALUE;
            AUDIO_INFO_LOG("Set EqMode to JAZZ");
            break;
        case POP_MODE:
            eqValue = EQUALIZER_POP_VALUE;
            AUDIO_INFO_LOG("Set EqMode to");
            break;
        case RB_MODE:
            eqValue = EQUALIZER_RB_VALUE;
            AUDIO_INFO_LOG("Set EqMode to RB");
            break;
        case ROCK_MODE:
            eqValue = EQUALIZER_ROCK_VALUE;
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
    if (name == "AudioEqualizerFrequencyBandgains") {
        eqAlgoInterfaceImpl_->SetParameter(value, value);
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else if (name == "EqualizerMode") {
        EqualizerMode eqMode = StringToEqualizerMode(value);
        if (SetEqMode(eqMode) && !eqValue.empty()) {
            eqAlgoInterfaceImpl_->SetParameter(eqValue, eqValue);
        }
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions UNKNOW TYPE");
        return ERROR;
    }
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS