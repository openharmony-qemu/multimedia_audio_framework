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

class AudioManagerAvailableDeviceChangeCallbackFuzzTest : public AudioManagerAvailableDeviceChangeCallback {
public:
    AudioManagerAvailableDeviceChangeCallbackFuzzTest() {}
    void OnAvailableDeviceChange(const AudioDeviceUsage usage,
        const DeviceChangeAction &deviceChangeAction) override {};
};

class AudioQueryClientTypeCallbackFuzzTest : public AudioQueryClientTypeCallback {
public:
    AudioQueryClientTypeCallbackFuzzTest() {}
    bool OnQueryClientType(const std::string &bundleName, uint32_t uid) override { return false; }
};

class AudioClientInfoMgrCallbackFuzzTest : public AudioClientInfoMgrCallback {
public:
    AudioClientInfoMgrCallbackFuzzTest() {}
    bool OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid) override { return false; }
};

class AudioQueryAllowedPlaybackCallbackFuzzTest : public AudioQueryAllowedPlaybackCallback {
public:
    AudioQueryAllowedPlaybackCallbackFuzzTest() {}
    bool OnQueryAllowedPlayback(int32_t uid, int32_t pid) override { return false; }
};

class AudioBackgroundMuteCallbackFuzzTest : public AudioBackgroundMuteCallback {
public:
    AudioBackgroundMuteCallbackFuzzTest() {}
    void OnBackgroundMute(const int32_t uid) override {};
};

class AudioQueryBundleNameListCallbackFuzzTest : public AudioQueryBundleNameListCallback {
public:
    AudioQueryBundleNameListCallbackFuzzTest() {}
    bool OnQueryBundleNameIsInList(const std::string &bundleName,
        const std::string &listType) override { return false; }
};

class AudioRouteCallbackFuzzTest : public AudioRouteCallback {
public:
    AudioRouteCallbackFuzzTest() {}
    void OnRouteUpdate(uint32_t routeFlag, const std::string &networkId) override {};
};

void OnInterruptFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    InterruptEventInternal interruptEvent;
    policyListenerStub->OnInterrupt(interruptEvent);
    policyListenerStub->callback_ = std::make_shared<AudioInterruptCallbackFuzzTest>();
    if (policyListenerStub->callback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnInterrupt(interruptEvent);
}

void OnAvailableDeviceChangeFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    DeviceChangeAction deviceChangeAction;
    uint32_t usage = g_fuzzUtils.GetData<uint32_t>();
    policyListenerStub->audioAvailableDeviceChangeCallback_ =
        std::make_shared<AudioManagerAvailableDeviceChangeCallbackFuzzTest>();
    if (policyListenerStub->audioAvailableDeviceChangeCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnAvailableDeviceChange(usage, deviceChangeAction);
}

void OnQueryClientTypeFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    uint32_t uid = g_fuzzUtils.GetData<uint32_t>();
    bool ret = g_fuzzUtils.GetData<bool>();
    std::string bundleName = "bundleName";
    policyListenerStub->audioQueryClientTypeCallback_ = std::make_shared<AudioQueryClientTypeCallbackFuzzTest>();
    if (policyListenerStub->audioQueryClientTypeCallback_.lock() == nullptr) {
        return;
    }
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
    policyListenerStub->audioClientInfoMgrCallback_ = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    if (policyListenerStub->audioClientInfoMgrCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnCheckClientInfo(bundleName, uid, pid, ret);
}

void OnQueryAllowedPlaybackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool ret = g_fuzzUtils.GetData<bool>();
    policyListenerStub->audioQueryAllowedPlaybackCallback_ =
        std::make_shared<AudioQueryAllowedPlaybackCallbackFuzzTest>();
    if (policyListenerStub->audioQueryAllowedPlaybackCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryAllowedPlayback(uid, pid, ret);
}

void OnBackgroundMuteFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    policyListenerStub->audioBackgroundMuteCallback_ = std::make_shared<AudioBackgroundMuteCallbackFuzzTest>();
    if (policyListenerStub->audioBackgroundMuteCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnBackgroundMute(uid);
}

void OnQueryDeviceVolumeBehaviorFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    VolumeBehavior volumeBehavior;
    policyListenerStub->audioQueryDeviceVolumeBehaviorCallback_ =
        std::make_shared<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest>();
    if (policyListenerStub->audioQueryDeviceVolumeBehaviorCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryDeviceVolumeBehavior(volumeBehavior);
}

void OnQueryBundleNameIsInListFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = "bundleName";
    std::string listType = "listType";
    bool ret = g_fuzzUtils.GetData<bool>();
    policyListenerStub->audioQueryBundleNameListCallback_ =
        std::make_shared<AudioQueryBundleNameListCallbackFuzzTest>();
    if (policyListenerStub->audioQueryBundleNameListCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryBundleNameIsInList(bundleName, listType, ret);
}

void OnRouteUpdateFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string networkId = "networkId";
    uint32_t routeFlag = g_fuzzUtils.GetData<uint32_t>();
    policyListenerStub->audioRouteCallback_ = std::make_shared<AudioRouteCallbackFuzzTest>();
    if (policyListenerStub->audioRouteCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnRouteUpdate(routeFlag, networkId);
}

void SetInterruptCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioInterruptCallbackFuzzTest> sharedCallback =
        std::make_shared<AudioInterruptCallbackFuzzTest>();
    std::weak_ptr callback(sharedCallback);
    policyListenerStub->SetInterruptCallback(callback);
}

void SetQueryClientTypeCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback;
    policyListenerStub->SetQueryClientTypeCallback(audioQueryClientTypeCallback);
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
    std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest> sharedCallback =
        std::make_shared<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest>();
    std::weak_ptr callback(sharedCallback);
    policyListenerStub->SetQueryDeviceVolumeBehaviorCallback(callback);
}

void SetAudioRouteCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioRouteCallbackFuzzTest> callback = std::make_shared<AudioRouteCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetAudioRouteCallback(callback);
}

void SetAudioClientInfoMgrCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioClientInfoMgrCallbackFuzzTest> callback =
        std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetAudioClientInfoMgrCallback(callback);
}

void SetQueryAllowedPlaybackCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioQueryAllowedPlaybackCallbackFuzzTest> callback =
        std::make_shared<AudioQueryAllowedPlaybackCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetQueryAllowedPlaybackCallback(callback);
}

void SetBackgroundMuteCallbackFuzzTest()
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioBackgroundMuteCallbackFuzzTest> callback =
        std::make_shared<AudioBackgroundMuteCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetBackgroundMuteCallback(callback);
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
    SetAudioRouteCallbackFuzzTest,
    SetAudioClientInfoMgrCallbackFuzzTest,
    SetQueryAllowedPlaybackCallbackFuzzTest,
    SetBackgroundMuteCallbackFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}