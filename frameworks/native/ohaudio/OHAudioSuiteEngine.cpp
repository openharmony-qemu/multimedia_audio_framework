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
#include "audio_suite_info.h"

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;

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

static OH_AudioSuite_Result ConvertError(int32_t err)
{
    if (err == OHOS::AudioStandard::SUCCESS) {
        return AUDIOSUITE_SUCCESS;
    } else if (err == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        return AUDIOSUITE_ERROR_INVALID_PARAM;
    } else if (err == OHOS::AudioStandard::ERR_ILLEGAL_STATE) {
        return AUDIOSUITE_ERROR_ILLEGAL_STATE;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT) {
        return AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_ENGINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_ENGINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_NODE_NOT_EXIST) {
        return AUDIOSUITE_ERROR_NODE_NOT_EXIST;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        return AUDIOSUITE_ERROR_UNSUPPORT_CONNECT;
    } else if (err == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        return AUDIOSUITE_ERROR_UNSUPPORT_OPERATION;
    } else if (err == OHOS::AudioStandard::ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS) {
        return AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS;
    }
    return AUDIOSUITE_ERROR_SYSTEM;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Create(OH_AudioSuiteEngine **audioSuiteEngine)
{
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Create suiteEngine audioSuiteEngine is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ILLEGAL_STATE, "Get suiteEngine suiteEngine is nullptr");

    int32_t ret = suiteEngine->CreateEngine();
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOSUITE_SUCCESS,
        ConvertError(ret), "Create suiteEngine failed, ret is %{public}d.", ret);

    *audioSuiteEngine = (OH_AudioSuiteEngine *)suiteEngine;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteEngine_Destroy(OH_AudioSuiteEngine *audioSuiteEngine)
{
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy audioSuiteEngine is nullptr");

    int32_t error = suiteEngine->DestroyEngine();
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_CreatePipeline(
    OH_AudioSuiteEngine *audioSuiteEngine, OH_AudioSuitePipeline **audioSuitePipeline)
{
    OHAudioSuiteEngine *suiteEngine = ConvertAudioSuiteEngine(audioSuiteEngine);
    CHECK_AND_RETURN_RET_LOG(audioSuiteEngine != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "CreatePipeline audioSuiteEngine is nullptr");

    int32_t error = suiteEngine->CreatePipeline(audioSuitePipeline);
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
    void *audioData, int32_t frameSize, int32_t *writeSize, bool *finishedFlag)
{
    OHAudioSuitePipeline *pipeline = ConvertAudioSuitePipeline(audioSuitePipeline);
    CHECK_AND_RETURN_RET_LOG(pipeline != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame pipeline is nullptr");
    CHECK_AND_RETURN_RET_LOG(writeSize != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame writeSize is nullptr");
    CHECK_AND_RETURN_RET_LOG(finishedFlag != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame finishedFlag is nullptr");
    CHECK_AND_RETURN_RET_LOG(frameSize != 0,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RenderFrame frameSize is zero");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "RenderFrame suiteEngine is nullptr");
    int32_t error = suiteEngine->RenderFrame(pipeline,
        static_cast<uint8_t *>(audioData), frameSize, writeSize, finishedFlag);
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

OH_AudioSuite_Result OH_AudioSuiteEngine_GetNodeEnableStatus(
    OH_AudioNode *audioNode, OH_AudioNodeEnable *audioNodeEnable)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeEnableStatus node is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNodeEnable != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "GetNodeEnableStatus audioNodeEnable is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "GetNodeEnableStatus suiteEngine is nullptr");

    int32_t error = suiteEngine->GetNodeEnableStatus(node, audioNodeEnable);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_EnableNode(OH_AudioNode *audioNode, OH_AudioNodeEnable audioNodeEnable)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "EnableNode node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "DestroyNode suiteEngine is nullptr");

    int32_t error = suiteEngine->EnableNode(node, audioNodeEnable);
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

OH_AudioSuite_Result OH_AudioSuiteEngine_ConnectNodes(OH_AudioNode *sourceAudioNode, OH_AudioNode *destAudioNode,
    OH_AudioNode_Port_Type sourcePortType, OH_AudioNode_Port_Type destPortType)
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

    int32_t error = suiteEngine->ConnectNodes(srcNode, destNode, sourcePortType, destPortType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_DisConnectNodes(OH_AudioNode *sourceAudioNode, OH_AudioNode *destAudioNode)
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

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEqualizerMode(OH_AudioNode *audioNode, OH_EqualizerMode eqMode)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "SetEqualizerMode node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetEqualizerMode suiteEngine is nullptr");

    int32_t error = suiteEngine->SetEqualizerMode(node, eqMode);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(
    OH_AudioNode *audioNode, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM,
        "SetEqualizerFrequencyBandGains node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetEqualizerFrequencyBandGains suiteEngine is nullptr");

    int32_t error = suiteEngine->SetEqualizerFrequencyBandGains(node, frequencyBandGains);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetSoundFiledType(OH_AudioNode *audioNode, OH_SoundFieldType soundFieldType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "SetSoundFieldType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetSoundFieldType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetSoundFieldType(node, soundFieldType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetEnvironmentType(OH_AudioNode *audioNode, OH_EnvironmentType environmentType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM, "SetEnvironmentType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetEnvironmentType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetEnvironmentType(node, environmentType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_SetVoiceBeautifierType(
    OH_AudioNode *audioNode, OH_VoiceBeautifierType voiceBeautifierType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, AUDIOSUITE_ERROR_INVALID_PARAM,
        "SetVoiceBeautifierType node is nullptr");
    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "SetVoiceBeautifierType suiteEngine is nullptr");

    int32_t error = suiteEngine->SetVoiceBeautifierType(node, voiceBeautifierType);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_InstallTap(OH_AudioNode* audioNode,
    OH_AudioNode_Port_Type portType, OH_AudioNode_OnReadTapDataCallback callback, void* userData)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "InstallTap node is nullptr");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "InstallTap callback is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ILLEGAL_STATE, "InstallTap suiteEngine is nullptr");

    int32_t error = suiteEngine->InstallTap(node, portType, callback, userData);
    return ConvertError(error);
}

OH_AudioSuite_Result OH_AudioSuiteEngine_RemoveTap(OH_AudioNode* audioNode,
    OH_AudioNode_Port_Type portType)
{
    OHAudioNode *node = ConvertAudioNode(audioNode);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "RemoveTap node is nullptr");

    OHAudioSuiteEngine *suiteEngine = OHAudioSuiteEngine::GetInstance();
    CHECK_AND_RETURN_RET_LOG(suiteEngine != nullptr,
        AUDIOSUITE_ERROR_ENGINE_NOT_EXIST, "RemoveTap suiteEngine is nullptr");

    int32_t error = suiteEngine->RemoveTap(node, portType);
    return ConvertError(error);
}

namespace OHOS {
namespace AudioStandard {

using namespace OHOS::AudioStandard::AudioSuite;

int32_t OHSuiteInputNodeWriteDataCallBack::OnWriteDataCallBack(void *audioData, int32_t audioDataSize, bool *finished)
{
    CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, 0, "OnWriteDataCallBack callback is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, 0, "OnWriteDataCallBack OH_audioNode is nullptr");

    return callback_(audioNode_, userData_, audioData, audioDataSize, finished);
}

OHAudioSuiteEngine *OHAudioSuiteEngine::GetInstance()
{
    static OHAudioSuiteEngine manager;
    return &manager;
}

int32_t OHAudioSuiteEngine::CreateEngine()
{
    return IAudioSuiteManager::GetAudioSuiteManager().Init();
}

int32_t OHAudioSuiteEngine::DestroyEngine()
{
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyEngine failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::CreatePipeline(OH_AudioSuitePipeline **audioSuitePipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioSuitePipeline != nullptr,
        ERR_INVALID_PARAM, "CreatePipeline pipeline is nullptr");

    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().CreatePipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreatePipeline failed, ret = %{public}d.", ret);
    CHECK_AND_RETURN_RET_LOG(pipelineId != INVALID_PIPELINE_ID, ret, "CreatePipeline failed, pipelineId invailed");

    OHAudioSuitePipeline *audioPipeline = new OHAudioSuitePipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "CreatePipeline pipeline failed, malloc failed.");
    *audioSuitePipeline = (OH_AudioSuitePipeline *)audioPipeline;
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::DestroyPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "DestroyPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::StartPipeline(OHAudioSuitePipeline *audioPipeline)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "StartPipeline failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().StartPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartPipeline failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::StopPipeline(OHAudioSuitePipeline *audioPipeline)
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
    uint8_t *audioData, int32_t frameSize, int32_t *writeSize, bool *finishedFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioPipeline != nullptr, ERR_INVALID_PARAM,
        "RenderFrame failed, audioPipeline is nullptr.");

    uint32_t pipelineId = audioPipeline->GetPipelineId();

    AUDIO_INFO_LOG("OHAudioSuiteEngine::RenderFrame enter start.");
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().RenderFrame(
        pipelineId, audioData, frameSize, writeSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderFrame failed, ret = %{public}d.", ret);
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
        CHECK_AND_RETURN_RET_LOG(builder->IsSetWriteDataCallBack() && builder->IsSetFormat(), ERR_NOT_SUPPORTED,
            "Create input Node must set WriteDataCallBack and audio format.");
    } else if (builder->GetNodeType() == NODE_TYPE_OUTPUT) {
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetWriteDataCallBack() && builder->IsSetFormat(), ERR_NOT_SUPPORTED,
            "Create output Node, can not set WriteDataCallBack, must set aduio format.");
    } else {
        CHECK_AND_RETURN_RET_LOG(!builder->IsSetWriteDataCallBack() && !builder->IsSetFormat(), ERR_NOT_SUPPORTED,
            "Create effect Node, can not set WriteDataCallBack and format.");
    }

    uint32_t nodeId = IAudioSuiteManager::GetAudioSuiteManager().CreateNode(pipelineId, nodeCfg);
    if (nodeId == INVALID_NODE_ID) {
        AUDIO_ERR_LOG("CreateNode from GetAudioSuiteManager failed.");
        return ERR_OPERATION_FAILED;
    }

    OHAudioNode *node = new OHAudioNode(nodeId, nodeCfg.nodeType);
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERROR, "CreateNode failed, malloc failed.");

    *audioNode = (OH_AudioNode *)node;
    if (!builder->IsSetWriteDataCallBack()) {
        return AUDIOSUITE_SUCCESS;
    }

    std::shared_ptr<OHSuiteInputNodeWriteDataCallBack> callback =
        std::make_shared<OHSuiteInputNodeWriteDataCallBack>(reinterpret_cast<OH_AudioNode *>(audioNode),
            builder->GetOnWriteDataCallBack(), builder->GetOnWriteUserData());
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetOnWriteDataCallback(nodeId, callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateNode SetOnWriteDataCallback failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::DestroyNode(OHAudioNode *node)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "DestroyNode failed, node is nullptr.");
    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed, ret = %{public}d.", ret);

    delete node;
    return ret;
}

int32_t OHAudioSuiteEngine::GetNodeEnableStatus(OHAudioNode *node, OH_AudioNodeEnable *enable)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "GetNodeEnableStatus failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(enable != nullptr, ERR_INVALID_PARAM, "GetNodeEnableStatus failed, enable is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "GetNodeEnableStatus failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    AudioNodeEnable enableStatus = NODE_DISABLE;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().GetNodeEnableStatus(nodeId, enableStatus);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetNodeEnableStatus failed, ret = %{public}d.", ret);

    *enable = static_cast<OH_AudioNodeEnable>(enableStatus);
    return ret;
}

