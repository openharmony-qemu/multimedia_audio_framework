/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <array>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_loud_volume_manager.h"
#include "audio_stream_info.h"
#include "iaudio_policy_interface.h"
#include "audio_policy_manager_factory.h"
#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;

constexpr int32_t NUM_2 = 2;
constexpr int32_t MAX_VOLUME_LEVEL = 15;
constexpr int32_t MIN_VOLUME_LEVEL = 0;

const std::array<AudioStreamType, 8> STREAM_TYPES = {
    STREAM_MUSIC,
    STREAM_VOICE_CALL,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_RING,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_ACCESSIBILITY,
    STREAM_DTMF,
};

const std::array<LoudVolumeHoldType, 3> LOUD_VOLUME_HOLD_TYPES = {
    LOUD_VOLUME_MODE_INVALID,
    LOUD_VOLUME_MODE_MUSIC,
    LOUD_VOLUME_MODE_VOICE,
};

const std::array<SetLoudVolMode, 5> SET_LOUD_VOL_MODES = {
    LOUD_VOLUME_SWITCH_INVALID,
    LOUD_VOLUME_SWITCH_AUTO,
    LOUD_VOLUME_SWITCH_PAUSE,
    LOUD_VOLUME_SWITCH_OFF,
    LOUD_VOLUME_SWITCH_ON,
};

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

template<typename T, size_t N>
T Pick(FuzzedDataProvider &fdp, const std::array<T, N> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, N - 1)];
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT

void FuzzReloadLoudVolumeMode(FuzzedDataProvider& fdp)
{
    AudioStreamType streamType = Pick(fdp, STREAM_TYPES);
    SetLoudVolMode setVolMode = Pick(fdp, SET_LOUD_VOL_MODES);

    LoudVolumeManager loudVolumeManager;
    loudVolumeManager.ReloadLoudVolumeMode(streamType, setVolMode);
}

void FuzzReloadLoudVolumeModeSwitch(FuzzedDataProvider& fdp)
{
    LoudVolumeHoldType holdType = Pick(fdp, LOUD_VOLUME_HOLD_TYPES);
    SetLoudVolMode setVolMode = Pick(fdp, SET_LOUD_VOL_MODES);

    LoudVolumeManager loudVolumeManager;
    loudVolumeManager.ReloadLoudVolumeModeSwitch(holdType, setVolMode);
}

void FuzzClearLoudVolumeHoldMap(FuzzedDataProvider& fdp)
{
    LoudVolumeHoldType holdType = Pick(fdp, LOUD_VOLUME_HOLD_TYPES);

    LoudVolumeManager loudVolumeManager;
    loudVolumeManager.ClearLoudVolumeHoldMap(holdType);
}

void FuzzCheckLoudVolumeMode(FuzzedDataProvider& fdp)
{
    int32_t volLevel = fdp.ConsumeIntegralInRange<int32_t>(MIN_VOLUME_LEVEL, MAX_VOLUME_LEVEL);
    int32_t keyType = fdp.ConsumeIntegral<int32_t>();
    AudioStreamType streamType = Pick(fdp, STREAM_TYPES);

    LoudVolumeManager loudVolumeManager;
    loudVolumeManager.CheckLoudVolumeMode(volLevel, keyType, streamType);
}

void FuzzCheckLoudVolumeModeWithKeyEvent(FuzzedDataProvider& fdp)
{
    int32_t volLevel = fdp.ConsumeIntegralInRange<int32_t>(MIN_VOLUME_LEVEL, MAX_VOLUME_LEVEL);
    AudioStreamType streamType = Pick(fdp, STREAM_TYPES);

    const std::array<int32_t, 3> KEY_TYPES = {
        MMI::KeyEvent::KEYCODE_VOLUME_UP,
        MMI::KeyEvent::KEYCODE_VOLUME_DOWN,
        fdp.ConsumeIntegral<int32_t>(),
    };
    int32_t keyType = Pick(fdp, KEY_TYPES);

    LoudVolumeManager loudVolumeManager;
    loudVolumeManager.CheckLoudVolumeMode(volLevel, keyType, streamType);
}

void FuzzTest(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        FuzzReloadLoudVolumeMode,
        FuzzReloadLoudVolumeModeSwitch,
        FuzzClearLoudVolumeHoldMap,
        FuzzCheckLoudVolumeMode,
        FuzzCheckLoudVolumeModeWithKeyEvent,
    });
    func(fdp);
}

#else

void FuzzTest(FuzzedDataProvider& fdp)
{
}

#endif

void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
