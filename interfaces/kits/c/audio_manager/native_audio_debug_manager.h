/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NATIVE_AUDIO_DEBUG_MANAGER_H
#define NATIVE_AUDIO_DEBUG_MANAGER_H

#include "native_audio_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AudioDebugManager OH_AudioDebugManager;
typedef struct OH_AudioRendererStruct OH_AudioRenderer;
typedef struct OH_AudioCapturerStruct OH_AudioCapturer;
typedef struct OH_AudioLoopbackStruct OH_AudioLoopback;
typedef struct OH_AudioSessionStruct OH_AudioSession;
typedef struct OH_AudioSuiteEngineStruct OH_AudioSuiteEngine;

OH_AudioCommon_Result OH_AudioManager_GetAudioDebugManager(OH_AudioDebugManager **debugManager);

OH_AudioCommon_Result OH_AudioDebugManager_PrintAppInfo(OH_AudioDebugManager *debugManager, int32_t fd);

OH_AudioCommon_Result OH_AudioDebugManager_PrintRendererInfo(
    OH_AudioDebugManager *debugManager, OH_AudioRenderer *renderer, int32_t fd);

OH_AudioCommon_Result OH_AudioDebugManager_PrintCapturerInfo(
    OH_AudioDebugManager *debugManager, OH_AudioCapturer *capturer, int32_t fd);

OH_AudioCommon_Result OH_AudioDebugManager_PrintLoopbackInfo(
    OH_AudioDebugManager *debugManager, OH_AudioLoopback *loopback, int32_t fd);

OH_AudioCommon_Result OH_AudioDebugManager_PrintSessionInfo(
    OH_AudioDebugManager *debugManager, OH_AudioSession *session, int32_t fd);

OH_AudioCommon_Result OH_AudioDebugManager_PrintSuiteInfo(
    OH_AudioDebugManager *debugManager, OH_AudioSuiteEngine *audioSuiteEngine, int32_t fd);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AUDIO_DEBUG_MANAGER_H
