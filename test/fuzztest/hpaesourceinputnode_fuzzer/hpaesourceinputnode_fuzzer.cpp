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
#include "source/i_audio_capture_source.h"
#include "common/hdi_adapter_type.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
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
static vector<StreamManagerState> streamManagerStateMap = {
    STREAM_MANAGER_INVALID,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED,
};

static vector<HpaeSourceInputNodeType> hpaeSourceInputNodeTypeMap = {
    HPAE_SOURCE_DEFAULT,
    HPAE_SOURCE_MIC,
    HPAE_SOURCE_MIC_EC,
    HPAE_SOURCE_EC,
    HPAE_SOURCE_MICREF,
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
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->ResetAll();
}

void GetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetSharedInstance();
}

void GetOutputPortBufferTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetOutputPortBufferType(nodeInfo);
}

void GetCapturerSourceInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::string deviceClass = "file_io";
    std::string deviceNetId = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_MIC;
    std::string sourceName = "mic";
    hpaeSourceInputNode->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName);
}

void CapturerSourceInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    IAudioSourceAttr attr;
    hpaeSourceInputNode->CapturerSourceInit(attr);
}

void CapturerSourceDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceDeInit();
}

void CapturerSourceFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceFlush();
}

void CapturerSourcePauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourcePause();
}

void CapturerSourceResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceReset();
}

void CapturerSourceResumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceResume();
}

void CapturerSourceStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceStart();
}

void CapturerSourceStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->CapturerSourceStop();
}

void GetSourceStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetSourceState();
}

void SetSourceStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % streamManagerStateMap.size();
    StreamManagerState sourceState = streamManagerStateMap[index];
    hpaeSourceInputNode->SetSourceState(sourceState);
}

void WriteCapturerDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint32_t dataSize = static_cast<uint32_t>(GetData<uint16_t>()) + 1;
    std::vector<char> testData(dataSize);
    hpaeSourceInputNode->WriteCapturerData(testData.data(), dataSize);
}

void GetOutputPortNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetOutputPortNum();
}

void GetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetSourceInputNodeType();
}

void SetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % hpaeSourceInputNodeTypeMap.size();
    HpaeSourceInputNodeType type = hpaeSourceInputNodeTypeMap[index];
    hpaeSourceInputNode->SetSourceInputNodeType(type);
}

void GetNodeInfoWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    HpaeSourceBufferType type;
    hpaeSourceInputNode->GetNodeInfoWithInfo(type);
}

void UpdateAppsUidAndSessionIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::vector<int32_t> appsUid = {GetData<int32_t>()};
    std::vector<int32_t> sessionsId = {GetData<int32_t>()};
    hpaeSourceInputNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
}

void GetCaptureIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    auto hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceInputNode->GetCaptureId();
}

typedef void (*TestFuncs[23])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    GetSharedInstanceFuzzTest,
    GetOutputPortBufferTypeFuzzTest,
    GetCapturerSourceInstanceFuzzTest,
    CapturerSourceInitFuzzTest,
    CapturerSourceDeInitFuzzTest,
    CapturerSourceFlushFuzzTest,
    CapturerSourcePauseFuzzTest,
    CapturerSourceResetFuzzTest,
    CapturerSourceResumeFuzzTest,
    CapturerSourceStartFuzzTest,
    CapturerSourceStopFuzzTest,
    GetSourceStateFuzzTest,
    SetSourceStateFuzzTest,
    WriteCapturerDataFuzzTest,
    GetOutputPortNumFuzzTest,
    GetSourceInputNodeTypeFuzzTest,
    SetSourceInputNodeTypeFuzzTest,
    GetNodeInfoWithInfoFuzzTest,
    UpdateAppsUidAndSessionIdFuzzTest,
    GetCaptureIdFuzzTest,
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
