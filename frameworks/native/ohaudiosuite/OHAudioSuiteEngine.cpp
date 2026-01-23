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

#ifndef LOG_TAG
#define LOG_TAG "OHAudioSuiteEngine"
#endif

#include <string>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteEngine.h"
#include "audio_suite_manager.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "audio_suite_capabilities.h"

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;

const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DEFAULT = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_BALLADS = {3, 5, 2, -4, 1, 2, -3, 1, 4, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CHINESE_STYLE = {0, 0, 2, 0, 0, 4, 4, 2, 2, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CLASSICAL = {2, 3, 2, 1, 0, 0, -5, -5, -5, -6};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DANCE_MUSIC = {4, 3, 2, -3, 0, 0, 5, 4, 2, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_JAZZ = {2, 0, 2, 3, 6, 5, -1, 3, 4, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_POP = {5, 2, 1, -1, -5, -5, -2, 1, 2, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_RB = {1, 4, 5, 3, -2, -2, 2, 3, 5, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_ROCK = {6, 4, 4, 2, 0, 1, 3, 3, 5, 4};

static OHAudioSuiteEngine *ConvertAudioSuiteEngine(OH_AudioSuiteEngine *audioSuiteEngine)
{
    return (OHAudioSuiteEngine *)audioSuiteEngine;
}

static OHAudioSuitePipeline *ConvertAudioSuitePipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    return (OHAudioSuitePipeline *)audioSuitePipeline;
}

static OHAudioNode *ConvertAudioNode(OH_AudioNode *audioNode)
{
    return (OHAudioNode *)audioNode;
}

static OHAudioSuiteNodeBuilder *ConvertAudioSuitBuilder(OH_AudioNodeBuilder *builder)
{
    return (OHAudioSuiteNodeBuilder *)builder;
}

static OHOS::AudioStandard::AudioSuite::AudioDataArray *ConvertAudioDataArray(OH_AudioDataArray *audioDataArray)
{
    return (OHOS::AudioStandard::AudioSuite::AudioDataArray *)audioDataArray;
}

static OH_AudioSuite_Result ConvertError(int32_t err)
{
    if (err == OHOS::AudioStandard::SUCCESS) {
        return AUDIOSUITE_SUCCESS;
    } else if (err == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    } else if (err == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        return AUDIOSUITE_ERROR_INVALID_STATE;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_ENGINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_ENGINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_NODE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_NODE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT;
    } else if (err == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS) {
        return AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS;
    } else if (err == OHOS::AudioStandard::ERR_MEMORY_ALLOC_FAILED) {
        return AUDIOSUITE_ERROR_MEMORY_ALLOC_FAILED;
    } else if (err == (int32_t)AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING) {
        return AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_TIMEOUT) {
        return AUDIOSUITE_ERROR_TIMEOUT;
    }
    return AUDIOSUITE_ERROR_SYSTEM;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Create(OH_AudioSuiteEngine **audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Create suiteEngine audioSuiteEngine is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_STATE, "Get suiteEngine suiteEngine is nullptr");

    int32_t ret = suiteEngine->CreateEngine();
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOSUITE_SUCCESS,
        ConvertError(ret), "Create suiteEngine failed, ret is %{public}d.", ret);

    *audioSuiteEngine = (OH_AudioSuiteEngine *)suiteEngine;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Destroy(OH_AudioSuiteEngine *audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy audioSuiteEngine is nullptr");
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy suiteEngine is nullptr");
    if (suiteEngine != OHAudioSuiteEngine::GetInstance()) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    }
    int32_t error = suiteEngine->DestroyEngine();
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreatePipeline(
    OH_AudioSuiteEngine *audioSuiteEngine,
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode workMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline audioSuiteEngine is nullptr");
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->CreatePipeline(audioSuitePipeline, workMode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyPipeline suiteEngine is nullptr");
    int32_t error = suiteEngine->DestroyPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StartPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StartPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "StartPipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->StartPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StopPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StopPipeline pipeline is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "StopPipeline suiteEngine is nullptr");

    int32_t error = suiteEngine->StopPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetPipelineState(
    OH_AudioSuitePipeline *audioSuitePipeline, OH_AudioSuite_PipelineState *pipelineState)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetPipelineState pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(pipelineState != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetPipelineState pipelineState is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetPipelineState suiteEngine is nullptr");

    int32_t error = suiteEngine->GetPipelineState(pipeline, pipelineState);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_RenderFrame(OH_AudioSuitePipeline *audioSuitePipeline,
    void *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame responseSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(requestFrameSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame requestFrameSize is zero");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "RenderFrame suiteEngine is nullptr");
    int32_t error = suiteEngine->RenderFrame(pipeline,
        static_cast<uint8_t *>(audioData), requestFrameSize, responseSize, finishedFlag);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_MultiRenderFrame(OH_AudioSuitePipeline *audioSuitePipeline,
    OH_AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    OHOS::AudioStandard::AudioSuite::AudioDataArray *dataFrame = ConvertAudioDataArray(audioDataArray);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame audioDataArray is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame responseSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame->audioDataArray != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame audioDataArray is nullptr");
    CHECK_AND_RETURN_RET_LOG(dataFrame->requestFrameSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame requestFrameSize is zero");
    CHECK_AND_RETURN_RET_LOG(dataFrame->arraySize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MultiRenderFrame arraySize is zero");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "MultiRenderFrame suiteEngine is nullptr");
    int32_t error = suiteEngine->MultiRenderFrame(pipeline,
        dataFrame, responseSize, finishedFlag);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreateNode(
    OH_AudioSuitePipeline *audioSuitePipeline, OH_AudioNodeBuilder *builder, OH_AudioNode **audioNode)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode pipeline is nullptr");
    OHAudioSuiteNodeBuilder  *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode builder is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "CreateNode suiteEngine is nullptr");

    int32_t error = suiteEngine->CreateNode(pipeline, nodeBuilder, audioNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyNode(OH_AudioNode *audioNode)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyNode node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyNode suiteEngine is nullptr");
        
    int32_t error = suiteEngine->DestroyNode(node);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetNodeBypassStatus(
    OH_AudioNode *audioNode, bool *bypassStatus)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeBypassStatus node is nullptr");
    CHECK_AND_RETURN_RET_LOG(bypassStatus != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeBypassStatus audioNodeEnable is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetNodeBypassStatus suiteEngine is nullptr");

    int32_t error = suiteEngine->GetNodeBypassStatus(node, bypassStatus);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_BypassEffectNode(OH_AudioNode *audioNode, bool bypass)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "BypassEffectNode node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyNode suiteEngine is nullptr");

    int32_t error = suiteEngine->BypassEffectNode(node, bypass);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetAudioFormat(OH_AudioNode *audioNode, OH_AudioFormat *audioFormat)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat node is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat audioFormat is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetAudioFormat suiteEngine is nullptr");

    int32_t error = suiteEngine->SetAudioFormat(node, audioFormat);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_ConnectNodes(
    OH_AudioNode* sourceAudioNode, OH_AudioNode* destAudioNode)
{
    OHAudioNode *srcNode = ConvertAudioNode(sourceAudioNode);
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "ConnectNodes srcNode is nullptr");
    OHAudioNode *destNode = ConvertAudioNode(destAudioNode);
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "ConnectNodes destNode is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "ConnectNodes suiteEngine is nullptr");

    int32_t error = suiteEngine->ConnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DisconnectNodes(OH_AudioNode *sourceAudioNode, OH_AudioNode *destAudioNode)
{
    OHAudioNode *srcNode = ConvertAudioNode(sourceAudioNode);
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisConnectNodes srcNode is nullptr");
    OHAudioNode *destNode = ConvertAudioNode(destAudioNode);
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisConnectNodes destNode is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DisConnectNodes suiteEngine is nullptr");

    int32_t error = suiteEngine->DisConnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported)
{
    AUDIO_INFO_LOG("IsNodeTypeSupported enter.");
    using namespace OHOS::AudioStandard::AudioSuite;
    CHECK_AND_RETURN_RET_LOG(
        isSupported != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "isSupported is nullptr.");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(
        suiteEngine != nullptr, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "isNodeTypeSupported suiteEngine is nullptr");
    int32_t error = suiteEngine->IsNodeTypeSupported(nodeType, isSupported);
    AUDIO_INFO_LOG("IsNodeTypeSupported leave with code: %{public}d.", error);
    return ConvertError(error);
}

template <typename Getter, typename... ParamTypes>
OH_AudioSuite_Result TemplateGetter(
    OH_AudioNode *audioNode, Getter getter, const char *getterName, ParamTypes*... params)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s node is nullptr", getterName);
    for (auto param : {params...}) {
        CHECK_AND_RETURN_RET_LOG(
            param != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s param is nullptr", getterName);
    }

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(
        suiteEngine != nullptr, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "%{public}s suiteEngine is nullptr", getterName);
    CHECK_AND_RETURN_RET_LOG(
        getter != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s getter is nullptr", getterName);
    int32_t error = (suiteEngine->*getter)(node, params...);
    return ConvertError(error);
}

template <typename Setter, typename... ParamTypes>
OH_AudioSuite_Result TemplateSetter(
    OH_AudioNode *audioNode, Setter setter, const char *setterName, ParamTypes... params)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s node is nullptr", setterName);
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(
        suiteEngine != nullptr, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "%{public}s suiteEngine is nullptr", setterName);
    int32_t error = (suiteEngine->*setter)(node, params...);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetEnvironmentType(
    OH_AudioNode *audioNode, OH_EnvironmentType *environmentType)
{
    return TemplateGetter(audioNode, &OHAudioSuiteEngine::GetEnvironmentType, "GetEnvironmentType", environmentType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetSoundFieldType(
    OH_AudioNode *audioNode, OH_SoundFieldType *soundFieldType)
{
    return TemplateGetter(audioNode, &OHAudioSuiteEngine::GetSoundFieldType, "GetSoundFieldType", soundFieldType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(
    OH_AudioNode *audioNode, OH_EqualizerFrequencyBandGains *frequencyBandGains)
{
    return TemplateGetter(audioNode,
        &OHAudioSuiteEngine::GetEqualizerFrequencyBandGains, "GetEqualizerFrequencyBandGains", frequencyBandGains);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetVoiceBeautifierType(
    OH_AudioNode *audioNode, OH_VoiceBeautifierType *voiceBeautifierType)
{
    return TemplateGetter(
        audioNode, &OHAudioSuiteEngine::GetVoiceBeautifierType, "GetVoiceBeautifierType", voiceBeautifierType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetSpaceRenderPositionParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderPositionParams *position)
{
    return TemplateGetter(
        audioNode, &OHAudioSuiteEngine::GetSpaceRenderPositionParams, "GetSpaceRenderPositionParams", position);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetSpaceRenderRotationParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderRotationParams *rotation)
{
    return TemplateGetter(
        audioNode, &OHAudioSuiteEngine::GetSpaceRenderRotationParams, "GetSpaceRenderRotationParams", rotation);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetSpaceRenderExtensionParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderExtensionParams *extension)
{
    return TemplateGetter(
        audioNode, &OHAudioSuiteEngine::GetSpaceRenderExtensionParams, "GetSpaceRenderExtensionParams", extension);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetPureVoiceChangeOption(
    OH_AudioNode *audioNode, OH_AudioSuite_PureVoiceChangeOption *option)
{
    return TemplateGetter(audioNode, &OHAudioSuiteEngine::GetPureVoiceChangeOption, "GetPureVoiceChangeOption", option);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetGeneralVoiceChangeType(
    OH_AudioNode *audioNode, OH_AudioSuite_GeneralVoiceChangeType *type)
{
    return TemplateGetter(audioNode, &OHAudioSuiteEngine::GetGeneralVoiceChangeType, "GetGeneralVoiceChangeType", type);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_GetTempoAndPitch(
    OH_AudioNode *audioNode, float *speed, float *pitch)
{
    return TemplateGetter(audioNode, &OHAudioSuiteEngine::GetTempoAndPitch, "GetTempoAndPitch", speed, pitch);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEnvironmentType(OH_AudioNode *audioNode, OH_EnvironmentType environmentType)
{
    return TemplateSetter(audioNode, &OHAudioSuiteEngine::SetEnvironmentType, "SetEnvironmentType", environmentType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSoundFieldType(OH_AudioNode *audioNode, OH_SoundFieldType soundFieldType)
{
    return TemplateSetter(audioNode, &OHAudioSuiteEngine::SetSoundFieldType, "SetSoundFieldType", soundFieldType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(
    OH_AudioNode *audioNode, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    return TemplateSetter(audioNode,
        &OHAudioSuiteEngine::SetEqualizerFrequencyBandGains, "SetEqualizerFrequencyBandGains", frequencyBandGains);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetVoiceBeautifierType(
    OH_AudioNode *audioNode, OH_VoiceBeautifierType voiceBeautifierType)
{
    return TemplateSetter(
        audioNode, &OHAudioSuiteEngine::SetVoiceBeautifierType, "SetVoiceBeautifierType", voiceBeautifierType);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSpaceRenderPositionParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderPositionParams position)
{
    return TemplateSetter(
        audioNode, &OHAudioSuiteEngine::SetSpaceRenderPositionParams, "SetSpaceRenderPositionParams", position);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSpaceRenderRotationParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderRotationParams rotation)
{
    return TemplateSetter(
        audioNode, &OHAudioSuiteEngine::SetSpaceRenderRotationParams, "SetSpaceRenderRotationParams", rotation);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSpaceRenderExtensionParams(
    OH_AudioNode *audioNode, OH_AudioSuite_SpaceRenderExtensionParams extension)
{
    return TemplateSetter(
        audioNode, &OHAudioSuiteEngine::SetSpaceRenderExtensionParams, "SetSpaceRenderExtensionParams", extension);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetPureVoiceChangeOption(
    OH_AudioNode *audioNode, OH_AudioSuite_PureVoiceChangeOption option)
{
    return TemplateSetter(audioNode, &OHAudioSuiteEngine::SetPureVoiceChangeOption, "SetPureVoiceChangeOption", option);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetGeneralVoiceChangeType(
    OH_AudioNode *audioNode, OH_AudioSuite_GeneralVoiceChangeType type)
{
    return TemplateSetter(audioNode, &OHAudioSuiteEngine::SetGeneralVoiceChangeType, "SetGeneralVoiceChangeType", type);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetTempoAndPitch(
    OH_AudioNode *audioNode, float speed, float pitch)
{
    return TemplateSetter(audioNode, &OHAudioSuiteEngine::SetTempoAndPitch, "SetTempoAndPitch", speed, pitch);
}

namespace OHOS {
namespace AudioStandard {

using namespace OHOS::AudioStandard::AudioSuite;

static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MIN = -10;
static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MAX = 10;
static constexpr float SPEED_RATE_MIN = 0.5f;
static constexpr float SPEED_RATE_MAX = 10.0f;
static constexpr float PITCH_RATE_MIN = 0.1f;
static constexpr float PITCH_RATE_MAX = 5.0f;
static const float SPACE_RENDER_MIN_CART_POINT_DISTANCE = -5.0f;
static const float SPACE_RENDER_MAX_CART_POINT_DISTANCE = 5.0f;
static const float SPACE_RENDER_MIN_TIME = 2.0f;
static const float SPACE_RENDER_MAX_TIME = 40.0f;
static const int SPACE_RENDER_MIN_EXPAND_ANGLE = 0;
static const int SPACE_RENDER_MAX_EXPAND_ANGLE = 360;
static const float SPACE_RENDER_MIN_EXPAND_RADIUS = 1.0f;
static const float SPACE_RENDER_MAX_EXPAND_RADIUS = 5.0f;
static const float AUDIO_VOICE_MORPHING_PITCH_MIN = 0.3f;
static const float AUDIO_VOICE_MORPHING_PITCH_MAX = 3.0f;
static const float AUDIO_VOICE_MORPHING_PITCH_DEFAULT = 0.0f;

int32_t OHSuiteInputNodeRequestDataCallBack::OnRequestDataCallBack(
    void *audioData, int32_t audioDataSize, bool *finished)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, 0, "OnRequestDataCallBack callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, 0, "OnRequestDataCallBack OH_audioNode is nullptr");

    return callback_(audioNode_, userData_, audioData, audioDataSize, finished);
}

OHAudioSuiteEngine::~OHAudioSuiteEngine()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::unordered_set<OHAudioSuitePipeline*> tempPipelines(pipelines_.begin(), pipelines_.end());
    for (auto pipeline : tempPipelines) {
        RemovePipeline(pipeline);
    }
    pipelines_.clear();
}

OHAudioSuiteEngine *OHAudioSuiteEngine::GetInstance()
{
    static OHAudioSuiteEngine manager;
    return &manager;
}

int32_t OHAudioSuiteEngine::CreateEngine() const
{
    return IAudioSuiteManager::GetAudioSuiteManager().Init();
}

int32_t OHAudioSuiteEngine::DestroyEngine() const
{
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyEngine DeInit failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::CreatePipeline(
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode ohWorkMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr,
        ERR_INVALID_PARAM, "CreatePipeline pipeline is nullptr");

    uint32_t pipelineId = INVALID_PIPELINE_ID;
    PipelineWorkMode workMode = static_cast<PipelineWorkMode>(ohWorkMode);
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().CreatePipeline(pipelineId, workMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreatePipeline failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(pipelineId != INVALID_PIPELINE_ID, ret, "CreatePipeline failed, pipelineId invailed");

    OHAudioSuitePipeline *audioPipeline = new OHAudioSuitePipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "CreatePipeline pipeline failed, malloc failed.");
    *audioSuitePipeline = (OH_AudioSuitePipeline *)audioPipeline;
    AddPipeline(audioPipeline);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "DestroyPipeline failed, audioPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsPipelineExists(audioPipeline), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "OHAudioSuiteEngine::The pipeline does not exist.");
    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyPipeline failed, ret = %{public}d.", ret);
    RemovePipeline(audioPipeline);
    return ret;
}

int32_t OHAudioSuiteEngine::StartPipeline(OHAudioSuitePipeline *audioPipeline) const
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "StartPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().StartPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::StopPipeline(OHAudioSuitePipeline *audioPipeline) const
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "StopPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().StopPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StopPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetPipelineState(OHAudioSuitePipeline *audioPipeline, OH_AudioSuite_PipelineState *state)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "GetPipelineState failed, audioPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(state != nullptr, ERR_INVALID_PARAM, "GetPipelineState failed, state is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    AudioSuitePipelineState pipelineState = PIPELINE_STOPPED;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetPipelineState(pipelineId, pipelineState);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPipelineState failed, ret = %{public}d.", ret);

    *state = static_cast<OH_AudioSuite_PipelineState>(pipelineState);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::RenderFrame(OHAudioSuitePipeline *audioPipeline,
    uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::RenderFrame Start");
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "RenderFrame failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();

    AUDIO_INFO_LOG("OHAudioSuiteEngine::RenderFrame enter start.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().RenderFrame(
        pipelineId, audioData, requestFrameSize, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::MultiRenderFrame(OHAudioSuitePipeline *audioPipeline,
    AudioSuite::AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::MultiRenderFrame Start");
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "MultiRenderFrame failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();

    AUDIO_INFO_LOG("OHAudioSuiteEngine::MultiRenderFrame enter start.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().MultiRenderFrame(
        pipelineId, audioDataArray, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MultiRenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::CreateNode(
    OHAudioSuitePipeline *audioSuitePipeline, OHAudioSuiteNodeBuilder *builder, OH_AudioNode **audioNode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr, ERR_INVALID_PARAM,
        "CreateNode failed, audioSuitePipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(builder != nullptr, ERR_INVALID_PARAM, "CreateNode failed, builder is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioNode != nullptr, ERR_INVALID_PARAM, "CreateNode audioNode is nullptr");

    uint32_t pipelineId = audioSuitePipeline->GetPipelineId();
    AudioNodeBuilder nodeCfg;
    nodeCfg.nodeType = builder->GetNodeType();
    if (builder->IsSetFormat()) {
        nodeCfg.nodeFormat = builder->GetNodeFormat();
    }

    if (builder->GetNodeType() == NODE_TYPE_INPUT) {
        CHECK_AND_RETURN_RET_LOG(builder->IsSetRequestDataCallback() && builder->IsSetFormat(),
            static_cast<int32_t>(AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING),
            "Create input Node must set RequestDataCallback and audio format.");
    } else if (builder->GetNodeType() == NODE_TYPE_OUTPUT) {
        CHECK_AND_RETURN_RET_LOG(builder->IsSetFormat(),
            static_cast<int32_t>(AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING),
            "Create output Node, must set aduio format.");
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetRequestDataCallback(), ERR_NOT_SUPPORTED,
            "Create output Node, can not set RequestDataCallback.");
    } else {
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetRequestDataCallback() && !builder->IsSetFormat(), ERR_NOT_SUPPORTED,
            "Create effect Node, can not set RequestDataCallback and format.");
    }

    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().CreateNode(pipelineId, nodeCfg, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateNode failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(nodeId != INVALID_PIPELINE_ID, ERR_OPERATION_FAILED, "CreateNode failed, nodeId invail");

    if (builder->IsSetRequestDataCallback()) {
        std::shared_ptr<OHSuiteInputNodeRequestDataCallBack> callback =
            std::make_shared<OHSuiteInputNodeRequestDataCallBack>(reinterpret_cast<OH_AudioNode *>(audioNode),
                builder->GetRequestDataCallback(), builder->GetCallBackUserData());
        ret = IAudioSuiteManager::GetAudioSuiteManager().SetRequestDataCallback(nodeId, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("CreateNode SetRequestDataCallback failed, ret = %{public}d.", ret);
            IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
            return ret;
        }
    }

    OHAudioNode *node = new OHAudioNode(nodeId, nodeCfg.nodeType);
    if (node == nullptr) {
        AUDIO_ERR_LOG("CreateNode failed, malloc failed.");
        IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
        return ERR_MEMORY_ALLOC_FAILED;
    }

    *audioNode = (OH_AudioNode *)node;
    audioSuitePipeline->AddNode(node);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyNode(OHAudioNode *node)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "DestroyNode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The node does not exist.");
    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed, ret = %{public}d.", ret);

    RemoveNode(node);
    return ret;
}

int32_t OHAudioSuiteEngine::GetNodeBypassStatus(OHAudioNode *node, bool *bypassStatus)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetNodeBypassStatus failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The node does not exist.");
    CHECK_AND_RETURN_RET_LOG(bypassStatus != nullptr, ERR_INVALID_PARAM,
        "GetNodeBypassStatus failed, bypassStatus is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "GetNodeBypassStatus failed, node type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    bool bypass = false;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetNodeBypassStatus(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNodeBypassStatus failed, ret = %{public}d.", ret);

    *bypassStatus = bypass;
    return ret;
}

int32_t OHAudioSuiteEngine::BypassEffectNode(OHAudioNode *node, bool bypass)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "EnableNode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The node does not exist.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "BypassEffectNode failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().BypassEffectNode(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "BypassEffectNode failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetAudioFormat(OHAudioNode *node, OH_AudioFormat *audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetAudioFormat failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The node does not exist.");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat failed, audioFormat is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() == NODE_TYPE_INPUT) || (node->GetNodeType() == NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "SetAudioFormat failed, enable type %{public}d not support.",
        static_cast<int32_t>(node->GetNodeType()));
    CHECK_AND_RETURN_RET(CheckAudioFormat(*audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    uint32_t nodeId = node->GetNodeId();
    AudioFormat format;
    format.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat->channelLayout);
    format.audioChannelInfo.numChannels = audioFormat->channelCount;
    format.encodingType = static_cast<AudioStreamEncodingType>(audioFormat->encodingType);
    format.format = static_cast<AudioSampleFormat>(audioFormat->sampleFormat);
    format.rate =  static_cast<AudioSamplingRate>(audioFormat->samplingRate);
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetAudioFormat(nodeId, format);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::ConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(srcNode), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The sec node does not exist.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(destNode), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The dest node does not exist.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().ConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::DisConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(srcNode), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The sec node does not exist.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(destNode), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
                             "OHAudioSuiteEngine::The dest node does not exist.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DisConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::ValidateNode(OHAudioNode* node, AudioNodeType expectedType, const char* funcName)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "%{public}s failed, node is nullptr.", funcName);
    CHECK_AND_RETURN_RET_LOG(IsNodeExists(node), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "%{public}s failed, node does not exist.", funcName);
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == expectedType, ERR_NOT_SUPPORTED,
        "%{public}s failed, node type = %{public}d must be %{public}d type.", funcName,
        static_cast<int32_t>(node->GetNodeType()), static_cast<int32_t>(expectedType));
    return SUCCESS;
}

template <typename T>
int32_t OHAudioSuiteEngine::SetAudioNodeProperty(OHAudioNode* node, T value, AudioNodeType nodeType,
                                                 std::function<int32_t(uint32_t, T)> setter, const char* funcName)
{
    int32_t valid = ValidateNode(node, nodeType, funcName);
    CHECK_AND_RETURN_RET_LOG(valid == SUCCESS, valid, "node operation is invalid");
    uint32_t nodeId = node->GetNodeId();
    int32_t ret = setter(nodeId, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s failed, ret = %{public}d.", funcName, ret);
    return SUCCESS;
}

template <typename T>
int32_t OHAudioSuiteEngine::GetAudioNodeProperty(OHAudioNode* node, T* outValue, AudioNodeType nodeType,
                                                 std::function<int32_t(uint32_t, T&)> getter, const char* funcName)
{
    int32_t valid = ValidateNode(node, nodeType, funcName);
    CHECK_AND_RETURN_RET_LOG(valid == SUCCESS, valid, "node operation is invalid");
    CHECK_AND_RETURN_RET_LOG(outValue != nullptr, ERR_INVALID_PARAM, "node set point is nullptr");
    uint32_t nodeId = node->GetNodeId();
    T value;
    int32_t ret = getter(nodeId, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "%{public}s failed, ret = %{public}d.", funcName, ret);
    *outValue = value;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    auto setter = [](uint32_t nodeId, OH_EqualizerFrequencyBandGains value) {
        AudioEqualizerFrequencyBandGains audioGains;
        size_t ohGainsNum = sizeof(value.gains) / sizeof(value.gains[0]);
        size_t gainsNum = sizeof(audioGains.gains) / sizeof(audioGains.gains[0]);
        for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
            audioGains.gains[idx] = value.gains[idx];
            CHECK_AND_RETURN_RET_LOG((audioGains.gains[idx] >= EQ_FREQUENCY_BAND_GAINS_MIN) &&
                                         (audioGains.gains[idx] <= EQ_FREQUENCY_BAND_GAINS_MAX),
                ERR_INVALID_PARAM, "SetEqualizerFrequencyBandGains failed, input value %{public}d not in -10, 10.",
                audioGains.gains[idx]);
        }
        return IAudioSuiteManager::GetAudioSuiteManager().SetEqualizerFrequencyBandGains(nodeId, audioGains);
    };
    return SetAudioNodeProperty(node, frequencyBandGains, NODE_TYPE_EQUALIZER,
                                std::function<int32_t(uint32_t, OH_EqualizerFrequencyBandGains)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains *frequencyBandGains)
{
    auto getter = [](uint32_t nodeId, OH_EqualizerFrequencyBandGains& value) {
        AudioEqualizerFrequencyBandGains frequency = {
            .gains = {0}
        };
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetEqualizerFrequencyBandGains(nodeId, frequency);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);
        size_t ohGainsNum = sizeof(value.gains) / sizeof(value.gains[0]);
        size_t gainsNum = sizeof(frequency.gains) / sizeof(frequency.gains[0]);
        for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
            value.gains[idx] = frequency.gains[idx];
        }
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, frequencyBandGains, NODE_TYPE_EQUALIZER,
                                std::function<int32_t(uint32_t, OH_EqualizerFrequencyBandGains&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::SetSoundFieldType(
    OHAudioNode *node, OH_SoundFieldType soundFieldType)
{
    auto setter = [](uint32_t nodeId, OH_SoundFieldType value) {
        return IAudioSuiteManager::GetAudioSuiteManager().SetSoundFieldType(
            nodeId, static_cast<SoundFieldType>(value));
    };
    return SetAudioNodeProperty(node, soundFieldType, NODE_TYPE_SOUND_FIELD,
                                std::function<int32_t(uint32_t, OH_SoundFieldType)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::SetEnvironmentType(
    OHAudioNode *node, OH_EnvironmentType environmentType)
{
    auto setter = [](uint32_t nodeId, OH_EnvironmentType value) {
        return IAudioSuiteManager::GetAudioSuiteManager().SetEnvironmentType(
            nodeId, static_cast<EnvironmentType>(value));
    };
    return SetAudioNodeProperty(node, environmentType, NODE_TYPE_ENVIRONMENT_EFFECT,
                                std::function<int32_t(uint32_t, OH_EnvironmentType)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::SetVoiceBeautifierType(
    OHAudioNode *node, OH_VoiceBeautifierType voiceBeautifierType)
{
    auto setter = [](uint32_t nodeId, OH_VoiceBeautifierType value) {
        return IAudioSuiteManager::GetAudioSuiteManager().SetVoiceBeautifierType(
            nodeId, static_cast<VoiceBeautifierType>(value));
    };
    return SetAudioNodeProperty(node, voiceBeautifierType, NODE_TYPE_VOICE_BEAUTIFIER,
                                std::function<int32_t(uint32_t, OH_VoiceBeautifierType)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::SetSpaceRenderPositionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderPositionParams position)
{
    auto setter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderPositionParams value) {
        CHECK_AND_RETURN_RET_LOG(
            value.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
                value.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
                value.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
            ERR_INVALID_PARAM, "SetSpaceRenderPositionParams failed, point distance must be in the -5.0f~5.0f");
        AudioSpaceRenderPositionParams positionParams;
        positionParams.x = value.x;
        positionParams.y = value.y;
        positionParams.z = value.z;
        return IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderPositionParams(nodeId, positionParams);
    };
    return SetAudioNodeProperty(node, position, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderPositionParams)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetSpaceRenderPositionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderPositionParams *position)
{
    auto getter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderPositionParams &value) {
        AudioSpaceRenderPositionParams positionParams;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderPositionParams(
            nodeId, positionParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderPositionParams failed, ret = %{public}d.", ret);
        value.x = positionParams.x;
        value.y = positionParams.y;
        value.z = positionParams.z;
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, position, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderPositionParams&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::SetSpaceRenderRotationParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderRotationParams rotation)
{
    auto setter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderRotationParams value) {
        CHECK_AND_RETURN_RET_LOG(
            value.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
            ERR_INVALID_PARAM, "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f");
        CHECK_AND_RETURN_RET_LOG(
            value.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
            ERR_INVALID_PARAM, "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f");
        CHECK_AND_RETURN_RET_LOG(
            value.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && value.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
            ERR_INVALID_PARAM, "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f");
        CHECK_AND_RETURN_RET_LOG(
            value.surroundTime >= SPACE_RENDER_MIN_TIME && value.surroundTime <= SPACE_RENDER_MAX_TIME,
            ERR_INVALID_PARAM, "SetSpaceRenderRotationParams failed, time must be in the 2.0f~40.0f");
        CHECK_AND_RETURN_RET_LOG(
            value.surroundDirection == OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW ||
            value.surroundDirection == OH_AudioSuite_SurroundDirection::SPACE_RENDER_CW,
            ERR_INVALID_PARAM, "SetSpaceRenderRotationParams failed, surround direction must be 0 or 1");
        AudioSpaceRenderRotationParams rotationParams;
        rotationParams.x = value.x;
        rotationParams.y = value.y;
        rotationParams.z = value.z;
        rotationParams.surroundTime = value.surroundTime;
        rotationParams.surroundDirection = static_cast<AudioSurroundDirection>(value.surroundDirection);
        return IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderRotationParams(nodeId, rotationParams);
    };
    return SetAudioNodeProperty(node, rotation, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderRotationParams)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetSpaceRenderRotationParams(
    OHAudioNode* node, OH_AudioSuite_SpaceRenderRotationParams* rotation)
{
    auto getter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderRotationParams& value) {
        AudioSpaceRenderRotationParams rotationParams;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderRotationParams(
            nodeId, rotationParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderRotationParams failed, ret = %{public}d.", ret);
        value.x = rotationParams.x;
        value.y = rotationParams.y;
        value.z = rotationParams.z;
        value.surroundTime = rotationParams.surroundTime;
        value.surroundDirection = static_cast<OH_AudioSuite_SurroundDirection>(rotationParams.surroundDirection);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, rotation, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderRotationParams&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::SetSpaceRenderExtensionParams(
    OHAudioNode* node, OH_AudioSuite_SpaceRenderExtensionParams extension)
{
    auto setter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderExtensionParams value) {
        CHECK_AND_RETURN_RET_LOG(
            value.extRadius >= SPACE_RENDER_MIN_EXPAND_RADIUS && value.extRadius <= SPACE_RENDER_MAX_EXPAND_RADIUS,
            ERR_INVALID_PARAM, "SetSpaceRenderExtensionParams failed, Radius must be in the 1.0f~5.0f");
        CHECK_AND_RETURN_RET_LOG(
            value.extAngle > SPACE_RENDER_MIN_EXPAND_ANGLE && value.extAngle < SPACE_RENDER_MAX_EXPAND_ANGLE,
            ERR_INVALID_PARAM, "SetSpaceRenderExtensionParams failed, Angle must be in the (1, 360).");
        AudioSpaceRenderExtensionParams extensionParams;
        extensionParams.extAngle = value.extAngle;
        extensionParams.extRadius = value.extRadius;
        return IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderExtensionParams(nodeId, extensionParams);
    };
    return SetAudioNodeProperty(node, extension, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderExtensionParams)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetSpaceRenderExtensionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderExtensionParams *extension)
{
    auto getter = [](uint32_t nodeId, OH_AudioSuite_SpaceRenderExtensionParams &value) {
        AudioSpaceRenderExtensionParams extensionParams;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderExtensionParams(
            nodeId, extensionParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderExtensionParams failed, ret = %{public}d.", ret);
        value.extRadius = extensionParams.extRadius;
        value.extAngle = extensionParams.extAngle;
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, extension, NODE_TYPE_SPACE_RENDER,
        std::function<int32_t(uint32_t, OH_AudioSuite_SpaceRenderExtensionParams&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::SetTempoAndPitch(OHAudioNode* node, float speed, float pitch)
{
    CHECK_AND_RETURN_RET_LOG(speed >= SPEED_RATE_MIN && speed <= SPEED_RATE_MAX,
        ERR_INVALID_PARAM, "SetTempoAndPitch failed, speed must be in the 0.5~10.0");
    CHECK_AND_RETURN_RET_LOG(pitch >= PITCH_RATE_MIN && pitch <= PITCH_RATE_MAX,
        ERR_INVALID_PARAM, "SetTempoAndPitch failed, pitch must be in the 0.1~5.0");
    struct TempoAndPitch {
        float speed;
        float pitch;
    } value = {speed, pitch};
    std::function<int32_t(uint32_t, TempoAndPitch)> setter =
        [](uint32_t nodeId, TempoAndPitch value) -> int32_t {
            return IAudioSuiteManager::GetAudioSuiteManager().SetTempoAndPitch(nodeId, value.speed, value.pitch);
        };
    return SetAudioNodeProperty(node, value, NODE_TYPE_TEMPO_PITCH, setter, "SetTempoAndPitch");
}

int32_t OHAudioSuiteEngine::GetTempoAndPitch(OHAudioNode* node, float* speed, float* pitch)
{
    CHECK_AND_RETURN_RET_LOG(speed != nullptr && pitch != nullptr, ERR_INVALID_PARAM,
        "GetTempoAndPitch failed, parameter is nullptr.");
    struct TempoAndPitch {
        float speed;
        float pitch;
    };
    std::function<int32_t(uint32_t, TempoAndPitch&)> getter =
        [](uint32_t nodeId, TempoAndPitch& value) -> int32_t {
            return IAudioSuiteManager::GetAudioSuiteManager().GetTempoAndPitch(
                nodeId, value.speed, value.pitch);
        };
    TempoAndPitch result = {1.0f, 1.0f};
    int32_t ret = GetAudioNodeProperty(node, &result, NODE_TYPE_TEMPO_PITCH, getter, "GetTempoAndPitch");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetTempoAndPitch failed, ret = %{public}d.", ret);
    *speed = result.speed;
    *pitch = result.pitch;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetPureVoiceChangeOption(
    OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption option)
{
    CHECK_AND_RETURN_RET_LOG(
        (option.pitch >= AUDIO_VOICE_MORPHING_PITCH_MIN && option.pitch <= AUDIO_VOICE_MORPHING_PITCH_MAX) ||
            option.pitch == AUDIO_VOICE_MORPHING_PITCH_DEFAULT,
        ERR_INVALID_PARAM,
        "SetPureVoicePitch failed, pitch must be in the 0.3f~3.0f and 0.0f");
    auto setter = [](uint32_t nodeId, OH_AudioSuite_PureVoiceChangeOption value) {
        AudioPureVoiceChangeOption optionParams;
        optionParams.optionGender = static_cast<AudioPureVoiceChangeGenderOption>(value.optionGender);
        optionParams.optionType = static_cast<AudioPureVoiceChangeType>(value.optionType);
        optionParams.pitch = static_cast<float>(value.pitch);
        return IAudioSuiteManager::GetAudioSuiteManager().SetPureVoiceChangeOption(nodeId, optionParams);
    };
    return SetAudioNodeProperty(node, option, NODE_TYPE_PURE_VOICE_CHANGE,
        std::function<int32_t(uint32_t, OH_AudioSuite_PureVoiceChangeOption)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetPureVoiceChangeOption(
    OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption* option)
{
    auto getter = [](uint32_t nodeId, OH_AudioSuite_PureVoiceChangeOption& value) {
        AudioPureVoiceChangeOption optionParams;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetPureVoiceChangeOption(nodeId, optionParams);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPureVoiceChangeOption failed, ret = %{public}d.", ret);
        value.optionGender = static_cast<OH_AudioSuite_PureVoiceChangeGenderOption>(optionParams.optionGender);
        value.optionType = static_cast<OH_AudioSuite_PureVoiceChangeType>(optionParams.optionType);
        value.pitch = static_cast<float>(optionParams.pitch);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, option, NODE_TYPE_PURE_VOICE_CHANGE,
        std::function<int32_t(uint32_t, OH_AudioSuite_PureVoiceChangeOption&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::SetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType type)
{
    auto setter = [](uint32_t nodeId, OH_AudioSuite_GeneralVoiceChangeType value) {
        return IAudioSuiteManager::GetAudioSuiteManager().SetGeneralVoiceChangeType(
            nodeId, static_cast<AudioGeneralVoiceChangeType>(value));
    };
    return SetAudioNodeProperty(node, type, NODE_TYPE_GENERAL_VOICE_CHANGE,
        std::function<int32_t(uint32_t, OH_AudioSuite_GeneralVoiceChangeType)>(setter), __func__);
}

int32_t OHAudioSuiteEngine::GetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType* type)
{
    auto getter = [](uint32_t nodeId, OH_AudioSuite_GeneralVoiceChangeType& value) {
        AudioGeneralVoiceChangeType typeParam;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetGeneralVoiceChangeType(nodeId, typeParam);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetGeneralVoiceChangeType failed, ret = %{public}d.", ret);
        value = static_cast<OH_AudioSuite_GeneralVoiceChangeType>(typeParam);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, type, NODE_TYPE_GENERAL_VOICE_CHANGE,
        std::function<int32_t(uint32_t, OH_AudioSuite_GeneralVoiceChangeType&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::GetEnvironmentType(OHAudioNode *node, OH_EnvironmentType *environmentType)
{
    auto getter = [](uint32_t nodeId, OH_EnvironmentType& value) {
        EnvironmentType environment = AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetEnvironmentType(nodeId, environment);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEnvironmentType failed, ret = %{public}d.", ret);
        value = static_cast<OH_EnvironmentType>(environment);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, environmentType, NODE_TYPE_ENVIRONMENT_EFFECT,
                                std::function<int32_t(uint32_t, OH_EnvironmentType&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::GetSoundFieldType(OHAudioNode *node, OH_SoundFieldType *soundFieldType)
{
    auto getter = [](uint32_t nodeId, OH_SoundFieldType& value) {
        SoundFieldType soundField = AUDIO_SUITE_SOUND_FIELD_CLOSE;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetSoundFieldType(nodeId, soundField);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSoundFieldType failed, ret = %{public}d.", ret);
        value = static_cast<OH_SoundFieldType>(soundField);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, soundFieldType, NODE_TYPE_SOUND_FIELD,
                                std::function<int32_t(uint32_t, OH_SoundFieldType&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::GetVoiceBeautifierType(OHAudioNode *node,
    OH_VoiceBeautifierType *voiceBeautifierType)
{
    auto getter = [](uint32_t nodeId, OH_VoiceBeautifierType& value) {
        VoiceBeautifierType voiceBeautifier = AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR;
        int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetVoiceBeautifierType(nodeId, voiceBeautifier);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetVoiceBeautifierType failed, ret = %{public}d.", ret);
        value = static_cast<OH_VoiceBeautifierType>(voiceBeautifier);
        return SUCCESS;
    };
    return GetAudioNodeProperty(node, voiceBeautifierType, NODE_TYPE_VOICE_BEAUTIFIER,
                                std::function<int32_t(uint32_t, OH_VoiceBeautifierType&)>(getter), __func__);
}

int32_t OHAudioSuiteEngine::IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported)
{
    CHECK_AND_RETURN_RET_LOG(
        isSupported != nullptr, ERR_INVALID_PARAM, "IsNodeTypeSupported function failed, isSupported is nullptr.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().IsNodeTypeSupported(
        static_cast<AudioNodeType>(nodeType), isSupported);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "IsNodeTypeSupported function failed, ret = %{public}d.", ret);
    return ret;
}

void OHAudioSuiteEngine::AddPipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    pipelines_.insert(pipeline);
}

void OHAudioSuiteEngine::RemovePipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (pipeline != nullptr) {
        pipelines_.erase(pipeline);
        delete pipeline;
    }
}

bool OHAudioSuiteEngine::IsPipelineExists(OHAudioSuitePipeline *pipeline)
{
    if (pipeline == nullptr) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return pipelines_.find(pipeline) != pipelines_.end();
}

void OHAudioSuitePipeline::AddNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.insert(node);
}

bool OHAudioSuitePipeline::IsNodeExists(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nodes_.find(node) != nodes_.end();
}

void OHAudioSuitePipeline::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr) {
        nodes_.erase(node);
        delete node;
    }
}
OHAudioSuitePipeline::~OHAudioSuitePipeline()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (OHAudioNode* node : nodes_) {
        if (node != nullptr) {
            delete node;
        }
    }
    nodes_.clear();
}

bool OHAudioSuiteEngine::IsNodeExists(OHAudioNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto pipeline : pipelines_) {
        if (pipeline && node && pipeline->IsNodeExists(node)) {
            return true;
        }
    }
    return false;
}

void OHAudioSuiteEngine::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto pipeline : pipelines_) {
        if (pipeline && node && pipeline->IsNodeExists(node)) {
            pipeline->RemoveNode(node);
        }
    }
}

}  // namespace AudioStandard
}  // namespace OHOS
