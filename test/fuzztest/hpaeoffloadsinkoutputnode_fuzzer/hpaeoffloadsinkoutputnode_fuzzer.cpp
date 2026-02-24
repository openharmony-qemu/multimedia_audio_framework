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

#include "hpaeoffloadsinkoutputnode_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <limits>

#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_offload_sinkoutput_node.h"
#include "hpae_node.h"
#include "hpae_info.h"
#include "hpae_msg_channel.h"
#include "hpae_pcm_buffer.h"
#include "audio_utils.h"
#include "hdi_adapter_type.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
constexpr size_t FUZZ_INPUT_SIZE_THRESHOLD = 32;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr uint32_t DEFAULT_SESSION_ID = 100001;
constexpr uint32_t DEFAULT_NODE_ID = 200;

// Loop and iteration constants
constexpr int32_t MAX_SHORT_ITERATIONS = 5;
constexpr int32_t MAX_MEDIUM_ITERATIONS = 10;
constexpr int32_t MAX_LONG_ITERATIONS = 100;
constexpr int32_t LOOP_START_NEGATIVE = -10;
constexpr int32_t LOOP_END_POSITIVE = 20;
constexpr size_t FUZZ_MODULO_COUNT_TEN = 10;
constexpr uint32_t ZERO_FRAME_LENGTH = 0;
constexpr size_t FUZZ_MAX_LEN_INCREMENT = 1;
constexpr size_t FUZZ_STRING_SHORT_LENGTH = 50;
constexpr uint32_t TIMEOUT_STOP_THREAD_MS = 1000;

const std::vector<AudioChannel> SUPPORTED_CHANNELS {
    MONO,
    STEREO,
};

const std::vector<AudioSamplingRate> SUPPORTED_SAMPLE_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const std::vector<AudioSampleFormat> SUPPORTED_FORMATS {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const std::vector<AudioStreamType> SUPPORTED_STREAM_TYPES {
    STREAM_MUSIC,
    STREAM_MOVIE,
    STREAM_SPEECH,
    STREAM_RING,
};

const std::vector<StreamManagerState> SUPPORTED_STATES {
    STREAM_MANAGER_INVALID,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED,
};

const std::vector<AudioOffloadType> SUPPORTED_OFFLOAD_STATES {
    OFFLOAD_DEFAULT,
    OFFLOAD_ACTIVE_FOREGROUND,
    OFFLOAD_ACTIVE_BACKGROUND,
    OFFLOAD_INACTIVE_BACKGROUND,
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
        roundVal = GetData<T>();
        return;
    }
    if (GetData<bool>()) {
        roundVal = GetData<T>();
    } else {
        roundVal = list[GetData<uint32_t>() % list.size()];
    }
}

std::string GetFuzzString(size_t maxLen = FUZZ_STRING_SHORT_LENGTH * 2)
{
    if (RAW_DATA == nullptr || g_pos >= g_dataSize) {
        return "";
    }
    size_t len = GetData<size_t>() % (maxLen + FUZZ_MAX_LEN_INCREMENT);
    if (len > g_dataSize - g_pos) {
        len = g_dataSize - g_pos;
    }
    std::string result(reinterpret_cast<const char*>(RAW_DATA + g_pos), len);
    g_pos += len;
    return result;
}

static void InitNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    RoundVal(nodeInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(nodeInfo.format, SUPPORTED_FORMATS);
    RoundVal(nodeInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(nodeInfo.streamType, SUPPORTED_STREAM_TYPES);
    nodeInfo.channelLayout = static_cast<AudioChannelLayout>(GetData<int32_t>());
    nodeInfo.sceneType = static_cast<HpaeProcessorType>(GetData<int32_t>());
    nodeInfo.deviceClass = "offload";
    nodeInfo.deviceNetId = "LocalDevice";
    nodeInfo.deviceName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    nodeInfo.sourceType = static_cast<SourceType>(GetData<int32_t>());
}

typedef void (*TestFuncs)();

// Test 1: Constructor Fuzz Test
void ConstructorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
}

