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
#define LOG_TAG "TaiheAudioEnum"
#endif

#include "taihe_audio_enum.h"
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "parameters.h"
#endif

namespace ANI::Audio {

static const std::map<OHOS::AudioStandard::DeviceRole, DeviceRole> DEVICE_ROLE_TAIHE_MAP = {
    {OHOS::AudioStandard::DeviceRole::INPUT_DEVICE, DeviceRole::key_t::INPUT_DEVICE},
    {OHOS::AudioStandard::DeviceRole::OUTPUT_DEVICE, DeviceRole::key_t::OUTPUT_DEVICE},
};

static const std::map<OHOS::AudioStandard::DeviceType, DeviceType> DEVICE_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_INVALID, DeviceType::key_t::INVALID},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE, DeviceType::key_t::EARPIECE},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER, DeviceType::key_t::SPEAKER},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADSET, DeviceType::key_t::WIRED_HEADSET},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADPHONES, DeviceType::key_t::WIRED_HEADPHONES},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceType::key_t::BLUETOOTH_SCO},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceType::key_t::BLUETOOTH_A2DP},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_MIC, DeviceType::key_t::MIC},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_USB_HEADSET, DeviceType::key_t::USB_HEADSET},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DP, DeviceType::key_t::DISPLAY_PORT},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_REMOTE_CAST, DeviceType::key_t::REMOTE_CAST},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_USB_DEVICE, DeviceType::key_t::USB_DEVICE},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_REMOTE_DAUDIO, DeviceType::key_t::REMOTE_DAUDIO},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT, DeviceType::key_t::DEFAULT},
};

static const std::map<OHOS::AudioStandard::AudioEncodingType, AudioEncodingType> AUDIO_ENCODING_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioEncodingType::ENCODING_INVALID, AudioEncodingType::key_t::ENCODING_TYPE_INVALID},
    {OHOS::AudioStandard::AudioEncodingType::ENCODING_PCM, AudioEncodingType::key_t::ENCODING_TYPE_RAW},
};

static const std::map<OHOS::AudioStandard::RendererState, AudioState> RENDERER_STATE_TAIHE_MAP = {
    {OHOS::AudioStandard::RendererState::RENDERER_INVALID, AudioState::key_t::STATE_INVALID},
    {OHOS::AudioStandard::RendererState::RENDERER_NEW, AudioState::key_t::STATE_NEW},
    {OHOS::AudioStandard::RendererState::RENDERER_PREPARED, AudioState::key_t::STATE_PREPARED},
    {OHOS::AudioStandard::RendererState::RENDERER_RUNNING, AudioState::key_t::STATE_RUNNING},
    {OHOS::AudioStandard::RendererState::RENDERER_STOPPED, AudioState::key_t::STATE_STOPPED},
    {OHOS::AudioStandard::RendererState::RENDERER_RELEASED, AudioState::key_t::STATE_RELEASED},
    {OHOS::AudioStandard::RendererState::RENDERER_PAUSED, AudioState::key_t::STATE_PAUSED},
};

static const std::map<OHOS::AudioStandard::CapturerState, AudioState> CAPTURER_STATE_TAIHE_MAP = {
    {OHOS::AudioStandard::CapturerState::CAPTURER_INVALID, AudioState::key_t::STATE_INVALID},
    {OHOS::AudioStandard::CapturerState::CAPTURER_NEW, AudioState::key_t::STATE_NEW},
    {OHOS::AudioStandard::CapturerState::CAPTURER_PREPARED, AudioState::key_t::STATE_PREPARED},
    {OHOS::AudioStandard::CapturerState::CAPTURER_RUNNING, AudioState::key_t::STATE_RUNNING},
    {OHOS::AudioStandard::CapturerState::CAPTURER_STOPPED, AudioState::key_t::STATE_STOPPED},
    {OHOS::AudioStandard::CapturerState::CAPTURER_RELEASED, AudioState::key_t::STATE_RELEASED},
    {OHOS::AudioStandard::CapturerState::CAPTURER_PAUSED, AudioState::key_t::STATE_PAUSED},
};

static const std::map<OHOS::AudioStandard::ContentType, ContentType> CONTENT_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_UNKNOWN, ContentType::key_t::CONTENT_TYPE_UNKNOWN},
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_SPEECH, ContentType::key_t::CONTENT_TYPE_SPEECH},
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_MUSIC, ContentType::key_t::CONTENT_TYPE_MUSIC},
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_MOVIE, ContentType::key_t::CONTENT_TYPE_MOVIE},
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_SONIFICATION, ContentType::key_t::CONTENT_TYPE_SONIFICATION},
    {OHOS::AudioStandard::ContentType::CONTENT_TYPE_RINGTONE, ContentType::key_t::CONTENT_TYPE_RINGTONE},
};

