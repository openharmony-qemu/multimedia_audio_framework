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
#include "taihe_audio_error.h"
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
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_ACCESSORY, DeviceType::key_t::ACCESSORY},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_HDMI, DeviceType::key_t::HDMI},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_LINE_DIGITAL, DeviceType::key_t::LINE_DIGITAL},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_REMOTE_DAUDIO, DeviceType::key_t::REMOTE_DAUDIO},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_HEARING_AID, DeviceType::key_t::HEARING_AID},
    {OHOS::AudioStandard::DeviceType::DEVICE_TYPE_NEARLINK, DeviceType::key_t::NEARLINK},
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
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_WAKEUP, SourceType::key_t::SOURCE_TYPE_WAKEUP},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_CALL, SourceType::key_t::SOURCE_TYPE_VOICE_CALL},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_COMMUNICATION,
        SourceType::key_t::SOURCE_TYPE_VOICE_COMMUNICATION},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_MESSAGE, SourceType::key_t::SOURCE_TYPE_VOICE_MESSAGE},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION,
        SourceType::key_t::SOURCE_TYPE_VOICE_TRANSCRIPTION},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_CAMCORDER, SourceType::key_t::SOURCE_TYPE_CAMCORDER},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_UNPROCESSED, SourceType::key_t::SOURCE_TYPE_UNPROCESSED},
    {OHOS::AudioStandard::SourceType::SOURCE_TYPE_LIVE, SourceType::key_t::SOURCE_TYPE_LIVE},
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
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_MUTE, InterruptHint::key_t::INTERRUPT_HINT_MUTE},
    {OHOS::AudioStandard::InterruptHint::INTERRUPT_HINT_UNMUTE, InterruptHint::key_t::INTERRUPT_HINT_UNMUTE},
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

static const std::map<OHOS::AudioStandard::ConnectType, ConnectType> CONNECT_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::ConnectType::CONNECT_TYPE_LOCAL, ConnectType::key_t::CONNECT_TYPE_LOCAL},
    {OHOS::AudioStandard::ConnectType::CONNECT_TYPE_DISTRIBUTED, ConnectType::key_t::CONNECT_TYPE_DISTRIBUTED},
};

static const std::map<::AsrNoiseSuppressionMode, ohos::multimedia::audio::AsrNoiseSuppressionMode>
    ASR_NOISE_SUPPRESSION_MODE_TAIHE_MAP = {
    {::AsrNoiseSuppressionMode::BYPASS, ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::BYPASS},
    {::AsrNoiseSuppressionMode::STANDARD, ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::STANDARD},
    {::AsrNoiseSuppressionMode::NEAR_FIELD, ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::NEAR_FIELD},
    {::AsrNoiseSuppressionMode::FAR_FIELD, ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::FAR_FIELD},
};

static const std::map<::AsrAecMode, ohos::multimedia::audio::AsrAecMode> ASR_AEC_MODE_TAIHE_MAP = {
    {::AsrAecMode::BYPASS, ohos::multimedia::audio::AsrAecMode::key_t::BYPASS},
    {::AsrAecMode::STANDARD, ohos::multimedia::audio::AsrAecMode::key_t::STANDARD},
};

static const std::map<::AsrWhisperDetectionMode, ohos::multimedia::audio::AsrWhisperDetectionMode>
    ASR_WHISPER_DETECTION_MODE_TAIHE_MAP = {
    {::AsrWhisperDetectionMode::BYPASS, ohos::multimedia::audio::AsrWhisperDetectionMode::key_t::BYPASS},
    {::AsrWhisperDetectionMode::STANDARD, ohos::multimedia::audio::AsrWhisperDetectionMode::key_t::STANDARD},
};

static const std::map<OHOS::AudioStandard::InterruptForceType, InterruptForceType> INTERRUPT_FORCE_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::InterruptForceType::INTERRUPT_FORCE, InterruptForceType::key_t::INTERRUPT_FORCE},
    {OHOS::AudioStandard::InterruptForceType::INTERRUPT_SHARE, InterruptForceType::key_t::INTERRUPT_SHARE},
};

static const std::map<OHOS::AudioStandard::AudioSpatializationSceneType, AudioSpatializationSceneType>
    AUDIO_SPATIALIZATION_SCENE_TYPE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT,
        AudioSpatializationSceneType::key_t::DEFAULT},
    {OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MUSIC,
        AudioSpatializationSceneType::key_t::MUSIC},
    {OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MOVIE,
        AudioSpatializationSceneType::key_t::MOVIE},
    {OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
        AudioSpatializationSceneType::key_t::AUDIOBOOK},
};

static const std::map<TaiheAudioEnum::AudioJsVolumeType, AudioVolumeType> AUDIO_VOLUME_TYPE_TAIHE_MAP = {
    {TaiheAudioEnum::AudioJsVolumeType::VOICE_CALL, AudioVolumeType::key_t::VOICE_CALL},
    {TaiheAudioEnum::AudioJsVolumeType::RINGTONE, AudioVolumeType::key_t::RINGTONE},
    {TaiheAudioEnum::AudioJsVolumeType::MEDIA, AudioVolumeType::key_t::MEDIA},
    {TaiheAudioEnum::AudioJsVolumeType::ALARM, AudioVolumeType::key_t::ALARM},
    {TaiheAudioEnum::AudioJsVolumeType::ACCESSIBILITY, AudioVolumeType::key_t::ACCESSIBILITY},
    {TaiheAudioEnum::AudioJsVolumeType::SYSTEM, AudioVolumeType::key_t::SYSTEM},
    {TaiheAudioEnum::AudioJsVolumeType::VOICE_ASSISTANT, AudioVolumeType::key_t::VOICE_ASSISTANT},
    {TaiheAudioEnum::AudioJsVolumeType::ULTRASONIC, AudioVolumeType::key_t::ULTRASONIC},
    {TaiheAudioEnum::AudioJsVolumeType::ALL, AudioVolumeType::key_t::ALL},
};

