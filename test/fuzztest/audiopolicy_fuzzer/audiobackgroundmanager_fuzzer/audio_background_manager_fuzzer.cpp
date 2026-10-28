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
#include "background_task_listener.h"
#include "client_type_manager.h"
#include "client_type_manager_handler.h"
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "iremote_object.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

const size_t THRESHOLD = 10;
const size_t MAX_STRING_SIZE = 64;

typedef void (*TestFuncs)();

class ClientTypeListenerFuzzTest : public ClientTypeListener {
public:
    void OnClientTypeQueryCompleted(uint32_t uid, ClientType clientType, const std::string &bundleName) override
    {
        uid_ = uid;
        clientType_ = clientType;
        bundleName_ = bundleName;
    }

    uint32_t uid_ = 0;
    ClientType clientType_ = CLIENT_TYPE_OTHERS;
    std::string bundleName_;
};

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

std::string ConsumeString(FuzzedDataProvider& fdp)
{
    return fdp.ConsumeRandomLengthString(MAX_STRING_SIZE);
}

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

void AudioBackgroundManagerPlaybackAndMapFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &manager = AudioBackgroundManager::GetInstance();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    StreamUsage streamUsage = fdp.PickValueInArray<StreamUsage>({
        STREAM_USAGE_UNKNOWN,
        STREAM_USAGE_MEDIA,
        STREAM_USAGE_MUSIC,
        STREAM_USAGE_ALARM,
        STREAM_USAGE_RINGTONE,
        STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_VIDEO_COMMUNICATION,
    });
    bool silentControl = fdp.ConsumeBool();
    AppState appState;
    appState.isFreeze = fdp.ConsumeBool();
    appState.isBack = fdp.ConsumeBool();
    appState.hasSession = fdp.ConsumeBool();
    appState.hasBackTask = fdp.ConsumeBool();
    appState.isBinder = fdp.ConsumeBool();
    appState.isSystem = fdp.ConsumeBool();

    manager.appStatesMap_.clear();
    manager.InsertIntoAppStatesMap(pid, uid, appState);
    manager.FindKeyInMap(pid);
    manager.IsAllowedPlayback(uid, pid, sessionId, streamUsage, silentControl);
    manager.InsertIntoAppStatesMapWithoutUid(fdp.ConsumeIntegral<int32_t>(), appState);
    manager.WriteAvSessionChangeSysEvent(pid, appState.hasSession, fdp.ConsumeBool());
    manager.WriteBackTaskChangeSysEvent(pid, appState.hasBackTask, fdp.ConsumeBool());
    manager.DeleteFromMap(pid);
}

void AudioBackgroundManagerRecoveryFuzzTest(FuzzedDataProvider& fdp)
{
    AudioBackgroundManager &manager = AudioBackgroundManager::GetInstance();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    AppState appState;
    appState.hasSession = fdp.ConsumeBool();
    appState.hasBackTask = fdp.ConsumeBool();

    manager.appStatesMap_.clear();
    manager.backgroundTaskListener_ = std::make_shared<BackgroundTaskListener>();
    manager.appStatesMap_.insert({pid, appState});
    manager.SubscribeBackgroundTask();
    manager.RecoveryAppState();
}

void BackgroundTaskListenerTaskIdFuzzTest(FuzzedDataProvider& fdp)
{
    BackgroundTaskListener listener;
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t taskId = fdp.ConsumeIntegral<int32_t>();
    int32_t anotherTaskId = fdp.ConsumeIntegral<int32_t>();

    listener.GetSizeOfContinuousTaskIdSet(pid);
    listener.AddTaskId(pid, taskId);
    listener.AddTaskId(pid, anotherTaskId);
    listener.GetSizeOfContinuousTaskIdSet(pid);
    listener.DeleteTaskId(pid, taskId);
    listener.GetSizeOfContinuousTaskIdSet(pid);
    listener.DeleteTaskId(pid, anotherTaskId);
    listener.GetSizeOfContinuousTaskIdSet(pid);
    listener.OnContinuousTaskStart(nullptr);
    listener.OnContinuousTaskStop(nullptr);
}

void ClientTypeManagerFuzzTest(FuzzedDataProvider& fdp)
{
    auto manager = ClientTypeManager::GetInstance();
    CHECK_AND_RETURN(manager != nullptr);
    uint32_t uid = fdp.ConsumeIntegral<uint32_t>();
    int32_t syncUid = fdp.ConsumeIntegralInRange<int32_t>(1, INT32_MAX);
    ClientType clientType = fdp.ConsumeBool() ? CLIENT_TYPE_GAME : CLIENT_TYPE_OTHERS;
    std::string bundleName = ConsumeString(fdp);

    manager->clientTypeMap_.clear();
    manager->GetClientTypeByUid(uid);
    manager->GetAndSaveClientType(uid, bundleName);
    manager->OnClientTypeQueryCompleted(uid, clientType, bundleName);
    manager->GetClientTypeByUid(uid);
    manager->OnClientTypeQueryCompleted(static_cast<uint32_t>(syncUid), clientType, bundleName);
    manager->GetClientTypeByUidSync(syncUid);
    manager->GetClientTypeByUidSync(0);
    manager->SetQueryClientTypeCallback(nullptr);
}

void ClientTypeManagerHandlerFuzzTest(FuzzedDataProvider& fdp)
{
    ClientTypeManagerHandler handler;
    ClientTypeListenerFuzzTest listener;
    uint32_t uid = fdp.ConsumeIntegral<uint32_t>();
    std::string bundleName = ConsumeString(fdp);
    auto eventObj = std::make_shared<ClientTypeManagerHandler::EventObj>();
    CHECK_AND_RETURN(eventObj != nullptr);
    eventObj->uid = uid;
    eventObj->bundleName = bundleName;

    handler.RegisterClientTypeListener(fdp.ConsumeBool() ? &listener : nullptr);
    handler.SetQueryClientTypeCallback(nullptr);
    handler.SendGetClientType(bundleName, uid);
    auto event = AppExecFwk::InnerEvent::Get(ClientTypeManagerHandler::GET_CLIENT_TYPE, eventObj);
    handler.ProcessEvent(event);
    auto unknownEvent = AppExecFwk::InnerEvent::Get(fdp.ConsumeIntegral<uint32_t>());
    handler.ProcessEvent(unknownEvent);
    handler.ReleaseEventRunner();
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
    AudioBackgroundManagerPlaybackAndMapFuzzTest,
    AudioBackgroundManagerRecoveryFuzzTest,
    BackgroundTaskListenerTaskIdFuzzTest,
    ClientTypeManagerFuzzTest,
    ClientTypeManagerHandlerFuzzTest,
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
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
