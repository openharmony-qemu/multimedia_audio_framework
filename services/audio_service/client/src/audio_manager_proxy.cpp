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
#define LOG_TAG "AudioManagerProxy"
#endif

#include "audio_manager_proxy.h"

#include <cinttypes>

#include <audio_errors.h>
#include "audio_system_manager.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "i_audio_process.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t MAX_OFFLINE_EFFECT_CHAIN_NUM = 10;
const size_t DEFAULT_MAX_RENDERER_INSTANCES = 1000;
}
AudioManagerProxy::AudioManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioService>(impl)
{
}

int32_t AudioManagerProxy::SetMicrophoneMute(bool isMute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "AudioManagerProxy: WriteInterfaceToken failed");
    data.WriteBool(isMute);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "SetMicrophoneMute failed, error: %d", error);

    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::SetVoiceVolume(float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteFloat(volume);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false,
        "SetVoiceVolume failed, error: %d", error);

    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::OffloadSetVolume(float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteFloat(volume);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "OffloadSetVolume failed, error: %d", error);

    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeOutputDevices,
    DeviceType activeInputDevice, BluetoothOffloadState a2dpOffloadFlag)
{
    CHECK_AND_RETURN_RET_LOG(!activeOutputDevices.empty() &&
        activeOutputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_NONE, "Invalid active output devices.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(audioScene));

    data.WriteInt32(static_cast<int32_t>(activeOutputDevices.size()));
    for (auto activeOutputDevice : activeOutputDevices) {
        data.WriteInt32(static_cast<int32_t>(activeOutputDevice));
    }
    data.WriteInt32(static_cast<int32_t>(activeInputDevice));
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "SetAudioScene failed, error: %d", error);

    int32_t result = reply.ReadInt32();
    return result;
}

const std::string AudioManagerProxy::GetAudioParameter(const std::string &key)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, "", "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(key));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("Get audio parameter failed, error: %d", error);
        const std::string value = "";
        return value;
    }

    const std::string value = reply.ReadString();
    return value;
}

const std::string AudioManagerProxy::GetAudioParameter(const std::string& networkId, const AudioParamKey key,
    const std::string& condition)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, "", "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(networkId));
    data.WriteInt32(static_cast<int32_t>(key));
    data.WriteString(static_cast<std::string>(condition));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER), data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("Get audio parameter failed, error: %d", error);
        const std::string value = "";
        return value;
    }

    const std::string value = reply.ReadString();
    return value;
}

int32_t AudioManagerProxy::GetExtraParameters(const std::string &mainKey, const std::vector<std::string> &subKeys,
    std::vector<std::pair<std::string, std::string>> &result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(mainKey));
    data.WriteInt32(static_cast<int32_t>(subKeys.size()));
    for (std::string subKey : subKeys) {
        data.WriteString(static_cast<std::string>(subKey));
    }
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Get extra audio parameters failed, error: %d", error);

    int32_t num = reply.ReadInt32();
    for (auto i = 0; i < num; i++) {
        const std::string key = reply.ReadString();
        const std::string value = reply.ReadString();
        result.push_back(std::make_pair(key, value));
    }
    return reply.ReadInt32();
}

void AudioManagerProxy::SetAudioParameter(const std::string &key, const std::string &value)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(key));
    data.WriteString(static_cast<std::string>(value));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Get audio parameter failed, error: %d", error);
}

void AudioManagerProxy::SetAudioParameter(const std::string& networkId, const AudioParamKey key,
    const std::string& condition, const std::string& value)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(networkId));
    data.WriteInt32(static_cast<int32_t>(key));
    data.WriteString(static_cast<std::string>(condition));
    data.WriteString(static_cast<std::string>(value));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Get audio parameter failed, error: %d", error);
}

int32_t AudioManagerProxy::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(static_cast<std::string>(key));
    data.WriteInt32(static_cast<int32_t>(kvpairs.size()));
    for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
        data.WriteString(static_cast<std::string>(it->first));
        data.WriteString(static_cast<std::string>(it->second));
    }

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Set extra audio parameters failed, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::SetAsrAecMode(AsrAecMode asrAecMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrAecMode));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_AEC_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::GetAsrAecMode(AsrAecMode &asrAecMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrAecMode));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    asrAecMode = static_cast<AsrAecMode>(result);
    return 0;
}

