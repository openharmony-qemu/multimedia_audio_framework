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

#include "hpaerenderermanager_add_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_ec_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"
#include "hpae_define.h"
#include "hpae_renderer_manager.h"
#include "hpae_sink_input_node.h"
#include "i_hpae_renderer_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const char* DEFAULT_TEST_DEVICE_CLASS = "file_io";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
constexpr size_t THRESHOLD = 10;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t DEFAULT_NODE_ID = 1;
const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8,
    CHANNEL_9,
    CHANNEL_10,
    CHANNEL_11,
    CHANNEL_12,
    CHANNEL_13,
    CHANNEL_14,
    CHANNEL_15,
    CHANNEL_16,
};

vector<IOperation> IOperationVec = {
    OPERATION_INVALID,
    OPERATION_STARTED,
    OPERATION_PAUSED,
    OPERATION_STOPPED,
    OPERATION_FLUSHED,
    OPERATION_DRAINED,
    OPERATION_RELEASED,
    OPERATION_UNDERRUN,
    OPERATION_UNDERFLOW,
    OPERATION_SET_OFFLOAD_ENABLE,
    OPERATION_UNSET_OFFLOAD_ENABLE,
    OPERATION_DATA_LINK_CONNECTING,
    OPERATION_DATA_LINK_CONNECTED,
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
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
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
        roundVal = list[GetData<uint32_t>()%list.size()];
    }
}

void RoundSinkInfo(HpaeSinkInfo &sinkInfo)
{
    RoundVal(sinkInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(sinkInfo.format, AUDIO_SUPPORTED_FORMATS);
}

void RoundStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, AUDIO_SUPPORTED_FORMATS);
}

static void InitHpaeSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = "g_rootCapturerPath";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    RoundSinkInfo(sinkInfo);
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
}

static void InitRenderStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

static void InitNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
}

void WaitForMsgProcessing(std::shared_ptr<IHpaeRendererManager> &hpaeRendererManager)
{
    if (!hpaeRendererManager->IsInit()) {
        return;
    }
    while (hpaeRendererManager->IsMsgProcessing()) {
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

void HpaeRendererManagerSetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    rendererManager->SetPrivacyType(sessionId, privacyType);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerGetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    rendererManager->GetPrivacyType(sessionId, privacyType);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->GetWritableSize(sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerUpdateSpatializationStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    rendererManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerUpdateMaxLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    rendererManager->UpdateMaxLength(sessionId, maxLength);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerGetAllSinkInputsInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    rendererManager->GetAllSinkInputsInfo();
    WaitForMsgProcessing(rendererManager);
    rendererManager->GetSinkInfo();
    WaitForMsgProcessing(rendererManager);
    rendererManager->OnNotifyQueue();
    WaitForMsgProcessing(rendererManager);
    rendererManager->GetThreadName();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DumpSinkInfo();
    WaitForMsgProcessing(rendererManager);
    rendererManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerAddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    rendererManager->AddNodeToSink(node);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerAddAllNodesToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(node);
    bool isConnect = GetData<bool>();
    rendererManager->AddAllNodesToSink(sinkInputs, isConnect);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>(g_rootCapturerPath);
    rendererManager->RegisterReadCallback(sessionId, readDataCb);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerOnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    for (size_t i = 0; i < IOperationVec.size(); i++) {
        IOperation operation = IOperationVec[i];
        rendererManager->OnNodeStatusUpdate(sessionId, operation);
    }
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerOnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->OnFadeDone(sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerOnRequestLatencyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = GetData<uint64_t>();
    rendererManager->OnRequestLatency(sessionId, latency);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerOnDisConnectProcessClusterFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    HpaeProcessorType sceneType = HPAE_SCENE_DEFAULT;
    rendererManager->OnDisConnectProcessCluster(sceneType);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    rendererManager->SetLoudnessGain(sessionId, loudnessGain);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerConnectCoBufferNodeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetNodeInfo(nodeInfo);
    rendererManager->ConnectCoBufferNode(coBufferNode);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerDisConnectCoBufferNodeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetNodeInfo(nodeInfo);
    rendererManager->DisConnectCoBufferNode(coBufferNode);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerStartWithSyncIdFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t syncId = GetData<int32_t>();
    rendererManager->StartWithSyncId(sessionId, syncId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    HpaeRendererManagerSetPrivacyTypeFuzzTest,
    HpaeRendererManagerGetPrivacyTypeFuzzTest,
    HpaeRendererManagerGetWritableSizeFuzzTest,
    HpaeRendererManagerUpdateSpatializationStateFuzzTest,
    HpaeRendererManagerUpdateMaxLengthFuzzTest,
    HpaeRendererManagerGetAllSinkInputsInfoFuzzTest,
    HpaeRendererManagerAddNodeToSinkFuzzTest,
    HpaeRendererManagerAddAllNodesToSinkFuzzTest,
    HpaeRendererManagerRegisterReadCallbackFuzzTest,
    HpaeRendererManagerOnNodeStatusUpdateFuzzTest,
    HpaeRendererManagerOnFadeDoneFuzzTest,
    HpaeRendererManagerOnRequestLatencyFuzzTest,
    HpaeRendererManagerOnDisConnectProcessClusterFuzzTest,
    HpaeRendererManagerSetLoudnessGainFuzzTest,
    HpaeRendererManagerConnectCoBufferNodeFuzzTest,
    HpaeRendererManagerDisConnectCoBufferNodeFuzzTest,
    HpaeRendererManagerStartWithSyncIdFuzzTest,
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
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
