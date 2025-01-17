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

#ifndef I_PROCESS_STATUS_LISTENER_H
#define I_PROCESS_STATUS_LISTENER_H

#include <memory>

#include "i_audio_process_stream.h"

namespace OHOS {
namespace AudioStandard {
class IProcessStatusListener {
public:
    virtual int32_t OnStart(IAudioProcessStream *processStream) = 0;

    virtual int32_t OnPause(IAudioProcessStream *processStream) = 0;

    virtual int32_t OnUpdateHandleInfo(IAudioProcessStream *processStream) = 0;

    virtual ~IProcessStatusListener() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_PROCESS_STATUS_LISTENER_H
