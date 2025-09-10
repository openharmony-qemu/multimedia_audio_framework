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
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "audio_effect.h"
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
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
}

static void CreateHpaeInfo(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &dummySinkInfo)
{
    GetTestNodeInfo(nodeInfo);
    dummySinkInfo.channels = STEREO;
    dummySinkInfo.frameLen = DEFAULT_FRAME_LENGTH;
    dummySinkInfo.format = SAMPLE_F32LE;
    dummySinkInfo.samplingRate = SAMPLE_RATE_48000;
}

void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->ResetAll();
}

void GetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetSharedInstance();
}

void GetOutputPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetOutputPort();
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
    hpaeProcessCluster->DisConnect(hpaeSinkInputNode);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeProcessCluster->DisConnect(hpaeSinkInputNode);
}

void GetGainNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetGainNodeCount();
}

void GetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetConverterNodeCount();
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetPreOutNum();
}

void AudioRendererCreateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->AudioRendererCreate(nodeInfo);
}

void AudioRendererStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->AudioRendererStart(nodeInfo);
}

void AudioRendererStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->AudioRendererStop(nodeInfo);
}

void AudioRendererReleaseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->AudioRendererRelease(nodeInfo);
}

void GetNodeInputFormatInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    AudioBasicFormat basicFormat;
    hpaeProcessCluster->GetNodeInputFormatInfo(sessionId, basicFormat);
}

void GetGainNodeByIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    uint32_t id = GetData<uint32_t>();
    hpaeProcessCluster->GetGainNodeById(id);
}

void GetConverterNodeByIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    uint32_t id = GetData<uint32_t>();
    hpaeProcessCluster->GetConverterNodeById(id);
}

void SetConnectedFlagFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->SetConnectedFlag(true);
}

void GetConnectedFlagFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->GetConnectedFlag();
}

void SetupAudioLimiterFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->SetupAudioLimiter();
}

void SetLoudnessGainFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    float loudnessGain = GetData<float>();
    hpaeProcessCluster->SetLoudnessGain(sessionId, loudnessGain);
}

void DisConnectMixerNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    hpaeProcessCluster->DisConnectMixerNode();
}

void InitEffectBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    CreateHpaeInfo(nodeInfo, dummySinkInfo);
    auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    uint32_t sessionId = GetData<uint32_t>();
    hpaeProcessCluster->InitEffectBuffer(sessionId);
}

typedef void (*TestFuncs[23])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    GetSharedInstanceFuzzTest,
    GetOutputPortFuzzTest,
    ConnectFuzzTest,
    DisConnectFuzzTest,
    GetGainNodeCountFuzzTest,
    GetConverterNodeCountFuzzTest,
    GetPreOutNumFuzzTest,
    AudioRendererCreateFuzzTest,
    AudioRendererStartFuzzTest,
    AudioRendererStopFuzzTest,
    AudioRendererReleaseFuzzTest,
    GetNodeInputFormatInfoFuzzTest,
    GetGainNodeByIdFuzzTest,
    GetConverterNodeByIdFuzzTest,
    SetConnectedFlagFuzzTest,
    GetConnectedFlagFuzzTest,
    SetupAudioLimiterFuzzTest,
    SetLoudnessGainFuzzTest,
    DisConnectMixerNodeFuzzTest,
    InitEffectBufferFuzzTest,
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
