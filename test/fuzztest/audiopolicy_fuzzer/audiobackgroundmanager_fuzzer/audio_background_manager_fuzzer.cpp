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
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "iremote_object.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

typedef void (*TestFuncs)();

class RemoteObjectFuzzTestStub : public IRemoteObject {
public:
    RemoteObjectFuzzTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectFuzzTestStub");
};
void AudioBackgroundManagerNotifySessionStateChangeFuzzTest(FuzzedDataProvider& fdp)
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

void AudioBackgroundManagerNotifyBackgroundTaskStateChangeFuzzTest(FuzzedDataProvider& fdp)
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

void AudioBackgroundManagerNotifyFreezeStateChangeFuzzTest(FuzzedDataProvider& fdp)
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

void AudioBackgroundManagerHandleSessionStateChangeFuzzTest(FuzzedDataProvider& fdp)
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
    bool silentControl = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.HandleSessionStateChange(uid, pid, silentControl);
}

void AudioBackgroundManagerHandleFreezeStateChangeFuzzTest(FuzzedDataProvider& fdp)
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

void AudioBackgroundManagerResetAllProxyFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AppState appState;
    appState.isFreeze = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appState});
    audioBackgroundManagerTest.ResetAllProxy();
}

void AppStateListenerOnAppStateChangedFuzzTest(FuzzedDataProvider& fdp)
{
    AppStateListener appStateListener;
    AppExecFwk::AppProcessData appProcessData;
    appStateListener.OnAppStateChanged(appProcessData);
}

void AudioBackgroundManagerDeleteFromMapFuzzTest(FuzzedDataProvider& fdp)
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

void AppStateListenerHandleAppStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    AppStateListener appStateListener;
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    appStateListener.HandleAppStateChange(pid, uid, state);
}

void AppStateListenerHandleBackgroundAppStateChangeFuzzTest(FuzzedDataProvider& fdp)
{
    AppStateListener appStateListener;
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    appStateListener.HandleBackgroundAppStateChange(pid, uid, state);
}

void AudioBackgroundManagerSetQueryAllowedPlaybackCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    sptr<IRemoteObject> object = nullptr;
    bool useNullObject = g_fuzzUtils.GetData<bool>();
    if (!useNullObject) {
        object = new (std::nothrow) RemoteObjectFuzzTestStub();
    }
    audioBackgroundManagerTest.SetQueryAllowedPlaybackCallback(object);
}

void AudioBackgroundManagerSetBackgroundMuteCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    sptr<IRemoteObject> object = nullptr;
    bool useNullObject = g_fuzzUtils.GetData<bool>();
    if (!useNullObject) {
        object = new (std::nothrow) RemoteObjectFuzzTestStub();
    }
    audioBackgroundManagerTest.SetBackgroundMuteCallback(object);
}

void AudioBackgroundManagerIsAppInBackStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &audioBackgroundManagerTest = AudioBackgroundManager::GetInstance();
    int32_t pid = g_fuzzUtils.GetData<int32_t>();

    audioBackgroundManagerTest.appStatesMap_.clear();
    bool resultNotFound = audioBackgroundManagerTest.IsAppInBackState(pid);

    AppState appStateBackTrue;
    appStateBackTrue.isBack = true;
    appStateBackTrue.isSystem = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appStateBackTrue});
    bool resultBackTrue = audioBackgroundManagerTest.IsAppInBackState(pid);

    AppState appStateBackFalse;
    appStateBackFalse.isBack = false;
    appStateBackFalse.isSystem = g_fuzzUtils.GetData<bool>();
    audioBackgroundManagerTest.appStatesMap_.clear();
    audioBackgroundManagerTest.appStatesMap_.insert({pid, appStateBackFalse});
    bool resultBackFalse = audioBackgroundManagerTest.IsAppInBackState(pid);

    int32_t pid2 = g_fuzzUtils.GetData<int32_t>();
    if (pid2 != pid) {
        AppState appState2;
        appState2.isBack = g_fuzzUtils.GetData<bool>();
        audioBackgroundManagerTest.appStatesMap_.insert({pid2, appState2});
        audioBackgroundManagerTest.IsAppInBackState(pid);
        audioBackgroundManagerTest.IsAppInBackState(pid2);
    }

    int32_t edgePid = fdp.PickValueInArray<int32_t>({0, -1, INT32_MIN, INT32_MAX});
    audioBackgroundManagerTest.IsAppInBackState(edgePid);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
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
    AudioBackgroundManagerSetQueryAllowedPlaybackCallbackFuzzTest,
    AudioBackgroundManagerSetBackgroundMuteCallbackFuzzTest,
    AudioBackgroundManagerIsAppInBackStateFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}