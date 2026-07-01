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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "audio_engine_log.h"
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_utils.h"
#include "hpae_direct_renderer_manager.h"
#include "hpae_info.h"
#include "hpae_sink_input_node.h"
#include "i_hpae_renderer_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

// ---------------------------------------------------------------------------
// Fuzzer input state (single translation unit - no .h declarations needed)
// ---------------------------------------------------------------------------
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos = 0;

// ---------------------------------------------------------------------------
// Named constants (no magic numbers)
// ---------------------------------------------------------------------------
constexpr size_t FUZZ_INPUT_SIZE_THRESHOLD = 32;
constexpr int32_t TEST_SLEEP_TIME_MS_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_MS_40 = 40;
constexpr int32_t MAX_MSG_PROCESSING_WAIT_LOOPS = 10;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr uint32_t TEST_STREAM_SESSION_ID_BASE = 123456;
constexpr size_t FUZZ_STRING_MAX_LEN_SHORT = 50;
constexpr size_t FUZZ_STRING_MAX_LEN_LONG = 100;
constexpr uint32_t MOVE_SESSION_TYPE_MOD = 4;

const char *DEFAULT_TEST_DEVICE_CLASS = "direct";
const char *VOIP_TEST_DEVICE_CLASS = "primary_direct_voip";
const char *DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";

// ---------------------------------------------------------------------------
// Supported enum lists.
//
// CRITICAL: samplingRate and channels drive the renderFrameData_ allocation
// (frameLen * channels * formatSize) inside HpaeDirectSinkOutputNode. InitSinkInner
// recomputes nodeInfo.frameLen = samplingRate * 20 / 1000 AFTER CheckFramelen
// (which only validates sinkInfo.frameLen). A raw out-of-range fuzz rate makes
// frameLen ~SIZE_MAX -> malloc(~SIZE_MAX) -> musl aborts the process. channels
// belongs to the same class of risk. Therefore BOTH must always be picked from
// the supported lists below, never from raw fuzz bytes.
//
// format is self-protecting: GetSizeFromFormat returns 0 for out-of-range
// values, so the allocation collapses to 0. format may stay fuzzed.
// ---------------------------------------------------------------------------
const std::vector<AudioChannel> SUPPORTED_CHANNELS = {
    MONO,
    STEREO,
};

const std::vector<AudioChannel> SUPPORTED_CHANNELS_MULTI = {
    AudioChannel::CHANNEL_3,
    AudioChannel::CHANNEL_4,
    AudioChannel::CHANNEL_6,
    AudioChannel::CHANNEL_8,
};

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioSamplingRate> HIGH_RES_SAMPLE_RATES = {
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS = {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

// ---------------------------------------------------------------------------
// Data extraction helpers
// ---------------------------------------------------------------------------
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

// Pick a value from a non-empty list using fuzz input. Always constrains the
// result to a supported enum value (used for samplingRate and channels).
template<class T>
T PickFromList(const std::vector<T> &list)
{
    if (list.empty()) {
        return T {};
    }
    return list[GetData<uint32_t>() % list.size()];
}

// format picker: format is self-protecting (GetSizeFromFormat -> 0 for
// out-of-range), so it may be raw-fuzzed half the time to exercise the
// out-of-range path, and picked from the supported list the other half.
template<class T>
void RoundVal(T &roundVal, const std::vector<T> &list)
{
    if (list.empty()) {
        roundVal = GetData<T>();
        return;
    }
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

std::string GetFuzzString(size_t maxLen)
{
    if (RAW_DATA == nullptr || g_pos >= g_dataSize) {
        return "";
    }
    size_t len = GetData<size_t>() % (maxLen + 1);
    if (len > g_dataSize - g_pos) {
        len = g_dataSize - g_pos;
    }
    std::string result(reinterpret_cast<const char *>(RAW_DATA + g_pos), len);
    g_pos += len;
    return result;
}

// ---------------------------------------------------------------------------
// Sink / stream info builders.
// All builders CONSTRAIN samplingRate and channels to supported lists.
// ---------------------------------------------------------------------------
static void InitHpaeSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.sinkId = GetData<uint32_t>();
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = GetFuzzString(FUZZ_STRING_MAX_LEN_LONG);
    sinkInfo.deviceName = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    sinkInfo.frameLen = FRAME_LENGTH_960;
    // CONSTRAIN rate + channels (see SUPPORTED list comment above).
    sinkInfo.samplingRate = PickFromList(SUPPORTED_SAMPLE_RATES);
    sinkInfo.channels = PickFromList(SUPPORTED_CHANNELS);
    // format is self-protecting; may use RoundVal.
    RoundVal(sinkInfo.format, SUPPORTED_FORMATS);
    sinkInfo.channelLayout = GetData<uint64_t>();
    sinkInfo.deviceType = GetData<int32_t>();
    sinkInfo.volume = GetData<float>();
    sinkInfo.openMicSpeaker = GetData<uint32_t>();
    sinkInfo.renderInIdleState = GetData<uint32_t>();
    sinkInfo.sourceType = GetData<uint32_t>();
    sinkInfo.offloadEnable = GetData<uint32_t>();
    sinkInfo.fixedLatency = GetData<uint32_t>();
    sinkInfo.sinkLatency = GetData<uint32_t>();
    sinkInfo.splitMode = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    sinkInfo.needEmptyChunk = GetData<bool>();
    sinkInfo.auxSinkEnable = GetData<bool>();
}

static void InitVoipSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.sinkId = GetData<uint32_t>();
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = VOIP_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = VOIP_TEST_DEVICE_CLASS;
    sinkInfo.filePath = GetFuzzString(FUZZ_STRING_MAX_LEN_LONG);
    sinkInfo.deviceName = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    sinkInfo.frameLen = FRAME_LENGTH_960;
    // Fixed valid values for the VoIP direct path.
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S16LE;
    sinkInfo.channels = MONO;
    sinkInfo.channelLayout = GetData<uint64_t>();
    sinkInfo.deviceType = GetData<int32_t>();
    sinkInfo.volume = GetData<float>();
    sinkInfo.openMicSpeaker = GetData<uint32_t>();
    sinkInfo.renderInIdleState = GetData<uint32_t>();
    sinkInfo.sourceType = GetData<uint32_t>();
    sinkInfo.offloadEnable = GetData<uint32_t>();
    sinkInfo.fixedLatency = GetData<uint32_t>();
    sinkInfo.sinkLatency = GetData<uint32_t>();
    sinkInfo.splitMode = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    sinkInfo.needEmptyChunk = GetData<bool>();
    sinkInfo.auxSinkEnable = GetData<bool>();
}

static void InitRenderStreamInfoBase(HpaeStreamInfo &streamInfo, uint32_t sessionId)
{
    streamInfo.sessionId = sessionId;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.nodeType = GetData<HpaeNodeType>();
    streamInfo.streamType = GetData<AudioStreamType>();
    streamInfo.fadeType = GetData<FadeType>();
    streamInfo.pipeType = GetData<AudioPipeType>();
    streamInfo.customSampleRate = GetData<uint32_t>();
    streamInfo.channelLayout = GetData<uint64_t>();
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.uid = GetData<int32_t>();
    streamInfo.pid = GetData<int32_t>();
    streamInfo.tokenId = GetData<uint32_t>();
    streamInfo.deviceName = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    streamInfo.isMoveAble = GetData<bool>();
    streamInfo.privacyType = GetData<AudioPrivacyType>();
}

// Normal-rate render stream. CONSTRAIN rate + channels.
static void InitRenderStreamInfo(HpaeStreamInfo &streamInfo)
{
    InitRenderStreamInfoBase(streamInfo, TEST_STREAM_SESSION_ID_BASE);
    streamInfo.samplingRate = PickFromList(SUPPORTED_SAMPLE_RATES);
    streamInfo.channels = PickFromList(SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, SUPPORTED_FORMATS);
}

// High-res render stream. CONSTRAIN rate + channels.
static void InitHighResStreamInfo(HpaeStreamInfo &streamInfo)
{
    InitRenderStreamInfoBase(streamInfo, TEST_STREAM_SESSION_ID_BASE);
    streamInfo.samplingRate = PickFromList(HIGH_RES_SAMPLE_RATES);
    streamInfo.channels = PickFromList(SUPPORTED_CHANNELS);
    streamInfo.format = SAMPLE_S32LE;
}

// Multi-channel render stream. CONSTRAIN rate + channels.
static void InitMultiChannelStreamInfo(HpaeStreamInfo &streamInfo)
{
    InitRenderStreamInfoBase(streamInfo, TEST_STREAM_SESSION_ID_BASE);
    streamInfo.samplingRate = PickFromList(SUPPORTED_SAMPLE_RATES);
    streamInfo.channels = PickFromList(SUPPORTED_CHANNELS_MULTI);
    streamInfo.format = SAMPLE_S32LE;
}

// ---------------------------------------------------------------------------
// Worker drain helper.
//
// HpaeDirectRendererManager owns a HpaeSignalProcessThread worker that drains
// an HpaeNoLockQueue. Public API enqueues requests (producer side, thread
// safe); the queued lambdas run on the worker (FIFO serial model). After any
// step that posts a request we must let the worker drain before the next step
// or before DeInit, otherwise the destructor may join mid-flight.
//
// IsMsgProcessing() returns !hpaeNoLockQueue_.IsFinishProcess() and reflects
// queue state, NOT isInit_. We therefore loop on IsMsgProcessing() unconditionally
// (regardless of init result) so the worker drains even on the init-failure
// path. This is safe because when Init() was never called there is no thread
// and no requests were enqueued (SendRequest rejects non-init requests), so
// IsMsgProcessing() is immediately false and we just do the tail sleep.
// ---------------------------------------------------------------------------
static void WaitForMsgProcessing(std::shared_ptr<HpaeDirectRendererManager> &manager)
{
    if (manager == nullptr) {
        return;
    }
    for (int32_t i = 0; i < MAX_MSG_PROCESSING_WAIT_LOOPS && manager->IsMsgProcessing(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_MS_20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_MS_40));
}

// ===========================================================================
// Test functions.
//
// Threading rules (Phase 0 trap #1): every test calls ONLY the public API from
// the fuzz main thread. Worker-internal methods (HandleMsg, InitSinkInner,
// Handle*/Process*, queue internals) are never called directly.
//
// Init tolerance (Phase 0 trap #3): "direct" deviceClass is not registered in
// the standalone fuzzer process, so GetRenderIdByDeviceClass returns
// HDI_INVALID_ID -> audioRendererSink_ stays null -> RenderSinkInit returns
// ERR_ILLEGAL_STATE -> isInit_ ends up FALSE. Every test tolerates init failure
// and just exercises the API; the engine's not-init guards handle the rest.
//
// Cleanup: every test that called Init() MUST call DeInit() before returning so
// the worker thread is stopped deterministically (do not rely on the
// destructor, which only DeInits when isInit_ is true - on the init-failure
// path isInit_ is false but the worker is still alive).
// ===========================================================================

// --- Construct only (no Init, no thread) ---
void HpaeDirectRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeDirectRendererManager directRendererManager(sinkInfo);
}

// --- Init(isReload=fuzz) + IsInit() + DeInit() ---
void HpaeDirectRendererManagerInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    bool isReload = GetData<bool>();
    manager->Init(isReload);
    WaitForMsgProcessing(manager);
    manager->IsInit();
    manager->DeInit();
}

// --- DeInit(isMoveDefault=fuzz) ---
void HpaeDirectRendererManagerDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    bool isMoveDefault = GetData<bool>();
    manager->DeInit(isMoveDefault);
}

// --- CreateStream / DestroyStream (fuzz sessionId) ---
void HpaeDirectRendererManagerCreateStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->DestroyStream(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- Start / Pause(isStandby=fuzz) / Flush / Drain / Stop / Release ---
void HpaeDirectRendererManagerStartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Start(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerPauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    bool isStandby = GetData<bool>();
    manager->Pause(sessionId, isStandby);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Flush(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Drain(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerStopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Stop(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Release(sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- Setters that tolerate not-init (no-op / early-return internally) ---
void HpaeDirectRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    manager->SetClientVolume(sessionId, volume);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    manager->SetRate(sessionId, rate);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    manager->SetAudioEffectMode(sessionId, effectMode);
    int32_t outEffectMode = 0;
    manager->GetAudioEffectMode(sessionId, outEffectMode);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    bool isMute = GetData<bool>();
    manager->SetMute(isMute);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    manager->SetSpeed(sessionId, speed);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    manager->SetPrivacyType(sessionId, privacyType);
    int32_t outPrivacyType = 0;
    manager->GetPrivacyType(sessionId, outPrivacyType);
    manager->DeInit();
}

void HpaeDirectRendererManagerUpdateSpatializationFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    manager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
    manager->DeInit();
}

void HpaeDirectRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    manager->GetWritableSize(sessionId);
    manager->DeInit();
}

void HpaeDirectRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    manager->SetLoudnessGain(sessionId, loudnessGain);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerUpdateMaxLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    manager->UpdateMaxLength(sessionId, maxLength);
    manager->DeInit();
}

// --- Move: MoveStream(sessionId, fuzz sinkName); MoveAllStream both paths ---
void HpaeDirectRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    std::string sinkName = GetFuzzString(FUZZ_STRING_MAX_LEN_LONG);
    manager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    std::string sinkName = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    std::vector<uint32_t> sessionIds = { streamInfo.sessionId };
    uint32_t moveType = GetData<uint32_t>() % MOVE_SESSION_TYPE_MOD;
    manager->MoveAllStream(sinkName, sessionIds, static_cast<MoveSessionType>(moveType));
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// MoveAllStream WITHOUT Init -> takes the synchronous MoveAllStreamToNewSink
// path (no thread, no queue). Safe and covers the !IsInit branch.
void HpaeDirectRendererManagerMoveAllStreamNoInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    // Deliberately do NOT Init: exercise the sync move path.
    std::string sinkName = GetFuzzString(FUZZ_STRING_MAX_LEN_SHORT);
    std::vector<uint32_t> sessionIds = { GetData<uint32_t>() };
    manager->MoveAllStream(sinkName, sessionIds, MOVE_ALL);
}

// --- SuspendStreamManager(fuzz bool) ---
void HpaeDirectRendererManagerSuspendFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    bool isSuspend = GetData<bool>();
    manager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- ReloadRenderManager (incl. reload-while-running variant) ---
void HpaeDirectRendererManagerReloadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    bool isReload = GetData<bool>();
    manager->ReloadRenderManager(newSinkInfo, isReload);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerReloadWhileRunningFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    manager->ReloadRenderManager(newSinkInfo, GetData<bool>());
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- Query / info batch ---
void HpaeDirectRendererManagerQueryInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    manager->GetAllSinkInputsInfo();
    uint32_t sessionId = GetData<uint32_t>();
    HpaeSinkInputInfo sinkInputInfo;
    manager->GetSinkInputInfo(sessionId, sinkInputInfo);
    manager->GetSinkInfo();
    manager->GetThreadName();
    manager->DumpSinkInfo();
    WaitForMsgProcessing(manager);
    manager->GetDeviceHDFDumpInfo();
    manager->IsRunning();
    manager->IsMsgProcessing();
    manager->RefreshProcessClusterByDevice();
    manager->DeInit();
}

