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
#ifndef HIGH_RRSOLUTION_TIMER_H
#define HIGH_RRSOLUTION_TIMER_H
#include <iostream>
#include <chrono>
#include <thread>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
typedef std::chrono::high_resolution_clock::time_point TimePoint;
class HighResolutionTimer {
public:
    HighResolutionTimer() : startTime_(), endTime_()
    {}

    void Start()
    {
        startTime_ = std::chrono::high_resolution_clock::now();
    }

    void Stop()
    {
        endTime_ = std::chrono::high_resolution_clock::now();
    }

    template <typename DurationType = std::chrono::milliseconds>
    auto Elapsed() const
    {
        return std::chrono::duration_cast<DurationType>(endTime_ - startTime_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point endTime_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // __HIGH_RRSOLUTION_TIMER_H__