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
#define LOG_TAG "AudioSuitePureVoiceChangeNode"
#endif

#include "audio_suite_pure_voice_change_node.h"
#include <fstream>
#include "audio_utils.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout VMPH_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string PURE_VOICE_CHANGE_MODE = "AudioPureVoiceChangeOption";
}  // namespace

AudioSuitePureVoiceChangeNode::AudioSuitePureVoiceChangeNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_PURE_VOICE_CHANGE)

{}

AudioSuitePureVoiceChangeNode::~AudioSuitePureVoiceChangeNode()
{
    DeInit();
}

int32_t AudioSuitePureVoiceChangeNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuitePureVoiceChangeNode::Init failed, already inited");
        return ERROR;
    }
    
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
        AUDIO_ERR_LOG("InitOutputStream SUCCESS");
    }
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_PURE_VOICE_CHANGE, nodeParameter_);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create nr algoInterface");
    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "algoInterface_ Init failed");
    nodeParameter_.inChannels = STEREO;
    nodeParameter_.outChannels = STEREO;
    SetAudioNodeFormat(AudioFormat{{VMPH_ALGO_CHANNEL_LAYOUT, nodeParameter_.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter_.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter_.inSampleRate)});

    CHECK_AND_RETURN_RET_LOG(nodeParameter_.inSampleRate != 0, ERROR, "Invalid input SampleRate");

    nodeNeedDataDuration_ =
        static_cast<uint64_t>(nodeParameter_.frameLen) * MILLISECONDS_TO_MICROSECONDS / nodeParameter_.inSampleRate;

    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuitePureVoiceChangeNode::Init end");
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeNode::DeInit()
{
    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuitePureVoiceChangeNode::DeInit end");
        return SUCCESS;
    }

    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_ = nullptr;
    }
    return ERROR;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS