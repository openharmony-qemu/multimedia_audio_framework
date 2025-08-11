/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioManagerUtil"
#endif

#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
const std::unordered_map<DeviceType, std::string> &GetSupportedDeviceType()
{
    static const std::unordered_map<DeviceType, std::string> supportedDevicetype = {
        {DEVICE_TYPE_NONE, "DEVICE_TYPE_NONE"},
        {DEVICE_TYPE_INVALID, "DEVICE_TYPE_INVALID"},
        {DEVICE_TYPE_EARPIECE, "DEVICE_TYPE_EARPIECE"},
        {DEVICE_TYPE_SPEAKER, "DEVICE_TYPE_SPEAKER"},
        {DEVICE_TYPE_WIRED_HEADSET, "DEVICE_TYPE_WIRED_HEADSET"},
        {DEVICE_TYPE_WIRED_HEADPHONES, "DEVICE_TYPE_WIRED_HEADPHONES"},
        {DEVICE_TYPE_BLUETOOTH_SCO, "DEVICE_TYPE_BLUETOOTH_SCO"},
        {DEVICE_TYPE_BLUETOOTH_A2DP, "DEVICE_TYPE_BLUETOOTH_A2DP"},
        {DEVICE_TYPE_BLUETOOTH_A2DP_IN, "DEVICE_TYPE_BLUETOOTH_A2DP_IN"},
        {DEVICE_TYPE_MIC, "DEVICE_TYPE_MIC"},
        {DEVICE_TYPE_WAKEUP, "DEVICE_TYPE_WAKEUP"},
        {DEVICE_TYPE_USB_HEADSET, "DEVICE_TYPE_USB_HEADSET"},
        {DEVICE_TYPE_USB_ARM_HEADSET, "DEVICE_TYPE_USB_ARM_HEADSET"},
        {DEVICE_TYPE_DP, "DEVICE_TYPE_DP"},
        {DEVICE_TYPE_HDMI, "DEVICE_TYPE_HDMI"},
        {DEVICE_TYPE_FILE_SINK, "DEVICE_TYPE_FILE_SINK"},
        {DEVICE_TYPE_FILE_SOURCE, "DEVICE_TYPE_FILE_SOURCE"},
        {DEVICE_TYPE_EXTERN_CABLE, "DEVICE_TYPE_EXTERN_CABLE"},
        {DEVICE_TYPE_DEFAULT, "DEVICE_TYPE_DEFAULT"},
        {DEVICE_TYPE_ACCESSORY, "DEVICE_TYPE_ACCESSORY"},
    };
    return supportedDevicetype;
}
} // namespace AudioStandard
} // namespace OHOS