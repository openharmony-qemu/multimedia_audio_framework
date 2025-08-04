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
#include "../fuzz_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const uint32_t UEVENT_MSG_PADDING = 2;
typedef void (*TestPtr)();

void AudioSocketThreadAudioAnahsDetectDeviceFuzzTest()
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent validUeventInsert = {
        .subSystem = UEVENT_PLATFORM,
        .anahsName = UEVENT_INSERT
    };

    bool isNull = g_fuzzUtils.GetData<bool>();
    if (isNull) {
        audioSocketThread.AudioAnahsDetectDevice(nullptr);
    } else {
        bool isSetAnahsName = g_fuzzUtils.GetData<bool>();
        if (isSetAnahsName) {
            audioSocketThread.audioSocketEvent_.anahsName = UEVENT_INSERT;
        }
        audioSocketThread.AudioAnahsDetectDevice(&validUeventInsert);
    }
}

void AudioSocketThreadAudioAnalogHeadsetDetectDeviceFuzzTest()
{
    AudioSocketThread audioSocketThread;

    AudioPnpUevent audioPnpUevent = {
        .action = "add",
        .name = "TestDevice",
        .state = "added",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };
    bool isNull = g_fuzzUtils.GetData<bool>();
    if (isNull) {
        audioSocketThread.AudioAnalogHeadsetDetectDevice(nullptr);
    } else {
        audioSocketThread.AudioAnalogHeadsetDetectDevice(&audioPnpUevent);
    }
}

void AudioSocketThreadAudioHDMIDetectDeviceFuzzTest()
{
    AudioSocketThread audioSocketThread;
    static const vector<string> testSubSystems = {
        "switch", "invalid",
    };
    static const vector<string> testSwitchNames = {
        "hdmi_mipi_audio", "invalid", "hdmi_mipi_audio,device_port=HDMI-0",
    };
    static const vector<string> testActions = {
        "change", "invalid",
    };
    static const vector<string> testSwitchStates = {
        "1", "0", "invalid",
    };
    if (testSubSystems.empty() || testSwitchNames.empty() || testActions.empty() || testSwitchStates.empty()) {
        return;
    }

    AudioPnpUevent uevent = {
        .subSystem = testSubSystems[g_fuzzUtils.GetData<uint32_t>() % testSubSystems.size()].c_str(),
        .switchName = testSwitchNames[g_fuzzUtils.GetData<uint32_t>() % testSwitchNames.size()].c_str(),
        .action = testActions[g_fuzzUtils.GetData<uint32_t>() % testActions.size()].c_str(),
        .switchState =
            testSwitchStates[g_fuzzUtils.GetData<uint32_t>() % testSwitchStates.size()].c_str()
    };
    bool isNull = g_fuzzUtils.GetData<bool>();
    if (isNull) {
        audioSocketThread.AudioHDMIDetectDevice(nullptr);
    } else {
        audioSocketThread.AudioHDMIDetectDevice(&uevent);
    }
}

void AudioSocketThreadAudioPnpUeventParseFuzzTest()
{
    AudioSocketThread audioSocketThread;
    static const vector<string> testMsgs = {
        "libudev",
        "test message",
        "unmatched event",
        "matched event",
    };
    if (testMsgs.empty()) {
        return;
    }
    const char *msg = testMsgs[g_fuzzUtils.GetData<uint32_t>() % testMsgs.size()].c_str();
    ssize_t strLength;
    bool isInvalid = g_fuzzUtils.GetData<bool>();
    if (isInvalid) {
        strLength = UEVENT_MSG_LEN + UEVENT_MSG_PADDING;
    } else {
        strLength = strlen(msg);
    }
    audioSocketThread.AudioPnpUeventParse(msg, strLength);
}

void AudioSocketThreadDetectAnalogHeadsetStateFuzzTest()
{
    AudioSocketThread audioSocketThread;
    std::ofstream ofs(SWITCH_STATE_PATH);
    ofs << '1';
    ofs.close();
    AudioEvent audioEvent;
    audioEvent.eventType = g_fuzzUtils.GetData<uint32_t>();
    audioSocketThread.DetectAnalogHeadsetState(&audioEvent);
}

void AudioSocketThreadDetectDPStateFuzzTest()
{
    AudioSocketThread audioSocketThread;
    AudioEvent audioEvent;
    audioEvent.eventType = g_fuzzUtils.GetData<uint32_t>();
    audioEvent.name = "testName";

    audioSocketThread.DetectDPState(&audioEvent);
}

void AudioSocketThreadReadAndScanDpStateFuzzTest()
{
    AudioSocketThread audioSocketThread;
    std::string testPath = "/tmp/test_path";
    std::ofstream file(testPath);
    file << '1';
    file.close();
    uint32_t eventType = g_fuzzUtils.GetData<uint32_t>();
    audioSocketThread.ReadAndScanDpState(testPath, eventType);
}

vector<TestPtr> g_testPtrs = {
    AudioSocketThreadAudioAnahsDetectDeviceFuzzTest,
    AudioSocketThreadAudioAnalogHeadsetDetectDeviceFuzzTest,
    AudioSocketThreadAudioHDMIDetectDeviceFuzzTest,
    AudioSocketThreadAudioPnpUeventParseFuzzTest,
    AudioSocketThreadDetectAnalogHeadsetStateFuzzTest,
    AudioSocketThreadDetectDPStateFuzzTest,
    AudioSocketThreadReadAndScanDpStateFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testPtrs);
    return 0;
}