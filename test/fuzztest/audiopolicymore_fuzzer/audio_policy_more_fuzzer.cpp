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
string DEFAULTNAME = "name";
string DEFAULTADDRESS = "address";
string DEFAULTINFO = "EVENT_NAME=name;DEVICE_ADDRESS=address";
const ssize_t DEFAULTSTRLENGTH = 2;

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

static AudioRendererInfo getAudioRenderInfo(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return {};
    }
    ContentType contentType = *reinterpret_cast<const ContentType *>(rawData);
    StreamUsage streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    int32_t rendererFlags = *reinterpret_cast<const int32_t *>(rawData);
    std::string sceneType = "SCENE_MOVIE";
    bool spatializationEnabled = *reinterpret_cast<const bool *>(rawData);
    bool headTrackingEnabled = *reinterpret_cast<const bool *>(rawData);
    int32_t originalFlag = *reinterpret_cast<const int32_t *>(rawData);
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

AudioPnpServer* GetPnpServerPtr()
{
    static AudioPnpServer pnpServer;
    if (!g_hasPnpServerInit) {
        pnpServer.init();
        g_hasPnpServerInit = true;
    }
    return &pnpServer;
}

void InitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->Init(server);
}

void GetHighestPriorityAudioSceneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetHighestPriorityAudioScene(zoneId);
}

void AudioInterruptZoneDumpFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::string dumpString = "";
    if (interruptService == nullptr) {
        return;
    }
    interruptService->AudioInterruptZoneDump(dumpString);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int zoneId = *reinterpret_cast<const int *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->ClearAudioFocusInfoListOnAccountsChanged(zoneId);
}

void GetStreamTypePriorityFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    OHOS::AudioStandard::AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetStreamTypePriority(streamType);
}

void GetStreamPriorityMapFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetStreamPriorityMap();
}

void SendInterruptEventFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }

    AudioFocuState oldState = *reinterpret_cast<const AudioFocuState *>(rawData);
    AudioFocuState newState = *reinterpret_cast<const AudioFocuState *>(rawData);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    focusInfo.first.contentType = *reinterpret_cast<const ContentType *>(rawData);
    focusInfo.first.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    focusInfo.first.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(rawData);
    focusInfo.first.audioFocusType.isPlay = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.sessionId = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.pauseWhenDucked = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.pid = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.mode = *reinterpret_cast<const InterruptMode *>(rawData);
    focusInfo.second = *reinterpret_cast<const AudioFocuState *>(rawData);
    focusInfoList.push_back(focusInfo);
    auto it = focusInfoList.begin();
    if (interruptService == nullptr) {
        return;
    }
    bool removeFocusInfo = *reinterpret_cast<const bool *>(rawData);
    interruptService->SendInterruptEvent(oldState, newState, it, removeFocusInfo);
}

void IsSameAppInShareModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt incomingInterrupt, activateInterrupt;
    incomingInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    incomingInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    incomingInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    activateInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    activateInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    activateInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->IsSameAppInShareMode(incomingInterrupt, activateInterrupt);
}

void SendFocusChangeEventFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioInterrupt audioInterrupt;
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    int32_t callbackCategory = *reinterpret_cast<const int32_t *>(rawData);
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
}

void GetAudioFocusInfoListFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < OHOS::AudioStandard::LIMITSIZE) {
        return;
    }
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    focusInfo.first.contentType = *reinterpret_cast<const ContentType *>(rawData);
    focusInfo.first.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    focusInfo.first.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(rawData);
    focusInfo.first.audioFocusType.isPlay = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.sessionId = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.pauseWhenDucked = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.pid = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.mode = *reinterpret_cast<const InterruptMode *>(rawData);
    focusInfo.second = *reinterpret_cast<const AudioFocuState *>(rawData);
    focusInfoList.push_back(focusInfo);
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    int32_t zoneId = *reinterpret_cast<const int32_t *>(rawData);
    if (interruptService == nullptr) {
        return;
    }
    interruptService->GetAudioFocusInfoList(zoneId, focusInfoList);
}

void AudioVolumeMoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    VolumeAdjustType adjustType = *reinterpret_cast<const VolumeAdjustType *>(rawData);
    int32_t volume = *reinterpret_cast<const int32_t *>(rawData);
    int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
    DeviceType deviceType = *reinterpret_cast<const DeviceType *>(rawData);
    int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t pid = *reinterpret_cast<const int32_t *>(rawData);

    bool mute = *reinterpret_cast<const bool *>(rawData);
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

    AudioRendererInfo rendererInfo = getAudioRenderInfo(rawData, size);
    GetServerPtr()->GetPreferredOutputStreamType(rendererInfo);

    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    AudioCapturerInfo capturerInfo = {
        sourceType,
        capturerFlags
    };
    GetServerPtr()->GetPreferredInputStreamType(capturerInfo);
}

void AudioDeviceMoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {return;}
    DeviceFlag flag = *reinterpret_cast<const DeviceFlag *>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    AudioCapturerInfo capturerInfo = {sourceType, capturerFlags};
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
    audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
    InternalDeviceType deviceType = *reinterpret_cast<const InternalDeviceType *>(rawData);
    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);
    bool active = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetDeviceActive(deviceType, active);
    GetServerPtr()->IsDeviceActive(deviceType);
    GetServerPtr()->NotifyCapturerAdded(capturerInfo, audioStreamInfo, sessionId);
    GetServerPtr()->GetDevices(flag);
    GetServerPtr()->GetDevicesInner(flag);
    AudioRingerMode ringMode = *reinterpret_cast<const AudioRingerMode *>(rawData);
    GetServerPtr()->SetRingerMode(ringMode);
    bool mute = *reinterpret_cast<const bool *>(rawData);
    bool legacy = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetMicrophoneMute(mute);
    GetServerPtr()->SetMicrophoneMuteCommon(mute, legacy);
    GetServerPtr()->SetMicrophoneMuteAudioConfig(mute);
    GetServerPtr()->SetMicrophoneMutePersistent(mute, PolicyType::PRIVACY_POLCIY_TYPE);
    GetServerPtr()->GetPersistentMicMuteState();
    GetServerPtr()->IsMicrophoneMuteLegacy();
    GetServerPtr()->GetAudioScene();
}

void AudioPolicySomeMoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
    GetServerPtr()->OnAudioStreamRemoved(sessionID);
    GetServerPtr()->ProcessSessionRemoved(sessionID);
    GetServerPtr()->ProcessorCloseWakeupSource(sessionID);
}

void AudioPolicyOtherMoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int pid = *reinterpret_cast<const int *>(rawData);
    GetServerPtr()->RegisteredTrackerClientDied(pid);
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::MONO;
    GetServerPtr()->IsAudioRendererLowLatencySupported(audioStreamInfo);

    int32_t clientUid = *reinterpret_cast<const int32_t *>(rawData);
    StreamSetState streamSetState = STREAM_PAUSE;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    GetServerPtr()->UpdateStreamState(clientUid, streamSetState, streamUsage);
    GetServerPtr()->IsHighResolutionExist();
    bool highResExist = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetHighResolutionExist(highResExist);
}

void AudioVolumeKeyCallbackStubMoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioPolicyClientStub> listener =
        static_cast<sptr<AudioPolicyClientStub>>(new(std::nothrow) AudioPolicyClientStubImpl());
    VolumeEvent volumeEvent = {};
    volumeEvent.volumeType =  *reinterpret_cast<const AudioStreamType *>(rawData);
    volumeEvent.volume = *reinterpret_cast<const int32_t *>(rawData);
    volumeEvent.updateUi = *reinterpret_cast<const bool *>(rawData);
    volumeEvent.volumeGroupId = *reinterpret_cast<const int32_t *>(rawData);
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

void AudioPolicyManagerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEvent audioEvent;
    uint32_t eventType = *reinterpret_cast<const uint32_t *>(rawData);
    uint32_t deviceType = *reinterpret_cast<const uint32_t *>(rawData);
    audioEvent.eventType = eventType;
    audioEvent.deviceType = deviceType;
    audioEvent.name = DEFAULTNAME;
    audioEvent.address = DEFAULTADDRESS;
    int fd = *reinterpret_cast<const int*>(rawData);
    ssize_t strLength = DEFAULTSTRLENGTH;
    const char *msg = reinterpret_cast<const char *>(rawData);
    AudioSocketThread::IsUpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::UpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::AudioPnpUeventOpen(&fd);
    AudioSocketThread::UpdateDeviceState(audioEvent);
    AudioSocketThread::DetectUsbHeadsetState(&audioEvent);
    AudioSocketThread::DetectAnalogHeadsetState(&audioEvent);
    AudioSocketThread::AudioPnpUeventParse(msg, strLength);
    AudioInputThread::AudioPnpInputOpen();

    GetPnpServerPtr()->GetAudioPnpServer();
    GetPnpServerPtr()->UnRegisterPnpStatusListener();
    GetPnpServerPtr()->OnPnpDeviceStatusChanged(DEFAULTINFO);
    AudioInputThread::AudioPnpInputPollAndRead();
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::InitFuzzTest(data, size);
    OHOS::AudioStandard::GetHighestPriorityAudioSceneFuzzTest(data, size);
    OHOS::AudioStandard::AudioInterruptZoneDumpFuzzTest(data, size);
    OHOS::AudioStandard::ClearAudioFocusInfoListOnAccountsChangedFuzzTest(data, size);
    OHOS::AudioStandard::GetStreamTypePriorityFuzzTest(data, size);
    OHOS::AudioStandard::GetStreamPriorityMapFuzzTest(data, size);
    OHOS::AudioStandard::SendInterruptEventFuzzTest(data, size);
    OHOS::AudioStandard::IsSameAppInShareModeFuzzTest(data, size);
    OHOS::AudioStandard::GetAudioFocusInfoListFuzzTest(data, size);
    OHOS::AudioStandard::AudioVolumeMoreFuzzTest(data, size);
    OHOS::AudioStandard::AudioDeviceMoreFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicySomeMoreFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyOtherMoreFuzzTest(data, size);
    OHOS::AudioStandard::AudioVolumeKeyCallbackStubMoreFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyManagerFuzzTest(data, size);
    return 0;
}