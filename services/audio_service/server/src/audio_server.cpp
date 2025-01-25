/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <dlfcn.h>
#include <format>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hisysevent.h"
#include "parameters.h"

#include "audio_capturer_source.h"
#include "bluetooth_capturer_source.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_asr.h"
#include "audio_manager_listener_proxy.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "i_audio_capturer_source.h"
#include "i_audio_renderer_sink.h"
#include "audio_renderer_sink.h"
#include "i_standard_audio_server_manager_listener.h"
#ifdef HAS_FEATURE_INNERCAPTURER
#include "playback_capturer_manager.h"
#endif
#include "config/audio_param_parser.h"
#include "media_monitor_manager.h"
#include "offline_stream_in_server.h"
#include "audio_dump_pcm.h"
#include "audio_info.h"

#ifdef SUPPORT_LOW_LATENCY
#include "fast_audio_renderer_sink.h"
#include "fast_audio_capturer_source.h"
#endif

#define PA
#ifdef PA
extern "C" {
    extern int ohos_pa_main(int argc, char *argv[]);
}
#endif

using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t INTELL_VOICE_SERVICR_UID = 1042;
constexpr int32_t SYSTEM_STATUS_START = 1;
constexpr int32_t SYSTEM_STATUS_STOP = 0;
constexpr int32_t SYSTEM_PROCESS_TYPE = 1;
uint32_t AudioServer::paDaemonTid_;
std::map<std::string, std::string> AudioServer::audioParameters;
std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> AudioServer::audioParameterKeys;
const string DEFAULT_COOKIE_PATH = "/data/data/.pulse_dir/state/cookie";
const std::string CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
constexpr const char *TEL_SATELLITE_SUPPORT = "const.telephony.satellite.supported";
const std::string SATEMODEM_PARAMETER = "usedmodem=satemodem";
const std::string PCM_DUMP_KEY = "PCM_DUMP";
constexpr int32_t UID_FOUNDATION_SA = 5523;
const unsigned int TIME_OUT_SECONDS = 10;
static const int32_t INVALID_APP_UID = -1;
static const int32_t INVALID_APP_CREATED_AUDIO_STREAM_NUM = -1;
const char* DUMP_AUDIO_PERMISSION = "ohos.permission.DUMP_AUDIO";
const char* MANAGE_INTELLIGENT_VOICE_PERMISSION = "ohos.permission.MANAGE_INTELLIGENT_VOICE";
const char* CAST_AUDIO_OUTPUT_PERMISSION = "ohos.permission.CAST_AUDIO_OUTPUT";
const char* CAPTURE_PLAYBACK_PERMISSION = "ohos.permission.CAPTURE_PLAYBACK";
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
static const size_t PARAMETER_SET_LIMIT = 1024;
constexpr int32_t UID_CAMERA = 1047;
constexpr int32_t MAX_RENDERER_STREAM_CNT_PER_UID = 40;
const int32_t DEFAULT_MAX_RENDERER_INSTANCES = 128;
const int32_t MCU_UID = 7500;
static const std::set<int32_t> RECORD_CHECK_FORWARD_LIST = {
    VM_MANAGER_UID,
    UID_CAMERA
};
// using pass-in appInfo for uids:
constexpr int32_t UID_MEDIA_SA = 1013;
enum PermissionStatus {
    PERMISSION_GRANTED = 0,
    PERMISSION_DENIED = 1,
    PERMISSION_UNKNOWN = 2,
};

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
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED
};

static constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;

static const std::vector<SourceType> AUDIO_SUPPORTED_SOURCE_TYPES = {
    SOURCE_TYPE_INVALID,
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
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
};

static const std::vector<SourceType> AUDIO_FAST_STREAM_SUPPORTED_SOURCE_TYPES = {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
};

static bool IsNeedVerifyPermission(const StreamUsage streamUsage)
{
    for (const auto& item : STREAMS_NEED_VERIFY_SYSTEM_PERMISSION) {
        if (streamUsage == item) {
            return true;
        }
    }
    return false;
}

static std::string GetField(const std::string &src, const char* field, const char sep)
{
    auto str = std::string(field) + '=';
    auto pos = src.find(str);
    CHECK_AND_RETURN_RET(pos != std::string::npos, "");
    pos += str.length();
    auto end = src.find(sep, pos);
    return end == std::string::npos ? src.substr(pos) : src.substr(pos, end - pos);
}

static void UpdateArmInstance(IAudioCapturerSource *&audioCapturerSourceInstance,
    IAudioRendererSink *&audioRendererSinkInstance)
{
    audioCapturerSourceInstance = AudioCapturerSource::GetInstance("usb");
    audioRendererSinkInstance = IAudioRendererSink::GetInstance("usb", "");
    auto primarySink = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_LOG(primarySink, "primarySink is nullptr");
    primarySink->ResetOutputRouteForDisconnect(DEVICE_TYPE_NONE);
}

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

REGISTER_SYSTEM_ABILITY_BY_ID(AudioServer, AUDIO_DISTRIBUTED_SERVICE_ID, true)

#ifdef PA
constexpr int PA_ARG_COUNT = 1;

void *AudioServer::paDaemonThread(void *arg)
{
    /* Load the mandatory pulseaudio modules at start */
    char *argv[] = {
        (char*)"pulseaudio",
    };
    // set audio thread priority
    ScheduleThreadInServer(getpid(), gettid());
    paDaemonTid_ = static_cast<uint32_t>(gettid());
    AUDIO_INFO_LOG("Calling ohos_pa_main\n");
    ohos_pa_main(PA_ARG_COUNT, argv);
    AUDIO_INFO_LOG("Exiting ohos_pa_main\n");
    UnscheduleThreadInServer(getpid(), gettid());
    _exit(-1);
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

void AudioServer::InitMaxRendererStreamCntPerUid()
{
    bool result = GetSysPara("const.multimedia.audio.stream_cnt_uid", maxRendererStreamCntPerUid_);
    if (!result || maxRendererStreamCntPerUid_ <= 0) {
        maxRendererStreamCntPerUid_ = MAX_RENDERER_STREAM_CNT_PER_UID;
    }
}

void AudioServer::OnStart()
{
    AUDIO_INFO_LOG("OnStart uid:%{public}d", getuid());
    InitMaxRendererStreamCntPerUid();
    AudioInnerCall::GetInstance()->RegisterAudioServer(this);
    bool res = Publish(this);
    if (!res) {
        AUDIO_ERR_LOG("start err");
        WriteServiceStartupError();
    }
    int32_t fastControlFlag = 1; // default 1, set isFastControlled_ true
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", fastControlFlag);
    if (fastControlFlag == 0) {
        isFastControlled_ = false;
    }
    int32_t audioCacheState = 0;
    GetSysPara("persist.multimedia.audio.audioCacheState", audioCacheState);
    if (audioCacheState != 0) {
        AudioCacheMgr::GetInstance().Init();
    }
    AddSystemAbilityListener(AUDIO_POLICY_SERVICE_ID);
    AddSystemAbilityListener(RES_SCHED_SYS_ABILITY_ID);
    AddSystemAbilityListener(MEMORY_MANAGER_SA_ID);
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
    RegisterAudioRendererSinkCallback();

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
        case MEMORY_MANAGER_SA_ID:
            NotifyProcessStatus(true);
            break;
        default:
            AUDIO_ERR_LOG("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
}

void AudioServer::NotifyProcessStatus(bool isStart)
{
    int pid = getpid();
    void *libMemMgrClientHandle = dlopen("libmemmgrclient.z.so", RTLD_NOW);
    if (!libMemMgrClientHandle) {
        AUDIO_ERR_LOG("dlopen libmemmgrclient library failed");
        return;
    }
    void *notifyProcessStatusFunc = dlsym(libMemMgrClientHandle, "notify_process_status");
    if (!notifyProcessStatusFunc) {
        AUDIO_ERR_LOG("dlsm notify_process_status failed");
        dlclose(libMemMgrClientHandle);
        return;
    }
    auto notifyProcessStatus = reinterpret_cast<int(*)(int, int, int, int)>(notifyProcessStatusFunc);
    if (isStart) {
        AUDIO_ERR_LOG("notify to memmgr when audio_server is started");
        // 1 indicates the service is started
        notifyProcessStatus(pid, SYSTEM_PROCESS_TYPE, SYSTEM_STATUS_START, AUDIO_DISTRIBUTED_SERVICE_ID);
    } else {
        AUDIO_ERR_LOG("notify to memmgr when audio_server is stopped");
        // 0 indicates the service is stopped
        notifyProcessStatus(pid, SYSTEM_PROCESS_TYPE, SYSTEM_STATUS_STOP, AUDIO_DISTRIBUTED_SERVICE_ID);
    }
    dlclose(libMemMgrClientHandle);
}

void AudioServer::OnStop()
{
    AUDIO_DEBUG_LOG("OnStop");
    NotifyProcessStatus(false);
}

bool AudioServer::SetPcmDumpParameter(const std::vector<std::pair<std::string, std::string>> &params)
{
    bool ret = VerifyClientPermission(DUMP_AUDIO_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, false, "set audiodump parameters failed: no permission.");
    CHECK_AND_RETURN_RET_LOG(params.size() > 0, false, "params is empty!");
    return AudioCacheMgr::GetInstance().SetDumpParameter(params);
}

int32_t AudioServer::SetExtraParameters(const std::string& key,
    const std::vector<std::pair<std::string, std::string>>& kvpairs)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "set extra parameters failed: not system app.");
    ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "set extra parameters failed: no permission.");

    if (key == PCM_DUMP_KEY) {
        ret = SetPcmDumpParameter(kvpairs);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "set audiodump parameters failed");
        return SUCCESS;
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
            if (it->first == "unprocess_audio_effect") {
                int appUid = IPCSkeleton::GetCallingUid();
                AUDIO_INFO_LOG("add unprocess UID [%{public}d]", appUid);
                IStreamManager::GetRecorderManager().AddUnprocessStream(appUid);
                continue;
            }
            auto valueIter = subKeyIt->second.find("effect");
            if (valueIter != subKeyIt->second.end()) {
                RecognizeAudioEffectType(key, it->first, it->second);
            }
        } else {
            match = false;
            break;
        }
    }
    if (!match) { return ERR_INVALID_PARAM; }

    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    audioRendererSinkInstance->SetAudioParameter(AudioParamKey::NONE, "", value);
    return SUCCESS;
}

void AudioServer::SetA2dpAudioParameter(const std::string &renderValue)
{
    auto parmKey = AudioParamKey::A2DP_SUSPEND_STATE;
    IAudioRendererSink* bluetoothSinkInstance = IAudioRendererSink::GetInstance("a2dp", "");
    CHECK_AND_RETURN_LOG(bluetoothSinkInstance != nullptr, "has no valid sink");
    bluetoothSinkInstance->SetAudioParameter(parmKey, "", renderValue);

    if (AudioService::GetInstance()->HasBluetoothEndpoint()) {
        IAudioRendererSink* fastBluetoothSinkInstance = IAudioRendererSink::GetInstance("a2dp_fast", "");
        CHECK_AND_RETURN_LOG(fastBluetoothSinkInstance != nullptr, "has no valid fast sink");
        fastBluetoothSinkInstance->SetAudioParameter(parmKey, "", renderValue);
        AUDIO_INFO_LOG("HasBlueToothEndpoint");
    }
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
        CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "A2dp offload modify audio settings permission denied");
    }

    CHECK_AND_RETURN_LOG(audioParameters.size() < PARAMETER_SET_LIMIT,
        "SetAudioParameter failed! audioParameters_map is too large!");
    AudioServer::audioParameters[key] = value;

    // send it to hal
    if (key == "A2dpSuspended") {
        std::string renderValue = key + "=" + value + ";";
        SetA2dpAudioParameter(renderValue);
        return;
    }

    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    CHECK_AND_RETURN_LOG(audioRendererSinkInstance != nullptr, "has no valid sink");

    AudioParamKey parmKey = AudioParamKey::NONE;
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

int32_t AudioServer::SuspendRenderSink(const std::string &sinkName)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance(sinkName.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->SuspendRenderSink();
}

int32_t AudioServer::RestoreRenderSink(const std::string &sinkName)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance(sinkName.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->RestoreRenderSink();
}

void AudioServer::SetAudioParameter(const std::string& networkId, const AudioParamKey key, const std::string& condition,
    const std::string& value)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bool ret = VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio() || ret, "refused for %{public}d", callingUid);
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
    CHECK_AND_RETURN_LOG(audioRendererSinkInstance != nullptr, "has no valid sink");

    audioRendererSinkInstance->SetAudioParameter(key, condition, value);
}

bool AudioServer::GetPcmDumpParameter(const std::vector<std::string> &subKeys,
    std::vector<std::pair<std::string, std::string>> &result)
{
    bool ret = VerifyClientPermission(DUMP_AUDIO_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, false, "get audiodump parameters no permission");
    CHECK_AND_RETURN_RET_LOG(subKeys.size() > 0, false, "subKeys is empty!");
    return AudioCacheMgr::GetInstance().GetDumpParameter(subKeys, result);
}

int32_t AudioServer::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    if (mainKey == PCM_DUMP_KEY) {
        bool ret = GetPcmDumpParameter(subKeys, result);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "get audiodump parameters failed");
        return SUCCESS;
    }

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
        AudioXCollie audioXCollie("AudioServer::PrintStackTrace", 1);
        sleep(2); // sleep 2 seconds to dump stacktrace
        return true;
    } else if (key == "recovery_audio_server") {
        AudioXCollie audioXCollie("AudioServer::Kill", 1, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_RECOVERY);
        sleep(2); // sleep 2 seconds to dump stacktrace
        return true;
    } else if (key == "dump_pa_stacktrace_and_kill") {
        AudioXCollie audioXCollie("AudioServer::PrintStackTraceAndKill", 1, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
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
        if (key.find("need_change_usb_device#C", 0) == 0) {
            parmKey = AudioParamKey::USB_DEVICE;
            return audioRendererSinkInstance->GetAudioParameter(AudioParamKey(parmKey), key);
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

const std::string AudioServer::GetUsbParameter(const std::string &condition)
{
    AUDIO_INFO_LOG("AudioServer::GetUsbParameter Entry. condition=%{public}s", condition.c_str());
    string address = GetField(condition, "address", ' ');
    int32_t deviceRoleNum = static_cast<int32_t>(DEVICE_ROLE_NONE);
    std::string usbInfoStr;
    CHECK_AND_RETURN_RET_LOG(StringConverter(GetField(condition, "role", ' '), deviceRoleNum), usbInfoStr,
        "convert invalid value: %{public}s", GetField(condition, "role", ' ').c_str());
    DeviceRole role = static_cast<DeviceRole>(deviceRoleNum);
    IAudioRendererSink *rendererSink = IAudioRendererSink::GetInstance("usb", "");
    CHECK_AND_RETURN_RET_LOG(rendererSink, "", "rendererSink is nullptr");
    std::string infoCond = std::string("get_usb_info#C") + GetField(address, "card", ';') + "D0";
    if (role == OUTPUT_DEVICE) {
        rendererSink->SetAddress(address);
        auto it = usbInfoMap_.find(address);
        if (it == usbInfoMap_.end()) {
            usbInfoStr = rendererSink->GetAudioParameter(USB_DEVICE, infoCond);
            usbInfoMap_[address] = usbInfoStr;
        } else {
            usbInfoStr = it->second;
        }
        rendererSink->Preload(usbInfoStr);
    } else if (role == INPUT_DEVICE) {
        IAudioCapturerSource *capturerSource = IAudioCapturerSource::GetInstance("usb", "");
        CHECK_AND_RETURN_RET_LOG(capturerSource, "", "capturerSource is nullptr");
        capturerSource->SetAddress(address);
        auto it = usbInfoMap_.find(address);
        if (it == usbInfoMap_.end()) {
            usbInfoStr = rendererSink->GetAudioParameter(USB_DEVICE, infoCond);
            usbInfoMap_[address] = usbInfoStr;
        } else {
            usbInfoStr = it->second;
        }
    } else {
        usbInfoMap_.erase(address);
    }
    AUDIO_INFO_LOG("infoCond=%{public}s, usbInfoStr=%{public}s", infoCond.c_str(), usbInfoStr.c_str());
    return usbInfoStr;
}

const std::string AudioServer::GetAudioParameter(const std::string& networkId, const AudioParamKey key,
    const std::string& condition)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio() ||
        VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION), "", "refused for %{public}d", callingUid);

    if (networkId == LOCAL_NETWORK_ID) {
        AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS);
        if (key == AudioParamKey::USB_DEVICE) {
            return GetUsbParameter(condition);
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

int32_t AudioServer::SetMicrophoneMute(bool isMute)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "refused for %{public}d",
        callingUid);

    std::vector<IAudioCapturerSource *> allSourcesInstance;
    IAudioCapturerSource::GetAllInstance(allSourcesInstance);
    for (auto it = allSourcesInstance.begin(); it != allSourcesInstance.end(); ++it) {
        (*it)->SetMute(isMute);
    }

    int32_t ret = SetMicrophoneMuteForEnhanceChain(isMute);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetMicrophoneMuteForEnhanceChain failed.");
    }
    return SUCCESS;
}

int32_t AudioServer::SetVoiceVolume(float volume)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d",
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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    AudioXCollie audioXCollie("AudioServer::SetAudioScene", TIME_OUT_SECONDS);
    AudioCapturerSource *audioCapturerSourceInstance;
    IAudioRendererSink *audioRendererSinkInstance;
    if (activeOutputDevice == DEVICE_TYPE_USB_ARM_HEADSET) {
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("usb", "");
        auto primarySink = IAudioRendererSink::GetInstance("primary", "");
        CHECK_AND_RETURN_RET_LOG(primarySink, ERROR, "primarySink is nullptr");
        primarySink->ResetOutputRouteForDisconnect(DEVICE_TYPE_NONE);
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

int32_t AudioServer::SetIORoutes(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
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
    AUDIO_INFO_LOG("SetIORoutes 1st deviceType: %{public}d, flag: %{public}d deviceName:%{public}s",
        type, flag, deviceName.c_str());
    int32_t ret = SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    return ret;
}

int32_t AudioServer::SetIORoutes(DeviceType type, DeviceFlag flag, std::vector<DeviceType> deviceTypes,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
{
    IAudioCapturerSource *audioCapturerSourceInstance;
    IAudioRendererSink *audioRendererSinkInstance;
    if (type == DEVICE_TYPE_USB_ARM_HEADSET) {
        UpdateArmInstance(audioCapturerSourceInstance, audioRendererSinkInstance);
    } else {
        audioCapturerSourceInstance = AudioCapturerSource::GetInstance("primary");
        audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
        if (!audioCapturerSourceInstance->IsInited()) {
#ifdef SUPPORT_LOW_LATENCY
            audioCapturerSourceInstance = FastAudioCapturerSource::GetInstance();
#endif
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
            audioCapturerSourceInstance->SetAudioScene(audioScene_, type, deviceName);
        } else {
            audioCapturerSourceInstance->SetInputRoute(type, deviceName);
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
            audioCapturerSourceInstance->SetAudioScene(audioScene_, type, deviceName);
            audioRendererSinkInstance->SetAudioScene(audioScene_, deviceTypes);
        } else {
            audioCapturerSourceInstance->SetInputRoute(type, deviceName);
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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back(make_pair(type, flag));
    return UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag);
}

int32_t AudioServer::UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    return SetIORoutes(activeDevices, a2dpOffloadFlag, deviceName);
}

void AudioServer::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("AudioMonoState = [%{public}s]", audioMono ? "true": "false");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "refused for %{public}d", callingUid);
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
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "refused for %{public}d", callingUid);
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
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "refused for %{public}d", callingUid);
    AUDIO_INFO_LOG("notify device info: networkId(%{public}s), connected(%{public}d)",
        GetEncryptStr(networkId).c_str(), connected);
    IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("remote", networkId.c_str());
    if (audioRendererSinkInstance != nullptr && connected) {
        audioRendererSinkInstance->RegisterAudioSinkCallback(this);
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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
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
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():GetHapBuildApiVersion");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(saManager != nullptr, 0, "failed: saManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = saManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, 0, "failed: remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, 0, "failed: bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetHapBuildApiVersion");
    bundleMgrProxy->GetNameForUid(callerUid, bundleName);
    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);
    reguard.CheckCurrTimeout();
    int32_t hapApiVersion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_REMAINDER;
    AUDIO_INFO_LOG("callerUid %{public}d, version %{public}d", callerUid, hapApiVersion);
    return hapApiVersion;
}

void AudioServer::ResetRecordConfig(AudioProcessConfig &config)
{
    if (config.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        config.isInnerCapturer = true;
        config.innerCapMode = LEGACY_INNER_CAP;
        if (PermissionUtil::VerifyPermission(CAPTURE_PLAYBACK_PERMISSION, IPCSkeleton::GetCallingTokenID())) {
            AUDIO_INFO_LOG("CAPTURE_PLAYBACK permission granted");
            config.innerCapMode = MODERN_INNER_CAP;
        } else if (config.callerUid == MEDIA_SERVICE_UID || config.callerUid == VASSISTANT_UID) {
            config.innerCapMode = MODERN_INNER_CAP;
        } else if (GetHapBuildApiVersion(config.callerUid) >= MODERN_INNER_API_VERSION) { // check build api-version
            config.innerCapMode = LEGACY_MUTE_CAP;
        }
        AUDIO_INFO_LOG("callerUid %{public}d, innerCapMode %{public}d", config.callerUid, config.innerCapMode);
    } else {
        AUDIO_INFO_LOG("CAPTURE_PLAYBACK permission denied");
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
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():GetBundleNameFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, "", "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, "", "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, "", "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetBundleNameFromUid");
    bundleMgrProxy->GetNameForUid(uid, bundleName);
    reguard.CheckCurrTimeout();

    return bundleName;
}

bool AudioServer::IsFastBlocked(int32_t uid)
{
    std::string bundleName = GetBundleNameFromUid(uid);
    std::string result = GetAudioParameter(CHECK_FAST_BLOCK_PREFIX + bundleName);
    return result == "true";
}

void AudioServer::SendRendererCreateErrorInfo(const StreamUsage &sreamUsage,
    const int32_t &errorCode)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_STREAM_CREATE_ERROR_STATS,
        Media::MediaMonitor::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 1);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("STREAM_TYPE", sreamUsage);
    bean->Add("ERROR_CODE", errorCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioServer::CheckParam(const AudioProcessConfig &config)
{
    ContentType contentType = config.rendererInfo.contentType;
    if (contentType < CONTENT_TYPE_UNKNOWN || contentType > CONTENT_TYPE_ULTRASONIC) {
        SendRendererCreateErrorInfo(config.rendererInfo.streamUsage,
            ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("Invalid content type");
        return ERR_INVALID_PARAM;
    }

    StreamUsage streamUsage = config.rendererInfo.streamUsage;
    if (streamUsage < STREAM_USAGE_UNKNOWN || streamUsage > STREAM_USAGE_MAX) {
        SendRendererCreateErrorInfo(config.rendererInfo.streamUsage,
            ERR_INVALID_PARAM);
        AUDIO_ERR_LOG("Invalid stream usage");
        return ERR_INVALID_PARAM;
    }

    if (contentType == CONTENT_TYPE_ULTRASONIC || IsNeedVerifyPermission(streamUsage)) {
        if (!PermissionUtil::VerifySystemPermission()) {
            SendRendererCreateErrorInfo(config.rendererInfo.streamUsage,
                ERR_PERMISSION_DENIED);
            AUDIO_ERR_LOG("CreateAudioRenderer failed! CONTENT_TYPE_ULTRASONIC or STREAM_USAGE_SYSTEM or "\
                "STREAM_USAGE_VOICE_MODEM_COMMUNICATION: No system permission");
            return ERR_PERMISSION_DENIED;
        }
    }
    return SUCCESS;
}

int32_t AudioServer::CheckMaxRendererInstances()
{
    int32_t maxRendererInstances = PolicyHandler::GetInstance().GetMaxRendererInstances();
    if (maxRendererInstances <= 0) {
        maxRendererInstances = DEFAULT_MAX_RENDERER_INSTANCES;
    }

    if (AudioService::GetInstance()->GetCurrentRendererStreamCnt() >= maxRendererInstances) {
        int32_t mostAppUid = INVALID_APP_UID;
        int32_t mostAppNum = INVALID_APP_CREATED_AUDIO_STREAM_NUM;
        AudioService::GetInstance()->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_EXHAUSTED_STATS,
            Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
        bean->Add("CLIENT_UID", mostAppUid);
        bean->Add("TIMES", mostAppNum);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        AUDIO_ERR_LOG("Current audio renderer stream num is greater than the maximum num of configured instances");
        return ERR_EXCEED_MAX_STREAM_CNT;
    }
    return SUCCESS;
}

sptr<IRemoteObject> AudioServer::CreateAudioStream(const AudioProcessConfig &config, int32_t callingUid)
{
    int32_t appUid = config.appInfo.appUid;
    if (callingUid != MEDIA_SERVICE_UID) {
        appUid = callingUid;
    }
    if (IsNormalIpcStream(config) || (isFastControlled_ && IsFastBlocked(config.appInfo.appUid))) {
        AUDIO_INFO_LOG("Create normal ipc stream, isFastControlled: %{public}d", isFastControlled_);
        int32_t ret = 0;
        sptr<IpcStreamInServer> ipcStream = AudioService::GetInstance()->GetIpcStream(config, ret);
        if (ipcStream == nullptr) {
            if (config.audioMode == AUDIO_MODE_PLAYBACK) {
                AudioService::GetInstance()->CleanAppUseNumMap(appUid);
            }
            AUDIO_ERR_LOG("GetIpcStream failed.");
            return nullptr;
        }
        AudioService::GetInstance()->SetIncMaxRendererStreamCnt(config.audioMode);
        sptr<IRemoteObject> remoteObject= ipcStream->AsObject();
        return remoteObject;
    }

#ifdef SUPPORT_LOW_LATENCY
    sptr<IAudioProcess> process = AudioService::GetInstance()->GetAudioProcess(config);
    if (process == nullptr) {
        if (config.audioMode == AUDIO_MODE_PLAYBACK) {
            AudioService::GetInstance()->CleanAppUseNumMap(appUid);
        }
        AUDIO_ERR_LOG("GetAudioProcess failed.");
        return nullptr;
    }
    AudioService::GetInstance()->SetIncMaxRendererStreamCnt(config.audioMode);
    sptr<IRemoteObject> remoteObject= process->AsObject();
    return remoteObject;
#else
    AUDIO_ERR_LOG("GetAudioProcess failed.");
    return nullptr;
#endif
}

sptr<IRemoteObject> AudioServer::CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode)
{
    Trace trace("AudioServer::CreateAudioProcess");
    AudioProcessConfig resetConfig = ResetProcessConfig(config);
    CHECK_AND_RETURN_RET_LOG(CheckConfigFormat(resetConfig), nullptr, "AudioProcessConfig format is wrong, please check"
        ":%{public}s", ProcessConfig::DumpProcessConfig(resetConfig).c_str());
    CHECK_AND_RETURN_RET_LOG(PermissionChecker(resetConfig), nullptr, "Create audio process failed, no permission");

    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    int32_t ret = CheckParam(config);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Check params failed");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (resetConfig.audioMode == AUDIO_MODE_PLAYBACK) {
        errorCode = CheckMaxRendererInstances();
        if (errorCode != SUCCESS) {
            return nullptr;
        }
        if (AudioService::GetInstance()->IsExceedingMaxStreamCntPerUid(callingUid, resetConfig.appInfo.appUid,
            maxRendererStreamCntPerUid_)) {
            errorCode = ERR_EXCEED_MAX_STREAM_CNT_PER_UID;
            AUDIO_ERR_LOG("Current audio renderer stream num exceeds maxRendererStreamCntPerUid");
            return nullptr;
        }
    }

    if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION && callingUid == UID_FOUNDATION_SA
        && config.rendererInfo.isSatellite) {
        bool isSupportSate = OHOS::system::GetBoolParameter(TEL_SATELLITE_SUPPORT, false);
        CHECK_AND_RETURN_RET_LOG(isSupportSate, nullptr, "Do not support satellite");
        IAudioRendererSink* audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
        if (audioRendererSinkInstance) {
            audioRendererSinkInstance->SetAudioParameter(AudioParamKey::NONE, "", SATEMODEM_PARAMETER);
        }
    }
#ifdef FEATURE_APPGALLERY
    PolicyHandler::GetInstance().GetAndSaveClientType(resetConfig.appInfo.appUid,
        GetBundleNameFromUid(resetConfig.appInfo.appUid));
#endif

    return CreateAudioStream(resetConfig, callingUid);
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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
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
    Trace traceCb("callbackToIntelligentVoice");
    int64_t stamp = ClockTime::GetCurNano();
    callback->OnCapturerState(isActive);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("isActive:%{public}d num:%{public}d cb cost[%{public}" PRId64 "]", isActive, num, stamp);
}

int32_t AudioServer::SetParameterCallback(const sptr<IRemoteObject>& object)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
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

int32_t AudioServer::CheckInnerRecorderPermission(const AudioProcessConfig &config)
{
    SourceType sourceType = config.capturerInfo.sourceType;
    if (sourceType != SOURCE_TYPE_REMOTE_CAST && sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        return PERMISSION_UNKNOWN;
    }
#ifdef HAS_FEATURE_INNERCAPTURER
    Security::AccessToken::AccessTokenID tokenId = config.appInfo.appTokenId;
    if (sourceType == SOURCE_TYPE_REMOTE_CAST) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission, PERMISSION_DENIED,
            "Create source remote cast failed: no system permission.");

        bool hasCastAudioOutputPermission = VerifyClientPermission(CAST_AUDIO_OUTPUT_PERMISSION, tokenId);
        CHECK_AND_RETURN_RET_LOG(hasCastAudioOutputPermission, PERMISSION_DENIED, "No cast audio output permission");
        return PERMISSION_GRANTED;
    }

    if (sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE && config.innerCapMode == MODERN_INNER_CAP) {
        AUDIO_INFO_LOG("modern inner-cap source, no need to check.");
        return PERMISSION_GRANTED;
    }
    return PERMISSION_UNKNOWN;
#else
    return PERMISSION_DENIED;
#endif
}

bool AudioServer::CheckRecorderPermission(const AudioProcessConfig &config)
{
    Security::AccessToken::AccessTokenID tokenId = config.appInfo.appTokenId;
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

    int32_t permission = CheckInnerRecorderPermission(config);
    AUDIO_INFO_LOG("CheckInnerRecorderPermission return %{public}d", permission);
    if (permission == PERMISSION_GRANTED) {
        return true;
    } else if (permission == PERMISSION_DENIED) {
        return false;
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

    CHECK_AND_RETURN_RET(HandleCheckRecorderBackgroundCapture(config), false,
        "VerifyBackgroundCapture failed for callerUid:%{public}d", config.callerUid);
    return true;
}

bool AudioServer::HandleCheckRecorderBackgroundCapture(const AudioProcessConfig &config)
{
    SwitchStreamInfo info = {
        config.originalSessionId,
        config.callerUid,
        config.appInfo.appUid,
        config.appInfo.appPid,
        config.appInfo.appTokenId,
        CAPTURER_PREPARED,
    };
    if (PermissionUtil::NeedVerifyBackgroundCapture(config.callerUid, config.capturerInfo.sourceType) &&
        !PermissionUtil::VerifyBackgroundCapture(info.appTokenId, config.appInfo.appFullTokenId)) {
        if (!SwitchStreamUtil::IsSwitchStreamSwtching(info, SWITCH_STATE_CREATED)) {
            AUDIO_INFO_LOG("Recreating stream for callerUid:%{public}d need not VerifyBackgroundCapture",
                config.callerUid);
            return true;
        }
        SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_CREATED);
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

void AudioServer::AudioServerDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("Policy server died: restart pulse audio");
    _Exit(0);
}

void AudioServer::RegisterPolicyServerDeathRecipient()
{
    AUDIO_INFO_LOG("Register policy server death recipient");
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(pid, uid);
    if (deathRecipient_ != nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_LOG(samgr != nullptr, "Failed to obtain system ability manager");
        sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::AUDIO_POLICY_SERVICE_ID);
        CHECK_AND_RETURN_LOG(object != nullptr, "Policy service unavailable");
        deathRecipient_->SetNotifyCb([this] (pid_t pid, pid_t uid) { this->AudioServerDied(pid, uid); });
        bool result = object->AddDeathRecipient(deathRecipient_);
        if (!result) {
            AUDIO_ERR_LOG("Failed to add deathRecipient");
        }
    }
}

bool AudioServer::CreatePlaybackCapturerManager()
{
#ifdef HAS_FEATURE_INNERCAPTURER
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return false;
    }
    std::vector<int32_t> usage;
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetSupportStreamUsage(usage);
    return true;
#else
    return false;
#endif
}

int32_t AudioServer::SetSupportStreamUsage(std::vector<int32_t> usage)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("SetSupportStreamUsage with usage num:%{public}zu", usage.size());

    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetSupportStreamUsage(usage);
    return SUCCESS;
#else
    return ERROR;
#endif
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
#ifdef SUPPORT_LOW_LATENCY
    IAudioCapturerSource *fastAudioCapturerSourceInstance = FastAudioCapturerSource::GetInstance();
    IAudioCapturerSource *voipFastAudioCapturerSourceInstance = FastAudioCapturerSource::GetVoipInstance();
#endif
    IAudioCapturerSource *bluetoothAudioCapturerSourceInstance = BluetoothCapturerSource::GetInstance();

    for (auto audioCapturerSourceInstance : {
        primaryAudioCapturerSourceInstance,
        usbAudioCapturerSinkInstance,
#ifdef SUPPORT_LOW_LATENCY
        fastAudioCapturerSourceInstance,
        voipFastAudioCapturerSourceInstance,
#endif
        bluetoothAudioCapturerSourceInstance
    }) {
        if (audioCapturerSourceInstance != nullptr) {
            audioCapturerSourceInstance->RegisterAudioCapturerSourceCallback(make_unique<CapturerStateOb>(
                [this] (bool isActive, int32_t num) {
                    this->OnCapturerState(isActive, num);
                }));
        }
    }
}

void AudioServer::RegisterAudioRendererSinkCallback()
{
    // Only watch primary and fast sink for now, watch other sinks later.
    IAudioRendererSink *primarySink = IAudioRendererSink::GetInstance("primary", "");
    IAudioRendererSink *usbSink = IAudioRendererSink::GetInstance("usb", "");
    IAudioRendererSink *directSink = IAudioRendererSink::GetInstance("direct", "");
    IAudioRendererSink *dpSink = IAudioRendererSink::GetInstance("dp", "");
    IAudioRendererSink *voipSink = IAudioRendererSink::GetInstance("voip", "");
    IAudioRendererSink *offloadSink = IAudioRendererSink::GetInstance("offload", "");
    IAudioRendererSink *mchSink = IAudioRendererSink::GetInstance("multichannel", "");
    IAudioRendererSink *a2dpSink = IAudioRendererSink::GetInstance("a2dp", "");
    IAudioRendererSink *a2dpFastSink = IAudioRendererSink::GetInstance("a2dp_fast", "");
#ifdef SUPPORT_LOW_LATENCY
    IAudioRendererSink *fastSink = FastAudioRendererSink::GetInstance();
    IAudioRendererSink *fastVoipSink = FastAudioRendererSink::GetVoipInstance();
#endif

    for (auto sinkInstance : {
        primarySink,
        usbSink,
        directSink,
        dpSink,
        voipSink,
        offloadSink,
        mchSink,
        a2dpSink,
        a2dpFastSink,
#ifdef SUPPORT_LOW_LATENCY
        fastSink,
        fastVoipSink
#endif
    }) {
        if (sinkInstance) {
            sinkInstance->RegisterAudioSinkCallback(this);
        }
    }
}

int32_t AudioServer::SetCaptureSilentState(bool state)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }

    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetCaptureSilentState(state);
    return SUCCESS;
#else
    return ERROR;
#endif
}

int32_t AudioServer::NotifyStreamVolumeChanged(AudioStreamType streamType, float volume)
{
    AUDIO_INFO_LOG("Enter the notifyStreamVolumeChanged interface");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("NotifyStreamVolumeChanged refused for %{public}d", callingUid);
        return ERR_NOT_SUPPORTED;
    }

    SetSystemVolumeToEffect(streamType, volume);

    int32_t ret = AudioService::GetInstance()->NotifyStreamVolumeChanged(streamType, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("NotifyStreamVolumeChanged failed");
    }
    ret = SetVolumeInfoForEnhanceChain(streamType);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetVolumeInfoForEnhanceChain failed");
    }
    return SUCCESS;
}

int32_t AudioServer::ResetRouteForDisconnect(DeviceType type)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("primary", "");
    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("audioRendererSinkInstance is null!");
        return ERROR;
    }
    audioRendererSinkInstance->ResetOutputRouteForDisconnect(type);

    // todo reset capturer

    return SUCCESS;
}

float AudioServer::GetMaxAmplitude(bool isOutputDevice, int32_t deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), 0, "GetMaxAmplitude refused for %{public}d", callingUid);

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
#ifdef SUPPORT_LOW_LATENCY
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->ResetAudioEndpoint();
#endif
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

int32_t AudioServer::UpdateDualToneState(bool enable, int32_t sessionId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

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
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "refused for %{public}d",
        callingUid);

    if (durationUs <= 0) {
        return SUCCESS;
    }

    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance(devceClass.c_str(), "");
    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "has no valid sink");
    return audioRendererSinkInstance->SetSinkMuteForSwitchDevice(mute);
}

void AudioServer::UpdateSessionConnectionState(const int32_t &sessionId, const int32_t &state)
{
    AUDIO_INFO_LOG("Server get sessionID: %{public}d, state: %{public}d", sessionId, state);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(),
        "Update session connection state refused for %{public}d", callingUid);
    std::shared_ptr<RendererInServer> renderer =
        AudioService::GetInstance()->GetRendererBySessionID(static_cast<uint32_t>(sessionId));

    if (renderer == nullptr) {
        AUDIO_ERR_LOG("No render in server has sessionID");
        return;
    }
    renderer->OnDataLinkConnectionUpdate(static_cast<IOperation>(state));
}

void AudioServer::SetNonInterruptMute(const uint32_t sessionId, const bool muteFlag)
{
    AUDIO_INFO_LOG("sessionId_: %{public}u, muteFlag: %{public}d", sessionId, muteFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
}

void AudioServer::RestoreSession(const int32_t &sessionID, bool isOutput)
{
    AUDIO_INFO_LOG("restore output: %{public}d, sessionID: %{public}d", isOutput, sessionID);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(),
        "Update session connection state refused for %{public}d", callingUid);
    if (isOutput) {
        std::shared_ptr<RendererInServer> renderer =
            AudioService::GetInstance()->GetRendererBySessionID(static_cast<uint32_t>(sessionID));
        if (renderer == nullptr) {
            AUDIO_ERR_LOG("No render in server has sessionID");
            return;
        }
        renderer->RestoreSession();
    } else {
        std::shared_ptr<CapturerInServer> capturer =
            AudioService::GetInstance()->GetCapturerBySessionID(static_cast<uint32_t>(sessionID));
        if (capturer == nullptr) {
            AUDIO_ERR_LOG("No capturer in server has sessionID");
            return;
        }
        capturer->RestoreSession();
    }
}

int32_t AudioServer::SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d",
        callingUid);
    return AudioService::GetInstance()->SetOffloadMode(sessionId, state, isAppBack);
}

int32_t AudioServer::UnsetOffloadMode(uint32_t sessionId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d",
        callingUid);
    return AudioService::GetInstance()->UnsetOffloadMode(sessionId);
}

void AudioServer::OnAudioSinkStateChange(uint32_t sinkId, bool started)
{
    AudioService::GetInstance()->UpdateAudioSinkState(sinkId, started);
    return;
}

void AudioServer::CheckHibernateState(bool hibernate)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifyIsAudio(), "refused for %{public}d", callingUid);
    return AudioService::GetInstance()->CheckHibernateState(hibernate);
}

sptr<IRemoteObject> AudioServer::CreateIpcOfflineStream(int32_t &errorCode)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), nullptr, "refused for %{public}d", callingUid);
    sptr<OfflineStreamInServer> stream = OfflineStreamInServer::GetOfflineStream(errorCode);
    CHECK_AND_RETURN_RET_LOG(stream, nullptr, "Create IpcOfflineStream failed.");
    sptr<IRemoteObject> remoteObject = stream->AsObject();
    return remoteObject;
}

int32_t AudioServer::GetOfflineAudioEffectChains(std::vector<std::string> &effectChains)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
#ifdef FEATURE_OFFLINE_EFFECT
    return OfflineStreamInServer::GetOfflineAudioEffectChains(effectChains);
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t AudioServer::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    Trace trace("AudioServer::GetStandbyStatus:" + std::to_string(sessionId));

    // only for native sa calling
    auto type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(IPCSkeleton::GetCallingTokenID());
    bool isAllowed = type == Security::AccessToken::TOKEN_NATIVE;
#ifdef AUDIO_BUILD_VARIANT_ROOT
    isAllowed = isAllowed || type == Security::AccessToken::TOKEN_SHELL; // for DT
#endif
    CHECK_AND_RETURN_RET_LOG(isAllowed, ERR_INVALID_OPERATION, "not allowed");

    return AudioService::GetInstance()->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
}

int32_t AudioServer::GenerateSessionId(uint32_t &sessionId)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(uid == MCU_UID, ERROR, "uid is %{public}d, not mcu uid", uid);
    sessionId = PolicyHandler::GetInstance().GenerateSessionId(uid);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
