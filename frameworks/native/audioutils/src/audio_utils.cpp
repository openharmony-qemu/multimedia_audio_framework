/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioUtils"
#endif

#include "audio_utils.h"
#include <cinttypes>
#include <ctime>
#include <sstream>
#include <ostream>
#include <climits>
#include <thread>
#include <string>
#include "audio_utils_c.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#ifdef FEATURE_HITRACE_METER
#include "hitrace_meter.h"
#endif
#include "bundle_mgr_interface.h"
#include "parameter.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include "securec.h"
#include "privacy_error.h"

using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t YEAR_BASE = 1900;
const size_t MOCK_INTERVAL = 2000;
const int32_t DETECTED_ZERO_THRESHOLD = 1;
const int32_t BLANK_THRESHOLD_MS = 100;
const int32_t SIGNAL_THRESHOLD = 10;
const uint32_t MAX_VALUE_OF_SIGNED_24_BIT = 8388607;
const int64_t PCM_MAYBE_SILENT = 1;
const int64_t PCM_MAYBE_NOT_SILENT = 5;
const int32_t SIGNAL_DATA_SIZE = 96;
const int32_t DECIMAL_EXPONENT = 10;
const size_t DATE_LENGTH = 17;
static uint32_t g_sessionToMock = 0;
constexpr int32_t UID_AUDIO = 1041;
constexpr int32_t UID_MSDP_SA = 6699;
constexpr int32_t UID_INTELLIGENT_VOICE_SA = 1042;
constexpr int32_t UID_CAAS_SA = 5527;
constexpr int32_t UID_DISTRIBUTED_AUDIO_SA = 3055;
constexpr int32_t UID_TELEPHONY_SA = 1001;
constexpr int32_t UID_THPEXTRA_SA = 5000;
constexpr int32_t TIME_OUT_SECONDS = 10;
constexpr int32_t BOOTUP_MUSIC_UID = 1003;

const uint32_t UNIQUE_ID_INTERVAL = 8;

constexpr size_t FIRST_CHAR = 1;
constexpr size_t MIN_LEN = 8;
constexpr size_t HEAD_STR_LEN = 2;
constexpr size_t TAIL_STR_LEN = 5;

const int32_t DATA_INDEX_0 = 0;
const int32_t DATA_INDEX_1 = 1;
const int32_t DATA_INDEX_2 = 2;
const int32_t DATA_INDEX_3 = 3;
const int32_t DATA_INDEX_4 = 4;
const int32_t DATA_INDEX_5 = 5;
const int32_t STEREO_CHANNEL_COUNT = 2;
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;

const char* DUMP_PULSE_DIR = "/data/data/.pulse_dir/";
const char* DUMP_SERVICE_DIR = "/data/local/tmp/";
const char* DUMP_APP_DIR = "/data/storage/el2/base/cache/";


const std::set<int32_t> RECORD_ALLOW_BACKGROUND_LIST = {
#ifdef AUDIO_BUILD_VARIANT_ROOT
    0, // UID_ROOT
#endif
    UID_MSDP_SA,
    UID_INTELLIGENT_VOICE_SA,
    UID_CAAS_SA,
    UID_DISTRIBUTED_AUDIO_SA,
    UID_THPEXTRA_SA,
    UID_TELEPHONY_SA // used in distributed communication call
};

const std::set<SourceType> NO_BACKGROUND_CHECK_SOURCE_TYPE = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_REMOTE_CAST
};
} // namespace

static std::unordered_map<AudioStreamType, std::string> STREAM_TYPE_NAME_MAP = {
    {STREAM_VOICE_ASSISTANT, "VOICE_ASSISTANT"},
    {STREAM_VOICE_CALL, "VOICE_CALL"},
    {STREAM_SYSTEM, "SYSTEM"},
    {STREAM_RING, "RING"},
    {STREAM_MUSIC, "MUSIC"},
    {STREAM_ALARM, "ALARM"},
    {STREAM_NOTIFICATION, "NOTIFICATION"},
    {STREAM_BLUETOOTH_SCO, "BLUETOOTH_SCO"},
    {STREAM_DTMF, "DTMF"},
    {STREAM_TTS, "TTS"},
    {STREAM_ACCESSIBILITY, "ACCESSIBILITY"},
    {STREAM_ULTRASONIC, "ULTRASONIC"},
    {STREAM_WAKEUP, "WAKEUP"},
    {STREAM_CAMCORDER, "CAMCORDER"},
    {STREAM_ENFORCED_AUDIBLE, "ENFORCED_AUDIBLE"},
    {STREAM_MOVIE, "MOVIE"},
    {STREAM_GAME, "GAME"},
    {STREAM_SPEECH, "SPEECH"},
    {STREAM_SYSTEM_ENFORCED, "SYSTEM_ENFORCED"},
    {STREAM_VOICE_MESSAGE, "VOICE_MESSAGE"},
    {STREAM_NAVIGATION, "NAVIGATION"},
    {STREAM_INTERNAL_FORCE_STOP, "INTERNAL_FORCE_STOP"},
    {STREAM_SOURCE_VOICE_CALL, "SOURCE_VOICE_CALL"},
    {STREAM_VOICE_COMMUNICATION, "VOICE_COMMUNICATION"},
    {STREAM_VOICE_RING, "VOICE_RING"},
    {STREAM_VOICE_CALL_ASSISTANT, "VOICE_CALL_ASSISTANT"},
};

uint32_t Util::GetSamplePerFrame(const AudioSampleFormat &format)
{
    uint32_t audioPerSampleLength = 2; // 2 byte
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
            audioPerSampleLength = 1;
            break;
        case AudioSampleFormat::SAMPLE_S16LE:
            audioPerSampleLength = 2; // 2 byte
            break;
        case AudioSampleFormat::SAMPLE_S24LE:
            audioPerSampleLength = 3; // 3 byte
            break;
        case AudioSampleFormat::SAMPLE_S32LE:
        case AudioSampleFormat::SAMPLE_F32LE:
            audioPerSampleLength = 4; // 4 byte
            break;
        default:
            break;
    }
    return audioPerSampleLength;
}

bool Util::IsScoSupportSource(const SourceType sourceType)
{
    return sourceType == SOURCE_TYPE_VOICE_RECOGNITION || sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION;
}

bool Util::IsDualToneStreamType(const AudioStreamType streamType)
{
    return streamType == STREAM_RING || streamType == STREAM_VOICE_RING || streamType == STREAM_ALARM;
}

bool Util::IsRingerOrAlarmerStreamUsage(const StreamUsage &usage)
{
    return usage == STREAM_USAGE_ALARM || usage == STREAM_USAGE_VOICE_RINGTONE || usage == STREAM_USAGE_RINGTONE;
}

bool Util::IsRingerAudioScene(const AudioScene &audioScene)
{
    return audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING;
}

WatchTimeout::WatchTimeout(const std::string &funcName, int64_t timeoutNs) : funcName_(funcName), timeoutNs_(timeoutNs)
{
    startTimeNs_ = ClockTime::GetCurNano();
}

WatchTimeout::~WatchTimeout()
{
    if (!isChecked_) {
        CheckCurrTimeout();
    }
}

void WatchTimeout::CheckCurrTimeout()
{
    int64_t cost = ClockTime::GetCurNano() - startTimeNs_;
    if (cost > timeoutNs_) {
        AUDIO_WARNING_LOG("[%{public}s] cost %{public}" PRId64"ms!", funcName_.c_str(), cost / AUDIO_US_PER_SECOND);
    }
    isChecked_ = true;
}

bool CheckoutSystemAppUtil::CheckoutSystemApp(int32_t uid)
{
    if (uid == BOOTUP_MUSIC_UID) {
        // boot animation must be system app, no need query from BMS, to redeuce boot latency.
        AUDIO_INFO_LOG("boot animation must be system app, no need query from BMS.");
        return true;
    }
    bool isSystemApp = false;
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():CheckoutSystemApp");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, false, "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, false, "remoteObject is nullptr");
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, false, "bundleMgrProxy is nullptr");
    WatchTimeout reguard("bundleMgrProxy->CheckIsSystemAppByUid:CheckoutSystemApp");
    isSystemApp = bundleMgrProxy->CheckIsSystemAppByUid(uid);
    reguard.CheckCurrTimeout();
    return isSystemApp;
}

