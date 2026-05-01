/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <string>
#include <memory>

#include "audio_loopback.h"
#include "audio_info.h"
#include "audio_errors.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

class TestLoopbackCallback : public AudioLoopbackCallback {
public:
    void OnStatusChange(const AudioLoopbackStatus status, const StateChangeCmdType cmdType) override
    {
        cout << "[Callback] OnStatusChange: status=" << static_cast<int32_t>(status)
             << " (" << StatusToString(status) << ")"
             << ", cmdType=" << static_cast<int32_t>(cmdType) << endl;
    }

private:
    string StatusToString(AudioLoopbackStatus status)
    {
        switch (status) {
            case LOOPBACK_AVAILABLE_IDLE:
                return "IDLE";
            case LOOPBACK_AVAILABLE_RUNNING:
                return "RUNNING";
            case LOOPBACK_UNAVAILABLE_SCENE:
                return "UNAVAILABLE_SCENE";
            case LOOPBACK_UNAVAILABLE_DEVICE:
                return "UNAVAILABLE_DEVICE";
            default:
                return "UNKNOWN";
        }
    }
};

static void PrintUsage()
{
    cout << "Audio Loopback Global Control Test" << endl;
    cout << "Usage:" << endl;
    cout << "  enable   - Enable loopback" << endl;
    cout << "  disable  - Disable loopback" << endl;
    cout << "  status   - Get loopback status" << endl;
    cout << "  volume   - Get loopback volume" << endl;
    cout << "  help     - Show this help" << endl;
    cout << "  quit     - Exit test" << endl;
}

static string ErrorToString(int32_t error)
{
    if (error == SUCCESS) {
        return "SUCCESS";
    } else if (error == ERROR_LOOPBACK_HANDLER_NOT_EXIST) {
        return "ERROR_LOOPBACK_HANDLER_NOT_EXIST";
    } else if (error == ERROR_LOOPBACK_HANDLER_ALREADY_EXIST) {
        return "ERROR_LOOPBACK_HANDLER_ALREADY_EXIST";
    } else if (error == ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED) {
        return "ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED";
    } else if (error == ERROR_LOOPBACK_PERMISSION_DENIED) {
        return "ERROR_LOOPBACK_PERMISSION_DENIED";
    } else if (error == ERR_NULL_POINTER) {
        return "ERR_NULL_POINTER";
    } else if (error == ERR_NOT_SUPPORTED) {
        return "ERR_NOT_SUPPORTED";
    } else {
        return "UNKNOWN_ERROR(" + to_string(error) + ")";
    }
}

int main()
{
    cout << "=== Audio Loopback Global Control Test ===" << endl;
    cout << "Creating GLOBAL_CONTROL type loopback..." << endl;

    AppInfo appInfo;
    int32_t result = SUCCESS;
    auto loopback = AudioLoopback::CreateAudioLoopback(
        LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, result, appInfo);
    if (result != SUCCESS || loopback == nullptr) {
        cout << "[Error] CreateAudioLoopback failed: " << ErrorToString(result) << endl;
        cout << "Note: GLOBAL_CONTROL requires GLOBAL_HANDLER process to be running first!" << endl;
        return result;
    }

    cout << "[Success] GLOBAL_CONTROL loopback created" << endl;

    auto callback = make_shared<TestLoopbackCallback>();
    int32_t callbackResult = loopback->SetAudioLoopbackCallback(callback);
    if (callbackResult != SUCCESS) {
        cout << "[Warning] SetAudioLoopbackCallback failed: " << ErrorToString(callbackResult) << endl;
    } else {
        cout << "[Success] Callback registered" << endl;
    }

    PrintUsage();
    cout << endl;

    string cmd;
    while (true) {
        cout << "> ";
        cin >> cmd;

        if (cmd == "quit" || cmd == "exit") {
            cout << "Exiting..." << endl;
            break;
        } else if (cmd == "help") {
            PrintUsage();
        } else if (cmd == "enable") {
            cout << "Calling Enable(true)..." << endl;
            bool ret = loopback->Enable(true);
            cout << "[Result] Enable(true): " << (ret ? "true" : "false") << endl;
        } else if (cmd == "disable") {
            cout << "Calling Enable(false)..." << endl;
            bool ret = loopback->Enable(false);
            cout << "[Result] Enable(false): " << (ret ? "true" : "false") << endl;
        } else if (cmd == "status") {
            cout << "Calling GetStatus()..." << endl;
            AudioLoopbackStatus status = loopback->GetStatus();
            cout << "[Result] GetStatus: " << static_cast<int32_t>(status)
                 << " (" << (status == LOOPBACK_AVAILABLE_IDLE ? "IDLE" :
                           status == LOOPBACK_AVAILABLE_RUNNING ? "RUNNING" : "UNKNOWN") << ")" << endl;
        } else if (cmd == "volume") {
            cout << "Calling GetVolume()..." << endl;
            float volume = loopback->GetVolume();
            cout << "[Result] GetVolume: " << volume << endl;
        } else if (cmd.empty()) {
            continue;
        } else {
            cout << "Unknown command: " << cmd << endl;
            PrintUsage();
        }
    }

    loopback->RemoveAudioLoopbackCallback();
    cout << "Test completed." << endl;
    return 0;
}