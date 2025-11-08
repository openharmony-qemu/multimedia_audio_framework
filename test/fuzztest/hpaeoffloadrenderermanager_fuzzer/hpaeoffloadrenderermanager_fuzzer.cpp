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

vector<MoveSessionType> MoveSessionTypeVec = {
    MOVE_SINGLE,
    MOVE_ALL,
    MOVE_PREFER,
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
    RoundSinkInfo(sinkInfo);
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
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

void UploadDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> offloadRendererManager =
        IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    string deviceName = "";
    offloadRendererManager->UploadDumpSinkInfo(deviceName);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void OnNotifyDfxNodeInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    bool isConnect = GetData<bool>();
    uint32_t preNodeId = GetData<uint32_t>();
    uint32_t nodeId = GetData<uint32_t>();
    offloadRendererManager->OnNotifyDfxNodeInfo(isConnect, preNodeId, nodeId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeOffloadRendererManager offloadRendererManager(sinkInfo);
}

void HpaeOffloadRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Flush(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Drain(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->Release(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "";
    offloadRendererManager->MoveStream(sessionId, sinkName);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    string sinkName = "";
    vector<uint32_t> sessionIds;
    for (size_t i = 0; i < MoveSessionTypeVec.size(); i++) {
        MoveSessionType moveSessionType = MoveSessionTypeVec[i];
        offloadRendererManager->MoveAllStream(sinkName, sessionIds, moveSessionType);
        WaitForMsgProcessing(offloadRendererManager);
    }
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    bool isSuspend = GetData<bool>();
    offloadRendererManager->SuspendStreamManager(isSuspend);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    bool isMute = GetData<bool>();
    offloadRendererManager->SetMute(isMute);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t privacyType = GetData<int32_t>();
    offloadRendererManager->GetPrivacyType(sessionId, privacyType);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetWritableSizeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->GetWritableSize(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t maxLength = GetData<uint32_t>();
    offloadRendererManager->UpdateMaxLength(sessionId, maxLength);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->GetAllSinkInputsInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->GetSinkInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerAddNodeToSinkFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
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
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    sinkInputs.emplace_back(node);
    bool isConnect = GetData<bool>();
    offloadRendererManager->AddAllNodesToSink(sinkInputs, isConnect);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<ReadDataCb> readDataCb = std::make_shared<ReadDataCb>(g_rootCapturerPath);
    offloadRendererManager->RegisterReadCallback(sessionId, readDataCb);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnFadeDoneFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    offloadRendererManager->OnFadeDone(sessionId);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerOnNotifyQueueFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->OnNotifyQueue();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetThreadNameFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->GetThreadName();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerDumpSinkInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->DumpSinkInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerReloadRenderManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    HpaeSinkInfo newSinkInfo;
    InitHpaeSinkInfo(newSinkInfo);
    newSinkInfo.samplingRate = SAMPLE_RATE_16000;
    offloadRendererManager->ReloadRenderManager(newSinkInfo);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    offloadRendererManager->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetLoudnessGainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    offloadRendererManager->SetLoudnessGain(sessionId, loudnessGain);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
}

void HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto offloadRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    offloadRendererManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t type = GetData<int32_t>();
    offloadRendererManager->SetOffloadRenderCallbackType(sessionId, type);
    WaitForMsgProcessing(offloadRendererManager);
    offloadRendererManager->DeInit();
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
    
    bool isReload = GetData<bool>();
    rendererManager->ReloadRenderManager(sinkInfo, isReload);
    WaitForMsgProcessing(rendererManager);
    rendererManager->IsInit();

    rendererManager->Start(sessionId);
    rendererManager->SetOffloadPolicy(sessionId, 0);
    WaitForMsgProcessing(rendererManager);

    rendererManager->SetSpeed(sessionId, 1.0f);
    WaitForMsgProcessing(rendererManager);

    rendererManager->DeInit();
    rendererManager->IsInit();

    rendererManager->ReloadRenderManager(sinkInfo, isReload);
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

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    UploadDumpSinkInfoFuzzTest,
    OnNotifyDfxNodeInfoFuzzTest,
    HpaeOffloadRendererManagerConstructFuzzTest,
    HpaeOffloadRendererManagerFlushFuzzTest,
    HpaeOffloadRendererManagerDrainFuzzTest,
    HpaeOffloadRendererManagerReleaseFuzzTest,
    HpaeOffloadRendererManagerMoveStreamFuzzTest,
    HpaeOffloadRendererManagerMoveAllStreamFuzzTest,
    HpaeOffloadRendererManagerSuspendStreamManagerFuzzTest,
    HpaeOffloadRendererManagerSetMuteFuzzTest,
    HpaeOffloadRendererManagerSetClientVolumeFuzzTest,
    HpaeOffloadRendererManagerSetRateFuzzTest,
    HpaeOffloadRendererManagerSetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerGetAudioEffectModeFuzzTest,
    HpaeOffloadRendererManagerSetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerGetPrivacyTypeFuzzTest,
    HpaeOffloadRendererManagerGetWritableSizeFuzzTest,
    HpaeOffloadRendererManagerUpdateSpatializationStateFuzzTest,
    HpaeOffloadRendererManagerUpdateMaxLengthFuzzTest,
    HpaeOffloadRendererManagerGetAllSinkInputsInfoFuzzTest,
    HpaeOffloadRendererManagerGetSinkInfoFuzzTest,
    HpaeOffloadRendererManagerAddNodeToSinkFuzzTest,
    HpaeOffloadRendererManagerAddAllNodesToSinkFuzzTest,
    HpaeOffloadRendererManagerRegisterReadCallbackFuzzTest,
    HpaeOffloadRendererManagerOnFadeDoneFuzzTest,
    HpaeOffloadRendererManagerOnNotifyQueueFuzzTest,
    HpaeOffloadRendererManagerGetThreadNameFuzzTest,
    HpaeOffloadRendererManagerDumpSinkInfoFuzzTest,
    HpaeOffloadRendererManagerReloadRenderManagerFuzzTest,
    HpaeOffloadRendererManagerGetDeviceHDFDumpInfoFuzzTest,
    HpaeOffloadRendererManagerSetLoudnessGainFuzzTest,
    HpaeOffloadRendererManagerSetOffloadRenderCallbackTypeFuzzTest,
    IRendererManagerReloadFuzzTest,
    IRendererManagerCreateDestoryStreamFuzzTest,
    IRendererManagerStartPuaseStreamFuzzTest,
    UpdateCollaborativeStateFuzzTest,
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
