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
#include <atomic>
#include <thread>
#include "audio_policy_server.h"
#include "audio_concurrency_service.h"
#include "power_state_listener.h"
#include "audio_device_info.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "audio_routing_manager.h"
#include "audio_stream_manager.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_hasPermission = false;
bool g_hasServerInit = false;

AudioPolicyServer *GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioConcurrencyServiceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioConcurrencyService> service = std::make_shared<AudioConcurrencyService>();
    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<AudioConcurrencyService::AudioConcurrencyDeathRecipient> concurrency =
        std::make_shared<AudioConcurrencyService::AudioConcurrencyDeathRecipient>(service, sessionID);

    wptr<IRemoteObject> remote;
    concurrency->OnRemoteDied(remote);

    std::shared_ptr<AudioConcurrencyCallback> callback;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<AudioConcurrencyService::AudioConcurrencyDeathRecipient> deathRecipient;
    std::shared_ptr<AudioConcurrencyService::AudioConcurrencyClient> audioConcurrencyClient =
        std::make_shared<AudioConcurrencyService::AudioConcurrencyClient>(callback, object, deathRecipient, sessionID);

    service->SetAudioConcurrencyCallback(sessionID, object);
    audioConcurrencyClient->OnConcedeStream();
}

void AudioPowerStateListenerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    sptr<AudioPolicyServer> audioPolicyServer;
    sptr<PowerStateListenerStub> powerStub =
        static_cast<sptr<PowerStateListenerStub>>(new(std::nothrow) PowerStateListener(audioPolicyServer));
    uint32_t code = *reinterpret_cast<const uint32_t*>(rawData);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    powerStub->OnRemoteRequest(code, data, reply, option);
    powerStub->OnSyncSleepCallbackStub(data);
    powerStub->OnSyncWakeupCallbackStub(data);

    GetServerPtr()->powerStateListener_->OnSyncSleep(true);
    GetServerPtr()->powerStateListener_->OnSyncWakeup(true);

    sptr<SyncHibernateListenerStub> syncStub =
        static_cast<sptr<SyncHibernateListenerStub>>(new(std::nothrow) SyncHibernateListener(audioPolicyServer));
    syncStub->OnRemoteRequest(code, data, reply, option);

    GetServerPtr()->syncHibernateListener_->OnSyncHibernate();
    GetServerPtr()->syncHibernateListener_->OnSyncWakeup(true);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *rawData, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioConcurrencyServiceFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPowerStateListenerFuzzTest(rawData, size);
    return 0;
}