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
 * @syscap SystemCapability.Multimedia.Audio.Core
 *
 * @since 26.0.0
 * @version 1.0
 */

/**
 * @file native_audio_routing_enhance_manager.h
 *
 * @brief Declares audio device enhance manager related interfaces.
 *
 * The interfaces in this file are used for obtaining the OH_AudioDeviceEnhanceManager handle,
 * switching the input or output devices of your application itself,
 * as well as other enhanced functions related to audio devices or routing.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @kit AudioKit
 * @since 26.0.0
 * @version 1.0
 */

#ifndef NATIVE_AUDIO_DEVICE_ENHANCE_MANAGER_H
#define NATIVE_AUDIO_DEVICE_ENHANCE_MANAGER_H

#include "native_audio_device_base.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the handle type of the audio device enhance manager,
 * which is used for enhanced audio device management functions.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioDeviceEnhanceManager OH_AudioDeviceEnhanceManager;

/**
 * @brief Obtains the audio device enhance manager handle, which is used as
 * the first parameter in enhanced audio device management functions.
 * The functional APIs of this manager are only available on specific devices.
 * Your application can first call {@link OH_AudioDeviceEnhanceManager_IsSupported} to check
 * if the system supports them before using.
 *
 * @param audioDeviceEnhanceManager Indicates the pointer to the {@link OH_AudioDeviceEnhanceManager}
 *     handle obtained by this function.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} If the execution is successful,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} if system state error.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioManager_GetAudioDeviceEnhanceManager(
    OH_AudioDeviceEnhanceManager **audioDeviceEnhanceManager);

/**
 * @brief Queries whether the system supports the enhanced routing functions provided by this manager,
 * including selecting input and output devices for the application or audio streams.
 * Your application is advised to call this API first to confirm system support before using
 * these enhanced routing APIs. Even for the same type of host device, some models may support
 * these functions while others may not due to hardware limitations. If the system does not support
 * these enhanced routing functions, calling them will have no effect, and the system will select
 * default input/output devices for the application or audio streams instead.
 *
 * @param audioDeviceEnhanceManager the {@link OH_AudioDeviceEnhanceManager} handle returned
 *     by {@link OH_AudioManager_GetAudioDeviceEnhanceManager}.
 * @param supported query result, true means the system supports the enhanced functions,
 *     false means not supported.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} Audio client call audio service error, System error.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, bool *supported);

/**
 * @brief Selects the output device for your application. This setting applies to
 * all playback streams created under your application, unless a specific output device
 * is designated for a particular stream. When application implements its own UX for
 * output device selection, it can obtain the list of available output devices through
 * {@link OH_AudioRoutingManager_GetAvailableDevices}, and use the
 * {@link OH_AudioRoutingManager_GetPreferredOutputDevice} API to obtain the currently selected output device.
 * Your application can use {@link OH_AudioDeviceEnhanceManager_RegisterCurrentOutputDeviceChangeCallback}
 * to register a callback to listen for the actual output device. The selection will become invalid when
 * your application exits or the selected device goes offline. After your application restarts or
 * the device comes back online, your application must re-issue the selection for it to take effect.
 * If the system does not support this function, it will select a default output device for your application.
 *
 * @param audioDeviceEnhanceManager the {@link OH_AudioDeviceEnhanceManager} handle returned
 *     by {@link OH_AudioManager_GetAudioDeviceEnhanceManager}.
 * @param deviceDescriptor The target device. The available device must be in the array returned
 *     by {@link OH_AudioRoutingManager_GetAvailableDevices}.
 *     When nullptr is passed, system will clear the last selection and select a default
 *     output device for your application.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} Audio client call audio service error, System error,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED} if the system does not support this function.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectOutputDevice(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioDeviceDescriptor *deviceDescriptor);

/**
 * @brief Selects the input device for your application. This setting applies to
 * all recording streams created under your application, unless a specific input device
 * is designated for a particular stream. When application implements its own UX for
 * input device selection, it can obtain the list of available input devices through
 * {@link OH_AudioRoutingManager_GetAvailableDevices}, and use the
 * {@link OH_AudioRoutingManager_GetPreferredInputDevice} API to obtain the currently selected input device.
 * Your application can use {@link OH_AudioDeviceEnhanceManager_RegisterCurrentInputDeviceChangeCallback}
 * to register a callback to listen for the actual input device. The selection will become invalid when
 * your application exits or the selected device goes offline. After your application restarts or
 * the device comes back online, your application must re-issue the selection for it to take effect.
 * If the system does not support this function, it will select a default input device for your application.
 *
 * @param audioDeviceEnhanceManager the {@link OH_AudioDeviceEnhanceManager} handle returned
 *     by {@link OH_AudioManager_GetAudioDeviceEnhanceManager}.
 * @param deviceDescriptor The target device. The available device must be in the array returned
 *     by {@link OH_AudioRoutingManager_GetAvailableDevices}.
 *     When nullptr is passed, system will clear the last selection and select a default
 *     input device for your application.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} Audio client call audio service error, System error,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED} if the system does not support this function.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectInputDevice(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioDeviceDescriptor *deviceDescriptor);

/**
 * @brief Selects the output device for the target renderer. Your application must ensure that the specified
 * renderer is valid. This selection only applies to the designated stream; other playback streams in
 * your application will use application's forced selection or the system's default output device.
 * The selection will become invalid when your application exits or the selected device goes offline.
 * After your application restarts or the device comes back online, your application must re-issue the
 * selection for it to take effect. If the system does not support this function, it will select a default
 * output device for the renderer.
 *
 * @param audioDeviceEnhanceManager the {@link OH_AudioDeviceEnhanceManager} handle returned
 *     by {@link OH_AudioManager_GetAudioDeviceEnhanceManager}.
 * @param renderer Indicates the renderer reference created by {@link OH_AudioStreamBuilder_GenerateRenderer}.
 * @param deviceDescriptor The target device. The available device must be in the array returned
 *     by {@link OH_AudioRoutingManager_GetAvailableDevices}.
 *     When nullptr is passed, system will clear the last selection and select a default
 *     output device for the renderer.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} Audio client call audio service error, System error,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED} if the system does not support this function.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioRenderer *renderer,
    OH_AudioDeviceDescriptor *deviceDescriptor);

/**
 * @brief Selects the input device for the target capturer. Your application must ensure that the specified
 * capturer is valid. This selection only applies to the designated stream; other recording streams in
 * your application will use application's forced selection or the system's default input device.
 * The selection will become invalid when your application exits or the selected device goes offline.
 * After your application restarts or the device comes back online, your application must re-issue the
 * selection for it to take effect. If the system does not support this function, it will select a default
 * input device for the capturer.
 *
 * @param audioDeviceEnhanceManager the {@link OH_AudioDeviceEnhanceManager} handle returned
 *     by {@link OH_AudioManager_GetAudioDeviceEnhanceManager}.
 * @param capturer Indicates the capturer reference created by {@link OH_AudioStreamBuilder_GenerateCapturer}.
 * @param deviceDescriptor The target device. The available device must be in the array returned
 *     by {@link OH_AudioRoutingManager_GetAvailableDevices}.
 *     When nullptr is passed, system will clear the last selection and select a default
 *     input device for the capturer.
 * @return {@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameter validation fails,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} Audio client call audio service error, System error,
 *     or {@link AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED} if the system does not support this function.
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioCapturer *capturer,
    OH_AudioDeviceDescriptor *deviceDescriptor);
#ifdef __cplusplus
}
#endif

#endif // NATIVE_AUDIO_DEVICE_ENHANCE_MANAGER_H
/** @} */
