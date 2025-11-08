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

static int32_t RequestDataCallback(OH_AudioNode *audioNode, void *userData,
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
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, type);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    if ((type == INPUT_NODE_TYPE_DEFAULT) || (type == OUTPUT_NODE_TYPE_DEFAULT)) {
        OH_AudioFormat audioFormat;
        audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
        audioFormat.channelCount = AudioChannel::STEREO;
        audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
        audioFormat.sampleFormat = AUDIO_SAMPLE_U8;
        ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    if (type == INPUT_NODE_TYPE_DEFAULT) {
        ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
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
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Create_001, TestSize.Level0)
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
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);
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
 * @tc.name  : Test OH_AudioSuiteEngine_Destroy.
 * @tc.number: OH_AudioSuiteEngine_Destroy_004
 * @tc.desc  : Test invalid OHAudioSuiteEngine.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_Destroy_004, TestSize.Level0)
{
    OH_AudioSuiteEngine* invalidInstance = reinterpret_cast<OH_AudioSuiteEngine*>(0x1234); // invalid
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_Destroy(invalidInstance);
    EXPECT_EQ(result, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreatePipeline.
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_CreatePipeline(nullptr, nullptr, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, nullptr, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &audioSuitePipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    for (uint32_t num = 0; num < MAX_PIPELINE_NUM - 1; num++) {
        ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine,
            &audioSuitePipeline[num], AUDIOSUITE_PIPELINE_EDIT_MODE);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine,
        &audioSuitePipeline[MAX_PIPELINE_NUM -1], AUDIOSUITE_PIPELINE_REALTIME_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_REALTIME_MODE);
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
    for (uint32_t num = 0; num < MAX_PIPELINE_NUM - 1; num++) {
        ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine,
            &audioSuitePipeline[num], AUDIOSUITE_PIPELINE_EDIT_MODE);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine,
        &audioSuitePipeline[MAX_PIPELINE_NUM -1], AUDIOSUITE_PIPELINE_REALTIME_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_REALTIME_MODE);
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
 * @tc.number: OH_AudioSuiteEngine_CreatePipeline_005
 * @tc.desc  : Test create pipeline more than limit.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_CreatePipeline_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_REALTIME_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);


    OH_AudioSuitePipeline *limitPipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &limitPipeline, AUDIOSUITE_PIPELINE_REALTIME_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    uint32_t audioDdata[2048] = {0};
    int32_t writeSize = 0;
    bool finish = false;
    ret = OH_AudioSuiteEngine_RenderFrame(pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finish);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StopPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    uint32_t audioDdata[2048] = {0};
    int32_t writeSize = 0;
    bool finish = false;
    ret = OH_AudioSuiteEngine_RenderFrame(pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finish);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_PipelineState pipelineState = AUDIOSUITE_PIPELINE_STOPPED;
    ret = OH_AudioSuiteEngine_GetPipelineState(pipeline, &pipelineState);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(pipelineState, AUDIOSUITE_PIPELINE_RUNNING);

    uint32_t audioDdata[2048] = {0};
    int32_t writeSize = 0;
    bool finish = false;
    ret = OH_AudioSuiteEngine_RenderFrame(pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finish);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    uint32_t audioDdata[10] = {0};
    int32_t writeSize = 0;
    bool finished = false;
    ret = OH_AudioSuiteEngine_RenderFrame(
        pipeline, (void *)audioDdata, sizeof(audioDdata), &writeSize, &finished);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
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
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);
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
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_ConnectNodes(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_ConnectNodes(nullptr, outputNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(outputNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, inputNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, mixNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineOne, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineTwo = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineTwo, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *srcMixNode = nullptr;
    CreateNode(pipelineOne, EFFECT_NODE_TYPE_AUDIO_MIXER, &srcMixNode);

    OH_AudioNode *destMixNode = nullptr;
    CreateNode(pipelineTwo, EFFECT_NODE_TYPE_AUDIO_MIXER, &destMixNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(srcMixNode, destMixNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, eqNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, mixNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeTwo = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeTwo);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    OH_AudioNode *nrNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_NOISE_REDUCTION, &nrNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeTwo, nrNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(nrNode, mixNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_DisconnectNodes(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DisconnectNodes(nullptr, outputNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_002
 * @tc.desc  : Test src node is output type or dest node is input type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DisconnectNodes(outputNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DisconnectNodes(mixNode, inputNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_003
 * @tc.desc  : Test src node is same dest node.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DisconnectNodes(mixNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_004
 * @tc.desc  : Test node not in same pipeline.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineOne = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineOne, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipelineTwo = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipelineTwo, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *srcMixNode = nullptr;
    CreateNode(pipelineOne, EFFECT_NODE_TYPE_AUDIO_MIXER, &srcMixNode);

    OH_AudioNode *destMixNode = nullptr;
    CreateNode(pipelineTwo, EFFECT_NODE_TYPE_AUDIO_MIXER, &destMixNode);

    ret = OH_AudioSuiteEngine_DisconnectNodes(srcMixNode, destMixNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_005
 * @tc.desc  : Test pipeline is running but dest node not mix type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *inputNodeThree = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeThree);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeThree, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_006
 * @tc.desc  : Test pipeline is running, dest node is mix, but only one connect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_006, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNodeOne = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNodeOne);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNodeOne, eqNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(mixNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisconnectNodes(eqNode, mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_CONNECT);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_007
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_007, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNode, outputNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_DisconnectNodes.
 * @tc.number: OH_AudioSuiteEngine_DisconnectNodes_008
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_DisconnectNodes_008, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DisconnectNodes(inputNode, outputNode);
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
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeBypassStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeBypassStatus_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeBypassStatus_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetNodeBypassStatus(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(mixNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    bool bypass = true;
    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(nullptr, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeBypassStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeBypassStatus_002
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeBypassStatus_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    bool bypass = true;
    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(mixNode, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeBypassStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeBypassStatus_003
 * @tc.desc  : Test node type not effect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeBypassStatus_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    bool bypass = true;
    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(inputNode, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(outputNode, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

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
 * @tc.name  : Test OH_AudioSuiteEngine_GetNodeBypassStatus.
 * @tc.number: OH_AudioSuiteEngine_GetNodeBypassStatus_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetNodeBypassStatus_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_BypassEffectNode(mixNode, true);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    bool bypass = false;
    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(mixNode, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(bypass, true);

    ret = OH_AudioSuiteEngine_DestroyNode(mixNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_BypassEffectNode.
 * @tc.number: OH_AudioSuiteEngine_BypassEffectNode_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_BypassEffectNode_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_BypassEffectNode(nullptr, true);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_BypassEffectNode.
 * @tc.number: OH_AudioSuiteEngine_BypassEffectNode_002
 * @tc.desc  : Test engine destroy.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_BypassEffectNode_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_BypassEffectNode(mixNode, true);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_BypassEffectNode.
 * @tc.number: OH_AudioSuiteEngine_BypassEffectNode_003
 * @tc.desc  : Test node type not effect.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_BypassEffectNode_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_BypassEffectNode(inputNode, true);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_BypassEffectNode(outputNode, true);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

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
 * @tc.name  : Test OH_AudioSuiteEngine_BypassEffectNode.
 * @tc.number: OH_AudioSuiteEngine_BypassEffectNode_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_BypassEffectNode_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *mixNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_AUDIO_MIXER, &mixNode);

    ret = OH_AudioSuiteEngine_BypassEffectNode(mixNode, true);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    bool bypass = false;
    ret = OH_AudioSuiteEngine_GetNodeBypassStatus(mixNode, &bypass);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(bypass, true);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_EqualizerFrequencyBandGains gains = {0};
    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(inputNode, gains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *eqNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &eqNode);

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
 * @tc.name  : Test OH_AudioSuiteEngine_SetSoundFieldType.
 * @tc.number: OH_AudioSuiteEngine_SetSoundFieldType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetSoundFieldType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetSoundFieldType(nullptr, SOUND_FIELD_WIDE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_SetSoundFieldType.
 * @tc.number: OH_AudioSuiteEngine_SetSoundFieldType_002
 * @tc.desc  : Test node type not support.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_SetSoundFieldType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    ret = OH_AudioSuiteEngine_SetSoundFieldType(inputNode, SOUND_FIELD_WIDE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    ret = OH_AudioSuiteEngine_SetEnvironmentType(inputNode, ENVIRONMENT_TYPE_GRAMOPHONE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

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
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(nullptr, VOICE_BEAUTIFIER_TYPE_CLEAR);
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
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(inputNode, VOICE_BEAUTIFIER_TYPE_CLEAR);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OHAudioSuiteEngine_RemovePipeline.
 * @tc.number: OHAudioSuiteEngine_RemovePipeline_001
 * @tc.desc  : Test remove pipeline success and nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OHAudioSuiteEngine_RemovePipeline_001, TestSize.Level0)
{
    OHAudioSuiteEngine* engine = OHAudioSuiteEngine::GetInstance();
    OHAudioSuitePipeline* pipeline = new OHAudioSuitePipeline(123);
    engine->AddPipeline(pipeline);
    EXPECT_TRUE(engine->IsPipelineExists(pipeline));
    engine->RemovePipeline(pipeline);
    EXPECT_FALSE(engine->IsPipelineExists(pipeline));

    engine->RemovePipeline(nullptr);
    EXPECT_FALSE(engine->IsPipelineExists(pipeline));
}

/**
 * @tc.name  : Test OHAudioSuitePipeline_RemoveNode.
 * @tc.number: OHAudioSuitePipeline_RemoveNode_001
 * @tc.desc  : Test remove node success and nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OHAudioSuitePipeline_RemoveNode_001, TestSize.Level0)
{
    OHAudioSuitePipeline* pipeline = new OHAudioSuitePipeline(123);
    OHAudioNode* eqNode = new OHAudioNode(456, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
    OHAudioNode* noiseNode = new OHAudioNode(111, AudioSuite::AudioNodeType::NODE_TYPE_NOISE_REDUCTION);
    OHAudioNode* soundNode = new OHAudioNode(222, AudioSuite::AudioNodeType::NODE_TYPE_SOUND_FIELD);
    OHAudioNode* aissNode = new OHAudioNode(333, AudioSuite::AudioNodeType::NODE_TYPE_AUDIO_SEPARATION);
    OHAudioNode* beautifierNode = new OHAudioNode(444, AudioSuite::AudioNodeType::NODE_TYPE_VOICE_BEAUTIFIER);
    pipeline->AddNode(eqNode);
    pipeline->AddNode(noiseNode);
    pipeline->AddNode(soundNode);
    pipeline->AddNode(aissNode);
    pipeline->AddNode(beautifierNode);
    EXPECT_TRUE(pipeline->IsNodeExists(eqNode));
    EXPECT_TRUE(pipeline->IsNodeExists(noiseNode));
    EXPECT_TRUE(pipeline->IsNodeExists(soundNode));
    EXPECT_TRUE(pipeline->IsNodeExists(aissNode));
    EXPECT_TRUE(pipeline->IsNodeExists(beautifierNode));

    pipeline->RemoveNode(eqNode);
    pipeline->RemoveNode(noiseNode);
    pipeline->RemoveNode(soundNode);
    pipeline->RemoveNode(aissNode);
    pipeline->RemoveNode(beautifierNode);
    EXPECT_FALSE(pipeline->IsNodeExists(eqNode));
    EXPECT_FALSE(pipeline->IsNodeExists(noiseNode));
    EXPECT_FALSE(pipeline->IsNodeExists(soundNode));
    EXPECT_FALSE(pipeline->IsNodeExists(aissNode));
    EXPECT_FALSE(pipeline->IsNodeExists(beautifierNode));

    pipeline->RemoveNode(nullptr);
    EXPECT_FALSE(pipeline->IsNodeExists(eqNode));
    delete pipeline;
}

/**
 * @tc.name  : Test OHAudioSuiteEngine_IsNodeExists.
 * @tc.number: OHAudioSuiteEngine_IsNodeExists_001
 * @tc.desc  : Test node exists.
 */
HWTEST(OHAudioSuiteEngineTest, OHAudioSuiteEngine_IsNodeExists_001, TestSize.Level0)
{
    OHAudioSuiteEngine* engine = OHAudioSuiteEngine::GetInstance();
    OHAudioSuitePipeline* pipeline = new OHAudioSuitePipeline(123);
    engine->AddPipeline(pipeline);

    OHAudioNode* node = new OHAudioNode(456, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
    pipeline->AddNode(node);
    // Conditions 1: pipeline is not nullptr, node is not nullptr, IsNodeExists is true
    {
        bool result = engine->IsNodeExists(node);
        EXPECT_TRUE(result);
    }
    // Conditions 2: pipeline is not nullptr, node is not nullptr, IsNodeExists is false
    {
        OHAudioNode* otherNode = new OHAudioNode(789, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
        bool result = engine->IsNodeExists(otherNode);
        EXPECT_FALSE(result);
        delete otherNode;
    }
    // Conditions 3: pipeline is not nullptr, node is nullptr
    {
        bool result = engine->IsNodeExists(nullptr);
        EXPECT_FALSE(result);
    }
    // Conditions 4: pipeline is nullptr, node is not nullptr
    {
        OHAudioNode* otherNode = new OHAudioNode(789, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
        bool result = engine->IsNodeExists(otherNode);
        EXPECT_FALSE(result);
        delete otherNode;
    }
    // Conditions 5: pipeline is nullptr, node is nullptr
    {
        bool result = engine->IsNodeExists(nullptr);
        EXPECT_FALSE(result);
    }
    engine->RemovePipeline(pipeline);
}

/**
 * @tc.name  : Test OHAudioSuiteEngine_RemoveNode.
 * @tc.number: OHAudioSuiteEngine_RemoveNode_001
 * @tc.desc  : Test remove node.
 */
HWTEST(OHAudioSuiteEngineTest, OHAudioSuiteEngine_RemoveNode_001, TestSize.Level0)
{
    OHAudioSuiteEngine* engine = OHAudioSuiteEngine::GetInstance();
    OHAudioSuitePipeline* pipeline = new OHAudioSuitePipeline(123);
    engine->AddPipeline(pipeline);
    OHAudioNode* node = new OHAudioNode(456, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
    pipeline->AddNode(node);

    // Conditions 1: pipeline is not nullptr, node is not nullptr, IsNodeExists is true
    {
        engine->RemoveNode(node);
        EXPECT_FALSE(pipeline->IsNodeExists(node));
    }
    // Conditions 2: pipeline is not nullptr, node is not nullptr, IsNodeExists is false
    {
        OHAudioNode* otherNode = new OHAudioNode(789, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
        engine->RemoveNode(otherNode);
        EXPECT_FALSE(pipeline->IsNodeExists(otherNode));
    }
    // Conditions 3: pipeline is not nullptr, node is nullptr
    {
        engine->RemoveNode(nullptr);
        EXPECT_FALSE(pipeline->IsNodeExists(node));
    }
    // Conditions 4: pipeline is nullptr, node is not nullptr
    {
        OHAudioNode* otherNode = new OHAudioNode(888, AudioSuite::AudioNodeType::NODE_TYPE_EQUALIZER);
        engine->RemovePipeline(pipeline);
        engine->RemoveNode(otherNode);
        EXPECT_FALSE(engine->IsNodeExists(otherNode));
        delete otherNode;
    }
    // Conditions 5: pipeline is nullptr, node is nullptr
    {
        engine->RemoveNode(nullptr);
        EXPECT_FALSE(engine->IsNodeExists(node));
    }
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetEnvironmentType.
 * @tc.number: OH_AudioSuiteEngine_GetEnvironmentType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetEnvironmentType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetEnvironmentType(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &audioNode);

    ret = OH_AudioSuiteEngine_GetEnvironmentType(audioNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_EnvironmentType environmentType;
    ret = OH_AudioSuiteEngine_GetEnvironmentType(audioNode, &environmentType);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetEnvironmentType.
 * @tc.number: OH_AudioSuiteEngine_GetEnvironmentType_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetEnvironmentType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_ENVIRONMENT_EFFECT, &audioNode);

    ret = OH_AudioSuiteEngine_SetEnvironmentType(audioNode, ENVIRONMENT_TYPE_EARPIECE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_EnvironmentType environmentType;
    ret = OH_AudioSuiteEngine_GetEnvironmentType(audioNode, &environmentType);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(environmentType, ENVIRONMENT_TYPE_EARPIECE);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetSoundFieldType.
 * @tc.number: OH_AudioSuiteEngine_GetSoundFieldType_001
 * @tc.desc  : Test nullptr and not support node type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetSoundFieldType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetSoundFieldType(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &audioNode);

    ret = OH_AudioSuiteEngine_GetSoundFieldType(audioNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_SoundFieldType soundFieldType;
    ret = OH_AudioSuiteEngine_GetSoundFieldType(audioNode, &soundFieldType);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetSoundFieldType.
 * @tc.number: OH_AudioSuiteEngine_GetSoundFieldType_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetSoundFieldType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_SOUND_FIELD, &audioNode);

    ret = OH_AudioSuiteEngine_SetSoundFieldType(audioNode, SOUND_FIELD_NEAR);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_SoundFieldType soundFieldType;
    ret = OH_AudioSuiteEngine_GetSoundFieldType(audioNode, &soundFieldType);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(soundFieldType, SOUND_FIELD_NEAR);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains_001
 * @tc.desc  : Test nullptr and not support node type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &audioNode);

    ret = OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(audioNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_EqualizerFrequencyBandGains frequencyBandGains;
    ret = OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(audioNode, &frequencyBandGains);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains.
 * @tc.number: OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_EQUALIZER, &audioNode);

    ret = OH_AudioSuiteEngine_SetEqualizerFrequencyBandGains(audioNode, OH_EQUALIZER_PARAM_BALLADS);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_EqualizerFrequencyBandGains frequencyBandGains;
    ret = OH_AudioSuiteEngine_GetEqualizerFrequencyBandGains(audioNode, &frequencyBandGains);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetVoiceBeautifierType.
 * @tc.number: OH_AudioSuiteEngine_GetVoiceBeautifierType_001
 * @tc.desc  : Test nullptr and not support node type.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetVoiceBeautifierType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_GetVoiceBeautifierType(nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &audioNode);

    ret = OH_AudioSuiteEngine_GetVoiceBeautifierType(audioNode, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_VoiceBeautifierType voiceBeautifierType;
    ret = OH_AudioSuiteEngine_GetVoiceBeautifierType(audioNode, &voiceBeautifierType);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}
 
/**
 * @tc.name  : Test OH_AudioSuiteEngine_GetVoiceBeautifierType.
 * @tc.number: OH_AudioSuiteEngine_GetVoiceBeautifierType_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_GetVoiceBeautifierType_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    CreateNode(pipeline, EFFECT_NODE_TYPE_VOICE_BEAUTIFIER, &audioNode);

    ret = OH_AudioSuiteEngine_SetVoiceBeautifierType(audioNode, VOICE_BEAUTIFIER_TYPE_CD);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_VoiceBeautifierType voiceBeautifierType;
    ret = OH_AudioSuiteEngine_GetVoiceBeautifierType(audioNode, &voiceBeautifierType);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(voiceBeautifierType, VOICE_BEAUTIFIER_TYPE_CD);

    ret = OH_AudioSuiteEngine_DestroyNode(audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MultiRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MultiRenderFrame_001
 * @tc.desc  : Test nullptr and invalid data.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MultiRenderFrame_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_MultiRenderFrame(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioDataArray audioDataArray;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    int32_t responseSize = 0;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    bool finishedFlag = false;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    void *audioData[10] = {0};
    audioDataArray.audioDataArray = (void **)audioData;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    audioDataArray.arraySize = 1;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MultiRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MultiRenderFrame_002
 * @tc.desc  : Test pipeline not running.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MultiRenderFrame_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioDataArray audioDataArray;
    uint32_t audioData[10] = {0};
    void *dataArray = (void*)audioData;
    audioDataArray.audioDataArray = &dataArray;
    audioDataArray.arraySize = 1;
    audioDataArray.requestFrameSize = sizeof(audioData);
    int32_t responseSize = 0;
    bool finishedFlag = false;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MultiRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MultiRenderFrame_003
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MultiRenderFrame_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioDataArray audioDataArray;
    uint32_t audioData[10] = {0};
    void *dataArray = (void*)audioData;
    audioDataArray.audioDataArray = &dataArray;
    audioDataArray.arraySize = 1;
    audioDataArray.requestFrameSize = sizeof(audioData);
    int32_t responseSize = 0;
    bool finishedFlag = false;
    ret = OH_AudioSuiteEngine_MultiRenderFrame(pipeline, &audioDataArray, &responseSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_IsNodeTypeSupported001.
 * @tc.number: OH_AudioSuiteEngine_IsNodeTypeSupported001
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_IsNodeTypeSupported001, TestSize.Level0)
{
    bool isSupported = false;
    OH_AudioSuite_Result ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_EQUALIZER, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_NOISE_REDUCTION, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_SOUND_FIELD, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_VOICE_BEAUTIFIER, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_ENVIRONMENT_EFFECT, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(EFFECT_NODE_TYPE_AUDIO_MIXER, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(INPUT_NODE_TYPE_DEFAULT, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);

    ret =  OH_AudioSuiteEngine_IsNodeTypeSupported(OUTPUT_NODE_TYPE_DEFAULT, &isSupported);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    EXPECT_EQ(isSupported, true);
}

} // namespace AudioStandard
} // namespace OHOS
