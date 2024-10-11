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

#ifndef LOG_TAG
#define LOG_TAG "OHAudioSessionManager"
#endif

#include "OHAudioSessionManager.h"
#include <ostream>
#include <iostream>

using OHOS::AudioStandard::OHAudioSessionManager;
using OHOS::AudioStandard::AudioSessionManager;
using namespace std;

static OHOS::AudioStandard::OHAudioSessionManager *convertManager(OH_AudioSessionManager* manager)
{
    return (OHAudioSessionManager*) manager;
}


OH_AudioCommon_Result OH_AudioManager_GetAudioSessionManager(OH_AudioSessionManager **audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = OHAudioSessionManager::GetInstance();
    *audioSessionManager = reinterpret_cast<OH_AudioSessionManager*>(ohAudioSessionManager);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioSessionManager_RegisterSessionDeactivatedCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_DeactivatedCallback callback)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->SetAudioSessionCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_UnregisterSessionDeactivatedCallback(
    OH_AudioSessionManager *audioSessionManager, OH_AudioSession_DeactivatedCallback callback)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "callback is nullptr");
    return ohAudioSessionManager->UnsetAudioSessionCallback(callback);
}

OH_AudioCommon_Result OH_AudioSessionManager_ActivateAudioSession(
    OH_AudioSessionManager *audioSessionManager, const OH_AudioSession_Strategy *strategy)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    OHOS::AudioStandard::AudioSessionStrategy audioStrategy;
    audioStrategy.concurrencyMode =
        static_cast<OHOS::AudioStandard::AudioConcurrencyMode>(strategy->concurrencyMode);
    return ohAudioSessionManager->ActivateAudioSession(audioStrategy);
}

OH_AudioCommon_Result OH_AudioSessionManager_DeactivateAudioSession(
    OH_AudioSessionManager *audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "ohAudioSessionManager is nullptr");
    return ohAudioSessionManager->DeactivateAudioSession();
}

bool OH_AudioSessionManager_IsAudioSessionActivated(
    OH_AudioSessionManager *audioSessionManager)
{
    OHAudioSessionManager* ohAudioSessionManager = convertManager(audioSessionManager);
    CHECK_AND_RETURN_RET_LOG(ohAudioSessionManager != nullptr, false, "ohAudioSessionManager is nullptr");
    return ohAudioSessionManager->IsAudioSessionActivated();
}


namespace OHOS {
namespace AudioStandard {

OHAudioSessionManager::OHAudioSessionManager()
{
    AUDIO_INFO_LOG("OHAudioSessionManager created!");
}

OHAudioSessionManager::~OHAudioSessionManager()
{
    AUDIO_INFO_LOG("OHAudioSessionManager destroyed!");
}

OH_AudioCommon_Result OHAudioSessionManager::SetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    std::shared_ptr<OHAudioSessionCallback> ohAudioSessionCallback =
        std::make_shared<OHAudioSessionCallback>(callback);
    if (ohAudioSessionCallback != nullptr) {
        audioSessionManager_->SetAudioSessionCallback(ohAudioSessionCallback);
        return AUDIOCOMMON_RESULT_SUCCESS;
    }
    return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
}

OH_AudioCommon_Result OHAudioSessionManager::UnsetAudioSessionCallback(OH_AudioSession_DeactivatedCallback callback)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    std::shared_ptr<OHAudioSessionCallback> ohAudioSessionCallback =
        std::make_shared<OHAudioSessionCallback>(callback);
    audioSessionManager_->UnsetAudioSessionCallback(ohAudioSessionCallback);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioSessionManager::ActivateAudioSession(const AudioSessionStrategy &strategy)
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t ret = audioSessionManager_->ActivateAudioSession(strategy);
    if (ret == 0) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioCommon_Result OHAudioSessionManager::DeactivateAudioSession()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "failed, audioSessionManager_ is null");
    int32_t ret = audioSessionManager_->DeactivateAudioSession();
    if (ret == 0) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }
}

bool OHAudioSessionManager::IsAudioSessionActivated()
{
    CHECK_AND_RETURN_RET_LOG(audioSessionManager_ != nullptr, false, "failed, audioSessionManager_ is null");
    return audioSessionManager_->IsAudioSessionActivated();
}

void OHAudioSessionCallback::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    OH_AudioSession_DeactivatedEvent event;
    event.reason = static_cast<OH_AudioSession_DeactivatedReason>(deactiveEvent.deactiveReason);
    callback_(event);
}

} // namespace AudioStandard
} // namespace OHOS