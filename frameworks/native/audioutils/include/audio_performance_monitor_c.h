/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_PERFORMANCE_MONITOR_C_H
#define AUDIO_PERFORMANCE_MONITOR_C_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

enum PA_PIPE_TYPE {
    PA_PIPE_TYPE_NORMAL,
    PA_PIPE_TYPE_MULTICHANNEL
};

void RecordPaSilenceState(uint32_t sessionId, bool isSilence, enum PA_PIPE_TYPE paPipeType, uint32_t uid);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_PERFORMANCE_MONITOR_C_H