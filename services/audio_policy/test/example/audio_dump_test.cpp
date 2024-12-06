/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDumpTest"
#endif

#include <iostream>
#include <set>
#include <string>
#include <unistd.h>

#include "audio_system_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace AudioDumpTest {
    const int FIRST_ARG = 1;
    const int SECOND_ARG = 2;
}

namespace {
    const string AudioDumpKey = "PCM_DUMP";
    const string AudioDumpType = "R_AND_D";
    const set<string> SupportInput = {"0", "1"};
}

static void PrintUsage(void)
{
    cout << "NAME" << endl << endl;
    cout << "\taudio_dump_test - Audio Dump Test " << endl << endl;
    cout << "\t./audio_dump_test [OPTIONS]..." << endl << endl;
    cout << "\t OPTIONS: 0(close) or 1(open) " << endl << endl;
}

static bool IsSupportInputArgs(string& input)
{
    if (SupportInput.find(input) != SupportInput.end()) {
        return true;
    }

    return false;
}

static void AudioDumpCmd(int opt)
{
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    vector<pair<string, string>> kvpairs;
    if (opt) {
        kvpairs.push_back({AudioDumpType, "true"});
    } else {
        kvpairs.push_back({AudioDumpType, "false"});
    }
    audioSystemMgr->SetExtraParameters(AudioDumpKey, kvpairs);
}

int main(int argc, char *argv[])
{
    int opt = 0;
    if (argc < AudioDumpTest::SECOND_ARG) {
        PrintUsage();
        return 0;
    }

    if (geteuid() != 0) {
        cout << "need root !" << endl << endl;
        return 0;
    }

    string inArgv = argv[AudioDumpTest::FIRST_ARG];
    if (!IsSupportInputArgs(inArgv)) {
        PrintUsage();
        return 0;
    }

    opt = stoi(inArgv);
    AudioDumpCmd(opt);
    return 0;
}
