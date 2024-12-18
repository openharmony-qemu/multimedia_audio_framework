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
#define LOG_TAG "AudioCombineDenoisingManager"
#endif

#include "audio_combine_denoising_manager.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioCombineDenoisingManager *AudioCombineDenoisingManager::GetInstance()
{
    static AudioCombineDenoisingManager audioCombineDenoisingManager;
    return &audioCombineDenoisingManager;
}

int32_t AudioCombineDenoisingManager::RegisterNnStateEventListener(
    const std::shared_ptr<AudioNnStateChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterNnStateEventListener(callback);
}

int32_t AudioCombineDenoisingManager::UnregisterNnStateEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterNnStateEventListener();
}
} // namespace AudioStandard
} // namespace OHOS