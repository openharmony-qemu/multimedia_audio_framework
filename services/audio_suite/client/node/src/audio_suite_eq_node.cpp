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
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout EQ_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string setBandGains = "AudioEqualizerFrequencyBandGains";
}  // namespace

AudioSuiteEqNode::AudioSuiteEqNode()
    : AudioSuiteProcessNode(NODE_TYPE_EQUALIZER)
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
    
    if (!isOutputStreamInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputStreamInit_ = true;
    }
    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_EQUALIZER, nodeParameter_);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create equalizer algoInterface");
    algoInterface_->Init();
    SetAudioNodeFormat(AudioFormat{{EQ_ALGO_CHANNEL_LAYOUT, nodeParameter_.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter_.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter_.inSampleRate)});
    CHECK_AND_RETURN_RET_LOG(nodeParameter_.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    nodeNeedDataDuration_ =
        static_cast<uint64_t>(nodeParameter_.frameLen) * MILLISECONDS_TO_MICROSECONDS / nodeParameter_.inSampleRate;

    isEqNodeInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEqNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEqNode::DeInit()
{
    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_ = nullptr;
    }

    if (isEqNodeInit_) {
        isEqNodeInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEqNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS