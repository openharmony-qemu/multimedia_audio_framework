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
#include "audio_errors.h"
#include "util/id_handler.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

#ifndef CONFIG_FACTORY_VERSION
static constexpr float FADE_LOW = 0.0f;
static constexpr float FADE_HIGH = 1.0f;
#endif
static constexpr float SHORT_FADE_PERIOD = 0.005f; // 5ms fade for 10ms < playback duration <= 40ms
static constexpr float EPSILON = 1e-6f;

HpaeGainNode::HpaeGainNode(HpaeNodeInfo &nodeInfo, bool isFast)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo), isFast_(isFast)
{
    isInnerCapturerOrInjector_ = !GetDeviceClass().compare(0, strlen(INNER_CAPTURER_SINK), INNER_CAPTURER_SINK) ||
        GetDeviceClass() == VIRTUAL_INJECTOR;
    AUDIO_INFO_LOG(
        "SessionId:%{public}u deviceClass :%{public}s", GetSessionId(), GetDeviceClass().c_str());
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeGainNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    RebuildTraceName();
}

void HpaeGainNode::OnTraceInfoChanged()
{
    HpaePluginNode::OnTraceInfoChanged();
    RebuildTraceName();
}

// Trace reflects the node-configured format; it may lag behind mid-stream upstream re-negotiation
// (loudness/SE/effect-chain changes). Gain processing still uses per-frame input buffer values.
void HpaeGainNode::RebuildTraceName()
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]";
    traceName_ = "[" + std::to_string(GetSessionId()) + "]HpaeGainNode::SignalProcess " + rate + ch + len;
}

HpaeGainNode::~HpaeGainNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

HpaePcmBuffer *HpaeGainNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }
    Trace trace(traceName_);
    if (fadeOutState_ == FadeOutState::DONE_FADEOUT) {
        AUDIO_INFO_LOG("fadeout done, set session %{public}d silence", GetSessionId());
        SilenceData(inputs[0]);
    }
    float *inputData = (float *)inputs[0]->GetPcmDataBuffer();
    uint32_t frameLen = inputs[0]->GetFrameLen();
    uint32_t channelCount = inputs[0]->GetChannelCount();
    uint32_t sampleRate = inputs[0]->GetSampleRate();

#ifdef ENABLE_HOOK_PCM
    if (!outputPcmDumper_ || channelCount != GetChannelCount() || sampleRate != GetSampleRate()) {
        // update node info and dump info
        HpaeNodeInfo nodeInfo = GetNodeInfo();
        nodeInfo.channels = (AudioChannel)channelCount;
        nodeInfo.samplingRate = (AudioSamplingRate)sampleRate;
        SetNodeInfo(nodeInfo);
        outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeGainNodeOut_id_" + std::to_string(GetSessionId()) + "_nodeId_" + std::to_string(GetNodeId()) +
            "_ch_" + std::to_string(GetChannelCount()) +
            "_rate_" + std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
    }
#endif
    if (needGainState_) {
        DoGain(inputs[0], frameLen, channelCount);
    }
    if (fadeInState_ || fadeOutState_ == FadeOutState::DO_FADEOUT) {
        DoFading(inputs[0]);
    }
    if (collFadeInState_ || collFadeOutState_ == FadeOutState::DO_FADEOUT) {
        DoCollFading(inputs[0]);
    }

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

void HpaeGainNode::ResetVolume()
{
    auto audioVolume = AudioVolume::GetInstance();
    float curSystemGain = 1.0f;
    uint32_t rendererId = IdHandler::GetInstance().GetRenderIdByDeviceClass(GetDeviceClass(),
        IdHandler::SelectIdInfo(GetMacAddress(), GetDeviceNetId()));
    audioVolume->GetStreamAndPipeVolume(GetSessionId(), GetStreamType(), rendererId,
        streamVolume_, pipeVolume_);
    if (!isInnerCapturerOrInjector_ && streamVolume_ && pipeVolume_) {
        struct VolumeValues volumes;
        curSystemGain = audioVolume->GetVolume(streamVolume_, pipeVolume_, &volumes);
    } else if (streamVolume_) {
        // inner capturer/injector, or normal playback without pipeVolume_: apply stream volume alone
        curSystemGain = streamVolume_->GetTotalVolume();
    }
    audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain);
    audioVolume->Monitor(GetSessionId(), true);
    AUDIO_INFO_LOG("curSystemGain:%{public}f streamType :%{public}d", curSystemGain, GetStreamType());
}

void HpaeGainNode::ClearFadeState()
{
    // A reused/reconnected GainNode may carry stale fade state from its previous lifecycle
    // (e.g. DONE_FADEOUT left by a Pause). DONE_FADEOUT makes SignalProcess silence every frame,
    // so it must be cleared; otherwise the reused node stays muted permanently.
    fadeOutState_ = FadeOutState::NO_FADEOUT;
    // fadeInState_ is intentionally NOT armed here: a hot re-plumb of a cleanly playing session
    // (scene refresh / effect-mode switch / move / manager reload) must keep gain continuity;
    // re-arming the fade-in there is an audible 20 ms duck mid-speech. Arming is reserved for
    // lifecycle transitions, see ResetFadeState().
    fadeProcessedBytes_ = 0;
    totalFadeBytes_ = 0;
    collFadeOutState_ = FadeOutState::NO_FADEOUT;
    collFadeInState_ = false;
    collFadeProcessedBytes_ = 0;
    collTotalFadeBytes_ = 0;
    operation_ = OPERATION_INVALID; // clear stale op; harmless (SetFadeState reassigns before use)
    AUDIO_INFO_LOG("ClearFadeState, session:%{public}u, no fade armed", GetSessionId());
}

bool HpaeGainNode::HasPendingFade() const
{
    // True when the node is in an in-flight fade lifecycle: fade-in not finished, fade-out
    // pending or terminal (DONE_FADEOUT is a per-frame SilenceData gate), or collab fade
    // pending. Such a node needs the arm path on reconnect, otherwise resuming straight to
    // full volume after a muted/partial state pops. collFadeOutState_==DONE_FADEOUT is NOT a
    // silence gate (DoCollFading only acts on DO_FADEOUT), but it is included so a collab
    // stream mid-fade keeps the compensated fade-in instead of an uncompensated step.
    return fadeInState_ || fadeOutState_ != FadeOutState::NO_FADEOUT ||
        collFadeInState_ || collFadeOutState_ != FadeOutState::NO_FADEOUT;
}

void HpaeGainNode::ResetFadeState()
{
    ClearFadeState();
    fadeInState_ = true; // fade-in on reconnect to avoid pop
    AUDIO_INFO_LOG("ResetFadeState, session:%{public}u", GetSessionId());
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
        if (fadeInState_ == false) {
            fadeInState_ = true;
            fadeProcessedBytes_ = 0;
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
    // do fade out
    if (fadeOutState_ == FadeOutState::DO_FADEOUT) {
        ApplyFadeOut(input, fadeOutState_, false);
        return;
    }
    // do fade in
    if (fadeInState_) {
        ApplyFadeIn(input, fadeInState_, fadeProcessedBytes_, totalFadeBytes_);
    }
}

void HpaeGainNode::ApplyFadeOut(HpaePcmBuffer *input, FadeOutState &fadeOutState, bool isCollFade)
{
#ifndef CONFIG_FACTORY_VERSION
    AudioRawFormat rawFormat;
    rawFormat.format = SAMPLE_F32LE;
    rawFormat.channels = GetChannelCount();
    uint32_t byteLength = 0;
    uint8_t *data = (uint8_t *)input->GetPcmDataBuffer();
    GetFadeLength(byteLength, input);
    // SHORT_FADE derives byteLength from sample rate (5ms of data) without knowing the
    // real buffer geometry; clamp to actual data size to avoid out-of-bounds access on
    // spans shorter than 5ms (e.g. ultra-fast sinks).
    byteLength = std::min(byteLength, static_cast<uint32_t>(input->DataSize()));
    int32_t bufferAvg = GetSimpleBufferAvg(data, byteLength);
    auto ret = ProcessVol(data, byteLength, rawFormat, FADE_HIGH, FADE_LOW);
    AUDIO_INFO_LOG("[%{public}d]: fade out started! buffer avg: %{public}d", GetSessionId(), bufferAvg);
#else
    auto ret = SUCCESS;
    AUDIO_INFO_LOG("fade out done, session %{public}d callback to update status", GetSessionId());
#endif
    fadeOutState = FadeOutState::DONE_FADEOUT;
    auto statusCallback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN_LOG(statusCallback != nullptr, "statusCallback is null, cannot callback");
    if (isCollFade) {
        statusCallback->OnCollborationFadeDone(GetSessionId());
    } else {
        statusCallback->OnFadeDone(GetSessionId());
    }
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "do fade out fail");
}

