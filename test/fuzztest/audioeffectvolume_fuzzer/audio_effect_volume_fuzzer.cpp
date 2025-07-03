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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 2;

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

TestFuncs g_testFuncs[TESTSIZE] = {
    SetStreamVolumeFuzzTest,
    StreamVolumeDeleteFuzzTest,
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
