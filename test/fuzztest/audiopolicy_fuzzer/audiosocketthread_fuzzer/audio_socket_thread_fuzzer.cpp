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

#include <fstream>
#include <securec.h>

#include "audio_log.h"
#include "audio_socket_thread.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const uint32_t UEVENT_MSG_PADDING = 2;
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

const vector<string> g_testSubSystems = {
    "switch", "invalid",
};

const vector<string> g_testSwitchNames = {
    "hdmi_mipi_audio", "invalid", "hdmi_mipi_audio,device_port=HDMI-0",
};

const vector<string> g_testActions = {
    "change", "invalid",
};

const vector<string> g_testSwitchStates = {
    "1", "0", "invalid",
};

const vector<string> g_testMsgs = {
    "libudev",
    "test message",
    "unmatched event",
    "matched event",
};

const vector<string> g_testUeventActions = {
    "add", "remove", "change",
};

const vector<string> g_testUeventNames = {
    "TestDevice", "headset", "h2w",
};

const vector<string> g_testUeventStates = {
    "added", "removed", "online",
};

const vector<string> g_testUeventDevTypes = {
    "headset", "usb", "hdmi",
};

const vector<string> g_testUeventSwitchNames = {
    "h2w", "hdmi_mipi_audio", "invalid",
};

const vector<string> g_testUeventSwitchStates = {
    "1", "0", "2",
};

const vector<string> g_testUeventHidNames = {
    "hid", "hidraw", "invalid",
};

const vector<string> g_testUeventDevNames = {
    "TestDevName", "usb_device", "invalid",
};

const vector<string> g_testUeventAnahsNames = {
    "anahs", "INSERT", "REMOVE", "",
};

void AudioSocketThreadAudioAnahsDetectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent validUeventInsert;
    validUeventInsert.subSystem = UEVENT_PLATFORM;
    std::string anahsNameStr = PickValue(fdp, g_testUeventAnahsNames);
    validUeventInsert.anahsName = anahsNameStr.c_str();

    bool isNull = fdp.ConsumeBool();
    if (isNull) {
        audioSocketThread.AudioAnahsDetectDevice(nullptr);
    } else {
        bool isSetAnahsName = fdp.ConsumeBool();
        if (isSetAnahsName) {
            audioSocketThread.audioSocketEvent_.anahsName = ConsumeString(fdp, UEVENT_INSERT);
        }
        audioSocketThread.AudioAnahsDetectDevice(&validUeventInsert);
    }
}

void AudioSocketThreadAudioAnalogHeadsetDetectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;

    std::string actionStr = PickValue(fdp, g_testUeventActions);
    std::string nameStr = PickValue(fdp, g_testUeventNames);
    std::string stateStr = PickValue(fdp, g_testUeventStates);
    std::string devTypeStr = PickValue(fdp, g_testUeventDevTypes);
    std::string subSystemStr = fdp.ConsumeBool() ? "switch" : ConsumeString(fdp, "switch");
    std::string switchNameStr = PickValue(fdp, g_testUeventSwitchNames);
    std::string switchStateStr = PickValue(fdp, g_testUeventSwitchStates);
    std::string hidNameStr = PickValue(fdp, g_testUeventHidNames);
    std::string devNameStr = PickValue(fdp, g_testUeventDevNames);
    std::string anahsNameStr = PickValue(fdp, g_testUeventAnahsNames);

    AudioPnpUevent audioPnpUevent = {
        .action = actionStr.c_str(),
        .name = nameStr.c_str(),
        .state = stateStr.c_str(),
        .devType = devTypeStr.c_str(),
        .subSystem = subSystemStr.c_str(),
        .switchName = switchNameStr.c_str(),
        .switchState = switchStateStr.c_str(),
        .hidName = hidNameStr.c_str(),
        .devName = devNameStr.c_str(),
        .anahsName = anahsNameStr.c_str()
    };
    bool isNull = fdp.ConsumeBool();
    if (isNull) {
        audioSocketThread.AudioAnalogHeadsetDetectDevice(nullptr);
    } else {
        audioSocketThread.AudioAnalogHeadsetDetectDevice(&audioPnpUevent);
    }
}

void AudioSocketThreadAudioHDMIDetectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    std::string subSystemStr = PickValue(fdp, g_testSubSystems);
    std::string switchNameStr = PickValue(fdp, g_testSwitchNames);
    std::string actionStr = PickValue(fdp, g_testActions);
    std::string switchStateStr = PickValue(fdp, g_testSwitchStates);

    AudioPnpUevent uevent = {
        .subSystem = subSystemStr.c_str(),
        .switchName = switchNameStr.c_str(),
        .action = actionStr.c_str(),
        .switchState = switchStateStr.c_str()
    };
    bool isNull = fdp.ConsumeBool();
    if (isNull) {
        audioSocketThread.AudioHDMIDetectDevice(nullptr);
    } else {
        audioSocketThread.AudioHDMIDetectDevice(&uevent);
    }
}

void AudioSocketThreadAudioPnpUeventParseFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    std::string msgStr = PickValue(fdp, g_testMsgs);
    ssize_t strLength;
    bool isInvalid = fdp.ConsumeBool();
    if (isInvalid) {
        strLength = UEVENT_MSG_LEN + UEVENT_MSG_PADDING;
    } else {
        strLength = strlen(msgStr.c_str());
    }
    audioSocketThread.AudioPnpUeventParse(msgStr.c_str(), strLength);
}

void AudioSocketThreadDetectAnalogHeadsetStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    std::ofstream ofs(SWITCH_STATE_PATH);
    if (!ofs.is_open()) {
        return;
    }
    ofs << (fdp.ConsumeBool() ? '1' : '0');
    ofs.close();
    AudioEvent audioEvent;
    audioEvent.eventType = fdp.ConsumeIntegral<uint32_t>();
    audioEvent.name = ConsumeString(fdp, "testName");
    audioSocketThread.DetectAnalogHeadsetState(&audioEvent);
}

void AudioSocketThreadDetectDPStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    AudioEvent audioEvent;
    audioEvent.eventType = fdp.ConsumeIntegral<uint32_t>();
    audioEvent.name = ConsumeString(fdp, "testName");

    audioSocketThread.DetectDPState(&audioEvent);
}

void AudioSocketThreadReadAndScanDpStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSocketThread audioSocketThread;
    std::string testPath = "/tmp/test_path";
    std::ofstream file(testPath);
    if (!file.is_open()) {
        return;
    }
    file << (fdp.ConsumeBool() ? '1' : '0');
    file.close();
    uint32_t eventType = fdp.ConsumeIntegral<uint32_t>();
    audioSocketThread.ReadAndScanDpState(testPath, eventType);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioSocketThreadAudioAnahsDetectDeviceFuzzTest,
    AudioSocketThreadAudioAnalogHeadsetDetectDeviceFuzzTest,
    AudioSocketThreadAudioHDMIDetectDeviceFuzzTest,
    AudioSocketThreadAudioPnpUeventParseFuzzTest,
    AudioSocketThreadDetectAnalogHeadsetStateFuzzTest,
    AudioSocketThreadDetectDPStateFuzzTest,
    AudioSocketThreadReadAndScanDpStateFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
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