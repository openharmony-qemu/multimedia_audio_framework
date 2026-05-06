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
#ifndef AUDIO_VOLUME_INFO_H
#define AUDIO_VOLUME_INFO_H

#ifdef __MUSL__
#include <stdint.h>
#endif // __MUSL__

namespace OHOS {
namespace AudioStandard {

/**
 * @brief Device group type used for volume.
 */
enum class DeviceGroupType : int32_t {
    /** Invalid device group type */
    INVALID = -1,
    /** Default device group type*/
    DEFAULT = 0,
    /* Earpiece device*/
    EARPIECE = 1,
    /** Built in device. (Speaker) */
    BUILT_IN = 2,
    /** Wired devices.*/
    WIRED = 3,
    /** Wireless devices. (Excluding absolute volume devices)*/
    WIRELESS = 4,
    /** Dp and HDMI devices */
    DP = 5,
    /** Remote cast devices */
    REMOTE_CAST = 6,
    /** Hearing aid device */
    HEARING_AID = 7,
    /** Line digital device */
    LINE_DIGITAL = 8,
    /** The maximum valid value */
    MAX = LINE_DIGITAL,
};

enum class VolumeKeyType : int32_t {
    LEVEL = 1,
    DEGREE = 2,
    MUTE = 3,
};

struct VolumeScale {
    VolumeScale(int32_t volumeLevel) : volumeLevel_(volumeLevel) {}
    VolumeScale(int32_t volumeLevel, int32_t volumeDegree, int32_t maxVolumeLevel = -1)
        : volumeLevel_(volumeLevel), volumeDegree_(volumeDegree), maxVolumeLevel_(maxVolumeLevel)
    {
        valueType_ = volumeDegree_ != -1 ?
            VolumeKeyType::DEGREE : VolumeKeyType::LEVEL;
        if (maxVolumeLevel != -1) {
            Refresh();
        }
    }

    VolumeScale Refresh();
    VolumeScale Update(const VolumeScale &volume);
    VolumeScale Adjust(const VolumeScale &volume);
    bool IsVolumeDegreeValid(int32_t min, int32_t max);
    void SetMaxVolumeLevel(int32_t maxVolumeLevel) { maxVolumeLevel_ = maxVolumeLevel; }
    int32_t VolumeLevel() const { return volumeLevel_; }
    int32_t VolumeDegree() const { return volumeDegree_; }
    int32_t MaxVolumeLevel() const { return maxVolumeLevel_; }

private:
    int32_t volumeLevel_ = -1;

    /**
     * if not initialized, volumeDegree_ can be calculated by
     * volumeLevel_ and maxVolumeLevel_ if they are valid;
     */
    int32_t volumeDegree_ = -1;
    int32_t maxVolumeLevel_ = -1;
    VolumeKeyType valueType_ = VolumeKeyType::LEVEL; // mark the struct used as level or degree;
};

struct VolumeValue {
    VolumeValue() {}
    VolumeValue(int32_t level, int32_t degree, bool mute)
        : volumeLevel_(level), volumeDegree_(degree), mute_(mute)
    {
    }

    int32_t volumeLevel_ = -1;
    int32_t volumeDegree_ = -1;
    bool mute_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_VOLUME_INFO_H
