/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioAnahsManager"
#endif

#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_service_log.h"

#include "audio_anahs_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioAnahsManager *AudioAnahsManager::GetInstance()
{
    static AudioAnahsManager audioAnahsManager;
    return &audioAnahsManager;
}

int32_t AudioAnahsManager::SetAudioDeviceAnahsCallback(const std::shared_ptr<AudioDeviceAnahs> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioPolicyManager::GetInstance().SetAudioDeviceAnahsCallback(callback);
}

int32_t AudioAnahsManager::UnsetAudioDeviceAnahsCallback()
{
    return AudioPolicyManager::GetInstance().UnsetAudioDeviceAnahsCallback();
}
} // namespace AudioStandard
} // namespace OHOS
