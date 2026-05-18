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

/**
 * @addtogroup OHAudio
 * @{
 *
 * @brief Provide the definition of the C interface for the audio module.
 *
 * @since 26.0.0
 */

/**
 * @file native_audio_debugging_manager.h
 *
 * @brief Declare audio debugging manager related interfaces.
 *
 * This file interfaces are used for the creation of AudioDebuggingManager.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @kit AudioKit
 * @since 26.0.0
 */

#ifndef NATIVE_AUDIO_DEBUGGING_MANAGER_H
#define NATIVE_AUDIO_DEBUGGING_MANAGER_H

#include "native_audio_common.h"
#include "native_audiostream_base.h"
#include "native_audio_session_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Declare the audio debugging manager.
 * Audio debugging manager provides many functions for developer to get the
 * information about audio system runtime info.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioDebuggingManager OH_AudioDebuggingManager;

/**
 * @brief Gets the audio debugging manager handle, which is a singleton.
 *
 * @param manager The output parameter to get {@link OH_AudioDebuggingManager} instance.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.
 *     {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} The param of manager is nullptr.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioManager_GetAudioDebuggingManager(OH_AudioDebuggingManager **manager);

/**
 * @brief Prints full audio runtime snapshot for current app process.
 * The snapshot will contain all audio renderers, capturers, audio session information.
 * Note that the information details and format may vary from different version, it can only be used for
 * manual debugging, user should not rely on the information for actual function realization or file
 * content extraction.
 *
 * @param manager {@link OH_AudioDebuggingManager} handle provided by {@link OH_AudioManager_GetAudioDebuggingManager}.
 * @param fd is a file descriptor, indicates the location that the snapshot information will be written to.
 *     If the fd is less than 0 or no writable, the snapshot information will be printed into the running log,
 *     otherwise the snapshot will be written into the file.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.
 *     {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} The param of manager is nullptr.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDebuggingManager_PrintAppInfo(OH_AudioDebuggingManager *manager, int32_t fd);

/**
 * @brief Prints full audio runtime snapshot for target audio renderer instance.
 * The snapshot will contain the stream, pipe, volume and device information.
 * Note that the information details and format may vary from different version, it can only be used for
 * manual debugging, user should not rely on the information for actual function realization or file
 * content extraction.
 *
 * @param manager {@link OH_AudioDebuggingManager} handle provided by {@link OH_AudioManager_GetAudioDebuggingManager}.
 * @param renderer Pointer to the target audio renderer instance to print snapshot.
 * @param fd is a file descriptor, indicates the location that the snapshot information will be written to.
 *     If the fd is less than 0 or no writable, the snapshot information will be printed into the running log,
 *     otherwise the snapshot will be written into the file.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.
 *     {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} 1.The param of manager is nullptr;
 *                                                    2.The param of renderer is nullptr;
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDebuggingManager_PrintRendererInfo(
    OH_AudioDebuggingManager *manager, OH_AudioRenderer *renderer, int32_t fd);

/**
 * @brief Prints full audio runtime snapshot for target audio capturer instance.
 * The snapshot will contain the stream, pipe, volume and device information.
 * Note that the information details and format may vary from different version, it can only be used for
 * manual debugging, user should not rely on the information for actual function realization or file
 * content extraction.
 *
 * @param manager {@link OH_AudioDebuggingManager} handle provided by {@link OH_AudioManager_GetAudioDebuggingManager}.
 * @param capturer Pointer to the target audio capturer instance to print snapshot.
 * @param fd is a file descriptor, indicates the location that the snapshot information will be written to.
 *     If the fd is less than 0 or no writable, the snapshot information will be printed into the running log,
 *     otherwise the snapshot will be written into the file.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.
 *     {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} 1.The param of manager is nullptr;
 *                                                    2.The param of capturer is nullptr;
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDebuggingManager_PrintCapturerInfo(
    OH_AudioDebuggingManager *manager, OH_AudioCapturer *capturer, int32_t fd);

/**
 * @brief Prints full audio runtime snapshot for target audio session manager instance.
 * The snapshot will contain the session status, scene, strategy and device information.
 * Note that the information details and format may vary from different version, it can only be used for
 * manual debugging, user should not rely on the information for actual function realization or file
 * content extraction.
 *
 * @param manager {@link OH_AudioDebuggingManager} handle provided by {@link OH_AudioManager_GetAudioDebuggingManager}.
 * @param session Pointer to the target audio session manager instance to print snapshot.
 * @param fd is a file descriptor, indicates the location that the snapshot information will be written to.
 *     If the fd is less than 0 or no writable, the snapshot information will be printed into the running log,
 *     otherwise the snapshot will be written into the file.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.
 *     {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} 1.The param of manager is nullptr;
 *                                                    2.The param of session is nullptr;
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDebuggingManager_PrintSessionInfo(
    OH_AudioDebuggingManager *manager, OH_AudioSessionManager *session, int32_t fd);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AUDIO_DEBUGGING_MANAGER_H
/** @} */
