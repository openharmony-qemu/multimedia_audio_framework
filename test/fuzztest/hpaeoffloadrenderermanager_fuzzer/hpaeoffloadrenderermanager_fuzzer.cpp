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

#include "hpaeoffloadrenderermanager_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <limits>

#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_offload_renderer_manager.h"
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
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t DEFAULT_NODE_ID = 1;

// Rewind and flush test constants
constexpr uint64_t DEFAULT_REWIND_TIME_MS = 1000;
constexpr uint64_t DEFAULT_HDI_FRAME_POSITION = 500;
constexpr uint64_t ZERO_HDI_FRAME_POSITION = 0;

// Speed test constants
constexpr float NORMAL_PLAYBACK_SPEED = 1.0f;
constexpr float FAST_PLAYBACK_SPEED = 1.5f;
constexpr float DOUBLE_PLAYBACK_SPEED = 2.0f;
const char* DEFAULT_TEST_DEVICE_CLASS = "offload";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
const char* REMOTE_OFFLOAD_DEVICE_CLASS = "remote_offload";

// Offload policy test constants
constexpr int32_t OFFLOAD_POLICY_DEFAULT = 0;

// Fuzz string length constants
constexpr size_t FUZZ_STRING_DEFAULT_LENGTH = 100;
constexpr size_t FUZZ_STRING_SHORT_LENGTH = 50;
constexpr size_t FUZZ_STRING_SPLIT_MODE_LENGTH = 20;
constexpr size_t FUZZ_STRING_VERY_LONG_DEVICE_NAME = 2000;
constexpr size_t FUZZ_STRING_VERY_LONG_FILE_PATH = 5000;
constexpr size_t FUZZ_STRING_VERY_LONG_ADAPTER_NAME = 1000;
constexpr size_t FUZZ_STRING_VERY_LONG_SPLIT_MODE = 3000;
constexpr size_t FUZZ_STRING_EXTRA_LONG_LENGTH = 5000;

// Loop and iteration constants
constexpr int32_t MAX_MSG_PROCESSING_WAIT_LOOPS = 10;
constexpr int32_t MAX_TEST_ITERATIONS = 10;
constexpr size_t MAX_STREAM_COUNT = 3;
constexpr size_t STREAM_COUNT_OFFSET = 1;
constexpr size_t MAX_MODULO_COUNT = 5;
constexpr size_t MODULO_COUNT_THREE = 3;
constexpr size_t HALF_DIVISOR = 2;

// Session ID offset constants
constexpr uint32_t SESSION_ID_OFFSET_1 = 1;
constexpr uint32_t SESSION_ID_OFFSET_2 = 2;
constexpr uint32_t SESSION_ID_OFFSET_3 = 3;
constexpr uint32_t SESSION_ID_LARGE_OFFSET = 1000;
constexpr uint32_t SESSION_ID_INVALID_OFFSET = 9999;
constexpr uint32_t FIXED_SESSION_ID = 99999;
constexpr uint32_t MIN_FRAME_LENGTH = 1;
constexpr uint32_t SMALL_FRAME_LENGTH = 2;
constexpr uint32_t ZERO_FRAME_LENGTH = 0;
constexpr size_t FUZZ_MAX_LEN_INCREMENT = 1;

// Thread sleep constants
constexpr int32_t CONCURRENT_THREAD_SLEEP_MS = 5;

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
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

std::string GetFuzzString(size_t maxLen = FUZZ_STRING_DEFAULT_LENGTH)
{
    if (RAW_DATA == nullptr || g_pos >= g_dataSize) {
        return "";
    }
    size_t len = GetData<size_t>() % (maxLen + FUZZ_MAX_LEN_INCREMENT);
    if (len > g_dataSize - g_pos) {
        len = g_dataSize - g_pos;
    }
    std::string result(reinterpret_cast<const char*>(RAW_DATA + g_pos), len);
    g_pos += len;
    return result;
}

static void InitHpaeSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.sinkId = GetData<uint32_t>();
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = REMOTE_OFFLOAD_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = GetFuzzString(FUZZ_STRING_DEFAULT_LENGTH);
    sinkInfo.deviceName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    sinkInfo.frameLen = FRAME_LENGTH_960;
    RoundVal(sinkInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(sinkInfo.format, SUPPORTED_FORMATS);
    RoundVal(sinkInfo.channels, SUPPORTED_CHANNELS);
    sinkInfo.suspendTime = GetData<uint32_t>();
    sinkInfo.channelLayout = GetData<uint64_t>();
    sinkInfo.deviceType = GetData<int32_t>();
    sinkInfo.volume = GetData<float>();
    sinkInfo.openMicSpeaker = GetData<uint32_t>();
    sinkInfo.renderInIdleState = GetData<uint32_t>();
    sinkInfo.sourceType = GetData<uint32_t>();
    sinkInfo.offloadEnable = GetData<uint32_t>();
    sinkInfo.fixedLatency = GetData<uint32_t>();
    sinkInfo.sinkLatency = GetData<uint32_t>();
    sinkInfo.splitMode = GetFuzzString(FUZZ_STRING_SPLIT_MODE_LENGTH);
    sinkInfo.needEmptyChunk = GetData<bool>();
    sinkInfo.auxSinkEnable = GetData<bool>();
}

static void InitRenderStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.nodeType = GetData<HpaeNodeType>();
    streamInfo.streamType = GetData<AudioStreamType>();
    streamInfo.fadeType = GetData<FadeType>();
    streamInfo.pipeType = GetData<AudioPipeType>();
    RoundVal(streamInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    streamInfo.customSampleRate = GetData<uint32_t>();
    RoundVal(streamInfo.format, SUPPORTED_FORMATS);
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    streamInfo.channelLayout = GetData<uint64_t>();
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.uid = GetData<int32_t>();
    streamInfo.pid = GetData<int32_t>();
    streamInfo.tokenId = GetData<uint32_t>();
    streamInfo.deviceName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    streamInfo.isMoveAble = GetData<bool>();
    streamInfo.privacyType = GetData<AudioPrivacyType>();
}

static void InitNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeOffloadRendererManager> &hpaeRendererManager)
{
    if (!hpaeRendererManager->IsInit()) {
        return;
    }
    for (int i = 0; i < MAX_MSG_PROCESSING_WAIT_LOOPS && hpaeRendererManager->IsMsgProcessing(); i++) {
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

void HpaeOffloadRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeOffloadRendererManager offloadRendererManager(sinkInfo);
}

void HpaeOffloadRendererManagerInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    bool isReload = GetData<bool>();
    offloadRendererManager->Init(isReload);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->IsInit();
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);
    bool isMoveDefault = GetData<bool>();
    offloadRendererManager->DeInit(isMoveDefault);
}

void HpaeOffloadRendererManagerCreateStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerStartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerPauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Pause(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Flush(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Drain(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerStopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Stop(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Release(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    std::string sinkName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    offloadRendererManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    std::string sinkName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    std::vector<uint32_t> sessionIds;
    size_t count = GetData<size_t>() % MAX_MODULO_COUNT;
    for (size_t i = 0; i < count; ++i) {
        sessionIds.push_back(GetData<uint32_t>());
    }

    MoveSessionType moveType = GetData<MoveSessionType>();
    if (moveType >= MOVE_SINGLE && moveType <= MOVE_DEFAULT) {
        offloadRendererManager->MoveAllStream(sinkName, sessionIds, moveType);
    }
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    bool isSuspend = GetData<bool>();
    offloadRendererManager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerStopManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->StopManager();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    bool isMute = GetData<bool>();
    offloadRendererManager->SetMute(isMute);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    offloadRendererManager->SetClientVolume(sessionId, volume);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    offloadRendererManager->SetRate(sessionId, rate);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    offloadRendererManager->SetAudioEffectMode(sessionId, effectMode);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    offloadRendererManager->GetAudioEffectMode(sessionId, effectMode);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    offloadRendererManager->SetPrivacyType(sessionId, privacyType);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    offloadRendererManager->GetPrivacyType(sessionId, privacyType);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerRegisterWriteCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    offloadRendererManager->RegisterWriteCallback(sessionId, writeIncDataCb);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>("/data/test_fuzz.pcm");
    offloadRendererManager->RegisterReadCallback(sessionId, readDataCb);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetOffloadPolicyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    offloadRendererManager->SetOffloadPolicy(sessionId, state);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->GetWritableSize(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    offloadRendererManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerUpdateMaxLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    offloadRendererManager->UpdateMaxLength(sessionId, maxLength);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    int32_t type = GetData<int32_t>();
    offloadRendererManager->SetOffloadRenderCallbackType(sessionId, type);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    offloadRendererManager->SetSpeed(sessionId, speed);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->GetAllSinkInputsInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetSinkInputInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    HpaeSinkInputInfo sinkInputInfo;
    offloadRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerRefreshProcessClusterByDeviceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->RefreshProcessClusterByDevice();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->GetSinkInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    offloadRendererManager->SetLoudnessGain(sessionId, loudnessGain);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetNodeInputFormatInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    AudioBasicFormat basicFormat;
    offloadRendererManager->GetNodeInputFormatInfo(sessionId, basicFormat);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->Process();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerHandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->HandleMsg();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeactivateThread();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerAddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    offloadRendererManager->AddNodeToSink(node);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerAddAllNodesToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(node);
    bool isConnect = GetData<bool>();
    offloadRendererManager->AddAllNodesToSink(sinkInputs, isConnect);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = static_cast<IOperation>(GetData<int32_t>());
    offloadRendererManager->OnNodeStatusUpdate(sessionId, operation);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnRequestLatencyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = GetData<uint64_t>();
    offloadRendererManager->OnRequestLatency(sessionId, latency);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnRewindAndFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint64_t rewindTime = GetData<uint64_t>();
    uint64_t hdiFramePosition = GetData<uint64_t>();
    offloadRendererManager->OnRewindAndFlush(rewindTime, hdiFramePosition);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnNotifyQueueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->OnNotifyQueue();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetThreadNameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->GetThreadName();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DumpSinkInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerReloadRenderManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    newSinkInfo.samplingRate = SAMPLE_RATE_16000;
    bool isReload = GetData<bool>();
    offloadRendererManager->ReloadRenderManager(newSinkInfo, isReload);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetOffloadCallbackDataFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->GetOffloadCallbackData();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerTriggerAppsUidUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->TriggerAppsUidUpdate(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnNotifyHdiDataFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    std::pair<uint64_t, TimePoint> hdiPos;
    hdiPos.first = GetData<uint64_t>();
    hdiPos.second = TimePoint::clock::now();
    offloadRendererManager->OnNotifyHdiData(hdiPos);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerFullLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }

    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;

    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    offloadRendererManager->RegisterWriteCallback(sessionId, writeIncDataCb);

    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->SetSpeed(sessionId, NORMAL_PLAYBACK_SPEED);
    offloadRendererManager->SetOffloadPolicy(sessionId, OFFLOAD_POLICY_DEFAULT);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->Pause(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->Flush(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->Stop(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerBoundaryValueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }

    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test boundary values for sessionId
    uint32_t boundarySessions[] = {0, UINT32_MAX, 1, UINT32_MAX - 1};
    for (uint32_t sessionId : boundarySessions) {
        offloadRendererManager->Start(sessionId);
        offloadRendererManager->Pause(sessionId);
        offloadRendererManager->Stop(sessionId);
        offloadRendererManager->Release(sessionId);
    }

    // Test extreme float values
    float extremeFloats[] = {-1000.0f, -1.0f, 0.0f, 0.5f, 1.0f, 2.0f, 1000.0f};
    for (float val : extremeFloats) {
        offloadRendererManager->SetClientVolume(1, val);
        offloadRendererManager->SetSpeed(1, val);
        offloadRendererManager->SetLoudnessGain(1, val);
    }

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerNullAndInvalidStateFuzzTest()
{
    // Test manager without init
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }

    // Test operations before Init
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Start(sessionId);
    offloadRendererManager->Pause(sessionId);
    offloadRendererManager->Stop(sessionId);
    offloadRendererManager->Release(sessionId);
    offloadRendererManager->SetMute(GetData<bool>());
    offloadRendererManager->GetSinkInfo();
    offloadRendererManager->IsInit();
    offloadRendererManager->IsRunning();
    offloadRendererManager->IsMsgProcessing();

    // Now init and test
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test with invalid sessionId
    uint32_t invalidSessionId = UINT32_MAX;
    offloadRendererManager->Start(invalidSessionId);
    offloadRendererManager->Pause(invalidSessionId);
    offloadRendererManager->Stop(invalidSessionId);
    offloadRendererManager->Release(invalidSessionId);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerMultipleStreamsFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }

    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create multiple streams
    std::vector<uint32_t> sessionIds;
    size_t streamCount = GetData<size_t>() % MODULO_COUNT_THREE + STREAM_COUNT_OFFSET;

    for (size_t i = 0; i < streamCount; ++i) {
        HpaeStreamInfo streamInfo;
        InitRenderStreamInfo(streamInfo);
        streamInfo.sessionId = TEST_STREAM_SESSION_ID + i;
        offloadRendererManager->CreateStream(streamInfo);
        sessionIds.push_back(streamInfo.sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    // Test operations on all streams
    for (uint32_t sessionId : sessionIds) {
        offloadRendererManager->Start(sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    for (uint32_t sessionId : sessionIds) {
        offloadRendererManager->Pause(sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    for (uint32_t sessionId : sessionIds) {
        offloadRendererManager->Stop(sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    for (uint32_t sessionId : sessionIds) {
        offloadRendererManager->Release(sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Critical Issues Tests - Null Pointer Dereference
// ============================================================================

void HpaeOffloadRendererManagerNullSharedPtrFuzzTest()
{
    // Test with null shared_ptr arguments
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test RegisterWriteCallback with null callback
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<WriteFixedDataCb> nullCallback = nullptr;
    offloadRendererManager->RegisterWriteCallback(sessionId, nullCallback);

    // Test RegisterReadCallback with null callback
    std::shared_ptr<ReadDataCb> nullReadCallback = nullptr;
    offloadRendererManager->RegisterReadCallback(sessionId, nullReadCallback);

    // Test AddNodeToSink with null node
    std::shared_ptr<HpaeSinkInputNode> nullNode = nullptr;
    offloadRendererManager->AddNodeToSink(nullNode);

    // Test AddAllNodesToSink with empty vector
    std::vector<std::shared_ptr<HpaeSinkInputNode>> emptyNodes;
    offloadRendererManager->AddAllNodesToSink(emptyNodes, GetData<bool>());

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Critical Issues Tests - Integer Overflow
// ============================================================================

void HpaeOffloadRendererManagerIntegerOverflowFuzzTest()
{
    // Test historyFrameCount calculation with extreme values
    // historyFrameCount = HISTORY_INTERVAL_S * customSampleRate / frameLen
    // With customSampleRate=UINT32_MAX and frameLen=1, this can cause overflow

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create stream with extreme customSampleRate value
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.customSampleRate = GetData<uint32_t>();  // May be UINT32_MAX
    streamInfo.frameLen = GetData<uint32_t>();
    if (streamInfo.frameLen == 0) {
        streamInfo.frameLen = MIN_FRAME_LENGTH;  // Avoid division by zero
    }

    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    // Test with UINT32_MAX for various operations
    uint32_t maxSessionId = UINT32_MAX;
    offloadRendererManager->Start(maxSessionId);
    offloadRendererManager->Pause(maxSessionId);
    offloadRendererManager->Stop(maxSessionId);
    offloadRendererManager->Release(maxSessionId);

    // Test extreme maxLength value
    offloadRendererManager->UpdateMaxLength(streamInfo.sessionId, UINT32_MAX);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Critical Issues Tests - Concurrent Operation
// ============================================================================

void HpaeOffloadRendererManagerConcurrentOperationFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = streamInfo.sessionId;

    // Start the stream
    offloadRendererManager->Start(sessionId);

    // Simulate concurrent DeInit while stream operations are in progress
    // by calling DeInit in quick succession
    std::thread concurrentThread([offloadRendererManager, sessionId]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(CONCURRENT_THREAD_SLEEP_MS));
        // Try operations while DeInit is happening
        offloadRendererManager->Pause(sessionId);
        offloadRendererManager->SetSpeed(sessionId, GetData<float>());
    });

    // Call DeInit immediately
    offloadRendererManager->DeInit();

    if (concurrentThread.joinable()) {
        concurrentThread.join();
    }
}

// ============================================================================
// Critical Issues Tests - Unsafe String Handling
// ============================================================================

void HpaeOffloadRendererManagerUnsafeStringFuzzTest()
{
    // Test with extremely long strings (>1000 chars)
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);

    // Set extremely long strings
    sinkInfo.deviceName = GetFuzzString(FUZZ_STRING_VERY_LONG_DEVICE_NAME);  // Very long device name
    sinkInfo.filePath = GetFuzzString(FUZZ_STRING_VERY_LONG_FILE_PATH);    // Very long file path
    sinkInfo.adapterName = GetFuzzString(FUZZ_STRING_VERY_LONG_ADAPTER_NAME); // Very long adapter name
    sinkInfo.splitMode = GetFuzzString(FUZZ_STRING_VERY_LONG_SPLIT_MODE);   // Very long split mode

    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test MoveStream with very long sink name
    uint32_t sessionId = GetData<uint32_t>();
    std::string longSinkName = GetFuzzString(FUZZ_STRING_VERY_LONG_FILE_PATH);
    offloadRendererManager->MoveStream(sessionId, longSinkName);

    // Test MoveAllStream with very long sink name
    std::vector<uint32_t> sessionIds;
    offloadRendererManager->MoveAllStream(longSinkName, sessionIds, MOVE_SINGLE);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Major Concerns Tests - Incomplete State Transitions
// ============================================================================

void HpaeOffloadRendererManagerInvalidStateTransitionsFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create and prepare a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = streamInfo.sessionId;

    // Test Start() on already running stream
    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->Start(sessionId);  // Start again while running
    WaitForMsgProcessing(offloadRendererManager);

    // Test Pause() on stopped stream
    offloadRendererManager->Stop(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->Pause(sessionId);  // Pause on stopped
    WaitForMsgProcessing(offloadRendererManager);

    // Test Flush() on prepared stream
    offloadRendererManager->Flush(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Test Drain() on paused stream
    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->Pause(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->Drain(sessionId);  // Drain on paused
    WaitForMsgProcessing(offloadRendererManager);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Major Concerns Tests - DestroyStream Twice
// ============================================================================

void HpaeOffloadRendererManagerDestroyStreamTwiceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = streamInfo.sessionId;

    // Destroy stream once
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy stream again - should handle gracefully
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to operate on destroyed stream
    offloadRendererManager->Start(sessionId);
    offloadRendererManager->Pause(sessionId);
    offloadRendererManager->Stop(sessionId);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Major Concerns Tests - Memory Leak Scenarios
// ============================================================================

void HpaeOffloadRendererManagerMemoryLeakFuzzTest()
{
    // Test streams created but never started/released
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create multiple streams without proper cleanup
    size_t leakCount = GetData<size_t>() % MAX_MODULO_COUNT + STREAM_COUNT_OFFSET;
    std::vector<uint32_t> leakedSessionIds;

    for (size_t i = 0; i < leakCount; ++i) {
        HpaeStreamInfo streamInfo;
        InitRenderStreamInfo(streamInfo);
        streamInfo.sessionId = TEST_STREAM_SESSION_ID + i;
        offloadRendererManager->CreateStream(streamInfo);
        leakedSessionIds.push_back(streamInfo.sessionId);
    }
    WaitForMsgProcessing(offloadRendererManager);

    // Some streams started but never stopped
    for (size_t i = 0; i < leakedSessionIds.size() / HALF_DIVISOR; ++i) {
        offloadRendererManager->Start(leakedSessionIds[i]);
    }
    WaitForMsgProcessing(offloadRendererManager);

    // DeInit without proper stream release - should clean up internally
    offloadRendererManager->DeInit();
}

// ============================================================================
// Major Concerns Tests - SessionId Collision
// ============================================================================

void HpaeOffloadRendererManagerSessionIdCollisionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create stream with specific sessionId
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.sessionId = FIXED_SESSION_ID;  // Fixed session ID
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to create another stream with same sessionId
    HpaeStreamInfo streamInfo2;
    InitRenderStreamInfo(streamInfo2);
    streamInfo2.sessionId = FIXED_SESSION_ID;  // Same session ID
    offloadRendererManager->CreateStream(streamInfo2);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to operate on the collided sessionId
    offloadRendererManager->Start(FIXED_SESSION_ID);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->Pause(FIXED_SESSION_ID);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy the stream
    offloadRendererManager->DestroyStream(FIXED_SESSION_ID);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to destroy again
    offloadRendererManager->DestroyStream(FIXED_SESSION_ID);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Major Concerns Tests - Offload-Specific Edge Cases
// ============================================================================

void HpaeOffloadRendererManagerOffloadEdgeCasesFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    uint32_t sessionId = streamInfo.sessionId;

    // Test SetOffloadPolicy with invalid enum values
    int32_t invalidPolicyValues[] = {
        -100, -10, -1, 0, 1, 2, 3, 100, INT32_MIN, INT32_MAX
    };

    for (int32_t policy : invalidPolicyValues) {
        offloadRendererManager->SetOffloadPolicy(sessionId, policy);
        WaitForMsgProcessing(offloadRendererManager);
    }

    // Test SetOffloadRenderCallbackType with invalid values
    int32_t invalidCallbackTypes[] = {
        -50, -5, -1, 0, 1, 10, 100, INT32_MIN, INT32_MAX
    };

    for (int32_t callbackType : invalidCallbackTypes) {
        offloadRendererManager->SetOffloadRenderCallbackType(sessionId, callbackType);
        WaitForMsgProcessing(offloadRendererManager);
    }

    // Test SetSpeed with extreme values
    float extremeSpeeds[] = {
        -1000.0f, -10.0f, -1.0f, 0.0f, 0.001f, 0.5f, 1.0f, 2.0f, 10.0f, 1000.0f,
        -std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
    };

    for (float speed : extremeSpeeds) {
        offloadRendererManager->SetSpeed(sessionId, speed);
        WaitForMsgProcessing(offloadRendererManager);
    }

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Minor Issues Tests - Use-After-Free
// ============================================================================

// Global weak callback reference for testing use-after-free
static std::weak_ptr<WriteFixedDataCb> g_weakCallback;

void HpaeOffloadRendererManagerUseAfterFreeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Create and register callback
    auto callback = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    g_weakCallback = callback;  // Keep weak reference
    offloadRendererManager->RegisterWriteCallback(sessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy the stream (should invalidate callback)
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Reset shared_ptr to simulate object destruction
    callback.reset();

    // Try to invoke callback through weak reference (use-after-free scenario)
    // This tests if the manager properly handles stale callbacks
    if (auto cb = g_weakCallback.lock()) {
        // Should not reach here if callback was properly cleaned up
        AudioCallBackStreamInfo info = {};
        cb->OnStreamData(info);
    }

    // Try to register the destroyed callback again
    offloadRendererManager->RegisterWriteCallback(sessionId, callback);

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Minor Issues Tests - Callback After Stream Destruction
// ============================================================================

void HpaeOffloadRendererManagerCallbackAfterDestructionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create and start a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;

    // Register callback
    auto callback = std::make_shared<TestCallbackWithDestruction>();
    offloadRendererManager->RegisterWriteCallback(sessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    // Start the stream to trigger potential callbacks
    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy the stream while callback is registered
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to trigger callback operations after stream destruction
    // This tests if the manager properly prevents callback invocation
    AudioCallBackStreamInfo dummyInfo = {};
    callback->OnStreamData(dummyInfo);

    // Re-create stream with same sessionId and register callback
    streamInfo.sessionId = sessionId;
    offloadRendererManager->CreateStream(streamInfo);
    offloadRendererManager->RegisterWriteCallback(sessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy again
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Additional Edge Cases - Rapid State Changes
// ============================================================================

void HpaeOffloadRendererManagerRapidStateChangesFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Rapid state changes without proper waits
    for (int i = 0; i < MAX_TEST_ITERATIONS; ++i) {
        offloadRendererManager->Start(sessionId);
        offloadRendererManager->Pause(sessionId);
        offloadRendererManager->Start(sessionId);
        offloadRendererManager->Stop(sessionId);
        offloadRendererManager->Start(sessionId);
    }

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Additional Edge Cases - Empty Sink Name
// ============================================================================

void HpaeOffloadRendererManagerEmptySinkNameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create a stream
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Test MoveStream with empty sink name
    offloadRendererManager->MoveStream(sessionId, "");
    WaitForMsgProcessing(offloadRendererManager);

    // Test MoveAllStream with empty sink name
    std::vector<uint32_t> sessionIds = {sessionId};
    offloadRendererManager->MoveAllStream("", sessionIds, MOVE_ALL);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Additional Edge Cases - Zero Frame Length
// ============================================================================

void HpaeOffloadRendererManagerZeroFrameLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    sinkInfo.frameLen = 0;  // Zero frame length - can cause division by zero

    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create stream with zero frame length
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.frameLen = 0;  // Zero frame length

    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to start the stream
    offloadRendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Round 2 Critical Fix Tests - Unprotected curNode_ Access After Destruction
// ============================================================================

void HpaeOffloadRendererManagerCurNodeUseAfterFreeFuzzTest()
{
    // CRITICAL: Test OnNodeStatusUpdate(), OnRewindAndFlush(), OnNotifyHdiData()
    // after DeleteInputSession() which sets curNode_ to nullptr.
    // Source lines 860-901 show these methods access curNode_ without null check.

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create and start a stream to make it the curNode_
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy the stream - this calls DeleteInputSession() which sets curNode_ = nullptr
    // Source line 106: curNode_ = nullptr; in RemoveNodeFromMap()
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Now test callbacks that access curNode_ WITHOUT null check:
    // Line 862: curNode_->GetState() - NO NULL CHECK
    offloadRendererManager->OnNodeStatusUpdate(sessionId, OPERATION_PAUSED);

    // Line 893: curNode_->RewindHistoryBuffer() - has CHECK_AND_RETURN_LOG but not safe
    offloadRendererManager->OnRewindAndFlush(GetData<uint64_t>(), GetData<uint64_t>());

    // Line 900: curNode_->NotifyOffloadHdiPos() - has CHECK_AND_RETURN_LOG
    offloadRendererManager->OnNotifyHdiData({GetData<uint64_t>(), TimePoint::clock::now()});

    // Also test with invalid sessionId
    offloadRendererManager->OnNodeStatusUpdate(UINT32_MAX, OPERATION_STOPPED);
    offloadRendererManager->OnRewindAndFlush(UINT64_MAX, UINT64_MAX);
    offloadRendererManager->OnNotifyHdiData({UINT64_MAX, TimePoint::clock::now()});

    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

// ============================================================================
// Round 2 Critical Fix Tests - SetSpeed With Null sinkOutputNode
// ============================================================================

void HpaeOffloadRendererManagerSetSpeedNullSinkOutputFuzzTest()
{
    // CRITICAL: Test SetSpeed when sinkOutputNode_ is nullptr.
    // Source lines 806-808: The null check for sinkOutputNode_ comes AFTER
    // the node->SetSpeed() call, but should check first.

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }

    // Test SetSpeed BEFORE Init (sinkOutputNode_ is nullptr)
    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    offloadRendererManager->SetSpeed(sessionId, speed);

    // Now Init and test
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create stream but don't start it - curNode_ exists but sinkOutputNode_ may not be fully initialized
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Test SetSpeed on a session that isn't curNode_
    // This should still call node->SetSpeed() but not sinkOutputNode_->SetSpeed()
    uint32_t invalidSessionId = sessionId + SESSION_ID_INVALID_OFFSET;  // Different from curNode_
    offloadRendererManager->SetSpeed(invalidSessionId, DOUBLE_PLAYBACK_SPEED);

    WaitForMsgProcessing(offloadRendererManager);

    // Now test with DeInit which destroys sinkOutputNode_
    offloadRendererManager->DeInit();

    // sinkOutputNode_ should be nullptr now, try SetSpeed again
    // This tests the null check at line 806
    offloadRendererManager->SetSpeed(sessionId, FAST_PLAYBACK_SPEED);
}

// ============================================================================
// Round 2 Critical Fix Tests - RegisterWriteCallback With Invalid SessionId
// ============================================================================

void HpaeOffloadRendererManagerRegisterCallbackInvalidSessionFuzzTest()
{
    // CRITICAL: Test RegisterWriteCallback with invalid sessionId before CreateStream.
    // Source lines 698-709: The method checks for nullptr node but the SafeGetMap()
    // returns nullptr for invalid sessionIds, testing this edge case.

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test with extreme boundary values for sessionId
    uint32_t boundarySessionIds[] = {0, 1, UINT32_MAX, UINT32_MAX - 1};

    for (uint32_t testSessionId : boundarySessionIds) {
        // Create callback with fuzzed format
        AudioSampleFormat format = GetData<AudioSampleFormat>();
        auto callback = std::make_shared<WriteFixedDataCb>(format);

        // Register callback BEFORE creating stream - tests null node handling
        offloadRendererManager->RegisterWriteCallback(testSessionId, callback);
        WaitForMsgProcessing(offloadRendererManager);

        // Also test with null callback (empty weak_ptr)
        offloadRendererManager->RegisterWriteCallback(testSessionId, std::weak_ptr<IStreamCallback>());
    }

    // Now create a valid stream and try invalid sessionIds again
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
    uint32_t validSessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Test registering callback with wrong sessionId (collision test)
    uint32_t wrongSessionId = validSessionId + SESSION_ID_LARGE_OFFSET;
    auto callback = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    offloadRendererManager->RegisterWriteCallback(wrongSessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    // Test valid callback registration for comparison
    offloadRendererManager->RegisterWriteCallback(validSessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Round 2 Critical Fix Tests - Integer Overflow in historyFrameCount
// ============================================================================

void HpaeOffloadRendererManagerHistoryFrameCountOverflowFuzzTest()
{
    // CRITICAL: Test integer overflow in historyFrameCount calculation.
    // Source lines 57-59: historyFrameCount = HISTORY_INTERVAL_S * customSampleRate / frameLen
    // With customSampleRate=UINT32_MAX and frameLen=1: 7 * UINT32_MAX / 1 = overflow!
    // Also at lines 134-135 in AddSingleNodeToSink

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Test 1: Maximum overflow scenario - customSampleRate = UINT32_MAX, frameLen = 1
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.customSampleRate = UINT32_MAX;  // Maximum sample rate
    streamInfo.frameLen = MIN_FRAME_LENGTH;  // Minimum frame length - causes division by 1 (maximizes overflow)

    offloadRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(offloadRendererManager);

    // Test 2: Near overflow with realistic values
    HpaeStreamInfo streamInfo2;
    InitRenderStreamInfo(streamInfo2);
    streamInfo2.sessionId = TEST_STREAM_SESSION_ID + SESSION_ID_OFFSET_1;
    streamInfo2.customSampleRate = UINT32_MAX / HALF_DIVISOR;  // Half of max
    streamInfo2.frameLen = SMALL_FRAME_LENGTH;  // Small frame len

    offloadRendererManager->CreateStream(streamInfo2);
    WaitForMsgProcessing(offloadRendererManager);

    // Test 3: Zero frameLen (division by zero protection)
    HpaeStreamInfo streamInfo3;
    InitRenderStreamInfo(streamInfo3);
    streamInfo3.sessionId = TEST_STREAM_SESSION_ID + SESSION_ID_OFFSET_2;
    streamInfo3.customSampleRate = SAMPLE_RATE_48000;
    streamInfo3.frameLen = 0;  // Division by zero - should be handled

    offloadRendererManager->CreateStream(streamInfo3);
    WaitForMsgProcessing(offloadRendererManager);

    // Test 4: Large frameLen that could cause wrap-around
    HpaeStreamInfo streamInfo4;
    InitRenderStreamInfo(streamInfo4);
    streamInfo4.sessionId = TEST_STREAM_SESSION_ID + SESSION_ID_OFFSET_3;
    streamInfo4.customSampleRate = UINT32_MAX;
    streamInfo4.frameLen = UINT32_MAX;  // Large divisor

    offloadRendererManager->CreateStream(streamInfo4);
    WaitForMsgProcessing(offloadRendererManager);

    // Try to start streams (may trigger overflow-related bugs)
    offloadRendererManager->Start(streamInfo.sessionId);
    offloadRendererManager->Start(streamInfo2.sessionId);
    offloadRendererManager->Start(streamInfo3.sessionId);
    offloadRendererManager->Start(streamInfo4.sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Test rewind operations that use historyFrameCount
    // RewindAndFlush internally uses the history buffer sized by historyFrameCount
    offloadRendererManager->OnRewindAndFlush(UINT64_MAX, UINT64_MAX);
    offloadRendererManager->Flush(streamInfo.sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    offloadRendererManager->DeInit();
}

// ============================================================================
// Round 2 Critical Fix Tests - Combined Edge Cases
// ============================================================================

void HpaeOffloadRendererManagerCombinedCriticalEdgeCasesFuzzTest()
{
    // Combine multiple critical edge cases in one test

    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    if (offloadRendererManager == nullptr) {
        return;
    }
    offloadRendererManager->Init();
    WaitForMsgProcessing(offloadRendererManager);

    // Create stream with overflow-prone parameters
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.customSampleRate = GetData<uint32_t>();
    streamInfo.frameLen = GetData<uint32_t>();
    if (streamInfo.frameLen == 0) {
        streamInfo.frameLen = MIN_FRAME_LENGTH;
    }

    offloadRendererManager->CreateStream(streamInfo);
    uint32_t sessionId = streamInfo.sessionId;
    WaitForMsgProcessing(offloadRendererManager);

    // Register callback with invalid session first
    auto callback = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    offloadRendererManager->RegisterWriteCallback(UINT32_MAX, callback);

    // Now register valid callback
    offloadRendererManager->RegisterWriteCallback(sessionId, callback);
    WaitForMsgProcessing(offloadRendererManager);

    // Start the stream
    offloadRendererManager->Start(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Test SetSpeed with fuzzed values
    offloadRendererManager->SetSpeed(sessionId, GetData<float>());
    WaitForMsgProcessing(offloadRendererManager);

    // Destroy stream (sets curNode_ to nullptr)
    offloadRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(offloadRendererManager);

    // Try callbacks after destruction (tests curNode_ null access)
    offloadRendererManager->OnNodeStatusUpdate(sessionId, OPERATION_DRAINED);
    offloadRendererManager->OnRewindAndFlush(DEFAULT_REWIND_TIME_MS, DEFAULT_HDI_FRAME_POSITION);
    offloadRendererManager->OnNotifyHdiData({ZERO_HDI_FRAME_POSITION, TimePoint::clock::now()});

    // Try SetSpeed after destruction
    offloadRendererManager->SetSpeed(sessionId, NORMAL_PLAYBACK_SPEED);

    offloadRendererManager->DeInit();
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    HpaeOffloadRendererManagerConstructFuzzTest,
    HpaeOffloadRendererManagerInitFuzzTest,
    HpaeOffloadRendererManagerDeInitFuzzTest,
    HpaeOffloadRendererManagerCreateStreamFuzzTest,
    HpaeOffloadRendererManagerDestroyStreamFuzzTest,
    HpaeOffloadRendererManagerStartFuzzTest,
    HpaeOffloadRendererManagerPauseFuzzTest,
    HpaeOffloadRendererManagerFlushFuzzTest,
    HpaeOffloadRendererManagerDrainFuzzTest,
    HpaeOffloadRendererManagerStopFuzzTest,
    HpaeOffloadRendererManagerReleaseFuzzTest,
    HpaeOffloadRendererManagerMoveStreamFuzzTest,
    HpaeOffloadRendererManagerMoveAllStreamFuzzTest,
    HpaeOffloadRendererManagerSuspendStreamManagerFuzzTest,
    HpaeOffloadRendererManagerStopManagerFuzzTest,
    HpaeOffloadRendererManagerSetMuteFuzzTest,
    HpaeOffloadRendererManagerSetClientVolumeFuzzTest,
    HpaeOffloadRendererManagerSetRateFuzzTest,
    HpaeOffloadRendererManagerSetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerGetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerSetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerGetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerRegisterWriteCallbackFuzzTest,
    HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest,
    HpaeOffloadRendererManagerSetOffloadPolicyFuzzTest,
    HpaeOffloadRendererManagerGetWritableSizeFuzzTest,
    HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest,
    HpaeOffloadRendererManagerUpdateMaxLengthFuzzTest,
    HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest,
    HpaeOffloadRendererManagerSetSpeedFuzzTest,
    HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest,
    HpaeOffloadRendererManagerGetSinkInputInfoFuzzTest,
    HpaeOffloadRendererManagerRefreshProcessClusterByDeviceFuzzTest,
    HpaeOffloadRendererManagerGetSinkInfoFuzzTest,
    HpaeOffloadRendererManagerSetLoudnessGainFuzzTest,
    HpaeOffloadRendererManagerGetNodeInputFormatInfoFuzzTest,
    HpaeOffloadRendererManagerProcessFuzzTest,
    HpaeOffloadRendererManagerHandleMsgFuzzTest,
    HpaeOffloadRendererManagerDeactivateThreadFuzzTest,
    HpaeOffloadRendererManagerAddNodeToSinkFuzzTest,
    HpaeOffloadRendererManagerAddAllNodesToSinkFuzzTest,
    HpaeOffloadRendererManagerOnNodeStatusUpdateFuzzTest,
    HpaeOffloadRendererManagerOnRequestLatencyFuzzTest,
    HpaeOffloadRendererManagerOnRewindAndFlushFuzzTest,
    HpaeOffloadRendererManagerOnNotifyQueueFuzzTest,
    HpaeOffloadRendererManagerGetThreadNameFuzzTest,
    HpaeOffloadRendererManagerDumpSinkInfoFuzzTest,
    HpaeOffloadRendererManagerReloadRenderManagerFuzzTest,
    HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest,
    HpaeOffloadRendererManagerGetOffloadCallbackDataFuzzTest,
    HpaeOffloadRendererManagerTriggerAppsUidUpdateFuzzTest,
    HpaeOffloadRendererManagerOnNotifyHdiDataFuzzTest,
    HpaeOffloadRendererManagerFullLifecycleFuzzTest,
    HpaeOffloadRendererManagerBoundaryValueFuzzTest,
    HpaeOffloadRendererManagerNullAndInvalidStateFuzzTest,
    HpaeOffloadRendererManagerMultipleStreamsFuzzTest,
    HpaeOffloadRendererManagerNullSharedPtrFuzzTest,
    HpaeOffloadRendererManagerIntegerOverflowFuzzTest,
    HpaeOffloadRendererManagerConcurrentOperationFuzzTest,
    HpaeOffloadRendererManagerUnsafeStringFuzzTest,
    HpaeOffloadRendererManagerInvalidStateTransitionsFuzzTest,
    HpaeOffloadRendererManagerDestroyStreamTwiceFuzzTest,
    HpaeOffloadRendererManagerMemoryLeakFuzzTest,
    HpaeOffloadRendererManagerSessionIdCollisionFuzzTest,
    HpaeOffloadRendererManagerOffloadEdgeCasesFuzzTest,
    HpaeOffloadRendererManagerUseAfterFreeFuzzTest,
    HpaeOffloadRendererManagerCallbackAfterDestructionFuzzTest,
    HpaeOffloadRendererManagerRapidStateChangesFuzzTest,
    HpaeOffloadRendererManagerEmptySinkNameFuzzTest,
    HpaeOffloadRendererManagerZeroFrameLengthFuzzTest,
    // Round 2 Critical Fix Tests
    HpaeOffloadRendererManagerCurNodeUseAfterFreeFuzzTest,
    HpaeOffloadRendererManagerSetSpeedNullSinkOutputFuzzTest,
    HpaeOffloadRendererManagerRegisterCallbackInvalidSessionFuzzTest,
    HpaeOffloadRendererManagerHistoryFrameCountOverflowFuzzTest,
    HpaeOffloadRendererManagerCombinedCriticalEdgeCasesFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
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

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    using namespace OHOS::AudioStandard;
    if (size < FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    FuzzTest(data, size);
    return 0;
}
