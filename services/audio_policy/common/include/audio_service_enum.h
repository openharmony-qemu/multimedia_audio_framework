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
#ifndef AUDIO_SERVICE_ENUM_H
#define AUDIO_SERVICE_ENUM_H
#include <cstdint>

namespace OHOS {
namespace AudioStandard {
enum SessionOperation : uint32_t {
    SESSION_OPERATION_START = 0,
    SESSION_OPERATION_PAUSE,
    SESSION_OPERATION_STOP,
    SESSION_OPERATION_RELEASE,
};

enum SessionOperationMsg : uint32_t {
    SESSION_OP_MSG_DEFAULT = 0,
    SESSION_OP_MSG_REMOVE_PIPE = 1,
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_ENUM_H
