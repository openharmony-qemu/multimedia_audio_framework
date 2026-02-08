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
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
constexpr size_t PARAM_MAX_SIZE = 1000;  // Match source definition

// Buffer size constants
constexpr size_t DEFAULT_BUFFER_SIZE_8192 = 8192;
constexpr size_t DEFAULT_BUFFER_SIZE_4096 = 4096;
constexpr size_t EXTRA_PARAM_SIZE = 100;
constexpr uint8_t PARAM_FILL_VALUE = 0x42;

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

// New test: SetConfig without Create (tests controller_ == nullptr path)
void OfflineAudioEffectServerChainSetConfigWithoutCreateFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    // Don't call Create() - controller_ will be nullptr
    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    inInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    inInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioStreamInfo outInfo;
    outInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    outInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    outInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    serverChain->SetConfig(inInfo, outInfo);
}

// New test: SetParam without Create (tests controller_ == nullptr path)
void OfflineAudioEffectServerChainSetParamWithoutCreateFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    // Don't call Create() - controller_ will be nullptr
    std::vector<uint8_t> param;
    param.push_back(g_fuzzUtils.GetData<uint8_t>());
    serverChain->SetParam(param);
}

// New test: SetParam with large param (tests param.size() >= PARAM_MAX_SIZE)
void OfflineAudioEffectServerChainSetParamLargeFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    // Create a large param vector to test PARAM_MAX_SIZE check
    std::vector<uint8_t> param(PARAM_MAX_SIZE + EXTRA_PARAM_SIZE, PARAM_FILL_VALUE);
    serverChain->SetParam(param);
}

// New test: GetEffectBufferSize without SetConfig (tests buffer size == 0 path)
void OfflineAudioEffectServerChainGetEffectBufferSizeWithoutSetConfigFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    // Don't call SetConfig() - inBufferSize_ and outBufferSize_ will be 0
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
}

// New test: Prepare with null buffers
void OfflineAudioEffectServerChainPrepareWithNullBuffersFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    // Test with null buffers
    serverChain->Prepare(nullptr, nullptr);
}

// New test: Process without Prepare (tests buffer nullptr checks)
void OfflineAudioEffectServerChainProcessWithoutPrepareFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    AudioStreamInfo inInfo;
    inInfo.samplingRate = SAMPLE_RATE_48000;
    inInfo.format = SAMPLE_S16LE;
    inInfo.channels = STEREO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = SAMPLE_RATE_48000;
    outInfo.format = SAMPLE_S16LE;
    outInfo.channels = STEREO;
    serverChain->SetConfig(inInfo, outInfo);
    // Don't call Prepare() - serverBufferIn_ and serverBufferOut_ will be nullptr
    uint32_t inSize = g_fuzzUtils.GetData<uint32_t>();
    uint32_t outSize = g_fuzzUtils.GetData<uint32_t>();
    serverChain->Process(inSize, outSize);
}

// New test: Process with size exceeding buffer size
void OfflineAudioEffectServerChainProcessSizeExceedFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    AudioStreamInfo inInfo;
    inInfo.samplingRate = SAMPLE_RATE_48000;
    inInfo.format = SAMPLE_S16LE;
    inInfo.channels = STEREO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = SAMPLE_RATE_48000;
    outInfo.format = SAMPLE_S16LE;
    outInfo.channels = STEREO;
    serverChain->SetConfig(inInfo, outInfo);

    size_t inBufferSize = DEFAULT_BUFFER_SIZE_8192;
    std::string inName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(inBufferSize, inName);
    size_t outBufferSize = DEFAULT_BUFFER_SIZE_8192;
    std::string outName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(outBufferSize, outName);
    serverChain->Prepare(bufferIn, bufferOut);

    // Test with sizes exceeding buffer sizes
    uint32_t inSize = 999999;  // Exceeds inBufferSize_
    uint32_t outSize = 999999;  // Exceeds outBufferSize_
    serverChain->Process(inSize, outSize);
}

// New test: Release without Create
void OfflineAudioEffectServerChainReleaseWithoutCreateFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    // Don't call Create() - controller_ will be nullptr
    serverChain->Release();
}

// New test: Multiple Release calls (tests double-release scenario)
void OfflineAudioEffectServerChainReleaseTwiceFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    serverChain->Release();
    // Second Release call - controller_ should be nullptr now
    serverChain->Release();
}

// New test: Full lifecycle test
void OfflineAudioEffectServerChainFullLifecycleFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);

    // Create
    int32_t ret = serverChain->Create();

    // SetConfig
    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    inInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    inInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioStreamInfo outInfo;
    outInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    outInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    outInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    if (ret == SUCCESS) {
        serverChain->SetConfig(inInfo, outInfo);
    }

    // SetParam
    if (ret == SUCCESS) {
        std::vector<uint8_t> param;
        param.push_back(g_fuzzUtils.GetData<uint8_t>());
        serverChain->SetParam(param);
    }

    // GetEffectBufferSize
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    if (ret == SUCCESS) {
        serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    }

    // Prepare
    size_t bufSize = DEFAULT_BUFFER_SIZE_8192;
    std::string bufName = "testBuffer";
    std::shared_ptr<AudioSharedMemory> bufferIn = AudioSharedMemory::CreateFromLocal(bufSize, bufName);
    std::shared_ptr<AudioSharedMemory> bufferOut = AudioSharedMemory::CreateFromLocal(bufSize, bufName);
    serverChain->Prepare(bufferIn, bufferOut);

    // Process
    if (ret == SUCCESS && inBufferSize > 0 && outBufferSize > 0) {
        uint32_t inSize = DEFAULT_BUFFER_SIZE_4096;
        uint32_t outSize = DEFAULT_BUFFER_SIZE_4096;
        serverChain->Process(inSize, outSize);
    }

    // Release
    serverChain->Release();
}

// New test: SetConfig with various AudioSampleFormat values
void OfflineAudioEffectServerChainSetConfigFormatsFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();

    // Test with all possible format values
    AudioStreamInfo inInfo;
    inInfo.samplingRate = SAMPLE_RATE_48000;
    inInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    inInfo.channels = STEREO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = SAMPLE_RATE_48000;
    outInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    outInfo.channels = STEREO;
    serverChain->SetConfig(inInfo, outInfo);
}

// New test: SetConfig with extreme sampling rates
void OfflineAudioEffectServerChainSetConfigExtremeRatesFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();

    // Test with boundary sampling rate values (min/max valid rates)
    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    inInfo.format = SAMPLE_S16LE;
    inInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioStreamInfo outInfo;
    outInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    outInfo.format = SAMPLE_S16LE;
    outInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    serverChain->SetConfig(inInfo, outInfo);
}

// New test: Destructor test (tests ~OfflineAudioEffectServerChain)
void OfflineAudioEffectServerChainDestructorFuzzTest()
{
    // Create and destroy without calling Release
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    // Let destructor handle cleanup
    serverChain.reset();

    // Create and destroy with Release
    serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    serverChain->Release();
    serverChain.reset();

    // Create without successful Create
    serverChain = make_shared<OfflineAudioEffectServerChain>("invalidChainName");
    CHECK_AND_RETURN(serverChain != nullptr);
    // Destructor should handle null controller_
    serverChain.reset();
}

// New test: Create with invalid chain name (tests map lookup failure)
void OfflineAudioEffectServerChainCreateInvalidNameFuzzTest()
{
    // Use a name that won't exist in the chain map
    std::string invalidName = "InvalidChainName_12345_XYZ_" + std::to_string(g_fuzzUtils.GetData<uint32_t>());
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>(invalidName);
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();  // Should return ERROR - chain not found in map
}

// New test: SetParam with empty vector
void OfflineAudioEffectServerChainSetParamEmptyFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    std::vector<uint8_t> emptyParam;  // Empty vector
    serverChain->SetParam(emptyParam);
}

// New test: SetConfig with extreme values to test overflow
void OfflineAudioEffectServerChainSetConfigOverflowFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();

    AudioStreamInfo inInfo;
    inInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    inInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    inInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioStreamInfo outInfo;
    outInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    outInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    outInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    serverChain->SetConfig(inInfo, outInfo);  // Test with various valid combinations
}

// New test: Multiple Create calls without Release
void OfflineAudioEffectServerChainCreateTwiceFuzzTest()
{
    shared_ptr<OfflineAudioEffectServerChain> serverChain = make_shared<OfflineAudioEffectServerChain>("serverChain");
    CHECK_AND_RETURN(serverChain != nullptr);
    serverChain->Create();
    // Second Create call without Release - tests controller_ reassignment
    serverChain->Create();
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
    // New tests for error handling and edge cases
    OfflineAudioEffectServerChainSetConfigWithoutCreateFuzzTest,
    OfflineAudioEffectServerChainSetParamWithoutCreateFuzzTest,
    OfflineAudioEffectServerChainSetParamLargeFuzzTest,
    OfflineAudioEffectServerChainGetEffectBufferSizeWithoutSetConfigFuzzTest,
    OfflineAudioEffectServerChainPrepareWithNullBuffersFuzzTest,
    OfflineAudioEffectServerChainProcessWithoutPrepareFuzzTest,
    OfflineAudioEffectServerChainProcessSizeExceedFuzzTest,
    OfflineAudioEffectServerChainReleaseWithoutCreateFuzzTest,
    OfflineAudioEffectServerChainReleaseTwiceFuzzTest,
    OfflineAudioEffectServerChainFullLifecycleFuzzTest,
    OfflineAudioEffectServerChainSetConfigFormatsFuzzTest,
    OfflineAudioEffectServerChainSetConfigExtremeRatesFuzzTest,
    OfflineAudioEffectServerChainDestructorFuzzTest,
    // Additional tests for coverage gaps
    OfflineAudioEffectServerChainCreateInvalidNameFuzzTest,
    OfflineAudioEffectServerChainSetParamEmptyFuzzTest,
    OfflineAudioEffectServerChainSetConfigOverflowFuzzTest,
    OfflineAudioEffectServerChainCreateTwiceFuzzTest,
};

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}