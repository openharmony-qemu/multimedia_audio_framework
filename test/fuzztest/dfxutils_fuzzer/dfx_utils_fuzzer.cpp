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

#include <securec.h>

#include "audio_log.h"
#include "dfx_utils.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

void SerializeToJSONString1FuzzTest()
{
    DfxUtils dfxUtils;
    RendererStats data;
    dfxUtils.SerializeToJSONString(data);
}

void SerializeToJSONString2FuzzTest()
{
    DfxUtils dfxUtils;
    CapturerStats data;
    dfxUtils.SerializeToJSONString(data);
}

void SerializeToJSONString3FuzzTest()
{
    DfxUtils dfxUtils;
    std::vector<InterruptEffect> data;
    InterruptEffect gameEffect;
    gameEffect.bundleName = "com.example.game";
    gameEffect.streamUsage = g_fuzzUtils.GetData<uint8_t>();
    gameEffect.appState = g_fuzzUtils.GetData<uint8_t>();
    gameEffect.interruptEvent = g_fuzzUtils.GetData<uint8_t>();
    data.push_back(gameEffect);
    dfxUtils.SerializeToJSONString(data);
}

vector<TestFuncs> g_testFuncs = {
    SerializeToJSONString1FuzzTest,
    SerializeToJSONString2FuzzTest,
    SerializeToJSONString3FuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
