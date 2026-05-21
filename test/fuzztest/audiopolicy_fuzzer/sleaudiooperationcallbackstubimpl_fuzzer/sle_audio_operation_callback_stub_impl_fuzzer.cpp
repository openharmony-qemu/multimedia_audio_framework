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
#include "audio_info.h"
#include "audio_device_info.h"
#include "sle_audio_operation_callback_stub_impl.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

const vector<AudioStreamType> g_testAudioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

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

void GetSleAudioDeviceListFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::vector<AudioDeviceDescriptor> devices;
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = PickValue(fdp, g_testDeviceTypes);
    deviceDesc.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    deviceDesc.deviceName_ = ConsumeString(fdp, "sle_fuzz_device");
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleAudioDeviceList(devices);
}

void GetSleAudioDeviceListFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::vector<AudioDeviceDescriptor> devices;
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = PickValue(fdp, g_testDeviceTypes);
    deviceDesc.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    deviceDesc.deviceName_ = ConsumeString(fdp, "sle_fuzz_device");
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleAudioDeviceList(devices);
}

void GetSleVirtualAudioDeviceListFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::vector<AudioDeviceDescriptor> devices;
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = PickValue(fdp, g_testDeviceTypes);
    deviceDesc.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    deviceDesc.deviceName_ = ConsumeString(fdp, "sle_fuzz_device");
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleVirtualAudioDeviceList(devices);
}

void GetSleVirtualAudioDeviceListFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::vector<AudioDeviceDescriptor> devices;
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = PickValue(fdp, g_testDeviceTypes);
    deviceDesc.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    deviceDesc.deviceName_ = ConsumeString(fdp, "sle_fuzz_device");
    devices.push_back(deviceDesc);
    operationCallbackStub->GetSleVirtualAudioDeviceList(devices);
}

void IsInBandRingOpenFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    bool ret = fdp.ConsumeBool();
    operationCallbackStub->IsInBandRingOpen(device, ret);
}

void IsInBandRingOpenFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    bool ret = fdp.ConsumeBool();
    operationCallbackStub->IsInBandRingOpen(device, ret);
}

void GetSupportStreamTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t retType = fdp.ConsumeIntegral<uint32_t>();
    operationCallbackStub->GetSupportStreamType(device, retType);
}

void GetSupportStreamTypeFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t retType = fdp.ConsumeIntegral<uint32_t>();
    operationCallbackStub->GetSupportStreamType(device, retType);
}

void SetActiveSinkDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SetActiveSinkDevice(device, streamType, ret);
}

void SetActiveSinkDeviceFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SetActiveSinkDevice(device, streamType, ret);
}

void StartPlayingFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    int32_t time = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->StartPlaying(device, streamType, time, ret);
}

void StartPlayingFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    int32_t time = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->StartPlaying(device, streamType, time, ret);
}

void StopPlayingFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->StopPlaying(device, streamType, ret);
}

void StopPlayingFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->StopPlaying(device, streamType, ret);
}

void ConnectAllowedProfilesFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string remoteAddr = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->ConnectAllowedProfiles(remoteAddr, ret);
}

void ConnectAllowedProfilesFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string remoteAddr = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->ConnectAllowedProfiles(remoteAddr, ret);
}

void SetDeviceAbsVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string remoteAddr = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    uint32_t volume = fdp.ConsumeIntegral<uint32_t>();
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SetDeviceAbsVolume(remoteAddr, volume, streamType, ret);
}

void SetDeviceAbsVolumeFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string remoteAddr = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    uint32_t volume = fdp.ConsumeIntegral<uint32_t>();
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SetDeviceAbsVolume(remoteAddr, volume, streamType, ret);
}

void SendUserSelectionFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SendUserSelection(device, streamType, USER_SELECT_SLE, ret);
}

void SendUserSelectionFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t streamType = static_cast<uint32_t>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    operationCallbackStub->SendUserSelection(device, streamType, USER_SELECT_SLE, ret);
}

void GetRenderPositionFuzzTest(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    operationCallbackStub->sleAudioOperationCallback_ = std::make_shared<SleAudioOperationCallbackTest>();
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t delayValue = fdp.ConsumeIntegral<uint32_t>();
    operationCallbackStub->GetRenderPosition(device, delayValue);
}

void GetRenderPositionFuzzTestNullCallback(FuzzedDataProvider& fdp)
{
    auto operationCallbackStub = std::make_shared<SleAudioOperationCallbackStubImpl>();
    CHECK_AND_RETURN(operationCallbackStub != nullptr);
    std::string device = ConsumeString(fdp, "test_device");
    uint32_t delayValue = fdp.ConsumeIntegral<uint32_t>();
    operationCallbackStub->GetRenderPosition(device, delayValue);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    GetSleAudioDeviceListFuzzTest,
    GetSleAudioDeviceListFuzzTestNullCallback,
    GetSleVirtualAudioDeviceListFuzzTest,
    GetSleVirtualAudioDeviceListFuzzTestNullCallback,
    IsInBandRingOpenFuzzTest,
    IsInBandRingOpenFuzzTestNullCallback,
    GetSupportStreamTypeFuzzTest,
    GetSupportStreamTypeFuzzTestNullCallback,
    SetActiveSinkDeviceFuzzTest,
    SetActiveSinkDeviceFuzzTestNullCallback,
    StartPlayingFuzzTest,
    StartPlayingFuzzTestNullCallback,
    StopPlayingFuzzTest,
    StopPlayingFuzzTestNullCallback,
    ConnectAllowedProfilesFuzzTest,
    ConnectAllowedProfilesFuzzTestNullCallback,
    SetDeviceAbsVolumeFuzzTest,
    SetDeviceAbsVolumeFuzzTestNullCallback,
    SendUserSelectionFuzzTest,
    SendUserSelectionFuzzTestNullCallback,
    GetRenderPositionFuzzTest,
    GetRenderPositionFuzzTestNullCallback,
    });
    func(fdp);
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
    return 0;
}