int64_t ClockTime::GetCurNano()
{
    int64_t result = -1; // -1 for bad result.
    struct timespec time;
    clockid_t clockId = CLOCK_MONOTONIC;
    int ret = clock_gettime(clockId, &time);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, result,
        "GetCurNanoTime fail, result:%{public}d", ret);
    result = (time.tv_sec * AUDIO_NS_PER_SECOND) + time.tv_nsec;
    return result;
}

int64_t ClockTime::GetRealNano()
{
    int64_t result = -1; // -1 for bad result
    struct timespec time;
    clockid_t clockId = CLOCK_REALTIME;
    int ret = clock_gettime(clockId, &time);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, result,
        "GetRealNanotime fail, result:%{public}d", ret);
    result = (time.tv_sec * AUDIO_NS_PER_SECOND) + time.tv_nsec;
    return result;
}

int32_t ClockTime::AbsoluteSleep(int64_t nanoTime)
{
    int32_t ret = -1; // -1 for bad result.
    CHECK_AND_RETURN_RET_LOG(nanoTime > 0, ret,
        "AbsoluteSleep invalid sleep time :%{public}" PRId64 " ns", nanoTime);
    struct timespec time;
    time.tv_sec = nanoTime / AUDIO_NS_PER_SECOND;
    time.tv_nsec = nanoTime - (time.tv_sec * AUDIO_NS_PER_SECOND); // Avoids % operation.

    clockid_t clockId = CLOCK_MONOTONIC;
    ret = clock_nanosleep(clockId, TIMER_ABSTIME, &time, nullptr);
    if (ret != 0) {
        AUDIO_WARNING_LOG("AbsoluteSleep may failed, ret is :%{public}d", ret);
    }

    return ret;
}

std::string ClockTime::NanoTimeToString(int64_t nanoTime)
{
    struct tm *tm_info;
    char buffer[80];
    time_t time_seconds = nanoTime / AUDIO_NS_PER_SECOND;

    tm_info = localtime(&time_seconds);
    if (tm_info == NULL) {
        AUDIO_ERR_LOG("get localtime failed!");
        return "";
    }

    size_t res = strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    CHECK_AND_RETURN_RET_LOG(res != 0, "", "strftime failed!");
    return std::string(buffer);
}

int32_t ClockTime::RelativeSleep(int64_t nanoTime)
{
    int32_t ret = -1; // -1 for bad result.
    CHECK_AND_RETURN_RET_LOG(nanoTime > 0, ret,
        "AbsoluteSleep invalid sleep time :%{public}" PRId64 " ns", nanoTime);
    struct timespec time;
    time.tv_sec = nanoTime / AUDIO_NS_PER_SECOND;
    time.tv_nsec = nanoTime - (time.tv_sec * AUDIO_NS_PER_SECOND); // Avoids % operation.

    clockid_t clockId = CLOCK_MONOTONIC;
    const int relativeFlag = 0; // flag of relative sleep.
    ret = clock_nanosleep(clockId, relativeFlag, &time, nullptr);
    if (ret != 0) {
        AUDIO_WARNING_LOG("RelativeSleep may failed, ret is :%{public}d", ret);
    }

    return ret;
}

void Trace::Count(const std::string &value, int64_t count)
{
#ifdef FEATURE_HITRACE_METER
    CountTrace(HITRACE_TAG_ZAUDIO, value, count);
#endif
}

void Trace::CountVolume(const std::string &value, uint8_t data)
{
#ifdef FEATURE_HITRACE_METER
    if (data == 0) {
        CountTrace(HITRACE_TAG_ZAUDIO, value, PCM_MAYBE_SILENT);
    } else {
        CountTrace(HITRACE_TAG_ZAUDIO, value, PCM_MAYBE_NOT_SILENT);
    }
#endif
}

Trace::Trace(const std::string &value)
{
    value_ = value;
    isFinished_ = false;
#ifdef FEATURE_HITRACE_METER
    StartTrace(HITRACE_TAG_ZAUDIO, value_);
#endif
}

void Trace::End()
{
#ifdef FEATURE_HITRACE_METER
    if (!isFinished_) {
        FinishTrace(HITRACE_TAG_ZAUDIO);
        isFinished_ = true;
    }
#endif
}

Trace::~Trace()
{
    End();
}

AudioXCollie::AudioXCollie(const std::string &tag, uint32_t timeoutSeconds,
    std::function<void(void *)> func, void *arg, uint32_t flag)
{
    AUDIO_DEBUG_LOG("Start AudioXCollie, tag: %{public}s, timeoutSeconds: %{public}u, flag: %{public}u",
        tag.c_str(), timeoutSeconds, flag);
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(tag, timeoutSeconds, func, arg, flag);
    tag_ = tag;
    isCanceled_ = false;
}

AudioXCollie::~AudioXCollie()
{
    CancelXCollieTimer();
}

void AudioXCollie::CancelXCollieTimer()
{
    if (!isCanceled_) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        isCanceled_ = true;
        AUDIO_DEBUG_LOG("CancelXCollieTimer: cancel timer %{public}s", tag_.c_str());
    }
}

bool PermissionUtil::VerifyIsShell()
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenTypeFlag == Security::AccessToken::TOKEN_SHELL) {
        return true;
    }
    return false;
}

bool PermissionUtil::VerifyIsAudio()
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (UID_AUDIO == callingUid) {
        return true;
    }
#ifdef AUDIO_BUILD_VARIANT_ROOT
    if (callingUid == 0) {
        AUDIO_WARNING_LOG("Root calling!");
        return true;
    }
#endif
    return false;
}

bool PermissionUtil::VerifyIsSystemApp()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    bool tmp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_PRERELEASE_LOGE("Check system app permission reject");
    return false;
}

bool PermissionUtil::VerifySelfPermission()
{
    Security::AccessToken::FullTokenID selfToken = IPCSkeleton::GetSelfTokenID();

    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(static_cast<uint32_t>(selfToken));

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_NATIVE, true);

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_SHELL, true);

    bool tmp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken);
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_ERR_LOG("Check self app permission reject");
    return false;
}

bool PermissionUtil::VerifySystemPermission()
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_NATIVE, true);
#ifdef AUDIO_BUILD_VARIANT_ROOT
    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_SHELL, true);
#endif
    bool tmp = VerifyIsSystemApp();
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_PRERELEASE_LOGE("Check system permission reject");
    return false;
}

bool PermissionUtil::VerifyPermission(const std::string &permissionName, uint32_t tokenId)
{
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, "Permission denied [%{public}s]", permissionName.c_str());

    return true;
}

bool PermissionUtil::NeedVerifyBackgroundCapture(int32_t callingUid, SourceType sourceType)
{
    if (RECORD_ALLOW_BACKGROUND_LIST.count(callingUid)) {
        AUDIO_INFO_LOG("internal sa(%{public}d) user directly recording", callingUid);
        return false;
    }
    if (NO_BACKGROUND_CHECK_SOURCE_TYPE.count(sourceType)) {
        AUDIO_INFO_LOG("sourceType %{public}d", sourceType);
        return false;
    }
    return true;
}

bool PermissionUtil::VerifyBackgroundCapture(uint32_t tokenId, uint64_t fullTokenId)
{
    Trace trace("PermissionUtil::VerifyBackgroundCapture");
    if (Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        AUDIO_INFO_LOG("system app recording");
        return true;
    }

    bool ret = Security::AccessToken::PrivacyKit::IsAllowedUsingPermission(tokenId, MICROPHONE_PERMISSION);
    if (!ret) {
        AUDIO_ERR_LOG("failed: not allowed!");
    }
    return ret;
}

std::mutex g_switchMapMutex;
static std::map<SwitchStreamInfo, SwitchState> g_switchStreamRecordMap = {};

