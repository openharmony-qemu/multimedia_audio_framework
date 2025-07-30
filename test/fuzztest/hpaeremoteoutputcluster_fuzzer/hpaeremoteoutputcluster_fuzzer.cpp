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
#include "hpae_process_cluster.h"
#include "audio_errors.h"
#include "hpae_sink_input_node.h"
#include "hpae_remote_output_cluster.h"
#include "hpae_mixer_node.h"
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
const uint32_t DEFAULT_NODE_ID = 1243;
static vector<HpaeProcessorType> hpaeProcessorTypeMap = {
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
};

typedef void (*TestPtr)(const uint8_t *, size_t);

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

static void GetTestNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
}

void DoProcessFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->DoProcess();
}

void ResetFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Reset();
}

void ResetAllFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->ResetAll();
}

void ConnectFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    hpaeRemoteOutputCluster->Connect(hpaeMixerNode);
    hpaeRemoteOutputCluster->DisConnect(hpaeMixerNode);
}

void DisConnectFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    hpaeRemoteOutputCluster->DisConnect(hpaeMixerNode);
}

void GetConverterNodeCountFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->GetConverterNodeCount();
}

void GetPreOutNumFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->GetPreOutNum();
}

void GetInstanceFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    std::string deviceClass = "123";
    std::string deviceNetId = "456";
    hpaeRemoteOutputCluster->GetInstance(deviceClass, deviceNetId);
}

void InitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    IAudioSinkAttr attr;
    hpaeRemoteOutputCluster->Init(attr);
}

void DeInitFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->DeInit();
}

void FlushFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Flush();
}

void PauseFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Pause();
}

void ResetRenderFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->ResetRender();
}

void ResumeFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Resume();
}

void StartFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Start();
}

void StopFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->Stop();
}

void SetTimeoutStopThdFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    uint32_t timeoutThdMs = GetData<uint32_t>();
    hpaeRemoteOutputCluster->SetTimeoutStopThd(timeoutThdMs);
}

void GetFrameDataFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->GetFrameData();
}

void GetStateFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    hpaeRemoteOutputCluster->GetState();
}

void IsProcessClusterConnectedFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    uint32_t index = GetData<uint32_t>() % hpaeProcessorTypeMap.size();
    HpaeProcessorType sceneType = hpaeProcessorTypeMap[index];
    hpaeRemoteOutputCluster->IsProcessClusterConnected(sceneType);
}

void UpdateAppsUidFuzzTest()
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteOutputCluster = std::make_shared<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo);
    std::vector<int32_t> appsUid = {GetData<int32_t>()};
    hpaeRemoteOutputCluster->UpdateAppsUid(appsUid);
}

typedef void (*TestFuncs[21])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    ConnectFuzzTest,
    DisConnectFuzzTest,
    GetConverterNodeCountFuzzTest,
    GetPreOutNumFuzzTest,
    GetInstanceFuzzTest,
    InitFuzzTest,
    DeInitFuzzTest,
    FlushFuzzTest,
    PauseFuzzTest,
    ResetRenderFuzzTest,
    ResumeFuzzTest,
    StartFuzzTest,
    StopFuzzTest,
    SetTimeoutStopThdFuzzTest,
    GetFrameDataFuzzTest,
    GetStateFuzzTest,
    IsProcessClusterConnectedFuzzTest,
    UpdateAppsUidFuzzTest,
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
