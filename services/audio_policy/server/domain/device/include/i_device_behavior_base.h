/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef I_DEVICE_BEHAVIOR_BASE_H
#define I_DEVICE_BEHAVIOR_BASE_H

#include "audio_device_descriptor.h"
#include "audio_device_info.h"
#include "audio_stream_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class IDeviceBehaviorBase {
public:
    IDeviceBehaviorBase() {}

    virtual ~IDeviceBehaviorBase() = default;

    virtual int32_t ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason) = 0;

    virtual int32_t DeactivateDevice() = 0;

    virtual int32_t DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc) = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif