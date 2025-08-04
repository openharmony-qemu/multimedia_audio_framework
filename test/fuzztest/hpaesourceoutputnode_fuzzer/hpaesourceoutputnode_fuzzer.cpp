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
#include "i_capturer_stream.h"
#include "hpae_source_input_node.h"
#include "hpae_source_output_node.h"
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
static vector<HpaeSessionState> sessionStateMap = {
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
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->ResetAll();
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceOutputNode->Connect(hpaeSourceInputNode);
    hpaeSourceOutputNode->DisConnect(hpaeSourceInputNode);
}

void ConnectWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceOutputNode->ConnectWithInfo(hpaeSourceInputNode, nodeInfo);
    hpaeSourceOutputNode->DisConnectWithInfo(hpaeSourceInputNode, nodeInfo);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceOutputNode->DisConnect(hpaeSourceInputNode);
}

void DisConnectWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceOutputNode->DisConnectWithInfo(hpaeSourceInputNode, nodeInfo);
}

void SetStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % sessionStateMap.size();
    HpaeSessionState captureState = sessionStateMap[index];
    hpaeSourceOutputNode->SetState(captureState);
}

void GetStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->GetState();
}

void SetAppUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    int32_t appUid = GetData<int32_t>();
    hpaeSourceOutputNode->SetAppUid(appUid);
}

void GetAppUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode->GetAppUid();
}

typedef void (*TestFuncs[11])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    ConnectFuzzTest,
    ConnectWithInfoFuzzTest,
    DisConnectFuzzTest,
    DisConnectWithInfoFuzzTest,
    SetStateFuzzTest,
    GetStateFuzzTest,
    SetAppUidFuzzTest,
    GetAppUidFuzzTest,
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
