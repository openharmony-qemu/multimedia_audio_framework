/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"

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
const int32_t DEVICE_COUNT = 4;
const int32_t COUNT = 8;
static int32_t NUM_2 = 2;
typedef void (*TestFuncs)();

class RemoteObjectFuzzTestStub : public IRemoteObject {
public:
    RemoteObjectFuzzTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectFuzzTestStub");
};

class AudioClientTrackerFuzzTest : public AudioClientTracker {
    public:
        virtual ~AudioClientTrackerFuzzTest() = default;
        virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void SetLowPowerVolumeImpl(float volume) {};
        virtual void GetLowPowerVolumeImpl(float &volume) {};
        virtual void GetSingleStreamVolumeImpl(float &volume) {};
        virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
        virtual void UnsetOffloadModeImpl() {};
    };

const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};

const vector<AudioStreamDeviceChangeReason> g_testReasons = {
    AudioStreamDeviceChangeReason::UNKNOWN,
    AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
    AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
    AudioStreamDeviceChangeReason::OVERRODE,
};

const vector<StreamSetState> g_testStreamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE,
};

vector<InterruptHint> g_testInterruptHints = {
    INTERRUPT_HINT_NONE,
    INTERRUPT_HINT_RESUME,
    INTERRUPT_HINT_PAUSE,
    INTERRUPT_HINT_STOP,
    INTERRUPT_HINT_DUCK,
    INTERRUPT_HINT_UNDUCK,
    INTERRUPT_HINT_MUTE,
    INTERRUPT_HINT_UNMUTE,
    INTERRUPT_HINT_EXIT_STANDALONE
};

vector<AudioParamKey> g_testAudioParamKeys = {
    NONE,
    VOLUME,
    INTERRUPT,
    PARAM_KEY_STATE,
    A2DP_SUSPEND_STATE,
    BT_HEADSET_NREC,
    BT_WBS,
    A2DP_OFFLOAD_STATE,
    GET_DP_DEVICE_INFO,
    GET_PENCIL_INFO,
    GET_UWB_INFO,
    USB_DEVICE,
    PERF_INFO,
    MMI,
    PARAM_KEY_LOWPOWER,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

void AudioPolicyServerRegisterDefaultVolumeTypeListenerFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->RegisterDefaultVolumeTypeListener();
}

void AudioPolicyServerOnAddSystemAbilityExtractFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t systemAbilityId = APP_MGR_SERVICE_ID;
    std::string deviceId = "";
    audioPolicyServer->OnAddSystemAbilityExtract(systemAbilityId, deviceId);
    int32_t systemAbilityId1 = 0;
    audioPolicyServer->OnAddSystemAbilityExtract(systemAbilityId1, deviceId);
}

void AudioPolicyServerOnRemoveSystemAbilityFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t systemAbilityId = APP_MGR_SERVICE_ID;
    std::string deviceId = "";
    audioPolicyServer->OnRemoveSystemAbility(systemAbilityId, deviceId);
}

void AudioPolicyServerMaxOrMinVolumeOptionFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volLevel = GetData<int32_t>();
    int32_t keyType = GetData<int32_t>();
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    audioPolicyServer->MaxOrMinVolumeOption(volLevel, keyType, streamInFocus);
}

void AudioPolicyServerChangeVolumeOnVoiceAssistantFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_VOICE_ASSISTANT;
    audioPolicyServer->ChangeVolumeOnVoiceAssistant(streamInFocus);
}

void AudioPolicyServerIsContinueAddVolFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->IsContinueAddVol();
}

void AudioPolicyServerTriggerMuteCheckFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->TriggerMuteCheck();
}

void AudioPolicyServerProcessVolumeKeyEventsFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t keyType = GetData<int32_t>();
    audioPolicyServer->ProcessVolumeKeyEvents(keyType);
}

void AudioPolicyServerSetVolumeInternalByKeyEventFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    int32_t zoneId = GetData<int32_t>();
    int32_t keyType = GetData<int32_t>();
    audioPolicyServer->SetVolumeInternalByKeyEvent(streamInFocus, zoneId, keyType);
}

void AudioPolicyServerSubscribeSafeVolumeEventFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    int32_t zoneId = GetData<int32_t>();
    int32_t keyType = GetData<int32_t>();
    audioPolicyServer->SubscribeSafeVolumeEvent();
}

void AudioPolicyServerIsVolumeTypeValidFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    audioPolicyServer->IsVolumeTypeValid(streamInFocus);
}

void AudioPolicyServerIsVolumeLevelValidFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->IsVolumeLevelValid(streamInFocus, volumeLevel);
}

void AudioPolicyServerIsRingerModeValidFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioRingerMode ringMode = AudioRingerMode::RINGER_MODE_SILENT;
    audioPolicyServer->IsRingerModeValid(ringMode);
}

void AudioPolicyServerSubscribeOsAccountChangeEventsFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->SubscribeOsAccountChangeEvents();
}

void AudioPolicyServerAddRemoteDevstatusCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->AddRemoteDevstatusCallback();
}

void AudioPolicyServerOnReceiveEventFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    EventFwk::CommonEventData eventData;
    audioPolicyServer->OnReceiveEvent(eventData);
}

void AudioPolicyServerSubscribeBackgroundTaskFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->SubscribeBackgroundTask();
}

void AudioPolicyServerSubscribeCommonEventExecuteFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->SubscribeCommonEventExecute();
}

void AudioPolicyServerCheckSubscribePowerStateChangeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->CheckSubscribePowerStateChange();
}

void AudioPolicyServerNotifySettingsDataReadyFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->NotifySettingsDataReady();
}

void AudioPolicyServerGetMaxVolumeLevelFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeType = GetData<int32_t>();
    int32_t deviceType = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->GetMaxVolumeLevel(volumeType, volumeLevel, deviceType);
}

void AudioPolicyServerGetMinVolumeLevelFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeType = GetData<int32_t>();
    int32_t deviceType = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->GetMinVolumeLevel(volumeType, volumeLevel, deviceType);
}

void AudioPolicyServerSetSystemVolumeLevelLegacyFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamTypeIn = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->SetSystemVolumeLevelLegacy(streamTypeIn, volumeLevel);
}

void AudioPolicyServerSetAdjustVolumeForZoneFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t zoneId = GetData<int32_t>();
    audioPolicyServer->SetAdjustVolumeForZone(zoneId);
}

void AudioPolicyServerGetSelfAppVolumeLevelFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->GetSelfAppVolumeLevel(volumeLevel);
}

void AudioPolicyServerGetSystemVolumeLevelFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamType = GetData<int32_t>();
    int32_t uid = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->GetSystemVolumeLevel(streamType, uid, volumeLevel);
}

void AudioPolicyServerGetSystemVolumeLevelNoMuteStateFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    audioPolicyServer->GetSystemVolumeLevelNoMuteState(streamInFocus);
}

void AudioPolicyServerGetSystemVolumeLevelInternalFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamInFocus = AudioStreamType::STREAM_ALL;
    int32_t zoneId = GetData<int32_t>();
    audioPolicyServer->GetSystemVolumeLevelInternal(streamInFocus, zoneId);
}

void AudioPolicyServerGetAppVolumeLevelInternalFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t appUid = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->GetAppVolumeLevelInternal(appUid, volumeLevel);
}

void AudioPolicyServerSetLowPowerVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamId = GetData<int32_t>();
    float volume = GetData<float>();
    audioPolicyServer->SetLowPowerVolume(streamId, volume);
}

void AudioPolicyServerGetFastStreamInfoFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamInfo streamInfo;
    audioPolicyServer->GetFastStreamInfo(streamInfo);
}

void AudioPolicyServerGetLowPowerVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamId = GetData<int32_t>();
    float outVolume;
    audioPolicyServer->GetLowPowerVolume(streamId, outVolume);
}

void AudioPolicyServerGetSingleStreamVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamId = GetData<int32_t>();
    float outVolume;
    audioPolicyServer->GetSingleStreamVolume(streamId, outVolume);
}

void AudioPolicyServerIsVolumeUnadjustableFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool unadjustable;
    audioPolicyServer->IsVolumeUnadjustable(unadjustable);
}

void AudioPolicyServerCheckCanMuteVolumeTypeByStepFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioVolumeType volumeType = AudioVolumeType::STREAM_VOICE_CALL;
    int32_t volumeLevel = GetData<int32_t>();
    audioPolicyServer->CheckCanMuteVolumeTypeByStep(volumeType, volumeLevel);
}

void AudioPolicyServerGetSystemVolumeInDbFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeTypeIn = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t deviceTypeIn = GetData<int32_t>();
    float volume;
    audioPolicyServer->GetSystemVolumeInDb(volumeTypeIn, volumeLevel, deviceTypeIn, volume);
}

void AudioPolicyServerSetStreamMuteLegacyFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t streamTypeIn = GetData<int32_t>();
    int32_t deviceTypeIn = GetData<int32_t>();
    bool mute = true;
    audioPolicyServer->SetStreamMuteLegacy(streamTypeIn, mute, deviceTypeIn);
}

void AudioPolicyServerSetStreamMuteInternalFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t zoneId = GetData<int32_t>();
    DeviceType deviceType = DEVICE_TYPE_NONE;
    bool mute = true;
    bool isUpdateUi = true;
    AudioStreamType streamType = AudioStreamType::STREAM_ALL;
    audioPolicyServer->SetStreamMuteInternal(streamType, mute, isUpdateUi, deviceType, zoneId);
}

void AudioPolicyServerUpdateSystemMuteStateAccordingMusicStateFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool mute = true;
    bool isUpdateUi = true;
    AudioStreamType streamType = AudioStreamType::STREAM_ALL;
    audioPolicyServer->UpdateSystemMuteStateAccordingMusicState(streamType, mute, isUpdateUi);
}

void AudioPolicyServerSendMuteKeyEventCbWithUpdateUiOrNotFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t zoneId = GetData<int32_t>();
    bool isUpdateUi = true;
    AudioStreamType streamType = AudioStreamType::STREAM_ALL;
    audioPolicyServer->SendMuteKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId);
}

void AudioPolicyServerSetSingleStreamMuteFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t zoneId = GetData<int32_t>();
    bool isUpdateUi = true;
    AudioStreamType streamType = AudioStreamType::STREAM_ALL;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    bool mute = true;
    audioPolicyServer->SetSingleStreamMute(streamType, mute, isUpdateUi, deviceType, zoneId);
}

void AudioPolicyServerProcUpdateRingerModeForMuteFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool updateRingerMode = true;
    bool mute = true;
    audioPolicyServer->ProcUpdateRingerModeForMute(updateRingerMode, mute);
}

void AudioPolicyServerGetSystemVolumeDbFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_ALL;
    audioPolicyServer->GetSystemVolumeDb(streamType);
}

void AudioPolicyServerSetSelfAppVolumeLevelFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    audioPolicyServer->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
}

void AudioPolicyServerSetAppVolumeLevelInternalFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool isUpdateUi = GetData<bool>();
    bool muted = GetData<bool>();
    int32_t appUid = GetData<int32_t>();
    audioPolicyServer->SetAppVolumeLevelInternal(appUid, muted, isUpdateUi);
}

void AudioPolicyServerSetSystemVolumeLevelInternalFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL_ASSISTANT;
    bool isUpdateUi = GetData<bool>();
    int32_t volumeLevel = 0;
    int32_t zoneId = GetData<int32_t>();
    audioPolicyServer->SetSystemVolumeLevelInternal(streamType, volumeLevel, isUpdateUi, zoneId);
}

void AudioPolicyServerSetSystemVolumeLevelWithDeviceInternalFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL_ASSISTANT;
    bool isUpdateUi = GetData<bool>();
    int32_t volumeLevel = 0;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_EARPIECE;
    audioPolicyServer->SetSystemVolumeLevelWithDeviceInternal(streamType, volumeLevel, isUpdateUi, deviceType);
}

void AudioPolicyServerSendVolumeKeyEventCbWithUpdateUiOrNotFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL_ASSISTANT;
    bool isUpdateUi = GetData<bool>();
    int32_t zoneId = GetData<int32_t>();
    audioPolicyServer->SendVolumeKeyEventCbWithUpdateUiOrNot(streamType, isUpdateUi, zoneId);
}

void AudioPolicyServerUpdateMuteStateAccordingToVolLevelFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    bool isUpdateUi = GetData<bool>();
    int32_t zoneId = GetData<int32_t>();
    int32_t volumeLevel = 0;
    bool mute = false;
    audioPolicyServer->UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute, isUpdateUi, zoneId);
    volumeLevel = 1;
    mute = true;
    audioPolicyServer->UpdateMuteStateAccordingToVolLevel(streamType, volumeLevel, mute, isUpdateUi, zoneId);
}

void AudioPolicyServerProcUpdateRingerModeFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->supportVibrator_ = true;
    audioPolicyServer->ProcUpdateRingerMode();
    audioPolicyServer->supportVibrator_ = false;
    audioPolicyServer->ProcUpdateRingerMode();
}

void AudioPolicyServerSetAppSingleStreamVolumeFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeLevel = GetData<int32_t>();
    bool isUpdateUi = GetData<bool>();
    int32_t appUid = GetData<int32_t>();
    audioPolicyServer->SetAppSingleStreamVolume(appUid, volumeLevel, isUpdateUi);
}

void AudioPolicyServerSetSingleStreamVolumeFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    int32_t volumeLevel = GetData<int32_t>();
    bool isUpdateUi = GetData<bool>();
    bool mute = GetData<bool>();
    int32_t zoneId = GetData<int32_t>();
    audioPolicyServer->SetSingleStreamVolume(AudioStreamType::STREAM_RING, volumeLevel, isUpdateUi, mute, zoneId);
    audioPolicyServer->SetSingleStreamVolume(AudioStreamType::STREAM_VOICE_ASSISTANT, volumeLevel,
        isUpdateUi, mute, zoneId);
}

void AudioPolicyServerSetSingleStreamVolumeWithDeviceFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool mute = GetData<bool>();
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = GetData<int32_t>();
    bool isUpdateUi = GetData<bool>();
    DeviceType deviceType = GetData<DeviceType>();
    audioPolicyServer->SetSingleStreamVolumeWithDevice(streamType, volumeLevel, isUpdateUi, deviceType);
}

void AudioPolicyServerGetStreamMuteFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool mute = GetData<bool>();
    AudioStreamType streamTypeIn = STREAM_RING;
    audioPolicyServer->GetStreamMute(streamTypeIn, mute);
}

void AudioPolicyServerMapExternalToInternalDeviceTypeFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioDeviceDescriptor desc = std::make_shared<AudioDeviceDescriptor>();
    desc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    audioPolicyServer->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc.deviceRole_ = INPUT_DEVICE;
    audioPolicyServer->MapExternalToInternalDeviceType(desc);
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.deviceRole_ = INPUT_DEVICE;
    audioPolicyServer->MapExternalToInternalDeviceType(desc);
}

void AudioPolicyServerGetPreferredOutputDeviceDescriptorsFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioRendererInfo rendererInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    bool forceNoBTPermission = false;
    audioPolicyServer->GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission, deviceDescs);
}

void AudioPolicyServerGetPreferredInputDeviceDescriptorsFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioCapturerInfo capturerInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDescs.push_back(deviceDesc);
    audioPolicyServer->GetPreferredInputDeviceDescriptors(capturerInfo, deviceDescs);
}