static const std::map<OHOS::AudioStandard::AudioRingerMode, AudioRingMode> AUDIO_RING_MODE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_SILENT,
        ohos::multimedia::audio::AudioRingMode::key_t::RINGER_MODE_SILENT},
    {OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_VIBRATE,
        ohos::multimedia::audio::AudioRingMode::key_t::RINGER_MODE_VIBRATE},
    {OHOS::AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL,
        ohos::multimedia::audio::AudioRingMode::key_t::RINGER_MODE_NORMAL},
};

static const std::map<OHOS::AudioStandard::AudioEffectMode, AudioEffectMode> AUDIO_EFFECT_MODE_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioEffectMode::EFFECT_NONE, AudioEffectMode::key_t::EFFECT_NONE},
    {OHOS::AudioStandard::AudioEffectMode::EFFECT_DEFAULT, AudioEffectMode::key_t::EFFECT_DEFAULT},
};

static const std::map<OHOS::AudioStandard::AudioStreamDeviceChangeReason, AudioStreamDeviceChangeReason>
    AUDIO_STREAM_DEVICE_CHANGE_REASON_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::UNKNOWN, AudioStreamDeviceChangeReason::key_t::REASON_UNKNOWN},
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
        AudioStreamDeviceChangeReason::key_t::REASON_NEW_DEVICE_AVAILABLE},
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
        AudioStreamDeviceChangeReason::key_t::REASON_OLD_DEVICE_UNAVAILABLE},
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::OVERRODE,
        AudioStreamDeviceChangeReason::key_t::REASON_OVERRODE},
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::AUDIO_SESSION_ACTIVATE,
        AudioStreamDeviceChangeReason::key_t::REASON_SESSION_ACTIVATED},
    {OHOS::AudioStandard::AudioStreamDeviceChangeReason::STREAM_PRIORITY_CHANGED,
        AudioStreamDeviceChangeReason::key_t::REASON_STREAM_PRIORITY_CHANGED},
};

static const std::map<OHOS::AudioStandard::AudioChannelLayout, AudioChannelLayout> AUDIO_CHANNEL_LAYOUT_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_UNKNOWN, AudioChannelLayout::key_t::CH_LAYOUT_UNKNOWN},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_MONO, AudioChannelLayout::key_t::CH_LAYOUT_MONO},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_STEREO, AudioChannelLayout::key_t::CH_LAYOUT_STEREO},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX,
        AudioChannelLayout::key_t::CH_LAYOUT_STEREO_DOWNMIX},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_2POINT1, AudioChannelLayout::key_t::CH_LAYOUT_2POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_3POINT0, AudioChannelLayout::key_t::CH_LAYOUT_3POINT0},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_SURROUND, AudioChannelLayout::key_t::CH_LAYOUT_SURROUND},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_3POINT1, AudioChannelLayout::key_t::CH_LAYOUT_3POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_4POINT0, AudioChannelLayout::key_t::CH_LAYOUT_4POINT0},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_QUAD, AudioChannelLayout::key_t::CH_LAYOUT_QUAD},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_QUAD_SIDE, AudioChannelLayout::key_t::CH_LAYOUT_QUAD_SIDE},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_2POINT0POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_2POINT0POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER1_ACN_N3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER1_ACN_N3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER1_ACN_SN3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER1_ACN_SN3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER1_FUMA,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER1_FUMA},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_4POINT1, AudioChannelLayout::key_t::CH_LAYOUT_4POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT0, AudioChannelLayout::key_t::CH_LAYOUT_5POINT0},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT0_BACK,
        AudioChannelLayout::key_t::CH_LAYOUT_5POINT0_BACK},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_2POINT1POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_2POINT1POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_3POINT0POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_3POINT0POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT1, AudioChannelLayout::key_t::CH_LAYOUT_5POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT1_BACK,
        AudioChannelLayout::key_t::CH_LAYOUT_5POINT1_BACK},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_6POINT0, AudioChannelLayout::key_t::CH_LAYOUT_6POINT0},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HEXAGONAL, AudioChannelLayout::key_t::CH_LAYOUT_HEXAGONAL},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_3POINT1POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_3POINT1POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioChannelLayout::key_t::CH_LAYOUT_6POINT0_FRONT},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_6POINT1, AudioChannelLayout::key_t::CH_LAYOUT_6POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_6POINT1_BACK,
        AudioChannelLayout::key_t::CH_LAYOUT_6POINT1_BACK},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT,
        AudioChannelLayout::key_t::CH_LAYOUT_6POINT1_FRONT},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT0, AudioChannelLayout::key_t::CH_LAYOUT_7POINT0},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT,
        AudioChannelLayout::key_t::CH_LAYOUT_7POINT0_FRONT},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT1, AudioChannelLayout::key_t::CH_LAYOUT_7POINT1},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_OCTAGONAL, AudioChannelLayout::key_t::CH_LAYOUT_OCTAGONAL},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT1POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_5POINT1POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE,
        AudioChannelLayout::key_t::CH_LAYOUT_7POINT1_WIDE},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK,
        AudioChannelLayout::key_t::CH_LAYOUT_7POINT1_WIDE_BACK},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_N3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER2_ACN_N3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_SN3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER2_ACN_SN3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_FUMA,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER2_FUMA},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_5POINT1POINT4,
        AudioChannelLayout::key_t::CH_LAYOUT_5POINT1POINT4},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT1POINT2,
        AudioChannelLayout::key_t::CH_LAYOUT_7POINT1POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_7POINT1POINT4,
        AudioChannelLayout::key_t::CH_LAYOUT_7POINT1POINT4},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_10POINT2, AudioChannelLayout::key_t::CH_LAYOUT_10POINT2},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_9POINT1POINT4,
        AudioChannelLayout::key_t::CH_LAYOUT_9POINT1POINT4},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_9POINT1POINT6,
        AudioChannelLayout::key_t::CH_LAYOUT_9POINT1POINT6},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HEXADECAGONAL,
        AudioChannelLayout::key_t::CH_LAYOUT_HEXADECAGONAL},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER3_ACN_N3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER3_ACN_N3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER3_ACN_SN3D,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER3_ACN_SN3D},
    {OHOS::AudioStandard::AudioChannelLayout::CH_LAYOUT_HOA_ORDER3_FUMA,
        AudioChannelLayout::key_t::CH_LAYOUT_AMB_ORDER3_FUMA},

};

