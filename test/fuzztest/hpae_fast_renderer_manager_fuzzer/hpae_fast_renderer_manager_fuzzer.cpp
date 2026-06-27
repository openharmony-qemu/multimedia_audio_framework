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

#include "hpae_fast_renderer_manager_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <atomic>

#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_fast_renderer_manager.h"
#include "hpae_fast_sink_output_node.h"
#include "hpae_info.h"
#include "hpae_sink_input_node.h"
#include "i_hpae_renderer_manager.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
constexpr size_t FUZZ_INPUT_SIZE_THRESHOLD = 32;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t MAX_MSG_PROCESSING_WAIT_LOOPS = 10;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t DEFAULT_NODE_ID = 1;
const char* DEFAULT_TEST_DEVICE_CLASS = "primary_mmap";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";

constexpr uint64_t DEFAULT_REWIND_TIME_MS = 1000;
constexpr uint64_t DEFAULT_HDI_FRAME_POSITION = 500;

constexpr float NORMAL_PLAYBACK_SPEED = 1.0f;
constexpr float FAST_PLAYBACK_SPEED = 1.5f;

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8,
};

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

vector<MoveSessionType> MoveSessionTypeVec = {
    MOVE_SINGLE,
    MOVE_ALL,
    MOVE_PREFER,
    MOVE_DEFAULT,
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
        roundVal = GetData<T>();
        return;
    }
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

static void InitHpaeSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = "/data/fast_renderer_fuzz_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    RoundVal(sinkInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(sinkInfo.format, SUPPORTED_FORMATS);
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = DEFAULT_TEST_DEVICE_CLASS;
}

static void InitRenderStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    RoundVal(streamInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(streamInfo.format, SUPPORTED_FORMATS);
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
}

static void InitNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S32LE;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeFastRendererManager> &manager)
{
    if (!manager->IsInit()) {
        return;
    }
    for (int i = 0; i < MAX_MSG_PROCESSING_WAIT_LOOPS && manager->IsMsgProcessing(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

int32_t WriteFixedDataCb::OnStreamData(AudioCallBackStreamInfo& callBackStremInfo)
{
    return SUCCESS;
}

ReadDataCb::ReadDataCb(const std::string &fileName)
{
    testFile_ = fopen(fileName.c_str(), "ab");
    if (testFile_ == nullptr) {
        AUDIO_ERR_LOG("Open file failed");
    }
}

ReadDataCb::~ReadDataCb()
{
    if (testFile_) {
        fclose(testFile_);
        testFile_ = nullptr;
    }
}

int32_t ReadDataCb::OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo)
{
    return SUCCESS;
}

// ============================================================================
// Constructor / Lifecycle Tests
// ============================================================================

void HpaeFastRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeFastRendererManager fastManager(sinkInfo);
}

void HpaeFastRendererManagerIsInitBeforeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->IsInit();
    fastManager->IsRunning();
    fastManager->IsMsgProcessing();
}

void HpaeFastRendererManagerInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    bool isReload = GetData<bool>();
    fastManager->Init(isReload);
    WaitForMsgProcessing(fastManager);
    fastManager->IsInit();
    fastManager->DeInit();
}

void HpaeFastRendererManagerDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);
    bool isMoveDefault = GetData<bool>();
    fastManager->DeInit(isMoveDefault);
}

void HpaeFastRendererManagerDoubleDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
    fastManager->DeInit();
}

void HpaeFastRendererManagerDeInitWithoutInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->DeInit();
}

// ============================================================================
// Stream Lifecycle Tests
// ============================================================================

void HpaeFastRendererManagerCreateStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    fastManager->CreateStream(streamInfo);
    WaitForMsgProcessing(fastManager);

    fastManager->DeInit();
}

void HpaeFastRendererManagerCreateStreamNotInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    fastManager->CreateStream(streamInfo);
}

void HpaeFastRendererManagerDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->DestroyStream(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerCreateDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);
    fastManager->IsInit();

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId : GetData<uint32_t>();

    fastManager->DestroyStream(sessionId);
    WaitForMsgProcessing(fastManager);

    InitRenderStreamInfo(streamInfo);
    fastManager->CreateStream(streamInfo);
    WaitForMsgProcessing(fastManager);

    fastManager->DestroyStream(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->DeInit();
}

