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
#define LOG_TAG "AudioSuiteGeneralVoiceChangeNode"
#endif

#include "audio_suite_general_voice_change_node.h"
#include "audio_utils.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout VM_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string setVoiceChangeMode = "AudioGeneralVoiceChangeType";
}

AudioSuiteGeneralVoiceChangeNode::AudioSuiteGeneralVoiceChangeNode()
    : AudioSuiteProcessNode(NODE_TYPE_GENERAL_VOICE_CHANGE)
{}

AudioSuiteGeneralVoiceChangeNode::~AudioSuiteGeneralVoiceChangeNode()
{
    DeInit();
}

int32_t AudioSuiteGeneralVoiceChangeNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode Init begin");
    nodeName = setVoiceChangeMode;
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_GENERAL_VOICE_CHANGE, nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create General Voice Change algoInterface");
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to Init General Voice Change Algo");

    SetAudioNodeFormat(AudioFormat{{VM_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});
    
    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;

    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode Init end");
    return SUCCESS;
}

int32_t AudioSuiteGeneralVoiceChangeNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode DeInit begin");
    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to DeInit voice beautifier algorithm");
        algoInterface_ = nullptr;
    }

    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode DeInit end");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS