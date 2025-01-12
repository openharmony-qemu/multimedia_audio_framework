/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#ifndef MULTIMEDIA_AUDIO_ERROR_H
#define MULTIMEDIA_AUDIO_ERROR_H

namespace OHOS {
namespace AudioStandard {
/* cangjie error code */
const int32_t CJ_ERROR_INVALID_PARAM = 6800101;
const int32_t CJ_ERR_NO_PERMISSION = 201;
const int32_t CJ_ERR_PERMISSION_DENIED = 202;
const int32_t CJ_ERR_INPUT_INVALID = 401;
const int32_t CJ_ERR_INVALID_PARAM = 6800101;
const int32_t CJ_ERR_NO_MEMORY = 6800102;
const int32_t CJ_ERR_ILLEGAL_STATE = 6800103;
const int32_t CJ_ERR_UNSUPPORTED = 6800104;
const int32_t CJ_ERR_TIMEOUT = 6800105;
const int32_t CJ_ERR_STREAM_LIMIT = 6800201;
const int32_t CJ_ERR_SYSTEM = 6800301;

const size_t CJ_ERR_INVALID_RETURN_VALUE = 0;
const int32_t CJ_ERR_INVALID_VALUE = -1;
const double CJ_ERR_INVALID_RETURN_DOUBLE_VALUE = 0.0;
const float CJ_ERR_INVALID_RETURN_FLOAT_VALUE = 0.0;

/* native error code */
const int32_t NATIVE_SUCCESS = 0;
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_ERROR_H
