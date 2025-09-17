/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 * @since 22
 * @version 1.0
 */

/**
 * @file native_audio_suite_base.h
 *
 * @brief Declare underlying data structure.
 *
 * @library libohaudio.so
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @since 22
 * @version 1.0
 */

#ifndef NATIVE_AUDIO_SUITE_BASE_H
#define NATIVE_AUDIO_SUITE_BASE_H

#include <stdint.h>
#include "multimedia/native_audio_channel_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Define audio node type.
 *
 * @since 22
 */
typedef enum {
    /**
     * input node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_INPUT = 1,
    /**
     * output node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_OUTPUT = 2,
    /**
     * Equalization node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_EQUALIZER = 3,
    /**
     * Noise reduction node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_NOISE_REDUCTION = 4,
    /**
     * Sound field node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_SOUND_FIELD = 5,
    /**
     * Audio Separation node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_AUDIO_SEPARATION = 6,
    /**
     * Tempo Pitch node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_TEMPO_PITCH = 7,
    /**
     * Space Render node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_SPACE_REDNER = 8,
    /**
     * Voice beautifier node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_VOICE_BEAUTIFIER = 9,
    /**
     * Scene effect node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_ENVIRONMENT_EFFECT = 10,
    /**
     * Audio mixer node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_TYPE_AUDIO_MIXER = 11,
} OH_AudioNode_Type;

/**
 * @brief Define audio node enable status, This property is only supported for effect nodes.
 *
 * @since 22
 */
typedef enum {
    /**
     * input node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_ENABLE = 1,
    /**
     * input node type.
     *
     * @since 22
     */
    AUDIOSUITE_NODE_DISABLE = 2,
} OH_AudioNodeEnable;

/**
 * @brief Define pipeline work mode
 *
 * @since 22
 */
typedef enum {
    /**
     * The pipeline supports creating various effect nodes in suite mode.
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_EDIT_MODE = 1,
    /**
     * If you need to play audio after effect processing, you should select this mode.
     * In real-time rendering mode, the pipeline supports EQ effect processing.
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_REALTIME_MODE = 2,
} OH_AudioSuite_PipelineWorkMode;

/**
 * @brief Define pipeline state
 *
 * @since 22
 */
typedef enum {
    /**
     * The pipeline is in a stopped state.
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_STOPPED = 1,
    /**
     * The pipeline is in a running state.
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_RUNNING = 2,
} OH_AudioSuite_PipelineState;

/**
 * @brief Define pipeline processing state. When the pipeline is in the running state,
 *        the application calls the OH_AudioSuiteEngine_RenderFrame interface to process the raw data.
 *        At this point, the pipeline is in the AUDIOSUITE_PIPELINE_PROCESSING state.
 *        Once all the raw data has been processed, the pipeline transitions to the AUDIOSUITE_PIPELINE_FINISHED state.
 *
 * @since 22
 */
typedef enum {
    /**
     * The pipeline is processing data.
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_PROCESSING = 1,
    /**
     * The pipeline has completed data processing. .
     *
     * @since 22
     */
    AUDIOSUITE_PIPELINE_FINISHED = 2,
} OH_AudioSuite_PipelineProcessState;

/**
 * @brief Define the result of the function execution.
 *
 * @since 22
 */
typedef enum {
    /**
     * @error The call was successful.
     *
     * @since 22
     */
    AUDIOSUITE_SUCCESS = 0,
    /**
     * @error This means that the function was executed with an invalid input parameter.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_INVALID_PARAM = 1,
    /**
     * @error Execution status exception.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_ILLEGAL_STATE = 2,
    /**
     * @error An system error has occurred.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_SYSTEM = 3,
    /**
     * @error Unsupported audio format, such as unsupported encoding type, sample format etc.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT = 4,
    /**
     * @error audio engine not exist.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_ENGINE_NOT_EXIST = 5,
    /**
     * @error audio pipeline not exist.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST = 6,
    /**
     * @error audio pipeline not exist.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_NODE_NOT_EXIST = 7,
    /**
     * @error the connect or disconnect betwen the nodes is unsupported.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_UNSUPPORT_CONNECT = 8,
    /**
     * @error the operation of current pipeline or node is unsupported.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_UNSUPPORT_OPERATION = 9,
    /**
     * @error The number of created pipelines or nodes exceeds the system specification.
     *
     * @since 22
     */
    AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS = 10,
} OH_AudioSuite_Result;

/**
 * @brief Define the result of the callback function.
 *
 * @since 22
 */
typedef enum {
    /**
     * @Result of audio data callabck is invalid.
     *
     * @since 22
     */
    AUDIOSUITE_DATA_CALLBACK_RESULT_INVALID = -1,
    /**
     * @Result of audio data callabck is valid.
     *
     * @since 22
     */
    AUDIOSUITE_DATA_CALLBACK_RESULT_VALID = 0,
} OH_AudioSuite_Callback_Result;

/**
 * @brief The port of a node refers to the input or output port that connects this node to other nodes.
 *        Most nodes have only one outport, with the type set to default.
 *
 * @since 22
 */
typedef enum {
    /**
     * @default type.
     *
     * @since 22
     */
    AUDIO_NODE_DEFAULT_OUTPORT_TYPE = 0,
    /**
     * @human sound type.
     *
     * @since 22
     */
    AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE = 1,
    /**
     * @background sound type.
     *
     * @since 22
     */
    AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE = 2,
} OH_AudioNode_Port_Type;

/**
 * @brief Define the audio sample format.
 *
 * @since 22
 */
typedef enum {
    /**
     * Unsigned 8 format.
     *
     * @since 22
     */
    AUDIO_SAMPLE_U8 = 0,
    /**
     * Signed 16 bit integer, little endian.
     *
     * @since 22
     */
    AUDIO_SAMPLE_S16LE = 1,
    /**
     * Signed 24 bit integer, little endian.
     *
     * @since 22
     */
    AUDIO_SAMPLE_S24LE = 2,
    /**
     * Signed 32 bit integer, little endian.
     *
     * @since 22
     */
    AUDIO_SAMPLE_S32LE = 3,
    /**
     * 32 bit IEEE floating point, little endian.
     *
     * @since 22
     */
    AUDIO_SAMPLE_F32LE = 4,
} OH_Audio_SampleFormat;

/**
 * @brief Define the audio encoding type.
 *
 * @since 22
 */
typedef enum {
    /**
     * PCM encoding type.
     *
     * @since 22
     */
    AUDIO_ENCODING_TYPE_RAW = 0,
} OH_Audio_EncodingType;

/**
 * @brief Define the audio format info structure, used to describe basic audio format.
 *
 * @since 22
 */
typedef struct OH_AudioFormat {
    /**
     * @brief Audio sampling rate.
     *
     * @since 22
     */
    int32_t samplingRate;
    /**
     * @brief Audio channel layout.
     *
     * @since 22
     */
    OH_AudioChannelLayout channelLayout;
    /**
     * @brief Audio channel layout count.
     *
     * @since 22
     */
    uint32_t channelCount;
    /**
     * @brief Audio encoding format type.
     *
     * @since 22
     */
    OH_Audio_EncodingType encodingType;
    /**
     * @brief Audio sample format.
     *
     * @since 22
     */
    OH_Audio_SampleFormat sampleFormat;
} OH_AudioFormat;

/**
 * @brief Define the equalizer mode.
 *
 * @since 22
 */
typedef enum {
    /**
     * Default equalization effect
     *
     * @since 22
     */
    EQUALIZER_DEFAULT_MODE = 1,
    /**
     * Ballad equalization effect
     *
     * @since 22
     */
    EQUALIZER_BALLADS_MODE = 2,
    /**
     * Chinese style equalization effect.
     *
     * @since 22
     */
    EQUALIZER_CHINESE_STYLE_MODE = 3,
    /**
     * Classic equalization effect
     *
     * @since 22
     */
    EQUALIZER_CLASSICAL_MODE = 4,
    /**
     * Dance music equalization effect
     *
     * @since 22
     */
    EQUALIZER_DANCE_MUSIC_MODE = 5,
    /**
     * Jazz equalization effect
     *
     * @since 22
     */
    EQUALIZER_JAZZ_MODE = 6,
    /**
     * Pop equalization effect
     *
     * @since 22
     */
    EQUALIZER_POP_MODE = 7,
    /**
     * R&B equalization effect
     *
     * @since 22
     */
    EQUALIZER_RB_MODE = 8,
    /**
     * Rock equalization effect
     *
     * @since 22
     */
    EQUALIZER_ROCK_MODE = 9,
} OH_EqualizerMode;

/**
 * @brief Define the sound field type.
 *
 * @since 22
 */
typedef enum {
    /**
     * Close sound field type.
     *
     * @since 22
     */
    SOUND_FIELD_CLOSE = 1,
    /**
     * Front facing sound field type.
     *
     * @since 22
     */
    SOUND_FIELD_FRONT_FACING = 2,
    /**
     * Grand sound field type.
     *
     * @since 22
     */
    SOUND_FIELD_GRAND = 3,
    /**
     * Near sound field type.
     *
     * @since 22
     */
    SOUND_FIELD_NEAR = 4,
    /**
     * Near sound field type.
     *
     * @since 22
     */
    SOUND_FIELD_WIDE = 5,
} OH_SoundFieldType;

/**
 * @brief Define the environment type.
 *
 * @since 22
 */
typedef enum {
    /**
     * Close environment effect type.
     *
     * @since 22
     */
    ENVIRONMENT_TYPE_CLOSE = -1,
    /**
     * Broadcast environment effect type.
     *
     * @since 22
     */
    ENVIRONMENT_TYPE_BROADCAST = 0,
    /**
     * Earpiece environment effect type.
     *
     * @since 22
     */
    ENVIRONMENT_TYPE_EARPIECE = 1,
    /**
     * Earpiece environment effect type.
     *
     * @since 22
     */
    ENVIRONMENT_TYPE_UNDERWATER = 2,
    /**
     * Gramophone environment effect type.
     *
     * @since 22
     */
    ENVIRONMENT_TYPE_GRAMOPHONE = 3
} OH_EnvironmentType;

/**
 * @brief Define voice beautifier type.
 *
 * @since 22
 */
typedef enum {
    /**
     * Clear voice beautifier type.
     *
     * @since 22
     */
    VOICE_BEAUTIFIER_TYPE_CLEAR,
    /**
     * Theatre voice beautifier type.
     *
     * @since 22
     */
    VOICE_BEAUTIFIER_TYPE_THEATRE,
    /**
     * CD voice beautifier type.
     *
     * @since 22
     */
    VOICE_BEAUTIFIER_TYPE_CD,
    /**
     * Studio voice beautifier type.
     *
     * @since 22
     */
    VOICE_BEAUTIFIER_TYPE_STUDIO,
    /**
     * Normal voice beautifier type.
     *
     * @since 22
     */
    VOICE_BEAUTIFIER_TYPE_NORMAL
} OH_VoiceBeautifierType;


/**
 * @brief Define the number of equalizer frequency bands
 *
 * @since 22
 */
#define EQUALIZER_BAND_NUM (10)

/**
 * @brief Define equalizer frequency band gains type
 *
 * @since 22
 */
typedef struct OH_EqualizerFrequencyBandGains {
    int32_t gains[EQUALIZER_BAND_NUM];
} OH_EqualizerFrequencyBandGains;

/**
 * @brief Declare the audio engine.
 * The handle of audio suite engine is used for audio suite engine related functions.
 *
 * @since 22
 */
typedef struct OH_AudioEditEngineStruct OH_AudioSuiteEngine;

/**
 * @brief Declare the audio pipe line.
 * The handle of audio suite pipe line is used for audio pipe line related functions.
 *
 * @since 22
 */
typedef struct OH_AudioEditPipelineStruct OH_AudioSuitePipeline;

/**
 * @brief Declare the audio node.
 * The handle of audio suite node is used for audio suite node related functions.
 *
 * @since 22
 */
typedef struct OH_AudioNodeStruct OH_AudioNode;

/**
 * @brief Declare the audio node builder.
 * The handle of audio node builder is used for audio node create.
 *
 * @since 22
 */
typedef struct OH_AudioNodeBuilderStruct OH_AudioNodeBuilder;

#ifdef __cplusplus
}
#endif
/** @} */
#endif // NATIVE_AUDIO_SUITE_BASE_H