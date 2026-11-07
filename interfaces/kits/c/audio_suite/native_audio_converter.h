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
 * @addtogroup OHAudioSuite
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
 * @brief Define the extended audio converter format info structure, used to describe an audio format that supports
 * arbitrary sampling rates.
 *
 * @since 26.1.0
 */
typedef struct OH_AudioSuite_ConverterFormatEx {
    /**
     * @brief Audio encoding format type.
     *
     * @since 26.1.0
     */
    OH_Audio_EncodingType encodingType;
    /**
     * @brief Audio sampling rate in Hz.
     * Supported sampling rates are integer multiples of 10 Hz in the range [8000, 5644800], plus 11025 Hz.
     *
     * @since 26.1.0
     */
    int32_t samplingRate;
    /**
     * @brief Audio channel layout.
     *
     * @since 26.1.0
     */
    OH_AudioChannelLayout channelLayout;
    /**
     * @brief Audio sample format.
     *
     * @since 26.1.0
     */
    OH_Audio_SampleFormat sampleFormat;
} OH_AudioSuite_ConverterFormatEx;

/**
 * @brief Define the input data structure of the audio converter.
 *
 * @since 26.1.0
 */
typedef struct OH_AudioSuite_ConverterInputInfo {
    /**
     * @brief Pointer to the input audio data buffer. It points to the raw audio data and is allocated by the caller.
     * The data type is determined by the sample format {@link OH_Audio_SampleFormat}.
     * It must remain valid during the {@link OH_AudioSuite_ConverterDirectProcess} call. The caller retains ownership.
     * It cannot be NULL.
     *
     * @since 26.1.0
     */
    const void *data;
    /**
     * @brief Capacity of the input buffer in bytes. It must be greater than 0.
     *
     * @since 26.1.0
     */
    uint32_t size;
} OH_AudioSuite_ConverterInputInfo;

/**
 * @brief Define the output data structure of the audio converter, used in direct processing mode to receive the
 * converted audio data.
 *
 * @since 26.1.0
 */
typedef struct OH_AudioSuite_ConverterOutputInfo {
    /**
     * @brief Pointer to the output audio data buffer, allocated by the caller.
     * The conversion result is written to this buffer.
     * The data type is determined by the sample format {@link OH_Audio_SampleFormat}.
     * It must remain valid during the {@link OH_AudioSuite_ConverterDirectProcess} call. The caller retains ownership.
     * It cannot be NULL.
     *
     * @since 26.1.0
     */
    void *data;
    /**
     * @brief Capacity of the output buffer in bytes. It must be greater than 0.
     *
     * @since 26.1.0
     */
    uint32_t size;
    /**
     * @brief Output parameter that returns the number of bytes actually written to the output buffer.
     * It does not need to be initialized before the call; it remains unchanged if the call fails.
     *
     * @since 26.1.0
     */
    uint32_t writeSize;
} OH_AudioSuite_ConverterOutputInfo;

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
     * @error A system error has occurred.
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
    /**
     * @brief Permission verification fails. A non-system application attempts to call a system API.
     * @since 26.1.0
     */
    AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP = 202,
    /**
     * @brief This means the operation is unsupported.
     * @since 26.1.0
     */
    AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION = 6800104,
} OH_AudioConverter_Result;

