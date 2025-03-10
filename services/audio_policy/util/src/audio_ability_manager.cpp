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

#ifndef LOG_TAG
#define LOG_TAG "AudioAbilityManager"
#endif

#include "audio_ability_manager.h"

#include <mutex>
#include "audio_common_log.h"
#include "audio_manager_base.h"
#include "audio_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AudioStandard {
constexpr unsigned int XCOLLIE_TIME_OUT_SECONDS = 10;
std::mutex g_asProxyMutex;
sptr<IStandardAudioService> g_asProxy = nullptr;

AudioAbilityManager *AudioAbilityManager::GetInstance()
{
    static AudioAbilityManager audioManager;
    return &audioManager;
}

AudioAbilityManager::~AudioAbilityManager() {}

static const sptr<IStandardAudioService> GetAudioAbilityManagerProxy()
{
    AudioXCollie xcollieGetAudioSystemManagerProxy("GetAudioSystemManagerProxy", XCOLLIE_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(g_asProxyMutex);
    if (g_asProxy == nullptr) {
        AudioXCollie xcollieGetSystemAbilityManager("GetSystemAbilityManager", XCOLLIE_TIME_OUT_SECONDS);
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        xcollieGetSystemAbilityManager.CancelXCollieTimer();

        AudioXCollie xcollieGetSystemAbility("GetSystemAbility", XCOLLIE_TIME_OUT_SECONDS);
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        g_asProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_asProxy != nullptr, nullptr, "get audio service proxy failed");
        xcollieGetSystemAbility.CancelXCollieTimer();
    }
    sptr<IStandardAudioService> gasp = g_asProxy;
    return gasp;
}

uint64_t AudioAbilityManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    const sptr<IStandardAudioService> gasp = GetAudioAbilityManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, 0, "Audio service unavailable.");
    return gasp->GetTransactionId(deviceType, deviceRole);
}
} // namespace AudioStandard
} // namespace OHOS