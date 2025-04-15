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

#include <cstdint>

enum StreamClass : uint32_t {
    PA_STREAM = 0,
    FAST_STREAM,
    VOIP_STREAM,
};

enum AudioFlag : uint32_t {
    AUDIO_FLAG_NONE = 0x0, // select
    AUDIO_OUTPUT_FLAG_NORMAL = 0x1, // route
    AUDIO_OUTPUT_FLAG_DIRECT = 0x2, // route
    AUDIO_OUTPUT_FLAG_HD = 0x4, // select
    AUDIO_OUTPUT_FLAG_MULTICHANNEL = 0x8, // select, route
    AUDIO_OUTPUT_FLAG_LOWPOWER = 0x10, // select, route
    AUDIO_OUTPUT_FLAG_FAST = 0x20, // select, route
    AUDIO_OUTPUT_FLAG_VOIP = 0x40, // select
    AUDIO_OUTPUT_FLAG_VOIP_FAST = 0x80, // select, route
    AUDIO_OUTPUT_FLAG_HWDECODING = 0x100, // select, route
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD = 0x200, // select, route
    AUDIO_INPUT_FLAG_NORMAL = 0x1000, // route
    AUDIO_INPUT_FLAG_FAST = 0x2000, // select, route
    AUDIO_INPUT_FLAG_VOIP = 0x4000, // select
    AUDIO_INPUT_FLAG_VOIP_FAST = 0x8000, // select, route
    AUDIO_INPUT_FLAG_WAKEUP = 0x10000, // select, route
    AUDIO_FLAG_MAX,
};

enum AudioStreamStatus : uint32_t {
    STREAM_STATUS_NEW = 0,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};
#endif // AUDIO_STREAM_ENUM_H