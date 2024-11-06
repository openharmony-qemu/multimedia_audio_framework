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
#ifndef LOG_TAG
#define LOG_TAG "AudioServer"
#endif

#include "audio_server.h"

#include <cinttypes>
#include <codecvt>
#include <csignal>
#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hisysevent.h"

#include "audio_capturer_source.h"
#include "fast_audio_capturer_source.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_asr.h"
#include "audio_manager_listener_proxy.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "i_audio_capturer_source.h"
#include "i_audio_renderer_sink.h"
#include "audio_renderer_sink.h"
#include "i_standard_audio_server_manager_listener.h"
#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain_manager.h"
#include "playback_capturer_manager.h"
#include "policy_handler.h"
#include "config/audio_param_parser.h"
#include "media_monitor_manager.h"

static const std::map<std::string, AsrAecMode> AEC_MODE_MAP = {
    {"BYPASS", AsrAecMode::BYPASS},
    {"STANDARD", AsrAecMode::STANDARD}
};

static const std::map<AsrAecMode, std::string> AEC_MODE_MAP_VERSE = {
    {AsrAecMode::BYPASS, "BYPASS"},
    {AsrAecMode::STANDARD, "STANDARD"}
};

static const std::map<std::string, AsrNoiseSuppressionMode> NS_MODE_MAP = {
    {"BYPASS", AsrNoiseSuppressionMode::BYPASS},
    {"STANDARD", AsrNoiseSuppressionMode::STANDARD},
    {"NEAR_FIELD", AsrNoiseSuppressionMode::NEAR_FIELD},
    {"FAR_FIELD", AsrNoiseSuppressionMode::FAR_FIELD}
};

static const std::map<AsrNoiseSuppressionMode, std::string> NS_MODE_MAP_VERSE = {
    {AsrNoiseSuppressionMode::BYPASS, "BYPASS"},
    {AsrNoiseSuppressionMode::STANDARD, "STANDARD"},
    {AsrNoiseSuppressionMode::NEAR_FIELD, "NEAR_FIELD"},
    {AsrNoiseSuppressionMode::FAR_FIELD, "FAR_FIELD"}
};

static const std::map<std::string, AsrWhisperDetectionMode> WHISPER_DETECTION_MODE_MAP = {
    {"BYPASS", AsrWhisperDetectionMode::BYPASS},
    {"STANDARD", AsrWhisperDetectionMode::STANDARD},
};

static const std::map<AsrWhisperDetectionMode, std::string> WHISPER_DETECTION_MODE_MAP_VERSE = {
    {AsrWhisperDetectionMode::BYPASS, "BYPASS"},
    {AsrWhisperDetectionMode::STANDARD, "STANDARD"},
};

static const std::map<std::string, AsrVoiceControlMode> VC_MODE_MAP = {
    {"audio2voicetx", AsrVoiceControlMode::AUDIO_2_VOICETX},
    {"audiomix2voicetx", AsrVoiceControlMode::AUDIO_MIX_2_VOICETX},
    {"audio2voicetxex", AsrVoiceControlMode::AUDIO_2_VOICE_TX_EX},
    {"audiomix2voicetxex", AsrVoiceControlMode::AUDIO_MIX_2_VOICE_TX_EX},
};

static const std::map<AsrVoiceControlMode, std::string> VC_MODE_MAP_VERSE = {
    {AsrVoiceControlMode::AUDIO_2_VOICETX, "audio2voicetx"},
    {AsrVoiceControlMode::AUDIO_MIX_2_VOICETX, "audiomix2voicetx"},
    {AsrVoiceControlMode::AUDIO_2_VOICE_TX_EX, "audio2voicetxex"},
    {AsrVoiceControlMode::AUDIO_MIX_2_VOICE_TX_EX, "audiomix2voicetxex"},
};

static const std::map<AsrVoiceControlMode, std::vector<std::string>> VOICE_CALL_ASSISTANT_SUPPRESSION = {
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_OPPOSITE, {"TTS_2_DEVICE", "TTS_2_MODEM"}},
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_LOCAL, {"TTS_2_DEVICE", "TTS_2_MODEM"}},
    {AsrVoiceControlMode::VOICE_TXRX_DECREASE, {"MIC_2_MODEM", "MODEM_2_DEVICE"}},
};

static const std::map<AsrVoiceControlMode, std::set<std::string>> VOICE_CALL_ASSISTANT_NEED_SUPPRESSION = {
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_OPPOSITE, {"TTS_2_MODEM"}},
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_LOCAL, {"TTS_2_DEVICE"}},
    {AsrVoiceControlMode::VOICE_TXRX_DECREASE, {"MIC_2_MODEM", "MODEM_2_DEVICE"}},
};

