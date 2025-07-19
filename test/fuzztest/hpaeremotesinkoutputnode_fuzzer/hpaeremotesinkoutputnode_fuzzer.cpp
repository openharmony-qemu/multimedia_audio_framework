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
#include "hpae_remote_sink_output_node.h"
#include "hpae_mixer_node.h"
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
static vector<StreamManagerState> streamManagerStateMap = {
    STREAM_MANAGER_INVALID,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED,
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
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->ResetAll();
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->Connect(hpaeMixerNode);
    hpaeRemoteSinkOutputNode->DisConnect(hpaeMixerNode);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->DisConnect(hpaeMixerNode);
}

void GetRenderSinkInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    std::string deviceClass = "123";
    std::string deviceNetId = "456";
    hpaeRemoteSinkOutputNode->GetRenderSinkInstance(deviceClass, deviceNetId);
}

void RenderSinkInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    IAudioSinkAttr attr;
    hpaeRemoteSinkOutputNode->RenderSinkInit(attr);
}

void RenderSinkDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkDeInit();
}

void RenderSinkFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkFlush();
}

void RenderSinkPauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkPause();
}

void RenderSinkResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkReset();
}

void RenderSinkResumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkResume();
}

void RenderSinkStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkStart();
}

void RenderSinkStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->RenderSinkStop();
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->GetPreOutNum();
}

void GetRenderFrameDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->GetRenderFrameData();
}

void GetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    hpaeRemoteSinkOutputNode->GetSinkState();
}

void SetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % streamManagerStateMap.size();
    StreamManagerState sinkState = streamManagerStateMap[index];
    hpaeRemoteSinkOutputNode->SetSinkState(sinkState);
}

void UpdateAppsUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeRemoteSinkOutputNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo);
    std::vector<int32_t> appsUid = {GetData<int32_t>()};
    hpaeRemoteSinkOutputNode->UpdateAppsUid(appsUid);
}


typedef void (*TestFuncs[19])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    ConnectFuzzTest,
    DisConnectFuzzTest,
    GetRenderSinkInstanceFuzzTest,
    RenderSinkInitFuzzTest,
    RenderSinkDeInitFuzzTest,
    RenderSinkFlushFuzzTest,
    RenderSinkPauseFuzzTest,
    RenderSinkResetFuzzTest,
    RenderSinkResumeFuzzTest,
    RenderSinkStartFuzzTest,
    RenderSinkStopFuzzTest,
    GetPreOutNumFuzzTest,
    GetRenderFrameDataFuzzTest,
    GetSinkStateFuzzTest,
    SetSinkStateFuzzTest,
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