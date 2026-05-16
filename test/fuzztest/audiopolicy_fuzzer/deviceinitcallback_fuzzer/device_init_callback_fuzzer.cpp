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

#include <string>

#include "audio_log.h"
#include "device_init_callback.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;

void CopyStringToArray(const std::string &src, char *dest, size_t destLen)
{
    if (dest == nullptr || destLen == 0) {
        return;
    }
    std::string value = src.substr(0, destLen - 1);
    if (strncpy_s(dest, destLen, value.c_str(), value.length()) != EOK) {
        dest[0] = '\0';
    }
}

std::string MakeExtraData(FuzzedDataProvider& fdp)
{
    std::string fuzzValue = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    auto extraData = fdp.PickValueInArray<std::string>({
        fuzzValue,
        "",
        "{\"CAR_BRAND\\}",
        "{\"CAR_BRAND\\:B}",
        "{\"CAR_BRAND\\:BUGATTI}",
        "{\"CAR_BRAND\\:\"BUGATTI\"}",
        "{\"CAR_BRAND\\:\\\"BUGATTI\\\",\"OTHER\":1}",
        "{\\\"\\\"CAR_BRAND\\\\\\\": \\\"\\\\\\\"\\\\\\\\\\\\\\\"Bugatti\\\"}",
    });
    if (fdp.ConsumeBool()) {
        extraData += fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    }
    return extraData;
}

DistributedHardware::DmDeviceInfo MakeDmDeviceInfo(FuzzedDataProvider& fdp)
{
    DistributedHardware::DmDeviceInfo dmDeviceInfo;
    CopyStringToArray(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN);
    CopyStringToArray(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), dmDeviceInfo.networkId, DM_MAX_DEVICE_ID_LEN);
    dmDeviceInfo.deviceTypeId = static_cast<decltype(dmDeviceInfo.deviceTypeId)>(fdp.ConsumeIntegral<int32_t>());
    dmDeviceInfo.extraData = MakeExtraData(fdp);
    return dmDeviceInfo;
}

void DeviceStatusCallbackImplOnDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    auto dmDeviceInfo = MakeDmDeviceInfo(fdp);
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceInfo);
}

void DeviceStatusCallbackImplOnDeviceOnlineFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    auto dmDeviceInfo = MakeDmDeviceInfo(fdp);
    deviceStatusCallbackImpl->OnDeviceOnline(dmDeviceInfo);
}

void DeviceStatusCallbackImplOnDeviceOfflineFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    auto dmDeviceInfo = MakeDmDeviceInfo(fdp);
    deviceStatusCallbackImpl->OnDeviceOffline(dmDeviceInfo);
}

void DeviceStatusCallbackImplBasicInfoFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    DistributedHardware::DmDeviceBasicInfo dmDeviceBasicInfo;
    CopyStringToArray(fdp.ConsumeRandomLengthString(MAX_STRING_LEN),
        dmDeviceBasicInfo.deviceName, DM_MAX_DEVICE_NAME_LEN);
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceBasicInfo);
    deviceStatusCallbackImpl->OnDeviceOnline(dmDeviceBasicInfo);
    deviceStatusCallbackImpl->OnDeviceOffline(dmDeviceBasicInfo);
    deviceStatusCallbackImpl->OnDeviceReady(dmDeviceBasicInfo);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        DeviceStatusCallbackImplOnDeviceChangedFuzzTest,
        DeviceStatusCallbackImplOnDeviceOnlineFuzzTest,
        DeviceStatusCallbackImplOnDeviceOfflineFuzzTest,
        DeviceStatusCallbackImplBasicInfoFuzzTest,
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
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
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
