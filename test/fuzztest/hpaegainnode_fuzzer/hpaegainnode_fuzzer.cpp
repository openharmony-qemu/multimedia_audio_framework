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
#include <cmath>
#include <memory>
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_gain_node.h"
#include "../fuzz_utils.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
constexpr uint32_t DEFAULT_NODE_ID = 1234;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
typedef void (*TestPtr)();

static void GetTestNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.deviceClass = "primary";
}

void HpaeGainNodeSignalProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    std::vector<HpaePcmBuffer *> inputs;
    hpaeGainNode->SignalProcess(inputs);
}

void HpaeGainNodeSetClientVolumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    float gain = g_fuzzUtils.GetData<float>();
    hpaeGainNode->SetClientVolume(gain);
}

void HpaeGainNodeGetClientVolumeFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    hpaeGainNode->GetClientVolume();
}

void HpaeGainNodeSetFadeStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    IOperation operation = OPERATION_DATA_LINK_CONNECTED;
    hpaeGainNode->SetFadeState(operation);
}

void HpaeGainNodeSilenceDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LEN, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaeGainNode->SilenceData(&hpaePcmBuffer);
}

void HpaeGainNodeIsSilentDataFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    GetTestNodeInfo(nodeInfo);
    std::shared_ptr<HpaeGainNode> hpaeGainNode = std::make_shared<HpaeGainNode>(nodeInfo);
    PcmBufferInfo pcmBufferInfo(MONO, DEFAULT_FRAME_LEN, SAMPLE_RATE_44100);
    HpaePcmBuffer hpaePcmBuffer(pcmBufferInfo);
    hpaeGainNode->IsSilentData(&hpaePcmBuffer);
}

vector<TestPtr> g_testPtrs = {
    HpaeGainNodeSignalProcessFuzzTest,
    HpaeGainNodeSetClientVolumeFuzzTest,
    HpaeGainNodeGetClientVolumeFuzzTest,
    HpaeGainNodeSetFadeStateFuzzTest,
    HpaeGainNodeSilenceDataFuzzTest,
    HpaeGainNodeIsSilentDataFuzzTest,
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