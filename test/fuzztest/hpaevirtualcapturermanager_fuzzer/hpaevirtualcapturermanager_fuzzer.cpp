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
#include <vector>
#include <memory>
#include <string>
#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "hpae_virtual_capturer_manager.h"
#include "hpae_source_output_node.h"
#include "audio_engine_log.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_SESSION_ID = 123456;
const uint32_t MAX_MULTI_STREAM_COUNT = 5;
const uint32_t MIN_MULTI_STREAM_COUNT = 1;
const uint32_t MAX_MOVE_ALL_STREAM_COUNT = 4;
const uint32_t MIN_MOVE_ALL_STREAM_COUNT = 2;
const uint32_t MAX_ADD_NODE_COUNT = 3;
const uint32_t MIN_ADD_NODE_COUNT = 1;

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO, STEREO, CHANNEL_3, CHANNEL_4, CHANNEL_5, CHANNEL_6,
    CHANNEL_7, CHANNEL_8, CHANNEL_9, CHANNEL_10, CHANNEL_11,
    CHANNEL_12, CHANNEL_13, CHANNEL_14, CHANNEL_15, CHANNEL_16,
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
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

void RoundStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, AUDIO_SUPPORTED_FORMATS);
}

void InitStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

HpaeNodeInfo GetFuzzNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = GetData<uint32_t>();
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    return nodeInfo;
}

HpaeCaptureMoveInfo GetHpaeCaptureMoveInfo()
{
    HpaeCaptureMoveInfo moveInfo;
    HpaeNodeInfo nodeInfo = GetFuzzNodeInfo();
    moveInfo.sessionId = GetData<uint32_t>();
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    return moveInfo;
}

// --- Test functions ---

void CreateAndDestroyStreamFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();
    mgr->CreateStream(streamInfo);
    mgr->DestroyStream(streamInfo.sessionId);
}

void CreateStreamWithFuzzedInfoFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.channels = GetData<AudioChannel>();
    streamInfo.format = GetData<AudioSampleFormat>();
    streamInfo.sessionId = GetData<uint32_t>();
    mgr->CreateStream(streamInfo);
    mgr->DestroyStream(streamInfo.sessionId);
}

void FullLifecycleFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);

    mgr->CreateStream(streamInfo);
    mgr->Start(streamInfo.sessionId);
    mgr->Pause(streamInfo.sessionId);
    mgr->Start(streamInfo.sessionId);
    mgr->Drain(streamInfo.sessionId);
    mgr->Stop(streamInfo.sessionId);
    mgr->Release(streamInfo.sessionId);
}

void FullLifecycleFuzzedSessionIdFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);

    mgr->CreateStream(streamInfo);

    uint32_t fuzzSessionId = GetData<uint32_t>();
    mgr->Start(fuzzSessionId);
    mgr->Pause(fuzzSessionId);
    mgr->Stop(fuzzSessionId);
    mgr->Drain(fuzzSessionId);
    mgr->Release(fuzzSessionId);
}

void MultipleStreamsLifecycleFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    std::vector<uint32_t> sessionIds;

    int count = (GetData<uint32_t>() % MAX_MULTI_STREAM_COUNT) + MIN_MULTI_STREAM_COUNT;
    for (int i = 0; i < count; i++) {
        HpaeStreamInfo streamInfo;
        InitStreamInfo(streamInfo);
        streamInfo.sessionId = GetData<uint32_t>();
        mgr->CreateStream(streamInfo);
        sessionIds.push_back(streamInfo.sessionId);
    }

    for (auto sid : sessionIds) {
        mgr->Start(sid);
    }
    for (auto sid : sessionIds) {
        mgr->Stop(sid);
    }
    for (auto sid : sessionIds) {
        mgr->DestroyStream(sid);
    }
}

void MoveStreamFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);

    std::string sourceName = "test_source";
    mgr->MoveStream(streamInfo.sessionId, sourceName);
}

void MoveStreamInvalidSessionFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t invalidSessionId = GetData<uint32_t>();
    std::string sourceName = "test_source";
    mgr->MoveStream(invalidSessionId, sourceName);
}

void MoveAllStreamFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    std::vector<uint32_t> sessionIds;

    int count = (GetData<uint32_t>() % MAX_MOVE_ALL_STREAM_COUNT) + MIN_MOVE_ALL_STREAM_COUNT;
    for (int i = 0; i < count; i++) {
        HpaeStreamInfo streamInfo;
        InitStreamInfo(streamInfo);
        streamInfo.sessionId = GetData<uint32_t>();
        mgr->CreateStream(streamInfo);
        sessionIds.push_back(streamInfo.sessionId);
    }

    std::string sourceName = "test_source";
    mgr->MoveAllStream(sourceName, sessionIds, MOVE_ALL);
}

void MoveAllStreamWithMoveTypeFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);

    std::string sourceName = "test_source";
    std::vector<uint32_t> sessionIds = {streamInfo.sessionId};
    MoveSessionType moveType = GetData<MoveSessionType>();
    mgr->MoveAllStream(sourceName, sessionIds, moveType);
}

void SetStreamMuteFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);

    bool isMute = GetData<bool>();
    mgr->SetStreamMute(streamInfo.sessionId, isMute);
    mgr->DestroyStream(streamInfo.sessionId);
}

void SetStreamMuteInvalidSessionFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t invalidSessionId = GetData<uint32_t>();
    bool isMute = GetData<bool>();
    mgr->SetStreamMute(invalidSessionId, isMute);
}

void SetMuteFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    bool isMute = GetData<bool>();
    mgr->SetMute(isMute);
}

void GetSourceOutputInfoFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);

    HpaeSourceOutputInfo sourceOutputInfo;
    mgr->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    mgr->DestroyStream(streamInfo.sessionId);
}

void GetSourceOutputInfoInvalidSessionFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t invalidSessionId = GetData<uint32_t>();
    HpaeSourceOutputInfo sourceOutputInfo;
    mgr->GetSourceOutputInfo(invalidSessionId, sourceOutputInfo);
}

void AddNodeToSourceFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeCaptureMoveInfo moveInfo;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = GetData<uint32_t>();
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    moveInfo.sessionId = GetData<uint32_t>();
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    mgr->AddNodeToSource(moveInfo);
    mgr->DestroyStream(moveInfo.sessionId);
}

void AddAllNodesToSourceFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    int count = (GetData<uint32_t>() % MAX_ADD_NODE_COUNT) + MIN_ADD_NODE_COUNT;
    for (int i = 0; i < count; i++) {
        HpaeNodeInfo nodeInfo;
        nodeInfo.nodeId = GetData<uint32_t>();
        nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
        nodeInfo.samplingRate = SAMPLE_RATE_48000;
        nodeInfo.channels = STEREO;
        nodeInfo.format = SAMPLE_S16LE;
        nodeInfo.sceneType = HPAE_SCENE_RECORD;
        HpaeCaptureMoveInfo moveInfo;
        moveInfo.sessionId = GetData<uint32_t>();
        moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
        moveInfos.push_back(moveInfo);
    }
    bool isConnect = GetData<bool>();
    mgr->AddAllNodesToSource(moveInfos, isConnect);
}

void OnNodeStatusUpdateFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);

    IOperation operation = GetData<bool>() ? OPERATION_STOPPED : OPERATION_PAUSED;
    mgr->OnNodeStatusUpdate(streamInfo.sessionId, operation);
    mgr->DestroyStream(streamInfo.sessionId);
}

void OnNodeStatusUpdateInvalidSessionFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t invalidSessionId = GetData<uint32_t>();
    mgr->OnNodeStatusUpdate(invalidSessionId, OPERATION_STOPPED);
}

void ProcessAndHandleMsgFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    mgr->Process();
    mgr->HandleMsg();
    mgr->OnNotifyQueue();
    mgr->GetThreadName();
}

void InitDeInitFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    mgr->Init();
    mgr->DeInit();
    mgr->IsInit();
    mgr->IsRunning();
    mgr->IsMsgProcessing();
    mgr->DeactivateThread();
    mgr->StopManager();
}

void GetAllSourceOutputsInfoFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    mgr->GetAllSourceOutputsInfo();
    mgr->GetSourceInfo();
    mgr->DestroyStream(streamInfo.sessionId);
}

void OnRequestLatencyFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = 0;
    mgr->OnRequestLatency(sessionId, latency);
}

void DumpSourceInfoFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    mgr->DumpSourceInfo();
    mgr->GetDeviceHDFDumpInfo();
    mgr->ReloadCaptureManager(HpaeSourceInfo());
}

void RegisterReadCallbackFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t sessionId = GetData<uint32_t>();
    std::weak_ptr<ICapturerStreamCallback> callback;
    mgr->RegisterReadCallback(sessionId, callback);
}

void AddCaptureInjectorFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    SourceType sourceType = GetData<SourceType>();
    mgr->AddCaptureInjector(nullptr, sourceType);
    mgr->RemoveCaptureInjector(nullptr, sourceType);
}

void FlushDrainWithoutStreamFuzzTest()
{
    auto mgr = std::make_shared<HpaeVirtualCapturerManager>();
    uint32_t sessionId = GetData<uint32_t>();
    mgr->Flush(sessionId);
    mgr->Drain(sessionId);
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    CreateAndDestroyStreamFuzzTest,
    CreateStreamWithFuzzedInfoFuzzTest,
    FullLifecycleFuzzTest,
    FullLifecycleFuzzedSessionIdFuzzTest,
    MultipleStreamsLifecycleFuzzTest,
    MoveStreamFuzzTest,
    MoveStreamInvalidSessionFuzzTest,
    MoveAllStreamFuzzTest,
    MoveAllStreamWithMoveTypeFuzzTest,
    SetStreamMuteFuzzTest,
    SetStreamMuteInvalidSessionFuzzTest,
    SetMuteFuzzTest,
    GetSourceOutputInfoFuzzTest,
    GetSourceOutputInfoInvalidSessionFuzzTest,
    AddNodeToSourceFuzzTest,
    AddAllNodesToSourceFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnNodeStatusUpdateInvalidSessionFuzzTest,
    ProcessAndHandleMsgFuzzTest,
    InitDeInitFuzzTest,
    GetAllSourceOutputsInfoFuzzTest,
    OnRequestLatencyFuzzTest,
    DumpSourceInfoFuzzTest,
    RegisterReadCallbackFuzzTest,
    AddCaptureInjectorFuzzTest,
    FlushDrainWithoutStreamFuzzTest,
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

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}