bool SwitchStreamUtil::IsSwitchStreamSwitching(SwitchStreamInfo &info, SwitchState targetState)
{
    std::lock_guard<std::mutex> lock(g_switchMapMutex);
    auto iter = g_switchStreamRecordMap.find(info);
    if (iter != g_switchStreamRecordMap.end() && targetState == SWITCH_STATE_CREATED &&
        iter->second == SWITCH_STATE_WAITING && (info.nextState == CAPTURER_PREPARED)) {
        AUDIO_INFO_LOG("stream:%{public}u is recreating , need not check using mic in background !",
            info.sessionId);
        return true;
    }
    if (iter != g_switchStreamRecordMap.end() && targetState == SWITCH_STATE_STARTED &&
        iter->second == SWITCH_STATE_CREATED && (info.nextState == CAPTURER_RUNNING)) {
        AUDIO_INFO_LOG("stream:%{public}u is restarting , need not check using mic in background !",
            info.sessionId);
        return true;
    }
    return false;
}

bool SwitchStreamUtil::InsertSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    if (RECORD_ALLOW_BACKGROUND_LIST.count(info.callerUid)) {
        AUDIO_INFO_LOG("internal sa(%{public}d) user directly recording", info.callerUid);
        return true;
    }
    auto ret = g_switchStreamRecordMap.insert(std::make_pair(info, targetState));
    CHECK_AND_RETURN_RET_LOG(ret.second, false, "Update Record switchState:%{public}d for stream:%{public}u failed",
        targetState, info.sessionId);
    AUDIO_INFO_LOG("SwitchStream will start!Update Record switchState:%{public}d for stream:%{public}u"
        "uid:%{public}d CapturerState:%{public}d success", targetState, info.sessionId, info.appUid, info.nextState);
    std::thread(TimeoutThreadHandleTimeoutRecord, info, targetState).detach();
    return true;
}

void SwitchStreamUtil::TimeoutThreadHandleTimeoutRecord(SwitchStreamInfo info, SwitchState targetState)
{
    const std::chrono::seconds TIMEOUT_DURATION(2);
    AUDIO_INFO_LOG("Start timing. It will change to SWITCH_STATE_TIMEOUT after 2 seconds.");
    std::this_thread::sleep_for(TIMEOUT_DURATION);

    {
        std::lock_guard<std::mutex> lock(g_switchMapMutex);
        auto it = g_switchStreamRecordMap.find(info);
        if (it != g_switchStreamRecordMap.end()) {
            it->second = SWITCH_STATE_TIMEOUT;
            g_switchStreamRecordMap.erase(it);
            AUDIO_INFO_LOG("SwitchStream:%{public}u uid:%{public}d CapturerState:%{public}d was timeout! "
                "Update Record switchState:%{public}d success",
                info.sessionId, info.appUid, info.nextState, SWITCH_STATE_TIMEOUT);
        }
    }
}

//Remove switchStreamInfo from  switchStreamRecordMap must be called with g_switchMapMutex held
bool SwitchStreamUtil::RemoveSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    if (g_switchStreamRecordMap.count(info) != 0) {
        g_switchStreamRecordMap.erase(info);

        CHECK_AND_RETURN_RET((g_switchStreamRecordMap.count(info) == 0), false,
            "Remove exist record failed for stream:%{public}u", info.sessionId);
        AUDIO_WARNING_LOG("Exist Record has been Removed for stream:%{public}u", info.sessionId);
    }
    return true;
}

bool SwitchStreamUtil::RemoveAllRecordBySessionId(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(g_switchMapMutex);

    for (auto it = g_switchStreamRecordMap.begin(); it != g_switchStreamRecordMap.end();) {
        if (it->first.sessionId == sessionId) {
            it = g_switchStreamRecordMap.erase(it);
        } else {
            ++it;
        }
    }
    return true;
}

bool SwitchStreamUtil::UpdateSwitchStreamRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    std::lock_guard<std::mutex> lock(g_switchMapMutex);
    auto iter = g_switchStreamRecordMap.find(info);
    bool isInfoInRecord = (iter != g_switchStreamRecordMap.end());
    if (!isInfoInRecord) {
        if (targetState == SWITCH_STATE_WAITING) {
            CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState),
                false, "Insert SwitchStream into Record fail!");
            AUDIO_INFO_LOG("Insert SwitchStream into Record success!");
        }
        return true;
    }
        
    switch (targetState) {
        case SWITCH_STATE_WAITING:
            CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState),
                false, "Remove Error Record for Stream:%{public}u Failed!", iter->first.sessionId);
            CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::InsertSwitchStreamRecord(info, targetState),
                false, "Insert SwitchStream into Record fail!");
            break;
        case SWITCH_STATE_CREATED:
            CHECK_AND_RETURN_RET_LOG(HandleCreatedSwitchInfoInRecord(info, targetState), false,
                "Handle switch record to SWITCH_STATE_CREATED failed!");
            break;
        case SWITCH_STATE_STARTED:
            CHECK_AND_RETURN_RET_LOG(HandleStartedSwitchInfoInRecord(info, targetState), false,
                "Handle switch record to SWITCH_STATE_STARTED failed!");
            break;
        default:
            CHECK_AND_RETURN_RET_LOG(HandleSwitchInfoInRecord(info, targetState), false,
                "Handle switch info in record failed!");
            break;
    }
    if (iter->first.nextState == info.nextState) {
        g_switchStreamRecordMap[info] = SWITCH_STATE_FINISHED;
        g_switchStreamRecordMap.erase(info);
        AUDIO_INFO_LOG("SwitchStream will finish!Remove Record for stream:%{public}u uid:%{public}d ",
            info.sessionId, info.appUid);
    }
    if (iter->second == SWITCH_STATE_TIMEOUT || iter->second == SWITCH_STATE_FINISHED) {
        CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState), false,
            "Remove TIMEOUT or FINISHED Record for Stream:%{public}u Failed!", iter->first.sessionId);
        return false;
    }
    return true;
}

bool SwitchStreamUtil::HandleCreatedSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    auto iter = g_switchStreamRecordMap.find(info);
    if (iter->second == SWITCH_STATE_WAITING && (info.nextState == CAPTURER_PREPARED)) {
        g_switchStreamRecordMap[info] = targetState;
        AUDIO_INFO_LOG("SwitchStream will reCreated!Update Record switchState:%{public}d for"
            "stream:%{public}u uid:%{public}d streamState:%{public}d success",
            targetState, info.sessionId, info.appUid, info.nextState);
    } else {
        CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState),
            false, "Remove Error Record for Stream:%{public}u Failed!", iter->first.sessionId);
    }
    return true;
}

bool SwitchStreamUtil::HandleSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    auto iter = g_switchStreamRecordMap.find(info);
    if (((iter->second == SWITCH_STATE_CREATED) || (iter->second == SWITCH_STATE_STARTED)) &&
        (info.nextState == CAPTURER_STOPPED || info.nextState == CAPTURER_PAUSED ||
        info.nextState == CAPTURER_RELEASED || info.nextState == CAPTURER_INVALID)) {
        CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState),
            false, "Remove Finished Record for Stream:%{public}u Failed!", iter->first.sessionId);
    } else if ((iter->second == SWITCH_STATE_WAITING) && (info.nextState == CAPTURER_STOPPED ||
        info.nextState == CAPTURER_PAUSED || info.nextState == CAPTURER_RELEASED ||
        info.nextState == CAPTURER_INVALID)) {
        AUDIO_INFO_LOG("SwitchStream streamState has been changed to [%{public}d] before recreate!",
            info.nextState);
    } else {
        CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState),
            false, "Remove Error Record for Stream:%{public}u Failed!", iter->first.sessionId);
        AUDIO_INFO_LOG("Error Record has been Removed for stream:%{public}u", iter->first.sessionId);
    }
    return true;
}

bool SwitchStreamUtil::HandleStartedSwitchInfoInRecord(SwitchStreamInfo &info, SwitchState targetState)
{
    auto iter = g_switchStreamRecordMap.find(info);
    if ((iter->second == SWITCH_STATE_CREATED) && (info.nextState == CAPTURER_RUNNING)) {
        g_switchStreamRecordMap[info] = targetState;
        AUDIO_INFO_LOG("SwitchStream will reStarted!Update Record switchState:%{public}d for"
            "stream:%{public}u uid:%{public}d streamState:%{public}d success",
            targetState, info.sessionId, info.appUid, info.nextState);
    } else {
        CHECK_AND_RETURN_RET_LOG(SwitchStreamUtil::RemoveSwitchStreamRecord(info, targetState),
            false, "Remove Error Record for Stream:%{public}u Failed!", iter->first.sessionId);
    }
    return true;
}

std::mutex g_recordMapMutex;
std::map<std::uint32_t, std::set<uint32_t>> g_tokenIdRecordMap = {};

int32_t PermissionUtil::StartUsingPermission(uint32_t targetTokenId, const char* permission)
{
    Trace trace("PrivacyKit::StartUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StartUsingPermission tokenId:%{public}d permission:%{public}s",
        targetTokenId, permission);
    WatchTimeout guard("PrivacyKit::StartUsingPermission:PermissionUtil::StartUsingPermission");
    int32_t res = Security::AccessToken::PrivacyKit::StartUsingPermission(targetTokenId, permission);
    guard.CheckCurrTimeout();
    return res;
}

int32_t PermissionUtil::StopUsingPermission(uint32_t targetTokenId, const char* permission)
{
    Trace trace("PrivacyKit::StopUsingPermission");
    AUDIO_WARNING_LOG("PrivacyKit::StopUsingPermission tokenId:%{public}d permission:%{public}s",
        targetTokenId, permission);
    WatchTimeout guard("PrivacyKit::StopUsingPermission:PermissionUtil::StopUsingPermission");
    int32_t res = Security::AccessToken::PrivacyKit::StopUsingPermission(targetTokenId, permission);
    guard.CheckCurrTimeout();
    return res;
}

bool PermissionUtil::NotifyPrivacyStart(uint32_t targetTokenId, uint32_t sessionId)
{
    AudioXCollie audioXCollie("PermissionUtil::NotifyPrivacyStart", TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(g_recordMapMutex);
    if (g_tokenIdRecordMap.count(targetTokenId)) {
        if (!g_tokenIdRecordMap[targetTokenId].count(sessionId)) {
            g_tokenIdRecordMap[targetTokenId].emplace(sessionId);
        } else {
            AUDIO_WARNING_LOG("this stream %{public}u is already running, no need call start", sessionId);
        }
    } else {
        AUDIO_INFO_LOG("Notify PrivacyKit to display the microphone privacy indicator "
            "for tokenId: %{public}d sessionId:%{public}d", targetTokenId, sessionId);
        int32_t res = PermissionUtil::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(res == 0 || res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING, false,
            "StartUsingPermission for tokenId:%{public}u, PrivacyKit error code:%{public}d", targetTokenId, res);
        if (res == Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
            AUDIO_ERR_LOG("The PrivacyKit return ERR_PERMISSION_ALREADY_START_USING error code:%{public}d", res);
        }
        WatchTimeout reguard("Security::AccessToken::PrivacyKit::AddPermissionUsedRecord:NotifyPrivacyStart");
        res = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(targetTokenId, MICROPHONE_PERMISSION, 1, 0);
        reguard.CheckCurrTimeout();
        CHECK_AND_RETURN_RET_LOG(res == 0, false, "AddPermissionUsedRecord for tokenId %{public}u!"
            "The PrivacyKit error code:%{public}d", targetTokenId, res);
        g_tokenIdRecordMap[targetTokenId] = {sessionId};
    }
    return true;
}

bool PermissionUtil::NotifyPrivacyStop(uint32_t targetTokenId, uint32_t sessionId)
{
    AudioXCollie audioXCollie("PermissionUtil::NotifyPrivacyStop", TIME_OUT_SECONDS);
    std::unique_lock<std::mutex> lock(g_recordMapMutex);
    if (!g_tokenIdRecordMap.count(targetTokenId)) {
        AUDIO_INFO_LOG("this TokenId %{public}u is already not in using", targetTokenId);
        return true;
    }

    if (g_tokenIdRecordMap[targetTokenId].count(sessionId)) {
        g_tokenIdRecordMap[targetTokenId].erase(sessionId);
    }
    AUDIO_DEBUG_LOG("this TokenId %{public}u set size is %{public}zu!", targetTokenId,
        g_tokenIdRecordMap[targetTokenId].size());
    if (g_tokenIdRecordMap[targetTokenId].empty()) {
        g_tokenIdRecordMap.erase(targetTokenId);
        AUDIO_INFO_LOG("Notify PrivacyKit to remove the microphone privacy indicator "
            "for tokenId: %{public}d sessionId:%{public}d", targetTokenId, sessionId);
        int32_t res = PermissionUtil::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(res == 0, false, "StopUsingPermission for tokenId %{public}u!"
            "The PrivacyKit error code:%{public}d", targetTokenId, res);
    }
    return true;
}

void AdjustStereoToMonoForPCM8Bit(int8_t *data, uint64_t len)
{
    // the number 2: stereo audio has 2 channels
    uint64_t count = len / 2;

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustStereoToMonoForPCM16Bit(int16_t *data, uint64_t len)
{
    uint64_t count = len / 2 / 2;
    // first number 2: stereo audio has 2 channels
    // second number 2: the bit depth of PCM16Bit is 16 bits (2 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustStereoToMonoForPCM24Bit(uint8_t *data, uint64_t len)
{
    uint64_t count = len / STEREO_CHANNEL_COUNT / 3; // 3: the bit depth of PCM24Bit is 24 bits (3 bytes)

    while (count > 0) {
        uint32_t leftData = (static_cast<uint32_t>(data[DATA_INDEX_2]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_1]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_0]));
        uint32_t rightData = (static_cast<uint32_t>(data[DATA_INDEX_5]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_4]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_3]));

        leftData = static_cast<uint32_t>(static_cast<int32_t>(leftData << BIT_8) / STEREO_CHANNEL_COUNT +
            static_cast<int32_t>(rightData << BIT_8) / STEREO_CHANNEL_COUNT) >> BIT_8;
        rightData = leftData;

        data[DATA_INDEX_0] = static_cast<uint8_t>(leftData);
        data[DATA_INDEX_1] = static_cast<uint8_t>(leftData >> BIT_8);
        data[DATA_INDEX_2] = static_cast<uint8_t>(leftData >> BIT_16);
        data[DATA_INDEX_3] = static_cast<uint8_t>(rightData);
        data[DATA_INDEX_4] = static_cast<uint8_t>(rightData >> BIT_8);
        data[DATA_INDEX_5] = static_cast<uint8_t>(rightData >> BIT_16);
        data += 6; // 6: 2 channels, 24 bits (3 bytes), 2 * 3 = 6
        count--;
    }
}

void AdjustStereoToMonoForPCM32Bit(int32_t *data, uint64_t len)
{
    uint64_t count = len / 2 / 4;
    // first number 2: stereo audio has 2 channels
    // second number 4: the bit depth of PCM32Bit is 32 bits (4 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM8Bit(int8_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2;
    // the number 2: stereo audio has 2 channels

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM16Bit(int16_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2 / 2;
    // first number 2: stereo audio has 2 channels
    // second number 2: the bit depth of PCM16Bit is 16 bits (2 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM24Bit(uint8_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / STEREO_CHANNEL_COUNT / 3; // 3: the bit depth of PCM24Bit is 24 bits (3 bytes)

    while (count > 0) {
        uint32_t leftData = (static_cast<uint32_t>(data[DATA_INDEX_2]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_1]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_0]));
        int32_t leftTemp = static_cast<int32_t>(leftData << BIT_8);
        leftTemp *= left;
        leftData = static_cast<uint32_t>(leftTemp) >> BIT_8;
        data[DATA_INDEX_0] = static_cast<uint8_t>(leftData);
        data[DATA_INDEX_1] = static_cast<uint8_t>(leftData >> BIT_8);
        data[DATA_INDEX_2] = static_cast<uint8_t>(leftData >> BIT_16);

        uint32_t rightData = (static_cast<uint32_t>(data[DATA_INDEX_5]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_4]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_3]));
        int32_t rightTemp = static_cast<int32_t>(rightData << BIT_8);
        rightTemp *= right;
        rightData = static_cast<uint32_t>(rightTemp) >> BIT_8;
        data[DATA_INDEX_3] = static_cast<uint8_t>(rightData);
        data[DATA_INDEX_4] = static_cast<uint8_t>(rightData >> BIT_8);
        data[DATA_INDEX_5] = static_cast<uint8_t>(rightData >> BIT_16);
        data += 6; // 6: 2 channels, 24 bits (3 bytes), 2 * 3 = 6
        count--;
    }
}

