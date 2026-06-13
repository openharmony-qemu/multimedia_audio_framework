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
#include "audio_suite_engine.h"
#include "audio_suite_manager_private.h"
#include "audio_suite_manager_callback.h"
#include "audio_suite_pipeline.h"
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
static constexpr uint32_t TEST_PIPELINE_ID = 100;
// Invalid node ID for testing non-existent nodes
static constexpr uint32_t TEST_INVALID_NODE_ID = 999;

// Helper function to create a simple pipeline with space render node
static std::shared_ptr<AudioSuitePipeline> CreateSpaceRenderPipeline(AudioSuiteEngine &engine)
{
    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();

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

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;
    return pipeline;
}

// Engine creation: The engine already exists.
HWTEST_F(AudioSuiteEngineManagerUnitTest, duplicateInitTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    int32_t result = manager.Init();
    EXPECT_EQ(result, SUCCESS);

    result = manager.Init();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, duplicateDeInitTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    int32_t result = manager.Init();
    EXPECT_EQ(result, SUCCESS);

    result = manager.DeInit();
    EXPECT_EQ(result, SUCCESS);

    result = manager.DeInit();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

// Pipeline creation: Engine does not exist.
HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineWithoutEngineTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t result = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST);

    result = manager.Init();
    EXPECT_EQ(result, SUCCESS);

    manager.DeInit();
    result = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST);
}

// Pipeline creation: The number of pipelines exceeds the limit.
HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineExceedLimitTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    engineManger.pipelineMap_.clear();

    for (uint32_t num = 0; num < engineManger.engineCfg_.maxPipelineNum_ - 1; num++) {
        callback.isFinishCreatePipeline_ = false;
        callback.engineCreateResult_ = 0;
        int32_t result = engineManger.CreatePipeline(PIPELINE_EDIT_MODE);
        EXPECT_EQ(result, SUCCESS);
    }
    callback.isFinishCreatePipeline_ = false;
    callback.engineCreateResult_ = 0;
    int32_t result = engineManger.CreatePipeline(PIPELINE_REALTIME_MODE);
    EXPECT_EQ(result, SUCCESS);

    callback.isFinishCreatePipeline_ = false;
    callback.engineCreateResult_ = 0;
    result = engineManger.CreatePipeline(PIPELINE_EDIT_MODE);
    EXPECT_EQ(result, SUCCESS);
    sleep(1);
    EXPECT_EQ(callback.isFinishCreatePipeline_, true);
    EXPECT_EQ(callback.engineCreateResult_, ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS);

    callback.isFinishCreatePipeline_ = false;
    callback.engineCreateResult_ = 0;
    result = engineManger.CreatePipeline(PIPELINE_REALTIME_MODE);
    EXPECT_EQ(result, SUCCESS);
    sleep(1);
    EXPECT_EQ(callback.isFinishCreatePipeline_, true);
    EXPECT_EQ(callback.engineCreateResult_, ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS);

    engineManger.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createNodeExceedLimitTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_INPUT;

    for (uint32_t num = 0; num < audioSuitePipeline.pipelineCfg_.maxInputNodeNum_; num++) {
        int32_t result = audioSuitePipeline.CreateNodeCheckParme(builder);
        EXPECT_EQ(result, SUCCESS);
        audioSuitePipeline.nodeCounts_[builder.nodeType]++;
    }

    int32_t result = audioSuitePipeline.CreateNodeCheckParme(builder);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS);
}

