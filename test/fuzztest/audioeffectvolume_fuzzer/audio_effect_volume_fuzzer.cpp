/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_server.h"
#include "audio_effect_volume.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
// Modulus used to clamp a fuzz int into a valid stream-category range.
const int32_t CATEGORY_RANGE = 20;
// Modulus for the fuzzed session-ID string length (0..15 chars).
const uint32_t SESSION_ID_LEN_RANGE = 16;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
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

void SetStreamVolumeFuzzTest()
{
    std::string sessionID = "123";
    float streamVolume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
}

void StreamVolumeDeleteFuzzTest()
{
    std::string sessionID = "123";
    float streamVolume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    audioEffectVolume->StreamVolumeDelete(sessionID);
    audioEffectVolume->StreamVolumeDelete(sessionID);
}

void GetStreamVolumeFuzzTest()
{
    std::string sessionID = "456";
    float streamVolume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetStreamVolume(sessionID, streamVolume);
    audioEffectVolume->GetStreamVolume(sessionID);
    audioEffectVolume->StreamVolumeDelete(sessionID);
}

void SystemVolumeFuzzTest()
{
    int32_t category = GetData<int32_t>() % CATEGORY_RANGE;
    float volume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetSystemVolume(category, volume);
    audioEffectVolume->GetSystemVolume(category);
}

void DspVolumeFuzzTest()
{
    float volume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetDspVolume(volume);
    audioEffectVolume->GetDspVolume();
}

void VolumeDeleteAndGetFuzzTest()
{
    std::string sessionID = "789";
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->StreamVolumeDelete(sessionID);
    audioEffectVolume->GetStreamVolume(sessionID);
}

// Query a system-volume type that was never Set -> exercises GetSystemVolume's
// not-found branch (return SystemVolumeMap_[STREAM_MUSIC]).
void SystemVolumeNotFoundFuzzTest()
{
    int32_t category = GetData<int32_t>(); // un-clamped: very likely absent from the map
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->GetSystemVolume(category); // not-found -> STREAM_MUSIC fallback
    float volume = GetData<float>();
    audioEffectVolume->SetSystemVolume(category, volume);
    audioEffectVolume->GetSystemVolume(category); // now found
}

// Fuzz-driven sessionID across the full Set/Get/Delete cycle, hitting every branch:
// SetStreamVolume (insert), GetStreamVolume found, StreamVolumeDelete erase,
// GetStreamVolume not-found, StreamVolumeDelete not-found.
void FuzzedSessionVolumeFuzzTest()
{
    uint32_t len = GetData<uint32_t>() % SESSION_ID_LEN_RANGE; // 0..15 chars
    std::string sessionID;
    for (uint32_t i = 0; i < len; i++) {
        sessionID.push_back(static_cast<char>(GetData<uint8_t>()));
    }
    float volume = GetData<float>();
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume == nullptr) {
        return;
    }
    audioEffectVolume->SetStreamVolume(sessionID, volume); // insert (line 69)
    audioEffectVolume->GetStreamVolume(sessionID);         // found (line 78)
    audioEffectVolume->StreamVolumeDelete(sessionID);      // erase (line 88)
    audioEffectVolume->GetStreamVolume(sessionID);         // not-found (line 75-76)
    audioEffectVolume->StreamVolumeDelete(sessionID);      // not-found (line 85-86)
}

TestFuncs g_testFuncs[] = {
    SetStreamVolumeFuzzTest,
    StreamVolumeDeleteFuzzTest,
    GetStreamVolumeFuzzTest,
    SystemVolumeFuzzTest,
    DspVolumeFuzzTest,
    VolumeDeleteAndGetFuzzTest,
    SystemVolumeNotFoundFuzzTest,
    FuzzedSessionVolumeFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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

    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
