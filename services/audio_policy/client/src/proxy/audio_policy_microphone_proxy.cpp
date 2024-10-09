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
#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyProxy"
#endif

#include "audio_policy_proxy.h"

#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

int32_t AudioPolicyProxy::SetMicrophoneMute(bool isMute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteBool(isMute);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set microphoneMute failed, error: %d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetMicrophoneMuteAudioConfig(bool isMute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteBool(isMute);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_AUDIO_CONFIG), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set microphoneMute failed, error: %d", error);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");
    data.WriteBool(isMute);
    data.WriteInt32(static_cast<int32_t>(type));
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_PERSISTENT), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set microphoneMute persistent failed, error: %d", error);

    return reply.ReadInt32();
}

bool AudioPolicyProxy::IsMicrophoneMuteLegacy()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE_LEGACY), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set microphoneMute failed, error: %d", error);

    return reply.ReadBool();
}

bool AudioPolicyProxy::IsMicrophoneMute()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, -1, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "set microphoneMute failed, error: %d", error);

    return reply.ReadBool();
}

std::vector<sptr<MicrophoneDescriptor>> AudioPolicyProxy::GetAudioCapturerMicrophoneDescriptors(
    int32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<sptr<MicrophoneDescriptor>> micDescs;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, micDescs, "WriteInterfaceToken failed");

    data.WriteInt32(sessionId);
    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS),
        data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, micDescs,
        "Get audio capturer microphonedescriptors failed, error: %d", error);

    int32_t size = reply.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        micDescs.push_back(MicrophoneDescriptor::Unmarshalling(reply));
    }

    return micDescs;
}

std::vector<sptr<MicrophoneDescriptor>> AudioPolicyProxy::GetAvailableMicrophones()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<sptr<MicrophoneDescriptor>> micDescs;

    bool ret = data.WriteInterfaceToken(GetDescriptor());
    CHECK_AND_RETURN_RET_LOG(ret, micDescs, "WriteInterfaceToken failed");

    int32_t error = Remote()->SendRequest(
        static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_MICROPHONE_DESCRIPTORS), data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, micDescs,
        "Get available microphonedescriptors failed, error: %d", error);

    int32_t size = reply.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        micDescs.push_back(MicrophoneDescriptor::Unmarshalling(reply));
    }

    return micDescs;
}

} // namespace AudioStandard
} // namespace OHOS