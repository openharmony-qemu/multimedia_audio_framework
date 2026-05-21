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

#include "audio_log.h"
#include "audio_usb_manager.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
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

const vector<string> g_testMatchingSkills = {
    "usual.event.hardware.usb.action.USB_DEVICE_ATTACHED",
    "usual.event.hardware.usb.action.USB_DEVICE_DETACHED",
};

class FuzzTestDeviceStatusObserver : public IDeviceStatusObserver {
public:
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false) override {};
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status) override {};
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {};
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo) override {};
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false) override {};
    void OnServiceConnected(AudioServiceIndex serviceIndex) override {};
    void OnServiceDisconnected(AudioServiceIndex serviceIndex) override {};
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr, const std::string &caller = "") override {};
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
        const std::string &caller = "") override {};
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {};
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) override {};
    void OnConnectFailed(AudioDeviceDescriptor &desc) override {}

    FuzzTestDeviceStatusObserver() = default;
    virtual ~FuzzTestDeviceStatusObserver() = default;
};

void AudioUsbManagerInitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager.initialized_ = fdp.ConsumeBool();
    audioUsbManager.Init(observer);
}

void AudioUsbManagerDeinitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    audioUsbManager.eventSubscriber_ = std::make_shared<AudioUsbManager::EventSubscriber>(subscribeInfo);
    audioUsbManager.initialized_ = fdp.ConsumeBool();
    audioUsbManager.Deinit();
}

void AudioUsbManagerSubscribeEventFuzzTest(FuzzedDataProvider& fdp)
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    audioUsbManager.initialized_ = fdp.ConsumeBool();
    audioUsbManager.eventSubscriber_ = nullptr;
    audioUsbManager.SubscribeEvent();
}

void AudioUsbManagerGetUsbSoundCardMapFuzzTest(FuzzedDataProvider& fdp)
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    audioUsbManager.initialized_ = fdp.ConsumeBool();
    audioUsbManager.GetUsbSoundCardMap();
}

void AudioUsbManagerOnReceiveEventFuzzTest(FuzzedDataProvider& fdp)
{

    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto subscriber = std::make_shared<AudioUsbManager::EventSubscriber>(subscribeInfo);
    if (subscriber == nullptr) {
        return;
    }

    EventFwk::CommonEventData data;
    OHOS::EventFwk::Want want;
    string s = "{\"busNum\":1,\"devAddress\":1,\"configs\":[{\"interfaces\":[{\"clazz\":1,\"subClass\":1}]}]}";
    data.SetData(s);
    want.SetAction(PickValue(fdp, g_testMatchingSkills));
    data.SetWant(want);
    subscriber->OnReceiveEvent(data);
}

void AudioUsbManagerHandleAudioDeviceEventFuzzTest(FuzzedDataProvider& fdp)
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager.Init(observer);

    UsbAudioDevice device;
    device.usbAddr_.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    device.usbAddr_.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    device.name_ = ConsumeString(fdp, "usb_fuzz_device");
    device.cardNum_ = fdp.ConsumeIntegral<uint32_t>();
    device.isCapturer_ = fdp.ConsumeBool();
    device.isPlayer_ = fdp.ConsumeBool();
    bool isConnected = fdp.ConsumeBool();
    audioUsbManager.HandleAudioDeviceEvent(make_pair(device, isConnected));
    audioUsbManager.Deinit();
}

void AudioUsbManagerNotifyDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioUsbManager = &AudioUsbManager::GetInstance();
    CHECK_AND_RETURN(audioUsbManager != nullptr);

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager->Init(observer);
    CHECK_AND_RETURN(observer != nullptr);

    UsbAudioDevice device;
    device.usbAddr_.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    device.usbAddr_.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    device.name_ = ConsumeString(fdp, "usb_fuzz_device");
    device.cardNum_ = fdp.ConsumeIntegral<uint32_t>();
    device.isCapturer_ = fdp.ConsumeBool();
    device.isPlayer_ = fdp.ConsumeBool();
    bool isConnected = fdp.ConsumeBool();
    audioUsbManager->HandleAudioDeviceEvent(make_pair(device, isConnected));
    audioUsbManager->Deinit();
}

void UsbAddr1FuzzTest(FuzzedDataProvider& fdp)
{
    UsbAddr usbAddr1;
    usbAddr1.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAddr1.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    UsbAddr usbAddr2;
    usbAddr2.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAddr2.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    bool isEqual = (usbAddr1 == usbAddr2);
}

void UsbAddr2FuzzTest(FuzzedDataProvider& fdp)
{
    UsbAddr usbAddr1;
    usbAddr1.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAddr1.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    UsbAddr usbAddr2;
    usbAddr2.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAddr2.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    bool isLess = (usbAddr1 < usbAddr2);
}

void UsbAudioDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    UsbAudioDevice usbAudioDevice1;
    usbAudioDevice1.usbAddr_.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAudioDevice1.usbAddr_.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    usbAudioDevice1.name_ = ConsumeString(fdp, "usb_fuzz_device1");
    usbAudioDevice1.cardNum_ = fdp.ConsumeIntegral<uint32_t>();
    usbAudioDevice1.isCapturer_ = fdp.ConsumeBool();
    usbAudioDevice1.isPlayer_ = fdp.ConsumeBool();
    UsbAudioDevice usbAudioDevice2;
    usbAudioDevice2.usbAddr_.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAudioDevice2.usbAddr_.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    usbAudioDevice2.name_ = ConsumeString(fdp, "usb_fuzz_device2");
    usbAudioDevice2.cardNum_ = fdp.ConsumeIntegral<uint32_t>();
    usbAudioDevice2.isCapturer_ = fdp.ConsumeBool();
    usbAudioDevice2.isPlayer_ = fdp.ConsumeBool();
    bool isEqual = (usbAudioDevice1 == usbAudioDevice2);
}

void AudioUsbManagerSoundCard(FuzzedDataProvider& fdp)
{
    auto &man = AudioUsbManager::GetInstance();
    auto observer = make_shared<FuzzTestDeviceStatusObserver>();
    man.SetObserver(observer);
    UsbAddr usbAddr;
    usbAddr.busNum_ = fdp.ConsumeIntegral<uint8_t>();
    usbAddr.devAddr_ = fdp.ConsumeIntegral<uint8_t>();
    std::string name = ConsumeString(fdp, "test_usb_name");
    man.UpdateDeviceName(usbAddr, name);
    man.pendingMap_[usbAddr] = ConsumeString(fdp, "1");
    man.UpdateDeviceName(usbAddr, name);
    man.AddDeviceBySoundCard(fdp.ConsumeIntegral<uint32_t>());
    std::string soundCardId = ConsumeString(fdp, "2");
    bool isConnected = fdp.ConsumeBool();
    man.NotifySoundCardChange(soundCardId, isConnected);
    man.audioDevices_.push_back({
        .cardNum_ = fdp.ConsumeIntegral<uint32_t>(),
    });
    man.NotifySoundCardChange(soundCardId, !isConnected);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioUsbManagerInitFuzzTest,
    AudioUsbManagerDeinitFuzzTest,
    AudioUsbManagerSubscribeEventFuzzTest,
    AudioUsbManagerGetUsbSoundCardMapFuzzTest,
    AudioUsbManagerOnReceiveEventFuzzTest,
    AudioUsbManagerHandleAudioDeviceEventFuzzTest,
    AudioUsbManagerNotifyDeviceFuzzTest,
    AudioUsbManagerSoundCard,
    UsbAddr1FuzzTest,
    UsbAddr2FuzzTest,
    UsbAudioDeviceFuzzTest
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