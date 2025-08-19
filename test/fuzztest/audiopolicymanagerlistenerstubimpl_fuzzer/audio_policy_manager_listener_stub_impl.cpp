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
#include "audio_policy_manager_listener_stub_impl.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

class AudioQueryDeviceVolumeBehaviorCallbackFuzzTest : public AudioQueryDeviceVolumeBehaviorCallback {
public:
    AudioQueryDeviceVolumeBehaviorCallbackFuzzTest() {}
    VolumeBehavior OnQueryDeviceVolumeBehavior() override
    {
        VolumeBehavior volumeBehavior;
        return volumeBehavior;
    }
};

class AudioInterruptCallbackFuzzTest : public AudioInterruptCallback {
public:
    AudioInterruptCallbackFuzzTest() {}
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override {};
};

void OnInterruptFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    InterruptEventInternal interruptEvent;
    policyListenerStub->OnInterrupt(interruptEvent);
}

void OnAvailableDeviceChangeFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    DeviceChangeAction deviceChangeAction;
    uint32_t usage = g_fuzzUtils.GetData<uint32_t>();
    policyListenerStub->OnAvailableDeviceChange(usage, deviceChangeAction);
}

void OnQueryClientTypeFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    uint32_t uid = g_fuzzUtils.GetData<uint32_t>();
    bool ret = g_fuzzUtils.GetData<bool>();
    std::string bundleName = "bundleName";
    policyListenerStub->OnQueryClientType(bundleName, uid, ret);
}

void OnCheckClientInfoFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = "bundleName";
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool ret = g_fuzzUtils.GetData<bool>();
    policyListenerStub->OnCheckClientInfo(bundleName, uid, pid, ret);
}

void OnQueryAllowedPlaybackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool ret = g_fuzzUtils.GetData<bool>();
    policyListenerStub->OnQueryAllowedPlayback(uid, pid, ret);
}

void OnBackgroundMuteFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    policyListenerStub->OnBackgroundMute(uid);
}

void OnQueryDeviceVolumeBehaviorFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    VolumeBehavior volumeBehavior;
    policyListenerStub->OnQueryDeviceVolumeBehavior(volumeBehavior);
}

void OnQueryBundleNameIsInListFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = "bundleName";
    std::string listType = "listType";
    bool ret = g_fuzzUtils.GetData<bool>();
    policyListenerStub->OnQueryBundleNameIsInList(bundleName, listType, ret);
}

void OnRouteUpdateFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string networkId = "networkId";
    uint32_t routeFlag = g_fuzzUtils.GetData<uint32_t>();
    policyListenerStub->OnRouteUpdate(routeFlag, networkId);
}

void SetInterruptCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioInterruptCallback> callback = std::make_shared<AudioInterruptCallbackFuzzTest>();
    policyListenerStub->SetInterruptCallback(callback);
}

void SetQueryClientTypeCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback;
    policyListenerStub->SetQueryClientTypeCallback(audioQueryClientTypeCallback);
}

void SetAudioClientInfoMgrCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioClientInfoMgrCallback> audioClientInfoMgrCallback;
    policyListenerStub->SetAudioClientInfoMgrCallback(audioClientInfoMgrCallback);
}

void SetQueryBundleNameListCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryBundleNameListCallback> audioQueryBundleNameListCallback;
    policyListenerStub->SetQueryBundleNameListCallback(audioQueryBundleNameListCallback);
}

void SetQueryDeviceVolumeBehaviorCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioQueryDeviceVolumeBehaviorCallback> callback =
        std::make_shared<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest>();
    policyListenerStub->SetQueryDeviceVolumeBehaviorCallback(callback);
}

vector<TestFuncs> g_testFuncs = {
    OnInterruptFuzzTest,
    OnAvailableDeviceChangeFuzzTest,
    OnQueryClientTypeFuzzTest,
    OnCheckClientInfoFuzzTest,
    OnQueryAllowedPlaybackFuzzTest,
    OnBackgroundMuteFuzzTest,
    OnQueryDeviceVolumeBehaviorFuzzTest,
    OnQueryBundleNameIsInListFuzzTest,
    OnRouteUpdateFuzzTest,
    SetInterruptCallbackFuzzTest,
    SetQueryClientTypeCallbackFuzzTest,
    SetQueryBundleNameListCallbackFuzzTest,
    SetQueryDeviceVolumeBehaviorCallbackFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}