const std::map<std::string, int32_t> TaiheAudioEnum::deviceTypeMap = {
    {"NONE", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_NONE},
    {"INVALID", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_INVALID},
    {"EARPIECE", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE},
    {"SPEAKER", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER},
    {"WIRED_HEADSET", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADSET},
    {"WIRED_HEADPHONES", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADPHONES},
    {"BLUETOOTH_SCO", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO},
    {"BLUETOOTH_A2DP", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP},
    {"NEARLINK", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_NEARLINK},
    {"MIC", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_MIC},
    {"WAKEUP", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WAKEUP},
    {"USB_HEADSET", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_USB_HEADSET},
    {"DISPLAY_PORT", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DP},
    {"REMOTE_CAST", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_REMOTE_CAST},
    {"USB_DEVICE", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_USB_DEVICE},
    {"HDMI", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_HDMI},
    {"LINE_DIGITAL", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_LINE_DIGITAL},
    {"REMOTE_DAUDIO", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_REMOTE_DAUDIO},
    {"ACCESSORY", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_ACCESSORY},
    {"DEFAULT", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT},
    {"MAX", OHOS::AudioStandard::DeviceType::DEVICE_TYPE_MAX},
};

static const std::map<OHOS::AudioStandard::DeviceBlockStatus, DeviceBlockStatus> DEVICE_BLOCK_STATUS_TAIHE_MAP = {
    {OHOS::AudioStandard::DeviceBlockStatus::DEVICE_UNBLOCKED, DeviceBlockStatus::key_t::UNBLOCKED},
    {OHOS::AudioStandard::DeviceBlockStatus::DEVICE_BLOCKED, DeviceBlockStatus::key_t::BLOCKED},
};

static const std::map<OHOS::AudioStandard::AudioLoopbackStatus, AudioLoopbackStatus> AUDIO_LOOPBACK_STATUS_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioLoopbackStatus::LOOPBACK_UNAVAILABLE_DEVICE,
        AudioLoopbackStatus::key_t::UNAVAILABLE_DEVICE},
    {OHOS::AudioStandard::AudioLoopbackStatus::LOOPBACK_UNAVAILABLE_SCENE,
        AudioLoopbackStatus::key_t::UNAVAILABLE_SCENE},
    {OHOS::AudioStandard::AudioLoopbackStatus::LOOPBACK_AVAILABLE_IDLE,
        AudioLoopbackStatus::key_t::AVAILABLE_IDLE},
    {OHOS::AudioStandard::AudioLoopbackStatus::LOOPBACK_AVAILABLE_RUNNING,
        AudioLoopbackStatus::key_t::AVAILABLE_RUNNING},
};

static const std::map<OHOS::AudioStandard::AudioSessionStateChangeHint,
    AudioSessionStateChangeHint> AUDIO_SESSION_STATE_CHANGE_HINT_TAIHE_MAP = {
    {OHOS::AudioStandard::AudioSessionStateChangeHint::RESUME,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_RESUME},
    {OHOS::AudioStandard::AudioSessionStateChangeHint::PAUSE,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_PAUSE},
    {OHOS::AudioStandard::AudioSessionStateChangeHint::STOP,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_STOP},
    {OHOS::AudioStandard::AudioSessionStateChangeHint::TIME_OUT_STOP,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_TIME_OUT_STOP},
    {OHOS::AudioStandard::AudioSessionStateChangeHint::DUCK,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_DUCK},
    {OHOS::AudioStandard::AudioSessionStateChangeHint::UNDUCK,
        AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_UNDUCK},
};

static const std::map<OHOS::AudioStandard::OutputDeviceChangeRecommendedAction,
    OutputDeviceChangeRecommendedAction> OUTPUT_DEVICE_CHANGE_RECOMMENDED_ACTION_TAIHE_MAP = {
    {OHOS::AudioStandard::OutputDeviceChangeRecommendedAction::RECOMMEND_TO_CONTINUE,
        OutputDeviceChangeRecommendedAction::key_t::DEVICE_CHANGE_RECOMMEND_TO_CONTINUE},
    {OHOS::AudioStandard::OutputDeviceChangeRecommendedAction::RECOMMEND_TO_STOP,
        OutputDeviceChangeRecommendedAction::key_t::DEVICE_CHANGE_RECOMMEND_TO_STOP},
};

static const std::map<OHOS::AudioStandard::RenderTarget, RenderTarget> RENDER_TARGET_TAIHE_MAP = {
    {OHOS::AudioStandard::RenderTarget::NORMAL_PLAYBACK, RenderTarget::key_t::NORMAL_PLAYBACK},
    {OHOS::AudioStandard::RenderTarget::INJECT_TO_VOICE_COMMUNICATION_CAPTURE,
        RenderTarget::key_t::INJECT_TO_VOICE_COMMUNICATION_CAPTURE},
};

