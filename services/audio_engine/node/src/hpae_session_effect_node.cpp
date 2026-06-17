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
#define LOG_TAG "HpaeSessionEffectNode"
#endif

#include "hpae_session_effect_node.h"
#include "audio_engine_log.h"
#include "hpae_node_common.h"
#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <limits>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

namespace {
static constexpr uint32_t BACKOFF_MULTIPLIER = 2;
static constexpr uint32_t PREHEAT_NO_DATA_TIMEOUT_MS = 100;
static constexpr int32_t HALF_DIVISOR = 2;

bool GetBytesPerFrame(size_t channels, size_t &bytesPerFrame)
{
    if (channels == 0 || channels > std::numeric_limits<size_t>::max() / sizeof(float)) {
        return false;
    }
    bytesPerFrame = channels * sizeof(float);
    return true;
}

bool GetPcmByteLen(size_t frameLen, size_t channels, size_t &byteLen)
{
    size_t sampleCount = 0;
    if (channels == 0 || frameLen > std::numeric_limits<size_t>::max() / channels) {
        return false;
    }
    sampleCount = frameLen * channels;
    if (sampleCount > std::numeric_limits<size_t>::max() / sizeof(float)) {
        return false;
    }
    byteLen = sampleCount * sizeof(float);
    return true;
}

bool HasValidPcmInput(const std::vector<HpaePcmBuffer *> &inputs)
{
    return !inputs.empty() && inputs[0] != nullptr && inputs[0]->IsValid();
}
} // namespace

uint32_t HpaeSessionEffectNode::ComputeSkipFrames()
{
    auto now = std::chrono::steady_clock::now();
    int64_t elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
        now - warmupStartTime_).count();
    elapsedUs -= static_cast<int64_t>(pauseAccumulatedUs_);
    if (elapsedUs <= 0) {
        return 0;
    }
    return static_cast<uint32_t>(
        (elapsedUs + FRAME_LEN_20MS * MILLISECOND_PER_SECOND / HALF_DIVISOR)
        / (FRAME_LEN_20MS * MILLISECOND_PER_SECOND));
}

static const char *StatusToString(HpaeSessionEffectNode::STATUS s)
{
    switch (s) {
        case HpaeSessionEffectNode::STATUS::IDLE: return "IDLE";
        case HpaeSessionEffectNode::STATUS::WARMING_UP: return "WARMING_UP";
        case HpaeSessionEffectNode::STATUS::INTERCEPTING: return "INTERCEPTING";
        case HpaeSessionEffectNode::STATUS::ACTIVE: return "ACTIVE";
        case HpaeSessionEffectNode::STATUS::STOPPING: return "STOPPING";
        default: return "UNKNOWN";
    }
}

HpaeSessionEffectNode::HpaeSessionEffectNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
      tailOutputBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
      tailOutputData_(tailOutputBufferInfo_)
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeSessionEffectNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    AUDIO_INFO_LOG("SessionId:%{public}u created", GetSessionId());
}

HpaeSessionEffectNode::~HpaeSessionEffectNode()
{
    StopPreheatThread();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
    effectInstance_.reset();
    AUDIO_INFO_LOG("SessionId:%{public}u destructing", GetSessionId());
}

HpaePcmBuffer *HpaeSessionEffectNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    if (inputs.empty() || inputs[0] == nullptr) {
        return nullptr;
    }

    // --- Phase 3C: Close SE crossfade ---
    if (closeCrossfadePending_) {
        return ProcessCloseCrossfade(inputs);
    }

    HpaePcmBuffer *result = nullptr;

    STATUS processingStatus = status_;
    if (!effectInstance_) {
        result = inputs[0];
    } else {
        switch (processingStatus) {
            case STATUS::IDLE:
                result = inputs[0];
                break;
            case STATUS::WARMING_UP:
                result = &silenceData_;
                break;
            case STATUS::INTERCEPTING:
                result = ProcessIntercepting(inputs);
                break;
            case STATUS::ACTIVE:
                result = ProcessActive(inputs);
                break;
            case STATUS::STOPPING:
                result = &silenceData_;
                break;
            default:
                result = inputs[0];
                break;
        }
    }

    return result;
}

void HpaeSessionEffectNode::StartCloseCrossfade()
{
    closeCrossfadePending_ = true;
}

HpaePcmBuffer *HpaeSessionEffectNode::ProcessCloseCrossfade(const std::vector<HpaePcmBuffer *> &inputs)
{
    closeCrossfadePending_ = false;
    size_t byteLen = inputs[0]->GetFrameLen() * inputs[0]->GetChannelCount() * sizeof(float);

    if (effectInstance_ && status_ == STATUS::ACTIVE) {
        size_t channels = audioConfig_.inputCfg.channels;
        size_t frameLen = inputs[0]->GetFrameLen();
        size_t sampleCount = frameLen * channels;
        size_t algoByteLen = sampleCount * sizeof(float);

        float *raw = silenceData_.GetPcmDataBuffer();
        float *buf = inputs[0]->GetPcmDataBuffer();
        for (size_t i = 0; i < sampleCount; ++i) {
            raw[i] = buf[i];
        }

        FeedInputFromBuffer(inputs[0]);
        int32_t readRet = effectInstance_->ReadOutput(reinterpret_cast<uint8_t *>(buf), algoByteLen);
        if (readRet > 0) {
            ApplyCrossfadeMix(buf, raw, sampleCount);
        } else {
            for (size_t i = 0; i < sampleCount; ++i) {
                buf[i] = raw[i];
            }
        }
    }
    status_ = STATUS::IDLE;
    AccountOutput(byteLen);
    return inputs[0];
}

bool HpaeSessionEffectNode::IsCloseCrossfadePending() const
{
    return closeCrossfadePending_;
}

uint64_t HpaeSessionEffectNode::GetLatency(uint32_t sessionId)
{
    (void)sessionId;
    if (totalBytesIn_ <= totalBytesOut_) {
        return 0;
    }
    return BytesToLatencyUs(totalBytesIn_ - totalBytesOut_);
}

uint64_t HpaeSessionEffectNode::BytesToLatencyUs(size_t bytes) const
{
    uint32_t channels = audioConfig_.inputCfg.channels;
    uint32_t sampleRate = audioConfig_.inputCfg.samplingRate;
    size_t bytesPerFrame = 0;
    if (sampleRate == 0 || !GetBytesPerFrame(channels, bytesPerFrame)) {
        return 0;
    }
    uint64_t frames = bytes / bytesPerFrame;
    return frames * 1000000ULL / sampleRate;
}

void HpaeSessionEffectNode::ResetAccounting()
{
    totalBytesIn_ = 0;
    totalBytesOut_ = 0;
    paddingBytesIn_ = 0;
    stats_.Reset();
}

void HpaeSessionEffectNode::ResetTailPaddingState()
{
    activeNoDataStart_.reset();
    activeTailPaddingFrames_ = 0;
}

void HpaeSessionEffectNode::ReconfigOutputBuffers(const AudioEffectConfig &config)
{
    uint32_t samplingRate = config.outputCfg.samplingRate != 0 ?
        config.outputCfg.samplingRate : config.inputCfg.samplingRate;
    uint32_t channels = config.outputCfg.channels != 0 ? config.outputCfg.channels : config.inputCfg.channels;
    if (samplingRate == 0 || channels == 0) {
        AUDIO_WARNING_LOG("ReconfigOutputBuffers skipped: samplingRate=%{public}u channels=%{public}u",
            samplingRate, channels);
        return;
    }

    uint32_t frameLen = static_cast<uint32_t>(CalculateFrameLenBySampleRate(samplingRate));
    uint64_t channelLayout = config.outputCfg.channelLayout != 0 ?
        config.outputCfg.channelLayout : config.inputCfg.channelLayout;
    PcmBufferInfo pcmBufferInfo(channels, frameLen, samplingRate, channelLayout);

    silenceData_.ReConfig(pcmBufferInfo);
    silenceData_.Reset();
    silenceData_.SetBufferValid(false);
    silenceData_.SetBufferSilence(true);
    silenceData_.SetSplitStreamType(GetNodeInfo().GetSplitStreamType());
    silenceData_.SetAudioStreamType(GetNodeInfo().streamType);
    silenceData_.SetAudioStreamUsage(GetNodeInfo().effectInfo.streamUsage);

    tailOutputBufferInfo_ = pcmBufferInfo;
    tailOutputData_.ReConfig(tailOutputBufferInfo_);

    HpaeNodeInfo nodeInfo = GetNodeInfo();
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(samplingRate);
    nodeInfo.channels = static_cast<AudioChannel>(channels);
    nodeInfo.frameLen = frameLen;
    nodeInfo.channelLayout = static_cast<AudioChannelLayout>(channelLayout);
    SetNodeInfo(nodeInfo);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), GetNodeInfo());
    }
#endif
    AUDIO_INFO_LOG("ReconfigOutputBuffers: samplingRate=%{public}u channels=%{public}u frameLen=%{public}u",
        samplingRate, channels, frameLen);
}

int32_t HpaeSessionEffectNode::CreateEffectWithInstance(const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config)
{
    if (instance == nullptr) {
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_CREATE_FAILED);
    }
    effectInstance_ = std::move(instance);
    effectName_ = effectName;
    audioConfig_ = config;
    ReconfigOutputBuffers(config);
    UpdatePrebufTargetBytes();
    AUDIO_INFO_LOG("create effect with instance %{public}s", effectName.c_str());
    return 0;
}

int32_t HpaeSessionEffectNode::DropEffect()
{
    StopPreheatThread();  // Always join if joinable
    if (effectInstance_) {
        effectInstance_->StopEffect();  // DISABLE if not already
    }
    AUDIO_INFO_LOG("[SE::SessionEffectNode] %s→IDLE reason=DropEffect sessionId=%{public}u",
        StatusToString(status_), GetSessionId());
    effectInstance_.reset();
    status_ = STATUS::IDLE;
    prebufTargetBytes_ = 0;
    ResetAccounting();
    ResetTailPaddingState();
    return 0;
}

std::shared_ptr<IEffectInstance> HpaeSessionEffectNode::GetEffectInstance() const
{
    return effectInstance_;
}

std::string HpaeSessionEffectNode::GetEffectName() const
{
    return effectName_;
}

AudioEffectConfig HpaeSessionEffectNode::GetAudioConfig() const
{
    return audioConfig_;
}

size_t HpaeSessionEffectNode::GetPrebufTargetBytes() const
{
    return prebufTargetBytes_;
}

void HpaeSessionEffectNode::UpdatePrebufTargetBytes()
{
    prebufTargetBytes_ = 0;
    if (effectInstance_ == nullptr) {
        return;
    }

    uint32_t preheatFrames = effectInstance_->GetPreheatFrames();
    size_t frameLen = GetFrameLen();
    size_t frameBytes = 0;
    if (!GetPcmByteLen(frameLen, audioConfig_.inputCfg.channels, frameBytes)) {
        AUDIO_WARNING_LOG("UpdatePrebufTargetBytes failed: frameLen=%{public}zu channels=%{public}u",
            frameLen, audioConfig_.inputCfg.channels);
        return;
    }
    if (frameBytes != 0 && preheatFrames > std::numeric_limits<size_t>::max() / frameBytes) {
        AUDIO_WARNING_LOG("UpdatePrebufTargetBytes overflow: preheatFrames=%{public}u frameBytes=%{public}zu",
            preheatFrames, frameBytes);
        return;
    }

    prebufTargetBytes_ = static_cast<size_t>(preheatFrames) * frameBytes;
    AUDIO_INFO_LOG("Prebuf target=%{public}zu preheatFrames=%{public}u",
        prebufTargetBytes_, preheatFrames);
}

void HpaeSessionEffectNode::RestartPreheat()
{
    stopRequested_ = false;
    prebufFilled_ = false;
    if (prebufTargetBytes_ > 0) {
        StartPreheatThread();
    } else {
        prebufFilled_ = true;
    }
}

int32_t HpaeSessionEffectNode::StartEffect()
{
    if (!effectInstance_) return 0;

    // Resume from pause
    if (isPaused_) {
        isPaused_ = false;
        auto now = std::chrono::steady_clock::now();
        pauseAccumulatedUs_ += static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(now - pauseStartTime_).count());

        if (status_ == STATUS::WARMING_UP) {
            // Restart preheat thread with the target computed at effect creation.
            RestartPreheat();
            ResetTailPaddingState();
            AUDIO_INFO_LOG("StartEffect resume: WARMING_UP, restart preheat, prebuf=%{public}zu",
                prebufTargetBytes_);
        } else {
            // INTERCEPTING / ACTIVE: just clear isPaused_, DoProcess resumes naturally
            AUDIO_INFO_LOG("StartEffect resume: status=%{public}d", static_cast<int>(status_));
        }
        return 0;
    }

    AUDIO_INFO_LOG("StartEffect: prebufTargetBytes_=%{public}zu", prebufTargetBytes_);

    // Record warmup start timestamp for time-based skip calculation
    warmupStartTime_ = std::chrono::steady_clock::now();
    pauseAccumulatedUs_ = 0;

    // Enter WARMING_UP and start preheat thread (skip if no preheat needed)
    status_ = STATUS::WARMING_UP;
    ResetTailPaddingState();
    RestartPreheat();

    return 0;
}

int32_t HpaeSessionEffectNode::StopEffect()
{
    if (effectInstance_) {
        effectInstance_->StopEffect();  // Keep DISABLE behavior unchanged.
    }

    // Stop preheat thread if running
    if (status_ == STATUS::WARMING_UP) {
        StopPreheatThread();
    }
    auto oldStatus = status_;
    status_ = STATUS::IDLE;
    ResetTailPaddingState();
    AUDIO_INFO_LOG("[SE::SessionEffectNode] %s→IDLE reason=StopEffect sessionId=%{public}u totalBytesIn=%{public}zu",
        StatusToString(oldStatus), GetSessionId(), totalBytesIn_);
    return 0;
}

int32_t HpaeSessionEffectNode::PauseEffect()
{
    if (!effectInstance_) return 0;

    // Join preheat thread if running (WARMING_UP)
    if (status_ == STATUS::WARMING_UP) {
        StopPreheatThread();
    }

    // Set paused flag — status_ preserved for resume
    isPaused_ = true;
    pauseStartTime_ = std::chrono::steady_clock::now();

    // Key: do NOT call effectInstance_->StopEffect() (no DISABLE)
    // This preserves algorithm state and caches for resume
    AUDIO_INFO_LOG("PauseEffect: paused, status=%{public}d", static_cast<int>(status_));
    return 0;
}

int32_t HpaeSessionEffectNode::FlushEffect()
{
    if (!effectInstance_) return 0;
    int32_t ret = effectInstance_->Flush();
    pauseAccumulatedUs_ = 0;
    ResetAccounting();
    ResetTailPaddingState();

    if (isPaused_) {
        // Paused: reset to IDLE, clear isPaused_ (Seek scenario → full restart on next Start)
        status_ = STATUS::IDLE;
        isPaused_ = false;
        AUDIO_INFO_LOG("FlushEffect: paused → reset to IDLE");
    } else if (status_ == STATUS::WARMING_UP) {
        // WARMING_UP: stop preheat thread first (avoid data race), then restart preheat
        StopPreheatThread();
        warmupStartTime_ = std::chrono::steady_clock::now();
        RestartPreheat();
        AUDIO_INFO_LOG("FlushEffect: WARMING_UP restart preheat, prebufTarget=%{public}zu", prebufTargetBytes_);
    } else if (status_ == STATUS::INTERCEPTING || status_ == STATUS::ACTIVE) {
        status_ = STATUS::WARMING_UP;
        ResetTailPaddingState();
        warmupStartTime_ = std::chrono::steady_clock::now();
        RestartPreheat();
        AUDIO_INFO_LOG("FlushEffect: restart preheat, prebufTarget=%{public}zu", prebufTargetBytes_);
    }
    // IDLE + not paused: only reset counters (already done above)

    return ret;
}

int32_t HpaeSessionEffectNode::SetParameter(int32_t type, float value)
{
    return effectInstance_ ? effectInstance_->SetParameter(type, value) : 0;
}

void HpaeSessionEffectNode::DoProcess()
{
    // Paused: output silence, don't pull upstream
    if (isPaused_) {
        outputStream_.WriteDataToOutput(&silenceData_);
        return;
    }

    // Auto-trigger IDLE→WARMING_UP on first DoProcess when effect instance is bound
    if (status_ == STATUS::IDLE && effectInstance_ != nullptr) {
        StartEffect();
    }

    if (status_ == STATUS::WARMING_UP) {
        if (prebufFilled_.load()) {
            // Prebuf filled -> transition to INTERCEPTING
            AUDIO_INFO_LOG("[SE::SessionEffectNode] WARMING_UP→INTERCEPTING reason=prebuffer filled "
                "sessionId=%{public}u prebufBytes=%{public}zu",
                GetSessionId(), prebufTargetBytes_);
            status_ = STATUS::INTERCEPTING;
            // Fall through to normal DoProcess (pull upstream + SignalProcess)
        } else {
            // Disconnect: don't pull upstream, output silence directly
            outputStream_.WriteDataToOutput(&silenceData_);
            return;
        }
    }
    // Normal flow: pull upstream → SignalProcess (with timing)
    auto t0 = std::chrono::steady_clock::now();
    HpaePcmBuffer *tempOut = nullptr;
    std::vector<HpaePcmBuffer *> &preOutputs = inputStream_.ReadPreOutputData();
    bool hasInput = !preOutputs.empty() && preOutputs[0] != nullptr;
    if (hasInput) {
        tempOut = enableProcess_ ? SignalProcess(preOutputs) : preOutputs[0];
    } else if (!enableProcess_) {
        tempOut = SignalProcess(preOutputs);
    } else if (!hasInput && status_ == STATUS::ACTIVE) {
        tempOut = ProcessActiveNoInput();
    }
    outputStream_.WriteDataToOutput(tempOut != nullptr ? tempOut : &silenceData_);
    auto dt = std::chrono::steady_clock::now() - t0;
    float ms = std::chrono::duration<float, std::milli>(dt).count();
    stats_.lastProcessMs = ms;
    if (ms > stats_.maxProcessMs) {
        stats_.maxProcessMs = ms;
    }

    // Threshold report
    if (stats_.framesProcessed >= statsReportThreshold_ && statsReportThreshold_ > 0) {
        AUDIO_INFO_LOG("[SE::SessionEffectNode] stats: %{public}s sessionId=%{public}u",
            stats_.ToString().c_str(), GetSessionId());
        stats_.Reset();
    }
}

void HpaeSessionEffectNode::StartPreheatThread()
{
    if (preheatThread_.joinable()) {
        StopPreheatThread();
    }
    stopRequested_ = false;
    preheatThread_ = std::thread(&HpaeSessionEffectNode::PreheatThreadFunc, this);
    AUDIO_INFO_LOG("PreheatThread started for %{public}s", effectName_.c_str());
}

void HpaeSessionEffectNode::PreheatThreadFunc()
{
    AUDIO_INFO_LOG("PreheatThread running, actively pulling upstream data");
    uint32_t backoffMs = 0;
    bool noDataTimerActive = false;
    std::chrono::steady_clock::time_point noDataStartTime;

    while (!stopRequested_) {
        if (effectInstance_ == nullptr) {
            AUDIO_WARNING_LOG("PreheatThread exiting: effectInstance is null");
            break;
        }

        std::vector<HpaePcmBuffer *> &upstreamData = inputStream_.ReadPreOutputData();

        bool hasData = HasValidPcmInput(upstreamData);
        if (hasData) {
            size_t fedBytes = FeedInputFromBuffer(upstreamData[0]);
            if (fedBytes > 0) {
                noDataTimerActive = false;
                backoffMs = 0;
            } else {
                OnNoDataReceived(noDataTimerActive, noDataStartTime, backoffMs);
            }
        } else {
            OnNoDataReceived(noDataTimerActive, noDataStartTime, backoffMs);
        }

        if (stopRequested_) {
            break;
        }

        size_t inputLevel = effectInstance_->GetInputLevel();
        if (inputLevel >= prebufTargetBytes_) {
            AUDIO_INFO_LOG("Prebuf filled: %{public}zu >= %{public}zu",
                inputLevel, prebufTargetBytes_);
            break;
        }
        if (noDataTimerActive && HandleNoDataTimeout(inputLevel)) {
            break;
        }
    }

    prebufFilled_.store(true);
    AUDIO_INFO_LOG("PreheatThread exiting, stopRequested=%{public}d", stopRequested_.load());
}

void HpaeSessionEffectNode::OnNoDataReceived(bool &noDataTimerActive,
    std::chrono::steady_clock::time_point &noDataStartTime, uint32_t &backoffMs)
{
    static constexpr uint32_t MAX_BACKOFF_MS = 20;

    if (!noDataTimerActive) {
        noDataTimerActive = true;
        noDataStartTime = std::chrono::steady_clock::now();
        preheatNoDataStartTime_ = noDataStartTime;
    }
    backoffMs = backoffMs == 0 ? 1 : std::min(backoffMs * BACKOFF_MULTIPLIER, MAX_BACKOFF_MS);
    std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
}

bool HpaeSessionEffectNode::HandleNoDataTimeout(size_t inputLevel)
{
    auto noDataDuration = std::chrono::steady_clock::now() - preheatNoDataStartTime_;
    if (noDataDuration < std::chrono::milliseconds(PREHEAT_NO_DATA_TIMEOUT_MS)) {
        return false;
    }
    if (stopRequested_) {
        return true;
    }
    if (inputLevel >= prebufTargetBytes_) {
        return true;
    }
    size_t paddingBytes = prebufTargetBytes_;
    paddingBytes -= inputLevel;
    int32_t ret = FeedPaddingInput(paddingBytes);
    if (ret != 0) {
        AUDIO_WARNING_LOG("PreheatThread FeedPaddingInput failed: %{public}d", ret);
    }
    AUDIO_INFO_LOG("PreheatThread no-data timeout, padding=%{public}zu", paddingBytes);
    return true;
}

void HpaeSessionEffectNode::StopPreheatThread()
{
    if (!preheatThread_.joinable()) return;
    stopRequested_ = true;
    preheatThread_.join();
    AUDIO_INFO_LOG("PreheatThread joined");
}

HpaePcmBuffer *HpaeSessionEffectNode::ProcessIntercepting(const std::vector<HpaePcmBuffer *> &inputs)
{
    FeedInputFromBuffer(inputs[0]);

    HpaePcmBuffer *inputBuf = inputs[0];
    size_t channels = audioConfig_.inputCfg.channels;
    size_t byteLen = inputBuf->GetFrameLen() * channels * sizeof(float);

    uint32_t computedSkipFrames = ComputeSkipFrames();
    totalBytesOut_ = static_cast<size_t>(computedSkipFrames) * byteLen;

    uint32_t skipFrames = computedSkipFrames;
    int32_t ret = effectInstance_->ReadOutput(
        reinterpret_cast<uint8_t *>(inputBuf->GetPcmDataBuffer()), byteLen,
        &skipFrames);
    if (ret > 0) {
        inputBuf->SetBufferValid(true);
        inputBuf->SetBufferSilence(false);
        ApplyInterceptingFadeIn(inputBuf->GetPcmDataBuffer(),
            inputBuf->GetFrameLen() * audioConfig_.inputCfg.channels);
        status_ = STATUS::ACTIVE;
        AccountOutput(byteLen);
        return inputBuf;
    }

    return &silenceData_;
}

HpaePcmBuffer *HpaeSessionEffectNode::ProcessActive(const std::vector<HpaePcmBuffer *> &inputs)
{
    HpaePcmBuffer *inputBuf = inputs[0];
    size_t channels = audioConfig_.inputCfg.channels;
    size_t byteLen = inputBuf->GetFrameLen() * channels * sizeof(float);

    // Continue feeding upstream data to algorithm
    size_t fedBytes = FeedInputFromBuffer(inputBuf);
    if (fedBytes > 0) {
        ResetTailPaddingState();
    }

    HpaePcmBuffer *output = ReadAlgoOutput(inputBuf, byteLen);
    if (output == inputBuf) {
        AccountOutput(byteLen);
    }
    return output;
}

HpaePcmBuffer *HpaeSessionEffectNode::ProcessActiveNoInput()
{
    if (!effectInstance_ || status_ != STATUS::ACTIVE) {
        return &silenceData_;
    }

    size_t channels = tailOutputData_.GetChannelCount();
    if (audioConfig_.inputCfg.channels != channels) {
        AUDIO_WARNING_LOG("ProcessActiveNoInput channel mismatch: config=%{public}u buffer=%{public}zu",
            audioConfig_.inputCfg.channels, channels);
        return &silenceData_;
    }
    size_t byteLen = 0;
    if (!GetPcmByteLen(tailOutputData_.GetFrameLen(), channels, byteLen)) {
        return &silenceData_;
    }

    auto now = std::chrono::steady_clock::now();
    if (!activeNoDataStart_.has_value()) {
        activeNoDataStart_ = now;
    } else {
        auto noDataDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - *activeNoDataStart_);
        if (noDataDuration >= std::chrono::milliseconds(ACTIVE_NO_DATA_PADDING_DELAY_MS) &&
            activeTailPaddingFrames_ < ACTIVE_TAIL_PADDING_MAX_FRAMES) {
            int32_t ret = FeedPaddingInput(byteLen);
            activeTailPaddingFrames_++;
            (void)ret;
        }
    }

    tailOutputData_.Reset();
    tailOutputData_.SetBufferValid(true);
    tailOutputData_.SetBufferSilence(false);
    HpaePcmBuffer *output = ReadAlgoOutput(&tailOutputData_, byteLen);
    if (output != &silenceData_) {
        AccountOutput(byteLen);
    }
    return output;
}

size_t HpaeSessionEffectNode::FeedInputFromBuffer(HpaePcmBuffer *buf)
{
    if (buf == nullptr || !buf->IsValid() || effectInstance_ == nullptr) {
        return 0;
    }

    size_t channels = audioConfig_.inputCfg.channels;
    size_t byteLen = 0;
    if (!GetPcmByteLen(buf->GetFrameLen(), channels, byteLen)) {
        return 0;
    }
    if (byteLen > 0) {
        int32_t ret = effectInstance_->FeedInput(
            reinterpret_cast<const uint8_t *>(buf->GetPcmDataBuffer()), byteLen);
        if (ret != 0) {
            AUDIO_WARNING_LOG("FeedInput failed: %{public}d", ret);
            stats_.processErrors++;
            return 0;
        }
        totalBytesIn_ += byteLen;
        size_t bytesPerFrame = 0;
        if (GetBytesPerFrame(audioConfig_.inputCfg.channels, bytesPerFrame)) {
            stats_.framesProcessed += static_cast<int64_t>(byteLen / bytesPerFrame);
        }
        return byteLen;
    }
    return 0;
}

int32_t HpaeSessionEffectNode::FeedPaddingInput(size_t byteLen)
{
    if (!effectInstance_ || byteLen == 0) {
        return 0;
    }

    std::vector<uint8_t> padding(byteLen, 0);
    int32_t ret = effectInstance_->FeedInput(padding.data(), byteLen);
    if (ret != 0) {
        AUDIO_WARNING_LOG("FeedPaddingInput failed: %{public}d", ret);
        stats_.processErrors++;
        return ret;
    }
    paddingBytesIn_ += byteLen;
    return 0;
}

void HpaeSessionEffectNode::AccountOutput(size_t byteLen)
{
    totalBytesOut_ += byteLen;
    size_t bytesPerFrame = 0;
    if (GetBytesPerFrame(audioConfig_.inputCfg.channels, bytesPerFrame)) {
        stats_.framesOutput += static_cast<int64_t>(byteLen / bytesPerFrame);
    }
}

HpaePcmBuffer *HpaeSessionEffectNode::ReadAlgoOutput(HpaePcmBuffer *inputBuf, size_t byteLen)
{
    int32_t ret = effectInstance_->ReadOutput(
        reinterpret_cast<uint8_t *>(inputBuf->GetPcmDataBuffer()), byteLen);
    if (ret > 0) {
        inputBuf->SetBufferValid(true);
        inputBuf->SetBufferSilence(false);
        return inputBuf;
    }
    // No output → return silence
    stats_.underrunCount++;
    return &silenceData_;
}

void HpaeSessionEffectNode::CloseSessionEffectCore()
{
    // Step 1: Join preheat thread if running
    StopPreheatThread();

    // Step 2: Calculate pipeline latency and trigger RewindBuffer callback
    uint64_t latencyUs = GetLatency();
    if (latencyUs > 0) {
        auto callback = GetNodeInfo().statusCallback.lock();
        if (callback) {
            callback->OnRewindBuffer(GetSessionId(), latencyUs);
            AUDIO_INFO_LOG("CloseSE: OnRewindBuffer sessionId=%{public}u latency=%{public}" PRIu64 "us",
                GetSessionId(), latencyUs);
        }
    }

    // Step 3: Flush algorithm caches (StopEffect is NOT called here intentionally;
    // it will be called by DropEffect or destructor which follows this function)
    if (effectInstance_) {
        effectInstance_->Flush();
    }

    // Step 4: Enter bypass mode
    status_ = STATUS::IDLE;
    isPaused_ = false;
    AUDIO_INFO_LOG("CloseSE: node bypass, sessionId=%{public}u", GetSessionId());
}

void HpaeSessionEffectNode::ApplyInterceptingFadeIn(float *algoOutput, size_t sampleCount)
{
    for (size_t i = 0; i < sampleCount; ++i) {
        algoOutput[i] *= static_cast<float>(i + 1) / static_cast<float>(sampleCount);
    }
}

void HpaeSessionEffectNode::ApplyCrossfadeMix(float *algoOutput, const float *rawInput, size_t sampleCount)
{
    for (size_t i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i + 1) / static_cast<float>(sampleCount);
        algoOutput[i] = algoOutput[i] * (1.0f - t) + rawInput[i] * t;
    }
}

void HpaeSessionEffectNode::PrepareMoveStream()
{
    CloseSessionEffectCore();
    AUDIO_INFO_LOG("PrepareMoveStream: sessionId=%{public}u ready for move", GetSessionId());
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
