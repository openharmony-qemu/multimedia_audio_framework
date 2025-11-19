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

#include "hpae_dfx_map_tree.h"
#include "audio_log.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace HPAE;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t THRESHOLD = 10;
typedef void (*TestFuncs)();

void PrintNodeInfoFuzzTest()
{
    std::shared_ptr<HpaeDfxMapTree> hpaeDfxTree = std::make_shared<HpaeDfxMapTree>();
    CHECK_AND_RETURN(hpaeDfxTree != nullptr);
    string outStr = "test";
    HpaeDfxNodeInfo nodeInfo = {
        .nodeName = "test",
        .sessionId = g_fuzzUtils.GetData<uint32_t>(),
        .nodeId = g_fuzzUtils.GetData<uint32_t>(),
        .samplingRate = g_fuzzUtils.GetData< AudioSamplingRate>(),
        .channels = g_fuzzUtils.GetData<AudioChannel>(),
        .format = g_fuzzUtils.GetData<AudioSampleFormat>(),
        .frameLen = g_fuzzUtils.GetData<uint32_t>(),
        .sceneType = g_fuzzUtils.GetData<HpaeProcessorType>(),
    };

    hpaeDfxTree->PrintNodeInfo(outStr, nodeInfo);
}
vector<TestFuncs> g_testFuncs = {
    PrintNodeInfoFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