bool TaiheAudioEnum::IsLegalInputArgumentInterruptMode(int32_t interruptMode)
{
    bool result = false;
    switch (interruptMode) {
        case InterruptMode::SHARE_MODE:
        case InterruptMode::INDEPENDENT_MODE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode)
{
    bool result = false;
    switch (audioEffectMode) {
        case OHOS::AudioStandard::AudioEffectMode::EFFECT_NONE:
        case OHOS::AudioStandard::AudioEffectMode::EFFECT_DEFAULT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentChannelBlendMode(int32_t blendMode)
{
    bool result = false;
    switch (blendMode) {
        case OHOS::AudioStandard::ChannelBlendMode::MODE_DEFAULT:
        case OHOS::AudioStandard::ChannelBlendMode::MODE_BLEND_LR:
        case OHOS::AudioStandard::ChannelBlendMode::MODE_ALL_LEFT:
        case OHOS::AudioStandard::ChannelBlendMode::MODE_ALL_RIGHT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

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
        case TYPE_LIVE:
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
        case AudioJsVolumeType::NOTIFICATION:
        case AudioJsVolumeType::NAVIGATION:
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

bool TaiheAudioEnum::IsLegalRenderTarget(int32_t target)
{
    bool result = false;
    switch (target) {
        case TaiheAudioEnum::RenderTarget::NORMAL_PLAYBACK:
        case TaiheAudioEnum::RenderTarget::INJECT_TO_VOICE_COMMUNICATION_CAPTURE:
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
        case AudioJsVolumeType::NOTIFICATION:
            result = OHOS::AudioStandard::STREAM_RING;
            break;
        case AudioJsVolumeType::NAVIGATION:
            result = OHOS::AudioStandard::STREAM_MUSIC;
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

OHOS::AudioStandard::InterruptMode TaiheAudioEnum::GetNativeInterruptMode(int32_t interruptMode)
{
    OHOS::AudioStandard::InterruptMode result;
    switch (interruptMode) {
        case TaiheAudioEnum::InterruptMode::SHARE_MODE:
            result = OHOS::AudioStandard::InterruptMode::SHARE_MODE;
            break;
        case TaiheAudioEnum::InterruptMode::INDEPENDENT_MODE:
            result = OHOS::AudioStandard::InterruptMode::INDEPENDENT_MODE;
            break;
        default:
            result = OHOS::AudioStandard::InterruptMode::SHARE_MODE;
            AUDIO_ERR_LOG("Unknown interruptMode type, Set it to default SHARE_MODE!");
            break;
    }
    return result;
}

OHOS::AudioStandard::StreamUsage TaiheAudioEnum::GetNativeStreamUsage(int32_t streamUsage)
{
    OHOS::AudioStandard::StreamUsage result = OHOS::AudioStandard::STREAM_USAGE_UNKNOWN;

    switch (streamUsage) {
        case TaiheAudioEnum::USAGE_UNKNOW:
            result = OHOS::AudioStandard::STREAM_USAGE_UNKNOWN;
            break;
        case TaiheAudioEnum::USAGE_MEDIA:
            result = OHOS::AudioStandard::STREAM_USAGE_MEDIA;
            break;
        case TaiheAudioEnum::USAGE_VOICE_COMMUNICATION:
            result = OHOS::AudioStandard::STREAM_USAGE_VOICE_COMMUNICATION;
            break;
        case TaiheAudioEnum::USAGE_VOICE_ASSISTANT:
            result = OHOS::AudioStandard::STREAM_USAGE_VOICE_ASSISTANT;
            break;
        case TaiheAudioEnum::USAGE_ALARM:
            result = OHOS::AudioStandard::STREAM_USAGE_ALARM;
            break;
        case TaiheAudioEnum::USAGE_VOICE_MESSAGE:
            result = OHOS::AudioStandard::STREAM_USAGE_VOICE_MESSAGE;
            break;
        case TaiheAudioEnum::USAGE_RINGTONE:
            result = OHOS::AudioStandard::STREAM_USAGE_RINGTONE;
            break;
        case TaiheAudioEnum::USAGE_NOTIFICATION:
            result = OHOS::AudioStandard::STREAM_USAGE_NOTIFICATION;
            break;
        case TaiheAudioEnum::USAGE_ACCESSIBILITY:
            result = OHOS::AudioStandard::STREAM_USAGE_ACCESSIBILITY;
            break;
        case TaiheAudioEnum::USAGE_SYSTEM:
            result = OHOS::AudioStandard::STREAM_USAGE_SYSTEM;
            break;
        case TaiheAudioEnum::USAGE_MOVIE:
            result = OHOS::AudioStandard::STREAM_USAGE_MOVIE;
            break;
        default:
            result = GetNativeStreamUsageFir(streamUsage);
            AUDIO_DEBUG_LOG("Unknown streamUsage type: %{public}d", streamUsage);
            break;
    }

    return result;
}

OHOS::AudioStandard::StreamUsage TaiheAudioEnum::GetNativeStreamUsageFir(int32_t streamUsage)
{
    OHOS::AudioStandard::StreamUsage result = OHOS::AudioStandard::STREAM_USAGE_UNKNOWN;

    switch (streamUsage) {
        case TaiheAudioEnum::USAGE_GAME:
            result = OHOS::AudioStandard::STREAM_USAGE_GAME;
            break;
        case TaiheAudioEnum::USAGE_AUDIOBOOK:
            result = OHOS::AudioStandard::STREAM_USAGE_AUDIOBOOK;
            break;
        case TaiheAudioEnum::USAGE_NAVIGATION:
            result = OHOS::AudioStandard::STREAM_USAGE_NAVIGATION;
            break;
        case TaiheAudioEnum::USAGE_DTMF:
            result = OHOS::AudioStandard::STREAM_USAGE_DTMF;
            break;
        case TaiheAudioEnum::USAGE_ENFORCED_TONE:
            result = OHOS::AudioStandard::STREAM_USAGE_ENFORCED_TONE;
            break;
        case TaiheAudioEnum::USAGE_ULTRASONIC:
            result = OHOS::AudioStandard::STREAM_USAGE_ULTRASONIC;
            break;
        case TaiheAudioEnum::USAGE_VIDEO_COMMUNICATION:
            result = OHOS::AudioStandard::STREAM_USAGE_VIDEO_COMMUNICATION;
            break;
        case TaiheAudioEnum::USAGE_VOICE_CALL_ASSISTANT:
            result = OHOS::AudioStandard::STREAM_USAGE_VOICE_CALL_ASSISTANT;
            break;
        case TaiheAudioEnum::USAGE_MAX:
            result = OHOS::AudioStandard::STREAM_USAGE_MAX;
            break;
        default:
            result = OHOS::AudioStandard::STREAM_USAGE_INVALID;
            AUDIO_ERR_LOG("Unknown streamUsage type: %{public}d", streamUsage);
            break;
    }

    return result;
}

AudioVolumeType TaiheAudioEnum::GetJsAudioVolumeType(OHOS::AudioStandard::AudioStreamType volumeType)
{
    AudioVolumeType result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::MEDIA);
    switch (volumeType) {
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_CALL:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_CALL_ASSISTANT:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::VOICE_CALL);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_RING:
        case OHOS::AudioStandard::AudioStreamType::STREAM_DTMF:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::RINGTONE);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_MUSIC:
        case OHOS::AudioStandard::AudioStreamType::STREAM_MEDIA:
        case OHOS::AudioStandard::AudioStreamType::STREAM_MOVIE:
        case OHOS::AudioStandard::AudioStreamType::STREAM_GAME:
        case OHOS::AudioStandard::AudioStreamType::STREAM_SPEECH:
        case OHOS::AudioStandard::AudioStreamType::STREAM_NAVIGATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_CAMCORDER:
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_MESSAGE:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::MEDIA);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ALARM:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::ALARM);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ACCESSIBILITY:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::ACCESSIBILITY);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_VOICE_ASSISTANT:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::VOICE_ASSISTANT);
            break;
        case OHOS::AudioStandard::AudioStreamType::STREAM_ULTRASONIC:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::ULTRASONIC);
            break;
        default:
            result = GetJsAudioVolumeTypeMore(volumeType);
            break;
    }
    return result;
}

