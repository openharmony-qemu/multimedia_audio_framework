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
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#undef private
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_source_input_cluster.h"
#include "hpae_info.h"
#include "hpae_define.h"
#include "hpae_msg_channel.h"
#include "hpaesourceinputcluster_fuzzer.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960;

// Loop and iteration constants
constexpr int32_t MAX_SHORT_ITERATIONS = 5;
constexpr int32_t MAX_THREE_ITERATIONS = 3;
constexpr int32_t MAX_TEN_ITERATIONS = 10;
constexpr size_t FRAME_LENGTH_OFFSET = 1;
constexpr uint32_t FUZZ_BYTE_MODULO = 256;
constexpr size_t FUZZ_MODULO_COUNT_THREE = 3;
constexpr size_t FUZZ_MODULO_OFFSET_TWO = 2;
constexpr size_t FUZZ_MODULO_COUNT_TEN = 10;
constexpr size_t FUZZ_MODULO_COUNT_FIVE = 5;
constexpr size_t FUZZ_MODULO_OFFSET_ONE = 1;
constexpr uint32_t DEFAULT_BUFFER_SIZE = 4096;
constexpr uint32_t ZERO_SESSION_ID = 0;
constexpr uint32_t MIN_FRAME_LENGTH = 1;
constexpr uint32_t ZERO_FRAME_LENGTH = 0;

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_F32LE,
};

const std::vector<SourceType> SUPPORTED_SOURCE_TYPES {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_ULTRASONIC,
};

const std::vector<AudioStreamType> SUPPORTED_STREAM_TYPES {
    STREAM_DEFAULT,
    STREAM_MUSIC,
    STREAM_SPEECH,
    STREAM_RING,
};

const std::vector<HpaeSourceBufferType> SUPPORTED_BUFFER_TYPES {
    HPAE_SOURCE_BUFFER_TYPE_DEFAULT,
    HPAE_SOURCE_BUFFER_TYPE_MIC,
    HPAE_SOURCE_BUFFER_TYPE_EC,
    HPAE_SOURCE_BUFFER_TYPE_MICREF,
};

const std::vector<HpaeSourceInputNodeType> SUPPORTED_NODE_TYPES {
    HPAE_SOURCE_DEFAULT,
    HPAE_SOURCE_MIC,
    HPAE_SOURCE_MIC_EC,
    HPAE_SOURCE_EC,
    HPAE_SOURCE_MICREF,
    HPAE_SOURCE_OFFLOAD,
};

const std::vector<StreamManagerState> SUPPORTED_STATES {
    STREAM_MANAGER_INVALID,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED,
};

const std::vector<StreamChangeType> SUPPORTED_CHANGE_TYPES {
    STREAM_CHANGE_TYPE_ADD,
    STREAM_CHANGE_TYPE_REMOVE,
    STREAM_CHANGE_TYPE_REMOVE_ALL,
    STREAM_CHANGE_TYPE_STATE_CHANGE,
};

const std::vector<CapturerState> SUPPORTED_CAPTURER_STATES {
    CAPTURER_INVALID,
    CAPTURER_NEW,
    CAPTURER_PREPARED,
    CAPTURER_RUNNING,
    CAPTURER_STOPPED,
};

const std::vector<HpaeProcessorType> SUPPORTED_PROCESSOR_TYPES {
    HPAE_SCENE_UNCONNECTED,
    HPAE_SCENE_DEFAULT,
    HPAE_SCENE_MUSIC,
    HPAE_SCENE_GAME,
    HPAE_SCENE_MOVIE,
    HPAE_SCENE_SPEECH,
    HPAE_SCENE_RING,
    HPAE_SCENE_VOIP_DOWN,
    HPAE_SCENE_OTHERS,
    HPAE_SCENE_EFFECT_NONE,
    HPAE_SCENE_EFFECT_OUT,
    HPAE_SCENE_SPLIT_MEDIA,
    HPAE_SCENE_SPLIT_NAVIGATION,
    HPAE_SCENE_SPLIT_COMMUNICATION,
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_RECORD,
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
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
void RoundVal(T &roundVal, const std::vector<T>& list)
{
    if (list.empty()) {
        return;
    }
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

void GetFuzzNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.sessionId = GetData<uint32_t>();
    nodeInfo.frameLen = GetData<uint32_t>() % DEFAULT_FRAME_LENGTH + FRAME_LENGTH_OFFSET;
    nodeInfo.streamType = GetData<AudioStreamType>();
    RoundVal(nodeInfo.streamType, SUPPORTED_STREAM_TYPES);
    RoundVal(nodeInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(nodeInfo.format, SUPPORTED_FORMATS);
    RoundVal(nodeInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(nodeInfo.sourceType, SUPPORTED_SOURCE_TYPES);
    nodeInfo.sceneType = GetData<HpaeProcessorType>();
    RoundVal(nodeInfo.sceneType, SUPPORTED_PROCESSOR_TYPES);
    RoundVal(nodeInfo.sourceBufferType, SUPPORTED_BUFFER_TYPES);
    nodeInfo.sourceInputNodeType = GetData<HpaeSourceInputNodeType>();
    RoundVal(nodeInfo.sourceInputNodeType, SUPPORTED_NODE_TYPES);
}

std::string GetFuzzString()
{
    uint32_t len = GetData<uint32_t>() % FUZZ_BYTE_MODULO;
    std::string result;
    for (uint32_t i = 0; i < len && g_pos < g_dataSize; i++) {
        result += static_cast<char>(GetData<uint8_t>());
    }
    return result;
}

// Test function 1: Constructor with single HpaeNodeInfo
void ConstructorSingleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
}

// Test function 2: Constructor with vector of HpaeNodeInfo
void ConstructorVectorFuzzTest()
{
    std::vector<HpaeNodeInfo> nodeInfos;
    uint32_t count = GetData<uint32_t>() % FUZZ_MODULO_COUNT_FIVE + FUZZ_MODULO_OFFSET_ONE;
    for (uint32_t i = 0; i < count; i++) {
        HpaeNodeInfo nodeInfo;
        GetFuzzNodeInfo(nodeInfo);
        nodeInfos.push_back(nodeInfo);
    }
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfos);
    if (cluster == nullptr) {
        return;
    }
}

// Test function 3: Constructor with empty vector (should log error)
void ConstructorEmptyVectorFuzzTest()
{
    std::vector<HpaeNodeInfo> emptyNodeInfos;
    auto cluster = std::make_shared<HpaeSourceInputCluster>(emptyNodeInfos);
    if (cluster == nullptr) {
        return;
    }
}

// Test function 4: DoProcess (empty implementation)
void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    cluster->DoProcess();
}

// Test function 5: Reset
void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    cluster->Reset();
}

// Test function 6: ResetAll
void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    cluster->ResetAll();
}

// Test function 7: GetSharedInstance (no parameter)
void GetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    auto instance = cluster->GetSharedInstance();
    (void)instance;
}

// Test function 8: GetSharedInstance with HpaeNodeInfo parameter
void GetSharedInstanceWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    HpaeNodeInfo paramNodeInfo;
    GetFuzzNodeInfo(paramNodeInfo);
    auto instance = cluster->GetSharedInstance(paramNodeInfo);
    (void)instance;
}

// Test function 9: GetOutputPort (no parameter)
void GetOutputPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    auto port = cluster->GetOutputPort();
    (void)port;
}

// Test function 10: GetOutputPort with HpaeNodeInfo and isDisConnect parameters
void GetOutputPortWithParamsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    HpaeNodeInfo paramNodeInfo;
    GetFuzzNodeInfo(paramNodeInfo);
    bool isDisConnect = GetData<bool>();
    auto port = cluster->GetOutputPort(paramNodeInfo, isDisConnect);
    (void)port;
}

// Test function 11: GetCapturerSourceInstance
void GetCapturerSourceInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    std::string deviceClass = GetFuzzString();
    std::string deviceNetId = GetFuzzString();
    SourceType sourceType = GetData<SourceType>();
    RoundVal(sourceType, SUPPORTED_SOURCE_TYPES);
    std::string sourceName = GetFuzzString();
    int32_t result = cluster->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName);
    (void)result;
}

// Test function 12: CapturerSourceInit
void CapturerSourceInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    attr.adapterName = GetFuzzString();
    AudioSamplingRate sampleRate = GetData<AudioSamplingRate>();
    RoundVal(sampleRate, SUPPORTED_SAMPLE_RATES);
    attr.sampleRate = static_cast<uint32_t>(sampleRate);
    AudioChannel channel = GetData<AudioChannel>();
    RoundVal(channel, SUPPORTED_CHANNELS);
    attr.channel = static_cast<uint32_t>(channel);
    RoundVal(attr.format, SUPPORTED_FORMATS);
    attr.volume = GetData<float>();
    attr.bufferSize = GetData<uint32_t>();
    SourceType sourceType = GetData<SourceType>();
    RoundVal(sourceType, SUPPORTED_SOURCE_TYPES);
    attr.sourceType = static_cast<int32_t>(sourceType);
    int32_t result = cluster->CapturerSourceInit(attr);
    (void)result;
}

// Test function 13: CapturerSourceDeInit
void CapturerSourceDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceDeInit();
    (void)result;
}

// Test function 14: CapturerSourceFlush
void CapturerSourceFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceFlush();
    (void)result;
}

// Test function 15: CapturerSourcePause
void CapturerSourcePauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourcePause();
    (void)result;
}

// Test function 16: CapturerSourceReset
void CapturerSourceResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceReset();
    (void)result;
}

// Test function 17: CapturerSourceResume
void CapturerSourceResumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceResume();
    (void)result;
}

// Test function 18: CapturerSourceStart
void CapturerSourceStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceStart();
    (void)result;
}

// Test function 19: CapturerSourceStop
void CapturerSourceStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    int32_t result = cluster->CapturerSourceStop();
    (void)result;
}

// Test function 20: GetSourceState
void GetSourceStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    StreamManagerState state = cluster->GetSourceState();
    (void)state;
}

// Test function 21: GetOutputPortNum (no parameter)
void GetOutputPortNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    size_t portNum = cluster->GetOutputPortNum();
    (void)portNum;
}

// Test function 22: GetOutputPortNum with HpaeNodeInfo parameter
void GetOutputPortNumWithNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    HpaeNodeInfo paramNodeInfo;
    GetFuzzNodeInfo(paramNodeInfo);
    size_t portNum = cluster->GetOutputPortNum(paramNodeInfo);
    (void)portNum;
}

// Test function 23: GetSourceInputNodeType
void GetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    HpaeSourceInputNodeType nodeType = cluster->GetSourceInputNodeType();
    (void)nodeType;
}

// Test function 24: SetSourceInputNodeType
void SetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    HpaeSourceInputNodeType nodeType = GetData<HpaeSourceInputNodeType>();
    RoundVal(nodeType, SUPPORTED_NODE_TYPES);
    cluster->SetSourceInputNodeType(nodeType);
}

// Test function 25: UpdateAppsUidAndSessionId
void UpdateAppsUidAndSessionIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    std::vector<int32_t> appsUid;
    std::vector<int32_t> sessionsId;
    uint32_t count = GetData<uint32_t>() % FUZZ_MODULO_COUNT_TEN;
    for (uint32_t i = 0; i < count; i++) {
        appsUid.push_back(GetData<int32_t>());
        sessionsId.push_back(GetData<int32_t>());
    }
    cluster->UpdateAppsUidAndSessionId(appsUid, sessionsId);
}

// Test function 26: GetCaptureId
void GetCaptureIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    uint32_t captureId = cluster->GetCaptureId();
    (void)captureId;
}

// Test function 27: SetInjectState
void SetInjectStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    bool isInjecting = GetData<bool>();
    cluster->SetInjectState(isInjecting);
}

// Test function 28: NotifyStreamChangeToSource
void NotifyStreamChangeToSourceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }
    StreamChangeType change = GetData<StreamChangeType>();
    RoundVal(change, SUPPORTED_CHANGE_TYPES);
    uint32_t sessionId = GetData<uint32_t>();
    SourceType source = GetData<SourceType>();
    RoundVal(source, SUPPORTED_SOURCE_TYPES);
    CapturerState state = GetData<CapturerState>();
    RoundVal(state, SUPPORTED_CAPTURER_STATES);
    uint32_t appUid = GetData<uint32_t>();
    cluster->NotifyStreamChangeToSource(change, sessionId, source, state, appUid);
}

// Test function 29: Full source lifecycle test
void FullSourceLifecycleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Initialize source with fuzzed but valid attributes
    IAudioSourceAttr attr;
    attr.adapterName = "test_adapter";
    attr.sampleRate = static_cast<uint32_t>(SAMPLE_RATE_48000);
    attr.channel = static_cast<uint32_t>(STEREO);
    attr.format = SAMPLE_F32LE;
    attr.volume = 1.0f;
    attr.bufferSize = DEFAULT_BUFFER_SIZE;
    attr.sourceType = static_cast<int32_t>(SOURCE_TYPE_MIC);
    cluster->CapturerSourceInit(attr);

    // Start
    cluster->CapturerSourceStart();

    // Pause
    cluster->CapturerSourcePause();

    // Resume
    cluster->CapturerSourceResume();

    // Stop
    cluster->CapturerSourceStop();

    // Flush
    cluster->CapturerSourceFlush();

    // Reset
    cluster->CapturerSourceReset();

    // DeInit
    cluster->CapturerSourceDeInit();
}

// Test function 30: Multiple GetSharedInstance calls
void MultipleGetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Call GetSharedInstance multiple times with different nodeInfos
    for (int i = 0; i < MAX_SHORT_ITERATIONS; i++) {
        HpaeNodeInfo paramNodeInfo;
        GetFuzzNodeInfo(paramNodeInfo);
        auto instance = cluster->GetSharedInstance(paramNodeInfo);
        (void)instance;
    }
}

// Test function 31: GetConverterNodeCount (test method)
void GetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Add some converter nodes by calling GetSharedInstance with different nodeInfos
    for (int i = 0; i < MAX_THREE_ITERATIONS; i++) {
        HpaeNodeInfo paramNodeInfo;
        GetFuzzNodeInfo(paramNodeInfo);
        auto instance = cluster->GetSharedInstance(paramNodeInfo);
        (void)instance;
    }

    uint32_t count = cluster->GetConverterNodeCount();
    (void)count;
}

// Test function 32: GetSourceInputNodeUseCount (test method)
void GetSourceInputNodeUseCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    uint32_t useCount = cluster->GetSourceInputNodeUseCount();
    (void)useCount;
}

// Test function 33: State transition stress test
void StateTransitionStressFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Rapid state transitions
    for (int i = 0; i < MAX_TEN_ITERATIONS; i++) {
        cluster->CapturerSourceStart();
        cluster->CapturerSourcePause();
        cluster->CapturerSourceResume();
        cluster->CapturerSourceStop();
    }
}

// Test function 34: Multiple nodes GetOutputPort test
void MultipleNodesGetOutputPortFuzzTest()
{
    std::vector<HpaeNodeInfo> nodeInfos;
    uint32_t count = GetData<uint32_t>() % FUZZ_MODULO_COUNT_THREE + FUZZ_MODULO_OFFSET_TWO;
    for (uint32_t i = 0; i < count; i++) {
        HpaeNodeInfo nodeInfo;
        GetFuzzNodeInfo(nodeInfo);
        nodeInfos.push_back(nodeInfo);
    }
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfos);
    if (cluster == nullptr) {
        return;
    }

    // Get output ports for different nodeInfos
    for (uint32_t i = 0; i < count; i++) {
        auto port = cluster->GetOutputPort(nodeInfos[i], false);
        (void)port;
    }
}

// Test function 35: Boundary values test
void BoundaryValuesFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);

    // Test with extreme values
    nodeInfo.sessionId = ZERO_SESSION_ID;
    auto cluster1 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);

    nodeInfo.sessionId = UINT32_MAX;
    auto cluster2 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);

    nodeInfo.frameLen = MIN_FRAME_LENGTH;
    auto cluster3 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);

    nodeInfo.samplingRate = SAMPLE_RATE_8000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_S16LE;
    auto cluster4 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
}

// Test function 36: SetSourceType boundary test
void SetNodeTypeBoundaryFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Test with all node type values
    cluster->SetSourceInputNodeType(HPAE_SOURCE_DEFAULT);
    cluster->SetSourceInputNodeType(HPAE_SOURCE_MIC);
    cluster->SetSourceInputNodeType(HPAE_SOURCE_MIC_EC);
    cluster->SetSourceInputNodeType(HPAE_SOURCE_EC);
    cluster->SetSourceInputNodeType(HPAE_SOURCE_MICREF);
    cluster->SetSourceInputNodeType(HPAE_SOURCE_OFFLOAD);

    // Test with invalid value
    int32_t invalidType = GetData<int32_t>();
    cluster->SetSourceInputNodeType(static_cast<HpaeSourceInputNodeType>(invalidType));
}

// Test function 37: Empty vectors UpdateAppsUidAndSessionId test
void EmptyVectorsUpdateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    std::vector<int32_t> emptyAppsUid;
    std::vector<int32_t> emptySessionsId;
    cluster->UpdateAppsUidAndSessionId(emptyAppsUid, emptySessionsId);
}

// Test function 38: Disconnect port test
void DisconnectPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    HpaeNodeInfo paramNodeInfo;
    GetFuzzNodeInfo(paramNodeInfo);
    // Connect first
    auto port1 = cluster->GetOutputPort(paramNodeInfo, false);
    // Then disconnect
    auto port2 = cluster->GetOutputPort(paramNodeInfo, true);
    (void)port1;
    (void)port2;
}

// Test function 39: Multiple Reset operations
void MultipleResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Multiple reset calls
    for (int i = 0; i < MAX_SHORT_ITERATIONS; i++) {
        cluster->Reset();
        cluster->ResetAll();
    }
}

// Test function 40: GetSourceState with various states
void GetSourceStateVariousFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Get state before init
    StreamManagerState state1 = cluster->GetSourceState();
    (void)state1;

    // Initialize with fuzzed but valid attributes and get state
    IAudioSourceAttr attr;
    attr.adapterName = "test";
    attr.sampleRate = static_cast<uint32_t>(SAMPLE_RATE_48000);
    attr.channel = static_cast<uint32_t>(STEREO);
    attr.format = SAMPLE_F32LE;
    attr.sourceType = static_cast<int32_t>(SOURCE_TYPE_MIC);
    cluster->CapturerSourceInit(attr);
    StreamManagerState state2 = cluster->GetSourceState();
    (void)state2;

    // Start and get state
    cluster->CapturerSourceStart();
    StreamManagerState state3 = cluster->GetSourceState();
    (void)state3;
}

// Test function 41: Operations with uninitialized source
void UninitializedSourceOperationsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);
    auto cluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster == nullptr) {
        return;
    }

    // Test various operations without initializing the source
    // This tests handling of null/invalid internal source state
    cluster->CapturerSourceStart();
    cluster->CapturerSourcePause();
    cluster->CapturerSourceResume();
    cluster->CapturerSourceStop();
    cluster->CapturerSourceFlush();
    cluster->CapturerSourceReset();
    cluster->CapturerSourceDeInit();

    // Get state should handle uninitialized source
    StreamManagerState state = cluster->GetSourceState();
    (void)state;
}

// Test function 42: Edge case with empty and invalid node info
void InvalidNodeInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetFuzzNodeInfo(nodeInfo);

    // Test with invalid session ID
    nodeInfo.sessionId = UINT32_MAX;
    auto cluster1 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster1 != nullptr) {
        cluster1->GetSourceState();
    }

    // Test with minimal frame length
    nodeInfo.frameLen = ZERO_FRAME_LENGTH;
    auto cluster2 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster2 != nullptr) {
        cluster2->GetSourceState();
    }

    // Test with extreme sample rate
    nodeInfo.samplingRate = SAMPLE_RATE_192000;
    auto cluster3 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster3 != nullptr) {
        cluster3->GetSourceState();
    }

    // Test with invalid scene type
    nodeInfo.sceneType = HPAE_SCENE_UNCONNECTED;
    auto cluster4 = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (cluster4 != nullptr) {
        cluster4->GetSourceState();
    }
}

typedef void (*TestFuncs)();

TestFuncs g_testFuncs[] = {
    ConstructorSingleFuzzTest,
    ConstructorVectorFuzzTest,
    ConstructorEmptyVectorFuzzTest,
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    GetSharedInstanceFuzzTest,
    GetSharedInstanceWithNodeInfoFuzzTest,
    GetOutputPortFuzzTest,
    GetOutputPortWithParamsFuzzTest,
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
    GetOutputPortNumFuzzTest,
    GetOutputPortNumWithNodeInfoFuzzTest,
    GetSourceInputNodeTypeFuzzTest,
    SetSourceInputNodeTypeFuzzTest,
    UpdateAppsUidAndSessionIdFuzzTest,
    GetCaptureIdFuzzTest,
    SetInjectStateFuzzTest,
    NotifyStreamChangeToSourceFuzzTest,
    FullSourceLifecycleFuzzTest,
    MultipleGetSharedInstanceFuzzTest,
    GetConverterNodeCountFuzzTest,
    GetSourceInputNodeUseCountFuzzTest,
    StateTransitionStressFuzzTest,
    MultipleNodesGetOutputPortFuzzTest,
    BoundaryValuesFuzzTest,
    SetNodeTypeBoundaryFuzzTest,
    EmptyVectorsUpdateFuzzTest,
    DisconnectPortFuzzTest,
    MultipleResetFuzzTest,
    GetSourceStateVariousFuzzTest,
    UninitializedSourceOperationsFuzzTest,
    InvalidNodeInfoFuzzTest,
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

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::HPAE::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::HPAE::FuzzTest(data, size);
    return 0;
}
