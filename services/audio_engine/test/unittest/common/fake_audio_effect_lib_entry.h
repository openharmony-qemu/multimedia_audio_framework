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

#ifndef FAKE_AUDIO_EFFECT_LIB_ENTRY_H
#define FAKE_AUDIO_EFFECT_LIB_ENTRY_H

#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

// Realistic fake parameters
// 1 DoProcess frame = 960 samples @ 48kHz = 20ms
// Process threshold: 2s = 100 DoProcess frames = 96000 stereo samples = 192000 floats
static constexpr size_t fakeProcessThresholdFloats = 96000 * 2; // 2s of stereo data
// Preheat: 3s = 150 DoProcess frames
static constexpr uint32_t fakePreheatFrames = 150;
// Process latency: 800ms
static constexpr uint32_t fakeProcessDelayMs = 800;

/**
 * Fake implementation of AudioEffectLibrary + AudioEffectInterface for testing.
 *
 * Uses a static instance pointer so static C callbacks can reach C++ state.
 * Safe in test context (one Fake per test).
 *
 * Realistic behavior:
 * - feedInput accumulates data, triggers background process when threshold (2s) reached
 * - Process: sleep 800ms, swap left/right channels
 * - getOutput: returns processed data when available
 * - preheatFrames = 150 (= 3s)
 */
class FakeAudioEffectLibEntry {
public:
    FakeAudioEffectLibEntry()
    {
        // Fail loudly if two instances exist simultaneously (static pointer conflict)
        if (instance_ != nullptr) {
            return;
        }
        instance_ = this;
    }

    ~FakeAudioEffectLibEntry()
    {
        // Wait for any in-flight processing thread
        {
            std::lock_guard<std::mutex> lock(processMutex_);
            if (processThread_.joinable()) {
                processThread_.join();
            }
        }
        instance_ = nullptr;
    }

    // Non-copyable
    FakeAudioEffectLibEntry(const FakeAudioEffectLibEntry&) = delete;
    FakeAudioEffectLibEntry& operator=(const FakeAudioEffectLibEntry&) = delete;

    // --- Configuration ---

    void SetCreateFail(bool fail)
    {
        createFail_ = fail;
    }

    void SetProcessFail(bool fail)
    {
        processFail_ = fail;
    }

    void SetProcessDelayMs(uint32_t ms)
    {
        processDelayMs_ = ms;
    }

    void SetFlushFail(bool fail)
    {
        flushFail_ = fail;
    }

    void SetPreheatFrames(uint32_t frames)
    {
        preheatFrames_ = frames;
    }

    void SetChannels(uint32_t channels)
    {
        channels_ = channels;
    }

    void SetProcessThreshold(size_t floats)
    {
        processThreshold_ = floats;
    }

    // Block until any in-flight processing thread completes
    void WaitForProcessing()
    {
        std::lock_guard<std::mutex> lock(processMutex_);
        if (processThread_.joinable()) {
            processThread_.join();
        }
    }

    void SetFeedInputFail(bool fail) { feedInputFail_ = fail; }
    void SetGetOutputFail(bool fail) { getOutputFail_ = fail; }
    // When true, getOutput copies data but does not consume the output buffer.
    // Use in tests where SkipStaleFrames calls getOutput before the real read.
    void SetOutputPersistent(bool persistent) { outputPersistent_ = persistent; }
    uint32_t GetFeedInputCount() const { return feedInputCount_; }
    uint32_t GetGetOutputCount() const { return getOutputCount_; }
    uint32_t GetTriggerProcessCount() const { return triggerProcessCount_; }
    size_t GetProcessedOutputSize() const
    {
        std::lock_guard<std::mutex> lock(outputMutex_);
        return processedOutput_.size();
    }
    size_t GetAccumulatedInputSize() const { return accumulatedInput_.size(); }

    // --- Query ---

    struct CallEntry {
        uint32_t cmdCode;
    };

    const std::vector<CallEntry>& GetCallLog() const
    {
        return callLog_;
    }

    const std::vector<CallEntry>& GetParamLog() const
    {
        return paramLog_;
    }

    uint32_t GetCreateCount() const
    {
        return createCount_;
    }

    uint32_t GetReleaseCount() const
    {
        return releaseCount_;
    }

    uint32_t GetProcessCount() const
    {
        return processCount_;
    }

    uint32_t GetFlushCount() const
    {
        return flushCount_;
    }

    // Reset all state
    void Reset()
    {
        createFail_ = false;
        processFail_ = false;
        processDelayMs_ = fakeProcessDelayMs;
        flushFail_ = false;
        preheatFrames_ = fakePreheatFrames;
        processThreshold_ = fakeProcessThresholdFloats;
        createCount_ = 0;
        releaseCount_ = 0;
        processCount_ = 0;
        flushCount_ = 0;
        callLog_.clear();
        paramLog_.clear();
        feedInputFail_ = false;
        getOutputFail_ = false;
        outputPersistent_ = false;
        feedInputCount_ = 0;
        getOutputCount_ = 0;
        triggerProcessCount_ = 0;
        accumulatedInput_.clear();
        processedOutput_.clear();
        {
            std::lock_guard<std::mutex> lock(processMutex_);
            if (processThread_.joinable()) {
                processThread_.join();
            }
        }
    }

    // Get the AudioEffectLibrary struct with fake function pointers
    AudioEffectLibrary GetLibrary() const
    {
        return AudioEffectLibrary{
            .version = 1,
            .name = "FakeEffectLib",
            .implementor = "Test",
            .checkEffect = FakeCheckEffect,
            .createEffect = FakeCreateEffect,
            .releaseEffect = FakeReleaseEffect,
            .supportEffect = FakeSupportEffect,
        };
    }

private:
    // --- Static C callbacks ---

    static bool FakeCheckEffect(const AudioEffectDescriptor descriptor)
    {
        (void)descriptor;
        return true;
    }

    static int32_t FakeCreateEffect(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
    {
        (void)descriptor;
        if (!instance_) {
            return -1;
        }
        if (instance_->createFail_) {
            *handle = nullptr;
            return -1;
        }
        auto *iface = new AudioEffectInterface;
        iface->process = FakeProcess;
        iface->command = FakeCommand;
        iface->feedInput = FakeFeedInput;
        iface->getOutput = FakeGetOutput;
        // AudioEffectHandle = AudioEffectInterface**, so allocate the pointer-to-pointer
        auto **handlePtr = new AudioEffectInterface *(iface);
        *handle = handlePtr;
        instance_->createCount_++;
        return 0;
    }

    static int32_t FakeReleaseEffect(AudioEffectHandle handle)
    {
        if (!instance_ || !handle) {
            return -1;
        }
        // handle is AudioEffectInterface**, *handle is AudioEffectInterface*
        auto *iface = *handle;
        delete iface;
        delete handle;
        instance_->releaseCount_++;
        return 0;
    }

    static void FakeSupportEffect(AlgoSupportConfig *config)
    {
        (void)config;
    }

    static int32_t FakeProcess(AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer)
    {
        if (!instance_) {
            return -1;
        }
        instance_->processCount_++;
        if (instance_->processFail_) {
            return -1;
        }
        // Passthrough: copy f32 data (frameLength = number of float elements)
        if (inBuffer != nullptr && outBuffer != nullptr && inBuffer->f32 != nullptr && outBuffer->f32 != nullptr) {
            size_t frames = inBuffer->frameLength;
            (void)memcpy_s(outBuffer->f32, frames * sizeof(float), inBuffer->f32, frames * sizeof(float));
            outBuffer->frameLength = frames;
        }
        return 0;
    }

    static int32_t FakeCommand(AudioEffectHandle self, uint32_t cmdCode,
        AudioEffectTransInfo *cmdInfo, AudioEffectTransInfo *replyInfo)
    {
        if (!instance_) {
            return -1;
        }
        instance_->callLog_.push_back({cmdCode});

        switch (cmdCode) {
            case EFFECT_CMD_GET_PARAM:
                instance_->paramLog_.push_back({cmdCode});
                if (replyInfo != nullptr && replyInfo->data != nullptr) {
                    uint32_t frames = instance_->preheatFrames_;
                    (void)memcpy_s(replyInfo->data, sizeof(uint32_t), &frames, sizeof(uint32_t));
                    replyInfo->size = sizeof(uint32_t);
                }
                return 0;
            case EFFECT_CMD_FLUSH:
                instance_->flushCount_++;
                if (instance_->flushFail_) {
                    return -1;
                }
                return 0;
            default:
                return 0;
        }
    }

    static int32_t FakeFeedInput(AudioEffectHandle self, AudioBuffer *inBuffer)
    {
        if (!instance_) { return -1; }
        instance_->feedInputCount_++;
        if (instance_->feedInputFail_) { return -1; }
        if (inBuffer != nullptr && inBuffer->f32 != nullptr && inBuffer->frameLength > 0) {
            size_t floats = inBuffer->frameLength;
            instance_->accumulatedInput_.insert(
                instance_->accumulatedInput_.end(), inBuffer->f32, inBuffer->f32 + floats);

            if (instance_->accumulatedInput_.size() >= instance_->processThreshold_) {
                instance_->TriggerProcess();
            }
        }
        return 0;
    }

    static int32_t FakeGetOutput(AudioEffectHandle self, AudioBuffer *outBuffer)
    {
        if (!instance_) { return -1; }
        instance_->getOutputCount_++;
        if (instance_->getOutputFail_) { return -1; }
        if (outBuffer == nullptr || outBuffer->f32 == nullptr) { return -1; }

        std::lock_guard<std::mutex> lock(instance_->outputMutex_);
        if (instance_->processedOutput_.empty()) {
            outBuffer->frameLength = 0;
            return 0;
        }
        size_t toCopy = std::min(static_cast<size_t>(outBuffer->frameLength),
            instance_->processedOutput_.size());
        (void)memcpy_s(outBuffer->f32, toCopy * sizeof(float),
            instance_->processedOutput_.data(), toCopy * sizeof(float));
        if (!instance_->outputPersistent_) {
            instance_->processedOutput_.erase(
                instance_->processedOutput_.begin(), instance_->processedOutput_.begin() + toCopy);
        }
        outBuffer->frameLength = toCopy;
        return 0;
    }

    // --- Internal processing ---

    void TriggerProcess()
    {
        triggerProcessCount_++;
        std::lock_guard<std::mutex> lock(processMutex_);
        if (processThread_.joinable()) {
            processThread_.join();
        }

        size_t processLen = processThreshold_;
        std::vector<float> inputChunk(accumulatedInput_.begin(),
                                       accumulatedInput_.begin() + processLen);
        accumulatedInput_.erase(accumulatedInput_.begin(),
                                accumulatedInput_.begin() + processLen);

        processThread_ = std::thread([this, data = std::move(inputChunk)]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(processDelayMs_));
            // Swap left/right channels (interleaved stereo)
            std::vector<float> result = data;
            uint32_t ch = channels_ > 0 ? channels_ : 2;
            for (size_t i = 0; i + ch - 1 < result.size(); i += ch) {
                std::swap(result[i], result[i + 1]); // swap L and R
            }
            std::lock_guard<std::mutex> outLock(outputMutex_);
            processedOutput_.insert(processedOutput_.end(), result.begin(), result.end());
        });
    }

    // --- State ---

    static FakeAudioEffectLibEntry *instance_;

    bool createFail_ = false;
    bool processFail_ = false;
    uint32_t processDelayMs_ = fakeProcessDelayMs;
    bool flushFail_ = false;
    uint32_t preheatFrames_ = fakePreheatFrames;
    size_t processThreshold_ = fakeProcessThresholdFloats;
    uint32_t channels_ = 2;

    bool feedInputFail_ = false;
    bool getOutputFail_ = false;
    bool outputPersistent_ = false;
    uint32_t feedInputCount_ = 0;
    uint32_t getOutputCount_ = 0;
    uint32_t triggerProcessCount_ = 0;
    std::vector<float> accumulatedInput_;
    std::vector<float> processedOutput_;
    std::mutex processMutex_;
    mutable std::mutex outputMutex_;
    std::thread processThread_;

    uint32_t createCount_ = 0;
    uint32_t releaseCount_ = 0;
    uint32_t processCount_ = 0;
    uint32_t flushCount_ = 0;

    std::vector<CallEntry> callLog_;
    std::vector<CallEntry> paramLog_;
};

FakeAudioEffectLibEntry *FakeAudioEffectLibEntry::instance_ = nullptr;

} // namespace AudioStandard
} // namespace OHOS

#endif // FAKE_AUDIO_EFFECT_LIB_ENTRY_H