static const std::map<OHOS::AudioStandard::StreamUsage, StreamUsage> STREAM_USAGE_TAIHE_MAP = {
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN, StreamUsage::key_t::STREAM_USAGE_UNKNOWN},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MUSIC, StreamUsage::key_t::STREAM_USAGE_MUSIC},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION,
        StreamUsage::key_t::STREAM_USAGE_VOICE_COMMUNICATION},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT, StreamUsage::key_t::STREAM_USAGE_VOICE_ASSISTANT},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ALARM, StreamUsage::key_t::STREAM_USAGE_ALARM},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_MESSAGE, StreamUsage::key_t::STREAM_USAGE_VOICE_MESSAGE},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_RINGTONE, StreamUsage::key_t::STREAM_USAGE_RINGTONE},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION, StreamUsage::key_t::STREAM_USAGE_NOTIFICATION},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ACCESSIBILITY, StreamUsage::key_t::STREAM_USAGE_ACCESSIBILITY},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM, StreamUsage::key_t::STREAM_USAGE_SYSTEM},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MOVIE, StreamUsage::key_t::STREAM_USAGE_MOVIE},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_GAME, StreamUsage::key_t::STREAM_USAGE_GAME},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_AUDIOBOOK, StreamUsage::key_t::STREAM_USAGE_AUDIOBOOK},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION, StreamUsage::key_t::STREAM_USAGE_NAVIGATION},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_DTMF, StreamUsage::key_t::STREAM_USAGE_DTMF},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ENFORCED_TONE, StreamUsage::key_t::STREAM_USAGE_ENFORCED_TONE},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ULTRASONIC, StreamUsage::key_t::STREAM_USAGE_ULTRASONIC},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION,
        StreamUsage::key_t::STREAM_USAGE_VIDEO_COMMUNICATION},
    {OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT,
        StreamUsage::key_t::STREAM_USAGE_VOICE_CALL_ASSISTANT},
};

static const std::map<OHOS::AudioStandard::SourceType, SourceType> SOURCE_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_INVALID, SourceType::key_t::SOURCE_TYPE_INVALID},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC, SourceType::key_t::SOURCE_TYPE_MIC},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_RECOGNITION, SourceType::key_t::SOURCE_TYPE_VOICE_RECOGNITION},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE, SourceType::key_t::SOURCE_TYPE_PLAYBACK_CAPTURE},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_WAKEUP, SourceType::key_t::SOURCE_TYPE_WAKEUP},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_CALL, SourceType::key_t::SOURCE_TYPE_VOICE_CALL},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_COMMUNICATION,
        SourceType::key_t::SOURCE_TYPE_VOICE_COMMUNICATION},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_MESSAGE, SourceType::key_t::SOURCE_TYPE_VOICE_MESSAGE},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION,
        SourceType::key_t::SOURCE_TYPE_VOICE_TRANSCRIPTION},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_CAMCORDER, SourceType::key_t::SOURCE_TYPE_CAMCORDER},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_UNPROCESSED, SourceType::key_t::SOURCE_TYPE_UNPROCESSED},
};

static const std::map<OHOS::AudioStandard::EffectFlag, EffectFlag> EFFECT_FLAG_TAIHE_MAP = {
    {OHOS::AudioStandard::EffectFlag::RENDER_EFFECT_FLAG, EffectFlag::key_t::RENDER_EFFECT_FLAG},
    {OHOS::AudioStandard::EffectFlag::CAPTURE_EFFECT_FLAG, EffectFlag::key_t::CAPTURE_EFFECT_FLAG},
};

static const std::map<OHOS::AudioStandard::AudioScene, AudioScene> AUDIO_SCENE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_DEFAULT, AudioScene::key_t::AUDIO_SCENE_DEFAULT},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_RINGING, AudioScene::key_t::AUDIO_SCENE_RINGING},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CALL, AudioScene::key_t::AUDIO_SCENE_PHONE_CALL},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CHAT, AudioScene::key_t::AUDIO_SCENE_VOICE_CHAT},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_CALL_START, AudioScene::key_t::AUDIO_SCENE_DEFAULT},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_CALL_END, AudioScene::key_t::AUDIO_SCENE_DEFAULT},
    {OHOS::AudioStandard::AudioScene::AUDIO_SCENE_VOICE_RINGING, AudioScene::key_t::AUDIO_SCENE_RINGING},
};

