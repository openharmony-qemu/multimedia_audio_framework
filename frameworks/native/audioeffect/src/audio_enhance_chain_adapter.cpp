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
#define LOG_TAG "AudioEnhanceChainAdapter"
#endif

#include "audio_enhance_chain_adapter.h"
#include <map>

#include "securec.h"
#include "audio_effect_log.h"
#include "audio_effect.h"
#include "audio_errors.h"
#include "audio_enhance_chain_manager.h"

using namespace OHOS::AudioStandard;

int32_t EnhanceChainManagerProcess(const char *sceneType, EnhanceBufferAttr *enhanceBufferAttr)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (audioEnhanceChainMananger->ApplyAudioEnhanceChain(sceneTypeString, enhanceBufferAttr) != SUCCESS) {
        AUDIO_ERR_LOG("%{public}s process failed", sceneType);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("%{public}s process success", sceneType);
    return SUCCESS;
}

int32_t EnhanceChainManagerCreateCb(const char *sceneType, const char *enhanceMode, const char *upAndDownDevice)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    std::string sceneTypeString = "";
    std::string enhanceModeString = "";
    std::string upAndDownDeviceString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (enhanceMode) {
        enhanceModeString = enhanceMode;
    }
    if (upAndDownDevice) {
        upAndDownDeviceString = upAndDownDevice;
    }
    if (audioEnhanceChainMananger->CreateAudioEnhanceChainDynamic(sceneTypeString,
        enhanceModeString, upAndDownDeviceString) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t EnhanceChainManagerReleaseCb(const char *sceneType, const char *enhanceMode, const char *upAndDownDevice)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    std::string sceneTypeString = "";
    std::string upAndDownDeviceString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    if (upAndDownDevice) {
        upAndDownDeviceString = upAndDownDevice;
    }
    if (audioEnhanceChainMananger->ReleaseAudioEnhanceChainDynamic(sceneTypeString,
        upAndDownDeviceString) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

bool EnhanceChainManagerExist(const char *sceneType)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    std::string sceneTypeString = "";
    if (sceneType) {
        sceneTypeString = sceneType;
    }
    return audioEnhanceChainMananger->ExistAudioEnhanceChain(sceneTypeString);
}
