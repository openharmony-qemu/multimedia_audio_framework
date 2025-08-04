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
#include "hpae_offload_renderer_manager.h"
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
const char* DEFAULT_TEST_DEVICE_CLASS = "offload";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
constexpr size_t THRESHOLD = 10;
constexpr uint8_t TESTSIZE = 51;

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
    std::shared_ptr<IHpaeRendererManager> offloadRendererManager =
        IHpaeRendererManager::CreateRendererManager(sinkInfo);
    string deviceName = "";
    offloadRendererManager->UploadDumpSinkInfo(deviceName);
}

void OnNotifyDfxNodeInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isConnect = false;
    uint32_t preNodeId = GetData<uint32_t>();
    HpaeDfxNodeInfo nodeInfo = {};
    offloadRendererManager->OnNotifyDfxNodeInfo(isConnect, preNodeId, nodeInfo);
}

void HpaeOffloadRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeOffloadRendererManager offloadRendererManager(sinkInfo);
}

void HpaeOffloadRendererManagerCreateStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    offloadRendererManager->CreateStream(streamInfo);
}

void HpaeOffloadRendererManagerDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->DestroyStream(sessionId);
}

void HpaeOffloadRendererManagerStartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Start(sessionId);
}

void HpaeOffloadRendererManagerPauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Pause(sessionId);
}

void HpaeOffloadRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Flush(sessionId);
}

void HpaeOffloadRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Drain(sessionId);
}

void HpaeOffloadRendererManagerStopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Stop(sessionId);
}

void HpaeOffloadRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Release(sessionId);
}

void HpaeOffloadRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "";
    offloadRendererManager->MoveStream(sessionId, sinkName);
}

void HpaeOffloadRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    string sinkName = "";
    vector<uint32_t> sessionIds;
    MoveSessionType moveSessionType = MOVE_ALL;
    offloadRendererManager->MoveAllStream(sinkName, sessionIds, moveSessionType);
}

void HpaeOffloadRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isSuspend = false;
    offloadRendererManager->SuspendStreamManager(isSuspend);
}

void HpaeOffloadRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isMute = false;
    offloadRendererManager->SetMute(isMute);
}

void HpaeOffloadRendererManagerProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Process();
}

void HpaeOffloadRendererManagerHandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->HandleMsg();
}

void HpaeOffloadRendererManagerInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
}

void HpaeOffloadRendererManagerDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isMoveDefault = false;
    offloadRendererManager->DeInit(isMoveDefault);
}

void HpaeOffloadRendererManagerIsInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->IsInit();
}

void HpaeOffloadRendererManagerIsRunningFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->IsRunning();
}

void HpaeOffloadRendererManagerIsMsgProcessingFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->IsMsgProcessing();
}

void HpaeOffloadRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->DeactivateThread();
}

void HpaeOffloadRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    offloadRendererManager->SetClientVolume(sessionId, volume);
}

void HpaeOffloadRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    offloadRendererManager->SetRate(sessionId, rate);
}

void HpaeOffloadRendererManagerSetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    offloadRendererManager->SetAudioEffectMode(sessionId, effectMode);
}

void HpaeOffloadRendererManagerGetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    offloadRendererManager->GetAudioEffectMode(sessionId, effectMode);
}

void HpaeOffloadRendererManagerSetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    offloadRendererManager->SetPrivacyType(sessionId, privacyType);
}

void HpaeOffloadRendererManagerGetPrivacyTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    offloadRendererManager->GetPrivacyType(sessionId, privacyType);
}

void HpaeOffloadRendererManagerRegisterWriteCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    offloadRendererManager->RegisterWriteCallback(sessionId, writeIncDataCb);
}

void HpaeOffloadRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->GetWritableSize(sessionId);
}

void HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    offloadRendererManager->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

void HpaeOffloadRendererManagerUpdateMaxLengthFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    offloadRendererManager->UpdateMaxLength(sessionId, maxLength);
}

void HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->GetAllSinkInputsInfo();
}

void HpaeOffloadRendererManagerGetSinkInputInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    HpaeSinkInputInfo sinkInputInfo;
    offloadRendererManager->GetSinkInputInfo(sessionId, sinkInputInfo);
}

void HpaeOffloadRendererManagerGetSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->GetSinkInfo();
}

void HpaeOffloadRendererManagerAddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    offloadRendererManager->AddNodeToSink(node);
}

void HpaeOffloadRendererManagerAddAllNodesToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(node);
    bool isConnect = false;
    offloadRendererManager->AddAllNodesToSink(sinkInputs, isConnect);
}

void HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>(g_rootCapturerPath);
    offloadRendererManager->RegisterReadCallback(sessionId, readDataCb);
}

void HpaeOffloadRendererManagerOnNodeStatusUpdateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = OPERATION_INVALID;
    offloadRendererManager->OnNodeStatusUpdate(sessionId, operation);
}

void HpaeOffloadRendererManagerOnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    IOperation operation = OPERATION_INVALID;
    offloadRendererManager->OnFadeDone(sessionId, operation);
}

void HpaeOffloadRendererManagerOnNotifyQueueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->OnNotifyQueue();
}

void HpaeOffloadRendererManagerGetThreadNameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->GetThreadName();
}

void HpaeOffloadRendererManagerDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->DumpSinkInfo();
}

void HpaeOffloadRendererManagerReloadRenderManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    newSinkInfo.samplingRate = SAMPLE_RATE_16000;
    offloadRendererManager->ReloadRenderManager(newSinkInfo);
}

void HpaeOffloadRendererManagerSetOffloadPolicyFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    offloadRendererManager->SetOffloadPolicy(sessionId, state);
}

void HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->GetDeviceHDFDumpInfo();
}

void HpaeOffloadRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    offloadRendererManager->SetLoudnessGain(sessionId, loudnessGain);
}


void HpaeOffloadRendererManagerSetSpeedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    float speed = GetData<float>();
    offloadRendererManager->SetSpeed(sessionId, speed);
}

void HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t type = GetData<int32_t>();
    offloadRendererManager->SetOffloadRenderCallbackType(sessionId, type);
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[TESTSIZE] = {
    CreateRendererManagerFuzzTest,
    UploadDumpSinkInfoFuzzTest,
    OnNotifyDfxNodeInfoFuzzTest,
    HpaeOffloadRendererManagerConstructFuzzTest,
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
    HpaeOffloadRendererManagerSetMuteFuzzTest,
    HpaeOffloadRendererManagerProcessFuzzTest,
    HpaeOffloadRendererManagerHandleMsgFuzzTest,
    HpaeOffloadRendererManagerInitFuzzTest,
    HpaeOffloadRendererManagerDeInitFuzzTest,
    HpaeOffloadRendererManagerIsInitFuzzTest,
    HpaeOffloadRendererManagerIsRunningFuzzTest,
    HpaeOffloadRendererManagerIsMsgProcessingFuzzTest,
    HpaeOffloadRendererManagerDeactivateThreadFuzzTest,
    HpaeOffloadRendererManagerSetClientVolumeFuzzTest,
    HpaeOffloadRendererManagerSetRateFuzzTest,
    HpaeOffloadRendererManagerSetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerGetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerSetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerGetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerRegisterWriteCallbackFuzzTest,
    HpaeOffloadRendererManagerGetWritableSizeFuzzTest,
    HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest,
    HpaeOffloadRendererManagerUpdateMaxLengthFuzzTest,
    HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest,
    HpaeOffloadRendererManagerGetSinkInputInfoFuzzTest,
    HpaeOffloadRendererManagerGetSinkInfoFuzzTest,
    HpaeOffloadRendererManagerAddNodeToSinkFuzzTest,
    HpaeOffloadRendererManagerAddAllNodesToSinkFuzzTest,
    HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest,
    HpaeOffloadRendererManagerOnNodeStatusUpdateFuzzTest,
    HpaeOffloadRendererManagerOnFadeDoneFuzzTest,
    HpaeOffloadRendererManagerOnNotifyQueueFuzzTest,
    HpaeOffloadRendererManagerGetThreadNameFuzzTest,
    HpaeOffloadRendererManagerDumpSinkInfoFuzzTest,
    HpaeOffloadRendererManagerReloadRenderManagerFuzzTest,
    HpaeOffloadRendererManagerSetOffloadPolicyFuzzTest,
    HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest,
    HpaeOffloadRendererManagerSetLoudnessGainFuzzTest,
    HpaeOffloadRendererManagerSetSpeedFuzzTest,
    HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest,
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
