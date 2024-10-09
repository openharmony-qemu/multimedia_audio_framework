/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
 * @since 10
 * @version 1.0
 */

/**
 * @file native_audiostreambuilder.h
 *
 * @brief Declare audio stream builder related interfaces.
 *
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @since 10
 * @version 1.0
 */

#ifndef NATIVE_AUDIOSTREAM_BUILDER_H
#define NATIVE_AUDIOSTREAM_BUILDER_H

#include "native_audiostream_base.h"
#include "native_audiorenderer.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a stremBuilder can be used to open a renderer or capturer client.
 *
 * OH_AudioStreamBuilder_Destroy() must be called when you are done using the builder.
 *
 * @since 10
 *
 * @param builder The builder reference to the created result.
 * @param type The stream type to be created. {@link #AUDIOSTREAM_TYPE_RENDERER} or {@link #AUDIOSTREAM_TYPE_CAPTURER}
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_Create(OH_AudioStreamBuilder** builder, OH_AudioStream_Type type);

/**
 * Destroy a streamBulder.
 *
 * This function must be called when you are done using the builder.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_Destroy(OH_AudioStreamBuilder* builder);

/*
 * Set the channel count of the capturer client
 *
 * @since 10
 *
 * @param capturer Reference created by OH_AudioStreamBuilder
 * @param channelCount Pointer to a variable that will be set for the channel count.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetSamplingRate(OH_AudioStreamBuilder* builder, int32_t rate);

/*
 * Set the channel count of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param channelCount The channel count.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetChannelCount(OH_AudioStreamBuilder* builder, int32_t channelCount);

/*
 * Set the sample format of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param format Sample data format.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetSampleFormat(OH_AudioStreamBuilder* builder,
    OH_AudioStream_SampleFormat format);

/*
 * Set the encoding type of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param encodingType Encoding type for the stream client, {@link #AUDIOSTREAM_ENCODING_PCM}
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetEncodingType(OH_AudioStreamBuilder* builder,
    OH_AudioStream_EncodingType encodingType);

/*
 * Set the latency mode of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param latencyMode Latency mode for the stream client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetLatencyMode(OH_AudioStreamBuilder* builder,
    OH_AudioStream_LatencyMode latencyMode);

/**
 * @brief Set the channel layout to the stream client
 *
 * @since 12
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param channelLayout is the layout of the speaker.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetChannelLayout(OH_AudioStreamBuilder* builder,
    OH_AudioChannelLayout channelLayout);

/*
 * Set the renderer information of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param usage Set the stream usage for the renderer client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererInfo(OH_AudioStreamBuilder* builder,
    OH_AudioStream_Usage usage);

/*
 * Set the capturer information of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param sourceType Set the source type for the capturer client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetCapturerInfo(OH_AudioStreamBuilder* builder,
    OH_AudioStream_SourceType sourceType);

/*
 * Set the callbacks for the renderer client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callbacks Callbacks to the functions that will process renderer stream.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererCallback(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer_Callbacks callbacks, void* userData);

/**
 * @brief Set the callback when the output device of an audio renderer changed.
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callback Callback to the function that will process this device change event.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 * @since 11
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererOutputDeviceChangeCallback(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer_OutputDeviceChangeCallback callback, void* userData);


/*
 * Set the callbacks for the capturer client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callbacks Callbacks to the functions that will process capturer stream.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetCapturerCallback(OH_AudioStreamBuilder* builder,
    OH_AudioCapturer_Callbacks callbacks, void* userData);

/*
 * Create the audio renderer client.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param audioRenderer Pointer to a viriable to receive the stream client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_GenerateRenderer(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer** audioRenderer);
/*
 * Create the audio capturer client.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param audioCapturer Pointer to a viriable to receive the stream client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_GenerateCapturer(OH_AudioStreamBuilder* builder,
    OH_AudioCapturer** audioCapturer);

OH_AudioStream_Result OH_AudioStreamBuilder_SetFrameSizeInCallback(OH_AudioStreamBuilder* builder,
    int32_t frameSize);

/*
 * Set stream privacy type for plabck capture.
 *
 * @since 12
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create().
 * @param privacy The privacy type to be set.{@link #AUDIO_STREAM_PRIVACY_TYPE_PUBLIC} or
 *     {@link #AUDIO_STREAM_PRIVACY_TYPE_PRIVATE}.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererPrivacy(OH_AudioStreamBuilder* builder,
    OH_AudioStream_PrivacyType privacy);

/*
 * Set the interrupt mode of the stream client
 *
 * @since 12
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param mode The audio interrupt mode
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererInterruptMode(OH_AudioStreamBuilder* builder,
    OH_AudioInterrupt_Mode mode);

/**
 * @brief Set the callback of writing metadata to the renderer client
 *
 * @since 12
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callback Callback to the functions that will write audio data with metadata to the renderer.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer_WriteDataWithMetadataCallback callback, void* userData);

/**
 * @brief Set the callback of writing data to renderer client.
 *
 * This function is similar with {@link OH_AudioStreamBuilder_SetRendererCallback}. Only the last callback set by
 * OH_AudioStreamBuilder_SetRendererCallback or this function will become effective.
 *
 * @param builder Builder provided by OH_AudioStreamBuilder_Create()
 * @param callback Callback to functions that will write audio data to renderer client.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return Result code.
 *     {@link AUDIOSTREAM_SUCCESS} Success.
 *     {@link AUDIOSTREAM_ERROR_INVALID_PARAM} Parameter is invalid, e.g. builder is nullptr, e.t.c.
 * @since 12
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererWriteDataCallback(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer_OnWriteDataCallback callback, void* userData);
#ifdef __cplusplus
}
#endif

#endif // NATIVE_AUDIOSTREAM_BUILDER_H
