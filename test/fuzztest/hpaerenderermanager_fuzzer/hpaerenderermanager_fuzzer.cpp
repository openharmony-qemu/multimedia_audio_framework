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

static void InitHpaeSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = "g_rootCapturerPath";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
}

static void InitRenderStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
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

void CreateRendererManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    IHpaeRendererManager::CreateRendererManager(sinkInfo);
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
    bool isConnect = false;
    uint32_t preNodeId = GetData<uint32_t>();
    HpaeDfxNodeInfo nodeInfo = {};
    rendererManager->OnNotifyDfxNodeInfo(isConnect, preNodeId, nodeInfo);
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
    bool isSuspend = false;
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
    bool isMute = false;
    rendererManager->SetMute(isMute);
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    rendererManager->Process();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerHandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    rendererManager->HandleMsg();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerIsMsgProcessingFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    rendererManager->IsMsgProcessing();
    WaitForMsgProcessing(rendererManager);
    rendererManager->DeInit();
}

void HpaeRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
    rendererManager->DeactivateThread();
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
    uint32_t sessionId = GetData<uint32_t>();
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

    uint32_t sessionId = GetData<uint32_t>();
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
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);

    uint32_t sessionId = GetData<uint32_t>();
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
    hpaeRendererManager->UpdateCollaborativeState(true);

    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    uint32_t sessionId = GetData<uint32_t>();
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
    hpaeRendererManager->UpdateCollaborativeState(false);
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

void DeleteInputSessionForMoveFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = HpaeRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager.DeleteInputSessionForMove(sessionId);
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

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    CreateRendererManagerFuzzTest,
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
    HpaeRendererManagerProcessFuzzTest,
    HpaeRendererManagerHandleMsgFuzzTest,
    HpaeRendererManagerIsMsgProcessingFuzzTest,
    HpaeRendererManagerDeactivateThreadFuzzTest,
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
    DeleteInputSessionForMoveFuzzTest,
    DeleteProcessClusterFuzzTest,
    ConnectInputSessionFuzzTest,
    DisConnectInputSessionFuzzTest,
    CheckIsStreamRunningFuzzTest,
    CreateOutputClusterNodeInfoFuzzTest,
    StartRenderSinkFuzzTest,
    SetSpeedFuzzTest,
    ReConnectNodeForCollaborationFuzzTest,
    EnableAndDisableCollaborationFuzzTest,
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
