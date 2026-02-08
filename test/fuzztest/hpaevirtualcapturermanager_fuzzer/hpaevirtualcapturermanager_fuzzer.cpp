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
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_virtual_capturer_manager.h"
#include "hpae_source_output_node.h"
#include "i_stream.h"
#include "i_capturer_stream.h"
#include "hpaevirtualcapturermanager_fuzzer.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
using namespace std;

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960;

// Loop constants
constexpr int32_t MAX_THREE_ITERATIONS = 3;
constexpr int32_t MAX_SHORT_ITERATIONS = 5;
constexpr int32_t MAX_TEN_ITERATIONS = 10;
constexpr int32_t SESSION_ID_OFFSET_ONE = 1;
constexpr uint32_t ZERO_SESSION_ID = 0;
constexpr uint32_t LARGE_INVALID_SESSION_ID = 999999;

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_F32LE,
};

const std::vector<SourceType> SUPPORTED_SOURCE_TYPES {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_ULTRASONIC,
};

const std::vector<AudioStreamType> SUPPORTED_STREAM_TYPES {
    STREAM_DEFAULT,
    STREAM_MUSIC,
    STREAM_SPEECH,
    STREAM_RING,
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
void RoundVal(T &roundVal, const std::vector<T>& list)
{
    if (list.empty()) {
        return;
    }
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

void GetFuzzStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = GetData<AudioStreamType>();
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, SUPPORTED_FORMATS);
    RoundVal(streamInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(streamInfo.sourceType, SUPPORTED_SOURCE_TYPES);
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.uid = GetData<int32_t>();
}

void GetFuzzCaptureMoveInfo(HpaeCaptureMoveInfo &moveInfo)
{
    moveInfo.sessionId = GetData<uint32_t>();

    // Create proper HpaeNodeInfo for HpaeSourceOutputNode constructor
    HpaeNodeInfo nodeInfo;
    nodeInfo.format = GetData<AudioSampleFormat>();
    RoundVal(nodeInfo.format, SUPPORTED_FORMATS);
    nodeInfo.channels = GetData<AudioChannel>();
    RoundVal(nodeInfo.channels, SUPPORTED_CHANNELS);
    nodeInfo.samplingRate = GetData<AudioSamplingRate>();
    RoundVal(nodeInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    nodeInfo.sceneType = GetData<HpaeProcessorType>();
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;

    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    moveInfo.sessionInfo.sceneType = GetData<HpaeProcessorType>();
    moveInfo.sessionInfo.state = GetData<HpaeSessionState>();
    moveInfo.sessionInfo.isMoveAble = GetData<bool>();
}

void GetFuzzSourceOutputInfo(HpaeSourceOutputInfo &sourceOutputInfo)
{
    sourceOutputInfo.capturerSessionInfo.sceneType = GetData<HpaeProcessorType>();
    sourceOutputInfo.capturerSessionInfo.state = GetData<HpaeSessionState>();
    sourceOutputInfo.capturerSessionInfo.isMoveAble = GetData<bool>();
    sourceOutputInfo.nodeInfo.format = GetData<AudioSampleFormat>();
    sourceOutputInfo.nodeInfo.channels = GetData<AudioChannel>();
    sourceOutputInfo.nodeInfo.samplingRate = GetData<AudioSamplingRate>();
}

// Test function 1: Constructor
void ConstructorFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }
}

// Test function 2: CreateStream with various parameters
void CreateStreamFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);

    manager->CreateStream(streamInfo);
}

// Test function 3: CreateStream with boundary sessionId values
void CreateStreamBoundaryFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);

    // Test with boundary values
    streamInfo.sessionId = ZERO_SESSION_ID;
    manager->CreateStream(streamInfo);

    streamInfo.sessionId = UINT32_MAX;
    manager->CreateStream(streamInfo);
}

// Test function 4: DestroyStream
void DestroyStreamFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    // First create a stream
    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    // Then destroy it
    uint32_t sessionId = GetData<uint32_t>();
    manager->DestroyStream(sessionId);
}

// Test function 5: DestroyStream with non-existent sessionId
void DestroyStreamNonExistentFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    // Try to destroy non-existent streams
    manager->DestroyStream(UINT32_MAX);
    manager->DestroyStream(ZERO_SESSION_ID);
    manager->DestroyStream(LARGE_INVALID_SESSION_ID);
}

