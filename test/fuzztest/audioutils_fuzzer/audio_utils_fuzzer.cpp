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

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const int64_t LIMIT_TIME = 1;
const uint32_t ENUMSIZE = 4;
const uint64_t COMMON_UINT64_NUM = 2;
const int64_t COMMON_INT64_NUM = 2;
bool g_hasPermission = false;

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

void GetCurNanoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    ClockTime::GetCurNano();
}
void AbsoluteSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int64_t nanoTime = COMMON_INT64_NUM;
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::AbsoluteSleep(nanoTime);
}

void RelativeSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int64_t nanoTime = COMMON_INT64_NUM;
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::RelativeSleep(nanoTime);
}

void CountFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int64_t count = COMMON_INT64_NUM;
    const std::string value = "value";
    Trace::Count(value, count);
}

void CountVolumeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint8_t data = *(reinterpret_cast<const uint8_t*>(rawData));
    const std::string value = "value";
    Trace::CountVolume(value, data);
}

bool VerifySystemPermissionFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return false;
    }

    return PermissionUtil::VerifySystemPermission();
}

void VerifyPermissionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t tokenId = *reinterpret_cast<const uint32_t *>(rawData);
    PermissionUtil::VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION, tokenId);
}

void VerifyIsShellFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    PermissionUtil::VerifyIsShell();
}

void VerifyIsSystemAppFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    PermissionUtil::VerifyIsSystemApp();
}


void NeedVerifyBackgroundCaptureFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t callingUid = *(reinterpret_cast<const int32_t*>(rawData));
    SourceType sourceType = *(reinterpret_cast<const SourceType*>(rawData));
    PermissionUtil::NeedVerifyBackgroundCapture(callingUid, sourceType);
}

void VerifyBackgroundCaptureFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t tokenId = *(reinterpret_cast<const uint32_t*>(rawData));
    uint64_t fullTokenId = COMMON_UINT64_NUM;

    PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId);
}

void NotifyPrivacyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t targetTokenId = *(reinterpret_cast<const uint32_t*>(rawData));
    PermissionUtil::NotifyStart(targetTokenId, 0);
    PermissionUtil::NotifyStop(targetTokenId, 0);
}

void GetTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetTime();
}

void AudioBlendFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<AudioBlend> audioBlend = nullptr;
    audioBlend = std::make_shared<AudioBlend>();
    uint32_t blendMode_int = *reinterpret_cast<const uint32_t*>(rawData);
    blendMode_int = blendMode_int % ENUMSIZE;
    ChannelBlendMode blendMode = static_cast<ChannelBlendMode>(blendMode_int);
    uint8_t format = *reinterpret_cast<const uint8_t*>(rawData);
    format = format % ENUMSIZE;
    uint8_t channel = *reinterpret_cast<const uint8_t*>(rawData);
    audioBlend->SetParams(blendMode, format, channel);
    uint8_t *buffer = new uint8_t[LIMITSIZE];
    memcpy_s(buffer, LIMITSIZE, rawData, LIMITSIZE);
    audioBlend->Process(buffer, LIMITSIZE);
    delete[] buffer;
}

void VolumeRampFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::shared_ptr<VolumeRamp> volumeRamp = nullptr;
    volumeRamp = std::make_shared<VolumeRamp>();
    float targetVolume = *reinterpret_cast<const float*>(rawData);
    float currStreamVolume = *reinterpret_cast<const float*>(rawData);
    int32_t duration = *reinterpret_cast<const int32_t*>(rawData);
    volumeRamp->SetVolumeRampConfig(targetVolume, currStreamVolume, duration);
    volumeRamp->GetRampVolume();
    volumeRamp->IsActive();
    volumeRamp->Terminate();
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::GetCurNanoFuzzTest(data, size);
    OHOS::AudioStandard::AbsoluteSleepFuzzTest(data, size);
    OHOS::AudioStandard::RelativeSleepFuzzTest(data, size);
    OHOS::AudioStandard::CountFuzzTest(data, size);
    OHOS::AudioStandard::CountVolumeFuzzTest(data, size);
    OHOS::AudioStandard::GetTimeFuzzTest(data, size);
    OHOS::AudioStandard::AudioBlendFuzzTest(data, size);
    OHOS::AudioStandard::VolumeRampFuzzTest(data, size);
    return 0;
}