void AudioPolicyServerIsFastRecordingSupportedFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioStreamInfo streamInfo = {};
    int32_t source = GetData<uint16_t>();
    bool support = GetData<bool>();
    audioPolicyServer->IsFastRecordingSupported(streamInfo, source, support);
}

void AudioPolicyServerSetRingerModeInternalFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioRingerMode inputRingerMode = GetData<AudioRingerMode>();
    bool hasUpdatedVolume = false;
    audioPolicyServer->SetRingerModeInternal(inputRingerMode, hasUpdatedVolume);
}

void AudioPolicyServerInitMicrophoneMuteFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->isInitMuteState_ = true;
    audioPolicyServer->InitMicrophoneMute();
    audioPolicyServer->isInitMuteState_ = false;
    audioPolicyServer->InitMicrophoneMute();
}

void AudioPolicyServerSetMicrophoneMuteAudioConfigFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool isMute = GetData<bool>();
    audioPolicyServer->SetMicrophoneMuteAudioConfig(isMute);
}

void AudioPolicyServerSetMicrophoneMutePersistentFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    bool isMute = GetData<bool>();
    PolicyType typeIn = GetData<PolicyType>();
    audioPolicyServer->SetMicrophoneMutePersistent(isMute, typeIn);
}

void AudioPolicyServerSetAudioSceneFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    AudioScene audioSceneIn = AUDIO_SCENE_DEFAULT;
    audioPolicyServer->SetAudioScene(audioSceneIn);
    audioSceneIn = AUDIO_SCENE_PHONE_CALL;
    audioPolicyServer->SetAudioScene(audioSceneIn);
    audioSceneIn = AUDIO_SCENE_PHONE_CHAT;
    audioPolicyServer->SetAudioScene(audioSceneIn);
}

void AudioPolicyServerSetAndUnsetAudioInterruptCallbackFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    uint32_t sessionID = GetData<uint32_t>();
    uint32_t zoneID = GetData<uint32_t>();
    uint32_t clientUid = GetData<uint32_t>();
    sptr<IRemoteObject> object = new RemoteObjectFuzzTestStub();
    audioPolicyServer->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    audioPolicyServer->UnsetAudioInterruptCallback(sessionID, zoneID);
}

void AudioPolicyServerVerifySessionIdFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t clientUid = GetData<uint32_t>();
    audioPolicyServer->VerifySessionId(sessionId, clientUid);
}

void AudioPolicyServerSetAndUnsetAudioRouteCallbackFuzztest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    sptr<IRemoteObject> object = new RemoteObjectFuzzTestStub();
    uint32_t clientUid = GetData<uint32_t>();
    audioPolicyServer->SetAudioRouteCallback(sessionId, object, clientUid);
    audioPolicyServer->UnsetAudioRouteCallback(sessionId);
}

void AudioPolicyServerSubscribeAccessibilityConfigObserverFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);
    audioPolicyServer->SubscribeAccessibilityConfigObserver();
}

void AudioPolicyServerGetMinStreamVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    float volume = GetData<float>();
    audioPolicyServer->GetMinStreamVolume(volume);
}

void AudioPolicyServerGetMaxStreamVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    float volume = GetData<float>();
    audioPolicyServer->GetMaxStreamVolume(volume);
}

void AudioPolicyServerGetMaxRendererInstancesFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t ret = GetData<int32_t>();
    audioPolicyServer->isFirstAudioServiceStart_ = GetData<bool>();
    audioPolicyServer->GetMaxRendererInstances(ret);
}

void AudioPolicyServerRegisterDataObserverFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    audioPolicyServer->RegisterDataObserver();
}

void AudioPolicyServerQueryEffectSceneModeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    SupportedEffectConfig supportedEffectConfig;
    audioPolicyServer->QueryEffectSceneMode(supportedEffectConfig);
}

void AudioPolicyServerGetHardwareOutputSamplingRateFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = GetData<int32_t>();
    audioPolicyServer->GetHardwareOutputSamplingRate(desc, ret);
}

void AudioPolicyServerGetAudioCapturerMicrophoneDescriptorsFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t sessionId = GetData<int32_t>();
    vector<sptr<MicrophoneDescriptor>> micDescs;
    audioPolicyServer->GetAudioCapturerMicrophoneDescriptors(sessionId, micDescs);
}

void AudioPolicyServerGetAvailableMicrophonesFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    vector<sptr<MicrophoneDescriptor>> retMicList;
    audioPolicyServer->GetAvailableMicrophones(retMicList);
}

void AudioPolicyServerSetDeviceAbsVolumeSupportedFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::string macAddress = "test_mac";
    bool support = GetData<bool>();
    audioPolicyServer->SetDeviceAbsVolumeSupported(macAddress, support);
}

void AudioPolicyServerIsAbsVolumeSceneFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool ret = GetData<bool>();
    audioPolicyServer->IsAbsVolumeScene(ret);
}

void AudioPolicyServerSetA2dpDeviceVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::string macAddress = "test_mac";
    int32_t volume = GetData<int32_t>();
    bool updateUi = GetData<bool>();
    audioPolicyServer->SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

void AudioPolicyServerSetNearlinkDeviceVolumeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::string macAddress = "test_mac";
    int32_t volume = GetData<int32_t>();
    int32_t streamTypeIn = GetData<int32_t>();
    bool updateUi = GetData<bool>();
    audioPolicyServer->SetNearlinkDeviceVolume(macAddress, streamTypeIn, volume, updateUi);
}

void AudioPolicyServerGetAvailableDevicesFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t usageIn = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioPolicyServer->GetAvailableDevices(usageIn, descs);
}

void AudioPolicyServerSetAvailableDeviceChangeCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t clientId = GetData<int32_t>();
    int32_t usageIn = GetData<int32_t>();
    sptr<IRemoteObject> object = nullptr;

    audioPolicyServer->SetAvailableDeviceChangeCallback(clientId, usageIn, object);
}

void AudioPolicyServerUnsetAvailableDeviceChangeCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t clientId = GetData<int32_t>();
    int32_t usageIn = GetData<int32_t>();

    audioPolicyServer->UnsetAvailableDeviceChangeCallback(clientId, usageIn);
}

void AudioPolicyServerOffloadStopPlayingFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    StreamUsage streamUsage = GetData<StreamUsage>();
    ContentType contentType = GetData<ContentType>();
    AudioFocusType audioFocusType = GetData<AudioFocusType>();
    uint32_t streamId = GetData<uint32_t>();
    AudioInterrupt audioInterrupt(streamUsage, contentType, audioFocusType, streamId);

    audioPolicyServer->OffloadStopPlaying(audioInterrupt);
}

void AudioPolicyServerCheckAudioSessionStrategyFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    AudioSessionStrategy sessionStrategy;
    sessionStrategy.concurrencyMode = GetData<AudioConcurrencyMode>();

    audioPolicyServer->CheckAudioSessionStrategy(sessionStrategy);
}

void AudioPolicyServerSetAudioSessionSceneFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t audioSessionScene = GetData<int32_t>();

    audioPolicyServer->SetAudioSessionScene(audioSessionScene);
}

void AudioPolicyServerGetDefaultOutputDeviceFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t deviceType = GetData<int32_t>();
    std::shared_ptr<AudioCoreService> coreService = make_shared<AudioCoreService>();

    audioPolicyServer->GetDefaultOutputDevice(deviceType);
}

void AudioPolicyServerSetDefaultOutputDeviceFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t deviceType = GetData<int32_t>();

    audioPolicyServer->SetDefaultOutputDevice(deviceType);
}

void AudioPolicyServerLoadSplitModuleFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::string splitArgs = "test_split_args";
    std::string networkId = "test_network_id";

    audioPolicyServer->LoadSplitModule(splitArgs, networkId);
}

