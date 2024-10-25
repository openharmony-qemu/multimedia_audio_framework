/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "multimedia_audio_stream_manager_impl.h"
#include "cj_lambda.h"
#include "audio_info.h"
#include "audio_log.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
extern "C" {
MMAAudioStreamManagerImpl::MMAAudioStreamManagerImpl()
{
    streamMgr_ = AudioStreamManager::GetInstance();
    cachedClientId_ = getpid();
    callback_ = std::make_shared<CjAudioCapturerStateChangeCallback>();
}

MMAAudioStreamManagerImpl::~MMAAudioStreamManagerImpl()
{
    streamMgr_ = nullptr;
}

bool MMAAudioStreamManagerImpl::IsActive(int32_t volumeType)
{
    return streamMgr_->IsStreamActive(static_cast<AudioVolumeType>(volumeType));
}

CArrI32 MMAAudioStreamManagerImpl::GetAudioEffectInfoArray(int32_t usage, int32_t *errorCode)
{
    AudioSceneEffectInfo audioSceneEffectInfo;
    int32_t ret = streamMgr_->GetEffectInfoArray(audioSceneEffectInfo, static_cast<StreamUsage>(usage));
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetEffectInfoArray failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    CArrI32 arr;
    arr.size = static_cast<int64_t>(audioSceneEffectInfo.mode.size());
    auto head = static_cast<int32_t *>(malloc(sizeof(int32_t) * audioSceneEffectInfo.mode.size()));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(audioSceneEffectInfo.mode.size()); i++) {
        head[i] = static_cast<int32_t>(audioSceneEffectInfo.mode[i]);
    }
    arr.head = head;
    return arr;
}

CArrAudioCapturerChangeInfo MMAAudioStreamManagerImpl::GetAudioCapturerInfoArray(int32_t *errorCode)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t ret = streamMgr_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetCurrentCapturerChangeInfos failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    CArrAudioCapturerChangeInfo arrInfo;
    arrInfo.size = static_cast<int64_t>(audioCapturerChangeInfos.size());
    auto head = static_cast<CAudioCapturerChangeInfo *>(
        malloc(sizeof(CAudioCapturerChangeInfo) * audioCapturerChangeInfos.size()));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(audioCapturerChangeInfos.size()); i++) {
        Convert2CAudioCapturerChangeInfo(head[i], *(audioCapturerChangeInfos[i]), errorCode);
    }
    return arrInfo;
}

void MMAAudioStreamManagerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode)
{
    if (callbackType == AudioStreamManagerCallbackType::CAPTURER_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrAudioCapturerChangeInfo)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("Register AudioCapturerChangeInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
        callback_->RegisterFunc(func);
        streamMgr_->RegisterAudioCapturerEventListener(cachedClientId_, callback_);
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
