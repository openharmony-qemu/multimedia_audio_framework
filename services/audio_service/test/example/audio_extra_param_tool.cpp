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
#define LOG_TAG "AudioExtraParamTool"
#endif

#include <iostream>
#include <string>
#include <unistd.h>

#include "nativetoken_kit.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "audio_system_client_engine_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;
using namespace OHOS::Security::AccessToken;

namespace {
    constexpr int MIN_ARGS = 3;
    constexpr int SET_ARGS = 4;
    constexpr int GET_ARGS = 3;
    constexpr int UID_OPTION_ARG_INDEX = 1;
    constexpr int UID_VALUE_ARG_INDEX = 2;
    constexpr int UID_ARG_COUNT = 2;
    constexpr int OPERATION_ARG_INDEX = 1;
    constexpr int MAIN_KEY_ARG_INDEX = 2;
    constexpr int SUB_KEY_ARG_INDEX = 3;
    constexpr int VALUE_ARG_INDEX = 4;
    constexpr int INVALID_UID = -1;
}

static void PrintUsage(void)
{
    cout << "NAME" << endl << endl;
    cout << "\taudio_extra_param_tool - Get/Set audio extra parameters" << endl << endl;
    cout << "USAGE" << endl << endl;
    cout << "\tSet: ./audio_extra_param_tool [--uid <uid>] set <main_key> <sub_key> <value>" << endl;
    cout << "\tGet: ./audio_extra_param_tool [--uid <uid>] get <main_key> <sub_key>" << endl << endl;
    cout << "OPTIONS" << endl << endl;
    cout << "\t--uid <uid>  Switch to specified UID before execution" << endl << endl;
    cout << "EXAMPLES" << endl << endl;
    cout << "\t./audio_extra_param_tool set PCM_DUMP R_AND_D true" << endl;
    cout << "\t./audio_extra_param_tool --uid 1041 get PCM_DUMP R_AND_D" << endl;
    cout << "\t./audio_extra_param_tool --uid 1041 set PCM_DUMP R_AND_D true" << endl << endl;
}

static bool SetupPermission(void)
{
    cout << "SetupPermission start - UID: " << getuid() << endl;

    cout << "Creating native token..." << endl;

    const char *perms[] = {
        "ohos.permission.MODIFY_AUDIO_SETTINGS"
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "audio_server",
        .aplStr = "system_core",
    };

    uint64_t fullTokenId = GetAccessTokenId(&infoInstance);
    if (fullTokenId == 0) {
        cerr << "Failed to create native token ID" << endl;
        return false;
    }
    cout << "Native token created: " << fullTokenId << ", UID: " << getuid() << endl;

    int ret = SetSelfTokenID(fullTokenId);
    if (ret != 0) {
        cerr << "Failed to set self token ID: " << ret << endl;
        return false;
    }
    cout << "Native token set successfully, UID: " << getuid() << endl;

    AccessTokenKit::ReloadNativeTokenInfo();
    cout << "Native token info reloaded, UID: " << getuid() << endl;

    return true;
}

static int32_t GetAudioExtraParam(const string &mainKey, const string &subKey)
{
    vector<string> subKeys;
    subKeys.push_back(subKey);

    vector<pair<string, string>> result;
    int32_t ret = AudioSystemClientEngineManager::GetInstance().GetExtraParameters(mainKey, subKeys, result);
    if (ret != 0) {
        cerr << "GetExtraParameters failed with error: " << ret << endl;
        return ret;
    }

    for (const auto &pair : result) {
        cout << pair.first << " = " << pair.second << endl;
    }

    return 0;
}

static int32_t SetAudioExtraParam(const string &mainKey, const string &subKey, const string &value)
{
    vector<pair<string, string>> kvpairs;
    kvpairs.push_back({subKey, value});

    return AudioSystemClientEngineManager::GetInstance().SetExtraParameters(mainKey, kvpairs);
}

static bool ParseUidOption(int argc, char *argv[], int &targetUid, int &argOffset)
{
    targetUid = INVALID_UID;
    argOffset = 0;

    if (argc > UID_OPTION_ARG_INDEX && string(argv[UID_OPTION_ARG_INDEX]) == "--uid") {
        if (argc <= UID_VALUE_ARG_INDEX) {
            cerr << "--uid requires an argument" << endl;
            PrintUsage();
            return false;
        }
        targetUid = stoi(argv[UID_VALUE_ARG_INDEX]);
        argOffset = UID_ARG_COUNT;
    }
    return true;
}

static bool SwitchUid(int targetUid)
{
    if (targetUid < 0) {
        return true;
    }

    cout << "Switching UID from " << getuid() << " to " << targetUid << endl;
    if (setuid(targetUid) != 0) {
        cerr << "Failed to setuid to " << targetUid << endl;
        return false;
    }
    cout << "UID switched to " << getuid() << endl;
    return true;
}

static int32_t ExecuteOperation(int argc, char *argv[], int argOffset, int adjustedArgc)
{
    string operation = argv[OPERATION_ARG_INDEX + argOffset];
    string mainKey = argv[MAIN_KEY_ARG_INDEX + argOffset];

    if (operation == "get") {
        if (adjustedArgc < GET_ARGS + 1) {
            PrintUsage();
            return -1;
        }
        string subKey = argv[SUB_KEY_ARG_INDEX + argOffset];
        cout << "Getting extra parameter: mainKey=" << mainKey << ", subKey=" << subKey << endl;

        int32_t ret = GetAudioExtraParam(mainKey, subKey);
        if (ret != 0) {
            return -1;
        }
        return 0;
    }

    if (operation == "set") {
        if (adjustedArgc < SET_ARGS + 1) {
            PrintUsage();
            return -1;
        }
        string subKey = argv[SUB_KEY_ARG_INDEX + argOffset];
        string value = argv[VALUE_ARG_INDEX + argOffset];
        cout << "Setting extra parameter: mainKey=" << mainKey
             << ", subKey=" << subKey << ", value=" << value << endl;

        int32_t ret = SetAudioExtraParam(mainKey, subKey, value);
        if (ret != 0) {
            cerr << "SetExtraParameters failed with error: " << ret << endl;
            return -1;
        }
        cout << "SetExtraParameters success" << endl;
        return 0;
    }

    cerr << "Unknown operation: " << operation << endl;
    PrintUsage();
    return -1;
}

int main(int argc, char *argv[])
{
    cout << "Initial UID: " << getuid() << endl;

    int targetUid = INVALID_UID;
    int argOffset = 0;

    if (!ParseUidOption(argc, argv, targetUid, argOffset)) {
        return -1;
    }

    int adjustedArgc = argc - argOffset;
    if (adjustedArgc < MIN_ARGS + 1) {
        PrintUsage();
        return 0;
    }

    if (!SetupPermission()) {
        cerr << "Failed to setup permission" << endl;
        return -1;
    }

    if (!SwitchUid(targetUid)) {
        return -1;
    }

    return ExecuteOperation(argc, argv, argOffset, adjustedArgc);
}