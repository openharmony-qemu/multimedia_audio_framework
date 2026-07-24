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
 * @addtogroup AudioAccessoryManager
 * @{
 *
 * @brief Provide audio accessory manager C interface.
 *
 * @since 26.0.0
 */

/**
 * @file native_audio_accessory_manager.h
 *
 * @brief Declare audio accessory manager related interfaces.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @kit AudioKit
 * @since 26.0.0
 */

#ifndef NATIVE_AUDIO_ACCESSORY_MANAGER_H
#define NATIVE_AUDIO_ACCESSORY_MANAGER_H

#include <stdbool.h>
#include "native_audio_accessory_input_stream_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback for noise reduction mode change on an accessory.
 *
 * <b>When Called:</b> When the system requests a change to the noise
 * reduction mode on the accessory. This callback may be called at any time
 * after the accessory is connected.
 *
 * @param accessory [in] The audio accessory.
 * @param mode [in] The noise reduction mode to set on the accessory.
 * @return <ul>
 *         <li>`true` if the requested mode is handled successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessory_SetNoiseReductionCallback)(
    OH_AudioAccessory *accessory, OH_AudioNoiseReductionMode mode);

/**
 * @brief Obtains the audio accessory manager instance.
 *
 * @param outManager [out] Returns a pointer to the manager handle.
 *     Note that the handle is managed by the system and must not be released
 *     by the caller, otherwise an exception may occur.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if manager is null.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioManager_GetAccessoryManager(
    OH_AudioAccessoryManager **outManager);

/**
 * @brief Creates an input audio accessory instance and registers its capabilities.
 *
 * This function creates only the audio accessory instance. It does not create
 * any input stream immediately.
 *
 * The framework performs a deep copy of the accessoryName, manufacturer,
 * modelNumber, and macAddress fields. The caller may free these buffers
 * after this function returns.
 * The framework also performs a deep copy of the streamProperties array
 * in capabilities. The caller may free this array after this function returns.
 *
 * On success, the framework allocates an {@link OH_AudioAccessory} handle and
 * returns it through accessory pointer.
 *
 * Input streams are created lazily by the framework when an application
 * actually starts recording from this accessory. At that time, the framework
 * creates a new {@link OH_AudioAccessoryInputStream} handle and invokes
 * open stream. The callback receives the newly created stream handle
 * and the requested stream information, and is where the caller must register
 * the required stream callbacks.
 *
 * The stream handle is managed by the framework and must not be released by
 * the caller. A stream remains valid until the framework invokes
 * {@link OH_AudioAccessoryInputStream_ReleaseCallback} for that stream. After
 * the release callback returns, the stream handle becomes invalid and must not
 * be used again. During the lifetime of one accessory handle, input streams
 * may be created and released multiple times.
 *
 * @param manager [in] Pointer to the audio accessory manager.
 * @param info [in] Pointer to the accessory basic information. Must not be null.
 * @param capabilities [in] Pointer to the accessory capabilities. Must not be null.
 * @param openInputStream [in] Callback invoked when the framework opens an input stream.
 *     Must not be null. The callback is invoked only when the framework creates
 *     a stream for this accessory, not when this function is called.
 * @param outOwnedAccessory [out] Returns the created accessory handle.
 * @release Call {@link OH_AudioAccessoryManager_Disconnected} to disconnect
 *     the accessory first if it has been connected, and then call
 *     {@link OH_AudioAccessoryManager_Destroy} to destroy the accessory handle.
 *     After a successful destroy, the accessory handle becomes invalid and must
 *     not be used again.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if any parameter is null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the manager is not initialized.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_CreateInput(
    OH_AudioAccessoryManager *manager,
    const OH_AudioAccessoryInfo *info,
    const OH_AudioAccessoryCapabilities *capabilities,
    OH_AudioAccessory_OpenInputStreamCallback openInputStream,
    OH_AudioAccessory **outOwnedAccessory);

/**
 * @brief Sets the list of associated MAC addresses for the audio accessory.
 *
 * This interface replaces the existing list of associated MAC addresses
 * linked to the accessory instance. It is designed for multi-transmitter
 * scenarios (e.g., 1-to-2, 1-to-4 systems) where the group of connected
 * transmitters may change dynamically.Call this after the accessory is created
 * to report all currently active transmitters associated with the primary MAC.
 * If a transmitter is replaced or disconnected, call this again with the
 * updated list to overwrite the previous state. Safe to call during an active
 * recording session.
 *
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle.
 * @param macAddresses [in] Array of MAC addresses to associate.
 *     <b>Can be null if count is 0</b>, indicating that all associated MAC addresses
 *     should be cleared (e.g., when all secondary transmitters disconnect).
 *     If not null, the framework performs a deep copy of these strings.
 * @param count [in] Number of MAC addresses in the array.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are invalid.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the accessory is not created.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_SetAssociatedMacAddresses(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory,
    const char **macAddresses,
    uint32_t count);

/**
 * @brief Registers the noise reduction capability of an audio accessory.
 *
 * The framework performs a deep copy of the supportedModes array and other
 * fields in the capability structure. The caller may free the capability
 * structure and the supportedModes array after this function returns.
 *
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle created by CreateInput.
 * @param capability [in] Pointer to the noise reduction capability. Must not be null.
 * @param onNoiseReduction [in] Callback invoked when the framework
 *     requests a noise reduction mode change. May be null if the accessory
 *     does not support dynamic mode switching. If provided, the callback must
 *     return `true` on success and `false` on failure.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are invalid.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the accessory is not created.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_RegisterNoiseReductionCapability(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory,
    const OH_AudioAccessoryNoiseReductionCapability *capability,
    OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction);

/**
 * @brief Sets the noise reduction mode of an audio accessory.
 *
 * This function allows the accessory service to actively synchronize the
 * current noise reduction mode to the framework. It is typically used when
 * the mode is changed through other means (e.g., hardware buttons or a
 * companion app), ensuring the framework stays updated with the accessory's
 * actual state.
 *
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle.
 * @param mode [in] The noise reduction mode to set. Must be one of the modes
 *     registered via RegisterNoiseReductionCapability.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are invalid.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the accessory is not connected.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED} if the mode is not supported.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_SetNoiseReductionMode(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory,
    OH_AudioNoiseReductionMode mode);

/**
 * @brief Connects the audio accessory to the audio framework.
 *
 * All required capabilities must be registered before calling this function.
 *
 * <b>Recommendation:</b> It is recommended that third-party audio accessories
 * prioritize integration with the Smart Life app. This ensures a consistent
 * user experience for device discovery and connection, allowing the accessory
 * service to avoid direct permission management.
 *
 * @permission ohos.permission.MANAGE_AUDIO_ACCESSORY
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle to connect.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED} if the caller does not have the
 *                  required permission.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if accessory is null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if capabilities are not registered or
 *                  the accessory is already connected.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} if audio server process die.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_Connected(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory);

/**
 * @brief Disconnects the audio accessory from the audio framework.
 *
 * @permission ohos.permission.MANAGE_AUDIO_ACCESSORY
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle to disconnect.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED} if the caller does not have the
 *                  required permission.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if accessory is null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the accessory is not connected.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} if audio server process die.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_Disconnected(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory);

/**
 * @brief Destroys the audio accessory instance.
 *
 * The accessory must be disconnected before destroying.
 *
 * @param manager [in] Pointer to the audio accessory manager.
 * @param accessory [in] Pointer to the accessory handle to destroy.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if accessory is null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the accessory is still connected.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryManager_Destroy(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // NATIVE_AUDIO_ACCESSORY_MANAGER_H
