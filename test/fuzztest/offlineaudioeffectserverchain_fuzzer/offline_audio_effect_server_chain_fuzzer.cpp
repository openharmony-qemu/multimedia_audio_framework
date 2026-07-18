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

#include "offline_audio_effect_server_chain.h"
#include "audio_errors.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
// Mirrors the engine limit in offline_audio_effect_server_chain.cpp (PARAM_MAX_SIZE = 4096).
// SetParam/GetParam reject payloads whose size is >= this value; keep in sync with the engine.
const size_t PARAM_MAX_SIZE = 4096;
const size_t PARAM_OVERFLOW_RANGE = 1024;
// Audio I/O buffer size for the real-chain Prepare/Process path (>= inSize/outSize so the
// Process size checks pass). uint32_t to match inBufferSize_/outBufferSize_.
constexpr uint32_t PROCESS_BUFFER_SIZE = 4096;
// Minimal inSize/outSize for Process: 1 byte / S16LE(2 bytes) -> frameCount 0 -> EffectProcess no-op.
constexpr uint32_t MIN_PROCESS_IO_SIZE = 1;

typedef void (*TestFuncs)();

void OfflineAudioEffectServerChainInitDumpFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->InitDump();
}

void OfflineAudioEffectServerChainCreateFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
}

void OfflineAudioEffectServerChainSetConfigFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    AudioStreamInfo outInfo;
    serverChain->SetConfig(inInfo, outInfo);
}

void OfflineAudioEffectServerChainSetParamFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    std::vector<uint8_t> param;
    param.push_back(g_fuzzUtils.GetData<uint8_t>());
    serverChain->SetParam(param);
}

void OfflineAudioEffectServerChainGetEffectBufferSizeFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    serverChain->inBufferSize_ = g_fuzzUtils.GetData<uint32_t>();
    serverChain->outBufferSize_ = g_fuzzUtils.GetData<uint32_t>();
    serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
}

void OfflineAudioEffectServerChainPrepareFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    size_t inSize = g_fuzzUtils.GetData<size_t>();
    std::string inName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(inSize, inName);
    size_t outSize = g_fuzzUtils.GetData<size_t>();
    std::string outName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(outSize, outName);
    serverChain->Prepare(bufferIn, bufferOut);
}

void OfflineAudioEffectServerChainProcessFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    size_t inBufferSize = g_fuzzUtils.GetData<size_t>();
    std::string inName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(inBufferSize, inName);
    size_t outBufferSize = g_fuzzUtils.GetData<size_t>();
    std::string outName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(outBufferSize, outName);
    serverChain->inBufferSize_ = inBufferSize;
    serverChain->outBufferSize_ = outBufferSize;
    uint32_t inSize = g_fuzzUtils.GetData<uint32_t>();
    uint32_t outSize = g_fuzzUtils.GetData<uint32_t>();
    serverChain->Process(inSize, outSize);
}

void OfflineAudioEffectServerChainGetOfflineAudioEffectChainsFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    std::vector<std::string> chainNamesVector = {"abc", "link", "source"};
    serverChain->GetOfflineAudioEffectChains(chainNamesVector);
}

void OfflineAudioEffectServerChainReleaseFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    serverChain->Release();
}

void OfflineAudioEffectGetParamFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    std::vector<uint8_t> param;
    serverChain->GetParam(param);
}

void OfflineAudioEffectFullLifecycleFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();

    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    AudioStreamInfo outInfo;
    serverChain->SetConfig(inInfo, outInfo);

    std::vector<uint8_t> param;
    param.push_back(g_fuzzUtils.GetData<uint8_t>());
    serverChain->SetParam(param);

    size_t inSize = g_fuzzUtils.GetData<size_t>();
    std::shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(inSize, "bufIn");
    size_t outSize = g_fuzzUtils.GetData<size_t>();
    std::shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(outSize, "bufOut");
    serverChain->Prepare(bufferIn, bufferOut);

    serverChain->inBufferSize_ = inSize;
    serverChain->outBufferSize_ = outSize;
    serverChain->Process(1, 1);

    std::vector<uint8_t> outParam;
    serverChain->GetParam(outParam);

    serverChain->Release();
}

// Exercise the PARAM_MAX_SIZE (4096) boundary paths that the size-1 / empty param tests miss:
// GetParam oversized -> rejection (230-231).
void ParamBoundaryFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);

    size_t bigSize = PARAM_MAX_SIZE + g_fuzzUtils.GetData<uint32_t>() % PARAM_OVERFLOW_RANGE; // >= PARAM_MAX_SIZE
    std::vector<uint8_t> bigParam(bigSize, g_fuzzUtils.GetData<uint8_t>());
    serverChain->SetParam(bigParam); // rejected: size >= PARAM_MAX_SIZE (lines 214-215)

    size_t smallSize = g_fuzzUtils.GetData<uint32_t>() % (PARAM_MAX_SIZE - 1) + 1; // 1..4095, always non-empty
    std::vector<uint8_t> smallParam(smallSize, g_fuzzUtils.GetData<uint8_t>());
    serverChain->GetParam(smallParam); // memcpy branch (lines 234-237), then controller-null return

    std::vector<uint8_t> hugeParam(PARAM_MAX_SIZE + 1, 0);
    serverChain->GetParam(hugeParam); // rejected: size >= PARAM_MAX_SIZE (lines 230-231)
}

// Drive the controller_-non-null success paths. Every other test builds the chain with
// "serverChain", which is not a real effect name, so Create() fails at the chainName_
// lookup (offline_audio_effect_server_chain.cpp:156) and controller_ stays null --
// leaving the success bodies of Create/SetConfig/SetParam/GetParam/Prepare/Process/
// Release (and GetByteSize) uncovered. Here we first retrieve the real effect names
// registered with the HAL, build the chain with a real name so Create() succeeds, then
// run the full success path. No-op when no effect is registered (chainNames empty), so
// it stays safe on hosts without an effect HAL.
void OfflineAudioEffectRealChainSuccessFuzzTest()
{
    vector<string> chainNames;
    OfflineAudioEffectServerChain::GetOfflineAudioEffectChains(chainNames);
    CHECK_AND_RETURN(!chainNames.empty());
    shared_ptr<OfflineAudioEffectServerChain> serverChain =
        make_shared<OfflineAudioEffectServerChain>(chainNames[0]);
    CHECK_AND_RETURN(serverChain != nullptr);
    if (serverChain->Create() != SUCCESS) { // controller_ stays null if CreateEffectController fails
        return;
    }

    AudioStreamInfo inInfo;
    inInfo.samplingRate = SAMPLE_RATE_48000;
    inInfo.channels = STEREO;
    inInfo.format = SAMPLE_S16LE;
    AudioStreamInfo outInfo = inInfo;
    serverChain->SetConfig(inInfo, outInfo); // SendCommand SET_CONFIG + GetByteSize

    vector<uint8_t> param;
    param.push_back(g_fuzzUtils.GetData<uint8_t>());
    serverChain->SetParam(param); // SendCommand SET_PARAM

    shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(PROCESS_BUFFER_SIZE, "bufIn");
    shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(PROCESS_BUFFER_SIZE, "bufOut");
    // CreateFromLocal may return nullptr on shared-memory alloc failure. Bail out so the
    // later Process runs the intended success path (EffectProcess body) instead of its
    // nullptr ERROR branch. Process already null-checks serverBufferIn_/Out_, so this is
    // coverage hygiene for the success path, not a crash guard.
    if (bufferIn == nullptr || bufferOut == nullptr) {
        return;
    }
    serverChain->Prepare(bufferIn, bufferOut); // controller_ non-null -> SendCommand RESET

    uint32_t inBufSize = 0;
    uint32_t outBufSize = 0;
    serverChain->GetEffectBufferSize(inBufSize, outBufSize);
    (void)inBufSize; // called for coverage; values intentionally unused
    (void)outBufSize;

    // Ensure Process size checks pass so the EffectProcess body is reached even if
    // SetConfig's SET_CONFIG was rejected by the effect (which leaves inBufferSize_ = 0).
    serverChain->inBufferSize_ = PROCESS_BUFFER_SIZE;
    serverChain->outBufferSize_ = PROCESS_BUFFER_SIZE;
    serverChain->Process(MIN_PROCESS_IO_SIZE, MIN_PROCESS_IO_SIZE); // EffectProcess + memcpy + WriteDumpFile

    vector<uint8_t> outParam;
    serverChain->GetParam(outParam); // SendCommand GET_PARAM
    serverChain->Release(); // DestroyEffectController
}

vector<TestFuncs> g_testFuncs = {
    OfflineAudioEffectServerChainInitDumpFuzzTest,
    OfflineAudioEffectServerChainCreateFuzzTest,
    OfflineAudioEffectServerChainSetConfigFuzzTest,
    OfflineAudioEffectServerChainSetParamFuzzTest,
    OfflineAudioEffectServerChainGetEffectBufferSizeFuzzTest,
    OfflineAudioEffectServerChainPrepareFuzzTest,
    OfflineAudioEffectServerChainProcessFuzzTest,
    OfflineAudioEffectServerChainGetOfflineAudioEffectChainsFuzzTest,
    OfflineAudioEffectServerChainReleaseFuzzTest,
    OfflineAudioEffectGetParamFuzzTest,
    OfflineAudioEffectFullLifecycleFuzzTest,
    ParamBoundaryFuzzTest,
    OfflineAudioEffectRealChainSuccessFuzzTest,
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
