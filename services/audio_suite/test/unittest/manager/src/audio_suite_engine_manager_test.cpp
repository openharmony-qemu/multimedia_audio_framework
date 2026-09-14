/*
 * Copyright (c) 2025~2026 Huawei Device Co., Ltd.
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
#include "audio_suite_system_music_master_node.h"
#include "audio_suite_system_dialogue_enhance_node.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::shared_ptr<AudioSuitePipeline> pipeline1;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline1), SUCCESS);
    EXPECT_NE(pipeline1, nullptr);
    std::shared_ptr<AudioSuitePipeline> pipeline2;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_REALTIME_MODE, pipeline2), SUCCESS);
    EXPECT_NE(pipeline2, nullptr);
    std::shared_ptr<AudioSuitePipeline> pipeline3;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline3), SUCCESS);
    EXPECT_NE(pipeline3, nullptr);
    std::shared_ptr<AudioSuitePipeline> pipeline4;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_REALTIME_MODE, pipeline4), SUCCESS);
    EXPECT_NE(pipeline4, nullptr);

    std::vector<std::shared_ptr<AudioSuitePipeline>> pipelines = {pipeline1, pipeline2, pipeline3, pipeline4};
    while (pipelines.size() < 10) {
        std::shared_ptr<AudioSuitePipeline> p;
        EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, p), SUCCESS);
        EXPECT_NE(p, nullptr);
        pipelines.push_back(p);
    }

    std::shared_ptr<AudioSuitePipeline> exceed;
    EXPECT_NE(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, exceed), SUCCESS);

    pipelines.clear();
    AudioSuitePipelineImpl::DeInitGlobal();

    std::shared_ptr<AudioSuitePipeline> afterDeInit;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, afterDeInit), SUCCESS);
    EXPECT_NE(afterDeInit, nullptr);
    afterDeInit.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, destroyPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);
    pipeline.reset();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, startPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    int32_t result = pipeline->Start();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, stopPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    int32_t result = pipeline->Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioSuitePipelineState state;
    int32_t result = pipeline->GetPipelineState(state);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_INPUT;
    uint32_t nodeId = INVALID_NODE_ID;
    int32_t result = pipeline->CreateNode(builder, nodeId);
    EXPECT_EQ(result, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, destroyNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    int32_t result = pipeline->DestroyNode(999);
    EXPECT_NE(result, SUCCESS);

    AudioNodeBuilder builder;
    builder.nodeType = NODE_TYPE_INPUT;
    uint32_t nodeId = INVALID_NODE_ID;
    pipeline->CreateNode(builder, nodeId);
    result = pipeline->DestroyNode(nodeId);
    EXPECT_EQ(result, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, bypassEffectNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    int32_t result = pipeline->BypassEffectNode(999, true);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getNodeEnableStatusTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    bool bypass;
    int32_t result = pipeline->GetNodeBypassStatus(999, bypass);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, setAudioFormatTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat audioFormat;
    int32_t result = pipeline->SetAudioFormat(999, audioFormat);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetRequestDataCallbackTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    std::shared_ptr<InputNodeRequestDataCallBack> suiteCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();
    int32_t result = pipeline->SetRequestDataCallback(999, suiteCallback);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, connectNodesTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 4;
    uint32_t destNodeId = 5;
    int32_t result = pipeline->ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, connectNodesTest_002, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    std::shared_ptr<AudioSuitePipeline> pipeline;
    EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, pipeline), SUCCESS);
    EXPECT_NE(pipeline, nullptr);

    uint32_t inputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder inputBuilder;
    inputBuilder.nodeType = NODE_TYPE_INPUT;
    pipeline->CreateNode(inputBuilder, inputNodeId);

    uint32_t outputNodeId = INVALID_NODE_ID;
    AudioNodeBuilder outputBuilder;
    outputBuilder.nodeType = NODE_TYPE_OUTPUT;
    pipeline->CreateNode(outputBuilder, outputNodeId);

    int32_t result = pipeline->ConnectNodes(inputNodeId, outputNodeId);
    EXPECT_EQ(result, SUCCESS);

    pipeline.reset();
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, disConnectNodesTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 4;
    uint32_t destNodeId = 5;
    int32_t result = pipeline->DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    int32_t result = pipeline->RenderFrame(nullptr, 1, nullptr, nullptr);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, multiRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    uint8_t** audioDataArray = nullptr;
    int32_t arraySize = 0;
    int32_t requestFrameSize = 0;
    int32_t responseSize = 0;
    bool finishedFlag = false;
    int32_t result = pipeline->MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, &responseSize, &finishedFlag);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, setOptionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    std::string name = "abc";
    std::string value = "def";
    int32_t result = pipeline->SetOptions(1, name, value);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getOptionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    std::string name = "abc";
    std::string value = "def";
    int32_t result = pipeline->GetOptions(1, name, value);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);
    audioSuitePipeline.DeInit();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineStopTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    int32_t result = audioSuitePipeline.Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.nodeMap_[2] = nullptr;
    result = audioSuitePipeline.Stop();
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCreateNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioNodeBuilder audioNodeBuilder;
    audioNodeBuilder.nodeType = NODE_TYPE_INPUT;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_EDIT_MODE;

    uint32_t nodeId = INVALID_NODE_ID;
    audioSuitePipeline.nodeCounts_[audioNodeBuilder.nodeType] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) - 2;
    int32_t result = audioSuitePipeline.CreateNode(audioNodeBuilder, nodeId);

    audioSuitePipeline.nodeCounts_[audioNodeBuilder.nodeType] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) + 2;
    result = audioSuitePipeline.CreateNode(audioNodeBuilder, nodeId);

    audioNodeBuilder.nodeType = NODE_TYPE_SOUND_FIELD;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_REALTIME_MODE;
    audioSuitePipeline.nodeCounts_[audioNodeBuilder.nodeType] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) - 2;
    result = audioSuitePipeline.CreateNode(audioNodeBuilder, nodeId);
    EXPECT_EQ(result, SUCCESS);

    audioNodeBuilder.nodeType = NODE_TYPE_EMPTY;
    result = audioSuitePipeline.CreateNode(audioNodeBuilder, nodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCreateNodeCheckParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioNodeBuilder audioNodeBuilder;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_REALTIME_MODE;

    audioNodeBuilder.nodeType = NODE_TYPE_INPUT;
    int32_t result = audioSuitePipeline.CreateNodeCheckParam(audioNodeBuilder);

    audioNodeBuilder.nodeType = NODE_TYPE_OUTPUT;
    result = audioSuitePipeline.CreateNodeCheckParam(audioNodeBuilder);

    audioNodeBuilder.nodeType = NODE_TYPE_EQUALIZER;
    result = audioSuitePipeline.CreateNodeCheckParam(audioNodeBuilder);

    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_EDIT_MODE;
    result = audioSuitePipeline.CreateNodeCheckParam(audioNodeBuilder);

    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetMaxNodeNumsForTypeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioNodeType audioNodeType = NODE_TYPE_INPUT;
    int32_t result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, AudioSuitePipelineCfg{}.maxInputNodeNum_);

    audioNodeType = NODE_TYPE_OUTPUT;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, AudioSuitePipelineCfg{}.maxOutputNodeNum_);

    audioNodeType = NODE_TYPE_AUDIO_MIXER;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, AudioSuitePipelineCfg{}.maxMixNodeNum_);

    audioNodeType = NODE_TYPE_NOISE_REDUCTION;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, AudioSuitePipelineCfg{}.maxEffectNodeNum_);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDestroyNodeForRunTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat audioFormat;
    uint32_t nodeId = 1;
    std::shared_ptr<AudioNode> node = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = nodeId;
    int32_t result = audioSuitePipeline.DestroyNodeForRun(1, node);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.outputNode_ = nullptr;
    result = audioSuitePipeline.DestroyNodeForRun(1, node);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCreateNodeForTypeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioNodeBuilder audioNodeBuilder;

    audioNodeBuilder.nodeType = NODE_TYPE_INPUT;
    std::shared_ptr<AudioNode> inputNode = audioSuitePipeline.CreateNodeForType(audioNodeBuilder);
    EXPECT_TRUE(inputNode != nullptr);

    audioNodeBuilder.nodeType = NODE_TYPE_OUTPUT;
    std::shared_ptr<AudioNode> outputNode = audioSuitePipeline.CreateNodeForType(audioNodeBuilder);
    EXPECT_TRUE(outputNode != nullptr);

    audioNodeBuilder.nodeType = NODE_TYPE_SOUND_FIELD;
    std::shared_ptr<AudioNode> sfNode = audioSuitePipeline.CreateNodeForType(audioNodeBuilder);
    EXPECT_TRUE(sfNode != nullptr);

    audioNodeBuilder.nodeType = NODE_TYPE_NOISE_REDUCTION;
    std::shared_ptr<AudioNode> effNode = audioSuitePipeline.CreateNodeForType(audioNodeBuilder);
    EXPECT_TRUE(effNode != nullptr);

    audioNodeBuilder.nodeType = NODE_TYPE_VOICE_BEAUTIFIER;
    std::shared_ptr<AudioNode> vbNode = audioSuitePipeline.CreateNodeForType(audioNodeBuilder);
    EXPECT_TRUE(vbNode != nullptr);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineEnableNodeNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    int32_t result = audioSuitePipeline.BypassEffectNode(2, true);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.BypassEffectNode(1, true);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.BypassEffectNode(3, true);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.nodeMap_[4] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.BypassEffectNode(4, true);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetNodeBypassStatusTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    bool bypass;
    int32_t result = audioSuitePipeline.GetNodeBypassStatus(2, bypass);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.GetNodeBypassStatus(1, bypass);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.GetNodeBypassStatus(3, bypass);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetAudioFormatTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat audioFormat;

    int32_t result = audioSuitePipeline.SetAudioFormat(2, audioFormat);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetAudioFormat(1, audioFormat);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.SetAudioFormat(3, audioFormat);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetRequestDataCallbackTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);
    std::shared_ptr<InputNodeRequestDataCallBack> suitCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.SetRequestDataCallback(2, suitCallback);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.SetRequestDataCallback(4, suitCallback);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetRequestDataCallback(3, suitCallback);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);

    audioSuitePipeline.nodeMap_[5] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.SetRequestDataCallback(5, suitCallback);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    srcNodeId = 1;
    destNodeId = 2;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    srcNodeId = 3;
    destNodeId = 4;
    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    srcNodeId = 5;
    destNodeId = 6;
    audioSuitePipeline.nodeMap_[6] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    srcNodeId = 7;
    destNodeId = 8;
    audioSuitePipeline.nodeMap_[7] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    audioSuitePipeline.nodeMap_[8] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    srcNodeId = 9;
    destNodeId = 10;
    audioSuitePipeline.nodeMap_[9] = nullptr;
    audioSuitePipeline.nodeMap_[10] = nullptr;
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    srcNodeId = 11;
    destNodeId = 12;
    audioSuitePipeline.nodeMap_[12] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    srcNodeId = 13;
    destNodeId = 14;
    audioSuitePipeline.nodeMap_[13] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_SEPARATION);
    audioSuitePipeline.nodeMap_[14] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_002, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);
    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    srcNodeId = 15;
    destNodeId = 16;
    audioSuitePipeline.connections_[srcNodeId] = destNodeId;
    audioSuitePipeline.nodeMap_[15] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[16] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 17;
    destNodeId = 18;
    audioSuitePipeline.connections_[srcNodeId] = destNodeId + 1;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    audioSuitePipeline.nodeMap_[17] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[18] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 19;
    destNodeId = 20;
    audioSuitePipeline.connections_[srcNodeId] = destNodeId + 1;
    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.nodeMap_[19] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[20] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_003, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);
    uint32_t srcNodeId = 21;
    uint32_t destNodeId = 22;

    audioSuitePipeline.nodeMap_[21] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    audioSuitePipeline.nodeMap_[22] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);

    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    auto srcNode = audioSuitePipeline.nodeMap_[srcNodeId];
    auto destNode = audioSuitePipeline.nodeMap_[destNodeId];

    EXPECT_EQ(destNode->GetAudioNodeFormat().rate, srcNode->GetAudioNodeFormat().rate);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisconnectNodesTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    srcNodeId = 1;
    destNodeId = 2;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    srcNodeId = 3;
    destNodeId = 4;
    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    srcNodeId = 5;
    destNodeId = 6;
    audioSuitePipeline.nodeMap_[6] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    srcNodeId = 7;
    destNodeId = 8;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    audioSuitePipeline.nodeMap_[7] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[8] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    srcNodeId = 9;
    destNodeId = 10;
    audioSuitePipeline.nodeMap_[9] = nullptr;
    audioSuitePipeline.nodeMap_[10] = nullptr;
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    srcNodeId = 11;
    destNodeId = 12;
    audioSuitePipeline.nodeMap_[11] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[12] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    srcNodeId = 13;
    destNodeId = 14;
    audioSuitePipeline.nodeMap_[14] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisconnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_NODE_NOT_EXIST);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesForRunTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 1;
    uint32_t destNodeId = 2;
    std::shared_ptr<AudioNode> destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    std::shared_ptr<AudioNode> srcNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    destNode->audioNodeInfo_.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = nullptr;

    int32_t result = audioSuitePipeline.ConnectNodesForRun(srcNodeId, destNodeId, srcNode,  destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = destNodeId + 1;
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId, destNodeId + 1, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = srcNodeId + 1;
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId + 1, destNodeId + 4, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesForRunTest_002, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 1;
    uint32_t destNodeId = 2;
    std::shared_ptr<AudioNode> destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    std::shared_ptr<AudioNode> srcNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    destNode->audioNodeInfo_.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = nullptr;

    int32_t result = audioSuitePipeline.ConnectNodesForRun(srcNodeId, destNodeId, srcNode,  destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = destNodeId + 1;
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId, destNodeId + 1, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = srcNodeId + 1;
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId + 1, destNodeId + 4, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = srcNodeId;
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId + 1, destNodeId + 1, srcNode, destNode);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = destNodeId + 1;
    destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    result = audioSuitePipeline.ConnectNodesForRun(srcNodeId, destNodeId + 1, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisconnectNodesForRunTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 1;
    uint32_t destNodeId = 2;
    std::shared_ptr<AudioNode> destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    std::shared_ptr<AudioNode> srcNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    destNode->audioNodeInfo_.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = nullptr;

    int32_t result = audioSuitePipeline.DisconnectNodesForRun(srcNodeId, destNodeId, srcNode,  destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = destNodeId;
    result = audioSuitePipeline.DisconnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    result = audioSuitePipeline.DisconnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.reverseConnections_[destNodeId] = {};
    result = audioSuitePipeline.DisconnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint8_t *audioData = nullptr;
    int32_t frameSize = 0;
    int32_t *writeLen = nullptr;
    bool *finishedFlag = nullptr;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.outputNode_ = nullptr;
    result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineMultiRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint8_t **audioDataArray = nullptr;
    int arraySize = 0;
    int32_t requestFrameSize = 0;
    int32_t *responseSize = nullptr;
    bool *finishedFlag = nullptr;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.outputNode_ = nullptr;
    result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetOptionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t nodeId = 1;
    std::string name = "abc";
    std::string value = "def";
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.SetOptions(nodeId, name, value);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.nodeMap_[nodeId + 1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetOptions(nodeId + 1, name, value);
    EXPECT_EQ(result, ERROR);

    result = audioSuitePipeline.SetOptions(nodeId + 2, name, value);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetOptionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t nodeId = 1;
    std::string name = "abc";
    std::string value = "def";
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.GetOptions(nodeId, name, value);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.nodeMap_[nodeId + 1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.GetOptions(nodeId + 1, name, value);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    result = audioSuitePipeline.GetOptions(nodeId + 2, name, value);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDestroyNodeTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 0;
    int32_t result = audioSuitePipeline.DestroyNode(srcNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.nodeMap_[1] = nullptr;
    result = audioSuitePipeline.DestroyNode(srcNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    result = audioSuitePipeline.DestroyNode(srcNodeId);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineRemovceForwardConnetTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    uint32_t srcNodeId = 1;
    uint32_t destNodeId = 2;
    uint32_t outputNodeId = 3;

    audioSuitePipeline.reverseConnections_[srcNodeId].push_back(destNodeId);
    audioSuitePipeline.reverseConnections_[srcNodeId].push_back(outputNodeId);
    audioSuitePipeline.nodeMap_[destNodeId] = nullptr;

    audioSuitePipeline.RemovceForwardConnet(srcNodeId, nullptr);
    EXPECT_EQ(audioSuitePipeline.reverseConnections_[srcNodeId].size(), 2);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCheckPipelineNodeTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    bool result = audioSuitePipeline.CheckPipelineNode(0);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCheckRenderFrameTimeTest_001, TestSize.Level0)
{
    AudioSuitePipelineImpl pl(PIPELINE_REALTIME_MODE, 0);

    int32_t frameDurationMS = 20;  // 20 ms frame duraion
    // DurationBase is for compare use, frameduration * rtfBase(1.0 for pipeline)
    uint64_t processDurationBase = frameDurationMS * MILLISECONDS_TO_MICROSECONDS;
    uint64_t testDurationNormal = 1;  // 1 microsecond
    uint64_t testDurationBase = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_BASE];
    uint64_t testDuration110Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_110BASE];
    uint64_t testDuration120Base = processDurationBase * RTF_OVERTIME_THRESHOLDS[RtfOvertimeLevel::OVER_120BASE];
    uint64_t testDurationOver120Base = testDuration120Base + 1;
    // expected OvertimeCounters
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayEmpty = {0, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayBase = {1, 0, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArray110Base = {1, 1, 0};
    std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayOver120Base = {1, 1, 1};

    std::array<PipelineWorkMode, 2> workModeArray = {PIPELINE_REALTIME_MODE, PIPELINE_EDIT_MODE};
    for (PipelineWorkMode testWorkMode : workModeArray) {
        pl.pipelineWorkMode_ = testWorkMode;

        // rtf equal baseline
        pl.CheckRenderFrameTime(frameDurationMS, testDurationBase);
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayBase);
        pl.CheckRenderFrameOverTimeCount();
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayEmpty);

        // rtf equal 110% baseline
        pl.CheckRenderFrameTime(frameDurationMS, testDuration110Base);
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArray110Base);
        pl.CheckRenderFrameOverTimeCount();
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayEmpty);

        // rtf over 120% baseline
        pl.CheckRenderFrameTime(frameDurationMS, testDurationOver120Base);
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayOver120Base);
        pl.CheckRenderFrameOverTimeCount();
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayEmpty);

        // check multiple times
        pl.CheckRenderFrameTime(frameDurationMS, testDurationNormal);
        pl.CheckRenderFrameTime(frameDurationMS, testDurationNormal);
        pl.CheckRenderFrameTime(frameDurationMS, testDurationBase);
        pl.CheckRenderFrameTime(frameDurationMS, testDuration110Base);
        pl.CheckRenderFrameTime(frameDurationMS, testDuration120Base);
        pl.CheckRenderFrameTime(frameDurationMS, testDurationOver120Base);
        pl.CheckRenderFrameTime(0, 1);  // invalid data duration, ignore.
        EXPECT_EQ(pl.renderFrameTotalCount_, 6);
        std::array<int32_t, RTF_OVERTIME_LEVELS> expectedArrayMultiple = {4, 3, 2};
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayMultiple);
        pl.CheckRenderFrameOverTimeCount();
        EXPECT_EQ(pl.renderFrameOvertimeCounters_, expectedArrayEmpty);
        EXPECT_EQ(pl.renderFrameTotalCount_, 0);
    }
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, MixerNodeNotSetSampleRateWhenInputsDifferent, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat inputFormat1 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    AudioFormat inputFormat2 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_16000};
    AudioFormat mixerFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    AudioFormat outputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode1->audioNodeInfo_.nodeId = 2;
    inputNode1->SetAudioNodeFormat(inputFormat1);

    auto inputNode2 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode2->audioNodeInfo_.nodeId = 3;
    inputNode2->SetAudioNodeFormat(inputFormat2);

    auto mixerNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode->audioNodeInfo_.nodeId = 1;
    mixerNode->SetAudioNodeFormat(mixerFormat);

    auto outputNode = std::make_shared<AudioOutputNode>(outputFormat);
    outputNode->audioNodeInfo_.nodeId = 4;

    audioSuitePipeline.nodeMap_[2] = inputNode1;
    audioSuitePipeline.nodeMap_[3] = inputNode2;
    audioSuitePipeline.nodeMap_[1] = mixerNode;
    audioSuitePipeline.outputNode_ = outputNode;
    audioSuitePipeline.reverseConnections_[1] = {2, 3};
    audioSuitePipeline.connections_[2] = 1;
    audioSuitePipeline.connections_[3] = 1;
    audioSuitePipeline.reverseConnections_[4] = {1};
    audioSuitePipeline.connections_[1] = 4;

    std::shared_ptr<InputNodeRequestDataCallBack> suitCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();
    audioSuitePipeline.SetRequestDataCallback(2, suitCallback);
    audioSuitePipeline.SetRequestDataCallback(3, suitCallback);

    EXPECT_EQ(audioSuitePipeline.Start(), ERR_ILLEGAL_STATE);
    EXPECT_EQ(mixerNode->GetAudioNodeFormat().rate, SAMPLE_RATE_44100);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, MultipleMixerNodesEachSetFromInputs, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat inputFormat1 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    AudioFormat inputFormat2 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_16000};
    AudioFormat mixerFormat1 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    AudioFormat mixerFormat2 = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_8000};
    AudioFormat outputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode1->audioNodeInfo_.nodeId = 3;
    inputNode1->SetAudioNodeFormat(inputFormat1);

    auto inputNode2 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode2->audioNodeInfo_.nodeId = 4;
    inputNode2->SetAudioNodeFormat(inputFormat1);

    auto inputNode3 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode3->audioNodeInfo_.nodeId = 5;
    inputNode3->SetAudioNodeFormat(inputFormat2);

    auto mixerNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode1->audioNodeInfo_.nodeId = 1;
    mixerNode1->SetAudioNodeFormat(mixerFormat1);

    auto mixerNode2 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode2->audioNodeInfo_.nodeId = 2;
    mixerNode2->SetAudioNodeFormat(mixerFormat2);

    auto outputNode = std::make_shared<AudioOutputNode>(outputFormat);
    outputNode->audioNodeInfo_.nodeId = 6;

    audioSuitePipeline.nodeMap_[3] = inputNode1;
    audioSuitePipeline.nodeMap_[4] = inputNode2;
    audioSuitePipeline.nodeMap_[5] = inputNode3;
    audioSuitePipeline.nodeMap_[1] = mixerNode1;
    audioSuitePipeline.nodeMap_[2] = mixerNode2;
    audioSuitePipeline.outputNode_ = outputNode;
    audioSuitePipeline.reverseConnections_[1] = {3, 4};
    audioSuitePipeline.connections_[3] = 1;
    audioSuitePipeline.connections_[4] = 1;
    audioSuitePipeline.reverseConnections_[2] = {5};
    audioSuitePipeline.connections_[5] = 2;
    audioSuitePipeline.reverseConnections_[6] = {1, 2};
    audioSuitePipeline.connections_[1] = 6;
    audioSuitePipeline.connections_[2] = 6;

    std::shared_ptr<InputNodeRequestDataCallBack> suitCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();
    audioSuitePipeline.SetRequestDataCallback(3, suitCallback);
    audioSuitePipeline.SetRequestDataCallback(4, suitCallback);
    audioSuitePipeline.SetRequestDataCallback(5, suitCallback);

    EXPECT_EQ(audioSuitePipeline.Start(), ERR_ILLEGAL_STATE);
    EXPECT_EQ(mixerNode1->GetAudioNodeFormat().rate, SAMPLE_RATE_48000);
    EXPECT_EQ(mixerNode2->GetAudioNodeFormat().rate, SAMPLE_RATE_16000);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, MixerNodeHandleNullInputNodes, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    AudioFormat inputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    AudioFormat mixerFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_44100};
    AudioFormat outputFormat = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode1->audioNodeInfo_.nodeId = 2;
    inputNode1->SetAudioNodeFormat(inputFormat);

    auto mixerNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode->audioNodeInfo_.nodeId = 1;
    mixerNode->SetAudioNodeFormat(mixerFormat);

    auto outputNode = std::make_shared<AudioOutputNode>(outputFormat);
    outputNode->audioNodeInfo_.nodeId = 4;

    audioSuitePipeline.nodeMap_[2] = inputNode1;
    audioSuitePipeline.nodeMap_[1] = mixerNode;
    audioSuitePipeline.outputNode_ = outputNode;
    audioSuitePipeline.reverseConnections_[1] = {2, 3};
    audioSuitePipeline.connections_[2] = 1;
    audioSuitePipeline.reverseConnections_[4] = {1};
    audioSuitePipeline.connections_[1] = 4;

    std::shared_ptr<InputNodeRequestDataCallBack> suitCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();
    audioSuitePipeline.SetRequestDataCallback(2, suitCallback);

    EXPECT_EQ(audioSuitePipeline.Start(), ERR_ILLEGAL_STATE);
    EXPECT_EQ(mixerNode->GetAudioNodeFormat().rate, SAMPLE_RATE_48000);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoAllPipelinesTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline1 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    auto pipeline2 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    auto pipeline3 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_REALTIME_MODE, 0);

    int32_t result = pipeline1->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);
    result = pipeline2->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);
    result = pipeline3->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoSpecificPipelineTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline1 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    auto pipeline2 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    int32_t result = pipeline1->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    result = pipeline2->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoToFileTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline1 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    auto pipeline2 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    std::string filePath = "/data/audiosuite/test_printinfo.txt";
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    EXPECT_GE(fd, 0);

    int32_t result = pipeline1->PrintInfo(fd);
    EXPECT_EQ(result, SUCCESS);
    result = pipeline2->PrintInfo(fd);
    EXPECT_EQ(result, SUCCESS);

    close(fd);

    std::ifstream inFile(filePath);
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    EXPECT_FALSE(content.empty());

    std::remove(filePath.c_str());
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoAppendToFileTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline1 = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    std::string filePath = "/data/audiosuite/test_append.txt";

    int fd1 = open(filePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    EXPECT_GE(fd1, 0);

    int32_t result = pipeline1->PrintInfo(fd1);
    EXPECT_EQ(result, SUCCESS);
    close(fd1);

    int fd2 = open(filePath.c_str(), O_WRONLY | O_APPEND, 0644);
    EXPECT_GE(fd2, 0);

    result = pipeline1->PrintInfo(fd2);
    EXPECT_EQ(result, SUCCESS);
    close(fd2);

    std::ifstream inFile(filePath);
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    EXPECT_FALSE(content.empty());

    std::remove(filePath.c_str());
    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoPipelineWithNodesTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = 1;
    inputNode->SetAudioNodeFormat(format);
    inputNode->audioNodeInfo_.finishedFlag = false;

    auto outputNode = std::make_shared<AudioOutputNode>(format);
    outputNode->audioNodeInfo_.nodeId = 2;

    auto eqNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_EQUALIZER);
    eqNode->audioNodeInfo_.nodeId = 3;
    eqNode->audioNodeInfo_.bypassStatus = false;

    pipeline->nodeMap_[1] = inputNode;
    pipeline->nodeMap_[2] = outputNode;
    pipeline->nodeMap_[3] = eqNode;

    int32_t result = pipeline->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoPipelineWithConnectionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode->audioNodeInfo_.nodeId = 1;
    inputNode->SetAudioNodeFormat(format);

    auto eqNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_EQUALIZER);
    eqNode->audioNodeInfo_.nodeId = 2;
    eqNode->audioNodeInfo_.bypassStatus = true;

    pipeline->nodeMap_[1] = inputNode;
    pipeline->nodeMap_[2] = eqNode;
    pipeline->connections_[1] = 2;
    pipeline->reverseConnections_[2] = {1};

    int32_t result = pipeline->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoPipelineWithMixerNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto mixerNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode->audioNodeInfo_.nodeId = 1;
    mixerNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[1] = mixerNode;

    int32_t result = pipeline->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoPipelineWithEffectNodesTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    auto envNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_ENVIRONMENT_EFFECT);
    envNode->audioNodeInfo_.nodeId = 1;
    envNode->audioNodeInfo_.bypassStatus = true;

    auto nrNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_NOISE_REDUCTION);
    nrNode->audioNodeInfo_.nodeId = 2;
    nrNode->audioNodeInfo_.bypassStatus = false;

    pipeline->nodeMap_[1] = envNode;
    pipeline->nodeMap_[2] = nrNode;

    int32_t result = pipeline->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createHoaNodeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl audioSuitePipeline(PIPELINE_EDIT_MODE, 0);

    std::shared_ptr<AudioNode> node = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_HOA_RENDER);
    audioSuitePipeline.nodeMap_[1] = node;
    int32_t result = audioSuitePipeline.DestroyNode(1);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, PrintInfoMultipleConnectionsTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};

    auto inputNode1 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode1->audioNodeInfo_.nodeId = 1;
    inputNode1->SetAudioNodeFormat(format);

    auto inputNode2 = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    inputNode2->audioNodeInfo_.nodeId = 2;
    inputNode2->SetAudioNodeFormat(format);

    auto mixerNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    mixerNode->audioNodeInfo_.nodeId = 3;
    mixerNode->SetAudioNodeFormat(format);

    pipeline->nodeMap_[1] = inputNode1;
    pipeline->nodeMap_[2] = inputNode2;
    pipeline->nodeMap_[3] = mixerNode;
    pipeline->connections_[1] = 3;
    pipeline->connections_[2] = 3;
    pipeline->reverseConnections_[3] = {1, 2};

    int32_t result = pipeline->PrintInfo(-1);
    EXPECT_EQ(result, SUCCESS);

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, MetaRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);

    AudioMetaFrame metaFrame {nullptr, 0, nullptr, 0};
    int32_t responseAudioSize = 0;
    int32_t responseMetaSize = 0;
    bool finishedFlag = false;
    int32_t ret = pipeline->MetaRenderFrame(metaFrame, &responseAudioSize, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    pipeline->DeInit();

    pipeline->pipelineState_ = PIPELINE_RUNNING;
    ret = pipeline->MetaRenderFrame(metaFrame, &responseAudioSize, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    pipeline->DeInit();

    AudioFormat format = {{CH_LAYOUT_STEREO, 2}, SAMPLE_F32LE, SAMPLE_RATE_48000};
    pipeline->outputNode_ = std::make_shared<AudioOutputNode>(format);
    ret = pipeline->MetaRenderFrame(metaFrame, &responseAudioSize, &responseMetaSize, &finishedFlag);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    pipeline->DeInit();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetSystemNodeParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    pipeline->pipelineState_ = PIPELINE_RUNNING;

    std::vector<uint8_t> param;
    int32_t ret = pipeline->SetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    pipeline->DeInit();
    pipeline->pipelineState_ = PIPELINE_STOPPED;
    ret = pipeline->SetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    pipeline->nodeMap_[0] = nullptr;
    ret = pipeline->SetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    std::shared_ptr<AudioNode> node = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->SetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    pipeline->DeInit();
    param.resize(1);
    ret = pipeline->SetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    node = std::make_shared<AudioSuiteSystemDialogueEnhanceNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->SetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERROR);
    pipeline->DeInit();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, GetSystemNodeParamTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    pipeline->pipelineState_ = PIPELINE_RUNNING;

    std::vector<uint8_t> param;
    int32_t ret = pipeline->GetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    pipeline->DeInit();
    pipeline->pipelineState_ = PIPELINE_STOPPED;
    ret = pipeline->GetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    pipeline->nodeMap_[0] = nullptr;
    ret = pipeline->GetSystemNodeParam(0, param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    std::shared_ptr<AudioNode> node = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->GetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERROR);

    pipeline->DeInit();
    param.resize(1);
    ret = pipeline->GetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    node = std::make_shared<AudioSuiteSystemDialogueEnhanceNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->GetSystemNodeParam(node->GetAudioNodeId(), param);
    EXPECT_EQ(ret, ERROR);
    pipeline->DeInit();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, GetSystemNodeInOutSizeTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    pipeline->pipelineState_ = PIPELINE_RUNNING;

    pipeline->DeInit();
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    int32_t ret = pipeline->GetSystemNodeInOutSize(0, inSize, outSize);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);

    pipeline->DeInit();
    pipeline->pipelineState_ = PIPELINE_STOPPED;
    ret = pipeline->GetSystemNodeInOutSize(0, inSize, outSize);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    pipeline->nodeMap_[0] = nullptr;
    ret = pipeline->GetSystemNodeInOutSize(0, inSize, outSize);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);

    pipeline->DeInit();
    std::shared_ptr<AudioNode> node = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->GetSystemNodeInOutSize(node->GetAudioNodeId(), inSize, outSize);
    EXPECT_EQ(ret, ERROR);

    node = std::make_shared<AudioSuiteSystemDialogueEnhanceNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->GetSystemNodeInOutSize(node->GetAudioNodeId(), inSize, outSize);
    EXPECT_EQ(ret, ERROR);
    pipeline->DeInit();

    AudioSuitePipelineImpl::DeInitGlobal();
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetRequestMetaDataCallbackTest, TestSize.Level0)
{
    AudioSuitePipelineImpl::DeInitGlobal();
    AudioSuitePipelineImpl::InitGlobal();

    auto pipeline = std::make_shared<AudioSuitePipelineImpl>(AudioSuite::PIPELINE_EDIT_MODE, 0);
    pipeline->pipelineState_ = PIPELINE_RUNNING;
    int32_t ret = pipeline->SetRequestMetaDataCallback(0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    pipeline->DeInit();
    pipeline->pipelineState_ = PIPELINE_STOPPED;
    ret = pipeline->SetRequestMetaDataCallback(0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    pipeline->DeInit();
    pipeline->nodeMap_[0] = nullptr;
    ret = pipeline->SetRequestMetaDataCallback(0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    pipeline->DeInit();
    std::shared_ptr<AudioNode> node = std::make_shared<AudioSuiteSystemMusicMasterNode>();
    pipeline->nodeMap_[node->GetAudioNodeId()] = node;
    ret = pipeline->SetRequestMetaDataCallback(node->GetAudioNodeId(), nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    pipeline->DeInit();

    AudioSuitePipelineImpl::DeInitGlobal();
}

}  // namespace
