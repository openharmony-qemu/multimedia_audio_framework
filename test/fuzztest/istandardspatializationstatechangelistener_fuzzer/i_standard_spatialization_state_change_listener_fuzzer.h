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

#ifndef I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_FUZZER_H
#define I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_FUZZER_H

#include <iostream>
#include <random>
#include <vector>
#include "i_standard_client_tracker.h"

namespace OHOS {
namespace AudioStandard {
class SpatializationListenerFuzz : public IStandardSpatializationStateChangeListener {
public:
    SpatializationListenerFuzz() {}

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnSpatializationStateChange(const AudioSpatializationState &state) override
    {
        callCount_++;
        if (simulateProcessing_) {
            ProcessStateChange(state);
        }
        InjectRandomBehavior();
    }

    void ResetCallCount()
    {
        callCount_ = 0;
    }

    int GetCallCount() const
    {
        return callCount_;
    }

    void SetResponseDelayRange(int minMs, int maxMs)
    {
        minDelayMs_ = minMs;
        maxDelayMs_ = maxMs;
    }

    void EnableProcessingSimulation(bool enable)
    {
        simulateProcessing_ = enable;
    }

    ~SpatializationListenerFuzz() override
    {
        DumpStatistics();
    }

private:
    void ProcessStateChange(const AudioSpatializationState &state)
    {
        return;
    }

    void InjectRandomBehavior()
    {
        switch (behaviorMode_) {
            case BehaviorMode::DELAY:
                InjectRandomDelay();
                break;
            case BehaviorMode::FAILURE:
                InjectRandomFailure();
                break;
            case BehaviorMode::INVALID_RESPONSE:
                break;
            case BehaviorMode::NORMAL:
            default:
                break;
        }
    }

    void InjectRandomDelay()
    {
        std::uniform_int_distribution<int> dist(minDelayMs_, maxDelayMs_);
        int delay = dist(randomEngine_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    void InjectRandomFailure()
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(randomEngine_) < failureProbability_) {
            throw std::runtime_error("Random failure injected in spatialization listener");
        }
    }

    void DumpStatistics() {}

    enum class BehaviorMode {
        NORMAL,
        DELAY,
        FAILURE,
        INVALID_RESPONSE
    };

    std::default_random_engine randomEngine_;
    int callCount_ = 0;
    BehaviorMode behaviorMode_ = BehaviorMode::NORMAL;
    bool simulateProcessing_ = false;
    float failureProbability_ = 0.3f;
    int minDelayMs_ = 0;
    int maxDelayMs_ = 1000;
};
} // namespace AudioStandard
} // namesapce OHOS

#endif // I_STANDARD_SPATIALIZATION_STATE_CHANGE_LISTENER_FUZZER_H