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

#ifndef AUDIO_SLE_DEVICE_BEHAVIOR_H
#define AUDIO_SLE_DEVICE_BEHAVIOR_H

#include "i_device_behavior_base.h"

namespace OHOS {
namespace AudioStandard {
class AudioSleDeviceBehavior : public IDeviceBehaviorBase {
public:
    AudioSleDeviceBehavior() = default;
    virtual ~AudioSleDeviceBehavior() = default;

    int32_t ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason) override;
    int32_t DeactivateDevice() override;
    int32_t DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc) override;

private:
    void HandleNearlinkErrResult(int32_t result, std::shared_ptr<AudioDeviceDescriptor> devDesc, bool isVoiceType);
};
} // namespace AudioStandard
} // namespace OHOS

#endif