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

#include "oh_audio_suite_unit_test.h"
#include "OHAudioSuiteEngine.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "native_audio_suite_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioSuiteUnitTest::SetUpTestCase(void) { }

void OHAudioSuiteUnitTest::TearDownTestCase(void) { }

void OHAudioSuiteUnitTest::SetUp(void) { }

void OHAudioSuiteUnitTest::TearDown(void) { }

static int32_t WriteDataCallback(OH_AudioNode *audioNode, void *userData,
    void *audioData, int32_t audioDataSize, bool *finished)
{
    if (finished != nullptr) {
        *finished = true;
    }
    return 0;
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteNodeBuilder_Create_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(nullptr, AUDIOSUITE_NODE_TYPE_INPUT);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteNodeBuilder_Destroy_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Destroy(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteNodeBuilder_SetFormat_001, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetFormat(nullptr, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_INPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    audioFormat.samplingRate = 0;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    audioFormat.samplingRate = SAMPLE_RATE_8000;
    audioFormat.channelCount = 100;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    audioFormat.channelCount = 2;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_INPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, WriteDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_Create_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_Destroy_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Destroy(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_CreatePipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_CreatePipeline(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_DestroyPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_DestroyPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_StartPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_StartPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_StopPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_StopPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_GetPipelineState_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetPipelineState(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuite_PipelineState state = AUDIOSUITE_PIPELINE_RUNNING;
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &state);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(state, AUDIOSUITE_PIPELINE_STOPPED);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &state);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &state);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_CreateNode_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_CreateNode(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_INPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);
    
    ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, WriteDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = SAMPLE_RATE_8000;
    audioFormat.channelCount = 2;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_EnableNode_001, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_INPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    
    ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, WriteDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = SAMPLE_RATE_8000;
    audioFormat.channelCount = 2;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeEnable audioNoedEnable;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(audioNode, &audioNoedEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_EnableNode(audioNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_EnableNode_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_OUTPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = SAMPLE_RATE_8000;
    audioFormat.channelCount = 2;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeEnable audioNoedEnable;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(audioNode, &audioNoedEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_EnableNode(audioNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, WriteDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);


    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_Effect_Setopt_001, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder, AUDIOSUITE_NODE_TYPE_OUTPUT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = SAMPLE_RATE_8000;
    audioFormat.channelCount = 2;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_SetEquailizerMode(audioNode, EQUALIZER_ROCK_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    OH_EqualizerFrequencyBandGains gains = {0};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(audioNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_SetSoundFiledType(audioNode, SOUND_FIELD_WIDE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_SetEnvironmentType(audioNode, ENVIRONMENT_TYPE_GRAMOPHONE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(audioNode, VOICE_BEAUTIFIER_TYPE_NORMAL);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