/**
 * @brief Request to create the audio converter.
 *
 * The converter instance created by this function must be explicitly destroyed via {@link OH_AudioConverter_Destroy}.
 * Supported audio format specifications (valid for Input/Output)
 * The converter only supports PCM (Pulse Code Modulation) audio formats.
 * Sample rate supports 8000 Hz, 11025 Hz, 12000 Hz, 16000 Hz, 22050 Hz, 24000 Hz, 32000 Hz, 44100 Hz, 48000 Hz,
 * 64000 Hz, 88200 Hz, 96000 Hz, 176400 Hz and 192000 Hz.
 * Channel layout supports {@link CH_LAYOUT_MONO}, {@link CH_LAYOUT_STEREO}, {@link CH_LAYOUT_STEREO_DOWNMIX},
 * {@link CH_LAYOUT_2POINT1}, {@link CH_LAYOUT_3POINT0}, {@link CH_LAYOUT_SURROUND}, {@link CH_LAYOUT_3POINT1},
 * {@link CH_LAYOUT_4POINT0}, {@link CH_LAYOUT_QUAD_SIDE}, {@link CH_LAYOUT_QUAD}, {@link CH_LAYOUT_2POINT0POINT2},
 * {@link CH_LAYOUT_4POINT1}, {@link CH_LAYOUT_5POINT0}, {@link CH_LAYOUT_5POINT0_BACK},
 * {@link CH_LAYOUT_2POINT1POINT2}, {@link CH_LAYOUT_3POINT0POINT2}, {@link CH_LAYOUT_5POINT1},
 * {@link CH_LAYOUT_5POINT1_BACK}, {@link CH_LAYOUT_6POINT0}, {@link CH_LAYOUT_3POINT1POINT2},
 * {@link CH_LAYOUT_6POINT0_FRONT}, {@link CH_LAYOUT_HEXAGONAL}, {@link CH_LAYOUT_6POINT1},
 * {@link CH_LAYOUT_6POINT1_BACK}, {@link CH_LAYOUT_6POINT1_FRONT}, {@link CH_LAYOUT_7POINT0},
 * {@link CH_LAYOUT_7POINT0_FRONT}, {@link CH_LAYOUT_7POINT1}, {@link CH_LAYOUT_OCTAGONAL},
 * {@link CH_LAYOUT_5POINT1POINT2}, {@link CH_LAYOUT_7POINT1_WIDE} and {@link CH_LAYOUT_7POINT1_WIDE_BACK}.
 * Sample format (bit depth) supports SAMPLE_U8 (8-bit unsigned PCM),
 * SAMPLE_S16LE (16-bit short little-endian PCM), SAMPLE_S24LE (24-bit short little-endian PCM),
 * SAMPLE_S32LE (32-bit short little-endian PCM), and SAMPLE_F32LE (32-bit float little-endian PCM).
 *
 * @param inputFormat Pointer to the input audio format configuration.
 * @param outputFormat Pointer to the output audio format configuration.
 * @param converter Pointer to a variable that receives the created audio converter instance.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if execution succeeds.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if the input parameters are invalid</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT}
 *             if the specified input/output format combination is unsupported</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED} if memory allocation failed</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities</li>
 *         </ul>
 * @release OH_AudioConverter_Destroy {converter}
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
 * @since 26.0.0
 */
void OH_AudioConverter_Destroy(OH_AudioConverter* converter);


/**
 * @brief Define the status of input audio data provided by the callback {@link OH_AudioConverter_RequestDataCallback}.
 * The converter uses this status to determine how to handle subsequent conversion logic (e.g., continue pulling data,
 * pause, or flush cached data).
 * Note for callers: Even if the callback returns {@link AUDIOCONVERTER_INPUT_DATA_FINISHED},
 * {@link OH_AudioConverter_Process} must be called repeatedly
 * until it returns {@link AUDIOCONVERTER_SUCCESS} with outputSize
 * being 0 (indicating all cached data has been flushed).
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
 * The converter invokes this callback to actively request input audio data during {@link OH_AudioConverter_Process}.
 * The caller must populate the output parameters (outInputData, outStatus) and return the valid size of input data.
 * The maximum data size returned by a single callback is 400KB.
 * The memory pointed to by outInputData must remain valid until {@link OH_AudioConverter_Process} returns.
 * @param userData User-defined data passed to the callback.
 * @param outInputData Pointer to a pointer that the callback sets to point to the input audio data buffer.
 * @param outStatus Set by the callback to inform the converter about the current state of input data availability.
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
 * The callback is used by {@link OH_AudioConverter_Process} to pull input audio data dynamically.
 *
 * @param converter Reference created by OH_AudioConverter_Create.
 * @param callback Callback function that will be invoked during {@link OH_AudioConverter_Process}
 * to actively request input audio data.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if execution succeeds.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if parameter is invalid,
 *             e.g. converter is nullptr, e.t.c.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_INITIALIZED} if the converter instance is not initialized</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_CALLBACK_INVALID} if callback is invalid,
 *             e.g. invalid callback return, e.t.c.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION} if the current processing flow does not support
 *             setting a callback.</li>
 *         </ul>
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
 * This function executes audio conversion to convert to the target format, and writes the result to the user-provided
 * output buffer. This function must be called after {@link OH_AudioConverter_SetInputCallback}.
 * The output buffer must be allocated and managed by the caller.
 *
 * @param converter Reference created by {@link OH_AudioConverter_Create}.
 * @param outputData Pointer to the output buffer allocated by the caller to receive converted audio data.
 * @param outputCapacity Size of the output buffer in bytes specified by the user.
 * @param outputSize Size of output buffer the system really writes.
 * Returns the number of bytes actually written on success.
 * When {@link AUDIOCONVERTER_SUCCESS} is returned but outputSize is 0,
 * it indicates that all buffered data has been fully flushed.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if execution succeeds.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if parameter is invalid,
 *             e.g. converter is nullptr, e.t.c.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_INITIALIZED} if the converter instance is not initialized</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_CALLBACK_INVALID} if callback is invalid,
 *             e.g. callback returned an invalid value, e.t.c.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET} if no input callback is bound to the converter</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL} if output buffer capacity is insufficient</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_SYSTEM} if the system has other abnormalities</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioConverter_Result OH_AudioConverter_Process(
    OH_AudioConverter* converter,
    void* outputData,
    int32_t outputCapacity,
    int32_t* outputSize
);

/**
 * @brief Create an audio converter that supports extended formats.
 *
 * Thread-safe; not reentrant; cannot be called in a signal handler; non-blocking.
 * @systemapi
 * @param inputFormat [in] Pointer to the input audio format configuration. It cannot be NULL, must remain valid
 *      during the call, and the caller retains ownership.
 * @param outputFormat [in] Pointer to the output audio format configuration. It cannot be NULL, must remain valid
 *      during the call, and the caller retains ownership.
 * @param outOwnedConverter [out] Pointer that receives the created audio converter instance. It cannot be NULL and
 *      does not need to be initialized. It is set to NULL if the call fails.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if the call is successful.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP} if permission verification fails, that is, a non-system
 *         application uses a system API.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if a parameter is invalid. Possible causes:
 *         <ol><li>inputFormat is NULL.</li>
 *         <li>outputFormat is NULL.</li>
 *         <li>outOwnedConverter is NULL.</li></ol>
 *         <li>{@link #AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT} if the audio format in inputFormat/outputFormat is
 *         unsupported.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED} if memory allocation fails.</li>
 *         </ul>
 * @release OH_AudioConverter_Destroy {outOwnedConverter}
 * @since 26.1.0
 */
