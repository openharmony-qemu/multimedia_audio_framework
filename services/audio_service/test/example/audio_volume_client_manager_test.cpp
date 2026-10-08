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
#ifndef LOG_TAG
#define LOG_TAG "AudioVolumeClientManagerTest"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include "audio_common_log.h"
#include "audio_errors.h"
#include "audio_volume_client_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

static constexpr int32_t DELAY_TIME_ARG_INDEX = 1;
static constexpr int32_t UID_LIST_START_INDEX = 2;

static int32_t BlockGetInt32Input()
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
    CODE_SET_UNMUTE_WHITELIST = 0,
    CODE_CLEAR_UNMUTE_WHITELIST,
    CODE_EXIT,
};

static const std::map<int32_t, std::string> CMD_CODE = {
    {CODE_SET_UNMUTE_WHITELIST, "Test SetUnmuteWhitelist"},
    {CODE_CLEAR_UNMUTE_WHITELIST, "Test ClearUnmuteWhitelist"},
    {CODE_EXIT, "Exit test"},
};

static void PrintCmdCodeHelp()
{
    cout << endl << "============== VolumeClientCmdCode ===============" << endl;
    cout << "Use the number to run corresponding test:" << endl;
    for (const auto &pair : CMD_CODE) {
        cout << "Code " << pair.first << ": " << pair.second << endl;
    }
}

static void TestSetUnmuteWhitelist()
{
    cout << "Please input whitelist UIDs (separated by spaces):";
    std::string line;
    std::getline(cin >> ws, line);
    std::istringstream iss(line);
    std::vector<int32_t> uids;
    int32_t uid;
    while (iss >> uid) {
        uids.push_back(uid);
    }
    cout << "uids size: " << uids.size() << endl;
    for (size_t i = 0; i < uids.size(); i++) {
        cout << "  uid[" << i << "]: " << uids[i] << endl;
    }
    int32_t ret = AudioVolumeClientManager::GetInstance().SetUnmuteWhitelist(uids);
    cout << "SetUnmuteWhitelist " << (ret == SUCCESS ? "success" : "fail") << ", ret: " << ret << endl;
}

static void TestClearUnmuteWhitelist()
{
    int32_t ret = AudioVolumeClientManager::GetInstance().ClearUnmuteWhitelist();
    cout << "ClearUnmuteWhitelist " << (ret == SUCCESS ? "success" : "fail") << ", ret: " << ret << endl;
}

int main(int argc, char *argv[])
{
    if (argc > DELAY_TIME_ARG_INDEX) {
        int32_t delayTime = atoi(argv[DELAY_TIME_ARG_INDEX]);
        if (delayTime < 0) {
            delayTime = 0;
        }
        std::vector<int32_t> uids;
        for (int i = UID_LIST_START_INDEX; i < argc; i++) {
            int32_t uid = static_cast<int32_t>(atoi(argv[i]));
            uids.push_back(uid);
            cout << "  uid[" << (i - UID_LIST_START_INDEX) << "]: " << uid << endl;
        }
        cout << "uids size: " << uids.size() << endl;
        cout << "delayTime: " << delayTime << "s" << endl;
        int32_t ret = AudioVolumeClientManager::GetInstance().SetUnmuteWhitelist(uids);
        cout << "SetUnmuteWhitelist " << (ret == SUCCESS ? "success" : "fail") << ", ret: " << ret << endl;
        cout << "waiting " << delayTime << "s before clearing..." << endl;
        sleep(delayTime);
        int32_t clearRet = AudioVolumeClientManager::GetInstance().ClearUnmuteWhitelist();
        cout << "ClearUnmuteWhitelist " << (clearRet == SUCCESS ? "success" : "fail") << ", ret: " << clearRet << endl;
        return 0;
    }

    bool shouldExit = false;

    while (!shouldExit) {
        PrintCmdCodeHelp();
        int32_t code = BlockGetInt32Input();
        switch (code) {
            case CODE_SET_UNMUTE_WHITELIST:
                TestSetUnmuteWhitelist();
                break;
            case CODE_CLEAR_UNMUTE_WHITELIST:
                TestClearUnmuteWhitelist();
                break;
            case CODE_EXIT:
                shouldExit = true;
                break;
            default:
                cout << "invalid code: " << code << endl;
                break;
        }
    }

    return 0;
}
