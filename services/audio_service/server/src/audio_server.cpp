/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#define PA
#ifdef PA
extern "C" {
    extern int ohos_pa_main(int argc, char *argv[]);
}
#endif

using namespace std;

namespace OHOS {
namespace AudioStandard {
uint32_t AudioServer::paDaemonTid_;
std::map<std::string, std::string> AudioServer::audioParameters;
std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> AudioServer::audioParameterKeys;
const string DEFAULT_COOKIE_PATH = "/data/data/.pulse_dir/state/cookie";
const std::string CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
const unsigned int TIME_OUT_SECONDS = 10;
const unsigned int SCHEDULE_REPORT_TIME_OUT_SECONDS = 2;
static const std::vector<StreamUsage> STREAMS_NEED_VERIFY_SYSTEM_PERMISSION = {
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION
};
static const int32_t MODERN_INNER_API_VERSION = 12;
const int32_t API_VERSION_REMAINDER = 1000;
static constexpr int32_t VM_MANAGER_UID = 7700;
static const int32_t FAST_DUMPINFO_LEN = 2;
static const int32_t BUNDLENAME_LENGTH_LIMIT = 1024;
constexpr int32_t UID_CAMERA = 1047;
static const std::set<int32_t> RECORD_CHECK_FORWARD_LIST = {
    VM_MANAGER_UID,
    UID_CAMERA
};
// using pass-in appInfo for uids:
constexpr int32_t UID_MEDIA_SA = 1013;

const std::set<int32_t> RECORD_PASS_APPINFO_LIST = {
    UID_MEDIA_SA
};

const std::set<SourceType> VALID_SOURCE_TYPE = {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION
};


static constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;

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

class CapturerStateOb final : public ICapturerStateCallback {
public:
    explicit CapturerStateOb(std::function<void(bool, int32_t)> callback) : callback_(callback)
    {
        num_ = count_.fetch_add(1, std::memory_order_relaxed);
    }

    ~CapturerStateOb() override final
    {
        count_.fetch_sub(1, std::memory_order_relaxed);
    }

    void OnCapturerState(bool isActive) override final
    {
        callback_(isActive, num_);
    }

private:
    static inline std::atomic<int32_t> count_ = 0;
    int32_t num_;

    // callback to audioserver
    std::function<void(bool, int32_t)> callback_;
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

REGISTER_SYSTEM_ABILITY_BY_ID(AudioServer, AUDIO_DISTRIBUTED_SERVICE_ID, true)

#ifdef PA
constexpr int PA_ARG_COUNT = 1;

void *AudioServer::paDaemonThread(void *arg)
{
    /* Load the mandatory pulseaudio modules at start */
    char *argv[] = {
        (char*)"pulseaudio",
    };
    paDaemonTid_ = static_cast<uint32_t>(gettid());
    AUDIO_INFO_LOG("Calling ohos_pa_main\n");
    ohos_pa_main(PA_ARG_COUNT, argv);
    AUDIO_INFO_LOG("Exiting ohos_pa_main\n");
    exit(-1);
}
#endif

AudioServer::AudioServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
    audioEffectServer_(std::make_unique<AudioEffectServer>()) {}

void AudioServer::OnDump() {}

int32_t AudioServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    AUDIO_INFO_LOG("Dump Process Invoked");
    if (args.size() == FAST_DUMPINFO_LEN && args[0] == u"-fb") {
        std::string bundleName = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(args[1]);
        std::string result = GetAudioParameter(CHECK_FAST_BLOCK_PREFIX + bundleName);
        std::string dumpString = "check fast list :bundle name is" + bundleName + " result is " + result + "\n";
        return write(fd, dumpString.c_str(), dumpString.size());
    }
    std::queue<std::u16string> argQue;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argQue.push(args[index]);
    }
    std::string dumpString;

    AudioServerDump dumpObj;
    int32_t res = dumpObj.Initialize();
    CHECK_AND_RETURN_RET_LOG(res == AUDIO_DUMP_SUCCESS, AUDIO_DUMP_INIT_ERR,
        "Audio Service Dump Not initialised\n");
    dumpObj.AudioDataDump(dumpString, argQue);
    return write(fd, dumpString.c_str(), dumpString.size());
}

void AudioServer::OnStart()
{
    audioUid_ = static_cast<int32_t>(getuid());
    AUDIO_INFO_LOG("OnStart uid:%{public}d", audioUid_);
    AudioInnerCall::GetInstance()->RegisterAudioServer(this);
    bool res = Publish(this);
    if (!res) {
        AUDIO_ERR_LOG("start err");
        WriteServiceStartupError();
    }
    AddSystemAbilityListener(AUDIO_POLICY_SERVICE_ID);
    AddSystemAbilityListener(RES_SCHED_SYS_ABILITY_ID);
#ifdef PA
    int32_t ret = pthread_create(&m_paDaemonThread, nullptr, AudioServer::paDaemonThread, nullptr);
    pthread_setname_np(m_paDaemonThread, "OS_PaDaemon");
    if (ret != 0) {
        AUDIO_ERR_LOG("pthread_create failed %d", ret);
        WriteServiceStartupError();
    }
    AUDIO_DEBUG_LOG("Created paDaemonThread\n");
#endif

    RegisterAudioCapturerSourceCallback();

    std::unique_ptr<AudioParamParser> audioParamParser = make_unique<AudioParamParser>();
    if (audioParamParser == nullptr) {
        WriteServiceStartupError();
    }
    CHECK_AND_RETURN_LOG(audioParamParser != nullptr, "Failed to create audio extra parameters parser");
    if (audioParamParser->LoadConfiguration(audioParameterKeys)) {
        AUDIO_INFO_LOG("Audio extra parameters load configuration successfully.");
    }
}

void AudioServer::WriteServiceStartupError()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_SERVICE_STARTUP_ERROR,
        Media::MediaMonitor::FAULT_EVENT);
    bean->Add("SERVICE_ID", static_cast<int32_t>(Media::MediaMonitor::AUDIO_SERVER_ID));
    bean->Add("ERROR_CODE", static_cast<int32_t>(Media::MediaMonitor::AUDIO_SERVER));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioServer::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    AUDIO_DEBUG_LOG("systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case AUDIO_POLICY_SERVICE_ID:
            AUDIO_INFO_LOG("input service start");
            RegisterPolicyServerDeathRecipient();
            break;
        case RES_SCHED_SYS_ABILITY_ID:
            AUDIO_INFO_LOG("ressched service start");
            OnAddResSchedService(getpid());
            break;
        default:
            AUDIO_ERR_LOG("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
}

void AudioServer::OnStop()
{
    AUDIO_DEBUG_LOG("OnStop");
}

void AudioServer::RecognizeAudioEffectType(const std::string &mainkey, const std::string &subkey,
    const std::string &extraSceneType)
{
    AUDIO_DEBUG_LOG("mainkey is %{public}s, subkey is %{public}s, extraSceneType is %{public}s",
        mainkey.c_str(), subkey.c_str(), extraSceneType.c_str());
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        AUDIO_ERR_LOG("audioEffectChainManager is nullptr");
        return;
    }
    audioEffectChainManager->UpdateExtraSceneType(extraSceneType);
}

int32_t AudioServer::SetExtraParameters(const std::string& key,
    const std::vector<std::pair<std::string, std::string>>& kvpairs)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "set extra parameters failed: not system app.");
    ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "set extra parameters failed: no permission.");

    if (key == "PCM_DUMP") {
        ret = VerifyClientPermission(DUMP_AUDIO_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "set audiodump parameters failed: no permission.");
        ret = Media::MediaMonitor::MediaMonitorManager::GetInstance().SetMediaParameters(kvpairs);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetMediaParameters failed.");
    }

    if (audioParameterKeys.empty()) {
        AUDIO_ERR_LOG("audio extra parameters mainKey and subKey is empty");
        return ERROR;
    }

    auto mainKeyIt = audioParameterKeys.find(key);
    if (mainKeyIt == audioParameterKeys.end()) {
        return ERR_INVALID_PARAM;
    }

    std::unordered_map<std::string, std::set<std::string>> subKeyMap = mainKeyIt->second;
    std::string value;
    bool match = true;
    for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
        auto subKeyIt = subKeyMap.find(it->first);
        if (subKeyIt != subKeyMap.end()) {
            value += it->first + "=" + it->second + ";";
            auto valueIter = subKeyIt->second.find("effect");
            if (valueIter != subKeyIt->second.end()) {
                RecognizeAudioEffectType(key, it->first, it->second);
            }
        } else {
            match = false;
            break;
        }
    }
    if (!match) {
        return ERR_INVALID_PARAM;
    }

    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(AudioParamKey::NONE, "", value);
    return SUCCESS;
}

void AudioServer::SetAudioParameter(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    AudioXCollie audioXCollie("AudioServer::SetAudioParameter", TIME_OUT_SECONDS);
    AUDIO_DEBUG_LOG("server: set audio parameter");
    if (key != "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
        bool ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
        CHECK_AND_RETURN_LOG(ret, "MODIFY_AUDIO_SETTINGS permission denied");
    } else {
        int32_t audio_policy_server_id = 1041;
        CHECK_AND_RETURN_LOG(IPCSkeleton::GetCallingUid() == audio_policy_server_id,
            "A2dp offload modify audio settings permission denied");
    }

    AudioServer::audioParameters[key] = value;

    // send it to hal
    AudioParamKey parmKey = AudioParamKey::NONE;
    if (key == "A2dpSuspended") {
        parmKey = AudioParamKey::A2DP_SUSPEND_STATE;
        IAudioRendererSink* bluetoothSinkInstance = IAudioRendererSink::GetInstance("a2dp", "");
        CHECK_AND_RETURN_LOG(bluetoothSinkInstance != nullptr, "has no valid sink");
        std::string renderValue = key + "=" + value + ";";
        bluetoothSinkInstance->SetAudioParameter(parmKey, "", renderValue);
        return;
    }

    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_LOG(audioRendererSinkInstance != nullptr, "has no valid sink");

    if (key == "AUDIO_EXT_PARAM_KEY_LOWPOWER") {
        parmKey = AudioParamKey::PARAM_KEY_LOWPOWER;
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "SMARTPA_LOWPOWER",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "STATE", value == "SmartPA_lowpower=on" ? 1 : 0);
    } else if (key == "bt_headset_nrec") {
        parmKey = AudioParamKey::BT_HEADSET_NREC;
    } else if (key == "bt_wbs") {
        parmKey = AudioParamKey::BT_WBS;
    } else if (key == "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
        parmKey = AudioParamKey::A2DP_OFFLOAD_STATE;
        std::string value_new = "a2dpOffloadConfig=" + value;
        audioRendererSinkInstance->SetAudioParameter(parmKey, "", value_new);
        return;
    } else if (key == "mmi") {
        parmKey = AudioParamKey::MMI;
    } else if (key == "perf_info") {
        parmKey = AudioParamKey::PERF_INFO;
    } else {
        AUDIO_ERR_LOG("key %{public}s is invalid for hdi interface", key.c_str());
        return;
    }
    audioRendererSinkInstance->SetAudioParameter(parmKey, "", value);
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

int32_t AudioServer::SuspendRenderSink(const std::string &sinkName)
{
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return ERR_OPERATION_FAILED;
    }
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance(sinkName.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->SuspendRenderSink();
}

int32_t AudioServer::RestoreRenderSink(const std::string &sinkName)
{
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return ERR_OPERATION_FAILED;
    }
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance(sinkName.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->RestoreRenderSink();
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

    auto it = VC_MODE_MAP_VERSE.find(asrVoiceControlMode);
    auto res = RES_MAP_VERSE.find(on);
    if ((it != VC_MODE_MAP_VERSE.end()) && (res != RES_MAP_VERSE.end())) {
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

void AudioServer::SetAudioParameter(const std::string& networkId, const AudioParamKey key, const std::string& condition,
    const std::string& value)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bool ret = VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
    CHECK_AND_RETURN_LOG(callingUid == audioUid_ || ret, "SetAudioParameter refused for %{public}d", callingUid);
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
    CHECK_AND_RETURN_LOG(audioRendererSinkInstance != nullptr, "has no valid sink");

    audioRendererSinkInstance->SetAudioParameter(key, condition, value);
}

int32_t AudioServer::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    if (audioParameterKeys.empty()) {
        AUDIO_ERR_LOG("audio extra parameters mainKey and subKey is empty");
        return ERROR;
    }

    auto mainKeyIt = audioParameterKeys.find(mainKey);
    if (mainKeyIt == audioParameterKeys.end()) {
        return ERR_INVALID_PARAM;
    }

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = mainKeyIt->second;
    if (subKeys.empty()) {
        for (auto it = subKeyMap.begin(); it != subKeyMap.end(); it++) {
            std::string value = audioRendererSinkInstance->GetAudioParameter(AudioParamKey::NONE, it->first);
            result.emplace_back(std::make_pair(it->first, value));
        }
        return SUCCESS;
    }

    bool match = true;
    for (auto it = subKeys.begin(); it != subKeys.end(); it++) {
        auto subKeyIt = subKeyMap.find(*it);
        if (subKeyIt != subKeyMap.end()) {
            std::string value = audioRendererSinkInstance->GetAudioParameter(AudioParamKey::NONE, *it);
            result.emplace_back(std::make_pair(*it, value));
        } else {
            match = false;
            break;
        }
    }
    if (!match) {
        result.clear();
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

bool AudioServer::CheckAndPrintStacktrace(const std::string &key)
{
    AUDIO_WARNING_LOG("Start handle forced xcollie event for key %{public}s", key.c_str());
    if (key == "dump_pulseaudio_stacktrace") {
        AudioXCollie audioXCollie("AudioServer::PrintStackTrace", 1); // 1 means XCOLLIE_FLAG_LOG
        sleep(2); // sleep 2 seconds to dump stacktrace
        return true;
    } else if (key == "recovery_audio_server") {
        AudioXCollie audioXCollie("AudioServer::Kill", 1, nullptr, nullptr, 2); // 2 means RECOVERY
        sleep(2); // sleep 2 seconds to dump stacktrace
        return true;
    } else if (key == "dump_pa_stacktrace_and_kill") {
        uint32_t targetFlag = 3; // 3 means LOG & RECOVERY
        AudioXCollie audioXCollie("AudioServer::PrintStackTraceAndKill", 1, nullptr, nullptr, targetFlag);
        sleep(2); // sleep 2 seconds to dump stacktrace
        return true;
    }
    return false;
}

const std::string AudioServer::GetAudioParameter(const std::string &key)
{
    if (IPCSkeleton::GetCallingUid() == MEDIA_SERVICE_UID && CheckAndPrintStacktrace(key) == true) {
        return "";
    }
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS);

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    if (audioRendererSinkInstance != nullptr) {
        AudioParamKey parmKey = AudioParamKey::NONE;
        if (key == "AUDIO_EXT_PARAM_KEY_LOWPOWER") {
            parmKey = AudioParamKey::PARAM_KEY_LOWPOWER;
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey(parmKey), "");
        }
        if (key == "need_change_usb_device") {
            parmKey = AudioParamKey::USB_DEVICE;
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey(parmKey), "need_change_usb_device");
        }
        if (key == "getSmartPAPOWER" || key == "show_RealTime_ChipModel") {
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey::NONE, key);
        }
        if (key == "perf_info") {
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey::PERF_INFO, key);
        }
        if (key.size() < BUNDLENAME_LENGTH_LIMIT && key.size() > CHECK_FAST_BLOCK_PREFIX.size() &&
            key.substr(0, CHECK_FAST_BLOCK_PREFIX.size()) == CHECK_FAST_BLOCK_PREFIX) {
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey::NONE, key);
        }

        const std::string mmiPre = "mmi_";
        if (key.size() > mmiPre.size()) {
            if (key.substr(0, mmiPre.size()) == mmiPre) {
                parmKey = AudioParamKey::MMI;
                return audioRendererSinkInstance->GetAudioParameter(AudioParamKey(parmKey),
                    key.substr(mmiPre.size(), key.size() - mmiPre.size()));
            }
        }
    }

    if (AudioServer::audioParameters.count(key)) {
        return AudioServer::audioParameters[key];
    } else {
        return "";
    }
}

const std::string AudioServer::GetDPParameter(const std::string &condition)
{
    IAudioRendererSink *dpAudioRendererSinkInstance = IAudioRendererSink::GetInstance("dp", "");
    CHECK_AND_RETURN_RET_LOG(dpAudioRendererSinkInstance != nullptr, "", "get dp instance failed");

    return dpAudioRendererSinkInstance->GetAudioParameter(AudioParamKey::GET_DP_DEVICE_INFO, condition);
}

const std::string AudioServer::GetUsbParameter()
{
    IAudioRendererSink *usbAudioRendererSinkInstance = IAudioRendererSink::GetInstance("usb", "");
    IAudioCapturerSource *usbAudioCapturerSinkInstance = IAudioCapturerSource::GetInstance("usb", "");
    if (usbAudioRendererSinkInstance != nullptr && usbAudioCapturerSinkInstance != nullptr) {
        std::string usbInfoStr =
            usbAudioRendererSinkInstance->GetAudioParameter(AudioParamKey::USB_DEVICE, "get_usb_info");
        // Preload usb sink and source, make pa load module faster to avoid blocking client write
        usbAudioRendererSinkInstance->Preload(usbInfoStr);
        usbAudioCapturerSinkInstance->Preload(usbInfoStr);
        return usbInfoStr;
    }
    return "";
}

const std::string AudioServer::GetAudioParameter(const std::string& networkId, const AudioParamKey key,
    const std::string& condition)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bool ret = VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_ || ret, "", "refused for %{public}d", callingUid);

    if (networkId == LOCAL_NETWORK_ID) {
        AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS);
        if (key == AudioParamKey::USB_DEVICE) {
            return GetUsbParameter();
        }
        if (key == AudioParamKey::GET_DP_DEVICE_INFO) {
            return GetDPParameter(condition);
        }
    } else {
        IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
        CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, "", "has no valid sink");
        return audioRendererSinkInstance->GetAudioParameter(key, condition);
    }
    return "";
}

uint64_t AudioServer::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    uint64_t transactionId = 0;
    AUDIO_DEBUG_LOG("device type: %{public}d, device role: %{public}d", deviceType, deviceRole);
    if (deviceRole != INPUT_DEVICE && deviceRole != OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("AudioServer::GetTransactionId: error device role");
        return ERR_INVALID_PARAM;
    }
    if (deviceRole == INPUT_DEVICE) {
        AudioCapturerSource *audioCapturerSourceInstance;
        if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            audioCapturerSourceInstance = AudioCapturerSource::GetInstance("usb");
        } else {
            audioCapturerSourceInstance = AudioCapturerSource::GetInstance("primary");
        }
        if (audioCapturerSourceInstance) {
            transactionId = audioCapturerSourceInstance->GetTransactionId();
        }
        return transactionId;
    }

    // deviceRole OUTPUT_DEVICE
    IAudioRendererSink *iRendererInstance = nullptr;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        iRendererInstance = IAudioRendererSink::GetInstance("a2dp", "");
    } else if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
        iRendererInstance = IAudioRendererSink::GetInstance("usb", "");
    } else {
        iRendererInstance = IAudioRendererSink::GetInstance("primary", "");
    }

    int32_t ret = ERROR;
    if (iRendererInstance != nullptr) {
        ret = iRendererInstance->GetTransactionId(&transactionId);
    }

    CHECK_AND_RETURN_RET_LOG(!ret, transactionId, "Get transactionId failed.");

    AUDIO_DEBUG_LOG("Transaction Id: %{public}" PRIu64, transactionId);
    return transactionId;
}

bool AudioServer::LoadAudioEffectLibraries(const std::vector<Library> libraries, const std::vector<Effect> effects,
    std::vector<Effect>& successEffectList)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, false, "LoadAudioEffectLibraries refused for %{public}d",
        callingUid);
    bool loadSuccess = audioEffectServer_->LoadAudioEffects(libraries, effects, successEffectList);
    if (!loadSuccess) {
        AUDIO_WARNING_LOG("Load audio effect failed, please check log");
    }
    return loadSuccess;
}

bool AudioServer::CreateEffectChainManager(std::vector<EffectChain> &effectChains,
    std::unordered_map<std::string, std::string> &effectMap,
    std::unordered_map<std::string, std::string> &enhanceMap)
{
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return false;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->InitAudioEffectChainManager(effectChains, effectMap,
        audioEffectServer_->GetEffectEntries());
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    audioEnhanceChainManager->InitAudioEnhanceChainManager(effectChains, enhanceMap,
        audioEffectServer_->GetEffectEntries());
    return true;
}

void AudioServer::SetOutputDeviceSink(int32_t deviceType, std::string &sinkName)
{
    Trace trace("AudioServer::SetOutputDeviceSink:" + std::to_string(deviceType) + " sink:" + sinkName);
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return;
    }
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->SetOutputDeviceSink(deviceType, sinkName);
    return;
}

int32_t AudioServer::SetMicrophoneMute(bool isMute)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_PERMISSION_DENIED, "SetMicrophoneMute refused for %{public}d",
        callingUid);

    std::vector<IAudioCapturerSource *> allSourcesInstance;
    IAudioCapturerSource::GetAllInstance(allSourcesInstance);
    for (auto it = allSourcesInstance.begin(); it != allSourcesInstance.end(); ++it) {
        (*it)->SetMute(isMute);
    }

    return SUCCESS;
}

int32_t AudioServer::SetVoiceVolume(float volume)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "SetVoiceVolume refused for %{public}d",
        callingUid);
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");

    if (audioRendererSinkInstance == nullptr) {
        AUDIO_WARNING_LOG("Renderer is null.");
    } else {
        return audioRendererSinkInstance->SetVoiceVolume(volume);
    }
    return ERROR;
}

int32_t AudioServer::OffloadSetVolume(float volume)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("offload", "");

    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return audioRendererSinkInstance->SetVolume(volume, volume);
}

