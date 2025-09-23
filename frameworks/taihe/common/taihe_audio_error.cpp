/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "taihe_audio_error.h"
#include "audio_log.h"

namespace ANI::Audio {
const std::string TAIHE_ERROR_INVALID_PARAM_INFO = "input parameter value error";
const std::string TAIHE_ERROR_PERMISSION_DENIED_INFO = "not system app";
const std::string TAIHE_ERR_INPUT_INVALID_INFO = "input parameter type or number mismatch";
const std::string TAIHE_ERR_INVALID_PARAM_INFO = "invalid parameter";
const std::string TAIHE_ERR_NO_MEMORY_INFO = "allocate memory failed";
const std::string TAIHE_ERR_ILLEGAL_STATE_INFO = "Operation not permit at current state";
const std::string TAIHE_ERR_UNSUPPORTED_INFO = "unsupported option";
const std::string TAIHE_ERR_TIMEOUT_INFO = "time out";
const std::string TAIHE_ERR_STREAM_LIMIT_INFO = "stream number limited";
const std::string TAIHE_ERR_SYSTEM_INFO = "system error";
const std::string TAIHE_ERR_NO_PERMISSION_INFO = "permission denied";

void TaiheAudioError::ThrowError(int32_t code)
{
    std::string messageValue = GetMessageByCode(code);
    taihe::set_business_error(code, messageValue);
}

void TaiheAudioError::ThrowError(int32_t code, const std::string &errMessage)
{
    std::string messageValue;
    if (code == TAIHE_ERR_INVALID_PARAM || code == TAIHE_ERR_INPUT_INVALID) {
        messageValue = errMessage;
    } else {
        messageValue = GetMessageByCode(code);
    }
    taihe::set_business_error(code, messageValue);
}

void TaiheAudioError::ThrowErrorAndReturn(int32_t errCode)
{
    AUDIO_ERR_LOG("errCode: %{public}d", errCode);
    ThrowError(errCode);
}

void TaiheAudioError::ThrowErrorAndReturn(int32_t errCode, const std::string &errMessage)
{
    AUDIO_ERR_LOG("errCode: %{public}d, errMsg: %{public}s", errCode, errMessage.c_str());
    ThrowError(errCode, errMessage);
}

std::string TaiheAudioError::GetMessageByCode(int32_t &code)
{
    std::string errMessage;
    switch (code) {
        case TAIHE_ERR_INVALID_PARAM:
        case OHOS::AudioStandard::ERR_INVALID_PARAM:
            errMessage = TAIHE_ERR_INVALID_PARAM_INFO;
            code = TAIHE_ERR_INVALID_PARAM;
            break;
        case TAIHE_ERR_NO_MEMORY:
            errMessage = TAIHE_ERR_NO_MEMORY_INFO;
            break;
        case TAIHE_ERR_ILLEGAL_STATE:
            errMessage = TAIHE_ERR_ILLEGAL_STATE_INFO;
            break;
        case TAIHE_ERR_UNSUPPORTED:
        case OHOS::AudioStandard::ERR_NOT_SUPPORTED:
            errMessage = TAIHE_ERR_UNSUPPORTED_INFO;
            code = TAIHE_ERR_UNSUPPORTED;
            break;
        case TAIHE_ERR_TIMEOUT:
            errMessage = TAIHE_ERR_TIMEOUT_INFO;
            break;
        case TAIHE_ERR_STREAM_LIMIT:
            errMessage = TAIHE_ERR_STREAM_LIMIT_INFO;
            break;
        case TAIHE_ERR_SYSTEM:
            errMessage = TAIHE_ERR_SYSTEM_INFO;
            break;
        case TAIHE_ERR_INPUT_INVALID:
            errMessage = TAIHE_ERR_INPUT_INVALID_INFO;
            break;
        case TAIHE_ERR_PERMISSION_DENIED:
        case OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED:
            errMessage = TAIHE_ERROR_PERMISSION_DENIED_INFO;
            code = TAIHE_ERR_PERMISSION_DENIED;
            break;
        case TAIHE_ERR_NO_PERMISSION:
        case OHOS::AudioStandard::ERR_PERMISSION_DENIED:
            errMessage = TAIHE_ERR_NO_PERMISSION_INFO;
            code = TAIHE_ERR_NO_PERMISSION;
            break;
        default:
            errMessage = TAIHE_ERR_SYSTEM_INFO;
            code = TAIHE_ERR_SYSTEM;
            break;
    }
    return errMessage;
}
} // namespace ANI::Audio
