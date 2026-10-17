/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <cstdlib>
#include <string>
#include <thread>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "audio_suite_pipeline_impl.h"
#include "audio_suite_base.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_output_node.h"
#include "audio_suite_manager_test_common.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
// Test node IDs for DFX tests
static constexpr uint32_t TEST_INPUT_NODE_ID = 1;
static constexpr uint32_t TEST_SPACERENDER_NODE_ID = 2;
static constexpr uint32_t TEST_OUTPUT_NODE_ID = 3;
static constexpr uint32_t TEST_EQUALIZER_NODE_ID = 2;
static constexpr uint32_t TEST_TEMPOPITCH_NODE_ID = 2;
static constexpr uint32_t TEST_AUDIOMIXER_NODE_ID = 4;
// Invalid node ID for testing non-existent nodes
static constexpr uint32_t TEST_INVALID_NODE_ID = 999;

// Helper function to create a simple pipeline with space render node
static std::shared_ptr<AudioSuitePipelineImpl> CreateSpaceRenderPipeline()
{
    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    auto spaceRenderNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_SPACE_RENDER);
    spaceRenderNode->audioNodeInfo_.nodeId = TEST_SPACERENDER_NODE_ID;
    spaceRenderNode->SetAudioNodeFormat(format);

    auto outputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    outputNode->audioNodeInfo_.nodeId = TEST_OUTPUT_NODE_ID;
    outputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;
    pipeline->nodeMap_[TEST_SPACERENDER_NODE_ID] = spaceRenderNode;
    pipeline->nodeMap_[TEST_OUTPUT_NODE_ID] = outputNode;
    pipeline->connections_[TEST_INPUT_NODE_ID] = TEST_SPACERENDER_NODE_ID;
    pipeline->connections_[TEST_SPACERENDER_NODE_ID] = TEST_OUTPUT_NODE_ID;
    pipeline->reverseConnections_[TEST_SPACERENDER_NODE_ID] = {TEST_INPUT_NODE_ID};
    pipeline->reverseConnections_[TEST_OUTPUT_NODE_ID] = {TEST_SPACERENDER_NODE_ID};

    return pipeline;
}

// Engine creation: The engine already exists.
HWTEST_F(AudioSuiteEngineManagerUnitTest, duplicateInitTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    // InitGlobal is idempotent — second call is a no-op
    AudioSuitePipelineImpl::InitGlobal();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, duplicateDeInitTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();
    AudioSuitePipelineImpl::DeInitGlobal();

    // Should not crash on duplicate DeInit
    AudioSuitePipelineImpl::DeInitGlobal();
}

// Pipeline creation: Create lazily calls InitGlobal, so it always succeeds.
HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineWithoutEngineTest, TestSize.Level0)
{
    // Without prior InitGlobal, Create lazily initializes and succeeds
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);
    pipeline.reset();

    AudioSuitePipelineImpl::DeInitGlobal();

    // After DeInit, Create lazily re-initializes and succeeds
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);
    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

// Pipeline creation: The number of pipelines exceeds the limit.
HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineExceedLimitTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::vector<std::shared_ptr<AudioSuitePipeline>> pipelines;
    for (uint32_t num = 0; num < 10 - 1; num++) {
        std::shared_ptr<AudioSuitePipeline> pipeline;
        EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
        EXPECT_NE(pipeline, nullptr);
        pipelines.push_back(pipeline);
    }
    std::shared_ptr<AudioSuitePipeline> pipeline10;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_REALTIME_MODE, pipeline10), SUCCESS);
    EXPECT_NE(pipeline10, nullptr);
    pipelines.push_back(pipeline10);

    std::shared_ptr<AudioSuitePipeline> exceed1;
    EXPECT_NE(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, exceed1), SUCCESS);

    std::shared_ptr<AudioSuitePipeline> exceed2;
    EXPECT_NE(AudioSuitePipelineImpl::Create(PIPELINE_REALTIME_MODE, exceed2), SUCCESS);

    pipelines.clear();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createNodeExceedLimitTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_INPUT;

    for (uint32_t num = 0; num < AudioSuitePipelineCfg{}.maxInputNodeNum_; num++) {
        int32_t result = audioSuitePipeline.CreateNodeCheckParam(builder);
        EXPECT_EQ(result, SUCCESS);
        audioSuitePipeline.nodeCounts_[builder.nodeType]++;
    }

    int32_t result = audioSuitePipeline.CreateNodeCheckParam(builder);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Node parameter setting: The user-defined parameter is invalid or illegal.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsInvalidParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = CreateSpaceRenderPipeline();
    // Replace space render node with EQ node
    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    auto eqNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_EQUALIZER);
    eqNode->audioNodeInfo_.nodeId = TEST_EQUALIZER_NODE_ID;
    eqNode->SetAudioNodeFormat(format);
    pipeline->nodeMap_[TEST_EQUALIZER_NODE_ID] = eqNode;

    AudioEqualizerFrequencyBandGains gains;
    gains.gains[0] = 11;
    gains.gains[1] = -11;
    for (int i = 2; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = 0;
    }

    int32_t ret = pipeline->SetEqualizerFrequencyBandGains(TEST_EQUALIZER_NODE_ID, gains);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// The node parameter setting node does not exist.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsNodeNotExistTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = CreateSpaceRenderPipeline();

    AudioEqualizerFrequencyBandGains gains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = 0;
    }

    // Test the node ID 999 that does not exist — SetOptions returns ERR_INVALID_PARAM for non-existent node
    int32_t ret = pipeline->SetEqualizerFrequencyBandGains(TEST_INVALID_NODE_ID, gains);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// The node type not support AudioEqualizerFrequencyBandGains.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsNodeTypeNotSupportTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;

    AudioEqualizerFrequencyBandGains gains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = 0;
    }

    // Test the INPUT node type does not support AudioEqualizerFrequencyBandGains.
    int32_t ret = pipeline->SetEqualizerFrequencyBandGains(TEST_INPUT_NODE_ID, gains);
    EXPECT_EQ(ret, ERROR);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetTempoAndPitch with invalid speed and pitch range.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetTempoAndPitchInvalidParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    auto tempoPitchNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_TEMPO_PITCH);
    tempoPitchNode->audioNodeInfo_.nodeId = TEST_TEMPOPITCH_NODE_ID;
    tempoPitchNode->SetAudioNodeFormat(format);

    auto outputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    outputNode->audioNodeInfo_.nodeId = TEST_OUTPUT_NODE_ID;
    outputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;
    pipeline->nodeMap_[TEST_TEMPOPITCH_NODE_ID] = tempoPitchNode;
    pipeline->nodeMap_[TEST_OUTPUT_NODE_ID] = outputNode;
    pipeline->connections_[TEST_INPUT_NODE_ID] = TEST_TEMPOPITCH_NODE_ID;
    pipeline->connections_[TEST_TEMPOPITCH_NODE_ID] = TEST_OUTPUT_NODE_ID;
    pipeline->reverseConnections_[TEST_TEMPOPITCH_NODE_ID] = {TEST_INPUT_NODE_ID};
    pipeline->reverseConnections_[TEST_OUTPUT_NODE_ID] = {TEST_TEMPOPITCH_NODE_ID};

    // Test speed out of range (speed must be in 0.5~10.0)
    int32_t ret = pipeline->SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 0.4f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = pipeline->SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 10.1f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test pitch out of range (pitch must be in 0.1~5.0)
    ret = pipeline->SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 1.0f, 0.09f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = pipeline->SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 1.0f, 5.1f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test both speed and pitch out of range
    ret = pipeline->SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 0.4f, 0.09f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetPureVoiceChangeOption with invalid pitch range.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetPureVoiceChangeOptionInvalidParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    // Test pitch out of range (pitch must be in 0.3f~3.0f or 0.0f)
    AudioPureVoiceChangeOption option;
    option.optionGender = PURE_VOICE_CHANGE_FEMALE;
    option.optionType = PURE_VOICE_CHANGE_TYPE_SEASONED;
    option.pitch = 0.29f;
    int32_t ret = pipeline->SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    option.pitch = 3.01f;
    ret = pipeline->SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Valid pitch values pass param validation, but node 2 doesn't exist — SetOptions returns ERR_INVALID_PARAM
    option.pitch = 0.3f;
    ret = pipeline->SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    option.pitch = 3.0f;
    ret = pipeline->SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    option.pitch = 0.0f;
    ret = pipeline->SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderExtensionParamsInvalidParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    // Test extRadius out of range (must be in 1.0f~5.0f)
    AudioSpaceRenderExtensionParams extension;
    extension.extRadius = 0.9f;
    extension.extAngle = 180;
    int32_t ret = pipeline->SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extRadius = 5.1f;
    ret = pipeline->SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test extAngle out of range (must be in [1, 359])
    extension.extRadius = 3.0f;
    extension.extAngle = 0;
    ret = pipeline->SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extAngle = 361;
    ret = pipeline->SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extRadius = 5.0f;
    extension.extAngle = 360;
    ret = pipeline->SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderExtensionParamsParamTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    uint32_t inputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    int32_t ret = pipeline->CreateNode(inputBuilder, inputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t spaceRenderNodeId = INVALID_NODE_ID;
    AudioNodeBuilder spaceRenderBuilder;
    spaceRenderBuilder.nodeType = NODE_TYPE_SPACE_RENDER;
    ret = pipeline->CreateNode(spaceRenderBuilder, spaceRenderNodeId);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t outputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder outputBuilder;
    outputBuilder.nodeType = NODE_TYPE_OUTPUT;
    ret = pipeline->CreateNode(outputBuilder, outputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = pipeline->ConnectNodes(inputNodeId, spaceRenderNodeId);
    EXPECT_EQ(ret, SUCCESS);
    ret = pipeline->ConnectNodes(spaceRenderNodeId, outputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    AudioSpaceRenderExtensionParams extension;
    extension.extRadius = 1.0f;
    extension.extAngle = 1;
    ret = pipeline->SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    extension.extRadius = 5.0f;
    extension.extAngle = 359;
    ret = pipeline->SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    extension.extRadius = 3.0f;
    extension.extAngle = 180;
    ret = pipeline->SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEnvironmentTypeInvalidEnumValueTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    // Create environment effect node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    int32_t ret = pipeline->CreateNode(builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = pipeline->SetEnvironmentType(nodeId, static_cast<EnvironmentType>(10));
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Destroy node
    ret = pipeline->DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetVoiceBeautifierTypeInvalidEnumValueTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    // Create voice beautifier node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_VOICE_BEAUTIFIER;
    int32_t ret = pipeline->CreateNode(builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = pipeline->SetVoiceBeautifierType(nodeId, static_cast<VoiceBeautifierType>(10));
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Destroy node
    ret = pipeline->DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetGeneralVoiceChangeTypeInvalidEnumValueTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    // Create general voice change node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_GENERAL_VOICE_CHANGE;
    int32_t ret = pipeline->CreateNode(builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (20) to test deep layer fault interception
    ret = pipeline->SetGeneralVoiceChangeType(nodeId, static_cast<AudioGeneralVoiceChangeType>(20));
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Destroy node
    ret = pipeline->DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSoundFieldTypeInvalidEnumValueTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    // Create sound field effect node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_SOUND_FIELD;
    int32_t ret = pipeline->CreateNode(builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = pipeline->SetSoundFieldType(nodeId, static_cast<SoundFieldType>(10));
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Destroy node
    ret = pipeline->DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

// The current status of the node connection process pipeline is invalid.
HWTEST_F(AudioSuiteEngineManagerUnitTest, ConnectNodesRunningPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();
    AudioSuitePipelineImpl pipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode1->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode1->SetAudioNodeFormat(format);

    auto outputNode = std::make_shared<AudioOutputNode>(format);
    outputNode->audioNodeInfo_.nodeId = TEST_OUTPUT_NODE_ID;
    outputNode->SetAudioNodeFormat(format);

    auto mixNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixNode->audioNodeInfo_.nodeId = TEST_AUDIOMIXER_NODE_ID;
    mixNode->SetAudioNodeFormat(format);

    pipeline.nodeMap_[TEST_INPUT_NODE_ID] = inputNode1;
    pipeline.nodeMap_[TEST_OUTPUT_NODE_ID] = outputNode;
    pipeline.nodeMap_[TEST_AUDIOMIXER_NODE_ID] = mixNode;

    pipeline.connections_[TEST_INPUT_NODE_ID] = TEST_AUDIOMIXER_NODE_ID;
    pipeline.connections_[TEST_AUDIOMIXER_NODE_ID] = TEST_OUTPUT_NODE_ID;
    pipeline.reverseConnections_[TEST_AUDIOMIXER_NODE_ID] = {TEST_INPUT_NODE_ID};
    pipeline.reverseConnections_[TEST_OUTPUT_NODE_ID] = {TEST_AUDIOMIXER_NODE_ID};

    pipeline.outputNode_ = outputNode;
    pipeline.pipelineState_ = PIPELINE_RUNNING;

    // inputNode1 is already connected to outputNode via mixNode
    int32_t ret = pipeline.ConnectNodesForRun(TEST_INPUT_NODE_ID, TEST_AUDIOMIXER_NODE_ID, inputNode1, mixNode);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test stop pipeline when pipeline is already stopped, expect ERR_ILLEGAL_STATE error.
HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineStopWhenAlreadyStoppedTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    auto outputNode = std::make_shared<AudioOutputNode>(format);
    outputNode->audioNodeInfo_.nodeId = TEST_OUTPUT_NODE_ID;
    outputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;
    pipeline->nodeMap_[TEST_OUTPUT_NODE_ID] = outputNode;
    pipeline->outputNode_ = outputNode;

    // Pipeline is already in STOPPED state by default
    pipeline->pipelineState_ = PIPELINE_STOPPED;

    // Try to stop pipeline when it's already stopped, should return ERR_ILLEGAL_STATE directly
    int32_t ret = pipeline->Stop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetSpaceRenderPositionParams with invalid parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderPositionParamsInvalidParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioSpaceRenderPositionParams position;
    position.x = 6.0f;
    position.y = 6.0f;
    position.z = 6.0f;

    int32_t ret = pipeline->SetSpaceRenderPositionParams(2, position);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    position.x = -6.0f;
    position.y = -6.0f;
    position.z = -6.0f;

    ret = pipeline->SetSpaceRenderPositionParams(2, position);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisconnectNodesNodeTypeErrorTest, TestSize.Level0)
{
    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    uint32_t inputNodeId = INVALID_NODE_ID;
    uint32_t mixerNodeId = INVALID_NODE_ID;
    uint32_t outputNodeId = INVALID_NODE_ID;

    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    pipeline->CreateNode(inputBuilder, inputNodeId);

    AudioNodeBuilder mixerBuilder;
    mixerBuilder.nodeType = NODE_TYPE_AUDIO_MIXER;
    pipeline->CreateNode(mixerBuilder, mixerNodeId);

    AudioNodeBuilder outputBuilder;
    outputBuilder.nodeType = NODE_TYPE_OUTPUT;
    pipeline->CreateNode(outputBuilder, outputNodeId);

    pipeline->ConnectNodes(inputNodeId, mixerNodeId);
    pipeline->ConnectNodes(mixerNodeId, outputNodeId);

    // Test 1: Try to disconnect OUTPUT node as source (should fail)
    int32_t result = pipeline->DisconnectNodes(outputNodeId, mixerNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    // Test 2: Try to disconnect INPUT node as destination (should fail)
    result = pipeline->DisconnectNodes(mixerNodeId, inputNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetEqualizerFrequencyBandGains when node does not exist, expect error report.
HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetOptionsErrorReportTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;

    AudioEqualizerFrequencyBandGains gains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = 0;
    }

    // Test SetEqualizerFrequencyBandGains with node ID 999 that does not exist,
    // SetOptions returns ERR_INVALID_PARAM for non-existent node
    int32_t ret = pipeline->SetEqualizerFrequencyBandGains(TEST_INVALID_NODE_ID, gains);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetSpaceRenderRotationParams with invalid x, y, z parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidXYZTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = CreateSpaceRenderPipeline();

    AudioSpaceRenderRotationParams rotation;
    // Test x, y, z upper bound
    rotation.x = 6.0f;
    rotation.y = 6.0f;
    rotation.z = 6.0f;
    rotation.surroundTime = 5.0f;
    rotation.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;
    EXPECT_EQ(pipeline->SetSpaceRenderRotationParams(TEST_SPACERENDER_NODE_ID, rotation), ERR_INVALID_PARAM);

    // Test x, y, z lower bound
    rotation.x = -6.0f;
    rotation.y = -6.0f;
    rotation.z = -6.0f;
    EXPECT_EQ(pipeline->SetSpaceRenderRotationParams(TEST_SPACERENDER_NODE_ID, rotation), ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetSpaceRenderRotationParams with invalid surroundTime parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidSurroundTimeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = CreateSpaceRenderPipeline();

    AudioSpaceRenderRotationParams rotation;
    rotation.x = 1.0f;
    rotation.y = 2.0f;
    rotation.z = 3.0f;
    rotation.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;

    // Test surroundTime lower bound
    rotation.surroundTime = 1.0f;
    EXPECT_EQ(pipeline->SetSpaceRenderRotationParams(TEST_SPACERENDER_NODE_ID, rotation), ERR_INVALID_PARAM);

    // Test surroundTime upper bound
    rotation.surroundTime = 41.0f;
    EXPECT_EQ(pipeline->SetSpaceRenderRotationParams(TEST_SPACERENDER_NODE_ID, rotation), ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

// Test SetSpaceRenderRotationParams with invalid surroundDirection parameter.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidSurroundDirectionTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = CreateSpaceRenderPipeline();

    AudioSpaceRenderRotationParams rotation;
    rotation.x = 1.0f;
    rotation.y = 2.0f;
    rotation.z = 3.0f;
    rotation.surroundTime = 5.0f;
    rotation.surroundDirection = static_cast<AudioSurroundDirection>(2);
    EXPECT_EQ(pipeline->SetSpaceRenderRotationParams(TEST_SPACERENDER_NODE_ID, rotation), ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, startPipelineNoOutputNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    // Synchronous: pipeline has no output node, should return ERR_ILLEGAL_STATE
    int32_t result = pipeline->Start();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest_002, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    uint8_t audioData[10] = {0};
    int32_t writeLen = 0;
    bool finished = true;
    // Synchronous: pipeline state is STOPPED (default), should return ERR_ILLEGAL_STATE directly
    int32_t result = pipeline->RenderFrame(audioData, sizeof(audioData), &writeLen, &finished);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest_003, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    AudioFormat inputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_U8, SAMPLE_RATE_48000};
    inputBuilder.nodeFormat = inputFormat;
    auto inputNode = pipeline->CreateNodeForType(inputBuilder);
    inputNode->Init();
    uint32_t inputNodeId = 1;
    inputNode->audioNodeInfo_.nodeId = inputNodeId;
    pipeline->nodeMap_[inputNodeId] = inputNode;

    AudioFormat outputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_U8, SAMPLE_RATE_48000};
    auto outputNode = std::make_shared<AudioOutputNode>(outputFormat);
    outputNode->Init();
    pipeline->outputNode_ = outputNode;
    outputNode->Connect(inputNode);
    pipeline->connections_[inputNodeId] = 0;
    pipeline->reverseConnections_[0] = {inputNodeId};

    std::shared_ptr<SuiteInputNodeRequestDataCallBackFinishedTestImpl> requestDataCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackFinishedTestImpl>();
    pipeline->SetRequestDataCallback(inputNodeId, requestDataCallback);
    pipeline->pipelineState_ = PIPELINE_RUNNING;

    uint8_t audioData[10000] = {0};
    int32_t writeLen = 0;
    bool finished = false;

    int32_t result = pipeline->RenderFrame(audioData, sizeof(audioData), &writeLen, &finished);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(finished, true);

    result = pipeline->RenderFrame(audioData, sizeof(audioData), &writeLen, &finished);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_004, TestSize.Level0)
{
    AudioSuitePipelineImpl::InitGlobal();
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 23;
    uint32_t destNodeId = 24;

    // Test src node is OUTPUT type — not supported
    audioSuitePipeline.nodeMap_[23] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    audioSuitePipeline.nodeMap_[24] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    // Test dest node is INPUT type — not supported
    srcNodeId = 25;
    destNodeId = 26;
    audioSuitePipeline.nodeMap_[25] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    audioSuitePipeline.nodeMap_[26] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    // Test src node is AUDIO_SEPARATION type and dest is not OUTPUT type — not supported
    srcNodeId = 27;
    destNodeId = 28;
    audioSuitePipeline.nodeMap_[27] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_SEPARATION);
    audioSuitePipeline.nodeMap_[28] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    AudioSuitePipelineImpl::DeInitGlobal();
}
} //namespace
