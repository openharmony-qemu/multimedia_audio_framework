/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "device_status_listener.h"
#include "audio_policy_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const std::string AUDIO_HDI_SERVICE_NAME = "audio_manager_service";

typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

void DeviceStatusListenerOnMicrophoneBlockedFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=4;",
        "EVENT_TYPE=1;DEVICE_TYPE=2;",
        "EVENT_TYPE=1;DEVICE_TYPE=8;",
        "EVENT_TYPE=1;DEVICE_TYPE=2048;",
        "EVENT_TYPE=1;DEVICE_TYPE=4096;",
        "EVENT_TYPE=1;DEVICE_TYPE=8192;",
        "EVENT_TYPE=1;DEVICE_TYPE=1;",
        "abc",
    };
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    const std::string info = PickValue(fdp, testInfo);
    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
}


void DeviceStatusListenerOnPnpDeviceStatusChangedFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<string> testInfo = {
        "abc",
        "ANAHS_NAME=test;EVENT_TYPE=1;DEVICE_TYPE=1;DEVICE_ADDRESS=1;",
    };
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }

    const std::string info = PickValue(fdp, testInfo);
    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

void DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->hdiServiceManager_ = HDIServiceManagerGet();
    }
    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    }
    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();
    }
    deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
}

void DeviceStatusListenerUpdateAnahsPlatformTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    static const std::vector<std::string> testTypes = {
        "typeA", "typeB", "", "1234567890"
    };
    const std::string type = PickValue(fdp, testTypes);
    deviceStatusListenerPtr->UpdateAnahsPlatformType(type);
}

void DeviceStatusListenerRegisterAndCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr || deviceStatusListenerPtr->listener_ == nullptr) {
        return;
    }

    int32_t ret = deviceStatusListenerPtr->RegisterDeviceStatusListener();
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("RegisterDeviceStatusListener failed, continue with manual callback");
    }

    static const vector<string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=4;",
        "EVENT_TYPE=1;DEVICE_TYPE=2;",
        "EVENT_TYPE=1;DEVICE_TYPE=8;",
        "abc",
    };
    ServiceStatus serviceStatus;
    serviceStatus.serviceName = ConsumeString(fdp, AUDIO_HDI_SERVICE_NAME).c_str();
    serviceStatus.status = SERVIE_STATUS_CHANGE;
    serviceStatus.info = PickValue(fdp, testInfo).c_str();

    deviceStatusListenerPtr->listener_->callback(deviceStatusListenerPtr->listener_, &serviceStatus);

    deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
}

void DeviceStatusListenerPnpDeviceTypeBranchFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }

    static const vector<string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0_port1;",
        "EVENT_TYPE=1;DEVICE_TYPE=64;EVENT_NAME=Audio_Accessory;DEVICE_ADDRESS=usb_port2;",
        "EVENT_TYPE=1;DEVICE_TYPE=16;EVENT_NAME=DP_Device;DEVICE_ADDRESS=dp_port1;",
        "EVENT_TYPE=2;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0_port1;",
        "EVENT_TYPE=1;DEVICE_TYPE=999;EVENT_NAME=Unknown_Device;DEVICE_ADDRESS=unknown;",
    };

    const std::string info = PickValue(fdp, testInfo);
    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

void DeviceStatusListenerOnPnpDeviceStatusChangedWithAnahsFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }

    class MockAnahsManagerListener : public IStandardAudioAnahsManagerListener {
    public:
        ErrCode OnExtPnpDeviceStatusChanged(const std::string&, const std::string&) override
        {
            return 0;
        }
        sptr<IRemoteObject> AsObject() override
        {
            return nullptr;
        }
    };
    sptr<IStandardAudioAnahsManagerListener> mockListener = new MockAnahsManagerListener();
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = mockListener;
    deviceStatusListenerPtr->UpdateAnahsPlatformType("test_platform");

    static const vector<string> testInfo = {
        "ANAHS_NAME=insert;EVENT_TYPE=1;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0;",
        "ANAHS_NAME=remove;EVENT_TYPE=2;DEVICE_TYPE=64;EVENT_NAME=Accessory_Device;DEVICE_ADDRESS=usb1;",
        "ANAHS_NAME=other;EVENT_TYPE=1;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0;",
    };

    const std::string info = PickValue(fdp, testInfo);
    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

void DeviceStatusListenerDaudioServiceBranchFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr || deviceStatusListenerPtr->listener_ == nullptr) {
        return;
    }
    int32_t ret = deviceStatusListenerPtr->RegisterDeviceStatusListener();
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("RegisterDeviceStatusListener failed, continue with manual callback");
    }
    static const vector<string> testInfo = {
        "EVENT_TYPE=1;NID=abcd;PIN=123;VID=456;IID=789;",
        "EVENT_TYPE=2;NID=xyz;PIN=0;VID=0;IID=0;",
        "abc",
    };
    ServiceStatus serviceStatus;
    serviceStatus.serviceName = ConsumeString(fdp, "daudio_primary_service").c_str();
    serviceStatus.status = SERVIE_STATUS_CHANGE;
    serviceStatus.info = PickValue(fdp, testInfo).c_str();
    deviceStatusListenerPtr->listener_->callback(deviceStatusListenerPtr->listener_, &serviceStatus);
}

void AudioPnpStatusCallbackOnPnpDeviceStatusChangedFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (!deviceStatusListenerPtr) {
        return;
    }
    auto pnpCallback = std::make_shared<AudioPnpStatusCallback>();
    if (pnpCallback == nullptr) {
        return;
    }
    pnpCallback->SetDeviceStatusListener(deviceStatusListenerPtr.get());
    static const std::vector<std::string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0_port1;",
        "ANAHS_NAME=insert;EVENT_TYPE=1;DEVICE_TYPE=32;EVENT_NAME=HDMI_Device;DEVICE_ADDRESS=card0;",
        "EVENT_TYPE=1;DEVICE_TYPE=999;EVENT_NAME=Unknown_Device;DEVICE_ADDRESS=unknown;",
    };
    const std::string info = PickValue(fdp, testInfo);
    pnpCallback->OnPnpDeviceStatusChanged(info);
}

void AudioPnpStatusCallbackOnMicrophoneBlockedFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (!deviceStatusListenerPtr) {
        return;
    }
    auto pnpCallback = std::make_shared<AudioPnpStatusCallback>();
    if (pnpCallback == nullptr) {
        return;
    }
    pnpCallback->SetDeviceStatusListener(deviceStatusListenerPtr.get());

    static const std::vector<std::string> testInfo = {
        "EVENT_TYPE=1;DEVICE_TYPE=4;",
        "EVENT_TYPE=1;DEVICE_TYPE=999;",
        "abc",
    };
    const std::string info = PickValue(fdp, testInfo);
    pnpCallback->OnMicrophoneBlocked(info);
}

void DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (!deviceStatusListenerPtr) {
        return;
    }
    class MockAnahsManagerListener : public IStandardAudioAnahsManagerListener {
    public:
        ErrCode OnExtPnpDeviceStatusChanged(const std::string&, const std::string&) override { return 0; }
        sptr<IRemoteObject> AsObject() override { return nullptr; }
    };
    sptr<IStandardAudioAnahsManagerListener> mockListener = new MockAnahsManagerListener();
    sptr<IRemoteObject> obj = mockListener->AsObject();

    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->audioDeviceAnahsCb_ = mockListener;
    }
    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->SetAudioDeviceAnahsCallback(nullptr);
    }
    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->SetAudioDeviceAnahsCallback(obj);
    }
}

void DeviceStatusListenerUnsetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (!deviceStatusListenerPtr) {
        return;
    }
    class MockAnahsManagerListener : public IStandardAudioAnahsManagerListener {
    public:
        ErrCode OnExtPnpDeviceStatusChanged(const std::string&, const std::string&) override { return 0; }
        sptr<IRemoteObject> AsObject() override { return nullptr; }
    };
    sptr<IStandardAudioAnahsManagerListener> mockListener = new MockAnahsManagerListener();

    if (fdp.ConsumeBool()) {
        deviceStatusListenerPtr->audioDeviceAnahsCb_ = mockListener;
    }
    deviceStatusListenerPtr->UnsetAudioDeviceAnahsCallback();
}

void AudioPnpStatusCallbackDestructorFuzzTest(FuzzedDataProvider& fdp)
{
    auto* callback = new AudioPnpStatusCallback();
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (fdp.ConsumeBool() && deviceStatusListenerPtr != nullptr) {
        callback->SetDeviceStatusListener(deviceStatusListenerPtr.get());
    } else {
        callback->SetDeviceStatusListener(nullptr);
    }
    delete callback;
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    DeviceStatusListenerOnMicrophoneBlockedFuzzTest,
    DeviceStatusListenerOnPnpDeviceStatusChangedFuzzTest,
    DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest,
    DeviceStatusListenerUpdateAnahsPlatformTypeFuzzTest,
    DeviceStatusListenerRegisterAndCallbackFuzzTest,
    DeviceStatusListenerPnpDeviceTypeBranchFuzzTest,
    DeviceStatusListenerOnPnpDeviceStatusChangedWithAnahsFuzzTest,
    DeviceStatusListenerDaudioServiceBranchFuzzTest,
    AudioPnpStatusCallbackOnPnpDeviceStatusChangedFuzzTest,
    AudioPnpStatusCallbackOnMicrophoneBlockedFuzzTest,
    DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest,
    DeviceStatusListenerUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioPnpStatusCallbackDestructorFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}