void HpaeGainNode::ApplyFadeIn(HpaePcmBuffer *input, bool &fadeInState,
    uint32_t &processedBytes, uint32_t &totalFadeBytes)
{
    if (!input->IsValid() || (IsSilentData(input) && !isFast_)) {
        if (processedBytes > 0) {
            AUDIO_DEBUG_LOG("[%{public}d]: silent or invalid data no need to do fade in", GetSessionId());
            fadeInState = false;
            processedBytes = 0;
        }
        SilenceData(input);
        return;
    }
#ifndef CONFIG_FACTORY_VERSION
    if (GetNodeInfo().fadeType == FadeType::NONE_FADE) {
        fadeInState = false;
        return;
    }
    uint8_t *data = (uint8_t *)input->GetPcmDataBuffer();
    if (GetNodeInfo().fadeType == FadeType::SHORT_FADE) {
        AudioRawFormat rawFormat;
        rawFormat.format = SAMPLE_F32LE;
        rawFormat.channels = GetChannelCount();
        uint32_t byteLen = static_cast<float>(GetSampleRate()) * SHORT_FADE_PERIOD *
            rawFormat.channels * sizeof(float);
        // Clamp to actual data size: 5ms-derived length may exceed the buffer on
        // spans shorter than 5ms (e.g. ultra-fast sinks). See ApplyFadeOut.
        byteLen = std::min(byteLen, static_cast<uint32_t>(input->DataSize()));
        auto ret = ProcessVol(data, byteLen, rawFormat, FADE_LOW, FADE_HIGH);
        fadeInState = false;
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "do fade in fail");
        return;
    }
    ProcessDefaultFadeIn(input, data, fadeInState, processedBytes, totalFadeBytes);
#else
    fadeInState = false;
#endif
}

void HpaeGainNode::ProcessDefaultFadeIn(HpaePcmBuffer *input, uint8_t *data,
    bool &fadeInState, uint32_t &processedBytes, uint32_t &totalFadeBytes)
{
#ifndef CONFIG_FACTORY_VERSION
    AudioRawFormat rawFormat;
    rawFormat.format = SAMPLE_F32LE;
    rawFormat.channels = GetChannelCount();
    if (processedBytes == 0) {
        uint32_t byteLen = 0;
        uint32_t byteOffset = GetFadeInLength(byteLen, input);
        int32_t bufferAvg = GetSimpleBufferAvg(data, byteLen);
        AUDIO_INFO_LOG("[%{public}d]: fade in started! buffer avg: %{public}d", GetSessionId(), bufferAvg);
        totalFadeBytes = input->GetFrameLen() * rawFormat.channels * sizeof(float);
        CHECK_AND_RETURN_LOG(totalFadeBytes > 0, "totalFadeBytes is zero");
        uint32_t avail = std::min(byteLen, totalFadeBytes);
        float endGain = static_cast<float>(avail) / static_cast<float>(totalFadeBytes);
        auto ret = ProcessVol(data + byteOffset, avail, rawFormat, FADE_LOW, endGain);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "do fade in fail");
        processedBytes = avail;
    } else {
        uint32_t remain = totalFadeBytes - processedBytes;
        uint32_t avail = std::min(static_cast<uint32_t>(input->DataSize()), remain);
        float startGain = static_cast<float>(processedBytes) / static_cast<float>(totalFadeBytes);
        float endGain = static_cast<float>(processedBytes + avail) / static_cast<float>(totalFadeBytes);
        auto ret = ProcessVol(data, avail, rawFormat, startGain, endGain);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "do fade in fail");
        processedBytes += avail;
    }
    if (processedBytes >= totalFadeBytes || isFast_) {
        fadeInState = false;
        processedBytes = 0;
    }
#endif
}

void HpaeGainNode::SilenceData(HpaePcmBuffer *pcmBuffer)
{
    void *data = pcmBuffer->GetPcmDataBuffer();
    if (GetNodeInfo().format == INVALID_WIDTH) {
        AUDIO_WARNING_LOG("HpaePcmBuffer.SetDataSilence: invalid format");
    } else if (GetNodeInfo().format == SAMPLE_U8) {
        // set silence data for all the frames, 0x80 is the zero-amplitude value for unsigned 8-bit PCM
        memset_s(data, pcmBuffer->Size(), 0x80, pcmBuffer->Size());
    } else {
        memset_s(data, pcmBuffer->Size(), 0, pcmBuffer->Size());
    }
}

uint32_t HpaeGainNode::CalcRemainDurationMs(uint32_t duration, uint32_t frameLen, float *curSysGain, float *preSysGain)
{
    uint32_t remainDurationMs = 0;
    uint32_t sampleRate = static_cast<uint32_t>(GetSampleRate());
    uint32_t spaneInFrameMs = static_cast<uint32_t>((frameLen * 1000.0f) / sampleRate);
    uint32_t times = duration / spaneInFrameMs;
    if (times > 0) {
        *curSysGain = (*curSysGain - *preSysGain) / times + *preSysGain;
        remainDurationMs = duration - spaneInFrameMs;
    }
    return remainDurationMs;
}

void HpaeGainNode::EnsureVolumeAvailable()
{
    if (!streamVolume_ || (!isInnerCapturerOrInjector_ && !pipeVolume_)) {
        AudioVolume *audioVolume = AudioVolume::GetInstance();
        uint32_t rendererId = IdHandler::GetInstance().GetRenderIdByDeviceClass(GetDeviceClass(),
            IdHandler::SelectIdInfo(GetMacAddress(), GetDeviceNetId()));
        audioVolume->GetStreamAndPipeVolume(GetSessionId(), GetStreamType(), rendererId,
            streamVolume_, pipeVolume_);
    }
}

void HpaeGainNode::DoGain(HpaePcmBuffer *input, uint32_t frameLen, uint32_t channelCount)
{
    struct VolumeValues volumes;
    float *inputData = (float *)input->GetPcmDataBuffer();
    AudioVolume *audioVolume = AudioVolume::GetInstance();
    float curSystemGain = 1.0f;
    float preSystemGain = 1.0f;
    uint32_t durationMs = 0;
    uint32_t remainDurationMs = 0;
    EnsureVolumeAvailable();
    if (!isInnerCapturerOrInjector_ && streamVolume_ && pipeVolume_) {
        curSystemGain = audioVolume->GetVolume(streamVolume_, pipeVolume_, &volumes);
        preSystemGain = volumes.volumeHistory;
        durationMs = streamVolume_->GetDurationMs();
    } else if (streamVolume_) {
        // inner capturer/injector, or normal playback without pipeVolume_: apply stream volume alone
        curSystemGain = streamVolume_->GetTotalVolume();
        preSystemGain = streamVolume_->GetHistoryVolume();
        durationMs = streamVolume_->GetDurationMs();
    }

    remainDurationMs = CalcRemainDurationMs(durationMs, frameLen, &curSystemGain, &preSystemGain);

    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeGainNode::DoGain, curSystemGain: " +
        std::to_string(curSystemGain) + ", preSystemGain: " + std::to_string(preSystemGain)+ ", durationMs" +
        std::to_string(durationMs));
    CHECK_AND_RETURN_LOG(frameLen != 0, "framelen is zero, invalid val.");
    float systemStepGain = (curSystemGain - preSystemGain) / frameLen;
    AUDIO_DEBUG_LOG(
        "curSystemGain:%{public}f, preSystemGain:%{public}f, systemStepGain:%{public}f " \
        "durationMs: %{public}u deviceClass :%{public}s",
        curSystemGain, preSystemGain, systemStepGain, durationMs, GetDeviceClass().c_str());
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
        audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain, remainDurationMs);
        audioVolume->Monitor(GetSessionId(), true);
    }
}

