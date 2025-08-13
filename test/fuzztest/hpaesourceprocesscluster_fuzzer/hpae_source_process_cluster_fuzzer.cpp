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
#include "hpae_source_process_cluster.h"
#include "hpae_source_input_cluster.h"
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
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

void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->ResetAll();
}

void GetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetSharedInstance();
}

void GetSharedInstanceWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetSharedInstance(nodeInfo);
}

void GetOutputPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetOutputPort();
}

void GetOutputPortWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetOutputPort(nodeInfo);
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeSourceProcessCluster->Connect(hpaeSourceInputCluster);
    hpaeSourceProcessCluster->DisConnect(hpaeSourceInputCluster);
}

void ConnectWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeSourceProcessCluster->ConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
    hpaeSourceProcessCluster->DisConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeSourceProcessCluster->DisConnect(hpaeSourceInputCluster);
}

void DisConnectWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeSourceProcessCluster->DisConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
}

void GetCapturerEffectConfigFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetCapturerEffectConfig(nodeInfo);
}

void GetOutputPortNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetOutputPortNum();
}

void CaptureEffectCreateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    uint64_t sceneKeyCode = GetData<uint64_t>();
    CaptureEffectAttr attr;
    hpaeSourceProcessCluster->CaptureEffectCreate(sceneKeyCode, attr);
}

void CaptureEffectReleaseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    uint64_t sceneKeyCode = GetData<uint64_t>();
    hpaeSourceProcessCluster->CaptureEffectRelease(sceneKeyCode);
}

void GetCapturerEffectNodeUseCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetCapturerEffectNodeUseCount();
}

void GetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetConverterNodeCount();
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceProcessCluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    hpaeSourceProcessCluster->GetPreOutNum();
}

typedef void (*TestFuncs[18])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    GetSharedInstanceFuzzTest,
    GetSharedInstanceWithNodeInfoFuzzTest,
    GetOutputPortFuzzTest,
    GetOutputPortWithNodeInfoFuzzTest,
    ConnectFuzzTest,
    ConnectWithNodeInfoFuzzTest,
    DisConnectFuzzTest,
    DisConnectWithNodeInfoFuzzTest,
    GetCapturerEffectConfigFuzzTest,
    GetOutputPortNumFuzzTest,
    CaptureEffectCreateFuzzTest,
    CaptureEffectReleaseFuzzTest,
    GetCapturerEffectNodeUseCountFuzzTest,
    GetConverterNodeCountFuzzTest,
    GetPreOutNumFuzzTest,
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