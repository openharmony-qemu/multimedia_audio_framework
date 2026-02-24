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
#include <string>
#include <map>
#include <securec.h>
#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "hpae_source_process_cluster.h"
#include "hpae_source_input_node.h"
#include "hpae_info.h"
#include "hpae_define.h"
#include "audio_engine_log.h"
#include "hpaesourceprocesscluster_fuzzer.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

const size_t FUZZ_INPUT_THRESHOLD_SIZE = 10;

namespace OHOS {
namespace AudioStandard {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;

const uint32_t DEFAULT_FRAME_LENGTH = 960;

// Loop constants
constexpr int32_t MAX_SHORT_ITERATIONS = 5;
constexpr int32_t MAX_TEN_ITERATIONS = 10;

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_96000,
};

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const std::vector<HpaeProcessorType> SUPPORTED_SCENE_TYPES {
    HPAE_SCENE_DEFAULT,
    HPAE_SCENE_MUSIC,
    HPAE_SCENE_GAME,
    HPAE_SCENE_MOVIE,
    HPAE_SCENE_SPEECH,
    HPAE_SCENE_RING,
    HPAE_SCENE_VOIP_DOWN,
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_RECORD,
};

const std::vector<HpaeSourceBufferType> SUPPORTED_BUFFER_TYPES {
    HPAE_SOURCE_BUFFER_TYPE_DEFAULT,
    HPAE_SOURCE_BUFFER_TYPE_MIC,
    HPAE_SOURCE_BUFFER_TYPE_EC,
    HPAE_SOURCE_BUFFER_TYPE_MICREF,
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

template<class T>
void RoundVal(T &roundVal, const std::vector<T>& list)
{
    if (GetData<bool>() || list.empty()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>()%list.size()];
    }
}

void GetFuzzNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = GetData<uint32_t>();
    nodeInfo.sessionId = GetData<uint32_t>();
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    RoundVal(nodeInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(nodeInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(nodeInfo.format, SUPPORTED_FORMATS);
    RoundVal(nodeInfo.sceneType, SUPPORTED_SCENE_TYPES);
    RoundVal(nodeInfo.sourceBufferType, SUPPORTED_BUFFER_TYPES);
    nodeInfo.streamType = GetData<AudioStreamType>();
    nodeInfo.sourceType = GetData<SourceType>();
    nodeInfo.deviceClass = "test_device";
    nodeInfo.deviceNetId = "test_netid";
    nodeInfo.deviceName = "test_device_name";
    nodeInfo.nodeName = "test_node_name";
}

void ConstructorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
}

void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    cluster->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    cluster->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    cluster->ResetAll();
}

void GetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetSharedInstance();
}

void GetSharedInstanceWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    HpaeNodeInfo newNodeInfo;
    GetFuzzNodeInfo(newNodeInfo);
    (void)cluster->GetSharedInstance(newNodeInfo);
}

void GetOutputPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetOutputPort();
}

void GetOutputPortWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    HpaeNodeInfo newNodeInfo;
    GetFuzzNodeInfo(newNodeInfo);
    bool isDisConnect = GetData<bool>();
    (void)cluster->GetOutputPort(newNodeInfo, isDisConnect);
}

void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    cluster->Connect(mockPreNode);
}

void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    cluster->DisConnect(mockPreNode);
}

void ConnectWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    HpaeNodeInfo connectNodeInfo;
    GetFuzzNodeInfo(connectNodeInfo);
    cluster->ConnectWithInfo(mockPreNode, connectNodeInfo);
}

void DisConnectWithInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    HpaeNodeInfo disconnectNodeInfo;
    GetFuzzNodeInfo(disconnectNodeInfo);
    cluster->DisConnectWithInfo(mockPreNode, disconnectNodeInfo);
}

void ConnectInjectorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    cluster->ConnectInjector(mockPreNode);
}

void DisConnectInjectorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    cluster->DisConnectInjector(mockPreNode);
}

void GetCapturerEffectConfigFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    HpaeNodeInfo configNodeInfo;
    GetFuzzNodeInfo(configNodeInfo);
    HpaeSourceBufferType bufferType;
    RoundVal(bufferType, SUPPORTED_BUFFER_TYPES);
    cluster->GetCapturerEffectConfig(configNodeInfo, bufferType);
}

void GetOutputPortNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetOutputPortNum();
}

void CaptureEffectCreateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    uint64_t sceneKeyCode = GetData<uint64_t>();
    CaptureEffectAttr attr;
    attr.micChannels = GetData<uint32_t>();
    attr.ecChannels = GetData<uint32_t>();
    attr.micRefChannels = GetData<uint32_t>();
    cluster->CaptureEffectCreate(sceneKeyCode, attr);
}

void CaptureEffectReleaseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    uint64_t sceneKeyCode = GetData<uint64_t>();
    cluster->CaptureEffectRelease(sceneKeyCode);
}

void IsEffectNodeValidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->IsEffectNodeValid();
}

void GetMixerNodeUseCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetMixerNodeUseCount();
}

void GetCapturerEffectNodeUseCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetCapturerEffectNodeUseCount();
}

void GetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    HpaeNodeInfo newNodeInfo;
    GetFuzzNodeInfo(newNodeInfo);
    cluster->GetSharedInstance(newNodeInfo);

    (void)cluster->GetConverterNodeCount();
}

void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);
    (void)cluster->GetPreOutNum();
}

void MultipleGetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    for (int i = 0; i < MAX_TEN_ITERATIONS; i++) {
        HpaeNodeInfo newNodeInfo;
        GetFuzzNodeInfo(newNodeInfo);
        cluster->GetSharedInstance(newNodeInfo);
        cluster->GetOutputPort(newNodeInfo, GetData<bool>());
    }
}

void EffectLifecycleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    uint64_t sceneKeyCode = GetData<uint64_t>();
    CaptureEffectAttr attr;
    attr.micChannels = GetData<uint32_t>();
    attr.ecChannels = GetData<uint32_t>();
    attr.micRefChannels = GetData<uint32_t>();

    cluster->CaptureEffectCreate(sceneKeyCode, attr);
    cluster->IsEffectNodeValid();
    cluster->GetCapturerEffectNodeUseCount();
    cluster->CaptureEffectRelease(sceneKeyCode);
}

void ConnectDisconnectCycleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    auto mockPreNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    HpaeNodeInfo connectNodeInfo;
    GetFuzzNodeInfo(connectNodeInfo);

    for (int i = 0; i < MAX_SHORT_ITERATIONS; i++) {
        cluster->ConnectWithInfo(mockPreNode, connectNodeInfo);
        cluster->DisConnectWithInfo(mockPreNode, connectNodeInfo);
    }
}

void FullLifecycleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    cluster->DoProcess();
    cluster->GetOutputPort();

    HpaeNodeInfo newNodeInfo;
    GetFuzzNodeInfo(newNodeInfo);
    cluster->GetSharedInstance(newNodeInfo);
    cluster->GetOutputPort(newNodeInfo, false);

    cluster->Reset();
    cluster->ResetAll();
}

void NullPointerHandlingFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    std::shared_ptr<OutputNode<HpaePcmBuffer*>> nullNode = nullptr;
    cluster->Connect(nullNode);
    cluster->DisConnect(nullNode);
    cluster->ConnectInjector(nullNode);
    cluster->DisConnectInjector(nullNode);
}

void BoundaryValueFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);

    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(0xFFFFFFFF);
    nodeInfo.channels = static_cast<AudioChannel>(0xFFFFFFFF);
    nodeInfo.format = static_cast<AudioSampleFormat>(0xFFFFFFFF);
    nodeInfo.sceneType = static_cast<HpaeProcessorType>(0xFFFFFFFF);

    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    CHECK_AND_RETURN(cluster != nullptr);

    cluster->DoProcess();
    cluster->GetOutputPort();
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    ConstructorFuzzTest,
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    GetSharedInstanceFuzzTest,
    GetSharedInstanceWithNodeInfoFuzzTest,
    GetOutputPortFuzzTest,
    GetOutputPortWithNodeInfoFuzzTest,
    ConnectFuzzTest,
    DisConnectFuzzTest,
    ConnectWithInfoFuzzTest,
    DisConnectWithInfoFuzzTest,
    ConnectInjectorFuzzTest,
    DisConnectInjectorFuzzTest,
    GetCapturerEffectConfigFuzzTest,
    GetOutputPortNumFuzzTest,
    CaptureEffectCreateFuzzTest,
    CaptureEffectReleaseFuzzTest,
    IsEffectNodeValidFuzzTest,
    GetMixerNodeUseCountFuzzTest,
    GetCapturerEffectNodeUseCountFuzzTest,
    GetConverterNodeCountFuzzTest,
    GetPreOutNumFuzzTest,
    MultipleGetSharedInstanceFuzzTest,
    EffectLifecycleFuzzTest,
    ConnectDisconnectCycleFuzzTest,
    FullLifecycleFuzzTest,
    NullPointerHandlingFuzzTest,
    BoundaryValueFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;
    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    }
    return true;
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < FUZZ_INPUT_THRESHOLD_SIZE) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}