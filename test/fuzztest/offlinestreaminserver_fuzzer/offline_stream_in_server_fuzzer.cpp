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

#include "offline_stream_in_server_fuzzer.h"
#include "audio_shared_memory.h"
#include "../fuzz_utils.h"
#include <thread>
namespace OHOS {
namespace AudioStandard {
using namespace std;
class AudioSharedMemoryTest : public AudioSharedMemory {
public:
    uint8_t *GetBase() override { return nullptr; };
    size_t GetSize() override { return 0; };
    int GetFd() override { return 0; };
    std::string GetName() override { return "abc"; };
    bool Marshalling(Parcel &parcel) const override { return true; };
};

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
static int32_t NUM_4 = 4;
static int32_t NUM_3 = 3;

typedef void (*TestFuncs)();

void OfflineStreamInServerFuzzTest::OfflineStreamInServerFuzz()
{
    offlineStreamInServer_ = std::make_shared<OfflineStreamInServer>();
    std::vector<std::string> effectChains;
    offlineStreamInServer_->GetOfflineAudioEffectChains(effectChains);
    std::shared_ptr<AudioSharedMemory> inBuffer = std::make_shared<AudioSharedMemoryTest>();
    std::shared_ptr<AudioSharedMemory> outBuffer = std::make_shared<AudioSharedMemoryTest>();
    offlineStreamInServer_->serverBufferIn_ = g_fuzzUtils.GetData<bool>() ? inBuffer : nullptr;
    offlineStreamInServer_->serverBufferOut_ = g_fuzzUtils.GetData<bool>() ? outBuffer : nullptr;
    std::string chainName = "abc";
    
    AudioStreamInfo inInfo;
    inInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    inInfo.encoding = AudioEncodingType::ENCODING_PCM;
    inInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    inInfo.channels = AudioChannel::MONO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    outInfo.encoding = AudioEncodingType::ENCODING_PCM;
    outInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    outInfo.channels = AudioChannel::MONO;
    std::vector<uint8_t> param = { g_fuzzUtils.GetData<uint8_t>(),
        g_fuzzUtils.GetData<uint8_t>(), g_fuzzUtils.GetData<uint8_t>()};
    uint32_t inSize = g_fuzzUtils.GetData<uint32_t>();
    uint32_t outSize = g_fuzzUtils.GetData<uint32_t>();
    Funcs_.clear();
    Funcs_.push_back([=]() { offlineStreamInServer_->CreateOfflineEffectChain(chainName); });
    Funcs_.push_back([=]() {
        std::shared_ptr<AudioSharedMemory> in;
        std::shared_ptr<AudioSharedMemory> out;
        offlineStreamInServer_->PrepareOfflineEffectChain(in, out);
    });
    Funcs_.push_back([=]() { offlineStreamInServer_->ConfigureOfflineEffectChain(inInfo, outInfo); });
    Funcs_.push_back([=]() { offlineStreamInServer_->SetParamOfflineEffectChain(param); });
    Funcs_.push_back([=]() { offlineStreamInServer_->ProcessOfflineEffectChain(inSize, outSize); });
    Funcs_.push_back([=]() { offlineStreamInServer_->ReleaseOfflineEffectChain(); });
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_4; ++i) {
        threads.emplace_back([this]() {
            for (int j = 0; j < NUM_3; ++j) {
                size_t index = g_fuzzUtils.GetData<size_t>() % Funcs_.size();
                Funcs_[index]();
            }
        });
    }
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void OfflineStreamInServerFuzz()
{
    OfflineStreamInServerFuzzTest t;
    t.OfflineStreamInServerFuzz();
}

vector<TestFuncs> g_testFuncs = {
    OfflineStreamInServerFuzz,
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