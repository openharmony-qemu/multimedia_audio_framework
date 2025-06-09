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
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "message_parcel.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t OFFSET = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
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
    "live_effect",
    "test",
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
        sptr<AudioPolicyManagerListenerStub> focusListenerStub = new(std::nothrow) AudioPolicyManagerListenerStub();
        sptr<IRemoteObject> object = focusListenerStub->AsObject();
        AudioServerPtr->SetParameterCallback(object);
        return;
    }
    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE)) {
        AsrAecMode asrAecMode = (static_cast<AsrAecMode>(0));
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
    DeviceType outputDevice = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceType inputDevice = *reinterpret_cast<const DeviceType*>(rawData);
    data.WriteInt32(static_cast<int32_t>(audioScene));
    data.WriteInt32(static_cast<int32_t>(outputDevice));
    data.WriteInt32(static_cast<int32_t>(inputDevice));

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

    std::shared_ptr<AudioServer> audioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetUsbParameter("address=card2;device=0 role=1");
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

void AudioServerSetA2dpAudioParameterTest(const uint8_t *rawData, size_t size)
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
    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys;
    audioServerPtr->GetExtraParameters(g_testKeys[id], subKeys, result);
}

void AudioServerGetDPParameterTest(const uint8_t *rawData, size_t size)
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
    uint32_t id = static_cast<uint32_t>(size) % audioParamKey.size();
    AudioParamKey key = static_cast<AudioParamKey>(audioParamKey[id]);
    id = static_cast<uint32_t>(size) % tetsNetworkId.size();
    audioServerPtr->GetAudioParameter(tetsNetworkId[id], key, "");
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioServerDumpTest,
    OHOS::AudioStandard::AudioServerGetUsbParameterTest,
    OHOS::AudioStandard::AudioServerOnAddSystemAbilityTest,
    OHOS::AudioStandard::AudioServerSetExtraParametersTest,
    OHOS::AudioStandard::AudioServerSetA2dpAudioParameterTest,
    OHOS::AudioStandard::AudioServerGetExtraParametersTest,
    OHOS::AudioStandard::AudioServerGetDPParameterTest,
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