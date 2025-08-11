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

#include <unordered_map>
#include <vector>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_asr.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const std::map<std::string, AsrAecMode> AEC_MODE_MAP = {
    {"BYPASS", AsrAecMode::BYPASS},
    {"STANDARD", AsrAecMode::STANDARD},
    {"EXPAND", AsrAecMode::EXPAND},
    {"FOLDED", AsrAecMode::FOLDED}
};

static const std::map<AsrAecMode, std::string> AEC_MODE_MAP_VERSE = {
    {AsrAecMode::BYPASS, "BYPASS"},
    {AsrAecMode::STANDARD, "STANDARD"},
    {AsrAecMode::EXPAND, "EXPAND"},
    {AsrAecMode::FOLDED, "FOLDED"}
};

static const std::map<std::string, AsrNoiseSuppressionMode> NS_MODE_MAP = {
    {"BYPASS", AsrNoiseSuppressionMode::BYPASS},
    {"STANDARD", AsrNoiseSuppressionMode::STANDARD},
    {"NEAR_FIELD", AsrNoiseSuppressionMode::NEAR_FIELD},
    {"FAR_FIELD", AsrNoiseSuppressionMode::FAR_FIELD},
    {"FULL_DUPLEX_STANDARD", AsrNoiseSuppressionMode::FULL_DUPLEX_STANDARD},
    {"FULL_DUPLEX_NEAR_FIELD", AsrNoiseSuppressionMode::FULL_DUPLEX_NEAR_FIELD},
    {"ASR_WHISPER_MODE", AsrNoiseSuppressionMode::ASR_WHISPER_MODE}
};

static const std::map<AsrNoiseSuppressionMode, std::string> NS_MODE_MAP_VERSE = {
    {AsrNoiseSuppressionMode::BYPASS, "BYPASS"},
    {AsrNoiseSuppressionMode::STANDARD, "STANDARD"},
    {AsrNoiseSuppressionMode::NEAR_FIELD, "NEAR_FIELD"},
    {AsrNoiseSuppressionMode::FAR_FIELD, "FAR_FIELD"},
    {AsrNoiseSuppressionMode::FULL_DUPLEX_STANDARD, "FULL_DUPLEX_STANDARD"},
    {AsrNoiseSuppressionMode::FULL_DUPLEX_NEAR_FIELD, "FULL_DUPLEX_NEAR_FIELD"},
    {AsrNoiseSuppressionMode::ASR_WHISPER_MODE, "ASR_WHISPER_MODE"}
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

static const std::string TTS_2_DEVICE_STRING = "TTS_2_DEVICE";
static const std::string TTS_2_MODEM_STRING = "TTS_2_MODEM";

static const std::map<AsrVoiceControlMode, std::vector<std::string>> VOICE_CALL_ASSISTANT_SUPPRESSION = {
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_OPPOSITE, {TTS_2_DEVICE_STRING, TTS_2_MODEM_STRING}},
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_LOCAL, {TTS_2_DEVICE_STRING, TTS_2_MODEM_STRING}},
    {AsrVoiceControlMode::VOICE_TXRX_DECREASE, {"MIC_2_MODEM", "MODEM_2_DEVICE"}},
};

static const std::map<AsrVoiceControlMode, std::set<std::string>> VOICE_CALL_ASSISTANT_NEED_SUPPRESSION = {
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_OPPOSITE, {TTS_2_MODEM_STRING}},
    {AsrVoiceControlMode::AUDIO_SUPPRESSION_LOCAL, {TTS_2_DEVICE_STRING}},
    {AsrVoiceControlMode::VOICE_TXRX_DECREASE, {"MIC_2_MODEM", "MODEM_2_DEVICE"}},
};

static const std::string VOICE_CALL_SUPPRESSION_VOLUME = "2";
static const std::string VOICE_CALL_FULL_VOLUME = "32";

static const int32_t VOICE_CALL_MIN_VOLUME = 2;
static const int32_t VOICE_CALL_MAX_VOLUME = 32;

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

int32_t AudioServer::SetAsrAecMode(int32_t asrAecMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_aec_mode";
    std::string value = key + "=";
    std::string keyAec = "ASR_AEC";
    std::string valueAec = "";

    auto it = AEC_MODE_MAP_VERSE.find(static_cast<AsrAecMode>(asrAecMode));
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
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr!");
    deviceManager->SetAudioParameter("primary", parmKey, "", value);
    deviceManager->SetAudioParameter("primary", parmKey, "", valueAec);
    return 0;
}

int32_t AudioServer::GetAsrAecMode(int32_t& asrAecMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_aec_mode";
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr!");
    std::string asrAecModeSink = deviceManager->GetAudioParameter("primary", parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrAecModeSink = it->second;
    } else {
        // if asr_aec_mode null, return ASR_AEC.
        // if asr_aec_mode null and ASR_AEC null, return err.
        std::string keyAec = "ASR_AEC";
        auto itAec = AudioServer::audioParameters.find(keyAec);
        std::string asrAecSink = itAec->second;
        if (asrAecSink == "ASR_AEC=ON") {
            asrAecMode = static_cast<int>(AsrAecMode::STANDARD);
        } else if (asrAecSink == "ASR_AEC=OFF") {
            asrAecMode = static_cast<int>(AsrAecMode::BYPASS);
        } else {
            AUDIO_ERR_LOG("get value failed!");
            return ERR_INVALID_PARAM;
        }
        return 0;
    }

    std::vector<std::string> resMode = splitString(asrAecModeSink, "=");
    const int32_t resSize = 2;
    if (resMode.size() == resSize) {
        std::string modeString = "";
        modeString = resMode[1];
        auto itAecMode = AEC_MODE_MAP.find(modeString);
        if (itAecMode != AEC_MODE_MAP.end()) {
            asrAecMode = static_cast<int>(itAecMode->second);
        } else {
            AUDIO_ERR_LOG("get value failed!");
            return ERR_INVALID_PARAM;
        }
    } else {
        AUDIO_ERR_LOG("get value failed!");
        return ERR_INVALID_PARAM;
    }
    return 0;
}

int32_t AudioServer::SetAsrNoiseSuppressionMode(int32_t asrNoiseSuppressionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_ns_mode";
    std::string value = key + "=";

    auto it = NS_MODE_MAP_VERSE.find(static_cast<AsrNoiseSuppressionMode>(asrNoiseSuppressionMode));
    if (it != NS_MODE_MAP_VERSE.end()) {
        value = key + "=" + it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr!");
    deviceManager->SetAudioParameter("primary", parmKey, "", value);
    return 0;
}

int32_t AudioServer::GetAsrNoiseSuppressionMode(int32_t& asrNoiseSuppressionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_ns_mode";
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr!");
    std::string asrNoiseSuppressionModeSink = deviceManager->GetAudioParameter("primary", parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrNoiseSuppressionModeSink = it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    std::vector<std::string> resMode = splitString(asrNoiseSuppressionModeSink, "=");
    const int32_t resSize = 2;
    if (resMode.size() == resSize) {
        std::string modeString = "";
        modeString = resMode[1];
        auto itNsMode = NS_MODE_MAP.find(modeString);
        if (itNsMode != NS_MODE_MAP.end()) {
            asrNoiseSuppressionMode = static_cast<int>(itNsMode->second);
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

int32_t AudioServer::SetAsrWhisperDetectionMode(int32_t asrWhisperDetectionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_wd_mode";
    std::string value = key + "=";

    auto it = WHISPER_DETECTION_MODE_MAP_VERSE.find(static_cast<AsrWhisperDetectionMode>(asrWhisperDetectionMode));
    if (it != WHISPER_DETECTION_MODE_MAP_VERSE.end()) {
        value = key + "=" + it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", parmKey, "", value);
    return 0;
}

int32_t AudioServer::GetAsrWhisperDetectionMode(int32_t& asrWhisperDetectionMode)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_wd_mode";
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    std::string asrWhisperDetectionModeSink = deviceManager->GetAudioParameter("primary", parmKey, key);
    auto it = AudioServer::audioParameters.find(key);
    if (it != AudioServer::audioParameters.end()) {
        asrWhisperDetectionModeSink = it->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    std::vector<std::string> resMode = splitString(asrWhisperDetectionModeSink, "=");
    const int32_t resSize = 2;
    if (resMode.size() == resSize) {
        std::string modeString = "";
        modeString = resMode[1];
        auto itWhisper = WHISPER_DETECTION_MODE_MAP.find(modeString);
        if (itWhisper != WHISPER_DETECTION_MODE_MAP.end()) {
            asrWhisperDetectionMode = static_cast<int>(itWhisper->second);
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
// LCOV_EXCL_STOP

int32_t AudioServer::SetAsrVoiceSuppressionControlMode(
    const AudioParamKey paramKey, AsrVoiceControlMode asrVoiceControlMode, bool on, int32_t modifyVolume)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    std::vector<std::string> modes = VOICE_CALL_ASSISTANT_SUPPRESSION.at(asrVoiceControlMode);
    std::set<std::string> needSuppression = VOICE_CALL_ASSISTANT_NEED_SUPPRESSION.at(asrVoiceControlMode);
    for (size_t i = 0; i < modes.size(); i++) {
        if (needSuppression.contains(modes[i]) && on) {
            deviceManager->SetAudioParameter("primary", paramKey, "", modes[i] + "=" + VOICE_CALL_SUPPRESSION_VOLUME);
            continue;
        }
        if (modes[i] == TTS_2_MODEM_STRING || !on) {
            deviceManager->SetAudioParameter("primary", paramKey, "", modes[i] + "=" + VOICE_CALL_FULL_VOLUME);
            continue;
        }
        deviceManager->SetAudioParameter("primary", paramKey, "", modes[i] + "=" + std::to_string(modifyVolume));
    }

    return 0;
}

int32_t AudioServer::SetAsrVoiceControlMode(int32_t asrVoiceControlMode, bool on)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "avcm";
    std::string value = key + "=";

    AsrVoiceControlMode asrVoiceControlModeTmp = static_cast<AsrVoiceControlMode>(asrVoiceControlMode);
    auto itVerse = VC_MODE_MAP_VERSE.find(asrVoiceControlModeTmp);
    auto itCallAssistant = VOICE_CALL_ASSISTANT_SUPPRESSION.find(asrVoiceControlModeTmp);
    auto res = RES_MAP_VERSE.find(on);
    if (itVerse == VC_MODE_MAP_VERSE.end() && itCallAssistant == VOICE_CALL_ASSISTANT_SUPPRESSION.end()) {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }

    AudioParamKey paramKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    if (itVerse != VC_MODE_MAP_VERSE.end()) {
        value = itVerse->second + "=" + res->second;
        AudioServer::audioParameters[key] = value;
        deviceManager->SetAudioParameter("primary", paramKey, "", value);
        return 0;
    }
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    Volume vol = {false, 0.0f, 0};
    PolicyHandler::GetInstance().GetSharedVolume(STREAM_VOICE_CALL, deviceType, vol);
    float systemVol = vol.isMute ? 0.0f : vol.volumeFloat;
    AUDIO_INFO_LOG("STREAM_VOICE_CALL = [%{public}f]", systemVol);
    int32_t modifyVolume = std::floor(systemVol * VOICE_CALL_MAX_VOLUME);
    modifyVolume = modifyVolume < VOICE_CALL_MIN_VOLUME ? VOICE_CALL_MIN_VOLUME : modifyVolume;
    if ((itCallAssistant != VOICE_CALL_ASSISTANT_SUPPRESSION.end()) && (res != RES_MAP_VERSE.end())) {
        return SetAsrVoiceSuppressionControlMode(paramKey, asrVoiceControlModeTmp, on, modifyVolume);
    }
    
    return 0;
}

int32_t AudioServer::SetAsrVoiceMuteMode(int32_t asrVoiceMuteMode, bool on)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "avmm";
    std::string value = key + "=";

    auto it = VM_MODE_MAP_VERSE.find(static_cast<AsrVoiceMuteMode>(asrVoiceMuteMode));
    auto res = RES_MAP_VERSE.find(on);
    if ((it != VM_MODE_MAP_VERSE.end()) && (res != RES_MAP_VERSE.end())) {
        value = it->second + "=" + res->second;
    } else {
        AUDIO_ERR_LOG("get value failed.");
        return ERR_INVALID_PARAM;
    }
    AudioServer::audioParameters[key] = value;
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", parmKey, "", value);
    return 0;
}

int32_t AudioServer::IsWhispering(int32_t& whisperRes)
{
    whisperRes = 0;
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_SYSTEM_PERMISSION_DENIED,
        "Check playback permission failed, no system permission");
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    std::string key = "asr_is_whisper";
    AudioParamKey parmKey = AudioParamKey::NONE;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    std::string isWhisperSink = deviceManager->GetAudioParameter("primary", parmKey, key);
    if (isWhisperSink == "TRUE") {
        whisperRes = 1;
    }
    return SUCCESS;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS