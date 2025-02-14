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
#include <cstring>
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
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
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
        server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
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

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

static AudioRendererInfo getAudioRenderInfo()
{
    ContentType contentType = GetData<ContentType>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    int32_t rendererFlags = GetData<int32_t>();
    std::string sceneType = "SCENE_MOVIE";
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    int32_t originalFlag = GetData<int32_t>();
    AudioRendererInfo rendererInfo = {
        contentType,
        streamUsage,
        rendererFlags,
        sceneType,
        spatializationEnabled,
        headTrackingEnabled,
        originalFlag
    };
    return rendererInfo;
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

void InitFuzzTest()
{
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->Init(server);
}

void GetHighestPriorityAudioSceneFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = GetData<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetHighestPriorityAudioScene(zoneId);
}

void AudioInterruptZoneDumpFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::string dumpString = "";
    if (interruptService == nullptr) {
        return;
    }
    interruptService->AudioInterruptZoneDump(dumpString);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int zoneId = GetData<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->ClearAudioFocusInfoListOnAccountsChanged(zoneId);
}

void GetStreamTypePriorityFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    OHOS::AudioStandard::AudioStreamType streamType = GetData<AudioStreamType>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetStreamTypePriority(streamType);
}

void GetStreamPriorityMapFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetStreamPriorityMap();
}

void SendInterruptEventFuzzTest()
{
    AudioFocuState oldState = GetData<AudioFocuState>();
    AudioFocuState newState = GetData<AudioFocuState>();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = GetData<StreamUsage>();
    focusInfo.first.contentType = GetData<ContentType>();
    focusInfo.first.audioFocusType.streamType = GetData<AudioStreamType>();
    focusInfo.first.audioFocusType.sourceType = GetData<SourceType>();
    focusInfo.first.audioFocusType.isPlay = GetData<bool>();
    focusInfo.first.sessionId = GetData<int32_t>();
    focusInfo.first.pauseWhenDucked = GetData<bool>();
    focusInfo.first.pid = GetData<int32_t>();
    focusInfo.first.mode = GetData<InterruptMode>();
    focusInfo.second = GetData<AudioFocuState>();
    focusInfoList.push_back(focusInfo);
    auto it = focusInfoList.begin();
    if (interruptService == nullptr) {
        return;
    }
    bool removeFocusInfo = GetData<bool>();
    interruptService->SendInterruptEvent(oldState, newState, it, removeFocusInfo);
}

void IsSameAppInShareModeFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt, activateInterrupt;
    incomingInterrupt.contentType = GetData<ContentType>();
    incomingInterrupt.streamUsage = GetData<StreamUsage>();
    incomingInterrupt.audioFocusType.streamType = GetData<AudioStreamType>();
    activateInterrupt.contentType = GetData<ContentType>();
    activateInterrupt.streamUsage = GetData<StreamUsage>();
    activateInterrupt.audioFocusType.streamType = GetData<AudioStreamType>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->IsSameAppInShareMode(incomingInterrupt, activateInterrupt);
}

void SendFocusChangeEventFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt audioInterrupt;
    int32_t zoneId = GetData<int32_t>();
    int32_t callbackCategory = GetData<int32_t>();
    audioInterrupt.contentType = GetData<ContentType>();
    audioInterrupt.streamUsage = GetData<StreamUsage>();
    audioInterrupt.audioFocusType.streamType = GetData<AudioStreamType>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
}

void GetAudioFocusInfoListFuzzTest()
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = GetData<StreamUsage>();
    focusInfo.first.contentType = GetData<ContentType>();
    focusInfo.first.audioFocusType.streamType = GetData<AudioStreamType>();
    focusInfo.first.audioFocusType.sourceType = GetData<SourceType>();
    focusInfo.first.audioFocusType.isPlay = GetData<bool>();
    focusInfo.first.sessionId = GetData<int32_t>();
    focusInfo.first.pauseWhenDucked = GetData<bool>();
    focusInfo.first.pid = GetData<int32_t>();
    focusInfo.first.mode = GetData<InterruptMode>();
    focusInfo.second = GetData<AudioFocuState>();
    focusInfoList.push_back(focusInfo);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = GetData<int32_t>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetAudioFocusInfoList(zoneId, focusInfoList);
}

void AudioVolumeMoreFuzzTest()
{
    AudioStreamType streamType = GetData<AudioStreamType>();
    VolumeAdjustType adjustType = GetData<VolumeAdjustType>();
    int32_t volume = GetData<int32_t>();
    int32_t streamId = GetData<int32_t>();
    DeviceType deviceType = GetData<DeviceType>();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();

    bool mute = GetData<bool>();
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

    AudioRendererInfo rendererInfo = getAudioRenderInfo();
    GetServerPtr()->GetPreferredOutputStreamType(rendererInfo);

    SourceType sourceType = GetData<SourceType>();
    int32_t capturerFlags = GetData<int32_t>();
    AudioCapturerInfo capturerInfo = {
        sourceType,
        capturerFlags
    };
    GetServerPtr()->GetPreferredInputStreamType(capturerInfo);
}

void AudioDeviceMoreFuzzTest()
{
    DeviceFlag flag = GetData<DeviceFlag>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    SourceType sourceType = GetData<SourceType>();
    int32_t capturerFlags = GetData<int32_t>();
    AudioCapturerInfo capturerInfo = {sourceType, capturerFlags};
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = GetData<AudioSamplingRate>();
    audioStreamInfo.channels = GetData<AudioChannel>();
    audioStreamInfo.format = GetData<AudioSampleFormat>();
    audioStreamInfo.encoding = GetData<AudioEncodingType>();
    InternalDeviceType deviceType = GetData<InternalDeviceType>();
    uint32_t sessionId = GetData<uint32_t>();
    bool active = GetData<bool>();
    GetServerPtr()->SetDeviceActive(deviceType, active);
    GetServerPtr()->IsDeviceActive(deviceType);
    GetServerPtr()->GetDevices(flag);
    GetServerPtr()->GetDevicesInner(flag);
    AudioRingerMode ringMode = GetData<AudioRingerMode>();
    GetServerPtr()->SetRingerMode(ringMode);
    bool mute = GetData<bool>();
    bool legacy = GetData<bool>();
    GetServerPtr()->SetMicrophoneMute(mute);
    GetServerPtr()->SetMicrophoneMuteCommon(mute, legacy);
    GetServerPtr()->SetMicrophoneMuteAudioConfig(mute);

    PolicyType type = GetData<PolicyType>();
    GetServerPtr()->SetMicrophoneMutePersistent(mute, type);
    GetServerPtr()->GetPersistentMicMuteState();
    GetServerPtr()->IsMicrophoneMuteLegacy();
    GetServerPtr()->GetAudioScene();
}

void AudioPolicySomeMoreFuzzTest()
{
    uint32_t sessionID = GetData<uint32_t>();
    GetServerPtr()->OnAudioStreamRemoved(sessionID);
    GetServerPtr()->ProcessSessionRemoved(sessionID);
    GetServerPtr()->ProcessorCloseWakeupSource(sessionID);
}

void AudioPolicyOtherMoreFuzzTest()
{
    int pid = GetData<int>();
    GetServerPtr()->RegisteredTrackerClientDied(pid, 0);

    int32_t clientUid = GetData<int32_t>();
    StreamSetState streamSetState = GetData<StreamSetState>();
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    GetServerPtr()->UpdateStreamState(clientUid, streamSetState, streamUsage);
    GetServerPtr()->IsHighResolutionExist();
    bool highResExist = GetData<bool>();
    GetServerPtr()->SetHighResolutionExist(highResExist);
}

void AudioVolumeKeyCallbackStubMoreFuzzTest()
{
    sptr<AudioPolicyClientStub> listener =
        static_cast<sptr<AudioPolicyClientStub>>(new(std::nothrow) AudioPolicyClientStubImpl());
    VolumeEvent volumeEvent = {};
    volumeEvent.volumeType =  GetData<AudioStreamType>();
    volumeEvent.volume = GetData<int32_t>();
    volumeEvent.updateUi = GetData<bool>();
    volumeEvent.volumeGroupId = GetData<int32_t>();
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

void AudioPolicyManagerFuzzTest()
{
#ifdef AUDIO_WIRED_DETECT
    AudioEvent audioEvent;
    uint32_t eventType = GetData<uint32_t>();
    uint32_t deviceType = GetData<uint32_t>();
    audioEvent.eventType = eventType;
    audioEvent.deviceType = deviceType;
    audioEvent.name = DEFAULTNAME;
    audioEvent.address = DEFAULTADDRESS;
    int fd = GetData<int>();
    ssize_t strLength = DEFAULTSTRLENGTH;
    const char *msg = "SCENE";
    AudioSocketThread::IsUpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::UpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::AudioPnpUeventOpen(&fd);
    AudioSocketThread::UpdateDeviceState(audioEvent);
    AudioSocketThread::DetectAnalogHeadsetState(&audioEvent);
    AudioSocketThread::AudioPnpUeventParse(msg, strLength);
    AudioInputThread::AudioPnpInputOpen();

    GetPnpServerPtr()->GetAudioPnpServer();
    GetPnpServerPtr()->UnRegisterPnpStatusListener();
    GetPnpServerPtr()->OnPnpDeviceStatusChanged(DEFAULTINFO);
    AudioInputThread::AudioPnpInputPollAndRead();
#endif
}

typedef void (*TestFuncs[15])();

TestFuncs g_testFuncs = {
    InitFuzzTest,
    GetHighestPriorityAudioSceneFuzzTest,
    AudioInterruptZoneDumpFuzzTest,
    ClearAudioFocusInfoListOnAccountsChangedFuzzTest,
    GetStreamTypePriorityFuzzTest,
    GetStreamPriorityMapFuzzTest,
    SendInterruptEventFuzzTest,
    IsSameAppInShareModeFuzzTest,
    GetAudioFocusInfoListFuzzTest,
    AudioVolumeMoreFuzzTest,
    AudioDeviceMoreFuzzTest,
    AudioPolicySomeMoreFuzzTest,
    AudioPolicyOtherMoreFuzzTest,
    AudioVolumeKeyCallbackStubMoreFuzzTest,
    AudioPolicyManagerFuzzTest,
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

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
