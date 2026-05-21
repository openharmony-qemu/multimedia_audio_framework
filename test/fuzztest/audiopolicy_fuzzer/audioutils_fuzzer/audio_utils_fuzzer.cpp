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
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const int64_t LIMIT_TIME = 1;
const uint32_t ENUMSIZE = 4;
const uint64_t COMMON_UINT64_NUM = 2;
const int64_t COMMON_INT64_NUM = 2;
const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
bool g_hasPermission = false;

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<ChannelBlendMode> g_testBlendModes = {
    CHANNEL_BLEND_NORMAL,
    CHANNEL_BLEND_DUALMONO_L_R,
    CHANNEL_BLEND_DUALMONO_R_L,
    CHANNEL_BLEND_DUALMONO_LR,
};

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
};

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

void AbsoluteSleepFuzzTest(FuzzedDataProvider& fdp)
{
    int64_t nanoTime = fdp.ConsumeIntegralInRange<int64_t>(0, LIMIT_TIME);
    ClockTime::AbsoluteSleep(nanoTime);
}

void RelativeSleepFuzzTest(FuzzedDataProvider& fdp)
{
    int64_t nanoTime = fdp.ConsumeIntegralInRange<int64_t>(0, LIMIT_TIME);
    ClockTime::RelativeSleep(nanoTime);
}

void CountFuzzTest(FuzzedDataProvider& fdp)
{
    int64_t count = fdp.ConsumeIntegral<int64_t>();
    std::string value = ConsumeString(fdp, "value");
    Trace::Count(value, count);
}

void CountVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    uint8_t data = fdp.ConsumeIntegral<uint8_t>();
    std::string value = ConsumeString(fdp, "value");
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

void GetTimeFuzzTest(FuzzedDataProvider& fdp)
{
    int64_t nanoTime = fdp.ConsumeIntegral<int64_t>();
    GetTime();
}

void AudioBlendFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioBlend> audioBlend = nullptr;
    audioBlend = std::make_shared<AudioBlend>();
    ChannelBlendMode blendMode = ConsumeEnum<ChannelBlendMode>(fdp);
    uint8_t format = fdp.ConsumeIntegral<uint8_t>();
    uint8_t channel = fdp.ConsumeIntegral<uint8_t>();
    audioBlend->SetParams(blendMode, format, channel);
    std::vector<uint8_t> bufferData = fdp.ConsumeBytes<uint8_t>(LIMITSIZE);
    if (bufferData.size() < LIMITSIZE) {
        return;
    }
    audioBlend->Process(bufferData.data(), LIMITSIZE);
}

void VolumeRampFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<VolumeRamp> volumeRamp = nullptr;
    volumeRamp = std::make_shared<VolumeRamp>();
    float targetVolume = fdp.ConsumeFloatingPoint<float>();
    float currStreamVolume = fdp.ConsumeFloatingPoint<float>();
    int32_t duration = fdp.ConsumeIntegral<int32_t>();
    volumeRamp->SetVolumeRampConfig(targetVolume, currStreamVolume, duration);
    volumeRamp->GetRampVolume();
    volumeRamp->IsActive();
    volumeRamp->Terminate();
}

void GetSourceNameFuzzTest(FuzzedDataProvider& fdp)
{
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    AudioInfoDumpUtils::GetSourceName(sourceType);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AbsoluteSleepFuzzTest,
    RelativeSleepFuzzTest,
    CountFuzzTest,
    CountVolumeFuzzTest,
    GetTimeFuzzTest,
    AudioBlendFuzzTest,
    VolumeRampFuzzTest,
    GetSourceNameFuzzTest,
    });
    func(fdp);
}
void Init()
{
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