OH_AudioConverter_Result OH_AudioSuite_ConverterCreateEx(
    const OH_AudioSuite_ConverterFormatEx *inputFormat,
    const OH_AudioSuite_ConverterFormatEx *outputFormat,
    OH_AudioConverter **outOwnedConverter);

/**
 * @brief Calculate the minimum input size in bytes and the corresponding output size in bytes that guarantee the
 * output contains an integer number of samples.
 *
 * The minimum sizes are determined by the input and output formats set when the converter was created, and are related
 * to the sampling rate, channel count, and bit depth.
 * Thread-safe; reentrant; cannot be called in a signal handler; non-blocking.
 * @systemapi
 * @param converter [in] Converter instance created by {@link OH_AudioSuite_ConverterCreateEx} or
 *      {@link OH_AudioConverter_Create}. It cannot be NULL, must remain valid during the call, and the caller retains
 *      ownership.
 * @param inputMinSize [out] Returns the minimum input size in bytes. It cannot be NULL and does not need to be
 *      initialized; it remains unchanged if the call fails.
 * @param outputMinSize [out] Returns the corresponding minimum output size in bytes. It cannot be NULL and does not
 *      need to be initialized; it remains unchanged if the call fails.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if the call is successful.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP} if permission verification fails, that is, a non-system
 *         application uses a system API.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if a parameter is invalid. Possible causes:
 *         <ol><li>converter is NULL.</li>
 *         <li>inputMinSize is NULL.</li>
 *         <li>outputMinSize is NULL.</li></ol>
 *         </ul>
 * @since 26.1.0
 */
OH_AudioConverter_Result OH_AudioSuite_ConverterGetMinInOutSizes(
    OH_AudioConverter *converter, uint32_t *inputMinSize, uint32_t *outputMinSize);

