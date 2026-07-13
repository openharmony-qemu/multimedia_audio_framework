/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <filesystem>
#include "oh_audio_suite_engine_test.h"
#include "OHAudioSuiteEngine.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "native_audio_suite_engine.h"
#include "oh_audio_suite_tool_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetNodeType.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetNodeType_001
 * @tc.desc  : Test nullptr.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetNodeType_001, TestSize.Level0)
{
    int32_t ret = OH_AudioSuiteNodeBuilderSystem_SetNodeType(
        nullptr, OH_AUDIOSUITE_EFFECT_NODE_SYSTEM_TYPE_DIALOGUE_ENHANCE);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetNodeType.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetNodeType_002
 * @tc.desc  : Test valid node type.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetNodeType_002, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetNodeType(
        builder, OH_AUDIOSUITE_EFFECT_NODE_SYSTEM_TYPE_DIALOGUE_ENHANCE);
    EXPECT_EQ(ret2, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetNodeType.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetNodeType_003
 * @tc.desc  : Test another valid node type.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetNodeType_003, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetNodeType(
        builder, OH_AUDIOSUITE_EFFECT_NODE_SYSTEM_TYPE_VOICE_BEAUTIFIER);
    EXPECT_EQ(ret2, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetFormat_001
 * @tc.desc  : Test nullptr.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetFormat_001, TestSize.Level0)
{
    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    int32_t ret = OH_AudioSuiteNodeBuilderSystem_SetFormat(nullptr, audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetFormat_002
 * @tc.desc  : Test valid format.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetFormat_002, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = 2;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    audioFormat.sampleFormat = AUDIO_SAMPLE_S16LE;
    audioFormat.encoding = 0;

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret2, AUDIOSUITE_SUCCESS);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetFormat_003
 * @tc.desc  : Test invalid channel layout.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetFormat_003, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = 2;
    audioFormat.channelLayout = static_cast<OH_AudioChannelLayout>(9999);
    audioFormat.sampleFormat = AUDIO_SAMPLE_S16LE;
    audioFormat.encoding = 0;

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetFormat_004
 * @tc.desc  : Test invalid channel count.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetFormat_004, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = 5;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    audioFormat.sampleFormat = AUDIO_SAMPLE_S16LE;
    audioFormat.encoding = 0;

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteNodeBuilderSystem_SetFormat.
 * @tc.number: OH_AudioSuiteNodeBuilderSystem_SetFormat_005
 * @tc.desc  : Test invalid encoding.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteNodeBuilderSystem_SetFormat_005, TestSize.Level0)
{
    OH_AudioNodeBuilder *builder = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteNodeBuilder_Create(&builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    audioFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    audioFormat.channelCount = 2;
    audioFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    audioFormat.sampleFormat = AUDIO_SAMPLE_S16LE;
    audioFormat.encoding = 5;

    int32_t ret2 = OH_AudioSuiteNodeBuilderSystem_SetFormat(builder, audioFormat);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT);

    ret = OH_AudioSuiteNodeBuilder_Destroy(builder);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_SetAudioFormat.
 * @tc.number: OH_AudioSuiteEngineSystem_SetAudioFormat_001
 * @tc.desc  : Test nullptr node.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_SetAudioFormat_001, TestSize.Level0)
{
    OH_AudioSuite_SystemNodeFormat audioFormat = {};
    int32_t ret = OH_AudioSuiteEngineSystem_SetAudioFormat(nullptr, &audioFormat);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_SetAudioFormat.
 * @tc.number: OH_AudioSuiteEngineSystem_SetAudioFormat_002
 * @tc.desc  : Test nullptr format.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_SetAudioFormat_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    int32_t ret2 = OH_AudioSuiteEngineSystem_SetAudioFormat(audioNode, nullptr);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_SetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_SetNodeParam_001
 * @tc.desc  : Test nullptr node.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_SetNodeParam_001, TestSize.Level0)
{
    uint8_t param[10] = {0};
    int32_t ret = OH_AudioSuiteEngineSystem_SetNodeParam(nullptr, param, 10);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_SetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_SetNodeParam_002
 * @tc.desc  : Test nullptr param.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_SetNodeParam_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    int32_t ret2 = OH_AudioSuiteEngineSystem_SetNodeParam(audioNode, nullptr, 10);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_SetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_SetNodeParam_003
 * @tc.desc  : Test zero param size.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_SetNodeParam_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    uint8_t param[10] = {0};
    int32_t ret2 = OH_AudioSuiteEngineSystem_SetNodeParam(audioNode, param, 0);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeParam_001
 * @tc.desc  : Test nullptr node.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeParam_001, TestSize.Level0)
{
    uint8_t param[10] = {0};
    int32_t ret = OH_AudioSuiteEngineSystem_GetNodeParam(nullptr, param, 10);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeParam_002
 * @tc.desc  : Test nullptr param.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeParam_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    int32_t ret2 = OH_AudioSuiteEngineSystem_GetNodeParam(audioNode, nullptr, 10);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeParam.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeParam_003
 * @tc.desc  : Test zero param size.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeParam_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    uint8_t param[10] = {0};
    int32_t ret2 = OH_AudioSuiteEngineSystem_GetNodeParam(audioNode, param, 0);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeInOutSize.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeInOutSize_001
 * @tc.desc  : Test nullptr node.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeInOutSize_001, TestSize.Level0)
{
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    int32_t ret = OH_AudioSuiteEngineSystem_GetNodeInOutSize(nullptr, &inSize, &outSize);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeInOutSize.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeInOutSize_002
 * @tc.desc  : Test nullptr inSize.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeInOutSize_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    uint32_t outSize = 0;
    int32_t ret2 = OH_AudioSuiteEngineSystem_GetNodeInOutSize(audioNode, nullptr, &outSize);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngineSystem_GetNodeInOutSize.
 * @tc.number: OH_AudioSuiteEngineSystem_GetNodeInOutSize_003
 * @tc.desc  : Test nullptr outSize.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngineSystem_GetNodeInOutSize_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
    OH_AudioNode *audioNode = nullptr;
    ret = OH_AudioSuiteEngine_CreateNode(pipeline, nullptr, &audioNode);
    EXPECT_NE(ret, AUDIOSUITE_SUCCESS);

    uint32_t inSize = 0;
    int32_t ret2 = OH_AudioSuiteEngineSystem_GetNodeInOutSize(audioNode, &inSize, nullptr);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MetaRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MetaRenderFrame_001
 * @tc.desc  : Test nullptr pipeline.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MetaRenderFrame_001, TestSize.Level0)
{
    OH_AudioSuite_MetaFrame metaFrame = {};
    int32_t responseAudioSize = 0;
    int32_t responseMetaSize = 0;
    bool finishedFlag = false;
    int32_t ret = OH_AudioSuiteEngine_MetaRenderFrame(
        nullptr, &metaFrame, &responseAudioSize, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret, AUDIOSUITE_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MetaRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MetaRenderFrame_002
 * @tc.desc  : Test nullptr metaFrame.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MetaRenderFrame_002, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    int32_t responseAudioSize = 0;
    int32_t responseMetaSize = 0;
    bool finishedFlag = false;
    int32_t ret2 = OH_AudioSuiteEngine_MetaRenderFrame(
        pipeline, nullptr, &responseAudioSize, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MetaRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MetaRenderFrame_003
 * @tc.desc  : Test nullptr responseAudioSize.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MetaRenderFrame_003, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_MetaFrame metaFrame = {};
    int32_t responseMetaSize = 0;
    bool finishedFlag = false;
    int32_t ret2 = OH_AudioSuiteEngine_MetaRenderFrame(
        pipeline, &metaFrame, nullptr, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MetaRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MetaRenderFrame_004
 * @tc.desc  : Test nullptr responseMetaSize.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MetaRenderFrame_004, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_MetaFrame metaFrame = {};
    int32_t responseAudioSize = 0;
    bool finishedFlag = false;
    int32_t ret2 = OH_AudioSuiteEngine_MetaRenderFrame(
        pipeline, &metaFrame, &responseAudioSize, nullptr, &finishedFlag);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioSuiteEngine_MetaRenderFrame.
 * @tc.number: OH_AudioSuiteEngine_MetaRenderFrame_005
 * @tc.desc  : Test nullptr finishedFlag.
 */
HWTEST_F(OHAudioSuiteEngineTest, OH_AudioSuiteEngine_MetaRenderFrame_005, TestSize.Level0)
{
    OH_AudioSuiteEngine *audioSuiteEngine = nullptr;
    OH_AudioSuite_Result ret = OH_AudioSuiteEngine_Create(&audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuitePipeline *pipeline = nullptr;
    ret = OH_AudioSuiteEngine_CreatePipeline(audioSuiteEngine, &pipeline, AUDIOSUITE_PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);

    OH_AudioSuite_MetaFrame metaFrame = {};
    int32_t responseAudioSize = 0;
    int32_t responseMetaSize = 0;
    int32_t ret2 = OH_AudioSuiteEngine_MetaRenderFrame(
        pipeline, &metaFrame, &responseAudioSize, &responseMetaSize, nullptr);
    EXPECT_EQ(ret2, AUDIOSUITE_ERROR_INVALID_PARAM);

    ret = OH_AudioSuiteEngine_Destroy(audioSuiteEngine);
    EXPECT_EQ(ret, AUDIOSUITE_SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
