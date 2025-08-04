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
#include "audio_manager_listener_stub_impl.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

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
};

void SetWakeupSourceCallbackFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<WakeUpSourceCallbackFuzz> callback = std::make_shared<WakeUpSourceCallbackFuzz>();
    audioManagerListenerStubImpl.SetWakeupSourceCallback(callback);
}

void OnCapturerStateFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<WakeUpSourceCallbackFuzz> callback = std::make_shared<WakeUpSourceCallbackFuzz>();
    audioManagerListenerStubImpl.SetWakeupSourceCallback(callback);
    bool isActive = g_fuzzUtils.GetData<bool>();
    audioManagerListenerStubImpl.OnCapturerState(isActive);
}

void OnWakeupCloseFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<WakeUpSourceCallbackFuzz> callback = std::make_shared<WakeUpSourceCallbackFuzz>();
    audioManagerListenerStubImpl.SetWakeupSourceCallback(callback);
    bool isActive = g_fuzzUtils.GetData<bool>();
    audioManagerListenerStubImpl.OnWakeupClose();
}

void OnAudioParameterChangeFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    std::shared_ptr<AudioParameterCallbackFuzz> callback = std::make_shared<AudioParameterCallbackFuzz>();
    audioManagerListenerStubImpl.SetParameterCallback(callback);
    std::string networkId = "abc";
    int32_t key = g_fuzzUtils.GetData<int32_t>();
    std::string condition = "abc";
    std::string value = "abc";
    audioManagerListenerStubImpl.OnAudioParameterChange(networkId, key, condition, value);
}

void OnDataTransferStateChangeFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    int32_t callbackId = g_fuzzUtils.GetData<int32_t>();
    AudioRendererDataTransferStateChangeInfo info;
    audioManagerListenerStubImpl.OnDataTransferStateChange(callbackId, info);
}

void AddDataTransferStateChangeCallbackFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    DataTransferMonitorParam param;
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb;
    audioManagerListenerStubImpl.AddDataTransferStateChangeCallback(param, cb);
}

void RemoveDataTransferStateChangeCallbackFuzzTest()
{
    AudioManagerListenerStubImpl audioManagerListenerStubImpl;
    DataTransferMonitorParam param;
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> cb;
    audioManagerListenerStubImpl.AddDataTransferStateChangeCallback(param, cb);
    audioManagerListenerStubImpl.RemoveDataTransferStateChangeCallback(cb);
}

vector<TestFuncs> g_testFuncs = {
    SetWakeupSourceCallbackFuzzTest,
    OnCapturerStateFuzzTest,
    OnWakeupCloseFuzzTest,
    OnAudioParameterChangeFuzzTest,
    OnDataTransferStateChangeFuzzTest,
    AddDataTransferStateChangeCallbackFuzzTest,
    RemoveDataTransferStateChangeCallbackFuzzTest,
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
