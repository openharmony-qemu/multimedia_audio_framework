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
#define LOG_TAG "AudioPolicyServer"
#endif

#include "audio_policy_server.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"

#endif

#include "privacy_kit.h"
#include "tokenid_kit.h"
#include "common_event_manager.h"
#include "system_ability_definition.h"
#include "audio_policy_log.h"
#include "parameters.h"
#include "media_monitor_manager.h"
#include "client_type_manager.h"
#include "dfx_msg_manager.h"
#ifdef USB_ENABLE
#include "audio_usb_manager.h"
#endif
#include "audio_zone_service.h"
#include "istandard_audio_zone_client.h"
#include "audio_bundle_manager.h"
#include "audio_server_proxy.h"
#include "audio_policy_client_holder.h"
#include "standalone_mode_manager.h"

using OHOS::Security::AccessToken::PrivacyKit;
using OHOS::Security::AccessToken::TokenIdKit;
using namespace std;

namespace OHOS {
namespace AudioStandard {

const std::vector<AudioStreamType> GET_STREAM_ALL_VOLUME_TYPES {
    STREAM_VOICE_ASSISTANT,
    STREAM_VOICE_CALL,
    STREAM_ACCESSIBILITY,
    STREAM_RING,
    STREAM_ALARM,
    STREAM_VOICE_RING,
    STREAM_ULTRASONIC,
    // adjust the type of music from the head of list to end, make sure music is updated last.
    // avoid interference from ring updates on special platform.
    // when the device is switched to headset,ring and alarm is dualtone type.
    // dualtone type use fixed volume curve of speaker.
    // the ring and alarm are classified into the music group.
    // the music volume becomes abnormal when the db value of music is modified.
    STREAM_MUSIC
};

const std::list<AudioStreamType> CAN_MIX_MUTED_STREAM = {
    STREAM_NOTIFICATION
};

const size_t ADD_VOL_RECORD_LIMIT = 3;
constexpr int64_t ADD_VOL_DURATION_LIMIT = 800 * 1000 * 1000; // 800ms
const int64_t SILENT_FRAME_LIMIT = -50;
constexpr int32_t PARAMS_VOLUME_NUM = 5;
constexpr int32_t PARAMS_INTERRUPT_NUM = 4;
constexpr int32_t PARAMS_RENDER_STATE_NUM = 2;
constexpr int32_t EVENT_DES_SIZE = 80;
constexpr int32_t ADAPTER_STATE_CONTENT_DES_SIZE = 60;
constexpr int32_t API_VERSION_REMAINDER = 1000;
constexpr pid_t FIRST_SCREEN_ON_PID = 1000;
constexpr uid_t UID_CAST_ENGINE_SA = 5526;
constexpr uid_t UID_AUDIO = 1041;
constexpr uid_t UID_FOUNDATION_SA = 5523;
constexpr uid_t UID_BLUETOOTH_SA = 1002;
constexpr uid_t UID_CAR_DISTRIBUTED_ENGINE_SA = 65872;
constexpr uid_t UID_TV_PROCESS_SA = 7501;
constexpr uid_t UID_DP_PROCESS_SA = 7062;
constexpr uid_t UID_NEARLINK_SA = 7030;
constexpr uid_t UID_PENCIL_PROCESS_SA = 7555;
constexpr uid_t UID_RESOURCE_SCHEDULE_SERVICE = 1096;
constexpr uid_t UID_AVSESSION_SERVICE = 6700;
constexpr int64_t OFFLOAD_NO_SESSION_ID = -1;
const char* MANAGE_SYSTEM_AUDIO_EFFECTS = "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS";
const char* MANAGE_AUDIO_CONFIG = "ohos.permission.MANAGE_AUDIO_CONFIG";
const char* USE_BLUETOOTH_PERMISSION = "ohos.permission.USE_BLUETOOTH";
const char* MICROPHONE_CONTROL_PERMISSION = "ohos.permission.MICROPHONE_CONTROL";
const std::string CALLER_NAME = "audio_server";
static const int64_t WAIT_CLEAR_AUDIO_FOCUSINFOS_TIME_US = 300000; // 300ms
const std::string HIVIEWCARE_PERMISSION = "ohos.permission.ACCESS_HIVIEWCARE";
static const uint32_t DEVICE_CONNECTED_FLAG_DURATION_MS = 3000000; // 3s
constexpr int32_t MAX_STREAM_USAGE_COUNT = StreamUsage::STREAM_USAGE_MAX + 1;
constexpr int32_t MAX_SIZE = 1024;
constexpr int32_t DEFAULT_UID = 0;
constexpr int32_t DEFAULT_ZONEID = 0;

constexpr int32_t UID_MEDIA = 1013;
constexpr int32_t UID_MCU = 7500;
constexpr int32_t UID_CAAS = 5527;
constexpr int32_t UID_TELEPHONY = 1001;
constexpr int32_t UID_DMSDP = 7071;
constexpr int32_t UID_TV_SERVICE = 7501;
static const int32_t DATASHARE_SERVICE_TIMEOUT_SECONDS = 10; // 10s is better
const std::set<int32_t> CALLBACK_TRUST_LIST = {
    UID_MEDIA,
    UID_MCU,
    UID_CAAS,
    UID_TELEPHONY,
    UID_DMSDP,
    UID_TV_SERVICE
};

REGISTER_SYSTEM_ABILITY_BY_ID(AudioPolicyServer, AUDIO_POLICY_SERVICE_ID, true)

std::map<PolicyType, uint32_t> POLICY_TYPE_MAP = {
    {PolicyType::EDM_POLICY_TYPE, 0},
    {PolicyType::PRIVACY_POLCIY_TYPE, 1},
    {PolicyType::TEMPORARY_POLCIY_TYPE, 2}
};

AudioPolicyServer::AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
      audioEffectService_(AudioEffectService::GetAudioEffectService()),
      audioAffinityManager_(AudioAffinityManager::GetAudioAffinityManager()),
      audioCapturerSession_(AudioCapturerSession::GetInstance()),
      audioStateManager_(AudioStateManager::GetAudioStateManager()),
      audioToneManager_(AudioToneManager::GetInstance()),
      audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
      audioDeviceStatus_(AudioDeviceStatus::GetInstance()),
      audioConfigManager_(AudioPolicyConfigManager::GetInstance()),
      audioSceneManager_(AudioSceneManager::GetInstance()),
      audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
      audioDeviceLock_(AudioDeviceLock::GetInstance()),
      streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
      audioOffloadStream_(AudioOffloadStream::GetInstance()),
      audioBackgroundManager_(AudioBackgroundManager::GetInstance()),
      audioVolumeManager_(AudioVolumeManager::GetInstance()),
      audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
      audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
      audioPolicyConfigManager_(AudioPolicyConfigManager::GetInstance()),
      audioPolicyService_(AudioPolicyService::GetAudioPolicyService()),
      audioPolicyUtils_(AudioPolicyUtils::GetInstance()),
      audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
      audioSpatializationService_(AudioSpatializationService::GetAudioSpatializationService()),
      audioCollaborativeService_(AudioCollaborativeService::GetAudioCollaborativeService()),
      audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
      audioPolicyDump_(AudioPolicyDump::GetInstance()),
#ifdef USB_ENABLE
      usbManager_(AudioUsbManager::GetInstance()),
#endif
      audioActiveDevice_(AudioActiveDevice::GetInstance())

{
    volumeStep_ = system::GetIntParameter("const.multimedia.audio.volumestep", 1);
    AUDIO_INFO_LOG("Get volumeStep parameter success %{public}d", volumeStep_);

    powerStateCallbackRegister_ = false;
    supportVibrator_ = system::GetBoolParameter("const.vibrator.support_vibrator", true);
    volumeApplyToAll_ = system::GetBoolParameter("const.audio.volume_apply_to_all", false);
    screenOffAdjustVolumeEnable_ = system::GetBoolParameter("const.audio.screenoff_adjust_volume_enable", false);
    if (volumeApplyToAll_) {
        audioPolicyConfigManager_.SetNormalVoipFlag(true);
    }
}

static std::string TranslateKeyEvent(const int32_t keyType)
{
    string event = "KEYCODE_UNKNOWN";

    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) {
        event = "KEYCODE_VOLUME_UP";
    } else if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN) {
        event = "KEYCODE_VOLUME_DOWN";
    } else if (keyType == OHOS::MMI::KeyEvent::KEYCODE_MUTE) {
        event = "KEYCODE_MUTE";
    }
    return event;
}

static bool HasUsbDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    for (auto &item : devices) {
        CHECK_AND_CONTINUE(item != nullptr);
        if (IsUsb(item->deviceType_) && !item->hasPair_) {
            return true;
        }
    }
    return false;
}

uint32_t AudioPolicyServer::TranslateErrorCode(int32_t result)
{
    uint32_t resultForMonitor = 0;
    switch (result) {
        case ERR_INVALID_PARAM:
            resultForMonitor = ERR_SUBSCRIBE_INVALID_PARAM;
            break;
        case ERR_NULL_POINTER:
            resultForMonitor = ERR_SUBSCRIBE_KEY_OPTION_NULL;
            break;
        case ERR_MMI_CREATION:
            resultForMonitor = ERR_SUBSCRIBE_MMI_NULL;
            break;
        case ERR_MMI_SUBSCRIBE:
            resultForMonitor = ERR_MODE_SUBSCRIBE;
            break;
        default:
            break;
    }
    return resultForMonitor;
}

void AudioPolicyServer::OnDump()
{
    return;
}

void AudioPolicyServer::OnStart()
{
    AUDIO_INFO_LOG("Audio policy server on start");
    DlopenUtils::Init();
    interruptService_ = std::make_shared<AudioInterruptService>();
    interruptService_->Init(this);

    audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    audioPolicyServerHandler_->Init(interruptService_);

    interruptService_->SetCallbackHandler(audioPolicyServerHandler_);

    AudioZoneService::GetInstance().Init(audioPolicyServerHandler_, interruptService_);
    StandaloneModeManager::GetInstance().InIt(interruptService_);
    if (audioPolicyManager_.SetAudioStreamRemovedCallback(this)) {
        AUDIO_ERR_LOG("SetAudioStreamRemovedCallback failed");
    }
    audioPolicyService_.Init();

    coreService_ = AudioCoreService::GetCoreService();
    coreService_->SetCallbackHandler(audioPolicyServerHandler_);
    coreService_->Init();
    eventEntry_ = coreService_->GetEventEntry();

    bool res = Publish(this);
    if (!res) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_SERVICE_STARTUP_ERROR,
            Media::MediaMonitor::EventType::FAULT_EVENT);
        bean->Add("SERVICE_ID", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID));
        bean->Add("ERROR_CODE", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVER));
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        AUDIO_INFO_LOG("publish sa err");
    }
    AddSystemAbilityListeners();

    Security::AccessToken::PermStateChangeScope scopeInfo;
    scopeInfo.permList = {"ohos.permission.MICROPHONE"};
    auto callbackPtr = std::make_shared<PerStateChangeCbCustomizeCallback>(scopeInfo, this);
    callbackPtr->ready_ = false;
    int32_t iRes = Security::AccessToken::AccessTokenKit::RegisterPermStateChangeCallback(callbackPtr);
    if (iRes < 0) {
        AUDIO_ERR_LOG("fail to call RegisterPermStateChangeCallback.");
    }
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    SubscribeVolumeKeyEvents();
#endif
    if (getpid() > FIRST_SCREEN_ON_PID) {
        audioDeviceCommon_.SetFirstScreenOn();
    }
    // Restart to reload the volume.
    InitKVStore();
    isScreenOffOrLock_ = !PowerMgr::PowerMgrClient::GetInstance().IsScreenOn(true);
    DlopenUtils::DeInit();
    DfxMsgManager::GetInstance().Init();
    AUDIO_INFO_LOG("Audio policy server start end");
}

void AudioPolicyServer::AddSystemAbilityListeners()
{
    AddSystemAbilityListener(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    AddSystemAbilityListener(AUDIO_DISTRIBUTED_SERVICE_ID);
    AddSystemAbilityListener(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    AddSystemAbilityListener(APP_MGR_SERVICE_ID);
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
#endif
    AddSystemAbilityListener(BLUETOOTH_HOST_SYS_ABILITY_ID);
    AddSystemAbilityListener(POWER_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
#ifdef USB_ENABLE
    AddSystemAbilityListener(USB_SYSTEM_ABILITY_ID);
#endif
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
#ifdef SUPPORT_USER_ACCOUNT
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
#endif
}

void AudioPolicyServer::OnStop()
{
    audioPolicyService_.Deinit();
    coreService_->DeInit();
#ifdef USB_ENABLE
    usbManager_.Deinit();
#endif
    UnRegisterPowerStateListener();
    UnRegisterSyncHibernateListener();
    return;
}

void AudioPolicyServer::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    AUDIO_INFO_LOG("SA Id is :%{public}d", systemAbilityId);
    int64_t stamp = ClockTime::GetCurNano();
    switch (systemAbilityId) {
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        case MULTIMODAL_INPUT_SERVICE_ID:
            SubscribeVolumeKeyEvents();
            break;
#endif
        case DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID:
            HandleKvDataShareEvent();
            break;
        case DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID:
            AddRemoteDevstatusCallback();
            break;
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            AddAudioServiceOnStart();
            break;
        case BLUETOOTH_HOST_SYS_ABILITY_ID:
            RegisterBluetoothListener();
            break;
        case POWER_MANAGER_SERVICE_ID:
            SubscribePowerStateChangeEvents();
            RegisterPowerStateListener();
            RegisterSyncHibernateListener();
            break;
        case SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN:
            SubscribeOsAccountChangeEvents();
            break;
        case COMMON_EVENT_SERVICE_ID:
            SubscribeCommonEventExecute();
            break;
        case BACKGROUND_TASK_MANAGER_SERVICE_ID:
            SubscribeBackgroundTask();
            break;
#ifdef USB_ENABLE
        case USB_SYSTEM_ABILITY_ID:
            usbManager_.Init(eventEntry_);
            break;
#endif
        default:
            OnAddSystemAbilityExtract(systemAbilityId, deviceId);
            break;
    }
    // eg. done systemAbilityId: [3001] cost 780ms
    AUDIO_INFO_LOG("done systemAbilityId: [%{public}d] cost %{public}" PRId64 " ms", systemAbilityId,
        (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND);
}

void AudioPolicyServer::RegisterDefaultVolumeTypeListener()
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("RegisterDefaultVolumeTypeListener interruptService_ is nullptr!");
        return;
    }
    interruptService_->RegisterDefaultVolumeTypeListener();
}

void AudioPolicyServer::OnAddSystemAbilityExtract(int32_t systemAbilityId, const std::string &deviceId)
{
    switch (systemAbilityId) {
        case APP_MGR_SERVICE_ID:
            RegisterAppStateListener();
            break;
        default:
            AUDIO_WARNING_LOG("OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
}

void AudioPolicyServer::HandleKvDataShareEvent()
{
    AUDIO_INFO_LOG("OnAddSystemAbility kv data service start");
    if (isInitMuteState_ == false && AudioPolicyUtils::GetInstance().IsDataShareReady()) {
        AUDIO_INFO_LOG("datashare is ready and need init mic mute state");
        InitMicrophoneMute();
    }
}

void AudioPolicyServer::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    AUDIO_DEBUG_LOG("AudioPolicyServer::OnRemoveSystemAbility systemAbilityId:%{public}d removed", systemAbilityId);
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
bool AudioPolicyServer::MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType,
    const AudioStreamType &streamInFocus)
{
    int32_t streamInFocusInt = static_cast<int32_t>(streamInFocus);
    int32_t volumeLevelMax = -1;
    int32_t volumeLevelMin = -1;
    GetMaxVolumeLevel(streamInFocusInt, volumeLevelMax);
    GetMinVolumeLevel(streamInFocusInt, volumeLevelMin);
    bool volLevelCheck = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ?
        volLevel >= volumeLevelMax : volLevel <= volumeLevelMin;
    if (volLevelCheck) {
        VolumeEvent volumeEvent;
        volumeEvent.volumeType = (streamInFocus == STREAM_ALL) ? STREAM_MUSIC : streamInFocus;
        volumeEvent.volume = volLevel;
        volumeEvent.updateUi = true;
        volumeEvent.volumeGroupId = 0;
        volumeEvent.networkId = LOCAL_NETWORK_ID;
        CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, false, "audioPolicyServerHandler_ is nullptr");
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
        return true;
    }

    return false;
}
#endif

void AudioPolicyServer::ChangeVolumeOnVoiceAssistant(AudioStreamType &streamInFocus)
{
    if (streamInFocus == AudioStreamType::STREAM_VOICE_ASSISTANT &&
        audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_BLUETOOTH_A2DP) {
        streamInFocus = AudioStreamType::STREAM_MUSIC;
    }
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
int32_t AudioPolicyServer::RegisterVolumeKeyEvents(const int32_t keyType)
{
    if ((keyType != OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) && (keyType != OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN)) {
        AUDIO_ERR_LOG("VolumeKeyEvents: invalid key type : %{public}d", keyType);
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG("RegisterVolumeKeyEvents: volume key: %{public}s.",
        (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");

    MMI::InputManager *im = MMI::InputManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(im != nullptr, ERR_MMI_CREATION, "Failed to obtain INPUT manager");

    std::set<int32_t> preKeys;
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_RET_LOG(keyOption != nullptr, ERR_NULL_POINTER, "Invalid key option");
    WatchTimeout guard("keyOption->SetPreKeys:RegisterVolumeKeyEvents");
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(keyType);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(VOLUME_KEY_DURATION);
    guard.CheckCurrTimeout();
    int32_t keySubId = im->SubscribeKeyEvent(keyOption, [=](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
        AUDIO_PRERELEASE_LOGI("Receive volume key event: %{public}s.",
            (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
        int32_t ret = ProcessVolumeKeyEvents(keyType);
        if (ret != AUDIO_OK) {
            AUDIO_DEBUG_LOG("process volume key mute events need return[%{public}d]", ret);
            return;
        }
    });
    std::string RegistrationTime = GetTime();
    std::string keyName = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP ? "Volume Up" : "Volume Down");
    audioVolumeManager_.SaveVolumeKeyRegistrationInfo(keyName, RegistrationTime, keySubId,
        keySubId >= 0 ? true : false);
    AUDIO_INFO_LOG("RegisterVolumeKeyInfo keyType: %{public}s, RegistrationTime: %{public}s, keySubId: %{public}d,"
        " Regist Success: %{public}s",
        keyName.c_str(), RegistrationTime.c_str(), keySubId, keySubId >= 0 ? "true" : "false");

    if (keySubId < 0) {
        AUDIO_ERR_LOG("key: %{public}s failed", (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
        return ERR_MMI_SUBSCRIBE;
    }
    return keySubId;
}

bool AudioPolicyServer::IsContinueAddVol()
{
    int64_t cur = ClockTime::GetRealNano();
    // record history
    std::lock_guard<std::mutex> lock(volUpHistoryMutex_);
    volUpHistory_.push_back(cur);
    size_t total = volUpHistory_.size();
    if (total < ADD_VOL_RECORD_LIMIT) {
        return false;
    }
    // continue add vol for 3 times
    int64_t first = volUpHistory_.front();
    volUpHistory_.pop_front();
    if (cur - first >= ADD_VOL_DURATION_LIMIT) {
        return false;
    }
    AUDIO_WARNING_LOG("add vol %{public}zu times, first %{public}" PRId64" cur %{public}" PRId64 "", total, first, cur);
    return true;
}

void AudioPolicyServer::TriggerMuteCheck()
{
    Trace trace("AudioPolicyServer::TriggerMuteCheck");
    // get current running sessionId
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos = {};
    streamCollector_.GetRunningRendererInfos(infos);
    if (infos.size() == 0) {
        AUDIO_WARNING_LOG("no running stream");
        return;
    }

    std::set<std::string> deviceClassSet;
    for (auto info : infos) {
        if (info == nullptr || info->outputDeviceInfo.networkId_ != LOCAL_NETWORK_ID) {
            continue;
        }
        auto deviceType = info->outputDeviceInfo.getType();
        std::string sinkPortName = audioPolicyUtils_.GetSinkPortName(deviceType, info->rendererInfo.pipeType);
        std::string deviceClass = audioPolicyUtils_.GetOutputDeviceClassBySinkPortName(sinkPortName);
        deviceClassSet.insert(deviceClass);
        AUDIO_INFO_LOG("uid-%{public}d pid-%{public}d:[%{public}d] running with pipe %{public}d on sink:%{public}s",
            info->clientUID, info->clientPid, info->sessionId, info->rendererInfo.pipeType, deviceClass.c_str());
    }

    bool mutePlay = false;
    for (auto sinkName : deviceClassSet) {
        int64_t volumeDataCount = AudioServerProxy::GetInstance().GetVolumeDataCount(sinkName);
        if (volumeDataCount < SILENT_FRAME_LIMIT) {
            mutePlay = true;
            AUDIO_WARNING_LOG("sink:%{public}s running with mute data", sinkName.c_str());
        }
    }

    if (mutePlay) {
        // print volume info
        AudioStreamType streamInFocus = GetCurrentStreamInFocus();
        int32_t volume = GetSystemVolumeLevelInternal(VolumeUtils::GetVolumeTypeFromStreamType(streamInFocus));
        AUDIO_WARNING_LOG("StreamInFocus is [%{public}d], volume is %{public}d", streamInFocus, volume);
    }
}

int32_t AudioPolicyServer::ProcessVolumeKeyEvents(const int32_t keyType)
{
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && IsContinueAddVol()) {
        std::thread([this]() { TriggerMuteCheck(); }).detach();
    }
    int32_t zoneId = AudioZoneService::GetInstance().CheckZoneExist(audioVolumeManager_.GetVolumeAdjustZoneId()) ?
        audioVolumeManager_.GetVolumeAdjustZoneId() : 0;
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
    if (volumeApplyToAll_) {
        streamInFocus = AudioStreamType::STREAM_ALL;
    } else {
        streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetCurrentStreamInFocus());
    }
    bool active = false;
    IsStreamActive(streamInFocus, active);
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    if (isScreenOffOrLock_ && !active && !VolumeUtils::IsPCVolumeEnable() && !screenOffAdjustVolumeEnable_) {
        AUDIO_INFO_LOG("isScreenOffOrLock: %{public}d, active: %{public}d, screenOffAdjustVolumeEnable: %{public}d",
            isScreenOffOrLock_.load(), active, screenOffAdjustVolumeEnable_);
        return AUDIO_OK;
    }
    if (!VolumeUtils::IsPCVolumeEnable()) {
        ChangeVolumeOnVoiceAssistant(streamInFocus);
    }
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(streamInFocus, zoneId)) {
        AUDIO_INFO_LOG("VolumeKeyEvents: volumeKey: Up. volumeType %{public}d is mute. Unmute.", streamInFocus);
        SetStreamMuteInternal(streamInFocus, false, true, DEVICE_TYPE_NONE, zoneId);
        CHECK_AND_RETURN_RET_LOG(VolumeUtils::IsPCVolumeEnable(), ERROR_UNSUPPORTED, "phone need return");
    }
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(STREAM_SYSTEM) &&
        VolumeUtils::IsPCVolumeEnable()) {
        SetStreamMuteInternal(STREAM_SYSTEM, false, true);
    }
    return SetVolumeInternalByKeyEvent(streamInFocus, zoneId, keyType);
}
#endif

int32_t AudioPolicyServer::SetVolumeInternalByKeyEvent(AudioStreamType streamInFocus, int32_t zoneId,
    const int32_t keyType)
{
    int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus, zoneId);
    if (MaxOrMinVolumeOption(volumeLevelInInt, keyType, streamInFocus)) {
        AUDIO_ERR_LOG("device %{public}d, stream %{public}d, volumelevel %{public}d invalid",
            audioActiveDevice_.GetCurrentOutputDeviceType(), streamInFocus, volumeLevelInInt);
        return ERROR_INVALID_PARAM;
    }

    volumeLevelInInt = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? ++volumeLevelInInt : --volumeLevelInInt;
    int32_t ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, true, zoneId);
    if (ret == ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED) {
        SendVolumeKeyEventCbWithUpdateUiOrNot(streamInFocus, true);
    }
    if (volumeLevelInInt <= 0 && VolumeUtils::IsPCVolumeEnable()) {
        SetStreamMuteInternal(STREAM_SYSTEM, true, true);
    }
    return AUDIO_OK;
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
int32_t AudioPolicyServer::RegisterVolumeKeyMuteEvents()
{
    AUDIO_INFO_LOG("RegisterVolumeKeyMuteEvents: volume key: mute");
    MMI::InputManager *im = MMI::InputManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(im != nullptr, ERR_MMI_CREATION, "Failed to obtain INPUT manager");

    std::shared_ptr<OHOS::MMI::KeyOption> keyOptionMute = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_RET_LOG(keyOptionMute != nullptr, ERR_NULL_POINTER, "keyOptionMute: Invalid key option");
    std::set<int32_t> preKeys;
    WatchTimeout guard("keyOption->SetPreKeys:RegisterVolumeKeyMuteEvents");
    keyOptionMute->SetPreKeys(preKeys);
    keyOptionMute->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_MUTE);
    keyOptionMute->SetFinalKeyDown(true);
    keyOptionMute->SetFinalKeyDownDuration(VOLUME_MUTE_KEY_DURATION);
    keyOptionMute->SetRepeat(false);
    guard.CheckCurrTimeout();
    int32_t muteKeySubId = im->SubscribeKeyEvent(keyOptionMute,
        [this](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
            AUDIO_INFO_LOG("Receive volume key event: mute");
            AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
            bool isStreamMuted = false;
            if (volumeApplyToAll_) {
                streamInFocus = STREAM_ALL;
            } else {
                streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetCurrentStreamInFocus());
            }
            std::lock_guard<std::mutex> lock(systemVolumeMutex_);
            isStreamMuted = GetStreamMuteInternal(streamInFocus);
            SetStreamMuteInternal(streamInFocus, !isStreamMuted, true);
        });
    std::string keyType = "mute";
    std::string RegistrationTime = GetTime();
    audioVolumeManager_.SaveVolumeKeyRegistrationInfo(keyType, RegistrationTime, muteKeySubId,
        muteKeySubId >= 0 ? true : false);
    AUDIO_INFO_LOG("RegisterVolumeKeyInfo keyType: %{public}s, RegistrationTime: %{public}s, keySubId: %{public}d,"
        " Regist Success: %{public}s",
        keyType.c_str(), RegistrationTime.c_str(), muteKeySubId, muteKeySubId >= 0 ? "true" : "false");

    if (muteKeySubId < 0) {
        AUDIO_ERR_LOG("SubscribeKeyEvent: subscribing for mute failed ");
        return ERR_MMI_SUBSCRIBE;
    }
    return muteKeySubId;
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
void AudioPolicyServer::SubscribeVolumeKeyEvents()
{
    std::lock_guard<std::mutex> lock(subscribeVolumeKey_);
    if (hasSubscribedVolumeKeyEvents_.load()) {
        AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: volume key events has been sunscirbed!");
        return;
    }

    AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: first time.");
    int32_t resultOfVolumeUp = RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP);
    SendMonitrtEvent(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, resultOfVolumeUp);
    int32_t resultOfVolumeDown = RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN);
    SendMonitrtEvent(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, resultOfVolumeDown);
    int32_t resultOfMute = RegisterVolumeKeyMuteEvents();
    SendMonitrtEvent(OHOS::MMI::KeyEvent::KEYCODE_MUTE, resultOfMute);
    if (resultOfVolumeUp >= 0 && resultOfVolumeDown >= 0 && resultOfMute >= 0) {
        hasSubscribedVolumeKeyEvents_.store(true);
    } else {
        AUDIO_ERR_LOG("SubscribeVolumeKeyEvents: failed to subscribe key events.");
        hasSubscribedVolumeKeyEvents_.store(false);
    }
}
#endif

void AudioPolicyServer::SendMonitrtEvent(const int32_t keyType, int32_t resultOfVolumeKey)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::VOLUME_SUBSCRIBE,
        Media::MediaMonitor::EventType::BEHAVIOR_EVENT);
    bean->Add("SUBSCRIBE_KEY", TranslateKeyEvent(keyType));
    bean->Add("SUBSCRIBE_RESULT", static_cast<int32_t>(TranslateErrorCode(resultOfVolumeKey)));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioPolicyServer::SubscribeSafeVolumeEvent()
{
    AUDIO_INFO_LOG("enter");
    audioPolicyService_.SubscribeSafeVolumeEvent();
}

bool AudioPolicyServer::IsVolumeTypeValid(AudioStreamType streamType)
{
    bool result = false;
    switch (streamType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_SYSTEM:
        case STREAM_ACCESSIBILITY:
        case STREAM_ULTRASONIC:
        case STREAM_NAVIGATION:
        case STREAM_ALL:
        case STREAM_VOICE_RING:
        case STREAM_CAMCORDER:
            result = true;
            break;
        default:
            result = false;
            AUDIO_ERR_LOG("IsVolumeTypeValid: streamType[%{public}d] is not supported", streamType);
            break;
    }
    return result;
}

bool AudioPolicyServer::IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel)
{
    bool result = true;
    if (volumeLevel < audioVolumeManager_.GetMinVolumeLevel(streamType) ||
        volumeLevel > audioVolumeManager_.GetMaxVolumeLevel(streamType)) {
        AUDIO_ERR_LOG("IsVolumeLevelValid: volumeLevel[%{public}d] is out of valid range for streamType[%{public}d]",
            volumeLevel, streamType);
        result = false;
    }
    return result;
}

bool AudioPolicyServer::IsRingerModeValid(AudioRingerMode ringMode)
{
    bool result = false;
    switch (ringMode) {
        case RINGER_MODE_SILENT:
        case RINGER_MODE_VIBRATE:
        case RINGER_MODE_NORMAL:
            result = true;
            break;
        default:
            result = false;
            AUDIO_ERR_LOG("IsRingerModeValid: ringMode[%{public}d] is not supported", ringMode);
            break;
    }
    return result;
}

void AudioPolicyServer::SubscribeOsAccountChangeEvents()
{
    AUDIO_INFO_LOG("OnAddSystemAbility os_account service start");
    if (accountObserver_ == nullptr) {
        AccountSA::OsAccountSubscribeInfo osAccountSubscribeInfo;
        osAccountSubscribeInfo.SetOsAccountSubscribeType(AccountSA::OS_ACCOUNT_SUBSCRIBE_TYPE::SWITCHED);
        accountObserver_ = std::make_shared<AudioOsAccountInfo>(osAccountSubscribeInfo, this);
        ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(accountObserver_);
        CHECK_AND_RETURN_LOG(errCode == ERR_OK, "account observer register fail");
        AUDIO_INFO_LOG("account observer register success");
    } else {
        AUDIO_ERR_LOG("account observer register already");
    }
}

void AudioPolicyServer::AddAudioServiceOnStart()
{
    AUDIO_INFO_LOG("OnAddSystemAbility audio service start");
    if (!isFirstAudioServiceStart_) {
        RegisterParamCallback();
        ConnectServiceAdapter();
        LoadEffectLibrary();
        isFirstAudioServiceStart_ = true;
    } else {
        AUDIO_WARNING_LOG("OnAddSystemAbility audio service is not first start");
    }
}

void AudioPolicyServer::AddRemoteDevstatusCallback()
{
    AUDIO_INFO_LOG("add remote dev status callback start");
    audioPolicyService_.RegisterRemoteDevStatusCallback();
}

void AudioPolicyServer::SubscribePowerStateChangeEvents()
{
    sptr<PowerMgr::IPowerStateCallback> powerStateCallback_;

    if (powerStateCallback_ == nullptr) {
        powerStateCallback_ = new (std::nothrow) AudioPolicyServerPowerStateCallback(this);
    }

    if (powerStateCallback_ == nullptr) {
        AUDIO_ERR_LOG("subscribe create power state callback Create Error");
        return;
    }

    WatchTimeout guard("PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback:AddRemoteDevstatus");
    bool RegisterSuccess = PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback(powerStateCallback_,
        false);
    guard.CheckCurrTimeout();
    if (!RegisterSuccess) {
        AUDIO_ERR_LOG("register power state callback failed");
    } else {
        AUDIO_INFO_LOG("register power state callback success");
        powerStateCallbackRegister_ = true;
    }
}

void AudioCommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    if (eventReceiver_ == nullptr) {
        AUDIO_ERR_LOG("eventReceiver_ is nullptr");
        return;
    }
    eventReceiver_(eventData);
}

void AudioPolicyServer::SubscribeBackgroundTask()
{
    AUDIO_INFO_LOG("In");
    audioBackgroundManager_.SubscribeBackgroundTask();
}

void AudioPolicyServer::SubscribeCommonEventExecute()
{
    if (isAlreadyRegisterCommonEventListener_) {
        AUDIO_INFO_LOG("already registed common event listener, return");
        return;
    }
    SubscribeCommonEvent("usual.event.DATA_SHARE_READY");
    SubscribeCommonEvent("usual.event.dms.rotation_changed");
    SubscribeCommonEvent("usual.event.bluetooth.remotedevice.NAME_UPDATE");
    SubscribeCommonEvent("usual.event.SCREEN_ON");
    SubscribeCommonEvent("usual.event.SCREEN_OFF");
    SubscribeCommonEvent("usual.event.SCREEN_LOCKED");
    SubscribeCommonEvent("usual.event.SCREEN_UNLOCKED");
    SubscribeCommonEvent("usual.event.LOCALE_CHANGED");
#ifdef USB_ENABLE
    usbManager_.SubscribeEvent();
#endif
    SubscribeSafeVolumeEvent();
    isAlreadyRegisterCommonEventListener_ = true;
}

void AudioPolicyServer::SubscribeCommonEvent(const std::string event)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    if (event == "usual.event.dms.rotation_changed") {
        subscribeInfo.SetPermission("ohos.permission.PUBLISH_DISPLAY_ROTATION_EVENT");
    }
    auto commonSubscribePtr = std::make_shared<AudioCommonEventSubscriber>(subscribeInfo,
        std::bind(&AudioPolicyServer::OnReceiveEvent, this, std::placeholders::_1));
    if (commonSubscribePtr == nullptr) {
        AUDIO_ERR_LOG("commonSubscribePtr is nullptr");
        return;
    }
    AUDIO_INFO_LOG("subscribe event: %s action", event.c_str());
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonSubscribePtr);
}

void AudioPolicyServer::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == "usual.event.DATA_SHARE_READY") {
        if (isInitRingtoneReady_ == false) {
            std::thread([&]() { CallRingtoneLibrary(); }).detach();
            isInitRingtoneReady_ = true;
        }
        audioPolicyManager_.SetDataShareReady(true);
        RegisterDataObserver();
        if (isInitMuteState_ == false) {
            AUDIO_INFO_LOG("receive DATA_SHARE_READY action and need init mic mute state");
            InitMicrophoneMute();
        }
        if (isInitSettingsData_ == false) {
            AUDIO_INFO_LOG("First receive DATA_SHARE_READY action and need init SettingsData");
            InitKVStore();
            NotifySettingsDataReady();
            isInitSettingsData_ = true;
        }
        RegisterDefaultVolumeTypeListener();
        SubscribeAccessibilityConfigObserver();
    } else if (action == "usual.event.dms.rotation_changed") {
        uint32_t rotate = static_cast<uint32_t>(want.GetIntParam("rotation", 0));
        AUDIO_INFO_LOG("Set rotation to audioeffectchainmanager is %{public}d", rotate);
        AudioServerProxy::GetInstance().SetRotationToEffectProxy(rotate);
    } else if (action == "usual.event.bluetooth.remotedevice.NAME_UPDATE") {
        std::string deviceName  = want.GetStringParam("remoteName");
        std::string macAddress = want.GetStringParam("deviceAddr");
        eventEntry_->OnReceiveBluetoothEvent(macAddress, deviceName);
    } else if (action == "usual.event.SCREEN_ON") {
        AUDIO_INFO_LOG("receive SCREEN_ON action, control audio focus if need");
        audioDeviceCommon_.SetFirstScreenOn();
        if (powerStateListener_ == nullptr) {
            AUDIO_ERR_LOG("powerStateListener_ is nullptr");
            return;
        }
        powerStateListener_->ControlAudioFocus(false);
    } else if (action == "usual.event.SCREEN_LOCKED") {
        AUDIO_INFO_LOG("receive SCREEN_OFF or SCREEN_LOCKED action, control audio volume change if stream is active");
        isScreenOffOrLock_ = true;
    } else if (action == "usual.event.SCREEN_UNLOCKED") {
        AUDIO_INFO_LOG("receive SCREEN_UNLOCKED action, can change volume");
        isScreenOffOrLock_ = false;
    } else if (action == "usual.event.LOCALE_CHANGED") {
        CallRingtoneLibrary();
    }
}

void AudioPolicyServer::CheckSubscribePowerStateChange()
{
    if (powerStateCallbackRegister_) {
        return;
    }

    SubscribePowerStateChangeEvents();

    if (powerStateCallbackRegister_) {
        AUDIO_DEBUG_LOG("PowerState CallBack Register Success");
    } else {
        AUDIO_ERR_LOG("PowerState CallBack Register Failed");
    }
}

AudioPolicyServer::AudioPolicyServerPowerStateCallback::AudioPolicyServerPowerStateCallback(
    AudioPolicyServer* policyServer) : PowerMgr::PowerStateCallbackStub(), policyServer_(policyServer)
{}

void AudioPolicyServer::AudioPolicyServerPowerStateCallback::OnAsyncPowerStateChanged(PowerMgr::PowerState state)
{
    policyServer_->audioOffloadStream_.HandlePowerStateChanged(state);
}

void AudioPolicyServer::InitKVStore()
{
    audioVolumeManager_.InitKVStore();
}

void AudioPolicyServer::NotifySettingsDataReady()
{
    Trace trace("AudioPolicyServer::NotifySettingsDataReady");
    AUDIO_INFO_LOG("SettingsData Ready");
    AudioServerProxy::GetInstance().NotifySettingsDataReady();
}

void AudioPolicyServer::ConnectServiceAdapter()
{
    if (!eventEntry_->ConnectServiceAdapter()) {
        AUDIO_ERR_LOG("ConnectServiceAdapter Error in connecting to audio service adapter");
        return;
    }
}

void AudioPolicyServer::LoadEffectLibrary()
{
    audioPolicyService_.LoadEffectLibrary();
}

int32_t AudioPolicyServer::GetMaxVolumeLevel(int32_t volumeType, int32_t &volumeLevel, int32_t deviceType)
{
    volumeLevel = audioVolumeManager_.GetMaxVolumeLevel(static_cast<AudioVolumeType>(volumeType),
                                                        static_cast<DeviceType>(deviceType));
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMinVolumeLevel(int32_t volumeType, int32_t &volumeLevel, int32_t deviceType)
{
    volumeLevel = audioVolumeManager_.GetMinVolumeLevel(static_cast<AudioVolumeType>(volumeType),
                                                        static_cast<DeviceType>(deviceType));
    return SUCCESS;
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetSystemVolumeLevelLegacy(int32_t streamTypeIn, int32_t volumeLevel)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    return SetSystemVolumeLevelInternal(streamType, volumeLevel, false, zoneId);
}

int32_t AudioPolicyServer::SetAdjustVolumeForZone(int32_t zoneId)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAppVolumeLevel: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret = audioVolumeManager_.SetAdjustVolumeForZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to SetAdjustVolumeForZone");
    return ret;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAppVolumeLevel: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetAppVolumeMutedInternal(appUid, muted, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, int32_t volumeFlag)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAppVolumeLevel: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    if (!IsVolumeLevelValid(STREAM_APP, volumeLevel)) {
        return ERR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetAppVolumeLevelInternal(appUid, volumeLevel, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::SetAppRingMuted(int32_t appUid, bool muted)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return SetAppRingMutedInternal(appUid, muted);
}

int32_t AudioPolicyServer::SetSystemVolumeLevel(int32_t streamTypeIn, int32_t volumeLevel, int32_t volumeFlag,
    int32_t uid)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetSystemVolumeLevel: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    bool flag = volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    int32_t zoneId = 0;
    if (uid != 0) {
        zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(uid);
        if (zoneId != 0 && AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(zoneId)) {
            return AudioZoneService::GetInstance().SetSystemVolumeLevel(zoneId,
                VolumeUtils::GetVolumeTypeFromStreamType(streamType), volumeLevel, flag);
        }
        return SetSystemVolumeLevelInternal(streamType, volumeLevel, flag, zoneId);
    }
    zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    return SetSystemVolumeLevelInternal(streamType, volumeLevel, flag, zoneId);
}

int32_t AudioPolicyServer::SetSystemVolumeLevelWithDevice(int32_t streamTypeIn, int32_t volumeLevel,
    int32_t deviceTypeIn, int32_t volumeFlag)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    DeviceType deviceType = static_cast<DeviceType>(deviceTypeIn);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetSystemVolumeLevelWithDevice: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetSystemVolumeLevelWithDeviceInternal(streamType, volumeLevel,
        volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI, deviceType);
}

int32_t AudioPolicyServer::GetSystemActiveVolumeType(int32_t clientUid, int32_t &streamType)
{
    streamType = static_cast<int32_t>(GetSystemActiveVolumeTypeInternal(clientUid));
    return SUCCESS;
}

AudioStreamType AudioPolicyServer::GetSystemActiveVolumeTypeInternal(const int32_t clientUid)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return AudioStreamType::STREAM_MUSIC;
    }
    int32_t zoneID = 0;
    AudioStreamType streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(
        GetCurrentStreamInFocus());
    if (clientUid != 0) {
        int32_t streamType = STREAM_DEFAULT;
        GetStreamInFocusByUid(clientUid, zoneID, streamType);
        streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(static_cast<AudioStreamType>(streamType));
    }

    AUDIO_INFO_LOG("Get active volume type success:= %{public}d", streamInFocus);
    return streamInFocus;
}

int32_t AudioPolicyServer::GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    AUDIO_INFO_LOG("GetAppVolumeLevel appUid : %{public}d", appUid);
    if (!PermissionUtil::VerifySystemPermission()) {
        return ERR_PERMISSION_DENIED;
    }
    return GetAppVolumeLevelInternal(appUid, volumeLevel);
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetSelfAppVolumeLevel(int32_t &volumeLevel)
{
    AUDIO_INFO_LOG("enter");
    int32_t appUid = IPCSkeleton::GetCallingUid();
    return GetAppVolumeLevelInternal(appUid, volumeLevel);
}

int32_t AudioPolicyServer::GetSystemVolumeLevel(int32_t streamType, int32_t uid, int32_t &volumeLevel)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    AudioStreamType newStreamType = static_cast<AudioStreamType>(streamType);
    int32_t zoneId = 0;
    if (uid != 0) {
        zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(uid);
        if (zoneId != 0 && AudioZoneService::GetInstance().IsSystemVolumeProxyEnable(zoneId)) {
            volumeLevel = AudioZoneService::GetInstance().GetSystemVolumeLevel(zoneId,
                VolumeUtils::GetVolumeTypeFromStreamType(newStreamType));
            return SUCCESS;
        }
    } else {
        zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    }
    volumeLevel = GetSystemVolumeLevelInternal(newStreamType, zoneId);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel = audioVolumeManager_.GetSystemVolumeLevelNoMuteState(streamType);
    AUDIO_DEBUG_LOG("GetVolumeNoMute streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    return volumeLevel;
}

int32_t AudioPolicyServer::GetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t zoneId)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel =  audioVolumeManager_.GetSystemVolumeLevel(streamType, zoneId);
    AUDIO_DEBUG_LOG("GetVolume streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    return volumeLevel;
}

int32_t AudioPolicyServer::GetAppVolumeLevelInternal(int32_t appUid, int32_t &volumeLevel)
{
    int32_t ret = audioVolumeManager_.GetAppVolumeLevel(appUid, volumeLevel);
    AUDIO_DEBUG_LOG("GetAppVolume appUid[%{public}d],volumeLevel[%{public}d]", appUid, volumeLevel);
    return ret;
}

int32_t AudioPolicyServer::SetLowPowerVolume(int32_t streamId, float volume)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_FOUNDATION_SA && callerUid != UID_RESOURCE_SCHEDULE_SERVICE) {
        AUDIO_ERR_LOG("SetLowPowerVolume callerUid Error: not foundation or resource_schedule_service");
        return ERROR;
    }
    return streamCollector_.SetLowPowerVolume(streamId, volume);
}

int32_t AudioPolicyServer::GetFastStreamInfo(AudioStreamInfo &streamInfo)
{
    streamInfo = audioPolicyService_.GetFastStreamInfo();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetLowPowerVolume(int32_t streamId, float &outVolume)
{
    outVolume = streamCollector_.GetLowPowerVolume(streamId);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetSingleStreamVolume(int32_t streamId, float &outVolume)
{
    outVolume = streamCollector_.GetSingleStreamVolume(streamId);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsVolumeUnadjustable(bool &unadjustable)
{
    unadjustable = audioPolicyManager_.IsVolumeUnadjustable();
    return SUCCESS;
}

bool AudioPolicyServer::CheckCanMuteVolumeTypeByStep(AudioVolumeType volumeType, int32_t volumeLevel)
{
    if ((volumeLevel - volumeStep_) == 0 && !VolumeUtils::IsPCVolumeEnable() && (volumeType == STREAM_VOICE_ASSISTANT
        || volumeType == STREAM_VOICE_CALL || volumeType == STREAM_ALARM || volumeType == STREAM_ACCESSIBILITY ||
        volumeType == STREAM_VOICE_COMMUNICATION)) {
        return false;
    }
    return true;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::AdjustVolumeByStep(int32_t adjustTypeIn)
{
    VolumeAdjustType adjustType = static_cast<VolumeAdjustType>(adjustTypeIn);
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("Uid %{public}d send AdjustVolumeByStep volume key: %{public}s.", callerUid,
        (adjustType == VolumeAdjustType::VOLUME_UP) ? "up" : "down");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    AudioStreamType streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(
        GetCurrentStreamInFocus());
    if (streamInFocus == AudioStreamType::STREAM_DEFAULT) {
        streamInFocus = AudioStreamType::STREAM_MUSIC;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t volumeLevelInInt = 0;
    if (adjustType == VolumeAdjustType::VOLUME_UP && GetStreamMuteInternal(streamInFocus)) {
        SetStreamMuteInternal(streamInFocus, false, false);
        if (!VolumeUtils::IsPCVolumeEnable()) {
            AUDIO_DEBUG_LOG("phone need return");
            return SUCCESS;
        }
    }
    volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus);
    int32_t minRet = -1;
    GetMinVolumeLevel(streamInFocus, minRet);
    int32_t maxRet = -1;
    GetMaxVolumeLevel(streamInFocus, maxRet);
    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        CHECK_AND_RETURN_RET_LOG(volumeLevelInInt < maxRet, ERR_OPERATION_FAILED, "volumeLevelInInt is biggest");
        volumeLevelInInt = volumeLevelInInt + volumeStep_;
    } else {
        if (!CheckCanMuteVolumeTypeByStep(streamInFocus, volumeLevelInInt)) {
            // This type can not set to mute, but don't return error
            AUDIO_INFO_LOG("SetSystemVolumeLevel this type can not set mute");
            return SUCCESS;
        }
        CHECK_AND_RETURN_RET_LOG(volumeLevelInInt > minRet, ERR_OPERATION_FAILED, "volumeLevelInInt is smallest");
        volumeLevelInInt = volumeLevelInInt - volumeStep_;
    }
    volumeLevelInInt = volumeLevelInInt > maxRet ? maxRet :
        volumeLevelInInt;
    volumeLevelInInt = volumeLevelInInt < minRet ? minRet :
        volumeLevelInInt;
    int32_t ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, false);
    return ret;
}

int32_t AudioPolicyServer::AdjustSystemVolumeByStep(int32_t volumeTypeIn, int32_t adjustTypeIn)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(volumeTypeIn);
    VolumeAdjustType adjustType = static_cast<VolumeAdjustType>(adjustTypeIn);
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("Uid %{public}d send AdjustSystemVolumeByStep VolumeType: %{public}d volume key: %{public}s.",
        callerUid, volumeType, (adjustType == VolumeAdjustType::VOLUME_UP) ? "up" : "down");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustSystemVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    if (adjustType == VolumeAdjustType::VOLUME_UP && GetStreamMuteInternal(volumeType)) {
        SetStreamMuteInternal(volumeType, false, false);
        if (!VolumeUtils::IsPCVolumeEnable()) {
            AUDIO_DEBUG_LOG("phone need return");
            return SUCCESS;
        }
    }
    int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(volumeType);
    int32_t minRet = -1;
    GetMinVolumeLevel(volumeType, minRet);
    int32_t maxRet = -1;
    GetMaxVolumeLevel(volumeType, maxRet);
    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        CHECK_AND_RETURN_RET_LOG(volumeLevelInInt < maxRet, ERR_OPERATION_FAILED, "volumeLevelInInt is biggest");
        volumeLevelInInt = volumeLevelInInt + volumeStep_;
    } else {
        if (!CheckCanMuteVolumeTypeByStep(volumeType, volumeLevelInInt)) {
            // This type can not set to mute, but don't return error
            AUDIO_INFO_LOG("SetSystemVolumeLevel this type can not set mute");
            return SUCCESS;
        }
        CHECK_AND_RETURN_RET_LOG(volumeLevelInInt > minRet, ERR_OPERATION_FAILED, "volumeLevelInInt is smallest");
        volumeLevelInInt = volumeLevelInInt - volumeStep_;
    }
    volumeLevelInInt = volumeLevelInInt > maxRet ? maxRet :
        volumeLevelInInt;
    volumeLevelInInt = volumeLevelInInt < minRet ? minRet :
        volumeLevelInInt;
    int32_t ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt, false);
    return ret;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetSystemVolumeInDb(int32_t volumeTypeIn,
    int32_t volumeLevel, int32_t deviceTypeIn, float &volume)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(volumeTypeIn);
    DeviceType deviceType = static_cast<DeviceType>(deviceTypeIn);
    if (!IsVolumeTypeValid(volumeType)) {
        volume = static_cast<float>(ERR_INVALID_PARAM);
        return ERR_INVALID_PARAM;
    }
    if (!IsVolumeLevelValid(volumeType, volumeLevel)) {
        volume = static_cast<float>(ERR_INVALID_PARAM);
        return ERR_INVALID_PARAM;
    }

    volume = audioPolicyManager_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
    return SUCCESS;
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetStreamMuteLegacy(int32_t streamTypeIn, bool mute, int32_t deviceTypeIn)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    DeviceType deviceType = static_cast<DeviceType>(deviceTypeIn);
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    return SetStreamMuteInternal(streamType, mute, false, deviceType, zoneId);
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetStreamMute(int32_t streamTypeIn, bool mute, int32_t deviceTypeIn)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    DeviceType deviceType = static_cast<DeviceType>(deviceTypeIn);
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    return SetStreamMuteInternal(streamType, mute, false, deviceType, zoneId);
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi,
    const DeviceType &deviceType, int32_t zoneId)
{
    AUDIO_INFO_LOG("SetStreamMuteInternal streamType: %{public}d, mute: %{public}d, updateUi: %{public}d",
        streamType, mute, isUpdateUi);

    if (streamType == STREAM_ALL) {
        for (auto audioStreamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetMute of STREAM_ALL for StreamType = %{public}d ", audioStreamType);
            int32_t setResult = SetSingleStreamMute(audioStreamType, mute, isUpdateUi, deviceType, zoneId);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }

    return SetSingleStreamMute(streamType, mute, isUpdateUi, deviceType, zoneId);
}

void AudioPolicyServer::UpdateSystemMuteStateAccordingMusicState(AudioStreamType streamType, bool mute, bool isUpdateUi)
{
    // This function only applies to mute/unmute scenarios where the input type is music on the PC platform
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) != AudioStreamType::STREAM_MUSIC ||
        !VolumeUtils::IsPCVolumeEnable()) {
        return;
    }
    if (mute && !GetStreamMuteInternal(STREAM_SYSTEM)) {
        // If the STREAM_MUSIC wants mute, synchronize the mute STREAM_SYSTEM
        audioVolumeManager_.SetStreamMute(STREAM_SYSTEM, mute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is mute or volume change to 0 and need mute system stream");
    } else if (!mute && GetStreamMuteInternal(STREAM_SYSTEM)) {
        // If you STREAM_MUSIC unmute, you need to determine whether the volume is 0
        // if it is 0, the prompt sound will continue to be mute, and if it is not 0
        // you need to synchronize the unmute prompt sound
        bool isMute = (GetSystemVolumeLevelInternal(STREAM_MUSIC) == 0) ? true : false;
        audioVolumeManager_.SetStreamMute(STREAM_SYSTEM, isMute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is unmute and volume is 0 and need %{public}d system stream", isMute);
    }
}

void AudioPolicyServer::SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType,
    const bool& isUpdateUi, int32_t zoneId)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInternal(streamType, zoneId);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

int32_t AudioPolicyServer::SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi,
    const DeviceType &deviceType, int32_t zoneId)
{
    bool updateRingerMode = false;
    if ((streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == AudioStreamType::STREAM_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone mute state.
        AudioRingerMode currentRingerMode = audioPolicyManager_.GetRingerMode();
        if ((currentRingerMode == RINGER_MODE_NORMAL && mute) || (currentRingerMode != RINGER_MODE_NORMAL && !mute)) {
            // When isUpdateUi is false, the func is called by others. Need to verify permission.
            if (!isUpdateUi && !VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
                AUDIO_ERR_LOG("ACCESS_NOTIFICATION_POLICY_PERMISSION permission denied for ringtone mute state!");
                return ERR_PERMISSION_DENIED;
            }
            updateRingerMode = true;
        }
    }

    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == AudioStreamType::STREAM_SYSTEM &&
        !mute && (GetSystemVolumeLevelNoMuteState(STREAM_MUSIC) == 0 || GetStreamMuteInternal(STREAM_MUSIC, zoneId))) {
        // when music type volume is not mute,system type volume can be muted separately.
        // but when trying to mute system type volume while the volume for music type is mute
        // or volume level is 0,system type volume can not be muted.
        AUDIO_WARNING_LOG("music volume is 0 or mute and no need unmute system stream!");
    } else {
        int32_t result = audioVolumeManager_.SetStreamMute(streamType, mute, STREAM_USAGE_UNKNOWN, deviceType, zoneId);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Fail to set stream mute!");
    }

    if (!mute && GetSystemVolumeLevelInternal(streamType, zoneId) == 0 && !VolumeUtils::IsPCVolumeEnable()) {
        // If mute state is set to false but volume is 0, set volume to 1
        audioVolumeManager_.SetSystemVolumeLevel(streamType, 1, zoneId);
    }
    ProcUpdateRingerModeForMute(updateRingerMode, mute);
    SendMuteKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId);
    UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
    return SUCCESS;
}

void AudioPolicyServer::ProcUpdateRingerModeForMute(bool updateRingerMode, bool mute)
{
    if (updateRingerMode) {
        AudioRingerMode ringerMode = mute ? (supportVibrator_ ? RINGER_MODE_VIBRATE : RINGER_MODE_SILENT) :
            RINGER_MODE_NORMAL;
        if (!supportVibrator_) {
            AUDIO_INFO_LOG("The device does not support vibration");
        }
        AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring mute state", ringerMode);
        // Update ringer mode but no need to update mute state again.
        SetRingerModeInternal(ringerMode, true);
    }
}

float AudioPolicyServer::GetSystemVolumeDb(AudioStreamType streamType)
{
    return audioPolicyManager_.GetSystemVolumeDb(streamType);
}

int32_t AudioPolicyServer::SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag)
{
    AUDIO_INFO_LOG("SetSelfAppVolumeLevel volumeLevel: %{public}d, volumeFlag: %{public}d",
        volumeLevel, volumeFlag);
    if (!IsVolumeLevelValid(STREAM_APP, volumeLevel)) {
        return ERR_INVALID_PARAM;
    }
    int32_t appUid = IPCSkeleton::GetCallingUid();
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetAppVolumeLevelInternal(appUid, volumeLevel, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::SetAppVolumeLevelInternal(int32_t appUid, int32_t volumeLevel, bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetAppVolumeLevelInternal appUid: %{public}d, volumeLevel: %{public}d, updateUi: %{public}d",
        appUid, volumeLevel, isUpdateUi);
    return SetAppSingleStreamVolume(appUid, volumeLevel, isUpdateUi);
}

int32_t AudioPolicyServer::SetAppRingMutedInternal(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("SetAppRingMutedInternal appUid: %{public}d, muted: %{public}d", appUid, muted);
    int32_t ret = audioVolumeManager_.SetAppRingMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to set app ring to mute");
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAppVolumeMutedInternal(int32_t appUid, bool muted, bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetAppVolumeLevelInternal appUid: %{public}d, muted: %{public}d, updateUi: %{public}d",
        appUid, muted, isUpdateUi);
    int32_t ret = audioVolumeManager_.SetAppVolumeMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set App Volume mute");
    return ret;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("IsAppVolumeMute appUid: %{public}d, owned: %{public}d", appUid, owned);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret = audioVolumeManager_.IsAppVolumeMute(appUid, owned, isMute);
    return ret;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, int32_t zoneId)
{
    AUDIO_INFO_LOG("streamType: %{public}d, volumeLevel: %{public}d, updateUi: %{public}d",
        streamType, volumeLevel, isUpdateUi);
    bool adjustable = false;
    IsVolumeUnadjustable(adjustable);
    if (adjustable) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }
    bool mute = GetStreamMuteInternal(streamType, zoneId);
    if (streamType == STREAM_ALL) {
        for (auto audioStreamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetVolume of STREAM_ALL, SteamType = %{public}d, mute = %{public}d, level = %{public}d",
                audioStreamType, mute, volumeLevel);
            int32_t setResult = SetSingleStreamVolume(audioStreamType, volumeLevel, isUpdateUi, mute, zoneId);
            if (setResult != SUCCESS && setResult != ERR_SET_VOL_FAILED_BY_SAFE_VOL &&
                setResult != ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED) {
                return setResult;
            }
        }
        return SUCCESS;
    }
    return SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi, mute, zoneId);
}

int32_t AudioPolicyServer::SetSystemVolumeLevelWithDeviceInternal(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, DeviceType deviceType)
{
    AUDIO_INFO_LOG("%{public}s streamType: %{public}d, volumeLevel: %{public}d, "
        "updateUi: %{public}d, deviceType: %{public}d", __func__, streamType, volumeLevel, isUpdateUi, deviceType);
    bool adjustable = false;
    IsVolumeUnadjustable(adjustable);
    if (adjustable) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }
    return SetSingleStreamVolumeWithDevice(streamType, volumeLevel, isUpdateUi, deviceType);
}

void AudioPolicyServer::SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType,
    const bool& isUpdateUi, int32_t zoneId)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInternal(streamType, zoneId);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    bool ringerModeMute = audioVolumeManager_.IsRingerModeMute();
    if (audioPolicyServerHandler_ != nullptr && ringerModeMute) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

void AudioPolicyServer::UpdateMuteStateAccordingToVolLevel(AudioStreamType streamType, int32_t volumeLevel,
    bool mute, const bool& isUpdateUi, int32_t zoneId)
{
    bool muteStatus = mute;
    if (volumeLevel == 0 && !mute) {
        muteStatus = true;
        audioVolumeManager_.SetStreamMute(streamType, true, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE, zoneId);
    } else if (volumeLevel > 0 && mute) {
        muteStatus = false;
        audioVolumeManager_.SetStreamMute(streamType, false, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE,
            zoneId);
    }
    SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId);
    if (VolumeUtils::IsPCVolumeEnable()) {
        // system mute status should be aligned with music mute status.
        if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC &&
            muteStatus != GetStreamMuteInternal(STREAM_SYSTEM, zoneId)) {
            AUDIO_DEBUG_LOG("set system mute to %{public}d when STREAM_MUSIC.", muteStatus);
            audioVolumeManager_.SetStreamMute(STREAM_SYSTEM, muteStatus, STREAM_USAGE_UNKNOWN,
                DEVICE_TYPE_NONE, zoneId);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, zoneId);
        } else if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_SYSTEM &&
            muteStatus != GetStreamMuteInternal(STREAM_MUSIC, zoneId)) {
            bool isMute = (GetSystemVolumeLevelInternal(STREAM_MUSIC, zoneId) == 0) ? true : false;
            AUDIO_DEBUG_LOG("set system same to music muted or level is zero to %{public}d.", isMute);
            audioVolumeManager_.SetStreamMute(STREAM_SYSTEM, isMute, STREAM_USAGE_UNKNOWN, DEVICE_TYPE_NONE, zoneId);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, zoneId);
        }
    }
}

void AudioPolicyServer::ProcUpdateRingerMode()
{
    int32_t curRingVolumeLevel = GetSystemVolumeLevelNoMuteState(STREAM_RING);
    AudioRingerMode ringerMode = (curRingVolumeLevel > 0) ? RINGER_MODE_NORMAL :
        (supportVibrator_ ? RINGER_MODE_VIBRATE : RINGER_MODE_SILENT);
    if (!supportVibrator_) {
        AUDIO_INFO_LOG("The device does not support vibration");
    }
    AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring volume level", ringerMode);
    // Update ringer mode but no need to update volume again.
    SetRingerModeInternal(ringerMode, true);
}

int32_t AudioPolicyServer::SetAppSingleStreamVolume(int32_t appUid, int32_t volumeLevel, bool isUpdateUi)
{
    int32_t ret = audioPolicyService_.SetAppVolumeLevel(appUid, volumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set App Volume level");

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = STREAM_APP;
    volumeEvent.volume = volumeLevel;
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    volumeEvent.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendAppVolumeChangeCallback(appUid, volumeEvent);
    }
    return ret;
}

int32_t AudioPolicyServer::SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi,
    bool mute, int32_t zoneId)
{
    bool updateRingerMode = false;
    if ((streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == AudioStreamType::STREAM_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone volume level.
        AudioRingerMode currentRingerMode = audioPolicyManager_.GetRingerMode();
        if ((currentRingerMode == RINGER_MODE_NORMAL && volumeLevel == 0) ||
            (currentRingerMode != RINGER_MODE_NORMAL && volumeLevel > 0)) {
            // When isUpdateUi is false, the func is called by others. Need to verify permission.
            if (!isUpdateUi && !VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
                AUDIO_ERR_LOG("ACCESS_NOTIFICATION_POLICY_PERMISSION permission denied for ringtone volume!");
                return ERR_PERMISSION_DENIED;
            }
            updateRingerMode = true;
        }
    }

    int32_t ret = audioVolumeManager_.SetSystemVolumeLevel(streamType, volumeLevel, zoneId);
    if (ret == SUCCESS) {
        std::string currentTime = GetTime();
        int32_t appUid = IPCSkeleton::GetCallingUid();
        AUDIO_INFO_LOG("SetSystemVolumeLevelInfo streamType: %{public}d, volumeLevel: %{public}d,"
            " appUid: %{public}d, setTime: %{public}s",
            streamType, volumeLevel, appUid, currentTime.c_str());
        audioVolumeManager_.SaveSystemVolumeLevelInfo(streamType, volumeLevel, appUid, currentTime);
        if (updateRingerMode) {
            ProcUpdateRingerMode();
        }
        UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute, isUpdateUi, zoneId);
    } else if (ret == ERR_SET_VOL_FAILED_BY_SAFE_VOL) {
        SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId);
        AUDIO_ERR_LOG("fail to set system volume level by safe vol");
    } else {
        AUDIO_ERR_LOG("fail to set system volume level, ret is %{public}d", ret);
    }

    return ret;
}

int32_t AudioPolicyServer::SetSingleStreamVolumeWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, DeviceType deviceType)
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    int32_t ret = SUCCESS;
    if (curOutputDeviceType != deviceType) {
        ret = audioVolumeManager_.SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    } else {
        bool mute = GetStreamMuteInternal(streamType);
        ret = SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi, mute);
    }
    return ret;
}

int32_t AudioPolicyServer::GetStreamMute(int32_t streamTypeIn, bool &mute)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    if (streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) {
        bool ret = VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "GetStreamMute permission denied for stream type : %{public}d", streamType);
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    mute = GetStreamMuteInternal(streamType, zoneId);
    return SUCCESS;
}

bool AudioPolicyServer::GetStreamMuteInternal(AudioStreamType streamType, int32_t zoneId)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    bool isMuted = audioVolumeManager_.GetStreamMute(streamType, zoneId);
    AUDIO_DEBUG_LOG("GetMute streamType[%{public}d],mute[%{public}d]", streamType, isMuted);
    return isMuted;
}

bool AudioPolicyServer::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
        return true;
    }
    if (desc.deviceType_ != DEVICE_TYPE_USB_HEADSET) {
        return false;
    }
    return eventEntry_->IsArmUsbDevice(desc);
}

void AudioPolicyServer::MapExternalToInternalDeviceType(AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ == DEVICE_TYPE_USB_HEADSET || desc.deviceType_ == DEVICE_TYPE_USB_DEVICE) {
        auto item = audioDeviceManager_.FindConnectedDeviceById(desc.deviceId_);
        if (item && IsUsb(item->deviceType_)) {
            desc.deviceType_ = item->deviceType_;
        }
    } else if (desc.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && desc.deviceRole_ == INPUT_DEVICE) {
        desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    } else if (desc.deviceType_ == DEVICE_TYPE_NEARLINK && desc.deviceRole_ == INPUT_DEVICE) {
        desc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    }
    if (desc.deviceType_ == DEVICE_TYPE_NEARLINK && desc.deviceRole_ == INPUT_DEVICE) {
        desc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    }
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SelectOutputDevice(const sptr<AudioRendererFilter> &audioRendererFilter,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "SelectOutputDevice: No system permission");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> targetOutputDevice;
    for (auto desc : audioDeviceDescriptors) {
        std::shared_ptr<AudioDeviceDescriptor> newDeviceDescriptor =
            std::const_pointer_cast<AudioDeviceDescriptor>(desc);
        CHECK_AND_RETURN_RET_LOG(newDeviceDescriptor != nullptr, ERR_MEMORY_ALLOC_FAILED, "memory alloc failed");
        MapExternalToInternalDeviceType(*newDeviceDescriptor);
        targetOutputDevice.push_back(newDeviceDescriptor);
    }

    return eventEntry_->SelectOutputDevice(audioRendererFilter, targetOutputDevice);
}

int32_t AudioPolicyServer::GetSelectedDeviceInfo(int32_t uid, int32_t pid, int32_t streamTypeIn,
    std::string &info)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    info = audioPolicyService_.GetSelectedDeviceInfo(uid, pid, streamType);
    return SUCCESS;
}

int32_t AudioPolicyServer::SelectInputDevice(const sptr<AudioCapturerFilter> &audioCapturerFilter,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "SelectInputDevice: No system permission");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> targetInputDevice;
    for (auto desc : audioDeviceDescriptors) {
        std::shared_ptr<AudioDeviceDescriptor> newDeviceDescriptor =
            std::const_pointer_cast<AudioDeviceDescriptor>(desc);
        CHECK_AND_RETURN_RET_LOG(newDeviceDescriptor != nullptr, ERR_MEMORY_ALLOC_FAILED, "memory alloc failed");
        MapExternalToInternalDeviceType(*newDeviceDescriptor);
        targetInputDevice.push_back(newDeviceDescriptor);
    }

    int32_t ret = eventEntry_->SelectInputDevice(audioCapturerFilter, targetInputDevice);
    return ret;
}

int32_t AudioPolicyServer::ExcludeOutputDevices(int32_t audioDevUsageIn,
    const vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    auto newAudioDeviceDescriptors = audioDeviceDescriptors;
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(audioDevUsageIn);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "No system permission");
    return eventEntry_->ExcludeOutputDevices(audioDevUsage, newAudioDeviceDescriptors);
}

int32_t AudioPolicyServer::UnexcludeOutputDevices(int32_t audioDevUsageIn,
    const vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    auto newAudioDeviceDescriptors = audioDeviceDescriptors;
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(audioDevUsageIn);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "No system permission");

    return audioDeviceLock_.UnexcludeOutputDevices(audioDevUsage, newAudioDeviceDescriptors);
}