// Node parameter setting: The user-defined parameter is invalid or illegal.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsInvalidParamTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    auto eqNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_EQUALIZER);
    eqNode->audioNodeInfo_.nodeId = TEST_EQUALIZER_NODE_ID;
    eqNode->SetAudioNodeFormat(format);

    auto outputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    outputNode->audioNodeInfo_.nodeId = TEST_OUTPUT_NODE_ID;
    outputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;
    pipeline->nodeMap_[TEST_EQUALIZER_NODE_ID] = eqNode;
    pipeline->nodeMap_[TEST_OUTPUT_NODE_ID] = outputNode;
    pipeline->connections_[TEST_INPUT_NODE_ID] = TEST_EQUALIZER_NODE_ID;
    pipeline->connections_[TEST_EQUALIZER_NODE_ID] = TEST_OUTPUT_NODE_ID;
    pipeline->reverseConnections_[TEST_EQUALIZER_NODE_ID] = {TEST_INPUT_NODE_ID};
    pipeline->reverseConnections_[TEST_OUTPUT_NODE_ID] = {TEST_EQUALIZER_NODE_ID};

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;

    AudioSuiteManager manager;
    manager.Init();

    AudioEqualizerFrequencyBandGains gains;
    gains.gains[0] = 11;
    gains.gains[1] = -11;
    for (int i = 2; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = 0;
    }

    int32_t ret = manager.SetEqualizerFrequencyBandGains(TEST_EQUALIZER_NODE_ID, gains);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    manager.DeInit();
    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// The node parameter setting node does not exist.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsNodeNotExistTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "0:0:0:0:0:0:0:0:0:0:";

    // Test the node ID 999 that does not exist.
    int32_t ret = engine.SetOptions(TEST_INVALID_NODE_ID, name, value);
    EXPECT_EQ(ret, SUCCESS);

    sleep(1);
    EXPECT_EQ(callback.setOptionsResult_, ERR_AUDIO_SUITE_NODE_NOT_EXIST);
    engine.DeInit();
}

// The node type not support AudioEqualizerFrequencyBandGains.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEqualizerFrequencyBandGainsNodeTypeNotSupportTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();

    // Register the callback to the engine so that TriggerCallback can be correctly triggered.
    auto engineSharedPtr = std::shared_ptr<AudioSuiteEngine>(&engine, [](AudioSuiteEngine *) {});
    pipeline->RegisterSendMsgCallback(engineSharedPtr);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;
    engine.nodeMap_[TEST_INPUT_NODE_ID] = TEST_PIPELINE_ID;
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "0:0:0:0:0:0:0:0:0:0:";

    // Test the INPUT node type does not support AudioEqualizerFrequencyBandGains.
    int32_t ret = engine.SetOptions(TEST_INPUT_NODE_ID, name, value);
    EXPECT_EQ(ret, SUCCESS);

    sleep(1);
    EXPECT_EQ(callback.setOptionsResult_, ERROR);

    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetTempoAndPitch with invalid speed and pitch range.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetTempoAndPitchInvalidParamTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();

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

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;

    AudioSuiteManager manager;
    manager.Init();

    // Test speed out of range (speed must be in 0.5~10.0)
    int32_t ret = manager.SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 0.4f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = manager.SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 10.1f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test pitch out of range (pitch must be in 0.1~5.0)
    ret = manager.SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 1.0f, 0.09f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = manager.SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 1.0f, 5.1f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test both speed and pitch out of range
    ret = manager.SetTempoAndPitch(TEST_TEMPOPITCH_NODE_ID, 0.4f, 0.09f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    manager.DeInit();
    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetPureVoiceChangeOption with invalid pitch range.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetPureVoiceChangeOptionInvalidParamTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Test pitch out of range (pitch must be in 0.3f~3.0f or 0.0f)
    AudioPureVoiceChangeOption option;
    option.optionGender = PURE_VOICE_CHANGE_FEMALE;
    option.optionType = PURE_VOICE_CHANGE_TYPE_SEASONED;
    option.pitch = 0.29f;
    int32_t ret = manager.SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    option.pitch = 3.01f;
    ret = manager.SetPureVoiceChangeOption(2, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test valid pitch values (will fail with node not exist error, but param check should pass)
    option.pitch = 0.3f;
    ret = manager.SetPureVoiceChangeOption(2, option);
    EXPECT_NE(ret, ERR_INVALID_PARAM);

    option.pitch = 3.0f;
    ret = manager.SetPureVoiceChangeOption(2, option);
    EXPECT_NE(ret, SUCCESS);

    option.pitch = 0.0f;
    ret = manager.SetPureVoiceChangeOption(2, option);
    EXPECT_NE(ret, ERR_INVALID_PARAM);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderExtensionParamsInvalidParamTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Test extRadius out of range (must be in 1.0f~5.0f)
    AudioSpaceRenderExtensionParams extension;
    extension.extRadius = 0.9f;
    extension.extAngle = 180;
    int32_t ret = manager.SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extRadius = 5.1f;
    ret = manager.SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // Test extAngle out of range (must be in [1, 359])
    extension.extRadius = 3.0f;
    extension.extAngle = 0;
    ret = manager.SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extAngle = 361;
    ret = manager.SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    extension.extRadius = 5.0f;
    extension.extAngle = 360;
    ret = manager.SetSpaceRenderExtensionParams(2, extension);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderExtensionParamsParamTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, SUCCESS);
    uint32_t inputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    ret = manager.CreateNode(pipelineId, inputBuilder, inputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t spaceRenderNodeId = INVALID_NODE_ID;
    AudioNodeBuilder spaceRenderBuilder;
    spaceRenderBuilder.nodeType = NODE_TYPE_SPACE_RENDER;
    ret = manager.CreateNode(pipelineId, spaceRenderBuilder, spaceRenderNodeId);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t outputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder outputBuilder;
    outputBuilder.nodeType = NODE_TYPE_OUTPUT;
    ret = manager.CreateNode(pipelineId, outputBuilder, outputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = manager.ConnectNodes(inputNodeId, spaceRenderNodeId);
    EXPECT_EQ(ret, SUCCESS);
    ret = manager.ConnectNodes(spaceRenderNodeId, outputNodeId);
    EXPECT_EQ(ret, SUCCESS);

    AudioSpaceRenderExtensionParams extension;
    extension.extRadius = 1.0f;
    extension.extAngle = 1;
    ret = manager.SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    extension.extRadius = 5.0f;
    extension.extAngle = 359;
    ret = manager.SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    extension.extRadius = 3.0f;
    extension.extAngle = 180;
    ret = manager.SetSpaceRenderExtensionParams(spaceRenderNodeId, extension);
    EXPECT_EQ(ret, SUCCESS);

    manager.DestroyPipeline(pipelineId);
    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetEnvironmentTypeInvalidEnumValueTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Create pipeline
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, SUCCESS);

    // Create environment effect node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    ret = manager.CreateNode(pipelineId, builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = manager.SetEnvironmentType(nodeId, static_cast<EnvironmentType>(10));
    EXPECT_EQ(ret, ERROR);

    // Destroy node and pipeline
    ret = manager.DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = manager.DestroyPipeline(pipelineId);
    EXPECT_EQ(ret, SUCCESS);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetVoiceBeautifierTypeInvalidEnumValueTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Create pipeline
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, SUCCESS);

    // Create voice beautifier node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_VOICE_BEAUTIFIER;
    ret = manager.CreateNode(pipelineId, builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = manager.SetVoiceBeautifierType(nodeId, static_cast<VoiceBeautifierType>(10));
    EXPECT_EQ(ret, ERROR);

    // Destroy node and pipeline
    ret = manager.DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = manager.DestroyPipeline(pipelineId);
    EXPECT_EQ(ret, SUCCESS);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetGeneralVoiceChangeTypeInvalidEnumValueTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Create pipeline
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, SUCCESS);

    // Create general voice change node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_GENERAL_VOICE_CHANGE;
    ret = manager.CreateNode(pipelineId, builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (20) to test deep layer fault interception
    ret = manager.SetGeneralVoiceChangeType(nodeId, static_cast<AudioGeneralVoiceChangeType>(20));
    EXPECT_EQ(ret, ERROR);

    // Destroy node and pipeline
    ret = manager.DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = manager.DestroyPipeline(pipelineId);
    EXPECT_EQ(ret, SUCCESS);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSoundFieldTypeInvalidEnumValueTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    // Create pipeline
    uint32_t pipelineId = INVALID_PIPELINE_ID;
    int32_t ret = manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);
    EXPECT_EQ(ret, SUCCESS);

    // Create sound field effect node
    uint32_t nodeId = INVALID_NODE_ID;
    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_SOUND_FIELD;
    ret = manager.CreateNode(pipelineId, builder, nodeId);
    EXPECT_EQ(ret, SUCCESS);

    // Test invalid enum value (10) to test deep layer fault interception
    ret = manager.SetSoundFieldType(nodeId, static_cast<SoundFieldType>(10));
    EXPECT_EQ(ret, ERROR);

    // Destroy node and pipeline
    ret = manager.DestroyNode(nodeId);
    EXPECT_EQ(ret, SUCCESS);

    ret = manager.DestroyPipeline(pipelineId);
    EXPECT_EQ(ret, SUCCESS);

    manager.DeInit();
}

