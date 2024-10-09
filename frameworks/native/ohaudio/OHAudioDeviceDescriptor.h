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

#ifndef OH_AUDIO_DEVICE_DESCRIPTOR_H
#define OH_AUDIO_DEVICE_DESCRIPTOR_H

#include "audio_info.h"
#include "audio_common_log.h"
#include "native_audio_routing_manager.h"
#include "native_audio_common.h"
#include "native_audio_device_base.h"
#include "audio_system_manager.h"
#include "audio_routing_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioDeviceDescriptor {
public:
    explicit OHAudioDeviceDescriptor(sptr<AudioDeviceDescriptor> audioDeviceDescriptor);
    ~OHAudioDeviceDescriptor();

    OH_AudioCommon_Result GetDeviceRole(OH_AudioDevice_Role *deviceRole);
    OH_AudioCommon_Result GetDeviceType(OH_AudioDevice_Type *deviceType);
    OH_AudioCommon_Result GetDeviceId(uint32_t *id);
    OH_AudioCommon_Result GetDeviceName(char **name);
    OH_AudioCommon_Result GetDeviceAddress(char **address);
    OH_AudioCommon_Result GetDeviceSampleRates(uint32_t **sampleRates, uint32_t *size);
    OH_AudioCommon_Result GetDeviceChannelCounts(uint32_t **channelCounts, uint32_t *size);
    OH_AudioCommon_Result GetDeviceDisplayName(char **displayName);
    OH_AudioCommon_Result GetDeviceEncodingTypes(OH_AudioStream_EncodingType **encodingTypes, uint32_t *size);

private:
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor_;
    uint32_t *audioSamplingRate_ = nullptr;
    uint32_t *audioChannel_ = nullptr;
    OH_AudioStream_EncodingType *encodingType_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_DEVICE_DESCRIPTOR_H