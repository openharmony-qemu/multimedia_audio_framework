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
#include "audio_log.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;
const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;

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
    bool OnQueryIsForceGetDevByVolumeType(const std::string &bundleName) override { return false; }
    bool OnCheckMediaControllerBundle(const std::string &bundleName) override { return false; }
    bool OnQueryIsForceGetZoneDevice(const std::string &bundleName) override { return false; }
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

class AudioVKBInfoMgrCallbackFuzzTest : public AudioVKBInfoMgrCallback {
public:
    AudioVKBInfoMgrCallbackFuzzTest() {}
    bool OnCheckVKBInfo(const std::string &bundleName) override { return false; }
};

void OnInterruptFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    InterruptEventInternal interruptEvent;
    int32_t eventTypeValue = fdp.ConsumeIntegral<int32_t>();
    interruptEvent.eventType = static_cast<InterruptType>(eventTypeValue);
    int32_t forceTypeValue = fdp.ConsumeIntegral<int32_t>();
    interruptEvent.forceType = static_cast<InterruptForceType>(forceTypeValue);
    int32_t hintTypeValue = fdp.ConsumeIntegral<int32_t>();
    interruptEvent.hintType = static_cast<InterruptHint>(hintTypeValue);
    interruptEvent.eventTimestamp = fdp.ConsumeIntegral<int64_t>();
    interruptEvent.callbackToApp = fdp.ConsumeBool();
    policyListenerStub->OnInterrupt(interruptEvent);
    policyListenerStub->callback_ = std::make_shared<AudioInterruptCallbackFuzzTest>();
    if (policyListenerStub->callback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnInterrupt(interruptEvent);
}

void OnAvailableDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    DeviceChangeAction deviceChangeAction;
    uint32_t usage = fdp.ConsumeIntegral<uint32_t>();
    policyListenerStub->audioAvailableDeviceChangeCallback_ =
        std::make_shared<AudioManagerAvailableDeviceChangeCallbackFuzzTest>();
    if (policyListenerStub->audioAvailableDeviceChangeCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnAvailableDeviceChange(usage, deviceChangeAction);
}

void OnQueryClientTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    uint32_t uid = fdp.ConsumeIntegral<uint32_t>();
    bool ret = fdp.ConsumeBool();
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    policyListenerStub->audioQueryClientTypeCallback_ = std::make_shared<AudioQueryClientTypeCallbackFuzzTest>();
    if (policyListenerStub->audioQueryClientTypeCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryClientType(bundleName, uid, ret);
}

void OnCheckClientInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    bool ret = fdp.ConsumeBool();
    policyListenerStub->audioClientInfoMgrCallback_ = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    policyListenerStub->OnCheckClientInfo(bundleName, uid, pid, ret);
}

void OnCheckMediaControllerBundleFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    bool ret = fdp.ConsumeBool();
    policyListenerStub->audioClientInfoMgrCallback_ = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    policyListenerStub->OnCheckMediaControllerBundle(bundleName, ret);
}

void OnQueryAllowedPlaybackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    bool ret = fdp.ConsumeBool();
    policyListenerStub->audioQueryAllowedPlaybackCallback_ =
        std::make_shared<AudioQueryAllowedPlaybackCallbackFuzzTest>();
    if (policyListenerStub->audioQueryAllowedPlaybackCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryAllowedPlayback(uid, pid, ret);
}

void OnBackgroundMuteFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    policyListenerStub->audioBackgroundMuteCallback_ = std::make_shared<AudioBackgroundMuteCallbackFuzzTest>();
    if (policyListenerStub->audioBackgroundMuteCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnBackgroundMute(uid);
}

void OnQueryDeviceVolumeBehaviorFuzzTest(FuzzedDataProvider& fdp)
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

void OnQueryBundleNameIsInListFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string listType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    bool ret = fdp.ConsumeBool();
    policyListenerStub->audioQueryBundleNameListCallback_ =
        std::make_shared<AudioQueryBundleNameListCallbackFuzzTest>();
    if (policyListenerStub->audioQueryBundleNameListCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryBundleNameIsInList(bundleName, listType, ret);
}

void OnRouteUpdateFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    policyListenerStub->audioRouteCallback_ = std::make_shared<AudioRouteCallbackFuzzTest>();
    if (policyListenerStub->audioRouteCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnRouteUpdate(routeFlag, networkId);
}

void SetInterruptCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioInterruptCallbackFuzzTest> sharedCallback =
        std::make_shared<AudioInterruptCallbackFuzzTest>();
    std::weak_ptr callback(sharedCallback);
    policyListenerStub->SetInterruptCallback(callback);
}

void SetQueryClientTypeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback;
    policyListenerStub->SetQueryClientTypeCallback(audioQueryClientTypeCallback);
}

void SetQueryBundleNameListCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryBundleNameListCallback> audioQueryBundleNameListCallback;
    policyListenerStub->SetQueryBundleNameListCallback(audioQueryBundleNameListCallback);
}

void SetQueryDeviceVolumeBehaviorCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest> sharedCallback =
        std::make_shared<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest>();
    std::weak_ptr callback(sharedCallback);
    policyListenerStub->SetQueryDeviceVolumeBehaviorCallback(callback);
}

void SetAudioRouteCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::weak_ptr<AudioRouteCallbackFuzzTest> callback = std::make_shared<AudioRouteCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetAudioRouteCallback(callback);
}

void SetAudioClientInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
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

void SetQueryAllowedPlaybackCallbackFuzzTest(FuzzedDataProvider& fdp)
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

void SetBackgroundMuteCallbackFuzzTest(FuzzedDataProvider& fdp)
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

void OnQueryIsForceGetZoneDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    bool ret = fdp.ConsumeBool();

    policyListenerStub->audioClientInfoMgrCallback_.reset();
    policyListenerStub->OnQueryIsForceGetZoneDevice(bundleName, ret);

    policyListenerStub->audioClientInfoMgrCallback_ = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    policyListenerStub->OnQueryIsForceGetZoneDevice(bundleName, ret);
}

void OnCheckVKBInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    bool isValid = fdp.ConsumeBool();

    policyListenerStub->audioVKBInfoMgrCallback_.reset();
    policyListenerStub->OnCheckVKBInfo(bundleName, isValid);

    policyListenerStub->audioVKBInfoMgrCallback_ = std::make_shared<AudioVKBInfoMgrCallbackFuzzTest>();
    if (policyListenerStub->audioVKBInfoMgrCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnCheckVKBInfo(bundleName, isValid);
}

void OnQueryIsForceGetDevByVolumeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    bool ret = fdp.ConsumeBool();

    policyListenerStub->audioClientInfoMgrCallback_.reset();
    policyListenerStub->OnQueryIsForceGetDevByVolumeType(bundleName, ret);

    policyListenerStub->audioClientInfoMgrCallback_ = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    if (policyListenerStub->audioClientInfoMgrCallback_.lock() == nullptr) {
        return;
    }
    policyListenerStub->OnQueryIsForceGetDevByVolumeType(bundleName, ret);
}

void SetAudioVKBInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto policyListenerStub = std::make_shared<AudioPolicyManagerListenerStubImpl>();
    CHECK_AND_RETURN(policyListenerStub != nullptr);
    std::shared_ptr<AudioVKBInfoMgrCallback> nullCallback;
    std::weak_ptr nullWeakPtr(nullCallback);
    policyListenerStub->SetAudioVKBInfoMgrCallback(nullWeakPtr);

    std::weak_ptr<AudioVKBInfoMgrCallbackFuzzTest> callback =
        std::make_shared<AudioVKBInfoMgrCallbackFuzzTest>();
    if (callback.lock() == nullptr) {
        return;
    }
    policyListenerStub->SetAudioVKBInfoMgrCallback(callback);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    OnInterruptFuzzTest,
    OnAvailableDeviceChangeFuzzTest,
    OnQueryClientTypeFuzzTest,
    OnCheckClientInfoFuzzTest,
    OnCheckMediaControllerBundleFuzzTest,
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
    OnQueryIsForceGetZoneDeviceFuzzTest,
    OnCheckVKBInfoFuzzTest,
    OnQueryIsForceGetDevByVolumeTypeFuzzTest,
    SetAudioVKBInfoMgrCallbackFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
