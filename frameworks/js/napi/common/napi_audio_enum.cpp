/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioEnum"
#endif

#include "napi_audio_enum.h"
#include "audio_renderer.h"
#include "audio_errors.h"
#include "audio_stream_manager.h"
#include "audio_interrupt_info.h"
#include "audio_device_info.h"
#include "napi_param_utils.h"
#include "audio_asr.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {

napi_ref NapiAudioEnum::sConstructor_ = nullptr;
unique_ptr<AudioParameters> NapiAudioEnum::sAudioParameters_ = nullptr;

napi_ref NapiAudioEnum::audioChannel_ = nullptr;
napi_ref NapiAudioEnum::samplingRate_ = nullptr;
napi_ref NapiAudioEnum::encodingType_ = nullptr;
napi_ref NapiAudioEnum::contentType_ = nullptr;
napi_ref NapiAudioEnum::streamUsage_ = nullptr;
napi_ref NapiAudioEnum::deviceRole_ = nullptr;
napi_ref NapiAudioEnum::deviceType_ = nullptr;
napi_ref NapiAudioEnum::sourceType_ = nullptr;
napi_ref NapiAudioEnum::volumeAdjustType_ = nullptr;
napi_ref NapiAudioEnum::channelBlendMode_ = nullptr;
napi_ref NapiAudioEnum::audioRendererRate_ = nullptr;
napi_ref NapiAudioEnum::interruptEventType_ = nullptr;
napi_ref NapiAudioEnum::interruptForceType_ = nullptr;
napi_ref NapiAudioEnum::interruptHintType_ = nullptr;
napi_ref NapiAudioEnum::audioState_ = nullptr;
napi_ref NapiAudioEnum::sampleFormat_ = nullptr;
napi_ref NapiAudioEnum::audioEffectMode_ = nullptr;
napi_ref NapiAudioEnum::audioPrivacyType_ = nullptr;
napi_ref NapiAudioEnum::audioVolumeTypeRef_ = nullptr;
napi_ref NapiAudioEnum::deviceFlagRef_ = nullptr;
napi_ref NapiAudioEnum::activeDeviceTypeRef_ = nullptr;
napi_ref NapiAudioEnum::audioRingModeRef_ = nullptr;
napi_ref NapiAudioEnum::deviceChangeType_ = nullptr;
napi_ref NapiAudioEnum::interruptActionType_ = nullptr;
napi_ref NapiAudioEnum::audioScene_ = nullptr;
napi_ref NapiAudioEnum::interruptMode_ = nullptr;
napi_ref NapiAudioEnum::focusType_ = nullptr;
napi_ref NapiAudioEnum::connectTypeRef_ = nullptr;
napi_ref NapiAudioEnum::audioErrors_ = nullptr;
napi_ref NapiAudioEnum::communicationDeviceType_ = nullptr;
napi_ref NapiAudioEnum::interruptRequestType_ = nullptr;
napi_ref NapiAudioEnum::interruptRequestResultType_ = nullptr;
napi_ref NapiAudioEnum::toneType_ = nullptr;
napi_ref NapiAudioEnum::audioDviceUsage_ = nullptr;
napi_ref NapiAudioEnum::audioSpatialDeivceType_ = nullptr;
napi_ref NapiAudioEnum::audioChannelLayout_ = nullptr;
napi_ref NapiAudioEnum::audioStreamDeviceChangeReason_ = nullptr;
napi_ref NapiAudioEnum::spatializationSceneType_ = nullptr;
napi_ref NapiAudioEnum::asrNoiseSuppressionMode_ = nullptr;
napi_ref NapiAudioEnum::asrAecMode_ = nullptr;
napi_ref NapiAudioEnum::asrWhisperDetectionMode_ = nullptr;
napi_ref NapiAudioEnum::asrVoiceControlMode_ = nullptr;
napi_ref NapiAudioEnum::asrVoiceMuteMode_ = nullptr;
napi_ref NapiAudioEnum::audioDataCallbackResult_ = nullptr;
napi_ref NapiAudioEnum::concurrencyMode_ = nullptr;
napi_ref NapiAudioEnum::reason_ = nullptr;
napi_ref NapiAudioEnum::policyType_ = nullptr;

static const std::string NAPI_AUDIO_ENUM_CLASS_NAME = "AudioEnum";

const std::map<std::string, int32_t> NapiAudioEnum::audioChannelMap = {
    {"CHANNEL_1", MONO},
    {"CHANNEL_2", STEREO},
    {"CHANNEL_3", CHANNEL_3},
    {"CHANNEL_4", CHANNEL_4},
    {"CHANNEL_5", CHANNEL_5},
    {"CHANNEL_6", CHANNEL_6},
    {"CHANNEL_7", CHANNEL_7},
    {"CHANNEL_8", CHANNEL_8},
    {"CHANNEL_9", CHANNEL_9},
    {"CHANNEL_10", CHANNEL_10},
    {"CHANNEL_12", CHANNEL_12},
    {"CHANNEL_14", CHANNEL_14},
    {"CHANNEL_16", CHANNEL_16},
};

const std::map<std::string, int32_t> NapiAudioEnum::samplingRateMap = {
    {"SAMPLE_RATE_8000", SAMPLE_RATE_8000},
    {"SAMPLE_RATE_11025", SAMPLE_RATE_11025},
    {"SAMPLE_RATE_12000", SAMPLE_RATE_12000},
    {"SAMPLE_RATE_16000", SAMPLE_RATE_16000},
    {"SAMPLE_RATE_22050", SAMPLE_RATE_22050},
    {"SAMPLE_RATE_24000", SAMPLE_RATE_24000},
    {"SAMPLE_RATE_32000", SAMPLE_RATE_32000},
    {"SAMPLE_RATE_44100", SAMPLE_RATE_44100},
    {"SAMPLE_RATE_48000", SAMPLE_RATE_48000},
    {"SAMPLE_RATE_64000", SAMPLE_RATE_64000},
    {"SAMPLE_RATE_88200", SAMPLE_RATE_88200},
    {"SAMPLE_RATE_96000", SAMPLE_RATE_96000},
    {"SAMPLE_RATE_176400", SAMPLE_RATE_176400},
    {"SAMPLE_RATE_192000", SAMPLE_RATE_192000},
};

const std::map<std::string, int32_t> NapiAudioEnum::encodingTypeMap = {
    {"ENCODING_TYPE_INVALID", ENCODING_INVALID},
    {"ENCODING_TYPE_RAW", ENCODING_PCM}
};

const std::map<std::string, int32_t> NapiAudioEnum::contentTypeMap = {
    {"CONTENT_TYPE_UNKNOWN", CONTENT_TYPE_UNKNOWN},
    {"CONTENT_TYPE_SPEECH", CONTENT_TYPE_SPEECH},
    {"CONTENT_TYPE_MUSIC", CONTENT_TYPE_MUSIC},
    {"CONTENT_TYPE_MOVIE", CONTENT_TYPE_MOVIE},
    {"CONTENT_TYPE_SONIFICATION", CONTENT_TYPE_SONIFICATION},
    {"CONTENT_TYPE_RINGTONE", CONTENT_TYPE_RINGTONE},
    {"CONTENT_TYPE_ULTRASONIC", CONTENT_TYPE_ULTRASONIC}
};

