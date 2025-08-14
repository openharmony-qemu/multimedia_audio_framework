/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include "audio_server.h"
#include "message_parcel.h"
#include "pulseaudio_ipc_interface_code.h"
#include "audio_service_types.h"
#include "../fuzz_utils.h"
using namespace std;
namespace OHOS {
namespace AudioStandard {
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
constexpr int32_t OFFSET = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"OHOS.AudioStandard.IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t NUM_2 = 2;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_MIN = 0;
const int32_t AUDIO_DISTRIBUTED_SERVICE_ID = 3001;
const int32_t AUDIO_POLICY_SERVICE_ID = 3009;
const uint32_t LIMIT_MAX = static_cast<uint32_t>(AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX);
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<std::string> g_testKeys = {
    "PCM_DUMP",
    "hpae_effect",
    "test",
};
const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX
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
const vector<HdiIdType> g_testHdiIdTypes = {
    HDI_ID_TYPE_PRIMARY,
    HDI_ID_TYPE_FAST,
    HDI_ID_TYPE_REMOTE,
    HDI_ID_TYPE_REMOTE_FAST,
    HDI_ID_TYPE_FILE,
    HDI_ID_TYPE_BLUETOOTH,
    HDI_ID_TYPE_OFFLOAD,
    HDI_ID_TYPE_EAC3,
    HDI_ID_TYPE_MULTICHANNEL,
    HDI_ID_TYPE_WAKEUP,
    HDI_ID_TYPE_ACCESSORY,
    HDI_ID_TYPE_NUM,
};

class DataTransferStateChangeCallbackInnerFuzzTest : public DataTransferStateChangeCallbackInner {
public:
    void OnDataTransferStateChange(const int32_t &callbackId,
            const AudioRendererDataTransferStateChangeInfo &info) override {}
    void OnMuteStateChange(const int32_t &callbackId, const int32_t &uid,
        const uint32_t &sessionId, const bool &isMuted) override {}
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
uint32_t Convert2Uint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    /* Move the 0th digit to the left by 24 bits, the 1st digit to the left by 16 bits,
       the 2nd digit to the left by 8 bits, and the 3rd digit not to the left */
    return (ptr[0] << SHIFT_LEFT_24) | (ptr[1] << SHIFT_LEFT_16) | (ptr[2] << SHIFT_LEFT_8) | (ptr[3]);
}

void AudioServerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code =  Convert2Uint32(rawData) % (LIMIT_MAX - LIMIT_MIN + 1) + LIMIT_MIN;
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioServer> AudioServerPtr =
        std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK)) {
        sptr<AudioPolicyManagerListenerStubImpl> focusListenerStub =
            new(std::nothrow) AudioPolicyManagerListenerStubImpl();
        sptr<IRemoteObject> object = focusListenerStub->AsObject();
        AudioServerPtr->SetParameterCallback(object);
        return;
    }
    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE)) {
        int32_t asrAecMode = 0;
        AudioServerPtr->SetAsrAecMode(asrAecMode);
        AudioServerPtr->OnRemoteRequest(code, data, reply, option);
        return;
    }
    AudioServerPtr->OnRemoteRequest(code, data, reply, option);
    if (size < LIMITSIZE) {
        return;
    }
    std::string netWorkId(reinterpret_cast<const char*>(rawData), size - 1);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey *>(rawData);
    std::string condition(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    AudioServerPtr->OnRenderSinkParamChange(netWorkId, key, condition, value);
}

float Convert2Float(const uint8_t *ptr)
{
    float floatValue = static_cast<float>(*ptr);
    return floatValue / 128.0f - 1.0f;
}

void AudioServerOffloadSetVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float volume = Convert2Float(rawData);
    data.WriteFloat(volume);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME),
        data, reply, option);
}

void AudioServerNotifyStreamVolumeChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED),
        data, reply, option);
}

void AudioServerResetRouteForDisconnectFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(deviceType);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT),
        data, reply, option);
}

void AudioServerGetEffectLatencyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string sessionId(reinterpret_cast<const char*>(rawData), size);
    data.WriteString(sessionId);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY),
        data, reply, option);
}

void AudioServerUpdateLatencyTimestampTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string timestamp(reinterpret_cast<const char*>(rawData), size - 1);
    bool isRenderer = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(timestamp);
    data.WriteBool(isRenderer);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP),
        data, reply, option);
}

void AudioServerGetMaxAmplitudeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isOutputDevice = *reinterpret_cast<const bool*>(rawData);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteBool(isOutputDevice);
    data.WriteInt32(deviceType);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE),
        data, reply, option);
}

void AudioServerResetAudioEndpointTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_AUDIO_ENDPOINT),
        data, reply, option);
}

void AudioServerCreatePlaybackCapturerManagerTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER),
        data, reply, option);
}

void AudioServerSetOutputDeviceSinkTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    std::string sinkName(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteInt32(deviceType);
    data.WriteString(sinkName);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK),
        data, reply, option);
}

void AudioServerSetAudioMonoStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool audioMono = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(audioMono);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE),
        data, reply, option);
}

void AudioServerSetVoiceVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float volume = *reinterpret_cast<const float*>(rawData);
    data.WriteFloat(volume);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME),
        data, reply, option);
}

void AudioServerCheckRemoteDeviceStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string networkId(reinterpret_cast<const char*>(rawData), size - 1);
    DeviceRole deviceRole = *reinterpret_cast<const DeviceRole*>(rawData);
    bool isStartDevice = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(deviceRole));
    data.WriteBool(isStartDevice);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE),
        data, reply, option);
}

void AudioServerNotifyDeviceInfoTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string networkId(reinterpret_cast<const char*>(rawData), size - 1);
    bool connected = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(networkId);
    data.WriteBool(connected);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO),
        data, reply, option);
}

void AudioServerGetAudioParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string key(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteString(key);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetAudioParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string key(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteString(key);
    data.WriteString(value);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetMicrophoneMuteTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isMute = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(isMute);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE),
        data, reply, option);
}

void AudioServerSetAudioBalanceValueTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float audioBalance = *reinterpret_cast<const float*>(rawData);
    data.WriteFloat(audioBalance);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE),
        data, reply, option);
}

void AudioServerSetAudioSceneTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    AudioScene audioScene = *reinterpret_cast<const AudioScene*>(rawData);
    data.WriteInt32(static_cast<int32_t>(audioScene));
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE),
        data, reply, option);
}

void AudioServerSetOffloadModeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    int32_t state = *reinterpret_cast<const int32_t*>(rawData);
    bool isAppBack = *reinterpret_cast<const bool*>(rawData);
    data.WriteUint32(sessionId);
    data.WriteInt32(state);
    data.WriteBool(isAppBack);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OFFLOAD_MODE),
        data, reply, option);
}

void AudioServerUnsetOffloadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sessionId);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UNSET_OFFLOAD_MODE),
        data, reply, option);
}

void AudioServerCheckHibernateStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool hibernate = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(hibernate);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_HIBERNATE_STATE),
        data, reply, option);
}

void AudioServerSetSessionMuteStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sessionId = 10;
    data.WriteInt32(sessionId);
    data.WriteBool(true);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_SESSION_MUTE_STATE),
        data, reply, option);
}

void AudioServerNotifyMuteStateChangeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sessionId = 10;
    data.WriteInt32(sessionId);
    data.WriteBool(true);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_MUTE_STATE_CHANGE),
        data, reply, option);
}

void AudioServerAudioWorkgroupCreateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupReleaseTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));
    data.WriteInt32(static_cast<int32_t>(workgroupId));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RELEASE_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupAddThreadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t tokenId = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteInt32(static_cast<int32_t>(tokenId));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::ADD_THREAD_TO_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupRemoveThreadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t tokenId = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteInt32(static_cast<int32_t>(tokenId));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::REMOVE_THREAD_FROM_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupStartGroupTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    uint64_t startTime = *reinterpret_cast<const int32_t*>(rawData);
    uint64_t deadlineTime = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteUint64(static_cast<int32_t>(startTime));
    data.WriteUint64(static_cast<int32_t>(deadlineTime));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::START_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupStopGroupTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(static_cast<int32_t>(testPid));
    data.WriteInt32(static_cast<int32_t>(workgroupId));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::STOP_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerDumpTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    vector<std::u16string> gTestDumpArges = {
        u"-fb",
        u"test",
        u"test2",
        u"test3",
    };
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<std::u16string> args(gTestDumpArges.begin(), gTestDumpArges.begin() +
        (static_cast<uint32_t>(size) % gTestDumpArges.size()));
    int32_t fd = static_cast<int32_t>(size);

    audioServerPtr->Dump(fd, args);
}

void AudioServerGetUsbParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const vector<std::string> params = {
        "address=card2;device=0 role=1",
        "address=card2;device=0 role=2"
    };
    std::string param = params[*reinterpret_cast<const uint32_t*>(rawData) % params.size()];
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetUsbParameter(param);
}

void AudioServerOnAddSystemAbilityTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    vector<int32_t> gTestSystemAbilityId = {
        0,
        AUDIO_POLICY_SERVICE_ID,
        AUDIO_DISTRIBUTED_SERVICE_ID,
    };
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    uint32_t id = static_cast<uint32_t>(size) % gTestSystemAbilityId.size();
    std::string deviceId = "0";

    audioServerPtr->OnAddSystemAbility(gTestSystemAbilityId[id], deviceId);
}

void AudioServerSetExtraParametersTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<std::pair<std::string, std::string>> kvpairs;
    uint32_t id = static_cast<uint32_t>(size) % g_testKeys.size();
    std::string key = g_testKeys[id];
    std::pair<std::string, std::string> kvpair = std::make_pair(g_testKeys[id], g_testKeys[id]);
    kvpairs.push_back(kvpair);
    audioServerPtr->CacheExtraParameters(key, kvpairs);
    audioServerPtr->ParseAudioParameter();
}

void AudioServerSetAudioParameterByKeyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    vector<std::string> gTestAudioParameterKeys = {
        "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG",
        "A2dpSuspended",
        "AUDIO_EXT_PARAM_KEY_LOWPOWER",
        "bt_headset_nrec",
        "bt_wbs",
        "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG",
        "mmi",
        "perf_info",
    };

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t id = static_cast<uint32_t>(size) % gTestAudioParameterKeys.size();
    audioServerPtr->SetAudioParameter(gTestAudioParameterKeys[id], "");
}

void AudioServerGetExtraParametersTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t id = static_cast<uint32_t>(size) % g_testKeys.size();
    bool isAudioParameterParsed = static_cast<uint32_t>(size) % NUM_2;
    audioServerPtr->isAudioParameterParsed_.store(isAudioParameterParsed);
    std::vector<StringPair> result;
    std::vector<std::string> subKeys;
    audioServerPtr->GetExtraParameters(g_testKeys[id], subKeys, result);
}

void AudioServerGetAudioParameterByIdTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    vector<std::string> tetsNetworkId = {
        "LocalDevice",
        "TestNetwork",
    };
    vector<AudioParamKey> audioParamKey {
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
    string str = "test";
    uint32_t id = static_cast<uint32_t>(size) % audioParamKey.size();
    AudioParamKey key = static_cast<AudioParamKey>(audioParamKey[id]);
    id = static_cast<uint32_t>(size) % tetsNetworkId.size();
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->GetAudioParameter(tetsNetworkId[id], key, "", str);
}

void AudioServerIsFastBlockedTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    vector<PlayerType> gPlayerType = {
        PLAYER_TYPE_DEFAULT,
        PLAYER_TYPE_SOUND_POOL,
        PLAYER_TYPE_AV_PLAYER,
    };

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    uint32_t id = static_cast<uint32_t>(size) % gPlayerType.size();
    audioServerPtr->IsFastBlocked(static_cast<uint32_t>(id), gPlayerType[id]);
}

void AudioServerCheckRemoteDeviceStateTestTwo(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    vector<DeviceRole> gDeviceRole = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    bool isStartDevice = static_cast<uint32_t>(size) % NUM_2;
    uint32_t id = static_cast<uint32_t>(size) % gDeviceRole.size();
    audioServerPtr->CheckRemoteDeviceState("LocalDevice", gDeviceRole[id], isStartDevice);
}

void AudioServerCreateAudioStreamTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    sptr<IRemoteObject> remoteObject = nullptr;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->NotifyProcessStatus();
    AudioProcessConfig config;
    std::shared_ptr<PipeInfoGuard> pipeinfoGuard = std::make_shared<PipeInfoGuard>(0);
    vector<int32_t> gTestCallingUid = {
        AudioServer::VASSISTANT_UID,
        AudioServer::MEDIA_SERVICE_UID,
    };
    uint32_t id = static_cast<uint32_t>(size) % gTestCallingUid.size();
    config.audioMode = AUDIO_MODE_RECORD;
    remoteObject = audioServerPtr->CreateAudioStream(config, gTestCallingUid[id], pipeinfoGuard);
    vector<AudioParamKey> audioParamKey {
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

    config.audioMode = static_cast<AudioMode>(-1);
    audioServerPtr->IsNormalIpcStream(config);
    id = static_cast<uint32_t>(size) % audioParamKey.size();
    audioServerPtr->OnRenderSinkParamChange("", audioParamKey[id], "", "");
    audioServerPtr->OnCaptureSourceParamChange("", audioParamKey[id], "", "");
    audioServerPtr->OnWakeupClose();
    id = static_cast<uint32_t>(size) % NUM_2;
    audioServerPtr->OnCapturerState(static_cast<bool>(id), id, id);
    audioServerPtr->SetParameterCallback(remoteObject);
    audioServerPtr->SetWakeupSourceCallback(remoteObject);
    audioServerPtr->RegiestPolicyProvider(remoteObject);
    audioServerPtr->RegistCoreServiceProvider(remoteObject);
}

void AudioServerSetSinkRenderEmptyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    uint32_t id = static_cast<uint32_t>(size) % NUM_2;
    audioServerPtr->SetSinkRenderEmpty("primary", id);
}

void AudioServerOnRenderSinkStateChangeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    uint32_t id = static_cast<uint32_t>(size) % NUM_2;
    audioServerPtr->OnRenderSinkStateChange(id, static_cast<bool>(id));
}

void AudioServerCreateHdiSinkPortTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t renderId = 0;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    IAudioSinkAttr attr;
    std::string deviceClass = "audio_test_class";
    std::string idInfo = "audio_indo";
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->CreateHdiSinkPort(deviceClass, idInfo, attr, renderId);
}

void AudioServerCreateHdiSourcePortTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint32_t captureId = 0;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    IAudioSourceAttr attr;
    std::string deviceClass = "audio_test_class";
    std::string idInfo = "audio_indo";
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->CreateHdiSourcePort(deviceClass, idInfo, attr, captureId);
}

void AudioServerRegisterDataTransferCallbackTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<AudioPolicyManagerListenerStubImpl> focusListenerStub = new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    sptr<IRemoteObject> object = focusListenerStub->AsObject();

    audioServerPtr->RegisterDataTransferCallback(object);
}

void AudioServerWriteServiceStartupErrorTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->WriteServiceStartupError();
}

void AudioServerProcessKeyValuePairsTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    static const vector<string> testPairs = {
        "unprocess_audio_effect",
        "test",
    };
    string key = "test_key";
    string value{};
    string pairTest = testPairs[static_cast<uint32_t>(size) % testPairs.size()];
    std::vector<std::pair<std::string, std::string>> kvpairs;
    kvpairs.push_back(make_pair(pairTest, "test_value"));
    set<std::string> subKeys = {"effect"};
    unordered_map<std::string, std::set<std::string>> subKeyMap;
    subKeyMap.insert(make_pair(pairTest, subKeys));

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ProcessKeyValuePairs(key, kvpairs, subKeyMap, value);
}

void AudioServerSetA2dpAudioParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    string renderValue;

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetA2dpAudioParameter(renderValue);
}

void AudioServerGetAudioParameterByKeyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    static const vector<string> testKeys = {
        "AUDIO_EXT_PARAM_KEY_LOWPOWER",
        "need_change_usb_device#C",
        "getSmartPAPOWER",
        "show_RealTime_ChipModel",
        "perf_info",
    };

    string value = "test";
    string key = testKeys[static_cast<uint32_t>(size) % testKeys.size()];
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->GetAudioParameter(key, value);
}

void AudioServerGetDPParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::string condition;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetDPParameter(condition);
}

void AudioServerSetAudioSceneByDeviceTypeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t index = static_cast<uint32_t>(size);
    static const vector<AudioScene> testAudioScenes = {
        AUDIO_SCENE_INVALID,
        AUDIO_SCENE_DEFAULT,
        AUDIO_SCENE_RINGING,
        AUDIO_SCENE_PHONE_CALL,
        AUDIO_SCENE_PHONE_CHAT,
        AUDIO_SCENE_CALL_START,
        AUDIO_SCENE_CALL_END,
        AUDIO_SCENE_VOICE_RINGING,
        AUDIO_SCENE_MAX,
    };
    static const vector<BluetoothOffloadState> testBluetoothOffloadStates = {
        NO_A2DP_DEVICE,
        A2DP_NOT_OFFLOAD,
        A2DP_OFFLOAD,
    };
    bool scoExcludeFlag = static_cast<bool>(index % NUM_2);
    BluetoothOffloadState a2dpOffloadFlag = testBluetoothOffloadStates[index % testBluetoothOffloadStates.size()];
    AudioScene audioScene = testAudioScenes[static_cast<uint32_t>(size) % testAudioScenes.size()];
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioScene(audioScene, a2dpOffloadFlag, scoExcludeFlag);
}

void AudioServerNotifyDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::string networkId = "test_network_id";
    bool connected = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->NotifyDeviceInfo(networkId, connected);
}

void AudioServerSetVoiceVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    float volume = *reinterpret_cast<const float*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->SetVoiceVolume(volume);
}

void AudioServerCheckRemoteDeviceStateFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<DeviceRole> testDeviceRole = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    if (rawData == nullptr || size < LIMITSIZE || testDeviceRole.size() == 0) {
        return;
    }

    std::string networkId = "test_network_id";
    uint32_t deviceId = *reinterpret_cast<const uint32_t*>(rawData);
    DeviceRole deviceRole = testDeviceRole[deviceId % testDeviceRole.size()];
    bool isStartDevice = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
}

void AudioServerSetAudioBalanceValueFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    float audioBalance = *reinterpret_cast<const float*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->SetAudioBalanceValue(audioBalance);
}

void AudioServerRemoveRendererDataTransferCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RemoveRendererDataTransferCallback(testPid);
}

void AudioServerRegisterDataTransferCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferCallback(object);
}

void AudioServerRegisterDataTransferMonitorParamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t testCallbackId = *reinterpret_cast<const int32_t*>(rawData);
    DataTransferMonitorParam param;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferMonitorParam(testCallbackId, param);
}

void AudioServerUnregisterDataTransferMonitorParamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t testCallbackId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UnregisterDataTransferMonitorParam(testCallbackId);
}

void AudioServerOnDataTransferStateChangeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t testCallbackId = *reinterpret_cast<const int32_t*>(rawData);
    AudioRendererDataTransferStateChangeInfo info;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->OnDataTransferStateChange(testPid, testCallbackId, info);
}

void AudioServerOnMuteStateChangeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t testPid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t testCallbackId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t testUid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t testSessionId = *reinterpret_cast<const uint32_t*>(rawData);
    bool testIsMuted = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->OnMuteStateChange(testPid, testCallbackId, testUid, testSessionId, testIsMuted);
}

void AudioServerRegisterDataTransferStateChangeCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferStateChangeCallback();
}

void AudioServerInitMaxRendererStreamCntPerUidFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->InitMaxRendererStreamCntPerUid();
}

void AudioServerSetPcmDumpParameterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const vector<string> testPairs = {
        "OPEN",
        "CLOSE",
        "UPLOAD",
        "test"
    };
    string pairTest = testPairs[*reinterpret_cast<const uint32_t*>(rawData) % testPairs.size()];
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back(make_pair(pairTest, "test_value"));
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetPcmDumpParameter(params);
}

void AudioServerSuspendRenderSinkFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string sinkName = "test_sinkName";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SuspendRenderSink(sinkName);
}

void AudioServerRestoreRenderSinkFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string sinkName = "test_sinkName";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RestoreRenderSink(sinkName);
}

void AudioServerSetAudioParameterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string networkId = "test_networkId";
    int32_t key = *reinterpret_cast<const int32_t*>(rawData);
    std::string condition = "test_condition";
    std::string value = "test_value";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioParameter(networkId, key, condition, value);
}

void AudioServerGetTransactionIdFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const vector<DeviceRole> g_deviceRole = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX
    };
    DeviceType deviceType = g_testDeviceTypes[*reinterpret_cast<const uint32_t*>(rawData) % g_testDeviceTypes.size()];
    DeviceRole deviceRole = g_deviceRole[*reinterpret_cast<const uint32_t*>(rawData) % g_deviceRole.size()];
    uint64_t transactionId = *reinterpret_cast<const uint64_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetTransactionId(deviceType, deviceRole, transactionId);
}

void AudioServerSetIORoutesFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t deviceTypeCount = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    DeviceType deviceType = g_testDeviceTypes[deviceTypeCount];
    uint32_t deviceFlagCount = static_cast<uint32_t>(size) % g_testDeviceFlags.size();
    DeviceFlag deviceFlag = g_testDeviceFlags[deviceFlagCount];
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    std::pair<DeviceType, DeviceFlag> activeDevice = std::make_pair(deviceType, deviceFlag);
    activeDevices.push_back(activeDevice);
    uint32_t index = static_cast<uint32_t>(size);
    static const vector<BluetoothOffloadState> testBluetoothOffloadStates = {
        NO_A2DP_DEVICE,
        A2DP_NOT_OFFLOAD,
        A2DP_OFFLOAD
    };
    BluetoothOffloadState a2dpOffloadFlag = testBluetoothOffloadStates[index % testBluetoothOffloadStates.size()];
    std::string deviceName = "test_deviceName";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetIORoutes(activeDevices, a2dpOffloadFlag, deviceName);
    std::vector<DeviceType> deviceTypes = {deviceType};
    audioServerPtr->SetIORoutes(deviceType, deviceFlag, deviceTypes, a2dpOffloadFlag, deviceName);
}

void AudioServerUpdateActiveDeviceRouteFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t type = *reinterpret_cast<const int32_t*>(rawData);
    int32_t flag = *reinterpret_cast<const int32_t*>(rawData);
    int32_t a2dpOffloadFlag = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateActiveDeviceRoute(type, flag, a2dpOffloadFlag);
}

void AudioServerUpdateActiveDevicesRouteFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::vector<IntPair> activeDevices;
    int32_t a2dpOffloadFlag = *reinterpret_cast<const int32_t*>(rawData);
    std::string deviceName = "test_deviceName";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag, deviceName);
}

void AudioServerSetDmDeviceTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint16_t dmDeviceType = *reinterpret_cast<const uint16_t*>(rawData);
    int32_t deviceTypeIn = *reinterpret_cast<const int32_t*>(rawData);
    std::vector<IntPair> activeDevices;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetDmDeviceType(dmDeviceType, deviceTypeIn);
}

void AudioServerSetAudioMonoStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool audioMono = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioMonoState(audioMono);
}

void AudioServerGetHapBuildApiVersionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t callerUid = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetHapBuildApiVersion(callerUid);
}

void AudioServerResetRecordConfigFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ResetRecordConfig(config);
}

void AudioServerResetProcessConfigFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ResetProcessConfig(config);
}

void AudioServerCheckStreamInfoFormatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckStreamInfoFormat(config);
}

void AudioServerCheckRendererFormatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckRendererFormat(config);
}

void AudioServerCheckRecorderFormatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckRecorderFormat(config);
}

void AudioServerCheckConfigFormatFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckConfigFormat(config);
}

void AudioServerSendCreateErrorInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    int32_t errorCode = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SendCreateErrorInfo(config, errorCode);
}

void AudioServerCheckMaxRendererInstancesFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckMaxRendererInstances();
}

void AudioServerCheckMaxLoopbackInstancesFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    uint8_t index = *reinterpret_cast<const uint8_t*>(rawData);
    AudioMode audioMode = static_cast<AudioMode>(index % modeCount);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckMaxLoopbackInstances(audioMode);
}

void AudioServerCheckAndWaitAudioPolicyReadyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckAndWaitAudioPolicyReady();
}

void AudioServerIsSatelliteFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    int32_t callingUid = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->IsSatellite(config, callingUid);
}

void AudioServerCreateAudioProcessFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<IRemoteObject> client = nullptr;
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    int32_t errorCode = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioProcess(config, errorCode, filterConfig, client);
}

void AudioServerCreateAudioProcessInnerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    int32_t errorCode = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioProcessInner(config, errorCode, filterConfig);
}

void AudioServerPermissionCheckerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->PermissionChecker(config);
}

void AudioServerCheckPlaybackPermissionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckPlaybackPermission(config);
}

void AudioServerCheckInnerRecorderPermissionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckInnerRecorderPermission(config);
}

void AudioServerHandleCheckRecorderBackgroundCaptureFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    config.callerUid = g_fuzzUtils.GetData<int32_t>();
    config.capturerInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    config.appInfo.appTokenId = g_fuzzUtils.GetData<uint32_t>();
    config.appInfo.appFullTokenId = g_fuzzUtils.GetData<uint64_t>();
    config.originalSessionId = g_fuzzUtils.GetData<uint32_t>();
    config.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    config.appInfo.appPid = g_fuzzUtils.GetData<int32_t>();

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->HandleCheckRecorderBackgroundCapture(config);
}

void AudioServerSetForegroundListFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::vector<std::string> list = {"test_list"};
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetForegroundList(list);
}

void AudioServerCreatePlaybackCapturerManagerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool isSuccess = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreatePlaybackCapturerManager(isSuccess);
}

void AudioServerRegisterAudioCapturerSourceCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterAudioCapturerSourceCallback();
}

void AudioServerRegisterAudioRendererSinkCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterAudioRendererSinkCallback();
}

void AudioServerGetMaxAmplitudeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool isOutputDevice = *reinterpret_cast<const bool*>(rawData);
    std::string deviceClass = "test_deviceClass";
    float maxAmplitude = *reinterpret_cast<const float*>(rawData);
    int32_t sourceType = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetMaxAmplitude(isOutputDevice, deviceClass, sourceType, maxAmplitude);
}

void AudioServerGetVolumeDataCountFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string sinkName = "test_sinkName";
    int64_t volumeData = *reinterpret_cast<const int64_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetVolumeDataCount(sinkName, volumeData);
}

void AudioServerUpdateLatencyTimestampFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string timestamp(reinterpret_cast<const char*>(rawData), size - 1);
    bool isRenderer = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateLatencyTimestamp(timestamp, isRenderer);
}

void AudioServerCheckHibernateStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool hibernate = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckHibernateState(hibernate);
}

void AudioServerCreateIpcOfflineStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t errorCode = *reinterpret_cast<const int32_t*>(rawData);
    sptr<IRemoteObject> client = nullptr;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateIpcOfflineStream(errorCode, client);
}

void AudioServerGetOfflineAudioEffectChainsFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::vector<std::string> effectChains = {"test_effectChains"};
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetOfflineAudioEffectChains(effectChains);
}

void AudioServerGetStandbyStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    bool isStandby = *reinterpret_cast<const bool*>(rawData);
    int64_t enterStandbyTime = *reinterpret_cast<const int64_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
}

void AudioServerGenerateSessionIdFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GenerateSessionId(sessionId);
}

void AudioServerNotifyAudioPolicyReadyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->NotifyAudioPolicyReady();
}

void AudioServerGetAllSinkInputsFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::vector<SinkInput> sinkInputs;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetAllSinkInputs(sinkInputs);
}

void AudioServerReleaseCaptureLimitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t innerCapId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ReleaseCaptureLimit(innerCapId);
}

void AudioServerLoadHdiAdapterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t devMgrType = *reinterpret_cast<const uint32_t*>(rawData);
    std::string adapterName = "test_adapterName";
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->LoadHdiAdapter(devMgrType, adapterName);
}

void AudioServerUnloadHdiAdapterFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t devMgrType = *reinterpret_cast<const uint32_t*>(rawData);
    std::string adapterName = "test_adapterName";
    bool force = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UnloadHdiAdapter(devMgrType, adapterName, force);
}

void AudioServerCreateSinkPortFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t idBase = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t idType = *reinterpret_cast<const uint32_t*>(rawData);
    std::string idInfo = "test_idInfo";
    IAudioSinkAttr attr;
    uint32_t renderId = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateSinkPort(idBase, idType, idInfo, attr, renderId);
}

void AudioServerCreateSourcePortFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t idBase = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t idType = *reinterpret_cast<const uint32_t*>(rawData);
    std::string idInfo = "test_idInfo";
    IAudioSourceAttr attr;
    uint32_t captureId = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateSourcePort(idBase, idType, idInfo, attr, captureId);
}

void AudioServerDestroyHdiPortFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t id = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->DestroyHdiPort(id);
}

void AudioServerSetDeviceConnectedFlagFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool flag = *reinterpret_cast<const bool*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetDeviceConnectedFlag(flag);
}

void AudioServerSetBtHdiInvalidStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetBtHdiInvalidState();
}

void AudioServerCreateAudioWorkgroupFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    sptr<IRemoteObject> object = nullptr;
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioWorkgroup(pid, object, workgroupId);
}

void AudioServerReleaseAudioWorkgroupFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ReleaseAudioWorkgroup(pid, workgroupId);
}

void AudioServerAddThreadToGroupFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t tokenId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->AddThreadToGroup(pid, workgroupId, tokenId);
}

void AudioServerForceStopAudioStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t audioType = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ForceStopAudioStream(audioType);
}

void AudioServerStartGroupFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    uint64_t startTime = *reinterpret_cast<const uint64_t*>(rawData);
    uint64_t deadlineTime = *reinterpret_cast<const uint64_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->StartGroup(pid, workgroupId, startTime, deadlineTime);
}

void AudioServerStopGroupFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t workgroupId = *reinterpret_cast<const int32_t*>(rawData);
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->StopGroup(pid, workgroupId);
}

void AudioServerSetActiveOutputDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t deviceTypeId = *reinterpret_cast<const int32_t*>(rawData) % g_testDeviceTypes.size();
    int32_t deviceType = g_testDeviceTypes[deviceTypeId];
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetActiveOutputDevice(deviceType);
}
void AudioServerResetRecordConfigSourceTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.capturerInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    audioServerPtr->ResetRecordConfig(config);
}

void AudioServerResetProcessConfigCallerUidFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.callerUid = g_fuzzUtils.GetData<int32_t>();
    audioServerPtr->ResetProcessConfig(config);
}

void AudioServerCheckStreamInfoFormatNotContainFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.streamInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    config.streamInfo.encoding = g_fuzzUtils.GetData<AudioEncodingType>();
    config.audioMode = g_fuzzUtils.GetData<AudioMode>();
    config.streamInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    audioServerPtr->CheckStreamInfoFormat(config);
}

void AudioServerCheckRendererFormatNotContainFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.rendererInfo.streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    audioServerPtr->CheckRendererFormat(config);
}

void AudioServerCheckRecorderFormatNotContainFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.capturerInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    config.capturerInfo.capturerFlags = g_fuzzUtils.GetData<int32_t>();
    audioServerPtr->CheckRecorderFormat(config);
}

void AudioServerCreateAudioProcessInnerAudioModeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    int32_t errorCode = g_fuzzUtils.GetData<int32_t>();
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.audioMode = g_fuzzUtils.GetData<AudioMode>();
    config.rendererInfo.streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    audioServerPtr->CreateAudioProcessInner(config, errorCode, filterConfig);
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioServerHandleCheckCaptureLimitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.capturerInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    audioServerPtr->HandleCheckCaptureLimit(config, filterConfig);
}

void AudioServerInnerCheckCaptureLimitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    int32_t innerCapId = g_fuzzUtils.GetData<int32_t>();
    audioServerPtr->InnerCheckCaptureLimit(filterConfig, innerCapId);
}
#endif

void AudioServerIsNormalIpcStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.audioMode = g_fuzzUtils.GetData<AudioMode>();
    audioServerPtr->IsNormalIpcStream(config);
}

void AudioServerCheckRemoteDeviceStateSwitchCaseFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    bool isStartDevice = g_fuzzUtils.GetData<bool>();
    int32_t deviceRole = g_fuzzUtils.GetData<int32_t>();
    audioServerPtr->CheckRemoteDeviceState("LocalDevice", deviceRole, isStartDevice);
}

void AudioServerCheckInnerRecorderPermissionSourceTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    config.capturerInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    audioServerPtr->CheckInnerRecorderPermission(config);
}

void AudioServerSetRenderWhitelistFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    std::vector<std::string> list;
    list.push_back(g_fuzzUtils.GetData<std::string>());
    audioServerPtr->SetRenderWhitelist(list);
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioServerDumpTest,
    OHOS::AudioStandard::AudioServerGetUsbParameterTest,
    OHOS::AudioStandard::AudioServerOnAddSystemAbilityTest,
    OHOS::AudioStandard::AudioServerSetExtraParametersTest,
    OHOS::AudioStandard::AudioServerSetAudioParameterByKeyTest,
    OHOS::AudioStandard::AudioServerGetExtraParametersTest,
    OHOS::AudioStandard::AudioServerGetAudioParameterByIdTest,
    OHOS::AudioStandard::AudioServerIsFastBlockedTest,
    OHOS::AudioStandard::AudioServerCheckRemoteDeviceStateTestTwo,
    OHOS::AudioStandard::AudioServerCreateAudioStreamTest,
    OHOS::AudioStandard::AudioServerSetSinkRenderEmptyTest,
    OHOS::AudioStandard::AudioServerOnRenderSinkStateChangeTest,
    OHOS::AudioStandard::AudioServerCreateHdiSinkPortTest,
    OHOS::AudioStandard::AudioServerCreateHdiSourcePortTest,
    OHOS::AudioStandard::AudioServerFuzzTest,
    OHOS::AudioStandard::AudioServerOffloadSetVolumeFuzzTest,
    OHOS::AudioStandard::AudioServerNotifyStreamVolumeChangedFuzzTest,
    OHOS::AudioStandard::AudioServerResetRouteForDisconnectFuzzTest,
    OHOS::AudioStandard::AudioServerGetEffectLatencyTest,
    OHOS::AudioStandard::AudioServerGetMaxAmplitudeTest,
    OHOS::AudioStandard::AudioServerResetAudioEndpointTest,
    OHOS::AudioStandard::AudioServerCreatePlaybackCapturerManagerTest,
    OHOS::AudioStandard::AudioServerSetOutputDeviceSinkTest,
    OHOS::AudioStandard::AudioServerSetAudioMonoStateTest,
    OHOS::AudioStandard::AudioServerSetVoiceVolumeTest,
    OHOS::AudioStandard::AudioServerCheckRemoteDeviceStateTest,
    OHOS::AudioStandard::AudioServerNotifyDeviceInfoTest,
    OHOS::AudioStandard::AudioServerGetAudioParameterTest,
    OHOS::AudioStandard::AudioServerSetAudioParameterTest,
    OHOS::AudioStandard::AudioServerSetMicrophoneMuteTest,
    OHOS::AudioStandard::AudioServerSetAudioBalanceValueTest,
    OHOS::AudioStandard::AudioServerSetAudioSceneTest,
    OHOS::AudioStandard::AudioServerUpdateLatencyTimestampTest,
    OHOS::AudioStandard::AudioServerSetOffloadModeTest,
    OHOS::AudioStandard::AudioServerUnsetOffloadTest,
    OHOS::AudioStandard::AudioServerCheckHibernateStateTest,
    OHOS::AudioStandard::AudioServerSetSessionMuteStateTest,
    OHOS::AudioStandard::AudioServerNotifyMuteStateChangeTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupCreateTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupReleaseTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupAddThreadTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupRemoveThreadTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupStartGroupTest,
    OHOS::AudioStandard::AudioServerAudioWorkgroupStopGroupTest,
    OHOS::AudioStandard::AudioServerRegisterDataTransferCallbackTest,
    OHOS::AudioStandard::AudioServerWriteServiceStartupErrorTest,
    OHOS::AudioStandard::AudioServerProcessKeyValuePairsTest,
    OHOS::AudioStandard::AudioServerSetA2dpAudioParameterTest,
    OHOS::AudioStandard::AudioServerGetAudioParameterByKeyTest,
    OHOS::AudioStandard::AudioServerGetDPParameterTest,
    OHOS::AudioStandard::AudioServerSetAudioSceneByDeviceTypeTest,
    OHOS::AudioStandard::AudioServerNotifyDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioServerSetVoiceVolumeFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRemoteDeviceStateFuzzTest,
    OHOS::AudioStandard::AudioServerSetAudioBalanceValueFuzzTest,
    OHOS::AudioStandard::AudioServerRemoveRendererDataTransferCallbackFuzzTest,
    OHOS::AudioStandard::AudioServerRegisterDataTransferCallbackFuzzTest,
    OHOS::AudioStandard::AudioServerRegisterDataTransferMonitorParamFuzzTest,
    OHOS::AudioStandard::AudioServerUnregisterDataTransferMonitorParamFuzzTest,
    OHOS::AudioStandard::AudioServerOnDataTransferStateChangeFuzzTest,
    OHOS::AudioStandard::AudioServerRegisterDataTransferStateChangeCallbackFuzzTest,
    OHOS::AudioStandard::AudioServerInitMaxRendererStreamCntPerUidFuzzTest,
    OHOS::AudioStandard::AudioServerSetPcmDumpParameterFuzzTest,
    OHOS::AudioStandard::AudioServerSuspendRenderSinkFuzzTest,
    OHOS::AudioStandard::AudioServerRestoreRenderSinkFuzzTest,
    OHOS::AudioStandard::AudioServerSetAudioParameterFuzzTest,
    OHOS::AudioStandard::AudioServerGetTransactionIdFuzzTest,
    OHOS::AudioStandard::AudioServerSetIORoutesFuzzTest,
    OHOS::AudioStandard::AudioServerUpdateActiveDeviceRouteFuzzTest,
    OHOS::AudioStandard::AudioServerUpdateActiveDevicesRouteFuzzTest,
    OHOS::AudioStandard::AudioServerSetDmDeviceTypeFuzzTest,
    OHOS::AudioStandard::AudioServerSetAudioMonoStateFuzzTest,
    OHOS::AudioStandard::AudioServerGetHapBuildApiVersionFuzzTest,
    OHOS::AudioStandard::AudioServerResetRecordConfigFuzzTest,
    OHOS::AudioStandard::AudioServerResetProcessConfigFuzzTest,
    OHOS::AudioStandard::AudioServerCheckStreamInfoFormatFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRendererFormatFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRecorderFormatFuzzTest,
    OHOS::AudioStandard::AudioServerCheckConfigFormatFuzzTest,
    OHOS::AudioStandard::AudioServerSendCreateErrorInfoFuzzTest,
    OHOS::AudioStandard::AudioServerCheckMaxRendererInstancesFuzzTest,
    OHOS::AudioStandard::AudioServerCheckMaxLoopbackInstancesFuzzTest,
    OHOS::AudioStandard::AudioServerCheckAndWaitAudioPolicyReadyFuzzTest,
    OHOS::AudioStandard::AudioServerIsSatelliteFuzzTest,
    OHOS::AudioStandard::AudioServerCreateAudioProcessFuzzTest,
    OHOS::AudioStandard::AudioServerCreateAudioProcessInnerFuzzTest,
    OHOS::AudioStandard::AudioServerPermissionCheckerFuzzTest,
    OHOS::AudioStandard::AudioServerCheckPlaybackPermissionFuzzTest,
    OHOS::AudioStandard::AudioServerCheckInnerRecorderPermissionFuzzTest,
    OHOS::AudioStandard::AudioServerHandleCheckRecorderBackgroundCaptureFuzzTest,
    OHOS::AudioStandard::AudioServerSetForegroundListFuzzTest,
    OHOS::AudioStandard::AudioServerCreatePlaybackCapturerManagerFuzzTest,
    OHOS::AudioStandard::AudioServerRegisterAudioCapturerSourceCallbackFuzzTest,
    OHOS::AudioStandard::AudioServerRegisterAudioRendererSinkCallbackFuzzTest,
    OHOS::AudioStandard::AudioServerGetMaxAmplitudeFuzzTest,
    OHOS::AudioStandard::AudioServerGetVolumeDataCountFuzzTest,
    OHOS::AudioStandard::AudioServerUpdateLatencyTimestampFuzzTest,
    OHOS::AudioStandard::AudioServerCheckHibernateStateFuzzTest,
    OHOS::AudioStandard::AudioServerCreateIpcOfflineStreamFuzzTest,
    OHOS::AudioStandard::AudioServerGetOfflineAudioEffectChainsFuzzTest,
    OHOS::AudioStandard::AudioServerGetStandbyStatusFuzzTest,
    OHOS::AudioStandard::AudioServerGenerateSessionIdFuzzTest,
    OHOS::AudioStandard::AudioServerNotifyAudioPolicyReadyFuzzTest,
    OHOS::AudioStandard::AudioServerGetAllSinkInputsFuzzTest,
    OHOS::AudioStandard::AudioServerReleaseCaptureLimitFuzzTest,
    OHOS::AudioStandard::AudioServerLoadHdiAdapterFuzzTest,
    OHOS::AudioStandard::AudioServerUnloadHdiAdapterFuzzTest,
    OHOS::AudioStandard::AudioServerCreateSinkPortFuzzTest,
    OHOS::AudioStandard::AudioServerCreateSourcePortFuzzTest,
    OHOS::AudioStandard::AudioServerDestroyHdiPortFuzzTest,
    OHOS::AudioStandard::AudioServerSetDeviceConnectedFlagFuzzTest,
    OHOS::AudioStandard::AudioServerSetBtHdiInvalidStateFuzzTest,
    OHOS::AudioStandard::AudioServerCreateAudioWorkgroupFuzzTest,
    OHOS::AudioStandard::AudioServerReleaseAudioWorkgroupFuzzTest,
    OHOS::AudioStandard::AudioServerAddThreadToGroupFuzzTest,
    OHOS::AudioStandard::AudioServerForceStopAudioStreamFuzzTest,
    OHOS::AudioStandard::AudioServerStartGroupFuzzTest,
    OHOS::AudioStandard::AudioServerStopGroupFuzzTest,
    OHOS::AudioStandard::AudioServerSetActiveOutputDeviceFuzzTest,
    OHOS::AudioStandard::AudioServerResetRecordConfigSourceTypeFuzzTest,
    OHOS::AudioStandard::AudioServerResetProcessConfigCallerUidFuzzTest,
    OHOS::AudioStandard::AudioServerCheckStreamInfoFormatNotContainFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRendererFormatNotContainFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRecorderFormatNotContainFuzzTest,
    OHOS::AudioStandard::AudioServerCreateAudioProcessInnerAudioModeFuzzTest,
#ifdef HAS_FEATURE_INNERCAPTURER
    OHOS::AudioStandard::AudioServerHandleCheckCaptureLimitFuzzTest,
    OHOS::AudioStandard::AudioServerInnerCheckCaptureLimitFuzzTest,
#endif
    OHOS::AudioStandard::AudioServerIsNormalIpcStreamFuzzTest,
    OHOS::AudioStandard::AudioServerCheckRemoteDeviceStateSwitchCaseFuzzTest,
    OHOS::AudioStandard::AudioServerCheckInnerRecorderPermissionSourceTypeFuzzTest,
    OHOS::AudioStandard::AudioServerSetRenderWhitelistFuzzTest,
    OHOS::AudioStandard::AudioServerOnMuteStateChangeFuzzTest
};
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint32_t len = OHOS::AudioStandard::GetArrLength(g_testPtrs);
    if (len > 0) {
        uint8_t firstByte = *data % len;
        if (firstByte >= len) {
            return 0;
        }
        data = data + 1;
        size = size - 1;
        g_testPtrs[firstByte](data, size);
    }
    return 0;
}