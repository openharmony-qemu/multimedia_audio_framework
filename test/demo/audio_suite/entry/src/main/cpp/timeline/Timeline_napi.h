/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_TIMELINE_NAPI_H
#define AUDIOEDITTESTAPP_TIMELINE_NAPI_H
#include "napi/native_api.h"
#include "Timeline.h"

napi_value AddAudioTrack(napi_env env, napi_callback_info info);

napi_value DeleteAudioTrack(napi_env env, napi_callback_info info);

napi_value SetAudioTrackSilent(napi_env env, napi_callback_info info);

napi_value AddAudioAsset(napi_env env, napi_callback_info info);

napi_value UpdateAudioAsset(napi_env env, napi_callback_info info);

napi_value DeleteAudioAsset(napi_env env, napi_callback_info info);

napi_value SetAudioAssetStartTime(napi_env env, napi_callback_info info);

napi_value SetAudioAssetPcmBufferLength(napi_env env, napi_callback_info info);

napi_value AddAudioAssetEffectNode(napi_env env, napi_callback_info info);

napi_value DeleteAudioAssetEffectNode(napi_env env, napi_callback_info info);

napi_value ClearTimeline(napi_env env, napi_callback_info info);

#endif //AUDIOEDITTESTAPP_TIMELINE_NAPI_H