// --- StopManager / DeactivateThread ---
void HpaeDirectRendererManagerStopManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    manager->StopManager();
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    manager->DeactivateThread();
}

// --- RegisterWriteCallback / RegisterReadCallback (empty weak_ptr) ---
void HpaeDirectRendererManagerRegisterWriteCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<IStreamCallback> callback;
    manager->RegisterWriteCallback(sessionId, callback);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<ICapturerStreamCallback> callback;
    manager->RegisterReadCallback(sessionId, callback);
    manager->DeInit();
}

// --- SetOffloadPolicy / TriggerAppsUidUpdate ---
void HpaeDirectRendererManagerSetOffloadPolicyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    manager->SetOffloadPolicy(sessionId, state);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

void HpaeDirectRendererManagerTriggerAppsUidUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->TriggerAppsUidUpdate(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- Suspend then Resume with a running stream ---
void HpaeDirectRendererManagerSuspendResumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->SuspendStreamManager(true);
    WaitForMsgProcessing(manager);
    manager->SuspendStreamManager(false);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// ===========================================================================
// Scenario tests
// ===========================================================================

// --- Full lifecycle: Init -> Create -> Start -> Flush -> Drain -> Pause ->
// Start -> Stop -> Destroy -> DeInit ---
void HpaeDirectRendererManagerFullLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Flush(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Drain(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Pause(streamInfo.sessionId, false);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit(GetData<bool>());
}

// --- Multi-stream + node switch (create 2, destroy current, start the other) ---
void HpaeDirectRendererManagerNodeSwitchFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo1;
    InitRenderStreamInfo(streamInfo1);
    streamInfo1.sessionId = TEST_STREAM_SESSION_ID_BASE;
    manager->CreateStream(streamInfo1);
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo2;
    InitRenderStreamInfo(streamInfo2);
    streamInfo2.sessionId = TEST_STREAM_SESSION_ID_BASE + 1;
    manager->CreateStream(streamInfo2);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo1.sessionId);
    WaitForMsgProcessing(manager);
    // Destroy current node to trigger the SetCurrentNode re-selection path.
    manager->DestroyStream(streamInfo1.sessionId);
    WaitForMsgProcessing(manager);
    // streamInfo2 should now be curNode_; starting it exercises ConnectInputSession.
    manager->Start(streamInfo2.sessionId);
    WaitForMsgProcessing(manager);
    manager->Stop(streamInfo2.sessionId);
    WaitForMsgProcessing(manager);
    manager->DestroyStream(streamInfo2.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- VoIP full lifecycle (fixed VoIP sink: 48k/S16LE/MONO) ---
void HpaeDirectRendererManagerVoipLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitVoipSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.samplingRate = SAMPLE_RATE_16000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.channels = MONO;
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Pause(streamInfo.sessionId, GetData<bool>());
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- VoIP with F32LE format conversion (direct VoIP converts F32LE -> S16LE) ---
void HpaeDirectRendererManagerVoipFormatConvertFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitVoipSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.channels = MONO;
    streamInfo.samplingRate = SAMPLE_RATE_8000;
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- High-res sample rate (88200/96000/176400/192000) exercises the
// GetDirectSampleRate up-rounding branches. ---
void HpaeDirectRendererManagerHighResSampleRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitHighResStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// --- Multi-channel (>STEREO) triggers limiterNode creation. ---
void HpaeDirectRendererManagerMultiChannelFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitMultiChannelStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(manager);
    manager->DeInit();
}

// ===========================================================================
// Callback coverage.
//
// The engine now guards OnNodeStatusUpdate / OnRequestLatency / OnNotifyQueue
// / OnFadeDone with CHECK_AND_RETURN_LOG(curNode_, ...) (or thread null checks),
// so calling these public callbacks from the fuzz main thread after
// WaitForMsgProcessing (worker idle) is safe and desired. OnFadeDone and
// OnNotifyQueue internally SendRequest (enqueue), so we drain again after.
// ===========================================================================
void HpaeDirectRendererManagerCallbacksFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    manager->Init();
    WaitForMsgProcessing(manager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);
    WaitForMsgProcessing(manager);
    manager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(manager);

    // Public callbacks (engine guards null curNode_/thread internally).
    manager->OnNodeStatusUpdate(streamInfo.sessionId, OPERATION_STARTED);
    uint64_t latency = 0;
    manager->OnRequestLatency(streamInfo.sessionId, latency);
    manager->OnNotifyQueue();
    WaitForMsgProcessing(manager);
    manager->OnFadeDone(streamInfo.sessionId);
    WaitForMsgProcessing(manager);

    manager->DeInit();
}

