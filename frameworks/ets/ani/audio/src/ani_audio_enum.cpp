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

#ifndef LOG_TAG
#define LOG_TAG "AniAudioEnum"
#endif

#include "ani_audio_enum.h"
#include "audio_common_log.h"
#include "audio_info.h"
#include "ani_class_name.h"
#include "ani_param_utils.h"

namespace OHOS {
namespace AudioStandard {
static const std::map<AudioEnumAni::AudioJsVolumeType, int32_t> ANI_AUDIO_JS_VOLUME_TYPE_INDEX_MAP = {
    {AudioEnumAni::AudioJsVolumeType::VOICE_CALL, 0},
    {AudioEnumAni::AudioJsVolumeType::RINGTONE, 1},
    {AudioEnumAni::AudioJsVolumeType::MEDIA, 2},
    {AudioEnumAni::AudioJsVolumeType::ALARM, 3},
    {AudioEnumAni::AudioJsVolumeType::ACCESSIBILITY, 4},
    {AudioEnumAni::AudioJsVolumeType::VOICE_ASSISTANT, 5},
    {AudioEnumAni::AudioJsVolumeType::ULTRASONIC, 6},
    {AudioEnumAni::AudioJsVolumeType::ALL, 7}
    
};

static const std::map<InterruptHint, int32_t> ANI_INTERRUPT_HINT_INDEX_MAP = {
    {InterruptHint::INTERRUPT_HINT_NONE, 0},
    {InterruptHint::INTERRUPT_HINT_RESUME, 1},
    {InterruptHint::INTERRUPT_HINT_PAUSE, 2},
    {InterruptHint::INTERRUPT_HINT_STOP, 3},
    {InterruptHint::INTERRUPT_HINT_DUCK, 4},
    {InterruptHint::INTERRUPT_HINT_UNDUCK, 5}
};

static const std::map<InterruptType, int32_t> ANI_INTERRUPT_TYPE_INDEX_MAP = {
    {InterruptType::INTERRUPT_TYPE_BEGIN, 0},
    {InterruptType::INTERRUPT_TYPE_END, 1}
};

static const std::map<DeviceRole, int32_t> ANI_DEVICE_ROLE_INDEX_MAP = {
    {DeviceRole::INPUT_DEVICE, 0},
    {DeviceRole::OUTPUT_DEVICE, 1}
};

static const std::map<DeviceType, int32_t> ANI_DEVICE_TYPE_INDEX_MAP = {
    {DeviceType::DEVICE_TYPE_INVALID, 0},
    {DeviceType::DEVICE_TYPE_EARPIECE, 1},
    {DeviceType::DEVICE_TYPE_SPEAKER, 2},
    {DeviceType::DEVICE_TYPE_WIRED_HEADSET, 3},
    {DeviceType::DEVICE_TYPE_WIRED_HEADPHONES, 4},
    {DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, 5},
    {DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, 6},
    {DeviceType::DEVICE_TYPE_MIC, 7},
    {DeviceType::DEVICE_TYPE_USB_HEADSET, 8},
    {DeviceType::DEVICE_TYPE_DP, 9},
    {DeviceType::DEVICE_TYPE_REMOTE_CAST, 10},
    {DeviceType::DEVICE_TYPE_DEFAULT, 11}
};

static const std::map<AudioStreamDeviceChangeReason, int32_t> ANI_AUDIO_STREAM_DEVICE_CHANGE_REASON_INDEX_MAP = {
    {AudioStreamDeviceChangeReason::UNKNOWN, 0},
    {AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE, 1},
    {AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE, 2},
    {AudioStreamDeviceChangeReason::OVERRODE, 3}
};

static const std::map<ContentType, int32_t> ANI_CONTENT_TYPE_INDEX_MAP = {
    {ContentType::CONTENT_TYPE_UNKNOWN, 0},
    {ContentType::CONTENT_TYPE_SPEECH, 1},
    {ContentType::CONTENT_TYPE_MUSIC, 2},
    {ContentType::CONTENT_TYPE_MOVIE, 3},
    {ContentType::CONTENT_TYPE_SONIFICATION, 4},
    {ContentType::CONTENT_TYPE_RINGTONE, 5}
};

static const std::map<StreamUsage, int32_t> ANI_STREAM_USAGE_INDEX_MAP = {
    {StreamUsage::STREAM_USAGE_UNKNOWN, 0},
    {StreamUsage::STREAM_USAGE_MEDIA, 1},
    {StreamUsage::STREAM_USAGE_MUSIC, 2},
    {StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION, 3},
    {StreamUsage::STREAM_USAGE_VOICE_ASSISTANT, 4},
    {StreamUsage::STREAM_USAGE_ALARM, 5},
    {StreamUsage::STREAM_USAGE_VOICE_MESSAGE, 6},
    {StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE, 7},
    {StreamUsage::STREAM_USAGE_RINGTONE, 7},
    {StreamUsage::STREAM_USAGE_NOTIFICATION, 8},
    {StreamUsage::STREAM_USAGE_ACCESSIBILITY, 9},
    {StreamUsage::STREAM_USAGE_SYSTEM, 10},
    {StreamUsage::STREAM_USAGE_MOVIE, 11},
    {StreamUsage::STREAM_USAGE_GAME, 12},
    {StreamUsage::STREAM_USAGE_AUDIOBOOK, 13},
    {StreamUsage::STREAM_USAGE_NAVIGATION, 14},
    {StreamUsage::STREAM_USAGE_DTMF, 15},
    {StreamUsage::STREAM_USAGE_ENFORCED_TONE, 16},
    {StreamUsage::STREAM_USAGE_ULTRASONIC, 17},
    {StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION, 18},
    {StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT, 19}
};

static const std::map<InterruptMode, int32_t> ANI_INTERRUPT_MODE_INDEX_MAP = {
    {InterruptMode::SHARE_MODE, 0},
    {InterruptMode::INDEPENDENT_MODE, 1}
};

static const std::map<InterruptForceType, int32_t> ANI_INTERRUPT_FORCE_TYPE_INDEX_MAP = {
    {InterruptForceType::INTERRUPT_FORCE, 0},
    {InterruptForceType::INTERRUPT_SHARE, 1}
};
static const std::map<AudioSamplingRate, int32_t> ANI_AUDIO_SAMPLING_RATE_INDEX_MAP = {
    {AudioSamplingRate::SAMPLE_RATE_8000, 0},
    {AudioSamplingRate::SAMPLE_RATE_11025, 1},
    {AudioSamplingRate::SAMPLE_RATE_12000, 2},
    {AudioSamplingRate::SAMPLE_RATE_16000, 3},
    {AudioSamplingRate::SAMPLE_RATE_22050, 4},
    {AudioSamplingRate::SAMPLE_RATE_24000, 5},
    {AudioSamplingRate::SAMPLE_RATE_32000, 6},
    {AudioSamplingRate::SAMPLE_RATE_44100, 7},
    {AudioSamplingRate::SAMPLE_RATE_48000, 8},
    {AudioSamplingRate::SAMPLE_RATE_64000, 9},
    {AudioSamplingRate::SAMPLE_RATE_88200, 10},
    {AudioSamplingRate::SAMPLE_RATE_96000, 11},
    {AudioSamplingRate::SAMPLE_RATE_176400, 12},
    {AudioSamplingRate::SAMPLE_RATE_192000, 13}
};

static const std::map<AudioEncodingType, int32_t> ANI_AUDIO_ENCODING_TYPE_INDEX_MAP = {
    {AudioEncodingType::ENCODING_INVALID, 0},
    {AudioEncodingType::ENCODING_PCM, 1}
};

static const std::map<DeviceBlockStatus, int32_t> ANI_DEVICE_BLOCK_STATUS_INDEX_MAP = {
    {DeviceBlockStatus::DEVICE_UNBLOCKED, 0},
    {DeviceBlockStatus::DEVICE_BLOCKED, 1}
};

ani_status AudioEnumAni::EnumGetValueInt32(ani_env *env, ani_enum_item enumItem, int32_t &value)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    ani_int aniInt {};
    ani_status status = env->EnumItem_GetValue_Int(enumItem, &aniInt);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("EnumItem_GetValue_Int failed");
        return status;
    }
    status = AniParamUtils::GetInt32(env, aniInt, value);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("GetInt32 failed");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::EnumGetValueString(ani_env *env, ani_enum_item enumItem, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    ani_string aniString {};
    ani_status status = env->EnumItem_GetValue_String(enumItem, &aniString);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("EnumItem_GetValue_String failed");
        return status;
    }
    status = AniParamUtils::GetString(env, aniString, value);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("GetString failed");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, AudioJsVolumeType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_AUDIO_JS_VOLUME_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_AUDIO_JS_VOLUME_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Error enum:%{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_AUDIO_VOLUME_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, InterruptHint value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_INTERRUPT_HINT_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_INTERRUPT_HINT_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_INTERRUPT_HINT.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, InterruptType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_INTERRUPT_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_INTERRUPT_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_INTERRUPT_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, DeviceRole value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_DEVICE_ROLE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_DEVICE_ROLE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_DEVICE_ROLE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, DeviceType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_DEVICE_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_DEVICE_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_DEVICE_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, AudioStreamDeviceChangeReason value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_AUDIO_STREAM_DEVICE_CHANGE_REASON_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_AUDIO_STREAM_DEVICE_CHANGE_REASON_INDEX_MAP.end(),
        ANI_INVALID_ARGS, "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_AUDIO_STREAM_DEVICE_CHANGE_REASON.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, ContentType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_CONTENT_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_CONTENT_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_CONTENT_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, StreamUsage value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_STREAM_USAGE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_STREAM_USAGE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_STREAM_USAGE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, InterruptMode value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_INTERRUPT_MODE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_INTERRUPT_MODE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_INTERRUPT_MODE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, InterruptForceType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_INTERRUPT_FORCE_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_INTERRUPT_FORCE_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_INTERRUPT_FORCE_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, AudioSamplingRate value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_AUDIO_SAMPLING_RATE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_AUDIO_SAMPLING_RATE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_INTERRUPT_FORCE_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, AudioEncodingType value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_AUDIO_ENCODING_TYPE_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_AUDIO_ENCODING_TYPE_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_AUDIO_ENCODING_TYPE.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

ani_status AudioEnumAni::ToAniEnum(ani_env *env, DeviceBlockStatus value, ani_enum_item &aniEnumItem)
{
    CHECK_AND_RETURN_RET_LOG(env != nullptr, ANI_INVALID_ARGS, "Invalid env");

    auto it = ANI_DEVICE_BLOCK_STATUS_INDEX_MAP.find(value);
    CHECK_AND_RETURN_RET_LOG(it != ANI_DEVICE_BLOCK_STATUS_INDEX_MAP.end(), ANI_INVALID_ARGS,
        "Unsupport enum: %{public}d", value);
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    ani_status status = env->FindEnum(ANI_CLASS_DEVICE_BLOCK_STATUS.c_str(), &aniEnum);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum Fail");
        return status;
    }
    status = env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem);
    if (status != ANI_OK) {
        AUDIO_ERR_LOG("Find Enum item Fail");
        return status;
    }
    return ANI_OK;
}

