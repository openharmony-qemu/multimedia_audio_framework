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
 * @addtogroup AudioAccessoryInputStreamManager
 * @{
 *
 * @brief Provide audio accessory input stream manager C interface.
 *
 * @since 26.0.0
 */

/**
 * @file native_audio_accessory_input_stream_manager.h
 *
 * @brief Declare audio accessory input stream manager related interfaces.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @kit AudioKit
 * @since 26.0.0
 */

#ifndef NATIVE_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H
#define NATIVE_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H

#include <stdbool.h>
#include "native_audio_accessory_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback for opening an input stream on an audio accessory.
 *
 * <b>When Called:</b> The audio framework calls this callback when an
 * application requests audio capture from this audio accessory.
 * The framework passes the audio stream information of the stream being
 * opened, so the accessory can prepare the corresponding data path.
 *
 * <b>Usage Requirements:</b> In this callback, you MUST call
 * {@link OH_AudioAccessoryInputStreamManager_RegisterStartCallback},
 * {@link OH_AudioAccessoryInputStreamManager_RegisterStopCallback},
 * {@link OH_AudioAccessoryInputStreamManager_RegisterReleaseCallback},
 * {@link OH_AudioAccessoryInputStreamManager_RegisterLatencyCallback}, and
 * {@link OH_AudioAccessoryInputStreamManager_RegisterFramePositionCallback} to
 * register required stream callbacks. This is the ONLY time when callback
 * registration is allowed.
 *
 * @param accessory [in] The audio accessory on which the stream is opened.
 * @param stream [in] Reference to the newly created input stream.
 *     Use this handle to register callbacks via Register...Callback.
 * @param streamInfo [in] Pointer to the audio stream information of the stream
 *     being opened. This parameter describes the requested stream format and
 *     can be used by the accessory to configure its data path.
 * @return <ul>
 *         <li>`true` if the stream is accepted.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @see OH_AudioAccessoryInputStreamManager_RegisterStartCallback
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessory_OpenInputStreamCallback)(
    OH_AudioAccessory *accessory,
    OH_AudioAccessoryInputStream *stream,
    OH_AudioStreamInfo *streamInfo);

/**
 * @brief Callback for stream started event.
 *
 * <b>When Called:</b> After the stream is successfully started and ready
 * to receive audio data. After this callback returns, you may call Write()
 * to send audio data.
 *
 * @param accessory [in] The audio accessory that owns this stream.
 * @param stream [in] Reference to the input stream that is started.
 * @return <ul>
 *         <li>`true` if the start event is handled successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessoryInputStream_StartCallback)(
    OH_AudioAccessory *accessory, OH_AudioAccessoryInputStream *stream);

/**
 * @brief Callback for stream stopped event.
 *
 * <b>When Called:</b> After the stream is stopped. After this callback
 * returns, you must stop calling Write(). The stream handle remains
 * valid and may be started again.
 *
 * @param accessory [in] The audio accessory that owns this stream.
 * @param stream [in] Reference to the input stream that is stopped.
 * @return <ul>
 *         <li>`true` if the stop event is handled successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessoryInputStream_StopCallback)(
    OH_AudioAccessory *accessory, OH_AudioAccessoryInputStream *stream);

/**
 * @brief Callback for stream released event.
 *
 * <b>When Called:</b> When the stream is being released. This is always
 * the last callback for a stream. After this callback returns, the stream
 * handle is no longer valid and must not be used.
 *
 * @param accessory [in] The audio accessory that owns this stream.
 * @param stream [in] Reference to the input stream that is released.
 * @return <ul>
 *         <li>`true` if the release event is handled successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessoryInputStream_ReleaseCallback)(
    OH_AudioAccessory *accessory, OH_AudioAccessoryInputStream *stream);

/**
 * @brief Callback for querying the current latency of the stream.
 *
 * <b>When Called:</b> When the framework needs the current latency value
 * reported by the accessory stream.
 *
 * @param accessory [in] The audio accessory that owns this stream.
 * @param stream [in] Reference to the input stream.
 * @param latency [out] Output parameter. Returns the latency, in milliseconds.
 * @return <ul>
 *         <li>`true` if the latency is obtained successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessoryInputStream_GetLatencyCallback)(
    OH_AudioAccessory *accessory,
    OH_AudioAccessoryInputStream *stream,
    int32_t *latency);

/**
 * @brief Callback for querying the current frame position of the stream.
 *
 * <b>When Called:</b> When the framework needs the current capture position
 * reported by the accessory stream.
 *
 * @param accessory [in] The audio accessory that owns this stream.
 * @param stream [in] Reference to the input stream.
 * @param framePosition [out] Output parameter. Returns the cumulative number of audio
 *     frames captured since the most recent successful start of this input
 *     stream.
 * @param timestamp [out] Returns the capture timestamp corresponding to the frame
 *     position reported through {@p framePosition}. The timestamp must use the
 *     {@link CLOCK_MONOTONIC} time base and is expressed in nanoseconds. It represents
 *     the monotonic clock time at which the frame identified by {@p framePosition} was captured.
 * @return <ul>
 *         <li>`true` if the frame position is obtained successfully.</li>
 *         <li>`false` otherwise.</li>
 *         </ul>
 * @since 26.0.0
 */
typedef bool (*OH_AudioAccessoryInputStream_GetFramePositionCallback)(
    OH_AudioAccessory *accessory, OH_AudioAccessoryInputStream *stream,
    int64_t *framePosition, int64_t *timestamp);

/**
 * @brief Registers the callback for stream started event.
 *
 * <b>CRITICAL: Registration Timing Constraint</b>
 *
 * This function MUST be called ONLY during the execution of
 * {@link OH_AudioAccessory_OpenInputStreamCallback}. Calling this
 * function at any other time will result in {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE}.
 *
 * <b>Requirement:</b> This callback is MANDATORY. If not registered,
 * the framework will reject the stream creation and trigger cleanup.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param callback [in] Pointer to the callback function. Must not be null.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if called outside
 *                  {@link OH_AudioAccessory_OpenInputStreamCallback} or stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterStartCallback(
    OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_StartCallback callback);

/**
 * @brief Registers the callback for stream stopped event.
 *
 * <b>CRITICAL: Registration Timing Constraint</b>
 *
 * This function MUST be called ONLY during the execution of
 * {@link OH_AudioAccessory_OpenInputStreamCallback}. Calling this
 * function at any other time will result in {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE}.
 *
 * <b>Requirement:</b> This callback is MANDATORY. If not registered,
 * the framework will reject the stream creation and trigger cleanup.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param callback [in] Pointer to the callback function. Must not be null.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if called outside
 *                  {@link OH_AudioAccessory_OpenInputStreamCallback} or stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterStopCallback(
    OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_StopCallback callback);

/**
 * @brief Registers the callback for stream released event.
 *
 * <b>CRITICAL: Registration Timing Constraint</b>
 *
 * This function MUST be called ONLY during the execution of
 * {@link OH_AudioAccessory_OpenInputStreamCallback}. Calling this
 * function at any other time will result in {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE}.
 *
 * <b>Requirement:</b> This callback is MANDATORY. If not registered,
 * the framework will reject the stream creation and trigger cleanup.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param callback [in] Pointer to the callback function. Must not be null.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if called outside
 *                  {@link OH_AudioAccessory_OpenInputStreamCallback} or stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterReleaseCallback(
    OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_ReleaseCallback callback);

/**
 * @brief Registers the callback for stream latency query.
 *
 * <b>CRITICAL: Registration Timing Constraint</b>
 *
 * This function MUST be called ONLY during the execution of
 * {@link OH_AudioAccessory_OpenInputStreamCallback}. Calling this
 * function at any other time will result in {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE}.
 *
 * <b>Requirement:</b> This callback is MANDATORY. If not registered,
 * the framework will reject the stream creation and trigger cleanup.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param callback [in] Pointer to the callback function. Must not be null.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if called outside
 *                  {@link OH_AudioAccessory_OpenInputStreamCallback} or stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterLatencyCallback(
    OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_GetLatencyCallback callback);

/**
 * @brief Registers the callback for stream frame position query.
 *
 * <b>CRITICAL: Registration Timing Constraint</b>
 *
 * This function MUST be called ONLY during the execution of
 * {@link OH_AudioAccessory_OpenInputStreamCallback}. Calling this
 * function at any other time will result in {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE}.
 *
 * <b>Requirement:</b> This callback is MANDATORY. If not registered,
 * the framework will reject the stream creation and trigger cleanup.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param callback [in] Pointer to the callback function. Must not be null.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if called outside
 *                  {@link OH_AudioAccessory_OpenInputStreamCallback} or stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterFramePositionCallback(
    OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_GetFramePositionCallback callback);

/**
 * @brief Writes audio data to the audio accessory input stream.
 *
 * This is a blocking interface. After being called, the function blocks until
 * the whole frame is written successfully or an error occurs. Each call must
 * write exactly 20 ms of audio data. The caller must ensure that dataSize
 * matches the byte count corresponding to 20 ms under the current stream
 * configuration. If dataSize does not match 20 ms of audio data, this
 * function returns {@link AUDIOCOMMON_RESULT_ERROR_FRAME_LENGTH_MISMATCH}.
 * The caller must invoke this function at a 20 ms cadence. That is, each call
 * must submit 20 ms of audio data, and the interval between two consecutive
 * calls must also be 20 ms.
 * If the stream buffer does not currently have enough writable space for the
 * whole frame, this function blocks until enough space becomes available or an
 * error occurs. Partial-frame writes are not supported by this interface. If
 * the last frame has less than 20 ms of audio data, the caller may discard
 * this frame or pad it with zeros to 20 ms before calling this function.
 *
 * <b>Calling Context and Concurrency:</b>
 *
 * This function is not reentrant for the same stream. The caller is advised
 * to use only one thread to write audio data serially to the same stream.
 * If this function is called concurrently with the stop or release callback
 * for the same stream, it returns
 * {@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the stop or release
 * operation completes before this function acquires the lock.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param data [in] Pointer to the audio data buffer. Must not be null.
 * @param dataSize [in] Size of the audio data in bytes. Must be > 0.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_FRAME_LENGTH_MISMATCH} if dataSize does not correspond
 *                  to 20 ms of audio data under the current stream configuration.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if stream is not started or the required
 *                  stream callbacks are not fully registered.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_SYSTEM} if audio server process die.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_Write(
    OH_AudioAccessoryInputStream *stream,
    const uint8_t *data,
    uint32_t dataSize);

/**
 * @brief Obtains the writable size of the audio accessory input stream buffer.
 *
 * This function can be used by the caller to probe current buffer availability
 * before calling {@link OH_AudioAccessoryInputStreamManager_Write}. The returned
 * writable size reflects the current state only, and may change immediately
 * after the function returns.
 *
 * @param stream [in] Pointer to the input stream handle.
 * @param writableSize [out] Output parameter. Returns the number of bytes that can be written.
 * @return <ul>
 *         <li>{@link AUDIOCOMMON_RESULT_SUCCESS} if execution succeeds.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM} if parameters are null.</li>
 *         <li>{@link AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE} if the stream is released.</li>
 *         </ul>
 * @since 26.0.0
 */
OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_GetWritableSize(
    OH_AudioAccessoryInputStream *stream,
    uint32_t *writableSize);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // NATIVE_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H
