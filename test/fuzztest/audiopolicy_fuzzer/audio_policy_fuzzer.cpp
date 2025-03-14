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
#include "audio_policy_server.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
constexpr int32_t OFFSET = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_ONE = 0;
const uint32_t LIMIT_TWO = 30;
const uint32_t LIMIT_THREE = 60;
const uint32_t LIMIT_FOUR = 90;
const uint32_t LIMIT_FIVE = 120;
const uint32_t LIMIT_SIX = static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX);
bool g_hasServerInit = false;
const uint8_t TESTSIZE = 5;
typedef void (*TestPtr)(const uint8_t *, size_t);

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

uint32_t Convert2Uint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    /* Move the 0th digit to the left by 24 bits, the 1st digit to the left by 16 bits,
       the 2nd digit to the left by 8 bits, and the 3rd digit not to the left */
    return (ptr[0] << SHIFT_LEFT_24) | (ptr[1] << SHIFT_LEFT_16) | (ptr[2] << SHIFT_LEFT_8) | (ptr[3]);
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

void ReleaseServer()
{
    GetServerPtr()->OnStop();
    g_hasServerInit = false;
}

void AudioPolicyFuzzFirstLimitTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_TWO - LIMIT_ONE + 1) + LIMIT_ONE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    if (code == static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL_WITH_DEVICE)) {
        return;
    }

    GetServerPtr()->OnRemoteRequest(code, data, reply, option);
    ReleaseServer();
}

void AudioPolicyFuzzSecondLimitTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_THREE - LIMIT_TWO + 1) + LIMIT_TWO;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(code, data, reply, option);
    ReleaseServer();
}

void AudioPolicyFuzzThirdLimitTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_FOUR - LIMIT_THREE + 1) + LIMIT_THREE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    if (code == static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_AUDIO_INTERRUPT_ZONE_PIDS)) {
        return;
    }

    GetServerPtr()->OnRemoteRequest(code, data, reply, option);
    ReleaseServer();
}

void AudioPolicyFuzzFouthLimitTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_FIVE - LIMIT_FOUR + 1) + LIMIT_FOUR;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(code, data, reply, option);
    ReleaseServer();
}

void AudioPolicyFuzzFifthLimitTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_SIX - LIMIT_FIVE + 1) + LIMIT_FIVE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    if (code == static_cast<uint32_t>(AudioPolicyInterfaceCode::EXCLUDE_OUTPUT_DEVICES) ||
        code == static_cast<uint32_t>(AudioPolicyInterfaceCode::UNEXCLUDE_OUTPUT_DEVICES)) {
        return;
    }

    GetServerPtr()->OnRemoteRequest(code, data, reply, option);
    ReleaseServer();
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

OHOS::AudioStandard::TestPtr g_testPtrs[OHOS::AudioStandard::TESTSIZE] = {
    OHOS::AudioStandard::AudioPolicyFuzzFirstLimitTest,
    OHOS::AudioStandard::AudioPolicyFuzzSecondLimitTest,
    OHOS::AudioStandard::AudioPolicyFuzzThirdLimitTest,
    OHOS::AudioStandard::AudioPolicyFuzzFouthLimitTest,
    OHOS::AudioStandard::AudioPolicyFuzzFifthLimitTest
};

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint8_t firstByte = *data % OHOS::AudioStandard::TESTSIZE;
    if (firstByte >= OHOS::AudioStandard::TESTSIZE) {
        return 0;
    }
    data = data + 1;
    size = size - 1;
    g_testPtrs[firstByte](data, size);
    return 0;
}