AudioVolumeType TaiheAudioEnum::GetJsAudioVolumeTypeMore(OHOS::AudioStandard::AudioStreamType volumeType)
{
    AudioVolumeType result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::MEDIA);
    switch (volumeType) {
        case OHOS::AudioStandard::AudioStreamType::STREAM_SYSTEM:
        case OHOS::AudioStandard::AudioStreamType::STREAM_NOTIFICATION:
        case OHOS::AudioStandard::AudioStreamType::STREAM_SYSTEM_ENFORCED:
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
            result = (OHOS::system::GetBoolParameter("const.multimedia.audio.fwk_ec.enable", 0))?
                TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::SYSTEM) :
                    TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::RINGTONE);
#else
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::RINGTONE);
#endif
            break;
        default:
            result = TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::MEDIA);
            break;
    }
    return result;
}

StreamUsage TaiheAudioEnum::GetJsStreamUsage(OHOS::AudioStandard::StreamUsage streamUsage)
{
    StreamUsage result = TaiheAudioEnum::ToTaiheStreamUsage(OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN);
    switch (streamUsage) {
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MUSIC:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_ASSISTANT:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ALARM:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_MESSAGE:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_RINGTONE:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NOTIFICATION:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ACCESSIBILITY:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_SYSTEM:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_MOVIE:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        default:
            result = GetJsStreamUsageFir(streamUsage);
            break;
    }
    return result;
}

