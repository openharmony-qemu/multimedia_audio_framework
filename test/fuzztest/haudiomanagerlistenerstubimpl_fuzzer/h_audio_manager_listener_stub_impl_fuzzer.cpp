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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "token_setproc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "../fuzz_utils.h"
#include "audio_manager_listener_stub_impl.h"
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {
using namespace std;

class WakeUpSourceCallbackFuzz : public WakeUpSourceCallback {
public:
    void OnCapturerState(bool isActive) override
    {
        return;
    }

    void OnWakeupClose() override
    {
        return;
    }
};

class AudioParameterCallbackFuzz : public AudioParameterCallback {
public:
    void OnAudioParameterChange(const std::string networkId, const AudioParamKey key, const std::string& condition,
        const std::string& value) override
    {
        return;
    }
    void OnHdiRouteStateChange(const std::string &networkId, bool enable) override
    {
        return;
    }
};

class DataTransferStateChangeCallbackTest : public AudioRendererDataTransferStateChangeCallback {
public:
    void OnDataTransferStateChange(const AudioRendererDataTransferStateChangeInfo &info) override {}
};

void OnAudioParameterChange(FuzzedDataProvider &provider)
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<AudioParameterCallbackFuzz> callback = std::make_shared<AudioParameterCallbackFuzz>();
    audioManagerListenerStubImpl.SetParameterCallback(callback);
    std::string networkId = provider.ConsumeRandomLengthString();
    int32_t key = provider.ConsumeIntegral<int32_t>();
    std::string condition = provider.ConsumeRandomLengthString();
    std::string value = provider.ConsumeRandomLengthString();
    audioManagerListenerStubImpl.OnAudioParameterChange(networkId, key, condition, value);
}

void OnCapturerState(FuzzedDataProvider &provider)
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<WakeUpSourceCallbackFuzz> callback = std::make_shared<WakeUpSourceCallbackFuzz>();
    audioManagerListenerStubImpl.SetWakeupSourceCallback(callback);
    bool isActive = provider.ConsumeBool();
    audioManagerListenerStubImpl.OnCapturerState(isActive);
}

void OnWakeupClose(FuzzedDataProvider &provider)
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<WakeUpSourceCallbackFuzz> callback = std::make_shared<WakeUpSourceCallbackFuzz>();
    audioManagerListenerStubImpl.SetWakeupSourceCallback(callback);
    bool isActive = provider.ConsumeBool();
    audioManagerListenerStubImpl.OnWakeupClose();
}

void OnDataTransferStateChange(FuzzedDataProvider &provider)
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    int32_t callbackId = provider.ConsumeIntegral<int32_t>();
    AudioRendererDataTransferStateChangeInfo info;
    audioManagerListenerStubImpl.OnDataTransferStateChange(callbackId, info);
}

void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray({
        OnAudioParameterChange,
        OnCapturerState,
        OnWakeupClose,
        OnDataTransferStateChange,
    });
    func(provider);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (SetSelfTokenID(718336240uLL | (1uLL << NUM_32)) < 0) {
        return -1;
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
