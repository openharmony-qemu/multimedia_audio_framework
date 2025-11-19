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
#define LOG_TAG "AudioSuiteSoundFieldNode"
#endif
#include <unordered_map>
#include "audio_suite_soundfield_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr AudioSamplingRate SOUNDFIELD_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat SOUNDFIELD_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel SOUNDFIELD_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout SOUNDFIELD_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;

static const std::unordered_map<SoundFieldType, iMedia_Surround_PARA> soundFieldParaMap = {
    {AUDIO_SUITE_SOUND_FIELD_FRONT_FACING, IMEDIA_SWS_SOUROUND_FRONT},
    {AUDIO_SUITE_SOUND_FIELD_GRAND, IMEDIA_SWS_SOUROUND_GRAND},
    {AUDIO_SUITE_SOUND_FIELD_NEAR, IMEDIA_SWS_SOUROUND_DEFAULT},
    {AUDIO_SUITE_SOUND_FIELD_WIDE, IMEDIA_SWS_SOUROUND_BROAD}
};
}

AudioSuiteSoundFieldNode::AudioSuiteSoundFieldNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_SOUND_FIELD,
          AudioFormat{{SOUNDFIELD_ALGO_CHANNEL_LAYOUT, SOUNDFIELD_ALGO_CHANNEL_COUNT},
              SOUNDFIELD_ALGO_SAMPLE_FORMAT,
              SOUNDFIELD_ALGO_SAMPLE_RATE}),
      outPcmBuffer_(PcmBufferFormat{SOUNDFIELD_ALGO_SAMPLE_RATE,
          SOUNDFIELD_ALGO_CHANNEL_COUNT,
          SOUNDFIELD_ALGO_CHANNEL_LAYOUT,
          SOUNDFIELD_ALGO_SAMPLE_FORMAT})
{}

AudioSuiteSoundFieldNode::~AudioSuiteSoundFieldNode()
{
    DeInit();
}

int32_t AudioSuiteSoundFieldNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteSoundFieldNode::Init begin");

    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_SOUND_FIELD, nodeCapability);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create soundField algoInterface");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to Init soundField algorithm");

    AUDIO_INFO_LOG("AudioSuiteSoundFieldNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteSoundFieldNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteSoundFieldNode::DeInit begin");

    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_.reset();
    }

    AUDIO_INFO_LOG("AudioSuiteSoundFieldNode::DeInit end");
    return SUCCESS;
}

int32_t AudioSuiteSoundFieldNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("SoundField node SetOptions [%{public}s]: %{public}s", name.c_str(), value.c_str());

    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");

    CHECK_AND_RETURN_RET_LOG(name == "SoundFieldType", ERROR, "SetOptions Unknow Type %{public}s", name.c_str());
    
    paraName_ = name;
    paraValue_ = value;

    // convert from SoundFieldType to iMedia_Surround_PARA
    auto it = soundFieldParaMap.find(static_cast<SoundFieldType>(std::stoi(value)));
    if (it != soundFieldParaMap.end()) {
        int32_t ret = algoInterface_->SetParameter(name, std::to_string(static_cast<int32_t>(it->second)));
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetOptions fail");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions Unknown value %{public}s", value.c_str());
        return ERROR;
    }
}

int32_t AudioSuiteSoundFieldNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("SoundField node GetOptions [%{public}s]", name.c_str());

    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");

    CHECK_AND_RETURN_RET_LOG(name == "SoundFieldType", ERROR, "GetOptions Unknown Para name: %{public}s", name.c_str());

    std::string tempValue = "";
    int32_t ret = algoInterface_->GetParameter(name, tempValue);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetOptions fail");

    // convert from iMedia_Surround_PARA to SoundFieldType
    iMedia_Surround_PARA paraValue = static_cast<iMedia_Surround_PARA>(std::stoi(tempValue));
    for (const auto& pair : soundFieldParaMap) {
        if (pair.second == paraValue) {
            value = std::to_string(static_cast<int32_t>(pair.first));
            AUDIO_INFO_LOG("SoundField node GetOptions success [%{public}s]: %{public}s", name.c_str(), value.c_str());
            return SUCCESS;
        }
    }

    AUDIO_ERR_LOG("GetOptions Unknown value %{public}s", tempValue.c_str());
    return ERROR;
}

AudioSuitePcmBuffer *AudioSuiteSoundFieldNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterface is nullptr, need Init first");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid input format");

    uint32_t inputDataSize = inputs[0]->GetDataSize();
    uint32_t frameSize = SOUNDFIELD_ALGO_FRAME_SIZE;
    uint32_t frameCount = inputDataSize / frameSize;
    CHECK_AND_RETURN_RET_LOG(inputDataSize % frameSize == 0, nullptr, "Invalid inputPcmBuffer size");

    uint8_t *inDataPtr = inputs[0]->GetPcmData();
    uint8_t *outDataPtr = outPcmBuffer_.GetPcmData();
    // apply algo for every frame
    for (uint32_t i = 0; i < frameCount; i++) {
        algoInputs_[0] = inDataPtr;
        algoOutputs_[0] = outDataPtr;

        int32_t ret = algoInterface_->Apply(algoInputs_, algoOutputs_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "soundField node Apply algo fail");

        inDataPtr += frameSize;
        outDataPtr += frameSize;
    }

    return &outPcmBuffer_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS