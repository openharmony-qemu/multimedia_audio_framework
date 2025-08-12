/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_COMMON_UTILS_H
#define AUDIO_COMMON_UTILS_H

#include <unordered_map>
#include <set>

namespace OHOS {
namespace AudioStandard {

class VolumeUtils {
public:
    static AudioVolumeType GetVolumeTypeFromStreamType(AudioStreamType streamType);
    static void SetPCVolumeEnable(const bool& isPCVolumeEnable);
    static bool IsPCVolumeEnable();
    static AudioVolumeType GetVolumeTypeFromStreamUsage(StreamUsage streamUsage);
    static std::set<StreamUsage> GetOverlapStreamUsageSet(const std::set<StreamUsage>& streamUsages,
        AudioVolumeType volumeType);
    static std::vector<AudioVolumeType> GetSupportedAudioVolumeTypes();
    static std::vector<StreamUsage> GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType);
    static int32_t VolumeDegreeToLevel(int32_t degree);
    static int32_t VolumeLevelToDegree(int32_t level);
private:
    static std::set<StreamUsage>& GetStreamUsageSetForVolumeType(AudioVolumeType volumeType);

    static std::unordered_map<AudioStreamType, AudioVolumeType> defaultVolumeMap_;
    static std::unordered_map<AudioStreamType, AudioVolumeType> audioPCVolumeMap_;
    static std::unordered_map<AudioStreamType, AudioVolumeType>& GetVolumeMap();
    static bool isPCVolumeEnable_;
    static std::unordered_map<AudioVolumeType, std::set<StreamUsage>> defaultVolumeToStreamUsageMap_;
    static std::unordered_map<AudioVolumeType, std::set<StreamUsage>> pcVolumeToStreamUsageMap_;
    static std::unordered_map<StreamUsage, AudioStreamType> streamUsageMap_;
    static std::unordered_set<AudioVolumeType> audioVolumeTypeSet_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_COMMON_UTILS_H