// ============================================================================
// Stream Control Tests
// ============================================================================

void HpaeFastRendererManagerStartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Start(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerPauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Pause(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Flush(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Drain(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerStopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Stop(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Release(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerStartPauseStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    fastManager->CreateStream(streamInfo);
    WaitForMsgProcessing(fastManager);

    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId : GetData<uint32_t>();
    fastManager->RegisterWriteCallback(sessionId, writeIncDataCb);

    fastManager->Start(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->IsRunning();

    fastManager->Pause(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->Stop(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->DestroyStream(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

// ============================================================================
// Stream Movement Tests
// ============================================================================

void HpaeFastRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "test_target_sink";
    fastManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerMoveStreamSyncFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, so MoveStreamSync is called directly
    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "test_target_sink";
    fastManager->MoveStream(sessionId, sinkName);
}

void HpaeFastRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    string sinkName = "test_target_sink";
    vector<uint32_t> sessionIds;
    size_t count = GetData<size_t>() % 5;
    for (size_t i = 0; i < count; ++i) {
        sessionIds.push_back(GetData<uint32_t>());
    }

    MoveSessionType moveType = GetData<MoveSessionType>();
    if (moveType >= MOVE_SINGLE && moveType <= MOVE_DEFAULT) {
        fastManager->MoveAllStream(sinkName, sessionIds, moveType);
    }
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerMoveAllStreamNotInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, uses sync mode
    string sinkName = "test_target_sink";
    vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    fastManager->MoveAllStream(sinkName, sessionIds, moveType);
}

// ============================================================================
// Node Management Tests
// ============================================================================

void HpaeFastRendererManagerAddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    fastManager->AddNodeToSink(node);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerAddNodeToSinkNullFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    std::shared_ptr<HpaeSinkInputNode> nullNode = nullptr;
    fastManager->AddNodeToSink(nullNode);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerAddAllNodesToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(node);
    bool isConnect = GetData<bool>();
    fastManager->AddAllNodesToSink(sinkInputs, isConnect);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerAddAllNodesToSinkEmptyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> emptyNodes;
    bool isConnect = GetData<bool>();
    fastManager->AddAllNodesToSink(emptyNodes, isConnect);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

// ============================================================================
// Manager Control Tests
// ============================================================================

void HpaeFastRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    bool isSuspend = GetData<bool>();
    fastManager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(fastManager);

    // Suspend again with same state
    fastManager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(fastManager);

    fastManager->DeInit();
}

void HpaeFastRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    bool isMute = GetData<bool>();
    fastManager->SetMute(isMute);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerSetMuteForSwitchDeviceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    bool mute = GetData<bool>();
    fastManager->SetMuteForSwitchDevice(mute);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerSetMuteForSwitchDeviceNotInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, sinkOutputNode_ is nullptr
    bool mute = GetData<bool>();
    fastManager->SetMuteForSwitchDevice(mute);
}

void HpaeFastRendererManagerStopManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    fastManager->StopManager();
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerStopManagerNotInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, sinkOutputNode_ is nullptr
    fastManager->StopManager();
}

// ============================================================================
// Volume / Rate / Effect Tests
// ============================================================================

void HpaeFastRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    fastManager->SetClientVolume(sessionId, volume);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    fastManager->SetRate(sessionId, rate);
}

void HpaeFastRendererManagerSetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    fastManager->SetAudioEffectMode(sessionId, effectMode);
}

void HpaeFastRendererManagerGetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    fastManager->GetAudioEffectMode(sessionId, effectMode);
}

void HpaeFastRendererManagerSetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    fastManager->SetPrivacyType(sessionId, privacyType);
}

void HpaeFastRendererManagerGetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    fastManager->GetPrivacyType(sessionId, privacyType);
}

void HpaeFastRendererManagerSetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    fastManager->SetSpeed(sessionId, speed);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    fastManager->SetLoudnessGain(sessionId, loudnessGain);
}

void HpaeFastRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    fastManager->GetWritableSize(sessionId);
}

void HpaeFastRendererManagerUpdateSpatializationStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    fastManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

void HpaeFastRendererManagerUpdateMaxLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    fastManager->UpdateMaxLength(sessionId, maxLength);
}

// ============================================================================
// Callback Tests
// ============================================================================

void HpaeFastRendererManagerRegisterWriteCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    fastManager->RegisterWriteCallback(sessionId, writeIncDataCb);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerRegisterWriteCallbackNullFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<WriteFixedDataCb> nullCallback = nullptr;
    fastManager->RegisterWriteCallback(sessionId, nullCallback);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>("/data/test_fuzz.pcm");
    fastManager->RegisterReadCallback(sessionId, readDataCb);
}

// ============================================================================
// INodeCallback Tests
// ============================================================================

void HpaeFastRendererManagerOnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = static_cast<IOperation>(GetData<int32_t>());
    fastManager->OnNodeStatusUpdate(sessionId, operation);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerOnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->OnFadeDone(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerOnRequestLatencyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = GetData<uint64_t>();
    fastManager->OnRequestLatency(sessionId, latency);
}

void HpaeFastRendererManagerOnRewindAndFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint64_t rewindTime = GetData<uint64_t>();
    uint64_t hdiFramePosition = GetData<uint64_t>();
    fastManager->OnRewindAndFlush(rewindTime, hdiFramePosition);
}

void HpaeFastRendererManagerOnNotifyQueueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, thread is nullptr
    fastManager->OnNotifyQueue();
}

// ============================================================================
// Query Tests
// ============================================================================

void HpaeFastRendererManagerGetAllSinkInputsInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->GetAllSinkInputsInfo();
}

void HpaeFastRendererManagerGetSinkInputInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    HpaeSinkInputInfo sinkInputInfo;
    fastManager->GetSinkInputInfo(sessionId, sinkInputInfo);
}

void HpaeFastRendererManagerRefreshProcessClusterByDeviceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->RefreshProcessClusterByDevice();
}

void HpaeFastRendererManagerGetSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->GetSinkInfo();
}

void HpaeFastRendererManagerGetSpanSizeInFrameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t spanSizeInFrame = 0;
    fastManager->GetSpanSizeInFrame(sessionId, spanSizeInFrame);
}

void HpaeFastRendererManagerGetThreadNameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->GetThreadName();
}

void HpaeFastRendererManagerDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->DumpSinkInfo();
}

void HpaeFastRendererManagerGetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->GetDeviceHDFDumpInfo();
}

void HpaeFastRendererManagerGetMaxAmplitudeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->GetMaxAmplitude();
}

// ============================================================================
// Reload Tests
// ============================================================================

void HpaeFastRendererManagerReloadRenderManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }

    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    bool isReload = GetData<bool>();
    fastManager->ReloadRenderManager(newSinkInfo, isReload);
    WaitForMsgProcessing(fastManager);

    fastManager->DeInit();
}

void HpaeFastRendererManagerReloadRenderManagerNotInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized, ReloadRenderManager will create thread
    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    bool isReload = GetData<bool>();
    fastManager->ReloadRenderManager(newSinkInfo, isReload);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

// ============================================================================
// Process / Thread Tests
// ============================================================================

void HpaeFastRendererManagerProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Not initialized
    fastManager->Process();
}

void HpaeFastRendererManagerHandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->HandleMsg();
}

void HpaeFastRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    fastManager->DeactivateThread();
    WaitForMsgProcessing(fastManager);
}

// ============================================================================
// App UID / Loopback Tests
// ============================================================================

void HpaeFastRendererManagerTriggerAppsUidUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t sessionId = GetData<uint32_t>();
    fastManager->TriggerAppsUidUpdate(sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerCreateStreamWithLoopbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.isLoopback = GetData<bool>();
    fastManager->CreateStream(streamInfo);
    WaitForMsgProcessing(fastManager);

    fastManager->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

// ============================================================================
// Boundary / Invalid State Tests
// ============================================================================

void HpaeFastRendererManagerBoundarySessionIdFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    uint32_t boundarySessions[] = {0, UINT32_MAX, 1, UINT32_MAX - 1};
    for (uint32_t sessionId : boundarySessions) {
        fastManager->Start(sessionId);
        fastManager->Pause(sessionId);
        fastManager->Stop(sessionId);
        fastManager->Release(sessionId);
    }

    float extremeFloats[] = {-1000.0f, -1.0f, 0.0f, 0.5f, 1.0f, 2.0f, 1000.0f};
    for (float val : extremeFloats) {
        fastManager->SetClientVolume(1, val);
        fastManager->SetSpeed(1, val);
    }

    WaitForMsgProcessing(fastManager);
    fastManager->DeInit();
}

void HpaeFastRendererManagerOperationsBeforeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    // Test operations before Init
    uint32_t sessionId = GetData<uint32_t>();
    fastManager->Start(sessionId);
    fastManager->Pause(sessionId);
    fastManager->Stop(sessionId);
    fastManager->Release(sessionId);
    fastManager->SetMute(GetData<bool>());
    fastManager->GetSinkInfo();
    fastManager->IsInit();
    fastManager->IsRunning();
    fastManager->IsMsgProcessing();
    fastManager->Process();
    fastManager->HandleMsg();
}

// ============================================================================
// Full Lifecycle Test
// ============================================================================

void HpaeFastRendererManagerFullLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }

    fastManager->Init();
    WaitForMsgProcessing(fastManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;

    fastManager->CreateStream(streamInfo);
    WaitForMsgProcessing(fastManager);

    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    fastManager->RegisterWriteCallback(sessionId, writeIncDataCb);

    fastManager->Start(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->SetSpeed(sessionId, NORMAL_PLAYBACK_SPEED);
    WaitForMsgProcessing(fastManager);

    fastManager->Pause(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->Flush(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->Stop(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->DestroyStream(sessionId);
    WaitForMsgProcessing(fastManager);

    fastManager->DeInit();
}

// ============================================================================
// Simple Operations Test (no init needed)
// ============================================================================

void HpaeFastRendererManagerSimpleOperationReturnsFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto fastManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    if (fastManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    fastManager->SetClientVolume(sessionId, GetData<float>());
    fastManager->SetRate(sessionId, GetData<int32_t>());
    fastManager->SetAudioEffectMode(sessionId, GetData<int32_t>());
    int32_t effectMode = 0;
    fastManager->GetAudioEffectMode(sessionId, effectMode);
    fastManager->SetPrivacyType(sessionId, GetData<int32_t>());
    int32_t privacyType = 0;
    fastManager->GetPrivacyType(sessionId, privacyType);
    fastManager->UpdateSpatializationState(sessionId, GetData<bool>(), GetData<bool>());
    fastManager->UpdateMaxLength(sessionId, GetData<uint32_t>());
    fastManager->RefreshProcessClusterByDevice();
    fastManager->SetLoudnessGain(sessionId, GetData<float>());
    fastManager->GetWritableSize(sessionId);
    fastManager->GetAllSinkInputsInfo();
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    HpaeFastRendererManagerConstructFuzzTest,
    HpaeFastRendererManagerIsInitBeforeInitFuzzTest,
    HpaeFastRendererManagerInitFuzzTest,
    HpaeFastRendererManagerDeInitFuzzTest,
    HpaeFastRendererManagerDoubleDeInitFuzzTest,
    HpaeFastRendererManagerDeInitWithoutInitFuzzTest,
    HpaeFastRendererManagerCreateStreamFuzzTest,
    HpaeFastRendererManagerCreateStreamNotInitFuzzTest,
    HpaeFastRendererManagerDestroyStreamFuzzTest,
    HpaeFastRendererManagerCreateDestroyStreamFuzzTest,
    HpaeFastRendererManagerStartFuzzTest,
    HpaeFastRendererManagerPauseFuzzTest,
    HpaeFastRendererManagerFlushFuzzTest,
    HpaeFastRendererManagerDrainFuzzTest,
    HpaeFastRendererManagerStopFuzzTest,
    HpaeFastRendererManagerReleaseFuzzTest,
    HpaeFastRendererManagerStartPauseStreamFuzzTest,
    HpaeFastRendererManagerMoveStreamFuzzTest,
    HpaeFastRendererManagerMoveStreamSyncFuzzTest,
    HpaeFastRendererManagerMoveAllStreamFuzzTest,
    HpaeFastRendererManagerMoveAllStreamNotInitFuzzTest,
    HpaeFastRendererManagerAddNodeToSinkFuzzTest,
    HpaeFastRendererManagerAddNodeToSinkNullFuzzTest,
    HpaeFastRendererManagerAddAllNodesToSinkFuzzTest,
    HpaeFastRendererManagerAddAllNodesToSinkEmptyFuzzTest,
    HpaeFastRendererManagerSuspendStreamManagerFuzzTest,
    HpaeFastRendererManagerSetMuteFuzzTest,
    HpaeFastRendererManagerSetMuteForSwitchDeviceFuzzTest,
    HpaeFastRendererManagerSetMuteForSwitchDeviceNotInitFuzzTest,
    HpaeFastRendererManagerStopManagerFuzzTest,
    HpaeFastRendererManagerStopManagerNotInitFuzzTest,
    HpaeFastRendererManagerSetClientVolumeFuzzTest,
    HpaeFastRendererManagerSetRateFuzzTest,
    HpaeFastRendererManagerSetAudioEffectModeFuzzTest,
    HpaeFastRendererManagerGetAudioEffectModeFuzzTest,
    HpaeFastRendererManagerSetPrivacyTypeFuzzTest,
    HpaeFastRendererManagerGetPrivacyTypeFuzzTest,
    HpaeFastRendererManagerSetSpeedFuzzTest,
    HpaeFastRendererManagerSetLoudnessGainFuzzTest,
    HpaeFastRendererManagerGetWritableSizeFuzzTest,
    HpaeFastRendererManagerUpdateSpatializationStateFuzzTest,
    HpaeFastRendererManagerUpdateMaxLengthFuzzTest,
    HpaeFastRendererManagerRegisterWriteCallbackFuzzTest,
    HpaeFastRendererManagerRegisterWriteCallbackNullFuzzTest,
    HpaeFastRendererManagerRegisterReadCallbackFuzzTest,
    HpaeFastRendererManagerOnNodeStatusUpdateFuzzTest,
    HpaeFastRendererManagerOnFadeDoneFuzzTest,
    HpaeFastRendererManagerOnRequestLatencyFuzzTest,
    HpaeFastRendererManagerOnRewindAndFlushFuzzTest,
    HpaeFastRendererManagerOnNotifyQueueFuzzTest,
    HpaeFastRendererManagerGetAllSinkInputsInfoFuzzTest,
    HpaeFastRendererManagerGetSinkInputInfoFuzzTest,
    HpaeFastRendererManagerRefreshProcessClusterByDeviceFuzzTest,
    HpaeFastRendererManagerGetSinkInfoFuzzTest,
    HpaeFastRendererManagerGetSpanSizeInFrameFuzzTest,
    HpaeFastRendererManagerGetThreadNameFuzzTest,
    HpaeFastRendererManagerDumpSinkInfoFuzzTest,
    HpaeFastRendererManagerGetDeviceHDFDumpInfoFuzzTest,
    HpaeFastRendererManagerGetMaxAmplitudeFuzzTest,
    HpaeFastRendererManagerReloadRenderManagerFuzzTest,
    HpaeFastRendererManagerReloadRenderManagerNotInitFuzzTest,
    HpaeFastRendererManagerProcessFuzzTest,
    HpaeFastRendererManagerHandleMsgFuzzTest,
    HpaeFastRendererManagerDeactivateThreadFuzzTest,
    HpaeFastRendererManagerTriggerAppsUidUpdateFuzzTest,
    HpaeFastRendererManagerCreateStreamWithLoopbackFuzzTest,
    HpaeFastRendererManagerBoundarySessionIdFuzzTest,
    HpaeFastRendererManagerOperationsBeforeInitFuzzTest,
    HpaeFastRendererManagerFullLifecycleFuzzTest,
    HpaeFastRendererManagerSimpleOperationReturnsFuzzTest,
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
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
