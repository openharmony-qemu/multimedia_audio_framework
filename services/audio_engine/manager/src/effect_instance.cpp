/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "EffectInstance"
#endif

#include "effect_instance.h"
#include "audio_engine_log.h"
#include <cstring>
#include <vector>

namespace {
constexpr uint32_t EFFECT_CMD_FLUSH = 10;  // Session Effect extended command
}
#include <algorithm>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

namespace {
const char *EffectStateToString(EffectInstance::EffectState s)
{
    switch (s) {
        case EffectInstance::EffectState::IDLE: return "IDLE";
        case EffectInstance::EffectState::ACTIVE: return "ACTIVE";
        case EffectInstance::EffectState::PENDING_RELEASE: return "PENDING_RELEASE";
        case EffectInstance::EffectState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
}  // namespace

EffectInstance::EffectInstance() {}

EffectInstance::~EffectInstance() {}

int32_t EffectInstance::Init(const std::string &effectName, const AudioEffectConfig &config)
{
    effectName_ = effectName;
    audioConfig_ = config;
    state_ = EffectState::ACTIVE;
    AUDIO_INFO_LOG("[SE::EffectInstance] →ACTIVE reason=Init effect=%{public}s sr=%{public}u ch=%{public}u",
        effectName_.c_str(), audioConfig_.inputCfg.samplingRate, audioConfig_.inputCfg.channels);
    return 0;
}

int32_t EffectInstance::Init(const std::string &effectName, const AudioEffectConfig &config,
    const AudioEffectLibrary *lib)
{
    effectName_ = effectName;
    audioConfig_ = config;

    if (lib == nullptr) {
        state_ = EffectState::ACTIVE;
        return 0;
    }

    effectLib_ = *lib;
    AudioEffectDescriptor desc;
    desc.libraryName = effectName;
    desc.effectName = effectName;

    int32_t ret = effectLib_.createEffect(desc, &effectHandle_);
    if (ret != 0 || effectHandle_ == nullptr) {
        AUDIO_ERR_LOG("createEffect failed for %{public}s, ret=%{public}d", effectName.c_str(), ret);
        state_ = EffectState::IDLE;
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_CREATE_FAILED);
    }
    hasHandle_ = true;

    // Send SET_CONFIG
    AudioEffectTransInfo cmdInfo = {};
    cmdInfo.size = sizeof(audioConfig_);
    cmdInfo.data = &audioConfig_;
    ret = (*effectHandle_)->command(effectHandle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, nullptr);
    if (ret != 0) {
        AUDIO_ERR_LOG("SET_CONFIG failed for %{public}s, ret=%{public}d", effectName.c_str(), ret);
    }

    state_ = EffectState::ACTIVE;
    AUDIO_INFO_LOG("[SE::EffectInstance] →ACTIVE reason=InitWithLibrary effect=%{public}s sr=%{public}u ch=%{public}u",
        effectName_.c_str(), audioConfig_.inputCfg.samplingRate, audioConfig_.inputCfg.channels);
    return 0;
}

int32_t EffectInstance::Release()
{
    AUDIO_INFO_LOG("[SE::EffectInstance] %s→IDLE reason=Release effect=%{public}s fedBytes=%{public}zu",
        EffectStateToString(state_), effectName_.c_str(), fedBytes_);
    if (hasHandle_ && effectHandle_ != nullptr) {
        effectLib_.releaseEffect(effectHandle_);
        effectHandle_ = nullptr;
        hasHandle_ = false;
    }
    skipFramesRemaining_ = 0;
    preheatFramesCached_ = false;
    fedBytes_ = 0;
    stats_.Reset();
    state_ = EffectState::IDLE;
    boundSessionId_ = 0;
    return 0;
}

EffectInstance::EffectState EffectInstance::GetState() const
{
    return state_;
}

void EffectInstance::SetState(EffectState state)
{
    state_ = state;
}

void EffectInstance::MarkPendingRelease()
{
    AUDIO_INFO_LOG("[SE::EffectInstance] ACTIVE→PENDING_RELEASE reason=MarkPendingRelease "
        "effect=%{public}s gen=%{public}u", effectName_.c_str(), releaseGeneration_ + 1);
    state_ = EffectState::PENDING_RELEASE;
    releaseGeneration_++;
    boundSessionId_ = 0;
}

uint32_t EffectInstance::GetBoundSessionId() const
{
    return boundSessionId_;
}

void EffectInstance::SetBoundSessionId(uint32_t sessionId)
{
    boundSessionId_ = sessionId;
}

const std::string &EffectInstance::GetEffectName() const
{
    return effectName_;
}

const AudioEffectConfig &EffectInstance::GetAudioConfig() const
{
    return audioConfig_;
}

uint32_t EffectInstance::GetReleaseGeneration() const
{
    return releaseGeneration_;
}

uint64_t EffectInstance::GetPendingTaskId() const
{
    return pendingTaskId_;
}

void EffectInstance::SetPendingTaskId(uint64_t taskId)
{
    pendingTaskId_ = taskId;
}

// Buffer management

int32_t EffectInstance::FeedInput(const uint8_t *data, size_t len)
{
    if (data == nullptr || len == 0) {
        return 0;
    }
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    auto *iface = *effectHandle_;
    if (iface->feedInput == nullptr) {
        return 0;
    }

    const float *fdata = reinterpret_cast<const float *>(data);
    size_t floatCount = len / sizeof(float);

    AudioBuffer inBuf = {};
    inBuf.f32 = const_cast<float *>(fdata);
    inBuf.frameLength = floatCount;

    int32_t ret = iface->feedInput(effectHandle_, &inBuf);
    if (ret != 0) {
        AUDIO_ERR_LOG("feedInput failed, ret=%{public}d", ret);
        stats_.processErrors++;
    } else {
        fedBytes_ += len;
        if (audioConfig_.inputCfg.channels > 0) {
            stats_.framesProcessed += static_cast<int64_t>(len / (audioConfig_.inputCfg.channels * sizeof(float)));
        }
    }

    // Threshold report
    if (stats_.framesProcessed >= statsReportThreshold_ && statsReportThreshold_ > 0) {
        AUDIO_INFO_LOG("[SE::EffectInstance] stats: %{public}s effect=%{public}s",
            stats_.ToString().c_str(), effectName_.c_str());
        stats_.Reset();
    }
    return ret;
}

int32_t EffectInstance::ReadOutput(uint8_t *buf, size_t len)
{
    if (buf == nullptr || len == 0) {
        return 0;
    }
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    auto *iface = *effectHandle_;
    if (iface->getOutput == nullptr) {
        return 0;
    }

    if (skipFramesRemaining_ > 0) {
        size_t floatLen = len / sizeof(float);
        size_t floatsToSkip = std::min(static_cast<size_t>(skipFramesRemaining_), floatLen);
        std::vector<float> skipBuf(floatsToSkip);
        AudioBuffer skipOut = {};
        skipOut.f32 = skipBuf.data();
        skipOut.frameLength = floatsToSkip;
        (void)iface->getOutput(effectHandle_, &skipOut);
        size_t actuallySkipped = std::min(static_cast<size_t>(skipOut.frameLength), floatsToSkip);
        skipFramesRemaining_ -= static_cast<uint32_t>(actuallySkipped);
        if (skipFramesRemaining_ > 0) {
            stats_.underrunCount++;
            return 0;
        }
    }

    float *fout = reinterpret_cast<float *>(buf);
    size_t floatLen = len / sizeof(float);

    AudioBuffer outBuf = {};
    outBuf.f32 = fout;
    outBuf.frameLength = floatLen;

    int32_t ret = iface->getOutput(effectHandle_, &outBuf);
    if (ret != 0) {
        AUDIO_ERR_LOG("getOutput failed, ret=%{public}d", ret);
        stats_.processErrors++;
        return 0;
    }
    if (audioConfig_.inputCfg.channels > 0) {
        stats_.framesOutput += static_cast<int64_t>(outBuf.frameLength / audioConfig_.inputCfg.channels);
    }
    return static_cast<int32_t>(outBuf.frameLength * sizeof(float));
}

size_t EffectInstance::GetInputLevel() const
{
    return fedBytes_;
}

size_t EffectInstance::GetOutputLevel() const
{
    return SIZE_MAX;
}

void EffectInstance::ClearBuffers()
{
    skipFramesRemaining_ = 0;
    fedBytes_ = 0;
    stats_.Reset();
}

int32_t EffectInstance::StartEffect()
{
    isEffectStarted_ = true;
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    AudioEffectTransInfo cmd = {};
    return (*effectHandle_)->command(effectHandle_, EFFECT_CMD_ENABLE, &cmd, nullptr);
}

int32_t EffectInstance::StopEffect()
{
    isEffectStarted_ = false;
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    AudioEffectTransInfo cmd = {};
    return (*effectHandle_)->command(effectHandle_, EFFECT_CMD_DISABLE, &cmd, nullptr);
}

int32_t EffectInstance::TriggerProcess()
{
    return 0;
}

int32_t EffectInstance::Flush()
{
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    AudioEffectTransInfo cmd = {};
    return (*effectHandle_)->command(effectHandle_, EFFECT_CMD_FLUSH, &cmd, nullptr);
}

int32_t EffectInstance::SetParameter(int32_t type, float value)
{
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }
    AudioEffectTransInfo cmdInfo = {};
    cmdInfo.size = sizeof(float);
    cmdInfo.data = &value;
    (void)type;
    return (*effectHandle_)->command(effectHandle_, EFFECT_CMD_SET_PARAM, &cmdInfo, nullptr);
}

uint32_t EffectInstance::GetPreheatFrames()
{
    if (preheatFramesCached_) {
        return preheatFrames_;
    }
    if (!hasHandle_ || effectHandle_ == nullptr) {
        return 0;
    }

    uint32_t frames = 0;
    AudioEffectTransInfo reply = {};
    reply.size = sizeof(uint32_t);
    reply.data = &frames;
    (*effectHandle_)->command(effectHandle_, EFFECT_CMD_GET_PARAM, nullptr, &reply);

    preheatFrames_ = frames;
    preheatFramesCached_ = true;
    return preheatFrames_;
}

void EffectInstance::SkipOutputFrames(uint32_t frames)
{
    skipFramesRemaining_ += frames;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