static const std::string VOICE_CALL_SUPPRESSION_VOLUME = "3";
static const std::string VOICE_CALL_FULL_VOLUME = "32";

static const std::map<std::string, AsrVoiceMuteMode> VM_MODE_MAP = {
    {"output_mute", AsrVoiceMuteMode::OUTPUT_MUTE},
    {"input_mute", AsrVoiceMuteMode::INPUT_MUTE},
    {"mute_tts", AsrVoiceMuteMode::TTS_MUTE},
    {"mute_call", AsrVoiceMuteMode::CALL_MUTE},
    {"ouput_mute_ex", AsrVoiceMuteMode::OUTPUT_MUTE_EX},
};

static const std::map<AsrVoiceMuteMode, std::string> VM_MODE_MAP_VERSE = {
    {AsrVoiceMuteMode::OUTPUT_MUTE, "output_mute"},
    {AsrVoiceMuteMode::INPUT_MUTE, "input_mute"},
    {AsrVoiceMuteMode::TTS_MUTE, "mute_tts"},
    {AsrVoiceMuteMode::CALL_MUTE, "mute_call"},
    {AsrVoiceMuteMode::OUTPUT_MUTE_EX, "ouput_mute_ex"},
};

static const std::map<std::string, bool> RES_MAP = {
    {"true", true},
    {"false", false},
};

static const std::map<bool, std::string> RES_MAP_VERSE = {
    {true, "true"},
    {false, "false"},
};

using namespace std;

