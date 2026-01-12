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
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(g_data) which size is according to sizeof(T)
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

void AbsoluteSleepFuzzTest()
{
    int64_t nanoTime = COMMON_INT64_NUM;
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::AbsoluteSleep(nanoTime);
}

void RelativeSleepFuzzTest()
{
    int64_t nanoTime = COMMON_INT64_NUM;
    if (nanoTime > LIMIT_TIME) {
        nanoTime = LIMIT_TIME;
    }
    ClockTime::RelativeSleep(nanoTime);
}

void CountFuzzTest()
{
    int64_t count = COMMON_INT64_NUM;
    const std::string value = "value";
    Trace::Count(value, count);
}

void CountVolumeFuzzTest()
{
    uint8_t data = GetData<uint8_t>();
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
    PermissionUtil::NotifyPrivacyStart(targetTokenId, 0);
    PermissionUtil::NotifyPrivacyStop(targetTokenId, 0);
}

void GetTimeFuzzTest()
{
    GetTime();
}

void AudioBlendFuzzTest()
{
    std::shared_ptr<AudioBlend> audioBlend = nullptr;
    audioBlend = std::make_shared<AudioBlend>();
    uint32_t blendModeInt = GetData<uint32_t>();
    blendModeInt = blendModeInt % ENUMSIZE;
    ChannelBlendMode blendMode = static_cast<ChannelBlendMode>(blendModeInt);
    uint8_t format = GetData<uint8_t>();
    format = format % ENUMSIZE;
    uint8_t channel = GetData<uint8_t>();
    audioBlend->SetParams(blendMode, format, channel);
    uint8_t *buffer = new uint8_t[LIMITSIZE];
    memcpy_s(buffer, LIMITSIZE, RAW_DATA, LIMITSIZE);
    audioBlend->Process(buffer, LIMITSIZE);
    delete[] buffer;
}

void VolumeRampFuzzTest()
{
    std::shared_ptr<VolumeRamp> volumeRamp = nullptr;
    volumeRamp = std::make_shared<VolumeRamp>();
    float targetVolume = GetData<float>();
    float currStreamVolume = GetData<float>();
    int32_t duration = GetData<int32_t>();
    volumeRamp->SetVolumeRampConfig(targetVolume, currStreamVolume, duration);
    volumeRamp->GetRampVolume();
    volumeRamp->IsActive();
    volumeRamp->Terminate();
}

typedef void (*TestFuncs[7])();

TestFuncs g_testFuncs = {
    AbsoluteSleepFuzzTest,
    RelativeSleepFuzzTest,
    CountFuzzTest,
    CountVolumeFuzzTest,
    GetTimeFuzzTest,
    AudioBlendFuzzTest,
    VolumeRampFuzzTest,
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
