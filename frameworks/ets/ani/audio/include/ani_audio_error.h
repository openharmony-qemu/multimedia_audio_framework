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
#ifndef ANI_AUDIO_ERROR_H
#define ANI_AUDIO_ERROR_H

#include <string>
#include "ani.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
class AniAudioError {
public:
    static void ThrowError(ani_env *env, const char *aniMessage, int32_t aniCode);
    static void ThrowError(ani_env *env, int32_t code);
    static void ThrowError(ani_env *env, int32_t code, const std::string &errMessage);
    static ani_object ThrowErrorAndReturn(ani_env *env, int32_t errCode);
    static ani_object ThrowErrorAndReturn(ani_env *env, int32_t errCode, const std::string &errMessage);
    static std::string GetMessageByCode(int32_t &code);
};

const int32_t ANI_ERROR_INVALID_PARAM = 6800101;
const int32_t ANI_ERR_NO_PERMISSION = 201;
const int32_t ANI_ERR_PERMISSION_DENIED = 202;
const int32_t ANI_ERR_INPUT_INVALID = 401;
const int32_t ANI_ERR_INVALID_PARAM = 6800101;
const int32_t ANI_ERR_NO_MEMORY = 6800102;
const int32_t ANI_ERR_ILLEGAL_STATE = 6800103;
const int32_t ANI_ERR_UNSUPPORTED = 6800104;
const int32_t ANI_ERR_TIMEOUT = 6800105;
const int32_t ANI_ERR_STREAM_LIMIT = 6800201;
const int32_t ANI_ERR_SYSTEM = 6800301;

const std::string ANI_ERROR_INVALID_PARAM_INFO = "input parameter value error";
const std::string ANI_ERROR_PERMISSION_DENIED_INFO = "not system app";
const std::string ANI_ERR_INPUT_INVALID_INFO = "input parameter type or number mismatch";
const std::string ANI_ERR_INVALID_PARAM_INFO = "invalid parameter";
const std::string ANI_ERR_NO_MEMORY_INFO = "allocate memory failed";
const std::string ANI_ERR_ILLEGAL_STATE_INFO = "Operation not permit at current state";
const std::string ANI_ERR_UNSUPPORTED_INFO = "unsupported option";
const std::string ANI_ERR_TIMEOUT_INFO = "time out";
const std::string ANI_ERR_STREAM_LIMIT_INFO = "stream number limited";
const std::string ANI_ERR_SYSTEM_INFO = "system error";
const std::string ANI_ERR_NO_PERMISSION_INFO = "permission denied";
} // namespace AudioStandard
} // namespace OHOS
#endif // ANI_AUDIO_ERROR_H