int32_t AudioPolicyServer::GetExcludedDevices(int32_t audioDevUsageIn,
    vector<shared_ptr<AudioDeviceDescriptor>> &device)
{
    AudioDeviceUsage audioDevUsage = static_cast<AudioDeviceUsage>(audioDevUsageIn);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "No system permission");

    device = audioDeviceLock_.GetExcludedDevices(audioDevUsage);

    int32_t apiVersion = HasUsbDevice(device) ? GetApiTargetVersion() : 0;
    for (auto &desc : device) {
        CHECK_AND_RETURN_RET_LOG(desc, ERR_MEMORY_ALLOC_FAILED, "nullptr");
        auto clientInfo = std::make_shared<AudioDeviceDescriptor::ClientInfo>(apiVersion);
        desc->SetClientInfo(clientInfo);
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::GetDevices(int32_t deviceFlagIn,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    DeviceFlag deviceFlag = static_cast<DeviceFlag>(deviceFlagIn);
    switch (deviceFlag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("GetDevices: No system permission");
                deviceDescs.clear();
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            break;
    }

    deviceDescs = eventEntry_->GetDevices(deviceFlag);

    int32_t apiVersion = HasUsbDevice(deviceDescs) ? GetApiTargetVersion() : 0;
    for (auto &desc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(desc, ERR_MEMORY_ALLOC_FAILED, "nullptr");
        if (desc->IsAudioDeviceDescriptor()) {
            desc->deviceType_ = desc->MapInternalToExternalDeviceType(apiVersion);
        }
        if (!hasSystemPermission) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetDevicesInner(int32_t deviceFlagIn,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERR_INVALID_OPERATION;
    }
    DeviceFlag deviceFlag = static_cast<DeviceFlag>(deviceFlagIn);
    deviceDescs = audioConnectedDevice_.GetDevicesInner(deviceFlag);

    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::GetOutputDevice(const sptr<AudioRendererFilter> &audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return ERR_INVALID_OPERATION;
    }
    deviceDescs = audioPolicyService_.GetOutputDevice(audioRendererFilter);

    int32_t apiVersion = HasUsbDevice(deviceDescs) ? GetApiTargetVersion() : 0;
    for (auto &desc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(desc, ERR_MEMORY_ALLOC_FAILED, "nullptr");
        if (desc->IsAudioDeviceDescriptor()) {
            desc->deviceType_ = desc->MapInternalToExternalDeviceType(apiVersion);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::GetInputDevice(const sptr<AudioCapturerFilter> &audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return ERR_INVALID_OPERATION;
    }
    deviceDescs = audioPolicyService_.GetInputDevice(audioCapturerFilter);

    int32_t apiVersion = HasUsbDevice(deviceDescs) ? GetApiTargetVersion() : 0;
    for (auto &desc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(desc, ERR_MEMORY_ALLOC_FAILED, "nullptr");
        if (desc->IsAudioDeviceDescriptor()) {
            desc->deviceType_ = desc->MapInternalToExternalDeviceType(apiVersion);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::VerifyVoiceCallPermission(
    uint64_t fullTokenId, Security::AccessToken::AccessTokenID tokenId)
{
    bool hasSystemPermission = TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    CHECK_AND_RETURN_RET_LOG(hasSystemPermission, ERR_PERMISSION_DENIED, "No system permission");

    bool hasRecordVoiceCallPermission = VerifyPermission(RECORD_VOICE_CALL_PERMISSION, tokenId, true);
    CHECK_AND_RETURN_RET_LOG(hasRecordVoiceCallPermission, ERR_PERMISSION_DENIED, "No permission");
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetPreferredOutputDeviceDescriptors(const AudioRendererInfo &rendererInfo,
    bool forceNoBTPermission, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AudioRendererInfo newRendererInfo = rendererInfo;
    deviceDescs = audioDeviceLock_.GetPreferredOutputDeviceDescriptors(newRendererInfo, LOCAL_NETWORK_ID);

    bool hasBTPermission = false;
    if (!forceNoBTPermission) {
        hasBTPermission = VerifyBluetoothPermission();
    }

    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    int32_t apiVersion = HasUsbDevice(deviceDescs) ? GetApiTargetVersion() : 0;
    for (auto &desc : deviceDescs) {
        CHECK_AND_RETURN_RET_LOG(desc, ERR_MEMORY_ALLOC_FAILED, "nullptr");
        desc->descriptorType_ = AudioDeviceDescriptor::AUDIO_DEVICE_DESCRIPTOR;
        if (desc->IsAudioDeviceDescriptor()) {
            desc->deviceType_ = desc->MapInternalToExternalDeviceType(apiVersion);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::GetPreferredInputDeviceDescriptors(const AudioCapturerInfo &captureInfoIn,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AudioCapturerInfo captureInfo = captureInfoIn;
    deviceDescs = eventEntry_->GetPreferredInputDeviceDescriptors(captureInfo);
    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::SetClientCallbacksEnable(int32_t callbackchangeIn, bool enable)
{
    CallbackChange callbackchange = static_cast<CallbackChange>(callbackchangeIn);
    if (audioPolicyServerHandler_ != nullptr) {
        return audioPolicyServerHandler_->SetClientCallbacksEnable(callbackchange, enable);
    } else {
        AUDIO_ERR_LOG("audioPolicyServerHandler_ is nullptr");
        return AUDIO_ERR;
    }
}

int32_t AudioPolicyServer::SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo)
{
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SetCallbackRendererInfo(rendererInfo);
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SetCallbackCapturerInfo(capturerInfo);
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::IsStreamActive(int32_t streamType, bool &active)
{
    active = audioSceneManager_.IsStreamActive(static_cast<AudioStreamType>(streamType));
    return SUCCESS;
}

int32_t AudioPolicyServer::IsStreamActiveByStreamUsage(int32_t streamUsage, bool &active)
{
    int32_t volumeType = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(
        static_cast<StreamUsage>(streamUsage)));
    return IsStreamActive(volumeType, active);
}

int32_t AudioPolicyServer::IsFastPlaybackSupported(const AudioStreamInfo &streamInfo, int32_t usage, bool &support)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.streamUsage = static_cast<StreamUsage>(usage);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc{};
    GetPreferredOutputDeviceDescriptors(rendererInfo, false, desc);
    AudioStreamInfo newStreamInfo = streamInfo;
    support = audioConfigManager_.IsFastStreamSupported(newStreamInfo, desc);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsFastRecordingSupported(const AudioStreamInfo &streamInfo, int32_t source, bool &support)
{
    AudioCapturerInfo capturerInfo = {};
    capturerInfo.sourceType = static_cast<SourceType>(source);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc{};
    GetPreferredInputDeviceDescriptors(capturerInfo, desc);
    AudioStreamInfo newStreamInfo = streamInfo;
    support = audioConfigManager_.IsFastStreamSupported(newStreamInfo, desc);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetDeviceActive(int32_t deviceType, bool active, int32_t uid)
{
    return eventEntry_->SetDeviceActive(static_cast<InternalDeviceType>(deviceType), active, uid);
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetInputDevice(int32_t deviceType, uint32_t sessionID, int32_t sourceType, bool isRunning)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetInputDevice: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    return eventEntry_->SetInputDevice(static_cast<DeviceType>(deviceType), sessionID,
        static_cast<SourceType>(sourceType), isRunning);
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::IsDeviceActive(int32_t deviceType, bool &active)
{
    active = audioActiveDevice_.IsDeviceActive(static_cast<InternalDeviceType>(deviceType));
    return SUCCESS;
}

int32_t AudioPolicyServer::GetActiveOutputDevice(int32_t &deviceType)
{
    deviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetDmDeviceType(uint16_t &deviceType)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("No permission");
        return ERROR;
    }
    deviceType = audioDeviceStatus_.GetDmDeviceType();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetActiveInputDevice(int32_t &deviceType)
{
    deviceType = static_cast<int32_t>(audioActiveDevice_.GetCurrentInputDeviceType());
    return SUCCESS;
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetRingerModeLegacy(int32_t ringMode)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    AudioRingerMode ringModeIn = static_cast<AudioRingerMode>(ringMode);
    AUDIO_INFO_LOG("Set ringer mode to %{public}d, pid : %{public}d", ringModeIn, pid);
    if (!IsRingerModeValid(ringModeIn)) {
        AUDIO_ERR_LOG("The ringerMode is an invalid parameter.");
        return ERR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetRingerModeInner(static_cast<AudioRingerMode>(ringMode));
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetRingerMode(int32_t ringModeIn)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    AudioRingerMode ringMode = static_cast<AudioRingerMode>(ringModeIn);
    AUDIO_INFO_LOG("Set ringer mode to %{public}d, pid : %{public}d", ringMode, pid);
    if (!IsRingerModeValid(ringMode)) {
        AUDIO_ERR_LOG("The ringerMode is an invalid parameter.");
        return ERR_INVALID_PARAM;
    }
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t result = SetRingerModeInner(ringMode);
    if (result == SUCCESS) {
        std::string currentTime = GetTime();
        std::string callerName = AudioBundleManager::GetBundleName();
        if (callerName == "") {
            callerName = CALLER_NAME;
        }
        AUDIO_INFO_LOG("SetRingerModeInfo ringerMode: %{public}d, bundleName: %{public}s, setTime: %{public}s",
            ringMode, callerName.c_str(), currentTime.c_str());
        audioPolicyManager_.SaveRingerModeInfo(ringMode, callerName, currentTime);
    }
    return result;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetRingerModeInner(AudioRingerMode ringMode)
{
    bool isPermissionRequired = false;

    if (ringMode == AudioRingerMode::RINGER_MODE_SILENT) {
        isPermissionRequired = true;
    } else {
        AudioRingerMode currentRingerMode = audioPolicyManager_.GetRingerMode();
        if (currentRingerMode == AudioRingerMode::RINGER_MODE_SILENT) {
            isPermissionRequired = true;
        }
    }

    // only switch to silent need check NOTIFICATION.
    if (isPermissionRequired) {
        bool result = VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(result, ERR_PERMISSION_DENIED,
            "Access policy permission denied for ringerMode : %{public}d", ringMode);
    }

    return SetRingerModeInternal(ringMode);
}

int32_t AudioPolicyServer::SetRingerModeInternal(AudioRingerMode inputRingerMode, bool hasUpdatedVolume)
{
    // PC ringmode not support silent or vibrate
    AudioRingerMode ringerMode = VolumeUtils::IsPCVolumeEnable() ? RINGER_MODE_NORMAL : inputRingerMode;
    AUDIO_INFO_LOG("Set ringer mode to %{public}d. hasUpdatedVolume %{public}d", ringerMode, hasUpdatedVolume);
    int32_t ret = coreService_->SetRingerMode(ringerMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set ringer mode!");

    if (!hasUpdatedVolume) {
        // need to set volume according to ringermode
        bool muteState = (ringerMode == RINGER_MODE_NORMAL) ? false : true;
        AudioInterrupt audioInterrupt;
        int32_t zoneID = 0;
        GetSessionInfoInFocus(audioInterrupt, zoneID);
        audioVolumeManager_.SetStreamMute(STREAM_RING, muteState, audioInterrupt.streamUsage);
        if (!muteState && GetSystemVolumeLevelInternal(STREAM_RING) == 0) {
            // if mute state is false but volume is 0, set volume to 1. Send volumeChange callback.
            SetSystemVolumeLevelInternal(STREAM_RING, 1, false);
        }
    }

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRingerModeUpdatedCallback(ringerMode);
    }
    return ret;
}

#ifdef FEATURE_DTMF_TONE
int32_t AudioPolicyServer::GetToneConfig(int32_t ltonetype, const std::string &countryCode,
    std::shared_ptr<ToneInfo> &config)
{
    config = audioToneManager_.GetToneConfig(ltonetype, countryCode);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetSupportedTones(const std::string &countryCode, std::vector<int32_t> &tones)
{
    tones = audioToneManager_.GetSupportedTones(countryCode);
    return SUCCESS;
}
#endif

void AudioPolicyServer::InitMicrophoneMute()
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (isInitMuteState_) {
        AUDIO_ERR_LOG("mic mutestate has already been initialized");
        return;
    }
    bool isMute = false;
    int32_t ret = audioMicrophoneDescriptor_.InitPersistentMicrophoneMuteState(isMute);
    AUDIO_INFO_LOG("Get persistent mic ismute: %{public}d  state from setting db", isMute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("InitMicrophoneMute InitPersistentMicrophoneMuteState result %{public}d", ret);
        return;
    }
    isInitMuteState_ = true;
    if (audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = isMute;
        audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
    }
}

int32_t AudioPolicyServer::SetMicrophoneMuteCommon(bool isMute, bool isLegacy)
{
    std::lock_guard<std::mutex> lock(micStateChangeMutex_);
    bool originalMicrophoneMute = false;
    IsMicrophoneMute(originalMicrophoneMute);
    int32_t ret = audioMicrophoneDescriptor_.SetMicrophoneMute(isMute);
    bool newMicrophoneMute = false;
    IsMicrophoneMute(newMicrophoneMute);
    if (ret == SUCCESS && originalMicrophoneMute != newMicrophoneMute && audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = newMicrophoneMute;
        AUDIO_INFO_LOG("SendMicStateUpdatedCallback when set common mic mute state:%{public}d, isLegacy:%{public}d",
            newMicrophoneMute, isLegacy);
        audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
    }
    return ret;
}

int32_t AudioPolicyServer::SetMicrophoneMute(bool isMute)
{
    AUDIO_INFO_LOG("[%{public}d] set to %{public}s", IPCSkeleton::GetCallingPid(), (isMute ? "true" : "false"));
    bool ret = VerifyPermission(MICROPHONE_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "MICROPHONE permission denied");
    return SetMicrophoneMuteCommon(isMute, true);
}

int32_t AudioPolicyServer::SetMicrophoneMuteAudioConfig(bool isMute)
{
    AUDIO_INFO_LOG("[%{public}d] set to %{public}s", IPCSkeleton::GetCallingPid(), (isMute ? "true" : "false"));
    const char* MANAGE_AUDIO_CONFIG = "ohos.permission.MANAGE_AUDIO_CONFIG";
    bool ret = VerifyPermission(MANAGE_AUDIO_CONFIG);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "MANAGE_AUDIO_CONFIG permission denied");
    lastMicMuteSettingPid_ = IPCSkeleton::GetCallingPid();
    WatchTimeout guard("PrivacyKit::SetMutePolicy:SetMicrophoneMuteAudioConfig");
    PrivacyKit::SetMutePolicy(POLICY_TYPE_MAP[TEMPORARY_POLCIY_TYPE], MICPHONE_CALLER, isMute,
        IPCSkeleton::GetCallingTokenID());
    guard.CheckCurrTimeout();
    return SetMicrophoneMuteCommon(isMute, false);
}

int32_t AudioPolicyServer::SetMicrophoneMutePersistent(bool isMute, int32_t typeIn)
{
    PolicyType type = static_cast<PolicyType>(typeIn);
    AUDIO_INFO_LOG("Entered %{public}s isMute:%{public}d, type:%{public}d", __func__, isMute, type);
    bool hasPermission = VerifyPermission(MICROPHONE_CONTROL_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(hasPermission, ERR_PERMISSION_DENIED,
        "MICROPHONE_CONTROL_PERMISSION permission denied");
    WatchTimeout guard("PrivacyKit::SetMutePolicy:SetMicrophoneMutePersistent");
    bool originalMicrophoneMute = false;
    IsMicrophoneMute(originalMicrophoneMute);
    int32_t ret = PrivacyKit::SetMutePolicy(POLICY_TYPE_MAP[type], MICPHONE_CALLER, isMute,
        IPCSkeleton::GetCallingTokenID());
    guard.CheckCurrTimeout();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("PrivacyKit SetMutePolicy failed ret is %{public}d", ret);
        return ret;
    }
    ret = audioMicrophoneDescriptor_.SetMicrophoneMutePersistent(isMute);
    bool newMicrophoneMute = false;
    IsMicrophoneMute(newMicrophoneMute);
    if (ret == SUCCESS && originalMicrophoneMute != newMicrophoneMute && audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = newMicrophoneMute;
        AUDIO_INFO_LOG("SendMicStateUpdatedCallback when set persistent mic mute state:%{public}d", newMicrophoneMute);
        audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
    }
    return ret;
}

int32_t AudioPolicyServer::GetPersistentMicMuteState(bool &mute)
{
    bool hasPermission = VerifyPermission(MICROPHONE_CONTROL_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(hasPermission, ERR_PERMISSION_DENIED,
        "MICROPHONE_CONTROL_PERMISSION permission denied");

    mute = audioMicrophoneDescriptor_.GetPersistentMicMuteState();
    return SUCCESS;
}

// deprecated since 9.
int32_t AudioPolicyServer::IsMicrophoneMuteLegacy(bool &mute)
{
    // AudioManager.IsMicrophoneMute check micphone right.
    if (!VerifyPermission(MICROPHONE_PERMISSION)) {
        AUDIO_ERR_LOG("MICROPHONE permission denied");
        mute = false;
        return ERR_PERMISSION_DENIED;
    }
    return IsMicrophoneMute(mute);
}

int32_t AudioPolicyServer::IsMicrophoneMute(bool &mute)
{
    // AudioVolumeGroupManager.IsMicrophoneMute didn't check micphone right.
    mute = audioMicrophoneDescriptor_.IsMicrophoneMute();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetRingerMode(int32_t &ringerMode)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    ringerMode = static_cast<int32_t>(audioPolicyManager_.GetRingerMode());
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetAudioScene(int32_t audioSceneIn)
{
    AudioScene audioScene = static_cast<AudioScene>(audioSceneIn);
    CHECK_AND_RETURN_RET_LOG(audioScene > AUDIO_SCENE_INVALID && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "param is invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_PHONE_CALL:
        case AUDIO_SCENE_PHONE_CHAT:
            return eventEntry_->SetAudioScene(audioScene);

        default:
            AUDIO_ERR_LOG("param is invalid: %{public}d", audioScene);
            return ERR_INVALID_PARAM;
    }
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetAudioSceneInternal(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    return eventEntry_->SetAudioScene(audioScene, uid, pid);
}

int32_t AudioPolicyServer::GetAudioScene(int32_t &scene)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    scene = static_cast<int32_t>(audioSceneManager_.GetAudioScene(hasSystemPermission));
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioInterruptCallback(uint32_t sessionID, const sptr<IRemoteObject> &object,
    uint32_t clientUid, int32_t zoneID)
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("The interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    if (coreService_ == nullptr) {
        AUDIO_ERR_LOG("The coreService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    AUDIO_INFO_LOG("The sessionId %{public}u, callingUid %{public}u, clientUid %{public}u",
        sessionID, callingUid, clientUid);
    if (CALLBACK_TRUST_LIST.count(callingUid) == 0) {
        // Verify whether the clientUid is valid.
        if (callingUid != clientUid) {
            AUDIO_ERR_LOG("The callingUid is not equal to clientUid and is not MEDIA_SERVICE_UID!");
            return ERR_UNKNOWN;
        }
        if (!coreService_->IsStreamBelongToUid(callingUid, sessionID)) {
            AUDIO_ERR_LOG("The sessionId %{public}u does not belong to uid %{public}u!", sessionID, callingUid);
            return ERR_UNKNOWN;
        }
    }

    return interruptService_->SetAudioInterruptCallback(zoneID, sessionID, object, clientUid);
}

int32_t AudioPolicyServer::UnsetAudioInterruptCallback(uint32_t sessionID, int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->UnsetAudioInterruptCallback(zoneID, sessionID);
    }
    return ERR_UNKNOWN;
}

bool AudioPolicyServer::VerifySessionId(uint32_t sessionId, uint32_t clientUid)
{
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    AUDIO_INFO_LOG("The sessionId %{public}u, callingUid %{public}u, clientUid %{public}u",
        sessionId, callingUid, clientUid);
    CHECK_AND_RETURN_RET(CALLBACK_TRUST_LIST.count(callingUid) == 0, true);

    CHECK_AND_RETURN_RET_LOG(callingUid == clientUid, false,
        "The callingUid is not equal to clientUid and is not MEDIA_SERVICE_UID!");
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, false, "coreService_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(coreService_->IsStreamBelongToUid(callingUid, sessionId), false,
        "The sessionId %{public}u does not belong to uid %{public}u!", false, callingUid);
    return true;
}

int32_t AudioPolicyServer::SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object,
    uint32_t clientUid)
{
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_UNKNOWN, "coreService_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_UNKNOWN, "object is nullptr");
    CHECK_AND_RETURN_RET_LOG(VerifySessionId(sessionId, clientUid), ERR_UNKNOWN, "invalid sessionId %{public}u",
        sessionId);
    coreService_->SetAudioRouteCallback(sessionId, object);
    return SUCCESS;
}

int32_t AudioPolicyServer::UnsetAudioRouteCallback(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_UNKNOWN, "coreService_ is nullptr");
    coreService_->UnsetAudioRouteCallback(sessionId);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioManagerInterruptCallback(int32_t /* clientId */,
                                                            const sptr<IRemoteObject> &object)
{
    if (interruptService_ != nullptr) {
        return interruptService_->SetAudioManagerInterruptCallback(object);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::UnsetAudioManagerInterruptCallback(int32_t /* clientId */)
{
    if (interruptService_ != nullptr) {
        return interruptService_->UnsetAudioManagerInterruptCallback();
    }
    return ERR_UNKNOWN;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetQueryClientTypeCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    return audioPolicyService_.SetQueryClientTypeCallback(object);
}

int32_t AudioPolicyServer::SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object)
{
    if (object == nullptr) {
        AUDIO_ERR_LOG("callback is nullptr!");
        return ERR_INVALID_PARAM;
    }
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    AUDIO_INFO_LOG("in!");
    return audioPolicyService_.SetQueryDeviceVolumeBehaviorCallback(object);
}

int32_t AudioPolicyServer::SetAudioClientInfoMgrCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }

    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        return audioStateManager_.SetAudioClientInfoMgrCallback(callback);
    } else {
        AUDIO_ERR_LOG("Client info manager callback is null");
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }

    if (interruptService_ != nullptr) {
        return interruptService_->SetQueryBundleNameListCallback(object);
    }
    return ERR_UNKNOWN;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetAudioVKBInfoMgrCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }

    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        return audioStateManager_.SetAudioVKBInfoMgrCallback(callback);
    } else {
        AUDIO_ERR_LOG("VKB info manager callback is null");
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::CheckVKBInfo(const std::string &bundleName, bool &isValid)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    return audioStateManager_.CheckVKBInfo(bundleName, isValid);
}

int32_t AudioPolicyServer::RequestAudioFocus(int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    if (interruptService_ != nullptr) {
        return interruptService_->RequestAudioFocus(clientId, audioInterrupt);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::AbandonAudioFocus(int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    if (interruptService_ != nullptr) {
        return interruptService_->AbandonAudioFocus(clientId, audioInterrupt);
    }
    return ERR_UNKNOWN;
}

void AudioPolicyServer::ProcessRemoteInterrupt(std::set<int32_t> sessionIds, InterruptEventInternal interruptEvent)
{
    if (interruptService_ != nullptr) {
        interruptService_->ProcessRemoteInterrupt(sessionIds, interruptEvent);
    }
}

std::set<int32_t> AudioPolicyServer::GetStreamIdsForAudioSessionByStreamUsage(
    const int32_t zoneId, const std::set<StreamUsage> &streamUsageSet)
{
    std::set<int32_t> streamIds;
    if (interruptService_ != nullptr) {
        streamIds = interruptService_->GetStreamIdsForAudioSessionByStreamUsage(zoneId, streamUsageSet);
    }

    return streamIds;
}

int32_t AudioPolicyServer::ActivateAudioInterrupt(
    const AudioInterrupt &audioInterruptIn, int32_t zoneID, bool isUpdatedAudioStrategy)
{
    Trace trace("AudioPolicyServer::ActivateAudioInterrupt");
    AudioInterrupt audioInterrupt = audioInterruptIn;
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("The interruptService_ is nullptr");
        return ERR_UNKNOWN;
    }

    auto it = std::find(CAN_MIX_MUTED_STREAM.begin(), CAN_MIX_MUTED_STREAM.end(),
        audioInterrupt.audioFocusType.streamType);
    if (it != CAN_MIX_MUTED_STREAM.end()) {
        AudioStreamType streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(
            audioInterrupt.audioFocusType.streamType);
        int32_t volumeLevel = GetSystemVolumeLevelInternal(streamInFocus);
        if (volumeLevel == 0) {
            audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
        }
    }
    StreamUsage streamUsage = interruptService_->GetAudioSessionStreamUsage(audioInterrupt.pid);
    streamUsage = ((streamUsage != StreamUsage::STREAM_USAGE_INVALID) &&
        interruptService_->IsAudioSessionActivated(audioInterrupt.pid)) ? streamUsage : audioInterrupt.streamUsage;
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZone(audioInterrupt.uid, streamUsage);
    int32_t ret = -1;
    if (StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(audioInterrupt.uid,
        false, audioInterrupt.streamId)) {
        return SUCCESS;
    } else {
        ret = AudioZoneService::GetInstance().ActivateAudioInterrupt(zoneId, audioInterrupt,
            isUpdatedAudioStrategy);
    }
    if ((ret == SUCCESS) && (interruptService_->IsSessionNeedToFetchOutputDevice(IPCSkeleton::GetCallingPid()))) {
        eventEntry_->FetchOutputDeviceAndRoute("ActivateAudioInterrupt",
            AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
    }

    return ret;
}

int32_t AudioPolicyServer::SetAppConcurrencyMode(const int32_t appUid, const int32_t mode)
{
    if (interruptService_ != nullptr) {
        return StandaloneModeManager::GetInstance().SetAppConcurrencyMode(IPCSkeleton::GetCallingPid(), appUid, mode);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::SetAppSlientOnDisplay(const int32_t displayId)
{
    if (interruptService_ != nullptr) {
        return StandaloneModeManager::GetInstance().SetAppSlientOnDisplay(IPCSkeleton::GetCallingPid(), displayId);
    }
    return ERR_UNKNOWN;
}
int32_t AudioPolicyServer::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        int32_t zoneId = AudioZoneService::GetInstance().FindAudioZone(audioInterrupt.uid,
            audioInterrupt.streamUsage);
        StandaloneModeManager::GetInstance().EraseDeactivateAudioStream(audioInterrupt.uid,
            audioInterrupt.streamId);
        return AudioZoneService::GetInstance().DeactivateAudioInterrupt(zoneId, audioInterrupt);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::ActivatePreemptMode()
{
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid != PREEMPT_UID) {
        AUDIO_ERR_LOG("Error callingUid uid: %{public}d", callingUid);
        return ERROR;
    }
    if (interruptService_ != nullptr) {
        return interruptService_->ActivatePreemptMode();
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::DeactivatePreemptMode()
{
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid != PREEMPT_UID) {
        AUDIO_ERR_LOG("Error callingUid uid: %{public}d", callingUid);
        return ERROR;
    }
    if (interruptService_ != nullptr) {
        return interruptService_->DeactivatePreemptMode();
    }
    return ERR_UNKNOWN;
}

void AudioPolicyServer::OnAudioStreamRemoved(const uint64_t sessionID)
{
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is nullptr");
    audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionID, false);
}

AudioStreamType AudioPolicyServer::GetCurrentStreamInFocus()
{
    int32_t streamType = STREAM_DEFAULT;
    GetStreamInFocus(DEFAULT_ZONEID, streamType);
    CHECK_AND_RETURN_RET(audioVolumeManager_.IsNeedForceControlVolumeType(), static_cast<AudioStreamType>(streamType));
    AUDIO_INFO_LOG("force volume type, type:%{public}d", audioVolumeManager_.GetForceControlVolumeType());
    return audioVolumeManager_.GetForceControlVolumeType();
}

int32_t AudioPolicyServer::GetStreamInFocus(int32_t zoneID, int32_t &streamType)
{
    if (interruptService_ != nullptr) {
        streamType = static_cast<int32_t>(interruptService_->GetStreamInFocus(zoneID));
    } else {
        streamType = static_cast<int32_t>(STREAM_MUSIC);
    }
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::GetStreamInFocusByUid(int32_t uid, int32_t zoneID, int32_t &streamType)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        streamType = static_cast<int32_t>(STREAM_MUSIC);
        return SUCCESS;
    }

    if (interruptService_ != nullptr) {
        streamType = static_cast<int32_t>(interruptService_->GetStreamInFocusByUid(uid, zoneID));
        return SUCCESS;
    }
    streamType = static_cast<int32_t>(STREAM_MUSIC);
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->GetSessionInfoInFocus(audioInterrupt, zoneID);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::GetAudioFocusInfoList(std::vector<std::map<AudioInterrupt, int32_t>> &focusInfoList,
    int32_t zoneID)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoListIn;
    if (interruptService_ != nullptr) {
        int32_t ret = interruptService_->GetAudioFocusInfoList(zoneID, focusInfoListIn);
        focusInfoList = ToIpcInterrupts(focusInfoListIn);
        return ret;
    }
    return ERR_UNKNOWN;
}

bool AudioPolicyServer::VerifyPermission(const std::string &permissionName, uint32_t tokenId, bool isRecording)
{
    AUDIO_DEBUG_LOG("Verify permission [%{public}s]", permissionName.c_str());

    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid == UID_AUDIO) {
        return true;
    }

    if (!isRecording) {
#ifdef AUDIO_BUILD_VARIANT_ROOT
        // root user case for auto test
        if (callingUid == ROOT_UID) {
            return true;
        }
#endif
        tokenId = IPCSkeleton::GetCallingTokenID();
    }

    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, "Permission denied [%{public}s]", permissionName.c_str());

    return true;
}

bool AudioPolicyServer::VerifyBluetoothPermission()
{
#ifdef AUDIO_BUILD_VARIANT_ROOT
    // root user case for auto test
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    if (callingUid == ROOT_UID) {
        return true;
    }
#endif
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();

    WatchTimeout guard("VerifyBluetooth");
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, USE_BLUETOOTH_PERMISSION);
    CHECK_AND_RETURN_RET(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED, false);

    return true;
}

void AudioPolicyServer::GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos)
{
    audioPolicyManager_.GetStreamVolumeInfoMap(streamVolumeInfos);
}

int32_t AudioPolicyServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    AUDIO_DEBUG_LOG("Dump Process Invoked");
    std::queue<std::u16string> argQue;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        argQue.push(args[index]);
    }
    std::string dumpString;
    InitPolicyDumpMap();
    ArgInfoDump(dumpString, argQue);

    return write(fd, dumpString.c_str(), dumpString.size());
}

void AudioPolicyServer::InitPolicyDumpMap()
{
    dumpFuncMap[u"-h"] = &AudioPolicyServer::InfoDumpHelp;
    dumpFuncMap[u"-d"] = &AudioPolicyServer::AudioDevicesDump;
    dumpFuncMap[u"-m"] = &AudioPolicyServer::AudioModeDump;
    dumpFuncMap[u"-v"] = &AudioPolicyServer::AudioVolumeDump;
    dumpFuncMap[u"-az"] = &AudioPolicyServer::AudioInterruptZoneDump;
    dumpFuncMap[u"-apc"] = &AudioPolicyServer::AudioPolicyParserDump;
    dumpFuncMap[u"-s"] = &AudioPolicyServer::AudioStreamDump;
    dumpFuncMap[u"-xp"] = &AudioPolicyServer::XmlParsedDataMapDump;
    dumpFuncMap[u"-e"] = &AudioPolicyServer::EffectManagerInfoDump;
    dumpFuncMap[u"-ms"] = &AudioPolicyServer::MicrophoneMuteInfoDump;
    dumpFuncMap[u"-as"] = &AudioPolicyServer::AudioSessionInfoDump;
    dumpFuncMap[u"-ap"] = &AudioPolicyServer::AudioPipeManagerDump;
    dumpFuncMap[u"-sd"] = &AudioPolicyServer::SelectDeviceDump;
}

void AudioPolicyServer::PolicyDataDump(std::string &dumpString)
{
    AudioDevicesDump(dumpString);
    AudioModeDump(dumpString);
    AudioVolumeDump(dumpString);
    AudioInterruptZoneDump(dumpString);
    AudioSessionInfoDump(dumpString);
    AudioPolicyParserDump(dumpString);
    AudioStreamDump(dumpString);
    XmlParsedDataMapDump(dumpString);
    EffectManagerInfoDump(dumpString);
    MicrophoneMuteInfoDump(dumpString);
    AudioPipeManagerDump(dumpString);
    SelectDeviceDump(dumpString);
}

void AudioPolicyServer::AudioDevicesDump(std::string &dumpString)
{
    audioPolicyDump_.DevicesInfoDump(dumpString);
}

void AudioPolicyServer::AudioModeDump(std::string &dumpString)
{
    audioPolicyDump_.AudioModeDump(dumpString);
}

void AudioPolicyServer::AudioInterruptZoneDump(std::string &dumpString)
{
    interruptService_->AudioInterruptZoneDump(dumpString);
}

void AudioPolicyServer::AudioPolicyParserDump(std::string &dumpString)
{
    audioPolicyDump_.AudioPolicyParserDump(dumpString);
}

void AudioPolicyServer::AudioVolumeDump(std::string &dumpString)
{
    audioPolicyDump_.StreamVolumesDump(dumpString);
}

void AudioPolicyServer::AudioStreamDump(std::string &dumpString)
{
    audioPolicyDump_.AudioStreamDump(dumpString);
}

void AudioPolicyServer::XmlParsedDataMapDump(std::string &dumpString)
{
    audioPolicyDump_.XmlParsedDataMapDump(dumpString);
}

void AudioPolicyServer::EffectManagerInfoDump(std::string &dumpString)
{
    audioPolicyDump_.EffectManagerInfoDump(dumpString);
}

void AudioPolicyServer::MicrophoneMuteInfoDump(std::string &dumpString)
{
    audioPolicyDump_.MicrophoneMuteInfoDump(dumpString);
}

void AudioPolicyServer::AudioSessionInfoDump(std::string &dumpString)
{
    interruptService_->AudioSessionInfoDump(dumpString);
}

void AudioPolicyServer::AudioPipeManagerDump(std::string &dumpString)
{
    if (coreService_ != nullptr) {
        coreService_->DumpPipeManager(dumpString);
    }
}

void AudioPolicyServer::SelectDeviceDump(std::string &dumpString)
{
    if (coreService_ != nullptr) {
        coreService_->DumpSelectHistory(dumpString);
    }
}

void AudioPolicyServer::ArgInfoDump(std::string &dumpString, std::queue<std::u16string> &argQue)
{
    dumpString += "AudioPolicyServer Data Dump:\n\n";
    if (argQue.empty()) {
        PolicyDataDump(dumpString);
        return;
    }
    while (!argQue.empty()) {
        std::u16string para = argQue.front();
        if (para == u"-h") {
            dumpString.clear();
            (this->*dumpFuncMap[para])(dumpString);
            return;
        } else if (dumpFuncMap.count(para) == 0) {
            dumpString.clear();
            AppendFormat(dumpString, "Please input correct param:\n");
            InfoDumpHelp(dumpString);
            return;
        } else {
            (this->*dumpFuncMap[para])(dumpString);
        }
        argQue.pop();
    }
}

void AudioPolicyServer::InfoDumpHelp(std::string &dumpString)
{
    AppendFormat(dumpString, "usage:\n");
    AppendFormat(dumpString, "  -h\t\t\t|help text for hidumper audio policy\n");
    AppendFormat(dumpString, "  -d\t\t\t|dump devices info\n");
    AppendFormat(dumpString, "  -m\t\t\t|dump ringer mode and call status\n");
    AppendFormat(dumpString, "  -v\t\t\t|dump stream volume info\n");
    AppendFormat(dumpString, "  -az\t\t\t|dump audio in interrupt zone info\n");
    AppendFormat(dumpString, "  -apc\t\t\t|dump audio policy config xml parser info\n");
    AppendFormat(dumpString, "  -s\t\t\t|dump stream info\n");
    AppendFormat(dumpString, "  -xp\t\t\t|dump xml data map\n");
    AppendFormat(dumpString, "  -e\t\t\t|dump audio effect manager info\n");
    AppendFormat(dumpString, "  -as\t\t\t|dump audio session info\n");
    AppendFormat(dumpString, "  -ap\t\t\t|dump audio pipe manager info\n");
}

int32_t AudioPolicyServer::GetPreferredOutputStreamType(const AudioRendererInfo &rendererInfo, int32_t &streamType)
{
    std::string bundleName = "";
    AudioRendererInfo newRendererInfo = rendererInfo;
    bool isFastControlled = audioPolicyService_.getFastControlParam();
    if (isFastControlled && rendererInfo.rendererFlags == AUDIO_FLAG_MMAP) {
        bundleName = AudioBundleManager::GetBundleName();
        AUDIO_INFO_LOG("bundleName %{public}s", bundleName.c_str());
        streamType = eventEntry_->GetPreferredOutputStreamType(newRendererInfo, bundleName);
    }
    streamType = eventEntry_->GetPreferredOutputStreamType(newRendererInfo, "");
    return SUCCESS;
}

int32_t AudioPolicyServer::GetPreferredInputStreamType(const AudioCapturerInfo &capturerInfo, int32_t &streamType)
{
    AudioCapturerInfo newCapturerInfo = capturerInfo;
    streamType = eventEntry_->GetPreferredInputStreamType(newCapturerInfo);
    return SUCCESS;
}

int32_t AudioPolicyServer::CreateRendererClient(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    uint32_t &flag, uint32_t &sessionId, std::string &networkId)
{
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr && eventEntry_ != nullptr, ERR_NULL_POINTER,
        "Core service not inited");
    bool disableFastStream = coreService_->GetDisableFastStreamParam();
    if (disableFastStream) {
        std::string bundleName = AudioBundleManager::GetBundleName();
        streamDesc->SetBunduleName(bundleName);
    }
    uint32_t flagIn = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkIdIn = LOCAL_NETWORK_ID;
    int32_t ret = eventEntry_->CreateRendererClient(streamDesc, flagIn, sessionId, networkIdIn);
    flag = flagIn;
    networkId = networkIdIn;
    return ret;
}

int32_t AudioPolicyServer::CreateCapturerClient(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    uint32_t &flag, uint32_t &sessionId)
{
    uint32_t flagIn = AUDIO_INPUT_FLAG_NORMAL;
    int32_t ret =  eventEntry_->CreateCapturerClient(streamDesc, flagIn, sessionId);
    flag = flagIn;
    return ret;
}

int32_t AudioPolicyServer::RegisterTracker(int32_t modeIn, const AudioStreamChangeInfo &streamChangeInfoIn,
    const sptr<IRemoteObject> &object)
{
    AudioMode mode = static_cast<AudioMode>(modeIn);
    AudioStreamChangeInfo streamChangeInfo = streamChangeInfoIn;
    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    int appVolume = 0;
    GetAppVolumeLevel(callerUid, appVolume);
    streamChangeInfo.audioRendererChangeInfo.appVolume = appVolume;
    AUDIO_DEBUG_LOG("RegisterTracker: [caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        if (mode == AUDIO_MODE_PLAYBACK) {
            streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioRendererChangeInfo.clientUID);
        } else {
            streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
            streamChangeInfo.audioCapturerChangeInfo.appTokenId = IPCSkeleton::GetCallingTokenID();

            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioCapturerChangeInfo.clientUID);
        }
    }
    RegisterClientDeathRecipient(object, TRACKER_CLIENT);
    int32_t apiVersion = GetApiTargetVersion();
    return eventEntry_->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioPolicyServer::UpdateTracker(int32_t modeIn, const AudioStreamChangeInfo &streamChangeInfoIn)
{
    Trace trace("AudioPolicyServer::UpdateTracker");
    AudioMode mode = static_cast<AudioMode>(modeIn);
    AudioStreamChangeInfo streamChangeInfo = streamChangeInfoIn;

    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    AUDIO_DEBUG_LOG("UpdateTracker: [caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        if (mode == AUDIO_MODE_PLAYBACK) {
            streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioRendererChangeInfo.clientUID);
        } else {
            streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioCapturerChangeInfo.clientUID);
        }
    }
    int appVolume = 0;
    GetAppVolumeLevel(streamChangeInfo.audioRendererChangeInfo.clientUID, appVolume);
    streamChangeInfo.audioRendererChangeInfo.appVolume = appVolume;
    int32_t ret = eventEntry_->UpdateTracker(mode, streamChangeInfo);
    return ret;
}

int32_t AudioPolicyServer::FetchOutputDeviceForTrack(const AudioStreamChangeInfo &streamChangeInfoIn,
    const AudioStreamDeviceChangeReasonExt &reason)
{
    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    AudioStreamChangeInfo streamChangeInfo = streamChangeInfoIn;
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    AUDIO_DEBUG_LOG("[caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
        AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
            streamChangeInfo.audioRendererChangeInfo.clientUID);
    }
    eventEntry_->FetchOutputDeviceForTrack(streamChangeInfo, reason);
    return SUCCESS;
}

int32_t AudioPolicyServer::FetchInputDeviceForTrack(const AudioStreamChangeInfo &streamChangeInfoIn)
{
    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    AudioStreamChangeInfo streamChangeInfo = streamChangeInfoIn;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    AUDIO_DEBUG_LOG("[caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
        AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
            streamChangeInfo.audioCapturerChangeInfo.clientUID);
    }
    eventEntry_->FetchInputDeviceForTrack(streamChangeInfo);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetCurrentRendererChangeInfos(
    std::vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return eventEntry_->GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
}

int32_t AudioPolicyServer::GetCurrentCapturerChangeInfos(
    std::vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return eventEntry_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
}

void AudioPolicyServer::RegisterClientDeathRecipient(const sptr<IRemoteObject> &object, DeathRecipientId id)
{
    AUDIO_DEBUG_LOG("Register clients death recipient!! RecipientId: %{public}d", id);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    CHECK_AND_RETURN_LOG(object != nullptr, "Client proxy obj NULL!!");

    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    if (id == TRACKER_CLIENT && std::find(clientDiedListenerState_.begin(), clientDiedListenerState_.end(), pid)
        != clientDiedListenerState_.end()) {
        AUDIO_INFO_LOG("Tracker has been registered for uid:%{public}d pid:%{public}d!", uid, pid);
        return;
    }
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(pid, uid);
    if (deathRecipient_ != nullptr) {
        if (id == TRACKER_CLIENT) {
            deathRecipient_->SetNotifyCb(
                [this] (pid_t pid, pid_t uid) { this->RegisteredTrackerClientDied(pid, uid); });
        } else {
            AUDIO_PRERELEASE_LOGI("RegisteredStreamListenerClientDied register!!");
            deathRecipient_->SetNotifyCb(
                [this] (pid_t pid, pid_t uid) { this->RegisteredStreamListenerClientDied(pid, uid); });
        }
        bool result = object->AddDeathRecipient(deathRecipient_);
        if (result && id == TRACKER_CLIENT) {
            clientDiedListenerState_.push_back(pid);
        }
        if (!result) {
            AUDIO_WARNING_LOG("failed to add deathRecipient");
        }
    }
}

void AudioPolicyServer::RegisteredTrackerClientDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredTrackerClient died: remove entry, pid %{public}d uid %{public}d", pid, uid);
    audioAffinityManager_.DelSelectCapturerDevice(uid);
    audioAffinityManager_.DelSelectRendererDevice(uid);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    eventEntry_->RegisteredTrackerClientDied(uid, pid);

    auto filter = [&pid](int val) {
        return pid == val;
    };
    clientDiedListenerState_.erase(std::remove_if(clientDiedListenerState_.begin(), clientDiedListenerState_.end(),
        filter), clientDiedListenerState_.end());
}

void AudioPolicyServer::RegisteredStreamListenerClientDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredStreamListenerClient died: remove entry, pid %{public}d uid %{public}d", pid, uid);
    audioAffinityManager_.DelSelectCapturerDevice(uid);
    audioAffinityManager_.DelSelectRendererDevice(uid);
    StandaloneModeManager::GetInstance().ResumeAllStandaloneApp(pid);
    if (pid == lastMicMuteSettingPid_) {
        // The last app with the non-persistent microphone setting died, restore the default non-persistent value
        AUDIO_INFO_LOG("Cliet died and reset non-persist mute state");
        audioMicrophoneDescriptor_.SetMicrophoneMute(false);
    }
    if (interruptService_ != nullptr) {
        int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(uid);
        AUDIO_INFO_LOG("deactivate audio session for pid %{public}d, zoneId %{public}d", pid, zoneId);
        interruptService_->DeactivateAudioSession(zoneId, pid);
    }

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->RemoveAudioPolicyClientProxyMap(pid);
    }

    AudioZoneService::GetInstance().UnRegisterAudioZoneClient(pid);
    AudioZoneService::GetInstance().ReleaseAudioZoneByClientPid(pid);
}

int32_t AudioPolicyServer::ResumeStreamState()
{
    AUDIO_INFO_LOG("AVSession is not alive.");
    return streamCollector_.ResumeStreamState();
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::UpdateStreamState(int32_t clientUid,
    int32_t streamSetStateIn, int32_t streamUsageIn)
{
    StreamSetState streamSetState = static_cast<StreamSetState>(streamSetStateIn);
    StreamUsage streamUsage = static_cast<StreamUsage>(streamUsageIn);

    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by av_session
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_AVSESSION_SERVICE, ERROR,
        "UpdateStreamState callerUid is error: not av_session");

    AUDIO_INFO_LOG("UpdateStreamState::uid:%{public}d streamSetState:%{public}d audioStreamUsage:%{public}d",
        clientUid, streamSetState, streamUsage);
    StreamSetState setState = StreamSetState::STREAM_PAUSE;
    switch (streamSetState) {
        case StreamSetState::STREAM_PAUSE:
            setState = StreamSetState::STREAM_PAUSE;
            break;
        case StreamSetState::STREAM_RESUME:
            setState = StreamSetState::STREAM_RESUME;
            break;
        case StreamSetState::STREAM_MUTE:
            setState = StreamSetState::STREAM_MUTE;
            break;
        case StreamSetState::STREAM_UNMUTE:
            setState = StreamSetState::STREAM_UNMUTE;
            break;
        default:
            AUDIO_INFO_LOG("UpdateStreamState::streamSetState value is error");
            break;
    }
    StreamSetStateEventInternal setStateEvent = {};
    setStateEvent.streamSetState = setState;
    setStateEvent.streamUsage = streamUsage;

    return streamCollector_.UpdateStreamState(clientUid, setStateEvent);
}

int32_t AudioPolicyServer::GetVolumeGroupInfos(const std::string &networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "No system permission");

    infos = eventEntry_->GetVolumeGroupInfos();
    auto filter = [&networkId](const sptr<VolumeGroupInfo>& info) {
        return networkId != info->networkId_;
    };
    infos.erase(std::remove_if(infos.begin(), infos.end(), filter), infos.end());

    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetNetworkIdByGroupId(int32_t groupId, std::string &networkId)
{
    auto volumeGroupInfos = eventEntry_->GetVolumeGroupInfos();

    auto filter = [&groupId](const sptr<VolumeGroupInfo> &info) {
        return groupId != info->volumeGroupId_;
    };
    volumeGroupInfos.erase(std::remove_if(volumeGroupInfos.begin(), volumeGroupInfos.end(), filter),
        volumeGroupInfos.end());
    if (volumeGroupInfos.size() > 0) {
        networkId = volumeGroupInfos[0]->networkId_;
        AUDIO_INFO_LOG("GetNetworkIdByGroupId: get networkId %{public}s.", networkId.c_str());
    } else {
        AUDIO_ERR_LOG("GetNetworkIdByGroupId: has no valid group");
        return ERROR;
    }

    return SUCCESS;
}

AudioPolicyServer::RemoteParameterCallback::RemoteParameterCallback(sptr<AudioPolicyServer> server)
{
    server_ = server;
}

void AudioPolicyServer::RemoteParameterCallback::OnAudioParameterChange(const std::string networkId,
    const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(server_ != nullptr, "AudioPolicyServer is nullptr");
    switch (key) {
        case VOLUME:
            VolumeOnChange(networkId, condition);
            break;
        case INTERRUPT:
            InterruptOnChange(networkId, condition);
            break;
        case PARAM_KEY_STATE:
            StateOnChange(networkId, condition, value);
            break;
        default:
            AUDIO_DEBUG_LOG("[AudioPolicyServer]: No processing");
            break;
    }
}

void AudioPolicyServer::RemoteParameterCallback::VolumeOnChange(const std::string networkId,
    const std::string &condition)
{
    VolumeEvent volumeEvent;
    volumeEvent.networkId = networkId;
    char eventDes[EVENT_DES_SIZE];
    if (sscanf_s(condition.c_str(), "%[^;];AUDIO_STREAM_TYPE=%d;VOLUME_LEVEL=%d;IS_UPDATEUI=%d;VOLUME_GROUP_ID=%d;",
        eventDes, EVENT_DES_SIZE, &(volumeEvent.volumeType), &(volumeEvent.volume), &(volumeEvent.updateUi),
        &(volumeEvent.volumeGroupId)) < PARAMS_VOLUME_NUM) {
        AUDIO_ERR_LOG("[VolumeOnChange]: Failed parse condition");
        return;
    }

    volumeEvent.updateUi = false;
    CHECK_AND_RETURN_LOG(server_->audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is nullptr");
    server_->audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
}

void AudioPolicyServer::RemoteParameterCallback::InterruptOnChange(const std::string networkId,
    const std::string &condition)
{
    AUDIO_INFO_LOG("InterruptOnChange : networkId: %{public}s, condition: %{public}s.", networkId.c_str(),
        condition.c_str());
    char eventDes[EVENT_DES_SIZE];
    InterruptType type = INTERRUPT_TYPE_BEGIN;
    InterruptForceType forceType = INTERRUPT_SHARE;
    InterruptHint hint = INTERRUPT_HINT_NONE;
    int32_t audioCategory = 0;

    int ret = sscanf_s(condition.c_str(), "%[^;];EVENT_TYPE=%d;FORCE_TYPE=%d;HINT_TYPE=%d;AUDIOCATEGORY=%d;",
        eventDes, EVENT_DES_SIZE, &type, &forceType, &hint, &audioCategory);
    CHECK_AND_RETURN_LOG(ret >= PARAMS_INTERRUPT_NUM, "[InterruptOnChange]: Failed parse condition");

    std::set<int32_t> sessionIdMedia = AudioStreamCollector::GetAudioStreamCollector().
        GetSessionIdsOnRemoteDeviceByStreamUsage(StreamUsage::STREAM_USAGE_MUSIC);
    std::set<int32_t> sessionIdMovie = AudioStreamCollector::GetAudioStreamCollector().
        GetSessionIdsOnRemoteDeviceByStreamUsage(StreamUsage::STREAM_USAGE_MOVIE);
    std::set<int32_t> sessionIdGame = AudioStreamCollector::GetAudioStreamCollector().
        GetSessionIdsOnRemoteDeviceByStreamUsage(StreamUsage::STREAM_USAGE_GAME);
    std::set<int32_t> sessionIdAudioBook = AudioStreamCollector::GetAudioStreamCollector().
        GetSessionIdsOnRemoteDeviceByStreamUsage(StreamUsage::STREAM_USAGE_AUDIOBOOK);
    std::set<int32_t> sessionIds = {};
    sessionIds.insert(sessionIdMedia.begin(), sessionIdMedia.end());
    sessionIds.insert(sessionIdMovie.begin(), sessionIdMovie.end());
    sessionIds.insert(sessionIdGame.begin(), sessionIdGame.end());
    sessionIds.insert(sessionIdAudioBook.begin(), sessionIdAudioBook.end());

    const std::set<StreamUsage> streamUsageSet = {
        StreamUsage::STREAM_USAGE_MUSIC,
        StreamUsage::STREAM_USAGE_MOVIE,
        StreamUsage::STREAM_USAGE_GAME,
        StreamUsage::STREAM_USAGE_AUDIOBOOK};

    InterruptEventInternal interruptEvent {type, forceType, hint, 0.2f};
    if (server_ != nullptr) {
        std::set<int32_t> fakeSessionIds = server_->GetStreamIdsForAudioSessionByStreamUsage(0, streamUsageSet);
        sessionIds.insert(fakeSessionIds.begin(), fakeSessionIds.end());
        server_->ProcessRemoteInterrupt(sessionIds, interruptEvent);
    }
}

void AudioPolicyServer::RemoteParameterCallback::StateOnChange(const std::string networkId,
    const std::string &condition, const std::string &value)
{
    char eventDes[EVENT_DES_SIZE];
    char contentDes[ADAPTER_STATE_CONTENT_DES_SIZE];
    int ret = sscanf_s(condition.c_str(), "%[^;];%s", eventDes, EVENT_DES_SIZE, contentDes,
        ADAPTER_STATE_CONTENT_DES_SIZE);
    CHECK_AND_RETURN_LOG(ret >= PARAMS_RENDER_STATE_NUM, "StateOnChange: Failed parse condition");
    CHECK_AND_RETURN_LOG(strcmp(eventDes, "ERR_EVENT") == 0,
        "StateOnChange: Event %{public}s is not supported.", eventDes);

    std::string devTypeKey = "DEVICE_TYPE=";
    std::string contentDesStr = std::string(contentDes);
    auto devTypeKeyPos =  contentDesStr.find(devTypeKey);
    CHECK_AND_RETURN_LOG(devTypeKeyPos != std::string::npos,
        "StateOnChange: Not find daudio device type info, contentDes %{public}s.", contentDesStr.c_str());
    size_t devTypeValPos = devTypeKeyPos + devTypeKey.length();
    CHECK_AND_RETURN_LOG(devTypeValPos < contentDesStr.length(),
        "StateOnChange: Not find daudio device type value, contentDes %{public}s.", contentDesStr.c_str());

    if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_SPK) {
        server_->coreService_->NotifyRemoteRenderState(networkId, contentDesStr, value);
    } else if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_MIC) {
        AUDIO_INFO_LOG("StateOnChange: ERR_EVENT of DAUDIO_DEV_TYPE_MIC.");
    } else {
        AUDIO_ERR_LOG("StateOnChange: Device type is not supported, contentDes %{public}s.", contentDesStr.c_str());
    }
}

void AudioPolicyServer::PerStateChangeCbCustomizeCallback::PermStateChangeCallback(
    Security::AccessToken::PermStateChangeInfo &result)
{
    ready_ = true;
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    int32_t res = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(result.tokenID, hapTokenInfo);
    if (res < 0) {
        AUDIO_ERR_LOG("Call GetHapTokenInfo fail.");
    }

    bool targetMuteState = (result.permStateChangeType > 0) ? false : true;
    int32_t appUid = AudioBundleManager::GetUidByBundleName(hapTokenInfo.bundleName, hapTokenInfo.userID);
    if (appUid < 0) {
        AUDIO_ERR_LOG("fail to get uid.");
    } else {
        int32_t streamSet = server_->audioPolicyService_.SetSourceOutputStreamMute(appUid, targetMuteState);
        if (streamSet > 0) {
            UpdateMicPrivacyByCapturerState(targetMuteState, result.tokenID, appUid);
        }
    }
}

void AudioPolicyServer::PerStateChangeCbCustomizeCallback::UpdateMicPrivacyByCapturerState(
    bool targetMuteState, uint32_t targetTokenId, int32_t appUid)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    server_->audioPolicyService_.GetCurrentCapturerChangeInfos(capturerChangeInfos, true, true);
    for (auto &info : capturerChangeInfos) {
        if (info->appTokenId == targetTokenId && info->capturerState == CAPTURER_RUNNING) {
            AUDIO_INFO_LOG("update using mic %{public}d for uid: %{public}d because permission changed",
                targetMuteState, appUid);
            int32_t res = SUCCESS;
            if (targetMuteState) {
                res = PermissionUtil::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
            } else {
                res = PermissionUtil::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
            }
            if (res != SUCCESS) {
                AUDIO_ERR_LOG("update using permission failed, error code %{public}d", res);
            }
        }
    }
}

void AudioPolicyServer::RegisterParamCallback()
{
    AUDIO_INFO_LOG("RegisterParamCallback");
    remoteParameterCallback_ = std::make_shared<RemoteParameterCallback>(this);
    audioPolicyService_.SetParameterCallback(remoteParameterCallback_);
    // regiest policy provider in audio server
    audioPolicyService_.RegiestPolicy();
    eventEntry_->RegistCoreService();
}

void AudioPolicyServer::RegisterBluetoothListener()
{
    AUDIO_INFO_LOG("OnAddSystemAbility bluetooth service start");
    coreService_->RegisterBluetoothListener();
}

void AudioPolicyServer::SubscribeAccessibilityConfigObserver()
{
    AUDIO_INFO_LOG("SubscribeAccessibilityConfigObserver");
    audioPolicyService_.SubscribeAccessibilityConfigObserver();
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    AUDIO_INFO_LOG("key: %{public}s, uri: %{public}s", key.c_str(), uri.c_str());
    return audioPolicyManager_.SetSystemSoundUri(key, uri);
}

int32_t AudioPolicyServer::GetSystemSoundUri(const std::string &key, std::string &uri)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        uri = "";
        return ERR_PERMISSION_DENIED;
    }
    AUDIO_INFO_LOG("key: %{public}s", key.c_str());
    uri = audioPolicyManager_.GetSystemSoundUri(key);
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetMinStreamVolume(float &volume)
{
    volume = audioPolicyManager_.GetMinStreamVolume();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMaxStreamVolume(float &volume)
{
    volume = audioPolicyManager_.GetMaxStreamVolume();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMaxRendererInstances(int32_t &ret)
{
    AUDIO_INFO_LOG("GetMaxRendererInstances");
    int32_t retryCount = 20; // 20 * 200000us = 4s, wait up to 4s
    while (!isFirstAudioServiceStart_) {
        retryCount--;
        if (retryCount > 0) {
            AUDIO_WARNING_LOG("Audio server is not start");
            usleep(200000); // Wait 200000us when audio server is not started
        } else {
            break;
        }
    }
    ret = audioPolicyService_.GetMaxRendererInstances();
    return SUCCESS;
}

void AudioPolicyServer::RegisterDataObserver()
{
    audioPolicyService_.RegisterDataObserver();
}

int32_t AudioPolicyServer::QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    audioPolicyService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    int32_t &ret)
{
    MapExternalToInternalDeviceType(*desc);
    ret = audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId,
    vector<sptr<MicrophoneDescriptor>> &micDescs)
{
    micDescs = coreService_->GetAudioCapturerMicrophoneDescriptors(sessionId);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAvailableMicrophones(std::vector<sptr<MicrophoneDescriptor>> &retMicList)
{
    retMicList = eventEntry_->GetAvailableMicrophones();
    return SUCCESS;
}

int32_t AudioPolicyServer::SetDeviceAbsVolumeSupported(const std::string &macAddress, bool support)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_BLUETOOTH_SA) {
        AUDIO_ERR_LOG("SetDeviceAbsVolumeSupported: Error caller uid: %{public}d", callerUid);
        return ERROR;
    }
    return audioVolumeManager_.SetDeviceAbsVolumeSupported(macAddress, support);
}

int32_t AudioPolicyServer::IsAbsVolumeScene(bool &ret)
{
    ret = audioPolicyManager_.IsAbsVolumeScene();
    return SUCCESS;
}

int32_t AudioPolicyServer::SetA2dpDeviceVolume(const std::string &macAddress, int32_t volume,
    bool updateUi)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_BLUETOOTH_SA) {
        AUDIO_ERR_LOG("SetA2dpDeviceVolume: Error caller uid: %{public}d", callerUid);
        return ERROR;
    }

    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type

    if (!IsVolumeLevelValid(streamInFocus, volume)) {
        return ERR_NOT_SUPPORTED;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t ret = audioVolumeManager_.SetA2dpDeviceVolume(macAddress, volume);

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamInFocus;
    volumeEvent.volume = volume;
    volumeEvent.updateUi = updateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (ret == SUCCESS && audioPolicyServerHandler_ != nullptr &&
        audioPolicyManager_.GetActiveDevice() == DEVICE_TYPE_BLUETOOTH_A2DP) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
    return ret;
}

