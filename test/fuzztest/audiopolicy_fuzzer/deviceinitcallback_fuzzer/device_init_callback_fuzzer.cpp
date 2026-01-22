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
#include "device_init_callback.h"
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const int32_t TEST_DEVICE_NAME_LENGTH = 4;

typedef void (*TestFuncs)();

void DeviceStatusCallbackImplOnDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    DistributedHardware::DmDeviceBasicInfo dmDeviceBasicInfo;
    strncpy_s(dmDeviceBasicInfo.deviceName, DM_MAX_DEVICE_NAME_LEN,
        "test", TEST_DEVICE_NAME_LENGTH);
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceBasicInfo);
}

void DeviceStatusCallbackImplOnDeviceOfflineFuzzTest(FuzzedDataProvider& fdp)
{
    shared_ptr<DeviceStatusCallbackImpl> deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    CHECK_AND_RETURN(deviceStatusCallbackImpl != nullptr);
    DistributedHardware::DmDeviceInfo dmDeviceInfo;
    strncpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN,
        "test", TEST_DEVICE_NAME_LENGTH);
    deviceStatusCallbackImpl->OnDeviceOffline(dmDeviceInfo);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    DeviceStatusCallbackImplOnDeviceChangedFuzzTest,
    DeviceStatusCallbackImplOnDeviceOfflineFuzzTest,
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
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}