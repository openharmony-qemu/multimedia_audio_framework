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

#include "oh_audio_suite_engine_test.h"
#include "OHAudioSuiteEngine.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "native_audio_suite_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioSuiteEngineTest::SetUpTestCase(void) { }

void OHAudioSuiteEngineTest::TearDownTestCase(void) { }

void OHAudioSuiteEngineTest::SetUp(void) { }

void OHAudioSuiteEngineTest::TearDown(void) { }

static const uint32_t MAX_PIPELINE_NUM = 10;

static int32_t WriteDataCallback(OH_AudioNode *audioNode, void *userData,
    void *audioData, int32_t audioDataSize, bool *finished)
{
    if (finished != nullptr) {
        *finished = true;
    }
    return 1;
}

static void CreateNode(OH_AudioSuitePipeline *pipeline, OH_AudioNode_Type type, OH_AudioNode **audioNode)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder, type);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    if ((type == AUDIOSUITE_NODE_TYPE_INPUT) || (type == AUDIOSUITE_NODE_TYPE_OUTPUT)) {
        OH_AudioFormat audioFormat;
        audioFormat.samplingRate = SAMPLE_RATE_48000;
        audioFormat.channelCount = AudioChannel::STEREO;
        audioFormat.sampleFormat = AUDIO_SAMPLE_U8;
        ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    if (type == AUDIOSUITE_NODE_TYPE_INPUT) {
        ret = OH_AudioSuiteNodeBuilder_SetOnWriteDataCallback(builder, WriteDataCallback, nullptr);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Create.
 * @tc.number: OH_AudioSuiteEngine_Create_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteUnitTest, OH_AudioSuiteEngine_Create_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Create.
 * @tc.number: OH_AudioSuiteEngine_Create_002
 * @tc.desc  : Test multiple calls.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Create_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Create.
 * @tc.number: OH_AudioSuiteEngine_Create_003
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Create_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Destroy.
 * @tc.number: OH_AudioSuiteEngine_Destroy_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Destroy_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Destroy(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Destroy.
 * @tc.number: OH_AudioSuiteEngine_Destroy_002
 * @tc.desc  : Test multiple calls.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Destroy_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_Destroy.
 * @tc.number: OH_AudioSuiteEngine_Destroy_003
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Destroy_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreatePipeline.
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_001, TestSize.Level0)
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

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreatePipeline.
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_002
 * @tc.desc  : Test engine not exit.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *audioSuitePipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ENGINE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreatePipeline.
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_003
 * @tc.desc  : Test create pipeline more than limit.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *audioSuitePipeline[MAX_PIPELINE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_PIPELINE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }
    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_PIPELINE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyPipeline(audioSuitePipeline[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreatePipeline.
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *audioSuitePipeline[MAX_PIPELINE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_PIPELINE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_PIPELINE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyPipeline(audioSuitePipeline[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyPipeline.
 * @tc.number: OH_AudioSuiteEngine_DestroyPipeline_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DestroyPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_DestroyPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyPipeline.
 * @tc.number: OH_AudioSuiteEngine_DestroyPipeline_002
 * @tc.desc  : Test engine not exit.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DestroyPipeline_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyPipeline.
 * @tc.number: OH_AudioSuiteEngine_DestroyPipeline_003
 * @tc.desc  : Test multiple calls.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DestroyPipeline_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyPipeline.
 * @tc.number: OH_AudioSuiteEngine_DestroyPipeline_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DestroyPipeline_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StartPipeline.
 * @tc.number: OH_AudioSuiteEngine_StartPipeline_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StartPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_StartPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StartPipeline.
 * @tc.number: OH_AudioSuiteEngine_StartPipeline_002
 * @tc.desc  : Test pipeline not output node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StartPipeline_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StartPipeline.
 * @tc.number: OH_AudioSuiteEngine_StartPipeline_003
 * @tc.desc  : Test must connet input node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StartPipeline_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StartPipeline.
 * @tc.number: OH_AudioSuiteEngine_StartPipeline_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StartPipeline_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StartPipeline.
 * @tc.number: OH_AudioSuiteEngine_StartPipeline_005
 * @tc.desc  : Test pipiline is running.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StartPipeline_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StopPipeline.
 * @tc.number: OH_AudioSuiteEngine_StopPipeline_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StopPipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_StopPipeline(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StopPipeline.
 * @tc.number: OH_AudioSuiteEngine_StopPipeline_002
 * @tc.desc  : Test pipeine is stopped.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StopPipeline_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_StopPipeline.
 * @tc.number: OH_AudioSuiteEngine_StopPipeline_003
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_StopPipeline_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetPipelineState.
 * @tc.number: OH_AudioSuiteEngine_GetPipelineState_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetPipelineState_001, TestSize.Level0)
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

    OH_AudioSuite_PipelineState pipelineState = AUDIOSUITE_PIPELINE_RUNNING;
    ret = OH_AudioSuiteEngine_GetPipelineState(nullptr, &pipelineState);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetPipelineState.
 * @tc.number: OH_AudioSuiteEngine_GetPipelineState_002
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetPipelineState_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_PipelineState pipelineState = AUDIOSUITE_PIPELINE_RUNNING;
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &pipelineState);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_PIPELINE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetPipelineState.
 * @tc.number: OH_AudioSuiteEngine_GetPipelineState_003
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetPipelineState_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_PipelineState pipelineState = AUDIOSUITE_PIPELINE_STOPPED;
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &pipelineState);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(pipelineState, AUDIOSUITE_PIPELINE_RUNNING);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_RenderFrame.
 * @tc.number: OH_AudioSuiteEngine_RenderFrame_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_RenderFrame_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_RenderFrame(nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_RenderFrame(pipeline, nullptr, 0, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    uint32_t audioDdata[10] = {0};
    ret = OH_AudioSuiteEngine_RenderFrame(pipeline, (void *)audioDdata, 0, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    int32_t writeSize = 0;
    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_RenderFrame.
 * @tc.number: OH_AudioSuiteEngine_RenderFrame_002
 * @tc.desc  : Test pipeline not running.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_RenderFrame_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    uint32_t audioDdata[10] = {0};
    int32_t writeSize = 0;
    bool finished = false;
    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finished);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_ILLEGAL_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_RenderFrame.
 * @tc.number: OH_AudioSuiteEngine_RenderFrame_003
 * @tc.desc  : Test success and call after finish.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_RenderFrame_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    uint32_t audioDdata[10000] = {0};
    int32_t writeSize = 0;
    bool finished = false;
    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finished);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(finished, true);

    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finished);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);
    EXPECT_EQ(finished, true);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_ConnectNodes(nullptr, nullptr,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, nullptr,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_ConnectNodes(nullptr, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_002
 * @tc.desc  : Test src node is output type or dest node is input type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(outputNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, inputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_003
 * @tc.desc  : Test src node is same dest node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_004
 * @tc.desc  : Test node not in same pipeline.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineOne = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineOne);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineTwo = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineTwo);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *srcMixNode = nullptr;
    CreateNode(pipelineOne, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &srcMixNode);

    OH_AudioNode *destMixNode = nullptr;
    CreateNode(pipelineTwo, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &destMixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(srcMixNode, destMixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(srcMixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(destMixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipelineOne);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipelineTwo);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_005
 * @tc.desc  : Test pipeline is running but dest node not mix type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_006
 * @tc.desc  : Test pipeline is running but dest not connnet from input node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_006, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_007
 * @tc.desc  : Test pipeline is running but dest node is using.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_007, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_008
 * @tc.desc  : Test pipeline is running src and dest node not used.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_008, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeTwo = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeTwo);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeTwo, nrNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_009
 * @tc.desc  : Test src and dest already connect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_009, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_ConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_ConnectNodes_010
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_ConnectNodes_010, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_DisConnectNodes(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DisConnectNodes(nullptr, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_002
 * @tc.desc  : Test src node is output type or dest node is input type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DisConnectNodes(outputNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DisConnectNodes(mixNode, inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_003
 * @tc.desc  : Test src node is same dest node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DisConnectNodes(mixNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_004
 * @tc.desc  : Test node not in same pipeline.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineOne = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineOne);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineTwo = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineTwo);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *srcMixNode = nullptr;
    CreateNode(pipelineOne, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &srcMixNode);

    OH_AudioNode *destMixNode = nullptr;
    CreateNode(pipelineTwo, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &destMixNode);

    ret = OH_AudioSuiteEngine_DisConnectNodes(srcMixNode, destMixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(srcMixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(destMixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipelineOne);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipelineTwo);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_005
 * @tc.desc  : Test pipeline is running but dest node not mix type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_006
 * @tc.desc  : Test pipeline is running, dest node is mix, but only one connect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_006, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNodeOne);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_007
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_007, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisConnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisConnectNodes_008
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisConnectNodes_008, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode,
        AUDIO_NODE_DEFAULT_OUTPORT_TYPE, AUDIO_NODE_DEFAULT_OUTPORT_TYPE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeEnableStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeEnableStatus_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeEnableStatus_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetNodeEnableStatus(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(mixNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeEnable audioNodeEnable = AUDIOSUITE_NODE_ENABLE;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(nullptr, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeEnableStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeEnableStatus_002
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeEnableStatus_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeEnable audioNodeEnable = AUDIOSUITE_NODE_ENABLE;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(mixNode, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeEnableStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeEnableStatus_003
 * @tc.desc  : Test node type not effect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeEnableStatus_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    OH_AudioNodeEnable audioNodeEnable = AUDIOSUITE_NODE_ENABLE;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(inputNode, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(outputNode, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeEnableStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeEnableStatus_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeEnableStatus_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNodeEnable audioNodeEnable = AUDIOSUITE_NODE_DISABLE;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(mixNode, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(audioNodeEnable, AUDIOSUITE_NODE_ENABLE);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_EnableNode.
 * @tc.number: OH_AudioSuiteEngine_EnableNode_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_EnableNode_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_EnableNode(nullptr, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_EnableNode.
 * @tc.number: OH_AudioSuiteEngine_EnableNode_002
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_EnableNode_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_EnableNode(mixNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_EnableNode.
 * @tc.number: OH_AudioSuiteEngine_EnableNode_003
 * @tc.desc  : Test node type not effect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_EnableNode_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_OUTPUT, &outputNode);

    ret = OH_AudioSuiteEngine_EnableNode(inputNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_EnableNode(outputNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_EnableNode.
 * @tc.number: OH_AudioSuiteEngine_EnableNode_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_EnableNode_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_EnableNode(mixNode, AUDIOSUITE_NODE_ENABLE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeEnable audioNodeEnable = AUDIOSUITE_NODE_DISABLE;
    ret = OH_AudioSuiteEngine_GetNodeEnableStatus(mixNode, &audioNodeEnable);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(audioNodeEnable, AUDIOSUITE_NODE_ENABLE);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_001, TestSize.Level0)
{
    OH_EqualizerFrequencyBandGains gains = {0};
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(nullptr, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_002
 * @tc.desc  : Test invail gains.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_EqualizerFrequencyBandGains gains = {11};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyNode(eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_003
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_EqualizerFrequencyBandGains gains = {0};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_004
 * @tc.desc  : Test node type not support.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    OH_EqualizerFrequencyBandGains gains = {0};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(inputNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_005
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_EQUALIZER, &eqNode);

    OH_EqualizerFrequencyBandGains gains = {0};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(eqNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetSoundFiledType.
 * @tc.number: OH_AudioSuiteEngine_SetSoundFiledType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetSoundFiledType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetSoundFiledType(nullptr, SOUND_FIELD_WIDE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetSoundFiledType.
 * @tc.number: OH_AudioSuiteEngine_SetSoundFiledType_002
 * @tc.desc  : Test node type not support.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetSoundFiledType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    ret = OH_AudioSuiteEngine_SetSoundFiledType(inputNode, SOUND_FIELD_WIDE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEnvironmentType.
 * @tc.number: OH_AudioSuiteEngine_SetEnvironmentType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEnvironmentType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetEnvironmentType(nullptr, ENVIRONMENT_TYPE_GRAMOPHONE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetEnvironmentType.
 * @tc.number: OH_AudioSuiteEngine_SetEnvironmentType_002
 * @tc.desc  : Test node type not support.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetEnvironmentType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    ret = OH_AudioSuiteEngine_SetEnvironmentType(inputNode, ENVIRONMENT_TYPE_GRAMOPHONE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetVoiceBeautifierType.
 * @tc.number: OH_AudioSuiteEngine_SetVoiceBeautifierType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetVoiceBeautifierType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(nullptr, VOICE_BEAUTIFIER_TYPE_NORMAL);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetVoiceBeautifierType.
 * @tc.number: OH_AudioSuiteEngine_SetVoiceBeautifierType_002
 * @tc.desc  : Test node type not support.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetVoiceBeautifierType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, AUDIOSUITE_NODE_TYPE_INPUT, &inputNode);

    ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(inputNode, VOICE_BEAUTIFIER_TYPE_NORMAL);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORT_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
