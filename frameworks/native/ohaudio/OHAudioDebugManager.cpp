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

#include "audio_manager/native_audio_debug_manager.h"

#include "audio_debug_manager.h"
#include "OHAudioCapturer.h"
#include "OHAudioRenderer.h"
#include "audio_loopback.h"

using OHOS::AudioStandard::AudioDebugManager;
using OHOS::AudioStandard::AudioLoopback;
using OHOS::AudioStandard::OHAudioCapturer;
using OHOS::AudioStandard::OHAudioRenderer;

static AudioDebugManager *ConvertDebugManager(OH_AudioDebugManager *debugManager)
{
    return reinterpret_cast<AudioDebugManager *>(debugManager);
}

static OHAudioRenderer *ConvertRenderer(OH_AudioRenderer *renderer)
{
    return reinterpret_cast<OHAudioRenderer *>(renderer);
}

static OHAudioCapturer *ConvertCapturer(OH_AudioCapturer *capturer)
{
    return reinterpret_cast<OHAudioCapturer *>(capturer);
}

static AudioLoopback *ConvertLoopback(OH_AudioLoopback *loopback)
{
    return reinterpret_cast<AudioLoopback *>(loopback);
}

OH_AudioCommon_Result OH_AudioManager_GetAudioDebugManager(OH_AudioDebugManager **debugManager)
{
    if (debugManager == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    *debugManager = reinterpret_cast<OH_AudioDebugManager *>(&AudioDebugManager::GetInstance());
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintAppInfo(OH_AudioDebugManager *debugManager, int32_t fd)
{
    if (debugManager == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    return ConvertDebugManager(debugManager)->PrintAppAudioDebugInfo(fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintRendererInfo(
    OH_AudioDebugManager *debugManager, OH_AudioRenderer *renderer, int32_t fd)
{
    if (debugManager == nullptr || renderer == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioRenderer *ohRenderer = ConvertRenderer(renderer);
    if (ohRenderer == nullptr || ohRenderer->GetAudioRenderer() == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }

    uintptr_t rendererKey = reinterpret_cast<uintptr_t>(ohRenderer->GetAudioRenderer());
    return ConvertDebugManager(debugManager)->PrintAudioRendererDebugInfo(rendererKey, fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintCapturerInfo(
    OH_AudioDebugManager *debugManager, OH_AudioCapturer *capturer, int32_t fd)
{
    if (debugManager == nullptr || capturer == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    OHAudioCapturer *ohCapturer = ConvertCapturer(capturer);
    if (ohCapturer == nullptr || ohCapturer->GetAudioCapturer() == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }

    uintptr_t capturerKey = reinterpret_cast<uintptr_t>(ohCapturer->GetAudioCapturer());
    return ConvertDebugManager(debugManager)->PrintAudioCapturerDebugInfo(capturerKey, fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintLoopbackInfo(
    OH_AudioDebugManager *debugManager, OH_AudioLoopback *loopback, int32_t fd)
{
    if (debugManager == nullptr || loopback == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    AudioLoopback *audioLoopback = ConvertLoopback(loopback);
    if (audioLoopback == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
    }

    uint32_t loopbackKey = audioLoopback->GetDebugKey();
    return ConvertDebugManager(debugManager)->PrintAudioLoopbackDebugInfo(loopbackKey, fd) == 0
        ? AUDIOCOMMON_RESULT_SUCCESS : AUDIOCOMMON_RESULT_ERROR_SYSTEM;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintSessionInfo(
    OH_AudioDebugManager *debugManager, OH_AudioSession *session, int32_t fd)
{
    (void)fd;
    if (debugManager == nullptr || session == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    return AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED;
}

OH_AudioCommon_Result OH_AudioDebugManager_PrintSuiteInfo(
    OH_AudioDebugManager *debugManager, OH_AudioSuiteEngine *audioSuiteEngine, int32_t fd)
{
    (void)fd;
    if (debugManager == nullptr || audioSuiteEngine == nullptr) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    return AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED;
}
