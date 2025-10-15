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

#include <vector>
#include <memory>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_env_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioSamplingRate ENV_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat ENV_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel ENV_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout ENV_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
}  // namespace

AudioSuiteEnvNode::AudioSuiteEnvNode()
    : AudioSuiteProcessNode(NODE_TYPE_ENVIRONMENT_EFFECT,
          AudioFormat{{ENV_ALGO_CHANNEL_LAYOUT, ENV_ALGO_CHANNEL_COUNT}, ENV_ALGO_SAMPLE_FORMAT, ENV_ALGO_SAMPLE_RATE}),
      outPcmBuffer_(ENV_ALGO_SAMPLE_RATE, ENV_ALGO_CHANNEL_COUNT, ENV_ALGO_CHANNEL_LAYOUT),
      tmpPcmBuffer_(ENV_ALGO_SAMPLE_RATE, ENV_ALGO_CHANNEL_COUNT, ENV_ALGO_CHANNEL_LAYOUT)
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
    envAlgoInterfaceImpl_ = std::make_shared<AudioSuiteEnvAlgoInterfaceImpl>();
    int32_t ret = envAlgoInterfaceImpl_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "envAlgoInterfaceImpl Init failed");
    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEnvNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::DeInit()
{
    tmpin_.resize(0);
    tmpout_.resize(0);
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

bool AudioSuiteEnvNode::Reset()
{
    return true;
}

AudioSuitePcmBuffer *AudioSuiteEnvNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_ERR_LOG("AudioSuiteEnvNode SignalProcess inputs is empty");
        return nullptr;
    } else {
        AUDIO_DEBUG_LOG("AudioSuiteEnvNode SignalProcess inputs frameLen:%{public}d", inputs[0]->GetFrameLen());
    }

    inputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    outputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    int32_t ret = ConvertProcess(inputs[0], &outPcmBuffer_, &tmpPcmBuffer_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, &outPcmBuffer_, "AudioSuiteEnvNode SignalProcess ConvertProcess failed");

    ConvertFromFloat(ENV_ALGO_SAMPLE_FORMAT,
        outPcmBuffer_.GetFrameLen(),
        outPcmBuffer_.GetPcmDataBuffer(),
        static_cast<void *>(inputDataBuffer_.data()));

    tmpin_.resize(1);
    tmpout_.resize(1);
    uint8_t *inputPointer = inputDataBuffer_.data();
    uint8_t *outputPointer = outputDataBuffer_.data();

    tmpin_[0] = inputPointer;
    tmpout_[0] = outputPointer;
    ret = envAlgoInterfaceImpl_->Apply(tmpin_, tmpout_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, &outPcmBuffer_, "AudioSuiteEnvNode SignalProcess Apply failed");

    ConvertToFloat(ENV_ALGO_SAMPLE_FORMAT,
        outPcmBuffer_.GetFrameLen(),
        outputDataBuffer_.data(),
        outPcmBuffer_.GetPcmDataBuffer());
    return &outPcmBuffer_;
}

int32_t AudioSuiteEnvNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteEnvNode::SetOptions Enter");
    if (name == "EnvironmentType") {
        CHECK_AND_RETURN_RET_LOG(envAlgoInterfaceImpl_ != nullptr, ERROR, "envAlgoInterfaceImpl_ is nullptr");
        envAlgoInterfaceImpl_->SetParameter(value, value);
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions Unknow Type %{public}s", name.c_str());
        return ERROR;
    }
}

int32_t AudioSuiteEnvNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("AudioSuiteEnvNode::GetOptions Enter");
    if (name == "EnvironmentType") {
        CHECK_AND_RETURN_RET_LOG(envAlgoInterfaceImpl_ != nullptr, ERROR, "envAlgoInterfaceImpl_ is nullptr");
        envAlgoInterfaceImpl_->GetParameter(value, value);
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions Unknow Type %{public}s", name.c_str());
        return ERROR;
    }
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS