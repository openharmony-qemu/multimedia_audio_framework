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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#undef private
#include "securec.h"
#include "session_effect_manager.h"
#include "hpae_manager.h"
#include "effect_instance.h"
#include "audio_effect.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos = 0;

// AudioBufferConfig.samplingRate/channels are uint32_t (not enums), and this TU does not
// include audio_info.h, so SAMPLE_RATE_48000/STEREO are not visible here. Use named values.
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_CHANNEL_COUNT = 2;
constexpr size_t MIN_FUZZ_INPUT_SIZE = 10;
// Modulus for the fuzzed library-entry count (0..3 entries) in LibraryAndTimeoutFuzzTest.
constexpr uint32_t LIB_ENTRY_COUNT_RANGE = 4;
// Max fuzz-string lengths (chars) for names passed to the SessionEffectManager APIs.
constexpr uint32_t EFFECT_NAME_MAX_LEN = 32;
constexpr uint32_t LIBRARY_NAME_MAX_LEN = 16;
constexpr uint32_t SHORT_NAME_MAX_LEN = 8;
// Number of sessions bound in ReuseWithDifferentSessionsFuzzTest (session-preemption path).
constexpr uint32_t REUSE_TEST_SESSION_COUNT = 3;

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

// Test 1: Construct and Deinit
void ConstructAndDeinitFuzzTest()
{
    HpaeManager *hpaeManager = nullptr;
    // Create SessionEffectManager with null HpaeManager reference
    // This tests the constructor and Deinit path gracefully
    // We test with a minimal safe approach
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    sem.Deinit();
}

// Test 2: TryReuseExistingInstance with no bound effects
void TryReuseEmptyFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    uint32_t sessionId = GetData<uint32_t>();
    AudioEffectConfig config = GetFuzzConfig();
    std::shared_ptr<EffectInstance> outInstance;
    sem.BindSessionEffect(sessionId, effectName, config, outInstance);
    sem.Deinit();
}

// Test 3: Bind an EffectInstance then try reuse
void TryReuseWithBoundFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    uint32_t sessionId = GetData<uint32_t>();
    AudioEffectConfig config = GetFuzzConfig();
    std::shared_ptr<EffectInstance> outInstance;
    sem.BindSessionEffect(sessionId, effectName, config, outInstance);
    // Try binding again with same effect name but different session
    uint32_t sessionId2 = GetData<uint32_t>();
    sem.BindSessionEffect(sessionId2, effectName, config, outInstance);
    sem.Deinit();
}

// Test 4: Deinit multiple times
void MultipleDeinitFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    sem.Deinit();
    sem.Deinit();
    sem.Deinit();
}

// Test 5: Reuse with different sessions - test session preemption logic
void ReuseWithDifferentSessionsFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = "test_effect";
    AudioEffectConfig config = GetFuzzConfig();
    for (int i = 0; i < REUSE_TEST_SESSION_COUNT; i++) {
        uint32_t sessionId = GetData<uint32_t>();
        std::shared_ptr<EffectInstance> outInstance;
        sem.BindSessionEffect(sessionId, effectName, config, outInstance);
    }
    sem.Deinit();
}

// Test 6: Full lifecycle
void FullLifecycleFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    uint32_t sessionId = GetData<uint32_t>();
    AudioEffectConfig config = GetFuzzConfig();
    // Bind effect
    std::shared_ptr<EffectInstance> outInstance;
    sem.BindSessionEffect(sessionId, effectName, config, outInstance);
    // Query active effect names
    std::vector<std::string> activeNames = sem.GetAllActiveEffectNames();
    // Unbind
    sem.UnbindSessionEffect(sessionId);
    // Query pending
    bool hasPending = sem.HasPendingEffectInstance(effectName);
    sem.Deinit();
}

// Test 7: OnSessionDestroyed + pending-effect getters/setters. Binds an effect (->ACTIVE),
// destroys its session (->PENDING_RELEASE), then exercises the pending + found branches of
// GetPendingEffectInstance / FlushPendingEffectInstance / GetBoundSessionId / SetEffectParameter,
// and the not-found branches with an unknown name/session.
void PendingAndDestroyFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = GetFuzzString(EFFECT_NAME_MAX_LEN);
    uint32_t sessionId = GetData<uint32_t>();
    AudioEffectConfig config = GetFuzzConfig();
    std::shared_ptr<EffectInstance> outInstance;
    sem.BindSessionEffect(sessionId, effectName, config, outInstance);

    sem.OnSessionDestroyed(sessionId); // ACTIVE -> PENDING_RELEASE (lines 197-213)
    (void)sem.GetPendingEffectInstance(effectName);  // PENDING -> returns instance (232-234)
    (void)sem.HasPendingEffectInstance(effectName);  // true (222)
    (void)sem.GetBoundSessionId(effectName);         // found (262)
    sem.SetEffectParameter(effectName, GetData<int32_t>(), GetData<float>()); // found (282)
    (void)sem.FlushPendingEffectInstance(effectName); // PENDING -> Flush (244-246)

    // not-found branches:
    std::string unknown = GetFuzzString(SHORT_NAME_MAX_LEN);
    sem.OnSessionDestroyed(GetData<uint32_t>());      // instance not found (186-188)
    (void)sem.GetPendingEffectInstance(unknown);      // not in map (229-231)
    (void)sem.FlushPendingEffectInstance(unknown);    // not in map (241-243)
    (void)sem.GetBoundSessionId(unknown);             // not in map (259-261)
    sem.SetEffectParameter(unknown, GetData<int32_t>(), GetData<float>()); // not found (279-281)
    sem.Deinit();
}

// Test 8: library loading + timeout setter. Covers SetLazyReleaseTimeout, InitEffectLibraries,
// GetEffectLibEntry (found + not-found), SetAudioEffectLibraryForTest.
void LibraryAndTimeoutFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    sem.SetLazyReleaseTimeout(GetData<uint64_t>()); // setter (253)

    std::vector<std::shared_ptr<AudioEffectLibEntry>> libEntries;
    uint32_t n = GetData<uint32_t>() % LIB_ENTRY_COUNT_RANGE; // 0..3 entries
    for (uint32_t i = 0; i < n; i++) {
        auto entry = std::make_shared<AudioEffectLibEntry>();
        entry->libraryName = GetFuzzString(LIBRARY_NAME_MAX_LEN);
        libEntries.push_back(entry);
    }
    sem.InitEffectLibraries(libEntries);              // populate effectLibMap_ (288-291)
    if (!libEntries.empty()) {
        (void)sem.GetEffectLibEntry(libEntries[0]->libraryName); // found (298)
    }
    (void)sem.GetEffectLibEntry(GetFuzzString(SHORT_NAME_MAX_LEN));    // not found (300)
    sem.SetAudioEffectLibraryForTest(nullptr);        // inline test setter
    sem.Deinit();
}

// Test 9: preempt with a non-null preemptedSessionId out-param. Existing reuse tests pass the
// default nullptr, so the *preemptedSessionId = ... branch (65-67) is never hit.
void PreemptedSessionIdFuzzTest()
{
    auto manager = std::make_shared<HpaeManager>();
    SessionEffectManager sem(*manager);
    std::string effectName = "test_effect";
    AudioEffectConfig config = GetFuzzConfig();
    uint32_t session1 = GetData<uint32_t>();
    uint32_t session2 = GetData<uint32_t>();
    std::shared_ptr<EffectInstance> out1;
    sem.BindSessionEffect(session1, effectName, config, out1); // effect ACTIVE on session1
    std::shared_ptr<EffectInstance> out2;
    uint32_t preempted = 0;
    sem.BindSessionEffect(session2, effectName, config, out2, &preempted); // preempt (65-67)
    sem.Deinit();
}

using TestFunc = void(*)();
static TestFunc g_testFuncs[] = {
    ConstructAndDeinitFuzzTest,
    TryReuseEmptyFuzzTest,
    TryReuseWithBoundFuzzTest,
    MultipleDeinitFuzzTest,
    ReuseWithDifferentSessionsFuzzTest,
    FullLifecycleFuzzTest,
    PendingAndDestroyFuzzTest,
    LibraryAndTimeoutFuzzTest,
    PreemptedSessionIdFuzzTest,
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_FUZZ_INPUT_SIZE) {
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
