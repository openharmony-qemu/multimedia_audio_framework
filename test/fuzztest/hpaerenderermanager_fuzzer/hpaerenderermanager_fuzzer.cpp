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

#include "hpaerenderermanager_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
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
#include "i_stream.h" // IOperation enum, used directly by FUZZ_OPERATIONS

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
constexpr int32_t MAX_MSG_PROCESSING_RETRY_COUNT = 50;
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

// IOperation values fed to OnNodeStatusUpdate. That method is a thin forwarder
// today (no branch on the value), so this set mainly guards the entry point and
// keeps diversity in case branching is added later.
const std::vector<IOperation> FUZZ_OPERATIONS {
    OPERATION_STARTED,
    OPERATION_PAUSED,
    OPERATION_STOPPED,
    OPERATION_FLUSHED,
    OPERATION_DRAINED,
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
    int retry = 0;
    while (hpaeRendererManager->IsMsgProcessing() && retry < MAX_MSG_PROCESSING_RETRY_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        retry += 1;
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

void UploadDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    hpaeRendererManager->sinkInfo_ = sinkInfo;
    rendererManager->Init();
    string deviceName = "";
    rendererManager->UploadDumpSinkInfo(deviceName);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void OnNotifyDfxNodeInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    bool isConnect = GetData<bool>();
    uint32_t preNodeId = GetData<uint32_t>();
    uint32_t nodeId = GetData<uint32_t>();
    rendererManager->OnNotifyDfxNodeInfo(isConnect, preNodeId, nodeId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeRendererManager rendererManager(sinkInfo);
}

void HpaeRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Flush(sessionId);
    rendererManager->DeInit();
}

void HpaeRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Drain(sessionId);
    rendererManager->DeInit();
}

void HpaeRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Release(sessionId);
    rendererManager->DeInit();
}

void HpaeRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "";
    rendererManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    string sinkName = "";
    vector<uint32_t> sessionIds;
    MoveSessionType moveSessionType = MOVE_ALL;
    rendererManager->MoveAllStream(sinkName, sessionIds, moveSessionType);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    bool isSuspend = GetData<bool>();
    rendererManager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    bool isMute = GetData<bool>();
    rendererManager->SetMute(isMute);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    rendererManager->SetClientVolume(sessionId, volume);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    rendererManager->SetRate(sessionId, rate);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerSetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    rendererManager->SetAudioEffectMode(sessionId, effectMode);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerGetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    rendererManager->GetAudioEffectMode(sessionId, effectMode);
    rendererManager->DeInit();
}

void IRendererManagerReloadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->IsInit();

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId: GetData<uint32_t>();
    rendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);

    rendererManager->ReloadRenderManager(sinkInfo, true);
    WaitForMsgProcessing(rendererManager);
    rendererManager->IsInit();

    rendererManager->Start(sessionId);
    rendererManager->SetOffloadPolicy(sessionId, 0);
    WaitForMsgProcessing(rendererManager);

    rendererManager->SetSpeed(sessionId, 1.0f);
    WaitForMsgProcessing(rendererManager);

    rendererManager->DeInit();
    rendererManager->IsInit();

    rendererManager->ReloadRenderManager(sinkInfo, true);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void IRendererManagerCreateDestoryStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    hpaeRendererManager->Init();
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);

    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId: GetData<uint32_t>();
    hpaeRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);

    InitRenderStreamInfo(streamInfo);
    hpaeRendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeRendererManager);

    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);
    hpaeRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);
    hpaeRendererManager->DeInit();
}

void IRendererManagerStartPuaseStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    hpaeRendererManager->Init();
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->IsInit();
    HpaeStreamInfo streamInfo;
    HpaeSinkInputInfo sinkInputInfo;
    InitRenderStreamInfo(streamInfo);
    hpaeRendererManager->CreateStream(streamInfo);
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId: GetData<uint32_t>();
    hpaeRendererManager->RegisterWriteCallback(sessionId, writeIncDataCb);
    hpaeRendererManager->Start(sessionId);

    hpaeRendererManager->SetOffloadPolicy(sessionId, 0);
    hpaeRendererManager->SetSpeed(sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);

    hpaeRendererManager->IsRunning();
    hpaeRendererManager->Pause(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);

    hpaeRendererManager->Stop(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);

    hpaeRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->DeInit();
}

void UpdateCollaborativeStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    hpaeRendererManager->Init();
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->IsInit();
    hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    hpaeRendererManager->CreateStream(streamInfo);
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t sessionId = GetData<bool>() ? streamInfo.sessionId: GetData<uint32_t>();
    hpaeRendererManager->RegisterWriteCallback(sessionId, writeIncDataCb);

    hpaeRendererManager->Start(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);

    hpaeRendererManager->IsRunning();
    hpaeRendererManager->Pause(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->Stop(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);

    hpaeRendererManager->DestroyStream(sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);
    hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->DeInit();
}

void AddSingleNodeToSinkAndCreateEffectAndConnectFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeNodeInfo nodeinfo;
    InitNodeInfo(nodeinfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeinfo);
    CHECK_AND_RETURN(node != nullptr);
    bool isConnect = GetData<bool>();
    rendererManager.AddSingleNodeToSink(node, isConnect);
}

void CreateDefaultProcessClusterFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    HpaeNodeInfo nodeinfo;
    InitNodeInfo(nodeinfo);
    rendererManager.CreateDefaultProcessCluster(nodeinfo);
}

void CreateProcessClusterInnerSwitchCaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    HpaeNodeInfo nodeinfo;
    InitNodeInfo(nodeinfo);
    int32_t processClusterDecision = GetData<int32_t>();
    rendererManager.CreateProcessClusterInner(nodeinfo, processClusterDecision);
}

void RefreshProcessClusterByDeviceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    CHECK_AND_RETURN(rendererManager != nullptr);
    rendererManager->RefreshProcessClusterByDevice();
}

void DeleteInputSessionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager.DeleteInputSession(sessionId);
}

void DeleteProcessClusterFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    HpaeProcessorType sceneType = GetData<HpaeProcessorType>();
    rendererManager.DeleteProcessCluster(sceneType);
}

void ConnectInputSessionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = sessionId;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
    rendererManager.sinkInputNodeMap_[sessionId] = node;
    rendererManager.ConnectInputSession(sessionId);
}

void DisConnectInputSessionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager.DisConnectInputSession(sessionId);
}

void CheckIsStreamRunningFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    rendererManager.CheckIsStreamRunning();
}

void CreateOutputClusterNodeInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    HpaeNodeInfo nodeinfo;
    InitNodeInfo(nodeinfo);
    rendererManager.CreateOutputClusterNodeInfo(nodeinfo);
}

void StartRenderSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    rendererManager.StartRenderSink();
}

void SetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    CHECK_AND_RETURN(rendererManager != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    rendererManager->SetSpeed(sessionId, speed);
}

void ReConnectNodeForCollaborationFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager.ReConnectNodeForCollaboration(sessionId);
}

void EnableAndDisableCollaborationFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    rendererManager.EnableCollaboration();
    rendererManager.DisableCollaboration();
}

void EnableCollaborationWithMovieSceneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    streamInfo.effectInfo.effectScene = SCENE_MOVIE;
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);
    WaitForMsgProcessing(rendererManager);
    rendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void OnCollaborationFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->OnCollborationFadeDone(sessionId);
    rendererManager->DeInit();
}

void OnFadeDoneWithActiveSessionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->OnFadeDone(streamInfo.sessionId);
    rendererManager->DeInit();
}

void CoBufferConnectDisconnectSequenceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    coBufferNode->SetNodeInfo(nodeInfo);
    rendererManager->ConnectCoBufferNode(coBufferNode);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DisConnectCoBufferNode(coBufferNode);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void ConnectProcessClusterFullFlowFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// ===== Incremental coverage supplement =====
// The functions below cover public methods of HpaeRendererManager that were not
// exercised by the original dispatch table. They only target code defined in
// hpae_renderer_manager.cpp (the Session Effect cluster lives in a sibling file
// and is intentionally out of scope here).

void StartWithSyncIdFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);
    // The real id reaches the request body (SafeGetMap passes); a random id
    // also exercises the not-found guard, so both branches are covered.
    int32_t syncId = GetData<int32_t>();
    rendererManager->StartWithSyncId(streamInfo.sessionId, syncId);
    WaitForMsgProcessing(rendererManager);
    uint32_t fuzzSessionId = GetData<uint32_t>();
    rendererManager->StartWithSyncId(fuzzSessionId, GetData<int32_t>());
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void SetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    // The real id reaches the body (node + cluster loudness set); a random id
    // also exercises the not-found guard.
    float loudnessGain = GetData<float>();
    rendererManager->SetLoudnessGain(streamInfo.sessionId, loudnessGain);
    WaitForMsgProcessing(rendererManager);
    rendererManager->SetLoudnessGain(GetData<uint32_t>(), GetData<float>());
    WaitForMsgProcessing(rendererManager);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void SetAndGetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    rendererManager->SetPrivacyType(sessionId, privacyType);
    int32_t outPrivacyType = GetData<int32_t>();
    rendererManager->GetPrivacyType(sessionId, outPrivacyType);
    rendererManager->DeInit();
}

void RendererManagerSetterStubFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->GetWritableSize(sessionId);
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    rendererManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
    uint32_t maxLength = GetData<uint32_t>();
    rendererManager->UpdateMaxLength(sessionId, maxLength);
    rendererManager->DeInit();
}

void RendererManagerQueryFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->GetAllSinkInputsInfo();
    rendererManager->GetSinkInfo();
    rendererManager->GetThreadName();
    rendererManager->GetDeviceHDFDumpInfo();
    rendererManager->DeInit();
}

void DumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DumpSinkInfo();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void AddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
    rendererManager->AddNodeToSink(node);
    WaitForMsgProcessing(rendererManager);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs = {node};
    bool isConnect = GetData<bool>();
    rendererManager->AddAllNodesToSink(sinkInputs, isConnect);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void RegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    uint32_t sessionId = GetData<uint32_t>();
    // RegisterReadCallback currently ignores its argument; pass an empty weak_ptr
    // to exercise the entry point without triggering any file I/O.
    std::weak_ptr<ICapturerStreamCallback> readCb;
    rendererManager->RegisterReadCallback(sessionId, readCb);
    rendererManager->DeInit();
}

// Uses a non-initialized stack instance on purpose: OnNodeStatusUpdate touches
// sessionNodeMap_ directly, so avoiding the SignalProcessThread removes any
// concurrent-access risk while still exercising the entry point.
void OnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeRendererManager rendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = FUZZ_OPERATIONS[GetData<uint32_t>() % FUZZ_OPERATIONS.size()];
    rendererManager.OnNodeStatusUpdate(sessionId, operation);
}

void OnRequestLatencyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    // A live session + cluster makes the body accumulate latency instead of
    // hitting the SafeGetMap guard. The query is read-only and is invoked after
    // the queue drains (same thread context the latency query runs in).
    uint64_t latency = GetData<uint64_t>();
    rendererManager->OnRequestLatency(streamInfo.sessionId, latency);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void OnRewindBufferFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    // Rewind on a stopped session (no active pull) keeps the history-buffer
    // mutation race-free; the real id reaches RewindHistoryBuffer.
    uint64_t rewindTimeUs = GetData<uint64_t>();
    rendererManager->OnRewindBuffer(streamInfo.sessionId, rewindTimeUs);
    rendererManager->DeInit();
}

void OnNotifyQueueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->OnNotifyQueue();
    rendererManager->DeInit();
}

void OnDisConnectProcessClusterFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    rendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    // A started stream populates sceneClusterMap_, so a matching sceneType
    // reaches the GetPreOutNum() check instead of the empty-map guard. The deep
    // DisConnectOutputCluster branch additionally needs GetPreOutNum() == 0.
    HpaeProcessorType sceneType = GetData<HpaeProcessorType>();
    rendererManager->OnDisConnectProcessCluster(sceneType);
    WaitForMsgProcessing(rendererManager);
    rendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void OnDisConnectCoBufferWaitFadeOutDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->OnDisConnectCoBufferWaitFadeOutDone();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// The file_io test sink never matches the BT/USB device-class guard, so this
// exercises the guarded early-return path. Reaching the real
// outputCluster_->SetAuxiliarySinkEnable branch requires live BT/USB hardware.
void SetAuxiliarySinkEnableFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    bool isEnabled = GetData<bool>();
    rendererManager->SetAuxiliarySinkEnable(isEnabled);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// Drive collaboration through the normal Init path (mirrors
// EnableCollaborationWithMovieSceneFuzzTest), then invoke SetCollDelayCount
// after the message queue has drained so the direct map access is race-free.
void SetCollDelayCountFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);
    WaitForMsgProcessing(rendererManager);
    rendererManager->SetCollDelayCount();
    rendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// The file_io test sink is not a remote device, so IsRemoteDevice() short-
// circuits the request. Reaching the apps-uid body needs a real remote HDI
// device with a non-null output cluster.
void TriggerAppsUidUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->TriggerAppsUidUpdate(sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void StopManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->StopManager();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// DeactivateThread tears down the SignalProcessThread, so it must run last on
// its own instance; nothing else is dispatched afterwards.
void DeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->IsMsgProcessing();
    rendererManager->DeactivateThread();
    rendererManager->DeInit();
}

// HandleMsg drains the message queue directly, so a non-initialized stack
// instance (no worker thread) is used to avoid any concurrent queue access.
void HandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeRendererManager rendererManager(sinkInfo);
    rendererManager.IsMsgProcessing();
    rendererManager.HandleMsg();
}

void SessionEffectLifecycleFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
    WaitForMsgProcessing(rendererManager);

    uint32_t sessionId = streamInfo.sessionId;
    std::string effectName = "test_effect";
    rendererManager->PostCreateSessionEffect(sessionId, effectName, nullptr, {});
    WaitForMsgProcessing(rendererManager);
    rendererManager->HasSessionEffect(sessionId);
    int32_t type = GetData<int32_t>();
    float value = GetData<float>();
    rendererManager->PostSetSessionEffectParameter(sessionId, type, value);
    rendererManager->PostDestroySessionEffect(sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->PostDestroySessionEffect(sessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

// Exercise the session-effect API with a sessionId that was never created via CreateStream,
// plus IsValidSession and the async Post* wrappers. Hits the not-found / false early-return
// branches (and IsValidSession / PostCreateSessionEffect, which the lifecycle test never calls).
void SessionEffectInvalidSessionFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);

    uint32_t invalidSessionId = GetData<uint32_t>(); // almost certainly absent from sessionNodeMap_
    int32_t type = GetData<int32_t>();
    float value = GetData<float>();
    std::string effectName = "test_effect";

    // Synchronous not-found / false branches:
    rendererManager->PostCreateSessionEffect(invalidSessionId, effectName, nullptr, {});
    rendererManager->PostDestroySessionEffect(invalidSessionId);
    (void)rendererManager->HasSessionEffect(invalidSessionId);
    (void)rendererManager->IsValidSession(invalidSessionId);
    rendererManager->PostSetSessionEffectParameter(invalidSessionId, type, value);

    // Asynchronous Post* wrappers (dispatched to the signal-process thread):
    rendererManager->PostSetSessionEffectParameter(invalidSessionId, type, value);
    rendererManager->PostCreateSessionEffect(invalidSessionId, effectName, nullptr, {});
    rendererManager->PostDestroySessionEffect(invalidSessionId);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HDFDumpFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    WaitForMsgProcessing(rendererManager);
    rendererManager->GetDeviceHDFDumpInfo();
    rendererManager->DeInit();
}
const std::vector<std::string> REMOTE_DEVICE_CLASS_LIST {
    "remote", "remote_multichannel", "primary", "a2dp", "offload", ""};

// HiCar MCH remote/multichannel device-class detection (PR #15881).
// Construct-only (no Init/worker thread), pure read of sinkInfo_.
void IsRemoteDeviceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    sinkInfo.deviceClass = REMOTE_DEVICE_CLASS_LIST[GetData<uint32_t>() % REMOTE_DEVICE_CLASS_LIST.size()];
    HpaeRendererManager rendererManager(sinkInfo);
    rendererManager.IsRemoteDevice();
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    UploadDumpSinkInfoFuzzTest,
    OnNotifyDfxNodeInfoFuzzTest,
    HpaeRendererManagerConstructFuzzTest,
    HpaeRendererManagerFlushFuzzTest,
    HpaeRendererManagerDrainFuzzTest,
    HpaeRendererManagerReleaseFuzzTest,
    HpaeRendererManagerMoveStreamFuzzTest,
    HpaeRendererManagerMoveAllStreamFuzzTest,
    HpaeRendererManagerSuspendStreamManagerFuzzTest,
    HpaeRendererManagerSetMuteFuzzTest,
    HpaeRendererManagerSetClientVolumeFuzzTest,
    HpaeRendererManagerSetRateFuzzTest,
    HpaeRendererManagerSetAudioEffectModeFuzzTest,
    HpaeRendererManagerGetAudioEffectModeFuzzTest,
    IRendererManagerReloadFuzzTest,
    IRendererManagerCreateDestoryStreamFuzzTest,
    IRendererManagerStartPuaseStreamFuzzTest,
    UpdateCollaborativeStateFuzzTest,
    AddSingleNodeToSinkAndCreateEffectAndConnectFuzzTest,
    CreateDefaultProcessClusterFuzzTest,
    CreateProcessClusterInnerSwitchCaseFuzzTest,
    RefreshProcessClusterByDeviceFuzzTest,
    DeleteInputSessionFuzzTest,
    DeleteProcessClusterFuzzTest,
    ConnectInputSessionFuzzTest,
    DisConnectInputSessionFuzzTest,
    CheckIsStreamRunningFuzzTest,
    CreateOutputClusterNodeInfoFuzzTest,
    StartRenderSinkFuzzTest,
    SetSpeedFuzzTest,
    ReConnectNodeForCollaborationFuzzTest,
    EnableAndDisableCollaborationFuzzTest,
    EnableCollaborationWithMovieSceneFuzzTest,
    OnCollaborationFadeDoneFuzzTest,
    OnFadeDoneWithActiveSessionFuzzTest,
    CoBufferConnectDisconnectSequenceFuzzTest,
    ConnectProcessClusterFullFlowFuzzTest,
    SessionEffectLifecycleFuzzTest,
    SessionEffectInvalidSessionFuzzTest,
    HDFDumpFuzzTest,
    StartWithSyncIdFuzzTest,
    SetLoudnessGainFuzzTest,
    SetAndGetPrivacyTypeFuzzTest,
    RendererManagerSetterStubFuzzTest,
    RendererManagerQueryFuzzTest,
    DumpSinkInfoFuzzTest,
    AddNodeToSinkFuzzTest,
    RegisterReadCallbackFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnRequestLatencyFuzzTest,
    OnRewindBufferFuzzTest,
    OnNotifyQueueFuzzTest,
    OnDisConnectProcessClusterFuzzTest,
    OnDisConnectCoBufferWaitFadeOutDoneFuzzTest,
    SetAuxiliarySinkEnableFuzzTest,
    SetCollDelayCountFuzzTest,
    TriggerAppsUidUpdateFuzzTest,
    StopManagerFuzzTest,
    DeactivateThreadFuzzTest,
    HandleMsgFuzzTest,
    IsRemoteDeviceFuzzTest,
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
