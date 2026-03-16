/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
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
#ifndef AUDIO_DEVICE_FACTORY_H
#define AUDIO_DEVICE_FACTORY_H

#include "audio_device_info.h"
#include "audio_stream_descriptor.h"
#include "i_device_behavior_base.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceFactory {
public:
    static AudioDeviceFactory& GetInstance()
    {
        static AudioDeviceFactory instance;
        return instance;
    }

    int32_t ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t DeactivateDevice(DeviceType deviceType);
    int32_t DeactivateDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);

private:
    AudioDeviceFactory() = default;
    ~AudioDeviceFactory() = default;

    AudioDeviceFactory(const AudioDeviceFactory &) = delete;
    AudioDeviceFactory &operator=(const AudioDeviceFactory &) = delete;
    AudioDeviceFactory(AudioDeviceFactory &&) = delete;
    AudioDeviceFactory &operator=(AudioDeviceFactory &&) = delete;

    void HandleDeviceConflictLocked(DeviceType deviceType);
    int32_t DeactivateDeviceLocked(DeviceType deviceType);

    std::shared_ptr<IDeviceBehaviorBase> CreateDeviceBehavior(DeviceType deviceType);

    std::unordered_map<DeviceType, std::shared_ptr<IDeviceBehaviorBase>> deviceBehaviorCache_;
    std::unordered_map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> activatedDevices_;
    std::mutex cacheMutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif