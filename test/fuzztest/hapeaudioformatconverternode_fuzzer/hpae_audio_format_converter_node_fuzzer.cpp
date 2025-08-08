/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <fstream>
#include <securec.h>

#include "hpae_audio_format_converter_node.h"
#include "hpae_capture_effect_node.h"
#include "hpae_co_buffer_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_source_input_node.h"
#include "../fuzz_utils.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960; // 20ms at 48kHz
const uint32_t DEFAULT_NODE_ID = 1243;
const uint32_t DEFAULT_CHANNEL_COUNT = 2;
const uint32_t DEFAULT_SAMPLE_RATE = 48000;
typedef void (*TestPtr)();

class INodeFormatInfoCallbackTest : public INodeFormatInfoCallback {
public:
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override
    {
        return 0;
    }
};

static void CreateHpaeAudioFormatConverterNode(std::shared_ptr<HpaeAudioFormatConverterNode> &nodePtr)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.channels = CHANNEL_UNKNOW;
    preNodeInfo.frameLen = 0;
    preNodeInfo.nodeId = g_fuzzUtils.GetData<uint32_t>();
    preNodeInfo.samplingRate = SAMPLE_RATE_8000;
    preNodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    preNodeInfo.deviceClass = "test_device_class";
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    nodePtr = make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
}

static void GetTestNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
}

PcmBufferInfo CreateBufferInfo(uint32_t frames, bool multiFrames = false)
{
    PcmBufferInfo info;
    info.ch = DEFAULT_CHANNEL_COUNT;
    info.frameLen = DEFAULT_FRAME_LENGTH;
    info.rate = DEFAULT_SAMPLE_RATE;
    info.frames = frames;
    info.isMultiFrames = multiFrames;
    return info;
}

void HpaeAudioFormatConverterNodeRegisterCallbackFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    INodeFormatInfoCallbackTest callback;
    INodeFormatInfoCallback *callbackPtr = &callback;
    nodePtr->RegisterCallback(callbackPtr);
}

void HpaeAudioFormatConverterNodeSignalProcessFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    std::vector<HpaePcmBuffer *> inputs;
    inputs.push_back(&hpaePcmBuffer);
    nodePtr->SignalProcess(inputs);
}

void HpaeAudioFormatConverterNodeConverterProcessFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    float *dstData = nodePtr->converterOutput_.GetPcmDataBuffer();
    float *tmpData = nodePtr->tmpOutBuf_.GetPcmDataBuffer();
    PcmBufferInfo pcmBufferInfo = CreateBufferInfo(1);
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    float *srcData = inputBuffer.GetPcmDataBuffer();
    nodePtr->ConverterProcess(srcData, dstData, tmpData, &inputBuffer);
}

void HpaeAudioFormatConverterNodeCheckUpdateOutInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    INodeFormatInfoCallbackTest callback;
    INodeFormatInfoCallback *callbackPtr = &callback;
    nodePtr->RegisterCallback(callbackPtr);
    nodePtr->CheckUpdateOutInfo();
}

void HpaeAudioFormatConverterNodeCheckUpdateInInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    nodePtr->CheckUpdateInInfo(&inputBuffer);
}

void HpaeAudioFormatConverterNodeUpdateTmpOutPcmBufferInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    PcmBufferInfo outPcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    nodePtr->UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);
}

void HpaeAudioFormatConverterNodeCheckAndUpdateInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer inputBuffer(pcmBufferInfo);
    nodePtr->CheckAndUpdateInfo(&inputBuffer);
}

void HpaeAudioFormatConverterNodeConnectWithInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSourceInputNode;

    HpaeNodeInfo nodeInfoTest;
    nodePtr->ConnectWithInfo(outputNode, nodeInfoTest);
}

void HpaeAudioFormatConverterNodeDisConnectWithInfoFuzzTest()
{
    std::shared_ptr<HpaeAudioFormatConverterNode> nodePtr;
    CreateHpaeAudioFormatConverterNode(nodePtr);
    if (nodePtr == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::shared_ptr<OutputNode<HpaePcmBuffer *>> outputNode = hpaeSourceInputNode;

    HpaeNodeInfo nodeInfoTest;
    nodePtr->DisConnectWithInfo(outputNode, nodeInfoTest);
}

void HpaeCaptureEffectNodeResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    if (hpaeCaptureEffectNode == nullptr) {
        return;
    }
    hpaeCaptureEffectNode->Reset();
}

void HpaeCaptureEffectNodeSignalProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    if (hpaeCaptureEffectNode == nullptr) {
        return;
    }
    PcmBufferInfo outPcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    hpaeCaptureEffectNode->outPcmBuffer_ = make_unique<HpaePcmBuffer>(outPcmBufferInfo);
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    std::vector<HpaePcmBuffer *> inputs;
    inputs.push_back(&hpaePcmBuffer);
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        inputs.clear();
    }
    hpaeCaptureEffectNode->SignalProcess(inputs);
}

void HpaeCaptureEffectNodeGetCapturerEffectConfigFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    if (hpaeCaptureEffectNode == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    HpaeSourceBufferType type = g_fuzzUtils.GetData<HpaeSourceBufferType>();
    hpaeCaptureEffectNode->capturerEffectConfigMap_.insert({type, nodeInfoTest});
    HpaeNodeInfo hpaeNodeInfoByGetCapturerEffectConfig;
    hpaeCaptureEffectNode->GetCapturerEffectConfig(hpaeNodeInfoByGetCapturerEffectConfig, type);
}

void HpaeCaptureEffectNodeSetCapturerEffectConfigFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeCaptureEffectNode> hpaeCaptureEffectNode = std::make_shared<HpaeCaptureEffectNode>(nodeInfo);
    if (hpaeCaptureEffectNode == nullptr) {
        return;
    }
    AudioBufferConfig audioBufferConfig1 = {
        .samplingRate = g_fuzzUtils.GetData<uint32_t>(),
        .channels = g_fuzzUtils.GetData<uint32_t>(),
        .format = g_fuzzUtils.GetData<uint8_t>(),
    };
    AudioBufferConfig audioBufferConfig2 = {
        .samplingRate = g_fuzzUtils.GetData<uint32_t>(),
        .channels = g_fuzzUtils.GetData<uint32_t>(),
        .format = g_fuzzUtils.GetData<uint8_t>(),
    };
    AudioBufferConfig audioBufferConfig3 = {
        .samplingRate = g_fuzzUtils.GetData<uint32_t>(),
        .channels = g_fuzzUtils.GetData<uint32_t>(),
        .format = g_fuzzUtils.GetData<uint8_t>(),
    };
    hpaeCaptureEffectNode->SetCapturerEffectConfig(audioBufferConfig1, audioBufferConfig2, audioBufferConfig3);
}

void HpaeCoBufferNodeEnqueueFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }

    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    coBufferNode->Enqueue(&hpaePcmBuffer);
}

void HpaeCoBufferNodeDoProcessFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    coBufferNode->enqueueRunning_ = g_fuzzUtils.GetData<bool>();
    coBufferNode->DoProcess();
}

void HpaeCoBufferNodeResetFuzzTest()
{
    HpaeCoBufferNode hpaeCoBufferNode;
    std::shared_ptr<OutputNode<HpaePcmBuffer*>> preNode = make_shared<HpaeCoBufferNode>();
    if (preNode == nullptr) {
        return;
    }
    preNode->GetOutputPort();
    hpaeCoBufferNode.inputStream_.outputPorts_.insert({preNode->GetOutputPort(), preNode->GetSharedInstance()});
    hpaeCoBufferNode.Reset();
}

void HpaeCoBufferNodeResetAllFuzzTest()
{
    HpaeCoBufferNode hpaeCoBufferNode;
    std::shared_ptr<OutputNode<HpaePcmBuffer*>> preNode = make_shared<HpaeCoBufferNode>();
    if (preNode == nullptr) {
        return;
    }
    preNode->GetOutputPort();
    hpaeCoBufferNode.inputStream_.outputPorts_.insert({preNode->GetOutputPort(), preNode->GetSharedInstance()});
    hpaeCoBufferNode.ResetAll();
}

void HpaeCoBufferNodeGetOutputPortFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    coBufferNode->GetOutputPort();
}

void HpaeCoBufferNodeConnectFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    HpaeNodeInfo sinkInputNodeInfo;
    GetTestNodeInfo(sinkInputNodeInfo);
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode = std::make_shared<HpaeSinkInputNode>(sinkInputNodeInfo);
    if (sinkInputNode == nullptr) {
        return;
    }

    coBufferNode->connectedProcessCluster_.insert(sinkInputNode->GetNodeInfo().sceneType);
    coBufferNode->Connect(sinkInputNode);
}

void HpaeCoBufferNodeDisConnectFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    HpaeNodeInfo sinkInputNodeInfo;
    GetTestNodeInfo(sinkInputNodeInfo);
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode = std::make_shared<HpaeSinkInputNode>(sinkInputNodeInfo);
    coBufferNode->DisConnect(sinkInputNode);
}

void HpaeCoBufferNodeSetLatencyFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    uint32_t latency = g_fuzzUtils.GetData<uint32_t>();
    coBufferNode->SetLatency(latency);
}

void HpaeCoBufferNodeFillSilenceFramesInnerFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    uint32_t latencyMs = g_fuzzUtils.GetData<uint32_t>();

    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    coBufferNode->ringCache_ = AudioRingCache::Create(cacheSize);
    coBufferNode->FillSilenceFramesInner(latencyMs);
}

void HpaeCoBufferNodeProcessInputFrameInnerFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }

    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    coBufferNode->ringCache_ = AudioRingCache::Create(cacheSize);

    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    coBufferNode->ProcessInputFrameInner(&hpaePcmBuffer);
}

void HpaeCoBufferNodeProcessOutputFrameInnerFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }
    size_t cacheSize = g_fuzzUtils.GetData<size_t>();
    coBufferNode->ringCache_ = AudioRingCache::Create(cacheSize);
    coBufferNode->ProcessOutputFrameInner();
}

void HpaeCoBufferNodeSetOutputClusterConnectedFuzzTest()
{
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    if (coBufferNode == nullptr) {
        return;
    }

    bool isConnect = g_fuzzUtils.GetData<bool>();
    coBufferNode->SetOutputClusterConnected(isConnect);
    coBufferNode->IsOutputClusterConnected();
}

vector<TestPtr> g_testPtrs = {
    HpaeAudioFormatConverterNodeRegisterCallbackFuzzTest,
    HpaeAudioFormatConverterNodeSignalProcessFuzzTest,
    HpaeAudioFormatConverterNodeConverterProcessFuzzTest,
    HpaeAudioFormatConverterNodeCheckUpdateOutInfoFuzzTest,
    HpaeAudioFormatConverterNodeCheckUpdateInInfoFuzzTest,
    HpaeAudioFormatConverterNodeUpdateTmpOutPcmBufferInfoFuzzTest,
    HpaeAudioFormatConverterNodeCheckAndUpdateInfoFuzzTest,
    HpaeAudioFormatConverterNodeConnectWithInfoFuzzTest,
    HpaeAudioFormatConverterNodeDisConnectWithInfoFuzzTest,
    HpaeCaptureEffectNodeResetFuzzTest,
    HpaeCaptureEffectNodeSignalProcessFuzzTest,
    HpaeCaptureEffectNodeGetCapturerEffectConfigFuzzTest,
    HpaeCaptureEffectNodeSetCapturerEffectConfigFuzzTest,
    HpaeCoBufferNodeEnqueueFuzzTest,
    HpaeCoBufferNodeDoProcessFuzzTest,
    HpaeCoBufferNodeResetFuzzTest,
    HpaeCoBufferNodeResetAllFuzzTest,
    HpaeCoBufferNodeGetOutputPortFuzzTest,
    HpaeCoBufferNodeConnectFuzzTest,
    HpaeCoBufferNodeDisConnectFuzzTest,
    HpaeCoBufferNodeSetLatencyFuzzTest,
    HpaeCoBufferNodeFillSilenceFramesInnerFuzzTest,
    HpaeCoBufferNodeProcessInputFrameInnerFuzzTest,
    HpaeCoBufferNodeProcessOutputFrameInnerFuzzTest,
    HpaeCoBufferNodeSetOutputClusterConnectedFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testPtrs);
    return 0;
}