int32_t AudioServer::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeOutputDevices,
    DeviceType activeInputDevice, BluetoothOffloadState a2dpOffloadFlag)
{
    std::lock_guard<std::mutex> lock(audioSceneMutex_);

    DeviceType activeOutputDevice = activeOutputDevices.front();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    AudioXCollie audioXCollie("AudioServer::SetAudioScene", TIME_OUT_SECONDS);
    AudioCapturerSource *audioCapturerSourceInstance;
    IAudioRendererSink *audioRendererSinkInstance;
    if (activeOutputDevice == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("usb", "");
    } else {
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    }
    if (activeInputDevice == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioCapturerSourceInstance = AudioCapturerSource::GetInstance("usb");
    } else {
        audioCapturerSourceInstance = AudioCapturerSource::GetInstance("primary");
    }

    if (audioCapturerSourceInstance == nullptr || !audioCapturerSourceInstance->IsInited()) {
        AUDIO_WARNING_LOG("Capturer is not initialized.");
    } else {
        audioCapturerSourceInstance->SetAudioScene(audioScene, activeInputDevice);
    }

    if (audioRendererSinkInstance == nullptr || !audioRendererSinkInstance->IsInited()) {
        AUDIO_WARNING_LOG("Renderer is not initialized.");
    } else {
        if (activeOutputDevice == DEVICE_TYPE_BLUETOOTH_A2DP && a2dpOffloadFlag != A2DP_OFFLOAD) {
            activeOutputDevices[0] = DEVICE_TYPE_NONE;
        }
        audioRendererSinkInstance->SetAudioScene(audioScene, activeOutputDevices);
    }

    audioScene_ = audioScene;
    return SUCCESS;
}

int32_t  AudioServer::SetIORoutes(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType type = activeDevices.front().first;
    DeviceFlag flag = activeDevices.front().second;

    std::vector<DeviceType> deviceTypes;
    for (auto activeDevice : activeDevices) {
        AUDIO_INFO_LOG("SetIORoutes device type:%{public}d", activeDevice.first);
        deviceTypes.push_back(activeDevice.first);
    }
    AUDIO_INFO_LOG("SetIORoutes 1st deviceType: %{public}d, flag: %{public}d", type, flag);
    int32_t ret = SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag);
    return ret;
}

int32_t AudioServer::SetIORoutes(DeviceType type, DeviceFlag flag, std::vector<DeviceType> deviceTypes,
    BluetoothOffloadState a2dpOffloadFlag)
{
    IAudioCapturerSource *audioCapturerSourceInstance;
    IAudioRendererSink *audioRendererSinkInstance;
    if (type == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioCapturerSourceInstance = AudioCapturerSource::GetInstance("usb");
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("usb", "");
    } else {
        audioCapturerSourceInstance = AudioCapturerSource::GetInstance("primary");
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
        if (!audioCapturerSourceInstance->IsInited()) {
            audioCapturerSourceInstance = FastAudioCapturerSource::GetInstance();
        }
        if (type == DEVICE_TYPE_BLUETOOTH_A2DP && a2dpOffloadFlag != A2DP_OFFLOAD &&
            deviceTypes.size() == 1 && deviceTypes[0] == DEVICE_TYPE_BLUETOOTH_A2DP) {
            deviceTypes[0] = DEVICE_TYPE_NONE;
        }
    }
    CHECK_AND_RETURN_RET_LOG(audioCapturerSourceInstance != nullptr && audioRendererSinkInstance != nullptr,
        ERR_INVALID_PARAM, "SetIORoutes failed for null instance!");

    std::lock_guard<std::mutex> lock(audioSceneMutex_);
    if (flag == DeviceFlag::INPUT_DEVICES_FLAG) {
        if (audioScene_ != AUDIO_SCENE_DEFAULT) {
            audioCapturerSourceInstance->SetAudioScene(audioScene_, type);
        } else {
            audioCapturerSourceInstance->SetInputRoute(type);
        }
    } else if (flag == DeviceFlag::OUTPUT_DEVICES_FLAG) {
        if (audioScene_ != AUDIO_SCENE_DEFAULT) {
            audioRendererSinkInstance->SetAudioScene(audioScene_, deviceTypes);
        } else {
            audioRendererSinkInstance->SetOutputRoutes(deviceTypes);
        }
        PolicyHandler::GetInstance().SetActiveOutputDevice(type);
    } else if (flag == DeviceFlag::ALL_DEVICES_FLAG) {
        if (audioScene_ != AUDIO_SCENE_DEFAULT) {
            audioCapturerSourceInstance->SetAudioScene(audioScene_, type);
            audioRendererSinkInstance->SetAudioScene(audioScene_, deviceTypes);
        } else {
            audioCapturerSourceInstance->SetInputRoute(type);
            audioRendererSinkInstance->SetOutputRoutes(deviceTypes);
        }
        PolicyHandler::GetInstance().SetActiveOutputDevice(type);
    } else {
        AUDIO_ERR_LOG("SetIORoutes invalid device flag");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t AudioServer::UpdateActiveDeviceRoute(DeviceType type, DeviceFlag flag, BluetoothOffloadState a2dpOffloadFlag)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back(make_pair(type, flag));
    return UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag);
}

int32_t AudioServer::UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    return SetIORoutes(activeDevices, a2dpOffloadFlag);
}

void AudioServer::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("AudioMonoState = [%{public}s]", audioMono ? "true": "false");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "refused for %{public}d", callingUid);
    // Set mono for audio_renderer_sink (primary)
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    if (audioRendererSinkInstance != nullptr) {
        audioRendererSinkInstance->SetAudioMonoState(audioMono);
    } else {
        AUDIO_ERR_LOG("AudioServer::SetAudioBalanceValue: primary = null");
    }

    // Set mono for bluetooth_renderer_sink (a2dp)
    IAudioRendererSink *a2dpIAudioRendererSink = IAudioRendererSink::GetInstance("a2dp", "");
    if (a2dpIAudioRendererSink != nullptr) {
        a2dpIAudioRendererSink->SetAudioMonoState(audioMono);
    } else {
        AUDIO_ERR_LOG("AudioServer::SetAudioBalanceValue: a2dp = null");
    }

    // Set mono for offload_audio_renderer_sink (offload)
    IAudioRendererSink *offloadIAudioRendererSink = IAudioRendererSink::GetInstance("offload", "");
    if (offloadIAudioRendererSink != nullptr) {
        offloadIAudioRendererSink->SetAudioMonoState(audioMono);
    } else {
        AUDIO_ERR_LOG("AudioServer::SetAudioBalanceValue: offload = null");
    }

    // Set mono for audio_renderer_sink (direct)
    IAudioRendererSink *directRenderSink = AudioRendererSink::GetInstance("direct");
    if (directRenderSink != nullptr) {
        directRenderSink->SetAudioMonoState(audioMono);
    } else {
        AUDIO_WARNING_LOG("direct = null");
    }

    // Set mono for audio_renderer_sink (voip)
    IAudioRendererSink *voipRenderSink = AudioRendererSink::GetInstance("voip");
    if (voipRenderSink != nullptr) {
        voipRenderSink->SetAudioMonoState(audioMono);
    } else {
        AUDIO_WARNING_LOG("voip = null");
    }
}

void AudioServer::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("AudioBalanceValue = [%{public}f]", audioBalance);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "refused for %{public}d", callingUid);
    CHECK_AND_RETURN_LOG(audioBalance >= -1.0f && audioBalance <= 1.0f,
        "audioBalance value %{public}f is out of range [-1.0, 1.0]", audioBalance);

    // Set balance for audio_renderer_sink (primary)
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    if (audioRendererSinkInstance != nullptr) {
        audioRendererSinkInstance->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("primary = null");
    }

    // Set balance for bluetooth_renderer_sink (a2dp)
    IAudioRendererSink *a2dpIAudioRendererSink = IAudioRendererSink::GetInstance("a2dp", "");
    if (a2dpIAudioRendererSink != nullptr) {
        a2dpIAudioRendererSink->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("a2dp = null");
    }

    // Set balance for offload_audio_renderer_sink (offload)
    IAudioRendererSink *offloadIAudioRendererSink = IAudioRendererSink::GetInstance("offload", "");
    if (offloadIAudioRendererSink != nullptr) {
        offloadIAudioRendererSink->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("offload = null");
    }

    // Set balance for audio_renderer_sink (direct)
    IAudioRendererSink *directRenderSink = AudioRendererSink::GetInstance("direct");
    if (directRenderSink != nullptr) {
        directRenderSink->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("direct = null");
    }

    // Set balance for audio_renderer_sink (voip)
    IAudioRendererSink *voipRenderSink = AudioRendererSink::GetInstance("voip");
    if (voipRenderSink != nullptr) {
        voipRenderSink->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("voip = null");
    }
}

