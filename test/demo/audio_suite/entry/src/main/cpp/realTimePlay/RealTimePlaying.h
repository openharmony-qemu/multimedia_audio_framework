/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_REALTIMEPLAYING_H
#define AUDIOEDITTESTAPP_REALTIMEPLAYING_H

#include "ohaudiosuite/native_audio_suite_base.h"
#include "ohaudio/native_audiostream_base.h"
#include <cstdint>
#include "napi/native_api.h"
#include <string>

extern OH_AudioRenderer *audioRenderer;

extern OH_AudioStreamBuilder *rendererBuilder;

// Real-time playback   if the rendering completed in one go
extern bool g_playFinishedFlag;

extern char *g_playAudioData;

extern int32_t g_playDataSize;

// Record or not?
extern bool g_isRecord;

// Real-time playback, used for saving audio data
// with the specific size varying according to the size of the file to be saved
extern char *g_playTotalAudioData;

// Total size of audio to be saved for real-time playback
extern int32_t g_playResultTotalSize;

extern OH_AudioDataArray* g_play_ohAudioDataArray;
 
extern uint32_t g_separationMode;

OH_AudioSuite_Result ProcessPipeline();

OH_AudioSuite_Result OneRenDerFrame(int32_t audioDataSize, int32_t *writeSize);

OH_AudioSuite_Result OneMulRenDerFrame(int32_t audioDataSize, int32_t *writeSize);

OH_AudioData_Callback_Result PlayAudioRendererOnWriteData(OH_AudioRenderer *renderer,
    void *userData, void *audioData, int32_t audioDataSize);

napi_value ModifyRenderTrack(napi_env env, napi_callback_info info);

void ReleaseExistingResources();

#endif //AUDIOEDITTESTAPP_REALTIMEPLAYING_H