void AdjustAudioBalanceForPCM32Bit(int32_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2 / 4;
    // first number 2: stereo audio has 2 channels
    // second number 4: the bit depth of PCM32Bit is 32 bits (4 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

uint32_t Read24Bit(const uint8_t *p)
{
    return ((uint32_t) p[BIT_DEPTH_TWO] << BIT_16) | ((uint32_t) p[1] << BIT_8) | ((uint32_t) p[0]);
}

void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = (uint8_t) (u >> BIT_16);
    p[1] = static_cast<uint8_t>(u >> BIT_8);
    p[0] = static_cast<uint8_t>(u);
}

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

float CapMax(float v)
{
    float value = v;
    if (v > 1.0f) {
        value = 1.0f - FLOAT_EPS;
    } else if (v < -1.0f) {
        value = -1.0f + FLOAT_EPS;
    }
    return value;
}

void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        Write24Bit(b, (static_cast<int32_t>(v)) >> BIT_8);
        b += OFFSET_BIT_24;
    }
}

void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        *(b++) = static_cast<int32_t>(v);
    }
}

float UpdateMaxAmplitude(ConvertHdiFormat adapterFormat, char *frame, uint64_t replyBytes)
{
    switch (adapterFormat) {
        case SAMPLE_U8_C: {
            return CalculateMaxAmplitudeForPCM8Bit(reinterpret_cast<int8_t *>(frame), replyBytes);
        }
        case SAMPLE_S16_C: {
            return CalculateMaxAmplitudeForPCM16Bit(reinterpret_cast<int16_t *>(frame),
                (replyBytes / sizeof(int16_t)));
        }
        case SAMPLE_S24_C: {
            return CalculateMaxAmplitudeForPCM24Bit(frame, (replyBytes / 3)); // 3 bytes
        }
        case SAMPLE_S32_C: {
            return CalculateMaxAmplitudeForPCM32Bit(reinterpret_cast<int32_t *>(frame),
                (replyBytes / sizeof(int32_t)));
        }
        default: {
            AUDIO_INFO_LOG("getMaxAmplitude: Unsupported audio format: %{public}d", adapterFormat);
            return 0;
        }
    }
}

float CalculateMaxAmplitudeForPCM8Bit(int8_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int8_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / SCHAR_MAX;
}

float CalculateMaxAmplitudeForPCM16Bit(int16_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int16_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / SHRT_MAX;
}

float CalculateMaxAmplitudeForPCM24Bit(char *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = 0; i < nSamples; ++i) {
        char *curPos = frame + (i * 3); // 3 bytes
        int curValue = 0;
        for (int j = 0; j < 3; ++j) { // 3 bytes
            curValue += (*(curPos + j) << (BIT_8 * j));
        }
        if (curValue < 0) {
            curValue = -curValue;
        }
        if (curMaxAmplitude < curValue) {
            curMaxAmplitude = curValue;
        }
    }
    return float(curMaxAmplitude) / MAX_VALUE_OF_SIGNED_24_BIT;
}

float CalculateMaxAmplitudeForPCM32Bit(int32_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int32_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / static_cast<float>(INT_MAX);
}

template <typename T>
bool StringConverter(const std::string &str, T &result)
{
    if (str == "-0") {
        result = 0;
        return true;
    }
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

template bool StringConverter(const std::string &str, uint64_t &result);
template bool StringConverter(const std::string &str, uint32_t &result);
template bool StringConverter(const std::string &str, int32_t &result);
template bool StringConverter(const std::string &str, uint16_t &result);
template bool StringConverter(const std::string &str, uint8_t &result);
template bool StringConverter(const std::string &str, int8_t &result);

bool StringConverterFloat(const std::string &str, float &result)
{
    char *end = nullptr;
    errno = 0;
    result = std::strtof(str.c_str(), &end);
    return end != str.c_str() && *end == '\0' && errno == 0;
}

bool SetSysPara(const std::string &key, int32_t value)
{
    auto res = SetParameter(key.c_str(), std::to_string(value).c_str());
    if (res < 0) {
        AUDIO_WARNING_LOG("SetSysPara fail, key:%{public}s res:%{public}d", key.c_str(), res);
        return false;
    }
    AUDIO_INFO_LOG("SetSysPara %{public}d success.", value);
    return true;
}

template <typename T>
bool GetSysPara(const char *key, T &value)
{
    CHECK_AND_RETURN_RET_LOG(key != nullptr, false, "key is nullptr");
    char paraValue[30] = {0}; // 30 for system parameter
    auto res = GetParameter(key, "-1", paraValue, sizeof(paraValue));

    CHECK_AND_RETURN_RET_LOG(res > 0, false, "GetSysPara fail, key:%{public}s res:%{public}d", key, res);
    AUDIO_DEBUG_LOG("key:%{public}s value:%{public}s", key, paraValue);
    std::stringstream valueStr;
    valueStr << paraValue;
    valueStr >> value;
    return true;
}

template bool GetSysPara(const char *key, int32_t &value);
template bool GetSysPara(const char *key, uint32_t &value);
template bool GetSysPara(const char *key, int64_t &value);
template bool GetSysPara(const char *key, std::string &value);

std::map<std::string, std::string> DumpFileUtil::g_lastPara = {};

FILE *DumpFileUtil::OpenDumpFileInner(std::string para, std::string fileName, AudioDumpFileType fileType)
{
    std::string filePath;
    switch (fileType) {
        case AUDIO_APP:
            filePath = DUMP_APP_DIR + fileName;
            break;
        case OTHER_NATIVE_SERVICE:
            filePath = DUMP_SERVICE_DIR + fileName;
            break;
        case AUDIO_PULSE:
            filePath = DUMP_PULSE_DIR + fileName;
            break;
        default:
            AUDIO_ERR_LOG("Invalid AudioDumpFileType");
            break;
    }
    std::string dumpPara;
    FILE *dumpFile = nullptr;
    bool res = GetSysPara(para.c_str(), dumpPara);
    if (!res || dumpPara.empty()) {
        AUDIO_INFO_LOG("%{public}s is not set, dump audio is not required", para.c_str());
        g_lastPara[para] = dumpPara;
        return dumpFile;
    }
    AUDIO_DEBUG_LOG("%{public}s = %{public}s", para.c_str(), dumpPara.c_str());
    if (dumpPara == "w") {
        dumpFile = fopen(filePath.c_str(), "wb+");
        CHECK_AND_RETURN_RET_LOG(dumpFile != nullptr, dumpFile,
            "Error opening pcm dump file:%{public}s", filePath.c_str());
    } else if (dumpPara == "a") {
        dumpFile = fopen(filePath.c_str(), "ab+");
        CHECK_AND_RETURN_RET_LOG(dumpFile != nullptr, dumpFile,
            "Error opening pcm dump file:%{public}s", filePath.c_str());
    }
    if (dumpFile != nullptr) {
        AUDIO_INFO_LOG("Dump file path: %{public}s", filePath.c_str());
    }
    g_lastPara[para] = dumpPara;
    return dumpFile;
}

void DumpFileUtil::WriteDumpFile(FILE *dumpFile, void *buffer, size_t bufferSize)
{
    if (dumpFile == nullptr) {
        return;
    }
    CHECK_AND_RETURN_LOG(buffer != nullptr, "Invalid write param");
    size_t writeResult = fwrite(buffer, 1, bufferSize, dumpFile);
    CHECK_AND_RETURN_LOG(writeResult == bufferSize, "Failed to write the file.");
}

void DumpFileUtil::CloseDumpFile(FILE **dumpFile)
{
    if (*dumpFile) {
        fclose(*dumpFile);
        *dumpFile = nullptr;
    }
}

void DumpFileUtil::ChangeDumpFileState(std::string para, FILE **dumpFile, std::string filePath)
{
    CHECK_AND_RETURN_LOG(*dumpFile != nullptr, "Invalid file para");
    CHECK_AND_RETURN_LOG(g_lastPara[para] == "w" || g_lastPara[para] == "a", "Invalid input para");
    std::string dumpPara;
    bool res = GetSysPara(para.c_str(), dumpPara);
    if (!res || dumpPara.empty()) {
        AUDIO_WARNING_LOG("get %{public}s fail", para.c_str());
    }
    if (g_lastPara[para] == "w" && dumpPara == "w") {
        return;
    }
    CloseDumpFile(dumpFile);
    OpenDumpFile(para, filePath, dumpFile);
}

void DumpFileUtil::OpenDumpFile(std::string para, std::string fileName, FILE **file)
{
    if (*file != nullptr) {
        DumpFileUtil::ChangeDumpFileState(para, file, fileName);
        return;
    }

    if (para == DUMP_SERVER_PARA) {
        *file = DumpFileUtil::OpenDumpFileInner(para, fileName, AUDIO_PULSE);
    } else {
        *file = DumpFileUtil::OpenDumpFileInner(para, fileName, AUDIO_APP);
        if (*file == nullptr) {
            *file = DumpFileUtil::OpenDumpFileInner(para, fileName, OTHER_NATIVE_SERVICE);
        }
    }
}

void CloseFd(int fd)
{
    // log stdin, stdout, stderr.
    if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        AUDIO_WARNING_LOG("special fd: %{public}d will be closed", fd);
    }
    int tmpFd = fd;
    close(fd);
    AUDIO_DEBUG_LOG("fd: %{public}d closed successfuly!", tmpFd);
}

