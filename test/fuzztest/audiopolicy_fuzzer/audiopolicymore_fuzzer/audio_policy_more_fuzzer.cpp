/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#undef private
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_interrupt_service.h"
#include "audio_socket_thread.h"
#include "audio_pnp_server.h"
#include "audio_input_thread.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
bool g_hasPnpServerInit = false;
bool g_hasServerInit = false;
bool g_hasPermission = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const bool RUN_ON_CREATE = false;
const int32_t SYSTEM_ABILITY_ID = 3009;
const string DEFAULTNAME = "name";
const string DEFAULTADDRESS = "address";
const string DEFAULTINFO = "EVENT_NAME=name;DEVICE_ADDRESS=address";
const ssize_t DEFAULTSTRLENGTH = 2;
const size_t THRESHOLD = 10;

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

#ifdef AUDIO_WIRED_DETECT
AudioPnpServer* GetPnpServerPtr()
{
    static AudioPnpServer pnpServer;
    if (!g_hasPnpServerInit) {
        pnpServer.init();
        g_hasPnpServerInit = true;
    }
    return &pnpServer;
}
#endif

void InitFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->Init(server);
}

void GetHighestPriorityAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetHighestPriorityAudioScene(zoneId);
}

void AudioInterruptZoneDumpFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::string dumpString = "";
    if (interruptService == nullptr) {
        return;
    }
    interruptService->AudioInterruptZoneDump(dumpString);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int zoneId = fdp.ConsumeIntegral<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->ClearAudioFocusInfoListOnAccountsChanged(zoneId, 1);
}

void GetStreamTypePriorityFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    OHOS::AudioStandard::AudioStreamType streamType = ConsumeEnum<AudioStreamType>(fdp);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetStreamTypePriority(streamType);
}

void SendInterruptEventFuzzTest(FuzzedDataProvider& fdp)
{
    AudioFocuState oldState = ConsumeEnum<AudioFocuState>(fdp);
    AudioFocuState newState = ConsumeEnum<AudioFocuState>(fdp);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    focusInfo.first.contentType = ConsumeEnum<ContentType>(fdp);
    focusInfo.first.audioFocusType.streamType = ConsumeEnum<AudioStreamType>(fdp);
    focusInfo.first.audioFocusType.sourceType = ConsumeEnum<SourceType>(fdp);
    focusInfo.first.audioFocusType.isPlay = fdp.ConsumeBool();
    focusInfo.first.sessionId = fdp.ConsumeIntegral<int32_t>();
    focusInfo.first.pauseWhenDucked = fdp.ConsumeBool();
    focusInfo.first.pid = fdp.ConsumeIntegral<int32_t>();
    focusInfo.first.mode = ConsumeEnum<InterruptMode>(fdp);
    focusInfo.second = ConsumeEnum<AudioFocuState>(fdp);
    focusInfoList.push_back(focusInfo);
    auto it = focusInfoList.begin();
    if (interruptService == nullptr) {
        return;
    }
    bool removeFocusInfo = fdp.ConsumeBool();
    interruptService->SendInterruptEvent(oldState, newState, it, removeFocusInfo);
}

void IsSameAppInShareModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activateInterrupt;
    incomingInterrupt.contentType = ConsumeEnum<ContentType>(fdp);
    incomingInterrupt.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    incomingInterrupt.audioFocusType.streamType = ConsumeEnum<AudioStreamType>(fdp);
    activateInterrupt.contentType = ConsumeEnum<ContentType>(fdp);
    activateInterrupt.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    activateInterrupt.audioFocusType.streamType = ConsumeEnum<AudioStreamType>(fdp);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->IsSameAppInShareMode(incomingInterrupt, activateInterrupt);
}

void SendFocusChangeEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt audioInterrupt;
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    int32_t callbackCategory = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.contentType = ConsumeEnum<ContentType>(fdp);
    audioInterrupt.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    audioInterrupt.audioFocusType.streamType = ConsumeEnum<AudioStreamType>(fdp);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
}

