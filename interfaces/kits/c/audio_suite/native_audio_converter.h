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
 * @addtogroup AudioConverter
 * @{
 *
 * @brief Provide the definition of the C interface for the audio module.
 *
 * @since 26.0.0
 * @version 1.0
 */
/**
 * @file native_audio_converter.h
 *
 * @brief Declare underlying data structure.
 *
 * @library libohaudiosuite.so
 * @syscap SystemCapability.Multimedia.Audio.SuiteEngine
 * @kit AudioKit
 * @since 26.0.0
 * @version 1.0
 */
#ifndef NATIVE_AUDIO_CONVERTER_H
#define NATIVE_AUDIO_CONVERTER_H
#include <stdint.h>
#include "multimedia/native_audio_channel_layout.h"
#include "native_audio_suite_base.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Define the audio converter format info structure, used to describe basic audio format.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioConverter_Format {
    /**
     * @brief Audio encoding format type.
     *
     * @since 26.0.0
     */
    OH_Audio_EncodingType encodingType;
    /**
     * @brief Audio sampling rate.
     *
     * @since 26.0.0
     */
    OH_Audio_SampleRate samplingRate;
    /**
     * @brief Audio channel layout.
     *
     * @since 26.0.0
     */
    OH_AudioChannelLayout channelLayout;
    /**
     * @brief Audio sample format.
     *
     * @since 26.0.0
     */
    OH_Audio_SampleFormat sampleFormat;
} OH_AudioConverter_Format;

/**
 * @brief Declare the audio converter.
 * The handle of audio converter is used for audio converter related functions.
 *
 * @since 26.0.0
 */
typedef struct OH_AudioConverterStruct OH_AudioConverter;

/**
 * @brief Define the result of the function execution.
 *
 * @since 26.0.0
 */
typedef enum {
    /**
     * @error The call was successful.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_SUCCESS = 0,
    /**
     * @error This means that the function was executed with an invalid input parameter.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_INVALID_PARAM = 1,
    /**
     * @error Unsupported audio format, such as unsupported encoding type, sample format etc.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT = 2,
    /**
     * @error An system error has occurred.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_SYSTEM = 3,
    /**
     * @error Memory allocation failed.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED = 4,
    /**
     * @error Buffer capacity is insufficient.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL = 5,
    /**
     * @error Audio converter instance is not initialized.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_NOT_INITIALIZED = 6,
    /**
     * @error Callback is invalid.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_CALLBACK_INVALID = 7,
    /**
     * @error Callback is not set.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET = 8,
} OH_AudioConverter_Result;

/**
 * @brief Request to create the audio converter.
 *
 * The converter instance created by this function must be explicitly destroyed via OH_AudioConverter_Destroy().
 * Supported audio format specifications (valid for Input/Output)
 * The converter only supports PCM (Pulse Code Modulation) audio formats.
 * Sample rate supports 8000 Hz, 11025 Hz, 12000 Hz, 16000 Hz, 22050 Hz, 24000 Hz, 32000 Hz, 44100 Hz, 48000 Hz,
 * 64000 Hz, 88200 Hz, 96000 Hz, 176400 Hz and 192000 Hz.
 * Channel layout supports CH_LAYOUT_MONO, CH_LAYOUT_STEREO, CH_LAYOUT_STEREO_DOWNMIX,
 * CH_LAYOUT_2POINT1, CH_LAYOUT_3POINT0, CH_LAYOUT_SURROUND, CH_LAYOUT_3POINT1, CH_LAYOUT_4POINT0, CH_LAYOUT_QUAD_SIDE,
 * CH_LAYOUT_QUAD, CH_LAYOUT_2POINT0POINT2, CH_LAYOUT_4POINT1, CH_LAYOUT_5POINT0, CH_LAYOUT_5POINT0_BACK,
 * CH_LAYOUT_2POINT1POINT2, CH_LAYOUT_3POINT0POINT2, CH_LAYOUT_5POINT1, CH_LAYOUT_5POINT1_BACK, CH_LAYOUT_6POINT0,
 * CH_LAYOUT_3POINT1POINT2, CH_LAYOUT_6POINT0_FRONT, CH_LAYOUT_HEXAGONAL, CH_LAYOUT_6POINT1, CH_LAYOUT_6POINT1_BACK,
 * CH_LAYOUT_6POINT1_FRONT, CH_LAYOUT_7POINT0, CH_LAYOUT_7POINT0_FRONT, CH_LAYOUT_7POINT1, CH_LAYOUT_OCTAGONAL,
 * CH_LAYOUT_5POINT1POINT2, CH_LAYOUT_7POINT1_WIDE and CH_LAYOUT_7POINT1_WIDE_BACK.
 * Sample format (bit depth) supports SAMPLE_U8 (8-bit unsigned PCM),
 * SAMPLE_S16LE (16-bit short little-endian PCM), SAMPLE_S24LE (24-bit short little-endian PCM),
 * SAMPLE_S32LE (32-bit short little-endian PCM), and SAMPLE_F32LE (32-bit float little-endian PCM).
 *
 * @param inputFormat Pointer to the input audio format configuration.
 * @param outputFormat Pointer to the output audio format configuration.
 * @param converter Pointer to a variable to receive audio converter.
 * @return {@link #AUDIOCONVERTER_SUCCESS} if execution succeeds,
 * or {@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} If the input parameters are invalid.
 * or {@link #AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT} if the specified input/output format combination is unsupported.
 * or {@link #AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED} if memory allocation failed.
 * or {@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities.
 * @since 26.0.0
 */
OH_AudioConverter_Result OH_AudioConverter_Create(
    const OH_AudioConverter_Format* inputFormat,
    const OH_AudioConverter_Format* outputFormat,
    OH_AudioConverter** converter);

/**
 * @brief Request to release the converter.
 *
 * @param converter Reference created by OH_AudioConverter_Create.
 * @return {@link #AUDIOCONVERTER_SUCCESS} if execution succeeds,
 * or {@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if converter is nullptr,
 * or {@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities.
 * @since 26.0.0
 */
void OH_AudioConverter_Destroy(OH_AudioConverter* converter);


/**
 * @brief Define the status of input audio data provided by the callback (OH_AudioConverter_RequestDataCallback).
 * The converter uses this status to determine how to handle subsequent conversion logic (e.g., continue pulling data,
 * pause, or flush cached data).
 * Note for callers: Even if the callback returns AUDIOCONVERTER_INPUT_DATA_FINISHED, OH_AudioConverter_Process()
 * must be called repeatedly
 * until it returns AUDIOCONVERTER_SUCCESS with outputSize=0 (indicating all cached data has been flushed)
 *
 * @since 26.0.0
 */
typedef enum {
    /**
     * Valid input data is available and provided.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_INPUT_HAVE_DATA = 1,
    /**
     * No input data is available temporarily.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA = 2,
    /**
     * The input data stream is complete.
     *
     * @since 26.0.0
     */
    AUDIOCONVERTER_INPUT_DATA_FINISHED = 3,
} OH_AudioConverter_InputStatus;

/**
 * @brief Callback function of request data.
 *
 * The converter invokes this callback to actively request input audio data during OH_AudioConverter_Process().
 * The caller must populate the output parameters (outInputData, outStatus) and return the valid size of input data.
  * The maximum data size returned by a single callback is 400KB.
 * The memory pointed to by outInputData must remain valid until OH_AudioConverter_Process() returns.
 * @param userData User-defined data passed to the callback.
 * @param outInputData Pointer to a pointer that the callback sets to point to the input audio data buffer.
 * @param outStatus Set by the callback to inform the converter of data availability.
 * @return Size of valid input data pointed to by outInputData.
 * @since 26.0.0
 */
typedef int32_t (*OH_AudioConverter_RequestDataCallback)(
    void* userData,
    const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus
);

/**
 * @brief Set converter request data callback.
 *
 * This function binds the input data callback function for the audio converter.
 * The callback is used by OH_AudioConverter_Process() to pull input audio data dynamically.
 *
 * @param converter Reference created by OH_AudioConverter_Create.
 * @param callback Callback to functions that will write audio data.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOCONVERTER_SUCCESS} if execution succeeds
 * or {@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if parameter is invalid, e.g. converter is nullptr, e.t.c.
 * or {@link #AUDIOCONVERTER_ERROR_NOT_INITIALIZED} if converter instance is not initialized.
 * or {@link #AUDIOCONVERTER_ERROR_CALLBACK_INVALID} if callback is invalid, e.g. invalid callback return, e.t.c.
 * or {@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities.
 * @since 26.0.0
 */
OH_AudioConverter_Result OH_AudioConverter_SetInputCallback(
    OH_AudioConverter* converter,
    OH_AudioConverter_RequestDataCallback callback,
    void* userData
);

/**
 * @brief Executing the audio format conversion.
 *
 * This function executes audio conversion to converts to the target format, and writes the result to the user-provided
 * output buffer. This function must be called after OH_AudioConverter_SetInputCallback.
 * The output buffer must be allocated and managed by the caller.
 *
 * @param converter Reference created by OH_AudioConverter_Create.
 * @param outputData Pointer to the output buffer allocated by the caller.
 * @param outputCapacity Size of the output buffer user specified.
 * @param outputSize Size of output buffer the system really write.
 * @return {@link #AUDIOCONVERTER_SUCCESS} if execution succeeds
 * or {@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if parameter is invalid, e.g. converter is nullptr, e.t.c.
 * or {@link #AUDIOCONVERTER_ERROR_NOT_INITIALIZED} if converter instance is not initialized.
 * or {@link #AUDIOCONVERTER_ERROR_CALLBACK_INVALID} if callback is invalid,
 * e.g. callback returned an invalid value, e.t.c.
 * or {@link #AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET} no input callback is bound to the converter.
 * or {@link #AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL} output buffer capacity is insufficient.
 * or {@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities.
 * @since 26.0.0
 */
OH_AudioConverter_Result OH_AudioConverter_Process(
    OH_AudioConverter* converter,
    void* outputData,
    int32_t outputCapacity,
    int32_t* outputSize
);

#ifdef __cplusplus
}
#endif
/** @} */
#endif // NATIVE_AUDIO_CONVERTER_H