/**
 * @brief Perform audio format conversion with the input data passed directly by the caller.
 *
 * This API processes the input data passed directly by the caller. Each call supports up to 1000 ms of audio data.
 * A single converter instance cannot both have a callback set via {@link OH_AudioConverter_SetInputCallback} and
 * receive data via this API; only one of the two approaches can be used.
 * When the input and output sampling rates differ, the first call, and the first call after each
 * {@link OH_AudioSuite_ConverterFlush} call, prepend silence of a certain length to the output data.
 * No silence is prepended when the sampling rates are the same.
 * The number of bytes of the prepended silence can be obtained via {@link OH_AudioSuite_ConverterGetDelaySize}.
 * Thread-safe; not reentrant; cannot be called in a signal handler; non-blocking;
 * not recommended to be called on the UI/main thread.
 * @systemapi
 * @param converter [in] Converter instance created by {@link OH_AudioSuite_ConverterCreateEx} or
 *      {@link OH_AudioConverter_Create}. It cannot be NULL, must remain valid during the call, and the caller retains
 *      ownership.
 * @param inData [in] Pointer to the input audio data descriptor. It cannot be NULL, must remain valid during the call,
 *      and the caller retains ownership.
 *     **inData->size** is the input size in bytes, and must be an integer multiple of the minimum input size returned
 *     by {@link OH_AudioSuite_ConverterGetMinInOutSizes}.
 * @param outData [in,out] Pointer to the output audio data descriptor. It cannot be NULL, must remain valid during the
 *     call, and the caller retains ownership.
 *     <br>The caller must allocate the output buffer and set **outData->data** and **outData->size**.
 *     If **inData->size** is k times the minimum input size, **outData->size** must be greater than k times the
 *     minimum output size, where the minimum input size and minimum output size are obtained from
 *     {@link OH_AudioSuite_ConverterGetMinInOutSizes}.
 *     <br>The conversion result is written to **outData->data**, and the number of bytes actually written is returned
 *     via **outData->writeSize**.
 *     <br>**outData->writeSize** remains unchanged if the call fails.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if the call is successful.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP} if permission verification fails, that is, a non-system
 *         application uses a system API.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if a parameter is invalid. Possible causes:
 *         <ol><li>converter is NULL.</li>
 *         <li>inData is NULL.</li>
 *         <li>**inData->data** is NULL.</li>
 *         <li>**inData->size** is 0.</li>
 *         <li>**inData->size** is not an integer multiple of the minimum input size returned by
 *         {@link OH_AudioSuite_ConverterGetMinInOutSizes}.</li>
 *         <li>The audio duration corresponding to **inData->size** exceeds 1000 ms.</li>
 *         <li>outData is NULL.</li>
 *         <li>**outData->data** is NULL.</li></ol>
 *         <li>{@link #AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION} if a callback has already been set via
 *         {@link OH_AudioConverter_SetInputCallback}.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL} if the output buffer is insufficient.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_SYSTEM} if an internal system error occurs.</li>
 *         </ul>
 * @since 26.1.0
 */
OH_AudioConverter_Result OH_AudioSuite_ConverterDirectProcess(
    OH_AudioConverter *converter,
    const OH_AudioSuite_ConverterInputInfo *inData,
    OH_AudioSuite_ConverterOutputInfo *outData);

/**
 * @brief Obtain the number of bytes of silence prepended before the first output data during sampling rate conversion.
 *
 * This API must be called after {@link OH_AudioSuite_ConverterDirectProcess} has produced the silence padding.
 * After {@link OH_AudioSuite_ConverterFlush} is called, the silence padding is produced again.
 * It returns 0 when the sampling rates are the same.
 * Thread-safe; not reentrant; cannot be called in a signal handler; non-blocking.
 * @systemapi
 * @param converter [in] Converter instance created by {@link OH_AudioSuite_ConverterCreateEx} or
 *      {@link OH_AudioConverter_Create}. It cannot be NULL, must remain valid during the call, and the caller retains
 *      ownership.
 * @param delaySize [out] Returns the number of bytes of silence prepended before the first output data.
 *      It cannot be NULL and does not need to be initialized; it remains unchanged if the call fails.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if the call is successful.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP} if permission verification fails, that is, a non-system
 *         application uses a system API.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if a parameter is invalid. Possible causes:
 *         <ol><li>converter is NULL.</li>
 *         <li>delaySize is NULL.</li></ol>
 *         </ul>
 * @since 26.1.0
 */
OH_AudioConverter_Result OH_AudioSuite_ConverterGetDelaySize(OH_AudioConverter *converter, uint32_t *delaySize);

/**
 * @brief Clear the internal buffers and processing state of the audio converter.
 *
 * When the audio data is discontinuous (for example, in fast-forward or rewind scenarios), or when a new audio stream
 * is started before the processing of the previous audio stream is finished, call this API to clear the internal
 * buffers and processing state of the converter, restoring the converter to its initial state.
 * After this API is called, the converter can continue to receive new audio data for conversion without being
 * destroyed and recreated.
 * Thread-safe; not reentrant; cannot be called in a signal handler; non-blocking.
 * @systemapi
 * @param converter [in] Converter instance created by {@link OH_AudioSuite_ConverterCreateEx} or
 *      {@link OH_AudioConverter_Create}. It cannot be NULL, must remain valid during the call, and the caller retains
 *      ownership.
 * @return <ul>
 *         <li>{@link #AUDIOCONVERTER_SUCCESS} if the call is successful.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP} if permission verification fails, that is, a non-system
 *         application uses a system API.</li>
 *         <li>{@link #AUDIOCONVERTER_ERROR_INVALID_PARAM} if a parameter is invalid, that is, converter is NULL.</li>
 *         </ul>
 * @since 26.1.0
 */
OH_AudioConverter_Result OH_AudioSuite_ConverterFlush(OH_AudioConverter *converter);

#ifdef __cplusplus
}
#endif
/** @} */
#endif // NATIVE_AUDIO_CONVERTER_H