namespace OHOS {
namespace AudioStandard {

std::vector<std::string> splitString(const std::string& str, const std::string& pattern)
{
    std::vector<std::string> res;
    if (str == "")
        return res;
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos) {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }
    return res;
}

int32_t AudioServer::SetAsrAecMode(AsrAecMode asrAecMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_aec_mode";
    std::string value = key + "=";
    std::string keyAec = "ASR_AEC";
    std::string valueAec = "";

    auto it = AEC_MODE_MAP_VERSE.find(asrAecMode);
    if (it != AEC_MODE_MAP_VERSE.end()) {
        value = key + "=" + it->second;
        if (it->second == "STANDARD") {
            valueAec = "ASR_AEC=ON";
        } else {
            valueAec = "ASR_AEC=OFF";
        }
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioServer::audioParameters[keyAec] = valueAec;
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", valueAec);
    return 0;
}

int32_t AudioServer::GetAsrAecMode(AsrAecMode& asrAecMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_aec_mode";
    std::string keyAec = "ASR_AEC";
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    std::string asrAecModeSink = audioRendererSinkInstance->GetAudioParameter(parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrAecModeSink = it->second;
    } else {
        // if asr_aec_mode null, return ASR_AEC.
        // if asr_aec_mode null and ASR_AEC null, return err.
        auto itAec = AudioServer::audioParameters.find(keyAec);
        std::string asrAecSink = itAec->second;
        if (asrAecSink == "ASR_AEC=ON") {
            asrAecMode = AsrAecMode::STANDARD;
        } else if (asrAecSink == "ASR_AEC=OFF") {
            asrAecMode = AsrAecMode::BYPASS;
        } else {
            AUDIO_ERR_LOG("get value failed.");
            return ERR_INVALID_PARAM;
        }
        return 0;
    }

    std::vector<std::string> resMode = splitString(asrAecModeSink, "=");
    const int32_t resSize = 2;
    std::string modeString = "";
    if (resMode.size() == resSize) {
        modeString = resMode[1];
        auto it = AEC_MODE_MAP.find(modeString);
        if (it != AEC_MODE_MAP.end()) {
            asrAecMode = it->second;
        } else {
            AUDIO_ERR_LOG("get value failed.");
            return ERR_INVALID_PARAM;
        }
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    return 0;
}

int32_t AudioServer::SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode asrNoiseSuppressionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_ns_mode";
    std::string value = key + "=";

    auto it = NS_MODE_MAP_VERSE.find(asrNoiseSuppressionMode);
    if (it != NS_MODE_MAP_VERSE.end()) {
        value = key + "=" + it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
    return 0;
}

int32_t AudioServer::GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode& asrNoiseSuppressionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_ns_mode";
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    std::string asrNoiseSuppressionModeSink = audioRendererSinkInstance->GetAudioParameter(parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrNoiseSuppressionModeSink = it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    std::vector<std::string> resMode = splitString(asrNoiseSuppressionModeSink, "=");
    const int32_t resSize = 2;
    std::string modeString = "";
    if (resMode.size() == resSize) {
        modeString = resMode[1];
        auto it = NS_MODE_MAP.find(modeString);
        if (it != NS_MODE_MAP.end()) {
            asrNoiseSuppressionMode = it->second;
        } else {
            AUDIO_ERR_LOG("get value failed.");
            return ERR_INVALID_PARAM;
        }
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    return 0;
}

int32_t AudioServer::SetAsrWhisperDetectionMode(AsrWhisperDetectionMode asrWhisperDetectionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_wd_mode";
    std::string value = key + "=";

    auto it = WHISPER_DETECTION_MODE_MAP_VERSE.find(asrWhisperDetectionMode);
    if (it != WHISPER_DETECTION_MODE_MAP_VERSE.end()) {
        value = key + "=" + it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
    return 0;
}

int32_t AudioServer::GetAsrWhisperDetectionMode(AsrWhisperDetectionMode& asrWhisperDetectionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_wd_mode";
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    std::string asrWhisperDetectionModeSink = audioRendererSinkInstance->GetAudioParameter(parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrWhisperDetectionModeSink = it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    std::vector<std::string> resMode = splitString(asrWhisperDetectionModeSink, "=");
    const int32_t resSize = 2;
    std::string modeString = "";
    if (resMode.size() == resSize) {
        modeString = resMode[1];
        auto it = WHISPER_DETECTION_MODE_MAP.find(modeString);
        if (it != WHISPER_DETECTION_MODE_MAP.end()) {
            asrWhisperDetectionMode = it->second;
        } else {
            AUDIO_ERR_LOG("get value failed.");
            return ERR_INVALID_PARAM;
        }
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    return 0;
}

int32_t AudioServer::SetAsrVoiceControlMode(AsrVoiceControlMode asrVoiceControlMode, bool on)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "avcm";
    std::string value = key + "=";

    auto itVerse = VC_MODE_MAP_VERSE.find(asrVoiceControlMode);
    auto itCallAssistant = VOICE_CALL_ASSISTANT_SUPPRESSION.find(asrVoiceControlMode);
    auto res = RES_MAP_VERSE.find(on);
    if ((itVerse == VC_MODE_MAP_VERSE.end() && itCallAssistant == VOICE_CALL_ASSISTANT_SUPPRESSION.end()) ||
        res == RES_MAP_VERSE.end()) {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    if ((itVerse != VC_MODE_MAP_VERSE.end()) && (res != RES_MAP_VERSE.end())) {
        value = itVerse->second + "=" + res->second;
        AudioServer::audioParameters[key] = value;
        audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
        return 0;
    }
    if ((itCallAssistant != VOICE_CALL_ASSISTANT_SUPPRESSION.end()) && (res != RES_MAP_VERSE.end())) {
        std::vector<std::string> modes = VOICE_CALL_ASSISTANT_SUPPRESSION.at(asrVoiceControlMode);
        std::set<std::string> needSuppression = VOICE_CALL_ASSISTANT_NEED_SUPPRESSION.at(asrVoiceControlMode);
        for (size_t i = 0; i < modes.size(); i++) {
            if (needSuppression.contains(modes[i]) && on) {
                audioRendererSinkInstance->SetAudioParameter(parmKey, "",
                    modes[i] + "=" + VOICE_CALL_SUPPRESSION_VOLUME);
                continue;
            }
            audioRendererSinkInstance->SetAudioParameter(parmKey, "",
                modes[i] + "=" + VOICE_CALL_FULL_VOLUME);
        }
    }
    
    return 0;
}

int32_t AudioServer::SetAsrVoiceMuteMode(AsrVoiceMuteMode asrVoiceMuteMode, bool on)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "avmm";
    std::string value = key + "=";

    auto it = VM_MODE_MAP_VERSE.find(asrVoiceMuteMode);
    auto res = RES_MAP_VERSE.find(on);
    if ((it != VM_MODE_MAP_VERSE.end()) && (res != RES_MAP_VERSE.end())) {
        value = it->second + "=" + res->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
    return 0;
}

int32_t AudioServer::IsWhispering()
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_is_whisper";
    AudioParamKey parmKey = AudioParamKey::NONE;
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");

    std::string isWhisperSink = audioRendererSinkInstance->GetAudioParameter(parmKey, key);
    int32_t whisperRes = 0;
    if (isWhisperSink == "TRUE") {
        whisperRes = 1;
    }
    return whisperRes;
}

} // namespace AudioStandard
} // namespace OHOS