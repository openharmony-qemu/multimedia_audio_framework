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
#ifndef NAPI_AUDIO_ERROR_H
#define NAPI_AUDIO_ERROR_H

#include <map>
#include <string>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioError {
public:
    static napi_status ThrowError(napi_env env, const char *napiMessage, int32_t napiCode);
    static void ThrowError(napi_env env, int32_t code);
    static void ThrowError(napi_env env, int32_t code, const std::string &errMessage);
    static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode);
    static napi_value ThrowErrorAndReturn(napi_env env, int32_t errCode, const std::string &errMessage);
    static std::string GetMessageByCode(int32_t &code);
};

const int32_t NAPI_ERROR_INVALID_PARAM = 6800101;
const int32_t NAPI_ERR_NO_PERMISSION = 201;
const int32_t NAPI_ERR_PERMISSION_DENIED = 202;
const int32_t NAPI_ERR_INPUT_INVALID = 401;
const int32_t NAPI_ERR_INVALID_PARAM = 6800101;
const int32_t NAPI_ERR_NO_MEMORY = 6800102;
const int32_t NAPI_ERR_ILLEGAL_STATE = 6800103;
const int32_t NAPI_ERR_UNSUPPORTED = 6800104;
const int32_t NAPI_ERR_TIMEOUT = 6800105;
const int32_t NAPI_ERR_STREAM_LIMIT = 6800201;
const int32_t NAPI_ERR_SYSTEM = 6800301;

const std::string NAPI_ERROR_INVALID_PARAM_INFO = "input parameter value error";
const std::string NAPI_ERROR_PERMISSION_DENIED_INFO = "not system app";
const std::string NAPI_ERR_INPUT_INVALID_INFO = "input parameter type or number mismatch";
const std::string NAPI_ERR_INVALID_PARAM_INFO = "invalid parameter";
const std::string NAPI_ERR_NO_MEMORY_INFO = "allocate memory failed";
const std::string NAPI_ERR_ILLEGAL_STATE_INFO = "Operation not permit at current state";
const std::string NAPI_ERR_UNSUPPORTED_INFO = "unsupported option";
const std::string NAPI_ERR_TIMEOUT_INFO = "time out";
const std::string NAPI_ERR_STREAM_LIMIT_INFO = "stream number limited";
const std::string NAPI_ERR_SYSTEM_INFO = "system error";
const std::string NAPI_ERR_NO_PERMISSION_INFO = "permission denied";
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_ERROR_H
