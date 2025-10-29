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

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
typedef void (*TestPtr)();

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
        sptr<AudioRendererFilter> filter = nullptr) override {};
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress) override {};
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override {};
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand) override {};

    FuzzTestDeviceStatusObserver() = default;
    virtual ~FuzzTestDeviceStatusObserver() = default;
};

template<class T>
uint32_t GetArrLength(T &arr)
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
    if (g_dataSize < g_pos) {
        return object;
    }
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

void AudioUsbManagerInitFuzzTest()
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager.initialized_ = GetData<bool>();
    audioUsbManager.Init(observer);
}

void AudioUsbManagerDeinitFuzzTest()
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    audioUsbManager.eventSubscriber_ = std::make_shared<AudioUsbManager::EventSubscriber>(subscribeInfo);
    audioUsbManager.initialized_ = true;
    audioUsbManager.Deinit();
}

void AudioUsbManagerSubscribeEventFuzzTest()
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    audioUsbManager.eventSubscriber_ = nullptr;
    audioUsbManager.SubscribeEvent();
}

void AudioUsbManagerGetUsbSoundCardMapFuzzTest()
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    audioUsbManager.initialized_ = true;
    audioUsbManager.GetUsbSoundCardMap();
}

void AudioUsbManagerOnReceiveEventFuzzTest()
{
    static const vector<string> matchingSkills = {
        "usual.event.hardware.usb.action.USB_DEVICE_ATTACHED",
        "usual.event.hardware.usb.action.USB_DEVICE_DETACHED",
    };

    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto subscriber = std::make_shared<AudioUsbManager::EventSubscriber>(subscribeInfo);
    if (subscriber == nullptr || matchingSkills.empty()) {
        return;
    }

    EventFwk::CommonEventData data;
    OHOS::EventFwk::Want want;
    string s = "{\"busNum\":1,\"devAddress\":1,\"configs\":[{\"interfaces\":[{\"clazz\":1,\"subClass\":1}]}]}";
    data.SetData(s);
    want.SetAction(matchingSkills[GetData<uint32_t>() % matchingSkills.size()]);
    data.SetWant(want);
    subscriber->OnReceiveEvent(data);
}

void AudioUsbManagerHandleAudioDeviceEventFuzzTest()
{
    AudioUsbManager &audioUsbManager = AudioUsbManager::GetInstance();

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager.Init(observer);

    UsbAudioDevice device;
    SoundCard soundCard;
    soundCard.isPlayer_ = GetData<bool>();
    soundCard.isCapturer_ = GetData<bool>();
    audioUsbManager.soundCardMap_.insert({device.usbAddr_, soundCard});
    audioUsbManager.HandleAudioDeviceEvent(make_pair(device, GetData<bool>()));
    audioUsbManager.Deinit();
}

void AudioUsbManagerNotifyDeviceFuzzTest()
{
    auto audioUsbManager = &AudioUsbManager::GetInstance();
    CHECK_AND_RETURN(audioUsbManager != nullptr);

    auto observer = std::make_shared<FuzzTestDeviceStatusObserver>();
    audioUsbManager->Init(observer);
    CHECK_AND_RETURN(observer != nullptr);

    UsbAudioDevice device;
    SoundCard soundCard;
    soundCard.isPlayer_ = GetData<bool>();
    soundCard.isCapturer_ = GetData<bool>();
    audioUsbManager->soundCardMap_.insert({device.usbAddr_, soundCard});
    audioUsbManager->HandleAudioDeviceEvent(make_pair(device, true));
    audioUsbManager->Deinit();
}

void UsbAddr1FuzzTest()
{
    UsbAddr usbAddr1;
    UsbAddr usbAddr2;
    CHECK_AND_RETURN(usbAddr1 == usbAddr2);
}

void UsbAddr2FuzzTest()
{
    UsbAddr usbAddr1;
    UsbAddr usbAddr2;
    CHECK_AND_RETURN(usbAddr1 < usbAddr2);
}

void UsbAudioDeviceFuzzTest()
{
    UsbAudioDevice usbAudioDevice1;
    UsbAudioDevice usbAudioDevice2;
    CHECK_AND_RETURN(usbAudioDevice1 == usbAudioDevice2);
}

TestPtr g_testPtrs[] = {
    AudioUsbManagerInitFuzzTest,
    AudioUsbManagerDeinitFuzzTest,
    AudioUsbManagerSubscribeEventFuzzTest,
    AudioUsbManagerGetUsbSoundCardMapFuzzTest,
    AudioUsbManagerOnReceiveEventFuzzTest,
    AudioUsbManagerHandleAudioDeviceEventFuzzTest,
    AudioUsbManagerNotifyDeviceFuzzTest,
    UsbAddr1FuzzTest,
    UsbAddr2FuzzTest,
    UsbAudioDeviceFuzzTest
};

void FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    return;
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}