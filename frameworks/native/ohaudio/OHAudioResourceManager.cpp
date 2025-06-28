/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "OHAudioResourceManager.h"

using OHOS::AudioStandard::OHAudioResourceManager;
using OHOS::AudioStandard::OHAudioWorkgroup;

static OHOS::AudioStandard::OHAudioResourceManager *convertResourceManager(OH_AudioResourceManager* resourceManager)
{
    return (OHAudioResourceManager*) resourceManager;
}

static OHOS::AudioStandard::OHAudioWorkgroup *convertWorkgroup(OH_AudioWorkgroup* group)
{
    return (OHAudioWorkgroup*) group;
}

OH_AudioCommon_Result OH_AudioManager_GetAudioResourceManager(OH_AudioResourceManager **resourceManager)
{
    if (resourceManager == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioResourceManager");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    OHAudioResourceManager *audioResourceManager = OHAudioResourceManager::GetInstance();
    *resourceManager = (OH_AudioResourceManager*)audioResourceManager;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioResourceManager_CreateWorkgroup(OH_AudioResourceManager *resourceManager,
    const char *name, OH_AudioWorkgroup **group)
{
    if (resourceManager == nullptr || name == nullptr || group == nullptr) {
        AUDIO_ERR_LOG("invalid create param");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    *group = (OH_AudioWorkgroup *)(convertResourceManager(resourceManager)->CreateWorkgroup());
    CHECK_AND_RETURN_RET_LOG(group != nullptr, AUDIOCOMMON_RESULT_ERROR_SYSTEM, "workgroup returned nullptr");
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioResourceManager_ReleaseWorkgroup(OH_AudioResourceManager *resourceManager,
    OH_AudioWorkgroup *group)
{
    if (resourceManager == nullptr || group == nullptr) {
        AUDIO_ERR_LOG("invalid OH_AudioResourceManager or OH_AudioWorkgroup");
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (convertResourceManager(resourceManager)->ReleaseWorkgroup((OHAudioWorkgroup *)group)) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
}

OH_AudioCommon_Result OH_AudioWorkgroup_AddCurrentThread(OH_AudioWorkgroup *group, int32_t *tokenId)
{
    CHECK_AND_RETURN_RET_LOG(group != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "workgroup is nullptr");
    *tokenId = gettid();
    if (convertWorkgroup(group)->AddThread(*tokenId)) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
}

OH_AudioCommon_Result OH_AudioWorkgroup_RemoveThread(OH_AudioWorkgroup *group, int32_t tokenId)
{
    CHECK_AND_RETURN_RET_LOG(group != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "workgroup is nullptr");
    if (convertWorkgroup(group)->RemoveThread(tokenId)) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
}

OH_AudioCommon_Result OH_AudioWorkgroup_Start(OH_AudioWorkgroup *group, uint64_t startTime, uint64_t deadlineTime)
{
    CHECK_AND_RETURN_RET_LOG(group != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "workgroup is nullptr");
    if (convertWorkgroup(group)->Start(startTime, deadlineTime)) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
}

OH_AudioCommon_Result OH_AudioWorkgroup_Stop(OH_AudioWorkgroup *group)
{
    CHECK_AND_RETURN_RET_LOG(group != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "workgroup is nullptr");
    if (convertWorkgroup(group)->Stop()) {
        return AUDIOCOMMON_RESULT_SUCCESS;
    } else {
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }
}

namespace OHOS {
namespace AudioStandard {
OHAudioResourceManager *OHAudioResourceManager::GetInstance()
{
    static OHAudioResourceManager audioResourceManager;
    return &audioResourceManager;
}

OHAudioWorkgroup *OHAudioResourceManager::CreateWorkgroup()
{
    int id = AudioSystemManager::GetInstance()->CreateAudioWorkgroup();
    CHECK_AND_RETURN_RET_LOG(id >= 0, nullptr, "Create failed, the max num of workgroup is 2.");
    OHAudioWorkgroup *group = new(std::nothrow) OHAudioWorkgroup(id);
    if (group == nullptr) {
        AUDIO_ERR_LOG("construct OHAudioWorkgroup failed");
        AudioSystemManager::GetInstance()->ReleaseAudioWorkgroup(id);
    }
    return group;
}

bool OHAudioResourceManager::ReleaseWorkgroup(OHAudioWorkgroup *group)
{
    CHECK_AND_RETURN_RET_LOG(group != nullptr, false, "group is nullptr");
    AudioSystemManager::GetInstance()->ReleaseAudioWorkgroup(group->workgroupId);
    delete group;
    group = nullptr;
    return true;
}
} //namespace AudioStandard
} //namespace OHOS
