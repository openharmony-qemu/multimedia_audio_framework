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
#include <thread>
#include <chrono>
#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_ec_info.h"
#include "i_stream.h"
#include "hpae_capturer_manager.h"
#include "hpae_source_output_node.h"
#include "hpaecapturermanager_fuzzer.h"
#include "audio_engine_log.h"
#include "securec.h"
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
constexpr int32_t MAX_MSG_PROCESSING_RETRY_COUNT = 50;
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

const std::vector<HpaeProcessorType> VALID_PROCESSOR_TYPES {
    HPAE_SCENE_DEFAULT,
    HPAE_SCENE_MUSIC,
    HPAE_SCENE_GAME,
    HPAE_SCENE_MOVIE,
    HPAE_SCENE_SPEECH,
    HPAE_SCENE_RING,
    HPAE_SCENE_VOIP_DOWN,
    HPAE_SCENE_OTHERS,
    HPAE_SCENE_EFFECT_NONE,
    HPAE_SCENE_EFFECT_OUT,
    HPAE_SCENE_SPLIT_MEDIA,
    HPAE_SCENE_SPLIT_NAVIGATION,
    HPAE_SCENE_SPLIT_COMMUNICATION,
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_RECORD,
    HPAE_SCENE_PRE_ENHANCE,
    HPAE_SCENE_ASR,
    HPAE_SCENE_VOICE_MESSAGE,
    HPAE_SCENE_COLLABORATIVE,
    HPAE_SCENE_RECOGNITION,
};

const std::vector<HpaeEcType> VALID_EC_TYPES {
    HPAE_EC_TYPE_NONE,
    HPAE_EC_TYPE_SAME_ADAPTER,
    HPAE_EC_TYPE_DIFF_ADAPTER,
};

const std::vector<HpaeMicRefSwitch> VALID_MIC_REF_SWITCHES {
    HPAE_REF_OFF,
    HPAE_REF_ON,
};

const std::vector<SourceType> VALID_SOURCE_TYPES {
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_OFFLOAD_CAPTURE,
    SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT,
    SOURCE_TYPE_INTERPHONE,
};

const std::vector<HpaeSessionState> VALID_SESSION_STATES {
    HPAE_SESSION_INVALID,
    HPAE_SESSION_NEW,
    HPAE_SESSION_PREPARED,
    HPAE_SESSION_RUNNING,
    HPAE_SESSION_PAUSING,
    HPAE_SESSION_PAUSED,
    HPAE_SESSION_STOPPING,
    HPAE_SESSION_STOPPED,
    HPAE_SESSION_RELEASED,
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
    RoundVal(sourceInfo.ecType, VALID_EC_TYPES);
    RoundVal(sourceInfo.micRef, VALID_MIC_REF_SWITCHES);
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

void InitStreamInfo(HpaeStreamInfo &streamInfo)
{
    RoundStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.deviceName = "Built_in_mic";
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
}

// forward declaration
static void WaitForMsgProcessing(std::shared_ptr<HpaeCapturerManager> &capturerManager);

// 进程级共享 capturerManager：整个进程只 Init 一次。原先每用例 Init/DeInit 每轮 spawn
// 一个 detached QoS 线程(SetThreadQosLevelAsync)，进程退出时撞 ConcurrentTaskClient
// 静态析构 heap-use-after-free；共享后全局只 1 个，Init 后毫秒级结束。与
// hpaerenderermanageradd_fuzzer 的 g_sharedRendererMgr 同配方：mgr 存储在文件级
// g_sharedCapturerMgr，守卫为函数级 static（注册必然晚于文件级存储构造，逆序析构时
// 守卫先跑），守卫直读 g_sharedCapturerMgr，绝不走本函数的再初始化分支。
// 例外：DeactivateThreadFuzzTest / DeInitFuzzTest /
// DeInitWithCapturerSourceStopForRemoteFuzzTest 专门 fuzz 拆除本身，保留局部自建自拆。
static std::shared_ptr<HpaeCapturerManager> g_sharedCapturerMgr;

class SharedCapturerManagerCleanupGuard {
public:
    ~SharedCapturerManagerCleanupGuard()
    {
        // Read g_sharedCapturerMgr DIRECTLY -- never via GetSharedCapturerManager().
        // Re-entering the create+Init branch at static-destruction time would spawn
        // worker threads (and consume fuzz input through GetData) during exit.
        if (g_sharedCapturerMgr != nullptr) {
            g_sharedCapturerMgr->DeactivateThread();
        }
    }
};

std::shared_ptr<HpaeCapturerManager> &GetSharedCapturerManager()
{
    if (g_sharedCapturerMgr == nullptr || !g_sharedCapturerMgr->IsInit()) {
        // Stop the previous (failed or DeInit'd) manager's worker before replacing it:
        // Init() spawns the worker unconditionally, so a re-entry would otherwise
        // leak one parked worker per call.
        if (g_sharedCapturerMgr != nullptr) {
            g_sharedCapturerMgr->DeactivateThread();
        }
        HpaeSourceInfo sourceInfo;
        InitSourceInfo(sourceInfo);
        g_sharedCapturerMgr = std::make_shared<HpaeCapturerManager>(sourceInfo);
        g_sharedCapturerMgr->Init();
        WaitForMsgProcessing(g_sharedCapturerMgr);
    }
    static SharedCapturerManagerCleanupGuard guard;
    return g_sharedCapturerMgr;
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
    int retry = 0;
    while (capturerManager->IsMsgProcessing() && retry < MAX_MSG_PROCESSING_RETRY_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 20 for sleep
        retry++;
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
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);

    StateControlTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->OnNotifyQueue();
}

void HpaeCapturerManagerFuzzTest2()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
    StateControlFuzzTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->OnNotifyQueue();
}

void HpaeCapturerManagerFuzzTest3()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->IsInit();
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);

    StateControlFuzzTest(capturerManager, streamInfo, sourceOutputInfo);
    capturerManager->OnNotifyQueue();
}

void HpaeCapturerManagerReloadFuzzTest1()
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);
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
}

// 保留局部 manager：本用例专门 fuzz 未 Init 管理器上的 reload（不建 worker/不派 QoS 线程，无 exit 风险），不入共享。
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

    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);
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
}

void MoveStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);
    uint32_t sessionId = GetData<uint32_t>();
    std::string sourceName = sourceInfo.sourceName;
    capturerManager->MoveStream(sessionId, sourceName);
    std::vector<uint32_t> sessionIds = {GetData<uint32_t>(), GetData<uint32_t>(), GetData<uint32_t>()};
    capturerManager->MoveAllStream(sourceName, sessionIds);
}

void GetSourceInfoFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->GetSourceInfo();
    capturerManager->GetAllSourceOutputsInfo();
}

void OnRequestLatencyFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    uint32_t sessionId = GetData<uint32_t>();
    uint64_t latency = 0;
    capturerManager->OnRequestLatency(sessionId, latency);
}

void AddNodeToSourceFuzzTest1()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeCaptureMoveInfo moveInfo;
    capturerManager->AddNodeToSource(moveInfo);
}

void AddNodeToSourceFuzzTest2()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeCaptureMoveInfo moveInfo = GetHpaeCaptureMoveInfo();
    capturerManager->AddNodeToSource(moveInfo);
}

void AddAllNodesToSourceFuzzTest1()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    std::vector<HpaeCaptureMoveInfo> moveInfos;
    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);
}

void AddAllNodesToSourceFuzzTest2()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    std::vector<HpaeCaptureMoveInfo> moveInfos = {GetHpaeCaptureMoveInfo(),
        GetHpaeCaptureMoveInfo(), GetHpaeCaptureMoveInfo()};
    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);
}

void GetDeviceHDFDumpInfoFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->GetDeviceHDFDumpInfo();
}

void CaptureEffectCreateFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    std::vector<uint32_t> sessionIds;

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    RoundVal(streamInfo.sourceType, VALID_SOURCE_TYPES);
    streamInfo.sessionId = GetData<uint32_t>();
    sessionIds.push_back(streamInfo.sessionId);

    capturerManager->CreateStream(streamInfo);

    for (uint32_t sessionId : sessionIds) {
        capturerManager->DestroyStream(sessionId);
    }
}

void StartWithEcAndMicRefFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    RoundVal(streamInfo.sourceType, VALID_SOURCE_TYPES);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Stop(streamInfo.sessionId);
    capturerManager->Release(streamInfo.sessionId);
}

// 保留局部 manager 自建自拆：本用例专门 fuzz 拆除本身，不入共享（见 GetSharedCapturerManager 注释）。
void DeInitWithCapturerSourceStopForRemoteFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    sourceInfo.deviceClass = "remote";
    RoundVal(sourceInfo.ecType, VALID_EC_TYPES);
    RoundVal(sourceInfo.micRef, VALID_MIC_REF_SWITCHES);

    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    CHECK_AND_RETURN(capturerManager != nullptr);
    capturerManager->Init();

    capturerManager->DeInit();
}

void PrepareCapturerEcFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);
}

// 保留局部 manager 自建自拆：本用例专门 fuzz 拆除本身，不入共享（见 GetSharedCapturerManager 注释）。
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
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

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
}

void CheckIfAnyStreamRunningFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

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
}

void DumpSourceInfoFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    capturerManager->DumpSourceInfo();
}

void AddAllNodesToSourceAdvancedFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    std::vector<HpaeCaptureMoveInfo> moveInfos;

    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    RoundVal(nodeInfo.sceneType, VALID_PROCESSOR_TYPES);

    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = GetData<uint32_t>();
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);

    RoundVal(moveInfo.sessionInfo.state, VALID_SESSION_STATES);

    moveInfos.push_back(moveInfo);

    bool isConnect = GetData<bool>();
    capturerManager->AddAllNodesToSource(moveInfos, isConnect);
}

void DisConnectSceneClusterFromSourceInputClusterFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    RoundVal(streamInfo.sourceType, VALID_SOURCE_TYPES);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Stop(streamInfo.sessionId);
    capturerManager->Release(streamInfo.sessionId);
}

void ConnectOutputSessionFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    RoundVal(streamInfo.sourceType, VALID_SOURCE_TYPES);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
}

void PauseFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);

    capturerManager->Pause(streamInfo.sessionId);
}

void SetStreamMuteFuzzTest()
{
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

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
}

void ReloadCaptureManagerFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    capturerManager->ReloadCaptureManager(sourceInfo);
}

// 保留局部 manager 自建自拆：本用例专门 fuzz 拆除本身，不入共享（见 GetSharedCapturerManager 注释）。
void DeInitFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    RoundVal(sourceInfo.ecType, VALID_EC_TYPES);
    RoundVal(sourceInfo.micRef, VALID_MIC_REF_SWITCHES);

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
    auto &capturerManager = GetSharedCapturerManager();
    CHECK_AND_RETURN(capturerManager != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = GetData<uint32_t>();

    capturerManager->CreateStream(streamInfo);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceOutputInfo sourceOutputInfo;
    capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo);
}

void MicRefSourceInitFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
}

void EcDiffAdapterFullFlowFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
}

void DeleteOutputSessionWithActiveClusterFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void MoveStreamWithValidSessionFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->MoveStream(streamInfo.sessionId, "test_source");
    WaitForMsgProcessing(mgr);
}

void DisConnectSceneClusterFullFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Release(streamInfo.sessionId);
}

void ConnectOutputSessionWithEcMicRefFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void AddSingleNodeRunningStateFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
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
    moveInfo.sessionInfo.state = HPAE_SESSION_RUNNING;
    bool isConnect = GetData<bool>();
    mgr->AddAllNodesToSource({moveInfo}, isConnect);
}

void EcSameAdapterFullFlowFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void EcDiffAdapterWithMicRefFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void ConnectOutputSessionSceneRecordFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void DisConnectOutputSessionFullFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Pause(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void ReloadCaptureManagerRunningFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);

    HpaeSourceInfo newSourceInfo;
    newSourceInfo.deviceNetId = "LocalDevice";
    newSourceInfo.deviceClass = "file_io";
    newSourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    newSourceInfo.samplingRate = SAMPLE_RATE_48000;
    newSourceInfo.channels = STEREO;
    RoundSourceInfo(newSourceInfo);

    mgr->ReloadCaptureManager(newSourceInfo);
    WaitForMsgProcessing(mgr);
}

void MultiStreamCapturerFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo1;
    InitReloadStreamInfo(streamInfo1);
    streamInfo1.sessionId = GetData<uint32_t>();
    mgr->CreateStream(streamInfo1);
    WaitForMsgProcessing(mgr);

    HpaeStreamInfo streamInfo2;
    InitReloadStreamInfo(streamInfo2);
    streamInfo2.sessionId = GetData<uint32_t>();
    mgr->CreateStream(streamInfo2);
    WaitForMsgProcessing(mgr);

    mgr->Start(streamInfo1.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo2.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->Stop(streamInfo1.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo1.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->Stop(streamInfo2.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo2.sessionId);
    WaitForMsgProcessing(mgr);
}

void CapturerSourceStartStopCycleFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);

    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void FlushDrainCapturerFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->Flush(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->Drain(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);

    mgr->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
}

void DeleteOutputSessionFullFuzzTest()
{
    auto &mgr = GetSharedCapturerManager();
    CHECK_AND_RETURN(mgr != nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    mgr->CreateStream(streamInfo);
    WaitForMsgProcessing(mgr);
    mgr->Start(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
    mgr->DestroyStream(streamInfo.sessionId);
    WaitForMsgProcessing(mgr);
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
    MicRefSourceInitFuzzTest,
    EcDiffAdapterFullFlowFuzzTest,
    DeleteOutputSessionWithActiveClusterFuzzTest,
    MoveStreamWithValidSessionFuzzTest,
    DisConnectSceneClusterFullFuzzTest,
    ConnectOutputSessionWithEcMicRefFuzzTest,
    AddSingleNodeRunningStateFuzzTest,
    EcSameAdapterFullFlowFuzzTest,
    EcDiffAdapterWithMicRefFuzzTest,
    ConnectOutputSessionSceneRecordFuzzTest,
    DisConnectOutputSessionFullFuzzTest,
    ReloadCaptureManagerRunningFuzzTest,
    MultiStreamCapturerFuzzTest,
    CapturerSourceStartStopCycleFuzzTest,
    FlushDrainCapturerFuzzTest,
    DeleteOutputSessionFullFuzzTest,
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