int32_t AudioPolicyServer::SetNearlinkDeviceVolume(const std::string &macAddress, int32_t streamTypeIn,
    int32_t volume, bool updateUi)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    std::vector<uid_t> allowedUids = { UID_NEARLINK_SA };
    bool ret = PermissionUtil::CheckCallingUidPermission(allowedUids);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "Uid Check Failed");

    CHECK_AND_RETURN_RET_LOG(IsVolumeLevelValid(streamType, volume), ERR_NOT_SUPPORTED,
        "Error volume level: %{public}d", volume);

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    if (streamType == STREAM_MUSIC) {
        audioPolicyManager_.SetSleVoiceStatusFlag(false);
        int32_t result = audioVolumeManager_.SetNearlinkDeviceVolume(macAddress, streamType, volume);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Set volume failed");

        VolumeEvent volumeEvent = VolumeEvent(streamType, volume, updateUi);

        CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is nullptr");
        if (audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_NEARLINK) {
            audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
        }
    } else {
        audioPolicyManager_.SetSleVoiceStatusFlag(true);
        return SetSystemVolumeLevelWithDeviceInternal(streamType, volume, updateUi, DEVICE_TYPE_NEARLINK);
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::GetAvailableDevices(int32_t usageIn,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(usageIn);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
        case MEDIA_INPUT_DEVICES:
        case ALL_MEDIA_DEVICES:
        case CALL_OUTPUT_DEVICES:
        case CALL_INPUT_DEVICES:
        case ALL_CALL_DEVICES:
        case D_ALL_DEVICES:
            break;
        default:
            AUDIO_ERR_LOG("Invalid device usage:%{public}d", usage);
            return ERR_INVALID_PARAM;
    }

    descs = eventEntry_->GetAvailableDevices(usage);

    if (!hasSystemPermission) {
        for (auto &desc : descs) {
            CHECK_AND_CONTINUE(desc != nullptr);
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDevices = {};
    for (auto &desc : descs) {
        deviceDevices.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
    }

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDevices);
        descs.clear();
        for (auto &dec : deviceDevices) {
            CHECK_AND_CONTINUE(dec != nullptr);
            descs.push_back(make_shared<AudioDeviceDescriptor>(*dec));
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::SetAvailableDeviceChangeCallback(int32_t /*clientId*/, int32_t usageIn,
    const sptr<IRemoteObject> &object)
{
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(usageIn);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetAvailableDeviceChangeCallback set listener object is nullptr");
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
        case MEDIA_INPUT_DEVICES:
        case ALL_MEDIA_DEVICES:
        case CALL_OUTPUT_DEVICES:
        case CALL_INPUT_DEVICES:
        case ALL_CALL_DEVICES:
        case D_ALL_DEVICES:
            break;
        default:
            AUDIO_ERR_LOG("Invalid AudioDeviceUsage");
            return ERR_INVALID_PARAM;
    }

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    bool hasBTPermission = VerifyBluetoothPermission();
    return audioPolicyService_.SetAvailableDeviceChangeCallback(clientPid, usage, object, hasBTPermission);
}

int32_t AudioPolicyServer::UnsetAvailableDeviceChangeCallback(int32_t /*clientId*/, int32_t usageIn)
{
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(usageIn);
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("Start");
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->RemoveAvailableDeviceChangeMap(clientPid, usage);
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::OffloadStopPlaying(const AudioInterrupt &audioInterrupt)
{
    return audioPolicyService_.OffloadStopPlaying(std::vector<int32_t>(1, audioInterrupt.streamId));
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> &descriptor, int32_t typeIn)
{
    CastType type = static_cast<CastType>(typeIn);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> lock(descLock_);
    coreService_->ConfigDistributedRoutingRole(descriptor, type);
    OnDistributedRoutingRoleChange(descriptor, type);
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetDistributedRoutingRoleCallback set listener object is nullptr");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    sptr<IStandardAudioRoutingManagerListener> listener = iface_cast<IStandardAudioRoutingManagerListener>(object);
    if (listener != nullptr && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->AddDistributedRoutingRoleChangeCbsMap(clientPid, listener);
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::UnsetDistributedRoutingRoleCallback()
{
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (audioPolicyServerHandler_ != nullptr) {
        return audioPolicyServerHandler_->RemoveDistributedRoutingRoleChangeCbsMap(clientPid);
    }
    return SUCCESS;
}

void AudioPolicyServer::OnDistributedRoutingRoleChange(const std::shared_ptr<AudioDeviceDescriptor> descriptor,
    const CastType type)
{
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is nullptr");
    audioPolicyServerHandler_->SendDistributedRoutingRoleChange(descriptor, type);
}

void AudioPolicyServer::RegisterPowerStateListener()
{
    if (powerStateListener_ == nullptr) {
        powerStateListener_ = new (std::nothrow) PowerStateListener(this);
    }

    if (powerStateListener_ == nullptr) {
        AUDIO_ERR_LOG("create power state listener failed");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    WatchTimeout guard("powerMgrClient.RegisterSyncSleepCallback:RegisterPowerStateListener");
    bool ret = powerMgrClient.RegisterSyncSleepCallback(powerStateListener_, SleepPriority::HIGH);
    guard.CheckCurrTimeout();
    if (!ret) {
        AUDIO_ERR_LOG("register sync sleep callback failed");
    } else {
        AUDIO_INFO_LOG("register sync sleep callback success");
    }
}

void AudioPolicyServer::UnRegisterPowerStateListener()
{
    if (powerStateListener_ == nullptr) {
        AUDIO_ERR_LOG("power state listener is null");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    WatchTimeout guard("powerMgrClient.UnRegisterSyncSleepCallback:UnRegisterPowerStateListener");
    bool ret = powerMgrClient.UnRegisterSyncSleepCallback(powerStateListener_);
    guard.CheckCurrTimeout();
    if (!ret) {
        AUDIO_WARNING_LOG("unregister sync sleep callback failed");
    } else {
        powerStateListener_ = nullptr;
        AUDIO_INFO_LOG("unregister sync sleep callback success");
    }
}

void AudioPolicyServer::RegisterAppStateListener()
{
    AUDIO_INFO_LOG("OnAddSystemAbility app manager service start");
    if (appStateListener_ == nullptr) {
        appStateListener_ = new(std::nothrow) AppStateListener();
    }

    if (appStateListener_ == nullptr) {
        AUDIO_ERR_LOG("create app state listener failed");
        return;
    }

    if (appManager_.RegisterAppStateCallback(appStateListener_) != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        AUDIO_ERR_LOG("register app state callback failed");
    }
}

void AudioPolicyServer::RegisterSyncHibernateListener()
{
    if (syncHibernateListener_ == nullptr) {
        syncHibernateListener_ = new (std::nothrow) SyncHibernateListener(this);
    }

    if (syncHibernateListener_ == nullptr) {
        AUDIO_ERR_LOG("create sync hibernate listener failed");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    WatchTimeout guard("powerMgrClient.RegisterSyncHibernateCallback:RegisterSyncHibernateListener");
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(syncHibernateListener_);
    guard.CheckCurrTimeout();
    if (!ret) {
        AUDIO_ERR_LOG("register sync hibernate callback failed");
    } else {
        AUDIO_INFO_LOG("register sync hibernate callback success");
    }
}

void AudioPolicyServer::UnRegisterSyncHibernateListener()
{
    if (syncHibernateListener_ == nullptr) {
        AUDIO_ERR_LOG("sync hibernate listener is null");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    WatchTimeout guard("powerMgrClient.UnRegisterSyncHibernateCallback:UnRegisterSyncHibernateListener");
    bool ret = powerMgrClient.UnRegisterSyncHibernateCallback(syncHibernateListener_);
    guard.CheckCurrTimeout();
    if (!ret) {
        AUDIO_WARNING_LOG("unregister sync hibernate callback failed");
    } else {
        syncHibernateListener_ = nullptr;
        AUDIO_INFO_LOG("unregister sync hibernate callback success");
    }
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::IsSpatializationEnabled(bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        ret = false;
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsSpatializationEnabled();
    return SUCCESS;
}

int32_t AudioPolicyServer::IsSpatializationEnabled(const std::string &address, bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    ret = audioSpatializationService_.IsSpatializationEnabled(address);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsSpatializationEnabledForCurrentDevice(bool &ret)
{
    ret = audioSpatializationService_.IsSpatializationEnabledForCurrentDevice();
    return SUCCESS;
}

int32_t AudioPolicyServer::SetSpatializationEnabled(bool enable)
{
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetSpatializationEnabled(enable);
}

int32_t AudioPolicyServer::SetSpatializationEnabled(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice,
    bool enable)
{
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetSpatializationEnabled(selectedAudioDevice, enable);
}

int32_t AudioPolicyServer::IsHeadTrackingEnabled(bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsHeadTrackingEnabled();
    return SUCCESS;
}

int32_t AudioPolicyServer::IsHeadTrackingEnabled(const std::string &address, bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsHeadTrackingEnabled(address);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetHeadTrackingEnabled(bool enable)
{
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetHeadTrackingEnabled(enable);
}

int32_t AudioPolicyServer::SetHeadTrackingEnabled(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice,
    bool enable)
{
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

int32_t AudioPolicyServer::GetSpatializationState(int32_t streamUsage,
    AudioSpatializationState &state)
{
    state = audioSpatializationService_.GetSpatializationState(static_cast<StreamUsage>(streamUsage));
    return SUCCESS;
}

int32_t AudioPolicyServer::IsSpatializationSupported(bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsSpatializationSupported();
    return SUCCESS;
}

int32_t AudioPolicyServer::IsSpatializationSupportedForDevice(const std::string &address, bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsSpatializationSupportedForDevice(address);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsHeadTrackingSupported(bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsHeadTrackingSupported();
    return SUCCESS;
}

int32_t AudioPolicyServer::IsHeadTrackingSupportedForDevice(const std::string &address, bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    ret = audioSpatializationService_.IsHeadTrackingSupportedForDevice(address);
    return SUCCESS;
}

int32_t AudioPolicyServer::UpdateSpatialDeviceState(const AudioSpatialDeviceState &audioSpatialDeviceState)
{
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.UpdateSpatialDeviceState(audioSpatialDeviceState);
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::RegisterSpatializationStateEventListener(uint32_t sessionID,
    int32_t streamUsageIn, const sptr<IRemoteObject> &object)
{
    StreamUsage streamUsage = static_cast<StreamUsage>(streamUsageIn);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "obj is null");
    return audioSpatializationService_.RegisterSpatializationStateEventListener(sessionID, streamUsage, object);
}

int32_t AudioPolicyServer::UnregisterSpatializationStateEventListener(uint32_t sessionID)
{
    return audioSpatializationService_.UnregisterSpatializationStateEventListener(sessionID);
}

int32_t AudioPolicyServer::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneID)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener object is nullptr");

    sptr<IAudioPolicyClient> audioPolicyClient = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(audioPolicyClient != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener obj cast failed");

    auto callback = std::make_shared<AudioPolicyClientHolder>(audioPolicyClient);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Create AudioPolicyClientHolder failed");

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_DEBUG_LOG("register clientPid: %{public}d", clientPid);

    bool hasBTPermission = VerifyBluetoothPermission();
    bool hasSysPermission = PermissionUtil::VerifySystemPermission();
    callback->hasBTPermission_ = hasBTPermission;
    callback->hasSystemPermission_ = hasSysPermission;
    callback->apiVersion_ = GetApiTargetVersion();
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->AddAudioPolicyClientProxyMap(clientPid, callback);
    }

    RegisterClientDeathRecipient(object, LISTENER_CLIENT);
    return SUCCESS;
}

int32_t AudioPolicyServer::CreateAudioInterruptZone(const std::set<int32_t> &pids, const int32_t zoneID)
{
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::AddAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
{
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::RemoveAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
{
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::ReleaseAudioInterruptZone(const int32_t zoneID)
{
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::RegisterAudioZoneClient(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "RegisterAudioZoneClient listener object is nullptr");

    sptr<IStandardAudioZoneClient> client = iface_cast<IStandardAudioZoneClient>(object);
    CHECK_AND_RETURN_RET_LOG(client != nullptr, ERR_INVALID_PARAM,
        "RegisterAudioZoneClient listener obj cast failed");

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_DEBUG_LOG("register clientPid: %{public}d", clientPid);

    AudioZoneService::GetInstance().RegisterAudioZoneClient(clientPid, client);

    RegisterClientDeathRecipient(object, LISTENER_CLIENT);
    return SUCCESS;
}

int32_t AudioPolicyServer::CreateAudioZone(const std::string &name, const AudioZoneContext &context, int32_t &zoneId,
    int32_t pid)
{
    CHECK_AND_RETURN_RET_LOG(!name.empty(), ERR_INVALID_PARAM, "audio zone name is empty");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    zoneId = AudioZoneService::GetInstance().CreateAudioZone(name, context, static_cast<pid_t>(pid));
    return SUCCESS;
}

int32_t AudioPolicyServer::ReleaseAudioZone(int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAllAudioZone(std::vector<std::shared_ptr<AudioZoneDescriptor>> &descs)
{
    descs = AudioZoneService::GetInstance().GetAllAudioZone();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAudioZone(int32_t zoneId, std::shared_ptr<AudioZoneDescriptor> &desc)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    desc = AudioZoneService::GetInstance().GetAudioZone(zoneId);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NULL_POINTER, "desc is nullptr");
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAudioZoneByName(const std::string &name, int32_t &zoneId)
{
    CHECK_AND_RETURN_RET_LOG(!name.empty(), ERR_INVALID_PARAM, "audio zone name is empty");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    zoneId = AudioZoneService::GetInstance().GetAudioZoneByName(name);
    return SUCCESS;
}

int32_t AudioPolicyServer::BindDeviceToAudioZone(int32_t zoneId,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    size_t size = devices.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size < MAX_SIZE, ERR_INVALID_PARAM, "invalid device size: %{public}zu", size);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId, devices);
}

int32_t AudioPolicyServer::UnBindDeviceToAudioZone(int32_t zoneId,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    size_t size = devices.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size < MAX_SIZE, ERR_INVALID_PARAM, "invalid device size: %{public}zu", size);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().UnBindDeviceToAudioZone(zoneId, devices);
}

int32_t AudioPolicyServer::EnableAudioZoneReport(bool enable)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return AudioZoneService::GetInstance().EnableAudioZoneReport(clientPid, enable);
}

int32_t AudioPolicyServer::EnableAudioZoneChangeReport(int32_t zoneId, bool enable)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return AudioZoneService::GetInstance().EnableAudioZoneChangeReport(clientPid, zoneId, enable);
}

int32_t AudioPolicyServer::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, uid);
}

int32_t AudioPolicyServer::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().RemoveUidFromAudioZone(zoneId, uid);
}

int32_t AudioPolicyServer::AddStreamToAudioZone(int32_t zoneId, const AudioZoneStream &stream)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().AddStreamToAudioZone(zoneId, stream);
}

int32_t AudioPolicyServer::AddStreamsToAudioZone(int32_t zoneId, const std::vector<AudioZoneStream> &streams)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().AddStreamsToAudioZone(zoneId, streams);
}

int32_t AudioPolicyServer::RemoveStreamFromAudioZone(int32_t zoneId, const AudioZoneStream &stream)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().RemoveStreamFromAudioZone(zoneId, stream);
}

int32_t AudioPolicyServer::RemoveStreamsFromAudioZone(int32_t zoneId, const std::vector<AudioZoneStream> &streams)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    return AudioZoneService::GetInstance().RemoveStreamsFromAudioZone(zoneId, streams);
}

int32_t AudioPolicyServer::SetZoneDeviceVisible(bool visible)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    AudioZoneService::GetInstance().SetZoneDeviceVisible(visible);
    return SUCCESS;
}

int32_t AudioPolicyServer::EnableSystemVolumeProxy(int32_t zoneId, bool enable)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return AudioZoneService::GetInstance().EnableSystemVolumeProxy(clientPid, zoneId, enable);
}

int32_t AudioPolicyServer::GetAudioInterruptForZone(int32_t zoneId,
    std::vector<std::map<AudioInterrupt, int32_t>> &retList)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    retList = ToIpcInterrupts(AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId));
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAudioInterruptForZone(int32_t zoneId, const std::string &deviceTag,
    std::vector<std::map<AudioInterrupt, int32_t>> &retList)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    retList = ToIpcInterrupts(AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId, deviceTag));
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::EnableAudioZoneInterruptReport(int32_t zoneId, const std::string &deviceTag, bool enable)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return AudioZoneService::GetInstance().EnableAudioZoneInterruptReport(clientPid, zoneId, deviceTag, enable);
}

int32_t AudioPolicyServer::InjectInterruptToAudioZone(int32_t zoneId,
    const std::vector<std::map<AudioInterrupt, int32_t>> &interruptsIn)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    size_t size = interruptsIn.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size < MAX_SIZE, ERR_INVALID_PARAM,
        "invalid interrupt size: %{public}zu", size);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    auto interrupts = FromIpcInterrupts(interruptsIn);
    return AudioZoneService::GetInstance().InjectInterruptToAudioZone(zoneId, interrupts);
}

int32_t AudioPolicyServer::InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
    const std::vector<std::map<AudioInterrupt, int32_t>> &interruptsIn)
{
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "audio zone id is invalid");
    size_t size = interruptsIn.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size < MAX_SIZE, ERR_INVALID_PARAM,
        "invalid interrupt size: %{public}zu", size);
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "no system permission");
    auto interrupts = FromIpcInterrupts(interruptsIn);
    return AudioZoneService::GetInstance().InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
}

int32_t AudioPolicyServer::SetCallDeviceActive(int32_t deviceTypeIn, bool active,
    const std::string &address, int32_t uid)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(deviceTypeIn);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_SYSTEM_PERMISSION_DENIED;
    }
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            break;
        default:
            AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
            return ERR_NOT_SUPPORTED;
    }
    return eventEntry_->SetCallDeviceActive(deviceType, active, address, uid);
}

int32_t AudioPolicyServer::GetActiveBluetoothDevice(std::shared_ptr<AudioDeviceDescriptor> &descs)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    descs = eventEntry_->GetActiveBluetoothDevice();

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        CHECK_AND_RETURN_RET_LOG(descs != nullptr, ERROR, "descs is nullptr");
        descs->deviceName_ = "";
        descs->macAddress_ = "";
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::GetConverterConfig(ConverterConfig &cfg)
{
    cfg = AudioConverterParser::GetInstance().LoadConfig();
    return SUCCESS;
}

int32_t AudioPolicyServer::GetSpatializationSceneType(int32_t &type)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    type = static_cast<int32_t>(audioSpatializationService_.GetSpatializationSceneType());
    return SUCCESS;
}

int32_t AudioPolicyServer::SetSpatializationSceneType(int32_t spatializationSceneTypeIn)
{
    AudioSpatializationSceneType spatializationSceneType =
        static_cast<AudioSpatializationSceneType>(spatializationSceneTypeIn);
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetSpatializationSceneType(spatializationSceneType);
}

int32_t AudioPolicyServer::DisableSafeMediaVolume()
{
    if (!VerifyPermission(MODIFY_AUDIO_SETTINGS_PERMISSION)) {
        AUDIO_ERR_LOG("MODIFY_AUDIO_SETTINGS_PERMISSION permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_SYSTEM_PERMISSION_DENIED;
    }
    return audioVolumeManager_.DisableSafeMediaVolume();
}

int32_t AudioPolicyServer::GetApiTargetVersion()
{
    AppExecFwk::BundleInfo bundleInfo = AudioBundleManager::GetBundleInfo();

    // Taking remainder of large integers
    int32_t apiTargetversion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_REMAINDER;
    return apiTargetversion;
}

int32_t AudioPolicyServer::IsHighResolutionExist(bool &ret)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    ret = isHighResolutionExist_;
    return SUCCESS;
}

int32_t AudioPolicyServer::SetHighResolutionExist(bool highResExist)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    isHighResolutionExist_ = highResExist;
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetMaxAmplitude(int32_t deviceId, float &ret)
{
    AudioInterrupt audioInterrupt;
    int32_t zoneID = 0;
    GetSessionInfoInFocus(audioInterrupt, zoneID);
    ret = audioActiveDevice_.GetMaxAmplitude(deviceId, audioInterrupt);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsHeadTrackingDataRequested(const std::string &macAddress, bool &ret)
{
    ret = audioSpatializationService_.IsHeadTrackingDataRequested(macAddress);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "SetAudioDeviceRefinerCallback object is nullptr");
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return audioRouterCenter_.SetAudioDeviceRefinerCallback(object);
}

int32_t AudioPolicyServer::UnsetAudioDeviceRefinerCallback()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return audioRouterCenter_.UnsetAudioDeviceRefinerCallback();
}

int32_t AudioPolicyServer::TriggerFetchDevice(const AudioStreamDeviceChangeReasonExt &reason)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(eventEntry_ != nullptr, ERR_NULL_POINTER, "eventEntry_ is nullptr");
    return eventEntry_->TriggerFetchDevice(reason);
}

int32_t AudioPolicyServer::SetPreferredDevice(int32_t preferredTypeIn,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid)
{
    PreferredType preferredType = static_cast<PreferredType>(preferredTypeIn);
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("No permission");
        return ERROR;
    }
    return audioPolicyUtils_.SetPreferredDevice(preferredType, desc, uid, "SetPreferredDevice");
}

int32_t AudioPolicyServer::SetDeviceVolumeBehavior(const std::string &networkId, int32_t deviceType,
    const VolumeBehavior &volumeBehavior)
{
    AUDIO_INFO_LOG("networkId [%{public}s], deviceType [%{public}d], isReady [%{public}d], "\
        "isVolumeControlDisabled [%{public}d], databaseVolumeName [%{public}s]", networkId.c_str(), deviceType,
        volumeBehavior.isReady, volumeBehavior.isVolumeControlDisabled, volumeBehavior.databaseVolumeName.c_str());
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("No permission");
        return ERR_PERMISSION_DENIED;
    }

    auto mediaDeviceList = audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1,
        "SetDeviceVolumeBehavior_1", ROUTER_TYPE_USER_SELECT);
    CHECK_AND_RETURN_RET_LOG(!mediaDeviceList.empty(), ERROR, "mediaDeviceList is empty");
    std::shared_ptr<AudioDeviceDescriptor> newMediaDescriptor = mediaDeviceList.front();
    CHECK_AND_RETURN_RET_LOG(newMediaDescriptor != nullptr, ERROR, "newMediaDescriptor is nullptr");

    auto callDeviceList = audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1,
        "SetDeviceVolumeBehavior_2", ROUTER_TYPE_USER_SELECT);
    CHECK_AND_RETURN_RET_LOG(!callDeviceList.empty(), ERROR, "callDeviceList is empty");
    std::shared_ptr<AudioDeviceDescriptor> newCallDescriptor = callDeviceList.front();
    CHECK_AND_RETURN_RET_LOG(newCallDescriptor != nullptr, ERROR, "newCallDescriptor is nullptr");

    DeviceType deviceTypeEnum = static_cast<DeviceType>(deviceType);
    if (networkId == newMediaDescriptor->networkId_ && deviceTypeEnum == newMediaDescriptor->deviceType_) {
        audioPolicyUtils_.SetPreferredDevice(AUDIO_MEDIA_RENDER, std::make_shared<AudioDeviceDescriptor>());
    }
    if (networkId == newCallDescriptor->networkId_ && deviceTypeEnum == newCallDescriptor->deviceType_) {
        audioPolicyUtils_.SetPreferredDevice(AUDIO_CALL_RENDER, std::make_shared<AudioDeviceDescriptor>(), SYSTEM_UID,
            "SetDeviceVolumeBehavior");
    }
    audioDeviceManager_.SetDeviceVolumeBehavior(networkId, deviceTypeEnum, volumeBehavior);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "SetAudioDeviceAnahsCallback object is nullptr");
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return coreService_->SetAudioDeviceAnahsCallback(object);
}

int32_t AudioPolicyServer::UnsetAudioDeviceAnahsCallback()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return coreService_->UnsetAudioDeviceAnahsCallback();
}

void AudioPolicyServer::SendVolumeKeyEventToRssWhenAccountsChanged()
{
    AUDIO_INFO_LOG("Send VolumeKeyEvent to Rss");
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = STREAM_MUSIC;
    volumeEvent.volume = GetSystemVolumeLevelInternal(STREAM_MUSIC);
    volumeEvent.updateUi = false;
    volumeEvent.notifyRssWhenAccountsChange = true;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

void AudioPolicyServer::NotifyAccountsChanged(const int &id)
{
    CHECK_AND_RETURN_LOG(interruptService_ != nullptr, "interruptService_ is nullptr");
    interruptService_->ClearAudioFocusInfoListOnAccountsChanged(id);
    // Asynchronous clear audio focus infos
    usleep(WAIT_CLEAR_AUDIO_FOCUSINFOS_TIME_US);
    audioPolicyService_.NotifyAccountsChanged(id);
    SendVolumeKeyEventToRssWhenAccountsChanged();
    RegisterDefaultVolumeTypeListener();
}

int32_t AudioPolicyServer::MoveToNewPipe(uint32_t sessionId, int32_t pipeType)
{
    return audioOffloadStream_.MoveToNewPipe(sessionId, static_cast<AudioPipeType>(pipeType));
}

void AudioPolicyServer::CheckHibernateState(bool hibernate)
{
    AudioServerProxy::GetInstance().CheckHibernateStateProxy(hibernate);
}

void AudioPolicyServer::UpdateSafeVolumeByS4()
{
    audioVolumeManager_.UpdateSafeVolumeByS4();
}

void AudioPolicyServer::CheckConnectedDevice()
{
    auto isUsbHeadsetConnected =
        audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_USB_HEADSET);
    auto isUsbArmHeadsetConnected =
        audioConnectedDevice_.GetConnectedDeviceByType(DEVICE_TYPE_USB_ARM_HEADSET);

    bool flag = (isUsbHeadsetConnected != nullptr || isUsbArmHeadsetConnected != nullptr) ? true : false;
    AudioServerProxy::GetInstance().SetDeviceConnectedFlag(flag);
}

