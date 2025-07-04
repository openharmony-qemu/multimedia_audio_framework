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

#include <fstream>
#include "audio_input_thread.h"
#include "audio_log.h"
#include "audio_pnp_server.h"
#include "audio_socket_thread.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

class TestAudioPnpDeviceChangeCallback : public AudioPnpDeviceChangeCallback {
public:
    virtual ~TestAudioPnpDeviceChangeCallback() {};
    void OnPnpDeviceStatusChanged(const std::string &info) {};
    void OnMicrophoneBlocked(const std::string &info) {};
};

const int32_t NUM_2 = 2;
AudioPnpServer *audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<PnpEventType> g_testPnpEventTypes = {
    PNP_EVENT_DEVICE_ADD,
    PNP_EVENT_DEVICE_REMOVE,
    PNP_EVENT_LOAD_SUCCESS,
    PNP_EVENT_LOAD_FAILURE,
    PNP_EVENT_UNLOAD,
    PNP_EVENT_SERVICE_VALID,
    PNP_EVENT_SERVICE_INVALID,
    PNP_EVENT_CAPTURE_THRESHOLD,
    PNP_EVENT_UNKNOWN,
    PNP_EVENT_MIC_BLOCKED,
    PNP_EVENT_MIC_UNBLOCKED,
};

const vector<PnpDeviceType> g_testPnpDeviceTypes = {
    PNP_DEVICE_LINEOUT,
    PNP_DEVICE_HEADPHONE,
    PNP_DEVICE_HEADSET,
    PNP_DEVICE_USB_HEADSET,
    PNP_DEVICE_USB_HEADPHONE,
    PNP_DEVICE_USBA_HEADSET,
    PNP_DEVICE_USBA_HEADPHONE,
    PNP_DEVICE_PRIMARY_DEVICE,
    PNP_DEVICE_USB_DEVICE,
    PNP_DEVICE_A2DP_DEVICE,
    PNP_DEVICE_HDMI_DEVICE,
    PNP_DEVICE_ADAPTER_DEVICE,
    PNP_DEVICE_DP_DEVICE,
    PNP_DEVICE_MIC,
    PNP_DEVICE_ACCESSORY,
    PNP_DEVICE_UNKNOWN,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioPnpServerUnRegisterPnpStatusListenerFuzzTest(const uint8_t *rawData, size_t size)
{
    audioPnpServer_->UnRegisterPnpStatusListener();
}

void AudioPnpServerOnPnpDeviceStatusChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    std::string info = "test_info";
    std::shared_ptr<AudioPnpDeviceChangeCallback> pnpCallback =
        std::make_shared<TestAudioPnpDeviceChangeCallback>();
    audioPnpServer_->pnpCallback_ = pnpCallback;
    audioPnpServer_->OnPnpDeviceStatusChanged(info);
}

void AudioPnpServerOnMicrophoneBlockedFuzzTest(const uint8_t *rawData, size_t size)
{
    MicrophoneBlocked::GetInstance().OnMicrophoneBlocked("test_info", *audioPnpServer_);
}

void AudioSocketThreadIsUpdatePnpDeviceStateFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioSocketThread audioSocketThread;
    PnpEventType pnpEventType = g_testPnpEventTypes[index % g_testPnpEventTypes.size()];
    PnpDeviceType pnpDeviceType = g_testPnpDeviceTypes[index % g_testPnpDeviceTypes.size()];
    AudioEvent event;
    event.eventType = static_cast<uint32_t>(pnpEventType);
    event.deviceType = static_cast<uint32_t>(pnpDeviceType);
    event.name = "Device1";
    event.address = "Address1";
    event.anahsName = "AnahsName1";

    audioSocketThread.UpdatePnpDeviceState(&event);
    audioSocketThread.IsUpdatePnpDeviceState(&event);
}

void AudioSocketThreadSetAudioPnpServerEventValueFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<string> testSubSystem = {
        "audio",
        "switch",
    };
    static const vector<string> testSwitchState = {
        "0",
        "1",
        "2",
        "3",
        "4",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioSocketThread audioSocketThread;
    PnpEventType pnpEventType = g_testPnpEventTypes[index % g_testPnpEventTypes.size()];
    PnpDeviceType pnpDeviceType = g_testPnpDeviceTypes[index % g_testPnpDeviceTypes.size()];
    AudioEvent event;
    event.eventType = static_cast<uint32_t>(pnpEventType);
    event.deviceType = static_cast<uint32_t>(pnpDeviceType);
    event.name = "Device1";
    event.address = "Address1";
    event.anahsName = "AnahsName1";
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "MICROPHONE=0",
        .devType = "extcon3",
        .subSystem = testSubSystem[index % testSubSystem.size()].c_str(),
        .switchName = "h2w",
        .switchState = testSwitchState[index % testSwitchState.size()].c_str(),
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    audioSocketThread.SetAudioPnpServerEventValue(&event, &uevent);
}

void AudioSocketThreadSetAudioAnahsEventValueFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<string> testAnahsNames = {
        "INSERT",
        "REMOVE",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioSocketThread audioSocketThread;
    PnpEventType pnpEventType = g_testPnpEventTypes[index % g_testPnpEventTypes.size()];
    PnpDeviceType pnpDeviceType = g_testPnpDeviceTypes[index % g_testPnpDeviceTypes.size()];
    AudioEvent event;
    event.eventType = static_cast<uint32_t>(pnpEventType);
    event.deviceType = static_cast<uint32_t>(pnpDeviceType);
    event.name = "Device1";
    event.address = "Address1";
    event.anahsName = "AnahsName1";
    AudioPnpUevent uevent = {
        .subSystem = "platform",
        .anahsName = testAnahsNames[index % testAnahsNames.size()].c_str(),
    };

    audioSocketThread.SetAudioAnahsEventValue(&event, &uevent);
}

void AudioSocketThreadAudioNnDetectDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<string> testNames = {
        "send_nn_state0",
        "send_nn_state1",
        "send_nn_state2",
        "send_nn_state3",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioSocketThread audioSocketThread;
    AudioPnpUevent uevent = {
        .action = "change",
        .name = testNames[index % testNames.size()].c_str(),
    };

    audioSocketThread.AudioNnDetectDevice(&uevent);
}

void AudioSocketThreadAudioDpDetectDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<string> testSwitchStates = {
        "0",
        "1",
        "2",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioSocketThread audioSocketThread;
    AudioPnpUevent uevent = {
        .subSystem = "switch",
        .switchName = "hdmi_audio",
        .action = "change",
        .switchState = testSwitchStates[index % testSwitchStates.size()].c_str(),
    };
    audioSocketThread.AudioDpDetectDevice(&uevent);
}

void AudioSocketThreadAudioMicBlockDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<string> testAudioPnpUeventName = {
        "mic_blocked",
        "mic_un_blocked",
        "test_name",
    };
    auto audioSocketThread = std::make_shared<AudioSocketThread>();

    bool testBool = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.name = testAudioPnpUeventName[static_cast<uint32_t>(size) % testAudioPnpUeventName.size()].c_str();
    struct AudioPnpUevent *audioPnpUeventPtr = &audioPnpUevent;
    if (testBool) {
        audioPnpUeventPtr = nullptr;
    }
    audioSocketThread->AudioMicBlockDevice(audioPnpUeventPtr);
}

void AudioSocketThreadUpdateDeviceStateFuzzTest(const uint8_t *rawData, size_t size)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    AudioEvent updateEvent = {
        static_cast<uint32_t>(size) % NUM_2,
        static_cast<uint32_t>(size) % NUM_2 + 1,
        "device",
        "address",
        "anahs",
    };

    audioSocketThread->UpdateDeviceState(updateEvent);
}

void AudioSocketThreadReadAndScanDpNameFuzzTest(const uint8_t *rawData, size_t size)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    std::string testPath = "/tmp/test_path";
    std::string testName = "test_name";
    std::ofstream file(testPath);
    file << testName;
    file.close();
    std::string name;
    audioSocketThread->ReadAndScanDpName(testPath, name);
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioPnpServerUnRegisterPnpStatusListenerFuzzTest,
    OHOS::AudioStandard::AudioPnpServerOnPnpDeviceStatusChangedFuzzTest,
    OHOS::AudioStandard::AudioPnpServerOnMicrophoneBlockedFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadIsUpdatePnpDeviceStateFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadSetAudioPnpServerEventValueFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadSetAudioAnahsEventValueFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadAudioNnDetectDeviceFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadAudioDpDetectDeviceFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadAudioMicBlockDeviceFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadUpdateDeviceStateFuzzTest,
    OHOS::AudioStandard::AudioSocketThreadReadAndScanDpNameFuzzTest,
};

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint32_t len = OHOS::AudioStandard::GetArrLength(g_testPtrs);
    if (len > 0) {
        uint8_t firstByte = *data % len;
        if (firstByte >= len) {
            return 0;
        }
        data = data + 1;
        size = size - 1;
        g_testPtrs[firstByte](data, size);
    }
    return 0;
}