void AudioPolicyServerIsAllowedPlaybackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    bool isAllowed = GetData<bool>();

    audioPolicyServer->IsAllowedPlayback(uid, pid, isAllowed);
}

void AudioPolicyServerSetVoiceRingtoneMuteFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool isMute = GetData<bool>();

    audioPolicyServer->SetVoiceRingtoneMute(isMute);
}

void AudioPolicyServerNotifySessionStateChangeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool hasSession = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();

    audioPolicyServer->NotifySessionStateChange(uid, pid, hasSession);
}

void AudioPolicyServerNotifyFreezeStateChangeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool isFreeze = GetData<bool>();
    int32_t pid = GetData<int32_t>();
    std::set<int32_t> pidList;
    pidList.insert(pid);
    audioPolicyServer->NotifyFreezeStateChange(pidList, isFreeze);
}

void AudioPolicyServerResetAllProxyFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    audioPolicyServer->ResetAllProxy();
}

void AudioPolicyServerNotifyProcessBackgroundStateFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();

    audioPolicyServer->NotifyProcessBackgroundState(uid, pid);
}

void AudioPolicyServerSetVirtualCallFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool isVirtual = GetData<bool>();

    audioPolicyServer->SetVirtualCall(isVirtual);
}

void AudioPolicyServerSetDeviceConnectionStatusFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool isConnected = GetData<bool>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();

    audioPolicyServer->SetDeviceConnectionStatus(desc, isConnected);
}

void AudioPolicyServerSetQueryAllowedPlaybackCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectFuzzTestStub();

    audioPolicyServer->SetQueryAllowedPlaybackCallback(object);
}

void AudioPolicyServerSetBackgroundMuteCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    sptr<IRemoteObject> object = new RemoteObjectFuzzTestStub();

    audioPolicyServer->SetBackgroundMuteCallback(object);
}

void AudioPolicyServerGetDirectPlaybackSupportFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    AudioStreamInfo streamInfo;
    int32_t streamUsage = GetData<int32_t>();
    int32_t retMod = GetData<int32_t>();

    audioPolicyServer->GetDirectPlaybackSupport(streamInfo, streamUsage, retMod);
}

void AudioPolicyServerGetMaxVolumeLevelByUsageFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsage = GetData<int32_t>();
    int32_t retMaxVolumeLevel = GetData<int32_t>();

    audioPolicyServer->GetMaxVolumeLevelByUsage(streamUsage, retMaxVolumeLevel);
}

void AudioPolicyServerGetMinVolumeLevelByUsageFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsage = GetData<int32_t>();
    int32_t retMinVolumeLevel = GetData<int32_t>();

    audioPolicyServer->GetMinVolumeLevelByUsage(streamUsage, retMinVolumeLevel);
}

void AudioPolicyServerGetVolumeLevelByUsageFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsage = GetData<int32_t>();
    int32_t retVolumeLevel = GetData<int32_t>();

    audioPolicyServer->GetVolumeLevelByUsage(streamUsage, retVolumeLevel);
}

void AudioPolicyServerGetStreamMuteByUsageFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsage = GetData<int32_t>();
    bool isMute = GetData<bool>();

    audioPolicyServer->GetStreamMuteByUsage(streamUsage, isMute);
}

void AudioPolicyServerGetVolumeInDbByStreamFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsageIn = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t deviceType = GetData<int32_t>();
    float ret = GetData<float>();

    audioPolicyServer->GetVolumeInDbByStream(streamUsageIn, volumeLevel, deviceType, ret);
}

void AudioPolicyServerGetSupportedAudioVolumeTypesFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::vector<int32_t> ret;
    ret.push_back(GetData<int32_t>());
    audioPolicyServer->GetSupportedAudioVolumeTypes(ret);
}

void AudioPolicyServerGetAudioVolumeTypeByStreamUsageFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t streamUsageIn = GetData<int32_t>();
    int32_t ret = 0;
    audioPolicyServer->GetAudioVolumeTypeByStreamUsage(streamUsageIn, ret);
}

void AudioPolicyServerGetStreamUsagesByVolumeTypeFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t audioVolumeTypeIn = GetData<int32_t>();
    std::vector<int32_t> ret;
    audioPolicyServer->GetStreamUsagesByVolumeType(audioVolumeTypeIn, ret);
}

void AudioPolicyServerSetCallbackStreamUsageInfoFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::set<int32_t> streamUsages;
    streamUsages.insert(GetData<int32_t>());
    audioPolicyServer->SetCallbackStreamUsageInfo(streamUsages);
}

void AudioPolicyServerForceStopAudioStreamFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t audioTypeIn = GetData<int32_t>();
    audioPolicyServer->ForceStopAudioStream(audioTypeIn);
}

void AudioPolicyServerIsCapturerFocusAvailableFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    SourceType sourceType = GetData<SourceType>();
    int32_t capturerFlags = GetData<int32_t>();
    AudioCapturerInfo capturerInfo(sourceType, capturerFlags);
    bool ret = GetData<bool>();

    audioPolicyServer->IsCapturerFocusAvailable(capturerInfo, ret);
}

void AudioPolicyServerUpdateDefaultOutputDeviceWhenStartingFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    uint32_t sessionID = GetData<uint32_t>();

    audioPolicyServer->UpdateDefaultOutputDeviceWhenStarting(sessionID);
}

void AudioPolicyServerUpdateDefaultOutputDeviceWhenStoppingFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    uint32_t sessionID = GetData<uint32_t>();

    audioPolicyServer->UpdateDefaultOutputDeviceWhenStopping(sessionID);
}

void AudioPolicyServerIsAcousticEchoCancelerSupportedFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t sourceType = GetData<int32_t>();
    bool ret = GetData<bool>();

    audioPolicyServer->IsAcousticEchoCancelerSupported(sourceType, ret);
}

void AudioPolicyServerSetKaraokeParametersFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::string parameters = "test_parameters";
    bool ret = GetData<bool>();

    audioPolicyServer->SetKaraokeParameters(parameters, ret);
}

void AudioPolicyServerUpdateDeviceInfoFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    int32_t command = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();

    audioPolicyServer->UpdateDeviceInfo(deviceDesc, command);
}

void AudioPolicyServerSetSleAudioOperationCallbackFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    sptr<IRemoteObject> object = nullptr;
    audioPolicyServer->SetSleAudioOperationCallback(object);
}

void AudioPolicyServerSetCollaborativePlaybackEnabledForDeviceFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    bool enabled = GetData<bool>();

    audioPolicyServer->SetCollaborativePlaybackEnabledForDevice(selectedAudioDevice, enabled);
}

void AudioPolicyServerIsCollaborativePlaybackSupportedFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    bool ret = GetData<bool>();

    audioPolicyServer->IsCollaborativePlaybackSupported(ret);
}

void AudioPolicyServerIsCollaborativePlaybackEnabledForDeviceFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    bool enabled = GetData<bool>();

    audioPolicyServer->IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice, enabled);
}

void AudioPolicyServerCallRingtoneLibraryFuzzTest()
{
    auto audioPolicyServer = GetServerPtr();
    CHECK_AND_RETURN(audioPolicyServer != nullptr);

    audioPolicyServer->CallRingtoneLibrary();
}

void SetAndUnsetDistributedRoutingRoleCallbackFuzzTest()
{
    sptr<RemoteObjectFuzzTestStub> remoteObjectStub = sptr<RemoteObjectFuzzTestStub>();
    CHECK_AND_RETURN(remoteObjectStub != nullptr);
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetDistributedRoutingRoleCallback(remoteObjectStub);
    server->UnsetDistributedRoutingRoleCallback();
}

void OnDistributedRoutingRoleChangeFuzzTest()
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    OHOS::AudioStandard::CastType castTypeValue = GetData<OHOS::AudioStandard::CastType>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->OnDistributedRoutingRoleChange(desc, castTypeValue);
}

