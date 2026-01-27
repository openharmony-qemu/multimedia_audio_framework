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
#include "audio_suite_log.h"

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
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_ENVIRONMENT_EFFECT, nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create environment algoInterface");
    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "envAlgoInterfaceImpl Init failed");
    SetAudioNodeFormat(AudioFormat{{ENV_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});
    
    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;

    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEnvNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::DeInit()
{
    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEnvNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS