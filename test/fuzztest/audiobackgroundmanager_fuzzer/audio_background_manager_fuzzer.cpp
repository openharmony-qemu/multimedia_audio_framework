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

#include <securec.h>

#include "audio_log.h"
#include "audio_background_manager.h"
#include "app_state_listener.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

typedef void (*TestFuncs)();

void AudioBackgroundManagerNotifyBackgroundTaskStateChangeFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool hasBackgroundTask = g_fuzzUtils.GetData<bool>();
    AppState appState;
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        audioBackgroundManagerTest.appStatesMap_.clear();
    }
    audioBackgroundManagerTest.NotifyBackgroundTaskStateChange(uid, pid, hasBackgroundTask);
}

void AudioBackgroundManagerNotifySessionStateChangeFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool hasSession = g_fuzzUtils.GetData<bool>();
    AppState appState;
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        audioBackgroundManagerTest.appStatesMap_.clear();
    }
    audioBackgroundManagerTest.NotifySessionStateChange(uid, pid, hasSession);
}

void AudioBackgroundManagerHandleSessionStateChangeFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AppState appState;
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        audioBackgroundManagerTest.appStatesMap_.clear();
    }
    audioBackgroundManagerTest.HandleSessionStateChange(uid, pid);
}

void AudioBackgroundManagerNotifyFreezeStateChangeFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool isFreeze = g_fuzzUtils.GetData<bool>();
    std::set<int32_t> pidList;
    pidList.insert(pid);
    pidList.insert(g_fuzzUtils.GetData<int32_t>());
    AppState appState;
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    audioBackgroundManagerTest.NotifyFreezeStateChange(pidList, isFreeze);
}

void AudioBackgroundManagerResetAllProxyFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AppState appState;
    appState.isFreeze = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    audioBackgroundManagerTest.ResetAllProxy();
}

void AudioBackgroundManagerHandleFreezeStateChangeFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    bool isFreeze = g_fuzzUtils.GetData<bool>();
    AppState appState;
    appState.hasBackTask = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    audioBackgroundManagerTest.HandleFreezeStateChange(pid, isFreeze);
}

void AudioBackgroundManagerDeleteFromMapFuzzTest()
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AppState appState;
    appState.hasBackTask = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        audioBackgroundManagerTest.appStatesMap_.clear();
    }
    audioBackgroundManagerTest.DeleteFromMap(pid);
}

void AppStateListenerOnAppStateChangedFuzzTest()
{
    AppStateListener appStateListener;
    AppExecFwk::AppProcessData appProcessData;
    appStateListener.OnAppStateChanged(appProcessData);
}

void AppStateListenerHandleAppStateChangeFuzzTest()
{
    AppStateListener appStateListener;
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    appStateListener.HandleAppStateChange(pid, uid, state);
}

void AppStateListenerHandleBackgroundAppStateChangeFuzzTest()
{
    AppStateListener appStateListener;
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    appStateListener.HandleBackgroundAppStateChange(pid, uid, state);
}

vector<TestFuncs> g_testFuncs = {
    AudioBackgroundManagerNotifyBackgroundTaskStateChangeFuzzTest,
    AudioBackgroundManagerNotifySessionStateChangeFuzzTest,
    AudioBackgroundManagerHandleSessionStateChangeFuzzTest,
    AudioBackgroundManagerNotifyFreezeStateChangeFuzzTest,
    AudioBackgroundManagerResetAllProxyFuzzTest,
    AudioBackgroundManagerHandleFreezeStateChangeFuzzTest,
    AudioBackgroundManagerDeleteFromMapFuzzTest,
    AppStateListenerOnAppStateChangedFuzzTest,
    AppStateListenerHandleAppStateChangeFuzzTest,
    AppStateListenerHandleBackgroundAppStateChangeFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
