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

#ifndef ANI_AUDIO_ENUM_H
#define ANI_AUDIO_ENUM_H

#include "ani.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioEnumAni {
public:
    enum AudioJsVolumeType {
        VOLUMETYPE_DEFAULT = -1,
        VOICE_CALL = 0,
        RINGTONE = 2,
        MEDIA = 3,
        ALARM = 4,
        ACCESSIBILITY = 5,
        VOICE_ASSISTANT = 9,
        ULTRASONIC = 10,
        VOLUMETYPE_MAX,
        ALL = 100
    };
    static ani_status EnumGetValueInt32(ani_env *env, ani_enum_item enumItem, int32_t &value);
    static ani_status EnumGetValueString(ani_env *env, ani_enum_item enumItem, std::string &value);

    static ani_status ToAniEnum(ani_env *env, AudioVolumeType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, AudioJsVolumeType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, InterruptHint value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, InterruptType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, DeviceRole value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, DeviceType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, AudioStreamDeviceChangeReason value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, ContentType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, StreamUsage value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, InterruptMode value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, InterruptForceType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, AudioSamplingRate value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, AudioEncodingType value, ani_enum_item &aniEnumItem);
    static ani_status ToAniEnum(ani_env *env, DeviceBlockStatus value, ani_enum_item &aniEnumItem);
    static bool IsLegalInputArgumentStreamUsage(int32_t streamUsage);
    static int32_t GetJsAudioVolumeType(AudioStreamType volumeType);
};

} // namespace AudioStandard
} // namespace OHOS
#endif  // ANI_AUDIO_ENUM_H
