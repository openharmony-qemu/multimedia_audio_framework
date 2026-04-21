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

#ifndef AUDIO_DEVICE_ENHANCE_MANAGER_H
#define AUDIO_DEVICE_ENHANCE_MANAGER_H

#include <memory>
#include <vector>

#include "audio_device_descriptor.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class AudioCapturer;
class AudioRenderer;

struct SoundCardInfo {
    std::string name;
    std::string vendor;
    std::string model;
    std::string busAddress;
    std::string driver;

    SoundCardInfo() : name(""), vendor(""), model(""), busAddress(""), driver("") {}

    bool IsEmpty() const
    {
        return name.empty() && vendor.empty() && model.empty() &&
            busAddress.empty() && driver.empty();
    }
};
class AudioDeviceEnhanceManager {
public:
    static AudioDeviceEnhanceManager &GetInstance();

    int32_t IsEnhancedRoutingSupported(bool &supported);
    int32_t SelectOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    int32_t SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    int32_t SelectInputDeviceForAudioCapturer(std::shared_ptr<AudioCapturer> &capturer,
        const std::shared_ptr<AudioDeviceDescriptor> &desc) const;
    int32_t SelectOutputDeviceForAudioRenderer(std::shared_ptr<AudioRenderer> &renderer,
        const std::shared_ptr<AudioDeviceDescriptor> &desc) const;
    SoundCardInfo GetSoundCardInfo() const;
private:
    AudioDeviceEnhanceManager() = default;
    ~AudioDeviceEnhanceManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEVICE_ENHANCE_MANAGER_H
