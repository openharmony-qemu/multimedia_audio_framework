/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <memory>
#include <string>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
struct ContextBase;

constexpr int32_t NAPI_ERROR_INVALID_PARAM = 6800101;
constexpr int32_t NAPI_ERR_NO_PERMISSION = 201;
constexpr int32_t NAPI_ERR_PERMISSION_DENIED = 202;
constexpr int32_t NAPI_ERR_INPUT_INVALID = 401;
constexpr int32_t NAPI_ERR_INVALID_PARAM = 6800101;
constexpr int32_t NAPI_ERR_NO_MEMORY = 6800102;
constexpr int32_t NAPI_ERR_ILLEGAL_STATE = 6800103;
constexpr int32_t NAPI_ERR_UNSUPPORTED = 6800104;
constexpr int32_t NAPI_ERR_TIMEOUT = 6800105;
constexpr int32_t NAPI_ERR_STREAM_LIMIT = 6800201;
constexpr int32_t NAPI_ERR_SYSTEM = 6800301;
constexpr int32_t NAPI_ERR_UNAVAILABLE_ON_DEVICE = 801;
constexpr int32_t NAPI_ERR_SERVICE_DIED = 6800302;
constexpr int32_t NAPI_ERR_NETWORK = 6800501;
constexpr int32_t NAPI_ERR_STORAGE_NOT_ENOUGH = 6800502;

class NapiAudioError {
public:
    static napi_status ThrowError(napi_env env, const char *napiMessage, int32_t napiCode);
    static void ThrowErrorDoNotUse(napi_env env, int32_t code);
    static void ThrowErrorDoNotUse(napi_env env, int32_t code, const std::string &errMessage);
    static napi_value ThrowErrorAndReturnDoNotUse(napi_env env, int32_t errCode);
    static napi_value ThrowErrorAndReturnDoNotUse(napi_env env, int32_t errCode, const std::string &errMessage);
    static napi_value ThrowErrorAndReturn(napi_env env, const char *napiMessage, int32_t napiCode);
    static std::string GetMessageByCode(int32_t &code);
    static bool HandleAudioManagerError(std::shared_ptr<ContextBase> context, int32_t audioManagerError);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_ERROR_H