static const std::map<OHOS::AudioStandard::InterruptType, InterruptType> INTERRUPT_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::InterruptType::INTERRUPT_TYPE_BEGIN, InterruptType::key_t::INTERRUPT_TYPE_BEGIN},
    {OHOS::AudioStandard::InterruptType::INTERRUPT_TYPE_END, InterruptType::key_t::INTERRUPT_TYPE_END},
};

static const std::map<OHOS::AudioStandard::InterruptHint, InterruptHint> INTERRUPT_HINT_TAIHE_MAP = {
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_NONE, InterruptHint::key_t::INTERRUPT_HINT_NONE},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_RESUME, InterruptHint::key_t::INTERRUPT_HINT_RESUME},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_PAUSE, InterruptHint::key_t::INTERRUPT_HINT_PAUSE},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_STOP, InterruptHint::key_t::INTERRUPT_HINT_STOP},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_DUCK, InterruptHint::key_t::INTERRUPT_HINT_DUCK},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_UNDUCK, InterruptHint::key_t::INTERRUPT_HINT_UNDUCK},
};

static const std::map<OHOS::AudioStandard::AudioVolumeMode, AudioVolumeMode> AUDIO_VOLUME_MODE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioVolumeMode::AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL,
        AudioVolumeMode::key_t::SYSTEM_GLOBAL},
    {OHOS::AudioStandard::AudioVolumeMode::AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL,
        AudioVolumeMode::key_t::APP_INDIVIDUAL},
};

static const std::map<OHOS::AudioStandard::DeviceChangeType, DeviceChangeType> DEVICE_CHANGE_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::DeviceChangeType::CONNECT, DeviceChangeType::key_t::CONNECT},
    {OHOS::AudioStandard::DeviceChangeType::DISCONNECT, DeviceChangeType::key_t::DISCONNECT},
};

static const std::map<OHOS::AudioStandard::AudioSessionDeactiveReason, AudioSessionDeactivatedReason>
    AUDIO_SESSION_DEACTIVE_REASON_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioSessionDeactiveReason::LOW_PRIORITY,
        AudioSessionDeactivatedReason::key_t::DEACTIVATED_LOWER_PRIORITY},
    {OHOS::AudioStandard::AudioSessionDeactiveReason::TIMEOUT,
        AudioSessionDeactivatedReason::key_t::DEACTIVATED_TIMEOUT},
};

bool TaiheAudioEnum::IsLegalCapturerType(int32_t type)
{
    bool result = false;
    switch (type) {
        case TYPE_INVALID:
        case TYPE_MIC:
        case TYPE_VOICE_RECOGNITION:
        case TYPE_PLAYBACK_CAPTURE:
        case TYPE_WAKEUP:
        case TYPE_COMMUNICATION:
        case TYPE_VOICE_CALL:
        case TYPE_MESSAGE:
        case TYPE_REMOTE_CAST:
        case TYPE_VOICE_TRANSCRIPTION:
        case TYPE_CAMCORDER:
        case TYPE_UNPROCESSED:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentVolType(int32_t inputType)
{
    bool result = false;
    switch (inputType) {
        case AudioJsVolumeType::RINGTONE:
        case AudioJsVolumeType::MEDIA:
        case AudioJsVolumeType::VOICE_CALL:
        case AudioJsVolumeType::VOICE_ASSISTANT:
        case AudioJsVolumeType::ALARM:
        case AudioJsVolumeType::SYSTEM:
        case AudioJsVolumeType::ACCESSIBILITY:
        case AudioJsVolumeType::ULTRASONIC:
        case AudioJsVolumeType::ALL:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentRingMode(int32_t ringMode)
{
    bool result = false;
    switch (ringMode) {
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_SILENT:
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_VIBRATE:
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_NORMAL:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

OHOS::AudioStandard::AudioVolumeType TaiheAudioEnum::GetNativeAudioVolumeType(int32_t volumeType)
{
    OHOS::AudioStandard::AudioVolumeType result = OHOS::AudioStandard::STREAM_MUSIC;

    switch (volumeType) {
        case AudioJsVolumeType::VOICE_CALL:
            result = OHOS::AudioStandard::STREAM_VOICE_CALL;
            break;
        case AudioJsVolumeType::RINGTONE:
            result = OHOS::AudioStandard::STREAM_RING;
            break;
        case AudioJsVolumeType::MEDIA:
            result = OHOS::AudioStandard::STREAM_MUSIC;
            break;
        case AudioJsVolumeType::ALARM:
            result = OHOS::AudioStandard::STREAM_ALARM;
            break;
        case AudioJsVolumeType::ACCESSIBILITY:
            result = OHOS::AudioStandard::STREAM_ACCESSIBILITY;
            break;
        case AudioJsVolumeType::VOICE_ASSISTANT:
            result = OHOS::AudioStandard::STREAM_VOICE_ASSISTANT;
            break;
        case AudioJsVolumeType::ULTRASONIC:
            result = OHOS::AudioStandard::STREAM_ULTRASONIC;
            break;
        case AudioJsVolumeType::SYSTEM:
            result = OHOS::AudioStandard::STREAM_SYSTEM;
            break;
        case AudioJsVolumeType::ALL:
            result = OHOS::AudioStandard::STREAM_ALL;
            break;
        default:
            result = OHOS::AudioStandard::STREAM_MUSIC;
            AUDIO_ERR_LOG("GetNativeAudioVolumeType: Unknown volume type, Set it to default MEDIA!");
            break;
    }

    return result;
}

OHOS::AudioStandard::AudioRingerMode TaiheAudioEnum::GetNativeAudioRingerMode(int32_t ringMode)
{
    OHOS::AudioStandard::AudioRingerMode result = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;

    switch (ringMode) {
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_SILENT:
            result = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_SILENT;
            break;
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_VIBRATE:
            result = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_VIBRATE;
            break;
        case TaiheAudioEnum::AudioRingMode::RINGER_MODE_NORMAL:
            result = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;
            break;
        default:
            result = OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;
            AUDIO_ERR_LOG("Unknown ringer mode requested by JS, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

AudioVolumeType TaiheAudioEnum::GetJsAudioVolumeType(OHOS::AudioStandard::AudioStreamType volumeType)
{
    AudioVolumeType result = static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::MEDIA);
    switch (volumeType) {
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_CALL:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_CALL_ASSISTANT:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::VOICE_CALL));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_RING:
        case OHOS::AudioStandard::AudioStreamType::STREAM_DTMF:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::RINGTONE));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_MUSIC:
        case OHOS::AudioStandard::AudioStreamType::STREAM_MEDIA:
        case OHOS::AudioStandard::AudioStreamType::STREAM_MOVIE:
        case OHOS::AudioStandard::AudioStreamType::STREAM_GAME:
        case OHOS::AudioStandard::AudioStreamType::STREAM_SPEECH:
        case OHOS::AudioStandard::AudioStreamType::STREAM_NAVIGATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_CAMCORDER:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_MESSAGE:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::MEDIA));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ALARM:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::ALARM));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ACCESSIBILITY:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::ACCESSIBILITY));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_ASSISTANT:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::VOICE_ASSISTANT));
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ULTRASONIC:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::ULTRASONIC));
            break;
        default:
            result = GetJsAudioVolumeTypeMore(volumeType);
            break;
    }
    return result;
}

AudioVolumeType TaiheAudioEnum::GetJsAudioVolumeTypeMore(OHOS::AudioStandard::AudioStreamType volumeType)
{
    AudioVolumeType result = static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::MEDIA);
    switch (volumeType) {
        case OHOS::AudioStandard::AudioStreamType::STREAM_SYSTEM:
        case OHOS::AudioStandard::AudioStreamType::STREAM_NOTIFICATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_SYSTEM_ENFORCED:
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
            result = (OHOS::system::GetBoolParameter("const.multimedia.audio.fwk_ec.enable", 0))?
                AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::SYSTEM)) :
                    AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::RINGTONE));
