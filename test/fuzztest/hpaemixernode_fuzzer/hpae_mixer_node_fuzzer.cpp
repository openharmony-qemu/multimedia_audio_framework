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

#include "hpae_format_convert.h"
#include "hpae_mixer_node.h"
#include "hpae_node_common.h"
#include "hpae_pcm_dumper.h"
#include "hpae_process_cluster.h"
#include "hpae_sink_input_node.h"
#include "../fuzz_utils.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960; // 20ms at 48kHz
const uint32_t DEFAULT_CHANNEL_COUNT = 2;
const uint32_t DEFAULT_SAMPLE_RATE = 48000;
const uint32_t TEST_ID = 1243;
typedef void (*TestPtr)();

static void GetTestNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = TEST_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
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

void HpaeMixerNodeSignalProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    if (hpaeMixerNode == nullptr) {
        return;
    }
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    std::vector<HpaePcmBuffer *> inputs;
    inputs.push_back(&hpaePcmBuffer);
    hpaeMixerNode->SignalProcess(inputs);
}

void HpaeMixerNodeCheckUpdateInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    if (hpaeMixerNode == nullptr) {
        return;
    }
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaeMixerNode->CheckUpdateInfo(&hpaePcmBuffer);
}

void HpaeMixerNodeCheckUpdateInfoForDisConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    if (hpaeMixerNode == nullptr) {
        return;
    }
    hpaeMixerNode->pcmBufferInfo_ = CreateBufferInfo(g_fuzzUtils.GetData<uint32_t>());
    hpaeMixerNode->CheckUpdateInfoForDisConnect();
}

void HpaeMixerNodeDrainProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    if (hpaeMixerNode == nullptr) {
        return;
    }
    hpaeMixerNode->DrainProcess();
}

void HpaeNodeCommonTransStreamTypeToSceneTypeFuzzTest()
{
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    TransStreamTypeToSceneType(streamType);
}

void HpaeNodeCommonTransNodeInfoForCollaborationFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    nodeInfo.effectInfo.effectScene = g_fuzzUtils.GetData<AudioEffectScene>();
    bool isCollaborationEnabled = g_fuzzUtils.GetData<bool>();
    TransNodeInfoForCollaboration(nodeInfo, isCollaborationEnabled);
}

void HpaeNodeCommonTransSourceTypeToSceneTypeFuzzTest()
{
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    TransSourceTypeToSceneType(sourceType);
}

void HpaeNodeCommonCheckSceneTypeNeedMicRefFuzzTest()
{
    HpaeProcessorType processorType = g_fuzzUtils.GetData<HpaeProcessorType>();
    CheckSceneTypeNeedEc(processorType);
    CheckSceneTypeNeedMicRef(processorType);
}

void HpaeNodeCommonTransProcessType2EnhanceSceneFuzzTest()
{
    HpaeProcessorType processorType = g_fuzzUtils.GetData<HpaeProcessorType>();
    TransProcessType2EnhanceScene(processorType);
}

void HpaeNodeCommonConvertUsToFrameCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    uint64_t usTime = g_fuzzUtils.GetData<uint64_t>();
    ConvertUsToFrameCount(usTime, nodeInfo);
}

void HpaeNodeCommonConvertDatalenToUsFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    size_t bufferSize = g_fuzzUtils.GetData<size_t>();
    ConvertDatalenToUs(bufferSize, nodeInfo);
}

void HpaeNodeCommonAdjustMchSinkInfoFuzzTest()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.channelLayout = "CH_LAYOUT_STEREO";
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = "DP_MCH_speaker";
    bool isTestString = g_fuzzUtils.GetData<bool>();
    if (isTestString) {
        sinkInfo.deviceName = "test_device_name";
    }

    AdjustMchSinkInfo(audioModuleInfo, sinkInfo);
}

void HpaeNodeCommonCheckSourceInfoIsDifferentFuzzTest()
{
    HpaeSourceInfo oldInfo;
    oldInfo.sourceId = g_fuzzUtils.GetData<uint32_t>();
    oldInfo.deviceNetId = "old_test_device_net_id";
    oldInfo.deviceClass = "old_test_device_class";
    oldInfo.adapterName = "old_test_adapter_name";
    oldInfo.sourceName = "old_test_source_name";
    oldInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    oldInfo.filePath = "old_test_file_path";
    oldInfo.deviceName = "old_test_device_name";
    HpaeSourceInfo info;
    info.sourceId = g_fuzzUtils.GetData<uint32_t>();
    info.deviceNetId = "test_device_net_id";
    info.deviceClass = "test_device_class";
    info.adapterName = "test_adapter_name";
    info.sourceName = "test_source_name";
    info.sourceType = g_fuzzUtils.GetData<SourceType>();
    info.filePath = "test_file_path";
    info.deviceName = "test_device_name";

    CheckSourceInfoIsDifferent(info, oldInfo);
}

void HpaeNodeCommonRecoverNodeInfoForCollaborationFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    nodeInfo.effectInfo.effectScene = g_fuzzUtils.GetData<AudioEffectScene>();

    RecoverNodeInfoForCollaboration(nodeInfo);
}

void HpaeProcessClusterDoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    hpaeProcessCluster->DoProcess();
}

void HpaeProcessClusterGetConverterNodeCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    hpaeProcessCluster->ResetAll();
    hpaeProcessCluster->GetGainNodeCount();
    hpaeProcessCluster->GetConverterNodeCount();
}

void HpaeProcessClusterConnectMixerNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    hpaeProcessCluster->ConnectMixerNode();
}

void HpaeProcessClusterCreateGainNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeNodeInfo preNodeInfo;
    GetTestNodeInfo(preNodeInfo);
    hpaeProcessCluster->CreateGainNode(sessionId, preNodeInfo);
}

void HpaeProcessClusterCreateConverterNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeNodeInfo preNodeInfo;
    GetTestNodeInfo(preNodeInfo);
    hpaeProcessCluster->CreateConverterNode(sessionId, preNodeInfo);
}

void HpaeProcessClusterCreateLoudnessGainNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeNodeInfo preNodeInfo;
    GetTestNodeInfo(preNodeInfo);
    hpaeProcessCluster->CreateLoudnessGainNode(sessionId, preNodeInfo);
}

void HpaeProcessClusterConnectFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }

    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeProcessCluster->Connect(hpaeSinkInputNode);
}

void HpaeProcessClusterDisConnectMixerNodeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    hpaeProcessCluster->renderEffectNode_ = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    hpaeProcessCluster->DisConnectMixerNode();
}

void HpaeProcessClusterGetNodeInputFormatInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    AudioBasicFormat basicFormat;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeProcessCluster->GetNodeInputFormatInfo(sessionId, basicFormat);
}

void HpaeProcessClusterAudioRendererStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    hpaeProcessCluster->AudioRendererStart(nodeInfoTest);
    hpaeProcessCluster->AudioRendererStop(nodeInfoTest);
}

void HpaeProcessClusterGetConverterNodeByIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeProcessCluster->GetGainNodeById(sessionId);
    hpaeProcessCluster->GetConverterNodeById(sessionId);
}

void HpaeProcessClusterSetupAudioLimiterFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    HpaeSinkInfo dummySinkInfo;
    dummySinkInfo.sinkId = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    if (hpaeProcessCluster == nullptr) {
        return;
    }
    hpaeProcessCluster->isConnectedToOutputCluster = g_fuzzUtils.GetData<bool>();
    hpaeProcessCluster->GetConnectedFlag();
    hpaeProcessCluster->SetupAudioLimiter();
}

void HpaeRenderEffectNodeDoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }
    hpaeRenderEffectNode->sceneType_ = "SCENE_COLLABORATIVE";
    hpaeRenderEffectNode->enableProcess_ = g_fuzzUtils.GetData<bool>();
    bool isTestString = g_fuzzUtils.GetData<bool>();
    if (isTestString) {
        hpaeRenderEffectNode->sceneType_ = "test_scene_type";
    }
    hpaeRenderEffectNode->DoProcess();
}

void HpaeRenderEffectNodeSignalProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    std::vector<HpaePcmBuffer *> inputs;
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    inputs.emplace_back(&hpaePcmBuffer);
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        inputs.clear();
    }
    hpaeRenderEffectNode->SignalProcess(inputs);
}

void HpaeRenderEffectNodeSplitCollaborativeDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    hpaeRenderEffectNode->SplitCollaborativeData();
}

void HpaeRenderEffectNodeAudioRendererStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    hpaeRenderEffectNode->AudioRendererStart(nodeInfoTest);
    hpaeRenderEffectNode->AudioRendererStop(nodeInfoTest);
}

void HpaeRenderEffectNodeUpdateAudioEffectChainInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    nodeInfoTest.effectInfo.effectScene = g_fuzzUtils.GetData<AudioEffectScene>();
    hpaeRenderEffectNode->UpdateAudioEffectChainInfo(nodeInfoTest);
}

void HpaeRenderEffectNodeReconfigOutputBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    hpaeRenderEffectNode->ReconfigOutputBuffer();
}

void HpaeRenderEffectNodeGetExpectedInputChannelInfoFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }
    AudioBasicFormat basicFormat;
    basicFormat.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    basicFormat.rate = g_fuzzUtils.GetData<AudioSamplingRate>();

    hpaeRenderEffectNode->GetExpectedInputChannelInfo(basicFormat);
}

void HpaeRenderEffectNodeIsByPassEffectZeroVolumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeRenderEffectNode> hpaeRenderEffectNode = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    if (hpaeRenderEffectNode == nullptr) {
        return;
    }

    hpaeRenderEffectNode->isDisplayEffectZeroVolume_ = g_fuzzUtils.GetData<bool>();
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LENGTH, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaeRenderEffectNode->IsByPassEffectZeroVolume(&hpaePcmBuffer);
}

void ConvertToFloatFuzzTest()
{
    AudioSampleFormat format = g_fuzzUtils.GetData<AudioSampleFormat>();
    float dst[1] = {0};
    if (format == SAMPLE_U8) {
        uint8_t src[1] = {g_fuzzUtils.GetData<uint8_t>()};
        ConvertToFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S16LE) {
        int16_t src[1] = {g_fuzzUtils.GetData<int16_t>()};
        ConvertToFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S24LE) {
        uint8_t src[1] = {g_fuzzUtils.GetData<uint8_t>()};
        ConvertToFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S32LE) {
        int32_t src[1] = {g_fuzzUtils.GetData<int32_t>()};
        ConvertToFloat(format, 1, src, dst);
    } else {
        float src[1] = {g_fuzzUtils.GetData<float>()};
        ConvertToFloat(format, 1, src, dst);
    }
}

void ConvertFromFloatFuzzTest()
{
    AudioSampleFormat format = g_fuzzUtils.GetData<AudioSampleFormat>();
    float src[1] = {g_fuzzUtils.GetData<float>()};
    if (format == SAMPLE_U8) {
        uint8_t dst[1] = {0};
        ConvertFromFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S16LE) {
        int16_t dst[1] = {0};
        ConvertFromFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S24LE) {
        uint8_t dst[4] = {0};
        ConvertFromFloat(format, 1, src, dst);
    } else if (format == SAMPLE_S32LE) {
        int32_t dst[1] = {0};
        ConvertFromFloat(format, 1, src, dst);
    } else {
        float dst[1] = {0};
        ConvertFromFloat(format, 1, src, dst);
    }
}

void HpaePcmDumperDumpFuzzTest()
{
    std::string testFilePath = "/test/test.txt";
    HpaePcmDumper dumper(testFilePath);
    int8_t buffer[] = {0, g_fuzzUtils.GetData<int8_t>()};
    int32_t length = sizeof(buffer) / sizeof(buffer[0]);
    dumper.Dump(buffer, length);
}

void HpaePcmDumperCheckAndReopenHandleFuzzTest()
{
    std::string testFilePath = "/test/test.txt";
    HpaePcmDumper dumper(testFilePath);
    dumper.CheckAndReopenHandle();
}

vector<TestPtr> g_testPtrs = {
    HpaeMixerNodeSignalProcessFuzzTest,
    HpaeMixerNodeCheckUpdateInfoFuzzTest,
    HpaeMixerNodeCheckUpdateInfoForDisConnectFuzzTest,
    HpaeMixerNodeDrainProcessFuzzTest,
    HpaeNodeCommonTransStreamTypeToSceneTypeFuzzTest,
    HpaeNodeCommonTransNodeInfoForCollaborationFuzzTest,
    HpaeNodeCommonTransSourceTypeToSceneTypeFuzzTest,
    HpaeNodeCommonCheckSceneTypeNeedMicRefFuzzTest,
    HpaeNodeCommonTransProcessType2EnhanceSceneFuzzTest,
    HpaeNodeCommonConvertUsToFrameCountFuzzTest,
    HpaeNodeCommonConvertDatalenToUsFuzzTest,
    HpaeNodeCommonAdjustMchSinkInfoFuzzTest,
    HpaeNodeCommonCheckSourceInfoIsDifferentFuzzTest,
    HpaeNodeCommonRecoverNodeInfoForCollaborationFuzzTest,
    HpaeProcessClusterDoProcessFuzzTest,
    HpaeProcessClusterGetConverterNodeCountFuzzTest,
    HpaeProcessClusterConnectMixerNodeFuzzTest,
    HpaeProcessClusterCreateGainNodeFuzzTest,
    HpaeProcessClusterCreateConverterNodeFuzzTest,
    HpaeProcessClusterCreateLoudnessGainNodeFuzzTest,
    HpaeProcessClusterConnectFuzzTest,
    HpaeProcessClusterDisConnectMixerNodeFuzzTest,
    HpaeProcessClusterGetNodeInputFormatInfoFuzzTest,
    HpaeProcessClusterAudioRendererStopFuzzTest,
    HpaeProcessClusterGetConverterNodeByIdFuzzTest,
    HpaeProcessClusterSetupAudioLimiterFuzzTest,
    HpaeRenderEffectNodeDoProcessFuzzTest,
    HpaeRenderEffectNodeSignalProcessFuzzTest,
    HpaeRenderEffectNodeSplitCollaborativeDataFuzzTest,
    HpaeRenderEffectNodeAudioRendererStartFuzzTest,
    HpaeRenderEffectNodeUpdateAudioEffectChainInfoFuzzTest,
    HpaeRenderEffectNodeReconfigOutputBufferFuzzTest,
    HpaeRenderEffectNodeGetExpectedInputChannelInfoFuzzTest,
    HpaeRenderEffectNodeIsByPassEffectZeroVolumeFuzzTest,
    ConvertToFloatFuzzTest,
    ConvertFromFloatFuzzTest,
    HpaePcmDumperDumpFuzzTest,
    HpaePcmDumperCheckAndReopenHandleFuzzTest,
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