static void MemcpyToI32FromI16(int16_t *src, int32_t *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        *(dst + i) = static_cast<int32_t>(*(src + i));
    }
}

static void MemcpyToI32FromI24(uint8_t *src, int32_t *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        uint8_t *tmp = src + 3 * i; // 3 is byte size of SAMPLE_S24LE;
        *(dst + i) = static_cast<int32_t>(tmp[2] << (2 * sizeof(uint8_t))) |
            static_cast<int32_t>(tmp[1] << sizeof(uint8_t)) | static_cast<int32_t>(tmp[0]);
    }
}

static void MemcpyToI32FromF32(float *src, int32_t *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        *(dst + i) = static_cast<int32_t>(*(src + i));
    }
}

bool NearZero(int16_t number)
{
    return number >= -DETECTED_ZERO_THRESHOLD && number <= DETECTED_ZERO_THRESHOLD;
}

std::string GetTime()
{
    std::string curTime;
    struct timeval tv;
    struct timezone tz;
    struct tm *t;
    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    if (t == nullptr) {
        return "";
    }

    curTime += std::to_string(YEAR_BASE + t->tm_year);
    curTime += (1 + t->tm_mon < DECIMAL_EXPONENT ? "0" + std::to_string(1 + t->tm_mon) :
        std::to_string(1 + t->tm_mon));
    curTime += (t->tm_mday < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_mday) :
        std::to_string(t->tm_mday));
    curTime += (t->tm_hour < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_hour) :
        std::to_string(t->tm_hour));
    curTime += (t->tm_min < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_min) :
        std::to_string(t->tm_min));
    curTime += (t->tm_sec < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_sec) :
        std::to_string(t->tm_sec));
    int64_t mSec = static_cast<int64_t>(tv.tv_usec / AUDIO_MS_PER_SECOND);
    curTime += (mSec < (DECIMAL_EXPONENT * DECIMAL_EXPONENT) ? (mSec < DECIMAL_EXPONENT ? "00" : "0") +
        std::to_string(mSec) : std::to_string(mSec));
    return curTime;
}

int32_t GetFormatByteSize(int32_t format)
{
    int32_t formatByteSize;
    switch (format) {
        case SAMPLE_S16LE:
            formatByteSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            formatByteSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            formatByteSize = 4; // size is 4
            break;
        case SAMPLE_F32LE:
            formatByteSize = 4; // size is 4
            break;
        default:
            formatByteSize = 2; // size is 2
            break;
    }
    return formatByteSize;
}

bool SignalDetectAgent::CheckAudioData(uint8_t *buffer, size_t bufferLen)
{
    CHECK_AND_RETURN_RET_LOG(formatByteSize_ != 0, false, "LatencyMeas checkAudioData failed, "
        "formatByteSize_ %{public}d", formatByteSize_);
    frameCountIgnoreChannel_ = bufferLen / static_cast<uint32_t>(formatByteSize_);
    if (cacheAudioData_.capacity() < frameCountIgnoreChannel_) {
        cacheAudioData_.clear();
        cacheAudioData_.reserve(frameCountIgnoreChannel_);
    }
    int32_t *cache = cacheAudioData_.data();
    if (sampleFormat_ == SAMPLE_S32LE) {
        int32_t ret = memcpy_s(cache, sizeof(int32_t) * cacheAudioData_.capacity(), buffer, bufferLen);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "LatencyMeas checkAudioData failed, dstSize "
            "%{public}zu, srcSize %{public}zu", sizeof(int32_t) * cacheAudioData_.capacity(), bufferLen);
    } else if (sampleFormat_ == SAMPLE_F32LE) {
        float *cp = reinterpret_cast<float*>(buffer);
        MemcpyToI32FromF32(cp, cache, frameCountIgnoreChannel_);
    } else if (sampleFormat_ == SAMPLE_S24LE) {
        MemcpyToI32FromI24(buffer, cache, frameCountIgnoreChannel_);
    } else {
        int16_t *cp = reinterpret_cast<int16_t*>(buffer);
        MemcpyToI32FromI16(cp, cache, frameCountIgnoreChannel_);
    }
    if (DetectSignalData(cache, frameCountIgnoreChannel_)) {
        ResetDetectResult();
        return true;
    }
    return false;
}

bool SignalDetectAgent::DetectSignalData(int32_t *buffer, size_t bufferLen)
{
    std::string curTime = GetTime();
    uint32_t rightZeroSignal = 0;
    int32_t currentPeakIndex = -1;
    int32_t currentPeakSignal = SHRT_MIN;
    bool hasNoneZero = false;
    size_t frameCount = bufferLen / static_cast<size_t>(channels_);
    for (size_t index = 0; index < frameCount; index++) {
        int32_t tempMax = SHRT_MIN;
        int32_t tempMin = SHRT_MAX;
        for (uint32_t channel = 0; channel < static_cast<uint32_t>(channels_); channel++) {
            int32_t temp = buffer[index * static_cast<uint32_t>(channels_) + channel];
            tempMax = temp > tempMax ? temp : tempMax;
            tempMin = temp < tempMin ? temp : tempMin;
        }
        if (!NearZero(tempMax) || !NearZero(tempMin)) {
            rightZeroSignal = index + 1;
            hasNoneZero = true;
            if (currentPeakIndex == -1 || tempMax > currentPeakSignal) {
                currentPeakIndex = static_cast<int32_t>(index);
                currentPeakSignal = tempMax;
            }
        }
    }
    if (!hasNoneZero) {
        blankPeriod_ += static_cast<int32_t>(frameCount);
    } else {
        if (!hasFirstNoneZero_) {
            lastPeakBufferTime_ = curTime;
            hasFirstNoneZero_ = true;
        }
        if (currentPeakSignal > lastPeakSignal_) {
            lastPeakSignal_ = currentPeakSignal;
            lastPeakSignalPos_ = currentPeakIndex;
        }
        blankHaveOutput_ = false;
        blankPeriod_ = static_cast<int32_t>(frameCount - rightZeroSignal);
    }
    int32_t thresholdBlankPeriod = BLANK_THRESHOLD_MS * sampleRate_ / MILLISECOND_PER_SECOND;
    if (blankPeriod_ > thresholdBlankPeriod) {
        return !blankHaveOutput_;
    }
    return false;
}

void SignalDetectAgent::ResetDetectResult()
{
    blankHaveOutput_ = true;
    hasFirstNoneZero_ = false;
    lastPeakSignal_ = SHRT_MIN;
    signalDetected_ = true;
    dspTimestampGot_ = false;
    return;
}