int32_t OHAudioSuiteEngine::EnableNode(OHAudioNode *node, OH_AudioNodeEnable enable)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "EnableNode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "GetNodeEnableStatus failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();

    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().EnableNode(nodeId, static_cast<AudioNodeEnable>(enable));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "EnableNode failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetAudioFormat(OHAudioNode *node, OH_AudioFormat *audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetAudioFormat failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(audioFormat != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat failed, audioFormat is nullptr.");
    CHECK_AND_RETURN_RET_LOG((node->GetNodeType() == NODE_TYPE_INPUT) || (node->GetNodeType() == NODE_TYPE_OUTPUT),
        ERR_NOT_SUPPORTED, "GetNodeEnableStatus failed, enable type %{public}d not support option.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    AudioFormat format;
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetAudioFormat(nodeId, format);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetAudioFormat failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::ConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode,
    OH_AudioNode_Port_Type sourcePortType, OH_AudioNode_Port_Type destPortType)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "ConnectNodes failed, srcNode is nullptr.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().ConnectNodes(srcNodeId, destNodeId,
        static_cast<AudioNodePortType>(sourcePortType), static_cast<AudioNodePortType>(destPortType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::DisConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode)
{
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    CHECK_AND_RETURN_RET_LOG(destNode != nullptr, ERR_INVALID_PARAM, "DisConnectNodes failed, srcNode is nullptr.");
    uint32_t srcNodeId = srcNode->GetNodeId();
    uint32_t destNodeId = destNode->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().DisConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisConnectNodes failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEqualizerMode(OHAudioNode *node, OH_EqualizerMode eqMode)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetEqualizerMode failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_EQUALIZER, ERR_NOT_SUPPORTED, "SetEqualizerMode "
        "failed, node type = %d{public}d must is EQUALIZER type.", static_cast<int32_t>(node->GetNodeType()));
    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetEqualizerMode(
        nodeId, static_cast<EqualizerMode>(eqMode));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEqualizerMode failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEqualizerFrequencyBandGains(
    OHAudioNode *node, OH_EqualizerFrequencyBandGains frequencyBandGains)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetEqualizerFrequencyBandGains failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_EQUALIZER, ERR_NOT_SUPPORTED,
        "SetEqualizerFrequencyBandGains failed, node type = %d{public}d must is EQUALIZER type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    AudioEqualizerFrequencyBandGains audioGains;
    size_t ohGainsNum = sizeof(frequencyBandGains.gains) / sizeof(frequencyBandGains.gains[0]);
    size_t gainsNum = sizeof(audioGains.gains) / sizeof(audioGains.gains[0]);

    for (uint32_t idx = 0; idx < (ohGainsNum < gainsNum ? ohGainsNum : gainsNum); idx++) {
        audioGains.gains[idx] = frequencyBandGains.gains[idx];
    }

    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetEqualizerFrequencyBandGains(nodeId, audioGains);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEqualizerFrequencyBandGains failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetSoundFieldType(
    OHAudioNode *node, OH_SoundFieldType soundFieldType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetEqualizerFrequencyBandGains failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_SOUND_FIELD, ERR_NOT_SUPPORTED, "SetSoundFieldType "
        "failed, node type = %d{public}d must is SOUND_FIELD type.", static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetSoundFieldType(
        nodeId, static_cast<SoundFieldType>(soundFieldType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSoundFieldType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetEnvironmentType(
    OHAudioNode *node, OH_EnvironmentType environmentType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetEnvironmentType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_ENVIRONMENT_EFFECT, ERR_NOT_SUPPORTED,
        "SetEnvironmentType failed, node type = %d{public}d must is ENVIRONMENT_EFFECT type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetEnvironmentType(
        nodeId, static_cast<EnvironmentType>(environmentType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetEnvironmentType failed, ret = %{public}d.", ret);
    return ret;
}

int32_t OHAudioSuiteEngine::SetVoiceBeautifierType(
    OHAudioNode *node, OH_VoiceBeautifierType voiceBeautifierType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "SetVoiceBeautifierType failed, node is nullptr.");
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_VOICE_BEAUTIFIER, ERR_NOT_SUPPORTED,
        "SetVoiceBeautifierType failed, node type = %d{public}d must is VOICE_BEAUTIFIER type.",
        static_cast<int32_t>(node->GetNodeType()));

    uint32_t nodeId = node->GetNodeId();
    int32_t ret = IAudioSuiteManager::GetAudioSuiteManager().SetVoiceBeautifierType(
        nodeId, static_cast<VoiceBeautifierType>(voiceBeautifierType));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetVoiceBeautifierType failed, ret = %{public}d.", ret);
    return ret;
}

void OHOnReadTapDataCallback::OnReadTapDataCallback(void *audioData, int32_t audioDataSize)
{
    CHECK_AND_RETURN_LOG(ohAudioNode_ != nullptr, "ohAudioNode_ is nullptr");
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "callback_ to the function is nullptr");
    CHECK_AND_RETURN_LOG(audioData != nullptr, "audioData is nullptr");

    callback_(ohAudioNode_, userData_, audioData, audioDataSize);
}

int32_t OHAudioSuiteEngine::InstallTap(OHAudioNode *node,
    OH_AudioNode_Port_Type portType, OH_AudioNode_OnReadTapDataCallback callback, void* userData)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "InstallTap node is nullptr");
    uint32_t nodeId = node->GetNodeId();
    std::shared_ptr<OHOnReadTapDataCallback> tapDataCallback= std::make_shared<OHOnReadTapDataCallback>(callback,
        reinterpret_cast<OH_AudioNode*>(node), userData);

    IAudioSuiteManager::GetAudioSuiteManager().InstallTap(
        nodeId, static_cast<AudioNodePortType>(portType), tapDataCallback);
    return SUCCESS;
}

int32_t OHAudioSuiteEngine::RemoveTap(OHAudioNode *node, OH_AudioNode_Port_Type portType)
{
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM, "RemoveTap node is nullptr");
    uint32_t nodeId = node->GetNodeId();

    IAudioSuiteManager::GetAudioSuiteManager().RemoveTap(nodeId, static_cast<AudioNodePortType>(portType));
    return SUCCESS;
}

}  // namespace AudioStandard
}  // namespace OHOS