void GetAudioFocusInfoListFuzzTest(FuzzedDataProvider& fdp)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    focusInfo.first.contentType = ConsumeEnum<ContentType>(fdp);
    focusInfo.first.audioFocusType.streamType = ConsumeEnum<AudioStreamType>(fdp);
    focusInfo.first.audioFocusType.sourceType = ConsumeEnum<SourceType>(fdp);
    focusInfo.first.audioFocusType.isPlay = fdp.ConsumeBool();
    focusInfo.first.sessionId = fdp.ConsumeIntegral<int32_t>();
    focusInfo.first.pauseWhenDucked = fdp.ConsumeBool();
    focusInfo.first.pid = fdp.ConsumeIntegral<int32_t>();
    focusInfo.first.mode = ConsumeEnum<InterruptMode>(fdp);
    focusInfo.second = ConsumeEnum<AudioFocuState>(fdp);
    focusInfoList.push_back(focusInfo);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetAudioFocusInfoList(zoneId, focusInfoList);
}

void AudioVolumeMoreFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamType streamType = ConsumeEnum<AudioStreamType>(fdp);
    VolumeAdjustType adjustType = ConsumeEnum<VolumeAdjustType>(fdp);
    int32_t volume = fdp.ConsumeIntegral<int32_t>();
    int32_t streamId = fdp.ConsumeIntegral<int32_t>();
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();

    bool mute = fdp.ConsumeBool();
    GetServerPtr()->SetSystemVolumeLevel(streamType, volume);
    GetServerPtr()->GetSystemVolumeLevel(streamType);
    GetServerPtr()->SetLowPowerVolume(streamId, volume);
    GetServerPtr()->GetLowPowerVolume(streamId);
    GetServerPtr()->GetSingleStreamVolume(streamId);
    GetServerPtr()->SetStreamMute(streamType, mute);
    GetServerPtr()->GetStreamMute(streamType);
    GetServerPtr()->IsStreamActive(streamType);
    GetServerPtr()->GetMaxVolumeLevel(streamType);
    GetServerPtr()->GetMinVolumeLevel(streamType);
    GetServerPtr()->SetSystemVolumeLevelLegacy(streamType, volume);
    GetServerPtr()->IsVolumeUnadjustable();
    GetServerPtr()->AdjustVolumeByStep(adjustType);
    GetServerPtr()->AdjustSystemVolumeByStep(streamType, adjustType);
    GetServerPtr()->GetSystemVolumeInDb(streamType, volume, deviceType);
    GetServerPtr()->GetSelectedDeviceInfo(uid, pid, streamType);
}

void AudioDeviceMoreFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag flag = ConsumeEnum<DeviceFlag>(fdp);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    size_t bufferSize = fdp.ConsumeIntegralInRange<size_t>(0,
        fdp.remaining_bytes() > 64 ? 64 : fdp.remaining_bytes());
    std::vector<uint8_t> parcelBuffer = fdp.ConsumeBytes<uint8_t>(bufferSize);
    if (!parcelBuffer.empty()) {
        data.WriteBuffer(parcelBuffer.data(), parcelBuffer.size());
    }
    data.RewindRead(0);
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    int32_t capturerFlags = fdp.ConsumeIntegral<int32_t>();
    AudioCapturerInfo capturerInfo = {sourceType, capturerFlags};
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    audioStreamInfo.channels = ConsumeEnum<AudioChannel>(fdp);
    audioStreamInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    audioStreamInfo.encoding = ConsumeEnum<AudioEncodingType>(fdp);
    InternalDeviceType deviceType = ConsumeEnum<InternalDeviceType>(fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool active = fdp.ConsumeBool();
    GetServerPtr()->SetDeviceActive(deviceType, active);
    GetServerPtr()->IsDeviceActive(deviceType);
    GetServerPtr()->GetDevices(flag);
    GetServerPtr()->GetDevicesInner(flag);
    AudioRingerMode ringMode = ConsumeEnum<AudioRingerMode>(fdp);
    GetServerPtr()->SetRingerMode(ringMode);
    bool mute = fdp.ConsumeBool();
    bool legacy = fdp.ConsumeBool();
    GetServerPtr()->SetMicrophoneMute(mute);
    GetServerPtr()->SetMicrophoneMuteCommon(mute, legacy);
    GetServerPtr()->SetMicrophoneMuteAudioConfig(mute);

    PolicyType type = ConsumeEnum<PolicyType>(fdp);
    GetServerPtr()->SetMicrophoneMutePersistent(mute, type);
    GetServerPtr()->GetPersistentMicMuteState();
    GetServerPtr()->IsMicrophoneMuteLegacy();
    GetServerPtr()->GetAudioScene();

    StreamUsage streamUsage = ConsumeEnum<StreamUsage>(fdp);
    GetServerPtr()->GetDirectPlaybackSupport(audioStreamInfo, streamUsage);
}

void AudioPolicyOtherMoreFuzzTest(FuzzedDataProvider& fdp)
{
    int pid = fdp.ConsumeIntegral<int>();
    GetServerPtr()->RegisteredTrackerClientDied(pid, 0);

    int32_t clientUid = fdp.ConsumeIntegral<int32_t>();
    StreamSetState streamSetState = ConsumeEnum<StreamSetState>(fdp);
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    GetServerPtr()->UpdateStreamState(clientUid, streamSetState, streamUsage);
    GetServerPtr()->IsHighResolutionExist();
    bool highResExist = fdp.ConsumeBool();
    GetServerPtr()->SetHighResolutionExist(highResExist);
}

void AudioVolumeKeyCallbackStubMoreFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<AudioPolicyClientStub> listener =
        static_cast<sptr<AudioPolicyClientStub>>(new(std::nothrow) AudioPolicyClientStubImpl());
    VolumeEvent volumeEvent = {};
    volumeEvent.volumeType = ConsumeEnum<AudioStreamType>(fdp);
    volumeEvent.volume = fdp.ConsumeIntegral<int32_t>();
    volumeEvent.updateUi = fdp.ConsumeBool();
    volumeEvent.volumeGroupId = fdp.ConsumeIntegral<int32_t>();
    std::string id = "123";
    volumeEvent.networkId = id;

    MessageParcel data;
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT));
    data.WriteInt32(static_cast<int32_t>(volumeEvent.volumeType));
    data.WriteInt32(volumeEvent.volume);
    data.WriteBool(volumeEvent.updateUi);
    data.WriteInt32(volumeEvent.volumeGroupId);
    data.WriteString(volumeEvent.networkId);
    MessageParcel reply;
    MessageOption option;
    listener->OnRemoteRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
}

void AudioPolicyManagerFuzzTest(FuzzedDataProvider& fdp)
{
#ifdef AUDIO_WIRED_DETECT
    AudioEvent audioEvent;
    uint32_t eventType = fdp.ConsumeIntegral<uint32_t>();
    uint32_t deviceType = fdp.ConsumeIntegral<uint32_t>();
    audioEvent.eventType = eventType;
    audioEvent.deviceType = deviceType;
    audioEvent.name = DEFAULTNAME;
    audioEvent.address = DEFAULTADDRESS;
    int fd = fdp.ConsumeIntegral<int>();
    ssize_t strLength = DEFAULTSTRLENGTH;
    const char *msg = "SCENE";
    AudioSocketThread::IsUpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::UpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::AudioPnpUeventOpen(&fd);
    AudioSocketThread::UpdateDeviceState(audioEvent);
    AudioSocketThread::DetectAnalogHeadsetState(&audioEvent);
    AudioSocketThread::SetAudioPnpUevent(&audioEvent);
    AudioSocketThread::AudioPnpUeventParse(msg, strLength);
    AudioInputThread::AudioPnpInputOpen();

    GetPnpServerPtr()->GetAudioPnpServer();
    GetPnpServerPtr()->UnRegisterPnpStatusListener();
    GetPnpServerPtr()->OnPnpDeviceStatusChanged(DEFAULTINFO);
#endif
}

void ForceVolumeKeyControlTypeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t volumeType = fdp.ConsumeIntegral<int32_t>();
    int32_t duration = fdp.ConsumeIntegral<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->ForceVolumeKeyControlType(static_cast<AudioVolumeType>(volumeType), duration);
}

typedef void (*TestFuncs[16])();

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    InitFuzzTest,
    GetHighestPriorityAudioSceneFuzzTest,
    AudioInterruptZoneDumpFuzzTest,
    ClearAudioFocusInfoListOnAccountsChangedFuzzTest,
    GetStreamTypePriorityFuzzTest,
    SendInterruptEventFuzzTest,
    IsSameAppInShareModeFuzzTest,
    GetAudioFocusInfoListFuzzTest,
    AudioVolumeMoreFuzzTest,
    AudioDeviceMoreFuzzTest,
    AudioPolicyOtherMoreFuzzTest,
    AudioVolumeKeyCallbackStubMoreFuzzTest,
    AudioPolicyManagerFuzzTest,
    ForceVolumeKeyControlTypeFuzzTest,
    });
    func(fdp);
}
void Init()
{
    manager = std::make_shared<AudioA2dpOffloadManager>();
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
