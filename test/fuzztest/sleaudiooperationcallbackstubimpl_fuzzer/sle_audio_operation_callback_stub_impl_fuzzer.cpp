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
#include "sle_audio_operation_callback_stub_impl.h"

#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

typedef void (*TestFuncs)();

class SleAudioOperationCallbackTest : public SleAudioOperationCallback {
public:
    virtual ~SleAudioOperationCallbackTest() {};
    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {};
    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {};
    bool IsInBandRingOpen(const std::string &device) const override { return false; };
    uint32_t GetSupportStreamType(const std::string &device) const override { return 0; };
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override { return 0; };
    int32_t StartPlaying(const std::string &device, uint32_t streamType, int32_t timeoutMs) override { return 0; };
    int32_t StopPlaying(const std::string &device, uint32_t streamType) override { return 0; };
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) const override { return 0; };
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume,
        uint32_t streamType) override { return 0; };
    int32_t SendUserSelection(const std::string &device, uint32_t streamType, int32_t eventType) override { return 0; };
    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override { return 0; };
};

void GetSleAudioDeviceListFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::vector<AudioDeviceDescriptor> devices;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(deviceDesc != nullptr);
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleAudioDeviceList(devices);
}

void GetSleVirtualAudioDeviceListFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::vector<AudioDeviceDescriptor> devices;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(deviceDesc != nullptr);
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleVirtualAudioDeviceList(devices);
}

void IsInBandRingOpenFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    bool ret = g_fuzzUtils.GetData<bool>();
    operationCallbackStub->IsInBandRingOpen(device, ret);
}

void GetSupportStreamTypeFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t retType = g_fuzzUtils.GetData<uint32_t>();
    operationCallbackStub->GetSupportStreamType(device, retType);
}

void SetActiveSinkDeviceFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t streamType = g_fuzzUtils.GetData<uint32_t>();
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->SetActiveSinkDevice(device, streamType, ret);
}

void StartPlayingFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t streamType = g_fuzzUtils.GetData<uint32_t>();
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    int32_t time = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->StartPlaying(device, streamType, time, ret);
}

void StopPlayingFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t streamType = g_fuzzUtils.GetData<uint32_t>();
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->StopPlaying(device, streamType, ret);
}

void ConnectAllowedProfilesFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string remoteAddr = "00:11:22:33:44:55";
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->ConnectAllowedProfiles(remoteAddr, ret);
}

void SetDeviceAbsVolumeFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string remoteAddr = "00:11:22:33:44:55";
    uint32_t volume = g_fuzzUtils.GetData<uint32_t>();
    uint32_t streamType = g_fuzzUtils.GetData<uint32_t>();
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->SetDeviceAbsVolume(remoteAddr, volume, streamType, ret);
}

void SendUserSelectionFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t streamType = g_fuzzUtils.GetData<uint32_t>();
    int32_t ret = g_fuzzUtils.GetData<int32_t>();
    operationCallbackStub->SendUserSelection(device, streamType, USER_SELECT_SLE, ret);
}

void GetRenderPositionFuzzTest()
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    if (operationCallbackStub->sleAudioOperationCallback_.lock() == nullptr) {
        return;
    }
    std::string device = "test_device";
    uint32_t delayValue = g_fuzzUtils.GetData<uint32_t>();
    operationCallbackStub->GetRenderPosition(device, delayValue);
}

vector<TestFuncs> g_testFuncs = {
    GetSleAudioDeviceListFuzzTest,
    GetSleVirtualAudioDeviceListFuzzTest,
    IsInBandRingOpenFuzzTest,
    GetSupportStreamTypeFuzzTest,
    SetActiveSinkDeviceFuzzTest,
    StartPlayingFuzzTest,
    StopPlayingFuzzTest,
    ConnectAllowedProfilesFuzzTest,
    SetDeviceAbsVolumeFuzzTest,
    SendUserSelectionFuzzTest,
    GetRenderPositionFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