int32_t AudioManagerProxy::SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode asrNoiseSuppressionMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrNoiseSuppressionMode));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_NOISE_SUPPRESSION_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrNoiseSuppressionMode));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    asrNoiseSuppressionMode = static_cast<AsrNoiseSuppressionMode>(result);
    return 0;
}

int32_t AudioManagerProxy::SetAsrWhisperDetectionMode(AsrWhisperDetectionMode asrWhisperDetectionMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrWhisperDetectionMode));

    int32_t error = Remote()->SendRequest(static_cast<uint32_t>(
        AudioServerInterfaceCode::SET_ASR_WHISPER_DETECTION_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrWhisperDetectionMode));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_WHISPER_DETECTION_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    asrWhisperDetectionMode = static_cast<AsrWhisperDetectionMode>(result);
    return 0;
}

int32_t AudioManagerProxy::SetAsrVoiceControlMode(AsrVoiceControlMode asrVoiceControlMode, bool on)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrVoiceControlMode));
    data.WriteBool(on);

    int32_t error = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_CONTROL_MODE),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::SetAsrVoiceMuteMode(AsrVoiceMuteMode asrVoiceMuteMode, bool on)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(asrVoiceMuteMode));
    data.WriteBool(on);

    int32_t error = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_MUTE_MODE),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, -1, "failed,error:%d", error);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t AudioManagerProxy::IsWhispering()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::IS_WHISPERING), data, reply, option);
    return result;
}

bool AudioManagerProxy::GetEffectOffloadEnabled()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_OFFLOAD_ENABLED), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "failed,error:%d", error);
    bool result = reply.ReadBool();
    return result;
}

uint64_t AudioManagerProxy::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t transactionId = 0;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, transactionId, "WriteInterfaceToken failed");

    data.WriteInt32(static_cast<int32_t>(deviceType));
    data.WriteInt32(static_cast<int32_t>(deviceRole));

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_TRANSACTION_ID), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, transactionId, "get transaction id failed, error: %d", error);

    transactionId = reply.ReadUint64();

    return transactionId;
}

void AudioManagerProxy::NotifyDeviceInfo(std::string networkId, bool connected)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteString(networkId);
    data.WriteBool(connected);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Get audio parameter failed, error: %d", error);
}

int32_t AudioManagerProxy::CheckRemoteDeviceState(std::string networkId, DeviceRole deviceRole, bool isStartDevice)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERR_TRANSACTION_FAILED, "WriteInterfaceToken failed");
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(deviceRole));
    data.WriteBool(isStartDevice);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "CheckRemoteDeviceState failed in proxy, error: %d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::UpdateActiveDeviceRoute(DeviceType type, DeviceFlag flag,
    BluetoothOffloadState a2dpOffloadFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteInt32(type);
    data.WriteInt32(flag);
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    auto error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTE_REQ), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "UpdateActiveDeviceRoute failed, error: %{public}d", error);

    auto result = reply.ReadInt32();
    AUDIO_DEBUG_LOG("[UPDATE_ROUTE_REQ] result %{public}d", result);
    return result;
}

int32_t AudioManagerProxy::UpdateActiveDevicesRoute(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
    BluetoothOffloadState a2dpOffloadFlag, const std::string &deviceName)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_NONE, "Invalid active output devices.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(activeDevices.size()));
    for (auto it = activeDevices.begin(); it != activeDevices.end(); it++) {
        data.WriteInt32(static_cast<int32_t>(it->first));
        data.WriteInt32(static_cast<int32_t>(it->second));
    }
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));
    data.WriteString(deviceName);

    auto error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "UpdateActiveDevicesRoute failed, error: %{public}d", error);

    auto result = reply.ReadInt32();
    AUDIO_DEBUG_LOG("[UPDATE_ROUTES_REQ] result %{public}d", result);
    return result;
}

int32_t AudioManagerProxy::UpdateDualToneState(bool enable, int32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    data.WriteBool(enable);
    data.WriteInt32(sessionId);
    auto error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "UpdateDualToneState failed, error: %{public}d", error);

    auto result = reply.ReadInt32();
    AUDIO_DEBUG_LOG("[UPDATE_DUAL_TONE_REQ] result %{public}d", result);
    return result;
}

int32_t AudioManagerProxy::SetParameterCallback(const sptr<IRemoteObject>& object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "SetParameterCallback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::RegiestPolicyProvider(const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::REGISET_POLICY_PROVIDER), data,
        reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "RegiestPolicyProvider failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::SetWakeupSourceCallback(const sptr<IRemoteObject>& object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_NULL_OBJECT, "object is null");
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_WAKEUP_CLOSE_CALLBACK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "SetWakeupCloseCallback failed, error: %{public}d", error);

    return reply.ReadInt32();
}

void AudioManagerProxy::SetAudioMonoState(bool audioMono)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    (void)data.WriteBool(audioMono);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "SetAudioMonoState failed, error: %{public}d", error);
}

void AudioManagerProxy::SetAudioBalanceValue(float audioBalance)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    (void)data.WriteFloat(audioBalance);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "SetAudioBalanceValue failed, error: %{public}d", error);
}

sptr<IRemoteObject> AudioManagerProxy::CreateAudioProcess(const AudioProcessConfig &config, int32_t &errorCode,
    const AudioPlaybackCaptureConfig &filterConfig)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, nullptr, "WriteInterfaceToken failed");
    ProcessConfig::WriteConfigToParcel(config, data);
    ProcessConfig::WriteInnerCapConfigToParcel(filterConfig, data);
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOPROCESS), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, nullptr, "CreateAudioProcess failed, error: %{public}d", error);
    errorCode = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(errorCode == SUCCESS, nullptr, "errcode: %{public}d", errorCode);
    sptr<IRemoteObject> process = reply.ReadRemoteObject();
    return process;
}

bool AudioManagerProxy::LoadAudioEffectLibraries(const vector<Library> libraries, const vector<Effect> effects,
    vector<Effect> &successEffects)
{
    int32_t error;
    int32_t i;

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    bool ret = dataParcel.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, false, "WriteInterfaceToken failed");

    uint32_t countLib = libraries.size();
    uint32_t countEff = effects.size();

    dataParcel.WriteInt32(countLib);
    dataParcel.WriteInt32(countEff);

    for (Library x : libraries) {
        dataParcel.WriteString(x.name);
        dataParcel.WriteString(x.path);
    }

    for (Effect x : effects) {
        dataParcel.WriteString(x.name);
        dataParcel.WriteString(x.libraryName);
    }

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES), dataParcel, replyParcel, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false, "LoadAudioEffectLibraries failed, error: %{public}d", error);

    int32_t successEffSize = replyParcel.ReadInt32();
    CHECK_AND_RETURN_RET_LOG((successEffSize >= 0) && (successEffSize <= AUDIO_EFFECT_COUNT_UPPER_LIMIT),
        false, "LOAD_AUDIO_EFFECT_LIBRARIES read replyParcel failed");

    for (i = 0; i < successEffSize; i++) {
        string effectName = replyParcel.ReadString();
        string libName = replyParcel.ReadString();
        successEffects.push_back({effectName, libName});
    }

    return true;
}
static void MarshallEffectChainMgrParam(const EffectChainManagerParam &effectChainMgrParam, MessageParcel &data)
{
    data.WriteInt32(effectChainMgrParam.maxExtraNum);
    data.WriteString(effectChainMgrParam.defaultSceneName);
    data.WriteInt32(effectChainMgrParam.priorSceneList.size());
    for (const auto &priorScene : effectChainMgrParam.priorSceneList) {
        data.WriteString(priorScene);
    }

    data.WriteInt32(effectChainMgrParam.sceneTypeToChainNameMap.size());
    for (const auto &[scene, chain] : effectChainMgrParam.sceneTypeToChainNameMap) {
        data.WriteString(scene);
        data.WriteString(chain);
    }

    data.WriteInt32(effectChainMgrParam.effectDefaultProperty.size());
    for (const auto &[effect, prop] : effectChainMgrParam.effectDefaultProperty) {
        data.WriteString(effect);
        data.WriteString(prop);
    }
}

bool AudioManagerProxy::CreateEffectChainManager(std::vector<EffectChain> &effectChains,
    const EffectChainManagerParam &effectParam, const EffectChainManagerParam &enhanceParam)
{
    int32_t error;

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;

    bool ret = dataParcel.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, false, "WriteInterfaceToken failed");

    uint32_t countEffectChains = effectChains.size();
    std::vector<int32_t> listCountEffects;

    for (EffectChain &effectChain: effectChains) {
        listCountEffects.emplace_back(effectChain.apply.size());
    }

    dataParcel.WriteInt32(countEffectChains);
    for (int32_t countEffects: listCountEffects) {
        dataParcel.WriteInt32(countEffects);
    }

    for (EffectChain &effectChain: effectChains) {
        dataParcel.WriteString(effectChain.name);
        for (std::string applyName: effectChain.apply) {
            dataParcel.WriteString(applyName);
        }
    }

    MarshallEffectChainMgrParam(effectParam, dataParcel);
    MarshallEffectChainMgrParam(enhanceParam, dataParcel);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIO_EFFECT_CHAIN_MANAGER),
        dataParcel, replyParcel, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false,
        "CreateAudioEffectChainManager failed, error: %{public}d", error);
    return true;
}

void AudioManagerProxy::SetOutputDeviceSink(int32_t deviceType, std::string &sinkName)
{
    int32_t error;

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    bool ret = dataParcel.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    dataParcel.WriteInt32(deviceType);
    dataParcel.WriteString(sinkName);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK), dataParcel, replyParcel, option);
        CHECK_AND_RETURN_LOG(error == ERR_NONE, "SetOutputDeviceSink failed, error: %{public}d", error);
    return;
}

void AudioManagerProxy::SetActiveOutputDevice(DeviceType deviceType)
{
    int32_t error;

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    bool ret = dataParcel.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    dataParcel.WriteInt32(static_cast<int32_t>(deviceType));

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_ACTIVE_OUTPUT_DEVICE), dataParcel, replyParcel, option);
        CHECK_AND_RETURN_LOG(error == ERR_NONE, "SetActiveOutputDevice failed, error: %{public}d", error);
    return;
}

bool AudioManagerProxy::CreatePlaybackCapturerManager()
{
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, false, "WriteInterfaceToken failed");

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, false,
        "CreatePlaybackCapturerManager failed, error: %{public}d", error);

    return reply.ReadBool();
#else
    return false;
#endif
}

int32_t AudioManagerProxy::NotifyStreamVolumeChanged(AudioStreamType streamType, float volume)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("NotifyStreamVolumeChanged: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(static_cast<int32_t>(streamType));
    data.WriteFloat(volume);
    error = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED),
        data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("NotifyStreamVolumeChanged failed, error: %{public}d", error);
        return error;
    }
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "NotifyStreamVolumeChanged failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::UpdateSpatializationState(AudioSpatializationState spatializationState)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteBool(spatializationState.spatializationEnabled);
    data.WriteBool(spatializationState.headTrackingEnabled);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIALIZATION_STATE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error,
        "UpdateSpatializationState failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(spatialDeviceType);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "UpdateSpatialDeviceType failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(spatializationSceneType));

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::ResetRouteForDisconnect(DeviceType type)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(type));

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "error: %{public}d", error);

    return reply.ReadInt32();
}

uint32_t AudioManagerProxy::GetEffectLatency(const std::string &sessionId)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(sessionId);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "error: %{public}d", error);

    return reply.ReadUint32();
}

float AudioManagerProxy::GetMaxAmplitude(bool isOutputDevice, std::string deviceClass, SourceType sourceType)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteBool(isOutputDevice);
    data.WriteString(deviceClass);
    data.WriteInt32(static_cast<int32_t>(sourceType));

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);

    return reply.ReadFloat();
}

void AudioManagerProxy::ResetAudioEndpoint()
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::RESET_AUDIO_ENDPOINT), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "Send request failed, error:%{public}d", error);
}

int32_t AudioManagerProxy::SuspendRenderSink(const std::string &sinkName)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(sinkName);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SUSPEND_RENDERSINK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);

    return reply.ReadInt32();
}

int32_t AudioManagerProxy::RestoreRenderSink(const std::string &sinkName)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(sinkName);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_RENDERSINK), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);

    return reply.ReadInt32();
}

void AudioManagerProxy::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteString(timestamp);
    data.WriteBool(isRenderer);

    error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE,
        "LatencyMeas UpdateLatencyTimestamp failed, error:%{public}d", error);
}

int32_t AudioManagerProxy::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray,
    const DeviceType& deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool res = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(res, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(deviceType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Get Audio Effect Property, error: %d", error);

    int32_t size = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "get audio effect property size invalid.");
    for (int32_t i = 0; i < size; i++) {
        // write and read must keep same order
        AudioEffectPropertyV3 prop = {};
        prop.Unmarshalling(reply);
        propertyArray.property.push_back(prop);
    }
    return AUDIO_OK;
}

int32_t AudioManagerProxy::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    const DeviceType& deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");

    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "set audio effect property size invalid.");
    data.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        // write and read must keep same order
        propertyArray.property[i].Marshalling(data);
    }
    data.WriteInt32(static_cast<int32_t>(deviceType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool res = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(res, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");
    data.WriteInt32(static_cast<int32_t>(deviceType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Get Audio Enhance Property, error: %d", error);

    int32_t size = reply.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        // write and read must keep same order
        AudioEnhanceProperty prop = {};
        prop.Unmarshalling(reply);
        propertyArray.property.push_back(prop);
    }
    return AUDIO_OK;
}

int32_t AudioManagerProxy::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool res = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(res, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "Get Audio Effect Property, error: %d", error);

    int32_t size = reply.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        AudioEffectProperty prop = {};
        prop.Unmarshalling(reply);
        // write and read must keep same order
        propertyArray.property.push_back(prop);
    }
    return AUDIO_OK;
}

int32_t AudioManagerProxy::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
    DeviceType deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");

    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    data.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        // write and read must keep same order
        propertyArray.property[i].Marshalling(data);
    }
    data.WriteInt32(static_cast<int32_t>(deviceType));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_OPERATION, "WriteInterfaceToken failed");

    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    data.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(data);
    }
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SendRequest failed, error: %{public}d", error);
    return reply.ReadInt32();
}

void AudioManagerProxy::LoadHdiEffectModel()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_EFFECT_MODEL), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed,error:%d", error);
}

void AudioManagerProxy::UpdateEffectBtOffloadSupported(const bool &isSupported)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteBool(isSupported);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed, error:%{public}d", error);
}

int32_t AudioManagerProxy::SetSinkMuteForSwitchDevice(const std::string &devceClass, int32_t durationUs, bool mute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteString(devceClass);
    data.WriteInt32(durationUs);
    data.WriteInt32(mute);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINK_MUTE_FOR_SWITCH_DEVICE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "failed, error:%{public}d", error);
    return reply.ReadInt32();
}

void AudioManagerProxy::SetRotationToEffect(const uint32_t rotate)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteUint32(rotate);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_ROTATION_TO_EFFECT), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed, error:%{public}d", error);
}

void AudioManagerProxy::UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteInt32(sessionID);
    data.WriteInt32(state);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SESSION_CONNECTION_STATE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed, error:%{public}d", error);
}

void AudioManagerProxy::SetNonInterruptMute(const uint32_t sessionId, const bool muteFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteUint32(sessionId);
    data.WriteBool(muteFlag);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINGLE_STREAM_MUTE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed, error:%{public}d", error);
}

int32_t AudioManagerProxy::SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "AudioManagerProxy: WriteInterfaceToken failed");
    data.WriteUint32(sessionId);
    data.WriteInt32(state);
    data.WriteBool(isAppBack);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_OFFLOAD_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "SetOffloadMode failed, error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t AudioManagerProxy::UnsetOffloadMode(uint32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "AudioManagerProxy: WriteInterfaceToken failed");
    data.WriteUint32(sessionId);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UNSET_OFFLOAD_MODE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "UnsetOffloadMode failed, error: %{public}d", error);
    return reply.ReadInt32();
}

void AudioManagerProxy::RestoreSession(const uint32_t &sessionID, RestoreInfo restoreInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteUint32(sessionID);
    data.WriteInt32(restoreInfo.restoreReason);
    data.WriteInt32(restoreInfo.deviceChangeReason);
    data.WriteInt32(restoreInfo.targetStreamFlag);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_SESSION), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed, error:%{public}d", error);
}

sptr<IRemoteObject> AudioManagerProxy::CreateIpcOfflineStream(int32_t &errorCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, nullptr, "WriteInterfaceToken failed");
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_IPC_OFFLINE_STREAM), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, nullptr, "CreateIpcOfflineStream failed, error: %{public}d", error);
    sptr<IRemoteObject> process = reply.ReadRemoteObject();
    errorCode = reply.ReadInt32();
    return process;
}

int32_t AudioManagerProxy::GetOfflineAudioEffectChains(vector<string> &effectChains)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, AUDIO_ERR, "WriteInterfaceToken failed");
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_OFFLINE_AUDIO_EFFECT_CHAINS), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "GetOfflineAudioEffectChains failed, error: %{public}d", error);
    int32_t vecSize = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(vecSize >= 0 && vecSize <= MAX_OFFLINE_EFFECT_CHAIN_NUM, AUDIO_ERR,
        "invalid offline effect chain num:%{public}d", vecSize);
    for (int i = 0; i < vecSize; i++) {
        effectChains.emplace_back(reply.ReadString());
    }
    return reply.ReadInt32();
}

void AudioManagerProxy::CheckHibernateState(bool onHibernate)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "AudioManagerProxy: WriteInterfaceToken failed");
    data.WriteBool(onHibernate);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_HIBERNATE_STATE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "CheckHibernateState failed, error: %{public}d", error);
    return;
}

int32_t AudioManagerProxy::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int32_t result = ERROR;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, result, "WriteInterfaceToken failed");

    data.WriteUint32(sessionId);

    int32_t error = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_STANDBY_STATUS), data,
        reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, result, "get transaction id failed, error: %d", error);

    result = reply.ReadInt32();
    isStandby = reply.ReadBool();
    enterStandbyTime = reply.ReadInt64();

    return result;
}

int32_t AudioManagerProxy::GenerateSessionId(uint32_t &sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, AUDIO_ERR, "WriteInterfaceToken failed");
    data.WriteUint32(sessionId);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GENERATE_SESSION_ID), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "generate sessionid failed,error:%{public}d", error);
    sessionId = reply.ReadUint32();
    return 0;
}

void AudioManagerProxy::NotifyAccountsChanged()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_ACCOUNTS_CHANGED), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed,error:%d", error);
}

void AudioManagerProxy::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::GET_ALL_SINK_INPUTS), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "getallsinkinputs failed, error: %{public}d", error);
    size_t size = reply.ReadUint64();
    CHECK_AND_RETURN_LOG(size <= DEFAULT_MAX_RENDERER_INSTANCES, "getallsinkinputs failed, size: %{public}zu", size);
    while (size > 0) {
        SinkInput sinkInput;
        sinkInput.Unmarshalling(reply);
        sinkInputs.push_back(sinkInput);
        size--;
    }
}

void AudioManagerProxy::SetDefaultAdapterEnable(bool isEnable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "AudioManagerProxy: WriteInterfaceToken failed");
    data.WriteBool(isEnable);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::SET_DEFAULT_ADAPTER_ENABLE), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "SetDefaultAdapterEnable failed, error: %{public}d", error);
    return;
}

void AudioManagerProxy::NotifyAudioPolicyReady()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_AUDIO_POLICY_READY), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed,error:%d", error);
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioManagerProxy::SetInnerCapLimit(uint32_t innerCapLimit)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), AUDIO_ERR, "Write descriptor failed!");
    data.WriteUint32(innerCapLimit);
    int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_CAPTURE_LIMIT),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "Failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

// for DT test
int32_t AudioManagerProxy::CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), AUDIO_ERR, "Write descriptor failed!");
    ProcessConfig::WriteInnerCapConfigToParcel(config, data);
    int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_CAPTURE_LIMIT),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "Failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}

// for DT test
int32_t AudioManagerProxy::ReleaseCaptureLimit(int32_t innerCapId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), AUDIO_ERR, "Write descriptor failed!");
    data.WriteInt32(innerCapId);
    int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RELEASE_CAPTURE_LIMIT),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "Failed, ipc error: %{public}d", ret);
    return reply.ReadInt32();
}
#endif

int32_t AudioManagerProxy::LoadHdiAdapter(uint32_t devMgrType, const std::string &adapterName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, AUDIO_ERR, "WriteInterfaceToken failed");
    data.WriteUint32(devMgrType);
    data.WriteString(adapterName);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_ADAPTER), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "LoadHdiAdapter failed, error: %{public}d", error);
    return reply.ReadInt32();
}

void AudioManagerProxy::UnloadHdiAdapter(uint32_t devMgrType, const std::string &adapterName, bool force)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");
    data.WriteUint32(devMgrType);
    data.WriteString(adapterName);
    data.WriteBool(force);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::UNLOAD_HDI_ADAPTER), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "UnloadHdiAdapter failed, error: %{public}d", error);
}

void AudioManagerProxy::SetDeviceConnectedFlag(bool flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    data.WriteBool(flag);

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::DEVICE_CONNECTED_FLAG), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed,error:%d", error);
}

void AudioManagerProxy::NotifySettingsDataReady()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_LOG(ret, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_SETTINGS_DATA_READY), data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "failed,error:%d", error);
}
} // namespace AudioStandard
} // namespace OHOS
