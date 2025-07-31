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
constexpr uint8_t TESTSIZE = 28;

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
    std::shared_ptr<IHpaeRendererManager> rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    string deviceName = "";
    rendererManager->UploadDumpSinkInfo(deviceName);
}

void OnNotifyDfxNodeInfoFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isConnect = false;
    uint32_t preNodeId = GetData<uint32_t>();
    HpaeDfxNodeInfo nodeInfo = {};
    rendererManager->OnNotifyDfxNodeInfo(isConnect, preNodeId, nodeInfo);
}

void HpaeRendererManagerConstructFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    HpaeRendererManager rendererManager(sinkInfo);
}

void HpaeRendererManagerCreateStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    HpaeStreamInfo streamInfo;
    InitRenderStreamInfo(streamInfo);
    rendererManager->CreateStream(streamInfo);
}

void HpaeRendererManagerDestroyStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->DestroyStream(sessionId);
}

void HpaeRendererManagerStartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Start(sessionId);
}

void HpaeRendererManagerPauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Pause(sessionId);
}

void HpaeRendererManagerFlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Flush(sessionId);
}

void HpaeRendererManagerDrainFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Drain(sessionId);
}

void HpaeRendererManagerStopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Stop(sessionId);
}

void HpaeRendererManagerReleaseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    rendererManager->Release(sessionId);
}

void HpaeRendererManagerMoveStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    string sinkName = "";
    rendererManager->MoveStream(sessionId, sinkName);
}

void HpaeRendererManagerMoveAllStreamFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    string sinkName = "";
    vector<uint32_t> sessionIds;
    MoveSessionType moveSessionType = MOVE_ALL;
    rendererManager->MoveAllStream(sinkName, sessionIds, moveSessionType);
}

void HpaeRendererManagerSuspendStreamManagerFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isSuspend = false;
    rendererManager->SuspendStreamManager(isSuspend);
}

void HpaeRendererManagerSetMuteFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isMute = false;
    rendererManager->SetMute(isMute);
}

void HpaeRendererManagerProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Process();
}

void HpaeRendererManagerHandleMsgFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->HandleMsg();
}

void HpaeRendererManagerInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->Init();
}

void HpaeRendererManagerDeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    bool isMoveDefault = false;
    rendererManager->DeInit(isMoveDefault);
}

void HpaeRendererManagerIsInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->IsInit();
}

void HpaeRendererManagerIsRunningFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->IsRunning();
}

void HpaeRendererManagerIsMsgProcessingFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->IsMsgProcessing();
}

void HpaeRendererManagerDeactivateThreadFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    rendererManager->DeactivateThread();
}

void HpaeRendererManagerSetClientVolumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    float volume = GetData<float>();
    rendererManager->SetClientVolume(sessionId, volume);
}

void HpaeRendererManagerSetRateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t rate = GetData<int32_t>();
    rendererManager->SetRate(sessionId, rate);
}

void HpaeRendererManagerSetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    rendererManager->SetAudioEffectMode(sessionId, effectMode);
}

void HpaeRendererManagerGetAudioEffectModeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    InitHpaeSinkInfo(sinkInfo);
    auto rendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    int32_t effectMode = GetData<int32_t>();
    rendererManager->GetAudioEffectMode(sessionId, effectMode);
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[TESTSIZE] = {
    CreateRendererManagerFuzzTest,
    UploadDumpSinkInfoFuzzTest,
    OnNotifyDfxNodeInfoFuzzTest,
    HpaeRendererManagerConstructFuzzTest,
    HpaeRendererManagerCreateStreamFuzzTest,
    HpaeRendererManagerDestroyStreamFuzzTest,
    HpaeRendererManagerStartFuzzTest,
    HpaeRendererManagerPauseFuzzTest,
    HpaeRendererManagerFlushFuzzTest,
    HpaeRendererManagerDrainFuzzTest,
    HpaeRendererManagerStopFuzzTest,
    HpaeRendererManagerReleaseFuzzTest,
    HpaeRendererManagerMoveStreamFuzzTest,
    HpaeRendererManagerMoveAllStreamFuzzTest,
    HpaeRendererManagerSuspendStreamManagerFuzzTest,
    HpaeRendererManagerSetMuteFuzzTest,
    HpaeRendererManagerProcessFuzzTest,
    HpaeRendererManagerHandleMsgFuzzTest,
    HpaeRendererManagerInitFuzzTest,
    HpaeRendererManagerDeInitFuzzTest,
    HpaeRendererManagerIsInitFuzzTest,
    HpaeRendererManagerIsRunningFuzzTest,
    HpaeRendererManagerIsMsgProcessingFuzzTest,
    HpaeRendererManagerDeactivateThreadFuzzTest,
    HpaeRendererManagerSetClientVolumeFuzzTest,
    HpaeRendererManagerSetRateFuzzTest,
    HpaeRendererManagerSetAudioEffectModeFuzzTest,
    HpaeRendererManagerGetAudioEffectModeFuzzTest,
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
