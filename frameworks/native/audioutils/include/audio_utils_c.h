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
#ifndef AUDIO_UTILS_C_H
#define AUDIO_UTILS_C_H

#include <inttypes.h>
#include <securec.h>

#ifdef __cplusplus
extern "C" {
#endif
#define SPRINTF_STRING_LEN 256
#define AUTO_CLEANUP(func) __attribute__((cleanup(func)))
#define AUTO_CLEAR AUTO_CLEANUP(CallEndAndClear)

typedef struct CTrace CTrace;

#define AUTO_NAME_LINE_INNER(name, line) name##line

#define AUTO_NAME_LINE(name, line) AUTO_NAME_LINE_INNER(name, line)

#define AUTO_NAME(name) AUTO_NAME_LINE(name, __LINE__)

// must use string length less than 256
#define AUTO_CTRACE(fmt, args...)                                           \
    char AUTO_NAME(str)[SPRINTF_STRING_LEN] = {0};                                     \
    int AUTO_NAME(ret) = sprintf_s(AUTO_NAME(str), SPRINTF_STRING_LEN, fmt, ##args);              \
    AUTO_CLEAR CTrace *AUTO_NAME(tmpCtrace) = (AUTO_NAME(ret) >= 0 ? GetAndStart(AUTO_NAME(str)) : NULL);    \
    (void)AUTO_NAME(tmpCtrace)

// must call with AUTO_CLEAR
CTrace *GetAndStart(const char *traceName);

void EndCTrace(CTrace *cTrace);

void CTraceCount(const char *traceName, int64_t count);

void CallEndAndClear(CTrace **cTrace);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_UTILS_C_H