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
#include "hpae_mixer_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_audio_format_converter_node.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;


namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *,size_t);

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


void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->ResetAll();
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> inputs;
    const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode = inputs;
    hpaeOutputCluster->Connect(preNode);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> inputs;
    const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode = inputs;
    hpaeOutputCluster->DisConnect(preNode);
}

void GetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->GetConverterNodeCount();
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->GetPreOutNum();
}

void GetInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    std::string deviceClass = "";
    std::string deviceNetId = "";
    hpaeOutputCluster->GetInstance(deviceClass,deviceNetId);
}

void InitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    IAudioSinkAttr attr;
    hpaeOutputCluster->Init(attr);
}

void DeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->DeInit();
}

void FlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Flush();
}

void PauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Pause();
}

void ResetRenderFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->ResetRender();
}

void ResumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Resume();
}

void StartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Start();
}

void StopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->Stop();
}

void SetTimeoutStopThdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    uint32_t timeoutThdMs = GetData<uint32_t>();
    hpaeOutputCluster->SetTimeoutStopThd();
}

void GetStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->GetState();
}

void IsProcessClusterConnectedFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    HpaeProcessorType sceneType;
    hpaeOutputCluster->IsProcessClusterConnected(sceneType);
}

void UpdateAppsUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    const std::vector<int32_t> &appsUid = GetData<uint32_t>();
    hpaeOutputCluster->UpdateAppsUid(appsUid);
}

void SetPriPaPowerFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->SetPriPaPower();
}

void SetSyncIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    int32_t syncId = GetData<int32_t>();
    hpaeOutputCluster->SetSyncId(syncId);
}

void GetLatencyFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeOutputCluster->GetLatency();
}

typedef void (*TestFuncs[23])();

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
    GetStateFuzzTest,
    IsProcessClusterConnectedFuzzTest,
    UpdateAppsUidFuzzTest,
    SetPriPaPowerFuzzTest,
    SetSyncIdFuzzTest,
    GetLatencyFuzzTest,
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
