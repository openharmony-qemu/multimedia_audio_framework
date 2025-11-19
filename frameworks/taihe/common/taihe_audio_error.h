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

#ifndef TAIHE_AUDIO_ERROR_H
#define TAIHE_AUDIO_ERROR_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_errors.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
constexpr int32_t TAIHE_ERROR_INVALID_PARAM = 6800101;
constexpr int32_t TAIHE_ERR_NO_PERMISSION = 201;
constexpr int32_t TAIHE_ERR_PERMISSION_DENIED = 202;
constexpr int32_t TAIHE_ERR_INPUT_INVALID = 401;
constexpr int32_t TAIHE_ERR_INVALID_PARAM = 6800101;
constexpr int32_t TAIHE_ERR_NO_MEMORY = 6800102;
constexpr int32_t TAIHE_ERR_ILLEGAL_STATE = 6800103;
constexpr int32_t TAIHE_ERR_UNSUPPORTED = 6800104;
constexpr int32_t TAIHE_ERR_TIMEOUT = 6800105;
constexpr int32_t TAIHE_ERR_STREAM_LIMIT = 6800201;
constexpr int32_t TAIHE_ERR_SYSTEM = 6800301;
constexpr int32_t TAIHE_ERR_UNAVAILABLE_ON_DEVICE = 801;

class TaiheAudioError {
public:
    static void ThrowError(int32_t code);
    static void ThrowError(int32_t code, const std::string &errMessage);
    static void ThrowErrorAndReturn(int32_t errCode);
    static void ThrowErrorAndReturn(int32_t errCode, const std::string &errMessage);
    static std::string GetMessageByCode(int32_t &code);
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_ERROR_H