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
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "high_resolution_timer.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"

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
static vector<StreamManagerState> streamManagerStateMap = {
    STREAM_MANAGER_INVALID,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED,
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


void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->ResetAll();
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->Connect(hpaeSinkInputNode);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkOutputNode->DisConnect(hpaeSinkInputNode);
}

void GetRenderSinkInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::string deviceClass = "123";
    std::string deviceNetId = "456";
    hpaeSinkOutputNode->GetRenderSinkInstance(deviceClass,deviceNetId);
}

void RenderSinkInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    IAudioSinkAttr attr;
    hpaeSinkOutputNode->RenderSinkInit(attr);
}

void RenderSinkDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkDeInit();
}

void RenderSinkFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkFlush();
}

void RenderSinkPauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkPause();
}
void RenderSinkResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkReset();
}
void RenderSinkResumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkResume();
}

void RenderSinkStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkStart();
}

void RenderSinkStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkStop();
}

void RenderSinkSetPriPaPowerFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->RenderSinkSetPriPaPower();
}

void RenderSinkSetSyncIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    int32_t syncId = GetData<int32_t>();
    hpaeSinkOutputNode->RenderSinkSetSyncId(syncId);
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->GetPreOutNum();
}

void GetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->GetSinkState();
}

void SetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % streamManagerStateMap.size();
    StreamManagerState sinkState = streamManagerStateMap[index];
    hpaeSinkOutputNode->SetSinkState(sinkState);
}

void UpdateAppsUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    std::vector<int32_t> appsUid = {GetData<int32_t>()};
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
}

void GetLatencyFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    hpaeSinkOutputNode->GetLatency();
}

typedef void (*TestFuncs[21])();

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
    RenderSinkSetPriPaPowerFuzzTest,
    RenderSinkSetSyncIdFuzzTest,
    GetPreOutNumFuzzTest,
    GetSinkStateFuzzTest,
    SetSinkStateFuzzTest,
    UpdateAppsUidFuzzTest,
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
