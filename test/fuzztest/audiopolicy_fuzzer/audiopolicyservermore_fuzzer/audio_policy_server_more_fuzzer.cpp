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
    if (strSize > g_dataSize - g_pos) {
        return object;
    }
    if (strSize > 0) {
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