// Test 2: Reset Fuzz Test
void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
    node->Reset();
}

// Test 3: ResetAll Fuzz Test
void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
    node->ResetAll();
}

// Test 4: Connect Fuzz Test
void ConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto sinkNode = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(sinkNode != nullptr);

    auto preNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    if (preNode != nullptr) {
        sinkNode->Connect(preNode);
    }
}

// Test 5: DisConnect Fuzz Test
void DisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto sinkNode = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(sinkNode != nullptr);

    auto preNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    if (preNode != nullptr) {
        sinkNode->Connect(preNode);
        sinkNode->DisConnect(preNode);
    }

    // Test disconnect on null node
    sinkNode->DisConnect(nullptr);
}

// Test 6: GetRenderSinkInstance Fuzz Test
void GetRenderSinkInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    std::string deviceClass = GetData<bool>() ? "offload" : "remote_offload";
    std::string deviceNetworkId = GetData<bool>() ? "LocalDevice" : GetFuzzString(FUZZ_STRING_SHORT_LENGTH);

    int32_t ret = node->GetRenderSinkInstance(deviceClass, deviceNetworkId);
    (void)ret; // May return ERROR if device not available
}

// Test 7: RenderSinkInit Fuzz Test
void RenderSinkInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Get render sink first
    node->GetRenderSinkInstance("offload", "LocalDevice");

    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.openMicSpeaker = GetData<uint32_t>();
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);

    int32_t ret = node->RenderSinkInit(attr);
    (void)ret;
}

// Test 8: RenderSinkDeInit Fuzz Test
void RenderSinkDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Init first
    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Then deinit
    int32_t ret = node->RenderSinkDeInit();
    (void)ret;
}

// Test 9: RenderSinkFlush Fuzz Test
void RenderSinkFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();

    int32_t ret = node->RenderSinkFlush();
    (void)ret;
}

// Test 10: RenderSinkStart Fuzz Test
void RenderSinkStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    int32_t ret = node->RenderSinkStart();
    (void)ret;
}

// Test 11: RenderSinkStop Fuzz Test
void RenderSinkStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();

    int32_t ret = node->RenderSinkStop();
    (void)ret;
}

// Test 12: FlushStream Fuzz Test
void FlushStreamFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);
    node->FlushStream();
}

// Test 13: GetPreOutNum Fuzz Test
void GetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    size_t count = node->GetPreOutNum();
    (void)count;

    // Test with connected node
    auto preNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    if (preNode != nullptr) {
        node->Connect(preNode);
        count = node->GetPreOutNum();
        (void)count;
    }
}

// Test 14: GetSinkState Fuzz Test
void GetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    StreamManagerState state = node->GetSinkState();
    (void)state;
}

// Test 15: SetSinkState Fuzz Test
void SetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    StreamManagerState newState = GetData<StreamManagerState>();
    int32_t ret = node->SetSinkState(newState);
    (void)ret;
}

// Test 16: GetRenderFrameData Fuzz Test
void GetRenderFrameDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    const char* data = node->GetRenderFrameData();
    (void)data;
}

// Test 17: StopStream Fuzz Test
void StopStreamFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();

    node->StopStream();
}

// Test 18: SetPolicyState Fuzz Test
void SetPolicyStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    int32_t policyState = GetData<int32_t>();
    node->SetPolicyState(policyState);
}

// Test 19: GetLatency Fuzz Test
void GetLatencyFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    uint64_t latency = node->GetLatency();
    (void)latency;
}

// Test 20: SetTimeoutStopThd Fuzz Test
void SetTimeoutStopThdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Test with various timeout values including boundary values
    uint32_t timeoutThdMs = GetData<uint32_t>();
    int32_t ret = node->SetTimeoutStopThd(timeoutThdMs);
    (void)ret;

    // Test with zero (edge case for frameLenMs_ calculation)
    ret = node->SetTimeoutStopThd(0);
    (void)ret;

    // Test with maximum value
    ret = node->SetTimeoutStopThd(UINT32_MAX);
    (void)ret;
}

// Test 21: SetOffloadRenderCallbackType Fuzz Test
void SetOffloadRenderCallbackTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    int32_t type = GetData<int32_t>();
    int32_t ret = node->SetOffloadRenderCallbackType(type);
    (void)ret;
}

// Test 22: SetSpeed Fuzz Test
void SetSpeedFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test with various speed values
    float speed = GetData<float>();
    node->SetSpeed(speed);

    // Test with extreme values
    float extremeSpeeds[] = {-10.0f, 0.0f, 0.5f, 1.0f, 2.0f, 10.0f,
        std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    for (float s : extremeSpeeds) {
        node->SetSpeed(s);
    }
}

// Test 23: UpdateAppsUid Fuzz Test
void UpdateAppsUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    std::vector<int32_t> appsUid;
    size_t count = GetData<size_t>() % FUZZ_MODULO_COUNT_TEN;
    for (size_t i = 0; i < count; ++i) {
        appsUid.push_back(GetData<int32_t>());
    }

    int32_t ret = node->UpdateAppsUid(appsUid);
    (void)ret;
}

// Test 24: NotifyStreamChangeToSink Fuzz Test
void NotifyStreamChangeToSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    StreamChangeType change = static_cast<StreamChangeType>(GetData<int32_t>());
    uint32_t sessionId = GetData<uint32_t>();
    StreamUsage usage = static_cast<StreamUsage>(GetData<int32_t>());
    RendererState state = static_cast<RendererState>(GetData<int32_t>());
    uint32_t appUid = GetData<uint32_t>();

    node->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}

// Test 25: RegisterOffloadCallback Fuzz Test
void RegisterOffloadCallbackFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Test with null callback
    node->RegisterOffloadCallback(nullptr);

    // Test with valid callback
    class TestOffloadCallback : public IOffloadCallback {
    public:
        void OnNotifyHdiData(const std::pair<uint64_t, TimePoint> &hdiPos) override {}
    };
    TestOffloadCallback callback;
    node->RegisterOffloadCallback(&callback);
}

// Test 26: GetOffloadCallbackData Fuzz Test
void GetOffloadCallbackDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    OffloadCallbackData data = node->GetOffloadCallbackData();
    (void)data;
}

// Test 27: DoProcess Fuzz Test
void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test DoProcess - should handle gracefully
    node->DoProcess();
}

// Test 28: Null Pointer Handling Test
void NullPointerHandlingFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Test methods that check for audioRendererSink_ null
    int32_t ret = node->RenderSinkDeInit();
    (void)ret;

    ret = node->RenderSinkFlush();
    (void)ret;

    ret = node->RenderSinkStart();
    (void)ret;

    ret = node->RenderSinkStop();
    (void)ret;

    node->StopStream();

    node->SetSpeed(1.0f);

    std::vector<int32_t> appsUid;
    ret = node->UpdateAppsUid(appsUid);
    (void)ret;

    node->NotifyStreamChangeToSink(static_cast<StreamChangeType>(0), 1,
        static_cast<StreamUsage>(0), static_cast<RendererState>(0), 0);
}

// Test 29: Boundary Values Test
void BoundaryValuesFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Test with zero frameLenMs_ (may cause division by zero)
    nodeInfo.frameLen = ZERO_FRAME_LENGTH;
    auto node2 = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    if (node2 != nullptr) {
        node2->SetTimeoutStopThd(TIMEOUT_STOP_THREAD_MS);
    }

    // Test with maximum timeout
    node->SetTimeoutStopThd(UINT32_MAX);

    // Test with minimum timeout
    node->SetTimeoutStopThd(0);
}

// Test 30: State Transition Test
void StateTransitionFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test various state transitions
    for (auto targetState : SUPPORTED_STATES) {
        node->SetSinkState(targetState);
        StreamManagerState state = node->GetSinkState();
        (void)state;
    }

    // Test Start->Stop->Start cycle
    node->RenderSinkStart();
    node->RenderSinkStop();
    node->RenderSinkStart();
}

// Test 31: Full Lifecycle Test
void FullLifecycleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Get render sink
    int32_t ret = node->GetRenderSinkInstance("offload", "LocalDevice");
    (void)ret;

    // Initialize
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    ret = node->RenderSinkInit(attr);
    (void)ret;

    // Start
    ret = node->RenderSinkStart();
    (void)ret;

    // Set speed
    node->SetSpeed(1.5f);

    // Set policy state
    node->SetPolicyState(OFFLOAD_ACTIVE_FOREGROUND);

    // Flush
    ret = node->RenderSinkFlush();
    (void)ret;

    // Stop
    ret = node->RenderSinkStop();
    (void)ret;

    // DeInit
    ret = node->RenderSinkDeInit();
    (void)ret;
}

// Test 32: Speed Extreme Values Test
void SpeedExtremeValuesFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MOVIE; // Speed only works for MOVIE or remote_offload
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();

    // Test with extreme speed values
    float speeds[] = {
        -1000.0f, -10.0f, -1.0f, -0.5f, -0.125f,
        0.0f, 0.001f, 0.125f, 0.5f, 1.0f, 1.5f, 2.0f, 4.0f,
        10.0f, 100.0f, 1000.0f,
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::epsilon()
    };

    for (float speed : speeds) {
        node->SetSpeed(speed);
    }

    node->RenderSinkStop();
}

// Test 33: OffloadCallbackType Boundary Test
void OffloadCallbackTypeBoundaryFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test with various callback types including invalid ones
    int32_t callbackTypes[] = {
        -100, -10, -1, 0, 1, 2, 3, 4, 10, 100,
        INT32_MIN, INT32_MAX
    };

    for (int32_t type : callbackTypes) {
        node->SetOffloadRenderCallbackType(type);
    }
}

// Test 34: Multiple Connect/Disconnect Test
void MultipleConnectDisconnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto sinkNode = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(sinkNode != nullptr);

    // Test multiple connect/disconnect cycles
    for (int i = 0; i < MAX_SHORT_ITERATIONS; ++i) {
        auto preNode = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
        if (preNode != nullptr) {
            sinkNode->Connect(preNode);
            size_t count = sinkNode->GetPreOutNum();
            (void)count;
            sinkNode->DisConnect(preNode);
        }
    }
}

// Test 35: Reset With Connections Test
void ResetWithConnectionsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto sinkNode = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(sinkNode != nullptr);

    // Add some connections
    auto preNode1 = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();
    auto preNode2 = std::shared_ptr<OutputNode<HpaePcmBuffer*>>();

    if (preNode1 != nullptr) {
        sinkNode->Connect(preNode1);
    }
    if (preNode2 != nullptr) {
        sinkNode->Connect(preNode2);
    }

    // Call Reset should disconnect all
    bool ret = sinkNode->Reset();
    (void)ret;

    // Call ResetAll
    ret = sinkNode->ResetAll();
    (void)ret;
}

// Test 36: DeviceClass Variation Test
void DeviceClassVariationFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);

    std::vector<std::string> deviceClasses = {
        "offload",
        "remote_offload",
        "unknown",
        "",
        "very_long_device_class_name_that_might_cause_issues"
    };

    std::vector<std::string> networkIds = {
        "LocalDevice",
        "remote_device_1",
        "",
        "very_long_network_id_that_might_cause_buffer_overflow"
    };

    for (const auto& deviceClass : deviceClasses) {
        for (const auto& networkId : networkIds) {
            auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
            if (node != nullptr) {
                int32_t ret = node->GetRenderSinkInstance(deviceClass, networkId);
                (void)ret;
            }
        }
    }
}

// Test 37: PolicyState Transition Test
void PolicyStateTransitionFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();

    // Test all policy state transitions
    for (auto state : SUPPORTED_OFFLOAD_STATES) {
        node->SetPolicyState(static_cast<int32_t>(state));
    }

    // Test rapid state changes
    for (int i = 0; i < MAX_MEDIUM_ITERATIONS; ++i) {
        node->SetPolicyState(OFFLOAD_ACTIVE_FOREGROUND);
        node->SetPolicyState(OFFLOAD_INACTIVE_BACKGROUND);
    }

    node->RenderSinkStop();
}

// Test 38: Empty AppsUid Vector Test
void EmptyAppsUidVectorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test with empty vector
    std::vector<int32_t> emptyAppsUid;
    int32_t ret = node->UpdateAppsUid(emptyAppsUid);
    (void)ret;

    // Test with large vector
    std::vector<int32_t> largeAppsUid;
    for (int i = 0; i < MAX_LONG_ITERATIONS; ++i) {
        largeAppsUid.push_back(GetData<int32_t>());
    }
    ret = node->UpdateAppsUid(largeAppsUid);
    (void)ret;
}

// Test 39: StreamChangeType All Values Test
void StreamChangeTypeAllValuesFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    // Test all stream change types
    for (int32_t i = LOOP_START_NEGATIVE; i < LOOP_END_POSITIVE; ++i) {
        StreamChangeType change = static_cast<StreamChangeType>(i);
        node->NotifyStreamChangeToSink(change, GetData<uint32_t>(),
            static_cast<StreamUsage>(GetData<int32_t>()),
            static_cast<RendererState>(GetData<int32_t>()),
            GetData<uint32_t>());
    }
}

// Test 40: GetOffloadCallbackData States Test
void GetOffloadCallbackDataStatesFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    CHECK_AND_RETURN(node != nullptr);

    // Test in various states
    OffloadCallbackData data1 = node->GetOffloadCallbackData();
    (void)data1;

    node->GetRenderSinkInstance("offload", "LocalDevice");
    IAudioSinkAttr attr = {};
    attr.adapterName = GetFuzzString(FUZZ_STRING_SHORT_LENGTH);
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = static_cast<uint32_t>(nodeInfo.channels);
    node->RenderSinkInit(attr);

    OffloadCallbackData data2 = node->GetOffloadCallbackData();
    (void)data2;

    node->RenderSinkStart();

    OffloadCallbackData data3 = node->GetOffloadCallbackData();
    (void)data3;
}

TestFuncs g_testFuncs[] = {
    ConstructorFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    ConnectFuzzTest,
    DisConnectFuzzTest,
    GetRenderSinkInstanceFuzzTest,
    RenderSinkInitFuzzTest,
    RenderSinkDeInitFuzzTest,
    RenderSinkFlushFuzzTest,
    RenderSinkStartFuzzTest,
    RenderSinkStopFuzzTest,
    FlushStreamFuzzTest,
    GetPreOutNumFuzzTest,
    GetSinkStateFuzzTest,
    SetSinkStateFuzzTest,
    GetRenderFrameDataFuzzTest,
    StopStreamFuzzTest,
    SetPolicyStateFuzzTest,
    GetLatencyFuzzTest,
    SetTimeoutStopThdFuzzTest,
    SetOffloadRenderCallbackTypeFuzzTest,
    SetSpeedFuzzTest,
    UpdateAppsUidFuzzTest,
    NotifyStreamChangeToSinkFuzzTest,
    RegisterOffloadCallbackFuzzTest,
    GetOffloadCallbackDataFuzzTest,
    DoProcessFuzzTest,
    NullPointerHandlingFuzzTest,
    BoundaryValuesFuzzTest,
    StateTransitionFuzzTest,
    FullLifecycleFuzzTest,
    SpeedExtremeValuesFuzzTest,
    OffloadCallbackTypeBoundaryFuzzTest,
    MultipleConnectDisconnectFuzzTest,
    ResetWithConnectionsFuzzTest,
    DeviceClassVariationFuzzTest,
    PolicyStateTransitionFuzzTest,
    EmptyAppsUidVectorFuzzTest,
    StreamChangeTypeAllValuesFuzzTest,
    GetOffloadCallbackDataStatesFuzzTest,
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

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
