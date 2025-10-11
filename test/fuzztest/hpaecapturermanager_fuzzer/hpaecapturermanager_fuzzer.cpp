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
#include <queue>
#include <string>
#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_ec_info.h"
#include "i_stream.h"
#include "hpae_capturer_manager.h"
#include "hpae_source_output_node.h"
#include "hpaecapturermanager_fuzzer.h"
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
static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const char* DEFAULT_TEST_DEVICE_CLASS = "file_io";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";
const uint32_t DEFAULT_SESSION_ID = 123456;
const uint32_t ECTYPENUM = 3;
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

typedef void (*TestPtr)(const uint8_t *, size_t);

class DummyCapturerStreamCallback : public ICapturerStreamCallback {
public:
    virtual ~DummyCapturerStreamCallback() = default;
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override
    {
        return SUCCESS;
    }
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

template<class T>
void RoundVal(T &roundVal, const std::vector<T>& list)
{
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>()%list.size()];
    }
}

void RoundSourceInfo(HpaeSourceInfo &sourceInfo)
{
    RoundVal(sourceInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(sourceInfo.format, AUDIO_SUPPORTED_FORMATS);
    if (GetData<bool>()) {
        sourceInfo.ecType = static_cast<OHOS::AudioStandard::HPAE::HpaeEcType>(GetData<int32_t>());
    } else {
        sourceInfo.ecType = static_cast<OHOS::AudioStandard::HPAE::HpaeEcType>(GetData<uint32_t>() % ECTYPENUM);
    }
    sourceInfo.micRef = HPAE_REF_OFF;
}

void RoundStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundVal(streamInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(streamInfo.format, AUDIO_SUPPORTED_FORMATS);
}

void InitSourceInfo(HpaeSourceInfo &sourceInfo)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = g_rootCapturerPath;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    RoundSourceInfo(sourceInfo);
}

void InitReloadStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.deviceName = "Built_in_mic";
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

void InitReloadSourceInfo(HpaeSourceInfo &sourceInfo, HpaeSourceInfo &newSourceInfo)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = g_rootCapturerPath;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    RoundSourceInfo(sourceInfo);

    newSourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    newSourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    newSourceInfo.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    newSourceInfo.filePath = g_rootCapturerPath;
    newSourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    newSourceInfo.samplingRate = SAMPLE_RATE_48000;
    RoundSourceInfo(newSourceInfo);
}

void GetFuzzNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = GetData<uint32_t>();
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
}

HpaeCaptureMoveInfo GetHpaeCaptureMoveInfo()
{
    HpaeCaptureMoveInfo moveInfo;
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    moveInfo.sessionId = GetData<uint32_t>();
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    return moveInfo;
}

static void WaitForMsgProcessing(std::shared_ptr<HpaeCapturerManager> &capturerManager)
{
    while (capturerManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 20 for sleep
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));  // 40 for sleep
}

void StateControlFuzzTest(std::shared_ptr<HpaeCapturerManager> &capturerManager, HpaeStreamInfo &streamInfo,
    HpaeSourceOutputInfo &sourceOutputInfo)
{
    uint32_t sessionId = GetData<uint32_t>();
    capturerManager->Start(sessionId);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
    
    capturerManager->Pause(sessionId);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
    capturerManager->IsRunning();

    capturerManager->Start(sessionId);
    bool isMute = GetData<bool>();
    capturerManager->SetMute(isMute);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
    capturerManager->Flush(sessionId);
    capturerManager->Drain(sessionId);
    capturerManager->Stop(sessionId);
    capturerManager->OnNodeStatusUpdate(sessionId, OPERATION_STOPPED);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
    capturerManager->IsRunning();

    capturerManager->Release(sessionId);
    WaitForMsgProcessing(capturerManager);
    
    capturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
}

void StateControlTest(std::shared_ptr<HpaeCapturerManager> &capturerManager, HpaeStreamInfo &streamInfo,
    HpaeSourceOutputInfo &sourceOutputInfo)
{
    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
    
    capturerManager->Pause(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->IsRunning();

    capturerManager->Start(streamInfo.sessionId);
    bool isMute = true;
    capturerManager->SetMute(isMute);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
    capturerManager->Flush(streamInfo.sessionId);
    capturerManager->Drain(streamInfo.sessionId);
    capturerManager->Stop(streamInfo.sessionId);
    capturerManager->OnNodeStatusUpdate(streamInfo.sessionId, OPERATION_STOPPED);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->IsRunning();

    capturerManager->Release(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);
    
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->IsRunning();
}

void HpaeCapturerManagerFuzzTest1()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);

    StateControlTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->OnNotifyQueue();
    capturerManager->DeInit();
}

void HpaeCapturerManagerFuzzTest2()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    StateControlFuzzTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->OnNotifyQueue();
    capturerManager->DeInit();
}

void HpaeCapturerManagerFuzzTest3()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);

    StateControlFuzzTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->DeInit();
    capturerManager->OnNotifyQueue();
}

void HpaeCapturerManagerReloadFuzzTest1()
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetThreadName();
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->ReloadCaptureManager(newSourceInfo);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->DeInit();
}

void HpaeCapturerManagerReloadFuzzTest2()
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->GetThreadName();
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->ReloadCaptureManager(newSourceInfo);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->DeInit();
}

void HpaeCapturerManagerReloadFuzzTest3()
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetThreadName();
    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->ReloadCaptureManager(newSourceInfo);
    WaitForMsgProcessing(capturerManager);
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->DeInit();
}

void MoveStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    std::string sourceName = sourceInfo.sourceName;
    capturerManager->MoveStream(sessionId, sourceName);
    std::vector<uint32_t> sessionIds = {GetData<uint32_t>(), GetData<uint32_t>(), GetData<uint32_t>()};
    capturerManager->MoveAllStream(sourceName, sessionIds);
    capturerManager->DeInit();
}

void GetSourceInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    capturerManager->GetSourceInfo();
    capturerManager->GetAllSourceOutputsInfo();
    capturerManager->DeInit();
}

void OnRequestLatencyFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = 0;
    capturerManager->OnRequestLatency(sessionId, latency);
    capturerManager->DeInit();
}

void AddNodeToSourceFuzzTest1()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    HpaeCaptureMoveInfo moveInfo;
    capturerManager->AddNodeToSource(moveInfo);
    capturerManager->DeInit();
}

void AddNodeToSourceFuzzTest2()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    HpaeCaptureMoveInfo moveInfo = GetHpaeCaptureMoveInfo();
    capturerManager->AddNodeToSource(moveInfo);
    capturerManager->DeInit();
}

void AddAllNodesToSourceFuzzTest1()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);
    capturerManager->DeInit();
}

void AddAllNodesToSourceFuzzTest2()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    std::vector<HpaeCaptureMoveInfo> moveInfos = {GetHpaeCaptureMoveInfo(),
        GetHpaeCaptureMoveInfo(), GetHpaeCaptureMoveInfo()};
    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);
    capturerManager->DeInit();
}

void GetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->Init();
    capturerManager->GetDeviceHDFDumpInfo();
    capturerManager->DeInit();
}

void CaptureEffectCreateFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    std::vector<uint32_t> sessionIds;

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.sessionId = GetData<uint32_t>();
    sessionIds.push_back(streamInfo.sessionId);

    capturerManager->CreateStream(streamInfo);

    for (uint32_t sessionId : sessionIds) {
        capturerManager->DestroyStream(sessionId);
    }

    capturerManager->DeInit();
}

void StartWithEcAndMicRefFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Stop(streamInfo.sessionId);
    capturerManager->Release(streamInfo.sessionId);
    capturerManager->DeInit();
}

void DeInitWithCapturerSourceStopForRemoteFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.deviceClass = "remote";
    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);
    capturerManager->Init();

    capturerManager->DeInit();
}

void PrepareCapturerEcFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();

    capturerManager->DeInit();
}

void DeactivateThreadFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    capturerManager->DeactivateThread();
}

void RegisterReadCallbackFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    auto callback = std::make_shared<DummyCapturerStreamCallback>();
    CHECK_AND_RETURN(callback != nullptr);
    capturerManager->RegisterReadCallback(streamInfo.sessionId, callback);
    WaitForMsgProcessing(capturerManager);

    uint32_t invalidSessionId = GetData<uint32_t>();
    capturerManager->RegisterReadCallback(invalidSessionId, callback);

    capturerManager->DeInit();
}

void CheckIfAnyStreamRunningFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceInfo newSourceInfo;
    InitSourceInfo(newSourceInfo);
    capturerManager->ReloadCaptureManager(newSourceInfo);

    capturerManager->DeInit();
}

void DumpSourceInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);
    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);
    capturerManager->DumpSourceInfo();
    capturerManager->DeInit();
}

void AddAllNodesToSourceAdvancedFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    std::vector<HpaeCaptureMoveInfo> moveInfos;

    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    nodeInfo.sceneType = GetData<HpaeProcessorType>();

    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = GetData<uint32_t>();
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);

    moveInfo.sessionInfo.state = GetData<HpaeSessionState>();

    moveInfos.push_back(moveInfo);

    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);

    capturerManager->DeInit();
}

void DisConnectSceneClusterFromSourceInputClusterFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Stop(streamInfo.sessionId);
    capturerManager->Release(streamInfo.sessionId);

    capturerManager->DeInit();
}

void ConnectOutputSessionFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = GetData<SourceType>();
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);

    capturerManager->DeInit();
}

void PauseFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Pause(streamInfo.sessionId);

    capturerManager->DeInit();
}

void SetStreamMuteFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    bool isMute1 = GetData<bool>();
    capturerManager->SetStreamMute(streamInfo.sessionId, isMute1);
    WaitForMsgProcessing(capturerManager);

    uint32_t invalidSessionId = GetData<uint32_t>();
    bool isMute2 = GetData<bool>();
    capturerManager->SetStreamMute(invalidSessionId, isMute2);
    capturerManager->DeInit();
}

void ReloadCaptureManagerFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->ReloadCaptureManager(sourceInfo);

    capturerManager->DeInit();
}

void DeInitFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    bool isMoveDefault = GetData<bool>();
    capturerManager->DeInit(isMoveDefault);
}

void GetSourceOutputInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = GetData<HpaeEcType>();
    sourceInfo.micRef = GetData<HpaeMicRefSwitch>();

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->Init();
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    capturerManager->DeInit();
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    HpaeCapturerManagerFuzzTest1,
    HpaeCapturerManagerFuzzTest2,
    HpaeCapturerManagerFuzzTest3,
    HpaeCapturerManagerReloadFuzzTest1,
    HpaeCapturerManagerReloadFuzzTest2,
    HpaeCapturerManagerReloadFuzzTest3,
    MoveStreamFuzzTest,
    GetSourceInfoFuzzTest,
    OnRequestLatencyFuzzTest,
    AddNodeToSourceFuzzTest1,
    AddNodeToSourceFuzzTest2,
    AddAllNodesToSourceFuzzTest1,
    AddAllNodesToSourceFuzzTest2,
    GetDeviceHDFDumpInfoFuzzTest,
    CaptureEffectCreateFuzzTest,
    StartWithEcAndMicRefFuzzTest,
    DeInitWithCapturerSourceStopForRemoteFuzzTest,
    PrepareCapturerEcFuzzTest,
    DeactivateThreadFuzzTest,
    RegisterReadCallbackFuzzTest,
    AddAllNodesToSourceAdvancedFuzzTest,
    CheckIfAnyStreamRunningFuzzTest,
    DumpSourceInfoFuzzTest,
    DisConnectSceneClusterFromSourceInputClusterFuzzTest,
    ConnectOutputSessionFuzzTest,
    PauseFuzzTest,
    SetStreamMuteFuzzTest,
    ReloadCaptureManagerFuzzTest,
    DeInitFuzzTest,
    GetSourceOutputInfoFuzzTest,
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