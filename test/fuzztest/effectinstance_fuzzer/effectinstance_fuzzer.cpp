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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#undef private
#include "securec.h"
#include "effect_instance.h"
#include "fake_audio_effect_lib_entry.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos = 0;

// --- Constants ---
// AudioBufferConfig.samplingRate and .channels are uint32_t (audio_effect.h).
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
// Minimum fuzz input size to allow dispatch code extraction.
constexpr size_t MIN_FUZZ_INPUT_SIZE = 10;
// Max fuzz-string length for effect names.
constexpr uint32_t EFFECT_NAME_MAX_LEN = 32;
// One stereo float frame = channels * sizeof(float) = 2 * 4 = 8 bytes.
constexpr size_t STEREO_FLOAT_FRAME_BYTES = DEFAULT_CHANNEL_COUNT * sizeof(float);
// Feed data size: 4 floats = 2 stereo frames (enough to trigger fake
// background processing when processThreshold is set to 4).
constexpr size_t FEED_DATA_BYTES = STEREO_FLOAT_FRAME_BYTES * 2;
// Output buffer size: 8 floats = 4 stereo frames.
constexpr size_t OUTPUT_BUF_BYTES = STEREO_FLOAT_FRAME_BYTES * 4;
// SIZE_MAX equivalent: prevents FakeAudioEffectLibEntry from spawning its
// background process thread (feedInput threshold never reached).
constexpr size_t PROCESS_THRESHOLD_DISABLED = static_cast<size_t>(-1);
// Low process threshold (4 floats = 2 stereo frames) to trigger fake
// background processing for SkipStaleFrames / ReadOutput success paths.
constexpr size_t PROCESS_THRESHOLD_LOW = 4;
// Invalid EffectState value to cover the default case in EffectStateToString.
constexpr int INVALID_STATE_VALUE = 42;
// Stats report threshold of 1 frame to trigger the L205-208 product path.
constexpr int64_t STATS_THRESHOLD_ONE_FRAME = 1;
// Number of stale frames to skip in ReadOutput SkipStaleFrames tests.
constexpr uint32_t SKIP_ONE_STALE_FRAME = 1;

// --- Data extraction (matches existing fuzzer pattern) ---

template <typename T>
T GetData()
{
    T object{};
    size_t remaining = g_dataSize - g_pos;
    if (remaining < sizeof(T)) {
        return object;
    }
    void *dst = &object;
    const void *src = RAW_DATA + g_pos;
    if (memcpy_s(dst, sizeof(T), src, sizeof(T)) != EOK) {
        return object;
    }
    g_pos += sizeof(T);
    return object;
}

std::string GetFuzzString(size_t maxLen)
{
    uint32_t len = GetData<uint32_t>() % (maxLen + 1);
    std::string result;
    for (uint32_t i = 0; i < len && g_pos < g_dataSize; i++) {
        result += static_cast<char>(RAW_DATA[g_pos++]);
    }
    return result;
}

AudioEffectConfig GetFuzzConfig()
{
    AudioEffectConfig config = {};
    config.inputCfg.samplingRate = DEFAULT_SAMPLE_RATE;
    config.inputCfg.channels = DEFAULT_CHANNEL_COUNT;
    config.outputCfg.samplingRate = DEFAULT_SAMPLE_RATE;
    config.outputCfg.channels = DEFAULT_CHANNEL_COUNT;
    return config;
}

// Test 1: Init all code paths.
// Covers:
//   - Init(name, config) 2-arg overload (L47-55)
//   - Init(name, config, nullptr) lib==nullptr early return (L63-66)
//   - Init(name, config, &lib) createEffect success + SET_CONFIG (L73-97)
//   - SetCreateFail(true) -> createEffect failure (L74-77)
void InitAllPathsFuzzTest()
{
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    AudioEffectConfig config = GetFuzzConfig();

    // Path A: Init(name, config) -- 2-arg overload, no library (L47-55).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->Release();
    }

    // Path B: Init(name, config, nullptr) -- lib==nullptr (L63-66).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config, nullptr);
        inst->Release();
    }

    // Path C: Init(name, config, &lib) -- createEffect success + SET_CONFIG.
    {
        FakeAudioEffectLibEntry fake;
        fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        inst->Release(); // release handle while fake is alive
    }

    // Path D: SetCreateFail(true) -> createEffect returns -1 (L74-77).
    {
        FakeAudioEffectLibEntry fake;
        fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);
        fake.SetCreateFail(true);
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib); // ERR_EFFECT_CREATE_FAILED
        inst->Release(); // no-op: hasHandle_ is false
    }
}

// Test 2: FeedInput all code paths.
// Covers:
//   - null data early return (L175)
//   - len==0 early return (L175)
//   - !hasHandle_ early return (L178)
//   - feedInput==nullptr early return (L182) via -fno-access-control
//   - success: fedBytes_ + framesProcessed accumulation (L198-201)
//   - failure: processErrors++ (L196)
//   - threshold report: stats_.ToString() + stats_.Reset() (L205-208)
//     Also covers se_process_stats.h ToString() via product path.
void FeedInputPathsFuzzTest()
{
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    AudioEffectConfig config = GetFuzzConfig();

    // Path A: null data early return (L175).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->FeedInput(nullptr, STEREO_FLOAT_FRAME_BYTES);
        inst->Release();
    }

    // Path B: len==0 early return (L175).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        uint8_t dummy = 0;
        inst->FeedInput(&dummy, 0);
        inst->Release();
    }

    // Path C: !hasHandle_ early return (L178) -- no library Init.
    {
        auto inst = std::make_shared<EffectInstance>();
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame));
        inst->Release();
    }

    // Paths D-H require a real handle (hasHandle_=true).
    FakeAudioEffectLibEntry fake;
    fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);

    // Path D: feedInput==nullptr early return (L182).
    // Use -fno-access-control to null out the feedInput function pointer.
    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        // Init's return isn't checked; guard the -fno-access-control deref against a
        // future where Init leaves effectHandle_ unpopulated. FeedInput itself null-
        // checks hasHandle_/effectHandle_ at L178; this raw poke bypasses that guard.
        if (inst->effectHandle_ != nullptr && *inst->effectHandle_ != nullptr) {
            (*inst->effectHandle_)->feedInput = nullptr;
        }
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame)); // L182 early return
        inst->Release();
    }

    // Path E: success path -- fedBytes_ + framesProcessed (L198-201).
    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame));
        inst->Release();
    }

    // Path F: feedInput failure -> processErrors++ (L196).
    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        fake.SetFeedInputFail(true);
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame));
        fake.SetFeedInputFail(false);
        inst->Release();
    }

    // Path G: threshold report -- ToString() + Reset() (L205-208).
    // Set threshold to 1, feed 1 frame -> framesProcessed=1 >= 1 triggers.
    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        inst->SetStatsReportThresholdForTest(STATS_THRESHOLD_ONE_FRAME);
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame)); // triggers L205-208
        inst->Release();
    }

    // Path H: threshold report with non-zero processErrors.
    // Cause an error first, then trigger the threshold with a success.
    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        fake.SetFeedInputFail(true);
        uint8_t frame[STEREO_FLOAT_FRAME_BYTES] = {0};
        inst->FeedInput(frame, sizeof(frame)); // processErrors++
        fake.SetFeedInputFail(false);
        inst->SetStatsReportThresholdForTest(STATS_THRESHOLD_ONE_FRAME);
        inst->FeedInput(frame, sizeof(frame)); // triggers report
        inst->Release();
    }
}

// Test 3: ReadOutput + SkipStaleFrames all code paths.
// Covers:
//   - null buf early return (L233)
//   - len==0 early return (L233)
//   - !hasHandle_ early return (L236)
//   - getOutput==nullptr early return (L240) via -fno-access-control
//   - SkipStaleFrames failure: getOutput returns empty (L221-223)
//   - SkipStaleFrames success: getOutput returns data (L227)
//   - getOutput success: framesOutput accumulation (L263)
//   - getOutput failure: processErrors++ (L259)
void ReadOutputPathsFuzzTest()
{
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    AudioEffectConfig config = GetFuzzConfig();

    // Path A: null buf early return (L233).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->ReadOutput(nullptr, OUTPUT_BUF_BYTES);
        inst->Release();
    }

    // Path B: len==0 early return (L233).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        uint8_t dummy = 0;
        inst->ReadOutput(&dummy, 0);
        inst->Release();
    }

    // Path C: !hasHandle_ early return (L236).
    {
        auto inst = std::make_shared<EffectInstance>();
        uint8_t buf[OUTPUT_BUF_BYTES] = {0};
        inst->ReadOutput(buf, sizeof(buf));
        inst->Release();
    }

    // Paths D-F: failure paths (no processing needed).
    {
        FakeAudioEffectLibEntry fake;
        fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);

        // Path D: getOutput==nullptr early return (L240).
        {
            auto inst = std::make_shared<EffectInstance>();
            AudioEffectLibrary lib = fake.GetLibrary();
            inst->Init(effectName, config, &lib);
            if (inst->effectHandle_ != nullptr && *inst->effectHandle_ != nullptr) {
                (*inst->effectHandle_)->getOutput = nullptr;
            }
            uint8_t buf[OUTPUT_BUF_BYTES] = {0};
            inst->ReadOutput(buf, sizeof(buf)); // L240 early return
            inst->Release();
        }

        // Path E: SkipStaleFrames failure -- empty output (L221-223).
        {
            auto inst = std::make_shared<EffectInstance>();
            AudioEffectLibrary lib = fake.GetLibrary();
            inst->Init(effectName, config, &lib);
            uint8_t buf[OUTPUT_BUF_BYTES] = {0};
            uint32_t skipCount = SKIP_ONE_STALE_FRAME;
            inst->ReadOutput(buf, sizeof(buf), &skipCount);
            inst->Release();
        }

        // Path F: getOutput failure -> processErrors++ (L259).
        {
            auto inst = std::make_shared<EffectInstance>();
            AudioEffectLibrary lib = fake.GetLibrary();
            inst->Init(effectName, config, &lib);
            fake.SetGetOutputFail(true);
            uint8_t buf[OUTPUT_BUF_BYTES] = {0};
            inst->ReadOutput(buf, sizeof(buf)); // L259 processErrors++
            fake.SetGetOutputFail(false);
            inst->Release();
        }
    }

    // Paths G-H: success paths (require processed data).
    {
        FakeAudioEffectLibEntry fake;
        fake.SetProcessThreshold(PROCESS_THRESHOLD_LOW);
        fake.SetProcessDelayMs(0);

        // Path G: SkipStaleFrames success (L227). After skip consumes all
        // processed data, the subsequent getOutput returns frameLength=0
        // (L263 executes with zero accumulation). Path H covers getOutput
        // success with non-zero framesOutput.
        {
            auto inst = std::make_shared<EffectInstance>();
            AudioEffectLibrary lib = fake.GetLibrary();
            inst->Init(effectName, config, &lib);
            uint8_t feedData[FEED_DATA_BYTES] = {0};
            inst->FeedInput(feedData, sizeof(feedData));
            fake.WaitForProcessing(); // wait for background process
            uint8_t buf[OUTPUT_BUF_BYTES] = {0};
            uint32_t skipCount = SKIP_ONE_STALE_FRAME;
            inst->ReadOutput(buf, sizeof(buf), &skipCount);
            inst->Release();
        }

        // Path H: getOutput success without skip (L263).
        {
            auto inst = std::make_shared<EffectInstance>();
            AudioEffectLibrary lib = fake.GetLibrary();
            inst->Init(effectName, config, &lib);
            uint8_t feedData[FEED_DATA_BYTES] = {0};
            inst->FeedInput(feedData, sizeof(feedData));
            fake.WaitForProcessing();
            uint8_t buf[OUTPUT_BUF_BYTES] = {0};
            inst->ReadOutput(buf, sizeof(buf)); // getOutput success
            inst->Release();
        }
    }
}

// Test 4: Command methods (StopEffect, Flush, SetParameter, GetPreheatFrames).
// Covers:
//   - StopEffect hasHandle_ path (L281-285) + !hasHandle_ early return
//   - Flush hasHandle_ path (L296-300) + !hasHandle_ early return
//   - Flush with SetFlushFail -> command returns -1
//   - SetParameter hasHandle_ path (L305-316) + !hasHandle_ early return
//   - GetPreheatFrames first call (L324-336) + cached (L321-323)
//     + !hasHandle_ early return (L324-326)
void CommandMethodsFuzzTest()
{
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    AudioEffectConfig config = GetFuzzConfig();
    int32_t paramType = GetData<int32_t>();
    float paramValue = GetData<float>();

    // !hasHandle_ early returns (no fake needed).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->StopEffect();   // !hasHandle_ -> return 0
        inst->Flush();        // !hasHandle_ -> ClearBuffers + return 0
        inst->SetParameter(paramType, paramValue); // !hasHandle_ -> 0
        inst->GetPreheatFrames(); // !hasHandle_ -> return 0
        inst->Release();
    }

    // hasHandle_ paths.
    FakeAudioEffectLibEntry fake;
    fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);

    {
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);

        // StopEffect with handle (L281-285).
        inst->StopEffect();

        // SetParameter with handle (L305-316).
        inst->SetParameter(paramType, paramValue);

        // GetPreheatFrames first call -- query via command (L324-336).
        inst->GetPreheatFrames();
        // GetPreheatFrames second call -- cached (L321-323).
        inst->GetPreheatFrames();

        // Flush with handle, command succeeds (L296-300).
        inst->Flush();

        // Flush with SetFlushFail -> command returns -1.
        fake.SetFlushFail(true);
        inst->Flush();
        fake.SetFlushFail(false);

        inst->Release();
    }
}

