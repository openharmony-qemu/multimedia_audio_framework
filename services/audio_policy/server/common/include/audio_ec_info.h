/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_EC_INFO_H
#define ST_AUDIO_EC_INFO_H

#include <string>

#include "audio_device_descriptor.h"
#include "audio_device_info.h"
#include "audio_adapter_info.h"

namespace OHOS {
namespace AudioStandard {

enum EcType {
    EC_TYPE_NONE,
    EC_TYPE_SAME_ADAPTER,
    EC_TYPE_DIFF_ADAPTER
};

static const std::unordered_set<DeviceType> ecSupportedOutputDevices = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
};

static const std::unordered_set<std::string> ecSupportedInputAdapters = {
    ADAPTER_PRIMARY_TYPE,
    ADAPTER_USB_TYPE,
    ADAPTER_DP_TYPE,
};

struct AudioEcInfo {
    AudioEcInfo()
    {
        inputDevice.deviceType_ = DEVICE_TYPE_DEFAULT;
        outputDevice.deviceType_ = DEVICE_TYPE_DEFAULT;
    }
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    EcType ecType = EC_TYPE_NONE;
    std::string ecInputAdapter = "";
    std::string ecOutputAdapter = "";
    std::string samplingRate = "";
    std::string format = "";
    std::string channels = "";
    PipeInfo pipeInfo;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_EC_INFO_H
