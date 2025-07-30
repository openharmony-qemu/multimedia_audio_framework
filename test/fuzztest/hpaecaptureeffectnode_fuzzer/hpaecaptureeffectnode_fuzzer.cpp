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
#include <cstring>
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "hpae_capture_effect_node.h"
#include "hpae_source_input_cluster.h"
#include "hpae_format_convert.h"
#include "audio_effect_log.h"

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
constexpr uint8_t TESTSIZE = 2;

const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_NODE_ID = 1243;

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

void ConnectWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    hpaeCaptureEffectNode->ConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
    hpaeCaptureEffectNode->DisConnectWithInfo(hpaeSourceInputCluster, nodeInfo);
}


void CaptureEffectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    hpaeCaptureEffectNode->GetCapturerEffectConfig(nodeInfo);
    CaptureEffectAttr attr = {
        .micChannels = GetData<uint32_t>(),
        .ecChannels = GetData<uint32_t>(),
        .micRefChannels = GetData<uint32_t>(),
    };
    uint64_t sceneKeyCode = GetData<uint64_t>();
    hpaeCaptureEffectNode->CaptureEffectCreate(sceneKeyCode, attr);
    hpaeCaptureEffectNode->CaptureEffectRelease(sceneKeyCode);
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[TESTSIZE] = {
    ConnectWithInfoFuzzTest,
    CaptureEffectFuzzTest,
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
