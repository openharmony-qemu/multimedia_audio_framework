/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
#ifndef MULTIPIPELINEEDIT_H
#define MULTIPIPELINEEDIT_H
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "napi/native_api.h"
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
#include "NodeManager.h"
#include "PipelineManager.h"
#include "callback/RegisterCallback.h"
#include "audioSuiteError/AudioSuiteError.h"

#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <./utils/Utils.h>
// 对外napi接口
napi_value AudioEditNodeInitMultiPipeline(napi_env env, napi_callback_info info);
napi_value multiAudioInAndOutInit(napi_env env, napi_callback_info info);
napi_value multiPipelineEnvPrepare(napi_env env, napi_callback_info info);
napi_value multiSetFormat(napi_env env, napi_callback_info info);
napi_value multiSetEqualizerMode(napi_env env, napi_callback_info info);
napi_value multiSetEqualizerFrequencyBandGains(napi_env env, napi_callback_info info);
napi_value multiStartFieldEffect(napi_env env, napi_callback_info info);
napi_value multiStartEnvEffect(napi_env env, napi_callback_info info);
napi_value multiAddAudioSeparation(napi_env env, napi_callback_info info);
napi_value multiAddNoiseReduction(napi_env env, napi_callback_info info);
napi_value multiStartVBEffect(napi_env env, napi_callback_info info);
napi_value multiSaveFileBuffer(napi_env env, napi_callback_info info);
napi_value multiGetSecondOutputAudio(napi_env env, napi_callback_info info);
napi_value multiDeleteSong(napi_env env, napi_callback_info info);
napi_value destroyMultiPipeline(napi_env env, napi_callback_info info);

// 实现多管线功能函数
void MultiStoreTotalBuffToMap(const char *totalBuff, size_t size, const std::string &key);
OH_AudioSuite_Result MultiPipelineRenderFrame();
int32_t MultiWriteDataCallBack(OH_AudioNode *audioNode, void *userData, void *audioData, int32_t audioDataSize,
    bool *finished);
OH_AudioSuite_Result MultiSetParamsAndWriteData(OH_AudioNodeBuilder *builder,
    std::string inputId, OH_AudioNode_Type type);
void MultiCreateInputNode(napi_env env, const std::string &inputId, napi_value &napiValue,
    OH_AudioSuite_Result &result);
void MultiUpdateInputNode(const std::string &inputId, unsigned int channels, unsigned int sampleRate,
    unsigned int bitsPerSample, napi_value &napiValue, OH_AudioSuite_Result &result);
void MultiReadTrackSamples(OH_AVDemuxer *demuxer, uint32_t trackIndex, int bufferSize, std::atomic<bool> &isEnd,
    std::shared_ptr<PipelineManager> threadPipelineManager);
bool MultiGetAudioProperties(OH_AVFormat *trackFormat, int32_t &sampleRate, int32_t &channels, int32_t &bitsPerSample);
void MultiRunAudioThread(OH_AVDemuxer *demuxer, int32_t fileLength);
void MultiManageExistingOutputNodes(const std::string &inputId,
    const std::string &mixerId, OH_AudioSuite_Result &result, std::vector<Node> outPutNodes);
void MultiCreateAndConnectOutputNodes(const std::string &inputId, const std::string &outputId,
    OH_AudioSuite_Result &result);
void MultiManageOutputNodes(napi_env env, const std::string &inputId, const std::string &outputId,
    const std::string &mixerId, OH_AudioSuite_Result &result);
int MultiAddEffectNodeToNodeManager(std::string &inputNodeId, std::string &effectNodeId);
Node MultiCreateNodeByType(std::string uuid, OH_AudioNode_Type nodeType);

#endif