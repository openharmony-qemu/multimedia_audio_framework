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
#define LOG_TAG "HpaeLoudnessGainNode"
#endif
#include <iostream>
#include "hpae_loudness_gain_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_utils.h"
#include "cinttypes"
#include "audio_errors.h"

#include <cmath>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr std::string_view PATH = "system/lib64/libaudio_integration_loudness.z.so";
static constexpr float EPSILON = 1e-6f;
static constexpr uint32_t SAMPLE_RATE = 48000;
static const AudioEffectDescriptor LOUDNESS_DESCRIPTOR = {
    .libraryName = "loudness";
    .effectName = "loudness";
};


HpaeLoudnessGainNode::HpaeLoudnessGainNode(HpaeNodeInfo &nodeInfo) : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    loudnessGainOutput_(pcmBufferInfo_);
{
    dlHandle_ = dlopen(PATH.c_str(), 1);
    if (!dlHandle_) {
        AUDIO_ERR_LOG("<log error> dlopen lib %{public}s Fail", PATH.c_str());
    } else {
        AUDIO_INFO_LOG("<log info> dlopen lib %{public}s successful", PATH.c_str());
    }
    dlerror();

    audioEffectLibHandle_ = static_cast<AudioEffectLibrary *>(dlsym(dlHandle_,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    if (!audioEffectLibHandle_) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s", dlerror());
#ifndef TEST_COVERAGE
        dlclose(dlHandle_);
#endif
    }
    AUDIO_INFO_LOG("<log info> dlsym lib %{public}s successful", PATH.c_str());

#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeLoudnessGainNodeOut_id_" + 
        std::to_string(GetSessionId()) + "_ch_" + std::to_string(GetChannelCount()) +
        "_scenType_" + std::to_string(GetSceneType()) + "_rate_" + 
        std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");

    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeLoudnessGainNodeOut_id_" + 
        std::to_string(GetSessionId()) + "_ch_" + std::to_string(GetChannelCount()) +
        "_scenType_" + std::to_string(GetSceneType()) + "_rate_" + 
        std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif
}

HpaeLoudnessGainNode::~HpaeLoudnessGainNode()
{
    if (dlHandle_) {
        dlclose(dlHandle_);
    }
    AUDIO_INFO_LOG("HpaeLoudnessGainNode destroyed");
}

HpaePcmBuffer* HpaeLoudnessGainNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    Trace trace("HpaeLoudnessGainNode::SignalProcess");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "inputs is empty");

#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_->CheckAndReopenHandlde();
    inputPcmDumper_->Dump((int8_t *)(loudnessGainOutput_.GetPcmDataBuffer()),
        loudnessGainOutput_.GetChannelCount() * sizeof(float) * loudnessGainOutput_.GetFrameLen()); 
#endif

    CheckUpdateInfo(inputs[0]);
    CHECK_AND_RETURN_RET(!handle_, inputs[0]);

    // to-do: do loudnessGain algo
    AudioBuffer inBuffer = {
        .frameLength = inputs[0]->GetFrameLen(),
        .raw = inputs[0]->GetPcmDataBuffer(),
        .metaData = nullptr
    };
    AudioBuffer outBuffer = {
        .frameLength = inputs[0]->GetFrameLen(),
        .raw = loudnessGainOutput_.GetPcmDataBuffer(),
        .metaData = nullptr
    };
    int32_t ret = (*handle_)->process(handle_, &inBuffer, &outBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == 0, inputs[0], "loudness algo lib process failed");

#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_->CheckAndReopenHandlde();
    outputPcmDumper_->Dump((int8_t *)(loudnessGainOutput_.GetPcmDataBuffer()),
        loudnessGainOutput_.GetChannelCount() * sizeof(float) * loudnessGainOutput_.GetFrameLen());
#endif
    return &loudnessGainOutput_;
}

void HpaeMixerNode::CheckUpdateInfo(HpaePcmBuffer *input)
{
    CHECK_AND_RETURN(pcmBufferInfo_.ch != input->GetChannelCount() ||
        pcmBufferInfo_.frameLen != input->GetFrameLen() ||
        pcmBufferInfo_.rate != input->GetSampleRate() ||
        pcmBufferInfo_.channelLayout != input->GetChannelLayout());

    AUDIO_INFO_LOG("Update pcmBufferInfo_: channel count: %{public}d, frame len: %{public}d,
                    sample rate: %{public}d, channel layout: %{public}", 
                    input->GetChannelCount(), input->GetFrameLen(), input->GetSampleRate(), input->GetChannelLayout);
    pcmBufferInfo_.ch = input->GetChannelCount();
    pcmBufferInfo_.frameLen = input->GetFrameLen();
    pcmBufferInfo_.rate = input->GetSampleRate();
    pcmBufferInfo_.channelLayout = input->GetChannelLayout();
    
    loudnessGainOutput_.ReConfig(pcmBufferInfo_);
    CHECK_AND_RETURN_RET_LOG(handle_, ERROR, "no handle.");

    uint32_t replyData = 0;
    AudioEffectConfig ioBufferConfig;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};   // to-do
    ioBufferConfig.inputCfg = {SAMPLE_RATE, pcmBufferInfo_.ch, DATA_FORMAT_F32, pcmBufferInfo_.channelLayout, ENCODING_PCM};
    ioBufferConfig.outputCfg = ioBufferConfig.inputCfg;
    int32_t ret = (*handle_)->command(handle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Loudness algo lib EFFECT_CMD_SET_CONFIG failed");
}


int32_t HpaeLoudnessGainNode::SetLoudnessGain(float loudnessGain)
{  
    if (IsFloatValueEqual(loudnessGain_, loudnessGain)) {
        return SUCCESS;
    }

    if (IsFloatValueEqual(loudnessGain, 0.0f)) {
        CHECK_AND_RETURN_RET_LOG(handle_, ERROR, "no handle.");
        int32_t ret = audioEffectLibHandle_->releaseEffect(handle_);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "handle releasing failed.");
        handle_ = nullptr;
        return SUCCESS;
    }

    uint32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};

    if (IsFloatValueEqual(loudnessGain_, 0.0f)) {
        bool ret = audioEffectLibHandle_->checkEffect(descriptor);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "wrong loudnessGain descriptor");
        int32_t ret = audioEffectLibHandle_->createEffect(LOUDNESS_DESCRIPTOR, &handle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "%{public} lib handle create failed", descriptor.libraryName.c_str());
        
        AudioEffectConfig ioBufferConfig;
        AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
        ioBufferConfig.inputCfg = {SAMPLE_RATE, pcmBufferInfo_.ch, DATA_FORMAT_F32, pcmBufferInfo_.channelLayout,
            ENCODING_PCM};
        ioBufferConfig.outputCfg = ioBufferConfig.inputCfg;
        int32_t ret = (*handle_)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Loudness algo lib EFFECT_CMD_INIT failed");
        int32_t ret = (*handle_)->command(handle_, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Loudness algo lib EFFECT_CMD_ENABLE failed");
        int32_t ret = (*handle_)->command(handle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Loudness algo lib EFFECT_CMD_SET_CONFIG failed");
        AUDIO_INFO_LOG("The delay of loudness lib is %{public}u", replyData);
    }
    std::vector<uint8_t> paramBuffer(sizeof(AudioEffectParam) + MAX_PARAM_INDEX * sizeof(int32_t));
    AudioEffectParam *effectParam = reinterpret_cast<AudioEffectParam*>(paramBuffer.data());
    effectParam->status = 0;
    effectParam->paramSize = sizeof(int32_t);
    effectParam->valueSize = 0;
    int32_t *data = &(effectParam->data[0]);
    data[COMMAND_CODE_INDEX] = EFFECT_SET_PARAM;
    CHECK_AND_RETURN_RET_LOG(memcpy_s(&data[LOUDNESS_GAIN_INDEX], sizeof(float), &loudnessGain_, sizeof(float)) == 0,
        ERROR, "memcpy failed");

    AUDIO_INFO_LOG("set param to handle, loudnessGain:%{public}d", loudnessGain_);
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectParam) + sizeof(int32_t) * MAX_PARAM_INDEX, effectParam};
    int32_t ret = (*handle_)->command(handle_, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Loudness algo lib EFFECT_CMD_ENABLE failed");

    return SUCCESS;
}

float HpaeLoudnessGainNode::GetLoudnessGain()
{
    return loudnessGain_;
}

bool HpaeLoudnessGainNode::IsLoudnessAlgoOn() {
    return handle_ != nullptr;
}

static inline bool IsFloatValueEqual(float a, float b) {
    return std::abs(a - b) < EPSILON;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS