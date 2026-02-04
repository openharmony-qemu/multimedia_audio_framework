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
#include "audio_utils.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr AudioChannelLayout SOUNDFIELD_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
}

AudioSuiteSoundFieldNode::AudioSuiteSoundFieldNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_SOUND_FIELD)
{}

AudioSuiteSoundFieldNode::~AudioSuiteSoundFieldNode()
{
    DeInit();
}

int32_t AudioSuiteSoundFieldNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteSoundFieldNode::Init begin");
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }
    
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_SOUND_FIELD, nodeParameter_);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create soundField algoInterface");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to Init soundField algorithm");

    SetAudioNodeFormat(AudioFormat{{SOUNDFIELD_ALGO_CHANNEL_LAYOUT, nodeParameter_.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter_.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter_.inSampleRate)});

    CHECK_AND_RETURN_RET_LOG(nodeParameter_.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    nodeNeedDataDuration_  = (nodeParameter_.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter_.inSampleRate;
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

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS