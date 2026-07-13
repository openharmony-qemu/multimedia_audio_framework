/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <thread>
#include <chrono>
#include <atomic>

#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_engine_log.h"
#include "hpae_fast_sink_output_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_mixer_node.h"
#include "hpae_gain_node.h"
#include "hpae_info.h"
#include "audio_utils.h"
#include "oh_audio_buffer.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
constexpr size_t FUZZ_INPUT_SIZE_THRESHOLD = 32;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr uint32_t DEFAULT_NODE_ID = 1243;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_SESSION_ID = 100;
constexpr AudioSamplingRate DEFAULT_SAMPLE_RATE = SAMPLE_RATE_48000;
constexpr AudioChannel DEFAULT_CHANNELS = STEREO;
constexpr uint32_t TOTAL_SIZE_IN_FRAME = 8;
constexpr uint32_t SPAN_SIZE_IN_FRAME = 2;
constexpr uint32_t BYTE_SIZE_PER_FRAME = 8;
const char* DEFAULT_TEST_DEVICE_CLASS = "primary_mmap";
const char* DEFAULT_TEST_DEVICE_NETWORKID = "LocalDevice";

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

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
}

static void PrepareFuzzNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = GetData<uint32_t>();
    nodeInfo.frameLen = GetData<uint32_t>();
    RoundVal(nodeInfo.samplingRate, SUPPORTED_SAMPLE_RATES);
    RoundVal(nodeInfo.channels, SUPPORTED_CHANNELS);
    RoundVal(nodeInfo.format, SUPPORTED_FORMATS);
    nodeInfo.sessionId = GetData<uint32_t>();
}

static void SetupMmapBuffer(std::shared_ptr<HpaeFastSinkOutputNode> &node)
{
    auto buffer = OHAudioBuffer::CreateFromLocal(TOTAL_SIZE_IN_FRAME, SPAN_SIZE_IN_FRAME, BYTE_SIZE_PER_FRAME);
    if (buffer == nullptr) {
        return;
    }
    node->dstAudioBuffer_ = buffer;
    node->dstTotalSizeInframe_ = TOTAL_SIZE_IN_FRAME;
    node->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    node->dstByteSizePerFrame_ = BYTE_SIZE_PER_FRAME;
    node->dstSpanSizeInByte_ = SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME;
    node->syncInfoSize_ = 0;
    node->renderFrameData_.resize(node->dstSpanSizeInByte_);
}

static void InitSinkAttr(IAudioSinkAttr &attr)
{
    attr.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.format = SAMPLE_F32LE;
    attr.deviceType = DEVICE_TYPE_SPEAKER;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 0;
    attr.deviceNetworkId = DEFAULT_TEST_DEVICE_NETWORKID;
    attr.filePath = "/data/fast_sink_fuzz_test.pcm";
    attr.audioStreamFlag = AUDIO_FLAG_MMAP;
}

// ============================================================================
// Constructor Tests
// ============================================================================

void HpaeFastSinkOutputNodeConstructFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

void HpaeFastSinkOutputNodeConstructWithFuzzInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareFuzzNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

void HpaeFastSinkOutputNodeConstructWithUltraFastFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "ultra_fast";
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

void HpaeFastSinkOutputNodeConstructWithVoipFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "primary_mmap_voip";
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

void HpaeFastSinkOutputNodeConstructWithEmptyDeviceClassFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "";
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

void HpaeFastSinkOutputNodeConstructZeroSamplingRateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_8000;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
}

// ============================================================================
// Destructor Test
// ============================================================================

void HpaeFastSinkOutputNodeDestructFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    {
        auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    }
}

// ============================================================================
// State Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetSetSinkStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    StreamManagerState states[] = {
        STREAM_MANAGER_NEW,
        STREAM_MANAGER_IDLE,
        STREAM_MANAGER_RUNNING,
        STREAM_MANAGER_SUSPENDED,
        STREAM_MANAGER_RELEASED,
    };
    uint32_t idx = GetData<uint32_t>() % 5;
    node->SetSinkState(states[idx]);
    StreamManagerState result = node->GetSinkState();
}

void HpaeFastSinkOutputNodeGetSinkStateDefaultFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->GetSinkState();
}

// ============================================================================
// SetTimeoutStopThd Tests
// ============================================================================

void HpaeFastSinkOutputNodeSetTimeoutStopThdValidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    int64_t timeoutMs = GetData<int64_t>();
    if (timeoutMs < 0) {
        timeoutMs = 0;
    }
    node->SetTimeoutStopThd(timeoutMs);
}

void HpaeFastSinkOutputNodeSetTimeoutStopThdNegativeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    int64_t timeoutMs = -1;
    node->SetTimeoutStopThd(timeoutMs);
}

void HpaeFastSinkOutputNodeSetTimeoutStopThdZeroFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->SetTimeoutStopThd(0);
}

void HpaeFastSinkOutputNodeSetTimeoutStopThdLargeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->SetTimeoutStopThd(INT64_MAX);
}

// ============================================================================
// UpdateAppsUid Tests
// ============================================================================

void HpaeFastSinkOutputNodeUpdateAppsUidNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid = {GetData<int32_t>()};
    node->UpdateAppsUid(appsUid);
}

void HpaeFastSinkOutputNodeUpdateAppsUidEmptyFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid;
    node->UpdateAppsUid(appsUid);
}

void HpaeFastSinkOutputNodeUpdateAppsUidLargeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid;
    size_t count = GetData<uint32_t>() % 50;
    for (size_t i = 0; i < count; ++i) {
        appsUid.push_back(GetData<int32_t>());
    }
    node->UpdateAppsUid(appsUid);
}

// ============================================================================
// SetNeedCheckZeroVolume Tests
// ============================================================================

void HpaeFastSinkOutputNodeSetNeedCheckZeroVolumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    bool needCheck = GetData<bool>();
    node->SetNeedCheckZeroVolume(needCheck);
}

// ============================================================================
// SetLoopbackState Tests
// ============================================================================

void HpaeFastSinkOutputNodeSetLoopbackStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    bool isExistLoopback = GetData<bool>();
    node->SetLoopbackState(isExistLoopback);
}

// ============================================================================
// SetMuteForSwitchDevice Tests
// ============================================================================

void HpaeFastSinkOutputNodeSetMuteForSwitchDeviceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    bool mute = GetData<bool>();
    node->SetMuteForSwitchDevice(mute);
}

void HpaeFastSinkOutputNodeSetMuteForSwitchDeviceSameStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    // Set same state twice (no-op)
    node->SetMuteForSwitchDevice(false);
    node->SetMuteForSwitchDevice(false);
}

// ============================================================================
// NotifyStreamChangeToSink Tests
// ============================================================================

void HpaeFastSinkOutputNodeNotifyStreamChangeNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;

    StreamChangeType change = static_cast<StreamChangeType>(GetData<int32_t>());
    uint32_t sessionId = GetData<uint32_t>();
    StreamUsage usage = STREAM_USAGE_UNKNOWN;
    RendererState state = static_cast<RendererState>(GetData<int32_t>());
    uint32_t appUid = GetData<uint32_t>();
    node->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}

// ============================================================================
// GetLatency Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetLatencyNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->GetLatency();
}

// ============================================================================
// GetMaxAmplitude Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetMaxAmplitudeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->maxAmplitude_ = GetData<float>();
    node->GetMaxAmplitude();
}

// ============================================================================
// GetSpanSizeInFrame Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetSpanSizeInFrameFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstSpanSizeInframe_ = GetData<uint32_t>();
    node->GetSpanSizeInFrame();
}

// ============================================================================
// RefreshSpanSize Tests
// ============================================================================

void HpaeFastSinkOutputNodeRefreshSpanSizeNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    bool isUpdated = false;
    node->RefreshSpanSize(isUpdated);
}

// ============================================================================
// GetPreOutNum Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetPreOutNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->GetPreOutNum();
}

// ============================================================================
// Connect / DisConnect Tests
// ============================================================================

void HpaeFastSinkOutputNodeConnectDisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);

    node->Connect(mixerNode);
    node->GetPreOutNum();

    node->DisConnect(mixerNode);
    node->GetPreOutNum();
}

void HpaeFastSinkOutputNodeConnectMultipleFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);

    node->Connect(mixerNode);

    // Try connecting gain node as second pre-output
    HpaeNodeInfo gainInfo;
    gainInfo.frameLen = DEFAULT_FRAME_LEN;
    gainInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    gainInfo.channels = DEFAULT_CHANNELS;
    gainInfo.format = SAMPLE_F32LE;
    auto gainNode = std::make_shared<HpaeGainNode>(gainInfo);

    node->Connect(gainNode);
    size_t preOutNum = node->GetPreOutNum();

    node->DisConnect(mixerNode);
    node->DisConnect(gainNode);
}

// ============================================================================
// Reset / ResetAll Tests
// ============================================================================

void HpaeFastSinkOutputNodeResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->Reset();
}

void HpaeFastSinkOutputNodeResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->ResetAll();
}

void HpaeFastSinkOutputNodeResetWithConnectionsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);

    node->Connect(mixerNode);
    node->Reset();
}

void HpaeFastSinkOutputNodeResetAllWithConnectionsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);

    node->Connect(mixerNode);
    node->ResetAll();
}

// ============================================================================
// RenderSink Lifecycle Tests (Null Sink)
// ============================================================================

void HpaeFastSinkOutputNodeRenderSinkInitNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;

    IAudioSinkAttr attr;
    InitSinkAttr(attr);
    node->RenderSinkInit(attr);
}

void HpaeFastSinkOutputNodeRenderSinkStartNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->RenderSinkStart();
}

void HpaeFastSinkOutputNodeRenderSinkStopNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->RenderSinkStop();
}

void HpaeFastSinkOutputNodeRenderSinkDeInitNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->RenderSinkDeInit();
}

void HpaeFastSinkOutputNodeRenderSinkFlushNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->RenderSinkFlush();
}

void HpaeFastSinkOutputNodeRenderSinkFlushNullBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstAudioBuffer_ = nullptr;
    node->RenderSinkFlush();
}

// ============================================================================
// GetRenderSinkInstance Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetRenderSinkInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->GetRenderSinkInstance(DEFAULT_TEST_DEVICE_CLASS, DEFAULT_TEST_DEVICE_NETWORKID);
}

void HpaeFastSinkOutputNodeGetRenderSinkInstanceEmptyNetIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->GetRenderSinkInstance(DEFAULT_TEST_DEVICE_CLASS, "");
}

void HpaeFastSinkOutputNodeGetRenderSinkInstanceInvalidDeviceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    std::string deviceClass = GetData<bool>() ? "invalid_class" : DEFAULT_TEST_DEVICE_CLASS;
    std::string deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    node->GetRenderSinkInstance(deviceClass, deviceNetId);
}

// ============================================================================
// GetRenderFrameData Tests
// ============================================================================

void HpaeFastSinkOutputNodeGetRenderFrameDataEmptyFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->GetRenderFrameData();
}

void HpaeFastSinkOutputNodeGetRenderFrameDataWithDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    size_t dataSize = GetData<uint32_t>() % 1000 + 1;
    node->renderFrameData_.resize(dataSize);
    node->GetRenderFrameData();
}

// ============================================================================
// DoProcess Tests (Null State)
// ============================================================================

void HpaeFastSinkOutputNodeDoProcessNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->DoProcess();
}

void HpaeFastSinkOutputNodeDoProcessNullBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstAudioBuffer_ = nullptr;
    node->DoProcess();
}

void HpaeFastSinkOutputNodeDoProcessSuspendCheckFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    SetupMmapBuffer(node);
    // No pre-output connected, CheckIfSuspend returns true
    node->DoProcess();
}

// ============================================================================
// TriggerPrepareNextLoop Tests
// ============================================================================

void HpaeFastSinkOutputNodeTriggerPrepareNextLoopNullBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstAudioBuffer_ = nullptr;
    node->TriggerPrepareNextLoop();
}

void HpaeFastSinkOutputNodeTriggerPrepareNextLoopWithBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    SetupMmapBuffer(node);
    node->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    node->TriggerPrepareNextLoop();
}

// ============================================================================
// WriteToDeviceBuffer Tests (Null/Boundary)
// ============================================================================

void HpaeFastSinkOutputNodeWriteToDeviceBufferNullBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstAudioBuffer_ = nullptr;
    HpaePcmBuffer *pcmBuffer = nullptr;
    node->WriteToDeviceBuffer(pcmBuffer, 0);
}

void HpaeFastSinkOutputNodeWriteToDeviceBufferSpanZeroFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    SetupMmapBuffer(node);
    node->dstSpanSizeInByte_ = 0;
    HpaePcmBuffer *pcmBuffer = nullptr;
    node->WriteToDeviceBuffer(pcmBuffer, 0);
}

void HpaeFastSinkOutputNodeWriteToDeviceBufferPcmNullFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    SetupMmapBuffer(node);
    HpaePcmBuffer *pcmBuffer = nullptr;
    node->WriteToDeviceBuffer(pcmBuffer, 0);
}

// ============================================================================
// ClientWakeCallback Tests
// ============================================================================

void HpaeFastSinkOutputNodeSetClientWakeCallbackFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    bool callbackInvoked = false;
    node->SetClientWakeCallback([&callbackInvoked]() { callbackInvoked = true; });
    if (node->clientWakeCallback_) {
        node->clientWakeCallback_();
    }
}

// ============================================================================
// CheckIfSuspend Tests
// ============================================================================

void HpaeFastSinkOutputNodeCheckIfSuspendNoPreOutFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->CheckIfSuspend();
}

void HpaeFastSinkOutputNodeCheckIfSuspendWithPreOutFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    node->Connect(mixerNode);

    node->CheckIfSuspend();
}

void HpaeFastSinkOutputNodeCheckIfSuspendTimeoutFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->SetSinkState(STREAM_MANAGER_RUNNING);
    node->timeoutThdFrames_ = GetData<uint32_t>() % 100 + 1;
    node->CheckIfSuspend();
}

// ============================================================================
// IsCheckingSuspend Tests
// ============================================================================

void HpaeFastSinkOutputNodeIsCheckingSuspendFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->IsCheckingSuspend();
    node->isCheckingSuspend_ = GetData<bool>();
    node->IsCheckingSuspend();
}

// ============================================================================
// ZeroVolumeCheck Tests (various device types)
// ============================================================================

void HpaeFastSinkOutputNodeZeroVolumeCheckDisabledFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->needCheckZeroVolume_ = false;
    node->ZeroVolumeCheck(0);
}

void HpaeFastSinkOutputNodeZeroVolumeCheckBluetoothA2dpFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->needCheckZeroVolume_ = true;
    node->isExistLoopback_ = false;
    node->currentOutputDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    node->ZeroVolumeCheck(0);
}

void HpaeFastSinkOutputNodeZeroVolumeCheckNearlinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->needCheckZeroVolume_ = true;
    node->isExistLoopback_ = false;
    node->currentOutputDevice_ = DEVICE_TYPE_NEARLINK;
    node->ZeroVolumeCheck(0);
}

void HpaeFastSinkOutputNodeZeroVolumeCheckSpeakerFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->needCheckZeroVolume_ = true;
    node->isExistLoopback_ = false;
    node->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    int32_t vol = GetData<int32_t>();
    node->ZeroVolumeCheck(vol);
}

void HpaeFastSinkOutputNodeZeroVolumeCheckLoopbackOverrideFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->needCheckZeroVolume_ = true;
    node->isExistLoopback_ = true;
    node->ZeroVolumeCheck(1);
}

void HpaeFastSinkOutputNodeZeroVolumeCheckVoipFastFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "primary_mmap_voip";
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->sinkOutAttr_.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
    node->needCheckZeroVolume_ = true;
    node->ZeroVolumeCheck(GetData<int32_t>());
}

// ============================================================================
// InitTransBuffer Tests
// ============================================================================

void HpaeFastSinkOutputNodeInitTransBufferValidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstSpanSizeInframe_ = GetData<uint32_t>() % 480 + 1;
    node->dstByteSizePerFrame_ = GetData<uint32_t>() % 32 + 1;
    node->InitTransBuffer();
}

void HpaeFastSinkOutputNodeInitTransBufferZeroFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstSpanSizeInframe_ = 0;
    node->dstByteSizePerFrame_ = 0;
    node->InitTransBuffer();
}

void HpaeFastSinkOutputNodeInitTransBufferVeryLargeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->dstSpanSizeInframe_ = UINT32_MAX;
    node->dstByteSizePerFrame_ = UINT32_MAX;
    node->InitTransBuffer();
}

// ============================================================================
// SyncCurrentOutputDevice Tests
// ============================================================================

void HpaeFastSinkOutputNodeSyncCurrentOutputDeviceNullSinkFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    node->audioRendererSink_ = nullptr;
    node->SyncCurrentOutputDevice();
}

// ============================================================================
// IsInvalidBuffer Tests (via UpdateSilentState)
// ============================================================================

void HpaeFastSinkOutputNodeUpdateSilentStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.format = GetData<bool>() ? SAMPLE_S16LE : SAMPLE_U8;
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    size_t bufferSize = GetData<uint32_t>() % 100 + 2;
    std::vector<uint8_t> buffer(bufferSize);
    for (size_t i = 0; i < bufferSize; ++i) {
        buffer[i] = GetData<uint8_t>();
    }
    node->sinkOutAttr_.format = nodeInfo.format;
    node->UpdateSilentState(buffer.data(), bufferSize);
}

// ============================================================================
// Combined Multi-Operation Tests
// ============================================================================

void HpaeFastSinkOutputNodeMultiStateTransitionFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    // State transitions
    StreamManagerState states[] = {
        STREAM_MANAGER_NEW,
        STREAM_MANAGER_IDLE,
        STREAM_MANAGER_RUNNING,
        STREAM_MANAGER_SUSPENDED,
        STREAM_MANAGER_RELEASED,
    };

    for (int i = 0; i < 5; i++) {
        node->SetSinkState(states[i]);
    }

    // ZeroVolume state transitions
    node->needCheckZeroVolume_ = true;
    node->isExistLoopback_ = false;
    node->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    node->ZeroVolumeCheck(0);  // enter IN_TIMING
    node->ZeroVolumeCheck(0);  // still IN_TIMING (or ACTIVE if time elapsed)
    node->ZeroVolumeCheck(1000);  // reset to INACTIVE

    // Loopback state transitions
    node->SetLoopbackState(true);
    node->SetLoopbackState(false);

    // Mute state transitions
    node->SetMuteForSwitchDevice(true);
    node->SetMuteForSwitchDevice(false);
    node->SetMuteForSwitchDevice(false); // same state, no-op
}

void HpaeFastSinkOutputNodeConnectDisconnectResetSequenceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;

    // Connect then reset
    auto mixerNode1 = std::make_shared<HpaeMixerNode>(mixerInfo);
    node->Connect(mixerNode1);
    node->Reset();

    // Connect then resetAll
    auto mixerNode2 = std::make_shared<HpaeMixerNode>(mixerInfo);
    node->Connect(mixerNode2);
    node->ResetAll();
}

void HpaeFastSinkOutputNodeAllNullStateOperationsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);

    // All null states - should not crash
    node->audioRendererSink_ = nullptr;
    node->dstAudioBuffer_ = nullptr;

    IAudioSinkAttr attr;
    InitSinkAttr(attr);
    node->RenderSinkInit(attr);
    node->RenderSinkStart();
    node->RenderSinkStop();
    node->RenderSinkDeInit();
    node->RenderSinkFlush();
    node->GetLatency();
    node->UpdateAppsUid(std::vector<int32_t>{1000});
    node->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 1, STREAM_USAGE_UNKNOWN, RENDERER_RUNNING);
    node->DoProcess();
    bool isUpdated = false;
    node->RefreshSpanSize(isUpdated);
}

typedef void (*TestFuncs)();
TestFuncs g_testFuncs[] = {
    HpaeFastSinkOutputNodeConstructFuzzTest,
    HpaeFastSinkOutputNodeConstructWithFuzzInfoFuzzTest,
    HpaeFastSinkOutputNodeConstructWithUltraFastFuzzTest,
    HpaeFastSinkOutputNodeConstructWithVoipFuzzTest,
    HpaeFastSinkOutputNodeConstructWithEmptyDeviceClassFuzzTest,
    HpaeFastSinkOutputNodeConstructZeroSamplingRateFuzzTest,
    HpaeFastSinkOutputNodeDestructFuzzTest,
    HpaeFastSinkOutputNodeGetSetSinkStateFuzzTest,
    HpaeFastSinkOutputNodeGetSinkStateDefaultFuzzTest,
    HpaeFastSinkOutputNodeSetTimeoutStopThdValidFuzzTest,
    HpaeFastSinkOutputNodeSetTimeoutStopThdNegativeFuzzTest,
    HpaeFastSinkOutputNodeSetTimeoutStopThdZeroFuzzTest,
    HpaeFastSinkOutputNodeSetTimeoutStopThdLargeFuzzTest,
    HpaeFastSinkOutputNodeUpdateAppsUidNullSinkFuzzTest,
    HpaeFastSinkOutputNodeUpdateAppsUidEmptyFuzzTest,
    HpaeFastSinkOutputNodeUpdateAppsUidLargeFuzzTest,
    HpaeFastSinkOutputNodeSetNeedCheckZeroVolumeFuzzTest,
    HpaeFastSinkOutputNodeSetLoopbackStateFuzzTest,
    HpaeFastSinkOutputNodeSetMuteForSwitchDeviceFuzzTest,
    HpaeFastSinkOutputNodeSetMuteForSwitchDeviceSameStateFuzzTest,
    HpaeFastSinkOutputNodeNotifyStreamChangeNullSinkFuzzTest,
    HpaeFastSinkOutputNodeGetLatencyNullSinkFuzzTest,
    HpaeFastSinkOutputNodeGetMaxAmplitudeFuzzTest,
    HpaeFastSinkOutputNodeGetSpanSizeInFrameFuzzTest,
    HpaeFastSinkOutputNodeRefreshSpanSizeNullSinkFuzzTest,
    HpaeFastSinkOutputNodeGetPreOutNumFuzzTest,
    HpaeFastSinkOutputNodeConnectDisConnectFuzzTest,
    HpaeFastSinkOutputNodeConnectMultipleFuzzTest,
    HpaeFastSinkOutputNodeResetFuzzTest,
    HpaeFastSinkOutputNodeResetAllFuzzTest,
    HpaeFastSinkOutputNodeResetWithConnectionsFuzzTest,
    HpaeFastSinkOutputNodeResetAllWithConnectionsFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkInitNullSinkFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkStartNullSinkFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkStopNullSinkFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkDeInitNullSinkFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkFlushNullSinkFuzzTest,
    HpaeFastSinkOutputNodeRenderSinkFlushNullBufferFuzzTest,
    HpaeFastSinkOutputNodeGetRenderSinkInstanceFuzzTest,
    HpaeFastSinkOutputNodeGetRenderSinkInstanceEmptyNetIdFuzzTest,
    HpaeFastSinkOutputNodeGetRenderSinkInstanceInvalidDeviceFuzzTest,
    HpaeFastSinkOutputNodeGetRenderFrameDataEmptyFuzzTest,
    HpaeFastSinkOutputNodeGetRenderFrameDataWithDataFuzzTest,
    HpaeFastSinkOutputNodeDoProcessNullSinkFuzzTest,
    HpaeFastSinkOutputNodeDoProcessNullBufferFuzzTest,
    HpaeFastSinkOutputNodeDoProcessSuspendCheckFuzzTest,
    HpaeFastSinkOutputNodeTriggerPrepareNextLoopNullBufferFuzzTest,
    HpaeFastSinkOutputNodeTriggerPrepareNextLoopWithBufferFuzzTest,
    HpaeFastSinkOutputNodeWriteToDeviceBufferNullBufferFuzzTest,
    HpaeFastSinkOutputNodeWriteToDeviceBufferSpanZeroFuzzTest,
    HpaeFastSinkOutputNodeWriteToDeviceBufferPcmNullFuzzTest,
    HpaeFastSinkOutputNodeSetClientWakeCallbackFuzzTest,
    HpaeFastSinkOutputNodeCheckIfSuspendNoPreOutFuzzTest,
    HpaeFastSinkOutputNodeCheckIfSuspendWithPreOutFuzzTest,
    HpaeFastSinkOutputNodeCheckIfSuspendTimeoutFuzzTest,
    HpaeFastSinkOutputNodeIsCheckingSuspendFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckDisabledFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckBluetoothA2dpFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckNearlinkFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckSpeakerFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckLoopbackOverrideFuzzTest,
    HpaeFastSinkOutputNodeZeroVolumeCheckVoipFastFuzzTest,
    HpaeFastSinkOutputNodeInitTransBufferValidFuzzTest,
    HpaeFastSinkOutputNodeInitTransBufferZeroFuzzTest,
    HpaeFastSinkOutputNodeInitTransBufferVeryLargeFuzzTest,
    HpaeFastSinkOutputNodeSyncCurrentOutputDeviceNullSinkFuzzTest,
    HpaeFastSinkOutputNodeUpdateSilentStateFuzzTest,
    HpaeFastSinkOutputNodeMultiStateTransitionFuzzTest,
    HpaeFastSinkOutputNodeConnectDisconnectResetSequenceFuzzTest,
    HpaeFastSinkOutputNodeAllNullStateOperationsFuzzTest,
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
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
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
