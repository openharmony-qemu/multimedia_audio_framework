/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "multimedia_audio_manager_impl.h"
#include "multimedia_audio_volume_manager_impl.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

#include "audio_info.h"
#include "audio_log.h"
#include "timestamp.h"
// using namespace OHOS::FFI;

namespace OHOS {
namespace AudioStandard {
extern "C" {
// Audio Manager

MMAAudioManagerImpl::MMAAudioManagerImpl()
{
    audioMngr_ = AudioSystemManager::GetInstance();
}

int64_t MMAAudioManagerImpl::GetVolumeManager(int32_t *errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetVolumeManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}
}
} // namespace AudioStandard
} // namespace OHOS