// ===========================================================================
// Pre-Init ops batch.
// Without Init there is no worker thread and SendRequest rejects every request
// at the !IsInit() guard, so none of these enqueue anything. This safely
// covers the not-init early-return branches in Start/Pause/Stop/Flush/Drain/
// Release/SetClientVolume/SetRate/SetSpeed/SetMute/DestroyStream/
// SuspendStreamManager/SetAudioEffectMode etc. The destructor handles cleanup
// (no Init => no thread to stop).
// ===========================================================================
void HpaeDirectRendererManagerUnInitOpsFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    CHECK_AND_RETURN(manager != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    manager->Start(sessionId);
    manager->Pause(sessionId, GetData<bool>());
    manager->Flush(sessionId);
    manager->Drain(sessionId);
    manager->Stop(sessionId);
    manager->Release(sessionId);
    manager->SetClientVolume(sessionId, GetData<float>());
    manager->SetRate(sessionId, GetData<int32_t>());
    manager->SetSpeed(sessionId, GetData<float>());
    manager->SetMute(GetData<bool>());
    manager->SetAudioEffectMode(sessionId, GetData<int32_t>());
    manager->DestroyStream(sessionId);
    manager->SuspendStreamManager(GetData<bool>());
    // No DeInit needed: Init was never called, no worker exists.
}