void AudioServer::NotifyDeviceInfo(std::string networkId, bool connected)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "refused for %{public}d", callingUid);
    AUDIO_INFO_LOG("notify device info: networkId(%{public}s), connected(%{public}d)",
        GetEncryptStr(networkId).c_str(), connected);
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
    if (audioRendererSinkInstance != nullptr && connected) {
        audioRendererSinkInstance->RegisterParameterCallback(this);
    }
}

inline bool IsParamEnabled(std::string key, bool &isEnabled)
{
    int32_t policyFlag = 0;
    if (GetSysPara(key.c_str(), policyFlag) && policyFlag == 1) {
        isEnabled = true;
        return true;
    }
    isEnabled = false;
    return false;
}

int32_t AudioServer::RegiestPolicyProvider(const sptr<IRemoteObject> &object)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    sptr<IPolicyProviderIpc> policyProvider = iface_cast<IPolicyProviderIpc>(object);
    CHECK_AND_RETURN_RET_LOG(policyProvider != nullptr, ERR_INVALID_PARAM,
        "policyProvider obj cast failed");
    bool ret = PolicyHandler::GetInstance().ConfigPolicyProvider(policyProvider);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "ConfigPolicyProvider failed!");
    return SUCCESS;
}

int32_t AudioServer::GetHapBuildApiVersion(int32_t callerUid)
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS);
    std::string bundleName {""};
    AppExecFwk::BundleInfo bundleInfo;
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(saManager != nullptr, 0, "failed: saManager is nullptr");

    sptr<IRemoteObject> remoteObject = saManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, 0, "failed: remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, 0, "failed: bundleMgrProxy is nullptr");

    bundleMgrProxy->GetNameForUid(callerUid, bundleName);
    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);
    int32_t hapApiVersion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_REMAINDER;
    AUDIO_INFO_LOG("callerUid %{public}d, version %{public}d", callerUid, hapApiVersion);
    return hapApiVersion;
}

void AudioServer::ResetRecordConfig(AudioProcessConfig &config)
{
    if (config.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        config.isInnerCapturer = true;
        config.innerCapMode = LEGACY_INNER_CAP;
        if (config.callerUid == MEDIA_SERVICE_UID) {
            config.innerCapMode = MODERN_INNER_CAP;
        } else if (GetHapBuildApiVersion(config.callerUid) >= MODERN_INNER_API_VERSION) { // check build api-version
            config.innerCapMode = LEGACY_MUTE_CAP;
        }
    } else {
        config.isInnerCapturer = false;
    }
#ifdef AUDIO_BUILD_VARIANT_ROOT
    if (config.callerUid == ROOT_UID) {
        config.innerCapMode = MODERN_INNER_CAP;
    }
#endif
    if (config.capturerInfo.sourceType == SourceType::SOURCE_TYPE_WAKEUP) {
        config.isWakeupCapturer = true;
    } else {
        config.isWakeupCapturer = false;
    }
}

AudioProcessConfig AudioServer::ResetProcessConfig(const AudioProcessConfig &config)
{
    AudioProcessConfig resetConfig(config);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();

    resetConfig.callerUid = callerUid;

    // client pid uid check.
    if (RECORD_PASS_APPINFO_LIST.count(callerUid)) {
        AUDIO_INFO_LOG("Create process for %{public}d, clientUid:%{public}d.", callerUid, config.appInfo.appUid);
    } else if (RECORD_CHECK_FORWARD_LIST.count(callerUid)) {
        AUDIO_INFO_LOG("Check forward calling for uid:%{public}d", callerUid);
        resetConfig.appInfo.appTokenId = IPCSkeleton::GetFirstTokenID();
        resetConfig.appInfo.appFullTokenId = IPCSkeleton::GetFirstFullTokenID();
    } else {
        AUDIO_INFO_LOG("Use true client appInfo instead for pid:%{public}d uid:%{public}d", callerPid, callerUid);
        resetConfig.appInfo.appPid = callerPid;
        resetConfig.appInfo.appUid = callerUid;
        resetConfig.appInfo.appTokenId = IPCSkeleton::GetCallingTokenID();
        resetConfig.appInfo.appFullTokenId = IPCSkeleton::GetCallingFullTokenID();
    }

    if (resetConfig.audioMode == AUDIO_MODE_RECORD) {
        ResetRecordConfig(resetConfig);
    }
    return resetConfig;
}

bool AudioServer::CheckStreamInfoFormat(const AudioProcessConfig &config)
{
    if (NotContain(AUDIO_SUPPORTED_SAMPLING_RATES, config.streamInfo.samplingRate)) {
        AUDIO_ERR_LOG("Check format failed invalid samplingRate:%{public}d", config.streamInfo.samplingRate);
        return false;
    }

    if (NotContain(AUDIO_SUPPORTED_FORMATS, config.streamInfo.format)) {
        AUDIO_ERR_LOG("Check format failed invalid format:%{public}d", config.streamInfo.format);
        return false;
    }

    if (NotContain(AUDIO_SUPPORTED_ENCODING_TYPES, config.streamInfo.encoding)) {
        AUDIO_ERR_LOG("Check format failed invalid encoding:%{public}d", config.streamInfo.encoding);
        return false;
    }

    // both renderer and capturer check RENDERER_SUPPORTED_CHANNELLAYOUTS, should we rename it?
    if (NotContain(RENDERER_SUPPORTED_CHANNELLAYOUTS, config.streamInfo.channelLayout)) {
        AUDIO_ERR_LOG("Check format failed invalid channelLayout:%{public}" PRId64".", config.streamInfo.channelLayout);
        return false;
    }

    if (config.audioMode == AUDIO_MODE_PLAYBACK && NotContain(RENDERER_SUPPORTED_CHANNELS,
        config.streamInfo.channels)) {
        AUDIO_ERR_LOG("Check format failed invalid renderer channels:%{public}d", config.streamInfo.channels);
        return false;
    }

    if (config.audioMode == AUDIO_MODE_RECORD && NotContain(CAPTURER_SUPPORTED_CHANNELS, config.streamInfo.channels)) {
        AUDIO_ERR_LOG("Check format failed invalid capturer channels:%{public}d", config.streamInfo.channels);
        return false;
    }

    return true;
}

bool AudioServer::CheckRendererFormat(const AudioProcessConfig &config)
{
    if (NotContain(AUDIO_SUPPORTED_STREAM_USAGES, config.rendererInfo.streamUsage)) {
        AUDIO_ERR_LOG("Check format failed invalid streamUsage:%{public}d", config.rendererInfo.streamUsage);
        return false;
    }
    return true;
}

bool AudioServer::CheckRecorderFormat(const AudioProcessConfig &config)
{
    if (NotContain(AUDIO_SUPPORTED_SOURCE_TYPES, config.capturerInfo.sourceType)) {
        AUDIO_ERR_LOG("Check format failed invalid sourceType:%{public}d", config.capturerInfo.sourceType);
        return false;
    }
    if (config.capturerInfo.capturerFlags != AUDIO_FLAG_NORMAL && NotContain(AUDIO_FAST_STREAM_SUPPORTED_SOURCE_TYPES,
        config.capturerInfo.sourceType)) {
        AUDIO_ERR_LOG("Check format failed invalid fast sourceType:%{public}d", config.capturerInfo.sourceType);
        return false;
    }
    return true;
}

bool AudioServer::CheckConfigFormat(const AudioProcessConfig &config)
{
    if (!CheckStreamInfoFormat(config)) {
        return false;
    }
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        return CheckRendererFormat(config);
    }

    if (config.audioMode == AUDIO_MODE_RECORD) {
        return CheckRecorderFormat(config);
    }

    AUDIO_ERR_LOG("Check format failed invalid mode.");
    return false;
}

const std::string AudioServer::GetBundleNameFromUid(int32_t uid)
{
    AudioXCollie audioXCollie("AudioServer::GetBundleNameFromUid",
        GET_BUNDLE_TIME_OUT_SECONDS);
    std::string bundleName {""};
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, "", "systemAbilityManager is nullptr");

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, "", "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, "", "bundleMgrProxy is nullptr");

    bundleMgrProxy->GetNameForUid(uid, bundleName);

    return bundleName;
}

bool AudioServer::IsFastBlocked(int32_t uid)
{
    std::string bundleName = GetBundleNameFromUid(uid);
    std::string result = GetAudioParameter(CHECK_FAST_BLOCK_PREFIX + bundleName);
    return result == "true";
}

sptr<IRemoteObject> AudioServer::CreateAudioProcess(const AudioProcessConfig &config)
{
    Trace trace("AudioServer::CreateAudioProcess");
    AudioProcessConfig resetConfig = ResetProcessConfig(config);
    CHECK_AND_RETURN_RET_LOG(CheckConfigFormat(resetConfig), nullptr, "AudioProcessConfig format is wrong, please check"
        ":%{public}s", ProcessConfig::DumpProcessConfig(resetConfig).c_str());
    CHECK_AND_RETURN_RET_LOG(PermissionChecker(resetConfig), nullptr, "Create audio process failed, no permission");

    if ((IsNormalIpcStream(resetConfig)) || IsFastBlocked(resetConfig.appInfo.appUid)) {
        AUDIO_INFO_LOG("Create normal ipc stream.");
        int32_t ret = 0;
        sptr<IpcStreamInServer> ipcStream = AudioService::GetInstance()->GetIpcStream(resetConfig, ret);
        CHECK_AND_RETURN_RET_LOG(ipcStream != nullptr, nullptr, "GetIpcStream failed.");
        sptr<IRemoteObject> remoteObject= ipcStream->AsObject();
        return remoteObject;
    }

    sptr<IAudioProcess> process = AudioService::GetInstance()->GetAudioProcess(resetConfig);
    CHECK_AND_RETURN_RET_LOG(process != nullptr, nullptr, "GetAudioProcess failed.");
    sptr<IRemoteObject> remoteObject= process->AsObject();
    return remoteObject;
}

bool AudioServer::IsNormalIpcStream(const AudioProcessConfig &config) const
{
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        return config.rendererInfo.rendererFlags == AUDIO_FLAG_NORMAL ||
            config.rendererInfo.rendererFlags == AUDIO_FLAG_VOIP_DIRECT;
    } else if (config.audioMode == AUDIO_MODE_RECORD) {
        return config.capturerInfo.capturerFlags == AUDIO_FLAG_NORMAL;
    }

    return false;
}

int32_t AudioServer::CheckRemoteDeviceState(std::string networkId, DeviceRole deviceRole, bool isStartDevice)
{
    AUDIO_INFO_LOG("CheckRemoteDeviceState: device[%{public}s] deviceRole[%{public}d] isStartDevice[%{public}s]",
        GetEncryptStr(networkId).c_str(), static_cast<int32_t>(deviceRole), (isStartDevice ? "true" : "false"));

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    CHECK_AND_RETURN_RET(isStartDevice, SUCCESS);

    int32_t ret = SUCCESS;
    switch (deviceRole) {
        case OUTPUT_DEVICE:
            {
                IAudioRendererSink* rendererInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
                if (rendererInstance == nullptr || !rendererInstance->IsInited()) {
                    AUDIO_ERR_LOG("Remote renderer[%{public}s] is uninit.", networkId.c_str());
                    return ERR_ILLEGAL_STATE;
                }
                ret = rendererInstance->Start();
                break;
            }
        case INPUT_DEVICE:
            {
                IAudioCapturerSource *capturerInstance = IAudioCapturerSource::GetInstance("remote", networkId.c_str());
                if (capturerInstance == nullptr || !capturerInstance->IsInited()) {
                    AUDIO_ERR_LOG("Remote capturer[%{public}s] is uninit.", networkId.c_str());
                    return ERR_ILLEGAL_STATE;
                }
                ret = capturerInstance->Start();
                break;
            }
        default:
            AUDIO_ERR_LOG("Remote device role %{public}d is not supported.", deviceRole);
            return ERR_NOT_SUPPORTED;
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Check remote device[%{public}s] fail, ret %{public}d.", networkId.c_str(), ret);
    }
    return ret;
}

void AudioServer::OnAudioSinkParamChange(const std::string &netWorkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    std::shared_ptr<AudioParameterCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(audioParamCbMtx_);
        AUDIO_INFO_LOG("OnAudioSinkParamChange Callback from networkId: %s", netWorkId.c_str());
        CHECK_AND_RETURN_LOG(audioParamCb_ != nullptr, "OnAudioSinkParamChange: audio param allback is null.");
        callback = audioParamCb_;
    }
    callback->OnAudioParameterChange(netWorkId, key, condition, value);
}

void AudioServer::OnAudioSourceParamChange(const std::string &netWorkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    std::shared_ptr<AudioParameterCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(audioParamCbMtx_);
        AUDIO_INFO_LOG("OnAudioSourceParamChange Callback from networkId: %s", netWorkId.c_str());
        CHECK_AND_RETURN_LOG(audioParamCb_ != nullptr, "OnAudioSourceParamChange: audio param allback is null.");
        callback = audioParamCb_;
    }
    callback->OnAudioParameterChange(netWorkId, key, condition, value);
}

void AudioServer::OnWakeupClose()
{
    AUDIO_INFO_LOG("OnWakeupClose Callback start");
    std::shared_ptr<WakeUpSourceCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(setWakeupCloseCallbackMutex_);
        CHECK_AND_RETURN_LOG(wakeupCallback_ != nullptr, "OnWakeupClose callback is nullptr.");
        callback = wakeupCallback_;
    }
    callback->OnWakeupClose();
}

void AudioServer::OnCapturerState(bool isActive, int32_t num)
{
    AUDIO_DEBUG_LOG("OnCapturerState Callback start");
    std::shared_ptr<WakeUpSourceCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(setWakeupCloseCallbackMutex_);
        callback = wakeupCallback_;
    }

    // Ensure that the send callback is not executed concurrently
    std::lock_guard<std::mutex> lockCb(onCapturerStateCbMutex_);

    uint64_t previousStateFlag;
    uint64_t currentStateFlag;
    if (isActive) {
        uint64_t tempFlag = static_cast<uint64_t>(1) << num;
        previousStateFlag = capturerStateFlag_.fetch_or(tempFlag);
        currentStateFlag = previousStateFlag | tempFlag;
    } else {
        uint64_t tempFlag = ~(static_cast<uint64_t>(1) << num);
        previousStateFlag = capturerStateFlag_.fetch_and(tempFlag);
        currentStateFlag = previousStateFlag & tempFlag;
    }
    bool previousState = previousStateFlag;
    bool currentState = currentStateFlag;

    if (previousState == currentState) {
        // state not change, need not trigger callback
        return;
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "OnCapturerState callback is nullptr.");
    callback->OnCapturerState(isActive);
}

int32_t AudioServer::SetParameterCallback(const sptr<IRemoteObject>& object)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    std::lock_guard<std::mutex> lock(audioParamCbMtx_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "AudioServer:set listener object is nullptr");

    sptr<IStandardAudioServerManagerListener> listener = iface_cast<IStandardAudioServerManagerListener>(object);

    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "AudioServer: listener obj cast failed");

    std::shared_ptr<AudioParameterCallback> callback = std::make_shared<AudioManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "AudioPolicyServer: failed to  create cb obj");

    audioParamCb_ = callback;
    AUDIO_INFO_LOG("AudioServer:: SetParameterCallback  done");

    return SUCCESS;
}

int32_t AudioServer::SetWakeupSourceCallback(const sptr<IRemoteObject>& object)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == INTELL_VOICE_SERVICR_UID, false,
        "SetWakeupSourceCallback refused for %{public}d", callingUid);

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetWakeupCloseCallback set listener object is nullptr");

    sptr<IStandardAudioServerManagerListener> listener = iface_cast<IStandardAudioServerManagerListener>(object);

    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetWakeupCloseCallback listener obj cast failed");

    std::shared_ptr<AudioManagerListenerCallback> wakeupCallback
        = std::make_shared<AudioManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(wakeupCallback != nullptr, ERR_INVALID_PARAM,
        "SetWakeupCloseCallback failed to create cb obj");

    {
        std::lock_guard<std::mutex> lockSet(setWakeupCloseCallbackMutex_);
        wakeupCallback_ = wakeupCallback;
    }

    std::thread([this, wakeupCallback] {
        std::lock_guard<std::mutex> lockCb(onCapturerStateCbMutex_);
        wakeupCallback->TrigerFirstOnCapturerStateCallback(capturerStateFlag_);
    }).detach();

    AUDIO_INFO_LOG("SetWakeupCloseCallback done");

    return SUCCESS;
}

bool AudioServer::VerifyClientPermission(const std::string &permissionName,
    Security::AccessToken::AccessTokenID tokenId)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("[%{public}s] for uid:%{public}d tokenId:%{public}u", permissionName.c_str(), callerUid, tokenId);

#ifdef AUDIO_BUILD_VARIANT_ROOT
    // Root users should be whitelisted
    if (callerUid == ROOT_UID) {
        AUDIO_INFO_LOG("Root user. Permission GRANTED!!!");
        return true;
    }
#endif
    Security::AccessToken::AccessTokenID clientTokenId = tokenId;
    if (clientTokenId == Security::AccessToken::INVALID_TOKENID) {
        clientTokenId = IPCSkeleton::GetCallingTokenID();
    }
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(clientTokenId, permissionName);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, "Permission denied [tid:%{public}d]", clientTokenId);

    return true;
}

bool AudioServer::PermissionChecker(const AudioProcessConfig &config)
{
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        return CheckPlaybackPermission(config);
    }

    if (config.audioMode == AUDIO_MODE_RECORD) {
        return CheckRecorderPermission(config);
    }

    AUDIO_ERR_LOG("Check failed invalid mode.");
    return false;
}

bool AudioServer::CheckPlaybackPermission(const AudioProcessConfig &config)
{
    StreamUsage streamUsage = config.rendererInfo.streamUsage;

    bool needVerifyPermission = false;
    for (const auto& item : STREAMS_NEED_VERIFY_SYSTEM_PERMISSION) {
        if (streamUsage == item) {
            needVerifyPermission = true;
            break;
        }
    }
    if (needVerifyPermission == false) {
        return true;
    }
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), false,
        "Check playback permission failed, no system permission");
    return true;
}

bool AudioServer::CheckRecorderPermission(const AudioProcessConfig &config)
{
    Security::AccessToken::AccessTokenID tokenId = config.appInfo.appTokenId;
    uint64_t fullTokenId = config.appInfo.appFullTokenId;
    SourceType sourceType = config.capturerInfo.sourceType;
    CHECK_AND_RETURN_RET_LOG(VALID_SOURCE_TYPE.count(sourceType), false, "invalid source type:%{public}d", sourceType);

#ifdef AUDIO_BUILD_VARIANT_ROOT
    int32_t appUid = config.appInfo.appUid;
    if (appUid == ROOT_UID) {
        return true;
    }
#endif

    AUDIO_INFO_LOG("check for uid:%{public}d source type:%{public}d", config.callerUid, sourceType);

    if (sourceType == SOURCE_TYPE_VOICE_CALL) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission, false, "VOICE_CALL failed: no system permission.");

        bool res = CheckVoiceCallRecorderPermission(tokenId);
        return res;
    }

    if (sourceType == SOURCE_TYPE_REMOTE_CAST) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission, false,
            "Create source remote cast failed: no system permission.");

        bool hasCastAudioOutputPermission = VerifyClientPermission(CAST_AUDIO_OUTPUT_PERMISSION, tokenId);
        CHECK_AND_RETURN_RET_LOG(hasCastAudioOutputPermission, false, "No cast audio output permission");
        return true;
    }

    if (sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE && config.innerCapMode == MODERN_INNER_CAP) {
        AUDIO_INFO_LOG("modern inner-cap source, no need to check.");
        return true;
    }

    // All record streams should be checked for MICROPHONE_PERMISSION
    bool res = VerifyClientPermission(MICROPHONE_PERMISSION, tokenId);
    CHECK_AND_RETURN_RET_LOG(res, false, "Check record permission failed: No permission.");

    if (sourceType == SOURCE_TYPE_WAKEUP) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        bool hasIntelVoicePermission = VerifyClientPermission(MANAGE_INTELLIGENT_VOICE_PERMISSION, tokenId);
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission && hasIntelVoicePermission, false,
            "Create wakeup record stream failed: no permission.");
        return true;
    }

    if (PermissionUtil::NeedVerifyBackgroundCapture(config.callerUid, sourceType) &&
        !PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId)) {
        AUDIO_ERR_LOG("VerifyBackgroundCapture failed uid:%{public}d", config.callerUid);
        return false;
    }

    return true;
}

bool AudioServer::CheckVoiceCallRecorderPermission(Security::AccessToken::AccessTokenID tokenId)
{
    bool hasRecordVoiceCallPermission = VerifyClientPermission(RECORD_VOICE_CALL_PERMISSION, tokenId);
    CHECK_AND_RETURN_RET_LOG(hasRecordVoiceCallPermission, false, "No permission");
    return true;
}

void AudioServer::AudioServerDied(pid_t pid)
{
    AUDIO_INFO_LOG("Policy server died: restart pulse audio");
    _Exit(0);
}

void AudioServer::RegisterPolicyServerDeathRecipient()
{
    AUDIO_INFO_LOG("Register policy server death recipient");
    pid_t pid = IPCSkeleton::GetCallingPid();
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(pid);
    if (deathRecipient_ != nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_LOG(samgr != nullptr, "Failed to obtain system ability manager");
        sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::AUDIO_POLICY_SERVICE_ID);
        CHECK_AND_RETURN_LOG(object != nullptr, "Policy service unavailable");
        deathRecipient_->SetNotifyCb(std::bind(&AudioServer::AudioServerDied, this, std::placeholders::_1));
        bool result = object->AddDeathRecipient(deathRecipient_);
        if (!result) {
            AUDIO_ERR_LOG("Failed to add deathRecipient");
        }
    }
}

void AudioServer::RequestThreadPriority(uint32_t tid, string bundleName)
{
    AUDIO_INFO_LOG("RequestThreadPriority tid: %{public}u", tid);

    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioXCollie audioXCollie("AudioServer::ScheduleReportData", SCHEDULE_REPORT_TIME_OUT_SECONDS);
    ScheduleReportData(pid, tid, bundleName.c_str());
}

bool AudioServer::CreatePlaybackCapturerManager()
{
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return false;
    }
    std::vector<int32_t> usage;
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetSupportStreamUsage(usage);
    return true;
}

int32_t AudioServer::SetSupportStreamUsage(std::vector<int32_t> usage)
{
    AUDIO_INFO_LOG("SetSupportStreamUsage with usage num:%{public}zu", usage.size());

    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return ERR_OPERATION_FAILED;
    }
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetSupportStreamUsage(usage);
    return SUCCESS;
}

void AudioServer::RegisterAudioCapturerSourceCallback()
{
    IAudioCapturerSource* audioCapturerSourceWakeupInstance =
        IAudioCapturerSource::GetInstance("primary", nullptr, SOURCE_TYPE_WAKEUP);
    if (audioCapturerSourceWakeupInstance != nullptr) {
        audioCapturerSourceWakeupInstance->RegisterWakeupCloseCallback(this);
    }

    IAudioCapturerSource* primaryAudioCapturerSourceInstance =
        IAudioCapturerSource::GetInstance("primary", nullptr, SOURCE_TYPE_MIC);
    IAudioCapturerSource *usbAudioCapturerSinkInstance = IAudioCapturerSource::GetInstance("usb", "");
    IAudioCapturerSource *fastAudioCapturerSourceInstance = FastAudioCapturerSource::GetInstance();
    IAudioCapturerSource *voipFastAudioCapturerSourceInstance = FastAudioCapturerSource::GetVoipInstance();

    for (auto audioCapturerSourceInstance : {
        primaryAudioCapturerSourceInstance,
        usbAudioCapturerSinkInstance,
        fastAudioCapturerSourceInstance,
        voipFastAudioCapturerSourceInstance
    }) {
        if (audioCapturerSourceInstance != nullptr) {
            audioCapturerSourceInstance->RegisterAudioCapturerSourceCallback(make_unique<CapturerStateOb>(
                [this] (bool isActive, int32_t num) {
                    this->OnCapturerState(isActive, num);
                }));
        }
    }
}

int32_t AudioServer::SetCaptureSilentState(bool state)
{
    int32_t audio_policy_server_id = 1041;
    if (IPCSkeleton::GetCallingUid() != audio_policy_server_id) {
        return ERR_OPERATION_FAILED;
    }

    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetCaptureSilentState(state);
    return SUCCESS;
}

int32_t AudioServer::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        AUDIO_ERR_LOG("audioEffectChainManager is nullptr");
        return ERROR;
    }
    return audioEffectChainManager->UpdateSpatializationState(spatializationState);
}

int32_t AudioServer::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");

    return audioEffectChainManager->UpdateSpatialDeviceType(spatialDeviceType);
}

int32_t AudioServer::NotifyStreamVolumeChanged(AudioStreamType streamType, float volume)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != audioUid_) {
        AUDIO_ERR_LOG("NotifyStreamVolumeChanged refused for %{public}d", callingUid);
        return ERR_NOT_SUPPORTED;
    }
    return AudioService::GetInstance()->NotifyStreamVolumeChanged(streamType, volume);
}

int32_t AudioServer::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->SetSpatializationSceneType(spatializationSceneType);
}

int32_t AudioServer::ResetRouteForDisconnect(DeviceType type)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("audioRendererSinkInstance is null!");
        return ERROR;
    }
    audioRendererSinkInstance->ResetOutputRouteForDisconnect(type);

    // todo reset capturer

    return SUCCESS;
}

uint32_t AudioServer::GetEffectLatency(const std::string &sessionId)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetLatency(sessionId);
}

float AudioServer::GetMaxAmplitude(bool isOutputDevice, int32_t deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, 0, "GetMaxAmplitude refused for %{public}d", callingUid);

    float fastMaxAmplitude = AudioService::GetInstance()->GetMaxAmplitude(isOutputDevice);
    if (isOutputDevice) {
        IAudioRendererSink *iRendererInstance = nullptr;
        if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            iRendererInstance = IAudioRendererSink::GetInstance("a2dp", "");
        } else if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            iRendererInstance = IAudioRendererSink::GetInstance("usb", "");
        } else {
            iRendererInstance = IAudioRendererSink::GetInstance("primary", "");
        }
        if (iRendererInstance != nullptr) {
            float normalMaxAmplitude = iRendererInstance->GetMaxAmplitude();
            return (normalMaxAmplitude > fastMaxAmplitude) ? normalMaxAmplitude : fastMaxAmplitude;
        }
    } else {
        AudioCapturerSource *audioCapturerSourceInstance;
        if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            audioCapturerSourceInstance = AudioCapturerSource::GetInstance("usb");
        } else {
            audioCapturerSourceInstance = AudioCapturerSource::GetInstance("primary");
        }
        if (audioCapturerSourceInstance != nullptr) {
            float normalMaxAmplitude = audioCapturerSourceInstance->GetMaxAmplitude();
            return (normalMaxAmplitude > fastMaxAmplitude) ? normalMaxAmplitude : fastMaxAmplitude;
        }
    }

    return 0;
}

void AudioServer::ResetAudioEndpoint()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->ResetAudioEndpoint();
}

void AudioServer::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    if (isRenderer) {
        LatencyMonitor::GetInstance().UpdateClientTime(true, timestamp);
    } else {
        LatencyMonitor::GetInstance().UpdateClientTime(false, timestamp);
        LatencyMonitor::GetInstance().ShowTimestamp(false);
    }
}

bool AudioServer::GetEffectOffloadEnabled()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERROR, "audioEffectChainManager is nullptr");
    return audioEffectChainManager->GetOffloadEnabled();
}

int32_t AudioServer::UpdateDualToneState(bool enable, int32_t sessionId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    if (enable) {
        return AudioService::GetInstance()->EnableDualToneList(static_cast<uint32_t>(sessionId));
    } else {
        return AudioService::GetInstance()->DisableDualToneList(static_cast<uint32_t>(sessionId));
    }
}

int32_t AudioServer::SetSinkRenderEmpty(const std::string &devceClass, int32_t durationUs)
{
    if (durationUs <= 0) {
        return SUCCESS;
    }
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");

    return audioRendererSinkInstance->SetRenderEmpty(durationUs);
}

int32_t AudioServer::SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callingUid == audioUid_, ERR_PERMISSION_DENIED, "refused for %{public}d", callingUid);

    if (durationUs <= 0) {
        return SUCCESS;
    }

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance(devceClass.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->SetSinkMuteForSwitchDevice(mute);
}

void AudioServer::LoadHdiEffectModel()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "load hdi effect model refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->InitHdiState();
}

void AudioServer::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->UpdateEffectBtOffloadSupported(isSupported);
}

void AudioServer::SetRotationToEffect(const uint32_t rotate)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(callingUid == audioUid_, "set rotation to effect refused for %{public}d", callingUid);

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "audioEffectChainManager is nullptr");
    audioEffectChainManager->EffectRotationUpdate(rotate);
}
} // namespace AudioStandard
} // namespace OHOS