void RegisterAndUnRegisterPowerStateListenerFuzzTest()
{
    sptr<AudioPolicyServer> server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterPowerStateListener();
    server->UnRegisterPowerStateListener();
}

void RegisterAppStateListenerFuzzTest()
{
    sptr<AudioPolicyServer> server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterAppStateListener();
}

void RegisterAndUnRegisterSyncHibernateListenerFuzzTest()
{
    sptr<AudioPolicyServer> server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterSyncHibernateListener();
    server->UnRegisterSyncHibernateListener();
}

void AudioPolicyServerRegisterSpatializationStateEventListenerFuzzTest()
{
    uint32_t sessionID = GetData<uint32_t>();
    int32_t streamUsageIn = GetData<int32_t>();
    
    sptr<RemoteObjectFuzzTestStub> object = sptr<RemoteObjectFuzzTestStub>();
    CHECK_AND_RETURN(object != nullptr);
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->RegisterSpatializationStateEventListener(sessionID, streamUsageIn, object);
    server->UnregisterSpatializationStateEventListener(sessionID);
}

void AudioPolicyServerAudioInterruptZoneFuzzTest()
{
    std::set<int32_t> pids;
    int32_t zoneID = GetData<int32_t>();
    int32_t count = DEVICE_COUNT;
    for (int32_t i = 0; i < count; ++i) {
        pids.insert(GetData<int32_t>());
    }

    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->CreateAudioInterruptZone(pids, zoneID);
    server->AddAudioInterruptZonePids(pids, zoneID);
    server->RemoveAudioInterruptZonePids(pids, zoneID);
    server->ReleaseAudioInterruptZone(zoneID);
}

void AudioPolicyServerRegisterAudioZoneClientFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    sptr<RemoteObjectFuzzTestStub> object = sptr<RemoteObjectFuzzTestStub>();
    CHECK_AND_RETURN(object != nullptr);
    server->RegisterAudioZoneClient(object);
}

void AudioPolicyServerAudioZoneQueryFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string name = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(name, context, zoneId, 0);

    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    server->GetAllAudioZone(descs);

    std::shared_ptr<AudioZoneDescriptor> desc;
    server->GetAudioZone(zoneId, desc);

    int32_t queriedZoneId = 0;
    server->GetAudioZoneByName(name, queriedZoneId);

    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerBindUnbindDeviceToAudioZoneFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string name = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(name, context, zoneId, 0);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    int32_t deviceCount = DEVICE_COUNT;
    for (int32_t i = 0; i < deviceCount; ++i) {
        auto device = std::make_shared<AudioDeviceDescriptor>();
        devices.push_back(device);
    }

    server->BindDeviceToAudioZone(zoneId, devices);

    server->UnBindDeviceToAudioZone(zoneId, devices);

    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerEnableAudioZoneReportFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool enable = GetData<bool>();
    server->EnableAudioZoneReport(enable);

    std::string name = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(name, context, zoneId, 0);

    bool changeEnable = GetData<bool>();
    server->EnableAudioZoneChangeReport(zoneId, changeEnable);

    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerAddRemoveUidToAudioZoneFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string name = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(name, context, zoneId, 0);

    int32_t uid = GetData<int32_t>();
    server->AddUidToAudioZone(zoneId, uid);
    server->RemoveUidFromAudioZone(zoneId, uid);

    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerAddStreamToAudioZoneFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string zoneName = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(zoneName, context, zoneId, 0);

    AudioZoneStream stream;
    stream.streamUsage = GetData<StreamUsage>();
    stream.sourceType = GetData<SourceType>();
    stream.isPlay = GetData<bool>();
    server->AddStreamToAudioZone(zoneId, stream);
    server->RemoveStreamFromAudioZone(zoneId, stream);
    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerAddStreamsToAudioZoneFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string zoneName = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(zoneName, context, zoneId, 0);

    std::vector<AudioZoneStream> streams;
    int32_t count = COUNT;
    for (int32_t i = 0; i < count; ++i) {
        AudioZoneStream stream;
        stream.streamUsage = GetData<StreamUsage>();
        stream.sourceType = GetData<SourceType>();
        stream.isPlay = GetData<bool>();
        streams.push_back(stream);
    }
    server->AddStreamsToAudioZone(zoneId, streams);
    server->RemoveStreamsFromAudioZone(zoneId, streams);
    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerSetZoneDeviceVisibleFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    bool visible = GetData<bool>();
    server->SetZoneDeviceVisible(visible);
}

void AudioPolicyServerEnableSystemVolumeProxyFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string zoneName = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(zoneName, context, zoneId, 0);
    bool enable = GetData<bool>();
    server->EnableSystemVolumeProxy(zoneId, enable);
    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerGetAudioInterruptForZoneFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string zoneName = "zone";
    AudioZoneContext context;
    int32_t zoneId = 0;
    server->CreateAudioZone(zoneName, context, zoneId, 0);
    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    server->GetAudioInterruptForZone(zoneId, retList);
    std::string deviceTag = "dev";
    server->GetAudioInterruptForZone(zoneId, deviceTag, retList);
    server->ReleaseAudioZone(zoneId);
}

void AudioPolicyServerGetMaxAmplitudeFuzzTest()
{
    int32_t deviceId = GetData<int32_t>();
    float ret = GetData<float>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->GetMaxAmplitude(deviceId, ret);
}

void AudioPolicyServerIsHeadTrackingDataRequestedFuzzTest()
{
    std::string macAddress = "test";
    bool ret = GetData<bool>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->IsHeadTrackingDataRequested(macAddress, ret);
}

void AudioPolicyServerUnsetAudioDeviceRefinerCallbackFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->UnsetAudioDeviceRefinerCallback();
}

void AudioPolicyServerTriggerFetchDeviceFuzzTest()
{
    AudioStreamDeviceChangeReasonExt reason = GetData<AudioStreamDeviceChangeReason>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->TriggerFetchDevice(reason);
}

void AudioPolicyServerSetPreferredDeviceFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t preferredTypeIn = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(audioDeviceDescriptor != nullptr);
    int32_t uid = GetData<int32_t>();
    server->SetPreferredDevice(preferredTypeIn, audioDeviceDescriptor, uid);
}

void AudioPolicyServerSetDeviceVolumeBehaviorFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t deviceType = GetData<int32_t>();
    VolumeBehavior volumeBehavior;
    std::string networkId = "test";
    server->SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

void AudioPolicyServerUnsetAudioDeviceAnahsCallbackFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->UnsetAudioDeviceAnahsCallback();
}

void AudioPolicyServerSendVolumeKeyEventToRssWhenAccountsChangedFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SendVolumeKeyEventToRssWhenAccountsChanged();
}

void AudioPolicyServerNotifyAccountsChangedFuzzTest()
{
    const int32_t id = GetData<int32_t>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->NotifyAccountsChanged(id);
}

void AudioPolicyServerMoveToNewPipeFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    int32_t pipeType = GetData<int32_t>();
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->MoveToNewPipe(sessionId, pipeType);
}

void AudioPolicyServerCheckHibernateStateFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->CheckHibernateState(GetData<bool>());
}

void AudioPolicyServerUpdateSafeVolumeByS4FuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->UpdateSafeVolumeByS4();
}

void AudioPolicyServerCheckConnectedDeviceFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->CheckConnectedDevice();
}

void AudioPolicyServerSetDeviceConnectedFlagFalseAfterDurationFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->SetDeviceConnectedFlagFalseAfterDuration();
}

void AudioPolicyServerInjectInterruptionFuzzTest()
{
    InterruptEvent event;
    event.eventType = GetData<InterruptType>();
    event.forceType = GetData<InterruptForceType>();
    event.hintType = GetData<InterruptHint>();
    event.callbackToApp = GetData<bool>();
    std::string a = "test";
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->InjectInterruption(a, event);
}

void AudioPolicyServerProcessRemoteInterruptFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::set<int32_t> sessionIds;
    sessionIds.insert(GetData<int32_t>());
    InterruptEventInternal interruptEvent;
    uint32_t index = GetData<uint32_t>() % g_testInterruptHints.size();
    interruptEvent.hintType = g_testInterruptHints[index];
    server->ProcessRemoteInterrupt(sessionIds, interruptEvent);
}

void AudioPolicyServerGetStreamIdsForAudioSessionByStreamUsageFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t zoneId = GetData<int32_t>();
    std::set<StreamUsage> streamUsageSet = {STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC, STREAM_USAGE_AUDIOBOOK};
    server->GetStreamIdsForAudioSessionByStreamUsage(zoneId, streamUsageSet);
}

void AudioPolicyServerActivateAudioInterruptFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterruptIn;
    int32_t zoneID = GetData<int32_t>();
    bool isUpdatedAudioStrategy = GetData<bool>();
    server->ActivateAudioInterrupt(audioInterruptIn, zoneID, isUpdatedAudioStrategy);
}

void AudioPolicyServerDeactivateAudioInterruptFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterruptIn;
    int32_t zoneID = GetData<int32_t>();
    server->DeactivateAudioInterrupt(audioInterruptIn, zoneID);
}

void AudioPolicyServerActivatePreemptModeFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ActivatePreemptMode();
}

void AudioPolicyServerDeactivatePreemptModeFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->DeactivatePreemptMode();
}

void AudioPolicyServerGetStreamInFocusFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t zoneID = GetData<int32_t>();
    int32_t streamType = GetData<int32_t>();
    server->GetStreamInFocus(zoneID, streamType);
}

void AudioPolicyServerGetSessionInfoInFocusFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterrupt;
    int32_t zoneID = GetData<int32_t>();
    server->GetSessionInfoInFocus(audioInterrupt, zoneID);
}

void AudioPolicyServerGetAudioFocusInfoListFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioInterrupt audioInterrupt;
    int32_t audioFocuState = GetData<int32_t>();
    std::vector<std::map<AudioInterrupt, int32_t>> focusInfoList;
    std::map<AudioInterrupt, int32_t> interruptMap;
    interruptMap[audioInterrupt] = audioFocuState;
    focusInfoList.emplace_back(interruptMap);
    int32_t zoneID = GetData<int32_t>();
    server->GetAudioFocusInfoList(focusInfoList, zoneID);
}

void AudioPolicyServerVerifyPermissionFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string permissionName = "ohos.permission.MANAGE_AUDIO_POLICY";
    uint32_t tokenId = GetData<uint32_t>();
    bool isRecording = GetData<bool>();
    server->VerifyPermission(permissionName, tokenId, isRecording);
}

void AudioPolicyServerGetStreamVolumeInfoMapFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    StreamVolumeInfoMap streamVolumeInfos;
    server->GetStreamVolumeInfoMap(streamVolumeInfos);
}

void AudioPolicyServerDumpFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t fd = GetData<int32_t>();
    vector<std::u16string> testDumpArges = {
        u"-fb",
        u"test",
        u"test2",
        u"test3",
    };
    std::vector<std::u16string> args(testDumpArges.begin(), testDumpArges.begin() +
        (GetData<uint32_t>() % testDumpArges.size()));
    server->Dump(fd, args);
}

void AudioPolicyServerInitPolicyDumpMapFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->InitPolicyDumpMap();
}

void AudioPolicyServerPolicyDataDumpFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string dumpString = "dump_string";
    server->PolicyDataDump(dumpString);
}

void AudioPolicyServerArgInfoDumpFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string dumpString = "dump_string";
    std::queue<std::u16string> argQue;
    server->ArgInfoDump(dumpString, argQue);
}

void AudioPolicyServerInfoDumpHelpFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::string dumpString = "dump_string";
    server->InfoDumpHelp(dumpString);
}

void AudioPolicyServerGetPreferredOutputStreamTypeFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioRendererInfo rendererInfo;
    int32_t streamType = GetData<int32_t>();
    server->GetPreferredOutputStreamType(rendererInfo, streamType);
}

void AudioPolicyServerGetPreferredInputStreamTypeFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioCapturerInfo capturerInfo;
    int32_t streamType = GetData<int32_t>();
    server->GetPreferredInputStreamType(capturerInfo, streamType);
}

void AudioPolicyServerCreateRendererClientFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    uint32_t flag = GetData<uint32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    std::string networkId = "abc";
    server->CreateRendererClient(streamDesc, flag, sessionId, networkId);
}

void AudioPolicyServerCreateCapturerClientFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    uint32_t flag = GetData<uint32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    server->CreateCapturerClient(streamDesc, flag, sessionId);
}

void AudioPolicyServerRegisterTrackerFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t modeIn = GetData<int32_t>();
    AudioStreamChangeInfo streamChangeInfoIn;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN(samgr != nullptr);
    sptr<IRemoteObject> object = samgr->GetSystemAbility(SYSTEM_ABILITY_ID);
    server->RegisterTracker(modeIn, streamChangeInfoIn, object);
}

void AudioPolicyServerUpdateTrackerFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t modeIn = GetData<int32_t>();
    AudioStreamChangeInfo streamChangeInfoIn;
    server->UpdateTracker(modeIn, streamChangeInfoIn);
}

void AudioPolicyServerFetchInputDeviceForTrackFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    AudioStreamChangeInfo streamChangeInfoIn;
    server->FetchInputDeviceForTrack(streamChangeInfoIn);
}

void AudioPolicyServerGetCurrentRendererChangeInfosFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (rendererChangeInfo == nullptr) {
        return;
    }
    std::vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    audioRendererChangeInfos.push_back(rendererChangeInfo);
    server->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

void AudioPolicyServerGetCurrentCapturerChangeInfosFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    if (captureChangeInfo == nullptr) {
        return;
    }
    std::vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(captureChangeInfo);
    server->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

void AudioPolicyServerRegisteredTrackerClientDiedFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    pid_t pid = GetData<pid_t>();
    pid_t uid = GetData<pid_t>();
    server->RegisteredTrackerClientDied(pid, uid);
}

void AudioPolicyServerRegisteredStreamListenerClientDiedFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    pid_t pid = GetData<pid_t>();
    pid_t uid = GetData<pid_t>();
    server->RegisteredStreamListenerClientDied(pid, uid);
}

void AudioPolicyServerResumeStreamStateFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    server->ResumeStreamState();
}

void AudioPolicyServerGetNetworkIdByGroupIdFuzzTest()
{
    auto server = GetServerPtr();
    CHECK_AND_RETURN(server != nullptr);
    int32_t groupId = GetData<int32_t>();
    std::string networkId = "network_id";
    server->GetNetworkIdByGroupId(groupId, networkId);
}

