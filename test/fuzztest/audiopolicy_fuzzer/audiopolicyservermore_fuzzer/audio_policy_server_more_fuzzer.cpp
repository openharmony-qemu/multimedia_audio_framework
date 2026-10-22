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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "audio_policy_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "system_ability_definition.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"
#include "bluetooth_host.h"
#include "ipc_skeleton.h"
#include "securec.h"

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_hasServerInit = false;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const int32_t MAXVALUE = 4;
typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<>
std::string GetData<std::string>()
{
    std::string object {};
    if (RAW_DATA == nullptr || sizeof(size_t) > g_dataSize - g_pos) {
        return object;
    }
    size_t strSize = 0;
    errno_t ret = memcpy_s(&strSize, sizeof(size_t), RAW_DATA + g_pos, sizeof(size_t));
    if (ret != EOK) {
        return {};
    }
    g_pos += sizeof(size_t);
    if (g_pos > g_dataSize || strSize > g_dataSize - g_pos) {
        return object;
    }
    if (strSize > 0) {
        if (strSize > SIZE_MAX - 1) {
            return object;
        }
        std::vector<char> buffer(strSize + 1, '\0');
        ret = memcpy_s(buffer.data(), strSize + 1, RAW_DATA + g_pos, strSize + 1);
        if (ret != EOK) {
            return object;
        }
        object = std::string(buffer.data());
    }
    g_pos += strSize + 1;
    return object;
}

sptr<AudioPolicyServer> GetServerPtr()
{
    static sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit && server != nullptr) {
        IdHandler::GetInstance();
        HdiAdapterManager::GetInstance();
        HPAE::IHpaeManager::GetHpaeManager().Init();
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
}

// Lines 247-265: TranslateErrorCode
void FuzzTranslateErrorCode()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t result = GetData<int32_t>();
    server->TranslateErrorCode(result);
    server->TranslateErrorCode(ERR_INVALID_PARAM);
    server->TranslateErrorCode(ERR_NULL_POINTER);
    server->TranslateErrorCode(ERR_MMI_CREATION);
    server->TranslateErrorCode(ERR_MMI_SUBSCRIBE);
    server->TranslateErrorCode(SUCCESS);
}

// Lines 267-271: OnDump
void FuzzOnDump()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->OnDump();
}

// Lines 310-320: RegisterPermStateChangeCallback
void FuzzRegisterPermStateChangeCallback()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterPermStateChangeCallback();
}

// Lines 322-326: GetScreenStates
void FuzzGetScreenStates()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->GetScreenStates();
}

// Lines 396-407: OnStop (with re-init)
void FuzzOnStop()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->OnStop();
    server->OnStart();
    server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
}

// Lines 419-451: OnAddSystemAbility switch cases
void FuzzOnAddSystemAbility()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    const std::vector<int32_t> saIds = {
        DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID,
        DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
        POWER_MANAGER_SERVICE_ID,
        SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN,
        COMMON_EVENT_SERVICE_ID,
        BACKGROUND_TASK_MANAGER_SERVICE_ID,
        APP_MGR_SERVICE_ID,
#ifdef USB_ENABLE
        USB_SYSTEM_ABILITY_ID,
#endif
        INPUT_METHOD_SYSTEM_ABILITY_ID,
        ACCESS_TOKEN_MANAGER_SERVICE_ID,
        0,
        GetData<int32_t>(),
    };
    for (int32_t saId : saIds) {
        server->OnAddSystemAbility(saId, "");
    }
}

// Lines 463-465, 483-491: RegisterDefaultVolumeTypeListener + HandleAudioServerRelatedSystemAbility
void FuzzHandleAudioServerRelatedSystemAbility()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string deviceId = "";
    server->HandleAudioServerRelatedSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, deviceId);
    server->HandleAudioServerRelatedSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, deviceId);
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    server->HandleAudioServerRelatedSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, deviceId);
#endif
    server->HandleAudioServerRelatedSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, deviceId);
    server->HandleAudioServerRelatedSystemAbility(0, deviceId);
    server->HandleAudioServerRelatedSystemAbility(GetData<int32_t>(), deviceId);
    server->RegisterDefaultVolumeTypeListener();
}

// Lines 552-567: ReloadLoudVolumeMode
void FuzzReloadLoudVolumeMode()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t streamInFocus = GetData<int32_t>();
    int32_t setVolMode = GetData<int32_t>();
    bool ret = false;
    server->ReloadLoudVolumeMode(streamInFocus, setVolMode, ret);
}

// Lines 572-576: ChangeVolumeOnVoiceAssistant
void FuzzChangeVolumeOnVoiceAssistant()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamType streamInFocus = GetData<AudioStreamType>();
    server->ChangeVolumeOnVoiceAssistant(streamInFocus);
    streamInFocus = AudioStreamType::STREAM_VOICE_ASSISTANT;
    server->ChangeVolumeOnVoiceAssistant(streamInFocus);
}

// Lines 583-585, 601-609, 619-621: RegisterVolumeKeyEvents
#ifdef FEATURE_MULTIMODALINPUT_INPUT
void FuzzRegisterVolumeKeyEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP);
    server->RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN);
    server->RegisterVolumeKeyEvents(GetData<int32_t>());
}
#endif

// Lines 624-643: IsContinueAddVol
void FuzzIsContinueAddVol()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    for (int32_t i = 0; i < MAXVALUE; i++) {
        server->IsContinueAddVol();
    }
}

// Lines 656-691: TriggerMuteCheck
void FuzzTriggerMuteCheck()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->TriggerMuteCheck();
}

// Lines 701-713: GetActiveAudioInterruptZone
void FuzzGetActiveAudioInterruptZone()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t zoneId = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    server->GetActiveAudioInterruptZone(zoneId, streamType);
    zoneId = GetData<int32_t>();
    server->GetActiveAudioInterruptZone(zoneId, streamType);
}

// Lines 718-719, 723-734, 747-762: ProcessVolumeKeyEvents
#ifdef FEATURE_MULTIMODALINPUT_INPUT
void FuzzProcessVolumeKeyEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ProcessVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP);
    server->ProcessVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN);
    server->ProcessVolumeKeyEvents(GetData<int32_t>());
}
#endif

// Lines 768: IsScreenOnAdjustVolume
void FuzzIsScreenOnAdjustVolume()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->IsScreenOnAdjustVolume();
}

// Lines 779-787, 800-805: SetVolumeInternalByKeyEvent
void FuzzSetVolumeInternalByKeyEvent()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamType streamInFocus = GetData<AudioStreamType>();
    int32_t zoneId = GetData<int32_t>();
    server->SetVolumeInternalByKeyEvent(streamInFocus, zoneId, 0);
    server->SetVolumeInternalByKeyEvent(streamInFocus, zoneId, 1);
}

