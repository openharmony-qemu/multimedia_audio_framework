/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_debug_manager_private.h"

#include <cinttypes>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>

#ifndef LOG_TAG
#define LOG_TAG "AudioDebugManager"
#endif

#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "audio_interrupt_info.h"
#include "audio_pipe_types.h"

namespace OHOS {
namespace AudioStandard {
namespace {

const std::map<DeviceType, std::string> DEVICE_TYPE_MAP = {
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
    {DEVICE_TYPE_DP, "DEVICE_TYPE_DP"},
    {DEVICE_TYPE_REMOTE_CAST, "DEVICE_TYPE_REMOTE_CAST"},
    {DEVICE_TYPE_USB_DEVICE, "DEVICE_TYPE_USB_DEVICE"},
    {DEVICE_TYPE_ACCESSORY, "DEVICE_TYPE_ACCESSORY"},
    {DEVICE_TYPE_REMOTE_DAUDIO, "DEVICE_TYPE_REMOTE_DAUDIO"},
    {DEVICE_TYPE_HEARING_AID, "DEVICE_TYPE_HEARING_AID"},
    {DEVICE_TYPE_HDMI, "DEVICE_TYPE_HDMI"},
    {DEVICE_TYPE_LINE_DIGITAL, "DEVICE_TYPE_LINE_DIGITAL"},
    {DEVICE_TYPE_NEARLINK, "DEVICE_TYPE_NEARLINK"},
    {DEVICE_TYPE_NEARLINK_IN, "DEVICE_TYPE_NEARLINK_IN"},
    {DEVICE_TYPE_BT_SPP, "DEVICE_TYPE_BT_SPP"},
    {DEVICE_TYPE_NEARLINK_PORT, "DEVICE_TYPE_NEARLINK_PORT"},
    {DEVICE_TYPE_FILE_SINK, "DEVICE_TYPE_FILE_SINK"},
    {DEVICE_TYPE_FILE_SOURCE, "DEVICE_TYPE_FILE_SOURCE"},
    {DEVICE_TYPE_DEFAULT, "DEVICE_TYPE_DEFAULT"},
};

const std::map<AudioSampleFormat, std::string> AUDIO_SAMPLE_FORMAT_MAP = {
    {SAMPLE_U8, "SAMPLE_U8"},
    {SAMPLE_S16LE, "SAMPLE_S16LE"},
    {SAMPLE_S24LE, "SAMPLE_S24LE"},
    {SAMPLE_S32LE, "SAMPLE_S32LE"},
    {SAMPLE_F32LE, "SAMPLE_F32LE"},
    {INVALID_WIDTH, "INVALID_WIDTH"},
};

const std::map<AudioEncodingType, std::string> AUDIO_ENCODING_TYPE_MAP = {
    {ENCODING_INVALID, "ENCODING_INVALID"},
    {ENCODING_PCM, "ENCODING_PCM"},
    {ENCODING_AUDIOVIVID, "ENCODING_AUDIOVIVID"},
    {ENCODING_EAC3, "ENCODING_EAC3"},
    {ENCODING_AC3, "ENCODING_AC3"},
    {ENCODING_TRUE_HD, "ENCODING_TRUE_HD"},
    {ENCODING_DTS_HD, "ENCODING_DTS_HD"},
    {ENCODING_DTS_X, "ENCODING_DTS_X"},
    {ENCODING_AUDIOVIVID_DIRECT, "ENCODING_AUDIOVIVID_DIRECT"},
    {ENCODING_AUDIOVIVID_3DA_DIRECT, "ENCODING_AUDIOVIVID_3DA_DIRECT"},
};

const std::map<StreamUsage, std::string> STREAM_USAGE_MAP = {
    {STREAM_USAGE_INVALID, "STREAM_USAGE_INVALID"},
    {STREAM_USAGE_UNKNOWN, "STREAM_USAGE_UNKNOWN"},
    {STREAM_USAGE_MEDIA, "STREAM_USAGE_MEDIA"},
    {STREAM_USAGE_MUSIC, "STREAM_USAGE_MUSIC"},
    {STREAM_USAGE_VOICE_COMMUNICATION, "STREAM_USAGE_VOICE_COMMUNICATION"},
    {STREAM_USAGE_VOICE_ASSISTANT, "STREAM_USAGE_VOICE_ASSISTANT"},
    {STREAM_USAGE_ALARM, "STREAM_USAGE_ALARM"},
    {STREAM_USAGE_VOICE_MESSAGE, "STREAM_USAGE_VOICE_MESSAGE"},
    {STREAM_USAGE_NOTIFICATION_RINGTONE, "STREAM_USAGE_NOTIFICATION_RINGTONE"},
    {STREAM_USAGE_RINGTONE, "STREAM_USAGE_RINGTONE"},
    {STREAM_USAGE_NOTIFICATION, "STREAM_USAGE_NOTIFICATION"},
    {STREAM_USAGE_SYSTEM, "STREAM_USAGE_SYSTEM"},
    {STREAM_USAGE_MOVIE, "STREAM_USAGE_MOVIE"},
    {STREAM_USAGE_GAME, "STREAM_USAGE_GAME"},
    {STREAM_USAGE_AUDIOBOOK, "STREAM_USAGE_AUDIOBOOK"},
    {STREAM_USAGE_NAVIGATION, "STREAM_USAGE_NAVIGATION"},
    {STREAM_USAGE_DTMF, "STREAM_USAGE_DTMF"},
    {STREAM_USAGE_ENFORCED_TONE, "STREAM_USAGE_ENFORCED_TONE"},
    {STREAM_USAGE_ULTRASONIC, "STREAM_USAGE_ULTRASONIC"},
    {STREAM_USAGE_VIDEO_COMMUNICATION, "STREAM_USAGE_VIDEO_COMMUNICATION"},
    {STREAM_USAGE_RANGING, "STREAM_USAGE_RANGING"},
    {STREAM_USAGE_VOICE_MODEM_COMMUNICATION, "STREAM_USAGE_VOICE_MODEM_COMMUNICATION"},
    {STREAM_USAGE_VOICE_RINGTONE, "STREAM_USAGE_VOICE_RINGTONE"},
    {STREAM_USAGE_VOICE_CALL_ASSISTANT, "STREAM_USAGE_VOICE_CALL_ASSISTANT"},
    {STREAM_USAGE_ANNOUNCEMENT, "STREAM_USAGE_ANNOUNCEMENT"},
    {STREAM_USAGE_EMERGENCY, "STREAM_USAGE_EMERGENCY"},
};

const std::map<SourceType, std::string> SOURCE_TYPE_MAP = {
    {SOURCE_TYPE_INVALID, "SOURCE_TYPE_INVALID"},
    {SOURCE_TYPE_MIC, "SOURCE_TYPE_MIC"},
    {SOURCE_TYPE_VOICE_RECOGNITION, "SOURCE_TYPE_VOICE_RECOGNITION"},
    {SOURCE_TYPE_PLAYBACK_CAPTURE, "SOURCE_TYPE_PLAYBACK_CAPTURE"},
    {SOURCE_TYPE_WAKEUP, "SOURCE_TYPE_WAKEUP"},
    {SOURCE_TYPE_VOICE_CALL, "SOURCE_TYPE_VOICE_CALL"},
    {SOURCE_TYPE_VOICE_DOWNLINK, "SOURCE_TYPE_VOICE_DOWNLINK"},
    {SOURCE_TYPE_VOICE_COMMUNICATION, "SOURCE_TYPE_VOICE_COMMUNICATION"},
    {SOURCE_TYPE_ULTRASONIC, "SOURCE_TYPE_ULTRASONIC"},
    {SOURCE_TYPE_VIRTUAL_CAPTURE, "SOURCE_TYPE_VIRTUAL_CAPTURE"},
    {SOURCE_TYPE_VOICE_MESSAGE, "SOURCE_TYPE_VOICE_MESSAGE"},
    {SOURCE_TYPE_REMOTE_CAST, "SOURCE_TYPE_REMOTE_CAST"},
    {SOURCE_TYPE_VOICE_TRANSCRIPTION, "SOURCE_TYPE_VOICE_TRANSCRIPTION"},
    {SOURCE_TYPE_CAMCORDER, "SOURCE_TYPE_CAMCORDER"},
    {SOURCE_TYPE_UNPROCESSED, "SOURCE_TYPE_UNPROCESSED"},
    {SOURCE_TYPE_EC, "SOURCE_TYPE_EC"},
    {SOURCE_TYPE_MIC_REF, "SOURCE_TYPE_MIC_REF"},
    {SOURCE_TYPE_LIVE, "SOURCE_TYPE_LIVE"},
    {SOURCE_TYPE_OFFLOAD_CAPTURE, "SOURCE_TYPE_OFFLOAD_CAPTURE"},
    {SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT, "SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT"},
};

const std::map<AudioStreamType, std::string> AUDIO_STREAM_TYPE_MAP = {
    {STREAM_DEFAULT, "STREAM_DEFAULT"},
    {STREAM_VOICE_CALL, "STREAM_VOICE_CALL"},
    {STREAM_MUSIC, "STREAM_MUSIC"},
    {STREAM_RING, "STREAM_RING"},
    {STREAM_MEDIA, "STREAM_MEDIA"},
    {STREAM_VOICE_ASSISTANT, "STREAM_VOICE_ASSISTANT"},
    {STREAM_SYSTEM, "STREAM_SYSTEM"},
    {STREAM_ALARM, "STREAM_ALARM"},
    {STREAM_NOTIFICATION, "STREAM_NOTIFICATION"},
    {STREAM_BLUETOOTH_SCO, "STREAM_BLUETOOTH_SCO"},
    {STREAM_ENFORCED_AUDIBLE, "STREAM_ENFORCED_AUDIBLE"},
    {STREAM_DTMF, "STREAM_DTMF"},
    {STREAM_TTS, "STREAM_TTS"},

    {STREAM_RECORDING, "STREAM_RECORDING"},
    {STREAM_MOVIE, "STREAM_MOVIE"},
    {STREAM_GAME, "STREAM_GAME"},
    {STREAM_SPEECH, "STREAM_SPEECH"},
    {STREAM_SYSTEM_ENFORCED, "STREAM_SYSTEM_ENFORCED"},
    {STREAM_ULTRASONIC, "STREAM_ULTRASONIC"},
    {STREAM_WAKEUP, "STREAM_WAKEUP"},
    {STREAM_VOICE_MESSAGE, "STREAM_VOICE_MESSAGE"},
    {STREAM_NAVIGATION, "STREAM_NAVIGATION"},
    {STREAM_INTERNAL_FORCE_STOP, "STREAM_INTERNAL_FORCE_STOP"},
    {STREAM_SOURCE_VOICE_CALL, "STREAM_SOURCE_VOICE_CALL"},
    {STREAM_VOICE_COMMUNICATION, "STREAM_VOICE_COMMUNICATION"},
    {STREAM_VOICE_RING, "STREAM_VOICE_RING"},
    {STREAM_VOICE_CALL_ASSISTANT, "STREAM_VOICE_CALL_ASSISTANT"},
    {STREAM_CAMCORDER, "STREAM_CAMCORDER"},
    {STREAM_APP, "STREAM_APP"},
    {STREAM_ANNOUNCEMENT, "STREAM_ANNOUNCEMENT"},
    {STREAM_EMERGENCY, "STREAM_EMERGENCY"},
    {STREAM_ALL, "STREAM_ALL"},
};

const std::map<AudioPipeRole, std::string> AUDIO_PIPE_ROLE_MAP = {
    {PIPE_ROLE_OUTPUT, "PIPE_ROLE_OUTPUT"},
    {PIPE_ROLE_INPUT, "PIPE_ROLE_INPUT"},
    {PIPE_ROLE_NONE, "PIPE_ROLE_NONE"},
};

const std::map<AudioEffectMode, std::string> AUDIO_EFFECT_MODE_MAP = {
    {EFFECT_NONE, "EFFECT_NONE"},
    {EFFECT_DEFAULT, "EFFECT_DEFAULT"},
};

const std::map<InterruptHint, std::string> INTERRUPT_HINT_MAP = {
    {INTERRUPT_HINT_NONE, "INTERRUPT_HINT_NONE"},
    {INTERRUPT_HINT_RESUME, "INTERRUPT_HINT_RESUME"},
    {INTERRUPT_HINT_PAUSE, "INTERRUPT_HINT_PAUSE"},
    {INTERRUPT_HINT_STOP, "INTERRUPT_HINT_STOP"},
    {INTERRUPT_HINT_DUCK, "INTERRUPT_HINT_DUCK"},
    {INTERRUPT_HINT_UNDUCK, "INTERRUPT_HINT_UNDUCK"},
};

template<typename T>
std::string EnumToString(const std::map<T, std::string> &enumMap, T key)
{
    auto it = enumMap.find(key);
    if (it != enumMap.end()) {
        return it->second;
    }
    return "UNKNOWN(" + std::to_string(static_cast<int32_t>(key)) + ")";
}

std::string DeviceTypeToString(DeviceType type)
{
    return EnumToString(DEVICE_TYPE_MAP, type);
}

std::string AudioChannelToString(AudioChannel channel)
{
    return std::to_string(static_cast<int32_t>(channel));
}

std::string AudioSampleFormatToString(AudioSampleFormat format)
{
    return EnumToString(AUDIO_SAMPLE_FORMAT_MAP, format);
}

std::string AudioEncodingTypeToString(AudioEncodingType encoding)
{
    return EnumToString(AUDIO_ENCODING_TYPE_MAP, encoding);
}

std::string StreamUsageToString(StreamUsage usage)
{
    return EnumToString(STREAM_USAGE_MAP, usage);
}

std::string SourceTypeToString(SourceType source)
{
    return EnumToString(SOURCE_TYPE_MAP, source);
}

std::string AudioPipeRoleToString(AudioPipeRole role)
{
    return EnumToString(AUDIO_PIPE_ROLE_MAP, role);
}

std::string AudioStreamTypeToString(AudioStreamType type)
{
    return EnumToString(AUDIO_STREAM_TYPE_MAP, type);
}

std::string VolumeTypeToString(AudioStreamType type)
{
    if (type == STREAM_MUSIC) {
        return "MEDIA_STREAM";
    }
    return EnumToString(AUDIO_STREAM_TYPE_MAP, type);
}

std::string AudioEffectModeToString(AudioEffectMode mode)
{
    return EnumToString(AUDIO_EFFECT_MODE_MAP, mode);
}

std::string InterruptHintToString(InterruptHint hint)
{
    return EnumToString(INTERRUPT_HINT_MAP, hint);
}

const std::map<AudioLoopbackState, std::string> AUDIO_LOOPBACK_STATE_MAP = {
    {LOOPBACK_STATE_IDLE, "LOOPBACK_STATE_IDLE"},
    {LOOPBACK_STATE_PREPARED, "LOOPBACK_STATE_PREPARED"},
    {LOOPBACK_STATE_RUNNING, "LOOPBACK_STATE_RUNNING"},
    {LOOPBACK_STATE_DESTROYING, "LOOPBACK_STATE_DESTROYING"},
    {LOOPBACK_STATE_DESTROYED, "LOOPBACK_STATE_DESTROYED"},
};

const std::map<AudioLoopbackMode, std::string> AUDIO_LOOPBACK_MODE_MAP = {
    {LOOPBACK_HARDWARE, "LOOPBACK_HARDWARE"},
};

std::string AudioLoopbackStateToString(AudioLoopbackState state)
{
    return EnumToString(AUDIO_LOOPBACK_STATE_MAP, state);
}

std::string AudioLoopbackModeToString(AudioLoopbackMode mode)
{
    return EnumToString(AUDIO_LOOPBACK_MODE_MAP, mode);
}

const std::map<RendererState, std::string> RENDERER_STATE_MAP = {
    {RENDERER_INVALID, "RENDERER_INVALID"},
    {RENDERER_NEW, "RENDERER_NEW"},
    {RENDERER_PREPARED, "RENDERER_PREPARED"},
    {RENDERER_RUNNING, "RENDERER_RUNNING"},
    {RENDERER_STOPPED, "RENDERER_STOPPED"},
    {RENDERER_RELEASED, "RENDERER_RELEASED"},
    {RENDERER_PAUSED, "RENDERER_PAUSED"},
};

const std::map<CapturerState, std::string> CAPTURER_STATE_MAP = {
    {CAPTURER_INVALID, "CAPTURER_INVALID"},
    {CAPTURER_NEW, "CAPTURER_NEW"},
    {CAPTURER_PREPARED, "CAPTURER_PREPARED"},
    {CAPTURER_RUNNING, "CAPTURER_RUNNING"},
    {CAPTURER_STOPPED, "CAPTURER_STOPPED"},
    {CAPTURER_RELEASED, "CAPTURER_RELEASED"},
    {CAPTURER_PAUSED, "CAPTURER_PAUSED"},
};

const std::map<AudioLoopbackReverbPreset, std::string> REVERB_PRESET_MAP = {
    {REVERB_PRESET_ORIGINAL, "REVERB_PRESET_ORIGINAL"},
    {REVERB_PRESET_KTV, "REVERB_PRESET_KTV"},
    {REVERB_PRESET_THEATER, "REVERB_PRESET_THEATER"},
    {REVERB_PRESET_CONCERT, "REVERB_PRESET_CONCERT"},
};

const std::map<AudioLoopbackEqualizerPreset, std::string> EQUALIZER_PRESET_MAP = {
    {EQUALIZER_PRESET_FLAT, "EQUALIZER_PRESET_FLAT"},
    {EQUALIZER_PRESET_FULL, "EQUALIZER_PRESET_FULL"},
    {EQUALIZER_PRESET_BRIGHT, "EQUALIZER_PRESET_BRIGHT"},
};

std::string RendererStateToString(RendererState state)
{
    return EnumToString(RENDERER_STATE_MAP, state);
}

std::string CapturerStateToString(CapturerState state)
{
    return EnumToString(CAPTURER_STATE_MAP, state);
}

std::string ReverbPresetToString(AudioLoopbackReverbPreset preset)
{
    return EnumToString(REVERB_PRESET_MAP, preset);
}

std::string EqualizerPresetToString(AudioLoopbackEqualizerPreset preset)
{
    return EnumToString(EQUALIZER_PRESET_MAP, preset);
}

const std::map<AudioSessionState, std::string> AUDIO_SESSION_STATE_MAP = {
    {AudioSessionState::SESSION_INVALID, "SESSION_INVALID"},
    {AudioSessionState::SESSION_NEW, "SESSION_NEW"},
    {AudioSessionState::SESSION_ACTIVE, "SESSION_ACTIVE"},
    {AudioSessionState::SESSION_DEACTIVE, "SESSION_DEACTIVE"},
    {AudioSessionState::SESSION_RELEASED, "SESSION_RELEASED"},
};

const std::map<AudioConcurrencyMode, std::string> AUDIO_CONCURRENCY_MODE_MAP = {
    {AudioConcurrencyMode::INVALID, "INVALID"},
    {AudioConcurrencyMode::DEFAULT, "DEFAULT"},
    {AudioConcurrencyMode::MIX_WITH_OTHERS, "MIX_WITH_OTHERS"},
    {AudioConcurrencyMode::DUCK_OTHERS, "DUCK_OTHERS"},
    {AudioConcurrencyMode::PAUSE_OTHERS, "PAUSE_OTHERS"},
    {AudioConcurrencyMode::SILENT, "SILENT"},
    {AudioConcurrencyMode::STANDALONE, "STANDALONE"},
};

const std::map<AudioFocuState, std::string> AUDIO_FOCUS_STATE_MAP = {
    {AudioFocuState::ACTIVE, "ACTIVE"},
    {AudioFocuState::MUTED, "MUTED"},
    {AudioFocuState::DUCK, "DUCK"},
    {AudioFocuState::PAUSE, "PAUSE"},
    {AudioFocuState::STOP, "STOP"},
    {AudioFocuState::PLACEHOLDER, "PLACEHOLDER"},
    {AudioFocuState::PAUSEDBYREMOTE, "PAUSEDBYREMOTE"},
};

const std::map<AudioSessionScene, std::string> AUDIO_SESSION_SCENE_MAP = {
    {AudioSessionScene::INVALID, "INVALID"},
    {AudioSessionScene::MEDIA, "MEDIA"},
    {AudioSessionScene::GAME, "GAME"},
    {AudioSessionScene::VOICE_COMMUNICATION, "VOICE_COMMUNICATION"},
    {AudioSessionScene::VOICE_MESSAGE, "VOICE_MESSAGE"},
};

std::string AudioSessionStateToString(AudioSessionState state)
{
    return EnumToString(AUDIO_SESSION_STATE_MAP, state);
}

std::string AudioConcurrencyModeToString(AudioConcurrencyMode mode)
{
    return EnumToString(AUDIO_CONCURRENCY_MODE_MAP, mode);
}

std::string AudioFocusStateToString(AudioFocuState state)
{
    return EnumToString(AUDIO_FOCUS_STATE_MAP, state);
}

std::string AudioSessionSceneToString(AudioSessionScene scene)
{
    return EnumToString(AUDIO_SESSION_SCENE_MAP, scene);
}

const std::map<int32_t, std::string> AUDIO_FLAG_MAP = {
    {AUDIO_FLAG_NORMAL, "AUDIO_FLAG_NORMAL"},
    {AUDIO_FLAG_MMAP, "AUDIO_FLAG_MMAP"},
    {AUDIO_FLAG_VOIP_FAST, "AUDIO_FLAG_VOIP_FAST"},
    {AUDIO_FLAG_DIRECT, "AUDIO_FLAG_DIRECT"},
    {AUDIO_FLAG_VOIP_DIRECT, "AUDIO_FLAG_VOIP_DIRECT"},
    {AUDIO_FLAG_PCM_OFFLOAD, "AUDIO_FLAG_PCM_OFFLOAD"},
    {AUDIO_FLAG_3DA_DIRECT, "AUDIO_FLAG_3DA_DIRECT"},
    {AUDIO_FLAG_FORCED_NORMAL, "AUDIO_FLAG_FORCED_NORMAL"},
    {AUDIO_FLAG_ULTRA_FAST, "AUDIO_FLAG_ULTRA_FAST"},
    {AUDIO_FLAG_INTERPHONE_VOIP, "AUDIO_FLAG_INTERPHONE_VOIP"},
    {AUDIO_FLAG_VKB_NORMAL, "AUDIO_FLAG_VKB_NORMAL"},
    {AUDIO_FLAG_VKB_FAST, "AUDIO_FLAG_VKB_FAST"},
};

std::string AudioFlagToString(int32_t flag)
{
    auto it = AUDIO_FLAG_MAP.find(flag);
    if (it != AUDIO_FLAG_MAP.end()) {
        return it->second;
    }
    return "UNKNOWN(" + std::to_string(flag) + ")";
}

struct RouteFlagEntry {
    uint32_t bits;
    std::string name;
};

const RouteFlagEntry ROUTE_FLAG_ENTRIES[] = {
    {AUDIO_OUTPUT_FLAG_NORMAL, "OUTPUT_FLAG_NORMAL"},
    {AUDIO_OUTPUT_FLAG_DIRECT, "OUTPUT_FLAG_DIRECT"},
    {AUDIO_OUTPUT_FLAG_HD, "OUTPUT_FLAG_HD"},
    {AUDIO_OUTPUT_FLAG_MULTICHANNEL, "OUTPUT_FLAG_MULTICHANNEL"},
    {AUDIO_OUTPUT_FLAG_LOWPOWER, "OUTPUT_FLAG_LOWPOWER"},
    {AUDIO_OUTPUT_FLAG_FAST, "OUTPUT_FLAG_FAST"},
    {AUDIO_OUTPUT_FLAG_VOIP, "OUTPUT_FLAG_VOIP"},
    {AUDIO_OUTPUT_FLAG_VOIP_FAST, "OUTPUT_FLAG_VOIP_FAST"},
    {AUDIO_OUTPUT_FLAG_HWDECODING, "OUTPUT_FLAG_HWDECODING"},
    {AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD, "OUTPUT_FLAG_COMPRESS_OFFLOAD"},
    {AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION, "OUTPUT_FLAG_MODEM_COMMUNICATION"},
    {AUDIO_OUTPUT_FLAG_3DA_DIRECT, "OUTPUT_FLAG_3DA_DIRECT"},
    {AUDIO_OUTPUT_FLAG_INTERPHONE, "OUTPUT_FLAG_INTERPHONE"},
    {AUDIO_INPUT_FLAG_NORMAL, "INPUT_FLAG_NORMAL"},
    {AUDIO_INPUT_FLAG_FAST, "INPUT_FLAG_FAST"},
    {AUDIO_INPUT_FLAG_VOIP, "INPUT_FLAG_VOIP"},
    {AUDIO_INPUT_FLAG_VOIP_FAST, "INPUT_FLAG_VOIP_FAST"},
    {AUDIO_INPUT_FLAG_WAKEUP, "INPUT_FLAG_WAKEUP"},
    {AUDIO_INPUT_FLAG_AI, "INPUT_FLAG_AI"},
    {AUDIO_INPUT_FLAG_UNPROCESS, "INPUT_FLAG_UNPROCESS"},
    {AUDIO_INPUT_FLAG_ULTRASONIC, "INPUT_FLAG_ULTRASONIC"},
    {AUDIO_INPUT_FLAG_VOICE_RECOGNITION, "INPUT_FLAG_VOICE_RECOGNITION"},
    {AUDIO_INPUT_FLAG_RAW_AI, "INPUT_FLAG_RAW_AI"},
    {AUDIO_INPUT_FLAG_OFFLOAD, "INPUT_FLAG_OFFLOAD"},
    {AUDIO_INPUT_FLAG_INTERPHONE, "INPUT_FLAG_INTERPHONE"},
    {AUDIO_INPUT_FLAG_LIVE, "INPUT_FLAG_LIVE"},
    {AUDIO_INPUT_FLAG_CAMCORDER, "INPUT_FLAG_CAMCORDER"},
    {AUDIO_INPUT_FLAG_VOICE_MESSAGE, "INPUT_FLAG_VOICE_MESSAGE"},
};

std::string AudioRouteFlagToString(uint32_t routeFlag)
{
    CHECK_AND_RETURN_RET(routeFlag != AUDIO_FLAG_NONE, "FLAG_NONE");
    std::string result;
    for (const auto &entry : ROUTE_FLAG_ENTRIES) {
        if ((routeFlag & entry.bits) != 0) {
            if (!result.empty()) {
                result += "|";
            }
            result += entry.name;
        }
    }
    if (result.empty()) {
        result = "UNKNOWN(0x" + std::to_string(routeFlag) + ")";
    }
    return result;
}

const std::map<AudioChannelLayout, std::string> AUDIO_CHANNEL_LAYOUT_MAP = {
    {CH_LAYOUT_UNKNOWN, "CH_LAYOUT_UNKNOWN"},
    {CH_LAYOUT_MONO, "CH_LAYOUT_MONO"},
    {CH_LAYOUT_STEREO, "CH_LAYOUT_STEREO"},
    {CH_LAYOUT_STEREO_DOWNMIX, "CH_LAYOUT_STEREO_DOWNMIX"},
    {CH_LAYOUT_2POINT1, "CH_LAYOUT_2POINT1"},
    {CH_LAYOUT_3POINT0, "CH_LAYOUT_3POINT0"},
    {CH_LAYOUT_SURROUND, "CH_LAYOUT_SURROUND"},
    {CH_LAYOUT_3POINT1, "CH_LAYOUT_3POINT1"},
    {CH_LAYOUT_4POINT0, "CH_LAYOUT_4POINT0"},
    {CH_LAYOUT_QUAD_SIDE, "CH_LAYOUT_QUAD_SIDE"},
    {CH_LAYOUT_QUAD, "CH_LAYOUT_QUAD"},
    {CH_LAYOUT_2POINT0POINT2, "CH_LAYOUT_2POINT0POINT2"},
    {CH_LAYOUT_4POINT1, "CH_LAYOUT_4POINT1"},
    {CH_LAYOUT_5POINT0, "CH_LAYOUT_5POINT0"},
    {CH_LAYOUT_5POINT0_BACK, "CH_LAYOUT_5POINT0_BACK"},
    {CH_LAYOUT_2POINT1POINT2, "CH_LAYOUT_2POINT1POINT2"},
    {CH_LAYOUT_3POINT0POINT2, "CH_LAYOUT_3POINT0POINT2"},
    {CH_LAYOUT_5POINT1, "CH_LAYOUT_5POINT1"},
    {CH_LAYOUT_5POINT1_BACK, "CH_LAYOUT_5POINT1_BACK"},
    {CH_LAYOUT_6POINT0, "CH_LAYOUT_6POINT0"},
    {CH_LAYOUT_HEXAGONAL, "CH_LAYOUT_HEXAGONAL"},
    {CH_LAYOUT_3POINT1POINT2, "CH_LAYOUT_3POINT1POINT2"},
    {CH_LAYOUT_6POINT0_FRONT, "CH_LAYOUT_6POINT0_FRONT"},
    {CH_LAYOUT_6POINT1, "CH_LAYOUT_6POINT1"},
    {CH_LAYOUT_6POINT1_BACK, "CH_LAYOUT_6POINT1_BACK"},
    {CH_LAYOUT_6POINT1_FRONT, "CH_LAYOUT_6POINT1_FRONT"},
    {CH_LAYOUT_7POINT0, "CH_LAYOUT_7POINT0"},
    {CH_LAYOUT_7POINT0_FRONT, "CH_LAYOUT_7POINT0_FRONT"},
    {CH_LAYOUT_7POINT1, "CH_LAYOUT_7POINT1"},
    {CH_LAYOUT_OCTAGONAL, "CH_LAYOUT_OCTAGONAL"},
    {CH_LAYOUT_5POINT1POINT2, "CH_LAYOUT_5POINT1POINT2"},
    {CH_LAYOUT_7POINT1_WIDE, "CH_LAYOUT_7POINT1_WIDE"},
    {CH_LAYOUT_7POINT1_WIDE_BACK, "CH_LAYOUT_7POINT1_WIDE_BACK"},
    {CH_LAYOUT_5POINT1POINT4, "CH_LAYOUT_5POINT1POINT4"},
    {CH_LAYOUT_7POINT1POINT2, "CH_LAYOUT_7POINT1POINT2"},
    {CH_LAYOUT_7POINT1POINT4, "CH_LAYOUT_7POINT1POINT4"},
    {CH_LAYOUT_10POINT2, "CH_LAYOUT_10POINT2"},
    {CH_LAYOUT_9POINT1POINT4, "CH_LAYOUT_9POINT1POINT4"},
    {CH_LAYOUT_9POINT1POINT6, "CH_LAYOUT_9POINT1POINT6"},
    {CH_LAYOUT_HEXADECAGONAL, "CH_LAYOUT_HEXADECAGONAL"},
    {CH_LAYOUT_HOA_ORDER1_ACN_N3D, "CH_LAYOUT_HOA_ORDER1_ACN_N3D"},
    {CH_LAYOUT_HOA_ORDER1_ACN_SN3D, "CH_LAYOUT_HOA_ORDER1_ACN_SN3D"},
    {CH_LAYOUT_HOA_ORDER1_FUMA, "CH_LAYOUT_HOA_ORDER1_FUMA"},
    {CH_LAYOUT_HOA_ORDER2_ACN_N3D, "CH_LAYOUT_HOA_ORDER2_ACN_N3D"},
    {CH_LAYOUT_HOA_ORDER2_ACN_SN3D, "CH_LAYOUT_HOA_ORDER2_ACN_SN3D"},
    {CH_LAYOUT_HOA_ORDER2_FUMA, "CH_LAYOUT_HOA_ORDER2_FUMA"},
    {CH_LAYOUT_HOA_ORDER3_ACN_N3D, "CH_LAYOUT_HOA_ORDER3_ACN_N3D"},
    {CH_LAYOUT_HOA_ORDER3_ACN_SN3D, "CH_LAYOUT_HOA_ORDER3_ACN_SN3D"},
    {CH_LAYOUT_HOA_ORDER3_FUMA, "CH_LAYOUT_HOA_ORDER3_FUMA"},
};

std::string AudioChannelLayoutToString(AudioChannelLayout layout)
{
    auto it = AUDIO_CHANNEL_LAYOUT_MAP.find(layout);
    CHECK_AND_RETURN_RET(it == AUDIO_CHANNEL_LAYOUT_MAP.end(), it->second);
    return "UNKNOWN(0x" + std::to_string(static_cast<uint64_t>(layout)) + ")";
}

void AppendFocusHistory(std::stringstream &ss, const std::vector<AudioFocusBehavior> &focusHistory)
{
    ss << "    focusHistory: [\n";
    for (const auto &item : focusHistory) {
        ss << "      { streamId: " << item.streamId
           << ", pid: " << item.pid
           << ", uid: " << item.uid
           << ", streamType: " << AudioStreamTypeToString(item.streamType)
           << ", sourceType: " << SourceTypeToString(item.sourceType)
           << ", hintType: " << InterruptHintToString(item.hintType)
           << ", timestamp: " << item.timestamp << " },\n";
    }
    ss << "    ]\n";
}

void AppendErrorInfos(std::stringstream &ss, const std::vector<AudioFaultRecord> &errorInfos)
{
    ss << "  errorInfos: [\n";
    for (const auto &record : errorInfos) {
        ss << "    { uid: " << record.uid
           << ", errorCode: " << record.errorCode
           << ", errorReason: " << record.errorReason
           << ", timestamp: " << record.timestamp << " },\n";
    }
    ss << "  ]\n";
}
} // namespace

AudioDebugManager &AudioDebugManager::GetInstance()
{
    static AudioDebugManagerPrivate instance;
    return instance;
}

void AudioDebugManagerPrivate::LogAudioDebugInfo(const std::string &debugInfo)
{
    AUDIO_INFO_LOG("%{public}s", debugInfo.c_str());
}

int32_t AudioDebugManagerPrivate::WriteAudioDebugInfo(int32_t fd, const std::string &debugInfo)
{
    std::string output = debugInfo + "\n";
    const char *buffer = output.c_str();
    size_t remainSize = output.size();
    while (remainSize > 0) {
        ssize_t writeSize = write(fd, buffer, remainSize);
        CHECK_AND_RETURN_RET_LOG(writeSize > 0, ERR_OPERATION_FAILED,
            "Write debug info failed, fd:%{public}d, writeSize:%{public}zd", fd, writeSize);
        buffer += writeSize;
        remainSize -= static_cast<size_t>(writeSize);
    }
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintfAudioDebugInfo(int32_t fd, const std::string &debugInfo)
{
    if (fd < 0) {
        AUDIO_INFO_LOG("fd is invalid, fallback to log output");
        LogAudioDebugInfo(debugInfo);
        return SUCCESS;
    }
    return WriteAudioDebugInfo(fd, debugInfo);
}

uint32_t AudioDebugManagerPrivate::GenerateDebugKey()
{
    return ++debugKeyCounter_;
}

int32_t AudioDebugManagerPrivate::PrintAppAudioDebugInfo(int32_t fd) const
{
    AUDIO_INFO_LOG("Print app audio debug info start, fd:%{public}d", fd);
    int32_t ret = SUCCESS;
    ret = PrintAllAudioRenderersDebugInfo(fd) != SUCCESS ? ERR_OPERATION_FAILED : ret;
    ret = PrintAllAudioCapturersDebugInfo(fd) != SUCCESS ? ERR_OPERATION_FAILED : ret;
    ret = PrintAllAudioLoopbacksDebugInfo(fd) != SUCCESS ? ERR_OPERATION_FAILED : ret;
    ret = PrintAudioSessionDebugInfo(fd) != SUCCESS ? ERR_OPERATION_FAILED : ret;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Print app audio debug info finished with partial failure");
    return ret;
}

int32_t AudioDebugManagerPrivate::GetAudioRendererDebugInfo(uint32_t rendererKey,
    AudioRendererDebugInfo &debugInfo) const
{
    std::shared_ptr<AudioRendererDebugCallback> debugCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(rendererMutex_);
        auto callbackIter = audioRendererDebugCallbacks_.find(rendererKey);
        CHECK_AND_RETURN_RET_LOG(callbackIter != audioRendererDebugCallbacks_.end() && callbackIter->second != nullptr,
            ERR_ILLEGAL_STATE, "Get renderer debug info failed: callback not found, key:%{public}u", rendererKey);
        debugCallback = callbackIter->second;
    }

    int32_t ret = debugCallback->GetRendererDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Get renderer debug info failed, ret:%{public}d", ret);
    return ret;
}

std::string AudioDebugManagerPrivate::ParserAudioRendererDebugInfo(
    const AudioRendererDebugInfo &debugInfo)
{
    std::stringstream ss;
    ss << "audioRenderer {\n"
       << "  streamInfo: {\n"
       << "    streamId: " << debugInfo.sessionId << ",\n"
       << "    samplingRate: " << debugInfo.samplingRate << ",\n"
       << "    channels: " << AudioChannelToString(debugInfo.channels) << ",\n"
       << "    format: " << AudioSampleFormatToString(debugInfo.format) << ",\n"
       << "    encoding: " << AudioEncodingTypeToString(debugInfo.encoding) << ",\n"
       << "    streamUsage: " << StreamUsageToString(debugInfo.streamUsage) << ",\n"
       << "    rendererFlags: " << AudioFlagToString(debugInfo.rendererFlags) << "\n"
       << "  },\n"
       << "  pipeInfo: {\n"
       << "    pipeId: " << debugInfo.pipeId << ",\n"
       << "    pipeRole: " << AudioPipeRoleToString(debugInfo.pipeRole) << ",\n"
       << "    pipeName: " << debugInfo.pipeName << ",\n"
       << "    routeFlag: " << AudioRouteFlagToString(debugInfo.routeFlag) << ",\n"
       << "    adapterName: " << debugInfo.adapterName << ",\n"
       << "    pipeFormat: " << debugInfo.pipeFormat << ",\n"
       << "    pipeRate: " << debugInfo.pipeRate << ",\n"
       << "    pipeChannels: " << debugInfo.pipeChannels << ",\n"
       << "    pipeChannelLayout: " << debugInfo.pipeChannelLayout << ",\n"
       << "    pipeDeviceType: " << debugInfo.pipeDeviceType << "\n"
       << "  },\n"
       << "  deviceInfo: {\n"
       << "    mainDeviceType: " << DeviceTypeToString(debugInfo.mainDeviceType) << "\n"
       << "  },\n"
       << "  volumeInfo: {\n"
       << "    volumeType: " << VolumeTypeToString(debugInfo.volumeType) << ",\n"
       << "    streamVolume: " << debugInfo.streamVolume << ",\n"
       << "    systemVolume: " << debugInfo.systemVolume << ",\n"
       << "    volume: " << debugInfo.volume << "\n"
       << "  },\n"
       << "  effectInfo: {\n"
       << "    speed: " << debugInfo.speed << ",\n"
       << "    pitch: " << debugInfo.pitch << ",\n"
       << "    effectMode: " << AudioEffectModeToString(debugInfo.effectMode) << "\n"
       << "  },\n"
       << "  focusInfo: {\n"
       << "    focusState: " << AudioFocusStateToString(debugInfo.focusState) << ",\n";
    AppendFocusHistory(ss, debugInfo.focusHistory);
    ss << "  },\n";
    AppendErrorInfos(ss, debugInfo.errorInfos);
    ss << "}";
    return ss.str();
}

int32_t AudioDebugManagerPrivate::RegisterAudioRenderer(uint32_t &rendererKey,
    const std::shared_ptr<AudioRendererDebugCallback> &debugCallback)
{
    CHECK_AND_RETURN_RET_LOG(debugCallback != nullptr, ERR_INVALID_PARAM,
        "Register renderer failed: debugCallback is nullptr");

    rendererKey = GenerateDebugKey();
    std::lock_guard<std::mutex> lock(rendererMutex_);
    audioRendererDebugCallbacks_[rendererKey] = debugCallback;
    AUDIO_INFO_LOG("Register renderer success, key:%{public}u, total:%{public}zu", rendererKey,
        audioRendererDebugCallbacks_.size());
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::UnregisterAudioRenderer(uint32_t rendererKey)
{
    std::lock_guard<std::mutex> lock(rendererMutex_);
    auto it = audioRendererDebugCallbacks_.find(rendererKey);
    CHECK_AND_RETURN_RET_LOG(it != audioRendererDebugCallbacks_.end(), ERR_ILLEGAL_STATE,
        "Unregister renderer failed: key not found, key:%{public}u", rendererKey);
    audioRendererDebugCallbacks_.erase(it);
    AUDIO_INFO_LOG("Unregister renderer success, key:%{public}u, total:%{public}zu", rendererKey,
        audioRendererDebugCallbacks_.size());
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAudioRendererDebugInfo(uint32_t rendererKey, int32_t fd) const
{
    AudioRendererDebugInfo debugInfo;
    CHECK_AND_RETURN_RET_LOG(GetAudioRendererDebugInfo(rendererKey, debugInfo) == SUCCESS, ERR_ILLEGAL_STATE,
        "Print renderer debug info failed: get info failed, key:%{public}u", rendererKey);
    const std::string parsedDebugInfo = ParserAudioRendererDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(PrintfAudioDebugInfo(fd, parsedDebugInfo) == SUCCESS, ERR_OPERATION_FAILED,
        "Print renderer debug info failed: output failed, key:%{public}u", rendererKey);
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAllAudioRenderersDebugInfo(int32_t fd) const
{
    std::set<uint32_t> rendererKeys;
    {
        std::lock_guard<std::mutex> lock(rendererMutex_);
        for (const auto &item : audioRendererDebugCallbacks_) {
            rendererKeys.insert(item.first);
        }
    }
    AUDIO_INFO_LOG("Print all renderer debug info, count:%{public}zu, fd:%{public}d", rendererKeys.size(), fd);

    int32_t ret = SUCCESS;
    for (const auto key : rendererKeys) {
        if (PrintAudioRendererDebugInfo(key, fd) != SUCCESS) {
            AUDIO_WARNING_LOG("Print renderer debug info failed in batch, key:%{public}u", key);
            ret = ERR_OPERATION_FAILED;
        }
    }
    return ret;
}

int32_t AudioDebugManagerPrivate::GetAudioCapturerDebugInfo(uint32_t capturerKey,
    AudioCapturerDebugInfo &debugInfo) const
{
    std::shared_ptr<AudioCapturerDebugCallback> debugCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        auto callbackIter = audioCapturerDebugCallbacks_.find(capturerKey);
        CHECK_AND_RETURN_RET_LOG(callbackIter != audioCapturerDebugCallbacks_.end() && callbackIter->second != nullptr,
            ERR_ILLEGAL_STATE, "Get capturer debug info failed: callback not found, key:%{public}u", capturerKey);
        debugCallback = callbackIter->second;
    }

    int32_t ret = debugCallback->GetCapturerDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Get capturer debug info failed, ret:%{public}d", ret);
    return ret;
}

std::string AudioDebugManagerPrivate::ParserAudioCapturerDebugInfo(
    const AudioCapturerDebugInfo &debugInfo)
{
    std::stringstream ss;
    const auto &pipeStream = debugInfo.pipeInfo;
    const auto &timestamp = debugInfo.captureTimestamp;
    const auto deviceType = debugInfo.inputDeviceInfo.getType();
    ss << "audioCapturer {\n"
       << "  streamInfo: {\n"
       << "    streamId: " << debugInfo.sessionId << ",\n"
       << "    samplingRate: " << debugInfo.samplingRate << ",\n"
       << "    channels: " << AudioChannelToString(debugInfo.channels) << ",\n"
       << "    format: " << AudioSampleFormatToString(debugInfo.format) << ",\n"
       << "    encoding: " << AudioEncodingTypeToString(debugInfo.encoding) << ",\n"
       << "    channelLayout: " << AudioChannelLayoutToString(debugInfo.channelLayout) << ",\n"
       << "    sourceType: " << SourceTypeToString(debugInfo.sourceType) << ",\n"
       << "    capturerFlag: " << AudioFlagToString(debugInfo.capturerFlag) << "\n"
       << "  },\n"
       << "  captureInfo: {\n"
       << "    timestamp(frame: " << timestamp.framePosition
       << ", sec: " << timestamp.time.tv_sec
       << ", nsec: " << timestamp.time.tv_nsec << "),\n"
       << "    bufferSize: " << debugInfo.bufferSize << ",\n"
       << "    overflowCount: " << debugInfo.overflowCount << ",\n"
       << "    muteWhenInterrupted: " << (debugInfo.muteWhenInterrupted ? "true" : "false") << ",\n"
       << "    pauseWhenInterrupted: " << (debugInfo.pauseWhenInterrupted ? "true" : "false") << ",\n"
       << "    inputDeviceType: " << static_cast<int32_t>(deviceType) << "\n"
       << "  },\n"
       << "  pipeInfo: {\n"
       << "    pipeRole: " << AudioPipeRoleToString(debugInfo.pipeRole) << ",\n"
       << "    samplingRate: " << static_cast<int32_t>(pipeStream.samplingRate) << ",\n"
       << "    channels: " << AudioChannelToString(pipeStream.channels) << ",\n"
       << "    format: " << AudioSampleFormatToString(pipeStream.format) << ",\n"
       << "    encoding: " << AudioEncodingTypeToString(pipeStream.encoding) << ",\n"
       << "    channelLayout: " << AudioChannelLayoutToString(pipeStream.channelLayout) << "\n"
       << "  },\n"
       << "  focusInfo: {\n"
       << "    focusState: " << AudioFocusStateToString(debugInfo.focusState) << ",\n";
    AppendFocusHistory(ss, debugInfo.focusHistory);
    ss << "  },\n";
    AppendErrorInfos(ss, debugInfo.errorInfos);
    ss << "}";
    return ss.str();
}

int32_t AudioDebugManagerPrivate::RegisterAudioCapturer(uint32_t &capturerKey,
    const std::shared_ptr<AudioCapturerDebugCallback> &debugCallback)
{
    CHECK_AND_RETURN_RET_LOG(debugCallback != nullptr, ERR_INVALID_PARAM,
        "Register capturer failed: debugCallback is nullptr");

    capturerKey = GenerateDebugKey();
    std::lock_guard<std::mutex> lock(capturerMutex_);
    audioCapturerDebugCallbacks_[capturerKey] = debugCallback;
    AUDIO_INFO_LOG("Register capturer success, key:%{public}u, total:%{public}zu", capturerKey,
        audioCapturerDebugCallbacks_.size());
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::UnregisterAudioCapturer(uint32_t capturerKey)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    auto it = audioCapturerDebugCallbacks_.find(capturerKey);
    CHECK_AND_RETURN_RET_LOG(it != audioCapturerDebugCallbacks_.end(), ERR_ILLEGAL_STATE,
        "Unregister capturer failed: key not found, key:%{public}u", capturerKey);
    audioCapturerDebugCallbacks_.erase(it);
    AUDIO_INFO_LOG("Unregister capturer success, key:%{public}u, total:%{public}zu", capturerKey,
        audioCapturerDebugCallbacks_.size());
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAudioCapturerDebugInfo(uint32_t capturerKey, int32_t fd) const
{
    AudioCapturerDebugInfo debugInfo;
    CHECK_AND_RETURN_RET_LOG(GetAudioCapturerDebugInfo(capturerKey, debugInfo) == SUCCESS, ERR_ILLEGAL_STATE,
        "Print capturer debug info failed: get info failed, key:%{public}u", capturerKey);
    const std::string parsedDebugInfo = ParserAudioCapturerDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(PrintfAudioDebugInfo(fd, parsedDebugInfo) == SUCCESS, ERR_OPERATION_FAILED,
        "Print capturer debug info failed: output failed, key:%{public}u", capturerKey);
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAllAudioCapturersDebugInfo(int32_t fd) const
{
    std::set<uint32_t> capturerKeys;
    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        for (const auto &item : audioCapturerDebugCallbacks_) {
            capturerKeys.insert(item.first);
        }
    }
    AUDIO_INFO_LOG("Print all capturer debug info, count:%{public}zu, fd:%{public}d", capturerKeys.size(), fd);

    int32_t ret = SUCCESS;
    for (const auto key : capturerKeys) {
        if (PrintAudioCapturerDebugInfo(key, fd) != SUCCESS) {
            AUDIO_WARNING_LOG("Print capturer debug info failed in batch, key:%{public}u", key);
            ret = ERR_OPERATION_FAILED;
        }
    }
    return ret;
}

int32_t AudioDebugManagerPrivate::GetAudioLoopbackDebugInfo(uint32_t loopbackKey,
    AudioLoopbackDebugInfo &debugInfo) const
{
    std::shared_ptr<AudioLoopbackDebugCallback> debugCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(loopbackMutex_);
        auto callbackIter = audioLoopbackDebugCallbacks_.find(loopbackKey);
        CHECK_AND_RETURN_RET_LOG(callbackIter != audioLoopbackDebugCallbacks_.end() && callbackIter->second != nullptr,
            ERR_ILLEGAL_STATE, "Get loopback debug info failed: callback not found, key:%{public}u", loopbackKey);
        debugCallback = callbackIter->second;
    }

    int32_t ret = debugCallback->GetLoopbackDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Get loopback debug info failed, ret:%{public}d", ret);
    return ret;
}

std::string AudioDebugManagerPrivate::ParserAudioLoopbackDebugInfo(
    const AudioLoopbackDebugInfo &debugInfo)
{
    std::stringstream ss;
    ss << "audioLoopback {\n"
       << "  appInfo: {\n"
       << "    appUid: " << debugInfo.appUid << ",\n"
       << "    appName: " << debugInfo.appName << "\n"
       << "  },\n"
       << "  statusInfo: {\n"
       << "    mode: " << AudioLoopbackModeToString(debugInfo.mode) << ",\n"
       << "    currentState: " << AudioLoopbackStateToString(debugInfo.currentState) << "\n"
       << "  },\n"
       << "  deviceInfo: {\n"
       << "    activeOutputDevice: " << DeviceTypeToString(debugInfo.activeOutputDevice) << ",\n"
       << "    activeInputDevice: " << DeviceTypeToString(debugInfo.activeInputDevice) << "\n"
       << "  },\n"
       << "  effectInfo: {\n"
       << "    reverbPreset: " << ReverbPresetToString(debugInfo.reverbPreset) << ",\n"
       << "    equalizerPreset: " << EqualizerPresetToString(debugInfo.equalizerPreset) << ",\n"
       << "    volume: " << debugInfo.volume << "\n"
       << "  },\n"
       << "  streamInfo: {\n"
       << "    uplinkStreamState: " << CapturerStateToString(debugInfo.uplinkStreamState) << ",\n"
       << "    downlinkStreamState: " << RendererStateToString(debugInfo.downlinkStreamState) << "\n"
       << "  }\n"
       << "}";
    return ss.str();
}

int32_t AudioDebugManagerPrivate::RegisterAudioLoopback(uint32_t &loopbackKey,
    const std::shared_ptr<AudioLoopbackDebugCallback> &debugCallback)
{
    CHECK_AND_RETURN_RET_LOG(debugCallback != nullptr, ERR_INVALID_PARAM,
        "Register loopback failed: debugCallback is nullptr");
    loopbackKey = GenerateDebugKey();
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    audioLoopbackDebugCallbacks_[loopbackKey] = debugCallback;
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::UnregisterAudioLoopback(uint32_t loopbackKey)
{
    std::lock_guard<std::mutex> lock(loopbackMutex_);
    audioLoopbackDebugCallbacks_.erase(loopbackKey);
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAudioLoopbackDebugInfo(uint32_t loopbackKey, int32_t fd) const
{
    AudioLoopbackDebugInfo debugInfo;
    CHECK_AND_RETURN_RET_LOG(GetAudioLoopbackDebugInfo(loopbackKey, debugInfo) == SUCCESS, ERR_ILLEGAL_STATE,
        "Print loopback debug info failed: get info failed, key:%{public}u", loopbackKey);
    const std::string parsedDebugInfo = ParserAudioLoopbackDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(PrintfAudioDebugInfo(fd, parsedDebugInfo) == SUCCESS, ERR_OPERATION_FAILED,
        "Print loopback debug info failed: output failed, key:%{public}u", loopbackKey);
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAllAudioLoopbacksDebugInfo(int32_t fd) const
{
    std::set<uint32_t> loopbackKeys;
    {
        std::lock_guard<std::mutex> lock(loopbackMutex_);
        for (const auto &item : audioLoopbackDebugCallbacks_) {
            loopbackKeys.insert(item.first);
        }
    }
    AUDIO_INFO_LOG("Print all loopback debug info, count:%{public}zu, fd:%{public}d", loopbackKeys.size(), fd);

    int32_t ret = SUCCESS;
    for (const auto key : loopbackKeys) {
        if (PrintAudioLoopbackDebugInfo(key, fd) != SUCCESS) {
            AUDIO_WARNING_LOG("Print loopback debug info failed in batch, key:%{public}u", key);
            ret = ERR_OPERATION_FAILED;
        }
    }
    return ret;
}

int32_t AudioDebugManagerPrivate::GetAudioSessionDebugInfo(AudioSessionDebugInfo &debugInfo) const
{
    std::shared_ptr<AudioSessionDebugCallback> debugCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(sessionMutex_);
        CHECK_AND_RETURN_RET_LOG(audioSessionDebugCallback_ != nullptr, ERR_ILLEGAL_STATE,
            "Get session debug info failed: callback not registered");
        debugCallback = audioSessionDebugCallback_;
    }
    int32_t ret = debugCallback->GetSessionDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "Get session debug info failed, ret:%{public}d", ret);
    return ret;
}

std::string AudioDebugManagerPrivate::ParserAudioSessionDebugInfo(
    const AudioSessionDebugInfo &debugInfo)
{
    std::stringstream ss;
    ss << "audioSession {\n"
    << "  strategy: " << AudioConcurrencyModeToString(debugInfo.strategy.concurrencyMode) << ",\n"
    << "  audioSessionScene: " << AudioSessionSceneToString(debugInfo.audioSessionScene) << ",\n"
    << "  defaultDeviceType: " << DeviceTypeToString(debugInfo.defaultDeviceType) << ",\n"
    << "  state: " << AudioSessionStateToString(debugInfo.state) << ",\n";
    if (debugInfo.audioSessionScene != AudioSessionScene::INVALID) {
        ss << "  fakeFocusState: " << AudioFocusStateToString(debugInfo.fakeFocusState) << ",\n";
    }
    ss << "  pid: " << debugInfo.pid << ",\n"
    << "  uid: " << debugInfo.uid << ",\n"
    << "  streams: [\n";
    for (const auto &stream : debugInfo.streamInfos) {
        ss << "    {\n"
        << "      streamId: " << stream.streamId << ",\n";
        if (debugInfo.audioSessionScene == AudioSessionScene::INVALID) {
            ss << "      focusState: " << AudioFocusStateToString(stream.focusState) << ",\n";
        }
        ss << "      streamType: " << AudioStreamTypeToString(stream.streamType) << "\n"
        << "    },\n";
    }
    ss << "  ]\n"
    << "}";
    return ss.str();
}

int32_t AudioDebugManagerPrivate::SetAudioSessionCallback(
    const std::shared_ptr<AudioSessionDebugCallback> &debugCallback)
{
    CHECK_AND_RETURN_RET_LOG(debugCallback != nullptr, ERR_INVALID_PARAM,
        "Set session callback failed: debugCallback is nullptr");

    std::lock_guard<std::mutex> lock(sessionMutex_);
    audioSessionDebugCallback_ = debugCallback;
    return SUCCESS;
}

int32_t AudioDebugManagerPrivate::PrintAudioSessionDebugInfo(int32_t fd) const
{
    AudioSessionDebugInfo debugInfo;
    CHECK_AND_RETURN_RET_LOG(GetAudioSessionDebugInfo(debugInfo) == SUCCESS, ERR_ILLEGAL_STATE,
        "Print session debug info failed: get info failed");
    const std::string parsedDebugInfo = ParserAudioSessionDebugInfo(debugInfo);
    CHECK_AND_RETURN_RET_LOG(PrintfAudioDebugInfo(fd, parsedDebugInfo) == SUCCESS, ERR_OPERATION_FAILED,
        "Print session debug info failed: output failed");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