bool HpaeGainNode::IsSilentData(HpaePcmBuffer *pcmBuffer)
{
    float *data = pcmBuffer->GetPcmDataBuffer();
    size_t length = pcmBuffer->DataSize() / sizeof(float);
    AUDIO_DEBUG_LOG("[%{public}d]: Data length:%{public}zu", GetSessionId(), length);
    return std::all_of(data, data + length, [](float value) {
        return fabs(value) < EPSILON;
        });
}

uint32_t HpaeGainNode::GetFadeLength(uint32_t &byteLength, HpaePcmBuffer *input)
{
    uint32_t index = 0;
    uint32_t channels = GetChannelCount();
    switch (GetNodeInfo().fadeType) {
        case FadeType::SHORT_FADE: {
            byteLength = static_cast<float>(GetSampleRate()) * SHORT_FADE_PERIOD * channels * sizeof(float);
            AUDIO_DEBUG_LOG("[%{public}d]: short fade length in Bytes: %{public}u", GetSessionId(), byteLength);
            break;
        }
        case FadeType::DEFAULT_FADE: {
            byteLength = input->DataSize();
            index = fadeInState_ ? GetFadeInLength(byteLength, input) : index;
            AUDIO_DEBUG_LOG("[%{public}d]: default fade length in Bytes: %{public}u", GetSessionId(), byteLength);
            break;
        }
        default:
            break;
    }
    return index;
}

uint32_t HpaeGainNode::GetFadeInLength(uint32_t &byteLength, HpaePcmBuffer *input)
{
    uint32_t index = 0;
    float *data = input->GetPcmDataBuffer();
    size_t length = input->DataSize() / sizeof(float);
    for (size_t i = 0; i < length; ++i) {
        if (fabs(data[i]) > EPSILON) {
            CHECK_AND_BREAK_LOG(GetChannelCount() > 0, "channel is zero!");
            index = i / GetChannelCount() * GetChannelCount() * sizeof(float);
            byteLength = input->DataSize() - index;
            break;
        }
    }
    return index;
}

uint64_t HpaeGainNode::GetLatency(uint32_t sessionId)
{
    return 0;
}

void HpaeGainNode::SetCollFadeState(IOperation operation)
{
    if (operation == OPERATION_STARTED) {
        collFadeInState_ = true;
        collFadeProcessedBytes_ = 0;
        collFadeOutState_ = FadeOutState::NO_FADEOUT; // reset fadeOutState_
    } else if (operation == OPERATION_PAUSED) {
        collFadeOutState_ = FadeOutState::DO_FADEOUT;
    }
}

void HpaeGainNode::DoCollFading(HpaePcmBuffer *input)
{
    // do fade out
    if (collFadeOutState_ == FadeOutState::DO_FADEOUT) {
        ApplyFadeOut(input, collFadeOutState_, true);
        return;
    }
    // do coll fade in
    if (collFadeInState_) {
        ApplyFadeIn(input, collFadeInState_, collFadeProcessedBytes_, collTotalFadeBytes_);
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS