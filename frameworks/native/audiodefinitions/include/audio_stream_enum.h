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

#ifndef AUDIO_STREAM_ENUM_H
#define AUDIO_STREAM_ENUM_H

#include "audio_pipe_types.h"

enum StreamClass : uint32_t {
    PA_STREAM = 0,
    FAST_STREAM,
    VOIP_STREAM,
};

enum AudioStreamStatus : uint32_t {
    STREAM_STATUS_NEW = 0,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};
#endif // AUDIO_STREAM_ENUM_H