bool AudioLatencyMeasurement::MockPcmData(uint8_t *buffer, size_t bufferLen)
{
    memset_s(buffer, bufferLen, 0, bufferLen);
    int16_t *signal = signalData_.get();
    size_t newlyMocked = bufferLen * MILLISECOND_PER_SECOND /
        static_cast<size_t>(channelCount_ * sampleRate_ * formatByteSize_);
    mockedTime_ += newlyMocked;
    if (mockedTime_ >= MOCK_INTERVAL) {
        mockedTime_ = 0;
        if (format_ == SAMPLE_S32LE) {
            MemcpyToI32FromI16(signal, reinterpret_cast<int32_t*>(buffer), SIGNAL_DATA_SIZE);
        } else {
            int32_t ret = memcpy_s(buffer, bufferLen, signal, SIGNAL_DATA_SIZE * sizeof(uint8_t));
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "LatencyMeas mockPcmData failed, dstSize "
                "%{public}zu, srcSize %{public}zu", bufferLen, SIGNAL_DATA_SIZE * sizeof(uint8_t));
        }
        return true;
    }
    return false;
}

AudioLatencyMeasurement::AudioLatencyMeasurement(const int32_t &sampleRate,
                                                 const int32_t &channelCount, const int32_t &sampleFormat,
                                                 const std::string &appName, const uint32_t &sessionId)
    :format_(sampleFormat),
     sampleRate_(sampleRate),
     channelCount_(channelCount),
     sessionId_(sessionId),
     appName_(appName)
{
    std::string appToMock = "com.example.null";
    GetSysPara("persist.multimedia.apptomock", appToMock);
    AUDIO_INFO_LOG("LatencyMeas appName:%{public}s, appToMock:%{public}s, g_sessionId:%{public}u",
        appName.c_str(), appToMock.c_str(), g_sessionToMock);
    if (appToMock == appName && g_sessionToMock == 0) {
        mockThisStream_ = true;
        g_sessionToMock = sessionId;
    }
    formatByteSize_ = GetFormatByteSize(sampleFormat);
    InitSignalData();
}

AudioLatencyMeasurement::~AudioLatencyMeasurement()
{
    if (mockThisStream_ && g_sessionToMock == sessionId_) {
        g_sessionToMock = 0;
    }
}

void AudioLatencyMeasurement::InitSignalData()
{
    signalData_ = std::make_unique<int16_t[]>(SIGNAL_DATA_SIZE);
    memset_s(signalData_.get(), SIGNAL_DATA_SIZE, 0, SIGNAL_DATA_SIZE);
    const int16_t channels = 2; // 2 channels
    const int16_t samplePerChannel = SIGNAL_DATA_SIZE / channels;
    int16_t *signalBuffer = signalData_.get();
    for (int16_t index = 0; index < samplePerChannel; index++) {
        signalBuffer[index * channels] = SIGNAL_THRESHOLD + static_cast<int16_t>(sinf(2.0f *
            static_cast<float>(M_PI) * index / samplePerChannel) * (SHRT_MAX - SIGNAL_THRESHOLD));
        for (int16_t k = 1; k < channels; k++) {
            signalBuffer[channels * index + k] = signalBuffer[channels * index];
        }
    }
    AUDIO_INFO_LOG("LatencyMeas signalData inited");
    return;
}

bool AudioLatencyMeasurement::CheckIfEnabled()
{
    int32_t latencyMeasure = -1;
    GetSysPara("persist.multimedia.audiolatency", latencyMeasure);
    return (latencyMeasure == 1);
}

LatencyMonitor& LatencyMonitor::GetInstance()
{
    static LatencyMonitor latencyMonitor_;
    return latencyMonitor_;
}

void LatencyMonitor::UpdateClientTime(bool isRenderer, std::string &timestamp)
{
    if (isRenderer) {
        rendererMockTime_ = timestamp;
    } else {
        capturerDetectedTime_ = timestamp;
    }
}

void LatencyMonitor::UpdateSinkOrSourceTime(bool isRenderer, std::string &timestamp)
{
    if (isRenderer) {
        sinkDetectedTime_ = timestamp;
    } else {
        sourceDetectedTime_ = timestamp;
    }
}

void LatencyMonitor::UpdateDspTime(std::string timestamp)
{
    dspDetectedTime_ = timestamp;
}

void LatencyMonitor::ShowTimestamp(bool isRenderer)
{
    if (extraStrLen_ == 0) {
        extraStrLen_ = dspDetectedTime_.find("20");
    }
    if (isRenderer) {
        if (dspDetectedTime_.length() == 0) {
            AUDIO_ERR_LOG("LatencyMeas GetExtraParameter failed!");
            AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, SinkDetectedTime:%{public}s",
                rendererMockTime_.c_str(), sinkDetectedTime_.c_str());
            return;
        }
        dspBeforeSmartPa_ = dspDetectedTime_.substr(extraStrLen_, DATE_LENGTH);
        dspAfterSmartPa_ = dspDetectedTime_.substr(extraStrLen_ + DATE_LENGTH + 1 +
            extraStrLen_, DATE_LENGTH);
        AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, SinkDetectedTime:%{public}s, "
                       "DspBeforeSmartPa:%{public}s, DspAfterSmartPa:%{public}s", rendererMockTime_.c_str(),
                       sinkDetectedTime_.c_str(), dspBeforeSmartPa_.c_str(), dspAfterSmartPa_.c_str());
    } else {
        AUDIO_INFO_LOG("renderer mock time %{public}s", rendererMockTime_.c_str());
        if (dspDetectedTime_.length() == 0) {
            AUDIO_ERR_LOG("LatencyMeas GetExtraParam failed!");
            AUDIO_INFO_LOG("LatencyMeas CapturerDetectedTime:%{public}s, SourceDetectedTime:%{public}s",
                capturerDetectedTime_.c_str(), sourceDetectedTime_.c_str());
            return;
        }
        dspMockTime_ = dspDetectedTime_.substr(extraStrLen_ + DATE_LENGTH + extraStrLen_ + 1 +
            DATE_LENGTH + extraStrLen_ + 1, DATE_LENGTH);
        AUDIO_INFO_LOG("LatencyMeas CapturerDetectedTime:%{public}s, SourceDetectedTime:%{public}s, "
                       "DspMockTime:%{public}s", capturerDetectedTime_.c_str(), sourceDetectedTime_.c_str(),
                       dspMockTime_.c_str());
    }
}

void LatencyMonitor::ShowBluetoothTimestamp()
{
    AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, BTSinkDetectedTime:%{public}s",
        rendererMockTime_.c_str(), sinkDetectedTime_.c_str());
}

const std::string AudioInfoDumpUtils::GetStreamName(AudioStreamType streamType)
{
    std::string name;
    std::unordered_map<AudioStreamType, std::string> map = STREAM_TYPE_NAME_MAP;
    auto it = map.find(streamType);
    if (it != map.end()) {
        name = it->second;
    } else {
        name = "UNKNOWN";
    }

    const std::string streamName = name;
    return streamName;
}

const std::string AudioInfoDumpUtils::GetDeviceTypeName(DeviceType deviceType)
{
    std::string device;
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
            device = "EARPIECE";
            break;
        case DEVICE_TYPE_SPEAKER:
            device = "SPEAKER";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            device = "WIRED_HEADSET";
            break;
        case DEVICE_TYPE_WIRED_HEADPHONES:
            device = "WIRED_HEADPHONES";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
             device = "BLUETOOTH_SCO";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            device = "BLUETOOTH_A2DP";
            break;
        case DEVICE_TYPE_MIC:
            device = "MIC";
            break;
        case DEVICE_TYPE_HDMI:
            device = "HDMI";
            break;
        case DEVICE_TYPE_WAKEUP:
            device = "WAKEUP";
            break;
        case DEVICE_TYPE_NONE:
            device = "NONE";
            break;
        case DEVICE_TYPE_INVALID:
            device = "INVALID";
            break;
        default:
            device = "UNKNOWN";
    }

    const std::string deviceTypeName = device;
    return deviceTypeName;
}

const std::string AudioInfoDumpUtils::GetConnectTypeName(ConnectType connectType)
{
    std::string connectName;
    switch (connectType) {
        case OHOS::AudioStandard::CONNECT_TYPE_LOCAL:
            connectName = "LOCAL";
            break;
        case OHOS::AudioStandard::CONNECT_TYPE_DISTRIBUTED:
            connectName = "REMOTE";
            break;
        default:
            connectName = "UNKNOWN";
            break;
    }
    const std::string connectTypeName = connectName;
    return connectTypeName;
}

