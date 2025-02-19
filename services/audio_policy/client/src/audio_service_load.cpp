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

#include "audio_service_load.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "audio_errors.h"
#include "audio_log.h"


namespace OHOS {
namespace AudioStandard {


AudioServiceLoad *AudioServiceLoad::GetInstance(void)
{
    static AudioServiceLoad audioServiceLoad;
    return &audioServiceLoad;
}

int32_t AudioServiceLoad::LoadAudioService(void)
{
    AUDIO_INFO_LOG("start LoadAudioService");
    std::lock_guard<std::mutex> lock(mutex_);
    if (isAudioServiceLoading_) {
        AUDIO_INFO_LOG("AudioService is loading");
        return SUCCESS;
    }
    isAudioServiceLoading_ = true;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        isAudioServiceLoading_ = false;
        AUDIO_ERR_LOG("get system ability failed");
        return ERR_READ_FAILED;
    }
    sptr<AudioLoadCallback> audioLoadCallback(new AudioLoadCallback());
    int32_t ret = samgr->LoadSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, audioLoadCallback);
    if (ret != SUCCESS) {
        isAudioServiceLoading_ = false;
        AUDIO_ERR_LOG("Load audio server SA failed, ret code:%{public}d", ret);
        return ret;
    }
    ret = samgr->LoadSystemAbility(AUDIO_POLICY_SERVICE_ID, audioLoadCallback);
    if (ret != SUCCESS) {
        isAudioServiceLoading_ = false;
        AUDIO_ERR_LOG("Load audio policy SA failed, ret code:%{public}d", ret);
        return ret;
    }
    return SUCCESS;
}

void AudioServiceLoad::SetLoadFinish(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isAudioServiceLoading_ = false;
}

void AudioLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    AUDIO_INFO_LOG("load Audio service success");
    AudioServiceLoad::GetInstance()->SetLoadFinish();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("remoteObject is nullptr");
        return;
    }
}

void AudioLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    AUDIO_ERR_LOG("load Audio service fail");
    AudioServiceLoad::GetInstance()->SetLoadFinish();
}
} // namespace AudioStandard
} // namespace OHOS
