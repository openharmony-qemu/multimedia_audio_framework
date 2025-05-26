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
#include "audio_policy_log.h"
#include "parameters.h"
#include "media_monitor_manager.h"
#include "client_type_manager.h"
#include "dfx_msg_manager.h"
#ifdef USB_ENABLE
#include "audio_usb_manager.h"
#endif

using OHOS::Security::AccessToken::PrivacyKit;
using OHOS::Security::AccessToken::TokenIdKit;
using namespace std;

namespace OHOS {
namespace AudioStandard {

// for phone
const std::vector<AudioStreamType> GET_STREAM_ALL_VOLUME_TYPES {
    STREAM_MUSIC,
    STREAM_VOICE_CALL,
    STREAM_RING,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC
};

const std::vector<AudioStreamType> GET_PC_STREAM_ALL_VOLUME_TYPES {
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
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
constexpr uid_t UID_RESOURCE_SCHEDULE_SERVICE = 1096;
constexpr int64_t OFFLOAD_NO_SESSION_ID = -1;
constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;
const char* MANAGE_SYSTEM_AUDIO_EFFECTS = "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS";
const char* MANAGE_AUDIO_CONFIG = "ohos.permission.MANAGE_AUDIO_CONFIG";
const char* USE_BLUETOOTH_PERMISSION = "ohos.permission.USE_BLUETOOTH";
const char* MICROPHONE_CONTROL_PERMISSION = "ohos.permission.MICROPHONE_CONTROL";

REGISTER_SYSTEM_ABILITY_BY_ID(AudioPolicyServer, AUDIO_POLICY_SERVICE_ID, true)

std::map<PolicyType, uint32_t> POLICY_TYPE_MAP = {
    {PolicyType::EDM_POLICY_TYPE, 0},
    {PolicyType::PRIVACY_POLCIY_TYPE, 1},
    {PolicyType::TEMPORARY_POLCIY_TYPE, 2}
};

AudioPolicyServer::AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
      audioPolicyService_(AudioPolicyService::GetAudioPolicyService()),
      audioPolicyUtils_(AudioPolicyUtils::GetInstance()),
      audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
      audioSpatializationService_(AudioSpatializationService::GetAudioSpatializationService()),
      audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
      audioPolicyDump_(AudioPolicyDump::GetInstance()),
      audioActiveDevice_(AudioActiveDevice::GetInstance())
{
    volumeStep_ = system::GetIntParameter("const.multimedia.audio.volumestep", 1);
    AUDIO_INFO_LOG("Get volumeStep parameter success %{public}d", volumeStep_);

    powerStateCallbackRegister_ = false;
    supportVibrator_ = system::GetBoolParameter("const.vibrator.support_vibrator", true);
    volumeApplyToAll_ = system::GetBoolParameter("const.audio.volume_apply_to_all", false);
    if (volumeApplyToAll_) {
        audioPolicyService_.SetNormalVoipFlag(true);
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

    if (audioPolicyService_.SetAudioStreamRemovedCallback(this)) {
        AUDIO_ERR_LOG("SetAudioStreamRemovedCallback failed");
    }
    audioPolicyService_.Init();

    AddSystemAbilityListeners();
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
        audioPolicyService_.SetFirstScreenOn();
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
    AddSystemAbilityListener(ACCESSIBILITY_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(POWER_MANAGER_SERVICE_ID);
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
#ifdef USB_ENABLE
    AudioUsbManager::GetInstance().Deinit();
#endif
    UnRegisterPowerStateListener();
    UnRegisterSyncHibernateListener();
    return;
}

void AudioPolicyServer::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
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
        case ACCESSIBILITY_MANAGER_SERVICE_ID:
            SubscribeAccessibilityConfigObserver();
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
#ifdef USB_ENABLE
        case USB_SYSTEM_ABILITY_ID:
            AudioUsbManager::GetInstance().Init(&audioPolicyService_);
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

void AudioPolicyServer::OnAddSystemAbilityExtract(int32_t systemAbilityId, const std::string& deviceId)
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
    if (isInitMuteState_ == false && audioPolicyService_.IsDataShareReady()) {
        AUDIO_INFO_LOG("datashare is ready and need init mic mute state");
        InitMicrophoneMute();
    }
}

void AudioPolicyServer::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    AUDIO_DEBUG_LOG("AudioPolicyServer::OnRemoveSystemAbility systemAbilityId:%{public}d removed", systemAbilityId);
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
bool AudioPolicyServer::MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType,
    const AudioStreamType &streamInFocus)
{
    bool volLevelCheck = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ?
        volLevel >= GetMaxVolumeLevel(streamInFocus) : volLevel <= GetMinVolumeLevel(streamInFocus);
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
        audioPolicyService_.GetActiveOutputDevice() == DEVICE_TYPE_BLUETOOTH_A2DP) {
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
        int32_t ret = ProcessVolumeKeyMuteEvents(keyType);
        if (ret != AUDIO_OK) {
            AUDIO_DEBUG_LOG("process volume key mute events need return[%{public}d]", ret);
            return;
        }
    });
    if (keySubId < 0) {
        AUDIO_ERR_LOG("key: %{public}s failed", (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
        return ERR_MMI_SUBSCRIBE;
    }
    return keySubId;
}

int32_t AudioPolicyServer::ProcessVolumeKeyMuteEvents(const int32_t keyType)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
    if (volumeApplyToAll_) {
        streamInFocus = AudioStreamType::STREAM_ALL;
    } else {
        streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamInFocus());
        ChangeVolumeOnVoiceAssistant(streamInFocus);
    }
    if (isScreenOffOrLock_ && !IsStreamActive(streamInFocus)) {
        AUDIO_INFO_LOG("screen off or screen lock, this stream is not active, not change volume.");
        return AUDIO_OK;
    }
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(streamInFocus)) {
        AUDIO_INFO_LOG("VolumeKeyEvents: volumeKey: Up. volumeType %{public}d is mute. Unmute.", streamInFocus);
        SetStreamMuteInternal(streamInFocus, false, true);
        if (!VolumeUtils::IsPCVolumeEnable()) {
            AUDIO_DEBUG_LOG("phone need return");
            return ERROR_UNSUPPORTED;
        }
    }
    if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(STREAM_SYSTEM) &&
        VolumeUtils::IsPCVolumeEnable()) {
        SetStreamMuteInternal(STREAM_SYSTEM, false, true);
    }
    int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus);
    if (MaxOrMinVolumeOption(volumeLevelInInt, keyType, streamInFocus)) {
        AUDIO_ERR_LOG("volumelevel[%{public}d] invalid", volumeLevelInInt);
        return ERROR_INVALID_PARAM;
    }

    volumeLevelInInt = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? ++volumeLevelInInt : --volumeLevelInInt;
    SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, true);
    if (volumeLevelInInt <= 0 && VolumeUtils::IsPCVolumeEnable()) {
        SetStreamMuteInternal(STREAM_SYSTEM, true, true);
    }
    return AUDIO_OK;
}
#endif

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
            std::lock_guard<std::mutex> lock(systemVolumeMutex_);
            AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
            if (volumeApplyToAll_) {
                bool isStreamMuted = GetStreamMuteInternal(STREAM_ALL);
                SetStreamMuteInternal(STREAM_ALL, !isStreamMuted, true);
            } else {
                streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamInFocus());
                bool isMuted = GetStreamMuteInternal(streamInFocus);
                SetStreamMuteInternal(streamInFocus, !isMuted, true);
            }
        });
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
    if (volumeLevel < audioPolicyService_.GetMinVolumeLevel(streamType) ||
        volumeLevel > audioPolicyService_.GetMaxVolumeLevel(streamType)) {
        AUDIO_ERR_LOG("IsVolumeLevelValid: volumeLevel[%{public}d] is out of valid range for streamType[%{public}d]",
            volumeLevel, streamType);
        result = false;
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
        ConnectServiceAdapter();
        sessionProcessor_.Start();
        RegisterParamCallback();
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
    AUDIO_INFO_LOG("receive DATA_SHARE_READY action success");
    eventReceiver_(eventData);
}

void AudioPolicyServer::SubscribeCommonEventExecute()
{
    SubscribeCommonEvent("usual.event.DATA_SHARE_READY");
    SubscribeCommonEvent("usual.event.dms.rotation_changed");
    SubscribeCommonEvent("usual.event.bluetooth.remotedevice.NAME_UPDATE");
    SubscribeCommonEvent("usual.event.SCREEN_ON");
    SubscribeCommonEvent("usual.event.SCREEN_OFF");
    SubscribeCommonEvent("usual.event.SCREEN_LOCKED");
    SubscribeCommonEvent("usual.event.SCREEN_UNLOCKED");
#ifdef USB_ENABLE
    AudioUsbManager::GetInstance().Init(&audioPolicyService_);
    AudioUsbManager::GetInstance().SubscribeEvent();
#endif
    SubscribeSafeVolumeEvent();
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
        audioPolicyService_.SetDataShareReady(true);
        RegisterDataObserver();
        if (isInitMuteState_ == false) {
            AUDIO_INFO_LOG("receive DATA_SHARE_READY action and need init mic mute state");
            InitMicrophoneMute();
        }
        if (isInitSettingsData_ == false) {
            AUDIO_INFO_LOG("First receive DATA_SHARE_READY action and need init SettingsData");
            InitKVStore();
            SettingsDataReady();
            isInitSettingsData_ = true;
        }
    } else if (action == "usual.event.dms.rotation_changed") {
        uint32_t rotate = static_cast<uint32_t>(want.GetIntParam("rotation", 0));
        AUDIO_INFO_LOG("Set rotation to audioeffectchainmanager is %{public}d", rotate);
        audioPolicyService_.SetRotationToEffect(rotate);
    } else if (action == "usual.event.bluetooth.remotedevice.NAME_UPDATE") {
        std::string deviceName  = want.GetStringParam("remoteName");
        std::string macAddress = want.GetStringParam("deviceAddr");
        audioPolicyService_.OnReceiveBluetoothEvent(macAddress, deviceName);
    } else if (action == "usual.event.SCREEN_ON") {
        AUDIO_INFO_LOG("receive SCREEN_ON action, control audio focus if need");
        audioPolicyService_.SetFirstScreenOn();
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

void AudioPolicyServer::OffloadStreamCheck(int64_t activateSessionId, int64_t deactivateSessionId)
{
    CheckSubscribePowerStateChange();
    if (deactivateSessionId != OFFLOAD_NO_SESSION_ID) {
        audioPolicyService_.OffloadStreamReleaseCheck(deactivateSessionId);
    }
    if (activateSessionId != OFFLOAD_NO_SESSION_ID) {
        audioPolicyService_.OffloadStreamSetCheck(activateSessionId);
    }
}

AudioPolicyServer::AudioPolicyServerPowerStateCallback::AudioPolicyServerPowerStateCallback(
    AudioPolicyServer* policyServer) : PowerMgr::PowerStateCallbackStub(), policyServer_(policyServer)
{}

void AudioPolicyServer::CheckStreamMode(const int64_t activateSessionId)
{
    Trace trace("AudioPolicyServer::CheckStreamMode: activateSessionId: " + std::to_string(activateSessionId));
    audioPolicyService_.CheckStreamMode(activateSessionId);
}

void AudioPolicyServer::AudioPolicyServerPowerStateCallback::OnAsyncPowerStateChanged(PowerMgr::PowerState state)
{
    policyServer_->audioPolicyService_.HandlePowerStateChanged(state);
}

void AudioPolicyServer::InitKVStore()
{
    audioPolicyService_.InitKVStore();
}

void AudioPolicyServer::SettingsDataReady()
{
    audioPolicyService_.SettingsDataReady();
}

void AudioPolicyServer::ConnectServiceAdapter()
{
    if (!audioPolicyService_.ConnectServiceAdapter()) {
        AUDIO_ERR_LOG("ConnectServiceAdapter Error in connecting to audio service adapter");
        return;
    }
}

void AudioPolicyServer::LoadEffectLibrary()
{
    audioPolicyService_.LoadEffectLibrary();
}

int32_t AudioPolicyServer::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    return audioPolicyService_.GetMaxVolumeLevel(volumeType);
}

int32_t AudioPolicyServer::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    return audioPolicyService_.GetMinVolumeLevel(volumeType);
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetSystemVolumeLevelLegacy(AudioStreamType streamType, int32_t volumeLevel)
{
    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetSystemVolumeLevelInternal(streamType, volumeLevel, false);
}

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
        return ERR_NOT_SUPPORTED;
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetAppVolumeLevelInternal(appUid, volumeLevel, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel, int32_t volumeFlag)
{
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
    return SetSystemVolumeLevelInternal(streamType, volumeLevel, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::SetSystemVolumeLevelWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType, int32_t volumeFlag)
{
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

AudioStreamType AudioPolicyServer::GetSystemActiveVolumeType(const int32_t clientUid)
{
    return GetSystemActiveVolumeTypeInternal(clientUid);
}

AudioStreamType AudioPolicyServer::GetSystemActiveVolumeTypeInternal(const int32_t clientUid)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return AudioStreamType::STREAM_MUSIC;
    }
    AudioStreamType streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamInFocus());
    if (clientUid != 0) {
        streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamInFocusByUid(clientUid));
    }

    AUDIO_INFO_LOG("Get active volume type success:= %{public}d", streamInFocus);
    return streamInFocus;
}

int32_t AudioPolicyServer::GetAppVolumeLevel(int32_t appUid)
{
    AUDIO_INFO_LOG("GetAppVolumeLevel appUid : %{public}d", appUid);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return ERR_PERMISSION_DENIED;
    }
    return GetAppVolumeLevelInternal(appUid);
}

int32_t AudioPolicyServer::GetSelfAppVolumeLevel()
{
    AUDIO_INFO_LOG("enter");
    int32_t appUid = IPCSkeleton::GetCallingUid();
    return GetAppVolumeLevelInternal(appUid);
}

int32_t AudioPolicyServer::GetSystemVolumeLevel(AudioStreamType streamType)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return GetSystemVolumeLevelInternal(streamType);
}

int32_t AudioPolicyServer::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel = audioPolicyService_.GetSystemVolumeLevelNoMuteState(streamType);
    AUDIO_DEBUG_LOG("GetVolumeNoMute streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    return volumeLevel;
}

int32_t AudioPolicyServer::GetSystemVolumeLevelInternal(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    int32_t volumeLevel = audioPolicyService_.GetSystemVolumeLevel(streamType);
    AUDIO_DEBUG_LOG("GetVolume streamType[%{public}d],volumeLevel[%{public}d]", streamType, volumeLevel);
    return volumeLevel;
}

int32_t AudioPolicyServer::GetAppVolumeLevelInternal(int32_t appUid)
{
    int32_t volumeLevel = audioPolicyService_.GetAppVolumeLevel(appUid);
    AUDIO_DEBUG_LOG("GetAppVolume appUid[%{public}d],volumeLevel[%{public}d]", appUid, volumeLevel);
    return volumeLevel;
}

int32_t AudioPolicyServer::SetLowPowerVolume(int32_t streamId, float volume)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_FOUNDATION_SA && callerUid != UID_RESOURCE_SCHEDULE_SERVICE) {
        AUDIO_ERR_LOG("SetLowPowerVolume callerUid Error: not foundation or resource_schedule_service");
        return ERROR;
    }
    return audioPolicyService_.SetLowPowerVolume(streamId, volume);
}

float AudioPolicyServer::GetLowPowerVolume(int32_t streamId)
{
    return audioPolicyService_.GetLowPowerVolume(streamId);
}

float AudioPolicyServer::GetSingleStreamVolume(int32_t streamId)
{
    return audioPolicyService_.GetSingleStreamVolume(streamId);
}

bool AudioPolicyServer::IsVolumeUnadjustable()
{
    return audioPolicyService_.IsVolumeUnadjustable();
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

int32_t AudioPolicyServer::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("Uid %{public}d send AdjustVolumeByStep volume key: %{public}s.", callerUid,
        (adjustType == VolumeAdjustType::VOLUME_UP) ? "up" : "down");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    AudioStreamType streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamInFocus());
    if (streamInFocus == AudioStreamType::STREAM_DEFAULT) {
        streamInFocus = AudioStreamType::STREAM_MUSIC;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t volumeLevelInInt = 0;
    if (GetStreamMuteInternal(streamInFocus)) {
        SetStreamMuteInternal(streamInFocus, false, false);
    }
    volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus);
    int32_t minRet = GetMinVolumeLevel(streamInFocus);
    int32_t maxRet = GetMaxVolumeLevel(streamInFocus);
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
    volumeLevelInInt = volumeLevelInInt > GetMaxVolumeLevel(streamInFocus) ? GetMaxVolumeLevel(streamInFocus) :
        volumeLevelInInt;
    volumeLevelInInt = volumeLevelInInt < GetMinVolumeLevel(streamInFocus) ? GetMinVolumeLevel(streamInFocus) :
        volumeLevelInInt;
    int32_t ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, false);
    return ret;
}

int32_t AudioPolicyServer::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("Uid %{public}d send AdjustSystemVolumeByStep VolumeType: %{public}d volume key: %{public}s.",
        callerUid, volumeType, (adjustType == VolumeAdjustType::VOLUME_UP) ? "up" : "down");
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustSystemVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(volumeType);
    int32_t minRet = GetMinVolumeLevel(volumeType);
    int32_t maxRet = GetMaxVolumeLevel(volumeType);
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
    volumeLevelInInt = volumeLevelInInt > GetMaxVolumeLevel(volumeType) ? GetMaxVolumeLevel(volumeType) :
        volumeLevelInInt;
    volumeLevelInInt = volumeLevelInInt < GetMinVolumeLevel(volumeType) ? GetMinVolumeLevel(volumeType) :
        volumeLevelInInt;
    int32_t ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt, false);
    return ret;
}

float AudioPolicyServer::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    if (!IsVolumeTypeValid(volumeType)) {
        return static_cast<float>(ERR_INVALID_PARAM);
    }
    if (!IsVolumeLevelValid(volumeType, volumeLevel)) {
        return static_cast<float>(ERR_INVALID_PARAM);
    }

    return audioPolicyService_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetStreamMuteLegacy(AudioStreamType streamType, bool mute, const DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetStreamMuteInternal(streamType, mute, false, deviceType);
}

int32_t AudioPolicyServer::SetStreamMute(AudioStreamType streamType, bool mute, const DeviceType &deviceType)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetStreamMuteInternal(streamType, mute, false, deviceType);
}

int32_t AudioPolicyServer::SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi,
    const DeviceType &deviceType)
{
    AUDIO_INFO_LOG("SetStreamMuteInternal streamType: %{public}d, mute: %{public}d, updateUi: %{public}d",
        streamType, mute, isUpdateUi);

    if (streamType == STREAM_ALL) {
        const std::vector<AudioStreamType> &streamTypeArray =
            (VolumeUtils::IsPCVolumeEnable())? GET_PC_STREAM_ALL_VOLUME_TYPES : GET_STREAM_ALL_VOLUME_TYPES;
        for (auto audioStreamType : streamTypeArray) {
            AUDIO_INFO_LOG("SetMute of STREAM_ALL for StreamType = %{public}d ", audioStreamType);
            int32_t setResult = SetSingleStreamMute(audioStreamType, mute, isUpdateUi, deviceType);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }

    return SetSingleStreamMute(streamType, mute, isUpdateUi, deviceType);
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
        audioPolicyService_.SetStreamMute(STREAM_SYSTEM, mute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is mute or volume change to 0 and need mute system stream");
    } else if (!mute && GetStreamMuteInternal(STREAM_SYSTEM)) {
        // If you STREAM_MUSIC unmute, you need to determine whether the volume is 0
        // if it is 0, the prompt sound will continue to be mute, and if it is not 0
        // you need to synchronize the unmute prompt sound
        bool isMute = (GetSystemVolumeLevelInternal(STREAM_MUSIC) == 0) ? true : false;
        audioPolicyService_.SetStreamMute(STREAM_SYSTEM, isMute);
        SendMuteKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM, isUpdateUi);
        AUDIO_WARNING_LOG("music is unmute and volume is 0 and need %{public}d system stream", isMute);
    }
}

void AudioPolicyServer::SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType, const bool& isUpdateUi)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInternal(streamType);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

int32_t AudioPolicyServer::SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi,
    const DeviceType &deviceType)
{
    bool updateRingerMode = false;
    if ((streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == AudioStreamType::STREAM_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone mute state.
        AudioRingerMode currentRingerMode = audioPolicyService_.GetRingerMode();
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
        !mute && (GetSystemVolumeLevelNoMuteState(STREAM_MUSIC) == 0 || GetStreamMuteInternal(STREAM_MUSIC))) {
        // when music type volume is not mute,system type volume can be muted separately.
        // but when trying to mute system type volume while the volume for music type is mute
        // or volume level is 0,system type volume can not be muted.
        AUDIO_WARNING_LOG("music volume is 0 or mute and no need unmute system stream!");
    } else {
        int32_t result = audioPolicyService_.SetStreamMute(streamType, mute, STREAM_USAGE_UNKNOWN, deviceType);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Fail to set stream mute!");
    }

    if (!mute && GetSystemVolumeLevelInternal(streamType) == 0 && !VolumeUtils::IsPCVolumeEnable()) {
        // If mute state is set to false but volume is 0, set volume to 1
        audioPolicyService_.SetSystemVolumeLevel(streamType, 1);
    }
    
    ProcUpdateRingerModeForMute(updateRingerMode, mute);
    SendMuteKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi);
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
    return audioPolicyService_.GetSystemVolumeDb(streamType);
}

int32_t AudioPolicyServer::SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag)
{
    AUDIO_INFO_LOG("SetSelfAppVolumeLevel volumeLevel: %{public}d, volumeFlag: %{public}d",
        volumeLevel, volumeFlag);
    if (!IsVolumeLevelValid(STREAM_APP, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
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

int32_t AudioPolicyServer::SetAppVolumeMutedInternal(int32_t appUid, bool muted, bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetAppVolumeLevelInternal appUid: %{public}d, muted: %{public}d, updateUi: %{public}d",
        appUid, muted, isUpdateUi);
    int32_t ret = audioPolicyService_.SetAppVolumeMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set App Volume mute");
    return ret;
}

bool AudioPolicyServer::IsAppVolumeMute(int32_t appUid, bool owned)
{
    AUDIO_INFO_LOG("IsAppVolumeMute appUid: %{public}d, owned: %{public}d", appUid, owned);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return ERR_PERMISSION_DENIED;
    }
    bool ret = audioPolicyService_.IsAppVolumeMute(appUid, owned);
    return ret;
}

int32_t AudioPolicyServer::SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelInternal streamType: %{public}d, volumeLevel: %{public}d, updateUi: %{public}d",
        streamType, volumeLevel, isUpdateUi);
    if (IsVolumeUnadjustable()) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }
    bool mute = GetStreamMuteInternal(streamType);
    if (streamType == STREAM_ALL) {
        const std::vector<AudioStreamType> &streamTypeArray =
            (VolumeUtils::IsPCVolumeEnable()) ? GET_PC_STREAM_ALL_VOLUME_TYPES : GET_STREAM_ALL_VOLUME_TYPES;
        for (auto audioStreamType : streamTypeArray) {
            AUDIO_INFO_LOG("SetVolume of STREAM_ALL, SteamType = %{public}d, mute = %{public}d, level = %{public}d",
                audioStreamType, mute, volumeLevel);
            int32_t setResult = SetSingleStreamVolume(audioStreamType, volumeLevel, isUpdateUi, mute);
            if (setResult != SUCCESS && setResult != ERR_SET_VOL_FAILED_BY_SAFE_VOL) {
                return setResult;
            }
        }
        return SUCCESS;
    }
    return SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi, mute);
}

int32_t AudioPolicyServer::SetSystemVolumeLevelWithDeviceInternal(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, DeviceType deviceType)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelWithDeviceInternal streamType: %{public}d, volumeLevel: %{public}d, "
        "updateUi: %{public}d, deviceType: %{public}d", streamType, volumeLevel, isUpdateUi, deviceType);
    if (IsVolumeUnadjustable()) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }
    bool mute = GetStreamMuteInternal(streamType);
    return SetSingleStreamVolumeWithDevice(streamType, volumeLevel, isUpdateUi, mute, deviceType);
}

void AudioPolicyServer::SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType, const bool& isUpdateUi)
{
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInternal(streamType);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    bool ringerModeMute = audioPolicyService_.IsRingerModeMute();
    if (audioPolicyServerHandler_ != nullptr && ringerModeMute) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

void AudioPolicyServer::UpdateMuteStateAccordingToVolLevel(AudioStreamType streamType, int32_t volumeLevel,
    bool mute, const bool& isUpdateUi)
{
    bool muteStatus = mute;
    if (volumeLevel == 0 && !mute) {
        muteStatus = true;
        audioPolicyService_.SetStreamMute(streamType, true);
    } else if (volumeLevel > 0 && mute) {
        muteStatus = false;
        audioPolicyService_.SetStreamMute(streamType, false);
    }
    SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi);
    if (VolumeUtils::IsPCVolumeEnable()) {
        // system mute status should be aligned with music mute status.
        if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC &&
            muteStatus != GetStreamMuteInternal(STREAM_SYSTEM)) {
            AUDIO_DEBUG_LOG("set system mute to %{public}d when STREAM_MUSIC.", muteStatus);
            audioPolicyService_.SetStreamMute(STREAM_SYSTEM, muteStatus);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM);
        } else if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_SYSTEM &&
            muteStatus != GetStreamMuteInternal(STREAM_MUSIC)) {
            bool isMute = (GetSystemVolumeLevelInternal(STREAM_MUSIC) == 0) ? true : false;
            AUDIO_DEBUG_LOG("set system same to music muted or level is zero to %{public}d.", isMute);
            audioPolicyService_.SetStreamMute(STREAM_SYSTEM, isMute);
            SendVolumeKeyEventCbWithUpdateUiOrNot(STREAM_SYSTEM);
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
    bool mute)
{
    bool updateRingerMode = false;
    if ((streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == AudioStreamType::STREAM_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone volume level.
        AudioRingerMode currentRingerMode = audioPolicyService_.GetRingerMode();
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

    int32_t ret = audioPolicyService_.SetSystemVolumeLevel(streamType, volumeLevel);
    if (ret == SUCCESS) {
        if (updateRingerMode) {
            ProcUpdateRingerMode();
        }
        UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute, isUpdateUi);
    } else if (ret == ERR_SET_VOL_FAILED_BY_SAFE_VOL) {
        SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi);
        AUDIO_ERR_LOG("fail to set system volume level by safe vol");
    } else {
        AUDIO_ERR_LOG("fail to set system volume level, ret is %{public}d", ret);
    }

    return ret;
}

int32_t AudioPolicyServer::SetSingleStreamVolumeWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi, bool mute, DeviceType deviceType)
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    int32_t ret = SUCCESS;
    if (curOutputDeviceType != deviceType) {
        ret = audioPolicyService_.SetSystemVolumeLevelWithDevice(streamType, volumeLevel, deviceType);
    } else {
        ret = SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi, mute);
    }
    return ret;
}

bool AudioPolicyServer::GetStreamMute(AudioStreamType streamType)
{
    if (streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) {
        bool ret = VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "GetStreamMute permission denied for stream type : %{public}d", streamType);
    }
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return GetStreamMuteInternal(streamType);
}

bool AudioPolicyServer::GetStreamMuteInternal(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
    }
    bool isMuted = audioPolicyService_.GetStreamMute(streamType);
    AUDIO_DEBUG_LOG("GetMute streamType[%{public}d],mute[%{public}d]", streamType, isMuted);
    return isMuted;
}

bool AudioPolicyServer::IsArmUsbDevice(const AudioDeviceDescriptor &desc)
{
    if (desc.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) return true;
    if (desc.deviceType_ != DEVICE_TYPE_USB_HEADSET) return false;

    return audioPolicyService_.IsArmUsbDevice(desc);
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
    }
}

int32_t AudioPolicyServer::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "SelectOutputDevice: No system permission");

    return audioPolicyService_.SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

std::string AudioPolicyServer::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType)
{
    return audioPolicyService_.GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioPolicyServer::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "SelectInputDevice: No system permission");
    int32_t ret = audioPolicyService_.SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

int32_t AudioPolicyServer::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "No system permission");

    return audioPolicyService_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioPolicyServer::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "No system permission");

    return audioPolicyService_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetExcludedDevices(AudioDeviceUsage audioDevUsage)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), vector<shared_ptr<AudioDeviceDescriptor>>(),
        "No system permission");

    return audioPolicyService_.GetExcludedDevices(audioDevUsage);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetDevices(DeviceFlag deviceFlag)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (deviceFlag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("GetDevices: No system permission");
                std::vector<std::shared_ptr<AudioDeviceDescriptor>> info = {};
                return info;
            }
            break;
        default:
            break;
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = audioPolicyService_.GetDevices(deviceFlag);

    if (!hasSystemPermission) {
        for (std::shared_ptr<AudioDeviceDescriptor> desc : deviceDescs) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetDevicesInner(DeviceFlag deviceFlag)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return {};
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = audioPolicyService_.GetDevicesInner(deviceFlag);

    return deviceDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return {};
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetOutputDevice(audioRendererFilter);
    return deviceDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("only for system app");
        return {};
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetInputDevice(audioCapturerFilter);
    return deviceDescs;
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

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, bool forceNoBTPermission)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetPreferredOutputDeviceDescriptors(rendererInfo);

    bool hasBTPermission = false;
    if (!forceNoBTPermission) {
        hasBTPermission = VerifyBluetoothPermission();
    }

    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetPreferredInputDeviceDescriptors(captureInfo);
    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

int32_t AudioPolicyServer::SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable)
{
    return audioPolicyService_.SetClientCallbacksEnable(callbackchange, enable);
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

bool AudioPolicyServer::IsStreamActive(AudioStreamType streamType)
{
    return audioPolicyService_.IsStreamActive(streamType);
}

int32_t AudioPolicyServer::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    return audioPolicyService_.SetDeviceActive(deviceType, active, uid);
}

bool AudioPolicyServer::IsDeviceActive(InternalDeviceType deviceType)
{
    return audioPolicyService_.IsDeviceActive(deviceType);
}

InternalDeviceType AudioPolicyServer::GetActiveOutputDevice()
{
    return audioPolicyService_.GetActiveOutputDevice();
}

InternalDeviceType AudioPolicyServer::GetActiveInputDevice()
{
    return audioPolicyService_.GetActiveInputDevice();
}

// deprecated since api 9.
int32_t AudioPolicyServer::SetRingerModeLegacy(AudioRingerMode ringMode)
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetRingerModeInner(ringMode);
}

int32_t AudioPolicyServer::SetRingerMode(AudioRingerMode ringMode)
{
    AUDIO_INFO_LOG("Set ringer mode to %{public}d", ringMode);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return SetRingerModeInner(ringMode);
}

int32_t AudioPolicyServer::SetRingerModeInner(AudioRingerMode ringMode)
{
    bool isPermissionRequired = false;

    if (ringMode == AudioRingerMode::RINGER_MODE_SILENT) {
        isPermissionRequired = true;
    } else {
        AudioRingerMode currentRingerMode = audioPolicyService_.GetRingerMode();
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
    int32_t ret = audioPolicyService_.SetRingerMode(ringerMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set ringer mode!");

    if (!hasUpdatedVolume) {
        // need to set volume according to ringermode
        bool muteState = (ringerMode == RINGER_MODE_NORMAL) ? false : true;
        AudioInterrupt audioInterrupt;
        GetSessionInfoInFocus(audioInterrupt);
        audioPolicyService_.SetStreamMute(STREAM_RING, muteState, audioInterrupt.streamUsage);
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
std::shared_ptr<ToneInfo> AudioPolicyServer::GetToneConfig(int32_t ltonetype, const std::string &countryCode)
{
    return audioPolicyService_.GetToneConfig(ltonetype, countryCode);
}

std::vector<int32_t> AudioPolicyServer::GetSupportedTones(const std::string &countryCode)
{
    return audioPolicyService_.GetSupportedTones(countryCode);
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
    int32_t ret = audioPolicyService_.InitPersistentMicrophoneMuteState(isMute);
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
    bool originalMicrophoneMute = IsMicrophoneMute();
    int32_t ret = audioPolicyService_.SetMicrophoneMute(isMute);
    bool newMicrophoneMute = IsMicrophoneMute();
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

int32_t AudioPolicyServer::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    AUDIO_INFO_LOG("Entered %{public}s isMute:%{public}d, type:%{public}d", __func__, isMute, type);
    bool hasPermission = VerifyPermission(MICROPHONE_CONTROL_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(hasPermission, ERR_PERMISSION_DENIED,
        "MICROPHONE_CONTROL_PERMISSION permission denied");
    WatchTimeout guard("PrivacyKit::SetMutePolicy:SetMicrophoneMutePersistent");
    bool originalMicrophoneMute = IsMicrophoneMute();
    int32_t ret = PrivacyKit::SetMutePolicy(POLICY_TYPE_MAP[type], MICPHONE_CALLER, isMute,
        IPCSkeleton::GetCallingTokenID());
    guard.CheckCurrTimeout();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("PrivacyKit SetMutePolicy failed ret is %{public}d", ret);
        return ret;
    }
    ret = audioPolicyService_.SetMicrophoneMutePersistent(isMute);
    bool newMicrophoneMute = IsMicrophoneMute();
    if (ret == SUCCESS && originalMicrophoneMute != newMicrophoneMute && audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = newMicrophoneMute;
        AUDIO_INFO_LOG("SendMicStateUpdatedCallback when set persistent mic mute state:%{public}d", newMicrophoneMute);
        audioPolicyServerHandler_->SendMicStateUpdatedCallback(micStateChangeEvent);
    }
    return ret;
}

bool AudioPolicyServer::GetPersistentMicMuteState()
{
    bool hasPermission = VerifyPermission(MICROPHONE_CONTROL_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(hasPermission, ERR_PERMISSION_DENIED,
        "MICROPHONE_CONTROL_PERMISSION permission denied");

    return audioPolicyService_.GetPersistentMicMuteState();
}

// deprecated since 9.
bool AudioPolicyServer::IsMicrophoneMuteLegacy()
{
    // AudioManager.IsMicrophoneMute check micphone right.
    if (!VerifyPermission(MICROPHONE_PERMISSION)) {
        AUDIO_ERR_LOG("MICROPHONE permission denied");
        return false;
    }
    return audioPolicyService_.IsMicrophoneMute();
}

bool AudioPolicyServer::IsMicrophoneMute()
{
    // AudioVolumeGroupManager.IsMicrophoneMute didn't check micphone right.
    return audioPolicyService_.IsMicrophoneMute();
}

AudioRingerMode AudioPolicyServer::GetRingerMode()
{
    std::lock_guard<std::mutex> lock(systemVolumeMutex_);
    return audioPolicyService_.GetRingerMode();
}

int32_t AudioPolicyServer::SetAudioScene(AudioScene audioScene)
{
    CHECK_AND_RETURN_RET_LOG(audioScene > AUDIO_SCENE_INVALID && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "param is invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_PHONE_CALL:
        case AUDIO_SCENE_PHONE_CHAT:
            return audioPolicyService_.SetAudioScene(audioScene);
    
        default:
            AUDIO_ERR_LOG("param is invalid: %{public}d", audioScene);
            return ERR_INVALID_PARAM;
    }
}

int32_t AudioPolicyServer::SetAudioSceneInternal(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    return audioPolicyService_.SetAudioScene(audioScene, uid, pid);
}

AudioScene AudioPolicyServer::GetAudioScene()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    return audioPolicyService_.GetAudioScene(hasSystemPermission);
}

int32_t AudioPolicyServer::SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object,
    uint32_t clientUid, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->SetAudioInterruptCallback(zoneID, sessionID, object, clientUid);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->UnsetAudioInterruptCallback(zoneID, sessionID);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::SetAudioManagerInterruptCallback(const int32_t /* clientId */,
                                                            const sptr<IRemoteObject> &object)
{
    if (interruptService_ != nullptr) {
        return interruptService_->SetAudioManagerInterruptCallback(object);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::UnsetAudioManagerInterruptCallback(const int32_t /* clientId */)
{
    if (interruptService_ != nullptr) {
        return interruptService_->UnsetAudioManagerInterruptCallback();
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::SetQueryClientTypeCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    return audioPolicyService_.SetQueryClientTypeCallback(object);
}

int32_t AudioPolicyServer::SetAudioClientInfoMgrCallback(const sptr<IRemoteObject> &object)
{
    if (!PermissionUtil::VerifyIsAudio()) {
        AUDIO_ERR_LOG("not audio calling!");
        return ERR_OPERATION_FAILED;
    }
    return audioPolicyService_.SetAudioClientInfoMgrCallback(object);
}

int32_t AudioPolicyServer::RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    if (interruptService_ != nullptr) {
        return interruptService_->RequestAudioFocus(clientId, audioInterrupt);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
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

int32_t AudioPolicyServer::ActivateAudioInterrupt(
    AudioInterrupt &audioInterrupt, const int32_t zoneID, const bool isUpdatedAudioStrategy)
{
    Trace trace("AudioPolicyServer::ActivateAudioInterrupt");
    if (interruptService_ != nullptr) {
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
        return interruptService_->ActivateAudioInterrupt(zoneID, audioInterrupt, isUpdatedAudioStrategy);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->DeactivateAudioInterrupt(zoneID, audioInterrupt);
    }
    return ERR_UNKNOWN;
}

void AudioPolicyServer::OnAudioStreamRemoved(const uint64_t sessionID)
{
    CHECK_AND_RETURN_LOG(audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is nullptr");
    audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionID, false);
}

void AudioPolicyServer::ProcessSessionRemoved(const uint64_t sessionID, const int32_t zoneID)
{
    AUDIO_DEBUG_LOG("Removed SessionId: %{public}" PRIu64, sessionID);
}

void AudioPolicyServer::ProcessSessionAdded(SessionEvent sessionEvent)
{
    AUDIO_DEBUG_LOG("Added Session");
}

void AudioPolicyServer::ProcessorCloseWakeupSource(const uint64_t sessionID)
{
    audioPolicyService_.CloseWakeUpAudioCapturer();
}

AudioStreamType AudioPolicyServer::GetStreamInFocus(const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->GetStreamInFocus(zoneID);
    }
    return STREAM_MUSIC;
}

AudioStreamType AudioPolicyServer::GetStreamInFocusByUid(const int32_t uid, const int32_t zoneID)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return STREAM_MUSIC;
    }

    if (interruptService_ != nullptr) {
        return interruptService_->GetStreamInFocusByUid(uid, zoneID);
    }
    return STREAM_MUSIC;
}

int32_t AudioPolicyServer::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->GetSessionInfoInFocus(audioInterrupt, zoneID);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
    const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->GetAudioFocusInfoList(zoneID, focusInfoList);
    }
    return ERR_UNKNOWN;
}

bool AudioPolicyServer::VerifyPermission(const std::string &permissionName, uint32_t tokenId, bool isRecording)
{
    AUDIO_DEBUG_LOG("Verify permission [%{public}s]", permissionName.c_str());

    if (!isRecording) {
#ifdef AUDIO_BUILD_VARIANT_ROOT
        // root user case for auto test
        uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
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

    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, USE_BLUETOOTH_PERMISSION);
    CHECK_AND_RETURN_RET(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED, false);

    return true;
}

int32_t AudioPolicyServer::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
#ifdef AUDIO_BUILD_VARIANT_ROOT
    // Only root users should have access to this api
    if (ROOT_UID != IPCSkeleton::GetCallingUid()) {
        AUDIO_INFO_LOG("Unautorized user. Cannot modify channel");
        return ERR_PERMISSION_DENIED;
    }

    return audioPolicyService_.ReconfigureAudioChannel(count, deviceType);
#else
    // this api is not supported
    return ERR_NOT_SUPPORTED;
#endif
}

void AudioPolicyServer::GetStreamVolumeInfoMap(StreamVolumeInfoMap& streamVolumeInfos)
{
    audioPolicyService_.GetStreamVolumeInfoMap(streamVolumeInfos);
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
}

void AudioPolicyServer::PolicyDataDump(std::string &dumpString)
{
    AudioDevicesDump(dumpString);
    AudioModeDump(dumpString);
    AudioVolumeDump(dumpString);
    AudioInterruptZoneDump(dumpString);
    AudioPolicyParserDump(dumpString);
    AudioStreamDump(dumpString);
    XmlParsedDataMapDump(dumpString);
    EffectManagerInfoDump(dumpString);
    MicrophoneMuteInfoDump(dumpString);
    AudioSessionInfoDump(dumpString);
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
    AppendFormat(dumpString, "  -h\t\t\t|help text for hidumper audio\n");
    AppendFormat(dumpString, "  -d\t\t\t|dump devices info\n");
    AppendFormat(dumpString, "  -m\t\t\t|dump ringer mode and call status\n");
    AppendFormat(dumpString, "  -v\t\t\t|dump stream volume info\n");
    AppendFormat(dumpString, "  -az\t\t\t|dump audio in interrupt zone info\n");
    AppendFormat(dumpString, "  -apc\t\t\t|dump audio policy config xml parser info\n");
    AppendFormat(dumpString, "  -s\t\t\t|dump stream info\n");
    AppendFormat(dumpString, "  -xp\t\t\t|dump xml data map\n");
    AppendFormat(dumpString, "  -e\t\t\t|dump audio effect manager Info\n");
    AppendFormat(dumpString, "  -as\t\t\t|dump audio session info\n");
}

int32_t AudioPolicyServer::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo)
{
    std::string bundleName = "";
    bool isFastControlled = audioPolicyService_.getFastControlParam();
    if (isFastControlled && rendererInfo.rendererFlags == AUDIO_FLAG_MMAP) {
        bundleName = GetBundleName();
        AUDIO_INFO_LOG("bundleName %{public}s", bundleName.c_str());
        return audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, bundleName);
    }
    return audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, "");
}

int32_t AudioPolicyServer::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    return audioPolicyService_.GetPreferredInputStreamType(capturerInfo);
}

int32_t AudioPolicyServer::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object)
{
    auto callerPid = IPCSkeleton::GetCallingPid();
    streamChangeInfo.audioRendererChangeInfo.callerPid = callerPid;
    streamChangeInfo.audioCapturerChangeInfo.callerPid = callerPid;

    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    int appVolume = GetAppVolumeLevel(callerUid);
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
    int32_t apiVersion = GetApiTargerVersion();
    return audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioPolicyServer::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    Trace trace("AudioPolicyServer::UpdateTracker");
    auto callerPid = IPCSkeleton::GetCallingPid();
    streamChangeInfo.audioRendererChangeInfo.callerPid = callerPid;
    streamChangeInfo.audioCapturerChangeInfo.callerPid = callerPid;

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
    int appVolume = GetAppVolumeLevel(streamChangeInfo.audioRendererChangeInfo.clientUID);
    streamChangeInfo.audioRendererChangeInfo.appVolume = appVolume;
    int32_t ret = audioPolicyService_.UpdateTracker(mode, streamChangeInfo);
    if (streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_PAUSED ||
        streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_STOPPED ||
        streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_RELEASED) {
        OffloadStreamCheck(OFFLOAD_NO_SESSION_ID, streamChangeInfo.audioRendererChangeInfo.sessionId);
    }
    if (streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_RUNNING) {
        OffloadStreamCheck(streamChangeInfo.audioRendererChangeInfo.sessionId, OFFLOAD_NO_SESSION_ID);
    }
    return ret;
}

void AudioPolicyServer::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    auto callerPid = IPCSkeleton::GetCallingPid();
    streamChangeInfo.audioRendererChangeInfo.callerPid = callerPid;

    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    AUDIO_DEBUG_LOG("[caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
        AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
            streamChangeInfo.audioRendererChangeInfo.clientUID);
    }
    audioPolicyService_.FetchOutputDeviceForTrack(streamChangeInfo, reason);
}

void AudioPolicyServer::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    auto callerPid = IPCSkeleton::GetCallingPid();
    streamChangeInfo.audioCapturerChangeInfo.callerPid = callerPid;

    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    AUDIO_DEBUG_LOG("[caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
        AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
            streamChangeInfo.audioCapturerChangeInfo.clientUID);
    }
    audioPolicyService_.FetchInputDeviceForTrack(streamChangeInfo);
}

int32_t AudioPolicyServer::GetCurrentRendererChangeInfos(
    std::vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return audioPolicyService_.GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
}

int32_t AudioPolicyServer::GetCurrentCapturerChangeInfos(
    std::vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
}

void AudioPolicyServer::RegisterClientDeathRecipient(const sptr<IRemoteObject> &object, DeathRecipientId id)
{
    AUDIO_DEBUG_LOG("Register clients death recipient!! RecipientId: %{public}d", id);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    CHECK_AND_RETURN_LOG(object != nullptr, "Client proxy obj NULL!!");

    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    if (id == TRACKER_CLIENT && std::find(clientDiedListenerState_.begin(), clientDiedListenerState_.end(), uid)
        != clientDiedListenerState_.end()) {
        AUDIO_INFO_LOG("Tracker has been registered for %{public}d!", uid);
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
            clientDiedListenerState_.push_back(uid);
        }
        if (!result) {
            AUDIO_WARNING_LOG("failed to add deathRecipient");
        }
    }
}

void AudioPolicyServer::RegisteredTrackerClientDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredTrackerClient died: remove entry, pid %{public}d uid %{public}d", pid, uid);
    audioPolicyService_.RemoveDeviceForUid(uid);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    audioPolicyService_.RegisteredTrackerClientDied(uid);

    auto filter = [&uid](int val) {
        return uid == val;
    };
    clientDiedListenerState_.erase(std::remove_if(clientDiedListenerState_.begin(), clientDiedListenerState_.end(),
        filter), clientDiedListenerState_.end());
}

void AudioPolicyServer::RegisteredStreamListenerClientDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredStreamListenerClient died: remove entry, pid %{public}d uid %{public}d", pid, uid);
    audioPolicyService_.RemoveDeviceForUid(uid);
    if (pid == lastMicMuteSettingPid_) {
        // The last app with the non-persistent microphone setting died, restore the default non-persistent value
        AUDIO_INFO_LOG("Cliet died and reset non-persist mute state");
        audioPolicyService_.SetMicrophoneMute(false);
    }
    if (interruptService_ != nullptr && interruptService_->IsAudioSessionActivated(pid)) {
        interruptService_->DeactivateAudioSession(pid);
    }
    audioPolicyService_.ReduceAudioPolicyClientProxyMap(pid);
}

int32_t AudioPolicyServer::ResumeStreamState()
{
    AUDIO_INFO_LOG("AVSession is not alive.");
    return audioPolicyService_.ResumeStreamState();
}

// LCOV_EXCL_START
int32_t AudioPolicyServer::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, StreamUsage streamUsage)
{
    constexpr int32_t avSessionUid = 6700; // "uid" : "av_session"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by av_session
    CHECK_AND_RETURN_RET_LOG(callerUid == avSessionUid, ERROR,
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

    return audioPolicyService_.UpdateStreamState(clientUid, setStateEvent);
}
// LCOV_EXCL_STOP

int32_t AudioPolicyServer::GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "No system permission");

    infos = audioPolicyService_.GetVolumeGroupInfos();
    auto filter = [&networkId](const sptr<VolumeGroupInfo>& info) {
        return networkId != info->networkId_;
    };
    infos.erase(std::remove_if(infos.begin(), infos.end(), filter), infos.end());

    return SUCCESS;
}

int32_t AudioPolicyServer::GetNetworkIdByGroupId(int32_t groupId, std::string &networkId)
{
    auto volumeGroupInfos = audioPolicyService_.GetVolumeGroupInfos();

    auto filter = [&groupId](const sptr<VolumeGroupInfo>& info) {
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
    const AudioParamKey key, const std::string& condition, const std::string& value)
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
    const std::string& condition)
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
    const std::string& condition)
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

    InterruptEventInternal interruptEvent {type, forceType, hint, 0.2f};
    if (server_ != nullptr) {
        server_->ProcessRemoteInterrupt(sessionIds, interruptEvent);
    }
}

void AudioPolicyServer::RemoteParameterCallback::StateOnChange(const std::string networkId,
    const std::string& condition, const std::string& value)
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
        server_->audioPolicyService_.NotifyRemoteRenderState(networkId, contentDesStr, value);
    } else if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_MIC) {
        AUDIO_INFO_LOG("StateOnChange: ERR_EVENT of DAUDIO_DEV_TYPE_MIC.");
    } else {
        AUDIO_ERR_LOG("StateOnChange: Device type is not supported, contentDes %{public}s.", contentDesStr.c_str());
    }
}

void AudioPolicyServer::PerStateChangeCbCustomizeCallback::PermStateChangeCallback(
    Security::AccessToken::PermStateChangeInfo& result)
{
    ready_ = true;
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    int32_t res = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(result.tokenID, hapTokenInfo);
    if (res < 0) {
        AUDIO_ERR_LOG("Call GetHapTokenInfo fail.");
    }

    bool targetMuteState = (result.permStateChangeType > 0) ? false : true;
    int32_t appUid = getUidByBundleName(hapTokenInfo.bundleName, hapTokenInfo.userID);
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

int32_t AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName(std::string bundle_name, int user_id)
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS);
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():getUidByBundleName");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        return ERR_INVALID_PARAM;
    }
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        return ERR_INVALID_PARAM;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        return ERR_INVALID_PARAM;
    }
    WatchTimeout reguard("bundleMgrProxy->GetUidByBundleName:getUidByBundleName");
    int32_t iUid = bundleMgrProxy->GetUidByBundleName(bundle_name, user_id);
    reguard.CheckCurrTimeout();

    return iUid;
}

void AudioPolicyServer::RegisterParamCallback()
{
    AUDIO_INFO_LOG("RegisterParamCallback");
    remoteParameterCallback_ = std::make_shared<RemoteParameterCallback>(this);
    audioPolicyService_.SetParameterCallback(remoteParameterCallback_);
    // regiest policy provider in audio server
    audioPolicyService_.RegiestPolicy();
}

void AudioPolicyServer::RegisterBluetoothListener()
{
    AUDIO_INFO_LOG("OnAddSystemAbility bluetooth service start");
    audioPolicyService_.RegisterBluetoothListener();
}

void AudioPolicyServer::SubscribeAccessibilityConfigObserver()
{
    AUDIO_INFO_LOG("SubscribeAccessibilityConfigObserver");
    audioPolicyService_.SubscribeAccessibilityConfigObserver();
}

int32_t AudioPolicyServer::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    AUDIO_INFO_LOG("key: %{public}s, uri: %{public}s", key.c_str(), uri.c_str());
    return audioPolicyService_.SetSystemSoundUri(key, uri);
}

std::string AudioPolicyServer::GetSystemSoundUri(const std::string &key)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return "";
    }
    AUDIO_INFO_LOG("key: %{public}s", key.c_str());
    return audioPolicyService_.GetSystemSoundUri(key);
}

float AudioPolicyServer::GetMinStreamVolume()
{
    return audioPolicyService_.GetMinStreamVolume();
}

float AudioPolicyServer::GetMaxStreamVolume()
{
    return audioPolicyService_.GetMaxStreamVolume();
}

int32_t AudioPolicyServer::GetMaxRendererInstances()
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
    return audioPolicyService_.GetMaxRendererInstances();
}

void AudioPolicyServer::RegisterDataObserver()
{
    audioPolicyService_.RegisterDataObserver();
}

int32_t AudioPolicyServer::QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    int32_t ret = audioPolicyService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    return ret;
}

int32_t AudioPolicyServer::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    return audioPolicyService_.GetHardwareOutputSamplingRate(desc);
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyServer::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    std::vector<sptr<MicrophoneDescriptor>> micDescs =
        audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(sessionId);
    return micDescs;
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyServer::GetAvailableMicrophones()
{
    std::vector<sptr<MicrophoneDescriptor>> micDescs = audioPolicyService_.GetAvailableMicrophones();
    return micDescs;
}

int32_t AudioPolicyServer::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_BLUETOOTH_SA) {
        AUDIO_ERR_LOG("SetDeviceAbsVolumeSupported: Error caller uid: %{public}d", callerUid);
        return ERROR;
    }
    return audioPolicyService_.SetDeviceAbsVolumeSupported(macAddress, support);
}

bool AudioPolicyServer::IsAbsVolumeScene()
{
    return audioPolicyService_.IsAbsVolumeScene();
}

int32_t AudioPolicyServer::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
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
    int32_t ret = audioPolicyService_.SetA2dpDeviceVolume(macAddress, volume);

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamInFocus;
    volumeEvent.volume = volume;
    volumeEvent.updateUi = updateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;

    if (ret == SUCCESS && audioPolicyServerHandler_!= nullptr && audioPolicyService_.IsCurrentActiveDeviceA2dp()) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> deviceDescs = {};
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
            return deviceDescs;
    }

    deviceDescs = audioPolicyService_.GetAvailableDevices(usage);

    if (!hasSystemPermission) {
        for (auto &desc : deviceDescs) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDevices = {};
    for (auto &desc : deviceDescs) {
        deviceDevices.push_back(std::make_shared<AudioDeviceDescriptor>(*desc));
    }

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDevices);
        deviceDescs.clear();
        for (auto &dec : deviceDevices) {
            deviceDescs.push_back(make_shared<AudioDeviceDescriptor>(*dec));
        }
    }

    return deviceDescs;
}

int32_t AudioPolicyServer::SetAvailableDeviceChangeCallback(const int32_t /*clientId*/, const AudioDeviceUsage usage,
    const sptr<IRemoteObject> &object)
{
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

int32_t AudioPolicyServer::UnsetAvailableDeviceChangeCallback(const int32_t /*clientId*/, AudioDeviceUsage usage)
{
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return audioPolicyService_.UnsetAvailableDeviceChangeCallback(clientPid, usage);
}

int32_t AudioPolicyServer::OffloadStopPlaying(const AudioInterrupt &audioInterrupt)
{
    return audioPolicyService_.OffloadStopPlaying(std::vector<int32_t>(1, audioInterrupt.streamId));
}

int32_t AudioPolicyServer::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> lock(descLock_);
    audioPolicyService_.ConfigDistributedRoutingRole(descriptor, type);
    OnDistributedRoutingRoleChange(descriptor, type);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetDistributedRoutingRoleCallback set listener object is nullptr");
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    sptr<IStandardAudioRoutingManagerListener> listener = iface_cast<IStandardAudioRoutingManagerListener>(object);
    if (listener != nullptr && audioPolicyServerHandler_ != nullptr) {
        listener->hasBTPermission_ = hasBTPermission;
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

bool AudioPolicyServer::IsSpatializationEnabled()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationEnabled();
}

bool AudioPolicyServer::IsSpatializationEnabled(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationEnabled(address);
}

bool AudioPolicyServer::IsSpatializationEnabledForCurrentDevice()
{
    return audioSpatializationService_.IsSpatializationEnabledForCurrentDevice();
}

int32_t AudioPolicyServer::SetSpatializationEnabled(const bool enable)
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
    const bool enable)
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

bool AudioPolicyServer::IsHeadTrackingEnabled()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingEnabled();
}

bool AudioPolicyServer::IsHeadTrackingEnabled(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingEnabled(address);
}

int32_t AudioPolicyServer::SetHeadTrackingEnabled(const bool enable)
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
    const bool enable)
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

AudioSpatializationState AudioPolicyServer::GetSpatializationState(const StreamUsage streamUsage)
{
    return audioSpatializationService_.GetSpatializationState(streamUsage);
}

bool AudioPolicyServer::IsSpatializationSupported()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationSupported();
}

bool AudioPolicyServer::IsSpatializationSupportedForDevice(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationSupportedForDevice(address);
}

bool AudioPolicyServer::IsHeadTrackingSupported()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingSupported();
}

bool AudioPolicyServer::IsHeadTrackingSupportedForDevice(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingSupportedForDevice(address);
}

int32_t AudioPolicyServer::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
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

int32_t AudioPolicyServer::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const StreamUsage streamUsage, const sptr<IRemoteObject> &object)
{
    return audioSpatializationService_.RegisterSpatializationStateEventListener(sessionID, streamUsage, object);
}

int32_t AudioPolicyServer::UnregisterSpatializationStateEventListener(const uint32_t sessionID)
{
    return audioSpatializationService_.UnregisterSpatializationStateEventListener(sessionID);
}

int32_t AudioPolicyServer::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneID)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener object is nullptr");

    sptr<IAudioPolicyClient> callback = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener obj cast failed");

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_DEBUG_LOG("register clientPid: %{public}d", clientPid);

    bool hasBTPermission = VerifyBluetoothPermission();
    bool hasSysPermission = PermissionUtil::VerifySystemPermission();
    callback->hasBTPermission_ = hasBTPermission;
    callback->hasSystemPermission_ = hasSysPermission;
    callback->apiVersion_ = GetApiTargerVersion();
    audioPolicyService_.AddAudioPolicyClientProxyMap(clientPid, callback);

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

int32_t AudioPolicyServer::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
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
    return audioPolicyService_.SetCallDeviceActive(deviceType, active, address, uid);
}

std::shared_ptr<AudioDeviceDescriptor> AudioPolicyServer::GetActiveBluetoothDevice()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return make_shared<AudioDeviceDescriptor>();
    }

    auto btdevice = audioPolicyService_.GetActiveBluetoothDevice();

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        btdevice->deviceName_ = "";
        btdevice->macAddress_ = "";
    }

    return btdevice;
}

std::string AudioPolicyServer::GetBundleName()
{
    AppExecFwk::BundleInfo bundleInfo = GetBundleInfoFromUid(IPCSkeleton::GetCallingUid());
    return bundleInfo.name;
}

ConverterConfig AudioPolicyServer::GetConverterConfig()
{
    return audioPolicyService_.GetConverterConfig();
}

AudioSpatializationSceneType AudioPolicyServer::GetSpatializationSceneType()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return SPATIALIZATION_SCENE_TYPE_MUSIC;
    }
    return audioSpatializationService_.GetSpatializationSceneType();
}

int32_t AudioPolicyServer::SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType)
{
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
    return audioPolicyService_.DisableSafeMediaVolume();
}

AppExecFwk::BundleInfo AudioPolicyServer::GetBundleInfoFromUid(int32_t callingUid)
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS);
    std::string bundleName {""};
    AppExecFwk::BundleInfo bundleInfo;
    WatchTimeout guard("SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager():GetBundleInfoFromUid");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, bundleInfo, "systemAbilityManager is nullptr");
    guard.CheckCurrTimeout();

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_PRELOG(remoteObject != nullptr, bundleInfo, "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, bundleInfo, "bundleMgrProxy is nullptr");

    WatchTimeout reguard("bundleMgrProxy->GetNameForUid:GetBundleInfoFromUid");
    bundleMgrProxy->GetNameForUid(callingUid, bundleName);

    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);
    reguard.CheckCurrTimeout();

    return bundleInfo;
}

int32_t AudioPolicyServer::GetApiTargerVersion()
{
    AppExecFwk::BundleInfo bundleInfo = GetBundleInfoFromUid(IPCSkeleton::GetCallingUid());

    // Taking remainder of large integers
    int32_t apiTargetversion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_REMAINDER;
    return apiTargetversion;
}

bool AudioPolicyServer::IsHighResolutionExist()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return false;
    }
    return isHighResolutionExist_;
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

float AudioPolicyServer::GetMaxAmplitude(int32_t deviceId)
{
    AudioInterrupt audioInterrupt;
    GetSessionInfoInFocus(audioInterrupt);
    return audioPolicyService_.GetMaxAmplitude(deviceId, audioInterrupt);
}

bool AudioPolicyServer::IsHeadTrackingDataRequested(const std::string &macAddress)
{
    return audioSpatializationService_.IsHeadTrackingDataRequested(macAddress);
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

int32_t AudioPolicyServer::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return audioPolicyService_.TriggerFetchDevice(reason);
}

int32_t AudioPolicyServer::SetPreferredDevice(const PreferredType preferredType,
    const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("No permission");
        return ERROR;
    }
    return audioPolicyUtils_.SetPreferredDevice(preferredType, desc, uid, "SetPreferredDevice");
}

void AudioPolicyServer::SaveRemoteInfo(const std::string &networkId, DeviceType deviceType)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("No permission");
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> newMediaDescriptor =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1, ROUTER_TYPE_USER_SELECT).front();
    std::shared_ptr<AudioDeviceDescriptor> newCallDescriptor =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1,
        ROUTER_TYPE_USER_SELECT).front();
    if (networkId == newMediaDescriptor->networkId_ && deviceType == newMediaDescriptor->deviceType_) {
        audioPolicyUtils_.SetPreferredDevice(AUDIO_MEDIA_RENDER, std::make_shared<AudioDeviceDescriptor>());
    }
    if (networkId == newCallDescriptor->networkId_ && deviceType == newCallDescriptor->deviceType_) {
        audioPolicyUtils_.SetPreferredDevice(AUDIO_CALL_RENDER, std::make_shared<AudioDeviceDescriptor>(), SYSTEM_UID,
            "SaveRemoteInfo");
    }
    audioDeviceManager_.SaveRemoteInfo(networkId, deviceType);
}

int32_t AudioPolicyServer::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "SetAudioDeviceAnahsCallback object is nullptr");
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return audioPolicyService_.SetAudioDeviceAnahsCallback(object);
}

int32_t AudioPolicyServer::UnsetAudioDeviceAnahsCallback()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        return ERROR;
    }
    return audioPolicyService_.UnsetAudioDeviceAnahsCallback();
}

void AudioPolicyServer::NotifyAccountsChanged(const int &id)
{
    CHECK_AND_RETURN_LOG(interruptService_ != nullptr, "interruptService_ is nullptr");
    interruptService_->ClearAudioFocusInfoListOnAccountsChanged(id);
    audioPolicyService_.NotifyAccountsChanged(id);
}

int32_t AudioPolicyServer::MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType)
{
    return audioPolicyService_.MoveToNewPipe(sessionId, pipeType);
}

int32_t AudioPolicyServer::SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object)
{
    return audioPolicyService_.SetAudioConcurrencyCallback(sessionID, object);
}

int32_t AudioPolicyServer::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    return audioPolicyService_.UnsetAudioConcurrencyCallback(sessionID);
}

int32_t AudioPolicyServer::ActivateAudioConcurrency(const AudioPipeType &pipeType)
{
    Trace trace("AudioPolicyServer::ActivateAudioConcurrency" + std::to_string(pipeType));
    return audioPolicyService_.ActivateAudioConcurrency(pipeType);
}

void AudioPolicyServer::CheckHibernateState(bool hibernate)
{
    audioPolicyService_.CheckHibernateState(hibernate);
}

void AudioPolicyServer::UpdateSafeVolumeByS4()
{
    audioPolicyService_.UpdateSafeVolumeByS4();
}

void AudioPolicyServer::CheckConnectedDevice()
{
    audioPolicyService_.CheckConnectedDevice();
}

void AudioPolicyServer::SetDeviceConnectedFlagFalseAfterDuration()
{
    audioPolicyService_.SetDeviceConnectedFlagFalseAfterDuration();
}

int32_t AudioPolicyServer::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    if (!VerifyPermission(MANAGE_SYSTEM_AUDIO_EFFECTS)) {
        AUDIO_ERR_LOG("MANAGE_SYSTEM_AUDIO_EFFECTS permission check failed");
        return ERR_PERMISSION_DENIED;
    }
    audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
    return AUDIO_OK;
}

int32_t AudioPolicyServer::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
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
    return audioPolicyService_.GetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.GetSupportedAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyServer::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.GetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyServer::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.SetAudioEnhanceProperty(propertyArray);
}

int32_t AudioPolicyServer::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.SetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_SYSTEM_PERMISSION_DENIED, "No system permission");
    return audioPolicyService_.GetAudioEffectProperty(propertyArray);
}

int32_t AudioPolicyServer::InjectInterruption(const std::string networkId, InterruptEvent &event)
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

int32_t AudioPolicyServer::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    if (!CheckAudioSessionStrategy(strategy)) {
        AUDIO_ERR_LOG("The audio session strategy is invalid!");
        return ERR_INVALID_PARAM;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("activate audio session with concurrencyMode %{public}d for pid %{public}d",
        static_cast<int32_t>(strategy.concurrencyMode), callerPid);
    return interruptService_->ActivateAudioSession(callerPid, strategy);
}

int32_t AudioPolicyServer::DeactivateAudioSession()
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("deactivate audio session for pid %{public}d", callerPid);
    return interruptService_->DeactivateAudioSession(callerPid);
}

bool AudioPolicyServer::IsAudioSessionActivated()
{
    if (interruptService_ == nullptr) {
        AUDIO_ERR_LOG("interruptService_ is nullptr!");
        return false;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    bool isActive = interruptService_->IsAudioSessionActivated(callerPid);
    AUDIO_INFO_LOG("callerPid %{public}d, isSessionActive: %{public}d.", callerPid, isActive);
    return isActive;
}

int32_t AudioPolicyServer::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_CAR_DISTRIBUTED_ENGINE_SA) {
        AUDIO_ERR_LOG("callerUid %{public}d is not allow LoadSplitModule", callerUid);
        return ERR_PERMISSION_DENIED;
    }
    return audioPolicyService_.LoadSplitModule(splitArgs, networkId);
}

bool AudioPolicyServer::IsAllowedPlayback(const int32_t &uid, const int32_t &pid)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != MEDIA_SERVICE_UID) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        return audioPolicyService_.IsAllowedPlayback(callerUid, callerPid);
    }
    return audioPolicyService_.IsAllowedPlayback(uid, pid);
}

int32_t AudioPolicyServer::SetVoiceRingtoneMute(bool isMute)
{
    constexpr int32_t foundationUid = 5523; // "uid" : "foundation"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by foundation
    CHECK_AND_RETURN_RET_LOG(callerUid == foundationUid, ERROR,
        "SetVoiceRingtoneMute callerUid is error: not foundation");
    AUDIO_INFO_LOG("Set VoiceRingtone is %{public}d", isMute);
    return audioPolicyService_.SetVoiceRingtoneMute(isMute);
}

int32_t AudioPolicyServer::SetVirtualCall(const bool isVirtual)
{
    constexpr int32_t meetServiceUid = 5523; // "uid" : "meetservice"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by meetservice
    CHECK_AND_RETURN_RET_LOG(callerUid == meetServiceUid, ERROR,
        "SetVirtualCall callerUid is error: not meetservice");
    AUDIO_INFO_LOG("Set VirtualCall is %{public}d", isVirtual);
    return audioPolicyService_.SetVirtualCall(isVirtual);
}

int32_t AudioPolicyServer::SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    const bool isConnected)
{
    AUDIO_INFO_LOG("deviceType: %{public}d, deviceRole: %{public}d, isConnected: %{public}d",
        desc->deviceType_, desc->deviceRole_, isConnected);
    auto callerUid = IPCSkeleton::GetCallingUid();
    CHECK_AND_RETURN_RET_LOG(callerUid == UID_TV_PROCESS_SA, ERR_PERMISSION_DENIED, "uid permission denied");
    bool ret = VerifyPermission(MANAGE_AUDIO_CONFIG);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "MANAGE_AUDIO_CONFIG permission denied");
    audioPolicyService_.OnDeviceStatusUpdated(*desc, isConnected);
    return SUCCESS;
}

int32_t AudioPolicyServer::SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object)
{
    constexpr int32_t avSessionUid = 6700; // "uid" : "av_session"
    auto callerUid = IPCSkeleton::GetCallingUid();
    // This function can only be used by av_session
    CHECK_AND_RETURN_RET_LOG(callerUid == avSessionUid, ERROR,
        "UpdateStreamState callerUid is error: not av_session");
    return audioPolicyService_.SetQueryAllowedPlaybackCallback(object);
}

void AudioPolicyServer::UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID)
{
    audioDeviceManager_.UpdateDefaultOutputDeviceWhenStarting(sessionID);
    audioPolicyService_.TriggerFetchDevice();
}

void AudioPolicyServer::UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID)
{
    audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(sessionID);
    audioPolicyService_.TriggerFetchDevice();
}
} // namespace AudioStandard
} // namespace OHOS
