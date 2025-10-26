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

#include "audio_suite_eq_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioSamplingRate EQ_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat EQ_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel EQ_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout EQ_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string setBandGains = "AudioEqualizerFrequencyBandGains";
}  // namespace

AudioSuiteEqNode::AudioSuiteEqNode()
    : AudioSuiteProcessNode(NODE_TYPE_EQUALIZER,
          AudioFormat{{EQ_ALGO_CHANNEL_LAYOUT, EQ_ALGO_CHANNEL_COUNT}, EQ_ALGO_SAMPLE_FORMAT, EQ_ALGO_SAMPLE_RATE}),
      outPcmBuffer_(
          PcmBufferFormat{EQ_ALGO_SAMPLE_RATE, EQ_ALGO_CHANNEL_COUNT, EQ_ALGO_CHANNEL_LAYOUT, EQ_ALGO_SAMPLE_FORMAT})
{}

AudioSuiteEqNode::~AudioSuiteEqNode()
{
    if (isEqNodeInit_) {
        DeInit();
    }
}

int32_t AudioSuiteEqNode::Init()
{
    if (isEqNodeInit_) {
        AUDIO_ERR_LOG("AudioSuiteEqNode::Init failed, already inited");
        return ERROR;
    }
    eqAlgoInterfaceImpl_ = std::make_shared<AudioSuiteEqAlgoInterfaceImpl>(nodeCapability);
    eqAlgoInterfaceImpl_->Init();
    isEqNodeInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEqNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEqNode::DeInit()
{
    if (eqAlgoInterfaceImpl_ != nullptr) {
        eqAlgoInterfaceImpl_->Deinit();
        eqAlgoInterfaceImpl_ = nullptr;
    }

    if (isEqNodeInit_) {
        isEqNodeInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEqNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

AudioSuitePcmBuffer *AudioSuiteEqNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuiteEqNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr, "AudioSuiteEqNode SignalProcess inputs[0] is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid inputs format");

    tmpin_[0] = inputs[0]->GetPcmData();
    tmpout_[0] = outPcmBuffer_.GetPcmData();
    CHECK_AND_RETURN_RET_LOG(eqAlgoInterfaceImpl_ != nullptr, nullptr, "eqAlgoInterfaceImpl_ is nullptr");
    int32_t ret = eqAlgoInterfaceImpl_->Apply(tmpin_, tmpout_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteEqNode SignalProcess Apply failed");
    return &outPcmBuffer_;
}

int32_t AudioSuiteEqNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteEqNode::SetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == setBandGains, ERROR, "SetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(eqAlgoInterfaceImpl_ != nullptr, ERROR, "eqAlgoInterfaceImpl_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(!value.empty(), ERROR, "Value is empty");

    paraName_ = name;
    paraValue_ = value;

    int32_t ret = eqAlgoInterfaceImpl_->SetParameter(value, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed");
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteEqNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("AudioSuiteEqNode::GetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == setBandGains, ERROR, "GetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "Eq BandGains is empty");

    value = paraValue_;
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS