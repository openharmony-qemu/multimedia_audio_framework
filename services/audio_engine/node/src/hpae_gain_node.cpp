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
#define LOG_TAG "HpaeGainNode"
#endif

#include <algorithm>
#include <cmath>
#include "hpae_gain_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_volume.h"
#include "audio_utils.h"
#include "securec.h"
#include "volume_tools_c.h"
#include "audio_stream_info.h"
#include "hpae_info.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr float FADE_LOW = 0.0f;
static constexpr float FADE_HIGH = 1.0f;
static constexpr float SHORT_FADE_PERIOD = 0.005f; // 5ms fade for 10ms < playback duration <= 40ms
static constexpr float EPSILON = 1e-6f;

HpaeGainNode::HpaeGainNode(HpaeNodeInfo &nodeInfo) : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo)
{
    isInnerCapturer_ = !GetDeviceClass().compare(0, strlen(INNER_CAPTURER_SINK), INNER_CAPTURER_SINK);
    auto audioVolume = AudioVolume::GetInstance();
    float curSystemGain = 1.0f;
    if (isInnerCapturer_) {
        curSystemGain = audioVolume->GetStreamVolume(GetSessionId());
    } else {
        struct VolumeValues volumes;
        curSystemGain = audioVolume->GetVolume(GetSessionId(), GetStreamType(), GetDeviceClass(), &volumes);
    }
    audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain);
    audioVolume->Monitor(GetSessionId(), true);
    AUDIO_INFO_LOG("curSystemGain:%{public}f streamType :%{public}d", curSystemGain, GetStreamType());
    AUDIO_INFO_LOG(
        "SessionId:%{public}u deviceClass :%{public}s", GetSessionId(), GetDeviceClass().c_str());
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeGainNodeOut_id_" + std::to_string(GetSessionId()) + "_ch_" +
                                                       std::to_string(GetChannelCount()) + "_rate_" +
                                                       std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeGainNode");
#endif
}

HpaeGainNode::~HpaeGainNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

HpaePcmBuffer *HpaeGainNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }
    auto rate = "rate[" + std::to_string(inputs[0]->GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(inputs[0]->GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(inputs[0]->GetFrameLen()) + "]";
    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeGainNode::SignalProcess " + rate + ch + len);
    if (fadeOutState_ == FadeOutState::DONE_FADEOUT) {
        AUDIO_INFO_LOG("fadeout done, set session %{public}d silence", GetSessionId());
        SilenceData(inputs[0]);
    }
    float *inputData = (float *)inputs[0]->GetPcmDataBuffer();
    uint32_t frameLen = inputs[0]->GetFrameLen();
    uint32_t channelCount = inputs[0]->GetChannelCount();
    
    if (needGainState_) {
        DoGain(inputs[0], frameLen, channelCount);
    }
    DoFading(inputs[0]);

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->Dump((int8_t *)(inputData), (frameLen * sizeof(float) * channelCount));
    }
#endif
    return inputs[0];
}

bool HpaeGainNode::SetClientVolume(float gain)
{
    preGain_ = curGain_;
    curGain_ = gain;
    isGainChanged_ = true;
    return true;
}

float HpaeGainNode::GetClientVolume()
{
    return curGain_;
}

void HpaeGainNode::SetFadeState(IOperation operation)
{
    operation_ = operation;
    // fade in
    if (operation_ == OPERATION_STARTED) {
        if (fadeInState_ == false) { // todo: add operation for softstart
            fadeInState_ = true;
        } else {
            AUDIO_WARNING_LOG("fadeInState already set");
        }
        fadeOutState_ = FadeOutState::NO_FADEOUT; // reset fadeOutState_
    }

    // fade out
    if (operation_ == OPERATION_PAUSED || operation_ == OPERATION_STOPPED) {
        if (fadeOutState_ == FadeOutState::NO_FADEOUT) {
            fadeOutState_ = FadeOutState::DO_FADEOUT;
        } else {
            AUDIO_WARNING_LOG("current fadeout state %{public}d, cannot prepare fadeout", fadeOutState_);
        }
    }
    AUDIO_DEBUG_LOG("fadeInState_[%{public}d], fadeOutState_[%{public}d]", fadeInState_, fadeOutState_);
}


void HpaeGainNode::DoFading(HpaePcmBuffer *input)
{
    if (!input->IsValid() && fadeOutState_ == FadeOutState::DO_FADEOUT) {
        AUDIO_WARNING_LOG("after drain, get invalid data, no need to do fade out");
        fadeOutState_ = FadeOutState::DONE_FADEOUT;
        auto statusCallback = GetNodeStatusCallback().lock();
        CHECK_AND_RETURN_LOG(statusCallback != nullptr, "statusCallback is null, cannot callback");
        statusCallback->OnFadeDone(GetSessionId());
        return;
    }
    AudioRawFormat rawFormat;
    rawFormat.format = SAMPLE_F32LE; // for now PCM in gain node is float32
    rawFormat.channels = GetChannelCount();
    uint32_t byteLength = 0;
    uint8_t *data = (uint8_t *)input->GetPcmDataBuffer();
    GetFadeLength(byteLength, input);
    int32_t bufferAvg = GetSimpleBufferAvg(data, byteLength);
    // do fade out
    if (fadeOutState_ == FadeOutState::DO_FADEOUT) {
        AUDIO_INFO_LOG("[%{public}d]: fade out started! buffer avg: %{public}d", GetSessionId(), bufferAvg);
        ProcessVol(data, byteLength, rawFormat, FADE_HIGH, FADE_LOW);
        fadeOutState_ = FadeOutState::DONE_FADEOUT;
        AUDIO_INFO_LOG("fade out done, session %{public}d callback to update status", GetSessionId());
        auto statusCallback = GetNodeStatusCallback().lock();
        CHECK_AND_RETURN_LOG(statusCallback != nullptr, "statusCallback is null, cannot callback");
        statusCallback->OnFadeDone(GetSessionId()); // if operation is stop or pause, callback
        return;
    }
    // do fade in
    if (fadeInState_) {
        if (!input->IsValid() || IsSilentData(input)) {
            AUDIO_DEBUG_LOG("[%{public}d]: silent or invalid data no need to do fade in", GetSessionId());
            return;
        }
        AUDIO_INFO_LOG("[%{public}d]: fade in started! buffer avg: %{public}d", GetSessionId(), bufferAvg);
        ProcessVol(data, byteLength, rawFormat, FADE_LOW, FADE_HIGH);
        fadeInState_ = false;
    }
}

void HpaeGainNode::SilenceData(HpaePcmBuffer *pcmBuffer)
{
    void *data = pcmBuffer->GetPcmDataBuffer();
    if (GetNodeInfo().format == INVALID_WIDTH) {
        AUDIO_WARNING_LOG("HpaePcmBuffer.SetDataSilence: invalid format");
    } else if (GetNodeInfo().format == SAMPLE_U8) {
        // set silence data for all the frames
        memset_s(data, pcmBuffer->Size(), 0x80, pcmBuffer->Size());
    } else {
        memset_s(data, pcmBuffer->Size(), 0, pcmBuffer->Size());
    }
}

void HpaeGainNode::DoGain(HpaePcmBuffer *input, uint32_t frameLen, uint32_t channelCount)
{
    struct VolumeValues volumes;
    float *inputData = (float *)input->GetPcmDataBuffer();
    AudioVolume *audioVolume = AudioVolume::GetInstance();
    float curSystemGain = 1.0f;
    float preSystemGain = 1.0f;
    if (isInnerCapturer_) {
        curSystemGain = audioVolume->GetStreamVolume(GetSessionId());
        preSystemGain = audioVolume->GetHistoryVolume(GetSessionId());
    } else {
        curSystemGain = audioVolume->GetVolume(GetSessionId(), GetStreamType(), GetDeviceClass(), &volumes);
        preSystemGain = volumes.volumeHistory;
    }
    CHECK_AND_RETURN_LOG(frameLen != 0, "framelen is zero, invalid val.");
    float systemStepGain = (curSystemGain - preSystemGain) / frameLen;
    AUDIO_DEBUG_LOG(
        "curSystemGain:%{public}f, preSystemGain:%{public}f, systemStepGain:%{public}f deviceClass :%{public}s",
        curSystemGain,
        preSystemGain,
        systemStepGain,
        GetDeviceClass().c_str());
    if (audioVolume->IsSameVolume(0.0f, curSystemGain) && audioVolume->IsSameVolume(0.0f, preSystemGain)) {
        SilenceData(input);
        input->SetBufferSilence(true);
    } else {
        for (uint32_t i = 0; i < frameLen; i++) {
            for (uint32_t j = 0; j < channelCount; j++) {
                inputData[channelCount * i + j] =
                    inputData[channelCount * i + j] * (preSystemGain + systemStepGain * i);
            }
        }
        input->SetBufferSilence(false);
    }
    if (fabs(curSystemGain - preSystemGain) > EPSILON) {
        audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain);
        audioVolume->Monitor(GetSessionId(), true);
    }
}

bool HpaeGainNode::IsSilentData(HpaePcmBuffer *pcmBuffer)
{
    float *data = pcmBuffer->GetPcmDataBuffer();
    size_t length = pcmBuffer->Size() / sizeof(float);
    AUDIO_DEBUG_LOG("HpaeGainNode::Data length:%{public}zu", length);
    return std::all_of(data, data + length, [](float value) {
        return fabs(value) < EPSILON;
        });
}

void HpaeGainNode::GetFadeLength(uint32_t &byteLength, HpaePcmBuffer *input)
{
    uint32_t channels = GetChannelCount();
    switch (GetNodeInfo().fadeType) {
        case FadeType::SHORT_FADE: {
            byteLength = static_cast<float>(GetSampleRate()) * SHORT_FADE_PERIOD * channels * sizeof(float);
            AUDIO_DEBUG_LOG("GainNode: short fade length in Bytes: %{public}u", byteLength);
            break;
        }
        case FadeType::DEFAULT_FADE: {
            byteLength = input->DataSize();
            AUDIO_DEBUG_LOG("GainNode: default fade length in Bytes: %{public}u", byteLength);
            break;
        }
        default:
            break;
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS