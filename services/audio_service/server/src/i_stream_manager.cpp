/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_utils.h"
#include "audio_engine_log.h"
#include "pa_adapter_manager.h"
#include "pro_adapter_manager.h"
#include "pro_audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
IStreamManager &IStreamManager::GetPlaybackManager(ManagerType managerType)
{
    switch (managerType) {
        case DIRECT_PLAYBACK:
            static ProAudioStreamManager directManager(DIRECT_PLAYBACK);
            return directManager;
        case VOIP_PLAYBACK:
            static ProAudioStreamManager voipManager(VOIP_PLAYBACK);
            return voipManager;
        case PLAYBACK:
        default:
            int32_t engineFlag = GetEngineFlag();
            if (engineFlag == 1) {
                static ProAdapterManager adapterManager(PLAYBACK);
                return adapterManager;
            } else {
                static PaAdapterManager adapterManager(PLAYBACK);
                return adapterManager;
            }
    }
}

IStreamManager &IStreamManager::GetDupPlaybackManager()
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        static ProAdapterManager adapterManager(DUP_PLAYBACK);
        return adapterManager;
    } else {
        static PaAdapterManager adapterManager(DUP_PLAYBACK);
        return adapterManager;
    }
}

IStreamManager &IStreamManager::GetDualPlaybackManager()
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        static ProAdapterManager adapterManager(DUAL_PLAYBACK);
        return adapterManager;
    } else {
        static PaAdapterManager adapterManager(DUAL_PLAYBACK);
        return adapterManager;
    }
}

IStreamManager &IStreamManager::GetRecorderManager()
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        static ProAdapterManager adapterManager(RECORDER);
        return adapterManager;
    } else {
        static PaAdapterManager adapterManager(RECORDER);
        return adapterManager;
    }
}
} // namespace AudioStandard
} // namespace OHOS
