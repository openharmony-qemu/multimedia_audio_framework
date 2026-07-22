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
 * @addtogroup AudioSuite
 * @{
 *
 * @brief Provide the definition of the C interface for the audio download manager module.
 *
 * @since 26.0.0
 */
/**
 * @file native_audio_suite_download_manager.h
 * @brief Declare audio download manager related interfaces.
 * @syscap SystemCapability.Multimedia.Audio.SuiteEngine
 * @library libohaudiosuite.so
 * @kit AudioKit
 * @since 26.0.0
 */
#ifndef NATIVE_AUDIO_SUITE_DOWNLOAD_MANAGER_H
#define NATIVE_AUDIO_SUITE_DOWNLOAD_MANAGER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Define download status information structure.
 * @systemapi
 * @since 26.0.0
 */
typedef struct OH_AudioSuite_DownloadStatusInfo {
    /**
     * @brief feature name.
     *
     * @since 26.0.0
     */
    char featureName[256];
    /**
     * @brief Download status.
     *
     * @since 26.0.0
     */
    int32_t downloadStatus;
    /**
     * @brief feature size.
     *
     * @since 26.0.0
     */
    int64_t size;
    /**
     * @brief Installation path.
     *
     * @since 26.0.0
     */
    char installPath[256];
    /**
     * @brief Download progress [0-100].
     *
     * @since 26.0.0
     */
    int32_t progress;
    /**
     * @brief Error code.
     *
     * @since 26.0.0
     */
    int32_t errorCode;
} OH_AudioSuite_DownloadStatusInfo;

/**
 * @brief Define download status information array structure.
 * @systemapi
 * @since 26.0.0
 */
typedef struct OH_AudioSuite_DownloadStatusInfoArray {
    /**
     * @brief Array size.
     *
     * @since 26.0.0
     */
    uint32_t size;
    /**
     * @brief Array of download status info pointers.
     *
     * @since 26.0.0
     */
    OH_AudioSuite_DownloadStatusInfo **downloadStatusInfo;
} OH_AudioSuite_DownloadStatusInfoArray;

/**
 * @brief Declare the audio download manager.
 * The handle of audio download manager is used for download related functions.
 * @systemapi
 * @since 26.0.0
 */
typedef struct OH_AudioSuite_DownloadManagerStruct OH_AudioSuite_DownloadManager;

/**
 * @brief Callback function for download status update.
 * @systemapi
 * @param downloadStatusInfoArray Pointer to array of download status information.
 * @since 26.0.0
 */
typedef void (*OH_AudioSuite_DownloadCallback)(OH_AudioSuite_DownloadStatusInfoArray *downloadStatusInfoArray);

/**
 * @brief Get the audio download manager handle.
 * @systemapi
 * @param downloadManager Pointer to receive the download manager handle.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager is nullptr.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_GetDownloadManager(OH_AudioSuite_DownloadManager **downloadManager);

/**
 * @brief Register download status callback.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param callback Callback function to receive download status updates.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or callback is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_RegisterDownloadCallback(
    OH_AudioSuite_DownloadManager *downloadManager, const OH_AudioSuite_DownloadCallback *callback);

/**
 * @brief Unregister download status callback.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param callback Callback function to unregister.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or callback is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_UnregisterDownloadCallback(
    OH_AudioSuite_DownloadManager *downloadManager, const OH_AudioSuite_DownloadCallback *callback);

/**
 * @brief Start downloading a feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature to download.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or featureName is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_StartDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName);

/**
 * @brief Pause downloading a feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature to pause.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or featureName is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_PauseDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName);

/**
 * @brief Cancel downloading a feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature to cancel.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or featureName is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} If the current state does not allow cancellation.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_CancelDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName);

/**
 * @brief Start background downloading a feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature to download.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or featureName is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_StartBackgroundDownload(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName);

/**
 * @brief Get download status of a feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature.
 * @param status Pointer to receive download status information.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager, featureName or status is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_GetDownloadStatus(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName,
    OH_AudioSuite_DownloadStatusInfo *status);

/**
 * @brief Uninstall a downloaded feature.
 * @systemapi
 * @param downloadManager Download manager handle.
 * @param featureName Name of the feature to uninstall.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful.</li>
 *         <li>202 if a non-system application calls this system API.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} If downloadManager or featureName is nullptr.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} If IPC communication fails or the operation fails.</li>
 *         </ul>
 * @since 26.0.0
 */
int32_t OH_AudioSuite_UninstallCloudRom(
    OH_AudioSuite_DownloadManager *downloadManager, const char *featureName);

#ifdef __cplusplus
}
#endif
/** @} */
#endif // NATIVE_AUDIO_SUITE_DOWNLOAD_MANAGER_H
