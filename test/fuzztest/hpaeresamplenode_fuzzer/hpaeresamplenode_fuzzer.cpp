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
#include "hpae_sink_input_node.h"
#include "hpae_resample_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_source_input_node.h"
#include <fstream>
#include <streambuf>
#include <string>
#include "test_case_common.h"
#include "audio_errors.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;


namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static constexpr uint32_t TEST_ID = 1243;
static constexpr uint32_t TEST_ID2 = 1246;
static constexpr uint32_t TEST_FRAMELEN1 = 960;
static constexpr uint32_t TEST_FRAMELEN2 = 640;
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
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
}

static void GetTestDtsNodeInfo(HpaeNodeInfo &nodeInfo)
{
    dstNodeInfo.nodeId = TEST_ID2;
    dstNodeInfo.frameLen = DEFAULT_FRAME_LENGTH2;
    dstNodeInfo.samplingRate = SAMPLE_RATE_44100;
    dstNodeInfo.channels = CHANNEL_4;
    dstNodeInfo.format = SAMPLE_F32LE;
}

void HpaeResampleNodeResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeNodeInfo dstNodeInfo;
    GetTestNodeInfo(nodeInfo);
    GetTestDtsNodeInfo(dstNodeInfo);
    auto hpaeResampleNode = std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo, ResamplerType::PRORESAMPLER);
    hpaeResampleNode->GetSampleRate();
    hpaeResampleNode->GetFrameLen();
    hpaeResampleNode->GetChannelCount();
    hpaeResampleNode->GetBitWidth();
    hpaeResampleNode->Reset();
}

void HpaeResampleNodeSignalProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    HpaeNodeInfo dstNodeInfo;
    GetTestNodeInfo(nodeInfo);
    GetTestDtsNodeInfo(dstNodeInfo);
    auto hpaeResampleNode = std::make_shared<HpaeResampleNode>(nodeInfo, dstNodeInfo);
    std::vector<HpaePcmBuffer *> inputs;
    hpaeResampleNode->SignalProcess(inputs);
}

void HpaeResampleNodeConnectAndDisconnectWithInfoFuzzTest()
{
    HpaeNodeInfo srcNodeInfo;
    HpaeNodeInfo dstNodeInfo;
    GetTestNodeInfo(srcNodeInfo);
    GetTestDtsNodeInfo(dstNodeInfo);
    auto hpaeResampleNode = std::make_shared<HpaeResampleNode>(srcNodeInfo, dstNodeInfo);
    auto hpaeInputNode = std::make_shared<HpaeSourceInputNode>(srcNodeInfo);
    hpaeResampleNode->ConnectWithInfo(hpaeInputNode, srcNodeInfo);
    hpaeResampleNode->DisConnectWithInfo(hpaeInputNode, srcNodeInfo);
}

typedef void (*TestFuncs[3])();

TestFuncs g_testFuncs = {
    HpaeResampleNodeResetFuzzTest,
    HpaeResampleNodeSignalProcessFuzzTest,
    HpaeResampleNodeConnectAndDisconnectWithInfoFuzzTest,
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
