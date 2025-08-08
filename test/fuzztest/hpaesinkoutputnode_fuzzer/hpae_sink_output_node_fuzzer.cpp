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

#include "hdi_adapter_factory.h"
#include "hpae_sink_output_node.h"
#include "hpae_source_input_cluster.h"
#include "../fuzz_utils.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t DEFAULT_FRAME_LENGTH = 960; // 20ms at 48kHz
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

void HpaeSinkOutputNodeHandleRemoteTimingFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "test_device_class";
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    hpaeSinkOutputNode->HandleRemoteTiming();
}

void HpaeSinkOutputNodeDoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "test_device_class";
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->DoProcess();
}

void HpaeSinkOutputNodeGetRenderFrameDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    hpaeSinkOutputNode->GetRenderFrameData();
}

void HpaeSinkOutputNodeResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    hpaeSinkOutputNode->Reset();
}

void HpaeSinkOutputNodeRenderSinkFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->RenderSinkFlush();
}

void HpaeSinkOutputNodeRenderSinkPauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->RenderSinkPause();
}

void HpaeSinkOutputNodeRenderSinkResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->RenderSinkReset();
}

void HpaeSinkOutputNodeRenderSinkStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->RenderSinkStart();
}

void HpaeSinkOutputNodeUpdateAppsUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    std::vector<int32_t> appsUid;
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    appsUid.push_back(uid);
    hpaeSinkOutputNode->UpdateAppsUid(appsUid);
    hpaeSinkOutputNode->GetPreOutNum();
}

void HpaeSinkOutputNodeHandlePaPowerFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    IAudioSinkAttr attr;
    attr.adapterName = "primary";
    attr.openMicSpeaker = g_fuzzUtils.GetData<uint32_t>();
    attr.format = nodeInfo.format;
    attr.sampleRate = nodeInfo.samplingRate;
    attr.channel = nodeInfo.channels;

    hpaeSinkOutputNode->RenderSinkInit(attr);
    PcmBufferInfo pcmBufferInfo(0, 1, g_fuzzUtils.GetData<uint32_t>());
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaeSinkOutputNode->isDisplayPaPowerState_ = g_fuzzUtils.GetData<bool>();
    hpaeSinkOutputNode->HandlePaPower(&hpaePcmBuffer);
}

void HpaeSinkOutputNodeRenderSinkSetPriPaPowerFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->RenderSinkSetPriPaPower();
}

void HpaeSinkOutputNodeHandleHapticParamFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode = std::make_shared<HpaeSinkOutputNode>(nodeInfo);
    if (hpaeSinkOutputNode == nullptr) {
        return;
    }
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSinkOutputNode->audioRendererSink_ = fac.CreateRenderSink(renderId);
    hpaeSinkOutputNode->GetLatency();
    uint64_t syncTime = g_fuzzUtils.GetData<uint64_t>();
    hpaeSinkOutputNode->HandleHapticParam(syncTime);
}

void HpaeSourceInputClusterDoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::vector<HpaeNodeInfo> nodeInfos;
    nodeInfos.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(nodeInfos);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }
    hpaeSourceInputCluster->DoProcess();
}

void HpaeSourceInputClusterResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }
    hpaeSourceInputCluster->ResetAll();
}

void HpaeSourceInputClusterGetSharedInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }
    hpaeSourceInputCluster->GetSharedInstance();
}

void HpaeSourceInputClusterGetOutputPortFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }
    hpaeSourceInputCluster->GetOutputPort();
    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    bool isDisConnect = g_fuzzUtils.GetData<bool>();
    hpaeSourceInputCluster->GetOutputPort(nodeInfoTest, isDisConnect);
}

void HpaeSourceInputClusterGetCapturerSourceInstanceFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    std::string deviceClass = "test_device_class";
    std::string deviceNetId = "test_device_net_id";
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    std::string sourceName = "test_source_name";

    hpaeSourceInputCluster->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName);
}

void HpaeSourceInputClusterCapturerSourceInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    IAudioSourceAttr attr;
    hpaeSourceInputCluster->CapturerSourceInit(attr);
}

void HpaeSourceInputClusterCapturerSourcePauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    hpaeSourceInputCluster->CapturerSourceDeInit();
    hpaeSourceInputCluster->CapturerSourceFlush();
    hpaeSourceInputCluster->CapturerSourcePause();
}

void HpaeSourceInputClusterCapturerSourceStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    hpaeSourceInputCluster->CapturerSourceReset();
    hpaeSourceInputCluster->CapturerSourceResume();
    hpaeSourceInputCluster->CapturerSourceStart();
}

void HpaeSourceInputClusterGetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    hpaeSourceInputCluster->CapturerSourceStop();
    hpaeSourceInputCluster->GetSourceState();
    hpaeSourceInputCluster->GetOutputPortNum();
    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    hpaeSourceInputCluster->GetOutputPortNum(nodeInfoTest);
    hpaeSourceInputCluster->GetSourceInputNodeType();
}

void HpaeSourceInputClusterSetSourceInputNodeTypeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    HpaeSourceInputNodeType type = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    hpaeSourceInputCluster->SetSourceInputNodeType(type);
}

void HpaeSourceInputClusterUpdateAppsUidAndSessionIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    std::vector<int32_t> appsUid;
    appsUid.push_back(uid);
    std::vector<int32_t> sessionsId;
    sessionsId.push_back(sessionId);
    hpaeSourceInputCluster->UpdateAppsUidAndSessionId(appsUid, sessionsId);
}

void HpaeSourceInputClusterGetSourceInputNodeUseCountFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    hpaeSourceInputCluster->GetConverterNodeCount();
    hpaeSourceInputCluster->GetSourceInputNodeUseCount();
}

void HpaeSourceInputClusterWriteCapturerDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    if (hpaeSourceInputCluster == nullptr) {
        return;
    }

    char data[] = "test_data";
    int32_t length = strlen(data);
    hpaeSourceInputCluster->GetCaptureId();
    hpaeSourceInputCluster->WriteCapturerData(data, length);
}

void HpaeSourceInputNodeByVectorFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::vector<HpaeNodeInfo> nodeInfos;
    nodeInfos.push_back(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfos);
}

void HpaeSourceInputNodeSetBufferValidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HpaeSourceBufferType bufferType = g_fuzzUtils.GetData<HpaeSourceBufferType>();
    uint64_t replyBytes = g_fuzzUtils.GetData<uint64_t>();
    hpaeSourceInputNode->SetBufferValid(bufferType, replyBytes);
}

void HpaeSourceInputNodeDoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();

    hpaeSourceInputNode->DoProcess();
}

void HpaeSourceInputNodeCapturerSourceDeInitFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    hpaeSourceInputNode->CapturerSourceDeInit();
}

void HpaeSourceInputNodeCapturerSourceFlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    hpaeSourceInputNode->CapturerSourceFlush();
}

void HpaeSourceInputNodeCapturerSourcePauseFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    hpaeSourceInputNode->CapturerSourcePause();
}

void HpaeSourceInputNodeCapturerSourceStartFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    hpaeSourceInputNode->CapturerSourceStart();
}

void HpaeSourceInputNodeCapturerSourceStopFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    hpaeSourceInputNode->CapturerSourceStop();
}

void HpaeSourceInputNodeUpdateAppsUidAndSessionIdFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    uint32_t captureId = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->audioCapturerSource_ = fac.CreateCaptureSource(captureId);
    hpaeSourceInputNode->captureId_ = g_fuzzUtils.GetData<uint32_t>();
    hpaeSourceInputNode->sourceInputNodeType_ = g_fuzzUtils.GetData<HpaeSourceInputNodeType>();
    if (hpaeSourceInputNode->audioCapturerSource_ == nullptr) {
        return;
    }
    IAudioSourceAttr attr;
    hpaeSourceInputNode->audioCapturerSource_->Init(attr);
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    std::vector<int32_t> appsUid = {uid};
    std::vector<int32_t> sessionsId = {sessionId};
    hpaeSourceInputNode->UpdateAppsUidAndSessionId(appsUid, sessionsId);
}

void HpaeSourceInputNodeGetOutputPortNumFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> hpaeSourceInputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    if (hpaeSourceInputNode == nullptr) {
        return;
    }

    HpaeNodeInfo nodeInfoTest;
    GetTestNodeInfo(nodeInfoTest);
    hpaeSourceInputNode->GetOutputPortNum(nodeInfoTest);
}

vector<TestPtr> g_testPtrs = {
    HpaeSinkOutputNodeHandleRemoteTimingFuzzTest,
    HpaeSinkOutputNodeDoProcessFuzzTest,
    HpaeSinkOutputNodeGetRenderFrameDataFuzzTest,
    HpaeSinkOutputNodeResetFuzzTest,
    HpaeSinkOutputNodeRenderSinkFlushFuzzTest,
    HpaeSinkOutputNodeRenderSinkPauseFuzzTest,
    HpaeSinkOutputNodeRenderSinkResetFuzzTest,
    HpaeSinkOutputNodeRenderSinkStartFuzzTest,
    HpaeSinkOutputNodeUpdateAppsUidFuzzTest,
    HpaeSinkOutputNodeHandlePaPowerFuzzTest,
    HpaeSinkOutputNodeRenderSinkSetPriPaPowerFuzzTest,
    HpaeSinkOutputNodeHandleHapticParamFuzzTest,
    HpaeSourceInputClusterDoProcessFuzzTest,
    HpaeSourceInputClusterResetAllFuzzTest,
    HpaeSourceInputClusterGetSharedInstanceFuzzTest,
    HpaeSourceInputClusterGetOutputPortFuzzTest,
    HpaeSourceInputClusterGetCapturerSourceInstanceFuzzTest,
    HpaeSourceInputClusterCapturerSourceInitFuzzTest,
    HpaeSourceInputClusterCapturerSourcePauseFuzzTest,
    HpaeSourceInputClusterCapturerSourceStartFuzzTest,
    HpaeSourceInputClusterGetSourceInputNodeTypeFuzzTest,
    HpaeSourceInputClusterSetSourceInputNodeTypeFuzzTest,
    HpaeSourceInputClusterUpdateAppsUidAndSessionIdFuzzTest,
    HpaeSourceInputClusterGetSourceInputNodeUseCountFuzzTest,
    HpaeSourceInputClusterWriteCapturerDataFuzzTest,
    HpaeSourceInputNodeByVectorFuzzTest,
    HpaeSourceInputNodeSetBufferValidFuzzTest,
    HpaeSourceInputNodeDoProcessFuzzTest,
    HpaeSourceInputNodeCapturerSourceDeInitFuzzTest,
    HpaeSourceInputNodeCapturerSourceFlushFuzzTest,
    HpaeSourceInputNodeCapturerSourcePauseFuzzTest,
    HpaeSourceInputNodeCapturerSourceStartFuzzTest,
    HpaeSourceInputNodeCapturerSourceStopFuzzTest,
    HpaeSourceInputNodeUpdateAppsUidAndSessionIdFuzzTest,
    HpaeSourceInputNodeGetOutputPortNumFuzzTest,
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