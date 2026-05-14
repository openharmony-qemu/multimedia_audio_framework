/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "AudioRenderer.h"
#include "hilog/log.h"
#include "ohaudio/native_audiorenderer.h"
#include "ohaudio/native_audiostreambuilder.h"
#include "ohaudio/native_audiostream_base.h"
#include "ohaudiosuite/native_audio_suite_engine.h"
#include "realTimePlay/RealTimePlaying.h"
#include "callback/RegisterCallback.h"
#include "multiPipelineEdit/MultiPipelineEdit.h"
#include "audioEffectNode/Input.h"
#include <algorithm>

const int GLOBAL_RESMGR = 0xFF00;
static const char *TAG = "[AudioEditTestApp_AudioRenderer_cpp]";

// Local constants
const int32_t SAMPLINGRATE_MULTI = 20;
const int32_t CHANNELCOUNT_MULTI = 1000;
const int32_t BITSPERSAMPLE_MULTI = 8;
const int32_t ARG_COUNT_1 = 1;
const int32_t ARG_COUNT_4 = 4;
const int32_t MIN_ARGS_1 = 1;
const int32_t MIN_ARGS_3 = 3;
const int32_t ARGVNUM_0 = 0;
const int32_t ARGVNUM_1 = 1;
const int32_t ARGVNUM_2 = 2;
const int32_t ARGVNUM_3 = 3;
const int32_t BITDEPTH_MODE_INT = 0;
const int32_t BITDEPTH_MODE_FLOAT = 0;
const int32_t BITS_PER_SAMPLE_8 = 8;
const int32_t BITS_PER_SAMPLE_16 = 16;
const int32_t BITS_PER_SAMPLE_24 = 24;
const int32_t BITS_PER_SAMPLE_32 = 32;
const int32_t RETURN_SUCCESS = 0;
const int32_t RETURN_FAILED = 1;
const int32_t FIRST_PIPELINE_INDEX = 0;
const int32_t TSFN_INITIAL_QUEUE_SIZE = 0;
const int32_t TSFN_THREAD_COUNT = 1;
const int32_t MAX_PLAY_RESULT_BUFFER_SIZE = 1024 * 1024 * 1024;

// Helper function to parse audio renderer parameters
static bool ParseAudioRendererParams(napi_env env, napi_callback_info info, AudioRendererParams &params)
{
    size_t argc = ARG_COUNT_4;
    napi_value *argv = new napi_value[argc];
    napi_status napiStatus = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (napiStatus != napi_ok || argc < MIN_ARGS_3) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Invalid arguments, status=%{public}d", napiStatus);
        delete[] argv;
        return false;
    }

    napiStatus = napi_get_value_int32(env, argv[ARGVNUM_0], &params.sampleRate);
    if (napiStatus != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Failed to get sampleRate, status=%{public}d", napiStatus);
        delete[] argv;
        return false;
    }

    napiStatus = napi_get_value_int32(env, argv[ARGVNUM_1], &params.channels);
    if (napiStatus != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Failed to get channels, status=%{public}d", napiStatus);
        delete[] argv;
        return false;
    }

    napiStatus = napi_get_value_int32(env, argv[ARGVNUM_2], &params.bitDepth);
    if (napiStatus != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Failed to get bitDepth, status=%{public}d", napiStatus);
        delete[] argv;
        return false;
    }

    params.bitDepthMode = BITDEPTH_MODE_INT;
    if (argc >= ARG_COUNT_4) {
        napiStatus = napi_get_value_int32(env, argv[ARGVNUM_3], &params.bitDepthMode);
        if (napiStatus != napi_ok) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                         "audioEditTest InitAudioRenderer: Failed to get bitDepthMode, using default (int mode)");
        }
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "audioEditTest InitAudioRenderer: sampleRate=%{public}d, channels=%{public}d, bitDepth=%{public}d, "
                 "bitDepthMode=%{public}d",
                 params.sampleRate, params.channels, params.bitDepth, params.bitDepthMode);
    delete[] argv;
    return true;
}

// Helper function to convert bit depth to sample format
static bool ConvertBitDepthToSampleFormat(const AudioRendererParams &params,
                                          OH_AudioStream_SampleFormat &streamSampleFormat)
{
    if (params.bitDepth == BITS_PER_SAMPLE_8) {
        streamSampleFormat = AUDIOSTREAM_SAMPLE_U8;
    } else if (params.bitDepth == BITS_PER_SAMPLE_16) {
        streamSampleFormat = AUDIOSTREAM_SAMPLE_S16LE;
    } else if (params.bitDepth == BITS_PER_SAMPLE_24) {
        streamSampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    } else if (params.bitDepth == BITS_PER_SAMPLE_32) {
        if (params.bitDepthMode == BITDEPTH_MODE_FLOAT) {
            streamSampleFormat = AUDIOSTREAM_SAMPLE_F32LE;
        } else {
            streamSampleFormat = AUDIOSTREAM_SAMPLE_S32LE;
        }
    } else {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Unsupported bit depth %{public}d", params.bitDepth);
        return false;
    }
    return true;
}

// Helper function to configure audio stream builder
static void ConfigureAudioStreamBuilder(const AudioRendererParams &params,
                                        OH_AudioStream_SampleFormat streamSampleFormat)
{
    OH_AudioStreamBuilder_SetSamplingRate(rendererBuilder, params.sampleRate);
    OH_AudioStreamBuilder_SetChannelCount(rendererBuilder, params.channels);
    OH_AudioStreamBuilder_SetSampleFormat(rendererBuilder, streamSampleFormat);
    OH_AudioStreamBuilder_SetEncodingType(rendererBuilder, AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_SetRendererInfo(rendererBuilder, AUDIOSTREAM_USAGE_MUSIC);

    g_playDataSize = SAMPLINGRATE_MULTI * params.sampleRate * params.channels * params.bitDepth / BITSPERSAMPLE_MULTI /
                     CHANNELCOUNT_MULTI;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest InitAudioRenderer: g_playDataSize=%{public}d",
                 g_playDataSize);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(rendererBuilder, g_playDataSize);

    OH_AudioRenderer_OnWriteDataCallback rendererCallbacks = PlayAudioRendererOnWriteData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(rendererBuilder, rendererCallbacks, nullptr);
}

// Initialize audio renderer
napi_value InitAudioRenderer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest InitAudioRenderer start");

    AudioRendererParams params;
    if (!ParseAudioRendererParams(env, info, params)) {
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    ReleaseExistingResources();

    OH_AudioStream_Type type = OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result builderResult = OH_AudioStreamBuilder_Create(&rendererBuilder, type);
    if (builderResult != AUDIOSTREAM_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Failed to create builder, result=%{public}d", builderResult);
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    OH_AudioStream_SampleFormat streamSampleFormat;
    if (!ConvertBitDepthToSampleFormat(params, streamSampleFormat)) {
        OH_AudioStreamBuilder_Destroy(rendererBuilder);
        rendererBuilder = nullptr;
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    ConfigureAudioStreamBuilder(params, streamSampleFormat);

    OH_AudioStream_Result genResult = OH_AudioStreamBuilder_GenerateRenderer(rendererBuilder, &audioRenderer);
    if (genResult != AUDIOSTREAM_SUCCESS || audioRenderer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest InitAudioRenderer: Failed to generate renderer, result=%{public}d", genResult);
        OH_AudioStreamBuilder_Destroy(rendererBuilder);
        rendererBuilder = nullptr;
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest InitAudioRenderer: Success");
    napi_value result;
    napi_create_int32(env, RETURN_SUCCESS, &result);
    return result;
}

// Start audio renderer (triggers callback loop)
napi_value StartAudioRenderer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest StartAudioRenderer start");

    if (audioRenderer == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest StartAudioRenderer: audioRenderer is nullptr");
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    g_audioSuitePipeline = g_multiAudioSuitePipeline[FIRST_PIPELINE_INDEX];
    // Start the pipeline first
    ProcessPipeline();

    // Allocate buffer for recording if needed
    if (g_isRecord) {
        if (g_playTotalAudioData == nullptr) {
            g_playTotalAudioData = (char *)calloc(1, MAX_PLAY_RESULT_BUFFER_SIZE);
            if (g_playTotalAudioData == nullptr) {
                OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                             "audioEditTest StartAudioRenderer: Failed to allocate g_playTotalAudioData");
                napi_value result;
                napi_create_int32(env, RETURN_FAILED, &result);
                return result;
            }
        }
        g_playResultTotalSize = 0;
    }

    // Start the audio renderer (this triggers the callback loop)
    OH_AudioStream_Result startResult = OH_AudioRenderer_Start(audioRenderer);
    if (startResult != AUDIOSTREAM_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest StartAudioRenderer: Failed to start renderer, result=%{public}d", startResult);
        napi_value result;
        napi_create_int32(env, RETURN_FAILED, &result);
        return result;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest StartAudioRenderer: Success");
    napi_value result;
    napi_create_int32(env, RETURN_SUCCESS, &result);
    return result;
}

// Stop audio renderer
napi_value StopAudioRenderer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest StopAudioRenderer start");

    OH_AudioStream_Result stopResult = AUDIOSTREAM_SUCCESS;
    if (audioRenderer != nullptr) {
        stopResult = OH_AudioRenderer_Stop(audioRenderer);
        if (stopResult != AUDIOSTREAM_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                         "audioEditTest StopAudioRenderer: Failed to stop renderer, result=%{public}d", stopResult);
        } else {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest StopAudioRenderer: Success");
        }
    } else {
        OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, TAG,
                     "audioEditTest StopAudioRenderer: audioRenderer is nullptr");
    }

    // Stop the pipeline
    if (g_audioSuitePipeline != nullptr) {
        OH_AudioSuiteEngine_StopPipeline(g_audioSuitePipeline);
    }

    napi_value result;
    napi_create_int32(env, (stopResult == AUDIOSTREAM_SUCCESS) ? RETURN_SUCCESS : RETURN_FAILED, &result);
    return result;
}

// Release audio renderer resources
napi_value ReleaseAudioRenderer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest ReleaseAudioRenderer start");

    ReleaseExistingResources();

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest ReleaseAudioRenderer: Success");
    return nullptr;
}

// Set record flag to enable/disable recording of output
napi_value SetRecordFlag(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetRecordFlag start");
    size_t argc = ARG_COUNT_1;
    napi_value *argv = new napi_value[argc];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < MIN_ARGS_1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest SetRecordFlag: Invalid arguments, status=%{public}d, argc=%{public}d", status,
                     (int)argc);
        delete[] argv;
        return nullptr;
    }

    bool enable;
    status = napi_get_value_bool(env, argv[0], &enable);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest SetRecordFlag: Failed to get bool value, status=%{public}d", status);
        delete[] argv;
        return nullptr;
    }

    g_isRecord = enable;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest SetRecordFlag: g_isRecord=%{public}s",
                 g_isRecord ? "true" : "false");

    delete[] argv;
    return nullptr;
}

// Get recorded audio data
napi_value GetRecordedAudioData(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "audioEditTest GetRecordedAudioData: g_playResultTotalSize=%{public}d", g_playResultTotalSize);

    if (g_playResultTotalSize == 0 || g_playTotalAudioData == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_WARN, GLOBAL_RESMGR, TAG,
                     "audioEditTest GetRecordedAudioData: No audio data recorded");
        return nullptr;
    }

    // Check for buffer overflow
    if (g_playResultTotalSize > MAX_PLAY_RESULT_BUFFER_SIZE) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest GetRecordedAudioData: Buffer overflow detected, size=%{public}d, max=%{public}d",
                     g_playResultTotalSize, MAX_PLAY_RESULT_BUFFER_SIZE);
        return nullptr;
    }

    // Create ArrayBuffer and copy data
    napi_value arrayBuffer;
    void *data;
    napi_status status = napi_create_arraybuffer(env, g_playResultTotalSize, &data, &arrayBuffer);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest GetRecordedAudioData: Failed to create ArrayBuffer, status=%{public}d", status);
        return nullptr;
    }

    std::copy(reinterpret_cast<const uint8_t *>(g_playTotalAudioData),
              reinterpret_cast<const uint8_t *>(g_playTotalAudioData) + g_playResultTotalSize,
              static_cast<uint8_t *>(data));
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest GetRecordedAudioData: Returned %{public}d bytes",
                 g_playResultTotalSize);

    return arrayBuffer;
}

// Register callback for playback finish notification
napi_value RegisterPlaybackFinishCallback(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RegisterPlaybackFinishCallback start");

    size_t argc = ARG_COUNT_1;
    napi_value *argv = new napi_value[argc];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < MIN_ARGS_1) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "audioEditTest RegisterPlaybackFinishCallback: Invalid arguments, status=%{public}d", status);
        return nullptr;
    }

    // Clean up existing threadsafe function if any
    if (tsfnBoolean != nullptr) {
        napi_release_threadsafe_function(tsfnBoolean, napi_tsfn_release);
        tsfnBoolean = nullptr;
    }

    // Create a name for the threadsafe function
    napi_value resourceName;
    napi_create_string_utf8(env, "PlaybackFinishCallback", NAPI_AUTO_LENGTH, &resourceName);

    // Create the threadsafe function
    status =
        napi_create_threadsafe_function(env, argv[0], nullptr, resourceName, TSFN_INITIAL_QUEUE_SIZE, TSFN_THREAD_COUNT,
                                        nullptr, nullptr, nullptr, CallBoolThread, &tsfnBoolean);
    if (status != napi_ok) {
        OH_LOG_Print(
            LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "audioEditTest RegisterPlaybackFinishCallback: Failed to create threadsafe function, status=%{public}d",
            status);
        return nullptr;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest RegisterPlaybackFinishCallback: Success");
    delete[] argv;
    return nullptr;
}

// Unregister playback finish callback
napi_value UnregisterPlaybackFinishCallback(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest UnregisterPlaybackFinishCallback start");

    if (tsfnBoolean != nullptr) {
        napi_release_threadsafe_function(tsfnBoolean, napi_tsfn_release);
        tsfnBoolean = nullptr;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "audioEditTest UnregisterPlaybackFinishCallback: Success");
    }

    return nullptr;
}