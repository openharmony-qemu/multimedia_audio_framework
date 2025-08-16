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

#include "../fuzz_utils.h"

#include "audio_common_log.h"
#include "audio_offload_stream.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

typedef void (*TestPtr)();

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
static const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

void FuzzTestGetOffloadSessionId()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    testModule.GetOffloadSessionId(g_fuzzUtils.GetData<OffloadAdapter>());
}

void FuzzTestSetOffloadStatus()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    testModule.SetOffloadStatus(
        g_fuzzUtils.GetData<OffloadAdapter>(),
        g_fuzzUtils.GetData<uint32_t>());
}

void FuzzTestUnsetOffloadStatus()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    testModule.UnsetOffloadStatus(g_fuzzUtils.GetData<uint32_t>());
}

void FuzzTestHandlePowerStateChanged()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    testModule.HandlePowerStateChanged(g_fuzzUtils.GetData<PowerMgr::PowerState>());
}

void FuzzTestUpdateOffloadStatusFromUpdateTracker()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    testModule.UpdateOffloadStatusFromUpdateTracker(
        g_fuzzUtils.GetData<uint32_t>(),
        g_fuzzUtils.GetData<RendererState>());
}

void FuzzTestDump()
{
    AudioOffloadStream &testModule = AudioOffloadStream::GetInstance();
    std::string fuzzStr = std::to_string(g_fuzzUtils.GetData<uint32_t>());
    testModule.Dump(fuzzStr);
}

vector<TestPtr> g_testPtrs = {
    FuzzTestGetOffloadSessionId,
    FuzzTestSetOffloadStatus,
    FuzzTestUnsetOffloadStatus,
    FuzzTestHandlePowerStateChanged,
    FuzzTestUpdateOffloadStatusFromUpdateTracker,
    FuzzTestDump,
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
