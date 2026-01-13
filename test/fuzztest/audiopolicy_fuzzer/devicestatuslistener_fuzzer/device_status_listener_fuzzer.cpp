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

#include <cerrno>
#include "device_status_listener.h"
#include "audio_policy_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const std::string AUDIO_HDI_SERVICE_NAME = "audio_manager_service";

typedef void (*TestPtr)();

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
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
    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
}

void DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
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

    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

void DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    if (deviceStatusListenerPtr == nullptr) {
        return;
    }
    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;
    deviceStatusListenerPtr->hdiServiceManager_ = HDIServiceManagerGet();
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

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
    const std::string type = testTypes[GetData<uint32_t>() % testTypes.size()];
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

    ServiceStatus serviceStatus;
    serviceStatus.serviceName = "audio_manager_service";
    serviceStatus.status = SERVIE_STATUS_CHANGE;
    serviceStatus.info = "EVENT_TYPE=1;DEVICE_TYPE=4;";

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

    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
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

    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
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
    ServiceStatus serviceStatus;
    serviceStatus.serviceName = "daudio_primary_service";
    serviceStatus.status = SERVIE_STATUS_CHANGE;
    serviceStatus.info = "EVENT_TYPE=1;NID=abcd;PIN=123;VID=456;IID=789;";
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
    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
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
    const std::string info = testInfo[GetData<uint32_t>() % testInfo.size()];
    pnpCallback->OnMicrophoneBlocked(info);
}

void DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest1(FuzzedDataProvider& fdp)
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

    deviceStatusListenerPtr->SetAudioDeviceAnahsCallback(nullptr);
    deviceStatusListenerPtr->SetAudioDeviceAnahsCallback(obj);
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
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = mockListener;

    deviceStatusListenerPtr->UnsetAudioDeviceAnahsCallback();
}

void AudioPnpStatusCallbackDestructorFuzzTest(FuzzedDataProvider& fdp)
{
    auto* callback = new AudioPnpStatusCallback();
    callback->SetDeviceStatusListener(nullptr);
    delete callback;
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    DeviceStatusListenerOnMicrophoneBlockedFuzzTest,
    DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest,
    DeviceStatusListenerOnPnpDeviceStatusChangedFuzzTest,
    DeviceStatusListenerUnRegisterDeviceStatusListenerFuzzTest,
    DeviceStatusListenerUpdateAnahsPlatformTypeFuzzTest,
    DeviceStatusListenerRegisterAndCallbackFuzzTest,
    DeviceStatusListenerPnpDeviceTypeBranchFuzzTest,
    DeviceStatusListenerOnPnpDeviceStatusChangedWithAnahsFuzzTest,
    DeviceStatusListenerDaudioServiceBranchFuzzTest,
    AudioPnpStatusCallbackOnPnpDeviceStatusChangedFuzzTest,
    AudioPnpStatusCallbackOnMicrophoneBlockedFuzzTest,
    DeviceStatusListenerSetAudioDeviceAnahsCallbackFuzzTest1,
    DeviceStatusListenerUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioPnpStatusCallbackDestructorFuzzTest,
    });
    func(fdp);
}
void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
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
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}