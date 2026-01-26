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
#define LOG_TAG "AudioSuiteVoiceBeautifierNode"
#endif

#include "audio_suite_voice_beautifier_node.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr AudioChannelLayout VM_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
static std::string VOICE_BEAUTIFIER_TYPE = "VoiceBeautifierType";

AudioSuiteVoiceBeautifierNode::AudioSuiteVoiceBeautifierNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_VOICE_BEAUTIFIER)
{}

AudioSuiteVoiceBeautifierNode::~AudioSuiteVoiceBeautifierNode()
{
    DeInit();
}

int32_t AudioSuiteVoiceBeautifierNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode Init begin");
    nodeName = VOICE_BEAUTIFIER_TYPE;
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_VOICE_BEAUTIFIER, nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create voice beautifier algoInterface");

    int32_t ret = algoInterface_->Init();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to Init voice beautifier algorithm.");
        DeInit();
        return ret;
    }
    SetAudioNodeFormat(AudioFormat{{VM_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});

    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode Init end");
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode DeInit begin");
    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to DeInit voice beautifier algorithm");
        algoInterface_.reset();
    }

    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode DeInit end");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS