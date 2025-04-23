/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AniAudioError"
#endif

#include "ani_audio_error.h"
#include "audio_common_log.h"
#include "ani_param_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
void AniAudioError::ThrowError(ani_env *env, const char *aniMessage, int32_t aniCode)
{
    CHECK_AND_RETURN_LOG(env != nullptr, "env is nullptr");
    string message = aniMessage;
    AUDIO_ERR_LOG("ThrowError errCode:%{public}d errMsg:%{public}s", aniCode, message.c_str());
    ani_object aniError = nullptr;
    AniParamUtils::CreateAniErrorObject(env, aniError, aniCode, message);
    env->ThrowError(static_cast<ani_error>(aniError));
}

void AniAudioError::ThrowError(ani_env *env, int32_t code)
{
    CHECK_AND_RETURN_LOG(env != nullptr, "env is nullptr");
    ani_object aniError = nullptr;
    std::string messageValue = GetMessageByCode(code);
    AniParamUtils::CreateAniErrorObject(env, aniError, code, messageValue);
    env->ThrowError(static_cast<ani_error>(aniError));
}

void AniAudioError::ThrowError(ani_env *env, int32_t code, const std::string &errMessage)
{
    CHECK_AND_RETURN_LOG(env != nullptr, "env is nullptr");
    std::string messageValue;
    if (code == ANI_ERR_INVALID_PARAM || code == ANI_ERR_INPUT_INVALID) {
        messageValue = errMessage;
    } else {
        messageValue = GetMessageByCode(code);
    }
    ani_object aniError = nullptr;
    AniParamUtils::CreateAniErrorObject(env, aniError, code, messageValue);
    env->ThrowError(static_cast<ani_error>(aniError));
}

ani_object AniAudioError::ThrowErrorAndReturn(ani_env *env, int32_t errCode)
{
    ThrowError(env, errCode);
    return nullptr;
}

ani_object AniAudioError::ThrowErrorAndReturn(ani_env *env, int32_t errCode, const std::string &errMessage)
{
    ThrowError(env, errCode, errMessage);
    return nullptr;
}

std::string AniAudioError::GetMessageByCode(int32_t &code)
{
    std::string errMessage;
    switch (code) {
        case ANI_ERR_INVALID_PARAM:
            errMessage = ANI_ERR_INVALID_PARAM_INFO;
            break;
        case ANI_ERR_NO_MEMORY:
            errMessage = ANI_ERR_NO_MEMORY_INFO;
            break;
        case ANI_ERR_ILLEGAL_STATE:
            errMessage = ANI_ERR_ILLEGAL_STATE_INFO;
            break;
        case ANI_ERR_UNSUPPORTED:
        case ERR_NOT_SUPPORTED:
            errMessage = ANI_ERR_UNSUPPORTED_INFO;
            code = ANI_ERR_UNSUPPORTED;
            break;
        case ANI_ERR_TIMEOUT:
            errMessage = ANI_ERR_TIMEOUT_INFO;
            break;
        case ANI_ERR_STREAM_LIMIT:
            errMessage = ANI_ERR_STREAM_LIMIT_INFO;
            break;
        case ANI_ERR_SYSTEM:
            errMessage = ANI_ERR_SYSTEM_INFO;
            break;
        case ANI_ERR_INPUT_INVALID:
            errMessage = ANI_ERR_INPUT_INVALID_INFO;
            break;
        case ANI_ERR_PERMISSION_DENIED:
        case ERR_PERMISSION_DENIED:
            errMessage = ANI_ERROR_PERMISSION_DENIED_INFO;
            code = ANI_ERR_PERMISSION_DENIED;
            break;
        case ANI_ERR_NO_PERMISSION:
        case ERR_SYSTEM_PERMISSION_DENIED:
            errMessage = ANI_ERR_NO_PERMISSION_INFO;
            code = ANI_ERR_NO_PERMISSION;
            break;
        default:
            errMessage = ANI_ERR_SYSTEM_INFO;
            code = ANI_ERR_SYSTEM;
            break;
    }
    return errMessage;
}
} // namespace AudioStandard
} // namespace OHOS
