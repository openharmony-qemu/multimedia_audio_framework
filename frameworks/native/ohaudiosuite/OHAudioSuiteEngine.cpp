/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <new>
#include "securec.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_common.h"
#include "OHAudioSuiteEngine.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "stream_dfx_manager.h"

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;
using OHOS::AudioStandard::AudioSuite::AudioMetaFrame;

const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DEFAULT = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_BALLADS = {3, 5, 2, -4, 1, 2, -3, 1, 4, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CHINESE_STYLE = {0, 0, 2, 0, 0, 4, 4, 2, 2, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_CLASSICAL = {2, 3, 2, 1, 0, 0, -5, -5, -5, -6};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_DANCE_MUSIC = {4, 3, 2, -3, 0, 0, 5, 4, 2, 0};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_JAZZ = {2, 0, 2, 3, 6, 5, -1, 3, 4, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_POP = {5, 2, 1, -1, -5, -5, -2, 1, 2, 4};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_RB = {1, 4, 5, 3, -2, -2, 2, 3, 5, 5};
const OH_EqualizerFrequencyBandGains OH_EQUALIZER_PARAM_ROCK = {6, 4, 4, 2, 0, 1, 3, 3, 5, 4};

static constexpr size_t SYSTEM_NODE_MAX_PARAM_SIZE = 8192;

static OHAudioSuiteEngine *ConvertAudioSuiteEngine(OH_AudioSuiteEngine *audioSuiteEngine)
{
    return reinterpret_cast<OHAudioSuiteEngine *>(audioSuiteEngine);
}

static OHAudioSuitePipeline *ConvertAudioSuitePipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    return reinterpret_cast<OHAudioSuitePipeline *>(audioSuitePipeline);
}

static OHAudioNode *ConvertAudioNode(OH_AudioNode *audioNode)
{
    return reinterpret_cast<OHAudioNode *>(audioNode);
}

static OHAudioSuiteNodeBuilder *ConvertAudioSuitBuilder(OH_AudioNodeBuilder *builder)
{
    return reinterpret_cast<OHAudioSuiteNodeBuilder *>(builder);
}

static OHOS::AudioStandard::AudioSuite::AudioDataArray *ConvertAudioDataArray(OH_AudioDataArray *audioDataArray)
{
    return reinterpret_cast<OHOS::AudioStandard::AudioSuite::AudioDataArray *>(audioDataArray);
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

    int32_t error = OHAudioSuiteEngine::GetInstance().CreateEngine();
    CHECK_AND_RETURN_RET_LOG(error == OHOS::AudioStandard::SUCCESS,
        ConvertError(error), "Create suiteEngine failed, error is %{public}d.", error);

    *audioSuiteEngine = (OH_AudioSuiteEngine *)&OHAudioSuiteEngine::GetInstance();
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Destroy(OH_AudioSuiteEngine *audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy audioSuiteEngine is nullptr");
    CHECK_AND_RETURN_RET_LOG(ConvertAudioSuiteEngine(audioSuiteEngine) == &OHAudioSuiteEngine::GetInstance(),
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy audioSuiteEngine is invalid");

    int32_t error = OHAudioSuiteEngine::GetInstance().DestroyEngine();
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreatePipeline(
    OH_AudioSuiteEngine *audioSuiteEngine,
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode workMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline audioSuiteEngine is nullptr");
    CHECK_AND_RETURN_RET_LOG(ConvertAudioSuiteEngine(audioSuiteEngine) == &OHAudioSuiteEngine::GetInstance(),
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline audioSuiteEngine is invalid");

    int32_t error = OHAudioSuiteEngine::GetInstance().CreatePipeline(audioSuitePipeline, workMode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyPipeline pipeline is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().DestroyPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StartPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StartPipeline pipeline is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().StartPipeline(pipeline);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_StopPipeline(OH_AudioSuitePipeline *audioSuitePipeline)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "StopPipeline pipeline is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().StopPipeline(pipeline);
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

    int32_t error = OHAudioSuiteEngine::GetInstance().GetPipelineState(pipeline, pipelineState);
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

    int32_t error = OHAudioSuiteEngine::GetInstance().RenderFrame(pipeline,
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

    int32_t error = OHAudioSuiteEngine::GetInstance().MultiRenderFrame(pipeline,
        dataFrame, responseSize, finishedFlag);
    return ConvertError(error);
}

int32_t OH_AudioSuiteEngine_MetaRenderFrame(OH_AudioSuitePipeline *audioSuitePipeline,
    OH_AudioSuite_MetaFrame *metaFrame, int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(metaFrame != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame metaFrame is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseAudioSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame responseAudioSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(responseMetaSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame responseMetaSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(metaFrame->audioData != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame audioData is nullptr");
    CHECK_AND_RETURN_RET_LOG(metaFrame->audioDataSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame audioDataSize is zero");
    CHECK_AND_RETURN_RET_LOG(metaFrame->metaData != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame metaData is nullptr");
    CHECK_AND_RETURN_RET_LOG(metaFrame->metaDataSize > 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "MetaRenderFrame metaDataSize is zero");

    AudioMetaFrame audioMetaFrame;
    audioMetaFrame.audioData = metaFrame->audioData;
    audioMetaFrame.audioDataSize = metaFrame->audioDataSize;
    audioMetaFrame.metaData = metaFrame->metaData;
    audioMetaFrame.metaDataSize = metaFrame->metaDataSize;

    int32_t error = OHAudioSuiteEngine::GetInstance().MetaRenderFrame(
        pipeline, audioMetaFrame, responseAudioSize, responseMetaSize, finishedFlag);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreateNode(
    OH_AudioSuitePipeline *audioSuitePipeline, OH_AudioNodeBuilder *builder, OH_AudioNode **audioNode)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode pipeline is nullptr");
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreateNode builder is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().CreateNode(pipeline, nodeBuilder, audioNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DestroyNode(OH_AudioNode *audioNode)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DestroyNode node is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().DestroyNode(node);
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

    int32_t error = OHAudioSuiteEngine::GetInstance().GetNodeBypassStatus(node, bypassStatus);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_BypassEffectNode(OH_AudioNode *audioNode, bool bypass)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "BypassEffectNode node is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().BypassEffectNode(node, bypass);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetAudioFormat(OH_AudioNode *audioNode, OH_AudioFormat *audioFormat)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat node is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetAudioFormat audioFormat is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().SetAudioFormat(node, audioFormat);
    return ConvertError(error);
}

int32_t OH_AudioSuiteEngineSystem_SetAudioFormat(
    OH_AudioNode *audioNode, OH_AudioSuite_SystemNodeFormat *audioFormat)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetSystemAudioFormat node is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetSystemAudioFormat audioFormat is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().SetAudioFormat(node, audioFormat);
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

    int32_t error = OHAudioSuiteEngine::GetInstance().ConnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DisconnectNodes(OH_AudioNode *sourceAudioNode, OH_AudioNode *destAudioNode)
{
    OHAudioNode *srcNode = ConvertAudioNode(sourceAudioNode);
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisconnectNodes srcNode is nullptr");
    OHAudioNode *destNode = ConvertAudioNode(destAudioNode);
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "DisconnectNodes destNode is nullptr");

    int32_t error = OHAudioSuiteEngine::GetInstance().DisconnectNodes(srcNode, destNode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported)
{
    AUDIO_DEBUG_LOG("IsNodeTypeSupported enter.");
    using namespace OHOS::AudioStandard::AudioSuite;
    CHECK_AND_RETURN_RET_LOG(
        isSupported != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "isSupported is nullptr.");

    int32_t error = OHAudioSuiteEngine::GetInstance().IsNodeTypeSupported(nodeType, isSupported);
    AUDIO_DEBUG_LOG("IsNodeTypeSupported leave with code: %{public}d.", error);
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

    CHECK_AND_RETURN_RET_LOG(
        getter != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s getter is nullptr", getterName);
    int32_t error = (OHAudioSuiteEngine::GetInstance().*getter)(node, params...);
    return ConvertError(error);
}

template <typename Setter, typename... ParamTypes>
OH_AudioSuite_Result TemplateSetter(
    OH_AudioNode *audioNode, Setter setter, const char *setterName, ParamTypes... params)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "%{public}s node is nullptr", setterName);
    int32_t error = (OHAudioSuiteEngine::GetInstance().*setter)(node, params...);
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

OH_AudioSuite_Result OH_AudioSuite_PrintInfo(
    OH_AudioSuiteEngine *audioSuiteEngine, OH_AudioSuitePipeline *audioSuitePipeline, int fd)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "PrintInfo audioSuiteEngine is nullptr");
    CHECK_AND_RETURN_RET_LOG(ConvertAudioSuiteEngine(audioSuiteEngine) == &OHAudioSuiteEngine::GetInstance(),
        AUDIOSUITE_ERROR_INVALID_PARAM, "PrintInfo audioSuiteEngine is invalid");
    
    OHAudioSuitePipeline *pipeline = nullptr;
    if (audioSuitePipeline != nullptr) {
        pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
        CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "PrintInfo pipeline is nullptr");
    }

    int32_t error = OHAudioSuiteEngine::GetInstance().PrintInfo(pipeline, fd);
    return ConvertError(error);
}

int32_t OH_AudioSuiteEngineSystem_SetNodeParam(
    OH_AudioNode *audioNode, uint8_t *param, uint32_t paramSize)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetSystemNodeParam audioNode is nullptr");
    CHECK_AND_RETURN_RET_LOG(param != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetSystemNodeParam param is nullptr");
    CHECK_AND_RETURN_RET_LOG(paramSize > 0 && paramSize <= SYSTEM_NODE_MAX_PARAM_SIZE,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetSystemNodeParam paramSize is %{public}u", paramSize);

    std::vector<uint8_t> paramVector(param, param + paramSize);
    int32_t error = OHAudioSuiteEngine::GetInstance().SetSystemNodeParam(node, paramVector);
    return ConvertError(error);
}

int32_t OH_AudioSuiteEngineSystem_GetNodeParam(
    OH_AudioNode *audioNode, uint8_t *param, uint32_t paramSize)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeParam audioNode is nullptr");
    CHECK_AND_RETURN_RET_LOG(param != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeParam param is nullptr");
    CHECK_AND_RETURN_RET_LOG(paramSize > 0 && paramSize <= SYSTEM_NODE_MAX_PARAM_SIZE,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeParam paramSize is %{public}u", paramSize);

    std::vector<uint8_t> paramVector(param, param + paramSize);
    int32_t error = OHAudioSuiteEngine::GetInstance().GetSystemNodeParam(node, paramVector);
    if (error == OHOS::AudioStandard::SUCCESS) {
        size_t copySize = std::min(paramVector.size(), static_cast<size_t>(paramSize));
        int32_t ret = memcpy_s(param, paramSize, paramVector.data(), copySize);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, AUDIOSUITE_ERROR_INVALID_PARAM, "memcpy inbuffer failed err%{public}d:, "
            "paramSize:%{public}u, vectorSize:%{public}zu", ret, paramSize, paramVector.size());
    }
    return ConvertError(error);
}

int32_t OH_AudioSuiteEngineSystem_GetNodeInOutSize(
    OH_AudioNode *audioNode, uint32_t *inSize, uint32_t *outSize)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeInOutSize audioNode is nullptr");
    CHECK_AND_RETURN_RET_LOG(inSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeInOutSize inSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(outSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetSystemNodeInOutSize outSize is nullptr");

    uint32_t inSizeValue = 0;
    uint32_t outSizeValue = 0;
    int32_t error = OHAudioSuiteEngine::GetInstance().GetSystemNodeInOutSize(node, inSizeValue, outSizeValue);
    if (error == OHOS::AudioStandard::SUCCESS) {
        *inSize = inSizeValue;
        *outSize = outSizeValue;
    }
    return ConvertError(error);
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

int32_t OHSuiteInputNodeRequestDataCallBack::OnRequestDataCallBack(
    void *audioData, int32_t audioDataSize, bool *finished)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, 0, "OnRequestDataCallBack callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, 0, "OnRequestDataCallBack OH_audioNode is nullptr");

    return callback_(audioNode_, userData_, audioData, audioDataSize, finished);
}

int32_t OHSuiteInputNodeRequestMetaDataCallBack::OnRequestDataCallBack(
    AudioMetaFrame &metaFrame, int32_t* responseMetaDataSize, bool *finished)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, 0, "OnRequestDataCallBack callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, 0, "OnRequestDataCallBack OH_audioNode is nullptr");

    OH_AudioSuite_MetaFrame metaData;
    metaData.audioData = metaFrame.audioData;
    metaData.audioDataSize = metaFrame.audioDataSize;
    metaData.metaData = metaFrame.metaData;
    metaData.metaDataSize = metaFrame.metaDataSize;
    return callback_(audioNode_, userData_, &metaData, responseMetaDataSize, finished);
}

uint32_t OHAudioNode::GetNodeId() const
{
    return nodeId_;
}

AudioSuite::AudioNodeType OHAudioNode::GetNodeType() const
{
    return type_;
}

std::shared_ptr<AudioSuitePipeline> OHAudioNode::GetPipeline() const
{
    return pipeline_;
}

OHAudioSuiteEngine::~OHAudioSuiteEngine()
{
    CleanupAll();
}

OHAudioSuiteEngine &OHAudioSuiteEngine::GetInstance()
{
    static OHAudioSuiteEngine manager;
    return manager;
}

int32_t OHAudioSuiteEngine::CreateEngine()
{
    std::lock_guard<std::mutex> lock(engineMutex_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_, ERR_ILLEGAL_STATE, "CreateEngine failed, engine already initialized.");
    isInitialized_ = true;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyEngine()
{
    std::lock_guard<std::mutex> lock(engineMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERR_ILLEGAL_STATE, "DestroyEngine failed, engine not initialized.");
    CleanupAll();
    isInitialized_ = false;
    return SUCCESS;
}

void OHAudioSuiteEngine::CleanupAll()
{
    std::lock_guard<std::mutex> lock(pipelinesMutex_);
    for (auto pipeline : pipelines_) {
        if (pipeline != nullptr) {
            delete pipeline;
        }
    }
    pipelines_.clear();

    std::lock_guard<std::mutex> nodelock(nodesMutex_);
    for (auto node : nodes_) {
        if (node != nullptr) {
            delete node;
        }
    }
    nodes_.clear();
}

int32_t OHAudioSuiteEngine::CreatePipeline(
    OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode ohWorkMode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr, ERR_INVALID_PARAM, "CreatePipeline pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(
        ohWorkMode == AUDIOSUITE_PIPELINE_EDIT_MODE || ohWorkMode == AUDIOSUITE_PIPELINE_REALTIME_MODE,
        ERR_INVALID_PARAM, "CreatePipeline failed, invalid workMode: %{public}d.",
        static_cast<int32_t>(ohWorkMode));
    std::lock_guard<std::mutex> lock(engineMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST,
        "CreatePipeline failed, engine not initialized.");

    PipelineWorkMode workMode = static_cast<PipelineWorkMode>(ohWorkMode);
    std::shared_ptr<AudioSuitePipeline> pipeline;
    int32_t ret = AudioSuitePipeline::Create(workMode, pipeline);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreatePipeline failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "CreatePipeline malloc failed, ret = %{public}d.", ret);

    OHAudioSuitePipeline *audioPipeline = new (std::nothrow) OHAudioSuitePipeline(std::move(pipeline));
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "CreatePipeline pipeline failed, malloc failed.");
    *audioSuitePipeline = (OH_AudioSuitePipeline *)audioPipeline;
    AddPipeline(audioPipeline);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyPipeline(OHAudioSuitePipeline *audioPipeline)
{
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    if (pipeline == nullptr) {
        (void)StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), ERR_DESTROY_PIPELINE_NOT_FOUND,
            "OHAudioSuiteEngine::The pipeline does not exist.", true);
        return ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST;
    }

    int32_t ret = pipeline->DeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyPipeline failed, ret = %{public}d.", ret);

    std::vector<OHAudioNode*> nodesToClean;
    audioPipeline->GetAllNodes(nodesToClean);
    RemoveNodes(nodesToClean);
    RemovePipeline(audioPipeline);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::StartPipeline(OHAudioSuitePipeline *audioPipeline)
{
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "StartPipeline failed, pipeline does not exist.");

    int32_t ret = pipeline->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::StopPipeline(OHAudioSuitePipeline *audioPipeline)
{
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "StopPipeline failed, pipeline does not exist.");

    int32_t ret = pipeline->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StopPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetPipelineState(OHAudioSuitePipeline *audioPipeline, OH_AudioSuite_PipelineState *state)
{
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "GetPipelineState failed, pipeline does not exist.");
    CHECK_AND_RETURN_RET_LOG(state != nullptr, ERR_INVALID_PARAM, "GetPipelineState failed, state is nullptr.");

    AudioSuitePipelineState pipelineState = PIPELINE_STOPPED;
    int32_t ret = pipeline->GetPipelineState(pipelineState);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPipelineState failed, ret = %{public}d.", ret);

    *state = static_cast<OH_AudioSuite_PipelineState>(pipelineState);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::RenderFrame(OHAudioSuitePipeline *audioPipeline,
    uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::RenderFrame Start");
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "RenderFrame failed, pipeline does not exist.");

    int32_t ret = pipeline->RenderFrame(
        audioData, requestFrameSize, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::MultiRenderFrame(OHAudioSuitePipeline *audioPipeline,
    AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::MultiRenderFrame Start");
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "MultiRenderFrame failed, pipeline does not exist.");

    int32_t ret = pipeline->MultiRenderFrame(
        reinterpret_cast<uint8_t**>(audioDataArray->audioDataArray), audioDataArray->arraySize,
        audioDataArray->requestFrameSize, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MultiRenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}

int32_t OHAudioSuiteEngine::MetaRenderFrame(OHAudioSuitePipeline *audioPipeline,
    AudioMetaFrame &metaFrame, int32_t *responseAudioSize,
    int32_t *responseMetaSize, bool *finishedFlag)
{
    Trace trace("OHAudioSuiteEngine::MetaRenderFrame Start");
    auto pipeline = GetAudioSuitePipeline(audioPipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "MetaRenderFrame failed, pipeline does not exist.");

    int32_t ret = pipeline->MetaRenderFrame(
        metaFrame, responseAudioSize, responseMetaSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MetaRenderFrame failed, ret = %{public}d.", ret);
    trace.End();
    return ret;
}


int32_t OHAudioSuiteEngine::InitInputCallback(
    std::shared_ptr<AudioSuitePipeline> pipeline,
    OHAudioSuiteNodeBuilder *builder, uint32_t nodeId, OH_AudioNode *audioNode)
{
    int32_t ret = SUCCESS;
    if (builder->IsSetRequestDataCallback()) {
        if (builder->IsSetRequestMetaDataCallback()) {
            std::shared_ptr<OHSuiteInputNodeRequestMetaDataCallBack> metaDataCallback =
                std::make_shared<OHSuiteInputNodeRequestMetaDataCallBack>(audioNode,
                    builder->GetRequestMetaDataCallback(), builder->GetCallBackUserData());
            ret = pipeline->SetRequestMetaDataCallback(nodeId, metaDataCallback);
        } else {
            std::shared_ptr<OHSuiteInputNodeRequestDataCallBack> callback =
                std::make_shared<OHSuiteInputNodeRequestDataCallBack>(audioNode,
                    builder->GetRequestDataCallback(), builder->GetCallBackUserData());
            ret = pipeline->SetRequestDataCallback(nodeId, callback);
        }
    }
    return ret;
}

int32_t OHAudioSuiteEngine::CreateNode(
    OHAudioSuitePipeline *audioSuitePipeline, OHAudioSuiteNodeBuilder *builder, OH_AudioNode **audioNode)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr, ERR_INVALID_PARAM,
        "CreateNode failed, audioSuitePipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(builder != nullptr, ERR_INVALID_PARAM, "CreateNode failed, builder is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioNode != nullptr, ERR_INVALID_PARAM, "CreateNode audioNode is nullptr");

    auto pipeline = GetAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "CreateNode failed, pipeline does not exist.");
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
    int32_t ret = pipeline->CreateNode(nodeCfg, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateNode failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(nodeId != INVALID_PIPELINE_ID, ERR_OPERATION_FAILED, "CreateNode failed, nodeId invail");

    OHAudioNode *node = new (std::nothrow) OHAudioNode(nodeId, nodeCfg.nodeType, pipeline);
    if (node == nullptr) {
        AUDIO_ERR_LOG("CreateNode failed, malloc failed.");
        pipeline->DestroyNode(nodeId);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ret = InitInputCallback(pipeline, builder, nodeId, (OH_AudioNode *)node);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("CreateNode SetRequestDataCallback failed, ret = %{public}d.", ret);
        pipeline->DestroyNode(nodeId);
        return ret;
    }

    audioSuitePipeline->AddNode(node);
    AddNode(node);
    *audioNode = (OH_AudioNode *)node;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyNode(OHAudioNode *node)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "DestroyNode failed, node is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EMPTY, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "DestroyNode failed, node is invalid.");

    ret = pipeline->DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed, ret = %{public}d.", ret);
    {
        std::lock_guard<std::mutex> lock(pipelinesMutex_);
        for (auto *p : pipelines_) {
            if (p != nullptr && p->IsNodeExists(node)) {
                p->RemoveNode(node);
                break;
            }
        }
    }
    RemoveNode(node);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::GetNodeBypassStatus(OHAudioNode *node, bool *bypassStatus)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetNodeBypassStatus failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(bypassStatus != nullptr, ERR_INVALID_PARAM,
        "GetNodeBypassStatus failed, bypassStatus is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeType nodeType = NODE_TYPE_EMPTY;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNodeBypassStatus failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetNodeBypassStatus failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(nodeType != NODE_TYPE_INPUT && nodeType != NODE_TYPE_OUTPUT,
        ERR_NOT_SUPPORTED, "GetNodeBypassStatus failed, node type %{public}d not support option.",
        static_cast<int32_t>(nodeType));

    bool bypass = false;
    ret = pipeline->GetNodeBypassStatus(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNodeBypassStatus failed, ret = %{public}d.", ret);

    *bypassStatus = bypass;
    return ret;
}

int32_t OHAudioSuiteEngine::BypassEffectNode(OHAudioNode *node, bool bypass)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "EnableNode failed, node is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeType nodeType = NODE_TYPE_EMPTY;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "BypassEffectNode failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "BypassEffectNode failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(nodeType != NODE_TYPE_INPUT && nodeType != NODE_TYPE_OUTPUT,
        ERR_NOT_SUPPORTED, "BypassEffectNode failed, enable type %{public}d not support option.",
        static_cast<int32_t>(nodeType));

    ret = pipeline->BypassEffectNode(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "BypassEffectNode failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetAudioFormat(OHAudioNode *node, OH_AudioFormat *audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetAudioFormat failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat failed, audioFormat is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeType nodeType = NODE_TYPE_EMPTY;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetAudioFormat failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(nodeType == NODE_TYPE_INPUT || nodeType == NODE_TYPE_OUTPUT,
        ERR_NOT_SUPPORTED, "SetAudioFormat failed, enable type %{public}d not support.",
        static_cast<int32_t>(nodeType));
    if (nodeType == NODE_TYPE_INPUT && IsHoaFormat(audioFormat->channelLayout)) {
        CHECK_AND_RETURN_RET_LOG(CheckHoaInputFormat(*audioFormat), ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT,
            "SetAudioFormat failed, input node HOA format check error.");
    } else {
        CHECK_AND_RETURN_RET_LOG(CheckAudioFormat(*audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT,
            "SetAudioFormat failed, audio format check error.");
    }

    AudioFormat format;
    format.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat->channelLayout);
    format.audioChannelInfo.numChannels = audioFormat->channelCount;
    format.encodingType = static_cast<AudioStreamEncodingType>(audioFormat->encodingType);
    format.format = static_cast<AudioSampleFormat>(audioFormat->sampleFormat);
    format.rate =  static_cast<AudioSamplingRate>(audioFormat->samplingRate);
    ret = pipeline->SetAudioFormat(nodeId, format);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetAudioFormat(OHAudioNode *node, OH_AudioSuite_SystemNodeFormat *audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetAudioFormat failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat failed, audioFormat is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeType nodeType = NODE_TYPE_EMPTY;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetAudioFormat failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(nodeType == NODE_TYPE_INPUT || nodeType == NODE_TYPE_OUTPUT,
        ERR_NOT_SUPPORTED, "SetAudioFormat failed, enable type %{public}d not support.",
        static_cast<int32_t>(nodeType));
    CHECK_AND_RETURN_RET_LOG(CheckAudioFormat(*audioFormat), ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT,
        "SetFormat failed AudioFormat invalid, nodeid = %{public}u.", nodeId);

    AudioFormat format;
    format.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat->channelLayout);
    format.audioChannelInfo.numChannels = audioFormat->channelCount;
    format.encodingType = static_cast<AudioStreamEncodingType>(audioFormat->encoding);
    format.format = static_cast<AudioSampleFormat>(audioFormat->sampleFormat);
    format.rate =  static_cast<AudioSamplingRate>(audioFormat->samplingRate);
    ret = pipeline->SetAudioFormat(nodeId, format);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::ConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    std::shared_ptr<AudioSuitePipeline> srcPipeline;
    uint32_t srcNodeId;
    int32_t ret = GetAudioSuitePipelineAndNodeId(srcNode, NODE_TYPE_EMPTY, srcPipeline, srcNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, srcNode is invalid.");
    CHECK_AND_RETURN_RET_LOG(srcPipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "ConnectNodes failed, srcPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, destNode is nullptr.");
    std::shared_ptr<AudioSuitePipeline> destPipeline;
    uint32_t destNodeId;
    ret = GetAudioSuitePipelineAndNodeId(destNode, NODE_TYPE_EMPTY, destPipeline, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, destNode is invalid.");
    CHECK_AND_RETURN_RET_LOG(destPipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "ConnectNodes failed, destPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(srcPipeline == destPipeline, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT,
        "ConnectNodes failed, nodes belong to different pipelines.");
    ret = srcPipeline->ConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::DisconnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "DisconnectNodes failed, srcNode is nullptr.");
    std::shared_ptr<AudioSuitePipeline> srcPipeline;
    uint32_t srcNodeId;
    int32_t ret = GetAudioSuitePipelineAndNodeId(srcNode, NODE_TYPE_EMPTY, srcPipeline, srcNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisconnectNodes failed, srcNode is invalid.");
    CHECK_AND_RETURN_RET_LOG(srcPipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "DisconnectNodes failed, srcPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "DisconnectNodes failed, destNode is nullptr.");
    std::shared_ptr<AudioSuitePipeline> destPipeline;
    uint32_t destNodeId;
    ret = GetAudioSuitePipelineAndNodeId(destNode, NODE_TYPE_EMPTY, destPipeline, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisconnectNodes failed, destNode is invalid.");
    CHECK_AND_RETURN_RET_LOG(destPipeline != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
        "DisconnectNodes failed, destPipeline is nullptr.");
    CHECK_AND_RETURN_RET_LOG(srcPipeline == destPipeline, ERR_NOT_SUPPORTED,
        "DisconnectNodes failed, nodes belong to different pipelines.");
    ret = srcPipeline->DisconnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisconnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::LookupPipelineAndNodeId(OHAudioNode* node,
    std::shared_ptr<AudioSuitePipeline>& pipeline, uint32_t& nodeId, AudioNodeType& nodeType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "get pipeline fail, node is nullptr");
    nodeType = NODE_TYPE_EMPTY;
    std::lock_guard<std::mutex> lock(nodesMutex_);
    if (nodes_.find(node) != nodes_.end()) {
        pipeline = node->GetPipeline();
        nodeId = node->GetNodeId();
        nodeType = node->GetNodeType();
        return SUCCESS;
    }
    StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
        BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
            BusinessScenario::CONFIG, ERR_AUDIO_SUITE_NODE_NOT_EXIST), "node does not exist.", true);
    return ERR_AUDIO_SUITE_NODE_NOT_EXIST;
}

int32_t OHAudioSuiteEngine::GetAudioSuitePipelineAndNodeId(OHAudioNode* node, AudioNodeType expectedType,
    std::shared_ptr<AudioSuitePipeline>& pipeline, uint32_t& nodeId)
{
    AudioNodeType nodeType = NODE_TYPE_EMPTY;
    int32_t ret = LookupPipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get pipeline and nodeId failed, ret = %{public}d.", ret);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        expectedType == NODE_TYPE_EMPTY || nodeType == expectedType, ERR_NOT_SUPPORTED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CONFIG,
                ERR_NOT_SUPPORTED), "Node type does not match the parameters.", true),
        "Operation failed, node type = %{public}d must be %{public}d type",
        static_cast<int32_t>(nodeType), static_cast<int32_t>(expectedType));
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::GetAudioSuitePipelineAndNodeId(OHAudioNode* node,
    std::shared_ptr<AudioSuitePipeline>& pipeline, uint32_t& nodeId, AudioNodeType& nodeType)
{
    int32_t ret = LookupPipelineAndNodeId(node, pipeline, nodeId, nodeType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get pipeline and nodeId failed, ret = %{public}d.", ret);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EQUALIZER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEqualizerFrequencyBandGains failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetEqualizerFrequencyBandGains failed, pipeline is nullptr.");

    AudioEqualizerFrequencyBandGains audioGains;
    size_t ohGainsNum = sizeof(frequencyBandGains.gains) / sizeof(frequencyBandGains.gains[0]);
    size_t gainsNum = sizeof(audioGains.gains) / sizeof(audioGains.gains[0]);
    for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
        audioGains.gains[idx] = frequencyBandGains.gains[idx];
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
            (audioGains.gains[idx] >= EQ_FREQUENCY_BAND_GAINS_MIN) &&
                (audioGains.gains[idx] <= EQ_FREQUENCY_BAND_GAINS_MAX),
            ERR_INVALID_PARAM,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                               BusinessScenario::CONFIG, ERR_INVALID_PARAM),
                "SetEqualizerFrequencyBandGains failed, input value not in -10, 10.", true),
            "SetEqualizerFrequencyBandGains failed, input value %{public}d not in -10, 10.", audioGains.gains[idx]);
    }
    ret = pipeline->SetEqualizerFrequencyBandGains(nodeId, audioGains);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains *frequencyBandGains)
{
    CHECK_AND_RETURN_RET_LOG(frequencyBandGains != nullptr, ERR_INVALID_PARAM,
        "GetEqualizerFrequencyBandGains failed, frequencyBandGains is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EQUALIZER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEqualizerFrequencyBandGains failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetEqualizerFrequencyBandGains failed, pipeline is nullptr.");

    AudioEqualizerFrequencyBandGains frequency = {.gains = {0}};
    ret = pipeline->GetEqualizerFrequencyBandGains(nodeId, frequency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);
    size_t ohGainsNum = sizeof(frequencyBandGains->gains) / sizeof(frequencyBandGains->gains[0]);
    size_t gainsNum = sizeof(frequency.gains) / sizeof(frequency.gains[0]);
    for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
        frequencyBandGains->gains[idx] = frequency.gains[idx];
    }
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetSoundFieldType(
    OHAudioNode *node, OH_SoundFieldType soundFieldType)
{
    CHECK_AND_RETURN_RET_LOG(
        soundFieldType >= SOUND_FIELD_FRONT_FACING && soundFieldType <= SOUND_FIELD_WIDE,
        ERR_INVALID_PARAM, "SetSoundFieldType failed, invalid soundFieldType: %{public}d.",
        static_cast<int32_t>(soundFieldType));
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SOUND_FIELD, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSoundFieldType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSoundFieldType failed, pipeline is nullptr.");
    ret = pipeline->SetSoundFieldType(nodeId, static_cast<SoundFieldType>(soundFieldType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSoundFieldType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEnvironmentType(
    OHAudioNode *node, OH_EnvironmentType environmentType)
{
    CHECK_AND_RETURN_RET_LOG(
        environmentType >= ENVIRONMENT_TYPE_BROADCAST && environmentType <= ENVIRONMENT_TYPE_GRAMOPHONE,
        ERR_INVALID_PARAM, "SetEnvironmentType failed, invalid environmentType: %{public}d.",
        static_cast<int32_t>(environmentType));
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_ENVIRONMENT_EFFECT, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEnvironmentType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetEnvironmentType failed, pipeline is nullptr.");
    ret = pipeline->SetEnvironmentType(nodeId, static_cast<EnvironmentType>(environmentType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEnvironmentType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetVoiceBeautifierType(
    OHAudioNode *node, OH_VoiceBeautifierType voiceBeautifierType)
{
    CHECK_AND_RETURN_RET_LOG(
        voiceBeautifierType >= VOICE_BEAUTIFIER_TYPE_CLEAR &&
        voiceBeautifierType <= VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO,
        ERR_INVALID_PARAM, "SetVoiceBeautifierType failed, invalid voiceBeautifierType: %{public}d.",
        static_cast<int32_t>(voiceBeautifierType));
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_VOICE_BEAUTIFIER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetVoiceBeautifierType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetVoiceBeautifierType failed, pipeline is nullptr.");
    ret = pipeline->SetVoiceBeautifierType(nodeId, static_cast<VoiceBeautifierType>(voiceBeautifierType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetVoiceBeautifierType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetSpaceRenderPositionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderPositionParams position)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderPositionParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSpaceRenderPositionParams failed, pipeline is nullptr.");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        position.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && position.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
            position.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && position.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
            position.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && position.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderPositionParams failed, point distance must be in the -5.0f~5.0f", true),
        "SetSpaceRenderPositionParams failed, point distance must be in the -5.0f~5.0f");
    AudioSpaceRenderPositionParams positionParams;
    positionParams.x = position.x;
    positionParams.y = position.y;
    positionParams.z = position.z;
    ret = pipeline->SetSpaceRenderPositionParams(nodeId, positionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderPositionParams failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSpaceRenderPositionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderPositionParams *position)
{
    CHECK_AND_RETURN_RET_LOG(position != nullptr, ERR_INVALID_PARAM,
        "GetSpaceRenderPositionParams failed, position is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderPositionParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSpaceRenderPositionParams failed, pipeline is nullptr.");

    AudioSpaceRenderPositionParams positionParams;
    ret = pipeline->GetSpaceRenderPositionParams(nodeId, positionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderPositionParams failed, ret = %{public}d.", ret);
    position->x = positionParams.x;
    position->y = positionParams.y;
    position->z = positionParams.z;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetSpaceRenderRotationParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderRotationParams rotation)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderRotationParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSpaceRenderRotationParams failed, pipeline is nullptr.");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        rotation.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && rotation.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
        rotation.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && rotation.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE &&
        rotation.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE && rotation.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE,
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f.", true),
        "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f.");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        rotation.surroundTime >= SPACE_RENDER_MIN_TIME && rotation.surroundTime <= SPACE_RENDER_MAX_TIME,
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, time must be in the 2.0f~40.0f.", true),
        "SetSpaceRenderRotationParams failed, time must be in the 2.0f~40.0f.");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        rotation.surroundDirection == OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW ||
        rotation.surroundDirection == OH_AudioSuite_SurroundDirection::SPACE_RENDER_CW,
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, surround direction must be 0 or 1.", true),
        "SetSpaceRenderRotationParams failed, surround direction must be 0 or 1.");
    AudioSpaceRenderRotationParams rotationParams;
    rotationParams.x = rotation.x;
    rotationParams.y = rotation.y;
    rotationParams.z = rotation.z;
    rotationParams.surroundTime = rotation.surroundTime;
    rotationParams.surroundDirection = static_cast<AudioSurroundDirection>(rotation.surroundDirection);
    ret = pipeline->SetSpaceRenderRotationParams(nodeId, rotationParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderRotationParams failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSpaceRenderRotationParams(
    OHAudioNode* node, OH_AudioSuite_SpaceRenderRotationParams* rotation)
{
    CHECK_AND_RETURN_RET_LOG(rotation != nullptr, ERR_INVALID_PARAM,
        "GetSpaceRenderRotationParams failed, rotation is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderRotationParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSpaceRenderRotationParams failed, pipeline is nullptr.");

    AudioSpaceRenderRotationParams rotationParams;
    ret = pipeline->GetSpaceRenderRotationParams(nodeId, rotationParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderRotationParams failed, ret = %{public}d.", ret);
    rotation->x = rotationParams.x;
    rotation->y = rotationParams.y;
    rotation->z = rotationParams.z;
    rotation->surroundTime = rotationParams.surroundTime;
    rotation->surroundDirection = static_cast<OH_AudioSuite_SurroundDirection>(rotationParams.surroundDirection);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetSpaceRenderExtensionParams(
    OHAudioNode* node, OH_AudioSuite_SpaceRenderExtensionParams extension)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderExtensionParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSpaceRenderExtensionParams failed, pipeline is nullptr.");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        extension.extRadius >= SPACE_RENDER_MIN_EXPAND_RADIUS && extension.extRadius <= SPACE_RENDER_MAX_EXPAND_RADIUS,
        ERR_INVALID_PARAM, StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderExtensionParams failed, Radius must be in the 1.0f~5.0f", true),
        "SetSpaceRenderExtensionParams failed, Radius must be in the 1.0f~5.0f");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        extension.extAngle > SPACE_RENDER_MIN_EXPAND_ANGLE && extension.extAngle < SPACE_RENDER_MAX_EXPAND_ANGLE,
        ERR_INVALID_PARAM, StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderExtensionParams failed, Angle must be in the (0, 360)", true),
        "SetSpaceRenderExtensionParams failed, Angle must be in the (0, 360).");
    AudioSpaceRenderExtensionParams extensionParams;
    extensionParams.extAngle = extension.extAngle;
    extensionParams.extRadius = extension.extRadius;
    ret = pipeline->SetSpaceRenderExtensionParams(nodeId, extensionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSpaceRenderExtensionParams failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSpaceRenderExtensionParams(
    OHAudioNode *node, OH_AudioSuite_SpaceRenderExtensionParams *extension)
{
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, ERR_INVALID_PARAM,
        "GetSpaceRenderExtensionParams failed, extension is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SPACE_RENDER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderExtensionParams failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSpaceRenderExtensionParams failed, pipeline is nullptr.");

    AudioSpaceRenderExtensionParams extensionParams;
    ret = pipeline->GetSpaceRenderExtensionParams(nodeId, extensionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSpaceRenderExtensionParams failed, ret = %{public}d.", ret);
    extension->extRadius = extensionParams.extRadius;
    extension->extAngle = extensionParams.extAngle;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetTempoAndPitch(OHAudioNode* node, float speed, float pitch)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        (speed >= SPEED_RATE_MIN && speed <= SPEED_RATE_MAX) && (pitch >= PITCH_RATE_MIN && pitch <= PITCH_RATE_MAX),
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetTempoAndPitch failed, speed must be in the 0.5~10.0 and pitch must be in the 0.1~5.0.", true),
        "SetTempoAndPitch failed, speed must be in the 0.5~10.0 and pitch must be in the 0.1~5.0.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_TEMPO_PITCH, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetTempoAndPitch failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetTempoAndPitch failed, pipeline is nullptr.");
    ret = pipeline->SetTempoAndPitch(nodeId, speed, pitch);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetTempoAndPitch failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetTempoAndPitch(OHAudioNode* node, float* speed, float* pitch)
{
    CHECK_AND_RETURN_RET_LOG(speed != nullptr && pitch != nullptr, ERR_INVALID_PARAM,
        "GetTempoAndPitch failed, parameter is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_TEMPO_PITCH, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetTempoAndPitch failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetTempoAndPitch failed, pipeline is nullptr.");

    ret = pipeline->GetTempoAndPitch(nodeId, *speed, *pitch);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetTempoAndPitch failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetPureVoiceChangeOption(
    OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption option)
{
    CHECK_AND_RETURN_RET_LOG(
        option.optionGender >= OH_AudioSuite_PureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE &&
        option.optionGender <= OH_AudioSuite_PureVoiceChangeGenderOption::PURE_VOICE_CHANGE_MALE,
        ERR_INVALID_PARAM, "SetPureVoiceChangeOption failed, invalid optionGender: %{public}d.",
        static_cast<int32_t>(option.optionGender));
    CHECK_AND_RETURN_RET_LOG(
        option.optionType >= OH_AudioSuite_PureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CARTOON &&
        option.optionType <= OH_AudioSuite_PureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_SEASONED,
        ERR_INVALID_PARAM, "SetPureVoiceChangeOption failed, invalid optionType: %{public}d.",
        static_cast<int32_t>(option.optionType));
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        (option.pitch >= AUDIO_VOICE_MORPHING_PITCH_MIN && option.pitch <= AUDIO_VOICE_MORPHING_PITCH_MAX) ||
            option.pitch == AUDIO_VOICE_MORPHING_PITCH_DEFAULT,
        ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetPureVoiceChangeOption failed, pitch must be in the 0.3f~3.0f or 0.0f.", true),
        "SetPureVoiceChangeOption failed, pitch must be in the 0.3f~3.0f or 0.0f.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_PURE_VOICE_CHANGE, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetPureVoiceChangeOption failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetPureVoiceChangeOption failed, pipeline is nullptr.");
    AudioPureVoiceChangeOption optionParams;
    optionParams.optionGender = static_cast<AudioPureVoiceChangeGenderOption>(option.optionGender);
    optionParams.optionType = static_cast<AudioPureVoiceChangeType>(option.optionType);
    optionParams.pitch = static_cast<float>(option.pitch);
    ret = pipeline->SetPureVoiceChangeOption(nodeId, optionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetPureVoiceChangeOption failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetPureVoiceChangeOption(
    OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption* option)
{
    CHECK_AND_RETURN_RET_LOG(option != nullptr, ERR_INVALID_PARAM,
        "GetPureVoiceChangeOption failed, option is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_PURE_VOICE_CHANGE, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPureVoiceChangeOption failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetPureVoiceChangeOption failed, pipeline is nullptr.");

    AudioPureVoiceChangeOption optionParams;
    ret = pipeline->GetPureVoiceChangeOption(nodeId, optionParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPureVoiceChangeOption failed, ret = %{public}d.", ret);
    option->optionGender = static_cast<OH_AudioSuite_PureVoiceChangeGenderOption>(optionParams.optionGender);
    option->optionType = static_cast<OH_AudioSuite_PureVoiceChangeType>(optionParams.optionType);
    option->pitch = static_cast<float>(optionParams.pitch);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::SetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType type)
{
    CHECK_AND_RETURN_RET_LOG(
        type >= OH_AudioSuite_GeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE &&
        type <= OH_AudioSuite_GeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_WAR,
        ERR_INVALID_PARAM, "SetGeneralVoiceChangeType failed, invalid type: %{public}d.",
        static_cast<int32_t>(type));
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_GENERAL_VOICE_CHANGE, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetGeneralVoiceChangeType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetGeneralVoiceChangeType failed, pipeline is nullptr.");
    ret = pipeline->SetGeneralVoiceChangeType(nodeId, static_cast<AudioGeneralVoiceChangeType>(type));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetGeneralVoiceChangeType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType* type)
{
    CHECK_AND_RETURN_RET_LOG(type != nullptr, ERR_INVALID_PARAM,
        "GetGeneralVoiceChangeType failed, type is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_GENERAL_VOICE_CHANGE, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetGeneralVoiceChangeType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetGeneralVoiceChangeType failed, pipeline is nullptr.");

    AudioGeneralVoiceChangeType typeParam;
    ret = pipeline->GetGeneralVoiceChangeType(nodeId, typeParam);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetGeneralVoiceChangeType failed, ret = %{public}d.", ret);
    *type = static_cast<OH_AudioSuite_GeneralVoiceChangeType>(typeParam);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::GetEnvironmentType(OHAudioNode *node, OH_EnvironmentType *environmentType)
{
    CHECK_AND_RETURN_RET_LOG(environmentType != nullptr, ERR_INVALID_PARAM,
        "GetEnvironmentType failed, environmentType is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_ENVIRONMENT_EFFECT, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEnvironmentType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetEnvironmentType failed, pipeline is nullptr.");

    EnvironmentType environment = AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE;
    ret = pipeline->GetEnvironmentType(nodeId, environment);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEnvironmentType failed, ret = %{public}d.", ret);
    *environmentType = static_cast<OH_EnvironmentType>(environment);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::GetSoundFieldType(OHAudioNode *node, OH_SoundFieldType *soundFieldType)
{
    CHECK_AND_RETURN_RET_LOG(soundFieldType != nullptr, ERR_INVALID_PARAM,
        "GetSoundFieldType failed, soundFieldType is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_SOUND_FIELD, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSoundFieldType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSoundFieldType failed, pipeline is nullptr.");

    SoundFieldType soundField = AUDIO_SUITE_SOUND_FIELD_CLOSE;
    ret = pipeline->GetSoundFieldType(nodeId, soundField);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSoundFieldType failed, ret = %{public}d.", ret);
    *soundFieldType = static_cast<OH_SoundFieldType>(soundField);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::GetVoiceBeautifierType(OHAudioNode *node,
    OH_VoiceBeautifierType *voiceBeautifierType)
{
    CHECK_AND_RETURN_RET_LOG(voiceBeautifierType != nullptr, ERR_INVALID_PARAM,
        "GetVoiceBeautifierType failed, voiceBeautifierType is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_VOICE_BEAUTIFIER, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetVoiceBeautifierType failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetVoiceBeautifierType failed, pipeline is nullptr.");

    VoiceBeautifierType voiceBeautifier = AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR;
    ret = pipeline->GetVoiceBeautifierType(nodeId, voiceBeautifier);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetVoiceBeautifierType failed, ret = %{public}d.", ret);
    *voiceBeautifierType = static_cast<OH_VoiceBeautifierType>(voiceBeautifier);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported)
{
    CHECK_AND_RETURN_RET_LOG(
        isSupported != nullptr, ERR_INVALID_PARAM, "IsNodeTypeSupported function failed, isSupported is nullptr.");
    bool tempIsSupported = false;
    int32_t ret = AudioSuitePipeline::IsNodeTypeSupported(
        static_cast<AudioNodeType>(nodeType), tempIsSupported);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "IsNodeTypeSupported function failed, ret = %{public}d.", ret);
    *isSupported = tempIsSupported;
    return ret;
}

int32_t OHAudioSuiteEngine::SetSystemNodeParam(OHAudioNode* node, std::vector<uint8_t> &param)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetSystemNodeParam failed, node is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EMPTY, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSystemNodeParam failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSystemNodeParam failed, pipeline is nullptr.");

    ret = pipeline->SetSystemNodeParam(nodeId, param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSystemNodeParam failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSystemNodeParam(OHAudioNode* node, std::vector<uint8_t> &param)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetSystemNodeParam failed, node is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EMPTY, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeParam failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeParam failed, pipeline is nullptr.");

    ret = pipeline->GetSystemNodeParam(nodeId, param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeParam failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::GetSystemNodeInOutSize(OHAudioNode* node, uint32_t &inSize, uint32_t &outSize)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetSystemNodeInOutSize failed, node is nullptr.");
    std::shared_ptr<AudioSuitePipeline> pipeline;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t ret = GetAudioSuitePipelineAndNodeId(node, NODE_TYPE_EMPTY, pipeline, nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeInOutSize failed, node is invalid.");
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeInOutSize failed, pipeline is nullptr.");

    ret = pipeline->GetSystemNodeInOutSize(nodeId, inSize, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeInOutSize failed, ret = %{public}d.", ret);
    return ret;
}

void OHAudioSuiteEngine::AddPipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::mutex> lock(pipelinesMutex_);
    pipelines_.insert(pipeline);
}

void OHAudioSuiteEngine::RemovePipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::mutex> lock(pipelinesMutex_);
    if (pipelines_.erase(pipeline) > 0 && pipeline != nullptr) {
        delete pipeline;
    }
}

std::shared_ptr<AudioSuitePipeline> OHAudioSuiteEngine::GetAudioSuitePipeline(OHAudioSuitePipeline *pipeline)
{
    std::lock_guard<std::mutex> lock(pipelinesMutex_);
    if (pipelines_.find(pipeline) == pipelines_.end()) {
        return nullptr;
    }
    return pipeline->GetPipeline();
}

std::shared_ptr<AudioSuitePipeline> OHAudioSuitePipeline::GetPipeline() const
{
    return pipeline_;
}

void OHAudioSuitePipeline::AddNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.insert(node);
}

void OHAudioSuitePipeline::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.erase(node);
}

bool OHAudioSuitePipeline::IsNodeExists(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nodes_.find(node) != nodes_.end();
}

void OHAudioSuitePipeline::GetAllNodes(std::vector<OHAudioNode*> &nodes)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto node : nodes_) {
        nodes.push_back(node);
    }
}

void OHAudioSuiteEngine::AddNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(nodesMutex_);
    nodes_.insert(node);
}

void OHAudioSuiteEngine::RemoveNode(OHAudioNode *node)
{
    std::lock_guard<std::mutex> lock(nodesMutex_);
    if (nodes_.erase(node) > 0 && node != nullptr) {
        delete node;
    }
}

void OHAudioSuiteEngine::RemoveNodes(std::vector<OHAudioNode*> &nodes)
{
    std::lock_guard<std::mutex> lock(nodesMutex_);
    for (auto node : nodes) {
        if (nodes_.erase(node) > 0 && node != nullptr) {
            delete node;
        }
    }
}

int32_t OHAudioSuiteEngine::PrintInfo(OHAudioSuitePipeline *pipeline, int32_t fd)
{
    if (pipeline != nullptr) {
        auto p = GetAudioSuitePipeline(pipeline);
        CHECK_AND_RETURN_RET_LOG(p != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
            "PrintInfo failed, pipeline does not exist.");
        return p->PrintInfo(fd);
    }
    std::lock_guard<std::mutex> lock(pipelinesMutex_);
    for (auto *p : pipelines_) {
        if (p != nullptr) {
            auto innerPipeline = p->GetPipeline();
            if (innerPipeline != nullptr) {
                innerPipeline->PrintInfo(fd);
            }
        }
    }
    return SUCCESS;
}

}  // namespace AudioStandard
}  // namespace OHOS
