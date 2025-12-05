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

#include "audio_service_proxy.h"
#include "audio_common_log.h"
#include "system_ability_definition.h"
#include "audio_server_death_recipient.h"
#include "audio_utils.h"
#include "iservice_registry.h"

namespace OHOS {
namespace AudioStandard {
std::mutex g_asProxyMutex;
sptr<IStandardAudioService> g_asProxy = nullptr;
constexpr unsigned int XCOLLIE_TIME_OUT_SECONDS = 10;

void AudioServerDied()
{
    std::lock_guard<std::mutex> lock(g_asProxyMutex);
    g_asProxy = nullptr;
}

const sptr<IStandardAudioService> AudioServiceProxy::GetAudioSystemManagerProxy()
{
    AudioXCollie xcollieGetAudioSystemManagerProxy("GetAudioSystemManagerProxy", XCOLLIE_TIME_OUT_SECONDS,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
    std::lock_guard<std::mutex> lock(g_asProxyMutex);
    if (g_asProxy == nullptr) {
        AudioXCollie xcollieGetSystemAbilityManager("GetSystemAbilityManager", XCOLLIE_TIME_OUT_SECONDS,
             nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        xcollieGetSystemAbilityManager.CancelXCollieTimer();

        AudioXCollie xcollieGetSystemAbility("GetSystemAbility", XCOLLIE_TIME_OUT_SECONDS);
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        g_asProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_asProxy != nullptr, nullptr, "get audio service proxy failed");
        xcollieGetSystemAbility.CancelXCollieTimer();

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
                AudioServerDied();
            });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_asProxy;
    return gasp;
}
} // namespace AudioStandard
} // namespace OHOS