const std::string AudioInfoDumpUtils::GetSourceName(SourceType sourceType)
{
    std::string name;
    switch (sourceType) {
        case SOURCE_TYPE_INVALID:
            name = "INVALID";
            break;
        case SOURCE_TYPE_MIC:
            name = "MIC";
            break;
        case SOURCE_TYPE_CAMCORDER:
            name = "CAMCORDER";
            break;
        case SOURCE_TYPE_VOICE_RECOGNITION:
            name = "VOICE_RECOGNITION";
            break;
        case SOURCE_TYPE_ULTRASONIC:
            name = "ULTRASONIC";
            break;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            name = "VOICE_COMMUNICATION";
            break;
        case SOURCE_TYPE_WAKEUP:
            name = "WAKEUP";
            break;
        case SOURCE_TYPE_UNPROCESSED:
            name = "SOURCE_TYPE_UNPROCESSED";
            break;
        default:
            name = "UNKNOWN";
    }

    const std::string sourceName = name;
    return sourceName;
}

const std::string AudioInfoDumpUtils::GetDeviceVolumeTypeName(DeviceVolumeType deviceType)
{
    std::string device;
    switch (deviceType) {
        case EARPIECE_VOLUME_TYPE:
            device = "EARPIECE";
            break;
        case SPEAKER_VOLUME_TYPE:
            device = "SPEAKER";
            break;
        case HEADSET_VOLUME_TYPE:
            device = "HEADSET";
            break;
        default:
            device = "UNKNOWN";
    }

    const std::string deviceTypeName = device;
    return deviceTypeName;
}

bool VolumeUtils::isPCVolumeEnable_ = false;

std::unordered_map<AudioStreamType, AudioVolumeType> VolumeUtils::defaultVolumeMap_ = {
    {STREAM_VOICE_CALL, STREAM_VOICE_CALL},
    {STREAM_VOICE_COMMUNICATION, STREAM_VOICE_CALL},
    {STREAM_VOICE_CALL_ASSISTANT, STREAM_VOICE_CALL_ASSISTANT},

    {STREAM_RING, STREAM_RING},
    {STREAM_SYSTEM, STREAM_RING},
    {STREAM_NOTIFICATION, STREAM_RING},
    {STREAM_SYSTEM_ENFORCED, STREAM_RING},
    {STREAM_DTMF, STREAM_RING},
    {STREAM_VOICE_RING, STREAM_RING},

    {STREAM_MUSIC, STREAM_MUSIC},
    {STREAM_MEDIA, STREAM_MUSIC},
    {STREAM_MOVIE, STREAM_MUSIC},
    {STREAM_GAME, STREAM_MUSIC},
    {STREAM_SPEECH, STREAM_MUSIC},
    {STREAM_NAVIGATION, STREAM_MUSIC},
    {STREAM_VOICE_MESSAGE, STREAM_MUSIC},

    {STREAM_VOICE_ASSISTANT, STREAM_VOICE_ASSISTANT},
    {STREAM_ALARM, STREAM_ALARM},
    {STREAM_ACCESSIBILITY, STREAM_ACCESSIBILITY},
    {STREAM_ULTRASONIC, STREAM_ULTRASONIC},
    {STREAM_ALL, STREAM_ALL},
    {STREAM_APP, STREAM_APP}
};

std::unordered_map<AudioStreamType, AudioVolumeType> VolumeUtils::audioPCVolumeMap_ = {
    {STREAM_VOICE_CALL, STREAM_MUSIC},
    {STREAM_VOICE_CALL_ASSISTANT, STREAM_VOICE_CALL_ASSISTANT},
    {STREAM_VOICE_MESSAGE, STREAM_MUSIC},
    {STREAM_VOICE_ASSISTANT, STREAM_MUSIC},
    {STREAM_VOICE_COMMUNICATION, STREAM_MUSIC},
    {STREAM_DTMF, STREAM_MUSIC},
    {STREAM_MUSIC, STREAM_MUSIC},
    {STREAM_MEDIA, STREAM_MUSIC},
    {STREAM_MOVIE, STREAM_MUSIC},
    {STREAM_GAME, STREAM_MUSIC},
    {STREAM_SPEECH, STREAM_MUSIC},
    {STREAM_RECORDING, STREAM_MUSIC},
    {STREAM_NAVIGATION, STREAM_MUSIC},
    {STREAM_ACCESSIBILITY, STREAM_MUSIC},
    {STREAM_ALL, STREAM_ALL},

    {STREAM_RING, STREAM_MUSIC},
    {STREAM_VOICE_RING, STREAM_MUSIC},
    {STREAM_ALARM, STREAM_MUSIC},

    {STREAM_SYSTEM, STREAM_SYSTEM},
    {STREAM_NOTIFICATION, STREAM_SYSTEM},
    {STREAM_SYSTEM_ENFORCED, STREAM_SYSTEM},

    {STREAM_ULTRASONIC, STREAM_ULTRASONIC},
    {STREAM_APP, STREAM_APP}
};

std::unordered_map<AudioStreamType, AudioVolumeType>& VolumeUtils::GetVolumeMap()
{
    if (isPCVolumeEnable_) {
        return audioPCVolumeMap_;
    } else {
        return defaultVolumeMap_;
    }
}

void VolumeUtils::SetPCVolumeEnable(const bool& isPCVolumeEnable)
{
    isPCVolumeEnable_ = isPCVolumeEnable;
}

bool VolumeUtils::IsPCVolumeEnable()
{
    return isPCVolumeEnable_;
}

AudioVolumeType VolumeUtils::GetVolumeTypeFromStreamType(AudioStreamType streamType)
{
    std::unordered_map<AudioStreamType, AudioVolumeType> map = GetVolumeMap();
    auto it = map.find(streamType);
    if (it != map.end()) {
        return it->second;
    }
    return STREAM_MUSIC;
}

std::string GetEncryptStr(const std::string &src)
{
    if (src.empty()) {
        return std::string("");
    }

    size_t strLen = src.length();
    std::string dst;

    if (strLen < MIN_LEN) {
        // src: abcdef
        // dst: *bcdef
        dst = '*' + src.substr(FIRST_CHAR, strLen - FIRST_CHAR);
    } else {
        // src: 00:00:00:00:00:00
        // dst: 00**********00:00
        dst = src.substr(0, HEAD_STR_LEN);
        std::string tempStr(strLen - HEAD_STR_LEN - TAIL_STR_LEN, '*');
        dst += tempStr;
        dst += src.substr(strLen - TAIL_STR_LEN, TAIL_STR_LEN);
    }

    return dst;
}

std::string ConvertNetworkId(const std::string &networkId)
{
    if (!networkId.empty() && networkId != LOCAL_NETWORK_ID) {
        return REMOTE_NETWORK_ID;
    }

    return networkId;
}

uint32_t GenerateUniqueID(AudioHdiUniqueIDBase base, uint32_t offset)
{
    return base + offset * UNIQUE_ID_INTERVAL;
}

AudioDump& AudioDump::GetInstance()
{
    static AudioDump mAudioDump;
    return mAudioDump;
}

void AudioDump::SetVersionType(const std::string& versionType)
{
    versionType_ = versionType;
}

std::string AudioDump::GetVersionType()
{
    return versionType_;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

struct CTrace {
    explicit CTrace(const char *traceName) : trace(OHOS::AudioStandard::Trace(traceName)) {};
    OHOS::AudioStandard::Trace trace;
};

CTrace *GetAndStart(const char *traceName)
{
    std::unique_ptr<CTrace> cTrace = std::make_unique<CTrace>(traceName);

    return cTrace.release();
}

void EndCTrace(CTrace *cTrace)
{
    if (cTrace != nullptr) {
        cTrace->trace.End();
    }
}

void CTraceCount(const char *traceName, int64_t count)
{
    OHOS::AudioStandard::Trace::Count(traceName, count);
}

void CallEndAndClear(CTrace **cTrace)
{
    if (cTrace != nullptr && *cTrace != nullptr) {
        EndCTrace(*cTrace);
        delete *cTrace;
        *cTrace = nullptr;
    }
}

#ifdef __cplusplus
}
#endif
