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
#define LOG_TAG "AudioSuiteAissNode"
#endif

#include "audio_suite_aiss_node.h"
#include "audio_utils.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteAissNode::AudioSuiteAissNode()
    : AudioSuiteProcessNode(NODE_TYPE_AUDIO_SEPARATION)
{
    AUDIO_INFO_LOG("AudioSuiteAissNode create success");
}

AudioSuiteAissNode::~AudioSuiteAissNode()
{
    DeInit();
}

int32_t AudioSuiteAissNode::Init()
{
    if (isInit_ == true) {
        AUDIO_DEBUG_LOG("AudioSuiteAissNode has inited");
        return SUCCESS;
    }
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }
    if (!algoInterface_) {
        algoInterface_ =
            AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION, nodeParameter);
    }
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create Aiss algoInterface");

    if (algoInterface_->Init() != SUCCESS) {
        AUDIO_ERR_LOG("InitAlgorithm failed");
        return ERROR;
    }
    resultNumber = 2;
    SetAudioNodeFormat(AudioFormat{{CH_LAYOUT_STEREO, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});
    
    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;
    isInit_ = true;
    AUDIO_DEBUG_LOG("AudioSuiteAissNode Init success");
    return SUCCESS;
}

int32_t AudioSuiteAissNode::DeInit()
{
    isInit_ = false;
    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
    }
    algoInterface_ = nullptr;
    
    AUDIO_DEBUG_LOG("AudioSuiteAissNode DeInit success");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS