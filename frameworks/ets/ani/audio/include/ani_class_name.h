/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef ANI_CLASS_NAME_H
#define ANI_CLASS_NAME_H

#include <string>

namespace OHOS {
namespace AudioStandard {

static const std::string ANI_CLASS_SPACE = "L@ohos/multimedia/audio/audio/";

static const std::string ANI_CLASS_AUDIO_MANAGER_HANDLE = ANI_CLASS_SPACE + "AudioManagerHandle;";
static const std::string ANI_CLASS_AUDIO_VOLUME_MANAGER_HANDLE = ANI_CLASS_SPACE + "AudioVolumeManagerHandle;";
static const std::string ANI_CLASS_VOLUME_EVENT_HANDLE = ANI_CLASS_SPACE + "VolumeEventHandle;";
static const std::string ANI_CLASS_AUDIO_VOLUME_TYPE = ANI_CLASS_SPACE + "AudioVolumeType;";
static const std::string ANI_CLASS_INTERRUPT_HINT = ANI_CLASS_SPACE + "InterruptHint;";
static const std::string ANI_CLASS_INTERRUPT_TYPE = ANI_CLASS_SPACE + "InterruptType;";
static const std::string ANI_CLASS_DEVICE_ROLE = ANI_CLASS_SPACE + "DeviceRole;";
static const std::string ANI_CLASS_DEVICE_TYPE = ANI_CLASS_SPACE + "DeviceType;";
static const std::string ANI_CLASS_AUDIO_STREAM_DEVICE_CHANGE_REASON =
    ANI_CLASS_SPACE + "AudioStreamDeviceChangeReason;";
static const std::string ANI_CLASS_CONTENT_TYPE = ANI_CLASS_SPACE + "ContentType;";
static const std::string ANI_CLASS_STREAM_USAGE = ANI_CLASS_SPACE + "StreamUsage;";
static const std::string ANI_CLASS_INTERRUPT_MODE = ANI_CLASS_SPACE + "InterruptMode;";
static const std::string ANI_CLASS_AUDIO_RENDERER_INFO_HANDLE = ANI_CLASS_SPACE + "AudioRendererInfoHandle;";
static const std::string ANI_CLASS_INTERRUPT_EVENT_HANDLE = ANI_CLASS_SPACE + "InterruptEventHandle;";
static const std::string ANI_CLASS_INTERRUPT_FORCE_TYPE = ANI_CLASS_SPACE + "InterruptForceType;";
static const std::string ANI_CLASS_AUDIO_DEVICE_DESCRIPTOR_HANDLE = ANI_CLASS_SPACE + "AudioDeviceDescriptorHandle;";
static const std::string ANI_CLASS_AUDIO_SAMPLING_RATE = ANI_CLASS_SPACE + "AudioSamplingRate;";
static const std::string ANI_CLASS_AUDIO_STREAM_DEVICE_CHANGE_HANDLE =
    ANI_CLASS_SPACE + "AudioStreamDeviceChangeHandle;";
static const std::string ANI_CLASS_AUDIO_ENCODING_TYPE = ANI_CLASS_SPACE + "AudioEncodingType;";
static const std::string ANI_CLASS_DEVICE_BLOCK_STATUS = ANI_CLASS_SPACE + "DeviceBlockStatus;";
static const std::string ANI_CLASS_DEVICE_BLOCK_STATUS_INFO_HANDLE =
    ANI_CLASS_SPACE + "DeviceBlockStatusInfoHandle;";

} // namespace Media
} // namespace OHOS
#endif  // ANI_CLASS_NAME_H
