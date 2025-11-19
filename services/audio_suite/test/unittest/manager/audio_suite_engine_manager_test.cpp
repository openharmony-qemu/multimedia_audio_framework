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
#include <string>
#include <thread>
#include <cstdio>
#include <unistd.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "audio_suite_engine.h"
#include "audio_suite_manager_private.h"
#include "audio_suite_manager_callback.h"
#include "audio_suite_pipeline.h"
#include "audio_suite_base.h"


using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

class AudioSuiteEngineManagerUnitTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

class MockAudioSuiteManagerCallback : public AudioSuiteManagerCallback {
public:
    MOCK_METHOD(void, OnDestroyPipeline, (int32_t result), (override));
    MOCK_METHOD(void, OnCreatePipeline, (int32_t result, uint32_t pipelineId), (override));
    MOCK_METHOD(void, OnStartPipeline, (int32_t result), (override));
    MOCK_METHOD(void, OnStopPipeline, (int32_t result), (override));
    MOCK_METHOD(void, OnGetPipelineState, (AudioSuitePipelineState state), (override));
    MOCK_METHOD(void, OnCreateNode, (int32_t result, uint32_t nodeId), (override));
    MOCK_METHOD(void, OnDestroyNode, (int32_t result), (override));
    MOCK_METHOD(void, OnBypassEffectNode, (int32_t result), (override));
    MOCK_METHOD(void, OnGetNodeBypass, (int32_t result, bool bypassStatus), (override));
    MOCK_METHOD(void, OnSetAudioFormat, (int32_t result), (override));
    MOCK_METHOD(void, OnWriteDataCallback, (int32_t result), (override));
    MOCK_METHOD(void, OnConnectNodes, (int32_t result), (override));
    MOCK_METHOD(void, OnDisConnectNodes, (int32_t result), (override));
    MOCK_METHOD(void, OnRenderFrame, (int32_t result, uint32_t pipelineId), (override));
    MOCK_METHOD(void, OnGetOptions, (int32_t result), (override));
};

class AudioSuiteManagerCallbackTestImpl : public AudioSuiteManagerCallback {
public:
    AudioSuiteManagerCallbackTestImpl() = default;
    ~AudioSuiteManagerCallbackTestImpl() = default;

    void OnCreatePipeline(int32_t result, uint32_t pipelineId) override
    {
        EXPECT_EQ(pipelineId, INVALID_PIPELINE_ID);
    }
    void OnDestroyPipeline(int32_t result) override
    {
        return;
    }
    void OnStartPipeline(int32_t result) override
    {
        return;
    }
    void OnStopPipeline(int32_t result) override
    {
        return;
    }
    void OnGetPipelineState(AudioSuitePipelineState state) override
    {
        return;
    }
    void OnCreateNode(int32_t result, uint32_t nodeId) override
    {
        return;
    }
    void OnDestroyNode(int32_t result) override
    {
        return;
    }
    void OnBypassEffectNode(int32_t result) override
    {
        return;
    }
    void OnGetNodeBypass(int32_t result, bool bypassStatus) override
    {
        return;
    }
    void OnSetAudioFormat(int32_t result) override
    {
        return;
    }
    void OnWriteDataCallback(int32_t result) override
    {
        return;
    }
    void OnConnectNodes(int32_t result) override
    {
        return;
    }
    void OnDisConnectNodes(int32_t result) override
    {
        return;
    }
    void OnRenderFrame(int32_t result, uint32_t pipelineId) override
    {
        return;
    }
    void OnMultiRenderFrame(int32_t result, uint32_t pipelineId) override
    {
        return;
    }
    void OnGetOptions(int32_t result) override
    {
        return;
    }
};

class IAudioSuitePipelineTestImpl : public IAudioSuitePipeline {
public:
    IAudioSuitePipelineTestImpl() = default;
    ~IAudioSuitePipelineTestImpl() = default;

    int32_t Init() override
    {
        return 0;
    }
    int32_t DeInit() override
    {
        return 0;
    }
    int32_t Start() override
    {
        return 0;
    }
    int32_t Stop() override
    {
        return 0;
    }
    int32_t GetPipelineState() override
    {
        return 0;
    }
    int32_t CreateNode(AudioNodeBuilder builder) override
    {
        return 0;
    }
    int32_t DestroyNode(uint32_t nodeId) override
    {
        return 0;
    }
    int32_t BypassEffectNode(uint32_t nodeId, bool bypass) override
    {
        return 0;
    }
    int32_t GetNodeBypassStatus(uint32_t nodeId) override
    {
        return 0;
    }
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override
    {
        return 0;
    }
    int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) override
    {
        return 0;
    }
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override
    {
        return 0;
    }
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override
    {
        return 0;
    }
    int32_t RenderFrame(uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag) override
    {
        return 0;
    }
    int32_t MultiRenderFrame(uint8_t **audioDataArray, int arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override
    {
        return 0;
    }
    int32_t SetOptions(uint32_t nodeId, std::string name, std::string value)  override
    {
        return 0;
    }
    int32_t GetOptions(uint32_t nodeId, std::string name, std::string &value) override
    {
        return 0;
    }
    uint32_t GetPipelineId()  override
    {
        return 0;
    }
};

class SuiteInputNodeRequestDataCallBackTestImpl : public InputNodeRequestDataCallBack {
public:
    ~SuiteInputNodeRequestDataCallBackTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override
    {
        return 0;
    }
};

class SuiteNodeReadTapDataCallbackTestImpl : public SuiteNodeReadTapDataCallback {
public:
    virtual ~SuiteNodeReadTapDataCallbackTestImpl() = default;
    void OnReadTapDataCallback(void *audioData, int32_t audioDataSize) override
    {
        return;
    }
};

class AudioNodeTestImpl : public AudioNode {
public:
    explicit AudioNodeTestImpl(AudioNodeType nodeType):AudioNode(nodeType)
    {
    }
    virtual ~AudioNodeTestImpl() = default;
    int32_t DoProcess() override
    {
        return 0;
    }
    int32_t Flush() override
    {
        return 0;
    }
    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override
    {
        return 0;
    }
    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override
    {
        return 0;
    }
};

HWTEST_F(AudioSuiteEngineManagerUnitTest, constructEngineManagerTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    sleep(1);
    EXPECT_EQ(engineManger.IsRunning(), true);
    engineManger.DeInit();
    EXPECT_EQ(engineManger.IsInit(), false);
    sleep(1);
    EXPECT_EQ(engineManger.IsRunning(), false);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createPipelineTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);

    int32_t result = engineManger.CreatePipeline(PIPELINE_EDIT_MODE);
    for (size_t i = 1;i < engineManger.engineCfg_.maxPipelineNum_ + 3; ++i)
    {
        engineManger.pipelineMap_.insert(std::make_pair(i, std::make_shared<IAudioSuitePipelineTestImpl>()));
    }

    result = engineManger.CreatePipeline(PIPELINE_EDIT_MODE);
    EXPECT_EQ(result, SUCCESS);

    engineManger.DeInit();
    result = engineManger.CreatePipeline(PIPELINE_EDIT_MODE);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, destroyPipelineTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);

    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
   
    engineManger.pipelineMap_[1] = nullptr;
    int32_t result = engineManger.DestroyPipeline(1);
    result = engineManger.DestroyPipeline(2);

    engineManger.pipelineMap_[3] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.DestroyPipeline(3);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, startPipelineTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.pipelineMap_[1] = nullptr;
    int32_t result = engineManger.StartPipeline(1);
    result = engineManger.StartPipeline(2);

    engineManger.pipelineMap_[3] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.StartPipeline(3);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, stopPipelineTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.pipelineMap_[1] = nullptr;
    int32_t result = engineManger.StopPipeline(1);
    result = engineManger.StopPipeline(2);

    engineManger.pipelineMap_[3] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.StopPipeline(3);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getPipelineTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.pipelineMap_[1] = nullptr;
    int32_t result = engineManger.GetPipelineState(1);
    result = engineManger.GetPipelineState(2);

    engineManger.pipelineMap_[3] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.GetPipelineState(3);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, createNodeTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    AudioNodeBuilder builder;
    engineManger.pipelineMap_[1] = nullptr;
    int32_t result = engineManger.CreateNode(1, builder);
    result = engineManger.CreateNode(2, builder);

    engineManger.pipelineMap_[3] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.CreateNode(3, builder);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, destroyNodeTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    int32_t result = engineManger.DestroyNode(2);

    engineManger.nodeMap_[1] = 3;
    result = engineManger.DestroyNode(1);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.DestroyNode(5);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, bypassEffectNodeTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.BypassEffectNode(1, true);
    result = engineManger.BypassEffectNode(2, true);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.BypassEffectNode(5, true);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getNodeEnableStatusTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.GetNodeBypassStatus(1);
    result = engineManger.GetNodeBypassStatus(2);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.GetNodeBypassStatus(5);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, setAudioFormatTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    AudioFormat audioFormat;
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.SetAudioFormat(1, audioFormat);
    result = engineManger.SetAudioFormat(2, audioFormat);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.SetAudioFormat(5, audioFormat);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, SetRequestDataCallbackTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    std::shared_ptr<InputNodeRequestDataCallBack> suiteCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.SetRequestDataCallback(1, suiteCallback);
    result = engineManger.SetRequestDataCallback(2, suiteCallback);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.SetRequestDataCallback(5, suiteCallback);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, connectNodesTest_001, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    uint32_t srcNodeId = 4;
    uint32_t destNodeId = 5;
    int32_t result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    srcNodeId = 1;
    destNodeId = 2;
    engineManger.nodeMap_[1] = 3;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    srcNodeId = 6;
    destNodeId = 7;
    engineManger.nodeMap_[7] = 7;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[8] = 8;
    engineManger.nodeMap_[9] = 9;
    srcNodeId = 8;
    destNodeId = 9;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[10] = 12;
    engineManger.nodeMap_[11] = 12;
    srcNodeId = 10;
    destNodeId = 11;
    engineManger.pipelineMap_[12] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, connectNodesTest_002, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    uint32_t srcNodeId = 4;
    uint32_t destNodeId = 5;
    int32_t result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    srcNodeId = 1;
    destNodeId = 2;
    engineManger.nodeMap_[1] = 3;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    srcNodeId = 6;
    destNodeId = 7;
    engineManger.nodeMap_[7] = 7;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[8] = 8;
    engineManger.nodeMap_[9] = 9;
    srcNodeId = 8;
    destNodeId = 9;
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[10] = 12;
    engineManger.nodeMap_[11] = 12;
    srcNodeId = 10;
    destNodeId = 11;
    engineManger.pipelineMap_[12] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, disConnectNodesTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    uint32_t srcNodeId = 4;
    uint32_t destNodeId = 5;
    int32_t result = engineManger.DisConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[1] = 3;
    srcNodeId = 1;
    result = engineManger.DisConnectNodes(srcNodeId, destNodeId);

    srcNodeId = 6;
    destNodeId = 7;
    engineManger.nodeMap_[7] = 7;
    result = engineManger.DisConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[8] = 8;
    engineManger.nodeMap_[9] = 9;
    srcNodeId = 8;
    destNodeId = 9;
    result = engineManger.DisConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[10] = 12;
    engineManger.nodeMap_[11] = 12;
    srcNodeId = 10;
    destNodeId = 11;
    result = engineManger.DisConnectNodes(srcNodeId, destNodeId);

    engineManger.nodeMap_[13] = 15;
    engineManger.nodeMap_[14] = 15;
    srcNodeId = 13;
    destNodeId = 14;
    engineManger.pipelineMap_[15] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, renderFrameTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
    
    engineManger.pipelineMap_[1] = std::make_shared<IAudioSuitePipelineTestImpl>();
    engineManger.pipelineMap_[2] = nullptr;
    int32_t result = engineManger.RenderFrame(1, nullptr, 1, nullptr, nullptr);
    result = engineManger.RenderFrame(2, nullptr, 1, nullptr, nullptr);
    result = engineManger.RenderFrame(3, nullptr, 1, nullptr, nullptr);

    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, multiRenderFrameTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);

    AudioDataArray audioDataArray;
    engineManger.pipelineMap_[1] = std::make_shared<IAudioSuitePipelineTestImpl>();
    engineManger.pipelineMap_[2] = nullptr;
    int32_t result = engineManger.MultiRenderFrame(1, &audioDataArray, nullptr, nullptr);
    result = engineManger.MultiRenderFrame(2, &audioDataArray, nullptr, nullptr);
    result = engineManger.MultiRenderFrame(3, &audioDataArray, nullptr, nullptr);

    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, setOptionsTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
   
    std::string name = "abc";
    std::string value = "def";
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.SetOptions(1, name, value);
    result = engineManger.SetOptions(2, name, value);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.SetOptions(5, name, value);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, getOptionsTest, TestSize.Level0)
{
    AudioSuiteManagerCallbackTestImpl callback;
    AudioSuiteEngine engineManger(callback);
    engineManger.Init();
    EXPECT_EQ(engineManger.IsInit(), true);
   
    std::string name = "abc";
    std::string value = "def";
    engineManger.nodeMap_[1] = 3;
    int32_t result = engineManger.GetOptions(1, name, value);
    result = engineManger.GetOptions(2, name, value);

    engineManger.nodeMap_[5] = 6;
    engineManger.pipelineMap_[6] = std::make_shared<IAudioSuitePipelineTestImpl>();
    result = engineManger.GetOptions(5, name, value);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    sleep(1);
    EXPECT_EQ(audioSuitePipeline.IsRunning(), true);
    audioSuitePipeline.DeInit();
    EXPECT_EQ(audioSuitePipeline.IsInit(), false);
    sleep(1);
    EXPECT_EQ(audioSuitePipeline.IsRunning(), false);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineStopTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    int32_t result = audioSuitePipeline.Stop();

    audioSuitePipeline.nodeMap_[2] = nullptr;
    result = audioSuitePipeline.Stop();
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCreateNodeTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    AudioNodeBuilder audioNodeBuilder;
    audioNodeBuilder.nodeType = NODE_TYPE_INPUT;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_EDIT_MODE;

    audioSuitePipeline.nodeCounts_[static_cast<std::size_t>(audioNodeBuilder.nodeType)] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) - 2;
    int32_t result = audioSuitePipeline.CreateNode(audioNodeBuilder);

    audioSuitePipeline.nodeCounts_[static_cast<std::size_t>(audioNodeBuilder.nodeType)] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) + 2;
    result = audioSuitePipeline.CreateNode(audioNodeBuilder);

    audioNodeBuilder.nodeType = NODE_TYPE_SOUND_FIELD;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_REALTIME_MODE;
    audioSuitePipeline.nodeCounts_[static_cast<std::size_t>(audioNodeBuilder.nodeType)] =
        audioSuitePipeline.GetMaxNodeNumsForType(audioNodeBuilder.nodeType) - 2;
    result = audioSuitePipeline.CreateNode(audioNodeBuilder);

    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineCreateNodeCheckParmeTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
   
    AudioNodeBuilder audioNodeBuilder;
    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_REALTIME_MODE;

    audioNodeBuilder.nodeType = NODE_TYPE_INPUT;
    int32_t result = audioSuitePipeline.CreateNodeCheckParme(audioNodeBuilder);

    audioNodeBuilder.nodeType = NODE_TYPE_OUTPUT;
    result = audioSuitePipeline.CreateNodeCheckParme(audioNodeBuilder);

    audioNodeBuilder.nodeType = NODE_TYPE_EQUALIZER;
    result = audioSuitePipeline.CreateNodeCheckParme(audioNodeBuilder);

    audioSuitePipeline.pipelineWorkMode_ = PIPELINE_EDIT_MODE;
    result = audioSuitePipeline.CreateNodeCheckParme(audioNodeBuilder);

    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetMaxNodeNumsForTypeTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    AudioNodeType audioNodeType = NODE_TYPE_INPUT;
    int32_t result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, audioSuitePipeline.pipelineCfg_.maxInputNodeNum_);

    audioNodeType = NODE_TYPE_OUTPUT;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, audioSuitePipeline.pipelineCfg_.maxOutputNodeNum_);

    audioNodeType = NODE_TYPE_AUDIO_MIXER;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, audioSuitePipeline.pipelineCfg_.maxMixNodeNum_);

    audioNodeType = NODE_TYPE_NOISE_REDUCTION;
    result = audioSuitePipeline.GetMaxNodeNumsForType(audioNodeType);
    EXPECT_EQ(result, audioSuitePipeline.pipelineCfg_.maxEffectNodeNum_);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDestroyNodeForRunTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
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
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
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
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    int32_t result = audioSuitePipeline.BypassEffectNode(2, true);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.BypassEffectNode(1, true);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.BypassEffectNode(3, true);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[4] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.BypassEffectNode(4, true);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetNodeBypassStatusTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    int32_t result = audioSuitePipeline.GetNodeBypassStatus(2);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.GetNodeBypassStatus(1);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.GetNodeBypassStatus(3);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetAudioFormatTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    AudioFormat audioFormat;

    int32_t result = audioSuitePipeline.SetAudioFormat(2, audioFormat);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetAudioFormat(1, audioFormat);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = nullptr;
    result = audioSuitePipeline.SetAudioFormat(3, audioFormat);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetRequestDataCallbackTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    std::shared_ptr<InputNodeRequestDataCallBack> suitCallback =
        std::make_shared<SuiteInputNodeRequestDataCallBackTestImpl>();

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.SetRequestDataCallback(2, suitCallback);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.SetRequestDataCallback(4, suitCallback);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetRequestDataCallback(3, suitCallback);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[5] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    result = audioSuitePipeline.SetRequestDataCallback(5, suitCallback);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_001, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 1;
    destNodeId = 2;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 3;
    destNodeId = 4;
    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 5;
    destNodeId = 6;
    audioSuitePipeline.nodeMap_[6] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 7;
    destNodeId = 8;
    audioSuitePipeline.nodeMap_[7] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_OUTPUT);
    audioSuitePipeline.nodeMap_[8] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 9;
    destNodeId = 10;
    audioSuitePipeline.nodeMap_[9] = nullptr;
    audioSuitePipeline.nodeMap_[10] = nullptr;
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 11;
    destNodeId = 12;
    audioSuitePipeline.nodeMap_[12] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 13;
    destNodeId = 14;
    audioSuitePipeline.nodeMap_[13] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_SEPARATION);
    audioSuitePipeline.nodeMap_[14] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_002, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    sleep(1);
    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.ConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

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
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesTest_003, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    sleep(1);
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

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisConnectNodesTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
   
    uint32_t srcNodeId = 0;
    uint32_t destNodeId = 0;

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    int32_t result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 1;
    destNodeId = 2;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 3;
    destNodeId = 4;
    audioSuitePipeline.nodeMap_[3] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 5;
    destNodeId = 6;
    audioSuitePipeline.nodeMap_[6] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 7;
    destNodeId = 8;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;
    audioSuitePipeline.nodeMap_[7] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[8] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 9;
    destNodeId = 10;
    audioSuitePipeline.nodeMap_[9] = nullptr;
    audioSuitePipeline.nodeMap_[10] = nullptr;
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 11;
    destNodeId = 12;
    audioSuitePipeline.nodeMap_[11] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.nodeMap_[12] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);

    srcNodeId = 13;
    destNodeId = 14;
    audioSuitePipeline.nodeMap_[14] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.DisConnectNodes(srcNodeId, destNodeId);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineConnectNodesForRunTest_001, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

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
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

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

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDisConnectNodesForRunTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    uint32_t srcNodeId = 1;
    uint32_t destNodeId = 2;
    std::shared_ptr<AudioNode> destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    std::shared_ptr<AudioNode> srcNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    destNode->audioNodeInfo_.nodeType = NODE_TYPE_ENVIRONMENT_EFFECT;
    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = nullptr;

    int32_t result = audioSuitePipeline.DisConnectNodesForRun(srcNodeId, destNodeId, srcNode,  destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    audioSuitePipeline.outputNode_->audioNodeInfo_.nodeId = destNodeId;
    result = audioSuitePipeline.DisConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);

    destNode = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_AUDIO_MIXER);
    result = audioSuitePipeline.DisConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_ILLEGAL_STATE);

    audioSuitePipeline.reverseConnections_[destNodeId] = {};
    result = audioSuitePipeline.DisConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    EXPECT_EQ(result, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    uint8_t *audioData = nullptr;
    int32_t frameSize = 0;
    int32_t *writeLen = nullptr;
    bool *finishedFlag = nullptr;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.outputNode_ = nullptr;
    result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, SUCCESS);

    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    result = audioSuitePipeline.RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineMultiRenderFrameTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    uint8_t **audioDataArray = nullptr;
    int arraySize = 0;
    int32_t requestFrameSize = 0;
    int32_t *responseSize = nullptr;
    bool *finishedFlag = nullptr;
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.outputNode_ = nullptr;
    result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, SUCCESS);

    AudioFormat audioFormat;
    audioSuitePipeline.outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
    result = audioSuitePipeline.MultiRenderFrame(
        audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineSetOptionsTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    uint32_t nodeId = 1;
    std::string name = "abc";
    std::string value = "def";
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.SetOptions(nodeId, name, value);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.nodeMap_[nodeId + 1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.SetOptions(nodeId + 1, name, value);
    EXPECT_EQ(result, SUCCESS);

    result = audioSuitePipeline.SetOptions(nodeId + 2, name, value);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineGetOptionsTest, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);
    
    uint32_t nodeId = 1;
    std::string name = "abc";
    std::string value = "def";
    audioSuitePipeline.pipelineState_ = PIPELINE_STOPPED;

    int32_t result = audioSuitePipeline.GetOptions(nodeId, name, value);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    audioSuitePipeline.nodeMap_[nodeId + 1] = std::make_shared<AudioNodeTestImpl>(NODE_TYPE_INPUT);
    result = audioSuitePipeline.GetOptions(nodeId + 1, name, value);
    EXPECT_EQ(result, SUCCESS);

    result = audioSuitePipeline.GetOptions(nodeId + 2, name, value);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineDestroyNodeTest_001, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    uint32_t srcNodeId = 0;
    int32_t result = audioSuitePipeline.DestroyNode(srcNodeId);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.nodeMap_[1] = nullptr;
    result = audioSuitePipeline.DestroyNode(srcNodeId);
    EXPECT_EQ(result, SUCCESS);

    audioSuitePipeline.pipelineState_ = PIPELINE_RUNNING;
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioSuiteEngineManagerUnitTest, audioSuitePipelineRemovceForwardConnetTest_001, TestSize.Level0)
{
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

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
    AudioSuitePipeline audioSuitePipeline(PIPELINE_EDIT_MODE);
    audioSuitePipeline.Init();
    EXPECT_EQ(audioSuitePipeline.IsInit(), true);

    bool result = audioSuitePipeline.CheckPipelineNode(0);
    EXPECT_FALSE(result);
}

}  // namespace