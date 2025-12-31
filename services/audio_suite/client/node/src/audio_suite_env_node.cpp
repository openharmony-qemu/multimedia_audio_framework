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
#define LOG_TAG "AudioSuiteEnvNode"
#endif

#include "audio_suite_env_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout ENV_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string setEnvMode = "EnvironmentType";
}  // namespace

AudioSuiteEnvNode::AudioSuiteEnvNode()
    : AudioSuiteProcessNode(NODE_TYPE_ENVIRONMENT_EFFECT)
{}

AudioSuiteEnvNode::~AudioSuiteEnvNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuiteEnvNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuiteEnvNode::Init failed, already inited");
        return ERROR;
    }
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }
    envAlgoInterfaceImpl_ = std::make_shared<AudioSuiteEnvAlgoInterfaceImpl>(nodeCapability);
    int32_t ret = envAlgoInterfaceImpl_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "envAlgoInterfaceImpl Init failed");
    SetAudioNodeFormat(AudioFormat{{ENV_ALGO_CHANNEL_LAYOUT, nodeCapability.inChannels},
        static_cast<AudioSampleFormat>(nodeCapability.inFormat),
        static_cast<AudioSamplingRate>(nodeCapability.inSampleRate)});
    
    outPcmBuffer_ = AudioSuitePcmBuffer(PcmBufferFormat{static_cast<AudioSamplingRate>(nodeCapability.outSampleRate),
        nodeCapability.outChannels,
        ENV_ALGO_CHANNEL_LAYOUT,
        static_cast<AudioSampleFormat>(nodeCapability.outFormat)});
    CHECK_AND_RETURN_RET_LOG(nodeCapability.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = nodeCapability.frameLen / nodeCapability.inSampleRate * MILLISECONDS_TO_MICROSECONDS;
    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEnvNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::DeInit()
{
    if (envAlgoInterfaceImpl_ != nullptr) {
        envAlgoInterfaceImpl_->Deinit();
        envAlgoInterfaceImpl_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEnvNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

AudioSuitePcmBuffer *AudioSuiteEnvNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuiteEnvNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr, "AudioSuiteEnvNode SignalProcess inputs[0] is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid inputs format");

    tmpIn_[0] = inputs[0]->GetPcmData();
    tmpOut_[0] = outPcmBuffer_.GetPcmData();
    CHECK_AND_RETURN_RET_LOG(envAlgoInterfaceImpl_ != nullptr, nullptr, "envAlgoInterfaceImpl_ is nullptr");
    int32_t ret = envAlgoInterfaceImpl_->Apply(tmpIn_, tmpOut_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteEnvNode SignalProcess Apply failed");

    return &outPcmBuffer_;
}

int32_t AudioSuiteEnvNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteEnvNode::SetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == setEnvMode, ERROR, "SetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(envAlgoInterfaceImpl_ != nullptr, ERROR, "envAlgoInterfaceImpl_ is nullptr");
    
    paraName_ = name;
    paraValue_ = value;
    
    int32_t ret = envAlgoInterfaceImpl_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed");
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("AudioSuiteEnvNode::GetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == setEnvMode, ERROR, "GetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(envAlgoInterfaceImpl_ != nullptr, ERROR, "envAlgoInterfaceImpl_ is nullptr");
    
    int32_t ret = envAlgoInterfaceImpl_->GetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetParameter failed");
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS