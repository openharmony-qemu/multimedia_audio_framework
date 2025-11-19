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
#include <format>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hisysevent.h"
#include "parameters.h"

#include "core_service_handler.h"
#include "icore_service_provider_ipc.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "util/id_handler.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_asr.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#ifdef HAS_FEATURE_INNERCAPTURER
#include "playback_capturer_manager.h"
#endif
#include "config/audio_param_parser.h"
#include "media_monitor_manager.h"
#include "offline_stream_in_server.h"
#include "audio_dump_pcm.h"
#include "audio_info.h"
#include "i_hpae_manager.h"
#include "audio_server_hpae_dump.h"
#include "audio_resource_service.h"
#include "audio_manager_listener.h"
#include "app_bundle_manager.h"
#include "audio_injector_service.h"
#ifdef SUPPORT_OLD_ENGINE
#define PA
#ifdef PA
extern "C" {
    extern int ohos_pa_main(int argc, char *argv[]);
}
#endif
#endif // SUPPORT_OLD_ENGINE
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t UID_MSDP_SA = 6699;
constexpr int32_t INTELL_VOICE_SERVICR_UID = 1042;
uint32_t AudioServer::paDaemonTid_;
std::map<std::string, std::string> AudioServer::audioParameters;
std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> AudioServer::audioParameterKeys;
const string DEFAULT_COOKIE_PATH = "/data/data/.pulse_dir/state/cookie";
const std::string CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
constexpr const char *TEL_SATELLITE_SUPPORT = "const.telephony.satellite.supported";
const std::string SATEMODEM_PARAMETER = "usedmodem=satemodem";
const std::string PCM_DUMP_KEY = "PCM_DUMP";
const std::string EFFECT_LIVE_KEY = "hpae_effect";
constexpr int32_t UID_FOUNDATION_SA = 5523;
const unsigned int TIME_OUT_SECONDS = 10;
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
constexpr int32_t MAX_RENDERER_STREAM_CNT_PER_UID = 128;
const int32_t DEFAULT_MAX_RENDERER_INSTANCES = 128;
const int32_t DEFAULT_MAX_LOOPBACK_INSTANCES = 1;
const int32_t MCU_UID = 7500;
const int32_t TV_SERVICE_UID = 7501;
const int32_t AAM_CONN_SVC_UID = 7878;
constexpr int32_t CHECK_ALL_RENDER_UID = -1;
constexpr int64_t RENDER_DETECTION_CYCLE_NS = 10000000000;
constexpr int32_t RENDER_BAD_FRAMES_RATIO = 100;
static const std::set<int32_t> RECORD_CHECK_FORWARD_LIST = {
    VM_MANAGER_UID,
    UID_CAMERA
};
static const std::set<int32_t> GENERATE_SESSIONID_UID_SET = {
    MCU_UID,
    TV_SERVICE_UID,
    AAM_CONN_SVC_UID
};
const int32_t RSS_THRESHOLD = 2;
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
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_LIVE
};

static constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;
static constexpr unsigned int WAIT_AUDIO_POLICY_READY_TIMEOUT_SECONDS = 5;
static constexpr int32_t MAX_WAIT_IN_SERVER_COUNT = 5;
static constexpr int32_t RESTORE_SESSION_TRY_COUNT = 10;
static constexpr uint32_t  RESTORE_SESSION_RETRY_WAIT_TIME_IN_MS = 50000;
static constexpr unsigned int CREATE_TIMEOUT_IN_SECOND = 9;

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
    SOURCE_TYPE_LIVE,
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
    SOURCE_TYPE_LIVE,
};

static bool IsVoiceModemCommunication(StreamUsage streamUsage, int32_t callingUid)
{
    return streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION && callingUid == UID_FOUNDATION_SA;
}

static inline std::shared_ptr<IAudioRenderSink> GetSinkByProp(HdiIdType type, const std::string &info =
    HDI_ID_INFO_DEFAULT, bool tryCreate = false, bool tryCreateId = true)
{
    uint32_t id = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, type, info, false, tryCreateId);
    return HdiAdapterManager::GetInstance().GetRenderSink(id, tryCreate);
}

static inline std::shared_ptr<IAudioCaptureSource> GetSourceByProp(HdiIdType type, const std::string &info =
    HDI_ID_INFO_DEFAULT, bool tryCreate = false)
{
    uint32_t id = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, type, info);
    return HdiAdapterManager::GetInstance().GetCaptureSource(id, tryCreate);
}

static void UpdateArmInstance(std::shared_ptr<IAudioRenderSink> &sink,
    std::shared_ptr<IAudioCaptureSource> &source)
{
    sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    source = GetSourceByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    std::shared_ptr<IAudioRenderSink> primarySink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    CHECK_AND_RETURN_LOG(primarySink, "primarySink is nullptr!");
    primarySink->ResetActiveDeviceForDisconnect(DEVICE_TYPE_NONE);
}

static void SetAudioSceneForAllSource(AudioScene audioScene)
{
    std::shared_ptr<IAudioCaptureSource> usbSource = GetSourceByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
    if (usbSource != nullptr && usbSource->IsInited()) {
        usbSource->SetAudioScene(audioScene);
    }
    std::shared_ptr<IAudioCaptureSource> accSource = GetSourceByProp(HDI_ID_TYPE_ACCESSORY, HDI_ID_INFO_ACCESSORY);
    if (accSource != nullptr && accSource->IsInited()) {
        accSource->SetAudioScene(audioScene);
    }
    std::shared_ptr<IAudioCaptureSource> primarySource = GetSourceByProp(HDI_ID_TYPE_PRIMARY);
    if (primarySource != nullptr && primarySource->IsInited()) {
        primarySource->SetAudioScene(audioScene);
    }
    std::shared_ptr<IAudioCaptureSource> aiSource = GetSourceByProp(HDI_ID_TYPE_AI);
    if (aiSource != nullptr && aiSource->IsInited()) {
        aiSource->SetAudioScene(audioScene);
    }
#ifdef SUPPORT_LOW_LATENCY
    std::shared_ptr<IAudioCaptureSource> fastSource = GetSourceByProp(HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT, true);
    if (fastSource != nullptr && fastSource->IsInited()) {
        fastSource->SetAudioScene(audioScene);
    }
    std::shared_ptr<IAudioCaptureSource> fastVoipSource = GetSourceByProp(HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP, true);
    if (fastVoipSource != nullptr && fastVoipSource->IsInited()) {
        fastVoipSource->SetAudioScene(audioScene);
    }
#endif
    std::shared_ptr<IAudioCaptureSource> a2dpInSource = GetSourceByProp(HDI_ID_TYPE_BLUETOOTH);
    if (a2dpInSource != nullptr && a2dpInSource->IsInited()) {
        a2dpInSource->SetAudioScene(audioScene);
    }
}

static void SetAudioSceneForAllSink(AudioScene audioScene, bool scoExcludeFlag)
{
    std::shared_ptr<IAudioRenderSink> usbSink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
    if (usbSink != nullptr && usbSink->IsInited()) {
        usbSink->SetAudioScene(audioScene, scoExcludeFlag);
    }
    std::shared_ptr<IAudioRenderSink> primarySink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    if (primarySink != nullptr && primarySink->IsInited()) {
        primarySink->SetAudioScene(audioScene, scoExcludeFlag);
    }
}

static void UpdateDeviceForAllSource(std::shared_ptr<IAudioCaptureSource> &source, DeviceType type)
{
    if (source == nullptr || !source->IsInited()) {
        AUDIO_WARNING_LOG("Capturer is not initialized.");
    } else {
        source->UpdateActiveDevice(type);
    }
#ifdef SUPPORT_LOW_LATENCY
    std::shared_ptr<IAudioCaptureSource> fastSource = GetSourceByProp(HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT, true);
    if (fastSource != nullptr && fastSource->IsInited()) {
        fastSource->UpdateActiveDevice(type);
    }
    std::shared_ptr<IAudioCaptureSource> fastVoipSource = GetSourceByProp(HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP, true);
    if (fastVoipSource != nullptr && fastVoipSource->IsInited()) {
        fastVoipSource->UpdateActiveDevice(type);
    }
#endif
    std::shared_ptr<IAudioCaptureSource> aiSource = GetSourceByProp(HDI_ID_TYPE_AI, HDI_ID_INFO_DEFAULT);
    if (aiSource != nullptr && aiSource->IsInited()) {
        aiSource->UpdateActiveDevice(type);
    }
}

// std::vector<StringPair> -> std::vector<std::pair<std::string, std::string>>
static std::vector<std::pair<std::string, std::string>> ConvertStringPair(const std::vector<StringPair> &stringPair)
{
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto &it : stringPair) {
        result.emplace_back(it.firstParam, it.secondParam);
    }
    return result;
}

// std::vector<std::pair<std::string, std::string>> -> std::vector<StringPair>
static std::vector<StringPair> ConvertStringPair(const std::vector<std::pair<std::string, std::string>> &result)
{
    std::vector<StringPair> stringPair;
    for (auto it = result.begin(); it != result.end(); it++) {
        stringPair.push_back({it->first, it->second});
    }
    return stringPair;
}

void ProxyDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    CHECK_AND_RETURN_LOG(audioServer_ != nullptr, "audioServer is nullptr!");
    audioServer_->RemoveRendererDataTransferCallback(pid_);
    AudioStreamMonitor::GetInstance().OnCallbackAppDied(pid_);
}

PipeInfoGuard::PipeInfoGuard(uint32_t sessionId)
{
    sessionId_ = sessionId;
}

PipeInfoGuard::~PipeInfoGuard()
{
    if (releaseFlag_) {
        CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_RELEASE,
            SESSION_OP_MSG_REMOVE_PIPE);
    }
}

void PipeInfoGuard::SetReleaseFlag(bool flag)
{
    releaseFlag_ = flag;
}

class CapturerStateOb final : public IAudioSourceCallback {
public:
    explicit CapturerStateOb(uint32_t captureId, std::function<void(bool, size_t, size_t)> callback)
        : captureId_(captureId), callback_(callback)
    {
    }

    ~CapturerStateOb() override final
    {
    }

// LCOV_EXCL_START
    void OnCaptureState(bool isActive) override final
    {
        std::lock_guard<std::mutex> lock(captureIdMtx_);
        auto preNum = captureIds_.size();
        if (isActive) {
            captureIds_.insert(captureId_);
        } else {
            captureIds_.erase(captureId_);
        }
        auto curNum = captureIds_.size();
        AUDIO_INFO_LOG("captureId: %{public}u, preNum: %{public}zu, curNum: %{public}zu, isActive: %{public}d",
            captureId_, preNum, curNum, isActive);
        callback_(isActive, preNum, curNum);
    }
// LCOV_EXCL_STOP

private:
    static inline std::unordered_set<uint32_t> captureIds_;
    static inline std::mutex captureIdMtx_;

    uint32_t captureId_;
    // callback to audioserver
    std::function<void(bool, size_t, size_t)> callback_;
};

REGISTER_SYSTEM_ABILITY_BY_ID(AudioServer, AUDIO_DISTRIBUTED_SERVICE_ID, true)

#ifdef SUPPORT_OLD_ENGINE
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
#endif // SUPPORT_OLD_ENGINE

AudioServer::AudioServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
    audioEffectServer_(std::make_unique<AudioEffectServer>())
{
    AudioStreamMonitor::GetInstance().SetAudioServerPtr(this);
}

void AudioServer::OnDump() {}

int32_t AudioServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    AUDIO_INFO_LOG("Dump Process Invoked");
    if (args.size() == FAST_DUMPINFO_LEN && args[0] == u"-fb") {
        std::string bundleName = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(args[1]);
        std::string result;
        GetAudioParameter(CHECK_FAST_BLOCK_PREFIX + bundleName, result);
        std::string dumpString = "check fast list :bundle name is" + bundleName + " result is " + result + "\n";
        return write(fd, dumpString.c_str(), dumpString.size());
    }

    if (args.size() == 1 && args[0] == u"-dfl") {
        std::string dumpString;
        AudioService::GetInstance()->DumpForegroundList(dumpString);
        return write(fd, dumpString.c_str(), dumpString.size());
    }

    std::queue<std::u16string> argQue;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argQue.push(args[index]);
    }
    std::lock_guard<std::mutex> lock(hpaeDumpMutex_);
    std::string dumpString;
    int32_t res = 0;
#ifdef SUPPORT_OLD_ENGINE
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        if (hpaeDumpObj_ == nullptr) {
            hpaeDumpObj_ = std::make_shared<AudioServerHpaeDump>();
        }
        res = hpaeDumpObj_->Initialize();
        CHECK_AND_RETURN_RET_LOG(res == AUDIO_DUMP_SUCCESS, AUDIO_DUMP_INIT_ERR,
            "Audio Service Hpae Dump Not Initialed");
        hpaeDumpObj_->AudioDataDump(dumpString, argQue);
    } else {
        AudioServerDump dumpObj;
        res = dumpObj.Initialize();
        CHECK_AND_RETURN_RET_LOG(res == AUDIO_DUMP_SUCCESS, AUDIO_DUMP_INIT_ERR,
            "Audio Service Dump Not initialised\n");
        dumpObj.AudioDataDump(dumpString, argQue);
    }
#else
    if (hpaeDumpObj_ == nullptr) {
        hpaeDumpObj_ = std::make_shared<AudioServerHpaeDump>();
    }
    res = hpaeDumpObj_->Initialize();
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERROR,
        "Audio Service Hpae Dump Not Initialed");
    hpaeDumpObj_->AudioDataDump(dumpString, argQue);
#endif // SUPPORT_OLD_ENGINE
    return write(fd, dumpString.c_str(), dumpString.size());
}

void AudioServer::RemoveRendererDataTransferCallback(const int32_t &pid)
{
    std::lock_guard<std::mutex> lock(audioDataTransferMutex_);
    if (audioDataTransferCbMap_.count(pid) > 0) {
        audioDataTransferCbMap_.erase(pid);
    }
}

int32_t AudioServer::RegisterDataTransferCallback(const sptr<IRemoteObject> &object)
{
    bool result = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(result, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "AudioServer:set listener object is nullptr");

    std::lock_guard<std::mutex> lock(audioDataTransferMutex_);

    sptr<IStandardAudioServerManagerListener> listener = iface_cast<IStandardAudioServerManagerListener>(object);

    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "AudioServer: listener obj cast failed");

    std::shared_ptr<DataTransferStateChangeCallbackInner> callback =
    std::make_shared<AudioManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "AudioPolicyServer: failed to  create cb obj");

    int32_t pid = IPCSkeleton::GetCallingPid();
    sptr<ProxyDeathRecipient> recipient = new ProxyDeathRecipient(pid, this);
    object->AddDeathRecipient(recipient);
    audioDataTransferCbMap_[pid] = callback;
    AUDIO_INFO_LOG("Pid: %{public}d registerDataTransferCallback done", pid);
    return SUCCESS;
}

int32_t AudioServer::RegisterDataTransferMonitorParam(int32_t callbackId,
    const DataTransferMonitorParam &param)
{
    bool result = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(result, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(
        param, pid, callbackId);
    AUDIO_INFO_LOG("Register end, pid = %{public}d, callbackId = %{public}d",
        pid, callbackId);
    return SUCCESS;
}

int32_t AudioServer::UnregisterDataTransferMonitorParam(int32_t callbackId)
{
    bool result = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(result, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(
        pid, callbackId);
    AUDIO_INFO_LOG("Unregister end, pid = %{public}d, callbackId = %{public}d",
        pid, callbackId);
    return SUCCESS;
}

void AudioServer::OnDataTransferStateChange(const int32_t &pid, const int32_t &callbackId,
    const AudioRendererDataTransferStateChangeInfo &info)
{
    std::shared_ptr<DataTransferStateChangeCallbackInner> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(audioDataTransferMutex_);
        if (audioDataTransferCbMap_.count(pid) > 0) {
            callback = audioDataTransferCbMap_[pid];
        } else {
            AUDIO_ERR_LOG("callback is null");
            return;
        }
    }
    callback->OnDataTransferStateChange(callbackId, info);
}

void AudioServer::OnMuteStateChange(const int32_t &pid, const int32_t &callbackId,
    const int32_t &uid, const uint32_t &sessionId, const bool &isMuted)
{
    std::shared_ptr<DataTransferStateChangeCallbackInner> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(audioDataTransferMutex_);
        CHECK_AND_RETURN_LOG(audioDataTransferCbMap_.find(pid) != audioDataTransferCbMap_.end(),
            "pid:%{public}d no callback in CbMap", pid);
        callback = audioDataTransferCbMap_[pid];
    }
    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is null");
    callback->OnMuteStateChange(callbackId, uid, sessionId, isMuted);
}

void AudioServer::RegisterDataTransferStateChangeCallback()
{
    DataTransferMonitorParam param;
    param.clientUID = CHECK_ALL_RENDER_UID;
    param.badDataTransferTypeBitMap = (1 << NO_DATA_TRANS);
    param.timeInterval = RENDER_DETECTION_CYCLE_NS;
    param.badFramesRatio = RENDER_BAD_FRAMES_RATIO;

    std::lock_guard<std::mutex> lock(audioDataTransferMutex_);

    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    CHECK_AND_RETURN_LOG(callback != nullptr, "AudioPolicyServer: failed to  create cb obj");

    int32_t pid = IPCSkeleton::GetCallingPid();
    callback->SetDataTransferMonitorParam(param);
    audioDataTransferCbMap_[pid] = callback;
    int32_t ret = AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(
        param, pid, -1);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "register fail");
    AUDIO_INFO_LOG("pid: %{public}d RegisterDataTransferStateChangeCallback done", pid);
}

void DataTransferStateChangeCallbackInnerImpl::SetDataTransferMonitorParam(
    const DataTransferMonitorParam &param)
{
    param_.clientUID = param.clientUID;
    param_.badDataTransferTypeBitMap = param.badDataTransferTypeBitMap;
    param_.timeInterval = param.timeInterval;
    param_.badFramesRatio = param.badFramesRatio;
}

// LCOV_EXCL_START
void DataTransferStateChangeCallbackInnerImpl::OnDataTransferStateChange(
    const int32_t &callbackId, const AudioRendererDataTransferStateChangeInfo &info)
{
    if (info.stateChangeType == DATA_TRANS_STOP) {
        ReportEvent(info);
        std::string bundleName = AppBundleManager::GetBundleNameFromUid(info.clientUID);
        CHECK_AND_RETURN_LOG(AudioService::GetInstance()->InRenderWhitelist(bundleName),
            "%{public}s not in whitelist", bundleName.c_str());
        if (((info.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION) ||
            (info.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) ||
            (info.streamUsage == STREAM_USAGE_NOTIFICATION_RINGTONE) ||
            (info.streamUsage == STREAM_USAGE_RINGTONE) ||
            (info.streamUsage == STREAM_USAGE_NOTIFICATION)) && info.isBackground) {
            int32_t ret = PolicyHandler::GetInstance().ClearAudioFocusBySessionID(info.sessionId);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "focus clear fail");
        }
    }
}
// LCOV_EXCL_STOP

void DataTransferStateChangeCallbackInnerImpl::ReportEvent(
    const AudioRendererDataTransferStateChangeInfo &info)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::STREAM_OCCUPANCY,
        Media::MediaMonitor::EventType::DURATION_AGGREGATION_EVENT);
    CHECK_AND_RETURN_LOG(bean != nullptr, "bean is nullptr");

    bean->Add("IS_PLAYBACK", 1);
    bean->Add("SESSIONID", static_cast<int32_t>(info.sessionId));
    bean->Add("UID", info.clientUID);
    bean->Add("STREAM_OR_SOURCE_TYPE", info.streamUsage);
    bean->Add("START_TIME", static_cast<uint64_t>(0));
    bean->Add("UPLOAD_TIME", static_cast<uint64_t>(0));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

// LCOV_EXCL_START
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
    DlopenUtils::Init();
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
#ifdef SUPPORT_OLD_ENGINE
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        HPAE::IHpaeManager::GetHpaeManager().Init();
        AUDIO_INFO_LOG("IHpaeManager Init\n");
    } else {
#ifdef PA
        int32_t ret = pthread_create(&m_paDaemonThread, nullptr, AudioServer::paDaemonThread, nullptr);
        pthread_setname_np(m_paDaemonThread, "OS_PaDaemon");
        if (ret != 0) {
            AUDIO_ERR_LOG("pthread_create failed %d", ret);
            WriteServiceStartupError();
        }
        AUDIO_DEBUG_LOG("Created paDaemonThread\n");
#endif // PA
    }
#else
    HPAE::IHpaeManager::GetHpaeManager().Init();
    AUDIO_INFO_LOG("IHpaeManager Init\n");
#endif // SUPPORT_OLD_ENGINE
    RegisterAudioCapturerSourceCallback();
    RegisterAudioRendererSinkCallback();
    ParseAudioParameter();
    NotifyProcessStatus();
    DlopenUtils::DeInit();
    RegisterDataTransferStateChangeCallback();
}
// LCOV_EXCL_STOP

void AudioServer::ParseAudioParameter()
{
    std::unique_ptr<AudioParamParser> audioParamParser = make_unique<AudioParamParser>();
    if (audioParamParser == nullptr) {
        WriteServiceStartupError();
    }
    CHECK_AND_RETURN_LOG(audioParamParser != nullptr, "Failed to create audio extra parameters parser");
    if (audioParamParser->LoadConfiguration(audioParameterKeys)) {
        AUDIO_INFO_LOG("Audio extra parameters load configuration successfully.");
    }
    isAudioParameterParsed_.store(true);

    {
        std::unique_lock<std::mutex> lock(audioParameterCacheMutex_);
        for (const auto &pair : audioExtraParameterCacheVector_) {
            std::vector<StringPair> params = ConvertStringPair(pair.second);
            SetExtraParameters(pair.first, params);
        }
        audioExtraParameterCacheVector_.clear();
    }
    AUDIO_INFO_LOG("Audio extra parameters replay cached successfully.");
    PermissionUtil::UpdateBGSet();
}

void AudioServer::WriteServiceStartupError()
{
    Trace trace("SYSEVENT FAULT EVENT AUDIO_SERVICE_STARTUP_ERROR, SERVICE_ID: "
            + std::to_string(Media::MediaMonitor::AUDIO_SERVER_ID) + ", ERROR_CODE: "
            + std::to_string(Media::MediaMonitor::AUDIO_SERVER));
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

bool AudioServer::SetPcmDumpParameter(const std::vector<std::pair<std::string, std::string>> &params)
{
    bool ret = VerifyClientPermission(DUMP_AUDIO_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, false, "set audiodump parameters failed: no permission.");
    CHECK_AND_RETURN_RET_LOG(params.size() > 0, false, "params is empty!");
    return AudioCacheMgr::GetInstance().SetDumpParameter(params);
}

// LCOV_EXCL_START
bool AudioServer::SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params)
{
    CHECK_AND_RETURN_RET_LOG(params.size() > 0, false, "params is empty!");
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetEffectLiveParameter(params);
    }
    AUDIO_INFO_LOG("SetEffectLiveParameter not support");
    return false;
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetExtraParameters(const std::string &key,
    const std::vector<StringPair> &kvpairs)
{
    Trace trace("AudioServer::SetExtraParameters" + key);
    CHECK_AND_RETURN_RET_LOG(kvpairs.size() >= 0 && kvpairs.size() <= AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Set extra audio parameters failed");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "set extra parameters failed: not system app.");
    ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "set extra parameters failed: no permission.");
    std::vector<std::pair<std::string, std::string>> newPair = ConvertStringPair(kvpairs);
    if (key == EFFECT_LIVE_KEY) {
        ret = SetEffectLiveParameter(newPair);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "set effect live parameters failed.");
        return SUCCESS;
    }

    if (key == PCM_DUMP_KEY) {
        ret = SetPcmDumpParameter(newPair);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "set audiodump parameters failed");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(!CacheExtraParameters(key, newPair), ERROR, "cached");

    if (audioParameterKeys.empty()) {
        AUDIO_ERR_LOG("audio extra parameters mainKey and subKey is empty");
        return ERROR;
    }

    auto mainKeyIt = audioParameterKeys.find(key);
    if (mainKeyIt == audioParameterKeys.end()) {
        return ERR_INVALID_PARAM;
    }

    std::string value;
    bool isParamValid = ProcessKeyValuePairs(key, newPair, mainKeyIt->second, value);
    CHECK_AND_RETURN_RET_LOG(isParamValid, ERR_INVALID_PARAM, "invalid subkey or value");

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", value);
    return SUCCESS;
}

bool AudioServer::ProcessKeyValuePairs(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs,
    const std::unordered_map<std::string, std::set<std::string>> &subKeyMap, std::string &value)
{
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
            return false;
        }
    }
    return true;
}

// LCOV_EXCL_START
bool AudioServer::CacheExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    if (!isAudioParameterParsed_.load()) {
        std::unique_lock<std::mutex> lock(audioParameterCacheMutex_);
        if (!isAudioParameterParsed_.load()) {
            AUDIO_INFO_LOG("Audio extra parameters will be cached");
            std::pair<std::string,
                std::vector<std::pair<std::string, std::string>>> cache(key, kvpairs);
            audioExtraParameterCacheVector_.push_back(cache);

            return true;
        }
    }

    return false;
}

void AudioServer::SetA2dpAudioParameter(const std::string &renderValue)
{
    std::lock_guard<std::mutex> lock(setA2dpParamMutex_);
    auto parmKey = AudioParamKey::A2DP_SUSPEND_STATE;
    std::shared_ptr<IAudioRenderSink> btSink = GetSinkByProp(HDI_ID_TYPE_BLUETOOTH);
    if (btSink == nullptr || !btSink->IsInited()) {
        AUDIO_WARNING_LOG("has no valid sink, need preStore a2dpParam.");
        HdiAdapterManager::GetInstance().
            UpdateSinkPrestoreInfo<std::pair<AudioParamKey, std::pair<std::string, std::string>>>(
            PRESTORE_INFO_AUDIO_BT_PARAM, {parmKey, {"", renderValue}});
        return;
    }
    btSink->SetAudioParameter(parmKey, "", renderValue);

    if (AudioService::GetInstance()->HasBluetoothEndpoint()) {
        std::shared_ptr<IAudioRenderSink> btFastSink = GetSinkByProp(HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP);
        CHECK_AND_RETURN_LOG(btFastSink != nullptr, "has no valid fast sink");
        btFastSink->SetAudioParameter(parmKey, "", renderValue);
        AUDIO_INFO_LOG("HasBlueToothEndpoint");
    }
}
// LCOV_EXCL_STOP
int32_t AudioServer::SetAudioParameter(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    AudioXCollie audioXCollie("AudioServer::SetAudioParameter", TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    if (key != "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
        bool ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "MODIFY_AUDIO_SETTINGS permission denied");
    } else {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "modify permission denied");
    }

    if (key == "VOICE_PHONE_STATUS") {
        AudioServer::audioParameters[key] = value;
        return SUCCESS;
    }

    if (key == "A2dpSuspended") {
        AudioServer::audioParameters[key] = value;
        SetA2dpAudioParameter(key + "=" + value + ";");
        return SUCCESS;
    }

    AudioParamKey parmKey = AudioParamKey::NONE;
    std::string valueNew = value;
    std::string halName = "primary";
    CHECK_AND_RETURN_RET(UpdateAudioParameterInfo(key, value, parmKey, valueNew, halName), SUCCESS);

    CHECK_AND_RETURN_RET_LOG(audioParameters.size() < PARAMETER_SET_LIMIT, ERR_INVALID_PARAM, "too large!");
    AudioServer::audioParameters[key] = value;

    std::shared_ptr<IAudioCaptureSource> source = GetSourceByProp(HDI_ID_TYPE_VA, HDI_ID_INFO_VA, true);
    if (source != nullptr) {
        source->SetAudioParameter(parmKey, "", valueNew);
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, SUCCESS, "deviceManager is null");
    deviceManager->SetAudioParameter(halName, parmKey, "", valueNew);

    return SUCCESS;
}

bool AudioServer::UpdateAudioParameterInfo(const std::string &key, const std::string &value,
    AudioParamKey &parmKey, std::string &valueNew, std::string &halName)
{
    if (key == "AUDIO_EXT_PARAM_KEY_LOWPOWER") {
        parmKey = AudioParamKey::PARAM_KEY_LOWPOWER;
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "SMARTPA_LOWPOWER",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "STATE", valueNew == "SmartPA_lowpower=on" ? 1 : 0);
    } else if (key == "bt_headset_nrec") {
        parmKey = AudioParamKey::BT_HEADSET_NREC;
    } else if (key == "bt_wbs") {
        parmKey = AudioParamKey::BT_WBS;
    } else if (key == "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
        parmKey = AudioParamKey::A2DP_OFFLOAD_STATE;
        valueNew = "a2dpOffloadConfig=" + value;
    } else if (key == "mmi") {
        parmKey = AudioParamKey::MMI;
    } else if (key == "perf_info") {
        parmKey = AudioParamKey::PERF_INFO;
    } else if (key == "mute_call" || key == "game_record_recognition") {
        valueNew = key + "=" + value;
    } else if (key == "LOUD_VOLUMN_MODE") {
        parmKey = AudioParamKey::NONE;
    } else if ((key == "pm_kara") || (key == "pm_kara_code")) {
        parmKey = AudioParamKey::USB_DEVICE;
        halName = "usb";
        valueNew = key + "=" +value;
    } else {
        return false;
    }
    return true;
}

int32_t AudioServer::SuspendRenderSink(const std::string &sinkName)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    uint32_t id = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(sinkName.c_str());
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERROR, "get sink fail, sinkName: %{public}s", sinkName.c_str());
    return sink->SuspendRenderSink();
}

int32_t AudioServer::RestoreRenderSink(const std::string &sinkName)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    uint32_t id = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(sinkName.c_str());
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERROR, "get sink fail, sinkName: %{public}s", sinkName.c_str());
    return sink->RestoreRenderSink();
}

int32_t AudioServer::SetAudioParameter(const std::string& networkId, int32_t key, const std::string& condition,
    const std::string& value)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bool ret = VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio() || ret, ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "device manager is nullptr");
    deviceManager->SetAudioParameter(networkId.c_str(), static_cast<AudioParamKey>(key), condition, value);
    return SUCCESS;
}

bool AudioServer::GetPcmDumpParameter(const std::vector<std::string> &subKeys,
    std::vector<std::pair<std::string, std::string>> &result)
{
    bool ret = VerifyClientPermission(DUMP_AUDIO_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, false, "get audiodump parameters no permission");
    CHECK_AND_RETURN_RET_LOG(subKeys.size() > 0, false, "subKeys is empty!");
    return AudioCacheMgr::GetInstance().GetDumpParameter(subKeys, result);
}

bool AudioServer::GetEffectLiveParameter(const std::vector<std::string> &subKeys,
    std::vector<std::pair<std::string, std::string>> &result)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().GetEffectLiveParameter(subKeys, result);
    }
    AUDIO_INFO_LOG("GetEffectLiveParameter not support");
    return false;
}

