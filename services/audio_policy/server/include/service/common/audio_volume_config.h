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

#ifndef ST_AUDIO_VOLUME_CONFIG_H
#define ST_AUDIO_VOLUME_CONFIG_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

struct VolumePoint {
    uint32_t index;
    int32_t dbValue;
};

struct DeviceVolumeInfo {
    DeviceVolumeType deviceType;
    std::vector<VolumePoint> volumePoints;
};

typedef std::map<DeviceVolumeType, std::shared_ptr<DeviceVolumeInfo>> DeviceVolumeInfoMap;

struct StreamVolumeInfo {
    AudioVolumeType streamType;
    int minLevel;
    int maxLevel;
    int defaultLevel;
    DeviceVolumeInfoMap deviceVolumeInfos;
};

typedef std::map<AudioVolumeType, std::shared_ptr<StreamVolumeInfo>> StreamVolumeInfoMap;
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_VOLUME_CONFIG_H
