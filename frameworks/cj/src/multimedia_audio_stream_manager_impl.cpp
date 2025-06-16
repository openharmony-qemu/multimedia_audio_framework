/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_manager_log.h"
#include "cj_lambda.h"
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
    callbackRenderer_ = std::make_shared<CjAudioRendererStateChangeCallback>();
}

MMAAudioStreamManagerImpl::~MMAAudioStreamManagerImpl()
{
    streamMgr_ = nullptr;
}

bool MMAAudioStreamManagerImpl::IsActive(int32_t volumeType)
{
    return streamMgr_->IsStreamActive(GetNativeAudioVolumeType(volumeType));
}

CArrI32 MMAAudioStreamManagerImpl::GetAudioEffectInfoArray(int32_t usage, int32_t* errorCode)
{
    AudioSceneEffectInfo audioSceneEffectInfo {};
    int32_t ret = streamMgr_->GetEffectInfoArray(audioSceneEffectInfo, static_cast<StreamUsage>(usage));
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetEffectInfoArray failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    CArrI32 arr {};
    arr.size = static_cast<int64_t>(audioSceneEffectInfo.mode.size());
    if (arr.size == 0) {
        return CArrI32();
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(int32_t)) * static_cast<int32_t>(arr.size);
    if (mallocSize > static_cast<int32_t>(sizeof(int32_t) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    auto head = static_cast<int32_t*>(malloc(mallocSize));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return CArrI32();
    }
    if (memset_s(head, arr.size, 0, arr.size) != EOK) {
        free(head);
        head = nullptr;
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    for (int32_t i = 0; i < static_cast<int32_t>(arr.size); i++) {
        head[i] = static_cast<int32_t>(audioSceneEffectInfo.mode[i]);
    }
    arr.head = head;
    return arr;
}

CArrAudioRendererChangeInfo MMAAudioStreamManagerImpl::GetCurrentRendererChangeInfos(int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos {};
    int32_t ret = streamMgr_->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetCurrentRendererChangeInfos failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    CArrAudioRendererChangeInfo arrInfo {};
    arrInfo.size = static_cast<int64_t>(audioRendererChangeInfos.size());
    if (arrInfo.size == 0) {
        return CArrAudioRendererChangeInfo();
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(CAudioRendererChangeInfo)) * static_cast<int32_t>(arrInfo.size);
    if (mallocSize > static_cast<int32_t>(sizeof(CAudioRendererChangeInfo) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    auto head = static_cast<CAudioRendererChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return CArrAudioRendererChangeInfo();
    }
    arrInfo.head = head;
    if (memset_s(head, arrInfo.size, 0, arrInfo.size) != EOK) {
        free(head);
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    for (int32_t i = 0; i < static_cast<int32_t>(audioRendererChangeInfos.size()); i++) {
        Convert2CAudioRendererChangeInfo(head[i], *(audioRendererChangeInfos[i]), errorCode);
        if (*errorCode != SUCCESS_CODE) {
            FreeCArrAudioRendererChangeInfo(arrInfo);
            *errorCode = CJ_ERR_SYSTEM;
            return CArrAudioRendererChangeInfo();
        }
    }
    return arrInfo;
}

CArrAudioCapturerChangeInfo MMAAudioStreamManagerImpl::GetAudioCapturerInfoArray(int32_t* errorCode)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos {};
    int32_t ret = streamMgr_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    if (ret != AUDIO_OK) {
        AUDIO_ERR_LOG("GetCurrentCapturerChangeInfos failure!");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    CArrAudioCapturerChangeInfo arrInfo {};
    arrInfo.size = static_cast<int64_t>(audioCapturerChangeInfos.size());
    if (arrInfo.size == 0) {
        return CArrAudioCapturerChangeInfo();
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(CAudioRendererChangeInfo)) * static_cast<int32_t>(arrInfo.size);
    if (mallocSize > static_cast<int32_t>(sizeof(AudioCapturerChangeInfo) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    auto head = static_cast<CAudioCapturerChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return CArrAudioCapturerChangeInfo();
    }
    arrInfo.head = head;
    if (memset_s(head, arrInfo.size, 0, arrInfo.size) != EOK) {
        free(head);
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    for (int32_t i = 0; i < static_cast<int32_t>(audioCapturerChangeInfos.size()); i++) {
        Convert2CAudioCapturerChangeInfo(head[i], *(audioCapturerChangeInfos[i]), errorCode);
        if (*errorCode != SUCCESS_CODE) {
            FreeCArrAudioCapturerChangeInfo(arrInfo);
            *errorCode = CJ_ERR_SYSTEM;
            return CArrAudioCapturerChangeInfo();
        }
    }
    return arrInfo;
}

void MMAAudioStreamManagerImpl::RegisterCallback(int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    if (callbackType == AudioStreamManagerCallbackType::CAPTURER_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrAudioCapturerChangeInfo)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("AudioCapturerChangeInfo event created failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callback_->RegisterFunc(func);
        int32_t ret = streamMgr_->RegisterAudioCapturerEventListener(cachedClientId_, callback_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("Register AudioCapturerChangeInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
    }
    if (callbackType == AudioStreamManagerCallbackType::RENDERER_CHANGE) {
        auto func = CJLambda::Create(reinterpret_cast<void (*)(CArrAudioRendererChangeInfo)>(callback));
        if (func == nullptr) {
            AUDIO_ERR_LOG("AudioRendererChangeInfo event created failure!");
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        callbackRenderer_->RegisterFunc(func);
        int32_t ret = streamMgr_->RegisterAudioRendererEventListener(cachedClientId_, callbackRenderer_);
        if (ret != SUCCESS_CODE) {
            AUDIO_ERR_LOG("Register AudioRendererChangeInfo event failure!");
            *errorCode = CJ_ERR_SYSTEM;
        }
    }
}
}
} // namespace AudioStandard
} // namespace OHOS
