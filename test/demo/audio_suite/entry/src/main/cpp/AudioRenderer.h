/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIORENDERER_H
#define AUDIORENDERER_H

#include "napi/native_api.h"
#include "ohaudio/native_audiostream_base.h"

// Audio renderer parameters structure
struct AudioRendererParams {
    int32_t sampleRate;
    int32_t channels;
    int32_t bitDepth;
    int32_t bitDepthMode;
};

// Real-time playback NAPI methods for single-pipeline audio rendering
// Call sequence: initAudioRenderer -> registerPlaybackFinishCallback -> setRecordFlag ->
// startAudioRenderer -> (wait for callback) -> getRecordedAudioData -> stopAudioRenderer -> releaseAudioRenderer

napi_value InitAudioRenderer(napi_env env, napi_callback_info info);
napi_value StartAudioRenderer(napi_env env, napi_callback_info info);
napi_value StopAudioRenderer(napi_env env, napi_callback_info info);
napi_value ReleaseAudioRenderer(napi_env env, napi_callback_info info);
napi_value SetRecordFlag(napi_env env, napi_callback_info info);
napi_value GetRecordedAudioData(napi_env env, napi_callback_info info);
napi_value RegisterPlaybackFinishCallback(napi_env env, napi_callback_info info);
napi_value UnregisterPlaybackFinishCallback(napi_env env, napi_callback_info info);

#endif // AUDIORENDERER_H