// Lines 828-839, 849-850: RegisterVolumeKeyMuteEvents
#ifdef FEATURE_MULTIMODALINPUT_INPUT
void FuzzRegisterVolumeKeyMuteEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterVolumeKeyMuteEvents();
}
#endif

// Lines 861-863, 869-871, 875-877, 881-883, 888-891: SubscribeVolumeKeyEvents
#ifdef FEATURE_MULTIMODALINPUT_INPUT
void FuzzSubscribeVolumeKeyEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SubscribeVolumeKeyEvents();
}
#endif

// Lines 894-933: SubscribeVolumeKeyEventsRetryFunc
#ifdef FEATURE_MULTIMODALINPUT_INPUT
void FuzzSubscribeVolumeKeyEventsRetryFunc()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SubscribeVolumeKeyEventsRetryFunc();
}
#endif

// Lines 974: SubscribeOsAccountSubProfileChangeEvents (call twice for "already" log)
void FuzzSubscribeOsAccountSubProfileChangeEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SubscribeOsAccountSubProfileChangeEvents();
    server->SubscribeOsAccountSubProfileChangeEvents();
}

// Lines 990: AddAudioServiceOnStart (call twice for "not first start")
void FuzzAddAudioServiceOnStart()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->AddAudioServiceOnStart();
}

// Lines 1006-1008, 1015-1016: SubscribePowerStateChangeEvents
void FuzzSubscribePowerStateChangeEvents()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SubscribePowerStateChangeEvents();
}

// Lines 1021-1029: AudioCommonEventSubscriber::OnReceiveEvent
void FuzzAudioCommonEventSubscriberOnReceiveEvent()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    EventFwk::CommonEventData eventData;
    server->OnReceiveEvent(eventData);
}

// Lines 1037-1187: SubscribeCommonEventExecute + OnReceiveEvent variations
void FuzzOnReceiveEventVariations()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    EventFwk::CommonEventData eventData;
    AAFwk::Want want;
    want.SetAction("usual.event.DATA_SHARE_READY");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.SCREEN_ON");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.SCREEN_OFF");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.SCREEN_LOCKED");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.SCREEN_UNLOCKED");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.LOCALE_CHANGED");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.dms.cast_plugged_changed");
    eventData.SetWant(want);
    eventData.SetData("1");
    server->OnReceiveEvent(eventData);
    eventData.SetData("0");
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.bluetooth.remotedevice.NAME_UPDATE");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    want.SetAction("usual.event.dms.rotation_changed");
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
    std::string actionString = GetData<std::string>();
    want.SetAction(actionString.c_str());
    eventData.SetWant(want);
    server->OnReceiveEvent(eventData);
}

// Lines 1198-1200: ConnectServiceAdapter
void FuzzConnectServiceAdapter()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ConnectServiceAdapter();
}

// Lines 1228-1232, 1239-1243: SetSystemVolumeLevelLegacy
void FuzzSetSystemVolumeLevelLegacy()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t streamTypeIn = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    server->SetSystemVolumeLevelLegacy(streamTypeIn, volumeLevel);
}

// Lines 1259-1329: SetVoipRendererMute, SetVoipCapturerMute, RegisterVoipMuteDeathRecipient, OnVoipMuteCallerDied
void FuzzSetVoipRendererMute()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t appUid = GetData<int32_t>();
    int64_t streamId = GetData<int64_t>();
    bool muted = GetData<int32_t>() % MAXVALUE;
    server->SetVoipRendererMute(appUid, streamId, muted, nullptr);
    server->SetVoipCapturerMute(appUid, streamId, muted, nullptr);
    server->OnVoipMuteCallerDied(GetData<int32_t>());
}

// Lines 1467-1470, 1483: GetSystemVolumeLevel, SetLowPowerVolume
void FuzzGetSystemVolumeLevel()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t streamType = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    int32_t volumeLevel = 0;
    server->GetSystemVolumeLevel(streamType, uid, volumeLevel);
    int32_t streamId = GetData<int32_t>();
    float volume = GetData<float>();
    server->SetLowPowerVolume(streamId, volume);
}

// Lines 1618-1620: GetSystemVolumeInDb
void FuzzGetSystemVolumeInDb()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t volumeTypeIn = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t deviceTypeIn = GetData<int32_t>();
    float volume = 0.0f;
    server->GetSystemVolumeInDb(volumeTypeIn, volumeLevel, deviceTypeIn, volume);
}

// Lines 1695-1754: SetStreamMuteInternal, SetSingleStreamMute
void FuzzSetStreamMuteInternal()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamType streamType = GetData<AudioStreamType>();
    bool mute = GetData<int32_t>() % MAXVALUE;
    bool isUpdateUi = GetData<int32_t>() % MAXVALUE;
    DeviceType deviceType = GetData<DeviceType>();
    int32_t zoneId = GetData<int32_t>();
    server->SetStreamMuteInternal(streamType, mute, isUpdateUi, deviceType, zoneId);
    server->SetStreamMuteInternal(STREAM_ALL, mute, isUpdateUi, deviceType, zoneId);
    server->SetStreamMuteInternal(STREAM_NOTIFICATION, !mute, isUpdateUi, deviceType, zoneId);
}

// Lines 1798-1799: SetSystemVolumeLevelWithDeviceInternal
void FuzzSetSystemVolumeLevelWithDeviceInternal()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamType streamType = GetData<AudioStreamType>();
    int32_t volumeLevel = GetData<int32_t>();
    bool isUpdateUi = GetData<int32_t>() % MAXVALUE;
    DeviceType deviceType = GetData<DeviceType>();
    server->SetSystemVolumeLevelWithDeviceInternal(streamType, volumeLevel, isUpdateUi, deviceType);
}

// Lines 1818-1856: IsArmUsbDevice, MapExternalToInternalDeviceType
void FuzzIsArmUsbDeviceAndMapExternal()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    server->IsArmUsbDevice(desc);
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->IsArmUsbDevice(desc);
    desc.deviceType_ = DEVICE_TYPE_SPEAKER;
    server->IsArmUsbDevice(desc);
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_USB_DEVICE;
    server->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.deviceRole_ = INPUT_DEVICE;
    server->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    desc.deviceRole_ = INPUT_DEVICE;
    server->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.highQualityRecordingSupported_ = true;
    server->MapExternalToInternalDeviceType(desc);
    desc.highQualityRecordingSupported_ = false;
    desc.isUnidirectional_ = true;
    server->MapExternalToInternalDeviceType(desc);
}

// Lines 2077-2095: GetDevicesInner, IsEnhancedRoutingSupported
void FuzzGetDevicesInner()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    server->GetDevicesInner(GetData<int32_t>(), deviceDescs);
    bool enhancedRet = false;
    server->IsEnhancedRoutingSupported(enhancedRet);
}

// Lines 2164-2221: GetPreferredOutputDeviceDescriptors, GetPreferredInputDeviceDescriptors
void FuzzGetPreferredDeviceDescriptors()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = GetData<ContentType>();
    rendererInfo.streamUsage = GetData<StreamUsage>();
    rendererInfo.rendererFlags = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    server->GetPreferredOutputDeviceDescriptors(rendererInfo, GetData<int32_t>() % MAXVALUE, deviceDescs);
    AudioCapturerInfo capturerInfo = {};
    capturerInfo.sourceType = GetData<SourceType>();
    capturerInfo.capturerFlags = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDescs;
    server->GetPreferredInputDeviceDescriptors(capturerInfo, inputDescs);
}

// Lines 2246-2272: SetDeclaredDeviceTypes, HandleRouteSelectorCallbackEnable
void FuzzSetDeclaredDeviceTypes()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::set<int32_t> deviceTypes = {GetData<int32_t>(), GetData<int32_t>()};
    server->SetDeclaredDeviceTypes(deviceTypes);
    server->HandleRouteSelectorCallbackEnable(GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 2274-2336: SetClientCallbacksEnable, SetCallbackRendererInfo, SetCallbackCapturerInfo, etc.
void FuzzSetClientCallbacks()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetClientCallbacksEnable(GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    AudioRendererInfo rendererInfo = {};
    server->SetCallbackRendererInfo(rendererInfo, GetData<int32_t>());
    AudioCapturerInfo capturerInfo = {};
    server->SetCallbackCapturerInfo(capturerInfo, GetData<int32_t>());
    SystemVolumeFilter filter = {};
    server->SetSystemVolumeChangeCallbackInfo(filter);
    server->RemoveSystemVolumeChangeCallbackInfo(filter);
    server->RemoveAllSystemVolumeChangeCallbackInfo();
}

// Lines 2345-2364: IsStreamActiveByStreamUsage, IsFastPlaybackSupported
void FuzzIsStreamActiveByStreamUsage()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool active = false;
    server->IsStreamActive(GetData<int32_t>(), active);
    server->IsStreamActiveByStreamUsage(GetData<int32_t>(), active);
    AudioStreamInfo streamInfo = {};
    bool support = false;
    server->IsFastPlaybackSupported(streamInfo, GetData<int32_t>(), support);
    server->IsFastRecordingSupported(streamInfo, GetData<int32_t>(), support);
}

// Lines 2377-2417: IsMultichannelPlaybackSupported, IsDirectPlaybackSupported, etc.
void FuzzIsPlaybackSupported()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamInfo streamInfo = {};
    bool support = false;
    server->IsMultichannelPlaybackSupported(streamInfo, GetData<int32_t>(), support);
    server->IsDirectPlaybackSupported(streamInfo, GetData<int32_t>(), support);
    server->IsOffloadPlaybackSupported(streamInfo, GetData<int32_t>(), support);
    server->SetDeviceActive(GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1, GetData<int32_t>());
}

// GetHwEncodingTypeMaxBitrate
void FuzzGetHwEncodingTypeMaxBitrate()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t maxBitrate = 0;
    server->GetHwEncodingTypeMaxBitrate(maxBitrate);
}

// Lines 2432-2474: IsDeviceActive, GetActiveOutputDevice, GetDmDeviceType, GetActiveInputDevice
void FuzzIsDeviceActive()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool active = false;
    server->IsDeviceActive(GetData<int32_t>(), active);
    int32_t deviceType = 0;
    server->GetActiveOutputDevice(deviceType);
    uint16_t dmDeviceType = 0;
    server->GetDmDeviceType(dmDeviceType);
    server->GetActiveInputDevice(deviceType);
    server->SetRingerModeLegacy(GetData<int32_t>());
    server->SetRingerMode(GetData<int32_t>());
}

// Lines 2516-2517, 2534-2549: SetRingerModeInner, GetToneConfig
void FuzzSetRingerModeInner()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetRingerModeInternal(static_cast<AudioRingerMode>(GetData<int32_t>()));
#ifdef FEATURE_DTMF_TONE
    std::shared_ptr<ToneInfo> config;
    server->GetToneConfig(GetData<int32_t>(), GetData<std::string>(), config);
    std::vector<int32_t> tones;
    server->GetSupportedTones(GetData<std::string>(), tones);
#endif
}

// Lines 2564-2569, 2591-2598: InitMicrophoneMute, SetMicrophoneMute
void FuzzInitMicrophoneMute()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->InitMicrophoneMute();
    server->SetMicrophoneMute(GetData<int32_t>() % MAXVALUE == 1);
    server->SetMicrophoneMuteAudioConfig(GetData<int32_t>() % MAXVALUE == 1);
    server->SetMicrophoneMutePersistent(GetData<int32_t>() % MAXVALUE == 1, GetData<int32_t>());
    bool mute = false;
    server->GetPersistentMicMuteState(mute);
    server->IsMicrophoneMuteLegacy(mute);
    server->IsMicrophoneMute(mute);
}

// Lines 2623-2662, 2669-2688: SetMicrophoneMutePersistent, GetRingerMode
void FuzzMicAndRingerMode()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t ringerMode = 0;
    server->GetRingerMode(ringerMode);
    server->SetMicrophoneMuteCommon(GetData<int32_t>() % MAXVALUE == 1, GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 2735-2761, 2771-2812: SetAudioInterruptCallback, VerifySessionId, SetAudioManagerInterruptCallback
void FuzzAudioInterruptCallbacks()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t clientUid = GetData<uint32_t>();
    int32_t zoneId = GetData<int32_t>();
    server->SetAudioInterruptCallback(sessionId, nullptr, clientUid, zoneId);
    server->UnsetAudioInterruptCallback(sessionId, zoneId);
    server->SetAudioManagerInterruptCallback(GetData<int32_t>(), nullptr);
    server->UnsetAudioManagerInterruptCallback(GetData<int32_t>());
    server->VerifySessionId(sessionId, clientUid);
    server->SetAudioRouteCallback(sessionId, nullptr, clientUid);
    server->UnsetAudioRouteCallback(sessionId);
}

// Lines 2888-2928: SetAudioVKBInfoMgrCallback, CheckVKBInfo, RequestAudioFocus, AbandonAudioFocus
void FuzzAudioFocusAndVKB()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetAudioVKBInfoMgrCallback(nullptr);
    bool isValid = false;
    server->CheckVKBInfo(GetData<std::string>(), isValid);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = GetData<uint32_t>();
    audioInterrupt.uid = GetData<int32_t>();
    audioInterrupt.pid = GetData<int32_t>();
    audioInterrupt.audioFocusType.streamType = GetData<AudioStreamType>();
    audioInterrupt.audioFocusType.sourceType = GetData<SourceType>();
    server->RequestAudioFocus(GetData<int32_t>(), audioInterrupt);
    server->AbandonAudioFocus(GetData<int32_t>(), audioInterrupt);
}

// Lines 2953-2960, 2993-3017, 3030-3031: VerifyUidAndPid, ActivateAudioInterrupt
void FuzzActivateAudioInterrupt()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = GetData<uint32_t>();
    audioInterrupt.uid = GetData<int32_t>();
    audioInterrupt.pid = GetData<int32_t>();
    server->VerifyUidAndPid(GetData<int32_t>(), GetData<int32_t>());
    server->ActivateAudioInterrupt(audioInterrupt, GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 3034-3058: SetAppConcurrencyMode, SetOffloadAllowedForUid, SetAppSilentOnDisplay
void FuzzSetAppConcurrency()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetAppConcurrencyMode(GetData<int32_t>(), GetData<int32_t>());
    server->SetOffloadAllowedForUid(GetData<uint32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    server->SetAppSilentOnDisplay(GetData<int32_t>());
}

// Lines 3076-3090: DeactivateAudioInterrupt, UpdateAudioSceneAfterDeactivateInterrupt
void FuzzDeactivateAudioInterrupt()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = GetData<uint32_t>();
    audioInterrupt.uid = GetData<int32_t>();
    audioInterrupt.pid = GetData<int32_t>();
    audioInterrupt.audioFocusType.sourceType = GetData<SourceType>();
    server->DeactivateAudioInterrupt(audioInterrupt, GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 3100-3116, 3130-3131, 3167, 3179: ActivatePreemptMode, GetCurrentStreamInFocus, etc.
void FuzzPreemptAndFocus()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ActivatePreemptMode();
    server->GetCurrentStreamInFocus(GetData<int32_t>());
    int32_t streamType = 0;
    server->GetStreamInFocus(GetData<int32_t>(), streamType);
    server->GetStreamInFocusByUid(GetData<int32_t>(), GetData<int32_t>(), streamType);
    AudioInterrupt audioInterrupt = {};
    server->GetSessionInfoInFocus(audioInterrupt, GetData<int32_t>());
    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    server->GetAudioFocusInfoList(focusInfoList, GetData<int32_t>());
}

// Lines 3430-3436, 3482-3483: RegisterTracker, UpdateTracker
void FuzzRegisterTracker()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.clientUID = GetData<int32_t>();
    streamChangeInfo.audioCapturerChangeInfo.clientUID = GetData<int32_t>();
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = GetData<SourceType>();
    streamChangeInfo.audioCapturerChangeInfo.sessionId = GetData<uint32_t>();
    server->RegisterTracker(AUDIO_MODE_PLAYBACK, streamChangeInfo, nullptr);
    server->RegisterTracker(AUDIO_MODE_RECORD, streamChangeInfo, nullptr);
    server->UpdateTracker(AUDIO_MODE_PLAYBACK, streamChangeInfo);
    server->UpdateTracker(AUDIO_MODE_RECORD, streamChangeInfo);
}

// Lines 3521-3526: GetCurrentRendererChangeInfosForZone
void FuzzGetCurrentRendererChangeInfosForZone()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    server->GetCurrentRendererChangeInfos(rendererChangeInfos);
    server->GetCurrentRendererChangeInfosForZone(GetData<int32_t>(), rendererChangeInfos);
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    server->GetCurrentCapturerChangeInfos(capturerChangeInfos);
}

// Lines 3574-3583, 3602: RegisterClientDeathRecipient, RegisteredTrackerClientDied
void FuzzRegisterClientDeathRecipient()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterClientDeathRecipient(nullptr, AudioPolicyServer::TRACKER_CLIENT);
    server->RegisterClientDeathRecipient(nullptr, AudioPolicyServer::LISTENER_CLIENT);
    server->RegisteredTrackerClientDied(GetData<int32_t>(), GetData<int32_t>());
    server->RegisteredStreamListenerClientDied(GetData<int32_t>(), GetData<int32_t>());
}

// Lines 3713-3720: GetNetworkIdByGroupId
void FuzzGetNetworkIdByGroupId()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string networkId;
    server->GetNetworkIdByGroupId(GetData<int32_t>(), networkId);
    std::vector<sptr<VolumeGroupInfo>> infos;
    server->GetVolumeGroupInfos(GetData<std::string>(), infos);
}

// Lines 3735-3842: RemoteParameterCallback
void FuzzRemoteParameterCallback()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    callback->OnAudioParameterChange(GetData<std::string>(), AudioParamKey::VOLUME,
        GetData<std::string>(), GetData<std::string>());
    callback->OnAudioParameterChange(GetData<std::string>(), AudioParamKey::INTERRUPT,
        GetData<std::string>(), GetData<std::string>());
    callback->OnAudioParameterChange(GetData<std::string>(), AudioParamKey::PARAM_KEY_STATE,
        GetData<std::string>(), GetData<std::string>());
    callback->OnHdiRouteStateChange(GetData<std::string>(), GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 3845-3890: PerStateChangeCbCustomizeCallback
void FuzzPerStateChangeCallback()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    Security::AccessToken::PermStateChangeScope scopeInfo;
    auto callback = std::make_shared<AudioPolicyServer::PerStateChangeCbCustomizeCallback>(scopeInfo, server);
    Security::AccessToken::PermStateChangeInfo result;
    result.permStateChangeType = GetData<int32_t>() % MAXVALUE;
    result.tokenID = GetData<uint32_t>();
    result.permissionName = "ohos.permission.MICROPHONE";
    callback->PermStateChangeCallback(result);
    callback->UpdateMicPrivacyByCapturerState(GetData<int32_t>() % MAXVALUE == 1, GetData<uint32_t>(),
        GetData<int32_t>());
}

// Lines 4008-4012, 4046-4064, 4072-4073, 4084: Misc functions
void FuzzMiscServerFunctions()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool ret = false;
    server->IsSupportInnerCaptureOffload(ret);
    server->RegisterDataObserver();
    SupportedEffectConfig supportedEffectConfig;
    server->QueryEffectSceneMode(supportedEffectConfig);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t sampleRate = 0;
    server->GetHardwareOutputSamplingRate(desc, sampleRate);
    std::vector<sptr<MicrophoneDescriptor>> micDescs;
    server->GetAudioCapturerMicrophoneDescriptors(GetData<int32_t>(), micDescs);
    server->GetAvailableMicrophones(micDescs);
    auto audioDebugInfo = std::make_shared<AudioDebugInfo>();
    server->GetAudioDebugInfo(GetData<uint32_t>(), audioDebugInfo);
    auto sessionDebugInfo = std::make_shared<AudioSessionDebugInfo>();
    server->GetSessionDebugInfo(sessionDebugInfo);
    server->SetDeviceAbsVolumeSupported(GetData<std::string>(),
        GetData<int32_t>() % MAXVALUE == 1, GetData<int32_t>());
    server->SetSleVoiceStatusFlag(GetData<int32_t>() % MAXVALUE == 1);
    std::shared_ptr<AudioDeviceDescriptor> selectedDevice;
    server->GetSelectedInputDevice(selectedDevice);
    server->ClearSelectedInputDevice();
    server->PreferBluetoothAndNearlinkRecord(GetData<uint32_t>());
    server->ResumeStreamState();
    server->UpdateStreamState(GetData<int32_t>(), GetData<int32_t>(), GetData<int32_t>());
}

// Lines 4104-4163: SetA2dpDeviceVolume, SetNearlinkDeviceVolume
void FuzzSetA2dpAndNearlinkVolume()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string macAddress = GetData<std::string>();
    int32_t volume = GetData<int32_t>();
    bool updateUi = GetData<int32_t>() % MAXVALUE == 1;
    server->SetA2dpDeviceVolume(macAddress, volume, updateUi);
    int32_t streamTypeIn = GetData<int32_t>();
    server->SetNearlinkDeviceVolume(macAddress, streamTypeIn, volume, updateUi);
}

// Lines 4166-4211: GetSelectedInputDevice, ClearSelectedInputDevice, PreferBluetoothAndNearlinkRecord, etc.
void FuzzDeviceSelection()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    uint32_t category = GetData<uint32_t>();
    server->PreferBluetoothAndNearlinkRecord(category);
    uint32_t retCategory = 0;
    server->GetPreferBluetoothAndNearlinkRecord(retCategory);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    server->GetAvailableDevices(GetData<int32_t>(), descs);
}

// Lines 4218-4267: GetAvailableDevices switch + body
void FuzzGetAvailableDevices()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    server->GetAvailableDevices(MEDIA_OUTPUT_DEVICES, descs);
    server->GetAvailableDevices(ALL_MEDIA_DEVICES, descs);
    server->GetAvailableDevices(CALL_OUTPUT_DEVICES, descs);
    server->GetAvailableDevices(ALL_CALL_DEVICES, descs);
    server->GetAvailableDevices(D_ALL_DEVICES, descs);
    server->GetAvailableDevices(GetData<int32_t>(), descs);
}

// Lines 4276-4292: SetAvailableDeviceChangeCallback
void FuzzSetAvailableDeviceChangeCallback()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetAvailableDeviceChangeCallback(GetData<int32_t>(), MEDIA_OUTPUT_DEVICES, nullptr);
    server->SetAvailableDeviceChangeCallback(GetData<int32_t>(), GetData<int32_t>(), nullptr);
    server->UnsetAvailableDeviceChangeCallback(GetData<int32_t>(), GetData<int32_t>());
}

// Lines 4322-4343: SetDistributedRoutingRoleCallback, UnsetDistributedRoutingRoleCallback
void FuzzDistributedRoutingRole()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetDistributedRoutingRoleCallback(nullptr);
    server->UnsetDistributedRoutingRoleCallback();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    server->ConfigDistributedRoutingRole(desc, GetData<int32_t>());
}

// Lines 4359-4387, 4436-4464: RegisterPowerStateListener, UnRegisterPowerStateListener, SyncHibernate
void FuzzPowerAndHibernateListeners()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterPowerStateListener();
    server->UnRegisterPowerStateListener();
    server->RegisterSyncHibernateListener();
    server->UnRegisterSyncHibernateListener();
}

// Lines 4401-4427: RegisterAppStateListener, RegisterAppStateObserver
void FuzzAppStateListeners()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterAppStateListener();
    server->RegisterAppStateObserver();
}

// Lines 4717-4776: RegisterPolicyCallbackClient, RegisterAudioZoneClient
void FuzzRegisterPolicyCallbackClient()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterPolicyCallbackClient(nullptr, GetData<int32_t>());
    server->RegisterAudioZoneClient(nullptr);
}

// Lines 4795-4801, 4889-4972: AudioZone operations
void FuzzAudioZoneOperations()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t zoneId = GetData<int32_t>();
    AudioZoneContext context = {};
    int32_t retZoneId = 0;
    server->CreateAudioZone(GetData<std::string>(), context, retZoneId, GetData<int32_t>());
    server->ReleaseAudioZone(GetData<int32_t>());
    server->UpdateContextForAudioZone(zoneId, context);
    std::vector<int32_t> userIds = {GetData<int32_t>()};
    std::vector<int32_t> retUserIds;
    server->GetAudioZoneForApp(userIds, retUserIds);
    std::vector<std::shared_ptr<AudioZoneDescriptor>> zoneDescs;
    server->GetAllAudioZone(zoneDescs);
    std::shared_ptr<AudioZoneDescriptor> zoneDesc;
    server->GetAudioZone(GetData<int32_t>(), zoneDesc);
    int32_t zoneIdByName = 0;
    server->GetAudioZoneByName(GetData<std::string>(), zoneIdByName);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices = {std::make_shared<AudioDeviceDescriptor>()};
    server->BindDeviceToAudioZone(GetData<int32_t>(), devices);
    server->UnBindDeviceToAudioZone(GetData<int32_t>(), devices);
    server->EnableAudioZoneReport(GetData<int32_t>() % MAXVALUE == 1);
    server->EnableAudioZoneChangeReport(GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    server->AddUidToAudioZone(GetData<int32_t>(), GetData<int32_t>());
    server->RemoveUidFromAudioZone(GetData<int32_t>(), GetData<int32_t>());
    std::set<int32_t> usages = {STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC};
    server->AddUidUsagesToAudioZone(GetData<int32_t>(), GetData<int32_t>(), usages);
    server->RemoveUidUsagesFromAudioZone(GetData<int32_t>(), GetData<int32_t>(), usages);
    AudioZoneStream stream = {};
    server->AddStreamToAudioZone(GetData<int32_t>(), stream);
    std::vector<AudioZoneStream> streams = {stream};
    server->AddStreamsToAudioZone(GetData<int32_t>(), streams);
    server->RemoveStreamFromAudioZone(GetData<int32_t>(), stream);
    server->RemoveStreamsFromAudioZone(GetData<int32_t>(), streams);
    server->AddStreamIdToAudioZone(GetData<int32_t>(), GetData<uint32_t>());
    server->RemoveStreamIdFromAudioZone(GetData<int32_t>(), GetData<uint32_t>());
    std::vector<int32_t> zoneIds = {GetData<int32_t>()};
    server->PauseAudioFocusForExtAudioZone(zoneIds);
    server->ResumeAudioFocusForExtAudioZone(zoneIds);
    server->SetZoneDeviceVisible(GetData<int32_t>() % MAXVALUE == 1);
    server->EnableSystemVolumeProxy(GetData<int32_t>(), GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    std::vector<std::map<AudioInterrupt, int32_t>> interruptList;
    server->GetAudioInterruptForZone(GetData<int32_t>(), interruptList);
    server->GetAudioInterruptForZone(GetData<int32_t>(), GetData<std::string>(), interruptList);
    server->EnableAudioZoneInterruptReport(GetData<int32_t>(), GetData<std::string>(),
        GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 5188-5226: SetAudioDeviceRefinerCallback, TriggerFetchDevice, SetPreferredDevice
void FuzzDeviceRefinerAndPreferred()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetAudioDeviceRefinerCallback(nullptr);
    server->UnsetAudioDeviceRefinerCallback();
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
    server->TriggerFetchDevice(reason);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    server->SetPreferredDevice(GetData<int32_t>(), desc, GetData<int32_t>());
}

// Lines 5241-5296: SetDeviceVolumeBehavior, SetAudioDeviceAnahsCallback
void FuzzDeviceVolumeBehavior()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    VolumeBehavior volumeBehavior = {};
    volumeBehavior.isReady = GetData<int32_t>() % MAXVALUE == 1;
    volumeBehavior.isVolumeControlDisabled = GetData<int32_t>() % MAXVALUE == 1;
    server->SetDeviceVolumeBehavior(GetData<std::string>(), GetData<int32_t>(), volumeBehavior);
    server->SetStopStreamInfoWhenDisconnect(GetData<std::string>(), {GetData<int32_t>()}, {GetData<int32_t>()});
    server->SetAudioDeviceAnahsCallback(nullptr);
    server->UnsetAudioDeviceAnahsCallback();
}

// Lines 5328-5332, 5413-5426: NotifySubProfileChanged, InjectInterruption
void FuzzNotifyAndInject()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->NotifySubProfileChanged();
    InterruptEvent event = {};
    event.eventType = GetData<InterruptType>();
    event.forceType = GetData<InterruptForceType>();
    event.hintType = GetData<InterruptHint>();
    server->InjectInterruption(GetData<std::string>(), event);
    server->CheckHibernateState(GetData<int32_t>() % MAXVALUE == 1);
    server->UpdateSafeVolumeByS4();
    server->CheckConnectedDevice();
}

// Lines 5448-5601: AudioSession operations
void FuzzAudioSessionOperations()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ActivateAudioSession(GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    server->DeactivateAudioSession();
    bool isActive = false;
    server->IsAudioSessionActivated(isActive);
    bool isExistence = false;
    server->IsOtherMediaPlaying(isExistence);
    server->SetRegisterInterruptCallbackFlag(GetData<uint32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    server->SetRegisterStateChangeCallbackFlag(GetData<int32_t>() % MAXVALUE == 1);
    server->SetAudioSessionScene(GetData<int32_t>());
    server->EnableMuteSuggestionWhenMixWithOthers(GetData<int32_t>() % MAXVALUE == 1);
    server->SetAudioSessionCapturerMuteHint(GetData<int32_t>() % MAXVALUE == 1);
    server->SetStreamInterruptMute(GetData<uint32_t>(), GetData<int32_t>() % MAXVALUE == 1);
}

// Lines 5606-5652: GetDefaultOutputDevice, SetDefaultOutputDevice, SetSplitModeReady, LoadSplitModule
void FuzzDefaultOutputAndSplit()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t deviceType = 0;
    server->GetDefaultOutputDevice(deviceType);
    server->SetDefaultOutputDevice(GetData<int32_t>());
    server->SetSplitModeReady();
    server->LoadSplitModule(GetData<std::string>(), GetData<std::string>());
}

// Lines 5666-5717: IsAllowedPlayback, SetVoiceRingtoneMute, NotifySessionStateChange, etc.
void FuzzPlaybackAndBackground()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool isAllowed = false;
    bool silentControl = false;
    server->IsAllowedPlayback(GetData<int32_t>(), GetData<int32_t>(), GetData<uint32_t>(),
        GetData<int32_t>(), isAllowed, silentControl);
    server->SetVoiceRingtoneMute(GetData<int32_t>() % MAXVALUE == 1);
    server->NotifySessionStateChange(GetData<int32_t>(), GetData<int32_t>(),
        GetData<int32_t>() % MAXVALUE == 1);
    std::set<int32_t> pidList = {GetData<int32_t>()};
    server->NotifyFreezeStateChange(pidList, GetData<int32_t>() % MAXVALUE == 1);
    server->ResetAllProxy();
    server->NotifyProcessBackgroundState(GetData<int32_t>(), GetData<int32_t>());
}

// Lines 5759-5783: SetDeviceConnectionStatus, SetQueryAllowedPlaybackCallback, SetBackgroundMuteCallback
void FuzzDeviceConnectionAndCallbacks()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = GetData<DeviceType>();
    desc->deviceRole_ = GetData<int32_t>() % MAXVALUE == 1 ? INPUT_DEVICE : OUTPUT_DEVICE;
    server->SetDeviceConnectionStatus(desc, GetData<int32_t>() % MAXVALUE == 1);
    server->SetQueryAllowedPlaybackCallback(nullptr);
    server->SetBackgroundMuteCallback(nullptr);
}

// Lines 5797-5830: GetMaxVolumeLevelByUsage, GetMinVolumeLevelByUsage, GetVolumeLevelByUsage, GetStreamMuteByUsage
void FuzzVolumeByUsage()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t retLevel = 0;
    server->GetMaxVolumeLevelByUsage(GetData<int32_t>(), retLevel);
    server->GetMinVolumeLevelByUsage(GetData<int32_t>(), retLevel);
    server->GetVolumeLevelByUsage(GetData<int32_t>(), retLevel);
    bool isMute = false;
    server->GetStreamMuteByUsage(GetData<int32_t>(), isMute);
    float retFloat = 0.0f;
    server->GetVolumeInDbByStream(GetData<int32_t>(), GetData<int32_t>(), GetData<int32_t>(), retFloat);
    int32_t retMod = 0;
    AudioStreamInfo streamInfo = {};
    server->GetDirectPlaybackSupport(streamInfo, GetData<int32_t>(), retMod);
    std::vector<int32_t> retTypes;
    server->GetSupportedAudioVolumeTypes(retTypes);
    server->GetAudioVolumeTypeByStreamUsage(GetData<int32_t>(), retLevel);
    server->GetStreamUsagesByVolumeType(GetData<int32_t>(), retTypes);
}

// Lines 5885-5892: SetCallbackStreamUsageInfo
void FuzzCallbackStreamUsageInfo()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::set<int32_t> streamUsages = {STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC};
    server->SetCallbackStreamUsageInfo(streamUsages);
    server->ForceStopAudioStream(GetData<int32_t>());
    AudioCapturerInfo capturerInfo = {};
    bool ret = false;
    server->IsCapturerFocusAvailable(capturerInfo, ret);
}

// Lines 5937-5947: NotifyVoipPriorityLoweredByMute, SetSourceOutputStreamMuteByStreamId
void FuzzVoipPriorityAndMute()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->NotifyVoipPriorityLoweredByMute(GetData<uint32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    server->SetSourceOutputStreamMuteByStreamId(GetData<int32_t>(), GetData<int32_t>() % MAXVALUE == 1);
    bool aecRet = false;
    server->IsAcousticEchoCancelerSupported(GetData<int32_t>(), aecRet);
    bool karaokeRet = false;
    server->SetKaraokeParameters(GetData<int32_t>(), GetData<int32_t>(), GetData<int32_t>(), karaokeRet);
}

// Lines 5982-6009: SetLoopbackVolume, GetLoopbackVolume, IsAudioLoopbackSupported
void FuzzLoopbackVolume()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    float volume = GetData<float>();
    bool ret = false;
    server->SetLoopbackVolume(volume, ret);
    float outVolume = 0.0f;
    server->GetLoopbackVolume(outVolume, ret);
    server->IsAudioLoopbackSupported(GetData<int32_t>(), GetData<int32_t>(), ret);
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->IsDeviceSupportLoopback(device);
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    server->IsDeviceSupportLoopback(device);
}

// Lines 6015-6016, 6020-6024, 6027-6106: IsDeviceSupportLoopback, GetLoopbackSupportedDevicePairs,
// GetLoopbackPreferredDevicePair
void FuzzLoopbackDevicePairs()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    server->IsDeviceSupportLoopback(device);
    device->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->IsDeviceSupportLoopback(device);
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    device->macAddress_ = GetData<std::string>();
    server->IsDeviceSupportLoopback(device);
    device->deviceType_ = GetData<DeviceType>();
    server->IsDeviceSupportLoopback(device);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDevices;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices;
    int32_t ret = 0;
    server->GetLoopbackSupportedDevicePairs(inputDevices, outputDevices, ret);
    std::shared_ptr<AudioDeviceDescriptor> inputDevice;
    std::shared_ptr<AudioDeviceDescriptor> outputDevice;
    server->GetLoopbackPreferredDevicePair(inputDevice, outputDevice, ret);
}

// Lines 6114-6118: UpdateDeviceInfo
void FuzzUpdateDeviceInfo()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t command = GetData<int32_t>();
    server->UpdateDeviceInfo(nullptr, command);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = GetData<DeviceType>();
    desc->deviceRole_ = GetData<int32_t>() % MAXVALUE == 1 ? INPUT_DEVICE : OUTPUT_DEVICE;
    server->UpdateDeviceInfo(desc, command);
}

// Lines 6124-6127, 6140-6142: SetSleAudioOperationCallback, IsCollaborativePlaybackSupported, etc.
void FuzzSleAndCollaborative()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetSleAudioOperationCallback(nullptr);
    sptr<IRemoteObject> brokerObj;
    server->GetVADeviceBroker(brokerObj);
    if (brokerObj != nullptr) {
        server->SetSleAudioOperationCallback(brokerObj);
    }
    bool collaborativeRet = false;
    server->IsCollaborativePlaybackSupported(collaborativeRet);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    server->SetCollaborativePlaybackEnabledForDevice(desc, GetData<int32_t>() % MAXVALUE == 1);
    bool isSupported = false;
    server->IsCollaborativePlaybackSupportedForDevice(desc, isSupported);
    bool isEnabled = false;
    server->IsCollaborativePlaybackEnabledForDevice(desc, isEnabled);
}

// Lines 6181-6182, 6190-6191: CallRingtoneLibrary
// Lines 6227-6228: SetActiveHfpDevice
void FuzzRingtoneAndHfp()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->CallRingtoneLibrary();
    server->SetActiveHfpDevice(GetData<std::string>());
    sptr<IRemoteObject> vaController;
    server->GetVADeviceController(GetData<std::string>(), vaController);
    server->SelectPrivateDevice(GetData<int32_t>(), GetData<std::string>());
    server->ForceSelectDevice(GetData<int32_t>(), GetData<std::string>(), nullptr);
    bool nrRet = false;
    server->IsIntelligentNoiseReductionEnabledForCurrentDevice(GetData<int32_t>(), nrRet);
}

// Lines 6248-6249: RestoreDistributedDeviceInfo
// Lines 6286-6295: GetSystemVolumeDegree
void FuzzRestoreAndVolumeDegree()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RestoreDistributedDeviceInfo();
    int32_t volumeDegree = 0;
    server->GetSystemVolumeDegree(GetData<int32_t>(), GetData<int32_t>(), volumeDegree);
    int32_t volumeDegreeWithDevice = 0;
    server->GetSystemVolumeDegreeWithDevice(GetData<int32_t>(), GetData<int32_t>(),
        GetData<std::string>(), volumeDegreeWithDevice);
    server->SetSystemVolumeDegree(GetData<int32_t>(), GetData<int32_t>(),
        GetData<int32_t>(), GetData<int32_t>());
    int32_t minVolumeDegree = 0;
    server->GetMinVolumeDegree(GetData<int32_t>(), GetData<int32_t>(), minVolumeDegree);
    server->UpdateSafeVolumeByStrMode();
}

// Lines 6324-6328: RecordBootAnimationClientStateTime
void FuzzBootAnimationAndScene()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    BootAnimationInfo bootInfo = {GetData<int64_t>(), GetData<int64_t>()};
    server->RecordBootAnimationClientStateTime(bootInfo);
    server->IsPublishCalled();
    std::vector<AudioZoneMix> audioMixes;
    AudioZoneMix mix = {};
    mix.streamUsages = {STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC};
    audioMixes.push_back(mix);
    server->SetCustomAudioMix(GetData<std::string>(), audioMixes);
    int32_t audioScene = 0;
    server->GetAudioSceneFromAllZones(audioScene);
    server->NotifyStreamSilentChange(GetData<int32_t>(), GetData<uint32_t>());
    std::vector<AudioZoneVolumeConfig> configs;
    AudioZoneVolumeConfig config = {};
    config.volumeType = static_cast<AudioVolumeType>(GetData<int32_t>());
    config.minVolume = GetData<int32_t>();
    config.maxVolume = GetData<int32_t>();
    config.defaultVolume = GetData<int32_t>();
    configs.push_back(config);
    server->SetAudioZoneVolumeConfig(GetData<std::string>(), configs);
    server->TriggerAudioZoneVolumeSync();
}

// Lines 6358-6362: SetAudioSessionBehavior
// Lines 6368-6376: GetActiveStreamsVolumeInfo
void FuzzSessionBehaviorAndVolumeInfo()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetAudioSessionBehavior(GetData<uint32_t>());
    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    server->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
}

// Lines 6395-6455: IsAudioSeparationEffectSupported, SetAudioSeparationEffectEnabled,
// SetAudioSeparationEffectVolume, CreateLoopback, DestroyLoopback
void FuzzAudioSeparationAndLoopback()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool separationRet = false;
    server->IsAudioSeparationEffectSupported(separationRet);
    server->SetAudioSeparationEffectEnabled(GetData<int32_t>() % MAXVALUE == 1,
        GetData<int32_t>(), GetData<int64_t>());
    server->SetAudioSeparationEffectVolume(GetData<int32_t>(), GetData<double>());
    sptr<IRemoteObject> loopback;
    int32_t createRet = 0;
    server->CreateLoopback(GetData<int32_t>(), GetData<int32_t>(), nullptr, loopback, createRet);
    sptr<IRemoteObject> brokerObj;
    server->GetVADeviceBroker(brokerObj);
    if (brokerObj != nullptr) {
        server->CreateLoopback(GetData<int32_t>(), GetData<int32_t>(), brokerObj, loopback, createRet);
    }
    int32_t destroyRet = 0;
    server->DestroyLoopback(GetData<int32_t>(), destroyRet);
}

// Lines 6462-6464, 6470-6472: GetAppBackGroundState
// Lines 6475-6489: EnableSystemRecordController
void FuzzAppBackgroundAndRecordController()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool isBack = false;
    server->GetAppBackGroundState(GetData<int32_t>(), isBack);
    server->GetAppBackGroundState(IPCSkeleton::GetCallingPid(), isBack);
    SystemRecordControllerConfig config = {};
    config.sourceType = GetData<SourceType>();
    server->EnableSystemRecordController(GetData<int32_t>() % MAXVALUE == 1, config, nullptr);
    sptr<IRemoteObject> brokerObj;
    server->GetVADeviceBroker(brokerObj);
    server->EnableSystemRecordController(true, config, brokerObj);
    server->EnableSystemRecordController(false, config, brokerObj);
}

TestFuncs g_testFuncs[] = {
    FuzzTranslateErrorCode,
    FuzzOnDump,
    FuzzRegisterPermStateChangeCallback,
    FuzzGetScreenStates,
    FuzzOnStop,
    FuzzOnAddSystemAbility,
    FuzzHandleAudioServerRelatedSystemAbility,
    FuzzReloadLoudVolumeMode,
    FuzzChangeVolumeOnVoiceAssistant,
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    FuzzRegisterVolumeKeyEvents,
#endif
    FuzzIsContinueAddVol,
    FuzzTriggerMuteCheck,
    FuzzGetActiveAudioInterruptZone,
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    FuzzProcessVolumeKeyEvents,
#endif
    FuzzIsScreenOnAdjustVolume,
    FuzzSetVolumeInternalByKeyEvent,
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    FuzzRegisterVolumeKeyMuteEvents,
    FuzzSubscribeVolumeKeyEvents,
    FuzzSubscribeVolumeKeyEventsRetryFunc,
#endif
    FuzzSubscribeOsAccountSubProfileChangeEvents,
    FuzzAddAudioServiceOnStart,
    FuzzSubscribePowerStateChangeEvents,
    FuzzAudioCommonEventSubscriberOnReceiveEvent,
    FuzzOnReceiveEventVariations,
    FuzzConnectServiceAdapter,
    FuzzSetSystemVolumeLevelLegacy,
    FuzzSetVoipRendererMute,
    FuzzGetSystemVolumeLevel,
    FuzzGetSystemVolumeInDb,
    FuzzSetStreamMuteInternal,
    FuzzSetSystemVolumeLevelWithDeviceInternal,
    FuzzIsArmUsbDeviceAndMapExternal,
    FuzzGetDevicesInner,
    FuzzGetPreferredDeviceDescriptors,
    FuzzSetDeclaredDeviceTypes,
    FuzzSetClientCallbacks,
    FuzzIsStreamActiveByStreamUsage,
    FuzzIsPlaybackSupported,
    FuzzGetHwEncodingTypeMaxBitrate,
    FuzzIsDeviceActive,
    FuzzSetRingerModeInner,
    FuzzInitMicrophoneMute,
    FuzzMicAndRingerMode,
    FuzzAudioInterruptCallbacks,
    FuzzAudioFocusAndVKB,
    FuzzActivateAudioInterrupt,
    FuzzSetAppConcurrency,
    FuzzDeactivateAudioInterrupt,
    FuzzPreemptAndFocus,
    FuzzRegisterTracker,
    FuzzGetCurrentRendererChangeInfosForZone,
    FuzzRegisterClientDeathRecipient,
    FuzzGetNetworkIdByGroupId,
    FuzzRemoteParameterCallback,
    FuzzPerStateChangeCallback,
    FuzzMiscServerFunctions,
    FuzzSetA2dpAndNearlinkVolume,
    FuzzDeviceSelection,
    FuzzGetAvailableDevices,
    FuzzSetAvailableDeviceChangeCallback,
    FuzzDistributedRoutingRole,
    FuzzPowerAndHibernateListeners,
    FuzzAppStateListeners,
    FuzzRegisterPolicyCallbackClient,
    FuzzAudioZoneOperations,
    FuzzDeviceRefinerAndPreferred,
    FuzzDeviceVolumeBehavior,
    FuzzNotifyAndInject,
    FuzzAudioSessionOperations,
    FuzzDefaultOutputAndSplit,
    FuzzPlaybackAndBackground,
    FuzzDeviceConnectionAndCallbacks,
    FuzzVolumeByUsage,
    FuzzCallbackStreamUsageInfo,
    FuzzVoipPriorityAndMute,
    FuzzLoopbackVolume,
    FuzzLoopbackDevicePairs,
    FuzzUpdateDeviceInfo,
    FuzzSleAndCollaborative,
    FuzzRingtoneAndHfp,
    FuzzRestoreAndVolumeDegree,
    FuzzBootAnimationAndScene,
    FuzzSessionBehaviorAndVolumeInfo,
    FuzzAudioSeparationAndLoopback,
    FuzzAppBackgroundAndRecordController,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    return sizeof(arr) / sizeof(arr[0]);
}

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    }
    return true;
}
} // namespace AudioStandard

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    OHOS::OnStop();
    return 0;
}
