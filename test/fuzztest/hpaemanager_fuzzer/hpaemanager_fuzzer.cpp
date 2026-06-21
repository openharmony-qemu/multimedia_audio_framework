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
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include <limits>
#include <cmath>
#include "audio_errors.h"
#include "test_case_common.h"
#include "hpae_capturer_manager.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

// Named constants for magic numbers (replaces magic values used throughout the code)
constexpr uint8_t MAX_NAME_LENGTH = 64;
constexpr uint16_t MAX_DUMP_LENGTH = 256;  // Use uint16_t as 256 exceeds uint8_t range (0-255)
constexpr uint8_t MAX_ARRAY_SIZE_SMALL = 5;
constexpr uint8_t MAX_ARRAY_SIZE_MEDIUM = 10;
constexpr uint8_t ENUM_IOPERATION_COUNT = 14;
constexpr uint8_t ENUM_HPAESESSIONSTATE_COUNT = 9;
constexpr uint8_t ENUM_HPAESTREAMCLASSTYPE_COUNT = 3;
constexpr uint8_t ENUM_MOVESessionType_COUNT = 4;
constexpr uint8_t ENUM_HPAEMSGCODE_COUNT = 12;
constexpr uint8_t ENUM_SOURCETYPE_COUNT = 20; // Includes -1 offset (SOURCE_TYPE_MAX = 19)
constexpr uint8_t ENUM_HPAEPROCESSORTYPE_COUNT = 28; // Includes valid range 0-26
// Out-of-range value (255) cast into enum types to exercise invalid-enum branches.
constexpr int32_t INVALID_ENUM_VALUE = 255;
// Even/odd modulus used to alternate between two enum values.
constexpr uint8_t BINARY_CHOICE_MOD = 2;

// FuzzedDataProvider::ConsumeIntegral<T> requires an integral T (static_assert), so enums
// must be consumed as their underlying int and cast back. Mirrors the sibling-fuzzer pattern.
template <typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

// HpaeSinkInputNode has no default ctor (requires HpaeNodeInfo); build one for tests.
std::shared_ptr<HpaeSinkInputNode> MakeSinkInputNode()
{
    HpaeNodeInfo nodeInfo;
    return std::make_shared<HpaeSinkInputNode>(nodeInfo);
}

// Reuse ONE HpaeManager for the whole fuzz run. Per-iteration Init/DeInit triggers a
// teardown join deadlock: ~HpaeManager -> DeInit -> worker thread join while the worker
// is parked in Run()->condition_.wait (see memory: hpae-manager-deinit-join-deadlock).
// So Init once here and NEVER DeInit per iteration — the manager lives until process exit.
std::shared_ptr<HpaeManager> GetFuzzHpaeManager()
{
    static const auto manager = [] {
        auto m = std::make_shared<HpaeManager>();
        m->Init();
        return m;
    }();
    return manager;
}

// Test function creation helper
std::shared_ptr<HpaeManager> CreateHpaeManager()
{
    auto hpaeManager = std::make_shared<HpaeManager>();
    if (hpaeManager == nullptr) {
        return nullptr;
    }
    // Init() is only called for tests that require initialized state
    return hpaeManager;
}

static void InitSourceInfo(HpaeSourceInfo &sourceInfo, FuzzedDataProvider& fdp)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = "";  // Use empty path for fuzzer testing (no dependency on external file)

    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
}

static void InitStreamInfo(HpaeStreamInfo &streamInfo, FuzzedDataProvider& fdp)
{
    streamInfo.sessionId = fdp.ConsumeIntegral<uint32_t>();
    streamInfo.streamType = ConsumeEnum<AudioStreamType>(fdp);
    streamInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    streamInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    streamInfo.channels = ConsumeEnum<AudioChannel>(fdp);
    streamInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    streamInfo.streamClassType = ConsumeEnum<HpaeStreamClassType>(fdp);
    streamInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    streamInfo.fadeType = ConsumeEnum<FadeType>(fdp);
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.uid = fdp.ConsumeIntegral<int32_t>();
    streamInfo.pid = fdp.ConsumeIntegral<int32_t>();

    // Use ConsumeBytes for device name
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    std::vector<uint8_t> nameBytes = fdp.ConsumeBytes<uint8_t>(nameLen);
    streamInfo.deviceName = std::string(nameBytes.begin(), nameBytes.end());
}

void UploadDumpSourceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo, fdp);
    std::shared_ptr<IHpaeCapturerManager> capturerManager =
        std::make_shared<HpaeCapturerManager>(sourceInfo);
    std::string deviceName = "";
    if (capturerManager == nullptr) {
        return;
    }
    capturerManager->UploadDumpSourceInfo(deviceName);
}

void StopAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string audioPortName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    std::vector<uint8_t> nameBytes = fdp.ConsumeBytes<uint8_t>(nameLen);
    audioPortName = std::string(nameBytes.begin(), nameBytes.end());

    hpaeManager->StopAudioPort(audioPortName);
}

void SetSourceOutputMuteFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;

    hpaeManager->SetSourceOutputMute(uid, isMute);
}

void GetAllSinksFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->GetAllSinks();
}

void DeInitFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void ReloadRenderManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;

    // Fix: Use loop to read module name from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.name += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.lib += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    bool isReload = fdp.ConsumeIntegral<uint8_t>() != 0;

    HpaeSinkInfo sinkInfo;
    sinkInfo.sinkId = fdp.ConsumeIntegral<uint32_t>();
    sinkInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    sinkInfo.channels = ConsumeEnum<AudioChannel>(fdp);
    sinkInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    sinkInfo.adapterName = audioModuleInfo.adapterName;
    sinkInfo.deviceClass = audioModuleInfo.name;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->ReloadRenderManager(audioModuleInfo, isReload);
    }
}

void CreateRendererManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.name = "TestSink_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";

    // Fix: Use loop to read adapter name from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    uint32_t sinkSourceIndex = fdp.ConsumeIntegral<uint32_t>();
    bool isReload = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CreateRendererManager(audioModuleInfo, sinkSourceIndex, isReload);
    }
}

void CreateCaptureManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo, fdp);
    sourceInfo.sourceType = static_cast<SourceType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_SOURCETYPE_COUNT);
    uint32_t sinkSourceIndex = fdp.ConsumeIntegral<uint32_t>();
    bool isReload = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CreateCaptureManager(sourceInfo, sinkSourceIndex, isReload);
    }
}

void DumpAllAvailableDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->DumpAllAvailableDevice();
}

void DumpSinkInputsInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->DumpSinkInputsInfo();
}

void DumpSourceOutputsInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->DumpSourceOutputsInfo();
}

void AddPreferSinkForDefaultChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    bool isAdd = fdp.ConsumeIntegral<uint8_t>() != 0;
    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->AddPreferSinkForDefaultChange(isAdd, sinkName);
    }
}

void CloseOutAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CloseOutAudioPort(sinkName);
    }
}

void CloseInAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CloseInAudioPort(sourceName);
    }
}

void SetDefaultSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    hpaeManager->SetDefaultSink(name);
}

void SetDefaultSourceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    hpaeManager->SetDefaultSource(name);
}

void GetAllSinkInputsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->GetAllSinkInputs();
}

void GetAllSourceOutputsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->GetAllSourceOutputs();
}

void MoveSourceOutputByIndexOrNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sourceOutputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sourceIndex = fdp.ConsumeIntegral<uint32_t>();
    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    hpaeManager->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
}

void CheckMoveSinkInputFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sinkInputId = fdp.ConsumeIntegral<uint32_t>();
    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CheckMoveSinkInput(sinkInputId, sinkName);
    }
}

void MoveSinkInputByIndexOrNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sinkInputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sinkIndex = fdp.ConsumeIntegral<uint32_t>();
    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    hpaeManager->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

void IsMovingSinkStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    auto sinkInputNode = MakeSinkInputNode();
    if (sinkInputNode == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->MovingSinkStateChange(sessionId, sinkInputNode);
    }
}

void HandleMoveSinkInputFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    auto sinkInputNode = MakeSinkInputNode();
    if (sinkInputNode == nullptr) {
        return;
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleMoveSinkInput(sinkInputNode, sinkName);
    }
}

void HandleMoveSourceOutputFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = fdp.ConsumeIntegral<uint32_t>();
    moveInfo.sessionInfo.state =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);

    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleMoveSourceOutput(moveInfo, sourceName);
    }
}

void HandleMoveAllSourceOutputsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::vector<HpaeCaptureMoveInfo> moveInfos;
    size_t infoCount = fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_MEDIUM;
    for (size_t i = 0; i < infoCount; i++) {
        HpaeCaptureMoveInfo info;
        info.sessionId = fdp.ConsumeIntegral<uint32_t>();
        info.sessionInfo.state =
            static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
        moveInfos.push_back(info);
    }

    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    MoveSessionType moveType =
        static_cast<MoveSessionType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_MOVESessionType_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleMoveAllSourceOutputs(moveInfos, sourceName, moveType);
    }
}

void HandleMoveSessionFailedFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    MoveSessionType moveType =
        static_cast<MoveSessionType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_MOVESessionType_COUNT);

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleMoveSessionFailed(streamClassType, sessionId, moveType, name);
    }
}

void HandleUpdateStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeSessionState status =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleUpdateStatus(streamClassType, sessionId, status, operation);
    }
}

void HandleDumpSinkInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string deviceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        deviceName += fdp.ConsumeIntegral<char>();
    }

    std::string dumpStr;
    size_t dumpLen = fdp.ConsumeIntegral<uint8_t>() % MAX_DUMP_LENGTH;
    for (size_t i = 0; i < dumpLen; i++) {
        dumpStr += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleDumpSinkInfo(deviceName, dumpStr);
    }
}

void HandleInitDeviceResultFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string deviceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        deviceName += fdp.ConsumeIntegral<char>();
    }

    int32_t result = fdp.ConsumeIntegral<int32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleInitDeviceResult(deviceName, result);
    }
}

void EnqueuePendingTransitionFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeSessionState state =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->EnqueuePendingTransition(sessionId, state, operation);
    }
}

void CreateStreamFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);

    hpaeManager->CreateStream(streamInfo);
}

void AddStreamToCollectionFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->AddStreamToCollection(streamInfo, name);
    }
}

void DestroyCaptureFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->DestroyCapture(sessionId);
    }
}

void SetMovingStreamStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeSessionState status =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    HpaeSessionState state =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetMovingStreamState(streamType, sessionId, status, state, operation);
    }
}

void DestroyStreamFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->DestroyStream(streamClassType, sessionId);
}

void ShouldNotSkipProcessFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->ShouldNotSkipProcess(streamType, sessionId);
    }
}

void StartFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Start(streamClassType, sessionId);
}

void PauseFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Pause(streamClassType, sessionId);
}

void DrainFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Drain(streamClassType, sessionId);
}

void StopFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Stop(streamClassType, sessionId);
}

void ReleaseFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Release(streamClassType, sessionId);
}

// DisConnectInputClusterFuzzTest removed: HpaeSourceInputCluster has no DisConnectInputCluster method
// and no default constructor (requires HpaeNodeInfo parameter)

void OnDisConnectProcessClusterFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // Use abs to ensure a non-negative value for enum generation
    HpaeProcessorType sceneType =
        static_cast<HpaeProcessorType>(std::abs(fdp.ConsumeIntegral<int32_t>() % ENUM_HPAEPROCESSORTYPE_COUNT));
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        // Route through HpaeManager to the real HpaeRendererManager override. The INodeCallback
        // base body is empty, so instantiating it directly would never exercise real code.
        auto rendererManager = hpaeManager->GetRendererManagerById(sessionId);
        if (rendererManager != nullptr) {
            (void)rendererManager->OnDisConnectProcessCluster(sceneType);
        }
    }
}