// ===========================================================================
// Dispatch table
// ===========================================================================
using TestFunc = void (*)();

static std::vector<TestFunc> g_testFuncs = {
    HpaeDirectRendererManagerConstructFuzzTest,
    HpaeDirectRendererManagerInitFuzzTest,
    HpaeDirectRendererManagerDeInitFuzzTest,
    HpaeDirectRendererManagerCreateStreamFuzzTest,
    HpaeDirectRendererManagerDestroyStreamFuzzTest,
    HpaeDirectRendererManagerStartFuzzTest,
    HpaeDirectRendererManagerPauseFuzzTest,
    HpaeDirectRendererManagerFlushFuzzTest,
    HpaeDirectRendererManagerDrainFuzzTest,
    HpaeDirectRendererManagerStopFuzzTest,
    HpaeDirectRendererManagerReleaseFuzzTest,
    HpaeDirectRendererManagerSetClientVolumeFuzzTest,
    HpaeDirectRendererManagerSetRateFuzzTest,
    HpaeDirectRendererManagerSetEffectModeFuzzTest,
    HpaeDirectRendererManagerSetMuteFuzzTest,
    HpaeDirectRendererManagerSetSpeedFuzzTest,
    HpaeDirectRendererManagerSetPrivacyTypeFuzzTest,
    HpaeDirectRendererManagerUpdateSpatializationFuzzTest,
    HpaeDirectRendererManagerGetWritableSizeFuzzTest,
    HpaeDirectRendererManagerSetLoudnessGainFuzzTest,
    HpaeDirectRendererManagerUpdateMaxLengthFuzzTest,
    HpaeDirectRendererManagerMoveStreamFuzzTest,
    HpaeDirectRendererManagerMoveAllStreamFuzzTest,
    HpaeDirectRendererManagerMoveAllStreamNoInitFuzzTest,
    HpaeDirectRendererManagerSuspendFuzzTest,
    HpaeDirectRendererManagerReloadFuzzTest,
    HpaeDirectRendererManagerReloadWhileRunningFuzzTest,
    HpaeDirectRendererManagerQueryInfoFuzzTest,
    HpaeDirectRendererManagerStopManagerFuzzTest,
    HpaeDirectRendererManagerDeactivateThreadFuzzTest,
    HpaeDirectRendererManagerRegisterWriteCallbackFuzzTest,
    HpaeDirectRendererManagerRegisterReadCallbackFuzzTest,
    HpaeDirectRendererManagerSetOffloadPolicyFuzzTest,
    HpaeDirectRendererManagerTriggerAppsUidUpdateFuzzTest,
    HpaeDirectRendererManagerSuspendResumeFuzzTest,
    HpaeDirectRendererManagerFullLifecycleFuzzTest,
    HpaeDirectRendererManagerNodeSwitchFuzzTest,
    HpaeDirectRendererManagerVoipLifecycleFuzzTest,
    HpaeDirectRendererManagerVoipFormatConvertFuzzTest,
    HpaeDirectRendererManagerHighResSampleRateFuzzTest,
    HpaeDirectRendererManagerMultiChannelFuzzTest,
    HpaeDirectRendererManagerCallbacksFuzzTest,
    HpaeDirectRendererManagerUnInitOpsFuzzTest,
};
}  // namespace AudioStandard
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::RAW_DATA = data;
    OHOS::AudioStandard::g_dataSize = size;
    OHOS::AudioStandard::g_pos = 0;

    uint32_t code = OHOS::AudioStandard::GetData<uint32_t>();
    const auto &funcs = OHOS::AudioStandard::g_testFuncs;
    if (!funcs.empty()) {
        funcs[code % funcs.size()]();
    }
    return 0;
}
