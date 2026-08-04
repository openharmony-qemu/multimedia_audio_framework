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
 * @brief Provide common types for audio accessory device C interface.
 *
 * @since 26.0.0
 */

/**
 * @file native_audio_accessory_common.h
 *
 * @brief Declare common types for external audio accessory device interfaces.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @kit AudioKit
 * @since 26.0.0
 */

#ifndef NATIVE_AUDIO_ACCESSORY_COMMON_H
#define NATIVE_AUDIO_ACCESSORY_COMMON_H

#include <stdbool.h>

#include "native_audio_common.h"
#include "native_audiostream_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Declare the audio accessory manager.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioAccessoryManager OH_AudioAccessoryManager;

/**
 * @brief Declare the audio accessory.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioAccessory OH_AudioAccessory;

/**
 * @brief Declare the audio accessory input stream.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioAccessoryInputStream OH_AudioAccessoryInputStream;

/**
 * @brief Enumerates audio accessory connection types.
 *
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief Bluetooth SPP (Signal Processing Plugin) connection.
     *
     * @since 26.0.0
     */
    AUDIO_ACCESSORY_TYPE_BT_SPP = 1,
} OH_AudioAccessoryType;

/**
 * @brief Defines the basic information of an audio accessory.
 *
 * <b>Version Control:</b> Callers MUST set structSize to sizeof(OH_AudioAccessoryInfo)
 * before passing this structure to the framework.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Size of this structure in bytes.
     * Must be initialized by the caller (e.g., info.structSize = sizeof(OH_AudioAccessoryInfo)).
     * The framework uses this to determine which version of the structure is being used.
     *
     * @since 26.0.0
     */
    uint32_t structSize;

    /**
     * @brief Accessory name for UX display, such as "DJI Mic 2".
     * The framework performs a deep copy of this field.
     *
     * @since 26.0.0
     */
    const char *accessoryName;

    /**
     * @brief Manufacturer name, such as "DJI".
     * The framework performs a deep copy of this field.
     *
     * @since 26.0.0
     */
    const char *manufacturer;

    /**
     * @brief Model number, such as "CP236".
     * The framework performs a deep copy of this field.
     *
     * @since 26.0.0
     */
    const char *modelNumber;

    /**
     * @brief MAC address of the accessory, such as "00:11:22:33:44:55".
     * The framework performs a deep copy of this field.
     *
     * @since 26.0.0
     */
    const char *macAddress;

    /**
     * @brief Accessory connection type.
     *
     * @since 26.0.0
     */
    OH_AudioAccessoryType type;

    /**
     * @brief Indicates whether the accessory is a unidirectional audio device.
     * true: unidirectional device; false: bidirectional device.
     *
     * @since 26.0.0
     */
    bool isUnidirectional;
} OH_AudioAccessoryInfo;

/**
 * @brief Defines the noise reduction capability of an audio accessory.
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Size of this structure in bytes.
     * Must be initialized by the caller
     * (e.g., info.structSize = sizeof(OH_AudioAccessoryNoiseReductionCapability)).
     * The framework uses this to determine which version of the structure is being used.
     *
     * @since 26.0.0
     */
    uint32_t structSize;
    /**
     * @brief Array of supported noise reduction modes.
     *
     * @since 26.0.0
     */
    const OH_AudioNoiseReductionMode *supportedModes;

    /**
     * @brief Number of supported noise reduction modes.
     *
     * @since 26.0.0
     */
    uint32_t supportedModeCount;

    /**
     * @brief The current noise reduction mode of the device.
     * This represents the initial state when the capability is registered.
     *
     * @since 26.0.0
     */
    OH_AudioNoiseReductionMode currentMode;
} OH_AudioAccessoryNoiseReductionCapability;

/**
 * @brief Defines the capabilities of an audio accessory.
 *
 * <b>Version Control:</b> Callers MUST set structSize to sizeof(OH_AudioAccessoryCapabilities).
 *
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Size of this structure in bytes.
     * Must be initialized by the caller (e.g., caps.structSize = sizeof(OH_AudioAccessoryCapabilities)).
     *
     * @since 26.0.0
     */
    uint32_t structSize;

    /**
     * @brief Array of supported stream configurations.
     * Each entry represents one valid combination of sample rate,
     * format, and channel count.
     * The framework performs a deep copy of this array.
     *
     * @since 26.0.0
     */
    const OH_AudioStreamInfo *streamProperties;

    /**
     * @brief Number of supported stream configurations.
     *
     * @since 26.0.0
     */
    uint32_t streamPropertyCount;
} OH_AudioAccessoryCapabilities;

#ifdef __cplusplus
}
#endif

/** @} */
#endif // NATIVE_AUDIO_ACCESSORY_COMMON_H