// The current status of the node connection process pipeline is invalid.
HWTEST_F(AudioSuiteEngineManagerUnitTest, ConnectNodesRunningPipelineTest, TestSize.Level0)
{
    AudioSuitePipeline pipeline(PIPELINE_EDIT_MODE);
    pipeline.Init();
    EXPECT_EQ(pipeline.IsInit(), true);

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
}

// Test stop pipeline when pipeline is already stopped, expect ERR_ILLEGAL_STATE error.
HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineStopWhenAlreadyStoppedTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();
    EXPECT_EQ(pipeline->IsInit(), true);

    // Register the callback to the engine so that TriggerCallback can be correctly triggered.
    auto engineSharedPtr = std::shared_ptr<AudioSuiteEngine>(&engine, [](AudioSuiteEngine *) {});
    pipeline->RegisterSendMsgCallback(engineSharedPtr);

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

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;
    engine.nodeMap_[TEST_INPUT_NODE_ID] = TEST_PIPELINE_ID;
    engine.nodeMap_[TEST_OUTPUT_NODE_ID] = TEST_PIPELINE_ID;

    // Pipeline is already in STOPPED state by default
    pipeline->pipelineState_ = PIPELINE_STOPPED;

    // Try to stop pipeline when it's already stopped, should trigger ERR_ILLEGAL_STATE
    int32_t ret = engine.StopPipeline(100);
    EXPECT_EQ(ret, SUCCESS);

    sleep(1);
    EXPECT_EQ(callback.stopPipelineResult_, ERR_ILLEGAL_STATE);

    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetSpaceRenderPositionParams with invalid parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderPositionParamsInvalidParamTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    AudioSpaceRenderPositionParams position;
    position.x = 6.0f;
    position.y = 6.0f;
    position.z = 6.0f;

    int32_t ret = manager.SetSpaceRenderPositionParams(2, position);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    position.x = -6.0f;
    position.y = -6.0f;
    position.z = -6.0f;

    ret = manager.SetSpaceRenderPositionParams(2, position);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    manager.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisConnectNodesNodeTypeErrorTest, TestSize.Level0)
{
    AudioSuiteManager manager;
    manager.Init();

    uint32_t pipelineId = INVALID_PIPELINE_ID;
    manager.CreatePipeline(pipelineId, PIPELINE_EDIT_MODE);

    uint32_t inputNodeId = INVALID_NODE_ID;
    uint32_t mixerNodeId = INVALID_NODE_ID;
    uint32_t outputNodeId = INVALID_NODE_ID;

    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    manager.CreateNode(pipelineId, inputBuilder, inputNodeId);

    AudioNodeBuilder mixerBuilder;
    mixerBuilder.nodeType = NODE_TYPE_AUDIO_MIXER;
    manager.CreateNode(pipelineId, mixerBuilder, mixerNodeId);

    AudioNodeBuilder outputBuilder;
    outputBuilder.nodeType = NODE_TYPE_OUTPUT;
    manager.CreateNode(pipelineId, outputBuilder, outputNodeId);

    manager.ConnectNodes(inputNodeId, mixerNodeId);
    manager.ConnectNodes(mixerNodeId, outputNodeId);

    // Test 1: Try to disconnect OUTPUT node as source (should fail)
    int32_t result = manager.DisConnectNodes(outputNodeId, mixerNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    // Test 2: Try to disconnect INPUT node as destination (should fail)
    result = manager.DisConnectNodes(mixerNodeId, inputNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    manager.DeInit();
}

// Test SetOptions when node does not exist, expect ERR_INVALID_PARAM error report.
HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetOptionsErrorReportTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();
    EXPECT_EQ(pipeline->IsInit(), true);

    // Register the callback to the engine so that TriggerCallback can be correctly triggered.
    auto engineSharedPtr = std::shared_ptr<AudioSuiteEngine>(&engine, [](AudioSuiteEngine *) {});
    pipeline->RegisterSendMsgCallback(engineSharedPtr);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = TEST_INPUT_NODE_ID;
    inputNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[TEST_INPUT_NODE_ID] = inputNode;

    engine.pipelineMap_[TEST_PIPELINE_ID] = pipeline;
    engine.nodeMap_[TEST_INPUT_NODE_ID] = TEST_PIPELINE_ID;
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "0:0:0:0:0:0:0:0:0:0:";

    // Test SetOptions with node ID 999 that does not exist, should trigger ERR_AUDIO_SUITE_NODE_NOT_EXIST
    int32_t ret = engine.SetOptions(TEST_INVALID_NODE_ID, name, value);
    EXPECT_EQ(ret, SUCCESS);

    sleep(1);
    EXPECT_EQ(callback.setOptionsResult_, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetSpaceRenderRotationParams with invalid x, y, z parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidXYZTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    CreateSpaceRenderPipeline(engine);

    AudioSuiteManager manager;
    EXPECT_EQ(manager.Init(), 0);

    AudioSpaceRenderRotationParams rotation;
    // Test x, y, z upper bound
    rotation.x = 6.0f;
    rotation.y = 6.0f;
    rotation.z = 6.0f;
    rotation.surroundTime = 5.0f;
    rotation.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;
    EXPECT_EQ(manager.SetSpaceRenderRotationParams(2, rotation), ERR_INVALID_PARAM);

    // Test x, y, z lower bound
    rotation.x = -6.0f;
    rotation.y = -6.0f;
    rotation.z = -6.0f;
    EXPECT_EQ(manager.SetSpaceRenderRotationParams(2, rotation), ERR_INVALID_PARAM);

    manager.DeInit();
    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetSpaceRenderRotationParams with invalid surroundTime parameters.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidSurroundTimeTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    CreateSpaceRenderPipeline(engine);

    AudioSuiteManager manager;
    EXPECT_EQ(manager.Init(), 0);

    AudioSpaceRenderRotationParams rotation;
    rotation.x = 1.0f;
    rotation.y = 2.0f;
    rotation.z = 3.0f;
    rotation.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;

    // Test surroundTime lower bound
    rotation.surroundTime = 1.0f;
    EXPECT_EQ(manager.SetSpaceRenderRotationParams(2, rotation), ERR_INVALID_PARAM);

    // Test surroundTime upper bound
    rotation.surroundTime = 41.0f;
    EXPECT_EQ(manager.SetSpaceRenderRotationParams(2, rotation), ERR_INVALID_PARAM);

    manager.DeInit();
    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

// Test SetSpaceRenderRotationParams with invalid surroundDirection parameter.
HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSpaceRenderRotationParamsInvalidSurroundDirectionTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engine(callback);
    engine.Init();
    EXPECT_EQ(engine.IsInit(), true);

    CreateSpaceRenderPipeline(engine);

    AudioSuiteManager manager;
    EXPECT_EQ(manager.Init(), 0);

    AudioSpaceRenderRotationParams rotation;
    rotation.x = 1.0f;
    rotation.y = 2.0f;
    rotation.z = 3.0f;
    rotation.surroundTime = 5.0f;
    rotation.surroundDirection = static_cast<AudioSurroundDirection>(2);
    EXPECT_EQ(manager.SetSpaceRenderRotationParams(2, rotation), ERR_INVALID_PARAM);

    manager.DeInit();
    engine.pipelineMap_.erase(TEST_PIPELINE_ID);
    engine.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, startPipelineNoOutputNodeTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);

    std::shared_ptr<AudioSuitePipeline> pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();
    EXPECT_EQ(pipeline->IsInit(), true);
    engineManger.pipelineMap_[1] = pipeline;

    int32_t result = engineManger.StartPipeline(1);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest_002, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    auto engineMangerPtr = std::make_shared<AudioSuiteEngine>(callback);
    engineMangerPtr->Init();
    EXPECT_EQ(engineMangerPtr->IsInit(), true);

    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();
    uint32_t pipelineId = pipeline->GetPipelineId();
    pipeline->RegisterSendMsgCallback(engineMangerPtr->weak_from_this());
    engineMangerPtr->pipelineMap_[pipelineId] = pipeline;

    uint8_t audioData[10] = {0};
    int32_t writeLen = 0;
    bool finished = true;
    int32_t result = engineMangerPtr->RenderFrame(pipelineId, audioData, sizeof(audioData), &writeLen, &finished);

    EXPECT_EQ(result, SUCCESS);

    sleep(1);

    EXPECT_EQ(callback.isFinishRenderFrameMap_[pipelineId], true);
    EXPECT_EQ(callback.renderFrameResultMap_[pipelineId], ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest_003, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    auto engineMangerPtr = std::make_shared<AudioSuiteEngine>(callback);
    engineMangerPtr->Init();
    auto pipeline = std::make_shared<AudioSuitePipeline>(AudioSuite::PIPELINE_EDIT_MODE);
    pipeline->Init();
    uint32_t pipelineId = pipeline->GetPipelineId();
    pipeline->RegisterSendMsgCallback(engineMangerPtr->weak_from_this());
    engineMangerPtr->pipelineMap_[pipelineId] = pipeline;

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

    int32_t result = engineMangerPtr->RenderFrame(pipelineId, audioData, sizeof(audioData), &writeLen, &finished);
    EXPECT_EQ(result, SUCCESS);
    sleep(1);
    EXPECT_EQ(callback.isFinishRenderFrameMap_[pipelineId], true);
    EXPECT_EQ(callback.renderFrameResultMap_[pipelineId], SUCCESS);
    EXPECT_EQ(finished, true);

    callback.isFinishRenderFrameMap_[pipelineId] = false;
    callback.renderFrameResultMap_[pipelineId] = SUCCESS;

    result = engineMangerPtr->RenderFrame(pipelineId, audioData, sizeof(audioData), &writeLen, &finished);
    EXPECT_EQ(result, SUCCESS);
    sleep(1);
    EXPECT_EQ(callback.isFinishRenderFrameMap_[pipelineId], true);
    EXPECT_EQ(callback.renderFrameResultMap_[pipelineId], ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_004, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    uint32_t srcNodeId = 23;
    uint32_t destNodeId = 24;

    // Test src node is OUTPUT type
    audioSuitePipeline.nodeMap_[23] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    audioSuitePipeline.nodeMap_[24] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    // Test dest node is INPUT type
    srcNodeId = 25;
    destNodeId = 26;
    audioSuitePipeline.nodeMap_[25] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    audioSuitePipeline.nodeMap_[26] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    // Test src node is AUDIO_SEPARATION type and dest is not OUTPUT type
    srcNodeId = 27;
    destNodeId = 28;
    audioSuitePipeline.nodeMap_[27] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_SEPARATION);
    audioSuitePipeline.nodeMap_[28] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}
} //namespace