void AudioPolicyServerOnAudioParameterChangeFuzzTest()
{
    std::string networkId = "network_id";
    uint32_t index = GetData<uint32_t>() % g_testAudioParamKeys.size();
    AudioParamKey key = g_testAudioParamKeys[index];
    std::string condition = "condition";
    std::string value = "value";
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    CHECK_AND_RETURN(callback != nullptr);
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

void AudioPolicyServerVolumeOnChangeFuzzTest()
{
    std::string networkId = "network_id";
    std::string condition = "condition";
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    CHECK_AND_RETURN(callback != nullptr);
    callback->VolumeOnChange(networkId, condition);
}

void AudioPolicyServerInterruptOnChangeFuzzTest()
{
    std::string networkId = "network_id";
    std::string condition = "condition";
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    CHECK_AND_RETURN(callback != nullptr);
    callback->InterruptOnChange(networkId, condition);
}

void AudioPolicyServerStateOnChangeFuzzTest()
{
    std::string networkId = "network_id";
    std::string condition = "condition";
    std::string value = "value";
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::RemoteParameterCallback>(server);
    CHECK_AND_RETURN(callback != nullptr);
    callback->StateOnChange(networkId, condition, value);
}

void AudioPolicyServerPermStateChangeCallbackFuzzTest()
{
    Security::AccessToken::PermStateChangeScope scopeInfo;
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::PerStateChangeCbCustomizeCallback>(scopeInfo, server);
    Security::AccessToken::PermStateChangeInfo result;
    CHECK_AND_RETURN(callback != nullptr);
    callback->PermStateChangeCallback(result);
}

void AudioPolicyServerUpdateMicPrivacyByCapturerStateFuzzTest()
{
    Security::AccessToken::PermStateChangeScope scopeInfo;
    sptr<AudioPolicyServer> server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto callback = std::make_shared<AudioPolicyServer::PerStateChangeCbCustomizeCallback>(scopeInfo, server);
    bool targetMuteState = GetData<bool>();
    uint32_t targetTokenId = GetData<uint32_t>();
    int32_t appUid = GetData<int32_t>();
    CHECK_AND_RETURN(callback != nullptr);
    callback->UpdateMicPrivacyByCapturerState(targetMuteState, targetTokenId, appUid);
}

TestFuncs g_testFuncs[] = {
    AudioPolicyServerRegisterDefaultVolumeTypeListenerFuzzTest,
    AudioPolicyServerOnAddSystemAbilityExtractFuzzTest,
    AudioPolicyServerOnRemoveSystemAbilityFuzzTest,
    AudioPolicyServerMaxOrMinVolumeOptionFuzzTest,
    AudioPolicyServerChangeVolumeOnVoiceAssistantFuzzTest,
    AudioPolicyServerIsContinueAddVolFuzzTest,
    AudioPolicyServerTriggerMuteCheckFuzzTest,
    AudioPolicyServerProcessVolumeKeyEventsFuzzTest,
    AudioPolicyServerSetVolumeInternalByKeyEventFuzzTest,
    AudioPolicyServerSubscribeSafeVolumeEventFuzzTest,
    AudioPolicyServerIsVolumeTypeValidFuzzTest,
    AudioPolicyServerIsVolumeLevelValidFuzzTest,
    AudioPolicyServerIsRingerModeValidFuzzTest,
    AudioPolicyServerSubscribeOsAccountChangeEventsFuzzTest,
    AudioPolicyServerAddRemoteDevstatusCallbackFuzzTest,
    AudioPolicyServerOnReceiveEventFuzzTest,
    AudioPolicyServerSubscribeBackgroundTaskFuzzTest,
    AudioPolicyServerSubscribeCommonEventExecuteFuzzTest,
    AudioPolicyServerCheckSubscribePowerStateChangeFuzzTest,
    AudioPolicyServerNotifySettingsDataReadyFuzzTest,
    AudioPolicyServerGetMaxVolumeLevelFuzzTest,
    AudioPolicyServerGetMinVolumeLevelFuzzTest,
    AudioPolicyServerSetSystemVolumeLevelLegacyFuzzTest,
    AudioPolicyServerSetAdjustVolumeForZoneFuzzTest,
    AudioPolicyServerGetSelfAppVolumeLevelFuzzTest,
    AudioPolicyServerGetSystemVolumeLevelFuzzTest,
    AudioPolicyServerGetSystemVolumeLevelNoMuteStateFuzzTest,
    AudioPolicyServerGetSystemVolumeLevelInternalFuzzTest,
    AudioPolicyServerGetAppVolumeLevelInternalFuzzTest,
    AudioPolicyServerSetLowPowerVolumeFuzzTest,
    AudioPolicyServerGetFastStreamInfoFuzzTest,
    AudioPolicyServerGetLowPowerVolumeFuzzTest,
    AudioPolicyServerGetSingleStreamVolumeFuzzTest,
    AudioPolicyServerIsVolumeUnadjustableFuzzTest,
    AudioPolicyServerCheckCanMuteVolumeTypeByStepFuzzTest,
    AudioPolicyServerGetSystemVolumeInDbFuzzTest,
    AudioPolicyServerSetStreamMuteLegacyFuzzTest,
    AudioPolicyServerSetStreamMuteInternalFuzzTest,
    AudioPolicyServerUpdateSystemMuteStateAccordingMusicStateFuzzTest,
    AudioPolicyServerSendMuteKeyEventCbWithUpdateUiOrNotFuzzTest,
    AudioPolicyServerSetSingleStreamMuteFuzzTest,
    AudioPolicyServerProcUpdateRingerModeForMuteFuzzTest,
    AudioPolicyServerGetSystemVolumeDbFuzzTest,
    AudioPolicyServerSetSelfAppVolumeLevelFuzztest,
    AudioPolicyServerSetAppVolumeLevelInternalFuzztest,
    AudioPolicyServerSetSystemVolumeLevelInternalFuzztest,
    AudioPolicyServerSetSystemVolumeLevelWithDeviceInternalFuzztest,
    AudioPolicyServerSendVolumeKeyEventCbWithUpdateUiOrNotFuzztest,
    AudioPolicyServerUpdateMuteStateAccordingToVolLevelFuzztest,
    AudioPolicyServerProcUpdateRingerModeFuzztest,
    AudioPolicyServerSetAppSingleStreamVolumeFuzztest,
    AudioPolicyServerSetSingleStreamVolumeFuzztest,
    AudioPolicyServerSetSingleStreamVolumeWithDeviceFuzztest,
    AudioPolicyServerGetStreamMuteFuzztest,
    AudioPolicyServerMapExternalToInternalDeviceTypeFuzztest,
    AudioPolicyServerGetPreferredInputDeviceDescriptorsFuzztest,
    AudioPolicyServerIsFastRecordingSupportedFuzztest,
    AudioPolicyServerSetRingerModeInternalFuzztest,
    AudioPolicyServerInitMicrophoneMuteFuzztest,
    AudioPolicyServerSetMicrophoneMuteAudioConfigFuzztest,
    AudioPolicyServerSetMicrophoneMutePersistentFuzztest,
    AudioPolicyServerSetAudioSceneFuzztest,
    AudioPolicyServerSetAndUnsetAudioInterruptCallbackFuzztest,
    AudioPolicyServerVerifySessionIdFuzztest,
    AudioPolicyServerSetAndUnsetAudioRouteCallbackFuzztest,
    AudioPolicyServerSubscribeAccessibilityConfigObserverFuzzTest,
    AudioPolicyServerGetMinStreamVolumeFuzzTest,
    AudioPolicyServerGetMaxStreamVolumeFuzzTest,
    AudioPolicyServerGetMaxRendererInstancesFuzzTest,
    AudioPolicyServerRegisterDataObserverFuzzTest,
    AudioPolicyServerQueryEffectSceneModeFuzzTest,
    AudioPolicyServerGetHardwareOutputSamplingRateFuzzTest,
    AudioPolicyServerGetAudioCapturerMicrophoneDescriptorsFuzzTest,
    AudioPolicyServerGetAvailableMicrophonesFuzzTest,
    AudioPolicyServerSetDeviceAbsVolumeSupportedFuzzTest,
    AudioPolicyServerIsAbsVolumeSceneFuzzTest,
    AudioPolicyServerSetA2dpDeviceVolumeFuzzTest,
    AudioPolicyServerSetNearlinkDeviceVolumeFuzzTest,
    AudioPolicyServerGetAvailableDevicesFuzzTest,
    AudioPolicyServerSetAvailableDeviceChangeCallbackFuzzTest,
    AudioPolicyServerUnsetAvailableDeviceChangeCallbackFuzzTest,
    AudioPolicyServerOffloadStopPlayingFuzzTest,
    AudioPolicyServerCheckAudioSessionStrategyFuzzTest,
    AudioPolicyServerSetAudioSessionSceneFuzzTest,
    AudioPolicyServerGetDefaultOutputDeviceFuzzTest,
    AudioPolicyServerSetDefaultOutputDeviceFuzzTest,
    AudioPolicyServerLoadSplitModuleFuzzTest,
    AudioPolicyServerIsAllowedPlaybackFuzzTest,
    AudioPolicyServerSetVoiceRingtoneMuteFuzzTest,
    AudioPolicyServerNotifySessionStateChangeFuzzTest,
    AudioPolicyServerNotifyFreezeStateChangeFuzzTest,
    AudioPolicyServerResetAllProxyFuzzTest,
    AudioPolicyServerNotifyProcessBackgroundStateFuzzTest,
    AudioPolicyServerSetVirtualCallFuzzTest,
    AudioPolicyServerSetDeviceConnectionStatusFuzzTest,
    AudioPolicyServerSetQueryAllowedPlaybackCallbackFuzzTest,
    AudioPolicyServerSetBackgroundMuteCallbackFuzzTest,
    AudioPolicyServerGetDirectPlaybackSupportFuzzTest,
    AudioPolicyServerGetMaxVolumeLevelByUsageFuzzTest,
    AudioPolicyServerGetMinVolumeLevelByUsageFuzzTest,
    AudioPolicyServerGetVolumeLevelByUsageFuzzTest,
    AudioPolicyServerGetStreamMuteByUsageFuzzTest,
    AudioPolicyServerGetVolumeInDbByStreamFuzzTest,
    AudioPolicyServerGetSupportedAudioVolumeTypesFuzzTest,
    AudioPolicyServerGetAudioVolumeTypeByStreamUsageFuzzTest,
    AudioPolicyServerGetStreamUsagesByVolumeTypeFuzzTest,
    AudioPolicyServerSetCallbackStreamUsageInfoFuzzTest,
    AudioPolicyServerForceStopAudioStreamFuzzTest,
    AudioPolicyServerIsCapturerFocusAvailableFuzzTest,
    AudioPolicyServerUpdateDefaultOutputDeviceWhenStartingFuzzTest,
    AudioPolicyServerUpdateDefaultOutputDeviceWhenStoppingFuzzTest,
    AudioPolicyServerIsAcousticEchoCancelerSupportedFuzzTest,
    AudioPolicyServerSetKaraokeParametersFuzzTest,
    AudioPolicyServerUpdateDeviceInfoFuzzTest,
    AudioPolicyServerSetSleAudioOperationCallbackFuzzTest,
    AudioPolicyServerSetCollaborativePlaybackEnabledForDeviceFuzzTest,
    AudioPolicyServerIsCollaborativePlaybackSupportedFuzzTest,
    AudioPolicyServerIsCollaborativePlaybackEnabledForDeviceFuzzTest,
    AudioPolicyServerCallRingtoneLibraryFuzzTest,
    SetAndUnsetDistributedRoutingRoleCallbackFuzzTest,
    OnDistributedRoutingRoleChangeFuzzTest,
    RegisterAndUnRegisterPowerStateListenerFuzzTest,
    RegisterAppStateListenerFuzzTest,
    RegisterAndUnRegisterSyncHibernateListenerFuzzTest,
    AudioPolicyServerRegisterSpatializationStateEventListenerFuzzTest,
    AudioPolicyServerAudioInterruptZoneFuzzTest,
    AudioPolicyServerRegisterAudioZoneClientFuzzTest,
    AudioPolicyServerAudioZoneQueryFuzzTest,
    AudioPolicyServerBindUnbindDeviceToAudioZoneFuzzTest,
    AudioPolicyServerEnableAudioZoneReportFuzzTest,
    AudioPolicyServerAddRemoveUidToAudioZoneFuzzTest,
    AudioPolicyServerAddStreamToAudioZoneFuzzTest,
    AudioPolicyServerAddStreamsToAudioZoneFuzzTest,
    AudioPolicyServerSetZoneDeviceVisibleFuzzTest,
    AudioPolicyServerEnableSystemVolumeProxyFuzzTest,
    AudioPolicyServerGetAudioInterruptForZoneFuzzTest,
    AudioPolicyServerTriggerFetchDeviceFuzzTest,
    AudioPolicyServerUnsetAudioDeviceRefinerCallbackFuzzTest,
    AudioPolicyServerIsHeadTrackingDataRequestedFuzzTest,
    AudioPolicyServerGetMaxAmplitudeFuzzTest,
    AudioPolicyServerSetPreferredDeviceFuzzTest,
    AudioPolicyServerSetDeviceVolumeBehaviorFuzzTest,
    AudioPolicyServerUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioPolicyServerSendVolumeKeyEventToRssWhenAccountsChangedFuzzTest,
    AudioPolicyServerNotifyAccountsChangedFuzzTest,
    AudioPolicyServerMoveToNewPipeFuzzTest,
    AudioPolicyServerCheckHibernateStateFuzzTest,
    AudioPolicyServerUpdateSafeVolumeByS4FuzzTest,
    AudioPolicyServerCheckConnectedDeviceFuzzTest,
    AudioPolicyServerSetDeviceConnectedFlagFalseAfterDurationFuzzTest,
    AudioPolicyServerInjectInterruptionFuzzTest,
    AudioPolicyServerProcessRemoteInterruptFuzzTest,
    AudioPolicyServerGetStreamIdsForAudioSessionByStreamUsageFuzzTest,
    AudioPolicyServerActivateAudioInterruptFuzzTest,
    AudioPolicyServerDeactivateAudioInterruptFuzzTest,
    AudioPolicyServerActivatePreemptModeFuzzTest,
    AudioPolicyServerDeactivatePreemptModeFuzzTest,
    AudioPolicyServerGetStreamInFocusFuzzTest,
    AudioPolicyServerGetSessionInfoInFocusFuzzTest,
    AudioPolicyServerGetAudioFocusInfoListFuzzTest,
    AudioPolicyServerVerifyPermissionFuzzTest,
    AudioPolicyServerGetStreamVolumeInfoMapFuzzTest,
    AudioPolicyServerDumpFuzzTest,
    AudioPolicyServerInitPolicyDumpMapFuzzTest,
    AudioPolicyServerPolicyDataDumpFuzzTest,
    AudioPolicyServerArgInfoDumpFuzzTest,
    AudioPolicyServerInfoDumpHelpFuzzTest,
    AudioPolicyServerGetPreferredOutputStreamTypeFuzzTest,
    AudioPolicyServerGetPreferredInputStreamTypeFuzzTest,
    AudioPolicyServerCreateRendererClientFuzzTest,
    AudioPolicyServerCreateCapturerClientFuzzTest,
    AudioPolicyServerRegisterTrackerFuzzTest,
    AudioPolicyServerUpdateTrackerFuzzTest,
    AudioPolicyServerFetchInputDeviceForTrackFuzzTest,
    AudioPolicyServerGetCurrentRendererChangeInfosFuzzTest,
    AudioPolicyServerGetCurrentCapturerChangeInfosFuzzTest,
    AudioPolicyServerRegisteredTrackerClientDiedFuzzTest,
    AudioPolicyServerRegisteredStreamListenerClientDiedFuzzTest,
    AudioPolicyServerResumeStreamStateFuzzTest,
    AudioPolicyServerGetNetworkIdByGroupIdFuzzTest,
    AudioPolicyServerOnAudioParameterChangeFuzzTest,
    AudioPolicyServerVolumeOnChangeFuzzTest,
    AudioPolicyServerInterruptOnChangeFuzzTest,
    AudioPolicyServerStateOnChangeFuzzTest,
    AudioPolicyServerUpdateMicPrivacyByCapturerStateFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