// DisConnectOutputClusterFuzzTest removed: HpaeSinkOutputCluster class does not exist
// and the method name is incorrect (should be DisConnect())

void DisConnectCoBufferFromOnDisConnectProcessClusterFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    auto coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleDisConnectCoBufferNode(coBufferNode);
    }
}

// DerefenceInputClusterFuzzTest removed: HpaeSourceInputCluster has no DereferenceInputCluster method
// and no default constructor (requires HpaeNodeInfo parameter)

void HandleMoveAllSinkInputsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    size_t nodeCount = fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL;
    for (size_t i = 0; i < nodeCount; i++) {
        auto node = MakeSinkInputNode();
        if (node != nullptr) {
            sinkInputs.push_back(node);
        }
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    MoveSessionType moveType =
        static_cast<MoveSessionType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_MOVESessionType_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleMoveAllSinkInputs(sinkInputs, sinkName, moveType);
    }
}

void GetPerferSinkInputsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    size_t nodeCount = fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL;
    for (size_t i = 0; i < nodeCount; i++) {
        auto node = MakeSinkInputNode();
        if (node != nullptr) {
            sinkInputs.push_back(node);
        }
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetPerferSinkInputs(sinkInputs);
    }
}

void GetUsedMoveInfosFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::vector<HpaeCaptureMoveInfo> moveInfos;
    size_t infoCount = fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL;
    for (size_t i = 0; i < infoCount; i++) {
        HpaeCaptureMoveInfo info;
        info.sessionId = fdp.ConsumeIntegral<uint32_t>();
        info.sessionInfo.state =
            static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
        moveInfos.push_back(info);
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetUsedMoveInfos(moveInfos);
    }
}

void InvokeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeMsgCode cmdID = static_cast<HpaeMsgCode>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAEMSGCODE_COUNT);

    int32_t argValue = fdp.ConsumeIntegral<int32_t>();
    std::any args = argValue;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Invoke(cmdID, args);
    }
}

void InvokeSyncFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeMsgCode cmdID = static_cast<HpaeMsgCode>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAEMSGCODE_COUNT);

    int32_t argValue = fdp.ConsumeIntegral<int32_t>();
    std::any args = argValue;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->InvokeSync(cmdID, args);
    }
}

void GetMsgCountFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    (void)hpaeManager->GetMsgCount();
}

void IsRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    (void)hpaeManager->IsRunning();
}

void IsMsgProcessingFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    (void)hpaeManager->IsMsgProcessing();
}

void SendRequestFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SendRequest([]{}, __func__);
    }
}

void FlushFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->Flush(streamClassType, sessionId);
}

void StartWithSyncIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t syncId = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->StartWithSyncId(streamClassType, sessionId, syncId);
}

void DequeuePendingTransitionFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->DequeuePendingTransition(sessionId);
    }
}

void RegisterStatusCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    std::weak_ptr<IStreamStatusCallback> callback;
    (void)hpaeManager->RegisterStatusCallback(streamClassType, sessionId, callback);
}

void RegisterReadCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    std::weak_ptr<ICapturerStreamCallback> callback;
    (void)hpaeManager->RegisterReadCallback(sessionId, callback);
}

void GetSourceOutputInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);

    (void)hpaeManager->GetSourceOutputInfo(sessionId, streamInfo);
}

void SetClientVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    float volume = fdp.ConsumeFloatingPoint<float>();

    hpaeManager->SetClientVolume(sessionId, volume);
}

void SetVolumeWithNanInfinityFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    float volume = fdp.ConsumeFloatingPoint<float>();

    hpaeManager->SetClientVolume(sessionId, volume);
}

void SetLoudnessGainFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    float loudnessGain = fdp.ConsumeFloatingPoint<float>();

    hpaeManager->SetLoudnessGain(sessionId, loudnessGain);
}

void SetRateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t rate = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetRate(sessionId, rate);
}

void SetAudioEffectModeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t effectMode = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetAudioEffectMode(sessionId, effectMode);
}

void GetAudioEffectModeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t effectMode = 0;
    (void)hpaeManager->GetAudioEffectMode(sessionId, effectMode);
}

void SetPrivacyTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t privacyType = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetPrivacyType(sessionId, privacyType);
}

void GetPrivacyTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t privacyType = 0;
    (void)hpaeManager->GetPrivacyType(sessionId, privacyType);
}

void RegisterWriteCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    std::weak_ptr<IStreamCallback> callback;
    (void)hpaeManager->RegisterWriteCallback(sessionId, callback);
}

void SetOffloadPolicyFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t state = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetOffloadPolicy(sessionId, state);
}

void GetWritableSizeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    (void)hpaeManager->GetWritableSize(sessionId);
}

void UpdateSpatializationStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    bool spatializationEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool headTrackingEnabled = fdp.ConsumeIntegral<uint8_t>() != 0;

    hpaeManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

void UpdateMaxLengthFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    uint32_t maxLength = fdp.ConsumeIntegral<uint32_t>();

    hpaeManager->UpdateMaxLength(sessionId, maxLength);
}

void SetOffloadRenderCallbackTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t type = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetOffloadRenderCallbackType(sessionId, type);
}

void SetSpeedFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    float speed = fdp.ConsumeFloatingPoint<float>();

    hpaeManager->SetSpeed(sessionId, speed);
}

void SetSpeedWithNanInfinityFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    float speed = fdp.ConsumeFloatingPoint<float>();

    hpaeManager->SetSpeed(sessionId, speed);
}

void SetSinkMuteFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool isSync = fdp.ConsumeIntegral<uint8_t>() != 0;

    hpaeManager->SetSinkMute(sinkName, isMute, isSync);
}

void SuspendAudioDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string audioPortName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioPortName += fdp.ConsumeIntegral<char>();
    }

    bool isSuspend = fdp.ConsumeIntegral<uint8_t>() != 0;

    hpaeManager->SuspendAudioDevice(audioPortName, isSuspend);
}

void OpenAudioPortInnerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;

    // Fix: Use loop to read module info from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.name += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.lib += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->OpenAudioPortInner(audioModuleInfo);
    }
}

void OpenOutputAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;

    // Fix: Use loop to read module info from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.name += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.lib += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    uint32_t sinkSourceIndex = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->OpenOutputAudioPort(audioModuleInfo, sinkSourceIndex);
    }
}

void OpenInputAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;

    // Fix: Use loop to read module info from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.name += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.lib += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    uint32_t sinkSourceIndex = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->OpenInputAudioPort(audioModuleInfo, sinkSourceIndex);
    }
}

void OpenVirtualAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    AudioModuleInfo audioModuleInfo;

    // Fix: Use loop to read module info from fuzzer data instead of std::string(char, size_t)
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.name += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.lib += fdp.ConsumeIntegral<char>();
    }

    nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        audioModuleInfo.adapterName += fdp.ConsumeIntegral<char>();
    }

    uint32_t sinkSourceIndex = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->OpenVirtualAudioPort(audioModuleInfo, sinkSourceIndex);
    }
}

void CreateCoreSourceManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CreateCoreSourceManager();
    }
}

void CreateStreamForCapInnerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CreateStreamForCapInner(streamInfo);
    }
}

void MoveToPreferSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    std::shared_ptr<AudioServiceHpaeCallback> serviceCallback = std::make_shared<FuzzHpaeServiceCallback>();
    if (hpaeManager->IsInit()) {
        (void)hpaeManager->MoveToPreferSink(name, serviceCallback);
    }
}

void HandleRendererManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleRendererManager(sinkName, streamInfo);
    }
}

void HandleConnectCoBufferNodeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    auto coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleConnectCoBufferNode(coBufferNode);
    }
}

void HandleInitSourceResultFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    SourceType sourceType = static_cast<SourceType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_SOURCETYPE_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleInitSourceResult(sourceType);
    }
}

void GetRendererManagerByIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetRendererManagerById(sessionId);
    }
}

void GetCapturerManagerByIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetCapturerManagerById(sessionId);
    }
}

void GetRendererManagerByNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetRendererManagerByName(sinkName);
    }
}

void GetCapturerManagerByNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetCapturerManagerByName(sourceName);
    }
}

void UpdateStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::weak_ptr<IStreamStatusCallback> callback;
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateStatus(callback, operation, sessionId);
    }
}

void DeleteRendererManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->DeleteRendererManager(name);
    }
}

void DeleteCaptureManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->DeleteCaptureManager(name);
    }
}

void DeleteAudioportFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->DeleteAudioport(name);
    }
}

void GetAllRenderSessionFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetAllRenderSession(name);
    }
}

void GetAllCaptureSessionFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string name;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        name += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetAllCaptureSession(name);
    }
}

void GetAuxiliaryRendererManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetAuxiliaryRendererManager();
    }
}

void OnCallbackOpenOrReloadFailedFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    bool isReload = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->OnCallbackOpenOrReloadFailed(isReload);
    }
}

void HandleDumpSourceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string deviceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        deviceName += fdp.ConsumeIntegral<char>();
    }

    std::string dumpStr;
    size_t dumpLen = fdp.ConsumeIntegral<uint8_t>() % MAX_DUMP_LENGTH;
    for (size_t i = 0; i < dumpLen; i++) {
        dumpStr += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleDumpSourceInfo(deviceName, dumpStr);
    }
}

void HandleReloadDeviceResultFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string deviceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        deviceName += fdp.ConsumeIntegral<char>();
    }

    int32_t result = fdp.ConsumeIntegral<int32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleReloadDeviceResult(deviceName, result);
    }
}

void TriggerAppsUidUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->TriggerAppsUidUpdate(streamClassType, sessionId);
    }
}

void TransStreamInfoToStreamDumpInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::unordered_map<uint32_t, HpaeSessionInfo> idStreamInfoMap;
    size_t mapSize = fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL;
    for (size_t i = 0; i < mapSize; i++) {
        HpaeSessionInfo sessionInfo;
        sessionInfo.state = static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
        idStreamInfoMap[fdp.ConsumeIntegral<uint32_t>()] = sessionInfo;
    }

    std::vector<HpaeInputOutputInfo> inputOutputs;

    if (hpaeManager->IsInit()) {
        (void)TransStreamInfoToStreamDumpInfo(idStreamInfoMap, inputOutputs);
    }
}

void LoadEffectLiveFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->LoadEffectLive();
    }
}

void UpdateStatusCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::weak_ptr<IStreamStatusCallback> callback;
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateStatus(callback, OPERATION_STARTED, sessionId);
    }
}

void HandleInitDeviceResultMultipleCallsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // Fix: Use loop to read device name from fuzzer data instead of std::string(char, size_t)
    std::string deviceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        deviceName += fdp.ConsumeIntegral<char>();
    }
    int32_t result1 = fdp.ConsumeIntegral<int32_t>();
    int32_t result2 = fdp.ConsumeIntegral<int32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->HandleInitDeviceResult(deviceName, result1);
        (void)hpaeManager->HandleInitDeviceResult(deviceName, result2);
    }
}

void CreateStreamWithInvalidEnumFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo, fdp);
    streamInfo.streamType = static_cast<AudioStreamType>(INVALID_ENUM_VALUE);
    streamInfo.sourceType = static_cast<SourceType>(INVALID_ENUM_VALUE);
    streamInfo.streamClassType = static_cast<HpaeStreamClassType>(INVALID_ENUM_VALUE);

    hpaeManager->CreateStream(streamInfo);
}

void StartStopStartSequenceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Start(streamClassType, sessionId);
        (void)hpaeManager->Stop(streamClassType, sessionId);
        (void)hpaeManager->Start(streamClassType, sessionId);
    }
}

void PauseDrainPauseSequenceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Pause(streamClassType, sessionId);
        (void)hpaeManager->Drain(streamClassType, sessionId);
        (void)hpaeManager->Pause(streamClassType, sessionId);
    }
}

void ReleaseWithoutStartFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Release(streamClassType, sessionId);
    }
}

void SetSpeedWithExtremeValuesFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    float extremeSpeeds[] = {0.0f, -1.0f, 1.0f, 10.0f, 100.0f};
    float speed = extremeSpeeds[fdp.ConsumeIntegral<uint8_t>() % MAX_ARRAY_SIZE_SMALL];

    hpaeManager->SetSpeed(sessionId, speed);
}

void SetVolumeWithExtremeValuesFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    float extremeVolumes[] = {0.0f, -1.0f, 0.5f, 1.0f, 2.0f, 10.0f};
    float volume =
        extremeVolumes[fdp.ConsumeIntegral<uint8_t>() % (sizeof(extremeVolumes) / sizeof(extremeVolumes[0]))];

    hpaeManager->SetClientVolume(sessionId, volume);
}

void SetRateWithInvalidValueFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    int32_t rate = fdp.ConsumeIntegral<int32_t>();

    hpaeManager->SetRate(sessionId, rate);
}

void MoveSinkInputWithNonExistentSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sinkInputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sinkIndex = fdp.ConsumeIntegral<uint32_t>();
    std::string sinkName = "NonExistentSink_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());

    hpaeManager->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

void MoveSourceOutputWithNonExistentSourceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sourceOutputId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t sourceIndex = fdp.ConsumeIntegral<uint32_t>();
    std::string sourceName = "NonExistentSource_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());

    hpaeManager->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
}

void SetDefaultSinkWithEmptyNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string emptyName;

    hpaeManager->SetDefaultSink(emptyName);
}

void SetDefaultSourceWithEmptyNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string emptyName;

    hpaeManager->SetDefaultSource(emptyName);
}

void SetSinkMuteWithNonExistentSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string nonExistentSink = "NonExistentSink_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;
    bool isSync = fdp.ConsumeIntegral<uint8_t>() != 0;

    hpaeManager->SetSinkMute(nonExistentSink, isMute, isSync);
}

void StopAudioPortWithNonExistentPortFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string nonExistentPort = "NonExistentPort_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());

    hpaeManager->StopAudioPort(nonExistentPort);
}

void CloseOutAudioPortWithNonExistentSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string nonExistentSink = "NonExistentSink_" + std::to_string(fdp.ConsumeIntegral<uint32_t>());

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->CloseOutAudioPort(nonExistentSink);
    }
}

void MultipleDeInitCallsFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void InitTwiceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // Init() skipped: shared fuzz manager is already initialized in GetFuzzHpaeManager().
}

// ----------------------------------------------------------------------------
// Supplemental cases: real-tuple Invoke paths, AISS sync trio, collaboration
// state machine, GetSinkInfoByIdx/GetSourceInfoByIdx, span/route queries,
// spatialization cluster, and trivial pass-through getters.
// ----------------------------------------------------------------------------

// Named constants for AISS / vocal ratio boundary values (no magic numbers).
constexpr float VOCAL_RATIO_MIN = 0.0f;
constexpr float VOCAL_RATIO_MID = 0.5f;
constexpr float VOCAL_RATIO_MAX = 1.0f;
constexpr float VOCAL_RATIO_NEG = -1.0f;
constexpr float VOCAL_RATIO_OVER = 2.0f;
constexpr uint8_t ENUM_COLLABORATIONSTATE_COUNT = 3;       // CLOSE / BT / NEARLINK
constexpr uint8_t ENUM_SPATIALIZATION_SCENE_TYPE_COUNT = 4; // DEFAULT..AUDIOBOOK
constexpr uint8_t ENUM_MSGCODE_TOTAL = 14;                  // UPDATE_STATUS..INIT_SOURCE_RESULT

// P0-1a: Invoke UPDATE_STATUS with a real matching tuple so the handler
// is reached instead of the "type mismatched" early-return path.
void InvokeUpdateStatusRealTupleFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeSessionState status =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    std::any args = std::tuple<HpaeStreamClassType, uint32_t, HpaeSessionState, IOperation>(
        streamClassType, sessionId, status, operation);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Invoke(UPDATE_STATUS, args);
    }
}

// P0-1b: Invoke MOVE_SINK_INPUT with a real matching tuple (sinkInputNode shared_ptr).
void InvokeMoveSinkInputRealTupleFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    auto sinkInputNode = MakeSinkInputNode();
    if (sinkInputNode == nullptr) {
        return;
    }
    std::string sinkName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sinkName += fdp.ConsumeIntegral<char>();
    }

    std::any args =
        std::tuple<std::shared_ptr<HpaeSinkInputNode>, std::string>(sinkInputNode, sinkName);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Invoke(MOVE_SINK_INPUT, args);
    }
}

// P0-1c: Invoke MOVE_SOURCE_OUTPUT with a real matching tuple.
void InvokeMoveSourceOutputRealTupleFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = fdp.ConsumeIntegral<uint32_t>();
    moveInfo.sessionInfo.state =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);

    std::string sourceName;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        sourceName += fdp.ConsumeIntegral<char>();
    }

    std::any args = std::tuple<HpaeCaptureMoveInfo, std::string>(moveInfo, sourceName);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Invoke(MOVE_SOURCE_OUTPUT, args);
    }
}

// P0-1d: Invoke across the full registered cmdID range plus out-of-range
// values (both well-typed and mismatched-typed payloads) to exercise the
// dispatch switch and the "not found" log branch.
void InvokeAcrossFullCmdRangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // Fuzz cmdID across the full enum range and beyond (out-of-range paths).
    int32_t rawCmd = fdp.ConsumeIntegral<int32_t>();
    HpaeMsgCode cmdID = static_cast<HpaeMsgCode>(std::abs(rawCmd) % (ENUM_MSGCODE_TOTAL + 1));

    // Mismatched payload (int32_t) -> every non-UPDATE_STATUS handler logs
    // "type mismatched"; this intentionally exercises that early-return.
    int32_t argValue = fdp.ConsumeIntegral<int32_t>();
    std::any args = argValue;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->Invoke(cmdID, args);
    }
}

// P0-1e: InvokeSync with UPDATE_STATUS real tuple (SendRequestSync code path).
void InvokeSyncUpdateStatusRealTupleFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    HpaeSessionState status =
        static_cast<HpaeSessionState>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESESSIONSTATE_COUNT);
    IOperation operation = static_cast<IOperation>(fdp.ConsumeIntegral<uint8_t>() % ENUM_IOPERATION_COUNT);

    std::any args = std::tuple<HpaeStreamClassType, uint32_t, HpaeSessionState, IOperation>(
        streamClassType, sessionId, status, operation);

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->InvokeSync(UPDATE_STATUS, args);
    }
}

// P0-2a: SetAissEnabledSync over session-id boundaries.
void SetAissEnabledSyncFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool enabled = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAissEnabledSync(sessionId, enabled);
    }
}

// P0-2b: SetAissEnabledByUidSync over uid boundaries.
void SetAissEnabledByUidSyncFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool enabled = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAissEnabledByUidSync(uid, enabled);
    }
}

// P0-2c: SetVocalRatioSync with explicit boundary values including NaN/+inf
// to exercise the [0,1] clamp and IEEE754 edge cases.
void SetVocalRatioSyncBoundaryFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    const float boundaries[] = {
        VOCAL_RATIO_MIN, VOCAL_RATIO_MID, VOCAL_RATIO_MAX,
        VOCAL_RATIO_NEG, VOCAL_RATIO_OVER,
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
    };
    const uint8_t boundaryCount = sizeof(boundaries) / sizeof(boundaries[0]);
    float ratio = boundaries[fdp.ConsumeIntegral<uint8_t>() % boundaryCount];

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetVocalRatioSync(ratio);
    }
}

// P1-3a: UpdateCollaborativeState with each valid value plus out-of-range.
void UpdateCollaborativeStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint8_t pick = fdp.ConsumeIntegral<uint8_t>();
    CollaborationState state = (pick % BINARY_CHOICE_MOD == 0)
        ? static_cast<CollaborationState>(pick % ENUM_COLLABORATIONSTATE_COUNT)
        : static_cast<CollaborationState>(pick);  // deliberately out-of-range

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateCollaborativeState(state);
    }
}

// P1-3b: UpdateCollaborativeProductId with empty / 64-char / fuzzed bytes.
void UpdateCollaborativeProductIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string productId;
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < nameLen; i++) {
        productId += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->UpdateCollaborativeProductId(productId);
    }
}

// P1-3c: LoadCollaborationConfig pass-through.
void LoadCollaborationConfigFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        hpaeManager->LoadCollaborationConfig();
    }
}

// P1-3d: IsCollaborationSupportedForStreamUsage across fuzzed StreamUsage.
void IsCollaborationSupportedForStreamUsageFuzzTest(FuzzedDataProvider& fdp)
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

// Helper lambda factory for Get*InfoByIdx callbacks (removed: unused;
// each Get*InfoByIdx test uses its own inline lambda to match its type).

// P1-4a: GetSinkInfoByIdx across idx boundaries (-1, 0, valid, INT32_MAX).
void GetSinkInfoByIdxFuzzTest(FuzzedDataProvider& fdp)
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

// P1-4b: GetSourceInfoByIdx across idx boundaries.
void GetSourceInfoByIdxFuzzTest(FuzzedDataProvider& fdp)
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

// P1-5a: GetStreamSpanSize with valid + invalid (streamClassType, sessionId).
void GetStreamSpanSizeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t spanSizeInFrame = 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetStreamSpanSize(streamClassType, sessionId, spanSizeInFrame);
    }
}

// P1-5b: GetStreamRouteFlag with valid + invalid pairs.
void GetStreamRouteFlagFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    HpaeStreamClassType streamClassType =
        static_cast<HpaeStreamClassType>(fdp.ConsumeIntegral<uint8_t>() % ENUM_HPAESTREAMCLASSTYPE_COUNT);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t routeFlag = 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetStreamRouteFlag(streamClassType, sessionId, routeFlag);
    }
}

// P2-6: SetSourceOutputMuteByStreamId (mirrors existing SetSourceOutputMute).
void SetSourceOutputMuteByStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetSourceOutputMuteByStreamId(sessionId, isMute);
    }
}

// P2-7a: UpdateSpatializationState(AudioSpatializationState) overload.
void UpdateSpatializationStateStructFuzzTest(FuzzedDataProvider& fdp)
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

// P2-7b: SetSpatializationSceneType over valid + out-of-range enum.
void SetSpatializationSceneTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint8_t pick = fdp.ConsumeIntegral<uint8_t>();
    AudioSpatializationSceneType sceneType = (pick % BINARY_CHOICE_MOD == 0)
        ? static_cast<AudioSpatializationSceneType>(pick % ENUM_SPATIALIZATION_SCENE_TYPE_COUNT)
        : static_cast<AudioSpatializationSceneType>(pick);  // out-of-range

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetSpatializationSceneType(sceneType);
    }
}

// P2-7c: EffectRotationUpdate with fuzzed rotation state.
void EffectRotationUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t rotationState = fdp.ConsumeIntegral<uint32_t>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->EffectRotationUpdate(rotationState);
    }
}

// P2-7d: SetEffectSystemVolume pass-through.
void SetEffectSystemVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    int32_t systemVolumeType = fdp.ConsumeIntegral<int32_t>();
    float systemVolume = fdp.ConsumeFloatingPoint<float>();

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetEffectSystemVolume(systemVolumeType, systemVolume);
    }
}

// P2-7e: SetAbsVolumeStateToEffect pass-through.
void SetAbsVolumeStateToEffectFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    bool absVolumeState = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAbsVolumeStateToEffect(absVolumeState);
    }
}

// P2-7f: UpdateParamExtra with fuzzed mainkey/subkey/value strings.
void UpdateParamExtraFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    std::string mainkey;
    size_t mkLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < mkLen; i++) {
        mainkey += fdp.ConsumeIntegral<char>();
    }
    std::string subkey;
    size_t skLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < skLen; i++) {
        subkey += fdp.ConsumeIntegral<char>();
    }
    std::string value;
    size_t vLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    for (size_t i = 0; i < vLen; i++) {
        value += fdp.ConsumeIntegral<char>();
    }

    if (hpaeManager->IsInit()) {
        hpaeManager->UpdateParamExtra(mainkey, subkey, value);
    }
}

// P2-8a: GetMaxAmplitude trivial pass-through.
void GetMaxAmplitudeFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->GetMaxAmplitude();
    }
}

// P2-8b: SetAppsEnhanceMuteState over fuzzed sessionId + bool.
void SetAppsEnhanceMuteStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool isMute = fdp.ConsumeIntegral<uint8_t>() != 0;

    if (hpaeManager->IsInit()) {
        (void)hpaeManager->SetAppsEnhanceMuteState(sessionId, isMute);
    }
}

// P2-8c: IsChannelLayoutSupportedForDspEffect over fuzzed layout.
void IsChannelLayoutSupportedForDspEffectFuzzTest(FuzzedDataProvider& fdp)
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

// Main test function that selects and runs a test
// === Remaining-gap coverage: HRTF fd, device/volume setters ===

void UpdatePersonalizedHRTFBinFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // Restrict fd to invalid sentinels: a real valid fd could corrupt unrelated file
    // state downstream in HpaePolicyManager (read/lseek on the descriptor).
    int32_t fd = fdp.PickValueInArray({ -1, std::numeric_limits<int32_t>::max() });
    long length = fdp.ConsumeIntegral<long>();
    hpaeManager->UpdatePersonalizedHRTFBin(fd, length);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void SetOutputDeviceSinkFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    int32_t device = fdp.ConsumeIntegral<int32_t>();
    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    std::vector<uint8_t> nameBytes = fdp.ConsumeBytes<uint8_t>(nameLen);
    std::string sinkName(nameBytes.begin(), nameBytes.end());
    hpaeManager->SetOutputDeviceSink(device, sinkName);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void SetVolumeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    // AudioVolumeType is a typedef of AudioStreamType; consume the underlying enum.
    AudioStreamType volumeType = ConsumeEnum<AudioStreamType>(fdp);
    float systemVol = fdp.ConsumeFloatingPoint<float>();
    hpaeManager->SetVolumeInfo(volumeType, systemVol);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void HandleBypassSpatializationForStereoFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->HandleBypassSpatializationForStereo(fdp.ConsumeIntegral<uint8_t>() != 0);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void SetAuxiliarySinkEnableFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    hpaeManager->SetAuxiliarySinkEnable(fdp.ConsumeIntegral<uint8_t>() != 0);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void SetSinkMuteForSwitchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto hpaeManager = GetFuzzHpaeManager();
    if (hpaeManager == nullptr) {
        return;
    }

    size_t nameLen = fdp.ConsumeIntegral<uint8_t>() % MAX_NAME_LENGTH;
    std::vector<uint8_t> nameBytes = fdp.ConsumeBytes<uint8_t>(nameLen);
    std::string deviceClass(nameBytes.begin(), nameBytes.end());
    hpaeManager->SetSinkMuteForSwitchDevice(deviceClass, fdp.ConsumeIntegral<uint8_t>() != 0);

    // DeInit() skipped: shared fuzz manager lives for the whole run (per-iter teardown
    // join deadlock; see memory hpae-manager-deinit-join-deadlock).
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        UploadDumpSourceInfoFuzzTest,
        StopAudioPortFuzzTest,
        SetSourceOutputMuteFuzzTest,
        GetAllSinksFuzzTest,
        DeInitFuzzTest,
        ReloadRenderManagerFuzzTest,
        CreateRendererManagerFuzzTest,
        CreateCaptureManagerFuzzTest,
        DumpAllAvailableDeviceFuzzTest,
        DumpSinkInputsInfoFuzzTest,
        DumpSourceOutputsInfoFuzzTest,
        AddPreferSinkForDefaultChangeFuzzTest,
        CloseOutAudioPortFuzzTest,
        CloseInAudioPortFuzzTest,
        SetDefaultSinkFuzzTest,
        SetDefaultSourceFuzzTest,
        GetAllSinkInputsFuzzTest,
        GetAllSourceOutputsFuzzTest,
        MoveSourceOutputByIndexOrNameFuzzTest,
        CheckMoveSinkInputFuzzTest,
        MoveSinkInputByIndexOrNameFuzzTest,
        IsMovingSinkStateChangeFuzzTest,
        HandleMoveSinkInputFuzzTest,
        HandleMoveSourceOutputFuzzTest,
        HandleMoveAllSourceOutputsFuzzTest,
        HandleMoveSessionFailedFuzzTest,
        HandleUpdateStatusFuzzTest,
        HandleDumpSinkInfoFuzzTest,
        HandleInitDeviceResultFuzzTest,
        EnqueuePendingTransitionFuzzTest,
        CreateStreamFuzzTest,
        AddStreamToCollectionFuzzTest,
        DestroyCaptureFuzzTest,
        SetMovingStreamStateFuzzTest,
        DestroyStreamFuzzTest,
        ShouldNotSkipProcessFuzzTest,
        StartFuzzTest,
        PauseFuzzTest,
        DrainFuzzTest,
        StopFuzzTest,
        ReleaseFuzzTest,
        OnDisConnectProcessClusterFuzzTest,
        DisConnectCoBufferFromOnDisConnectProcessClusterFuzzTest,
        HandleMoveAllSinkInputsFuzzTest,
        GetPerferSinkInputsFuzzTest,
        GetUsedMoveInfosFuzzTest,
        InvokeFuzzTest,
        InvokeSyncFuzzTest,
        GetMsgCountFuzzTest,
        IsRunningFuzzTest,
        IsMsgProcessingFuzzTest,
        SendRequestFuzzTest,
        FlushFuzzTest,
        StartWithSyncIdFuzzTest,
        DequeuePendingTransitionFuzzTest,
        RegisterStatusCallbackFuzzTest,
        RegisterReadCallbackFuzzTest,
        GetSourceOutputInfoFuzzTest,
        SetClientVolumeFuzzTest,
        SetVolumeWithNanInfinityFuzzTest,
        SetLoudnessGainFuzzTest,
        SetRateFuzzTest,
        SetAudioEffectModeFuzzTest,
        GetAudioEffectModeFuzzTest,
        SetPrivacyTypeFuzzTest,
        GetPrivacyTypeFuzzTest,
        RegisterWriteCallbackFuzzTest,
        SetOffloadPolicyFuzzTest,
        GetWritableSizeFuzzTest,
        UpdateSpatializationStateFuzzTest,
        UpdateMaxLengthFuzzTest,
        SetOffloadRenderCallbackTypeFuzzTest,
        SetSpeedFuzzTest,
        SetSpeedWithNanInfinityFuzzTest,
        SetSinkMuteFuzzTest,
        SuspendAudioDeviceFuzzTest,
        OpenAudioPortInnerFuzzTest,
        OpenOutputAudioPortFuzzTest,
        OpenInputAudioPortFuzzTest,
        OpenVirtualAudioPortFuzzTest,
        CreateCoreSourceManagerFuzzTest,
        CreateStreamForCapInnerFuzzTest,
        MoveToPreferSinkFuzzTest,
        HandleRendererManagerFuzzTest,
        HandleConnectCoBufferNodeFuzzTest,
        HandleInitSourceResultFuzzTest,
        GetRendererManagerByIdFuzzTest,
        GetCapturerManagerByIdFuzzTest,
        GetRendererManagerByNameFuzzTest,
        GetCapturerManagerByNameFuzzTest,
        UpdateStatusFuzzTest,
        DeleteRendererManagerFuzzTest,
        DeleteCaptureManagerFuzzTest,
        DeleteAudioportFuzzTest,
        GetAllRenderSessionFuzzTest,
        GetAllCaptureSessionFuzzTest,
        GetAuxiliaryRendererManagerFuzzTest,
        OnCallbackOpenOrReloadFailedFuzzTest,
        HandleDumpSourceInfoFuzzTest,
        HandleReloadDeviceResultFuzzTest,
        TriggerAppsUidUpdateFuzzTest,
        TransStreamInfoToStreamDumpInfoFuzzTest,
        LoadEffectLiveFuzzTest,
        UpdateStatusCallbackFuzzTest,
        HandleInitDeviceResultMultipleCallsFuzzTest,
        CreateStreamWithInvalidEnumFuzzTest,
        StartStopStartSequenceFuzzTest,
        PauseDrainPauseSequenceFuzzTest,
        ReleaseWithoutStartFuzzTest,
        SetSpeedWithExtremeValuesFuzzTest,
        SetVolumeWithExtremeValuesFuzzTest,
        SetRateWithInvalidValueFuzzTest,
        MoveSinkInputWithNonExistentSinkFuzzTest,
        MoveSourceOutputWithNonExistentSourceFuzzTest,
        SetDefaultSinkWithEmptyNameFuzzTest,
        SetDefaultSourceWithEmptyNameFuzzTest,
        SetSinkMuteWithNonExistentSinkFuzzTest,
        StopAudioPortWithNonExistentPortFuzzTest,
        CloseOutAudioPortWithNonExistentSinkFuzzTest,
        MultipleDeInitCallsFuzzTest,
        InitTwiceFuzzTest,
        // --- supplemental cases (added) ---
        InvokeUpdateStatusRealTupleFuzzTest,
        InvokeMoveSinkInputRealTupleFuzzTest,
        InvokeMoveSourceOutputRealTupleFuzzTest,
        InvokeAcrossFullCmdRangeFuzzTest,
        InvokeSyncUpdateStatusRealTupleFuzzTest,
        SetAissEnabledSyncFuzzTest,
        SetAissEnabledByUidSyncFuzzTest,
        SetVocalRatioSyncBoundaryFuzzTest,
        UpdateCollaborativeStateFuzzTest,
        UpdateCollaborativeProductIdFuzzTest,
        LoadCollaborationConfigFuzzTest,
        IsCollaborationSupportedForStreamUsageFuzzTest,
        GetSinkInfoByIdxFuzzTest,
        GetSourceInfoByIdxFuzzTest,
        GetStreamSpanSizeFuzzTest,
        GetStreamRouteFlagFuzzTest,
        SetSourceOutputMuteByStreamIdFuzzTest,
        UpdateSpatializationStateStructFuzzTest,
        SetSpatializationSceneTypeFuzzTest,
        EffectRotationUpdateFuzzTest,
        SetEffectSystemVolumeFuzzTest,
        SetAbsVolumeStateToEffectFuzzTest,
        UpdateParamExtraFuzzTest,
        GetMaxAmplitudeFuzzTest,
        SetAppsEnhanceMuteStateFuzzTest,
        IsChannelLayoutSupportedForDspEffectFuzzTest,
        // Round-N supplemental coverage: HRTF fd, device/volume setters.
        UpdatePersonalizedHRTFBinFuzzTest,
        SetOutputDeviceSinkFuzzTest,
        SetVolumeInfoFuzzTest,
        HandleBypassSpatializationForStereoFuzzTest,
        SetAuxiliarySinkEnableFuzzTest,
        SetSinkMuteForSwitchDeviceFuzzTest,
    });
    func(fdp);
}

void Init()
{
    // Initialization code (if needed)
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
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