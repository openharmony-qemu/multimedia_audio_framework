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
#define LOG_TAG "AudioPolicyServer"
#endif

#include "audio_policy_server.h"

#include <csignal>
#include <memory>
#include <unordered_set>
#include <vector>
#include <condition_variable>

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#include "key_event.h"
#include "key_option.h"
#endif
#include "power_mgr_client.h"

#include "privacy_kit.h"
#include "accesstoken_kit.h"
#include "permission_state_change_info.h"
#include "token_setproc.h"
#include "tokenid_kit.h"
#include "want.h"
#include "common_event_manager.h"

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_policy_manager_listener_proxy.h"
#include "audio_routing_manager_listener_proxy.h"
#include "i_standard_audio_policy_manager_listener.h"
#include "microphone_descriptor.h"
#include "parameter.h"
#include "parameters.h"

#include "media_monitor_manager.h"

using OHOS::Security::AccessToken::PrivacyKit;
using OHOS::Security::AccessToken::TokenIdKit;
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t PARAMS_VOLUME_NUM = 5;
constexpr int32_t PARAMS_INTERRUPT_NUM = 4;
constexpr int32_t PARAMS_RENDER_STATE_NUM = 2;
constexpr int32_t EVENT_DES_SIZE = 60;
constexpr int32_t ADAPTER_STATE_CONTENT_DES_SIZE = 60;
constexpr int32_t API_VERSION_REMAINDER = 1000;
constexpr int32_t API_VERSION_14 = 14; // for deprecated since 9
constexpr uid_t UID_CAST_ENGINE_SA = 5526;
constexpr uid_t UID_AUDIO = 1041;
constexpr uid_t UID_FOUNDATION_SA = 5523;
constexpr uid_t UID_BLUETOOTH_SA = 1002;
constexpr int64_t OFFLOAD_NO_SESSION_ID = -1;
constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;

REGISTER_SYSTEM_ABILITY_BY_ID(AudioPolicyServer, AUDIO_POLICY_SERVICE_ID, true)

std::map<PolicyType, uint32_t> POLICY_TYPE_MAP = {
    {PolicyType::EDM_POLICY_TYPE, 0},
    {PolicyType::PRIVACY_POLCIY_TYPE, 1},
    {PolicyType::TEMPORARY_POLCIY_TYPE, 2}
};

AudioPolicyServer::AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
      audioPolicyService_(AudioPolicyService::GetAudioPolicyService()),
      audioSpatializationService_(AudioSpatializationService::GetAudioSpatializationService()),
      audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter())
{
    volumeStep_ = system::GetIntParameter("const.multimedia.audio.volumestep", 1);
    AUDIO_INFO_LOG("Get volumeStep parameter success %{public}d", volumeStep_);

    powerStateCallbackRegister_ = false;
    volumeApplyToAll_ = system::GetBoolParameter("const.audio.volume_apply_to_all", false);
    if (volumeApplyToAll_) {
        audioPolicyService_.SetNormalVoipFlag(true);
    }
}

void AudioPolicyServer::OnDump()
{
    return;
}

void AudioPolicyServer::OnStart()
{
    AUDIO_INFO_LOG("Audio policy server on start");

    interruptService_ = std::make_shared<AudioInterruptService>();
    interruptService_->Init(this);

    audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    audioPolicyServerHandler_->Init(interruptService_);

    interruptService_->SetCallbackHandler(audioPolicyServerHandler_);

    if (audioPolicyService_.SetAudioStreamRemovedCallback(this)) {
        AUDIO_ERR_LOG("SetAudioStreamRemovedCallback failed");
    }
    audioPolicyService_.Init();

    AddSystemAbilityListener(AUDIO_DISTRIBUTED_SERVICE_ID);
    AddSystemAbilityListener(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
#endif
    AddSystemAbilityListener(BLUETOOTH_HOST_SYS_ABILITY_ID);
    AddSystemAbilityListener(ACCESSIBILITY_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(POWER_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
#ifdef SUPPORT_USER_ACCOUNT
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
#endif
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
    AUDIO_INFO_LOG("Audio policy server start end");
}

void AudioPolicyServer::OnStop()
{
    audioPolicyService_.Deinit();
    UnRegisterPowerStateListener();
    UnRegisterSyncHibernateListener();
    UnregisterCommonEventReceiver();
    return;
}

void AudioPolicyServer::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    AUDIO_INFO_LOG("SA Id is :%{public}d", systemAbilityId);
    int64_t stamp = ClockTime::GetCurNano();
    switch (systemAbilityId) {
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        case MULTIMODAL_INPUT_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility input service start");
            SubscribeVolumeKeyEvents();
            break;
#endif
        case DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID:
            HandleKvDataShareEvent();
            break;
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility audio service start");
            AddAudioServiceOnStart();
            break;
        case BLUETOOTH_HOST_SYS_ABILITY_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility bluetooth service start");
            RegisterBluetoothListener();
            break;
        case ACCESSIBILITY_MANAGER_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility accessibility service start");
            SubscribeAccessibilityConfigObserver();
            InitKVStore();
            RegisterDataObserver();
            break;
        case POWER_MANAGER_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility power manager service start");
            SubscribePowerStateChangeEvents();
            RegisterPowerStateListener();
            RegisterSyncHibernateListener();
            break;
        case SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN:
            AUDIO_INFO_LOG("OnAddSystemAbility os_account service start");
            SubscribeOsAccountChangeEvents();
            break;
        case COMMON_EVENT_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility common event service start");
            SubscribeCommonEvent("usual.event.DATA_SHARE_READY");
            SubscribeCommonEvent("usual.event.dms.rotation_changed");
            break;
        default:
            AUDIO_WARNING_LOG("OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    // eg. done systemAbilityId: [3001] cost 780ms
    AUDIO_INFO_LOG("done systemAbilityId: [%{public}d] cost %{public}" PRId64 " ms", systemAbilityId,
        (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND);
}

void AudioPolicyServer::HandleKvDataShareEvent()
{
    AUDIO_INFO_LOG("OnAddSystemAbility kv data service start");
    if (isInitMuteState_ == false && audioPolicyService_.IsDataShareReady()) {
        AUDIO_INFO_LOG("datashare is ready and need init mic mute state");
        InitMicrophoneMute();
    }
    InitKVStore();
    RegisterDataObserver();
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
    CHECK_AND_RETURN_RET_LOG(im != nullptr, ERR_INVALID_PARAM, "Failed to obtain INPUT manager");

    std::set<int32_t> preKeys;
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_RET_LOG(keyOption != nullptr, ERR_INVALID_PARAM, "Invalid key option");
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(keyType);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(VOLUME_KEY_DURATION);
    int32_t keySubId = im->SubscribeKeyEvent(keyOption, [=](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
        AUDIO_INFO_LOG("Receive volume key event: %{public}s.",
            (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
        std::lock_guard<std::mutex> lock(keyEventMutex_);
        AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
        if (volumeApplyToAll_) {
            streamInFocus = AudioStreamType::STREAM_ALL;
        } else {
            streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
        }
        if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(streamInFocus)) {
            AUDIO_INFO_LOG("VolumeKeyEvents: volumeKey: Up. volumeType %{public}d is mute. Unmute.", streamInFocus);
            SetStreamMuteInternal(streamInFocus, false, true);
            return;
        }
        int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus);
        if (MaxOrMinVolumeOption(volumeLevelInInt, keyType, streamInFocus)) {
            return;
        }

        volumeLevelInInt = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ?
            ++volumeLevelInInt : --volumeLevelInInt;
        SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, true);
    });
    if (keySubId < 0) {
        AUDIO_ERR_LOG("SubscribeKeyEvent: subscribing for volume key: %{public}s option failed",
            (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
    }
    return keySubId;
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
int32_t AudioPolicyServer::RegisterVolumeKeyMuteEvents()
{
    AUDIO_INFO_LOG("RegisterVolumeKeyMuteEvents: volume key: mute");
    MMI::InputManager *im = MMI::InputManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(im != nullptr, ERR_INVALID_PARAM, "Failed to obtain INPUT manager");

    std::shared_ptr<OHOS::MMI::KeyOption> keyOptionMute = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_RET_LOG(keyOptionMute != nullptr, ERR_INVALID_PARAM, "keyOptionMute: Invalid key option");
    std::set<int32_t> preKeys;
    keyOptionMute->SetPreKeys(preKeys);
    keyOptionMute->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_MUTE);
    keyOptionMute->SetFinalKeyDown(true);
    keyOptionMute->SetFinalKeyDownDuration(VOLUME_MUTE_KEY_DURATION);
    int32_t muteKeySubId = im->SubscribeKeyEvent(keyOptionMute,
        [this](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
            AUDIO_INFO_LOG("Receive volume key event: mute");
            std::lock_guard<std::mutex> lock(keyEventMutex_);
            AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
            if (volumeApplyToAll_) {
                streamInFocus = AudioStreamType::STREAM_ALL;
            } else {
                streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
            }
            bool isMuted = GetStreamMuteInternal(streamInFocus);
            SetStreamMuteInternal(streamInFocus, !isMuted, true);
        });
    if (muteKeySubId < 0) {
        AUDIO_ERR_LOG("SubscribeKeyEvent: subscribing for mute failed ");
    }
    return muteKeySubId;
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
void AudioPolicyServer::SubscribeVolumeKeyEvents()
{
    if (hasSubscribedVolumeKeyEvents_.load()) {
        AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: volume key events has been sunscirbed!");
        return;
    }

    AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: first time.");
    int32_t resultOfVolumeUp = RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP);
    int32_t resultOfVolumeDown = RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN);
    int32_t resultOfMute = RegisterVolumeKeyMuteEvents();
    if (resultOfVolumeUp >= 0 && resultOfVolumeDown >= 0 && resultOfMute >= 0) {
        hasSubscribedVolumeKeyEvents_.store(true);
    } else {
        AUDIO_ERR_LOG("SubscribeVolumeKeyEvents: failed to subscribe key events.");
        hasSubscribedVolumeKeyEvents_.store(false);
    }
}
#endif

AudioVolumeType AudioPolicyServer::GetVolumeTypeFromStreamType(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_MESSAGE:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_CALL_ASSISTANT:
            return STREAM_VOICE_CALL;
        case STREAM_RING:
        case STREAM_SYSTEM:
        case STREAM_NOTIFICATION:
        case STREAM_SYSTEM_ENFORCED:
        case STREAM_DTMF:
        case STREAM_VOICE_RING:
            return STREAM_RING;
        case STREAM_MUSIC:
        case STREAM_MEDIA:
        case STREAM_MOVIE:
        case STREAM_GAME:
        case STREAM_SPEECH:
        case STREAM_NAVIGATION:
            return STREAM_MUSIC;
        case STREAM_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_ALARM:
            return STREAM_ALARM;
        case STREAM_ACCESSIBILITY:
            return STREAM_ACCESSIBILITY;
        case STREAM_ULTRASONIC:
            return STREAM_ULTRASONIC;
        case STREAM_ALL:
            return STREAM_ALL;
        default:
            return STREAM_MUSIC;
    }
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
        case STREAM_ACCESSIBILITY:
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
        case STREAM_VOICE_RING:
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
    AccountSA::OsAccountSubscribeInfo osAccountSubscribeInfo;
    osAccountSubscribeInfo.SetOsAccountSubscribeType(AccountSA::OS_ACCOUNT_SUBSCRIBE_TYPE::SWITCHED);
    std::shared_ptr<AudioOsAccountInfo> accountInfoObs =
    std::make_shared<AudioOsAccountInfo>(osAccountSubscribeInfo, this);
    ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(accountInfoObs);
    if (errCode != SUCCESS) {
        AUDIO_ERR_LOG("SubscribeOsAccount failed");
    }
}

void AudioPolicyServer::AddAudioServiceOnStart()
{
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

    bool RegisterSuccess = PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback(powerStateCallback_);
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
        RegisterDataObserver();
        if (isInitMuteState_ == false) {
            AUDIO_INFO_LOG("receive DATA_SHARE_READY action and need init mic mute state");
            InitMicrophoneMute();
        }
    } else if (action == "usual.event.dms.rotation_changed") {
        uint32_t rotate = static_cast<uint32_t>(want.GetIntParam("rotation", 0));
        AUDIO_INFO_LOG("Set rotation to audioeffectchainmanager is %{public}d", rotate);
        audioPolicyService_.SetRotationToEffect(rotate);
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
    audioPolicyService_.CheckStreamMode(activateSessionId);
}

void AudioPolicyServer::AudioPolicyServerPowerStateCallback::OnPowerStateChanged(PowerMgr::PowerState state)
{
    policyServer_->audioPolicyService_.HandlePowerStateChanged(state);
}

void AudioPolicyServer::InitKVStore()
{
    audioPolicyService_.InitKVStore();
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

int32_t AudioPolicyServer::SetSystemVolumeLevelLegacy(AudioStreamType streamType, int32_t volumeLevel)
{
    int32_t buildApi = GetApiTargerVersion();
    if (buildApi >= API_VERSION_14 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission for legacy call");
        return ERR_PERMISSION_DENIED;
    }

    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }

    return SetSystemVolumeLevelInternal(streamType, volumeLevel, false);
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

    return SetSystemVolumeLevelInternal(streamType, volumeLevel, volumeFlag == VolumeFlag::FLAG_SHOW_SYSTEM_UI);
}

int32_t AudioPolicyServer::GetSystemVolumeLevel(AudioStreamType streamType)
{
    return GetSystemVolumeLevelInternal(streamType);
}

int32_t AudioPolicyServer::GetSystemVolumeLevelInternal(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
        AUDIO_DEBUG_LOG("GetVolume of STREAM_ALL for streamType = %{public}d ", streamType);
    }
    return audioPolicyService_.GetSystemVolumeLevel(streamType);
}

int32_t AudioPolicyServer::SetLowPowerVolume(int32_t streamId, float volume)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_FOUNDATION_SA) {
        AUDIO_ERR_LOG("SetLowPowerVolume callerUid Error: not foundation or component_schedule_service");
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

int32_t AudioPolicyServer::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    AudioStreamType streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
    if (streamInFocus == AudioStreamType::STREAM_DEFAULT) {
        streamInFocus = AudioStreamType::STREAM_MUSIC;
    }

    int32_t volumeLevelInInt = GetSystemVolumeLevel(streamInFocus);
    int32_t ret = ERROR;
    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt + volumeStep_, false);
        AUDIO_INFO_LOG("AdjustVolumeByStep Up, VolumeLevel is %{public}d", GetSystemVolumeLevel(streamInFocus));
    }

    if (adjustType == VolumeAdjustType::VOLUME_DOWN) {
        ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt - volumeStep_, false);
        AUDIO_INFO_LOG("AdjustVolumeByStep Down, VolumeLevel is %{public}d", GetSystemVolumeLevel(streamInFocus));
    }
    return ret;
}

int32_t AudioPolicyServer::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("AdjustSystemVolumeByStep: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    int32_t volumeLevelInInt = GetSystemVolumeLevel(volumeType);
    int32_t ret = ERROR;

    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt + volumeStep_, false);
        AUDIO_INFO_LOG("AdjustSystemVolumeByStep Up, VolumeLevel:%{public}d", GetSystemVolumeLevel(volumeType));
    }

    if (adjustType == VolumeAdjustType::VOLUME_DOWN) {
        ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt - volumeStep_, false);
        AUDIO_INFO_LOG("AdjustSystemVolumeByStep Down, VolumeLevel:%{public}d", GetSystemVolumeLevel(volumeType));
    }
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

int32_t AudioPolicyServer::SetStreamMuteLegacy(AudioStreamType streamType, bool mute)
{
    int32_t buildApi = GetApiTargerVersion();
    if (buildApi >= API_VERSION_14 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return SetStreamMuteInternal(streamType, mute, false);
}

int32_t AudioPolicyServer::SetStreamMute(AudioStreamType streamType, bool mute)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return SetStreamMuteInternal(streamType, mute, false);
}

int32_t AudioPolicyServer::SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetStreamMuteInternal streamType: %{public}d, mute: %{public}d, updateUi: %{public}d",
        streamType, mute, isUpdateUi);

    if (streamType == STREAM_ALL) {
        for (auto audioStreamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetMute of STREAM_ALL for StreamType = %{public}d ", audioStreamType);
            int32_t setResult = SetSingleStreamMute(audioStreamType, mute, isUpdateUi);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }

    return SetSingleStreamMute(streamType, mute, isUpdateUi);
}

int32_t AudioPolicyServer::SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi)
{
    bool updateRingerMode = false;
    if (streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone mute state.
        AudioRingerMode currentRingerMode = GetRingerMode();
        if ((currentRingerMode == RINGER_MODE_NORMAL && mute) || (currentRingerMode != RINGER_MODE_NORMAL && !mute)) {
            // When isUpdateUi is false, the func is called by others. Need to verify permission.
            if (!isUpdateUi && !VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
                AUDIO_ERR_LOG("ACCESS_NOTIFICATION_POLICY_PERMISSION permission denied for ringtone mute state!");
                return ERR_PERMISSION_DENIED;
            }
            updateRingerMode = true;
        }
    }

    int32_t result = audioPolicyService_.SetStreamMute(streamType, mute);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "Fail to set stream mute!");

    if (!mute && GetSystemVolumeLevelInternal(streamType) == 0) {
        // If mute state is set to false but volume is 0, set volume to 1
        audioPolicyService_.SetSystemVolumeLevel(streamType, 1);
    }

    if (updateRingerMode) {
        AudioRingerMode ringerMode = mute ? RINGER_MODE_VIBRATE : RINGER_MODE_NORMAL;
        AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring mute state", ringerMode);
        // Update ringer mode but no need to update mute state again.
        SetRingerModeInternal(ringerMode, true);
    }

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevelInternal(streamType);
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
    return result;
}

float AudioPolicyServer::GetSystemVolumeDb(AudioStreamType streamType)
{
    return audioPolicyService_.GetSystemVolumeDb(streamType);
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
    if (streamType == STREAM_ALL) {
        for (auto audioSteamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetVolume of STREAM_ALL, SteamType = %{public}d ", audioSteamType);
            int32_t setResult = SetSingleStreamVolume(audioSteamType, volumeLevel, isUpdateUi);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }
    return SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi);
}

int32_t AudioPolicyServer::SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi)
{
    bool updateRingerMode = false;
    if (streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) {
        // Check whether the currentRingerMode is suitable for the ringtone volume level.
        AudioRingerMode currentRingerMode = GetRingerMode();
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
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set system volume level!");

    // Update mute state according to volume level
    if (volumeLevel == 0 && !GetStreamMuteInternal(streamType)) {
        audioPolicyService_.SetStreamMute(streamType, true);
    } else if (volumeLevel > 0 && GetStreamMuteInternal(streamType)) {
        audioPolicyService_.SetStreamMute(streamType, false);
    }

    if (updateRingerMode) {
        int32_t curRingVolumeLevel = GetSystemVolumeLevelInternal(STREAM_RING);
        AudioRingerMode ringerMode = (curRingVolumeLevel > 0) ? RINGER_MODE_NORMAL : RINGER_MODE_VIBRATE;
        AUDIO_INFO_LOG("RingerMode should be set to %{public}d because of ring volume level", ringerMode);
        // Update ringer mode but no need to update volume again.
        SetRingerModeInternal(ringerMode, true);
    }

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
    return ret;
}

bool AudioPolicyServer::GetStreamMute(AudioStreamType streamType)
{
    if (streamType == AudioStreamType::STREAM_RING || streamType == AudioStreamType::STREAM_VOICE_RING) {
        bool ret = VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION);
        CHECK_AND_RETURN_RET_LOG(ret, false,
            "GetStreamMute permission denied for stream type : %{public}d", streamType);
    }

    return GetStreamMuteInternal(streamType);
}

bool AudioPolicyServer::GetStreamMuteInternal(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
        AUDIO_INFO_LOG("GetStreamMute of STREAM_ALL for streamType = %{public}d ", streamType);
    }
    return audioPolicyService_.GetStreamMute(streamType);
}