// Test 5: Lifecycle, state transitions, and EffectStateToString coverage.
//   - ClearBuffers (L273-277)
//   - TriggerProcess (L288-291)
//   - MarkPendingRelease (L127-134)
//   - SetState + Release -> EffectStateToString all cases (L31-40, L102)
//     States: IDLE, ACTIVE, PENDING_RELEASE, ERROR, default (invalid)
//   - Getters/setters: GetState, GetBoundSessionId/SetBoundSessionId,
//     GetEffectName, GetAudioConfig, GetReleaseGeneration,
//     GetPendingTaskId/SetPendingTaskId, GetInputLevel
//   - Release with hasHandle_=true (L104-108) + hasHandle_=false
void LifecycleAndStateFuzzTest()
{
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    AudioEffectConfig config = GetFuzzConfig();
    uint32_t sessionId = GetData<uint32_t>();
    uint64_t taskId = GetData<uint64_t>();

    // EffectStateToString via Release log (L102).
    // Each block sets a different state, then Release logs it (L31-40).

    // IDLE state (default, no Init) -> logs "IDLE" (L34).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Release();
    }

    // ACTIVE state (after Init) -> logs "ACTIVE" (L35).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->Release();
    }

    // PENDING_RELEASE state -> logs "PENDING_RELEASE" (L36).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->MarkPendingRelease();
        inst->Release();
    }

    // ERROR state -> logs "ERROR" (L37).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->SetState(EffectInstance::EffectState::ERROR);
        inst->Release();
    }

    // Invalid state -> logs "UNKNOWN" (default case, L38).
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);
        inst->SetState(
            static_cast<EffectInstance::EffectState>(INVALID_STATE_VALUE));
        inst->Release();
    }

    // Getters/setters + ClearBuffers + TriggerProcess.
    {
        auto inst = std::make_shared<EffectInstance>();
        inst->Init(effectName, config);

        inst->SetBoundSessionId(sessionId);
        inst->SetPendingTaskId(taskId);

        (void)inst->GetState();
        (void)inst->GetBoundSessionId();
        (void)inst->GetEffectName();
        (void)inst->GetAudioConfig();
        (void)inst->GetReleaseGeneration();
        (void)inst->GetPendingTaskId();
        (void)inst->GetInputLevel();

        inst->ClearBuffers();   // L273-277
        inst->TriggerProcess(); // L288-291

        inst->Release();
    }

    // Release with hasHandle_=true (L104-108).
    {
        FakeAudioEffectLibEntry fake;
        fake.SetProcessThreshold(PROCESS_THRESHOLD_DISABLED);
        auto inst = std::make_shared<EffectInstance>();
        AudioEffectLibrary lib = fake.GetLibrary();
        inst->Init(effectName, config, &lib);
        inst->Release(); // releaseEffect + cleanup + stats_.Reset()
    }
}

using TestFunc = void(*)();
static TestFunc g_testFuncs[] = {
    InitAllPathsFuzzTest,
    FeedInputPathsFuzzTest,
    ReadOutputPathsFuzzTest,
    CommandMethodsFuzzTest,
    LifecycleAndStateFuzzTest,
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr ||
        size < OHOS::AudioStandard::HPAE::MIN_FUZZ_INPUT_SIZE) {
        return 0;
    }
    OHOS::AudioStandard::HPAE::RAW_DATA = data;
    OHOS::AudioStandard::HPAE::g_dataSize = size;
    OHOS::AudioStandard::HPAE::g_pos = 0;
    uint32_t code = OHOS::AudioStandard::HPAE::GetData<uint32_t>();
    uint32_t len = sizeof(OHOS::AudioStandard::HPAE::g_testFuncs) /
        sizeof(OHOS::AudioStandard::HPAE::g_testFuncs[0]);
    if (len > 0) {
        OHOS::AudioStandard::HPAE::g_testFuncs[code % len]();
    }
    return 0;
}