bool AudioEnumAni::IsLegalInputArgumentStreamUsage(int32_t streamUsage)
{
    bool result = false;
    switch (streamUsage) {
        case STREAM_USAGE_UNKNOWN:
        case STREAM_USAGE_MEDIA:
        case STREAM_USAGE_VOICE_COMMUNICATION:
        case STREAM_USAGE_VOICE_ASSISTANT:
        case STREAM_USAGE_ALARM:
        case STREAM_USAGE_VOICE_MESSAGE:
        case STREAM_USAGE_NOTIFICATION_RINGTONE:
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_ACCESSIBILITY:
        case STREAM_USAGE_SYSTEM:
        case STREAM_USAGE_MOVIE:
        case STREAM_USAGE_GAME:
        case STREAM_USAGE_AUDIOBOOK:
        case STREAM_USAGE_NAVIGATION:
        case STREAM_USAGE_DTMF:
        case STREAM_USAGE_ENFORCED_TONE:
        case STREAM_USAGE_ULTRASONIC:
        case STREAM_USAGE_VIDEO_COMMUNICATION:
        case STREAM_USAGE_VOICE_CALL_ASSISTANT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

int32_t AudioEnumAni::GetJsAudioVolumeType(AudioStreamType volumeType)
{
    int32_t result = MEDIA;
    switch (volumeType) {
        case AudioStreamType::STREAM_VOICE_CALL:
        case AudioStreamType::STREAM_VOICE_COMMUNICATION:
        case AudioStreamType::STREAM_VOICE_CALL_ASSISTANT:
            result = AudioEnumAni::VOICE_CALL;
            break;
        case AudioStreamType::STREAM_RING:
        case AudioStreamType::STREAM_SYSTEM:
        case AudioStreamType::STREAM_NOTIFICATION:
        case AudioStreamType::STREAM_SYSTEM_ENFORCED:
        case AudioStreamType::STREAM_DTMF:
            result = AudioEnumAni::RINGTONE;
            break;
        case AudioStreamType::STREAM_MUSIC:
        case AudioStreamType::STREAM_MEDIA:
        case AudioStreamType::STREAM_MOVIE:
        case AudioStreamType::STREAM_GAME:
        case AudioStreamType::STREAM_SPEECH:
        case AudioStreamType::STREAM_NAVIGATION:
        case AudioStreamType::STREAM_CAMCORDER:
        case AudioStreamType::STREAM_VOICE_MESSAGE:
            result = AudioEnumAni::MEDIA;
            break;
        case AudioStreamType::STREAM_ALARM:
            result = AudioEnumAni::ALARM;
            break;
        case AudioStreamType::STREAM_ACCESSIBILITY:
            result = AudioEnumAni::ACCESSIBILITY;
            break;
        case AudioStreamType::STREAM_VOICE_ASSISTANT:
            result = AudioEnumAni::VOICE_ASSISTANT;
            break;
        case AudioStreamType::STREAM_ULTRASONIC:
            result = AudioEnumAni::ULTRASONIC;
            break;
        default:
            result = AudioEnumAni::MEDIA;
            break;
    }
    return result;
}

} // namespace AudioStandard
} // namespace OHOS
