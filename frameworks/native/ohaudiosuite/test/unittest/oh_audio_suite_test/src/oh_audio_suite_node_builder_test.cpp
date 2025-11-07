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

#include "oh_audio_suite_node_builder_test.h"
#include "OHAudioSuiteEngine.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "native_audio_suite_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioSuiteNodeBuilderTest::SetUpTestCase(void) { }

void OHAudioSuiteNodeBuilderTest::TearDownTestCase(void) { }

void OHAudioSuiteNodeBuilderTest::SetUp(void) { }

void OHAudioSuiteNodeBuilderTest::TearDown(void) { }

static const uint32_t MAX_INPUT_NODE_NUM = 5;
static const uint32_t MAX_OUTPUT_NODE_NUM = 1;
static const uint32_t MAX_EFFECT_NODE_NUM = 5;
static const uint32_t MAX_MIX_NODE_NUM = 3;

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
 * @tc.name  : Test OH_AudioSuiteNodeBuilder.
 * @tc.number: OH_AudioSuiteNodeBuilder_Create_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Create_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder.
 * @tc.number: OH_AudioSuiteNodeBuilder_Create_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Create_002, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_Destroy.
 * @tc.number: OH_AudioSuiteNodeBuilder_Destroy_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Destroy_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Destroy(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_Destroy.
 * @tc.number: OH_AudioSuiteNodeBuilder_Destroy_002
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Destroy_002, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetFormat_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetFormat_001, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetFormat(nullptr, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetFormat_002
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetFormat_002, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetFormat(nullptr, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetFormat_003
 * @tc.desc  : Test audioFormat for effect node.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetFormat_003, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetFormat_004
 * @tc.desc  : Test audioFormat invaild value.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetFormat_004, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = 100;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetFormat_006
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetFormat_005, TestSize.Level0)
{
    OH_AudioFormat audioFormat;
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetRequestDataCallback.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetRequestDataCallback_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetRequestDataCallback_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetRequestDataCallback.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetRequestDataCallback_002
 * @tc.desc  : Test userData is nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetRequestDataCallback_002, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetRequestDataCallback.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetRequestDataCallback_003
 * @tc.desc  : Test node type not input node.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetRequestDataCallback_003, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetRequestDataCallback.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetRequestDataCallback_004
 * @tc.desc  : Test success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetRequestDataCallback_004, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_CreateNode(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_002
 * @tc.desc  : Test input node not set callback.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_003
 * @tc.desc  : Test input node not set audioFormat.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_004
 * @tc.desc  : Test output node not set audioFormat.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_REQUIRED_PARAMETERS_MISSING);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_005
 * @tc.desc  : Test output node set callback.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_006
 * @tc.desc  : Test effect node set audioFormat.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_006, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_007
 * @tc.desc  : Test effect node set callback.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_007, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_UNSUPPORTED_OPERATION);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_008
 * @tc.desc  : Test input node num more than limit.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_008, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode[MAX_INPUT_NODE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_INPUT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_INPUT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyNode(audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_009
 * @tc.desc  : Test output node num more than limit.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_009, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode[MAX_OUTPUT_NODE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_OUTPUT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_OUTPUT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyNode(audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_010
 * @tc.desc  : Test effect node num more than limit.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_010, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_EQUALIZER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode[MAX_EFFECT_NODE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_EFFECT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_EFFECT_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyNode(audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_011
 * @tc.desc  : Test mix node num more than limit.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_011, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode[MAX_MIX_NODE_NUM] = {nullptr};
    for (uint32_t num = 0; num < MAX_MIX_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_CREATED_EXCEED_SYSTEM_LIMITS);

    for (uint32_t num = 0; num < MAX_MIX_NODE_NUM; num++) {
        ret = OH_AudioSuiteEngine_DestroyNode(audioNode[num]);
        EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    }

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_012
 * @tc.desc  : Test input node creat success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_012, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetRequestDataCallback(builder, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_013
 * @tc.desc  : Test output node creat success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_013, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, OUTPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioFormat audioFormat;
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = AudioChannel::STEREO;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    ret = OH_AudioSuiteNodeBuilder_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_014
 * @tc.desc  : Test mix node create success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_014, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_AUDIO_MIXER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_015
 * @tc.desc  : Test EQUALIZER node create success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_015, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_EQUALIZER);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_CreateNode.
 * @tc.number: OH_AudioSuiteEngine_CreateNode_017
 * @tc.desc  : Test NOISE_REDUCTION node create success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_CreateNode_017, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyNode.
 * @tc.number: OH_AudioSuiteEngine_DestroyNode_001
 * @tc.desc  : Test nullptr.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_DestroyNode_001, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyNode.
 * @tc.number: OH_AudioSuiteEngine_DestroyNode_002
 * @tc.desc  : Test destry node when pipeline destroy.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_DestroyNode_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyNode.
 * @tc.number: OH_AudioSuiteEngine_DestroyNode_003
 * @tc.desc  : Test destry node when eninge destroy.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_DestroyNode_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_NODE_NOT_EXIST);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyNode.
 * @tc.number: OH_AudioSuiteEngine_DestroyNode_004
 * @tc.desc  : Test destry node when the pipeline running and tho node is used.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_DestroyNode_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *inputNode = nullptr;
    CreateNode(pipeline, INPUT_NODE_TYPE_DEFAULT, &inputNode);

    OH_AudioNode *outputNode = nullptr;
    CreateNode(pipeline, OUTPUT_NODE_TYPE_DEFAULT, &outputNode);

    ret = OH_AudioSuiteEngine_ConnectNodes(inputNode, outputNode);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_StartPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(inputNode);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_STATE);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_DestroyNode.
 * @tc.number: OH_AudioSuiteEngine_DestroyNode_005
 * @tc.desc  : Test destry node success.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteEngine_DestroyNode_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_Reset.
 * @tc.number: OH_AudioSuiteNodeBuilder_Reset_001
 * @tc.desc  : Test nodeBuilder reset.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Reset_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Reset(nullptr);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Reset(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_Reset.
 * @tc.number: OH_AudioSuiteNodeBuilder_Reset_002
 * @tc.desc  : Test nodeBuilder reset.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_Reset_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *node = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, builder, &node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Reset(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyNode(node);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_DestroyPipeline(pipeline);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilder_SetNodeType.
 * @tc.number: OH_AudioSuiteNodeBuilder_SetNodeType_001
 * @tc.desc  : Test setNodeType.
 */
HWTEST(OHAudioSuiteNodeBuilderTest, OH_AudioSuiteNodeBuilder_SetNodeType_001, TestSize.Level0)
{
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_SetNodeType(nullptr, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);

    OH_AudioNodeBuilder *builder = nullptr;
    ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, EFFECT_NODE_TYPE_NOISE_REDUCTION);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_SetNodeType(builder, INPUT_NODE_TYPE_DEFAULT);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
