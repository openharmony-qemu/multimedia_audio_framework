/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "multimedia_audio_session_manager_impl.h"

#include "audio_policy_log.h"
#include "cj_lambda.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SESSION_CALLBACK_NAME = "audioSessionDeactived";
MMAAudioSessionManagerImpl::MMAAudioSessionManagerImpl()
{
    sessionMgr_ = AudioSessionManager::GetInstance();
}

MMAAudioSessionManagerImpl::~MMAAudioSessionManagerImpl()
{
    sessionMgr_ = nullptr;
}

void MMAAudioSessionManagerImpl::ActivateAudioSession(CAudioSessionStrategy& strategy, int32_t* errorCode)
{
    AudioSessionStrategy sessionStrategy;
    sessionStrategy.concurrencyMode = static_cast<AudioConcurrencyMode>(strategy.concurrencyMode);
    auto ret = sessionMgr_->ActivateAudioSession(sessionStrategy);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("ActivateAudioSession failure!");
        *errorCode = CJ_ERR_SYSTEM;
    }
}

void MMAAudioSessionManagerImpl::DeactivateAudioSession(int32_t* errorCode)
{
    auto ret = sessionMgr_->DeactivateAudioSession();
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("DeactivateAudioSession failure!");
        *errorCode = CJ_ERR_SYSTEM;
    }
}

bool MMAAudioSessionManagerImpl::IsAudioSessionActivated()
{
    return sessionMgr_->IsAudioSessionActivated();
}

void MMAAudioSessionManagerImpl::On(std::string type, int64_t id, int32_t* errorCode)
{
    if (!sessionCallback_) {
        sessionCallback_ = std::make_shared<CjAudioSessionCallback>();
        if (!sessionCallback_) {
            AUDIO_ERR_LOG("Memory allocation failed!");
            *errorCode = CJ_ERR_NO_MEMORY;
            return;
        }
    }

    int ret = sessionMgr_->SetAudioSessionCallback(sessionCallback_);
    if (ret != SUCCESS_CODE) {
        AUDIO_ERR_LOG("SetAudioSessionCallback failed!");
        return;
    }

    auto callback = reinterpret_cast<void (*)(CAudioSessionDeactiveEvent)>(id);
    sessionCallback_->RegisterFunc(CJLambda::Create(callback));
}
} // namespace AudioStandard
} // namespace OHOS