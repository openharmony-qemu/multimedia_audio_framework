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

#ifndef AUDIO_EFFECT_VOLUME_H
#define AUDIO_EFFECT_VOLUME_H

#include <cstdint>
#include <map>
#include <mutex>

namespace OHOS {
namespace AudioStandard {
class AudioEffectVolume {
public:
    AudioEffectVolume();
    ~AudioEffectVolume();
    static std::shared_ptr<AudioEffectVolume> GetInstance();
    void SetDspVolume(const float volume);
    float GetDspVolume();
    void SetSystemVolume(const int32_t systemVolumeType, const float systemVolume);
    float GetSystemVolume(const int32_t systemVolumeType);
    void SetStreamVolume(const std::string sessionID, float systemVolume);
    float GetStreamVolume(const std::string sessionID);
    int32_t StreamVolumeDelete(const std::string sessionID);
private:
    float dspVolume_ = -1.0f;
    std::mutex volumeMutex_;
    std::map<std::string, float> SceneTypeToVolumeMap_;
    std::map<std::string, float> SessionIDToVolumeMap_;
    std::map<int32_t, float> SystemVolumeMap_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_ROTATION_H