#else
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::RINGTONE));
#endif
            break;
        default:
            result = AudioVolumeType(static_cast<AudioVolumeType::key_t>(TaiheAudioEnum::MEDIA));
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentDeviceFlag(int32_t deviceFlag)
{
    bool result = false;
    switch (deviceFlag) {
        case OHOS::AudioStandard::DeviceFlag::NONE_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::OUTPUT_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::INPUT_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::ALL_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG:
        case OHOS::AudioStandard::DeviceFlag::ALL_L_D_DEVICES_FLAG:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentVolumeMode(int32_t volumeMode)
{
    bool result = false;
    switch (volumeMode) {
        case OHOS::AudioStandard::AudioVolumeMode::AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL:
        case OHOS::AudioStandard::AudioVolumeMode::AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentVolumeAdjustType(int32_t adjustType)
{
    bool result = false;
    switch (adjustType) {
        case OHOS::AudioStandard::VolumeAdjustType::VOLUME_UP:
        case OHOS::AudioStandard::VolumeAdjustType::VOLUME_DOWN:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentStreamUsage(int32_t streamUsage)
{
    bool result = false;
    switch (streamUsage) {
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MEDIA:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ALARM:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_MESSAGE:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ACCESSIBILITY:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MOVIE:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_GAME:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_AUDIOBOOK:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_DTMF:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ENFORCED_TONE:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ULTRASONIC:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentConcurrencyMode(int32_t concurrencyMode)
{
    bool result = false;
    switch (concurrencyMode) {
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::INVALID):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::DEFAULT):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::MIX_WITH_OTHERS):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::DUCK_OTHERS):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::PAUSE_OTHERS):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioConcurrencyMode::SILENT):
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsValidSourceType(int32_t intValue)
{
    switch (intValue) {
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_MIC:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_ULTRASONIC:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_RECOGNITION:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_WAKEUP:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_CALL:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_MESSAGE:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_REMOTE_CAST:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_CAMCORDER:
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_UNPROCESSED:
            return true;
        default:
            return false;
    }
}

bool TaiheAudioEnum::IsLegalDeviceUsage(int32_t usage)
{
    bool result = false;
    switch (usage) {
        case OHOS::AudioStandard::AudioDeviceUsage::MEDIA_OUTPUT_DEVICES:
        case OHOS::AudioStandard::AudioDeviceUsage::MEDIA_INPUT_DEVICES:
        case OHOS::AudioStandard::AudioDeviceUsage::ALL_MEDIA_DEVICES:
        case OHOS::AudioStandard::AudioDeviceUsage::CALL_OUTPUT_DEVICES:
        case OHOS::AudioStandard::AudioDeviceUsage::CALL_INPUT_DEVICES:
        case OHOS::AudioStandard::AudioDeviceUsage::ALL_CALL_DEVICES:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

DeviceRole TaiheAudioEnum::ToTaiheDeviceRole(OHOS::AudioStandard::DeviceRole type)
{
    auto iter = DEVICE_ROLE_TAIHE_MAP.find(type);
    if (iter == DEVICE_ROLE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceRole invalid type: %{public}d", static_cast<int32_t>(type));
        return DeviceRole::key_t::OUTPUT_DEVICE;
    }
    return iter->second;
}

DeviceType TaiheAudioEnum::ToTaiheDeviceType(OHOS::AudioStandard::DeviceType type)
{
    auto iter = DEVICE_TYPE_TAIHE_MAP.find(type);
    if (iter == DEVICE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceType invalid type: %{public}d", static_cast<int32_t>(type));
        return DeviceType::key_t::INVALID;
    }
    return iter->second;
}

AudioEncodingType TaiheAudioEnum::ToTaiheAudioEncodingType(OHOS::AudioStandard::AudioEncodingType type)
{
    auto iter = AUDIO_ENCODING_TYPE_TAIHE_MAP.find(type);
    if (iter == AUDIO_ENCODING_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioEncodingType invalid type: %{public}d", static_cast<int32_t>(type));
        return AudioEncodingType::key_t::ENCODING_TYPE_INVALID;
    }
    return iter->second;
}

AudioState TaiheAudioEnum::ToTaiheAudioState(OHOS::AudioStandard::RendererState state)
{
    auto iter = RENDERER_STATE_TAIHE_MAP.find(state);
    if (iter == RENDERER_STATE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioState(Renderer) invalid state: %{public}d", static_cast<int32_t>(state));
        return AudioState::key_t::STATE_INVALID;
    }
    return iter->second;
}

AudioState TaiheAudioEnum::ToTaiheAudioState(OHOS::AudioStandard::CapturerState state)
{
    auto iter = CAPTURER_STATE_TAIHE_MAP.find(state);
    if (iter == CAPTURER_STATE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioState(Capturer) invalid state: %{public}d", static_cast<int32_t>(state));
        return AudioState::key_t::STATE_INVALID;
    }
    return iter->second;
}

ContentType TaiheAudioEnum::ToTaiheContentType(OHOS::AudioStandard::ContentType type)
{
    auto iter = CONTENT_TYPE_TAIHE_MAP.find(type);
    if (iter == CONTENT_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheContentType invalid type: %{public}d", static_cast<int32_t>(type));
        return ContentType::key_t::CONTENT_TYPE_UNKNOWN;
    }
    return iter->second;
}

StreamUsage TaiheAudioEnum::ToTaiheStreamUsage(OHOS::AudioStandard::StreamUsage usage)
{
    auto iter = STREAM_USAGE_TAIHE_MAP.find(usage);
    if (iter == STREAM_USAGE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheStreamUsage invalid usage: %{public}d", static_cast<int32_t>(usage));
        return StreamUsage::key_t::STREAM_USAGE_UNKNOWN;
    }
    return iter->second;
}

SourceType TaiheAudioEnum::ToTaiheSourceType(OHOS::AudioStandard::SourceType type)
{
    auto iter = SOURCE_TYPE_TAIHE_MAP.find(type);
    if (iter == SOURCE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheSourceType invalid type: %{public}d", static_cast<int32_t>(type));
        return SourceType::key_t::SOURCE_TYPE_INVALID;
    }
    return iter->second;
}

EffectFlag TaiheAudioEnum::ToTaiheEffectFlag(OHOS::AudioStandard::EffectFlag flag)
{
    auto iter = EFFECT_FLAG_TAIHE_MAP.find(flag);
    if (iter == EFFECT_FLAG_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheEffectFlag invalid flag: %{public}d", static_cast<int32_t>(flag));
        return EffectFlag::key_t::RENDER_EFFECT_FLAG;
    }
    return iter->second;
}

AudioScene TaiheAudioEnum::ToTaiheAudioScene(OHOS::AudioStandard::AudioScene scene)
{
    auto iter = AUDIO_SCENE_TAIHE_MAP.find(scene);
    if (iter == AUDIO_SCENE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioScene invalid scene: %{public}d", static_cast<int32_t>(scene));
        return AudioScene::key_t::AUDIO_SCENE_DEFAULT;
    }
    return iter->second;
}

InterruptType TaiheAudioEnum::ToTaiheInterruptType(OHOS::AudioStandard::InterruptType type)
{
    auto iter = INTERRUPT_TYPE_TAIHE_MAP.find(type);
    if (iter == INTERRUPT_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheInterruptType invalid type: %{public}d", static_cast<int32_t>(type));
        return InterruptType::key_t::INTERRUPT_TYPE_BEGIN;
    }
    return iter->second;
}

InterruptHint TaiheAudioEnum::ToTaiheInterruptHint(OHOS::AudioStandard::InterruptHint hint)
{
    auto iter = INTERRUPT_HINT_TAIHE_MAP.find(hint);
    if (iter == INTERRUPT_HINT_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheInterruptHint invalid hint: %{public}d", static_cast<int32_t>(hint));
        return InterruptHint::key_t::INTERRUPT_HINT_NONE;
    }
    return iter->second;
}

AudioVolumeMode TaiheAudioEnum::ToTaiheAudioVolumeMode(OHOS::AudioStandard::AudioVolumeMode mode)
{
    auto iter = AUDIO_VOLUME_MODE_TAIHE_MAP.find(mode);
    if (iter == AUDIO_VOLUME_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioVolumeMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        return AudioVolumeMode::key_t::SYSTEM_GLOBAL;
    }
    return iter->second;
}

DeviceChangeType TaiheAudioEnum::ToTaiheDeviceChangeType(OHOS::AudioStandard::DeviceChangeType type)
{
    auto iter = DEVICE_CHANGE_TYPE_TAIHE_MAP.find(type);
    if (iter == DEVICE_CHANGE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioVolumeMode invalid mode: %{public}d", static_cast<int32_t>(type));
        return DeviceChangeType::key_t::CONNECT;
    }
    return iter->second;
}

AudioSessionDeactivatedReason TaiheAudioEnum::ToTaiheSessionDeactiveReason(
    OHOS::AudioStandard::AudioSessionDeactiveReason reason)
{
    auto iter = AUDIO_SESSION_DEACTIVE_REASON_TAIHE_MAP.find(reason);
    if (iter == AUDIO_SESSION_DEACTIVE_REASON_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheSessionDeactiveReason invalid mode: %{public}d", static_cast<int32_t>(reason));
        return AudioSessionDeactivatedReason::key_t::DEACTIVATED_TIMEOUT;
    }
    return iter->second;
}
} // namespace ANI::Audio
