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
#ifndef LOG_TAG
#define LOG_TAG "AudioEngineClientManagerTest"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sstream>
#include <iostream>
#include <securec.h>
#include <unistd.h>

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_engine_client_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

static std::shared_ptr<AudioOutputPipeCallback> gTestOutCallback = nullptr;
static std::shared_ptr<AudioInputPipeCallback> gTestInCallback = nullptr;

static void PrintOutputPipeInfo(const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
{
    cout << "Output Pipe id: " << changedPipeInfo->GetId() << endl;
    cout << "  - adapter: " << changedPipeInfo->GetAdapter() << endl;
    cout << "  - route: " << changedPipeInfo->GetRouteFlag() << endl;
    cout << "  - status: " << changedPipeInfo->GetStatus() << endl;
    cout << "  - devices: " << endl;
    auto devices = changedPipeInfo->GetDevices();
    for (auto &device : devices) {
        cout << "    - type: " << device << endl;
    }
    cout << "  - streams: " << endl;
    auto streams = changedPipeInfo->GetStreams();
    for (auto &streamIter : streams) {
        cout << "    - id: " << streamIter.second.streamId_ <<
            " usage: " << streamIter.second.usage_ <<
            " state: " << streamIter.second.state_ <<
            " uid: " << streamIter.second.appUid_ << endl;
    }
}

static void PrintInputPipeInfo(const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
{
    cout << "Input Pipe id: " << changedPipeInfo->GetId() << endl;
    cout << "  - adapter: " << changedPipeInfo->GetAdapter() << endl;
    cout << "  - route: " << changedPipeInfo->GetRouteFlag() << endl;
    cout << "  - status: " << changedPipeInfo->GetStatus() << endl;
    cout << "  - devices: " << endl;
    auto devices = changedPipeInfo->GetDevices();
    for (auto &device : devices) {
        cout << "    - type: " << device << endl;
    }
    cout << "  - streams: " << endl;
    auto streams = changedPipeInfo->GetStreams();
    for (auto &streamIter : streams) {
        cout << "    - id: " << streamIter.second.streamId_ <<
            " source: " << streamIter.second.source_ <<
            " state: " << streamIter.second.state_ <<
            " uid: " << streamIter.second.appUid_ <<endl;
    }
}

class TestAudioOutputPipeCallback : public AudioOutputPipeCallback {
public:
    void OnOutputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
    {
        cout << "Receive pipe change type: "<< changeType << endl;
        PrintOutputPipeInfo(changedPipeInfo);
    }
};

class TestAudioInputPipeCallback : public AudioInputPipeCallback {
public:
    void OnInputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
    {
        cout << "Receive pipe change type: "<< changeType << endl;
        PrintInputPipeInfo(changedPipeInfo);
    }
};

static void TestGetPipeChangeInfos(AudioMode mode)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        std::vector<std::shared_ptr<AudioOutputPipeInfo>> outputPipeInfos;
        int32_t ret = DelayedSingleton<AudioEngineClientManager>::GetInstance()->GetCurrentOutputPipeChangeInfos(
            outputPipeInfos);
        cout << "Get pipe infos " << (ret == SUCCESS ? "success" : "fail") << endl;
        cout << "pipe size:" << outputPipeInfos.size() << endl;
        for (auto &pipeInfo : outputPipeInfos) {
            PrintOutputPipeInfo(pipeInfo);
        }
    } else {
        std::vector<std::shared_ptr<AudioInputPipeInfo>> inputPipeInfos;
        int32_t ret = DelayedSingleton<AudioEngineClientManager>::GetInstance()->GetCurrentInputPipeChangeInfos(
            inputPipeInfos);
        cout << "Get pipe infos " << (ret == SUCCESS ? "success" : "fail") << endl;
        cout << "pipe size:" << inputPipeInfos.size() << endl;
        for (auto &pipeInfo : inputPipeInfos) {
            PrintInputPipeInfo(pipeInfo);
        }
    }
}


static void TestRegisterPipeChangeCallback(AudioMode mode)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        gTestOutCallback = std::make_shared<TestAudioOutputPipeCallback>();
        int32_t ret =
            DelayedSingleton<AudioEngineClientManager>::GetInstance()->
            RegisterOutputPipeChangeCallback(gTestOutCallback);
        cout << "Register callback " << (ret == SUCCESS ? "success" : "fail") << endl;
    } else {
        gTestInCallback = std::make_shared<TestAudioInputPipeCallback>();
        int32_t ret =
            DelayedSingleton<AudioEngineClientManager>::GetInstance()->
            RegisterInputPipeChangeCallback(gTestInCallback);
        cout << "Register callback " << (ret == SUCCESS ? "success" : "fail") << endl;
    }
}

static void TestUnregisterPipeChangeCallback(AudioMode mode)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        int32_t ret =
            DelayedSingleton<AudioEngineClientManager>::GetInstance()->
            UnregisterOutputPipeChangeCallback(gTestOutCallback);
        cout << "Unregister callback " << (ret == SUCCESS ? "success" : "fail") << endl;
    } else {
        int32_t ret =
            DelayedSingleton<AudioEngineClientManager>::GetInstance()->
            UnregisterInputPipeChangeCallback(gTestInCallback);
        cout << "Unregister callback " << (ret == SUCCESS ? "success" : "fail") << endl;
    }
}

int32_t BlockGetInt32Input()
{
    int32_t input = -1;
    int32_t tryCount = 3;
    cout << "Please input code:";
    cin >> input;
    while (cin.fail() && tryCount-- > 0) {
        cin.clear();
        cin.ignore();
        cout << "invalid input, not a number! Please retry with a number." << endl;
        cout << "Please input code:";
        cin >> input;
    }
    return input;
}

enum CmdCode : int32_t {
    CODE_GET_OUTPUT_PIPE = 0,
    CODE_GET_OUTPUT_REGISTER,
    CODE_GET_OUTPUT_UNREGISTER,
    CODE_GET_INPUT_PIPE,
    CODE_GET_INPUT_REGISTER,
    CODE_GET_INPUT_UNREGISTER,
    CODE_EXIT,
};

static const std::map<int32_t, std::string> CMD_CODE = {
    {CODE_GET_OUTPUT_PIPE, "Test get current output pipes"},
    {CODE_GET_OUTPUT_REGISTER, "Test register output pipe change callback"},
    {CODE_GET_OUTPUT_UNREGISTER, "Test unregister output pipe change callback"},
    {CODE_GET_INPUT_PIPE, "Test get current intput pipes"},
    {CODE_GET_INPUT_REGISTER, "Test register intput pipe change callback"},
    {CODE_GET_INPUT_UNREGISTER, "Test unregister input pipe change callback"},
    {CODE_EXIT, "Exit test"},
};

void PrintCmdCodeHelp()
{
    cout << endl << "============== EngineClientCmdCode ===============" << endl;
    cout << "Use the number to run corresponding test:" << endl;
    for (auto it = CMD_CODE.begin(); it != CMD_CODE.end(); it ++) {
        cout << "Code " << it->first << ": " << it->second << endl;
    }
}

int main(int argc, char *argv[])
{
    bool exit = false;
    while (!exit) {
        PrintCmdCodeHelp();
        int32_t code = BlockGetInt32Input();
        switch (code) {
            case CODE_GET_OUTPUT_PIPE:
                TestGetPipeChangeInfos(AUDIO_MODE_PLAYBACK);
                break;
            case CODE_GET_OUTPUT_REGISTER:
                TestRegisterPipeChangeCallback(AUDIO_MODE_PLAYBACK);
                break;
            case CODE_GET_OUTPUT_UNREGISTER:
                TestUnregisterPipeChangeCallback(AUDIO_MODE_PLAYBACK);
                break;
            case CODE_GET_INPUT_PIPE:
                TestGetPipeChangeInfos(AUDIO_MODE_RECORD);
                break;
            case CODE_GET_INPUT_REGISTER:
                TestRegisterPipeChangeCallback(AUDIO_MODE_RECORD);
                break;
            case CODE_GET_INPUT_UNREGISTER:
                TestUnregisterPipeChangeCallback(AUDIO_MODE_RECORD);
                break;
            case CODE_EXIT:
                exit = true;
                break;
            default:
                cout << "invalid code: " << code << endl;
                break;
        }
    }

    return 0;
}