StreamUsage TaiheAudioEnum::GetJsStreamUsageFir(OHOS::AudioStandard::StreamUsage streamUsage)
{
    StreamUsage result = TaiheAudioEnum::ToTaiheStreamUsage(OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN);
    switch (streamUsage) {
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_GAME:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_AUDIOBOOK:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_NAVIGATION:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_DTMF:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ENFORCED_TONE:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_ULTRASONIC:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VIDEO_COMMUNICATION:
            result = TaiheAudioEnum::ToTaiheStreamUsage(streamUsage);
            break;
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_RANGING:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_RINGTONE:
        case OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT:
            result = TaiheAudioEnum::ToTaiheStreamUsage(
                OHOS::AudioStandard::StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT);
            break;
        default:
            result = TaiheAudioEnum::ToTaiheStreamUsage(OHOS::AudioStandard::StreamUsage::STREAM_USAGE_UNKNOWN);
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentCommunicationDeviceType(int32_t communicationDeviceType)
{
    bool result = false;
    switch (communicationDeviceType) {
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER:
            result = true;
            break;
        default:
            result = false;
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

bool TaiheAudioEnum::IsLegalInputArgumentDefaultOutputDeviceType(int32_t deviceType)
{
    bool result = false;
    switch (deviceType) {
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE:
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER:
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentActiveDeviceType(int32_t activeDeviceFlag)
{
    bool result = false;
    switch (activeDeviceFlag) {
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER:
        case OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentDeviceType(int32_t deviceType)
{
    for (const auto &iter : TaiheAudioEnum::deviceTypeMap) {
        if (deviceType == iter.second && deviceType != OHOS::AudioStandard::DeviceType::DEVICE_TYPE_NONE &&
            deviceType != OHOS::AudioStandard::DeviceType::DEVICE_TYPE_INVALID &&
            deviceType != OHOS::AudioStandard::DeviceType::DEVICE_TYPE_MAX) {
            return true;
        }
    }
    return false;
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
        case OHOS::AudioStandard::SourceType::SOURCE_TYPE_LIVE:
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

bool TaiheAudioEnum::IsLegalInputArgumentSpatializationSceneType(int32_t spatializationSceneType)
{
    bool result = false;
    switch (spatializationSceneType) {
        case OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT:
        case OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MUSIC:
        case OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MOVIE:
        case OHOS::AudioStandard::AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_AUDIOBOOK:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentSessionScene(int32_t scene)
{
    bool result = false;
    switch (scene) {
        case static_cast<int32_t>(OHOS::AudioStandard::AudioSessionScene::MEDIA):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioSessionScene::GAME):
        case static_cast<int32_t>(OHOS::AudioStandard::AudioSessionScene::VOICE_COMMUNICATION):
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool TaiheAudioEnum::IsLegalInputArgumentAudioLoopbackMode(int32_t inputMode)
{
    bool result = false;
    switch (inputMode) {
        case AudioLoopbackModeTaihe::LOOPBACK_MODE_HARDWARE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

OHOS::AudioStandard::AudioScene TaiheAudioEnum::GetJsAudioScene(OHOS::AudioStandard::AudioScene audioScene)
{
    OHOS::AudioStandard::AudioScene newAudioScene = OHOS::AudioStandard::AudioScene::AUDIO_SCENE_DEFAULT;
    switch (audioScene) {
        case OHOS::AudioStandard::AudioScene::AUDIO_SCENE_DEFAULT:
        case OHOS::AudioStandard::AudioScene::AUDIO_SCENE_RINGING:
        case OHOS::AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CALL:
        case OHOS::AudioStandard::AudioScene::AUDIO_SCENE_PHONE_CHAT:
            newAudioScene = audioScene;
            break;
        case OHOS::AudioStandard::AudioScene::AUDIO_SCENE_VOICE_RINGING:
            newAudioScene = OHOS::AudioStandard::AudioScene::AUDIO_SCENE_RINGING;
            break;
        default:
            newAudioScene = OHOS::AudioStandard::AudioScene::AUDIO_SCENE_DEFAULT;
            AUDIO_ERR_LOG("Unknown audio scene, Set it to default AUDIO_SCENE_DEFAULT!");
            break;
    }
    return newAudioScene;
}

ConnectType TaiheAudioEnum::ToTaiheConnectType(OHOS::AudioStandard::ConnectType type)
{
    auto iter = CONNECT_TYPE_TAIHE_MAP.find(type);
    if (iter == CONNECT_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceRole invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheConnectType fail");
        return ConnectType::key_t::CONNECT_TYPE_LOCAL;
    }
    return iter->second;
}

DeviceRole TaiheAudioEnum::ToTaiheDeviceRole(OHOS::AudioStandard::DeviceRole type)
{
    auto iter = DEVICE_ROLE_TAIHE_MAP.find(type);
    if (iter == DEVICE_ROLE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceRole invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheDeviceRole fail");
        return DeviceRole::key_t::INPUT_DEVICE;
    }
    return iter->second;
}

DeviceType TaiheAudioEnum::ToTaiheDeviceType(OHOS::AudioStandard::DeviceType type)
{
    auto iter = DEVICE_TYPE_TAIHE_MAP.find(type);
    if (iter == DEVICE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceType invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheDeviceType fail");
        return DeviceType::key_t::INVALID;
    }
    return iter->second;
}

AudioEncodingType TaiheAudioEnum::ToTaiheAudioEncodingType(OHOS::AudioStandard::AudioEncodingType type)
{
    auto iter = AUDIO_ENCODING_TYPE_TAIHE_MAP.find(type);
    if (iter == AUDIO_ENCODING_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioEncodingType invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioEncodingType fail");
        return AudioEncodingType::key_t::ENCODING_TYPE_INVALID;
    }
    return iter->second;
}

AudioState TaiheAudioEnum::ToTaiheAudioState(OHOS::AudioStandard::RendererState state)
{
    auto iter = RENDERER_STATE_TAIHE_MAP.find(state);
    if (iter == RENDERER_STATE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioState(Renderer) invalid state: %{public}d", static_cast<int32_t>(state));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioState fail");
        return AudioState::key_t::STATE_INVALID;
    }
    return iter->second;
}

AudioState TaiheAudioEnum::ToTaiheAudioState(OHOS::AudioStandard::CapturerState state)
{
    auto iter = CAPTURER_STATE_TAIHE_MAP.find(state);
    if (iter == CAPTURER_STATE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioState(Capturer) invalid state: %{public}d", static_cast<int32_t>(state));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioState fail");
        return AudioState::key_t::STATE_INVALID;
    }
    return iter->second;
}

StreamUsage TaiheAudioEnum::ToTaiheStreamUsage(OHOS::AudioStandard::StreamUsage usage)
{
    auto iter = STREAM_USAGE_TAIHE_MAP.find(usage);
    if (iter == STREAM_USAGE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheStreamUsage invalid usage: %{public}d", static_cast<int32_t>(usage));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheStreamUsage fail");
        return StreamUsage::key_t::STREAM_USAGE_UNKNOWN;
    }
    return iter->second;
}

SourceType TaiheAudioEnum::ToTaiheSourceType(OHOS::AudioStandard::SourceType type)
{
    auto iter = SOURCE_TYPE_TAIHE_MAP.find(type);
    if (iter == SOURCE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheSourceType invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheSourceType fail");
        return SourceType::key_t::SOURCE_TYPE_INVALID;
    }
    return iter->second;
}

EffectFlag TaiheAudioEnum::ToTaiheEffectFlag(OHOS::AudioStandard::EffectFlag flag)
{
    auto iter = EFFECT_FLAG_TAIHE_MAP.find(flag);
    if (iter == EFFECT_FLAG_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheEffectFlag invalid flag: %{public}d", static_cast<int32_t>(flag));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheEffectFlag fail");
        return EffectFlag::key_t::RENDER_EFFECT_FLAG;
    }
    return iter->second;
}

AudioScene TaiheAudioEnum::ToTaiheAudioScene(OHOS::AudioStandard::AudioScene scene)
{
    auto iter = AUDIO_SCENE_TAIHE_MAP.find(scene);
    if (iter == AUDIO_SCENE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioScene invalid scene: %{public}d", static_cast<int32_t>(scene));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioScene fail");
        return AudioScene::key_t::AUDIO_SCENE_DEFAULT;
    }
    return iter->second;
}

InterruptType TaiheAudioEnum::ToTaiheInterruptType(OHOS::AudioStandard::InterruptType type)
{
    auto iter = INTERRUPT_TYPE_TAIHE_MAP.find(type);
    if (iter == INTERRUPT_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheInterruptType invalid type: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheInterruptType fail");
        return InterruptType::key_t::INTERRUPT_TYPE_BEGIN;
    }
    return iter->second;
}

InterruptHint TaiheAudioEnum::ToTaiheInterruptHint(OHOS::AudioStandard::InterruptHint hint)
{
    auto iter = INTERRUPT_HINT_TAIHE_MAP.find(hint);
    if (iter == INTERRUPT_HINT_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheInterruptHint invalid hint: %{public}d", static_cast<int32_t>(hint));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheInterruptHint fail");
        return InterruptHint::key_t::INTERRUPT_HINT_NONE;
    }
    return iter->second;
}

AudioVolumeMode TaiheAudioEnum::ToTaiheAudioVolumeMode(OHOS::AudioStandard::AudioVolumeMode mode)
{
    auto iter = AUDIO_VOLUME_MODE_TAIHE_MAP.find(mode);
    if (iter == AUDIO_VOLUME_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioVolumeMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioVolumeMode fail");
        return AudioVolumeMode::key_t::SYSTEM_GLOBAL;
    }
    return iter->second;
}

DeviceChangeType TaiheAudioEnum::ToTaiheDeviceChangeType(OHOS::AudioStandard::DeviceChangeType type)
{
    auto iter = DEVICE_CHANGE_TYPE_TAIHE_MAP.find(type);
    if (iter == DEVICE_CHANGE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioVolumeMode invalid mode: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheDeviceChangeType fail");
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
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheSessionDeactiveReason fail");
        return AudioSessionDeactivatedReason::key_t::DEACTIVATED_LOWER_PRIORITY;
    }
    return iter->second;
}

ohos::multimedia::audio::AsrNoiseSuppressionMode TaiheAudioEnum::ToTaiheAsrNoiseSuppressionMode(
    ::AsrNoiseSuppressionMode mode)
{
    auto iter = ASR_NOISE_SUPPRESSION_MODE_TAIHE_MAP.find(mode);
    if (iter == ASR_NOISE_SUPPRESSION_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAsrNoiseSuppressionMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAsrNoiseSuppressionMode fail");
        return ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::BYPASS;
    }
    return iter->second;
}

ohos::multimedia::audio::AsrAecMode TaiheAudioEnum::ToTaiheAsrAecMode(::AsrAecMode mode)
{
    auto iter = ASR_AEC_MODE_TAIHE_MAP.find(mode);
    if (iter == ASR_AEC_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAsrAecMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAsrAecMode fail");
        return ohos::multimedia::audio::AsrAecMode::key_t::BYPASS;
    }
    return iter->second;
}

ohos::multimedia::audio::AsrWhisperDetectionMode TaiheAudioEnum::ToTaiheAsrWhisperDetectionMode(
    ::AsrWhisperDetectionMode mode)
{
    auto iter = ASR_WHISPER_DETECTION_MODE_TAIHE_MAP.find(mode);
    if (iter == ASR_WHISPER_DETECTION_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAsrWhisperDetectionMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAsrWhisperDetectionMode fail");
        return ohos::multimedia::audio::AsrWhisperDetectionMode::key_t::BYPASS;
    }
    return iter->second;
}

InterruptForceType TaiheAudioEnum::ToTaiheInterruptForceType(OHOS::AudioStandard::InterruptForceType type)
{
    auto iter = INTERRUPT_FORCE_TYPE_TAIHE_MAP.find(type);
    if (iter == INTERRUPT_FORCE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheInterruptForceType invalid mode: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheInterruptForceType fail");
        return InterruptForceType::key_t::INTERRUPT_FORCE;
    }
    return iter->second;
}

AudioSpatializationSceneType TaiheAudioEnum::ToTaiheAudioSpatializationSceneType(
    OHOS::AudioStandard::AudioSpatializationSceneType type)
{
    auto iter = AUDIO_SPATIALIZATION_SCENE_TYPE_TAIHE_MAP.find(type);
    if (iter == AUDIO_SPATIALIZATION_SCENE_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioSpatializationSceneType invalid mode: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioSpatializationSceneType fail");
        return AudioSpatializationSceneType::key_t::DEFAULT;
    }
    return iter->second;
}

AudioVolumeType TaiheAudioEnum::ToTaiheAudioVolumeType(TaiheAudioEnum::AudioJsVolumeType type)
{
    auto iter = AUDIO_VOLUME_TYPE_TAIHE_MAP.find(type);
    if (iter == AUDIO_VOLUME_TYPE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioVolumeType invalid mode: %{public}d", static_cast<int32_t>(type));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioVolumeType fail");
        return AudioVolumeType::key_t::VOICE_CALL;
    }
    return iter->second;
}

ohos::multimedia::audio::AudioRingMode TaiheAudioEnum::ToTaiheAudioRingMode(OHOS::AudioStandard::AudioRingerMode mode)
{
    auto iter = AUDIO_RING_MODE_TAIHE_MAP.find(mode);
    if (iter == AUDIO_RING_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioRingMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioRingMode fail");
        return ohos::multimedia::audio::AudioRingMode::key_t::RINGER_MODE_SILENT;
    }
    return iter->second;
}

AudioEffectMode TaiheAudioEnum::ToTaiheAudioEffectMode(OHOS::AudioStandard::AudioEffectMode mode)
{
    auto iter = AUDIO_EFFECT_MODE_TAIHE_MAP.find(mode);
    if (iter == AUDIO_EFFECT_MODE_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioEffectMode invalid mode: %{public}d", static_cast<int32_t>(mode));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioEffectMode fail");
        return AudioEffectMode::key_t::EFFECT_NONE;
    }
    return iter->second;
}

AudioStreamDeviceChangeReason TaiheAudioEnum::ToTaiheAudioStreamDeviceChangeReason(
    OHOS::AudioStandard::AudioStreamDeviceChangeReason reason)
{
    auto iter = AUDIO_STREAM_DEVICE_CHANGE_REASON_TAIHE_MAP.find(reason);
    if (iter == AUDIO_STREAM_DEVICE_CHANGE_REASON_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioStreamDeviceChangeReason invalid mode: %{public}d",
            static_cast<int32_t>(reason));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioStreamDeviceChangeReason fail");
        return AudioStreamDeviceChangeReason::key_t::REASON_UNKNOWN;
    }
    return iter->second;
}

AudioChannelLayout TaiheAudioEnum::ToTaiheAudioChannelLayout(OHOS::AudioStandard::AudioChannelLayout layout)
{
    auto iter = AUDIO_CHANNEL_LAYOUT_TAIHE_MAP.find(layout);
    if (iter == AUDIO_CHANNEL_LAYOUT_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioChannelLayout invalid mode: %{public}d", static_cast<int32_t>(layout));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioChannelLayout fail");
        return AudioChannelLayout::key_t::CH_LAYOUT_UNKNOWN;
    }
    return iter->second;
}

DeviceBlockStatus TaiheAudioEnum::ToTaiheDeviceBlockStatus(OHOS::AudioStandard::DeviceBlockStatus status)
{
    auto iter = DEVICE_BLOCK_STATUS_TAIHE_MAP.find(status);
    if (iter == DEVICE_BLOCK_STATUS_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheDeviceBlockStatus invalid mode: %{public}d", static_cast<int32_t>(status));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheDeviceBlockStatus fail");
        return DeviceBlockStatus::key_t::UNBLOCKED;
    }
    return iter->second;
}

AudioSessionStateChangeHint TaiheAudioEnum::ToTaiheAudioSessionStateChangeHint(
    OHOS::AudioStandard::AudioSessionStateChangeHint hint)
{
    auto iter = AUDIO_SESSION_STATE_CHANGE_HINT_TAIHE_MAP.find(hint);
    if (iter == AUDIO_SESSION_STATE_CHANGE_HINT_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioSessionStateChangeHint invalid mode: %{public}d", static_cast<int32_t>(hint));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioSessionStateChangeHint fail");
        return AudioSessionStateChangeHint::key_t::AUDIO_SESSION_STATE_CHANGE_HINT_RESUME;
    }
    return iter->second;
}

OutputDeviceChangeRecommendedAction TaiheAudioEnum::ToTaiheOutputDeviceChangeRecommendedAction(
    OHOS::AudioStandard::OutputDeviceChangeRecommendedAction action)
{
    auto iter = OUTPUT_DEVICE_CHANGE_RECOMMENDED_ACTION_TAIHE_MAP.find(action);
    if (iter == OUTPUT_DEVICE_CHANGE_RECOMMENDED_ACTION_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheOutputDeviceChangeRecommendedAction invalid mode: %{public}d",
            static_cast<int32_t>(action));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheOutputDeviceChangeRecommendedAction fail");
        return OutputDeviceChangeRecommendedAction::key_t::DEVICE_CHANGE_RECOMMEND_TO_CONTINUE;
    }
    return iter->second;
}

AudioLoopbackStatus TaiheAudioEnum::ToTaiheAudioLoopbackStatus(OHOS::AudioStandard::AudioLoopbackStatus status)
{
    auto iter = AUDIO_LOOPBACK_STATUS_TAIHE_MAP.find(status);
    if (iter == AUDIO_LOOPBACK_STATUS_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheAudioLoopbackStatus invalid mode: %{public}d", static_cast<int32_t>(status));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheAudioLoopbackStatus fail");
        return AudioLoopbackStatus::key_t::UNAVAILABLE_DEVICE;
    }
    return iter->second;
}

ohos::multimedia::audio::RenderTarget TaiheAudioEnum::ToTaiheRenderTarget(OHOS::AudioStandard::RenderTarget target)
{
    auto iter = RENDER_TARGET_TAIHE_MAP.find(target);
    if (iter == RENDER_TARGET_TAIHE_MAP.end()) {
        AUDIO_WARNING_LOG("ToTaiheRenderTarget invalid mode: %{public}d", static_cast<int32_t>(target));
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheRenderTarget fail");
        return ohos::multimedia::audio::RenderTarget::key_t::NORMAL_PLAYBACK;
    }
    return iter->second;
}
} // namespace ANI::Audio
