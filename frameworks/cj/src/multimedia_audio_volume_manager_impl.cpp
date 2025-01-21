/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "multimedia_audio_volume_manager_impl.h"
#include "cj_lambda.h"
#include "audio_policy_log.h"
#include "multimedia_audio_error.h"
#include "multimedia_audio_volume_group_manager_impl.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioVolumeManagerImpl::MMAAudioVolumeManagerImpl()
{
    audioMngr_ = AudioSystemManager::GetInstance();
    volumeChangeCallback_ = std::make_shared<CjVolumeKeyEventCallback>();
    cachedClientId_ = getpid();
}

int64_t MMAAudioVolumeManagerImpl::GetVolumeGroupManager(int32_t groupId, int32_t *errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeGroupManagerImpl>(groupId);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetVolumeGroupManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

void MMAAudioVolumeManagerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode)
{
    if (callbackType != AudioVolumeManagerCallbackType::VOLUME_CHANGE) {
        return;
    }
    auto func = CJLambda::Create(reinterpret_cast<void (*)(CVolumeEvent)>(callback));
    if (func == nullptr) {
        AUDIO_ERR_LOG("Register CVolumeEvent event failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    volumeChangeCallback_->RegisterFunc(func);
    audioMngr_->RegisterVolumeKeyEventCallback(cachedClientId_, volumeChangeCallback_);
}
}
} // namespace AudioStandard
} // namespace OHOS
