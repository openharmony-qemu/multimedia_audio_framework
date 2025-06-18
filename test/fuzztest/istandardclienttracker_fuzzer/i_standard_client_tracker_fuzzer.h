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

#ifndef I_STANDARD_CLIENT_TRACKER_FUZZER_H
#define I_STANDARD_CLIENT_TRACKER_FUZZER_H

#include <iostream>
#include <random>
#include <vector>
#include "i_standard_client_tracker.h"

namespace OHOS {
namespace AudioStandard {
class IStandardClientTrackerFuzz : public IStandardClientTracker {
public:
    IStandardClientTrackerFuzz()
        : randomEngine_(std::random_device{}())
    {
        callCounts_.resize(UNSETOFFLOADMODE + 1, 0);
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void MuteStreamImpl(const StreamSetStateEventInternal &event) override
    {
        LogCall(AudioClientTrackerMsg::MUTESTREAM);
        InjectRandomBehavior();
    }

    void UnmuteStreamImpl(const StreamSetStateEventInternal &event) override
    {
        LogCall(AudioClientTrackerMsg::UNMUTESTREAM);
        InjectRandomBehavior();
    }

    void PausedStreamImpl(const StreamSetStateEventInternal &event) override
    {
        LogCall(AudioClientTrackerMsg::PAUSEDSTREAM);
        InjectRandomBehavior();
    }

    void ResumeStreamImpl(const StreamSetStateEventInternal &event) override
    {
        LogCall(AudioClientTrackerMsg::RESUMESTREAM);
        InjectRandomBehavior();
    }

    void SetLowPowerVolumeImpl(float volume) override
    {
        LogCall(AudioClientTrackerMsg::SETLOWPOWERVOL);
        InjectRandomBehavior();
    }

    void GetLowPowerVolumeImpl(float& volume) override
    {
        LogCall(AudioClientTrackerMsg::GETLOWPOWERVOL);
        InjectRandomBehavior();
        volume = GenerateRandomFloat(0.0f, 1.0f);
    }

    void SetOffloadModeImpl(int32_t state, bool isAppBack) override
    {
        LogCall(AudioClientTrackerMsg::SETOFFLOADMODE);
        InjectRandomBehavior();
    }

    void UnsetOffloadModeImpl() override
    {
        LogCall(AudioClientTrackerMsg::UNSETOFFLOADMODE);
        InjectRandomBehavior();
    }

    void GetSingleStreamVolumeImpl(float& volume) override
    {
        LogCall(AudioClientTrackerMsg::GETSINGLESTREAMVOL);
        InjectRandomBehavior();
        volume = GenerateRandomFloat(0.0f, 1.0f);
    }

    void ResetCallCounts()
    {
        std::fill(callCounts_.begin(), callCounts_.end(), 0);
    }

    int GetCallCount(AudioClientTrackerMsg msg) const
    {
        if (msg >= ON_ERROR && msg <= UNSETOFFLOADMODE) {
            return callCounts_[static_cast<int>(msg)];
        }
        return -1;
    }

    enum FuzzMode {
        MODE_NORMAL,
        MODE_DELAY,
        MODE_FAILURE,
        MODE_CORRUPT
    };

    void SetFuzzMode(FuzzMode mode)
    {
        currentMode_ = mode;
    }

    void EnableExceptionInjection(bool enable)
    {
        injectExceptions_ = enable;
    }

    void SetMaxDelayMs(int maxDelay)
    {
        maxDelayMs_ = maxDelay;
    }

private:
    void LogCall(AudioClientTrackerMsg msg)
    {
        if (msg >= ON_ERROR && msg <= UNSETOFFLOADMODE) {
            callCounts_[static_cast<int>(msg)]++;
        }
    }

    void InjectRandomBehavior()
    {
        switch (currentMode_) {
            case MODE_DELAY:
                InjectRandomDelay();
                break;
            case MODE_FAILURE:
                InjectRandomFailure();
                break;
            case MODE_CORRUPT:
                break;
            case MODE_NORMAL:
            default:
                break;
        }
    }

    void InjectRandomDelay()
    {
        std::uniform_int_distribution<int> dist(0, maxDelayMs_);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(dist(randomEngine_))
        );
    }

    void InjectRandomFailure()
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(randomEngine_) < 0.3f) {
            return;
        }
    }

    float GenerateRandomFloat(float min, float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine_);
    }

    std::default_random_engine randomEngine_;
    std::vector<int> callCounts_;
    FuzzMode currentMode_ = MODE_NORMAL;
    bool injectExceptions_ = false;
    int maxDelayMs_ = 1000;
};

class AudioFuzzException : public std::runtime_error {
public:
    explicit AudioFuzzException(const std::string& what)
        : std::runtime_error("AudioFuzzException: " + what) {}
};
} // namespace AudioStandard
} // namesapce OHOS

#endif // I_STANDARD_CLIENT_TRACKER_FUZZER_H