// Test function 6: Start stream
void StartFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Start(sessionId);
}

// Test function 7: Start non-existent stream
void StartNonExistentFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    manager->Start(UINT32_MAX);
    manager->Start(ZERO_SESSION_ID);
    manager->Start(LARGE_INVALID_SESSION_ID);
}

// Test function 8: Pause stream
void PauseFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Pause(sessionId);
}

// Test function 9: Flush stream (unsupported operation)
void FlushFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Flush(sessionId);
}

// Test function 10: Drain stream
void DrainFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Drain(sessionId);
}

// Test function 11: Stop stream
void StopFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Stop(sessionId);
}

// Test function 12: Release stream
void ReleaseFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    manager->Release(sessionId);
}

// Test function 13: SetStreamMute
void SetStreamMuteFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    bool isMute = GetData<bool>();
    manager->SetStreamMute(sessionId, isMute);
}

// Test function 14: SetStreamMute with non-existent stream
void SetStreamMuteNonExistentFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool isMute = GetData<bool>();
    manager->SetStreamMute(UINT32_MAX, isMute);
    manager->SetStreamMute(ZERO_SESSION_ID, isMute);
    manager->SetStreamMute(LARGE_INVALID_SESSION_ID, isMute);
}

// Test function 15: SetMute (unsupported operation)
void SetMuteFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool isMute = GetData<bool>();
    manager->SetMute(isMute);
}

// Test function 16: MoveStream
void MoveStreamFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    std::string sourceName = "test_source";
    manager->MoveStream(sessionId, sourceName);
}

// Test function 17: MoveStream with empty source name
void MoveStreamEmptyNameFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    std::string emptyName = "";
    manager->MoveStream(streamInfo.sessionId, emptyName);
}

// Test function 18: MoveAllStream
void MoveAllStreamFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    // Create multiple streams
    for (int i = 0; i < MAX_THREE_ITERATIONS; i++) {
        HpaeStreamInfo streamInfo;
        GetFuzzStreamInfo(streamInfo);
        streamInfo.sessionId = i + SESSION_ID_OFFSET_ONE;
        manager->CreateStream(streamInfo);
    }

    std::string sourceName = "test_source";
    std::vector<uint32_t> moveIds = {1, 2};
    MoveSessionType moveType = GetData<MoveSessionType>();
    manager->MoveAllStream(sourceName, moveIds, moveType);
}

// Test function 19: MoveAllStream with MOVE_ALL type
void MoveAllStreamMoveAllFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    // Create multiple streams
    for (int i = 0; i < MAX_THREE_ITERATIONS; i++) {
        HpaeStreamInfo streamInfo;
        GetFuzzStreamInfo(streamInfo);
        streamInfo.sessionId = i + SESSION_ID_OFFSET_ONE;
        manager->CreateStream(streamInfo);
    }

    std::string sourceName = "test_source";
    std::vector<uint32_t> emptyIds;
    manager->MoveAllStream(sourceName, emptyIds, MOVE_ALL);
}

// Test function 20: AddNodeToSource
void AddNodeToSourceFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeCaptureMoveInfo moveInfo;
    GetFuzzCaptureMoveInfo(moveInfo);
    manager->AddNodeToSource(moveInfo);
}

// Test function 21: AddAllNodesToSource
void AddAllNodesToSourceFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    std::vector<HpaeCaptureMoveInfo> moveInfos;
    for (int i = 0; i < MAX_THREE_ITERATIONS; i++) {
        HpaeCaptureMoveInfo moveInfo;
        GetFuzzCaptureMoveInfo(moveInfo);
        moveInfos.push_back(moveInfo);
    }

    bool isConnect = GetData<bool>();
    manager->AddAllNodesToSource(moveInfos, isConnect);
}

// Test function 22: AddAllNodesToSource with empty vector
void AddAllNodesToSourceEmptyFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    std::vector<HpaeCaptureMoveInfo> emptyMoveInfos;
    manager->AddAllNodesToSource(emptyMoveInfos, true);
    manager->AddAllNodesToSource(emptyMoveInfos, false);
}

// Test function 23: GetSourceOutputInfo
void GetSourceOutputInfoFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    HpaeSourceOutputInfo sourceOutputInfo;
    uint32_t sessionId = GetData<uint32_t>();
    manager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
}

// Test function 24: GetSourceOutputInfo with non-existent stream
void GetSourceOutputInfoNonExistentFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeSourceOutputInfo sourceOutputInfo;
    manager->GetSourceOutputInfo(UINT32_MAX, sourceOutputInfo);
    manager->GetSourceOutputInfo(ZERO_SESSION_ID, sourceOutputInfo);
    manager->GetSourceOutputInfo(LARGE_INVALID_SESSION_ID, sourceOutputInfo);
}

// Test function 25: GetSourceInfo
void GetSourceInfoFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeSourceInfo sourceInfo = manager->GetSourceInfo();
    (void)sourceInfo;
}

// Test function 26: GetAllSourceOutputsInfo
void GetAllSourceOutputsInfoFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    std::vector<SourceOutput> outputs = manager->GetAllSourceOutputsInfo();
    (void)outputs;
}

// Test function 27: OnNodeStatusUpdate
void OnNodeStatusUpdateFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = GetData<IOperation>();
    manager->OnNodeStatusUpdate(sessionId, operation);
}

// Test function 28: OnRequestLatency
void OnRequestLatencyFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = GetData<uint64_t>();
    manager->OnRequestLatency(sessionId, latency);
}

// Test function 29: Init (unsupported operation)
void InitFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool isReload = GetData<bool>();
    manager->Init(isReload);
}

// Test function 30: DeInit (unsupported operation)
void DeInitFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool isMoveDefault = GetData<bool>();
    manager->DeInit(isMoveDefault);
}

// Test function 31: IsInit
void IsInitFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool result = manager->IsInit();
    (void)result;
}

// Test function 32: IsRunning
void IsRunningFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool result = manager->IsRunning();
    (void)result;
}

// Test function 33: IsMsgProcessing
void IsMsgProcessingFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool result = manager->IsMsgProcessing();
    (void)result;
}

// Test function 34: DeactivateThread
void DeactivateThreadFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    bool result = manager->DeactivateThread();
    (void)result;
}

// Test function 35: GetThreadName
void GetThreadNameFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    std::string threadName = manager->GetThreadName();
    (void)threadName;
}

// Test function 36: StopManager
void StopManagerFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    manager->StopManager();
}

// Test function 37: Process (unsupported operation)
void ProcessFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    manager->Process();
}

// Test function 38: HandleMsg (unsupported operation)
void HandleMsgFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    manager->HandleMsg();
}

// Test function 39: RegisterReadCallback (unsupported operation)
void RegisterReadCallbackFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<ICapturerStreamCallback> callback;
    manager->RegisterReadCallback(sessionId, callback);
}

// Test function 40: Full lifecycle test
void FullLifecycleFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);

    // Create
    manager->CreateStream(streamInfo);

    // Start
    manager->Start(streamInfo.sessionId);

    // Pause
    manager->Pause(streamInfo.sessionId);

    // Stop
    manager->Stop(streamInfo.sessionId);

    // Release
    manager->Release(streamInfo.sessionId);
}

// Test function 41: Multiple streams operations
void MultipleStreamsFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    std::vector<uint32_t> sessionIds;
    // Create multiple streams
    for (int i = 0; i < MAX_SHORT_ITERATIONS; i++) {
        HpaeStreamInfo streamInfo;
        GetFuzzStreamInfo(streamInfo);
        streamInfo.sessionId = i + SESSION_ID_OFFSET_ONE;
        manager->CreateStream(streamInfo);
        sessionIds.push_back(streamInfo.sessionId);
    }

    // Start all streams
    for (auto sessionId : sessionIds) {
        manager->Start(sessionId);
    }

    // Stop all streams
    for (auto sessionId : sessionIds) {
        manager->Stop(sessionId);
    }

    // Release all streams
    for (auto sessionId : sessionIds) {
        manager->Release(sessionId);
    }
}

// Test function 42: State transition stress test
void StateTransitionStressFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    // Rapid state transitions
    for (int i = 0; i < MAX_TEN_ITERATIONS; i++) {
        manager->Start(streamInfo.sessionId);
        manager->Pause(streamInfo.sessionId);
        manager->Stop(streamInfo.sessionId);
        manager->Start(streamInfo.sessionId);
    }

    manager->Release(streamInfo.sessionId);
}

// Test function 43: Move stream operations
void MoveStreamOperationsFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    // Create stream
    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    // Move stream
    std::string sourceName = "new_source";
    manager->MoveStream(streamInfo.sessionId, sourceName);

    // Add back
    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = streamInfo.sessionId;

    // Create proper HpaeNodeInfo for HpaeSourceOutputNode constructor
    HpaeNodeInfo nodeInfo;
    nodeInfo.format = streamInfo.format;
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.samplingRate = streamInfo.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;

    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    moveInfo.sessionInfo.sceneType = HPAE_SCENE_DEFAULT;
    moveInfo.sessionInfo.state = HPAE_SESSION_NEW;
    manager->AddNodeToSource(moveInfo);
}

// Test function 44: SetStreamMute toggle
void SetStreamMuteToggleFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);
    manager->CreateStream(streamInfo);

    // Toggle mute multiple times
    for (int i = 0; i < MAX_TEN_ITERATIONS; i++) {
        manager->SetStreamMute(streamInfo.sessionId, true);
        manager->SetStreamMute(streamInfo.sessionId, false);
    }
}

// Test function 45: Boundary sessionId values
void BoundarySessionIdFuzzTest()
{
    auto manager = std::make_shared<HpaeVirtualCapturerManager>();
    if (manager == nullptr) {
        return;
    }

    HpaeStreamInfo streamInfo;
    GetFuzzStreamInfo(streamInfo);

    // Test with 0
    streamInfo.sessionId = ZERO_SESSION_ID;
    manager->CreateStream(streamInfo);
    manager->Start(ZERO_SESSION_ID);
    manager->Pause(ZERO_SESSION_ID);
    manager->Stop(ZERO_SESSION_ID);
    manager->Release(ZERO_SESSION_ID);

    // Test with UINT32_MAX
    streamInfo.sessionId = UINT32_MAX;
    manager->CreateStream(streamInfo);
    manager->Start(UINT32_MAX);
    manager->Pause(UINT32_MAX);
    manager->Stop(UINT32_MAX);
    manager->Release(UINT32_MAX);
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    ConstructorFuzzTest,
    CreateStreamFuzzTest,
    CreateStreamBoundaryFuzzTest,
    DestroyStreamFuzzTest,
    DestroyStreamNonExistentFuzzTest,
    StartFuzzTest,
    StartNonExistentFuzzTest,
    PauseFuzzTest,
    FlushFuzzTest,
    DrainFuzzTest,
    StopFuzzTest,
    ReleaseFuzzTest,
    SetStreamMuteFuzzTest,
    SetStreamMuteNonExistentFuzzTest,
    SetMuteFuzzTest,
    MoveStreamFuzzTest,
    MoveStreamEmptyNameFuzzTest,
    MoveAllStreamFuzzTest,
    MoveAllStreamMoveAllFuzzTest,
    AddNodeToSourceFuzzTest,
    AddAllNodesToSourceFuzzTest,
    AddAllNodesToSourceEmptyFuzzTest,
    GetSourceOutputInfoFuzzTest,
    GetSourceOutputInfoNonExistentFuzzTest,
    GetSourceInfoFuzzTest,
    GetAllSourceOutputsInfoFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnRequestLatencyFuzzTest,
    InitFuzzTest,
    DeInitFuzzTest,
    IsInitFuzzTest,
    IsRunningFuzzTest,
    IsMsgProcessingFuzzTest,
    DeactivateThreadFuzzTest,
    GetThreadNameFuzzTest,
    StopManagerFuzzTest,
    ProcessFuzzTest,
    HandleMsgFuzzTest,
    RegisterReadCallbackFuzzTest,
    FullLifecycleFuzzTest,
    MultipleStreamsFuzzTest,
    StateTransitionStressFuzzTest,
    MoveStreamOperationsFuzzTest,
    SetStreamMuteToggleFuzzTest,
    BoundarySessionIdFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;
    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    }
    return true;
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::HPAE::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::HPAE::FuzzTest(data, size);
    return 0;
}
