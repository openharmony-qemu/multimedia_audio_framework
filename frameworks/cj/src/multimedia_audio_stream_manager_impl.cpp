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
    size_t modeSize = audioSceneEffectInfo.mode.size();
    arr.size = static_cast<int64_t>(modeSize);
    if (arr.size == 0) {
        return CArrI32();
    }
    constexpr size_t maxCount = MAX_MEM_MALLOC_SIZE / sizeof(int32_t);
    if (modeSize > maxCount) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    size_t mallocSize = sizeof(int32_t) * modeSize;
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
    for (size_t i = 0; i < modeSize; i++) {
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
    size_t infoSize = audioRendererChangeInfos.size();
    arrInfo.size = static_cast<int64_t>(infoSize);
    if (arrInfo.size == 0) {
        return CArrAudioRendererChangeInfo();
    }
    constexpr size_t maxCount = MAX_MEM_MALLOC_SIZE / sizeof(CAudioRendererChangeInfo);
    if (infoSize > maxCount) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    
    size_t mallocSize = sizeof(CAudioRendererChangeInfo) * infoSize;
    auto head = static_cast<CAudioRendererChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return CArrAudioRendererChangeInfo();
    }
    if (memset_s(head, mallocSize, 0, mallocSize) != EOK) {
        free(head);
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    arrInfo.head = head;
    for (size_t i = 0; i < infoSize; i++) {
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
    size_t infoSize = audioCapturerChangeInfos.size();
    arrInfo.size = static_cast<int64_t>(infoSize);
    if (infoSize == 0) {
        return CArrAudioCapturerChangeInfo();
    }
    
    constexpr size_t maxCount = MAX_MEM_MALLOC_SIZE / sizeof(CAudioCapturerChangeInfo);
    if (infoSize > maxCount) {
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    
    size_t mallocSize = sizeof(CAudioCapturerChangeInfo) * infoSize;
    auto head = static_cast<CAudioCapturerChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return CArrAudioCapturerChangeInfo();
    }
    if (memset_s(head, mallocSize, 0, mallocSize) != EOK) {
        free(head);
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    arrInfo.head = head;
    for (size_t i = 0; i < infoSize; i++) {
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
