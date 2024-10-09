/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "VolumeRamp"
#endif

#include "volume_ramp.h"
#include <cinttypes>
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
constexpr float MIN_CURVE_TIME = 0.0f;
constexpr float MAX_CURVE_TIME = 1.0f;
constexpr int32_t MS_PER_S = 1000;
constexpr int32_t NS_PER_MS = 1000000;
constexpr unsigned long VOLUME_SIZE = 2;

VolumeRamp::VolumeRamp()
{
    initTime_ = -1;
    isVolumeRampActive_ = false;
    rampVolume_ = 1.0f;
}

void VolumeRamp::SetVolumeCurve(vector<float> &volumes)
{
    vector<float> times = {0.0f, 1.0f};
    CHECK_AND_RETURN_LOG(volumes.size() == VOLUME_SIZE, "Array size must 2!");

    std::lock_guard<std::mutex> lock(curveMapMutex_);
    curvePoints_.clear();
    for (size_t i = 0; i < times.size(); i++) {
        curvePoints_.emplace(times[i], volumes[i]);
    }
}

void VolumeRamp::SetVolumeRampConfig(float targetVolume, float currStreamVolume, int32_t duration)
{
    vector<float> volumes;

    isVolumeRampActive_ = true;
    initTime_ = -1;
    duration_ = duration;

    if (currStreamVolume > targetVolume) {
        volumes.assign({targetVolume, currStreamVolume});
        rampDirection_ = RAMP_DOWN;
    } else {
        volumes.assign({currStreamVolume, targetVolume});
        rampDirection_ = RAMP_UP;
    }
    SetVolumeCurve(volumes);
}

static int64_t GetCurrentTimeMS()
{
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

float VolumeRamp::GetRampVolume()
{
    if (!isVolumeRampActive_) {
        return rampVolume_;
    }

    int64_t currentTime = GetCurrentTimeMS();

    if (initTime_ < 0) {
        initTime_ = currentTime;
        scale_ = 1.0 / duration_;
    }

    float scaledTime = GetScaledTime(currentTime);
    rampVolume_ = FindRampVolume(scaledTime);
    return rampVolume_;
}

float VolumeRamp::GetScaledTime(int64_t currentTime)
{
    float offset = scale_ * (currentTime - initTime_);
    if (rampDirection_ == RAMP_DOWN) {
        offset =  1 - offset;
        if (offset < MIN_CURVE_TIME) {
            offset = MIN_CURVE_TIME;
            isVolumeRampActive_ = false;
        } else if (offset > MAX_CURVE_TIME) {
            offset = MAX_CURVE_TIME;
        }
    } else {
        if (offset < MIN_CURVE_TIME) {
            offset = MIN_CURVE_TIME;
        } else if (offset > MAX_CURVE_TIME) {
            offset = MAX_CURVE_TIME;
            isVolumeRampActive_ = false;
        }
    }

    return offset;
}

float VolumeRamp::FindRampVolume(float time)
{
    std::lock_guard<std::mutex> lock(curveMapMutex_);
    CHECK_AND_RETURN_RET_LOG(!curvePoints_.empty(), 0.0f, "Curve points map is empty");

    auto lowPoint = curvePoints_.begin();
    auto highPoint = curvePoints_.rbegin();

    float volume = lowPoint->second + ((time - lowPoint->first) / (highPoint->first - lowPoint->first))
        * (highPoint->second - lowPoint->second);
    return volume;
}

bool VolumeRamp::IsActive()
{
    return isVolumeRampActive_;
}

void VolumeRamp::Terminate()
{
    isVolumeRampActive_ = false;
}
} // namespace AudioStandard
} // namespace OHOS