int32_t AudioPolicyServer::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
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
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
        "SelectInputDevice: No system permission");
    int32_t ret = audioPolicyService_.SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetDevices(DeviceFlag deviceFlag)
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
                std::vector<sptr<AudioDeviceDescriptor>> info = {};
                return info;
            }
            break;
        default:
            break;
    }

    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs = audioPolicyService_.GetDevices(deviceFlag);

    if (!hasSystemPermission) {
        for (sptr<AudioDeviceDescriptor> desc : deviceDescs) {
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

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetDevicesInner(DeviceFlag deviceFlag)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_AUDIO) {
        AUDIO_ERR_LOG("only for audioUid");
        return {};
    }
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs = audioPolicyService_.GetDevicesInner(deviceFlag);

    return deviceDescs;
}

int32_t AudioPolicyServer::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    // Temporarily allow only media service to use non-IPC route
    CHECK_AND_RETURN_RET_LOG(callerUid == MEDIA_SERVICE_UID, ERR_PERMISSION_DENIED, "No permission");

    return audioPolicyService_.NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
}

int32_t AudioPolicyServer::VerifyVoiceCallPermission(uint64_t fullTokenId, Security::AccessToken::AccessTokenID tokenId)
{
    bool hasSystemPermission = TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    CHECK_AND_RETURN_RET_LOG(hasSystemPermission, ERR_PERMISSION_DENIED, "No system permission");

    bool hasRecordVoiceCallPermission = VerifyPermission(RECORD_VOICE_CALL_PERMISSION, tokenId, true);
    CHECK_AND_RETURN_RET_LOG(hasRecordVoiceCallPermission, ERR_PERMISSION_DENIED, "No permission");
    return SUCCESS;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetPreferredOutputDeviceDescriptors(rendererInfo);
    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs =
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

bool AudioPolicyServer::IsStreamActive(AudioStreamType streamType)
{
    return audioPolicyService_.IsStreamActive(streamType);
}

int32_t AudioPolicyServer::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    return audioPolicyService_.SetDeviceActive(deviceType, active);
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

// deprecated since 9.
int32_t AudioPolicyServer::SetRingerModeLegacy(AudioRingerMode ringMode)
{
    AUDIO_INFO_LOG("Set ringer mode to %{public}d in legacy", ringMode);
    int32_t buildApi = GetApiTargerVersion();
    if (buildApi >= API_VERSION_14 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return SetRingerModeInner(ringMode);
}

int32_t AudioPolicyServer::SetRingerMode(AudioRingerMode ringMode)
{
    AUDIO_INFO_LOG("Set ringer mode to %{public}d", ringMode);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return SetRingerModeInner(ringMode);
}

int32_t AudioPolicyServer::SetRingerModeInner(AudioRingerMode ringMode)
{
    bool isPermissionRequired = false;

    if (ringMode == AudioRingerMode::RINGER_MODE_SILENT) {
        isPermissionRequired = true;
    } else {
        AudioRingerMode currentRingerMode = GetRingerMode();
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

int32_t AudioPolicyServer::SetRingerModeInternal(AudioRingerMode ringerMode, bool hasUpdatedVolume)
{
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
std::shared_ptr<ToneInfo> AudioPolicyServer::GetToneConfig(int32_t ltonetype)
{
    return audioPolicyService_.GetToneConfig(ltonetype);
}

std::vector<int32_t> AudioPolicyServer::GetSupportedTones()
{
    return audioPolicyService_.GetSupportedTones();
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
    bool isMicrophoneMute = isLegacy ? IsMicrophoneMuteLegacy() : IsMicrophoneMute();
    int32_t ret = audioPolicyService_.SetMicrophoneMute(isMute);
    if (ret == SUCCESS && isMicrophoneMute != isMute && audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = audioPolicyService_.IsMicrophoneMute();
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
    bool ret = VerifyPermission(MANAGE_AUDIO_CONFIG);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED,
        "MANAGE_AUDIO_CONFIG permission denied");
    lastMicMuteSettingPid_ = IPCSkeleton::GetCallingPid();
    PrivacyKit::SetMutePolicy(POLICY_TYPE_MAP[TEMPORARY_POLCIY_TYPE], MICPHONE_CALLER, isMute);
    return SetMicrophoneMuteCommon(isMute, false);
}

int32_t AudioPolicyServer::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    AUDIO_INFO_LOG("Entered %{public}s isMute:%{public}d, type:%{public}d", __func__, isMute, type);
    bool hasPermission = VerifyPermission(MICROPHONE_CONTROL_PERMISSION);
    CHECK_AND_RETURN_RET_LOG(hasPermission, ERR_PERMISSION_DENIED,
        "MICROPHONE_CONTROL_PERMISSION permission denied");
    int32_t ret = PrivacyKit::SetMutePolicy(POLICY_TYPE_MAP[type], MICPHONE_CALLER, isMute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("PrivacyKit SetMutePolicy failed ret is %{public}d", ret);
        return ret;
    }
    ret = audioPolicyService_.SetMicrophoneMutePersistent(isMute);
    if (ret == SUCCESS && audioPolicyServerHandler_ != nullptr) {
        MicStateChangeEvent micStateChangeEvent;
        micStateChangeEvent.mute = audioPolicyService_.IsMicrophoneMute();
        AUDIO_INFO_LOG("SendMicStateUpdatedCallback when set mic mute state persistent.");
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
    return audioPolicyService_.GetRingerMode();
}

int32_t AudioPolicyServer::SetAudioScene(AudioScene audioScene)
{
    CHECK_AND_RETURN_RET_LOG(audioScene > AUDIO_SCENE_INVALID && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "param is invalid");
    bool ret = PermissionUtil::VerifySystemPermission();
    CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "No system permission");
    if (audioScene == AUDIO_SCENE_CALL_START || audioScene == AUDIO_SCENE_CALL_END) {
        AUDIO_ERR_LOG("param is invalid");
        return ERR_INVALID_PARAM;
    }
    return audioPolicyService_.SetAudioScene(audioScene);
}

int32_t AudioPolicyServer::SetAudioSceneInternal(AudioScene audioScene)
{
    return audioPolicyService_.SetAudioScene(audioScene);
}

AudioScene AudioPolicyServer::GetAudioScene()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    return audioPolicyService_.GetAudioScene(hasSystemPermission);
}

int32_t AudioPolicyServer::SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object,
    const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->SetAudioInterruptCallback(zoneID, sessionID, object);
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

int32_t AudioPolicyServer::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->ActivateAudioInterrupt(zoneID, audioInterrupt);
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

bool AudioPolicyServer::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    return false;
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
    CHECK_AND_RETURN_RET_PRELOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, "Permission denied [%{public}s]", USE_BLUETOOTH_PERMISSION.c_str());

    return true;
}

bool AudioPolicyServer::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    return false;
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
}

void AudioPolicyServer::AudioDevicesDump(std::string &dumpString)
{
    audioPolicyService_.DevicesInfoDump(dumpString);
}

void AudioPolicyServer::AudioModeDump(std::string &dumpString)
{
    audioPolicyService_.AudioModeDump(dumpString);
}

void AudioPolicyServer::AudioInterruptZoneDump(std::string &dumpString)
{
    interruptService_->AudioInterruptZoneDump(dumpString);
}

void AudioPolicyServer::AudioPolicyParserDump(std::string &dumpString)
{
    audioPolicyService_.AudioPolicyParserDump(dumpString);
}

void AudioPolicyServer::AudioVolumeDump(std::string &dumpString)
{
    audioPolicyService_.StreamVolumesDump(dumpString);
}

void AudioPolicyServer::AudioStreamDump(std::string &dumpString)
{
    audioPolicyService_.AudioStreamDump(dumpString);
}

void AudioPolicyServer::XmlParsedDataMapDump(std::string &dumpString)
{
    audioPolicyService_.XmlParsedDataMapDump(dumpString);
}

void AudioPolicyServer::EffectManagerInfoDump(std::string &dumpString)
{
    audioPolicyService_.EffectManagerInfoDump(dumpString);
}

void AudioPolicyServer::MicrophoneMuteInfoDump(std::string &dumpString)
{
    audioPolicyService_.MicrophoneMuteInfoDump(dumpString);
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
}

int32_t AudioPolicyServer::GetAudioLatencyFromXml()
{
    return audioPolicyService_.GetAudioLatencyFromXml();
}

uint32_t AudioPolicyServer::GetSinkLatencyFromXml()
{
    return audioPolicyService_.GetSinkLatencyFromXml();
}

int32_t AudioPolicyServer::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo)
{
    std::string bundleName = "";
    if (rendererInfo.rendererFlags == AUDIO_FLAG_MMAP) {
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
    AUDIO_DEBUG_LOG("RegisterTracker: [caller uid: %{public}d]", callerUid);
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
    RegisterClientDeathRecipient(object, TRACKER_CLIENT);
    int32_t apiVersion = GetApiTargerVersion();
    return audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioPolicyServer::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
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
    std::vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    bool hasBTPermission = VerifyBluetoothPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return audioPolicyService_.GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
}

int32_t AudioPolicyServer::GetCurrentCapturerChangeInfos(
    std::vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
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

    pid_t uid = 0;
    if (id == TRACKER_CLIENT) {
        // Deliberately casting UID to pid_t
        uid = static_cast<pid_t>(IPCSkeleton::GetCallingUid());
    } else {
        uid = IPCSkeleton::GetCallingPid();
    }
    if (id == TRACKER_CLIENT && std::find(clientDiedListenerState_.begin(), clientDiedListenerState_.end(), uid)
        != clientDiedListenerState_.end()) {
        AUDIO_INFO_LOG("Tracker has been registered for %{public}d!", uid);
        return;
    }
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(uid);
    if (deathRecipient_ != nullptr) {
        if (id == TRACKER_CLIENT) {
            deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyServer::RegisteredTrackerClientDied,
                this, std::placeholders::_1));
        } else {
            AUDIO_INFO_LOG("RegisteredStreamListenerClientDied register!!");
            deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyServer::RegisteredStreamListenerClientDied,
                this, std::placeholders::_1));
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

void AudioPolicyServer::RegisteredTrackerClientDied(pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredTrackerClient died: remove entry, uid %{public}d", uid);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    audioPolicyService_.RegisteredTrackerClientDied(uid);

    auto filter = [&uid](int val) {
        return uid == val;
    };
    clientDiedListenerState_.erase(std::remove_if(clientDiedListenerState_.begin(), clientDiedListenerState_.end(),
        filter), clientDiedListenerState_.end());
}

void AudioPolicyServer::RegisteredStreamListenerClientDied(pid_t pid)
{
    AUDIO_INFO_LOG("RegisteredStreamListenerClient died: remove entry, uid %{public}d", pid);
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
    if (streamSetState == StreamSetState::STREAM_RESUME) {
        setState  = StreamSetState::STREAM_RESUME;
    } else if (streamSetState != StreamSetState::STREAM_PAUSE) {
        AUDIO_ERR_LOG("UpdateStreamState streamSetState value is error");
        return ERROR;
    }
    StreamSetStateEventInternal setStateEvent = {};
    setStateEvent.streamSetState = setState;
    setStateEvent.streamUsage = streamUsage;

    return audioPolicyService_.UpdateStreamState(clientUid, setStateEvent);
}

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
    char eventDes[EVENT_DES_SIZE];
    InterruptType type = INTERRUPT_TYPE_BEGIN;
    InterruptForceType forceType = INTERRUPT_SHARE;
    InterruptHint hint = INTERRUPT_HINT_NONE;

    int ret = sscanf_s(condition.c_str(), "%[^;];EVENT_TYPE=%d;FORCE_TYPE=%d;HINT_TYPE=%d;", eventDes,
        EVENT_DES_SIZE, &type, &forceType, &hint);
    CHECK_AND_RETURN_LOG(ret >= PARAMS_INTERRUPT_NUM, "[InterruptOnChange]: Failed parse condition");

    InterruptEventInternal interruptEvent {type, forceType, hint, 0.2f};
    CHECK_AND_RETURN_LOG(server_->audioPolicyServerHandler_ != nullptr, "audioPolicyServerHandler_ is nullptr");
    server_->audioPolicyServerHandler_->SendInterruptEventInternalCallback(interruptEvent);
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
            AUDIO_INFO_LOG("update using mic %{public}d for uid: %{public}d because permission changed",
                targetMuteState, appUid);
            if (targetMuteState) {
                PrivacyKit::StopUsingPermission(result.tokenID, MICROPHONE_PERMISSION);
            } else {
                PrivacyKit::StartUsingPermission(result.tokenID, MICROPHONE_PERMISSION);
            }
        }
    }
}

int32_t AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName(std::string bundle_name, int user_id)
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS);
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        return ERR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        return ERR_INVALID_PARAM;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        return ERR_INVALID_PARAM;
    }
    int32_t iUid = bundleMgrProxy->GetUidByBundleName(bundle_name, user_id);

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
    AUDIO_INFO_LOG("RegisterBluetoothListener");
    audioPolicyService_.RegisterBluetoothListener();
    RegisterCommonEventReceiver();
}

void AudioPolicyServer::SubscribeAccessibilityConfigObserver()
{
    AUDIO_INFO_LOG("SubscribeAccessibilityConfigObserver");
    audioPolicyService_.SubscribeAccessibilityConfigObserver();
}

bool AudioPolicyServer::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_INFO_LOG("IsAudioRendererLowLatencySupported server call");
    return true;
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

int32_t AudioPolicyServer::SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config,
    uint32_t appTokenId)
{
    for (auto &usg : config.filterOptions.usages) {
        if (usg != STREAM_USAGE_VOICE_COMMUNICATION) {
            continue;
        }

        if (!VerifyPermission(CAPTURER_VOICE_DOWNLINK_PERMISSION, appTokenId)) {
            AUDIO_ERR_LOG("downlink capturer permission check failed");
            return ERR_PERMISSION_DENIED;
        }
    }
    return audioPolicyService_.SetPlaybackCapturerFilterInfos(config);
}

int32_t AudioPolicyServer::SetCaptureSilentState(bool state)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_CAST_ENGINE_SA) {
        AUDIO_ERR_LOG("SetCaptureSilentState callerUid is Error: not cast_engine");
        return ERROR;
    }
    return audioPolicyService_.SetCaptureSilentState(state);
}

int32_t AudioPolicyServer::GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc)
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
    int32_t ret = audioPolicyService_.SetA2dpDeviceVolume(macAddress, volume);

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamInFocus;
    volumeEvent.volume = volume;
    volumeEvent.updateUi = updateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;

    if (ret == SUCCESS && audioPolicyServerHandler_!= nullptr) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
    return ret;
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> deviceDescs = {};
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

    std::vector<sptr<AudioDeviceDescriptor>> deviceDevices = {};
    for (auto &desc : deviceDescs) {
        deviceDevices.push_back(new(std::nothrow) AudioDeviceDescriptor(*desc));
    }

    bool hasBTPermission = VerifyBluetoothPermission();
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDevices);
        deviceDescs.clear();
        for (auto &dec : deviceDevices) {
            deviceDescs.push_back(make_unique<AudioDeviceDescriptor>(*dec));
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
    return audioPolicyService_.OffloadStopPlaying(std::vector<int32_t>(1, audioInterrupt.sessionId));
}

int32_t AudioPolicyServer::ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type)
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

void AudioPolicyServer::OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> descriptor,
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
    bool ret = powerMgrClient.RegisterSyncSleepCallback(powerStateListener_, SleepPriority::HIGH);
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
    bool ret = powerMgrClient.UnRegisterSyncSleepCallback(powerStateListener_);
    if (!ret) {
        AUDIO_WARNING_LOG("unregister sync sleep callback failed");
    } else {
        powerStateListener_ = nullptr;
        AUDIO_INFO_LOG("unregister sync sleep callback success");
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
    bool ret = powerMgrClient.RegisterSyncHibernateCallback(syncHibernateListener_);
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
    bool ret = powerMgrClient.UnRegisterSyncHibernateCallback(syncHibernateListener_);
    if (!ret) {
        AUDIO_WARNING_LOG("unregister sync hibernate callback failed");
    } else {
        delete syncHibernateListener_;
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

int32_t AudioPolicyServer::SetSpatializationEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
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

int32_t AudioPolicyServer::SetHeadTrackingEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
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
    if (interruptService_ != nullptr) {
        return interruptService_->CreateAudioInterruptZone(zoneID, pids);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::AddAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->AddAudioInterruptZonePids(zoneID, pids);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::RemoveAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->RemoveAudioInterruptZonePids(zoneID, pids);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::ReleaseAudioInterruptZone(const int32_t zoneID)
{
    if (interruptService_ != nullptr) {
        return interruptService_->ReleaseAudioInterruptZone(zoneID);
    }
    return ERR_UNKNOWN;
}

int32_t AudioPolicyServer::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return ERR_SYSTEM_PERMISSION_DENIED;
    }
    switch (deviceType) {
        case EARPIECE:
        case SPEAKER:
        case BLUETOOTH_SCO:
            break;
        default:
            AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
            return ERR_NOT_SUPPORTED;
    }
    return audioPolicyService_.SetCallDeviceActive(deviceType, active, address);
}

std::unique_ptr<AudioDeviceDescriptor> AudioPolicyServer::GetActiveBluetoothDevice()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("No system permission");
        return make_unique<AudioDeviceDescriptor>();
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
    AppExecFwk::BundleInfo bundleInfo = GetBundleInfoFromUid();
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
        return SPATIALIZATION_SCENE_TYPE_DEFAULT;
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

AppExecFwk::BundleInfo AudioPolicyServer::GetBundleInfoFromUid()
{
    AudioXCollie audioXCollie("AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName",
        GET_BUNDLE_TIME_OUT_SECONDS);
    std::string bundleName {""};
    AppExecFwk::BundleInfo bundleInfo;
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityManager != nullptr, bundleInfo, "systemAbilityManager is nullptr");

    sptr<IRemoteObject> remoteObject = systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, bundleInfo, "remoteObject is nullptr");

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(bundleMgrProxy != nullptr, bundleInfo, "bundleMgrProxy is nullptr");

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    bundleMgrProxy->GetNameForUid(callingUid, bundleName);

    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);

    return bundleInfo;
}

int32_t AudioPolicyServer::GetApiTargerVersion()
{
    AppExecFwk::BundleInfo bundleInfo = GetBundleInfoFromUid();

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
    return audioPolicyService_.GetMaxAmplitude(deviceId);
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

void AudioPolicyServer::NotifyAccountsChanged(const int &id)
{
    audioPolicyService_.NotifyAccountsChanged(id);
    CHECK_AND_RETURN_LOG(interruptService_ != nullptr, "interruptService_ is nullptr");
    interruptService_->ClearAudioFocusInfoListOnAccountsChanged(id);
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
    return audioPolicyService_.ActivateAudioConcurrency(pipeType);
}
void AudioPolicyServer::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    audioPolicyService_.OnReceiveBluetoothEvent(macAddress, deviceName);
}

void BluetoothEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    std::string deviceName  = want.GetStringParam("remoteName");
    std::string macAddress = want.GetStringParam("deviceAddr");
    AUDIO_INFO_LOG("receive action %{public}s", deviceName.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE) {
        audioPolicyServer_->OnReceiveBluetoothEvent(macAddress, deviceName);
    }
}

void AudioPolicyServer::RegisterCommonEventReceiver()
{
    if (bluetoothEventSubscriberOb_ != nullptr) {
        AUDIO_WARNING_LOG("bluetoothEvent is already registed");
        return;
    }
    AUDIO_INFO_LOG("register bluetooth remote device name receiver");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bluetoothEventSubscriberOb_ =
        std::make_shared<BluetoothEventSubscriber>(subscribeInfo, this);
    EventFwk::CommonEventManager::NewSubscribeCommonEvent(bluetoothEventSubscriberOb_);
}

void AudioPolicyServer::UnregisterCommonEventReceiver()
{
    if (bluetoothEventSubscriberOb_ != nullptr) {
        EventFwk::CommonEventManager::NewUnSubscribeCommonEvent(bluetoothEventSubscriberOb_);
        AUDIO_INFO_LOG("unregister bluetooth device name commonevent");
    }
}

int32_t AudioPolicyServer::InjectInterruption(const std::string networkId, InterruptEvent &event)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_CAST_ENGINE_SA) {
        AUDIO_ERR_LOG("InjectInterruption callerUid is Error: not cast_engine");
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is nullptr");
    InterruptEventInternal interruptEvent { event.eventType, event.forceType, event.hintType, 0.2f};
    return audioPolicyServerHandler_->SendInterruptEventInternalCallback(interruptEvent);
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

int32_t AudioPolicyServer::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    return audioPolicyService_.SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}
} // namespace AudioStandard
} // namespace OHOS
