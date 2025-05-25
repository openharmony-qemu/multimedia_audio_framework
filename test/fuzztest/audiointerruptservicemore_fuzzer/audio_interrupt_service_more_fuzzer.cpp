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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_interrupt_service.h"
#include "audio_policy_server.h"
#include "audio_session_info.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
bool g_hasServerInit = false;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

void MoreFuzzTest()
{
    GetServerPtr()->interruptService_->GetAudioServerProxy();
    GetServerPtr()->interruptService_->WriteServiceStartupError();

    int32_t pid = GetData<int32_t>();
    GetServerPtr()->interruptService_->OnSessionTimeout(pid);
    GetServerPtr()->interruptService_->HandleSessionTimeOutEvent(pid);
}

void AddAudioSessionFuzzTest()
{
    AudioSessionStrategy sessionStrategy;
    sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    GetServerPtr()->ActivateAudioSession(sessionStrategy);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;

    GetServerPtr()->interruptService_->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    GetServerPtr()->interruptService_->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    GetServerPtr()->interruptService_->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    GetServerPtr()->interruptService_->IsIncomingStreamLowPriority(focusEntry);
    GetServerPtr()->interruptService_->IsActiveStreamLowPriority(focusEntry);
}

void AddSetAudioManagerInterruptCallbackFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->interruptService_->GetAudioServerProxy();
    if (object == nullptr) {
        return;
    }
    GetServerPtr()->interruptService_->SetAudioManagerInterruptCallback(object);

    int32_t zoneId = GetData<int32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t uid = GetData<uint32_t>();
    GetServerPtr()->interruptService_->SetAudioInterruptCallback(zoneId, sessionId, object, uid);
}

void ResetNonInterruptControlFuzzTest() //build.gn 未添加宏定义defines
{
    uint32_t sessionId = GetData<uint32_t>();
    GetServerPtr()->interruptService_->GetClientTypeByStreamId(sessionId);
    GetServerPtr()->interruptService_->ResetNonInterruptControl(sessionId);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest()
{
    int id = GetData<int>();
    GetServerPtr()->interruptService_->ClearAudioFocusInfoListOnAccountsChanged(id);
}

typedef void (*TestFuncs[5])();

TestFuncs g_testFuncs = {
    MoreFuzzTest,
    AddAudioSessionFuzzTest,
    AddSetAudioManagerInterruptCallbackFuzzTest,
    ResetNonInterruptControlFuzzTest,
    ClearAudioFocusInfoListOnAccountsChangedFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}