int32_t AudioServer::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<StringPair> &parameters)
{
    CHECK_AND_RETURN_RET_LOG(subKeys.size() >= 0 && subKeys.size() <= AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Get extra audio parameters failed");
    std::vector<std::pair<std::string, std::string>> result;
    int32_t res = GetExtraParametersInner(mainKey, subKeys, result);
    if (res == SUCCESS) {
        parameters = ConvertStringPair(result);
    }
    return res;
}

int32_t AudioServer::GetExtraParametersInner(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    if (mainKey == EFFECT_LIVE_KEY) {
        bool ret = GetEffectLiveParameter(subKeys, result);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "get effect live parameters failed.");
        return SUCCESS;
    }
    if (mainKey == PCM_DUMP_KEY) {
        bool ret = GetPcmDumpParameter(subKeys, result);
        CHECK_AND_RETURN_RET_LOG(ret, ERROR, "get audiodump parameters failed");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(isAudioParameterParsed_.load(), ERROR, "audioParameterKeys is not ready");

    if (audioParameterKeys.empty()) {
        AUDIO_ERR_LOG("audio extra parameters mainKey and subKey is empty");
        return ERROR;
    }

    auto mainKeyIt = audioParameterKeys.find(mainKey);
    if (mainKeyIt == audioParameterKeys.end()) {
        return ERR_INVALID_PARAM;
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERROR, "device manager is nullptr");
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = mainKeyIt->second;
    if (subKeys.empty()) {
        for (auto it = subKeyMap.begin(); it != subKeyMap.end(); it++) {
            std::string value = deviceManager->GetAudioParameter("primary", AudioParamKey::NONE, it->first);
            result.emplace_back(std::make_pair(it->first, value));
        }
        return SUCCESS;
    }

    bool match = true;
    for (auto it = subKeys.begin(); it != subKeys.end(); it++) {
        auto subKeyIt = subKeyMap.find(*it);
        if (subKeyIt != subKeyMap.end()) {
            std::string value = deviceManager->GetAudioParameter("primary", AudioParamKey::NONE, *it);
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

int32_t AudioServer::GetAudioParameter(const std::string &key, std::string &value)
{
    value = GetAudioParameterInner(key);
    if (value == "") {
        value = GetVAParameter(key);
    }
    return SUCCESS;
}

const std::string AudioServer::GetAudioParameterInner(const std::string &key)
{
    if (IPCSkeleton::GetCallingUid() == MEDIA_SERVICE_UID) {
        return "";
    }
    std::lock_guard<std::mutex> lockSet(audioParameterMutex_);
    AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    AudioParamKey parmKey = AudioParamKey::NONE;
    if (deviceManager != nullptr) {
        if (key == "AUDIO_EXT_PARAM_KEY_LOWPOWER") {
            parmKey = AudioParamKey::PARAM_KEY_LOWPOWER;
            return deviceManager->GetAudioParameter("primary", AudioParamKey(parmKey), "");
        }
        if (key.find("need_change_usb_device#C", 0) == 0) {
            parmKey = AudioParamKey::USB_DEVICE;
            return deviceManager->GetAudioParameter("primary", AudioParamKey(parmKey), key);
        }
        if (key == "getSmartPAPOWER" || key == "show_RealTime_ChipModel") {
            return deviceManager->GetAudioParameter("primary", AudioParamKey::NONE, key);
        }
        if (key == "perf_info") {
            return deviceManager->GetAudioParameter("primary", AudioParamKey::PERF_INFO, key);
        }
        if (key == "concurrent_capture_stream_info") {
            return deviceManager->GetAudioParameter("primary", AudioParamKey::NONE, key);
        }
        if ((key == "pm_kara") || (key == "pm_kara_code")) {
            return deviceManager->GetAudioParameter("usb", AudioParamKey::USB_DEVICE, key);
        }
        if (key.size() < BUNDLENAME_LENGTH_LIMIT && key.size() > CHECK_FAST_BLOCK_PREFIX.size() &&
            key.substr(0, CHECK_FAST_BLOCK_PREFIX.size()) == CHECK_FAST_BLOCK_PREFIX) {
            return deviceManager->GetAudioParameter("primary", AudioParamKey::NONE, key);
        }

        const std::string mmiPre = "mmi_";
        if (key.size() > mmiPre.size()) {
            if (key.substr(0, mmiPre.size()) == mmiPre) {
                parmKey = AudioParamKey::MMI;
                return deviceManager->GetAudioParameter("primary", AudioParamKey(parmKey),
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

const std::string AudioServer::GetVAParameter(const std::string &key)
{
    AudioParamKey parmKey = AudioParamKey::NONE;
    std::shared_ptr<IAudioCaptureSource> source = GetSourceByProp(HDI_ID_TYPE_VA, HDI_ID_INFO_VA, true);
    if (source != nullptr) {
        if (key == "AUDIO_EXT_PARAM_KEY_LOWPOWER") {
            parmKey = AudioParamKey::PARAM_KEY_LOWPOWER;
            return source->GetAudioParameter(AudioParamKey(parmKey), "");
        }
        if (key.find("need_change_usb_device#C") == 0) {
            parmKey = AudioParamKey::USB_DEVICE;
            return source->GetAudioParameter(AudioParamKey(parmKey), key);
        }
        if (key == "getSmartPAPOWER" || key == "show_RealTime_ChipModel") {
            return source->GetAudioParameter(AudioParamKey::NONE, key);
        }
        if (key == "perf_info") {
            return source->GetAudioParameter(AudioParamKey::PERF_INFO, key);
        }
        if (key == "concurrent_capture_stream_info") {
            return source->GetAudioParameter(AudioParamKey::NONE, key);
        }
        if (key.size() < BUNDLENAME_LENGTH_LIMIT && key.size() > CHECK_FAST_BLOCK_PREFIX.size() &&
            key.substr(0, CHECK_FAST_BLOCK_PREFIX.size()) == CHECK_FAST_BLOCK_PREFIX) {
            return source->GetAudioParameter(AudioParamKey::NONE, key);
        }

        const std::string mmiPre2 = "mmi_";
        if (key.size() > mmiPre2.size() && key.substr(0, mmiPre2.size()) == mmiPre2) {
            parmKey = AudioParamKey::MMI;
            return source->GetAudioParameter(AudioParamKey(parmKey),
                key.substr(mmiPre2.size(),
                           key.size() - mmiPre2.size()));
        } else {
            return "";
        }
    } else {
        return "";
    }
}

const std::string AudioServer::GetDPParameter(const std::string &condition)
{
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DP, true);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, "", "get dp sink fail");

    return sink->GetAudioParameter(AudioParamKey::GET_DP_DEVICE_INFO, condition);
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
    lock_guard<mutex> lg(mtxGetUsbParameter_);
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    CHECK_AND_RETURN_RET_LOG(sink, "", "rendererSink is nullptr");
    std::string infoCond = std::string("get_usb_info#C") + GetField(address, "card", ';') + "D0";
    if (role == OUTPUT_DEVICE) {
        sink->SetAddress(address);
        auto it = usbInfoMap_.find(address);
        if (it == usbInfoMap_.end()) {
            usbInfoStr = sink->GetAudioParameter(USB_DEVICE, infoCond);
            usbInfoMap_[address] = usbInfoStr;
        } else {
            usbInfoStr = it->second;
        }
    } else if (role == INPUT_DEVICE) {
        std::shared_ptr<IAudioCaptureSource> source = GetSourceByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
        CHECK_AND_RETURN_RET_LOG(source, "", "capturerSource is nullptr");
        source->SetAddress(address);
        auto it = usbInfoMap_.find(address);
        if (it == usbInfoMap_.end()) {
            usbInfoStr = sink->GetAudioParameter(USB_DEVICE, infoCond);
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

int32_t AudioServer::GetAudioParameter(const std::string& networkId, int32_t key,
    const std::string& condition, std::string& value)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio() ||
        VerifyClientPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    value = GetAudioParameterInner(networkId, static_cast<AudioParamKey>(key), condition);
    return SUCCESS;
}

const std::string AudioServer::GetAudioParameterInner(const std::string& networkId, const AudioParamKey key,
    const std::string& condition)
{
    if (networkId == LOCAL_NETWORK_ID) {
        AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS,
            nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
        if (key == AudioParamKey::USB_DEVICE) {
            return GetUsbParameter(condition);
        }
        if (key == AudioParamKey::GET_DP_DEVICE_INFO) {
            return GetDPParameter(condition);
        }
    } else {
        std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_REMOTE, networkId);
        CHECK_AND_RETURN_RET_LOG(sink != nullptr, "", "get remote sink fail");
        return sink->GetAudioParameter(key, condition);
    }
    return "";
}

int32_t AudioServer::GetTransactionId(int32_t deviceType, int32_t deviceRole, uint64_t& transactionId)
{
    AUDIO_DEBUG_LOG("device type: %{public}d, device role: %{public}d", deviceType, deviceRole);
    if (deviceRole != INPUT_DEVICE && deviceRole != OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("AudioServer::GetTransactionId: error device role");
        transactionId =  static_cast<uint64_t>(ERR_INVALID_PARAM);
        return SUCCESS;
    }
    std::shared_ptr<IAudioCaptureSource> source = nullptr;
    if (deviceRole == INPUT_DEVICE) {
        if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
            source = GetSourceByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
        } else {
            source = GetSourceByProp(HDI_ID_TYPE_PRIMARY);
        }
        if (source) {
            transactionId = source->GetTransactionId();
        }
        return SUCCESS;
    }

    // deviceRole OUTPUT_DEVICE
    std::shared_ptr<IAudioRenderSink> sink = nullptr;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        sink = GetSinkByProp(HDI_ID_TYPE_BLUETOOTH);
    } else if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
        sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
    } else {
        sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    }
    int32_t ret = ERROR;
    if (sink != nullptr) {
        ret = sink->GetTransactionId(transactionId);
    }

    CHECK_AND_RETURN_RET_LOG(!ret, SUCCESS, "Get transactionId failed.");

    AUDIO_DEBUG_LOG("Transaction Id: %{public}" PRIu64, transactionId);
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioServer::SetMicrophoneMute(bool isMute)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "refused for %{public}d",
        callingUid);
    auto limitFunc = [](uint32_t captureId) -> bool {
        std::string info = IdHandler::GetInstance().ParseInfo(captureId);
#ifdef DAUDIO_ENABLE
        if (IdHandler::GetInstance().ParseType(captureId) == HDI_ID_TYPE_REMOTE) {
            return true;
        }
#endif
        if (IdHandler::GetInstance().ParseType(captureId) == HDI_ID_TYPE_PRIMARY) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_USB;
        }
        if (IdHandler::GetInstance().ParseType(captureId) == HDI_ID_TYPE_BLUETOOTH) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        return false;
    };
    auto processFunc = [isMute, limitFunc](uint32_t captureId, std::shared_ptr<IAudioCaptureSource> source) -> int32_t {
        CHECK_AND_RETURN_RET(limitFunc(captureId), SUCCESS);
        CHECK_AND_RETURN_RET(source != nullptr, SUCCESS);

        source->SetMute(isMute);
        return SUCCESS;
    };
    (void)HdiAdapterManager::GetInstance().ProcessSource(processFunc);
    std::shared_ptr<IDeviceManager> deviceManager = HdiAdapterManager::GetInstance().GetDeviceManager(
        HDI_DEVICE_MANAGER_TYPE_LOCAL);
    if (deviceManager != nullptr) {
        deviceManager->AllAdapterSetMicMute(isMute);
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
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);

    if (deviceManager == nullptr) {
        AUDIO_WARNING_LOG("device manager is null.");
    } else {
        return deviceManager->SetVoiceVolume("primary", volume);
    }
    return ERROR;
}

int32_t AudioServer::OffloadSetVolume(float volume, const std::string &deviceClass, const std::string &networkId)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    std::string info = networkId == LOCAL_NETWORK_ID ? HDI_ID_INFO_DEFAULT : networkId;
    uint32_t id = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, info);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return sink->SetVolume(volume, volume);
}

int32_t AudioServer::SetAudioScene(int32_t audioScene, int32_t a2dpOffloadFlag, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("Scene: %{public}d, a2dpOffloadFlag: %{public}d, scoExcludeFlag: %{public}d",
        audioScene, a2dpOffloadFlag, scoExcludeFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    return SetAudioSceneInner(static_cast<AudioScene>(audioScene), static_cast<BluetoothOffloadState>(a2dpOffloadFlag),
        scoExcludeFlag);
}

int32_t AudioServer::SetAudioSceneInner(AudioScene audioScene, BluetoothOffloadState a2dpOffloadFlag,
    bool scoExcludeFlag)
{
    std::lock_guard<std::mutex> lock(audioSceneMutex_);
    AudioXCollie audioXCollie("AudioServer::SetAudioScene", TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    SetAudioSceneForAllSource(audioScene);
    SetAudioSceneForAllSink(audioScene, scoExcludeFlag);

    audioScene_ = audioScene;
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetIORoutes(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType type = activeDevices.front().first;
    DeviceFlag flag = activeDevices.front().second;

    std::vector<DeviceType> deviceTypes;
    for (auto activeDevice : activeDevices) {
        deviceTypes.push_back(activeDevice.first);
    }
    HILOG_COMM_INFO("SetIORoutes 1st deviceType: %{public}d, deviceSize : %{public}zu, flag: %{public}d",
        type, deviceTypes.size(), flag);
    int32_t ret = SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    return ret;
}

int32_t AudioServer::SetIORoutes(DeviceType type, DeviceFlag flag, std::vector<DeviceType> deviceTypes,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
{
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    std::shared_ptr<IAudioCaptureSource> source = nullptr;

    if (type == DEVICE_TYPE_USB_ARM_HEADSET) {
        UpdateArmInstance(sink, source);
    } else if (type == DEVICE_TYPE_ACCESSORY) {
        source = GetSourceByProp(HDI_ID_TYPE_ACCESSORY, HDI_ID_INFO_ACCESSORY, true);
    } else {
        source = GetSourceByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
        if (type == DEVICE_TYPE_BLUETOOTH_A2DP && a2dpOffloadFlag != A2DP_OFFLOAD) {
            deviceTypes[0] = DEVICE_TYPE_NONE;
        }
    }
    CHECK_AND_RETURN_RET_LOG(sink != nullptr || source != nullptr,
        ERR_INVALID_PARAM, "SetIORoutes failed for null instance!");

    std::lock_guard<std::mutex> lock(audioSceneMutex_);
    if (flag == DeviceFlag::INPUT_DEVICES_FLAG) {
        UpdateDeviceForAllSource(source, type);
    } else if (flag == DeviceFlag::OUTPUT_DEVICES_FLAG) {
        PolicyHandler::GetInstance().SetActiveOutputDevice(type);
        sink->UpdateActiveDevice(deviceTypes);
    } else if (flag == DeviceFlag::ALL_DEVICES_FLAG) {
        UpdateDeviceForAllSource(source, type);
        PolicyHandler::GetInstance().SetActiveOutputDevice(type);
        sink->UpdateActiveDevice(deviceTypes);
    } else {
        AUDIO_ERR_LOG("SetIORoutes invalid device flag");
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

int32_t AudioServer::UpdateActiveDeviceRoute(int32_t type, int32_t flag, int32_t a2dpOffloadFlag)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    std::vector<IntPair> activeDevices;
    activeDevices.push_back({type, flag});
    return UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag, "");
}

int32_t AudioServer::UpdateActiveDevicesRoute(const std::vector<IntPair> &activeDevices,
    int32_t a2dpOffloadFlag, const std::string &deviceName)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeOutputDevices;
    for (auto activeDevice : activeDevices) {
        DeviceType type = static_cast<DeviceType>(activeDevice.firstParam);
        DeviceFlag flag = static_cast<DeviceFlag>(activeDevice.secondParam);
        activeOutputDevices.push_back({type, flag});
    }
    return SetIORoutes(activeOutputDevices, static_cast<BluetoothOffloadState>(a2dpOffloadFlag), deviceName);
}

// LCOV_EXCL_START
int32_t AudioServer::SetDmDeviceType(uint16_t dmDeviceType, int32_t deviceType)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    std::shared_ptr<IAudioCaptureSource> source;
    if (static_cast<DeviceType>(deviceType) == DEVICE_TYPE_NEARLINK_IN) {
        source = GetSourceByProp(HDI_ID_TYPE_PRIMARY);
    } else {
        source = GetSourceByProp(HDI_ID_TYPE_ACCESSORY, HDI_ID_INFO_ACCESSORY, true);
    }
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERROR, "has no valid source");

    source->SetDmDeviceType(dmDeviceType, static_cast<DeviceType>(deviceType));
    return SUCCESS;
}

int32_t AudioServer::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("AudioMonoState = [%{public}s]", audioMono ? "true": "false");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    auto limitFunc = [](uint32_t renderId) -> bool {
        uint32_t type = IdHandler::GetInstance().ParseType(renderId);
        std::string info = IdHandler::GetInstance().ParseInfo(renderId);
        if (type == HDI_ID_TYPE_PRIMARY) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_DIRECT || info == HDI_ID_INFO_VOIP;
        }
        if (type == HDI_ID_TYPE_BLUETOOTH) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        if (type == HDI_ID_TYPE_OFFLOAD) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        return false;
    };
    auto processFunc = [audioMono, limitFunc](uint32_t renderId, std::shared_ptr<IAudioRenderSink> sink) -> int32_t {
        CHECK_AND_RETURN_RET(limitFunc(renderId), SUCCESS);
        CHECK_AND_RETURN_RET(sink != nullptr, SUCCESS);

        sink->SetAudioMonoState(audioMono);
        return SUCCESS;
    };
    (void)HdiAdapterManager::GetInstance().ProcessSink(processFunc);
    HdiAdapterManager::GetInstance().UpdateSinkPrestoreInfo<bool>(PRESTORE_INFO_AUDIO_MONO, audioMono);
    return SUCCESS;
}

int32_t AudioServer::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("AudioBalanceValue = [%{public}f]", audioBalance);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    CHECK_AND_RETURN_RET_LOG(audioBalance >= -1.0f && audioBalance <= 1.0f, ERR_INVALID_PARAM,
        "audioBalance value %{public}f is out of range [-1.0, 1.0]", audioBalance);

    auto limitFunc = [](uint32_t renderId) -> bool {
        uint32_t type = IdHandler::GetInstance().ParseType(renderId);
        std::string info = IdHandler::GetInstance().ParseInfo(renderId);
        if (type == HDI_ID_TYPE_PRIMARY) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_DIRECT || info == HDI_ID_INFO_VOIP;
        }
        if (type == HDI_ID_TYPE_BLUETOOTH) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        if (type == HDI_ID_TYPE_OFFLOAD) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        return false;
    };
    auto processFunc = [audioBalance, limitFunc](uint32_t renderId, std::shared_ptr<IAudioRenderSink> sink) -> int32_t {
        CHECK_AND_RETURN_RET(limitFunc(renderId), SUCCESS);
        CHECK_AND_RETURN_RET(sink != nullptr, SUCCESS);

        sink->SetAudioBalanceValue(audioBalance);
        return SUCCESS;
    };
    (void)HdiAdapterManager::GetInstance().ProcessSink(processFunc);
    HdiAdapterManager::GetInstance().UpdateSinkPrestoreInfo<float>(PRESTORE_INFO_AUDIO_BALANCE, audioBalance);
    return SUCCESS;
}

int32_t AudioServer::NotifyDeviceInfo(const std::string &networkId, bool connected)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    AUDIO_INFO_LOG("notify device info: networkId(%{public}s), connected(%{public}d)",
        GetEncryptStr(networkId).c_str(), connected);
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_REMOTE, networkId.c_str());
    if (sink != nullptr && connected) {
        sink->RegistCallback(HDI_CB_RENDER_PARAM, this);
    }
    std::shared_ptr<IAudioRenderSink> sinkOffload = GetSinkByProp(HDI_ID_TYPE_REMOTE_OFFLOAD, networkId.c_str(),
        false, false);
    CHECK_AND_RETURN_RET(sinkOffload != nullptr && connected, SUCCESS);
    sinkOffload->RegistCallback(HDI_CB_RENDER_PARAM, this);
    return SUCCESS;
}
// LCOV_EXCL_STOP

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

int32_t AudioServer::RegistCoreServiceProvider(const sptr<IRemoteObject> &object)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    CHECK_AND_RETURN_RET_LOG(coreServiceProvider != nullptr, ERR_INVALID_PARAM,
        "coreServiceProvider obj cast failed");
    int32_t ret = CoreServiceHandler::GetInstance().ConfigCoreServiceProvider(coreServiceProvider);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConfigCoreServiceProvider failed!");
    return SUCCESS;
}

int32_t AudioServer::GetHapBuildApiVersion(int32_t callerUid)
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
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

    WatchTimeout reguard("bundleMgrProxy->GetBundleNameForUid:GetHapBuildApiVersion");
    bundleMgrProxy->GetBundleNameForUid(callerUid, bundleName);
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
        SendCreateErrorInfo(config, ERR_INVALID_PARAM);
        return false;
    }
    return true;
}

bool AudioServer::CheckRecorderFormat(const AudioProcessConfig &config)
{
    if (NotContain(AUDIO_SUPPORTED_SOURCE_TYPES, config.capturerInfo.sourceType)) {
        AUDIO_ERR_LOG("Check format failed invalid sourceType:%{public}d", config.capturerInfo.sourceType);
        SendCreateErrorInfo(config, ERR_INVALID_PARAM);
        return false;
    }
    if (config.capturerInfo.capturerFlags != AUDIO_FLAG_NORMAL && NotContain(AUDIO_FAST_STREAM_SUPPORTED_SOURCE_TYPES,
        config.capturerInfo.sourceType)) {
        AUDIO_ERR_LOG("Check format failed invalid fast sourceType:%{public}d", config.capturerInfo.sourceType);
        SendCreateErrorInfo(config, ERR_INVALID_PARAM);
        return false;
    }
    return true;
}

bool AudioServer::CheckConfigFormat(const AudioProcessConfig &config)
{
    if (!CheckStreamInfoFormat(config)) {
        SendCreateErrorInfo(config, ERR_INVALID_PARAM);
        return false;
    }
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        return CheckRendererFormat(config);
    }

    if (config.audioMode == AUDIO_MODE_RECORD) {
        return CheckRecorderFormat(config);
    }

    SendCreateErrorInfo(config, ERR_INVALID_PARAM);
    AUDIO_ERR_LOG("Check format failed invalid mode.");
    return false;
}

bool AudioServer::IsFastBlocked(int32_t uid, PlayerType playerType)
{
    // if call from soundpool without the need for check.
    if (playerType == PLAYER_TYPE_SOUND_POOL) {
        return false;
    }
    std::string bundleName = AppBundleManager::GetBundleNameFromUid(uid);
    std::string result;
    GetAudioParameter(CHECK_FAST_BLOCK_PREFIX + bundleName, result);
    return result == "true";
}

void AudioServer::SendCreateErrorInfo(const AudioProcessConfig &config, int32_t errorCode)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_STREAM_CREATE_ERROR_STATS,
        Media::MediaMonitor::FREQUENCY_AGGREGATION_EVENT);
    bool isPlayBack = config.audioMode == AUDIO_MODE_PLAYBACK ? 1 : 0;
    bean->Add("IS_PLAYBACK", (isPlayBack ? 1 : 0));
    bean->Add("CLIENT_UID", config.appInfo.appUid);
    bean->Add("STREAM_TYPE", isPlayBack ? static_cast<int32_t>(config.rendererInfo.streamUsage) :
        static_cast<int32_t>(config.capturerInfo.sourceType));
    bean->Add("ERROR_CODE", errorCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioServer::CheckMaxRendererInstances()
{
    int32_t maxRendererInstances = PolicyHandler::GetInstance().GetMaxRendererInstances();
    if (maxRendererInstances <= 0) {
        maxRendererInstances = DEFAULT_MAX_RENDERER_INSTANCES;
    }
    if (AudioService::GetInstance()->GetCurrentRendererStreamCnt() >= maxRendererInstances) {
        AUDIO_ERR_LOG("Current audio renderer stream num is greater than the maximum num of configured instances");
        return ERR_EXCEED_MAX_STREAM_CNT;
    }
    return SUCCESS;
}

int32_t AudioServer::CheckMaxLoopbackInstances(AudioMode audioMode)
{
    if (AudioService::GetInstance()->GetCurrentLoopbackStreamCnt(audioMode) >= DEFAULT_MAX_LOOPBACK_INSTANCES) {
        AUDIO_ERR_LOG("Current Loopback stream num is greater than the maximum num of configured instances");
        return ERR_EXCEED_MAX_STREAM_CNT;
    }
    return SUCCESS;
}

sptr<IRemoteObject> AudioServer::CreateAudioStream(const AudioProcessConfig &config, int32_t callingUid,
    std::shared_ptr<PipeInfoGuard> &pipeInfoGuard)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfoGuard != nullptr, nullptr, "PipeInfoGuard is nullptr");
    AudioXCollie audioXCollie(
        "AudioServer::CreateAudioStream", CREATE_TIMEOUT_IN_SECOND, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    int32_t appUid = config.appInfo.appUid;
    if (callingUid != MEDIA_SERVICE_UID) {
        appUid = callingUid;
    }
    if (IsNormalIpcStream(config)) {
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
        pipeInfoGuard->SetReleaseFlag(false);
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
    if (config.capturerInfo.isLoopback || config.rendererInfo.isLoopback) {
        AudioService::GetInstance()->SetIncMaxLoopbackStreamCnt(config.audioMode);
    }
    sptr<IRemoteObject> remoteObject= process->AsObject();
    pipeInfoGuard->SetReleaseFlag(false);
    return remoteObject;
#else
    AUDIO_ERR_LOG("GetAudioProcess failed.");
    return nullptr;
#endif
}

int32_t AudioServer::CheckAndWaitAudioPolicyReady()
{
    if (!isAudioPolicyReady_) {
        std::unique_lock lock(isAudioPolicyReadyMutex_);
        if (waitCreateStreamInServerCount_ > MAX_WAIT_IN_SERVER_COUNT) {
            AUDIO_WARNING_LOG("let client retry");
            return ERR_RETRY_IN_CLIENT;
        }
        waitCreateStreamInServerCount_++;
        isAudioPolicyReadyCv_.wait_for(lock, std::chrono::seconds(WAIT_AUDIO_POLICY_READY_TIMEOUT_SECONDS), [this] () {
            return isAudioPolicyReady_.load();
        });
        waitCreateStreamInServerCount_--;
    }

    return SUCCESS;
}

void AudioServer::NotifyProcessStatus()
{
    // when audio_server start, set audio_server rssThresHold
    void *libMemMgrClientHandle = dlopen("libmemmgrclient.z.so", RTLD_NOW);
    if (!libMemMgrClientHandle) {
        AUDIO_INFO_LOG("dlopen libmemmgrclient library failed");
        return;
    }
    void *notifyProcessStatusFunc = dlsym(libMemMgrClientHandle, "notify_process_status");
    if (!notifyProcessStatusFunc) {
        AUDIO_INFO_LOG("dlsm notify_process_status failed");
#ifndef TEST_COVERAGE
        dlclose(libMemMgrClientHandle);
#endif
        return;
    }
    auto notifyProcessStatus = reinterpret_cast<int(*)(int, int, int, int)>(notifyProcessStatusFunc);
    AUDIO_INFO_LOG("notify to memmgr when audio_server is started");
    int pid = getpid();
    notifyProcessStatus(pid, 1, RSS_THRESHOLD, 0);
#ifndef TEST_COVERAGE
    dlclose(libMemMgrClientHandle);
#endif
}

// LCOV_EXCL_START
int32_t AudioServer::CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode,
    const AudioPlaybackCaptureConfig &filterConfig, sptr<IRemoteObject>& client)
{
    client = CreateAudioProcessInner(config, errorCode, filterConfig);
    if (client == nullptr) {
        AUDIO_ERR_LOG("CreateAudioProcessInner failed");
        if (errorCode == 0) {
            errorCode = AUDIO_ERR;
        }
    }
    return SUCCESS;
}

bool AudioServer::IsSatellite(const AudioProcessConfig &config, int32_t callingUid)
{
    return config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        callingUid == UID_FOUNDATION_SA && config.rendererInfo.isSatellite;
}

sptr<IRemoteObject> AudioServer::CreateAudioProcessInner(const AudioProcessConfig &config, int32_t &errorCode,
    const AudioPlaybackCaptureConfig &filterConfig)
{
    Trace trace("AudioServer::CreateAudioProcess");
    std::shared_ptr<PipeInfoGuard> pipeinfoGuard = std::make_shared<PipeInfoGuard>(config.originalSessionId);

    errorCode = CheckAndWaitAudioPolicyReady();
    CHECK_AND_RETURN_RET(errorCode == SUCCESS, nullptr);

    AudioProcessConfig resetConfig = ResetProcessConfig(config);
    CHECK_AND_RETURN_RET_LOG(CheckConfigFormat(resetConfig), nullptr, "AudioProcessConfig format is wrong, please check"
        ":%{public}s", ProcessConfig::DumpProcessConfig(resetConfig).c_str());
    CHECK_AND_RETURN_RET_LOG(PermissionChecker(resetConfig), nullptr, "Create audio process failed, no permission");

    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (resetConfig.audioMode == AUDIO_MODE_PLAYBACK &&
        !IsVoiceModemCommunication(resetConfig.rendererInfo.streamUsage, callingUid)) {
        errorCode = CheckMaxRendererInstances();
        CHECK_AND_RETURN_RET(errorCode == SUCCESS, nullptr);
        if (AudioService::GetInstance()->IsExceedingMaxStreamCntPerUid(callingUid, resetConfig.appInfo.appUid,
            maxRendererStreamCntPerUid_)) {
            errorCode = ERR_EXCEED_MAX_STREAM_CNT_PER_UID;
            AUDIO_ERR_LOG("Current audio renderer stream num exceeds maxRendererStreamCntPerUid");
            return nullptr;
        }
    }
    if (resetConfig.rendererInfo.isLoopback || resetConfig.capturerInfo.isLoopback) {
        errorCode = CheckMaxLoopbackInstances(resetConfig.audioMode);
        CHECK_AND_RETURN_RET(errorCode == SUCCESS, nullptr);
    }
    if (IsSatellite(resetConfig, callingUid)) {
        bool isSupportSate = OHOS::system::GetBoolParameter(TEL_SATELLITE_SUPPORT, false);
        CHECK_AND_RETURN_RET_LOG(isSupportSate, nullptr, "Do not support satellite");
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        if (deviceManager != nullptr) {
            deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", SATEMODEM_PARAMETER);
        }
    }
#ifdef FEATURE_APPGALLERY
    PolicyHandler::GetInstance().GetAndSaveClientType(resetConfig.appInfo.appUid,
        AppBundleManager::GetBundleNameFromUid(resetConfig.appInfo.appUid));
#endif
#ifdef HAS_FEATURE_INNERCAPTURER
    if (!HandleCheckCaptureLimit(resetConfig, filterConfig)) {
        return nullptr;
    }
#endif
    return CreateAudioStream(resetConfig, callingUid, pipeinfoGuard);
}

#ifdef HAS_FEATURE_INNERCAPTURER
bool AudioServer::HandleCheckCaptureLimit(AudioProcessConfig &resetConfig,
    const AudioPlaybackCaptureConfig &filterConfig)
{
    if (resetConfig.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        int32_t innerCapId = 0;
        if (InnerCheckCaptureLimit(filterConfig, innerCapId) == SUCCESS) {
            resetConfig.innerCapId = innerCapId;
        } else {
            AUDIO_ERR_LOG("CheckCaptureLimit fail!");
            return false;
        }
    }
    return true;
}

int32_t AudioServer::InnerCheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    int32_t ret = playbackCapturerMgr->CheckCaptureLimit(config, innerCapId);
    if (ret == SUCCESS) {
        PolicyHandler::GetInstance().LoadModernInnerCapSink(innerCapId);
        bool isSupportInnerCaptureOffload = PolicyHandler::GetInstance().IsSupportInnerCaptureOffload();
        AUDIO_INFO_LOG("LoadModernOffloadCapSource %{public}d", isSupportInnerCaptureOffload);
        if (isSupportInnerCaptureOffload) {
            PolicyHandler::GetInstance().LoadModernOffloadCapSource();
        }
    }
    return ret;
}
#endif

bool AudioServer::IsNormalIpcStream(const AudioProcessConfig &config) const
{
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        return config.rendererInfo.rendererFlags == AUDIO_FLAG_NORMAL ||
            config.rendererInfo.rendererFlags == AUDIO_FLAG_VOIP_DIRECT ||
            config.rendererInfo.rendererFlags == AUDIO_FLAG_DIRECT;
    } else if (config.audioMode == AUDIO_MODE_RECORD) {
        return config.capturerInfo.capturerFlags == AUDIO_FLAG_NORMAL;
    }

    return false;
}

int32_t AudioServer::CheckRemoteDeviceState(const std::string &networkId, int32_t deviceRole, bool isStartDevice)
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
                std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_REMOTE, networkId.c_str());
                if (sink == nullptr || !sink->IsInited()) {
                    AUDIO_ERR_LOG("Remote renderer[%{public}s] is uninit.", networkId.c_str());
                    return ERR_ILLEGAL_STATE;
                }
                ret = sink->Start();
                break;
            }
        case INPUT_DEVICE:
            {
                std::shared_ptr<IAudioCaptureSource> source = GetSourceByProp(HDI_ID_TYPE_REMOTE, networkId.c_str());
                if (source == nullptr || !source->IsInited()) {
                    AUDIO_ERR_LOG("Remote capturer[%{public}s] is uninit.", networkId.c_str());
                    return ERR_ILLEGAL_STATE;
                }
                ret = source->Start();
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
// LCOV_EXCL_STOP

void AudioServer::OnRenderSinkParamChange(const std::string &networkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    std::shared_ptr<AudioParameterCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(audioParamCbMtx_);
        AUDIO_INFO_LOG("OnRenderSinkParamChange Callback from networkId: %s", networkId.c_str());
        CHECK_AND_RETURN_LOG(audioParamCb_ != nullptr, "OnRenderSinkParamChange: audio param allback is null.");
        callback = audioParamCb_;
    }
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

void AudioServer::OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    std::shared_ptr<AudioParameterCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(audioParamCbMtx_);
        AUDIO_INFO_LOG("OnCaptureSourceParamChange Callback from networkId: %s", networkId.c_str());
        CHECK_AND_RETURN_LOG(audioParamCb_ != nullptr, "OnCaptureSourceParamChange: audio param allback is null.");
        callback = audioParamCb_;
    }
    callback->OnAudioParameterChange(networkId, key, condition, value);
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

void AudioServer::OnCapturerState(bool isActive, size_t preNum, size_t curNum)
{
    AUDIO_DEBUG_LOG("OnCapturerState Callback start");
    std::shared_ptr<WakeUpSourceCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> lockSet(setWakeupCloseCallbackMutex_);
        callback = wakeupCallback_;
    }

    // Ensure that the send callback is not executed concurrently
    std::lock_guard<std::mutex> lockCb(onCapturerStateCbMutex_);
    bool previousState = preNum;
    bool currentState = curNum;

    if (previousState == currentState) {
        // state not change, need not trigger callback
        return;
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "OnCapturerState callback is nullptr.");
    Trace traceCb("callbackToIntelligentVoice");
    int64_t stamp = ClockTime::GetCurNano();
    callback->OnCapturerState(isActive);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("isActive:%{public}d cb cost[%{public}" PRId64 "]", isActive, stamp);
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
        false, "Permission denied [tid:%{public}d], [%{public}s] for uid:%{public}d tokenId:%{public}u",
        clientTokenId, permissionName.c_str(), callerUid, tokenId);

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

    if (streamUsage == STREAM_USAGE_ULTRASONIC && config.callerUid != UID_MSDP_SA) {
        AUDIO_ERR_LOG("not msdp using ultrasonic uid:%{public}d", config.callerUid);
        return false;
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

// LCOV_EXCL_START
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
    if (sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission, false, "VOICE_TRANSCRIPTION failed: no system permission.");
    }

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
    if (!res) {
        HILOG_COMM_INFO("Check record permission failed: No permission.");
        return false;
    }

    if (sourceType == SOURCE_TYPE_ULTRASONIC && config.callerUid != UID_MSDP_SA) {
        return false;
    }

    if (sourceType == SOURCE_TYPE_WAKEUP) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        bool hasIntelVoicePermission = VerifyClientPermission(MANAGE_INTELLIGENT_VOICE_PERMISSION, tokenId);
        CHECK_AND_RETURN_RET_LOG(hasSystemPermission && hasIntelVoicePermission, false,
            "Create wakeup record stream failed: no permission.");
        return true;
    }

    if (!HandleCheckRecorderBackgroundCapture(config)) {
        HILOG_COMM_INFO("VerifyBackgroundCapture failed for callerUid:%{public}d", config.callerUid);
        return false;
    }
    return true;
}
// LCOV_EXCL_STOP

bool AudioServer::HandleCheckRecorderBackgroundCapture(const AudioProcessConfig &config)
{
    if (!PermissionUtil::NeedVerifyBackgroundCapture(config.callerUid, config.capturerInfo.sourceType)) {
        // no need to check
        return true;
    }

    AppInfo appInfo = config.appInfo;
    if (PermissionUtil::VerifyBackgroundCapture(appInfo.appTokenId, appInfo.appFullTokenId)) {
        // check success
        return true;
    }

    SwitchStreamInfo info = {
        config.originalSessionId,
        config.callerUid,
        config.appInfo.appUid,
        config.appInfo.appPid,
        config.appInfo.appTokenId,
        CAPTURER_PREPARED,
    };
    if (SwitchStreamUtil::IsSwitchStreamSwitching(info, SWITCH_STATE_CREATED)) {
        AUDIO_INFO_LOG("switchStream is recreating, callerUid:%{public}d", config.callerUid);
        SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_CREATED);
        return true;
    }

    std::string bundleName = AppBundleManager::GetBundleNameFromUid(config.appInfo.appUid);
    if (AudioService::GetInstance()->MatchForegroundList(bundleName, config.appInfo.appUid) &&
        Util::IsBackgroundSourceType(config.capturerInfo.sourceType)) {
        AudioService::GetInstance()->UpdateForegroundState(config.appInfo.appTokenId, true);
        bool res = PermissionUtil::VerifyBackgroundCapture(appInfo.appTokenId, appInfo.appFullTokenId);
        AUDIO_INFO_LOG("Retry for %{public}s, result:%{public}s", bundleName.c_str(), (res ? "success" : "fail"));
        AudioService::GetInstance()->UpdateForegroundState(config.appInfo.appTokenId, false);
        return res;
    }

    AUDIO_WARNING_LOG("failed for %{public}s", bundleName.c_str());
    return false;
}

int32_t AudioServer::SetForegroundList(const std::vector<std::string> &list)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d",
        IPCSkeleton::GetCallingUid());
    AudioService::GetInstance()->SaveForegroundList(list);
    return SUCCESS;
}

int32_t AudioServer::SetRenderWhitelist(const std::vector<std::string> &list)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d",
        IPCSkeleton::GetCallingUid());
    AudioService::GetInstance()->SaveRenderWhitelist(list);
    return SUCCESS;
}

int32_t AudioServer::GetVolumeBySessionId(uint32_t sessionId, float &volume)
{
    bool result = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(result, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return AudioStreamMonitor::GetInstance().GetVolumeBySessionId(sessionId, volume);
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

int32_t AudioServer::CreatePlaybackCapturerManager(bool &isSuccess)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        isSuccess = false;
        return ERR_PERMISSION_DENIED;
    }
    std::vector<int32_t> usage;
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    playbackCapturerMgr->SetSupportStreamUsage(usage);
    isSuccess = true;
    return SUCCESS;
#else
    isSuccess = false;
    return ERR_NOT_SUPPORTED;
#endif
}
// LCOV_EXCL_STOP

void AudioServer::RegisterAudioCapturerSourceCallback()
{
    IdHandler &idHandler = IdHandler::GetInstance();
    std::function<bool(uint32_t)> limitFunc = [&idHandler] (uint32_t id) -> bool {
        return idHandler.ParseType(id) == HDI_ID_TYPE_WAKEUP && idHandler.ParseInfo(id) == "Built_in_wakeup";
    };
    HdiAdapterManager::GetInstance().RegistSourceCallback(HDI_CB_CAPTURE_WAKEUP, this, limitFunc);

    limitFunc = [&idHandler] (uint32_t id) -> bool {
        uint32_t type = idHandler.ParseType(id);
        std::string info = idHandler.ParseInfo(id);
        if (type == HDI_ID_TYPE_PRIMARY) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_USB;
        }
#ifdef SUPPORT_LOW_LATENCY
        if (type == HDI_ID_TYPE_FAST) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_VOIP;
        }
#endif
        if (type == HDI_ID_TYPE_BLUETOOTH) {
            return info == HDI_ID_INFO_DEFAULT;
        }

        if (type == HDI_ID_TYPE_AI) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        return false;
    };
    std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> callbackGenerator = [this](uint32_t captureId) ->
        std::shared_ptr<IAudioSourceCallback> {
        return std::make_shared<CapturerStateOb>(captureId,
            [this] (bool isActive, size_t preNum, size_t curNum) {
                this->OnCapturerState(isActive, preNum, curNum);
            }
        );
    };
    HdiAdapterManager::GetInstance().RegistSourceCallbackGenerator(HDI_CB_CAPTURE_STATE, callbackGenerator, limitFunc);
}

void AudioServer::RegisterAudioRendererSinkCallback()
{
    // Only watch primary and fast sink for now, watch other sinks later.
    IdHandler &idHandler = IdHandler::GetInstance();
    std::function<bool(uint32_t)> limitFunc = [&idHandler] (uint32_t id) -> bool {
        uint32_t type = idHandler.ParseType(id);
        std::string info = idHandler.ParseInfo(id);
        if (type == HDI_ID_TYPE_PRIMARY) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_USB ||
                info == HDI_ID_INFO_DIRECT || info == HDI_ID_INFO_DP ||
                info == HDI_ID_INFO_VOIP;
        }
        if (type == HDI_ID_TYPE_OFFLOAD) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        if (type == HDI_ID_TYPE_MULTICHANNEL) {
            return info == HDI_ID_INFO_DEFAULT;
        }
        if (type == HDI_ID_TYPE_BLUETOOTH) {
#ifdef SUPPORT_LOW_LATENCY
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_MMAP;
#else
            return info == HDI_ID_INFO_DEFAULT;
#endif
        }
#ifdef SUPPORT_LOW_LATENCY
        if (type == HDI_ID_TYPE_FAST) {
            return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_VOIP;
        }
#endif
        return false;
    };
    HdiAdapterManager::GetInstance().RegistSinkCallback(HDI_CB_RENDER_STATE, this, limitFunc);
}

int32_t AudioServer::NotifyStreamVolumeChanged(int32_t streamType, float volume)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("NotifyStreamVolumeChanged refused for %{public}d", callingUid);
        return ERR_NOT_SUPPORTED;
    }
    AudioStreamType streamTypeTmp = static_cast<AudioStreamType>(streamType);

    int32_t ret = AudioService::GetInstance()->NotifyStreamVolumeChanged(streamTypeTmp, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("NotifyStreamVolumeChanged failed");
    }
    ret = SetVolumeInfoForEnhanceChain(streamTypeTmp);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("SetVolumeInfoForEnhanceChain failed");
    }
    return SUCCESS;
}

int32_t AudioServer::ResetRouteForDisconnect(int32_t type)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("audioRendererSinkInstance is null!");
        return ERROR;
    }
    sink->ResetActiveDeviceForDisconnect(static_cast<DeviceType>(type));

    // todo reset capturer

    return SUCCESS;
}

int32_t AudioServer::GetMaxAmplitude(bool isOutputDevice, const std::string &deviceClass, int32_t sourceType,
    float &maxAmplitude)
{
    maxAmplitude = 0;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AUDIO_DEBUG_LOG("GetMaxAmplitude in audio server deviceClass %{public}s", deviceClass.c_str());
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "GetMaxAmplitude refused for %{public}d", callingUid);

    float fastMaxAmplitude = AudioService::GetInstance()->GetMaxAmplitude(isOutputDevice);
    std::shared_ptr<IAudioRenderSink> sink = nullptr;
    std::shared_ptr<IAudioCaptureSource> source = nullptr;
    if (isOutputDevice) {
        uint32_t renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass);
        sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId, false);
        if (sink != nullptr) {
            float normalMaxAmplitude = sink->GetMaxAmplitude();
            maxAmplitude = (normalMaxAmplitude > fastMaxAmplitude) ? normalMaxAmplitude : fastMaxAmplitude;
        }
    } else {
        uint32_t sourceId = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(deviceClass,
            static_cast<SourceType>(sourceType));
        source = HdiAdapterManager::GetInstance().GetCaptureSource(sourceId, false);
        if (source != nullptr) {
            float normalMaxAmplitude = source->GetMaxAmplitude();
            maxAmplitude = (normalMaxAmplitude > fastMaxAmplitude) ? normalMaxAmplitude : fastMaxAmplitude;
        }
    }

    return SUCCESS;
}

int32_t AudioServer::GetVolumeDataCount(const std::string &sinkName, int64_t &volumeDataCount)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "refused for %{public}d",
        IPCSkeleton::GetCallingUid());
    uint32_t renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(sinkName);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId, false);
    if (sink != nullptr) {
        volumeDataCount = sink->GetVolumeDataCount();
    } else {
        volumeDataCount = 0;
        AUDIO_WARNING_LOG("can not find: %{public}s", sinkName.c_str());
    }
    return SUCCESS;
}

int32_t AudioServer::UpdateLatencyTimestamp(const std::string &timestamp, bool isRenderer)
{
    std::string stringTimestamp = timestamp;
    if (isRenderer) {
        LatencyMonitor::GetInstance().UpdateClientTime(true, stringTimestamp);
    } else {
        LatencyMonitor::GetInstance().UpdateClientTime(false, stringTimestamp);
        LatencyMonitor::GetInstance().ShowTimestamp(false);
    }
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioServer::UpdateDualToneState(bool enable, int32_t sessionId, const std::string &dupSinkName)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    if (enable) {
        return AudioService::GetInstance()->EnableDualStream(static_cast<uint32_t>(sessionId), dupSinkName);
    } else {
        return AudioService::GetInstance()->DisableDualStream(static_cast<uint32_t>(sessionId));
    }
}
// LCOV_EXCL_STOP

int32_t AudioServer::SetSinkRenderEmpty(const std::string &devceClass, int32_t durationUs)
{
    if (durationUs <= 0) {
        return SUCCESS;
    }
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERROR, "has no valid sink");

    return sink->SetRenderEmpty(durationUs);
}

// LCOV_EXCL_START
int32_t AudioServer::SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED, "refused for %{public}d",
        callingUid);

    if (durationUs <= 0) {
        return SUCCESS;
    }

    uint32_t id = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(devceClass);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERROR, "has no valid sink");
    return sink->SetSinkMuteForSwitchDevice(mute);
}

int32_t AudioServer::UpdateSessionConnectionState(int32_t sessionId, int32_t state)
{
    AUDIO_INFO_LOG("Server get sessionID: %{public}d, state: %{public}d", sessionId, state);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Update session connection state refused for %{public}d", callingUid);
    std::shared_ptr<RendererInServer> renderer =
        AudioService::GetInstance()->GetRendererBySessionID(static_cast<uint32_t>(sessionId));

    if (renderer == nullptr) {
        AUDIO_ERR_LOG("No render in server has sessionID");
        return ERROR;
    }
    renderer->OnDataLinkConnectionUpdate(static_cast<IOperation>(state));
    std::shared_ptr<IAudioRenderSink> sink = GetSinkByProp(HDI_ID_TYPE_PRIMARY);
    CHECK_AND_RETURN_RET_LOG(sink, ERROR, "sink is nullptr");
    int32_t ret = sink->UpdatePrimaryConnectionState(state);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "sink do not support UpdatePrimaryConnectionState");
    return SUCCESS;
}

int32_t AudioServer::SetLatestMuteState(uint32_t sessionId, bool muteFlag)
{
    AUDIO_INFO_LOG("sessionId_: %{public}u, muteFlag: %{public}d", sessionId, muteFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->SetLatestMuteState(sessionId, muteFlag);
    return SUCCESS;
}

int32_t AudioServer::SetSessionMuteState(uint32_t sessionId, bool insert, bool muteFlag)
{
    AUDIO_INFO_LOG("sessionId_: %{public}u, muteFlag: %{public}d", sessionId, muteFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->SetSessionMuteState(sessionId, insert, muteFlag);
    return SUCCESS;
}

int32_t AudioServer::SetNonInterruptMute(uint32_t sessionId, bool muteFlag)
{
    AUDIO_INFO_LOG("sessionId_: %{public}u, muteFlag: %{public}d", sessionId, muteFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->SetNonInterruptMute(sessionId, muteFlag);
    return SUCCESS;
}

int32_t AudioServer::RestoreSession(uint32_t sessionID, const RestoreInfoIpc &restoreInfoIpc)
{
    const RestoreInfo &restoreInfo = restoreInfoIpc.restoreInfo;
    AUDIO_INFO_LOG("restore session: %{public}u, reason: %{public}d, device change reason %{public}d, "
        "target flag %{public}d", sessionID, restoreInfo.restoreReason, restoreInfo.deviceChangeReason,
        restoreInfo.targetStreamFlag);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Update session connection state refused for %{public}d", callingUid);
    int32_t tryCount = RESTORE_SESSION_TRY_COUNT;
    RestoreStatus restoreStatus;
    while (tryCount > 0) {
        restoreStatus = AudioService::GetInstance()->RestoreSession(sessionID, restoreInfo);
        if (restoreStatus == NEED_RESTORE) {
            return SUCCESS;
        }
        if (restoreStatus == RESTORING) {
            AUDIO_WARNING_LOG("Session %{public}u is restoring, wait 50ms, tryCount %{public}d", sessionID, tryCount);
            usleep(RESTORE_SESSION_RETRY_WAIT_TIME_IN_MS); // Sleep for 50ms and try restore again.
        }
        tryCount--;
    }

    if (restoreStatus != NEED_RESTORE) {
        AUDIO_WARNING_LOG("Restore session in server failed, restore status %{public}d", restoreStatus);
    }
    return SUCCESS;
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
// LCOV_EXCL_STOP

void AudioServer::OnRenderSinkStateChange(uint32_t sinkId, bool started)
{
    AudioService::GetInstance()->UpdateAudioSinkState(sinkId, started);
    return;
}

int32_t AudioServer::CheckHibernateState(bool hibernate)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    AudioService::GetInstance()->CheckHibernateState(hibernate);
    return SUCCESS;
}

int32_t AudioServer::CreateIpcOfflineStream(int32_t &errorCode, sptr<IRemoteObject>& client)
{
    client = nullptr;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    sptr<OfflineStreamInServer> stream = OfflineStreamInServer::GetOfflineStream(errorCode);
    CHECK_AND_RETURN_RET_LOG(stream, ERROR, "Create IIpcOfflineStream failed.");
    client = stream->AsObject();
    return SUCCESS;
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
    CHECK_AND_RETURN_RET_LOG(GENERATE_SESSIONID_UID_SET.count(uid) == 1, ERROR, "uid is %{public}d, not mcu uid", uid);
    sessionId = CoreServiceHandler::GetInstance().GenerateSessionId();
    return SUCCESS;
}

int32_t AudioServer::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->GetAllSinkInputs(sinkInputs);
    return SUCCESS;
}

int32_t AudioServer::SetDefaultAdapterEnable(bool isEnable)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "Refused for %{public}d", callingUid);
    AudioService::GetInstance()->SetDefaultAdapterEnable(isEnable);
    return SUCCESS;
}

int32_t AudioServer::NotifyAudioPolicyReady()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    std::lock_guard lock(isAudioPolicyReadyMutex_);
    isAudioPolicyReady_ = true;
    isAudioPolicyReadyCv_.notify_all();
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioServer::CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
#if defined(AUDIO_BUILD_VARIANT_ROOT) && defined(HAS_FEATURE_INNERCAPTURER)
    // root user case for auto test
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid == ROOT_UID) {
        return InnerCheckCaptureLimit(config, innerCapId);
    }
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t AudioServer::SetInnerCapLimit(uint32_t innerCapLimit)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED,
        "refused for %{public}d", callingUid);
    PlaybackCapturerManager *playbackCapturerMgr = PlaybackCapturerManager::GetInstance();
    int32_t ret = playbackCapturerMgr->SetInnerCapLimit(innerCapLimit);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetInnerCapLimit error");
    }
    return ret;
#endif
    return ERR_NOT_SUPPORTED;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
int32_t AudioServer::ReleaseCaptureLimit(int32_t innerCapId)
{
#if defined(AUDIO_BUILD_VARIANT_ROOT) && defined(HAS_FEATURE_INNERCAPTURER)
    // root user case for auto test
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid == ROOT_UID) {
        PlaybackCapturerManager::GetInstance()->CheckReleaseUnloadModernInnerCapSink(innerCapId);
        PlaybackCapturerManager::GetInstance()->CheckReleaseUnloadModernOffloadCapSource();
        return SUCCESS;
    }
#endif
    return ERR_NOT_SUPPORTED;
}

int32_t AudioServer::LoadHdiAdapter(uint32_t devMgrType, const std::string &adapterName)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_NOT_SUPPORTED, "refused for %{public}d", callingUid);

    return HdiAdapterManager::GetInstance().LoadAdapter(static_cast<HdiDeviceManagerType>(devMgrType), adapterName);
}

int32_t AudioServer::UnloadHdiAdapter(uint32_t devMgrType, const std::string &adapterName, bool force)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    HdiAdapterManager::GetInstance().UnloadAdapter(static_cast<HdiDeviceManagerType>(devMgrType), adapterName, force);
    return SUCCESS;
}

int32_t AudioServer::CreateHdiSinkPort(const std::string &deviceClass, const std::string &idInfo,
    const IAudioSinkAttr &attr, uint32_t &renderId)
{
    renderId = HDI_INVALID_ID;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), SUCCESS,
        "refused for %{public}d", callingUid);

    renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, idInfo, true);
    CHECK_AND_RETURN_RET(renderId != HDI_INVALID_ID, SUCCESS);
    CHECK_AND_RETURN_RET(deviceClass != "Virtual_Injector", SUCCESS);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId, true);
    if (sink == nullptr) {
        HdiAdapterManager::GetInstance().ReleaseId(renderId);
        renderId = HDI_INVALID_ID;
        return SUCCESS;
    }
    if (!sink->IsInited()) {
        // preSet a2dpParam needs to guarantee that init() and SetA2dpAudioParameter() not called in concurrency.
        std::lock_guard<std::mutex> lock(setA2dpParamMutex_);
        sink->Init(attr);
    }
    return SUCCESS;
}

int32_t AudioServer::CreateSinkPort(uint32_t idBase, uint32_t idType, const std::string &idInfo,
    const IAudioSinkAttr &attr, uint32_t &renderId)
{
    renderId = HDI_INVALID_ID;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), SUCCESS,
        "refused for %{public}d", callingUid);

    AUDIO_INFO_LOG("In, idBase: %{public}u, idType: %{public}u, info: %{public}s", idBase, idType, idInfo.c_str());
    renderId = HdiAdapterManager::GetInstance().GetId(static_cast<HdiIdBase>(idBase),
        static_cast<HdiIdType>(idType), idInfo, true);
    CHECK_AND_RETURN_RET(renderId != HDI_INVALID_ID, SUCCESS);
    if (idInfo.find("InnerCapturerSink") != string::npos) {
        AUDIO_INFO_LOG("Inner-cap stream return");
        return SUCCESS;
    }

    // if stream is fast, create when endpoint config to reduce power
    if (idBase == HDI_ID_BASE_RENDER && (HDI_ID_TYPE_FAST == idType || HDI_ID_INFO_MMAP == idInfo)) {
        AUDIO_INFO_LOG("Fast stream delay create");
        return SUCCESS;
    }
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId, true);
    if (sink == nullptr) {
        AUDIO_WARNING_LOG("Sink is nullptr");
        HdiAdapterManager::GetInstance().ReleaseId(renderId);
        renderId = HDI_INVALID_ID;
        return SUCCESS;
    }
    if (!sink->IsInited()) {
        sink->Init(attr);
    }
    return SUCCESS;
}

int32_t AudioServer::CreateSourcePort(uint32_t idBase, uint32_t idType, const std::string &idInfo,
    const IAudioSourceAttr &attr, uint32_t &captureId)
{
    captureId = HDI_INVALID_ID;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), SUCCESS,
        "refused for %{public}d", callingUid);
    AUDIO_INFO_LOG("In, idBase: %{public}u, idType: %{public}u, info: %{public}s", idBase, idType, idInfo.c_str());
    captureId = HdiAdapterManager::GetInstance().GetId(static_cast<HdiIdBase>(idBase),
        static_cast<HdiIdType>(idType), idInfo, true);
    CHECK_AND_RETURN_RET(captureId != HDI_INVALID_ID, SUCCESS);

    // if stream is fast, create when endpoint config to reduce power
    if (idBase == HDI_ID_BASE_CAPTURE && HDI_ID_TYPE_FAST == idType) {
        AUDIO_INFO_LOG("Fast stream delay create");
        return SUCCESS;
    }
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(captureId, true);
    if (source == nullptr) {
        AUDIO_WARNING_LOG("Source is nullptr");
        HdiAdapterManager::GetInstance().ReleaseId(captureId);
        captureId = HDI_INVALID_ID;
        return SUCCESS;
    }
    if (!source->IsInited()) {
        source->Init(attr);
    }
    return SUCCESS;
}

int32_t AudioServer::CreateHdiSourcePort(const std::string &deviceClass, const std::string &idInfo,
    const IAudioSourceAttr &attr, uint32_t &captureId)
{
    captureId = HDI_INVALID_ID;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), SUCCESS,
        "refused for %{public}d", callingUid);

    captureId = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(deviceClass,
        static_cast<SourceType>(attr.sourceType), idInfo, true);
    CHECK_AND_RETURN_RET(captureId != HDI_INVALID_ID, SUCCESS);
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(captureId, true);
    if (source == nullptr) {
        AUDIO_WARNING_LOG("Source is nullptr");
        HdiAdapterManager::GetInstance().ReleaseId(captureId);
        captureId = HDI_INVALID_ID;
        return SUCCESS;
    }
    if (!source->IsInited()) {
        source->Init(attr);
    }
    return SUCCESS;
}

int32_t AudioServer::DestroyHdiPort(uint32_t id)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    HdiAdapterManager::GetInstance().ReleaseId(id);
    return SUCCESS;
}

int32_t AudioServer::SetDeviceConnectedFlag(bool flag)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);

    std::shared_ptr<IAudioRenderSink> primarySink = GetSinkByProp(HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    CHECK_AND_RETURN_RET_LOG(primarySink, ERROR, "primarySink is nullptr");
    primarySink->SetDeviceConnectedFlag(flag);
    return SUCCESS;
}

int32_t AudioServer::CreateAudioWorkgroup(const sptr<IRemoteObject> &object, int32_t &workgroupId)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_RET_LOG(AudioResourceService::GetInstance() != nullptr, ERROR, "AudioResourceService is nullptr");
    workgroupId = AudioResourceService::GetInstance()->CreateAudioWorkgroup(pid, object);
    return SUCCESS;
}

int32_t AudioServer::ReleaseAudioWorkgroup(int32_t workgroupId)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->ReleaseAudioWorkgroup(pid, workgroupId);
}

int32_t AudioServer::AddThreadToGroup(int32_t workgroupId, int32_t tokenId)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->AddThreadToGroup(pid, workgroupId, tokenId);
}

int32_t AudioServer::RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->RemoveThreadFromGroup(pid, workgroupId, tokenId);
}

int32_t AudioServer::StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->StartGroup(pid, workgroupId, startTime, deadlineTime);
}

int32_t AudioServer::StopGroup(int32_t workgroupId)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->StopGroup(pid, workgroupId);
}

int32_t AudioServer::SetBtHdiInvalidState()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_PERMISSION_DENIED,
        "refused for %{public}d", callingUid);
    auto limitFunc = [](uint32_t id) -> bool {
        std::string info = IdHandler::GetInstance().ParseInfo(id);
        if (IdHandler::GetInstance().ParseType(id) == HDI_ID_TYPE_BLUETOOTH &&
            IdHandler::GetInstance().ParseInfo(id) != HDI_ID_INFO_HEARING_AID) {
            return true;
        }
        return false;
    };
    auto sinkProcessFunc = [limitFunc](uint32_t renderId, std::shared_ptr<IAudioRenderSink> sink) -> int32_t {
        CHECK_AND_RETURN_RET(limitFunc(renderId), SUCCESS);
        CHECK_AND_RETURN_RET(sink != nullptr, SUCCESS);

        sink->SetInvalidState();
        return SUCCESS;
    };
    (void)HdiAdapterManager::GetInstance().ProcessSink(sinkProcessFunc);
    auto sourceProcessFunc = [limitFunc](uint32_t captureId, std::shared_ptr<IAudioCaptureSource> source) -> int32_t {
        CHECK_AND_RETURN_RET(limitFunc(captureId), SUCCESS);
        CHECK_AND_RETURN_RET(source != nullptr, SUCCESS);

        source->SetInvalidState();
        return SUCCESS;
    };
    (void)HdiAdapterManager::GetInstance().ProcessSource(sourceProcessFunc);
    return SUCCESS;
}

int32_t AudioServer::SetActiveOutputDevice(int32_t deviceType)
{
    CHECK_AND_RETURN_RET_LOG(deviceType >= DEVICE_TYPE_NONE && deviceType <= DEVICE_TYPE_MAX, AUDIO_ERR,
        "Set active output device failed, please check log");
    Trace trace("AudioServer::SetActiveOutputDevice:" + std::to_string(deviceType));
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_PERMISSION_DENIED;
    }

    PolicyHandler::GetInstance().SetActiveOutputDevice(static_cast<DeviceType>(deviceType));
    return SUCCESS;
}

int32_t AudioServer::ForceStopAudioStream(int32_t audioType)
{
    CHECK_AND_RETURN_RET_LOG(audioType >= STOP_ALL && audioType <= STOP_RECORD,
        ERR_INVALID_PARAM, "Invalid audioType");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_SYSTEM_PERMISSION_DENIED, "not audio calling!");
    CHECK_AND_RETURN_RET_LOG(AudioService::GetInstance() != nullptr, ERR_INVALID_OPERATION, "AudioService is nullptr");
    return AudioService::GetInstance()->ForceStopAudioStream(static_cast<StopAudioType>(audioType));
}

int32_t AudioServer::ImproveAudioWorkgroupPrio(const std::unordered_map<int32_t, bool> &threads)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->ImproveAudioWorkgroupPrio(pid, threads);
}

int32_t AudioServer::RestoreAudioWorkgroupPrio(const std::unordered_map<int32_t, int32_t> &threads)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    return AudioResourceService::GetInstance()->RestoreAudioWorkgroupPrio(pid, threads);
}

int32_t AudioServer::GetPrivacyTypeAudioServer(uint32_t sessionId, int32_t &privacyType, int32_t &ret)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_SYSTEM_PERMISSION_DENIED, "not audio calling!");
    AudioPrivacyType type = PRIVACY_TYPE_PUBLIC;
    ret = AudioService::GetInstance()->GetPrivacyType(sessionId, type);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, SUCCESS, "%{public}u err", sessionId);
    privacyType = static_cast<int32_t>(type);
    return SUCCESS;
}

int32_t AudioServer::AddCaptureInjector(uint32_t sinkPortidx, std::string &rate, std::string &format,
    std::string &channels, std::string &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_SYSTEM_PERMISSION_DENIED, "not audio calling!");
    int32_t ret = ERROR; //if is not low latency, should return error
#ifdef SUPPORT_LOW_LATENCY
    auto ptr = AudioService::GetInstance()->GetEndPointByType(AudioEndpoint::EndpointType::TYPE_VOIP_MMAP);
    CHECK_AND_RETURN_RET_LOG(ptr != nullptr, ERROR, "endpoint not exist!");
    ret = ptr->AddCaptureInjector(sinkPortidx, SOURCE_TYPE_VOICE_COMMUNICATION);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "add injector fail!");
    AudioModuleInfo &info = AudioInjectorService::GetInstance().GetModuleInfo();
    rate = info.rate;
    format = info.format;
    channels = info.channels;
    bufferSize = info.bufferSize;
#endif
    return ret;
}

int32_t AudioServer::RemoveCaptureInjector(uint32_t sinkPortidx)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyIsAudio(), ERR_SYSTEM_PERMISSION_DENIED, "not audio calling!");
    int32_t ret = ERROR; //if is not low latency, should return error
#ifdef SUPPORT_LOW_LATENCY
    auto ptr = AudioService::GetInstance()->GetEndPointByType(AudioEndpoint::EndpointType::TYPE_VOIP_MMAP);
    CHECK_AND_RETURN_RET_LOG(ptr != nullptr, ERROR, "endpoint not exist!");
    ret = ptr->RemoveCaptureInjector(sinkPortidx, SOURCE_TYPE_VOICE_COMMUNICATION);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "remove injector fail!");
#endif
    return ret;
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
