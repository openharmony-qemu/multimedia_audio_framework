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
const int32_t LIMITSIZE = 4;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";

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

void MoreFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->interruptService_->GetAudioServerProxy();
    GetServerPtr()->interruptService_->WriteServiceStartupError();

    int32_t pid = *reinterpret_cast<const int32_t*>(rawData);
    GetServerPtr()->interruptService_->OnSessionTimeout(pid);
    GetServerPtr()->interruptService_->HandleSessionTimeOutEvent(pid);
}

void AddAudioSessionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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

void AddSetAudioManagerInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->interruptService_->GetAudioServerProxy();
    if (object == nullptr) {
        return;
    }
    GetServerPtr()->interruptService_->SetAudioManagerInterruptCallback(object);

    int32_t zoneId = *reinterpret_cast<const int32_t*>(rawData);
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t uid = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->interruptService_->SetAudioInterruptCallback(zoneId, sessionId, object, uid);
}

void ResetNonInterruptControlFuzzTest(const uint8_t *rawData, size_t size) //build.gn 未添加宏定义defines
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->interruptService_->GetClientTypeBySessionId(sessionId);
    GetServerPtr()->interruptService_->ResetNonInterruptControl(sessionId);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int id = *reinterpret_cast<const int*>(rawData);
    GetServerPtr()->interruptService_->ClearAudioFocusInfoListOnAccountsChanged(id);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::MoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AddAudioSessionFuzzTest(rawData, size);
    OHOS::AudioStandard::AddSetAudioManagerInterruptCallbackFuzzTest(rawData, size);
    OHOS::AudioStandard::ResetNonInterruptControlFuzzTest(rawData, size);
    OHOS::AudioStandard::ClearAudioFocusInfoListOnAccountsChangedFuzzTest(rawData, size);
    return 0;
}