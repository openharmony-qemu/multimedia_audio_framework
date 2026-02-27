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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifdef RESSCHE_ENABLE
#include "audio_service.h"
#include "audio_system_load_listener.h"
#endif
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

constexpr size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
constexpr int32_t MAX_RENDERER_STREAM_COUNT = 16;
constexpr int32_t MAX_DELAY_MS = 10;
constexpr int32_t MIN_NON_EMPTY_STREAM_COUNT = 1;
constexpr int32_t EMPTY_RENDERER_STREAM_COUNT = 0;
constexpr int32_t SYSTEM_LOAD_LEVEL_ESCAPE = 7;
constexpr int32_t SYSTEM_LOAD_LEVEL_EMERGENCY = 6;
constexpr int32_t SYSTEM_LOAD_LEVEL_RECOVERY_DELAY = 5;
constexpr int32_t SYSTEM_LOAD_LEVEL_RECOVERY_IMMEDIATE = 4;
constexpr int32_t SYSTEM_LOAD_LEVEL_MIN = 0;
constexpr int32_t SYSTEM_LOAD_LEVEL_OUT_OF_RANGE = 10;
constexpr uint32_t DELAY_MS_RANGE = MAX_DELAY_MS + 1;
constexpr const char *SPATIAL_AUDIO_DISABLE = "1";
constexpr const char *SPATIAL_AUDIO_ENABLE = "0";
constexpr const char *SYSTEM_LOAD_FUZZ_TASK = "AudioSystemLoadFuzzTask";
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

typedef void (*TestFuncs)();

void SetRendererStreamCount(int32_t streamCount)
{
    AudioService *audioService = AudioService::GetInstance();
    CHECK_AND_RETURN(audioService != nullptr);
    audioService->currentRendererStreamCnt_ = streamCount;
}

void AudioSystemloadListenerRegisterUnregisterFuzzTest()
{
    auto listener = std::make_shared<AudioSystemloadListener>();
    CHECK_AND_RETURN(listener != nullptr);

    SetRendererStreamCount(EMPTY_RENDERER_STREAM_COUNT);
    listener->RegisterResSchedSys();
    listener->UnregisterResSchedSys();

    int32_t streamCount = static_cast<int32_t>(g_fuzzUtils.GetData<uint32_t>() % MAX_RENDERER_STREAM_COUNT) +
        MIN_NON_EMPTY_STREAM_COUNT;
    SetRendererStreamCount(streamCount);
    listener->UnregisterResSchedSys();
    listener->RegisterResSchedSys();
}

void AudioSystemloadListenerOnSystemloadLevelFuzzTest()
{
    auto listener = std::make_shared<AudioSystemloadListener>();
    CHECK_AND_RETURN(listener != nullptr);

    SetRendererStreamCount(static_cast<int32_t>(g_fuzzUtils.GetData<uint32_t>() % MAX_RENDERER_STREAM_COUNT) +
        MIN_NON_EMPTY_STREAM_COUNT);
    listener->OnSystemloadLevel(g_fuzzUtils.GetData<int32_t>());
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_ESCAPE);
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_EMERGENCY);
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_RECOVERY_DELAY);
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_RECOVERY_IMMEDIATE);
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_MIN);
    listener->OnSystemloadLevel(SYSTEM_LOAD_LEVEL_OUT_OF_RANGE);

    SetRendererStreamCount(EMPTY_RENDERER_STREAM_COUNT);
    listener->OnSystemloadLevel(g_fuzzUtils.GetData<int32_t>());
}

void AudioSystemloadListenerPrivateApiFuzzTest()
{
    auto listener = std::make_shared<AudioSystemloadListener>();
    CHECK_AND_RETURN(listener != nullptr);

    SetRendererStreamCount(g_fuzzUtils.GetData<bool>() ? EMPTY_RENDERER_STREAM_COUNT : MIN_NON_EMPTY_STREAM_COUNT);
    (void)listener->IsAudioStreamEmpty();

    int32_t delayMs = static_cast<int32_t>(g_fuzzUtils.GetData<uint32_t>() % DELAY_MS_RANGE);
    std::string disableSpatialAudio = g_fuzzUtils.GetData<bool>() ? SPATIAL_AUDIO_DISABLE : SPATIAL_AUDIO_ENABLE;
    listener->PostControlSpatialAudioTask(delayMs, disableSpatialAudio);
}

void AudioSystemloadListenerHandlerFuzzTest()
{
    auto handler = std::make_shared<AudioSystemloadListenerHandler>();
    CHECK_AND_RETURN(handler != nullptr);

    auto task = []() {};
    int32_t delayMs = static_cast<int32_t>(g_fuzzUtils.GetData<uint32_t>() % DELAY_MS_RANGE);
    (void)handler->PostTask(task, SYSTEM_LOAD_FUZZ_TASK, delayMs);
    handler->RemoveTask(SYSTEM_LOAD_FUZZ_TASK);
}

std::vector<TestFuncs> g_testFuncs = {
    AudioSystemloadListenerRegisterUnregisterFuzzTest,
    AudioSystemloadListenerOnSystemloadLevelFuzzTest,
    AudioSystemloadListenerPrivateApiFuzzTest,
    AudioSystemloadListenerHandlerFuzzTest,
};

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
#ifdef RESSCHE_ENABLE
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return OHOS::AudioStandard::FUZZ_TEST_SUCCESS;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return OHOS::AudioStandard::FUZZ_TEST_SUCCESS;
#else
    return OHOS::AudioStandard::FUZZ_TEST_SUCCESS;
#endif
}