void AudioPolicyServer::SetDeviceConnectedFlagFalseAfterDuration()
{
    usleep(DEVICE_CONNECTED_FLAG_DURATION_MS); // 3s
    AudioServerProxy::GetInstance().SetDeviceConnectedFlag(false);
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT, ERROR,
        "get supported audio effect property size invalid.");
    return AUDIO_OK;
}

int32_t AudioPolicyServer::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "set audio effect property array size invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    return audioPolicyService_.SetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    int32_t res = audioPolicyService_.GetAudioEffectProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT, ERROR, "size invalid.");
    return res;
}

int32_t AudioPolicyServer::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    int32_t res = audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT, ERROR,
        "get supported audio effect property size invalid.");
    return res;
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    int32_t res = audioPolicyService_.GetSupportedAudioEnhanceProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT, ERROR,
        "get supported audio effect property size invalid.");
    return res;
}

int32_t AudioPolicyServer::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.GetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyServer::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "set audio enhance property array size invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.SetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyServer::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "set audio effect property array size invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.SetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    int32_t res = audioPolicyService_.GetAudioEffectProperty(propertyArray);
    size_t size = propertyArray.property.size();
    CHECK_AND_RETURN_RET_LOG(size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT, ERROR, "size invalid.");
    return res;
}

int32_t AudioPolicyServer::InjectInterruption(const std::string &networkId, const InterruptEvent &event)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_CAST_ENGINE_SA) {
        AUDIO_ERR_LOG("InjectInterruption callerUid is Error: not cast_engine");
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is nullptr");
    std::set<int32_t> sessionIds =
        AudioStreamCollector::GetAudioStreamCollector().GetSessionIdsOnRemoteDeviceByDeviceType(
            DEVICE_TYPE_REMOTE_CAST);

    if (interruptService_ != nullptr) {
        std::set<int32_t> fakeSessionIds =
            interruptService_->GetStreamIdsForAudioSessionByDeviceType(0, DEVICE_TYPE_REMOTE_CAST);
        sessionIds.insert(fakeSessionIds.begin(), fakeSessionIds.end());
    }

    InterruptEventInternal interruptEvent { event.eventType, event.forceType, event.hintType, 0.2f};
    ProcessRemoteInterrupt(sessionIds, interruptEvent);
    return SUCCESS;
}

bool AudioPolicyServer::CheckAudioSessionStrategy(const AudioSessionStrategy &sessionStrategy)
{
    bool result = false;
    switch (sessionStrategy.concurrencyMode) {
        case AudioConcurrencyMode::DEFAULT:
        case AudioConcurrencyMode::MIX_WITH_OTHERS:
        case AudioConcurrencyMode::DUCK_OTHERS:
        case AudioConcurrencyMode::PAUSE_OTHERS:
            result = true;
            break;
        default:
            AUDIO_ERR_LOG("Invalid concurrency mode: %{public}d!",
                static_cast<int32_t>(sessionStrategy.concurrencyMode));
            result = false;
            break;
    }
    return result;
}

int32_t AudioPolicyServer::ActivateAudioSession(int32_t strategyIn)
{
    AudioConcurrencyMode mode = static_cast<AudioConcurrencyMode>(strategyIn);
    AudioSessionStrategy strategy{mode};
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    if (!CheckAudioSessionStrategy(strategy)) {
        AUDIO_ERR_LOG("The audio session strategy is invalid!");
        return ERR_INVALID_PARAM;
    }

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioSessionZoneid(
        IPCSkeleton::GetCallingUid(), callerPid, true);
    AUDIO_INFO_LOG("activate audio session with concurrencyMode %{public}d for pid %{public}d, zoneId %{public}d",
        static_cast<int32_t>(strategy.concurrencyMode), callerPid, zoneId);

    bool isStandalone = StandaloneModeManager::GetInstance().CheckAndRecordStandaloneApp(
        IPCSkeleton::GetCallingUid(), true);
    int32_t ret = interruptService_->ActivateAudioSession(zoneId, callerPid, strategy, isStandalone);
    if ((ret == SUCCESS) && (interruptService_->IsSessionNeedToFetchOutputDevice(callerPid)) &&
        (eventEntry_ != nullptr)) {
        eventEntry_->FetchOutputDeviceAndRoute("ActivateAudioSession",
            AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
    }

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendAudioSessionDeviceChange(
            AudioStreamDeviceChangeReason::AUDIO_SESSION_ACTIVATE, callerPid);
    }

    return ret;
}

int32_t AudioPolicyServer::DeactivateAudioSession()
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioSessionZoneid(
        IPCSkeleton::GetCallingUid(), callerPid, false);
    AUDIO_INFO_LOG("deactivate audio session for pid %{public}d, zoneId %{public}d", callerPid, zoneId);
    return interruptService_->DeactivateAudioSession(zoneId, callerPid);
}

int32_t AudioPolicyServer::IsAudioSessionActivated(bool &isActive)
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        isActive = false;
        return ERR_MEMORY_ALLOC_FAILED;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    isActive = interruptService_->IsAudioSessionActivated(callerPid);
    AUDIO_INFO_LOG("callerPid %{public}d, isSessionActive: %{public}d.", callerPid, isActive);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioSessionScene(int32_t audioSessionScene)
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    return interruptService_->SetAudioSessionScene(callerPid, static_cast<AudioSessionScene>(audioSessionScene));
}

int32_t AudioPolicyServer::GetDefaultOutputDevice(int32_t &deviceType)
{
    if (eventEntry_ == nullptr) {
        AUDIO_ERR_LOG("eventEntry_ is nullptr!");
        return ERR_UNKNOWN;
    }

    DeviceType deviceTypeOut = DEVICE_TYPE_NONE;
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t ret = eventEntry_->GetSessionDefaultOutputDevice(callerPid, deviceTypeOut);
    deviceType = static_cast<int32_t>(deviceTypeOut);
    return ret;
}

int32_t AudioPolicyServer::SetDefaultOutputDevice(int32_t deviceType)
{
    if ((eventEntry_ == nullptr) || (interruptService_ == nullptr)) {
        AUDIO_ERR_LOG("eventEntry_ or interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t ret = eventEntry_->SetSessionDefaultOutputDevice(callerPid, static_cast<DeviceType>(deviceType));
    if ((ret == SUCCESS) && (interruptService_->IsSessionNeedToFetchOutputDevice(callerPid))) {
        eventEntry_->FetchOutputDeviceAndRoute("SetDefaultOutputDevice",
            AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
    }

    return ret;
}

int32_t AudioPolicyServer::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_CAR_DISTRIBUTED_ENGINE_SA) {
        AUDIO_ERR_LOG("callerUid %{public}d is not allow LoadSplitModule", callerUid);
        return ERR_PERMISSION_DENIED;
    }
    return eventEntry_->LoadSplitModule(splitArgs, networkId);
}

int32_t AudioPolicyServer::IsAllowedPlayback(int32_t uid, int32_t pid, bool &isAllowed)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != MEDIA_SERVICE_UID) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        isAllowed = audioBackgroundManager_.IsAllowedPlayback(callerUid, callerPid);
        return SUCCESS;
    }
    isAllowed = audioBackgroundManager_.IsAllowedPlayback(uid, pid);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetVoiceRingtoneMute(bool isMute)
{
    constexpr int32_t foundationUid = 5523; // "uid" : "foundation"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by foundation
    CHECK_AND_RETURN_RET_LOG(callerUid == foundationUid, ERROR,
        "SetVoiceRingtoneMute callerUid is error: not foundation");
    AUDIO_INFO_LOG("Set VoiceRingtone is %{public}d", isMute);
    return audioVolumeManager_.SetVoiceRingtoneMute(isMute);
}

int32_t AudioPolicyServer::NotifySessionStateChange(int32_t uid, int32_t pid, bool hasSession)
{
    AUDIO_INFO_LOG("UID:%{public}d, PID:%{public}d, Session State: %{public}d", uid, pid, hasSession);
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by av_session
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_AVSESSION_SERVICE, ERROR,
        "NotifySessionStateChange callerUid is error: not av_session");
    return audioBackgroundManager_.NotifySessionStateChange(uid, pid, hasSession);
}

int32_t AudioPolicyServer::NotifyFreezeStateChange(const std::set<int32_t> &pidList, bool isFreeze)
{
    constexpr size_t size = 100;
    CHECK_AND_RETURN_RET_LOG(pidList.size() <= size, ERROR, "pidList more than 100.");
    AUDIO_INFO_LOG("In");
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by RSS
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_RESOURCE_SCHEDULE_SERVICE, ERROR,
        "NotifyFreezeStateChange callerUid is error: not RSS");
    return audioBackgroundManager_.NotifyFreezeStateChange(pidList, isFreeze);
}

int32_t AudioPolicyServer::ResetAllProxy()
{
    AUDIO_INFO_LOG("In");
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by RSS
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_RESOURCE_SCHEDULE_SERVICE, ERROR,
        "ResetAllProxy callerUid is error: not RSS");
    return audioBackgroundManager_.ResetAllProxy();
}

int32_t AudioPolicyServer::NotifyProcessBackgroundState(int32_t uid, int32_t pid)
{
    AUDIO_INFO_LOG("In");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    audioBackgroundManager_.NotifyAppStateChange(uid, pid, STATE_BACKGROUND);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetVirtualCall(bool isVirtual)
{
    constexpr int32_t meetServiceUid = 5523; // "uid" : "meetservice"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by meetservice
    CHECK_AND_RETURN_RET_LOG(callerUid == meetServiceUid, ERROR,
        "SetVirtualCall callerUid is error: not meetservice");
    AUDIO_INFO_LOG("Set VirtualCall is %{public}d", isVirtual);
    return audioDeviceCommon_.SetVirtualCall(callerUid, isVirtual);
}

int32_t AudioPolicyServer::SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    bool isConnected)
{
    AUDIO_INFO_LOG("deviceType: %{public}d, deviceRole: %{public}d, isConnected: %{public}d",
        desc->deviceType_, desc->deviceRole_, isConnected);

    std::vector<uid_t> allowedUids = {
        UID_TV_PROCESS_SA, UID_DP_PROCESS_SA, UID_PENCIL_PROCESS_SA, UID_NEARLINK_SA, UID_BLUETOOTH_SA
    };
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::CheckCallingUidPermission(allowedUids), ERR_PERMISSION_DENIED,
        "uid permission denied");

    bool ret = VerifyPermission(MANAGE_AUDIO_CONFIG);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "MANAGE_AUDIO_CONFIG permission denied");

    eventEntry_->OnDeviceStatusUpdated(*desc, isConnected);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by av_session
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_AVSESSION_SERVICE, ERROR,
        "SetQueryAllowedPlaybackCallback callerUid is error: not av_session");
    return audioBackgroundManager_.SetQueryAllowedPlaybackCallback(object);
}

int32_t AudioPolicyServer::SetBackgroundMuteCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by media_service
    CHECK_AND_RETURN_RET_LOG(callerUid == MEDIA_SERVICE_UID, ERROR,
        "SetBackgroundMuteCallback callerUid is error: not media_service");
    return audioBackgroundManager_.SetBackgroundMuteCallback(object);
}

int32_t AudioPolicyServer::GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo,
    int32_t streamUsage, int32_t &retMod)
{
    retMod = coreService_->GetDirectPlaybackSupport(streamInfo, static_cast<StreamUsage>(streamUsage));
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMaxVolumeLevelByUsage(int32_t streamUsage, int32_t &retMaxVolumeLevel)
{
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
        ERR_INVALID_PARAM, "GetMaxVolumeLevelByUsage: Invalid streamUsage");
    int32_t volumeType = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(
        static_cast<StreamUsage>(streamUsage)));
    GetMaxVolumeLevel(volumeType, retMaxVolumeLevel);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMinVolumeLevelByUsage(int32_t streamUsage, int32_t &retMinVolumeLevel)
{
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
        ERR_INVALID_PARAM, "GetMinVolumeLevelByUsage: Invalid streamUsage");
    int32_t volumeType = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(
        static_cast<StreamUsage>(streamUsage)));
    GetMinVolumeLevel(volumeType, retMinVolumeLevel);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetVolumeLevelByUsage(int32_t streamUsage, int32_t &retVolumeLevel)
{
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
        ERR_INVALID_PARAM, "GetVolumeLevelByUsage: Invalid streamUsage");
    int32_t volumeType = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(
        static_cast<StreamUsage>(streamUsage)));
    GetSystemVolumeLevel(volumeType, DEFAULT_UID, retVolumeLevel);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetStreamMuteByUsage(int32_t streamUsage, bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
        ERR_INVALID_PARAM, "GetStreamMuteByUsage: Invalid streamUsage");
    int32_t volumeType = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(
        static_cast<StreamUsage>(streamUsage)));
    GetStreamMute(volumeType, isMute);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetVolumeInDbByStream(int32_t streamUsageIn, int32_t volumeLevel,
    int32_t deviceType, float &ret)
{
    StreamUsage streamUsage = static_cast<StreamUsage>(streamUsageIn);
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
        ERR_INVALID_PARAM, "GetVolumeInDbByStream: Invalid streamUsage");
    return GetSystemVolumeInDb(VolumeUtils::GetVolumeTypeFromStreamUsage(streamUsage), volumeLevel, deviceType, ret);
}

int32_t AudioPolicyServer::GetSupportedAudioVolumeTypes(std::vector<int32_t> &ret)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "No system permission");

    std::vector<AudioVolumeType> out = VolumeUtils::GetSupportedAudioVolumeTypes();
    ret.clear();
    for (auto &item : out) {
        ret.push_back(static_cast<int32_t>(item));
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::GetAudioVolumeTypeByStreamUsage(int32_t streamUsageIn, int32_t &ret)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "No system permission");

    StreamUsage streamUsage = static_cast<StreamUsage>(streamUsageIn);
    ret = static_cast<int32_t>(VolumeUtils::GetVolumeTypeFromStreamUsage(streamUsage));
    return SUCCESS;
}

int32_t AudioPolicyServer::GetStreamUsagesByVolumeType(int32_t audioVolumeTypeIn, std::vector<int32_t> &ret)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "No system permission");

    AudioVolumeType audioVolumeType = static_cast<AudioVolumeType>(audioVolumeTypeIn);
    std::vector<StreamUsage> out = VolumeUtils::GetStreamUsagesByVolumeType(audioVolumeType);

    ret.clear();
    for (auto &item : out) {
        ret.push_back(static_cast<int32_t>(item));
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::SetCallbackStreamUsageInfo(const std::set<int32_t> &streamUsages)
{
    size_t size = streamUsages.size();
    CHECK_AND_RETURN_RET_LOG(size <= MAX_STREAM_USAGE_COUNT, ERR_INVALID_PARAM, "size upper limit.");
    std::set<StreamUsage> streamUsagesTmp;
    for (auto streamUsage : streamUsages) {
        CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_MAX,
            ERR_INVALID_PARAM, "streamUsage is invalid.");
        streamUsagesTmp.insert(static_cast<StreamUsage>(streamUsage));
    }

    if (audioPolicyServerHandler_ != nullptr) {
        return audioPolicyServerHandler_->SetCallbackStreamUsageInfo(streamUsagesTmp);
    } else {
        AUDIO_ERR_LOG("audioPolicyServerHandler_ is nullptr");
        return AUDIO_ERR;
    }
}

int32_t AudioPolicyServer::ForceStopAudioStream(int32_t audioTypeIn)
{
    StopAudioType audioType = static_cast<StopAudioType>(audioTypeIn);
    CHECK_AND_RETURN_RET_LOG(audioType >= STOP_ALL && audioType <= STOP_RECORD,
        ERR_INVALID_PARAM, "Invalid audioType");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "No system permission");
#ifdef AUDIO_BUILD_VARIANT_ROOT
    // root user case for auto test
    CHECK_AND_RETURN_RET_LOG(static_cast<uid_t>(IPCSkeleton::GetCallingUid()) == ROOT_UID,
        ERR_PERMISSION_DENIED, "not root user");
#else
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyPermission(HIVIEWCARE_PERMISSION,
        IPCSkeleton::GetCallingTokenID()), ERR_PERMISSION_DENIED, "not hiviewcare");
#endif
    AUDIO_INFO_LOG("stop audio stream, type:%{public}d", audioType);
    return AudioServerProxy::GetInstance().ForceStopAudioStreamProxy(audioType);
}

int32_t AudioPolicyServer::IsCapturerFocusAvailable(const AudioCapturerInfo &capturerInfo, bool &ret)
{
    CHECK_AND_RETURN_RET_LOG(interruptService_ != nullptr, ERROR, "interruptService_ is nullptr");
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(IPCSkeleton::GetCallingUid());
    ret = interruptService_->IsCapturerFocusAvailable(zoneId, capturerInfo);
    return SUCCESS;
}

int32_t AudioPolicyServer::ForceVolumeKeyControlType(int32_t volumeType, int32_t duration, int32_t &ret)
{
    CHECK_AND_RETURN_RET_LOG(VerifyPermission(MODIFY_AUDIO_SETTINGS_PERMISSION), ERR_PERMISSION_DENIED,
        "MODIFY_AUDIO_SETTINGS_PERMISSION permission check failed");
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(),
        ERR_SYSTEM_PERMISSION_DENIED, "no system permission");
    ret = audioVolumeManager_.ForceVolumeKeyControlType(static_cast<AudioVolumeType>(volumeType), duration);
    return SUCCESS;
}

void AudioPolicyServer::UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID)
{
    audioDeviceManager_.UpdateDefaultOutputDeviceWhenStarting(sessionID);
    TriggerFetchDevice(AudioStreamDeviceChangeReasonExt());
}

void AudioPolicyServer::UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID)
{
    audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(sessionID);
    TriggerFetchDevice(AudioStreamDeviceChangeReasonExt());
}

int32_t AudioPolicyServer::IsAcousticEchoCancelerSupported(int32_t sourceType, bool &ret)
{
    ret = AudioServerProxy::GetInstance().IsAcousticEchoCancelerSupported(static_cast<SourceType>(sourceType));
    return SUCCESS;
}


int32_t AudioPolicyServer::SetKaraokeParameters(const std::string &parameters, bool &ret)
{
    ret = AudioServerProxy::GetInstance().SetKaraokeParameters(parameters);
    return SUCCESS;
}

int32_t AudioPolicyServer::IsAudioLoopbackSupported(int32_t modeIn, bool &ret)
{
    AudioLoopbackMode mode = static_cast<AudioLoopbackMode>(modeIn);
    ret = AudioServerProxy::GetInstance().IsAudioLoopbackSupported(mode);
    return SUCCESS;
}

int32_t AudioPolicyServer::UpdateDeviceInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, int32_t command)
{
    std::vector<uid_t> allowedUids = { UID_NEARLINK_SA };
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::CheckCallingUidPermission(allowedUids), ERR_PERMISSION_DENIED,
        "uid permission denied");

    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_INVALID_PARAM, "deviceDesc is nullptr");

    eventEntry_->OnDeviceInfoUpdated(*deviceDesc, static_cast<DeviceInfoUpdateCommand>(command));
    return SUCCESS;
}

int32_t AudioPolicyServer::SetSleAudioOperationCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "object is nullptr");

    bool ret = PermissionUtil::VerifyIsAudio();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "Uid Check Failed");

    return audioPolicyService_.SetSleAudioOperationCallback(object);
}

int32_t AudioPolicyServer::SetCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED, "No system permission");
    return audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(selectedAudioDevice, enabled);
}

int32_t AudioPolicyServer::IsCollaborativePlaybackSupported(bool &ret)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        ret = false;
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    ret = audioCollaborativeService_.IsCollaborativePlaybackSupported();
    return SUCCESS;
}

int32_t AudioPolicyServer::IsCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool &enabled)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        enabled = false;
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    enabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice);
    return SUCCESS;
}

int32_t AudioPolicyServer::CallRingtoneLibrary()
{
    Trace trace("AudioPolicyServer::CallRingtoneLibrary");
    AUDIO_INFO_LOG("Enter CallRingtoneLibrary");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(saManager != nullptr, ERROR, "Get system ability manager failed.");

    AudioXCollie audioXCollie("CallRingtoneLibrary::start", DATASHARE_SERVICE_TIMEOUT_SECONDS,
        [](void *) {
            AUDIO_ERR_LOG("CallRingtoneLibrary timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG);

    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObj != nullptr, ERROR, "Get system ability failed.");

    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, "datashare:///ringtone", "",
        DATASHARE_SERVICE_TIMEOUT_SECONDS);
    CHECK_AND_RETURN_RET_LOG(dataShareHelper != nullptr, ERROR, "Create dataShare failed, datashare or library error.");
    dataShareHelper->Release();
    return SUCCESS;
}

int32_t AudioPolicyServer::SetSystemVolumeDegree(int32_t streamTypeIn, int32_t volumeDegree, int32_t volumeFlag,
    int32_t uid)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(streamTypeIn);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }

    bool adjustable = false;
    IsVolumeUnadjustable(adjustable);
    if (adjustable) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set degree");
        return ERR_OPERATION_FAILED;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    bool flag = volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI;
    int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(volumeDegree);
    int32_t callingUid =  uid != 0 ? uid : IPCSkeleton::GetCallingUid();
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(callingUid);
    SetSystemVolumeLevelInternal(streamType, volumeLevel, flag, zoneId);

    return SetSystemVolumeDegreeInner(streamType, volumeDegree, flag, uid);
}

int32_t AudioPolicyServer::SetSystemVolumeDegreeInner(AudioStreamType streamType, int32_t volumeDegree,
    bool volumeFlag, int32_t uid)
{
    int32_t ret = audioVolumeManager_.SetSystemVolumeDegree(streamType, volumeDegree, volumeFlag);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Update volume degree failed, err:%{public}d", ret);
    }
    return ret;
}

int32_t AudioPolicyServer::GetSystemVolumeDegree(int32_t streamType, int32_t uid, int32_t &volumeDegree)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    AudioStreamType newStreamType = static_cast<AudioStreamType>(streamType);
    volumeDegree = audioVolumeManager_.GetSystemVolumeDegree(newStreamType);
    return SUCCESS;
}

int32_t AudioPolicyServer::GetMinVolumeDegree(int32_t volumeType, int32_t &volumeDegree)
{
    volumeDegree = audioVolumeManager_.GetMinVolumeDegree(static_cast<AudioVolumeType>(volumeType));
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