const std::map<std::string, int32_t> NapiAudioEnum::streamUsageMap = {
    {"STREAM_USAGE_UNKNOWN", STREAM_USAGE_UNKNOWN},
    {"STREAM_USAGE_MEDIA", STREAM_USAGE_MEDIA},
    {"STREAM_USAGE_MUSIC", STREAM_USAGE_MUSIC},
    {"STREAM_USAGE_VOICE_COMMUNICATION", STREAM_USAGE_VOICE_COMMUNICATION},
    {"STREAM_USAGE_VOICE_ASSISTANT", STREAM_USAGE_VOICE_ASSISTANT},
    {"STREAM_USAGE_ALARM", STREAM_USAGE_ALARM},
    {"STREAM_USAGE_VOICE_MESSAGE", STREAM_USAGE_VOICE_MESSAGE},
    {"STREAM_USAGE_VOICE_CALL_ASSISTANT", STREAM_USAGE_VOICE_CALL_ASSISTANT},
    {"STREAM_USAGE_NOTIFICATION_RINGTONE", STREAM_USAGE_NOTIFICATION_RINGTONE},
    {"STREAM_USAGE_RINGTONE", STREAM_USAGE_RINGTONE},
    {"STREAM_USAGE_NOTIFICATION", STREAM_USAGE_NOTIFICATION},
    {"STREAM_USAGE_ACCESSIBILITY", STREAM_USAGE_ACCESSIBILITY},
    {"STREAM_USAGE_SYSTEM", STREAM_USAGE_SYSTEM},
    {"STREAM_USAGE_MOVIE", STREAM_USAGE_MOVIE},
    {"STREAM_USAGE_GAME", STREAM_USAGE_GAME},
    {"STREAM_USAGE_AUDIOBOOK", STREAM_USAGE_AUDIOBOOK},
    {"STREAM_USAGE_NAVIGATION", STREAM_USAGE_NAVIGATION},
    {"STREAM_USAGE_DTMF", STREAM_USAGE_DTMF},
    {"STREAM_USAGE_ENFORCED_TONE", STREAM_USAGE_ENFORCED_TONE},
    {"STREAM_USAGE_ULTRASONIC", STREAM_USAGE_ULTRASONIC},
    {"STREAM_USAGE_VIDEO_COMMUNICATION", STREAM_USAGE_VIDEO_COMMUNICATION}
};

const std::map<std::string, int32_t> NapiAudioEnum::deviceRoleMap = {
    {"DEVICE_ROLE_NONE", DEVICE_ROLE_NONE},
    {"INPUT_DEVICE", INPUT_DEVICE},
    {"OUTPUT_DEVICE", OUTPUT_DEVICE},
    {"DEVICE_ROLE_MAX", DEVICE_ROLE_MAX}
};

const std::map<std::string, int32_t> NapiAudioEnum::deviceTypeMap = {
    {"NONE", DEVICE_TYPE_NONE},
    {"INVALID", DEVICE_TYPE_INVALID},
    {"EARPIECE", DEVICE_TYPE_EARPIECE},
    {"SPEAKER", DEVICE_TYPE_SPEAKER},
    {"WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"MIC", DEVICE_TYPE_MIC},
    {"WAKEUP", DEVICE_TYPE_WAKEUP},
    {"USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DISPLAY_PORT", DEVICE_TYPE_DP},
    {"REMOTE_CAST", DEVICE_TYPE_REMOTE_CAST},
    {"DEFAULT", DEVICE_TYPE_DEFAULT},
    {"MAX", DEVICE_TYPE_MAX},
};

const std::map<std::string, int32_t> NapiAudioEnum::sourceTypeMap = {
    {"SOURCE_TYPE_INVALID", SOURCE_TYPE_INVALID},
    {"SOURCE_TYPE_MIC", SOURCE_TYPE_MIC},
    {"SOURCE_TYPE_WAKEUP", SOURCE_TYPE_WAKEUP},
    {"SOURCE_TYPE_VOICE_RECOGNITION", SOURCE_TYPE_VOICE_RECOGNITION},
    {"SOURCE_TYPE_PLAYBACK_CAPTURE", SOURCE_TYPE_PLAYBACK_CAPTURE},
    {"SOURCE_TYPE_VOICE_COMMUNICATION", SOURCE_TYPE_VOICE_COMMUNICATION},
    {"SOURCE_TYPE_VOICE_CALL", SOURCE_TYPE_VOICE_CALL},
    {"SOURCE_TYPE_VOICE_MESSAGE", SOURCE_TYPE_VOICE_MESSAGE},
    {"SOURCE_TYPE_REMOTE_CAST", SOURCE_TYPE_REMOTE_CAST},
    {"SOURCE_TYPE_VOICE_TRANSCRIPTION", SOURCE_TYPE_VOICE_TRANSCRIPTION}
};

const std::map<std::string, int32_t> NapiAudioEnum::volumeAdjustTypeMap = {
    {"VOLUME_UP", VOLUME_UP},
    {"VOLUME_DOWN", VOLUME_DOWN},
};

const std::map<std::string, int32_t> NapiAudioEnum::channelBlendModeMap = {
    {"MODE_DEFAULT", MODE_DEFAULT},
    {"MODE_BLEND_LR", MODE_BLEND_LR},
    {"MODE_ALL_LEFT", MODE_ALL_LEFT},
    {"MODE_ALL_RIGHT", MODE_ALL_RIGHT},
};

const std::map<std::string, int32_t> NapiAudioEnum::rendererRateMap = {
    {"RENDER_RATE_NORMAL", RENDER_RATE_NORMAL},
    {"RENDER_RATE_DOUBLE", RENDER_RATE_DOUBLE},
    {"RENDER_RATE_HALF", RENDER_RATE_HALF}
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptEventTypeMap = {
    {"INTERRUPT_TYPE_BEGIN", INTERRUPT_TYPE_BEGIN},
    {"INTERRUPT_TYPE_END", INTERRUPT_TYPE_END}
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptForceTypeMap = {
    {"INTERRUPT_FORCE", INTERRUPT_FORCE},
    {"INTERRUPT_SHARE", INTERRUPT_SHARE},
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptHintTypeMap = {
    {"INTERRUPT_HINT_NONE", INTERRUPT_HINT_NONE},
    {"INTERRUPT_HINT_PAUSE", INTERRUPT_HINT_PAUSE},
    {"INTERRUPT_HINT_RESUME", INTERRUPT_HINT_RESUME},
    {"INTERRUPT_HINT_STOP", INTERRUPT_HINT_STOP},
    {"INTERRUPT_HINT_DUCK", INTERRUPT_HINT_DUCK},
    {"INTERRUPT_HINT_UNDUCK", INTERRUPT_HINT_UNDUCK}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioSampleFormatMap = {
    {"SAMPLE_FORMAT_INVALID", NapiAudioEnum::SAMPLE_FORMAT_INVALID},
    {"SAMPLE_FORMAT_U8", NapiAudioEnum::SAMPLE_FORMAT_U8},
    {"SAMPLE_FORMAT_S16LE", NapiAudioEnum::SAMPLE_FORMAT_S16LE},
    {"SAMPLE_FORMAT_S24LE", NapiAudioEnum::SAMPLE_FORMAT_S24LE},
    {"SAMPLE_FORMAT_S32LE", NapiAudioEnum::SAMPLE_FORMAT_S32LE},
    {"SAMPLE_FORMAT_F32LE", NapiAudioEnum::SAMPLE_FORMAT_F32LE}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioStateMap = {
    {"STATE_INVALID", RENDERER_INVALID},
    {"STATE_NEW", RENDERER_NEW},
    {"STATE_PREPARED", RENDERER_PREPARED},
    {"STATE_RUNNING", RENDERER_RUNNING},
    {"STATE_STOPPED", RENDERER_STOPPED},
    {"STATE_RELEASED", RENDERER_RELEASED},
    {"STATE_PAUSED", RENDERER_PAUSED}
};

const std::map<std::string, int32_t> NapiAudioEnum::effectModeMap = {
    {"EFFECT_NONE", EFFECT_NONE},
    {"EFFECT_DEFAULT", EFFECT_DEFAULT}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioPrivacyTypeMap = {
    {"PRIVACY_TYPE_PUBLIC", PRIVACY_TYPE_PUBLIC},
    {"PRIVACY_TYPE_PRIVATE", PRIVACY_TYPE_PRIVATE}
};

const std::map<std::string, int32_t> NapiAudioEnum::deviceChangeTypeMap = {
    {"CONNECT", CONNECT},
    {"DISCONNECT", DISCONNECT}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioSceneMap = {
    {"AUDIO_SCENE_DEFAULT", AUDIO_SCENE_DEFAULT},
    {"AUDIO_SCENE_RINGING", AUDIO_SCENE_RINGING},
    {"AUDIO_SCENE_PHONE_CALL", AUDIO_SCENE_PHONE_CALL},
    {"AUDIO_SCENE_VOICE_CHAT", AUDIO_SCENE_PHONE_CHAT}
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptActionTypeMap = {
    {"TYPE_ACTIVATED", TYPE_ACTIVATED},
    {"TYPE_INTERRUPT", TYPE_INTERRUPT}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioVolumeTypeMap = {
    {"VOICE_CALL", NapiAudioEnum::VOICE_CALL},
    {"RINGTONE", NapiAudioEnum::RINGTONE},
    {"MEDIA", NapiAudioEnum::MEDIA},
    {"VOICE_ASSISTANT", NapiAudioEnum::VOICE_ASSISTANT},
    {"ALARM", NapiAudioEnum::ALARM},
    {"ACCESSIBILITY", NapiAudioEnum::ACCESSIBILITY},
    {"ULTRASONIC", NapiAudioEnum::ULTRASONIC},
    {"ALL", NapiAudioEnum::ALL}
};

const std::map<std::string, int32_t> NapiAudioEnum::activeDeviceTypeMap = {
    {"SPEAKER", SPEAKER},
    {"BLUETOOTH_SCO", BLUETOOTH_SCO}
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptModeMap = {
    {"SHARE_MODE", NapiAudioEnum::SHARE_MODE},
    {"INDEPENDENT_MODE", NapiAudioEnum::INDEPENDENT_MODE}
};

const std::map<std::string, int32_t> NapiAudioEnum::focusTypeMap = {
    {"FOCUS_TYPE_RECORDING", NapiAudioEnum::FOCUS_TYPE_RECORDING}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioErrorsMap = {
    {"ERROR_INVALID_PARAM", ERROR_INVALID_PARAM},
    {"ERROR_NO_MEMORY", ERROR_NO_MEMORY},
    {"ERROR_ILLEGAL_STATE", ERROR_ILLEGAL_STATE},
    {"ERROR_UNSUPPORTED", ERROR_UNSUPPORTED},
    {"ERROR_TIMEOUT", ERROR_TIMEOUT},
    {"ERROR_STREAM_LIMIT", ERROR_STREAM_LIMIT},
    {"ERROR_SYSTEM", ERROR_SYSTEM}
};

const std::map<std::string, int32_t> NapiAudioEnum::communicationDeviceTypeMap = {
    {"SPEAKER", COMMUNICATION_SPEAKER},
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptRequestTypeMap = {
    {"INTERRUPT_REQUEST_TYPE_DEFAULT", INTERRUPT_REQUEST_TYPE_DEFAULT},
};

const std::map<std::string, int32_t> NapiAudioEnum::interruptRequestResultTypeMap = {
    {"INTERRUPT_REQUEST_GRANT", INTERRUPT_REQUEST_GRANT},
    {"INTERRUPT_REQUEST_REJECT", INTERRUPT_REQUEST_REJECT},
};

const std::map<std::string, int32_t> NapiAudioEnum::deviceFlagMap = {
    {"NONE_DEVICES_FLAG", NONE_DEVICES_FLAG},
    {"OUTPUT_DEVICES_FLAG", OUTPUT_DEVICES_FLAG},
    {"INPUT_DEVICES_FLAG", INPUT_DEVICES_FLAG},
    {"ALL_DEVICES_FLAG", ALL_DEVICES_FLAG},
    {"DISTRIBUTED_OUTPUT_DEVICES_FLAG", DISTRIBUTED_OUTPUT_DEVICES_FLAG},
    {"DISTRIBUTED_INPUT_DEVICES_FLAG", DISTRIBUTED_INPUT_DEVICES_FLAG},
    {"ALL_DISTRIBUTED_DEVICES_FLAG", ALL_DISTRIBUTED_DEVICES_FLAG}
};

const std::map<std::string, int32_t> NapiAudioEnum::connectTypeMap = {
    {"CONNECT_TYPE_LOCAL", CONNECT_TYPE_LOCAL},
    {"CONNECT_TYPE_DISTRIBUTED", CONNECT_TYPE_DISTRIBUTED}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioRingModeMap = {
    {"RINGER_MODE_SILENT", NapiAudioEnum::RINGER_MODE_SILENT},
    {"RINGER_MODE_VIBRATE", NapiAudioEnum::RINGER_MODE_VIBRATE},
    {"RINGER_MODE_NORMAL", NapiAudioEnum::RINGER_MODE_NORMAL}
};

const std::map<std::string, int32_t> NapiAudioEnum::toneTypeMap = {
    {"TONE_TYPE_DIAL_0", TONE_TYPE_DIAL_0},
    {"TONE_TYPE_DIAL_1", TONE_TYPE_DIAL_1},
    {"TONE_TYPE_DIAL_2", TONE_TYPE_DIAL_2},
    {"TONE_TYPE_DIAL_3", TONE_TYPE_DIAL_3},
    {"TONE_TYPE_DIAL_4", TONE_TYPE_DIAL_4},
    {"TONE_TYPE_DIAL_5", TONE_TYPE_DIAL_5},
    {"TONE_TYPE_DIAL_6", TONE_TYPE_DIAL_6},
    {"TONE_TYPE_DIAL_7", TONE_TYPE_DIAL_7},
    {"TONE_TYPE_DIAL_8", TONE_TYPE_DIAL_8},
    {"TONE_TYPE_DIAL_9", TONE_TYPE_DIAL_9},
    {"TONE_TYPE_DIAL_S", TONE_TYPE_DIAL_S},
    {"TONE_TYPE_DIAL_P", TONE_TYPE_DIAL_P},
    {"TONE_TYPE_DIAL_A", TONE_TYPE_DIAL_A},
    {"TONE_TYPE_DIAL_B", TONE_TYPE_DIAL_B},
    {"TONE_TYPE_DIAL_C", TONE_TYPE_DIAL_C},
    {"TONE_TYPE_DIAL_D", TONE_TYPE_DIAL_D},
    {"TONE_TYPE_COMMON_SUPERVISORY_DIAL", TONE_TYPE_COMMON_SUPERVISORY_DIAL},
    {"TONE_TYPE_COMMON_SUPERVISORY_BUSY", TONE_TYPE_COMMON_SUPERVISORY_BUSY},
    {"TONE_TYPE_COMMON_SUPERVISORY_CONGESTION", TONE_TYPE_COMMON_SUPERVISORY_CONGESTION},
    {"TONE_TYPE_COMMON_SUPERVISORY_RADIO_ACK", TONE_TYPE_COMMON_SUPERVISORY_RADIO_ACK},
    {"TONE_TYPE_COMMON_SUPERVISORY_RADIO_NOT_AVAILABLE", TONE_TYPE_COMMON_SUPERVISORY_RADIO_NOT_AVAILABLE},
    {"TONE_TYPE_COMMON_SUPERVISORY_CALL_WAITING", TONE_TYPE_COMMON_SUPERVISORY_CALL_WAITING},
    {"TONE_TYPE_COMMON_SUPERVISORY_RINGTONE", TONE_TYPE_COMMON_SUPERVISORY_RINGTONE},
    {"TONE_TYPE_COMMON_PROPRIETARY_BEEP", TONE_TYPE_COMMON_PROPRIETARY_BEEP},
    {"TONE_TYPE_COMMON_PROPRIETARY_ACK", TONE_TYPE_COMMON_PROPRIETARY_ACK},
    {"TONE_TYPE_COMMON_PROPRIETARY_PROMPT", TONE_TYPE_COMMON_PROPRIETARY_PROMPT},
    {"TONE_TYPE_COMMON_PROPRIETARY_DOUBLE_BEEP", TONE_TYPE_COMMON_PROPRIETARY_DOUBLE_BEEP},
};

const std::map<std::string, int32_t> NapiAudioEnum::audioDeviceUsageMap = {
    {"MEDIA_OUTPUT_DEVICES", MEDIA_OUTPUT_DEVICES },
    {"MEDIA_INPUT_DEVICES", MEDIA_INPUT_DEVICES},
    {"ALL_MEDIA_DEVICES", ALL_MEDIA_DEVICES},
    {"CALL_OUTPUT_DEVICES", CALL_OUTPUT_DEVICES},
    {"CALL_INPUT_DEVICES", CALL_INPUT_DEVICES},
    {"ALL_CALL_DEVICES", ALL_CALL_DEVICES},
};

const std::map<std::string, int32_t> NapiAudioEnum::policyTypeMap = {
    {"EDM", EDM_POLICY_TYPE},
    {"PRIVACY", PRIVACY_POLCIY_TYPE},
    {"TEMPORARY", TEMPORARY_POLCIY_TYPE},
};

const std::map<std::string, int32_t> NapiAudioEnum::audioDeviceChangeReasonMap = {
    {"REASON_UNKNOWN", static_cast<int32_t>(AudioStreamDeviceChangeReason::UNKNOWN)},
    {"REASON_NEW_DEVICE_AVAILABLE", static_cast<int32_t>(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE)},
    {"REASON_OLD_DEVICE_UNAVAILABLE", static_cast<int32_t>(AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE)},
    {"REASON_OVERRODE", static_cast<int32_t>(AudioStreamDeviceChangeReason::OVERRODE)},
};

const std::map<std::string, int32_t> NapiAudioEnum::audioSpatialDeivceTypeMap = {
    {"SPATIAL_DEVICE_TYPE_NONE", EARPHONE_TYPE_NONE },
    {"SPATIAL_DEVICE_TYPE_IN_EAR_HEADPHONE", EARPHONE_TYPE_INEAR},
    {"SPATIAL_DEVICE_TYPE_HALF_IN_EAR_HEADPHONE", EARPHONE_TYPE_HALF_INEAR},
    {"SPATIAL_DEVICE_TYPE_OVER_EAR_HEADPHONE", EARPHONE_TYPE_HEADPHONE},
    {"SPATIAL_DEVICE_TYPE_GLASSES", EARPHONE_TYPE_GLASSES},
    {"SPATIAL_DEVICE_TYPE_OTHERS", EARPHONE_TYPE_OTHERS},
};

const std::map<std::string, uint64_t> NapiAudioEnum::audioChannelLayoutMap = {
    {"CH_LAYOUT_UNKNOWN", CH_LAYOUT_UNKNOWN},
    {"CH_LAYOUT_MONO", CH_LAYOUT_MONO},
    {"CH_LAYOUT_STEREO", CH_LAYOUT_STEREO},
    {"CH_LAYOUT_STEREO_DOWNMIX", CH_LAYOUT_STEREO_DOWNMIX},
    {"CH_LAYOUT_2POINT1", CH_LAYOUT_2POINT1},
    {"CH_LAYOUT_3POINT0", CH_LAYOUT_3POINT0},
    {"CH_LAYOUT_SURROUND", CH_LAYOUT_SURROUND},
    {"CH_LAYOUT_3POINT1", CH_LAYOUT_3POINT1},
    {"CH_LAYOUT_4POINT0", CH_LAYOUT_4POINT0},
    {"CH_LAYOUT_QUAD_SIDE", CH_LAYOUT_QUAD_SIDE},
    {"CH_LAYOUT_QUAD", CH_LAYOUT_QUAD},
    {"CH_LAYOUT_2POINT0POINT2", CH_LAYOUT_2POINT0POINT2},
    {"CH_LAYOUT_4POINT1", CH_LAYOUT_4POINT1},
    {"CH_LAYOUT_5POINT0", CH_LAYOUT_5POINT0},
    {"CH_LAYOUT_5POINT0_BACK", CH_LAYOUT_5POINT0_BACK},
    {"CH_LAYOUT_2POINT1POINT2", CH_LAYOUT_2POINT1POINT2},
    {"CH_LAYOUT_3POINT0POINT2", CH_LAYOUT_3POINT0POINT2},
    {"CH_LAYOUT_5POINT1", CH_LAYOUT_5POINT1},
    {"CH_LAYOUT_5POINT1_BACK", CH_LAYOUT_5POINT1_BACK},
    {"CH_LAYOUT_6POINT0", CH_LAYOUT_6POINT0},
    {"CH_LAYOUT_HEXAGONAL", CH_LAYOUT_HEXAGONAL},
    {"CH_LAYOUT_3POINT1POINT2", CH_LAYOUT_3POINT1POINT2},
    {"CH_LAYOUT_6POINT0_FRONT", CH_LAYOUT_6POINT0_FRONT},
    {"CH_LAYOUT_6POINT1", CH_LAYOUT_6POINT1},
    {"CH_LAYOUT_6POINT1_BACK", CH_LAYOUT_6POINT1_BACK},
    {"CH_LAYOUT_6POINT1_FRONT", CH_LAYOUT_6POINT1_FRONT},
    {"CH_LAYOUT_7POINT0", CH_LAYOUT_7POINT0},
    {"CH_LAYOUT_7POINT0_FRONT", CH_LAYOUT_7POINT0_FRONT},
    {"CH_LAYOUT_7POINT1", CH_LAYOUT_7POINT1},
    {"CH_LAYOUT_OCTAGONAL", CH_LAYOUT_OCTAGONAL},
    {"CH_LAYOUT_5POINT1POINT2", CH_LAYOUT_5POINT1POINT2},
    {"CH_LAYOUT_7POINT1_WIDE", CH_LAYOUT_7POINT1_WIDE},
    {"CH_LAYOUT_7POINT1_WIDE_BACK", CH_LAYOUT_7POINT1_WIDE_BACK},
    {"CH_LAYOUT_5POINT1POINT4", CH_LAYOUT_5POINT1POINT4},
    {"CH_LAYOUT_7POINT1POINT2", CH_LAYOUT_7POINT1POINT2},
    {"CH_LAYOUT_7POINT1POINT4", CH_LAYOUT_7POINT1POINT4},
    {"CH_LAYOUT_10POINT2", CH_LAYOUT_10POINT2},
    {"CH_LAYOUT_9POINT1POINT4", CH_LAYOUT_9POINT1POINT4},
    {"CH_LAYOUT_9POINT1POINT6", CH_LAYOUT_9POINT1POINT6},
    {"CH_LAYOUT_HEXADECAGONAL", CH_LAYOUT_HEXADECAGONAL},
    {"CH_LAYOUT_AMB_ORDER1_ACN_N3D", CH_LAYOUT_HOA_ORDER1_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER1_ACN_SN3D", CH_LAYOUT_HOA_ORDER1_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER1_FUMA", CH_LAYOUT_HOA_ORDER1_FUMA},
    {"CH_LAYOUT_AMB_ORDER2_ACN_N3D", CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER2_ACN_SN3D", CH_LAYOUT_HOA_ORDER2_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER2_FUMA", CH_LAYOUT_HOA_ORDER2_FUMA},
    {"CH_LAYOUT_AMB_ORDER3_ACN_N3D", CH_LAYOUT_HOA_ORDER3_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER3_ACN_SN3D", CH_LAYOUT_HOA_ORDER3_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER3_FUMA", CH_LAYOUT_HOA_ORDER3_FUMA},
};

const std::map<std::string, int32_t> NapiAudioEnum::spatializationSceneTypeMap = {
    {"DEFAULT", SPATIALIZATION_SCENE_TYPE_DEFAULT },
    {"MUSIC", SPATIALIZATION_SCENE_TYPE_MUSIC},
    {"MOVIE", SPATIALIZATION_SCENE_TYPE_MOVIE},
    {"AUDIOBOOK", SPATIALIZATION_SCENE_TYPE_AUDIOBOOK},
};

const std::map<std::string, int32_t> NapiAudioEnum::asrNoiseSuppressionModeMap = {
    {"BYPASS", static_cast<int32_t>(AsrNoiseSuppressionMode::BYPASS)},
    {"STANDARD", static_cast<int32_t>(AsrNoiseSuppressionMode::STANDARD)},
    {"NEAR_FIELD", static_cast<int32_t>(AsrNoiseSuppressionMode::NEAR_FIELD)},
    {"FAR_FIELD", static_cast<int32_t>(AsrNoiseSuppressionMode::FAR_FIELD)},
};

const std::map<std::string, int32_t> NapiAudioEnum::asrAecModeMap = {
    {"BYPASS", static_cast<int32_t>(AsrAecMode::BYPASS)},
    {"STANDARD", static_cast<int32_t>(AsrAecMode::STANDARD)}
};

const std::map<std::string, int32_t> NapiAudioEnum::asrWhisperDetectionModeMap = {
    {"BYPASS", static_cast<int32_t>(AsrWhisperDetectionMode::BYPASS)},
    {"STANDARD", static_cast<int32_t>(AsrWhisperDetectionMode::STANDARD)}
};

const std::map<std::string, int32_t> NapiAudioEnum::asrVoiceControlModeMap = {
    {"AUDIO_2_VOICETX", static_cast<int32_t>(AsrVoiceControlMode::AUDIO_2_VOICETX)},
    {"AUDIO_MIX_2_VOICETX", static_cast<int32_t>(AsrVoiceControlMode::AUDIO_MIX_2_VOICETX)},
    {"AUDIO_2_VOICE_TX_EX", static_cast<int32_t>(AsrVoiceControlMode::AUDIO_2_VOICE_TX_EX)},
    {"AUDIO_MIX_2_VOICE_TX_EX", static_cast<int32_t>(AsrVoiceControlMode::AUDIO_MIX_2_VOICE_TX_EX)}
};

const std::map<std::string, int32_t> NapiAudioEnum::asrVoiceMuteModeMap = {
    {"OUTPUT_MUTE", static_cast<int32_t>(AsrVoiceMuteMode::OUTPUT_MUTE)},
    {"INPUT_MUTE", static_cast<int32_t>(AsrVoiceMuteMode::INPUT_MUTE)},
    {"TTS_MUTE", static_cast<int32_t>(AsrVoiceMuteMode::TTS_MUTE)},
    {"CALL_MUTE", static_cast<int32_t>(AsrVoiceMuteMode::CALL_MUTE)},
    {"OUTPUT_MUTE_EX", static_cast<int32_t>(AsrVoiceMuteMode::OUTPUT_MUTE_EX)}
};

const std::map<std::string, int32_t> NapiAudioEnum::audioDataCallbackResultMap = {
    {"INVALID", NapiAudioEnum::CALLBACK_RESULT_INVALID},
    {"VALID", NapiAudioEnum::CALLBACK_RESULT_VALID}
};

const std::map<std::string, int32_t> NapiAudioEnum::concurrencyModeMap = {
    {"CONCURRENCY_DEFAULT", static_cast<int32_t>(AudioConcurrencyMode::DEFAULT)},
    {"CONCURRENCY_MIX_WITH_OTHERS", static_cast<int32_t>(AudioConcurrencyMode::MIX_WITH_OTHERS)},
    {"CONCURRENCY_DUCK_OTHERS", static_cast<int32_t>(AudioConcurrencyMode::DUCK_OTHERS)},
    {"CONCURRENCY_PAUSE_OTHERS", static_cast<int32_t>(AudioConcurrencyMode::PAUSE_OTHERS)}
};

const std::map<std::string, int32_t> NapiAudioEnum::reasonMap = {
    {"DEACTIVATED_LOWER_PRIORITY", static_cast<int32_t>(AudioSessionDeactiveReason::LOW_PRIORITY)},
    {"DEACTIVATED_TIMEOUT", static_cast<int32_t>(AudioSessionDeactiveReason::TIMEOUT)}
};

NapiAudioEnum::NapiAudioEnum()
    : env_(nullptr) {
}

NapiAudioEnum::~NapiAudioEnum()
{
    audioParameters_ = nullptr;
}

void NapiAudioEnum::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioEnum *>(nativeObject);
        delete obj;
    }
}

napi_value NapiAudioEnum::CreateEnumObject(const napi_env &env, const std::map<std::string, int32_t> &map,
    napi_ref &ref)
{
    std::string propName;
    napi_value result = nullptr;
    napi_status status = napi_create_object(env, &result);
    if (status != napi_ok) {
        goto error;
    }

    for (const auto &iter : map) {
        propName = iter.first;
        status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
        CHECK_AND_BREAK_LOG(status == napi_ok, "Failed to add named prop!");
        propName.clear();
    }
    if (status != napi_ok) {
        goto error;
    }
    status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &ref);
    if (status != napi_ok) {
        goto error;
    }
    return result;

error:
    AUDIO_ERR_LOG("create Enum Object failed");
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioEnum::CreateEnumInt64Object(const napi_env &env, const std::map<std::string, uint64_t> &map,
    napi_ref &ref)
{
    std::string propName;
    napi_value result = nullptr;
    napi_status status = napi_create_object(env, &result);
    if (status != napi_ok) {
        goto error;
    }

    for (const auto &iter : map) {
        propName = iter.first;
        status = NapiParamUtils::SetValueInt64(env, propName, iter.second, result);
        CHECK_AND_BREAK_LOG(status == napi_ok, "Failed to add named prop!");
        propName.clear();
    }
    if (status != napi_ok) {
        goto error;
    }
    status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &ref);
    if (status != napi_ok) {
        goto error;
    }
    return result;

error:
    AUDIO_ERR_LOG("create Enum Object failed");
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioEnum::CreateLocalNetworkIdObject(napi_env env)
{
    napi_value result = nullptr;
    napi_create_string_utf8(env, LOCAL_NETWORK_ID.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value NapiAudioEnum::CreateDefaultVolumeGroupIdObject(napi_env env)
{
    napi_value defaultVolumeGroupId;
    napi_create_int32(env, DEFAULT_VOLUME_GROUP_ID, &defaultVolumeGroupId);
    return defaultVolumeGroupId;
}

napi_value NapiAudioEnum::CreateDefaultInterruptIdObject(napi_env env)
{
    napi_value defaultInterruptId;
    napi_create_int32(env, DEFAULT_VOLUME_INTERRUPT_ID, &defaultInterruptId);
    return defaultInterruptId;
}

napi_status NapiAudioEnum::InitAudioExternEnum(napi_env env, napi_value exports)
{
    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_PROPERTY("AudioSpatialDeviceType", CreateEnumObject(env,
            audioSpatialDeivceTypeMap, audioSpatialDeivceType_)),
        DECLARE_NAPI_PROPERTY("AudioChannelLayout", CreateEnumInt64Object(env,
            audioChannelLayoutMap, audioChannelLayout_)),
        DECLARE_NAPI_PROPERTY("AudioStreamDeviceChangeReason",
            CreateEnumObject(env, audioDeviceChangeReasonMap, audioStreamDeviceChangeReason_)),
        DECLARE_NAPI_PROPERTY("AudioSpatializationSceneType", CreateEnumObject(env,
            spatializationSceneTypeMap, spatializationSceneType_)),
        DECLARE_NAPI_PROPERTY("AsrNoiseSuppressionMode", CreateEnumObject(env, asrNoiseSuppressionModeMap,
            asrNoiseSuppressionMode_)),
        DECLARE_NAPI_PROPERTY("AsrAecMode", CreateEnumObject(env, asrAecModeMap, asrAecMode_)),
        DECLARE_NAPI_PROPERTY("AsrWhisperDetectionMode", CreateEnumObject(env,
            asrWhisperDetectionModeMap, asrWhisperDetectionMode_)),
        DECLARE_NAPI_PROPERTY("AsrVoiceControlMode", CreateEnumObject(env,
            asrVoiceControlModeMap, asrVoiceControlMode_)),
        DECLARE_NAPI_PROPERTY("AsrVoiceMuteMode", CreateEnumObject(env, asrVoiceMuteModeMap, asrVoiceMuteMode_)),
    };
    napi_status status =
        napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    return status;
}

napi_status NapiAudioEnum::InitAudioEnum(napi_env env, napi_value exports)
{
    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_PROPERTY("AudioChannel", CreateEnumObject(env, audioChannelMap, audioChannel_)),
        DECLARE_NAPI_PROPERTY("AudioSamplingRate", CreateEnumObject(env, samplingRateMap, samplingRate_)),
        DECLARE_NAPI_PROPERTY("AudioEncodingType", CreateEnumObject(env, encodingTypeMap, encodingType_)),
        DECLARE_NAPI_PROPERTY("ContentType", CreateEnumObject(env, contentTypeMap, contentType_)),
        DECLARE_NAPI_PROPERTY("StreamUsage", CreateEnumObject(env, streamUsageMap, streamUsage_)),
        DECLARE_NAPI_PROPERTY("DeviceRole", CreateEnumObject(env, deviceRoleMap, deviceRole_)),
        DECLARE_NAPI_PROPERTY("DeviceType", CreateEnumObject(env, deviceTypeMap, deviceType_)),
        DECLARE_NAPI_PROPERTY("SourceType", CreateEnumObject(env, sourceTypeMap, sourceType_)),
        DECLARE_NAPI_PROPERTY("VolumeAdjustType", CreateEnumObject(env, volumeAdjustTypeMap, volumeAdjustType_)),
        DECLARE_NAPI_PROPERTY("ChannelBlendMode", CreateEnumObject(env, channelBlendModeMap, channelBlendMode_)),
        DECLARE_NAPI_PROPERTY("AudioRendererRate", CreateEnumObject(env, rendererRateMap, audioRendererRate_)),
        DECLARE_NAPI_PROPERTY("InterruptType", CreateEnumObject(env, interruptEventTypeMap, interruptEventType_)),
        DECLARE_NAPI_PROPERTY("InterruptForceType", CreateEnumObject(env, interruptForceTypeMap, interruptForceType_)),
        DECLARE_NAPI_PROPERTY("InterruptHint", CreateEnumObject(env, interruptHintTypeMap, interruptHintType_)),
        DECLARE_NAPI_PROPERTY("AudioState", CreateEnumObject(env, audioStateMap, audioState_)),
        DECLARE_NAPI_PROPERTY("AudioSampleFormat", CreateEnumObject(env, audioSampleFormatMap, sampleFormat_)),
        DECLARE_NAPI_PROPERTY("AudioEffectMode", CreateEnumObject(env, effectModeMap, audioEffectMode_)),
        DECLARE_NAPI_PROPERTY("AudioPrivacyType", CreateEnumObject(env, audioPrivacyTypeMap, audioPrivacyType_)),
        DECLARE_NAPI_PROPERTY("AudioVolumeType", CreateEnumObject(env, audioVolumeTypeMap, audioVolumeTypeRef_)),
        DECLARE_NAPI_PROPERTY("DeviceFlag", CreateEnumObject(env, deviceFlagMap, deviceFlagRef_)),
        DECLARE_NAPI_PROPERTY("ActiveDeviceType", CreateEnumObject(env, activeDeviceTypeMap, activeDeviceTypeRef_)),
        DECLARE_NAPI_PROPERTY("ConnectType", CreateEnumObject(env, connectTypeMap, connectTypeRef_)),
        DECLARE_NAPI_PROPERTY("AudioRingMode", CreateEnumObject(env, audioRingModeMap, audioRingModeRef_)),
        DECLARE_NAPI_PROPERTY("AudioScene", CreateEnumObject(env, audioSceneMap, audioScene_)),
        DECLARE_NAPI_PROPERTY("DeviceChangeType", CreateEnumObject(env, deviceChangeTypeMap, deviceChangeType_)),
        DECLARE_NAPI_PROPERTY("InterruptActionType",
            CreateEnumObject(env, interruptActionTypeMap, interruptActionType_)),
        DECLARE_NAPI_PROPERTY("InterruptMode", CreateEnumObject(env, interruptModeMap, interruptMode_)),
        DECLARE_NAPI_PROPERTY("FocusType", CreateEnumObject(env, focusTypeMap, focusType_)),
        DECLARE_NAPI_PROPERTY("LOCAL_NETWORK_ID", CreateLocalNetworkIdObject(env)),
        DECLARE_NAPI_PROPERTY("DEFAULT_VOLUME_GROUP_ID", CreateDefaultVolumeGroupIdObject(env)),
        DECLARE_NAPI_PROPERTY("DEFAULT_INTERRUPT_GROUP_ID", CreateDefaultInterruptIdObject(env)),
        DECLARE_NAPI_PROPERTY("AudioErrors", CreateEnumObject(env, audioErrorsMap, audioErrors_)),
        DECLARE_NAPI_PROPERTY("CommunicationDeviceType",
            CreateEnumObject(env, communicationDeviceTypeMap, communicationDeviceType_)),
        DECLARE_NAPI_PROPERTY("InterruptRequestType",
            CreateEnumObject(env, interruptRequestTypeMap, interruptRequestType_)),
        DECLARE_NAPI_PROPERTY("InterruptRequestResultType",
            CreateEnumObject(env, interruptRequestResultTypeMap, interruptRequestResultType_)),
        DECLARE_NAPI_PROPERTY("ToneType", CreateEnumObject(env, toneTypeMap, toneType_)),
        DECLARE_NAPI_PROPERTY("DeviceUsage", CreateEnumObject(env, audioDeviceUsageMap, audioDviceUsage_)),
        DECLARE_NAPI_PROPERTY("AudioDataCallbackResult",
            CreateEnumObject(env, audioDataCallbackResultMap, audioDataCallbackResult_)),
        DECLARE_NAPI_PROPERTY("AudioConcurrencyMode",
            CreateEnumObject(env, concurrencyModeMap, concurrencyMode_)),
        DECLARE_NAPI_PROPERTY("AudioSessionDeactivatedReason", CreateEnumObject(env, reasonMap, reason_)),
        DECLARE_NAPI_PROPERTY("PolicyType", CreateEnumObject(env, policyTypeMap, policyType_)),
    };
    napi_status status =
        napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    return status;
}

napi_value NapiAudioEnum::Init(napi_env env, napi_value exports)
{
    AUDIO_DEBUG_LOG("NapiAudioEnum::Init()");
    napi_value constructor;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_parameters_properties[] = {
        DECLARE_NAPI_GETTER_SETTER("format", GetAudioSampleFormat, SetAudioSampleFormat),
        DECLARE_NAPI_GETTER_SETTER("channels", GetAudioChannel, SetAudioChannel),
        DECLARE_NAPI_GETTER_SETTER("samplingRate", GetAudioSamplingRate, SetAudioSamplingRate),
        DECLARE_NAPI_GETTER_SETTER("encoding", GetAudioEncodingType, SetAudioEncodingType),
        DECLARE_NAPI_GETTER_SETTER("contentType", GetContentType, SetContentType),
        DECLARE_NAPI_GETTER_SETTER("usage", GetStreamUsage, SetStreamUsage),
        DECLARE_NAPI_GETTER_SETTER("deviceRole", GetDeviceRole, SetDeviceRole),
        DECLARE_NAPI_GETTER_SETTER("deviceType", GetDeviceType, SetDeviceType)
    };

    napi_status status = napi_define_class(env, NAPI_AUDIO_ENUM_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr, sizeof(audio_parameters_properties) / sizeof(audio_parameters_properties[0]),
        audio_parameters_properties, &constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");

    status = napi_create_reference(env, constructor, REFERENCE_CREATION_COUNT, &sConstructor_);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_ENUM_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = InitAudioEnum(env, exports);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitAudioEnum failed");
    status = InitAudioExternEnum(env, exports);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitAudioExternEnum failed");

    return exports;
}

napi_value NapiAudioEnum::Construct(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    size_t argCount = 0;
    unique_ptr<NapiAudioEnum> obj = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Construct:napi_get_cb_info failed!");
        goto error;
    }

    obj = make_unique<NapiAudioEnum>();
    if (obj == nullptr) {
        AUDIO_ERR_LOG("obj make_unique failed,no memery.");
        goto error;
    }
    obj->env_ = env;
    obj->audioParameters_ = move(sAudioParameters_);
    status = napi_wrap(env, jsThis, static_cast<void*>(obj.get()), NapiAudioEnum::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        goto error;
    }
    obj.release();
    return jsThis;

error:
    napi_get_undefined(env, &jsThis);
    return jsThis;
}

NapiAudioEnum* NapiAudioEnum::SetValue(napi_env env, napi_callback_info info, napi_value *args, napi_value &result)
{
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value jsThis = nullptr;
    napi_get_undefined(env, &result);

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || argc < 1) {
        AUDIO_ERR_LOG("SetValue fail to napi_get_cb_info");
        return nullptr;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, napiAudioEnum, "napi_unwrap failed");

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
        AUDIO_ERR_LOG("SetValue fail: wrong data type");
    }
    return napiAudioEnum;
}

NapiAudioEnum* NapiAudioEnum::GetValue(napi_env env, napi_callback_info info)
{
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("GetValue fail to napi_get_cb_info");
        return nullptr;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, napiAudioEnum, "napi_unwrap failed");
    if (!((napiAudioEnum != nullptr) && (napiAudioEnum->audioParameters_ != nullptr))) {
        AUDIO_ERR_LOG("GetValue fail to napi_unwrap");
    }
    return napiAudioEnum;
}

napi_value NapiAudioEnum::GetAudioSampleFormat(napi_env env, napi_callback_info info)
{
    AudioSampleFormat audioSampleFormat;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    audioSampleFormat = napiAudioEnum->audioParameters_->format;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(audioSampleFormat), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetAudioSampleFormat fail");
    return jsResult;
}

napi_value NapiAudioEnum::SetAudioSampleFormat(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t audioSampleFormat;
    napi_status status = NapiParamUtils::GetValueInt32(env, audioSampleFormat, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->format = static_cast<AudioSampleFormat>(audioSampleFormat);

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioChannel(napi_env env, napi_callback_info info)
{
    AudioChannel audioChannel;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    audioChannel = napiAudioEnum->audioParameters_->channels;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(audioChannel), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetAudioChannel fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioChannel(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t audioChannel;
    napi_status status = NapiParamUtils::GetValueInt32(env, audioChannel, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->channels = static_cast<AudioChannel>(audioChannel);

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioSamplingRate(napi_env env, napi_callback_info info)
{
    AudioSamplingRate samplingRate;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    samplingRate = napiAudioEnum->audioParameters_->samplingRate;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(samplingRate), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetAudioSamplingRate fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioSamplingRate(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t samplingRate;
    napi_status status = NapiParamUtils::GetValueInt32(env, samplingRate, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->samplingRate = static_cast<AudioSamplingRate>(samplingRate);

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioEncodingType(napi_env env, napi_callback_info info)
{
    AudioEncodingType encodingType;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    encodingType = napiAudioEnum->audioParameters_->encoding;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(encodingType), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetAudioEncodingType fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioEncodingType(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t encodingType;
    napi_status status = NapiParamUtils::GetValueInt32(env, encodingType, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->encoding = static_cast<AudioEncodingType>(encodingType);

    return jsResult;
}

napi_value NapiAudioEnum::GetContentType(napi_env env, napi_callback_info info)
{
    ContentType contentType;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    contentType = napiAudioEnum->audioParameters_->contentType;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(contentType), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetContentType fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetContentType(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t contentType;
    napi_status status = NapiParamUtils::GetValueInt32(env, contentType, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->contentType = static_cast<ContentType>(contentType);

    return jsResult;
}

napi_value NapiAudioEnum::GetStreamUsage(napi_env env, napi_callback_info info)
{
    StreamUsage usage;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    usage = napiAudioEnum->audioParameters_->usage;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(usage), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetStreamUsage fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetStreamUsage(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t usage;
    napi_status status = NapiParamUtils::GetValueInt32(env, usage, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->usage = static_cast<StreamUsage>(usage);

    return jsResult;
}

napi_value NapiAudioEnum::GetDeviceRole(napi_env env, napi_callback_info info)
{
    DeviceRole deviceRole;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    deviceRole = napiAudioEnum->audioParameters_->deviceRole;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(deviceRole), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetDeviceRole fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetDeviceRole(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t deviceRole;
    napi_status status = NapiParamUtils::GetValueInt32(env, deviceRole, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->deviceRole = static_cast<DeviceRole>(deviceRole);

    return jsResult;
}

napi_value NapiAudioEnum::GetDeviceType(napi_env env, napi_callback_info info)
{
    DeviceType deviceType;
    napi_value jsResult = nullptr;
    NapiAudioEnum *napiAudioEnum = GetValue(env, info);

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    deviceType = napiAudioEnum->audioParameters_->deviceType;
    napi_status status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(deviceType), jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetDeviceType fail");

    return jsResult;
}

napi_value NapiAudioEnum::SetDeviceType(napi_env env, napi_callback_info info)
{
    napi_value jsResult = nullptr;
    napi_value args[1] = { nullptr };
    NapiAudioEnum *napiAudioEnum = SetValue(env, info, args, jsResult);
    int32_t deviceType;
    napi_status status = NapiParamUtils::GetValueInt32(env, deviceType, args[0]);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, jsResult, "GetValueInt32 fail");

    CHECK_AND_RETURN_RET_LOG(napiAudioEnum != nullptr, jsResult, "napiAudioEnum is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioEnum->audioParameters_ != nullptr, jsResult, "audioParameters_ is nullptr");
    napiAudioEnum->audioParameters_->deviceType = static_cast<DeviceType>(deviceType);

    return jsResult;
}

bool NapiAudioEnum::IsLegalInputArgumentInterruptMode(int32_t interruptMode)
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

bool NapiAudioEnum::IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode)
{
    bool result = false;
    switch (audioEffectMode) {
        case AudioEffectMode::EFFECT_NONE:
        case AudioEffectMode::EFFECT_DEFAULT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentChannelBlendMode(int32_t blendMode)
{
    bool result = false;
    switch (blendMode) {
        case ChannelBlendMode::MODE_DEFAULT:
        case ChannelBlendMode::MODE_BLEND_LR:
        case ChannelBlendMode::MODE_ALL_LEFT:
        case ChannelBlendMode::MODE_ALL_RIGHT:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalCapturerType(int32_t type)
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
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentVolType(int32_t inputType)
{
    bool result = false;
    switch (inputType) {
        case AudioJsVolumeType::RINGTONE:
        case AudioJsVolumeType::MEDIA:
        case AudioJsVolumeType::VOICE_CALL:
        case AudioJsVolumeType::VOICE_ASSISTANT:
        case AudioJsVolumeType::ALARM:
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

bool NapiAudioEnum::IsLegalInputArgumentRingMode(int32_t ringMode)
{
    bool result = false;
    switch (ringMode) {
        case AudioRingMode::RINGER_MODE_SILENT:
        case AudioRingMode::RINGER_MODE_VIBRATE:
        case AudioRingMode::RINGER_MODE_NORMAL:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentVolumeAdjustType(int32_t adjustType)
{
    bool result = false;
    switch (adjustType) {
        case VolumeAdjustType::VOLUME_UP:
        case VolumeAdjustType::VOLUME_DOWN:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentDeviceType(int32_t deviceType)
{
    bool result = false;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case DeviceType::DEVICE_TYPE_MIC:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_FILE_SINK:
        case DeviceType::DEVICE_TYPE_FILE_SOURCE:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

int32_t NapiAudioEnum::GetJsAudioVolumeType(AudioStreamType volumeType)
{
    int32_t result = MEDIA;
    switch (volumeType) {
        case AudioStreamType::STREAM_VOICE_CALL:
        case AudioStreamType::STREAM_VOICE_MESSAGE:
        case AudioStreamType::STREAM_VOICE_COMMUNICATION:
        case AudioStreamType::STREAM_VOICE_CALL_ASSISTANT:
            result = NapiAudioEnum::VOICE_CALL;
            break;
        case AudioStreamType::STREAM_RING:
        case AudioStreamType::STREAM_SYSTEM:
        case AudioStreamType::STREAM_NOTIFICATION:
        case AudioStreamType::STREAM_SYSTEM_ENFORCED:
        case AudioStreamType::STREAM_DTMF:
            result = NapiAudioEnum::RINGTONE;
            break;
        case AudioStreamType::STREAM_MUSIC:
        case AudioStreamType::STREAM_MEDIA:
        case AudioStreamType::STREAM_MOVIE:
        case AudioStreamType::STREAM_GAME:
        case AudioStreamType::STREAM_SPEECH:
        case AudioStreamType::STREAM_NAVIGATION:
            result = NapiAudioEnum::MEDIA;
            break;
        case AudioStreamType::STREAM_ALARM:
            result = NapiAudioEnum::ALARM;
            break;
        case AudioStreamType::STREAM_ACCESSIBILITY:
            result = NapiAudioEnum::ACCESSIBILITY;
            break;
        case AudioStreamType::STREAM_VOICE_ASSISTANT:
            result = NapiAudioEnum::VOICE_ASSISTANT;
            break;
        case AudioStreamType::STREAM_ULTRASONIC:
            result = NapiAudioEnum::ULTRASONIC;
            break;
        default:
            result = NapiAudioEnum::MEDIA;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentCommunicationDeviceType(int32_t communicationDeviceType)
{
    bool result = false;
    switch (communicationDeviceType) {
        case CommunicationDeviceType::COMMUNICATION_SPEAKER:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentDeviceFlag(int32_t deviceFlag)
{
    bool result = false;
    switch (deviceFlag) {
        case DeviceFlag::NONE_DEVICES_FLAG:
        case DeviceFlag::OUTPUT_DEVICES_FLAG:
        case DeviceFlag::INPUT_DEVICES_FLAG:
        case DeviceFlag::ALL_DEVICES_FLAG:
        case DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG:
        case DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG:
        case DeviceFlag::ALL_L_D_DEVICES_FLAG:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentActiveDeviceType(int32_t activeDeviceFlag)
{
    bool result = false;
    switch (activeDeviceFlag) {
        case ActiveDeviceType::SPEAKER:
        case ActiveDeviceType::BLUETOOTH_SCO:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

bool NapiAudioEnum::IsValidSourceType(int32_t intValue)
{
    SourceType sourceTypeValue = static_cast<SourceType>(intValue);
    switch (sourceTypeValue) {
        case SourceType::SOURCE_TYPE_MIC:
        case SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SourceType::SOURCE_TYPE_ULTRASONIC:
        case SourceType::SOURCE_TYPE_VOICE_COMMUNICATION:
        case SourceType::SOURCE_TYPE_VOICE_RECOGNITION:
        case SourceType::SOURCE_TYPE_WAKEUP:
        case SourceType::SOURCE_TYPE_VOICE_CALL:
        case SourceType::SOURCE_TYPE_VOICE_MESSAGE:
        case SourceType::SOURCE_TYPE_REMOTE_CAST:
        case SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION:
            return true;
        default:
            return false;
    }
}

bool NapiAudioEnum::IsLegalDeviceUsage(int32_t usage)
{
    bool result = false;
    switch (usage) {
        case AudioDeviceUsage::MEDIA_OUTPUT_DEVICES:
        case AudioDeviceUsage::MEDIA_INPUT_DEVICES:
        case AudioDeviceUsage::ALL_MEDIA_DEVICES:
        case AudioDeviceUsage::CALL_OUTPUT_DEVICES:
        case AudioDeviceUsage::CALL_INPUT_DEVICES:
        case AudioDeviceUsage::ALL_CALL_DEVICES:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}


bool NapiAudioEnum::IsLegalInputArgumentStreamUsage(int32_t streamUsage)
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

bool NapiAudioEnum::IsLegalOutputDeviceType(int32_t deviceType)
{
    bool result = false;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case DeviceType::DEVICE_TYPE_DP:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
        case DeviceType::DEVICE_TYPE_REMOTE_CAST:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}

AudioVolumeType NapiAudioEnum::GetNativeAudioVolumeType(int32_t volumeType)
{
    AudioVolumeType result = STREAM_MUSIC;

    switch (volumeType) {
        case NapiAudioEnum::VOICE_CALL:
            result = STREAM_VOICE_CALL;
            break;
        case NapiAudioEnum::RINGTONE:
            result = STREAM_RING;
            break;
        case NapiAudioEnum::MEDIA:
            result = STREAM_MUSIC;
            break;
        case NapiAudioEnum::ALARM:
            result = STREAM_ALARM;
            break;
        case NapiAudioEnum::ACCESSIBILITY:
            result = STREAM_ACCESSIBILITY;
            break;
        case NapiAudioEnum::VOICE_ASSISTANT:
            result = STREAM_VOICE_ASSISTANT;
            break;
        case NapiAudioEnum::ULTRASONIC:
            result = STREAM_ULTRASONIC;
            break;
        case NapiAudioEnum::ALL:
            result = STREAM_ALL;
            break;
        default:
            result = STREAM_MUSIC;
            AUDIO_ERR_LOG("GetNativeAudioVolumeType: Unknown volume type, Set it to default MEDIA!");
            break;
    }

    return result;
}

AudioRingerMode NapiAudioEnum::GetNativeAudioRingerMode(int32_t ringMode)
{
    AudioRingerMode result = AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;

    switch (ringMode) {
        case NapiAudioEnum::RINGER_MODE_SILENT:
            result = AudioStandard::AudioRingerMode::RINGER_MODE_SILENT;
            break;
        case NapiAudioEnum::RINGER_MODE_VIBRATE:
            result = AudioStandard::AudioRingerMode::RINGER_MODE_VIBRATE;
            break;
        case NapiAudioEnum::RINGER_MODE_NORMAL:
            result = AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;
            break;
        default:
            result = AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL;
            AUDIO_ERR_LOG("Unknown ringer mode requested by JS, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

NapiAudioEnum::AudioRingMode NapiAudioEnum::GetJsAudioRingMode(int32_t ringerMode)
{
    NapiAudioEnum::AudioRingMode result = NapiAudioEnum::AudioRingMode::RINGER_MODE_NORMAL;

    switch (ringerMode) {
        case AudioStandard::AudioRingerMode::RINGER_MODE_SILENT:
            result = NapiAudioEnum::RINGER_MODE_SILENT;
            break;
        case AudioStandard::AudioRingerMode::RINGER_MODE_VIBRATE:
            result = NapiAudioEnum::RINGER_MODE_VIBRATE;
            break;
        case AudioStandard::AudioRingerMode::RINGER_MODE_NORMAL:
            result = NapiAudioEnum::RINGER_MODE_NORMAL;
            break;
        default:
            result = NapiAudioEnum::RINGER_MODE_NORMAL;
            AUDIO_ERR_LOG("Unknown ringer mode returned from native, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

AudioStandard::FocusType NapiAudioEnum::GetNativeFocusType(int32_t focusType)
{
    AudioStandard::FocusType result = AudioStandard::FocusType::FOCUS_TYPE_RECORDING;
    switch (focusType) {
        case NapiAudioEnum::FocusType::FOCUS_TYPE_RECORDING:
            result =  AudioStandard::FocusType::FOCUS_TYPE_RECORDING;
            break;
        default:
            AUDIO_ERR_LOG("Unknown focusType type, Set it to default FOCUS_TYPE_RECORDING!");
            break;
    }

    return result;
}

AudioStandard::InterruptMode NapiAudioEnum::GetNativeInterruptMode(int32_t interruptMode)
{
    AudioStandard::InterruptMode result;
    switch (interruptMode) {
        case NapiAudioEnum::InterruptMode::SHARE_MODE:
            result = AudioStandard::InterruptMode::SHARE_MODE;
            break;
        case NapiAudioEnum::InterruptMode::INDEPENDENT_MODE:
            result = AudioStandard::InterruptMode::INDEPENDENT_MODE;
            break;
        default:
            result = AudioStandard::InterruptMode::SHARE_MODE;
            AUDIO_ERR_LOG("Unknown interruptMode type, Set it to default SHARE_MODE!");
            break;
    }
    return result;
}

bool NapiAudioEnum::IsLegalInputArgumentSpatializationSceneType(int32_t spatializationSceneType)
{
    bool result = false;
    switch (spatializationSceneType) {
        case AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT:
        case AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MUSIC:
        case AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_MOVIE:
        case AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_AUDIOBOOK:
            result = true;
            break;
        default:
            result = false;
            break;
    }
    return result;
}
}  // namespace AudioStandard
}  // namespace OHOS
