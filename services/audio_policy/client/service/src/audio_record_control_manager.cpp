/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRecordControlManager"
#endif

#include "audio_record_control_manager.h"
#include "audio_policy_manager.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {
AudioRecordControlManager &AudioRecordControlManager::GetInstance()
{
    static AudioRecordControlManager instance;
    return instance;
}

int32_t AudioRecordControlManager::EnableSystemRecordController(bool show, const SystemRecordControllerConfig &config)
{
    if (config.sourceType != SOURCE_TYPE_MIC && config.sourceType != SOURCE_TYPE_CAMCORDER &&
        config.sourceType != SOURCE_TYPE_LIVE) {
        AUDIO_ERR_LOG("EnableSystemRecordController: invalid sourceType[%{public}d]", config.sourceType);
        return ERR_INVALID_PARAM;
    }
    return AudioPolicyManager::GetInstance().EnableSystemRecordController(show, config);
}

int32_t AudioRecordControlManager::SetRecordControllerEnabledChangeCallback(
    const std::shared_ptr<SystemRecordControllerEnabledChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetRecordControllerEnabledChangeCallback(callback);
}

int32_t AudioRecordControlManager::UnsetRecordControllerEnabledChangeCallback(
    const std::shared_ptr<SystemRecordControllerEnabledChangeCallback> &callback)
{
    return AudioPolicyManager::GetInstance().UnsetRecordControllerEnabledChangeCallback(callback);
}
} // namespace AudioStandard
} // namespace OHOS