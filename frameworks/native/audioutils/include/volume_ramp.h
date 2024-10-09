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
#ifndef VOLUME_RAMP_H
#define VOLUME_RAMP_H

#include <cstdint>
#include <string>
#include <map>
#include <mutex>
#include <cmath>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

enum RampDirection {
    RAMP_UP = 0,
    RAMP_DOWN = 1,
};

class VolumeRamp {
public:
    VolumeRamp();
    ~VolumeRamp() = default;
    void SetVolumeRampConfig(float targetVolume, float currStreamVolume, int32_t duration);
    float GetRampVolume();
    bool IsActive();
    void Terminate();

private:
    void SetVolumeCurve(std::vector<float> &volumes);
    float GetScaledTime(int64_t currentTime);
    float FindRampVolume(float time);

    int32_t duration_ = 0;
    RampDirection rampDirection_ = RAMP_UP;
    std::mutex curveMapMutex_;
    std::map<float, float> curvePoints_;
    int64_t initTime_ = 0;
    float scale_ = 0.0f;
    bool isVolumeRampActive_ = false;
    float rampVolume_ = 0.0f;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // VOLUME_RAMP_H
