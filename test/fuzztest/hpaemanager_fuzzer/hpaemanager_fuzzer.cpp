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

#include "hpaemanager_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include <string>
#include <thread>
#include <chrono>
#include <limits>
#include <cmath>
#include <cstdlib>
#include "audio_errors.h"
#include "test_case_common.h"
#include "hpae_capturer_manager.h"
#include "hpae_renderer_manager.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

// ---------------------------------------------------------------------------
// Design rationale (two UAF root causes fixed):
//
// 1. worker-only violation: the old fuzzer called Handle*/Enqueue*/Invoke*/
//    OnXxx from the fuzz main thread, racing the HpaeNoLockQueue and the
//    bare (un-synchronized) maps/lists that are only safe on the worker
//    thread. All such test cases have been removed.
//
// 2. heavy singleton lifecycle: the old fuzzer ran every stream operation on
//    a single process-lifetime HpaeManager singleton. Each CreateStream
//    spawned a SignalProcessThread worker that accumulated across rounds and
//    collided with the static IdHandler destructor at exit. Stream operations
//    are now fuzzed on independently constructed HpaeRendererManager /
//    HpaeCapturerManager sub-objects (construct -> Init -> public-API ->
//    destructor), so each worker is born and dies within one test case.
//
// Thread safety rules enforced below:
//   - NEVER call Handle*/Process/Enqueue*/Dequeue*/Invoke*/InvokeSync/
//     SendRequest/OnNodeStatusUpdate/OnRequestLatency/OnNotifyQueue/
//     IsRunning/IsMsgProcessing/GetMsgCount from the fuzz main thread.
//   - Session Effect: only Post* (async) variants, never the sync trio.
//   - -fno-access-control is used only for the atexit cleanup hook, never to
//     reach into worker-only methods.
// ---------------------------------------------------------------------------

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

// Named constants replacing magic numbers throughout the fuzzer.
constexpr uint8_t MAX_NAME_LENGTH = 64;
constexpr uint8_t MAX_ARRAY_SIZE_SMALL = 5;
constexpr uint8_t ENUM_HPAESTREAMCLASSTYPE_COUNT = 3;
constexpr int32_t INVALID_ENUM_VALUE = 255;
constexpr uint8_t BINARY_CHOICE_MOD = 2;

// Worker-wait constants for sub-manager Init synchronization.
constexpr int32_t MAX_INIT_WAIT_LOOPS = 10;
constexpr int32_t INIT_WAIT_SLEEP_MS = 20;
constexpr int32_t INIT_SETTLE_SLEEP_MS = 40;

// AISS / vocal-ratio boundary constants.
constexpr float VOCAL_RATIO_MIN = 0.0f;
constexpr float VOCAL_RATIO_MID = 0.5f;
constexpr float VOCAL_RATIO_MAX = 1.0f;
constexpr float VOCAL_RATIO_NEG = -1.0f;
constexpr float VOCAL_RATIO_OVER = 2.0f;
constexpr uint8_t ENUM_COLLABORATIONSTATE_COUNT = 3;
constexpr uint8_t ENUM_SPATIALIZATION_SCENE_TYPE_COUNT = 4;

// Supported audio parameter vectors for constructing valid sub-managers.
const vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
};

const vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

// FuzzedDataProvider::ConsumeIntegral<T> requires an integral T, so enums
// must be consumed as int32_t and cast back.
template <typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

// Read a bounded fuzz string char-by-char (avoids std::string(char, n)).
string FuzzString(FuzzedDataProvider &fdp, uint8_t maxLen)
{
    string result;
    size_t len = fdp.ConsumeIntegral<uint8_t>() % maxLen;
    for (size_t i = 0; i < len; i++) {
        result += fdp.ConsumeIntegral<char>();
    }
    return result;
}

// ===========================================================================
// Sub-object helpers (B-class tests)
// ===========================================================================

HpaeSinkInfo MakeFuzzSinkInfo(FuzzedDataProvider &fdp)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.sinkId = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_SINK_NAME;
    sinkInfo.filePath = "";
    sinkInfo.deviceName = FuzzString(fdp, MAX_NAME_LENGTH);
    sinkInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sinkInfo.samplingRate =
        SUPPORTED_SAMPLE_RATES[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_SAMPLE_RATES.size()];
    sinkInfo.format =
        SUPPORTED_FORMATS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_FORMATS.size()];
    sinkInfo.channels =
        SUPPORTED_CHANNELS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_CHANNELS.size()];
    sinkInfo.suspendTime = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.channelLayout = fdp.ConsumeIntegral<uint64_t>();
    sinkInfo.deviceType = fdp.ConsumeIntegral<int32_t>();
    sinkInfo.volume = fdp.ConsumeFloatingPoint<float>();
    sinkInfo.openMicSpeaker = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.renderInIdleState = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.sourceType = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.offloadEnable = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.fixedLatency = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.sinkLatency = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.splitMode = FuzzString(fdp, MAX_NAME_LENGTH);
    sinkInfo.needEmptyChunk = fdp.ConsumeIntegral<uint8_t>() != 0;
    sinkInfo.auxSinkEnable = fdp.ConsumeIntegral<uint8_t>() != 0;
    return sinkInfo;
}

HpaeSourceInfo MakeFuzzSourceInfo(FuzzedDataProvider &fdp)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = "";
    sourceInfo.samplingRate =
        SUPPORTED_SAMPLE_RATES[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_SAMPLE_RATES.size()];
    sourceInfo.channels =
        SUPPORTED_CHANNELS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_CHANNELS.size()];
    sourceInfo.format =
        SUPPORTED_FORMATS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_FORMATS.size()];
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
    sourceInfo.deviceName = FuzzString(fdp, MAX_NAME_LENGTH);
    return sourceInfo;
}

HpaeStreamInfo MakeFuzzStreamInfo(FuzzedDataProvider &fdp)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = fdp.ConsumeIntegral<uint32_t>();
    streamInfo.streamType = ConsumeEnum<AudioStreamType>(fdp);
    streamInfo.samplingRate =
        SUPPORTED_SAMPLE_RATES[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_SAMPLE_RATES.size()];
    streamInfo.format =
        SUPPORTED_FORMATS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_FORMATS.size()];
    streamInfo.channels =
        SUPPORTED_CHANNELS[fdp.ConsumeIntegral<uint8_t>() % SUPPORTED_CHANNELS.size()];
    streamInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    streamInfo.fadeType = ConsumeEnum<FadeType>(fdp);
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.uid = fdp.ConsumeIntegral<int32_t>();
    streamInfo.pid = fdp.ConsumeIntegral<int32_t>();
    streamInfo.deviceName = FuzzString(fdp, MAX_NAME_LENGTH);
    return streamInfo;
}

// Give the worker thread time to drain the Init request. Uses only IsInit()
// (a public atomic read) plus sleep — never calls worker-only methods.
void WaitForRendererInit(shared_ptr<HpaeRendererManager> &mgr)
{
    if (mgr == nullptr) {
        return;
    }
    for (int32_t i = 0; i < MAX_INIT_WAIT_LOOPS; i++) {
        if (mgr->IsInit()) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(INIT_WAIT_SLEEP_MS));
    }
    this_thread::sleep_for(chrono::milliseconds(INIT_SETTLE_SLEEP_MS));
}

void WaitForCapturerInit(shared_ptr<HpaeCapturerManager> &mgr)
{
    if (mgr == nullptr) {
        return;
    }
    for (int32_t i = 0; i < MAX_INIT_WAIT_LOOPS; i++) {
        if (mgr->IsInit()) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(INIT_WAIT_SLEEP_MS));
    }
    this_thread::sleep_for(chrono::milliseconds(INIT_SETTLE_SLEEP_MS));
}

// Construct + Init a renderer sub-object. Worker is bounded to the returned
// shared_ptr's lifetime — when it goes out of scope the destructor calls
// DeInit -> DeactivateThread -> join, cleanly stopping the worker.
shared_ptr<HpaeRendererManager> MakeFuzzRendererManager(FuzzedDataProvider &fdp)
{
    HpaeSinkInfo sinkInfo = MakeFuzzSinkInfo(fdp);
    auto mgr = make_shared<HpaeRendererManager>(sinkInfo);
    if (mgr == nullptr) {
        return nullptr;
    }
    mgr->Init();
    WaitForRendererInit(mgr);
    return mgr;
}

shared_ptr<HpaeCapturerManager> MakeFuzzCapturerManager(FuzzedDataProvider &fdp)
{
    HpaeSourceInfo sourceInfo = MakeFuzzSourceInfo(fdp);
    auto mgr = make_shared<HpaeCapturerManager>(sourceInfo);
    if (mgr == nullptr) {
        return nullptr;
    }
    mgr->Init();
    WaitForCapturerInit(mgr);
    return mgr;
}

// Create a stream on an already-Init'd renderer so subsequent stream
// operations (Start/Stop/SetRate/...) have a session to act on.
void CreateStreamOnRenderer(shared_ptr<HpaeRendererManager> &mgr, FuzzedDataProvider &fdp)
{
    if (mgr == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    mgr->CreateStream(streamInfo);
    this_thread::sleep_for(chrono::milliseconds(INIT_SETTLE_SLEEP_MS));
}

void CreateStreamOnCapturer(shared_ptr<HpaeCapturerManager> &mgr, FuzzedDataProvider &fdp)
{
    if (mgr == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    mgr->CreateStream(streamInfo);
    this_thread::sleep_for(chrono::milliseconds(INIT_SETTLE_SLEEP_MS));
}

// ===========================================================================
// HpaeManager singleton (C-class tests) + atexit cleanup
// ===========================================================================

// Reuse ONE HpaeManager for the whole fuzz run. Per-iteration Init/DeInit
// triggers a teardown join deadlock (~HpaeManager -> DeInit -> worker join
// while the worker is parked in Run()->condition_.wait).
shared_ptr<HpaeManager> GetFuzzHpaeManager()
{
    static const auto manager = [] {
        auto m = make_shared<HpaeManager>();
        m->Init();
        return m;
    }();
    // Register a no-op service callback stub so HpaeManager APIs that dereference
    // serviceCallback_ (e.g. MoveToPreferSink -> OnOpenAudioPortCb) do not crash on
    // a null weak_ptr. The stub is process-lifetime static so the weak_ptr stays valid.
    static const auto serviceCb = make_shared<FuzzHpaeServiceCallback>();
    [[maybe_unused]] static const bool registered = (manager->RegisterSerivceCallback(serviceCb), true);
    return manager;
}

// HpaeSinkInputNode has no default ctor (requires HpaeNodeInfo); build one.
shared_ptr<HpaeSinkInputNode> MakeSinkInputNode()
{
    HpaeNodeInfo nodeInfo;
    return make_shared<HpaeSinkInputNode>(nodeInfo);
}

// atexit hook: stop residual sub-manager workers BEFORE static destruction.
// Calls DeactivateThread (not DeInit) to avoid the join deadlock that would
// occur if the worker is parked in condition_.wait during static teardown.
// -fno-access-control lets us reach the private maps for cleanup only.
void AtExitCleanup()
{
    auto mgr = GetFuzzHpaeManager();
    if (mgr == nullptr) {
        return;
    }
    for (auto &[name, rendererMgr] : mgr->rendererManagerMap_) {
        if (rendererMgr != nullptr) {
            rendererMgr->DeactivateThread();
        }
    }
    for (auto &[name, capturerMgr] : mgr->capturerManagerMap_) {
        if (capturerMgr != nullptr) {
            capturerMgr->DeactivateThread();
        }
    }
}

// ===========================================================================
// C-class tests: HpaeManager process-level operations (singleton).
// These do NOT spawn stream-level workers and do not call worker-only
// methods from the fuzz main thread.
// ===========================================================================

void GetAllSinksFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->GetAllSinks();
}

void DumpAllAvailableDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->DumpAllAvailableDevice();
}

void DumpSinkInputsInfoFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->DumpSinkInputsInfo();
}

void DumpSourceOutputsInfoFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->DumpSourceOutputsInfo();
}

void SetDefaultSinkFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetDefaultSink(FuzzString(fdp, MAX_NAME_LENGTH));
}

void SetDefaultSinkWithEmptyNameFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetDefaultSink("");
}

void SetDefaultSourceFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetDefaultSource(FuzzString(fdp, MAX_NAME_LENGTH));
}

void SetDefaultSourceWithEmptyNameFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetDefaultSource("");
}

void GetAllSinkInputsFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->GetAllSinkInputs();
}

void GetAllSourceOutputsFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->GetAllSourceOutputs();
}

void MoveSourceOutputByIndexOrNameFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sourceOutputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sourceIndex = fdp.ConsumeIntegral<uint32_t>();
    hpaeManager->MoveSourceOutputByIndexOrName(
        sourceOutputId, sourceIndex, FuzzString(fdp, MAX_NAME_LENGTH));
}

void MoveSourceOutputWithNonExistentSourceFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sourceOutputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sourceIndex = fdp.ConsumeIntegral<uint32_t>();
    string sourceName = "NonExistent_" + to_string(fdp.ConsumeIntegral<uint32_t>());
    hpaeManager->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
}

void MoveSinkInputByIndexOrNameFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sinkInputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sinkIndex = fdp.ConsumeIntegral<uint32_t>();
    hpaeManager->MoveSinkInputByIndexOrName(
        sinkInputId, sinkIndex, FuzzString(fdp, MAX_NAME_LENGTH));
}

void MoveSinkInputWithNonExistentSinkFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sinkInputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sinkIndex = fdp.ConsumeIntegral<uint32_t>();
    string sinkName = "NonExistent_" + to_string(fdp.ConsumeIntegral<uint32_t>());
    hpaeManager->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

void SetSinkMuteFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool isSync = fdp.ConsumeIntegral<uint8_t>() != 0;
    hpaeManager->SetSinkMute(FuzzString(fdp, MAX_NAME_LENGTH), isMute, isSync);
}

void SetSinkMuteWithNonExistentSinkFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    string sinkName = "NonExistent_" + to_string(fdp.ConsumeIntegral<uint32_t>());
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool isSync = fdp.ConsumeIntegral<uint8_t>() != 0;
    hpaeManager->SetSinkMute(sinkName, isMute, isSync);
}

void SetSinkMuteForSwitchDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetSinkMuteForSwitchDevice(
        FuzzString(fdp, MAX_NAME_LENGTH), fdp.ConsumeIntegral<uint8_t>() != 0);
}

void SuspendAudioDeviceFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    string portName = FuzzString(fdp, MAX_NAME_LENGTH);
    hpaeManager->SuspendAudioDevice(portName, fdp.ConsumeIntegral<uint8_t>() != 0);
}

void StopAudioPortFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->StopAudioPort(FuzzString(fdp, MAX_NAME_LENGTH));
}

void StopAudioPortWithNonExistentPortFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    string portName = "NonExistent_" + to_string(fdp.ConsumeIntegral<uint32_t>());
    hpaeManager->StopAudioPort(portName);
}

void SetSourceOutputMuteFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    hpaeManager->SetSourceOutputMute(uid, fdp.ConsumeIntegral<uint8_t>() != 0);
}

void SetSourceOutputMuteByStreamIdFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetSourceOutputMuteByStreamId(
            sessionId, fdp.ConsumeIntegral<uint8_t>() != 0);
    }
}

void TriggerAppsUidUpdateFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    HpaeStreamClassType streamType = static_cast<HpaeStreamClassType>(
        fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->TriggerAppsUidUpdate(streamType, sessionId);
    }
}

void LoadEffectLiveFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->LoadEffectLive();
    }
}

void UpdateCollaborativeStateFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint8_t pick = fdp.ConsumeIntegral<uint8_t>();
    CollaborationState state = (pick % BINARY_CHOICE_MOD == 0)
        ? static_cast<CollaborationState>(pick % ENUM_COLLABORATIONSTATE_COUNT)
        : static_cast<CollaborationState>(pick);
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateCollaborativeState(state);
    }
}

void UpdateCollaborativeProductIdFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateCollaborativeProductId(FuzzString(fdp, MAX_NAME_LENGTH));
    }
}

void LoadCollaborationConfigFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        hpaeManager->LoadCollaborationConfig();
    }
}

void IsCollaborationSupportedForStreamUsageFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    StreamUsage streamUsage = ConsumeEnum<StreamUsage>(fdp);
    if (hpaeManager->IsInit()) {
        DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
        (void)hpaeManager->IsCollaborationSupportedForStreamUsage(deviceType, streamUsage);
    }
}

void GetSinkInfoByIdxFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t sinkIdx = fdp.ConsumeIntegral<int32_t>();
    auto callback = [](const HpaeSinkInfo &sinkInfo, int32_t result) {
        (void)sinkInfo;
        (void)result;
    };
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetSinkInfoByIdx(sinkIdx, callback);
    }
}

void GetSourceInfoByIdxFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t sourceIdx = fdp.ConsumeIntegral<int32_t>();
    auto callback = [](const HpaeSourceInfo &sourceInfo, int32_t result) {
        (void)sourceInfo;
        (void)result;
    };
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetSourceInfoByIdx(sourceIdx, callback);
    }
}

void GetStreamSpanSizeFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    HpaeStreamClassType streamType = static_cast<HpaeStreamClassType>(
        fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t spanSizeInFrame = 0;
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetStreamSpanSize(streamType, sessionId, spanSizeInFrame);
    }
}

void GetStreamRouteFlagFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    HpaeStreamClassType streamType = static_cast<HpaeStreamClassType>(
        fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t routeFlag = 0;
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetStreamRouteFlag(streamType, sessionId, routeFlag);
    }
}

void RegisterStatusCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    HpaeStreamClassType streamType = static_cast<HpaeStreamClassType>(
        fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    weak_ptr<IStreamStatusCallback> callback;
    (void)hpaeManager->RegisterStatusCallback(streamType, sessionId, callback);
}

void GetSourceOutputInfoFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    (void)hpaeManager->GetSourceOutputInfo(sessionId, streamInfo);
}

void SetOffloadRenderCallbackTypeFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    hpaeManager->SetOffloadRenderCallbackType(sessionId, fdp.ConsumeIntegral<int32_t>());
}

void SetSpatializationSceneTypeFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint8_t pick = fdp.ConsumeIntegral<uint8_t>();
    AudioSpatializationSceneType sceneType = (pick % BINARY_CHOICE_MOD == 0)
        ? static_cast<AudioSpatializationSceneType>(
              pick % ENUM_SPATIALIZATION_SCENE_TYPE_COUNT)
        : static_cast<AudioSpatializationSceneType>(pick);
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetSpatializationSceneType(sceneType);
    }
}

void EffectRotationUpdateFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->EffectRotationUpdate(fdp.ConsumeIntegral<uint32_t>());
    }
}

void SetEffectSystemVolumeFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetEffectSystemVolume(
            fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeFloatingPoint<float>());
    }
}

void SetAbsVolumeStateToEffectFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAbsVolumeStateToEffect(fdp.ConsumeIntegral<uint8_t>() != 0);
    }
}

void UpdateParamExtraFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        hpaeManager->UpdateParamExtra(
            FuzzString(fdp, MAX_NAME_LENGTH), FuzzString(fdp, MAX_NAME_LENGTH),
            FuzzString(fdp, MAX_NAME_LENGTH));
    }
}

void GetMaxAmplitudeFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetMaxAmplitude();
    }
}

void SetAppsEnhanceMuteStateFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAppsEnhanceMuteState(sessionId, fdp.ConsumeIntegral<uint8_t>() != 0);
    }
}

void IsChannelLayoutSupportedForDspEffectFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    AudioChannelLayout channelLayout = ConsumeEnum<AudioChannelLayout>(fdp);
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->IsChannelLayoutSupportedForDspEffect(channelLayout);
    }
}

void UpdatePersonalizedHRTFBinFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    // Restrict fd to invalid sentinels to avoid corrupting file state.
    int32_t fd = fdp.PickValueInArray({-1, numeric_limits<int32_t>::max()});
    long length = fdp.ConsumeIntegral<long>();
    hpaeManager->UpdatePersonalizedHRTFBin(fd, length);
}

void SetOutputDeviceSinkFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t device = fdp.ConsumeIntegral<int32_t>();
    hpaeManager->SetOutputDeviceSink(device, FuzzString(fdp, MAX_NAME_LENGTH));
}

void SetVolumeInfoFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    AudioStreamType volumeType = ConsumeEnum<AudioStreamType>(fdp);
    hpaeManager->SetVolumeInfo(volumeType, fdp.ConsumeFloatingPoint<float>());
}

void HandleBypassSpatializationForStereoFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->HandleBypassSpatializationForStereo(fdp.ConsumeIntegral<uint8_t>() != 0);
}

void SetAuxiliarySinkEnableFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    hpaeManager->SetAuxiliarySinkEnable(fdp.ConsumeIntegral<uint8_t>() != 0);
}

void SetAissEnabledSyncFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAissEnabledSync(sessionId, fdp.ConsumeIntegral<uint8_t>() != 0);
    }
}

void SetAissEnabledByUidSyncFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAissEnabledByUidSync(uid, fdp.ConsumeIntegral<uint8_t>() != 0);
    }
}

void SetVocalRatioSyncBoundaryFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    const float boundaries[] = {
        VOCAL_RATIO_MIN, VOCAL_RATIO_MID, VOCAL_RATIO_MAX,
        VOCAL_RATIO_NEG, VOCAL_RATIO_OVER,
        numeric_limits<float>::quiet_NaN(),
        numeric_limits<float>::infinity(),
    };
    const uint8_t count = sizeof(boundaries) / sizeof(boundaries[0]);
    float ratio = boundaries[fdp.ConsumeIntegral<uint8_t>() % count];
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetVocalRatioSync(ratio);
    }
}

void UpdateSpatializationStateStructFuzzTest(FuzzedDataProvider &fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }
    AudioSpatializationState state;
    state.spatializationEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    state.headTrackingEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    state.adaptiveSpatialRenderingEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    state.personalizedSpatializationEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateSpatializationState(state);
    }
}

// ===========================================================================
// B-class tests: HpaeRendererManager sub-object (independent lifecycle).
// Each test constructs + Inits + operates + destructs. The worker thread
// is bounded to the test case scope — no cross-round accumulation.
// ===========================================================================

void RendererCreateStreamFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    (void)mgr->CreateStream(streamInfo);
}

void RendererCreateStreamWithInvalidEnumFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    streamInfo.streamType = static_cast<AudioStreamType>(INVALID_ENUM_VALUE);
    streamInfo.sourceType = static_cast<SourceType>(INVALID_ENUM_VALUE);
    streamInfo.streamClassType = static_cast<HpaeStreamClassType>(INVALID_ENUM_VALUE);
    (void)mgr->CreateStream(streamInfo);
}

void RendererDestroyStreamFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->DestroyStream(fdp.ConsumeIntegral<uint32_t>());
}

void RendererStartFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Start(fdp.ConsumeIntegral<uint32_t>());
}

void RendererStartWithSyncIdFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t syncId = fdp.ConsumeIntegral<int32_t>();
    (void)mgr->StartWithSyncId(sessionId, syncId);
}

void RendererStartStopStartSequenceFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    (void)mgr->Start(sessionId);
    (void)mgr->Stop(sessionId);
    (void)mgr->Start(sessionId);
}

void RendererStopFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Stop(fdp.ConsumeIntegral<uint32_t>());
}

void RendererPauseFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Pause(fdp.ConsumeIntegral<uint32_t>());
}

void RendererPauseDrainPauseSequenceFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    (void)mgr->Pause(sessionId);
    (void)mgr->Drain(sessionId);
    (void)mgr->Pause(sessionId);
}

void RendererDrainFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Drain(fdp.ConsumeIntegral<uint32_t>());
}

void RendererFlushFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Flush(fdp.ConsumeIntegral<uint32_t>());
}

void RendererReleaseFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->Release(fdp.ConsumeIntegral<uint32_t>());
}

void RendererReleaseWithoutStartFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    (void)mgr->Release(fdp.ConsumeIntegral<uint32_t>());
}

void RendererSetClientVolumeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetClientVolume(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeFloatingPoint<float>());
}

void RendererSetVolumeWithExtremeValuesFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    const float extremes[] = {0.0f, -1.0f, 0.5f, 1.0f, 2.0f, 10.0f};
    const uint8_t count = sizeof(extremes) / sizeof(extremes[0]);
    float volume = extremes[fdp.ConsumeIntegral<uint8_t>() % count];
    (void)mgr->SetClientVolume(fdp.ConsumeIntegral<uint32_t>(), volume);
}

void RendererSetVolumeWithNanInfinityFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetClientVolume(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeFloatingPoint<float>());
}

void RendererSetRateFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetRate(fdp.ConsumeIntegral<uint32_t>(), fdp.ConsumeIntegral<int32_t>());
}

void RendererSetRateWithInvalidValueFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    int32_t rate = fdp.PickValueInArray({
        numeric_limits<int32_t>::min(), 0, -1, numeric_limits<int32_t>::max()});
    (void)mgr->SetRate(fdp.ConsumeIntegral<uint32_t>(), rate);
}

void RendererSetSpeedFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    mgr->SetSpeed(fdp.ConsumeIntegral<uint32_t>(), fdp.ConsumeFloatingPoint<float>());
}

void RendererSetSpeedWithExtremeValuesFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    const float speeds[] = {0.0f, -1.0f, 1.0f, 10.0f, 100.0f};
    float speed = speeds[fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL];
    mgr->SetSpeed(fdp.ConsumeIntegral<uint32_t>(), speed);
}

void RendererSetSpeedWithNanInfinityFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    mgr->SetSpeed(fdp.ConsumeIntegral<uint32_t>(), fdp.ConsumeFloatingPoint<float>());
}

void RendererSetAudioEffectModeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetAudioEffectMode(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeIntegral<int32_t>());
}

void RendererGetAudioEffectModeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    int32_t effectMode = 0;
    (void)mgr->GetAudioEffectMode(fdp.ConsumeIntegral<uint32_t>(), effectMode);
}

void RendererSetPrivacyTypeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetPrivacyType(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeIntegral<int32_t>());
}

void RendererGetPrivacyTypeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    int32_t privacyType = 0;
    (void)mgr->GetPrivacyType(fdp.ConsumeIntegral<uint32_t>(), privacyType);
}

void RendererRegisterWriteCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    weak_ptr<IStreamCallback> callback;
    (void)mgr->RegisterWriteCallback(fdp.ConsumeIntegral<uint32_t>(), callback);
}

void RendererRegisterReadCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    weak_ptr<ICapturerStreamCallback> callback;
    (void)mgr->RegisterReadCallback(fdp.ConsumeIntegral<uint32_t>(), callback);
}

void RendererUpdateSpatializationStateFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool spatializationEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool headTrackingEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    (void)mgr->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

void RendererUpdateMaxLengthFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->UpdateMaxLength(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeIntegral<uint32_t>());
}

void RendererSetOffloadPolicyFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetOffloadPolicy(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeIntegral<int32_t>());
}

void RendererGetWritableSizeFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->GetWritableSize(fdp.ConsumeIntegral<uint32_t>());
}

void RendererSetLoudnessGainFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->SetLoudnessGain(fdp.ConsumeIntegral<uint32_t>(),
        fdp.ConsumeFloatingPoint<float>());
}

// Session Effect: only Post* (async) variants — the header explicitly says
// the sync trio "must be called from SignalProcessThread".
void RendererPostCreateSessionEffectFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    string effectName = FuzzString(fdp, MAX_NAME_LENGTH);
    shared_ptr<IEffectInstance> instance = nullptr;
    AudioEffectConfig config;
    mgr->PostCreateSessionEffect(sessionId, effectName, instance, config);
}

void RendererPostDestroySessionEffectFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    mgr->PostDestroySessionEffect(fdp.ConsumeIntegral<uint32_t>());
}

void RendererPostSetSessionEffectParameterFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t type = fdp.ConsumeIntegral<int32_t>();
    float value = fdp.ConsumeFloatingPoint<float>();
    mgr->PostSetSessionEffectParameter(sessionId, type, value);
}

void RendererHasSessionEffectFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->HasSessionEffect(fdp.ConsumeIntegral<uint32_t>());
}

void RendererIsValidSessionFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzRendererManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnRenderer(mgr, fdp);
    (void)mgr->IsValidSession(fdp.ConsumeIntegral<uint32_t>());
}

// ===========================================================================
// B-class tests: HpaeCapturerManager sub-object (independent lifecycle).
// ===========================================================================

void CapturerCreateStreamFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = MakeFuzzStreamInfo(fdp);
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    (void)mgr->CreateStream(streamInfo);
}

void CapturerStartFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->Start(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerStopFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->Stop(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerPauseFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->Pause(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerFlushFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->Flush(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerReleaseFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->Release(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerDestroyStreamFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    (void)mgr->DestroyStream(fdp.ConsumeIntegral<uint32_t>());
}

void CapturerRegisterReadCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    weak_ptr<ICapturerStreamCallback> callback;
    (void)mgr->RegisterReadCallback(fdp.ConsumeIntegral<uint32_t>(), callback);
}

void CapturerSetAppsEnhanceMuteStateFuzzTest(FuzzedDataProvider &fdp)
{
    auto mgr = MakeFuzzCapturerManager(fdp);
    if (mgr == nullptr) {
        return;
    }
    CreateStreamOnCapturer(mgr, fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    (void)mgr->SetAppsEnhanceMuteState(sessionId, fdp.ConsumeIntegral<uint8_t>() != 0);
}

// ===========================================================================
// Main dispatch
// ===========================================================================

using FuzzFunc = void (*)(FuzzedDataProvider &);

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray<FuzzFunc>({
        // --- C-class: HpaeManager singleton (process-level) ---
        GetAllSinksFuzzTest,
        DumpAllAvailableDeviceFuzzTest,
        DumpSinkInputsInfoFuzzTest,
        DumpSourceOutputsInfoFuzzTest,
        SetDefaultSinkFuzzTest,
        SetDefaultSinkWithEmptyNameFuzzTest,
        SetDefaultSourceFuzzTest,
        SetDefaultSourceWithEmptyNameFuzzTest,
        GetAllSinkInputsFuzzTest,
        GetAllSourceOutputsFuzzTest,
        MoveSourceOutputByIndexOrNameFuzzTest,
        MoveSourceOutputWithNonExistentSourceFuzzTest,
        MoveSinkInputByIndexOrNameFuzzTest,
        MoveSinkInputWithNonExistentSinkFuzzTest,
        SetSinkMuteFuzzTest,
        SetSinkMuteWithNonExistentSinkFuzzTest,
        SetSinkMuteForSwitchDeviceFuzzTest,
        SuspendAudioDeviceFuzzTest,
        StopAudioPortFuzzTest,
        StopAudioPortWithNonExistentPortFuzzTest,
        SetSourceOutputMuteFuzzTest,
        SetSourceOutputMuteByStreamIdFuzzTest,
        TriggerAppsUidUpdateFuzzTest,
        LoadEffectLiveFuzzTest,
        UpdateCollaborativeStateFuzzTest,
        UpdateCollaborativeProductIdFuzzTest,
        LoadCollaborationConfigFuzzTest,
        IsCollaborationSupportedForStreamUsageFuzzTest,
        GetSinkInfoByIdxFuzzTest,
        GetSourceInfoByIdxFuzzTest,
        GetStreamSpanSizeFuzzTest,
        GetStreamRouteFlagFuzzTest,
        RegisterStatusCallbackFuzzTest,
        GetSourceOutputInfoFuzzTest,
        SetOffloadRenderCallbackTypeFuzzTest,
        SetSpatializationSceneTypeFuzzTest,
        EffectRotationUpdateFuzzTest,
        SetEffectSystemVolumeFuzzTest,
        SetAbsVolumeStateToEffectFuzzTest,
        UpdateParamExtraFuzzTest,
        GetMaxAmplitudeFuzzTest,
        SetAppsEnhanceMuteStateFuzzTest,
        IsChannelLayoutSupportedForDspEffectFuzzTest,
        UpdatePersonalizedHRTFBinFuzzTest,
        SetOutputDeviceSinkFuzzTest,
        SetVolumeInfoFuzzTest,
        HandleBypassSpatializationForStereoFuzzTest,
        SetAuxiliarySinkEnableFuzzTest,
        SetAissEnabledSyncFuzzTest,
        SetAissEnabledByUidSyncFuzzTest,
        SetVocalRatioSyncBoundaryFuzzTest,
        UpdateSpatializationStateStructFuzzTest,
        // --- B-class: HpaeRendererManager sub-object ---
        RendererCreateStreamFuzzTest,
        RendererCreateStreamWithInvalidEnumFuzzTest,
        RendererDestroyStreamFuzzTest,
        RendererStartFuzzTest,
        RendererStartWithSyncIdFuzzTest,
        RendererStartStopStartSequenceFuzzTest,
        RendererStopFuzzTest,
        RendererPauseFuzzTest,
        RendererPauseDrainPauseSequenceFuzzTest,
        RendererDrainFuzzTest,
        RendererFlushFuzzTest,
        RendererReleaseFuzzTest,
        RendererReleaseWithoutStartFuzzTest,
        RendererSetClientVolumeFuzzTest,
        RendererSetVolumeWithExtremeValuesFuzzTest,
        RendererSetVolumeWithNanInfinityFuzzTest,
        RendererSetRateFuzzTest,
        RendererSetRateWithInvalidValueFuzzTest,
        RendererSetSpeedFuzzTest,
        RendererSetSpeedWithExtremeValuesFuzzTest,
        RendererSetSpeedWithNanInfinityFuzzTest,
        RendererSetAudioEffectModeFuzzTest,
        RendererGetAudioEffectModeFuzzTest,
        RendererSetPrivacyTypeFuzzTest,
        RendererGetPrivacyTypeFuzzTest,
        RendererRegisterWriteCallbackFuzzTest,
        RendererRegisterReadCallbackFuzzTest,
        RendererUpdateSpatializationStateFuzzTest,
        RendererUpdateMaxLengthFuzzTest,
        RendererSetOffloadPolicyFuzzTest,
        RendererGetWritableSizeFuzzTest,
        RendererSetLoudnessGainFuzzTest,
        RendererPostCreateSessionEffectFuzzTest,
        RendererPostDestroySessionEffectFuzzTest,
        RendererPostSetSessionEffectParameterFuzzTest,
        RendererHasSessionEffectFuzzTest,
        RendererIsValidSessionFuzzTest,
        // --- B-class: HpaeCapturerManager sub-object ---
        CapturerCreateStreamFuzzTest,
        CapturerStartFuzzTest,
        CapturerStopFuzzTest,
        CapturerPauseFuzzTest,
        CapturerFlushFuzzTest,
        CapturerReleaseFuzzTest,
        CapturerDestroyStreamFuzzTest,
        CapturerRegisterReadCallbackFuzzTest,
        CapturerSetAppsEnhanceMuteStateFuzzTest,
    });
    func(fdp);
}

void Init()
{
    static bool registered = false;
    if (!registered) {
        registered = true;
        atexit(AtExitCleanup);
    }
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
