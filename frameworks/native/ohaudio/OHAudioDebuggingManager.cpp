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

#include "audio_manager/native_audio_debugging_manager.h"

#include "audio_debug_manager.h"
#include "OHAudioCapturer.h"
#include "OHAudioRenderer.h"
#include "OHAudioSessionManager.h"

using OHOS::AudioStandard::AudioDebugManager;
using OHOS::AudioStandard::OHAudioCapturer;
using OHOS::AudioStandard::OHAudioRenderer;
using OHOS::AudioStandard::OHAudioSessionManager;

static AudioDebugManager *ConvertDebugManager(OH_AudioDebuggingManager *debuggingManager)
{
    return reinterpret_cast<AudioDebugManager *>(debuggingManager);
}

static OHAudioRenderer *ConvertRenderer(OH_AudioRenderer *renderer)
{
    return reinterpret_cast<OHAudioRenderer *>(renderer);
}

static OHAudioCapturer *ConvertCapturer(OH_AudioCapturer *capturer)
{
    return reinterpret_cast<OHAudioCapturer *>(capturer);
}

static OHAudioSessionManager *ConvertSessionManager(OH_AudioSessionManager *session)
{
    return reinterpret_cast<OHAudioSessionManager *>(session);
}

OH_AudioCommon_Result OH_AudioManager_GetAudioDebuggingManager(OH_AudioDebuggingManager **debuggingManager)
{
    if (debuggingManager == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    *debuggingManager = reinterpret_cast<OH_AudioDebuggingManager *>(&AudioDebugManager::GetInstance());
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioDebuggingManager_PrintAppInfo(OH_AudioDebuggingManager *debuggingManager, int32_t fd)
{
    if (debuggingManager == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    return ConvertDebugManager(debuggingManager)->PrintAppAudioDebugInfo(fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebuggingManager_PrintRendererInfo(
    OH_AudioDebuggingManager *debuggingManager, OH_AudioRenderer *renderer, int32_t fd)
{
    if (debuggingManager == nullptr || renderer == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioRenderer *ohRenderer = ConvertRenderer(renderer);
    if (ohRenderer == nullptr || ohRenderer->GetAudioRenderer() == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }

    uint32_t rendererKey = ohRenderer->GetAudioRenderer()->GetRendererDebugKey();
    return ConvertDebugManager(debuggingManager)->PrintAudioRendererDebugInfo(rendererKey, fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebuggingManager_PrintCapturerInfo(
    OH_AudioDebuggingManager *debuggingManager, OH_AudioCapturer *capturer, int32_t fd)
{
    if (debuggingManager == nullptr || capturer == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioCapturer *ohCapturer = ConvertCapturer(capturer);
    if (ohCapturer == nullptr || ohCapturer->GetAudioCapturer() == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }

    uint32_t capturerKey = ohCapturer->GetAudioCapturer()->GetCapturerDebugKey();
    return ConvertDebugManager(debuggingManager)->PrintAudioCapturerDebugInfo(capturerKey, fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}


OH_AudioCommon_Result OH_AudioDebuggingManager_PrintSessionInfo(
    OH_AudioDebuggingManager *debuggingManager, OH_AudioSessionManager *session, int32_t fd)
{
    if (debuggingManager == nullptr || session == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioSessionManager *ohSessionManager = ConvertSessionManager(session);
    if (ohSessionManager == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    return ConvertDebugManager(debuggingManager)->PrintAudioSessionDebugInfo(fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}