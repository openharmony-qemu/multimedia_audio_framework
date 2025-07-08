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

#include "multimedia_audio_manager_impl.h"

#include "audio_manager_log.h"
#include "cj_lambda.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"
#include "multimedia_audio_routing_manager_impl.h"
#include "multimedia_audio_session_manager_impl.h"
#include "multimedia_audio_stream_manager_impl.h"
#include "multimedia_audio_volume_manager_impl.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioManagerImpl::MMAAudioManagerImpl()
{
    audioMgr_ = AudioSystemManager::GetInstance();
}

int32_t MMAAudioManagerImpl::GetAudioScene()
{
    auto scene = audioMgr_->GetAudioScene();
    if (scene == AUDIO_SCENE_VOICE_RINGING) {
        scene = AUDIO_SCENE_RINGING;
    }
    return scene;
}

int64_t MMAAudioManagerImpl::GetStreamManager(int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioStreamManagerImpl>();
    if (!mgr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create StreamManager error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

int64_t MMAAudioManagerImpl::GetRoutingManager(int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioRoutingManagerImpl>();
    if (!mgr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create RoutingManager error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}
int64_t MMAAudioManagerImpl::GetVolumeManager(int32_t* errorCode)
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

int64_t MMAAudioManagerImpl::GetSessionManager(int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioSessionManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetSessionManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetID();
}
}
} // namespace AudioStandard
} // namespace OHOS
