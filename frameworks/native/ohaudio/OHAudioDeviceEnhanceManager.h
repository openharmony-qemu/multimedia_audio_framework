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

#ifndef OH_AUDIO_DEVICE_ENHANCE_MANAGER_H
#define OH_AUDIO_DEVICE_ENHANCE_MANAGER_H

#include <vector>
#include <memory>

#include "native_audio_device_enhance_manager.h"
#include "OHAudioDeviceDescriptor.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioDeviceEnhanceManager {
public:
    static OHAudioDeviceEnhanceManager &GetInstance();

    int32_t IsEnhancedRoutingSupported(bool &supported);
    int32_t SelectOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    int32_t SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);

private:
    OHAudioDeviceEnhanceManager() = default;
    ~OHAudioDeviceEnhanceManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // OH_AUDIO_DEVICE_ENHANCE_MANAGER_H
