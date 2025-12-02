/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
#ifndef MULTIPIPELINEEDIT_H
#define MULTIPIPELINEEDIT_H

#include "napi/native_api.h"
#include "audioEffectNode/Input.h"
#include "callback/RegisterCallback.h"
#include <./utils/Utils.h>

napi_value AudioEditNodeInitMultiPipeline(napi_env env, napi_callback_info info);
napi_value MultiAudioInAndOutInit(napi_env env, napi_callback_info info);
napi_value MultiPipelineEnvPrepare(napi_env env, napi_callback_info info);
napi_value MultiSetFormat(napi_env env, napi_callback_info info);
napi_value MultiSaveFileBuffer(napi_env env, napi_callback_info info);
napi_value MultiGetSecondOutputAudio(napi_env env, napi_callback_info info);
napi_value MultiDeleteSong(napi_env env, napi_callback_info info);
napi_value DestroyMultiPipeline(napi_env env, napi_callback_info info);
napi_value MultiAudioRendererInit(napi_env env, napi_callback_info info);
napi_value MultiAudioRendererStart(napi_env env, napi_callback_info info);
napi_value MultiRealTimeSaveFileBuffer(napi_env env, napi_callback_info info);
napi_value GetAutoTestProcess(napi_env env, napi_callback_info info);

#endif