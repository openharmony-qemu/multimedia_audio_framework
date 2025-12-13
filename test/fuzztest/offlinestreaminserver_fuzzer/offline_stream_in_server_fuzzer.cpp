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
#include <fuzzer/FuzzedDataProvider.h>

#include "offline_stream_in_server.h"
#include "audio_shared_memory.h"
#include "audio_info.h"
#include "token_setproc.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
std::shared_ptr<OfflineStreamInServer> offlineStreamInServer_;
void CreateOfflineEffectChain(FuzzedDataProvider& fdp)
{
    std::string chainName = "abc";
    offlineStreamInServer_->CreateOfflineEffectChain(chainName);
}

void PrepareOfflineEffectChain(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioSharedMemory> in;
    std::shared_ptr<AudioSharedMemory> out;
    offlineStreamInServer_->PrepareOfflineEffectChain(in, out);
}

void SetParamOfflineEffectChain(FuzzedDataProvider& fdp)
{
    std::vector<uint8_t> param = { fdp.ConsumeIntegral<uint8_t>(),
        fdp.ConsumeIntegral<uint8_t>(), fdp.ConsumeIntegral<uint8_t>()};
    offlineStreamInServer_->SetParamOfflineEffectChain(param);
}

void ProcessOfflineEffectChain(FuzzedDataProvider& fdp)
{
    uint32_t inSize = fdp.ConsumeIntegral<uint32_t>();
    uint32_t outSize = fdp.ConsumeIntegral<uint32_t>();
    offlineStreamInServer_->ProcessOfflineEffectChain(inSize, outSize); 
}

void ReleaseOfflineEffectChain(FuzzedDataProvider& fdp)
{
    offlineStreamInServer_->ReleaseOfflineEffectChain();
}

void ConfigureOfflineEffectChain(FuzzedDataProvider& fdp)
{
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
    offlineStreamInServer_->ConfigureOfflineEffectChain(inInfo, outInfo);
}
void GetOfflineAudioEffectChains(FuzzedDataProvider& fdp)
{
    std::vector<std::string> effectChains;
    offlineStreamInServer_->GetOfflineAudioEffectChains(effectChains);
}

void OfflineEffectChainInit()
{
    offlineStreamInServer_ = std::make_shared<OfflineStreamInServer>();
}

void OfflineEffectChainTest(FuzzedDataProvider& fdp)
{
    CHECK_AND_RETURN_LOG(offlineStreamInServer_ != nullptr, "offlineStreamInServer_ is nullptr");
    auto func = fdp.PickValueInArray({
        CreateOfflineEffectChain,
        PrepareOfflineEffectChain,
        SetParamOfflineEffectChain,
        ProcessOfflineEffectChain,
        ReleaseOfflineEffectChain,
        ConfigureOfflineEffectChain,
        GetOfflineAudioEffectChains
    });
    func(fdp);
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::OfflineEffectChainTest(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    if (SetSelfTokenID(718336240ull | (1ull << 32)) < 0) {
        return -1;
    }
    OHOS::AudioStandard::OfflineEffectChainInit();
    return 0;
}