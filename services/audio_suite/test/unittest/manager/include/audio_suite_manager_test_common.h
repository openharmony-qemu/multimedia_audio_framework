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

#ifndef AUDIO_SUITE_MANAGER_TEST_COMMON_H
#define AUDIO_SUITE_MANAGER_TEST_COMMON_H

#include "audio_suite_manager_callback.h"
#include "audio_suite_node.h"
#include <unordered_map>
class AudioSuiteManagerCallbackTestImpl : public OHOS::AudioStandard::AudioSuite::AudioSuiteManagerCallback {
public:
    AudioSuiteManagerCallbackTestImpl() = default;
    ~AudioSuiteManagerCallbackTestImpl() = default;

    void OnCreatePipeline(int32_t result, uint32_t pipelineId) override
    {
        isFinishCreatePipeline_ = true;
        engineCreateResult_ = result;
        engineCreatePipelineId_ = pipelineId;
    }
    void OnDestroyPipeline(int32_t result) override
    {
        isFinishDestroyPipeline_ = true;
        destroyPipelineResult_ = result;
    }
    void OnStartPipeline(int32_t result) override
    {
        isFinishStartPipeline_ = true;
        startPipelineResult_ = result;
    }
    void OnStopPipeline(int32_t result) override
    {
        isFinishStopPipeline_ = true;
        stopPipelineResult_ = result;
    }
    void OnGetPipelineState(OHOS::AudioStandard::AudioSuite::AudioSuitePipelineState state, int32_t result) override
    {
        isFinishGetPipelineState_ = true;
        getPipelineState_ = state;
        getPipelineResult_ = result;
    }
    void OnCreateNode(int32_t result, uint32_t nodeId) override
    {
        isFinishCreateNode_ = true;
        engineCreateNodeResult_ = result;
        engineCreateNodeId_ = nodeId;
    }
    void OnDestroyNode(int32_t result) override
    {
        isFinishDestroyNode_ = true;
        destroyNodeResult_ = result;
    }
    void OnBypassEffectNode(int32_t result) override
    {
        isFinishBypassEffectNode_ = true;
        bypassEffectNodeResult_ = result;
    }
    void OnGetNodeBypass(int32_t result, bool bypassStatus) override
    {
        isFinishGetNodeBypassStatus_ = true;
        getNodeBypassResult_ = result;
        getNodeBypassState_ = bypassStatus;
    }
    void OnSetAudioFormat(int32_t result) override
    {
        isFinishSetFormat_ = true;
        setFormatResult_ = result;
    }
    void OnWriteDataCallback(int32_t result) override
    {
        isFinishSetWriteData_ = true;
        setWriteDataResult_ = result;
    }
    void OnConnectNodes(int32_t result) override
    {
        isFinishConnectNodes_ = true;
        connectNodesResult_ = result;
    }
    void OnDisConnectNodes(int32_t result) override
    {
        isFinishDisConnectNodes_ = true;
        disConnectNodesResult_ = result;
    }
    void OnRenderFrame(int32_t result, uint32_t pipelineId) override
    {
        isFinishRenderFrameMap_[pipelineId] = true;
        renderFrameResultMap_[pipelineId] = result;
    }
    void OnMultiRenderFrame(int32_t result, uint32_t pipelineId) override
    {
        isFinishMultiRenderFrameMap_[pipelineId] = true;
        multiRenderFrameResultMap_[pipelineId] = result;
    }
    void OnGetOptions(int32_t result) override
    {
        isFinishGetOptions_ = true;
        getOptionsResult_ = result;
    }
    void OnSetOptions(int32_t result) override
    {
        isFinishSetOptions_ = true;
        setOptionsResult_  = result;
    }

    void OnPrintInfo(int32_t result) override
    {
        isFinishPrintInfo_ = true;
        printInfoResult_ = result;
    }
    void OnMetaRenderFrame(int32_t result, uint32_t pipelineId)
    {
        isFinishMetaRenderFrameMap_[pipelineId] = true;
        metaRenderFrameResultMap_[pipelineId] = result;
    }
    void OnSetSystemNodeParam(int32_t result)
    {
        isFinishSetSystemNodeParam_ = true;
        setSystemNodeParamResult_ = result;
    }
    void OnGetSystemNodeParam(int32_t result)
    {
        isFinishGetSystemNodeParam_ = true;
        getSystemNodeParamResult_ = result;
    }
    void OnGetSystemNodeInOutSize(int32_t result)
    {
        isFinishGetSystemNodeInOutSize_ = true;
        getSystemNodeInOutSizeResult_ = result;
    }

private:
    bool isFinishCreatePipeline_ = false;
    uint32_t engineCreatePipelineId_ = OHOS::AudioStandard::AudioSuite::INVALID_PIPELINE_ID;
    int32_t engineCreateResult_ = 0;
    bool isFinishDestroyPipeline_ = false;
    int32_t destroyPipelineResult_ = 0;
    bool isFinishStartPipeline_ = false;
    int32_t startPipelineResult_ = 0;
    bool isFinishStopPipeline_ = false;
    int32_t stopPipelineResult_ = 0;
    bool isFinishGetPipelineState_ = false;
    OHOS::AudioStandard::AudioSuite::AudioSuitePipelineState getPipelineState_ =
        OHOS::AudioStandard::AudioSuite::PIPELINE_STOPPED;
    int32_t getPipelineResult_ = 0;
    bool isFinishCreateNode_ = false;
    int32_t engineCreateNodeResult_ = 0;
    uint32_t engineCreateNodeId_ = OHOS::AudioStandard::AudioSuite::INVALID_NODE_ID;
    bool isFinishDestroyNode_ = false;
    int32_t destroyNodeResult_ = 0;
    bool isFinishBypassEffectNode_ = false;
    int32_t bypassEffectNodeResult_ = 0;
    bool isFinishGetNodeBypassStatus_ = false;
    bool getNodeBypassState_ = false;
    int32_t getNodeBypassResult_ = 0;
    bool isFinishSetFormat_ = false;
    int32_t setFormatResult_ = 0;
    bool isFinishSetWriteData_ = false;
    int32_t setWriteDataResult_ = 0;
    bool isFinishConnectNodes_ = false;
    int32_t connectNodesResult_ = 0;
    bool isFinishDisConnectNodes_ = false;
    int32_t disConnectNodesResult_ = 0;
    bool isFinishGetOptions_ = false;
    int32_t getOptionsResult_ = 0;
    bool isFinishSetOptions_ = false;
    int32_t setOptionsResult_ = 0;
    bool isFinishPrintInfo_ = false;
    int32_t printInfoResult_ = 0;
    int32_t isFinishSetSystemNodeParam_ = false;
    bool setSystemNodeParamResult_ = 0;
    int32_t isFinishGetSystemNodeParam_ = false;
    bool getSystemNodeParamResult_ = 0;
    int32_t isFinishGetSystemNodeInOutSize_ = false;
    bool getSystemNodeInOutSizeResult_ = 0;
    std::unordered_map<uint32_t, bool> isFinishRenderFrameMap_;
    std::unordered_map<uint32_t, int32_t> renderFrameResultMap_;
    std::unordered_map<uint32_t, bool> isFinishMultiRenderFrameMap_;
    std::unordered_map<uint32_t, int32_t> multiRenderFrameResultMap_;
    std::unordered_map<uint32_t, bool> isFinishMetaRenderFrameMap_;
    std::unordered_map<uint32_t, int32_t> metaRenderFrameResultMap_;
};

class AudioNodeTestImpl : public OHOS::AudioStandard::AudioSuite::AudioNode {
public:
    explicit AudioNodeTestImpl(OHOS::AudioStandard::AudioSuite::AudioNodeType nodeType)
        : OHOS::AudioStandard::AudioSuite::AudioNode(nodeType)
    {
    }
    virtual ~AudioNodeTestImpl() = default;
    int32_t DoProcess(uint32_t needDataLength) override
    {
        return 0;
    }
    int32_t Flush() override
    {
        return 0;
    }
    int32_t Connect(const std::shared_ptr<OHOS::AudioStandard::AudioSuite::AudioNode> &preNode) override
    {
        if (!preNode) {
            return -1;
        }
        return 0;
    }
    int32_t DisConnect(const std::shared_ptr<OHOS::AudioStandard::AudioSuite::AudioNode> &preNode) override
    {
        if (!preNode) {
            return -1;
        }
        return 0;
    }
};

class SuiteInputNodeRequestDataCallBackFinishedTestImpl
    : public OHOS::AudioStandard::AudioSuite::InputNodeRequestDataCallBack {
public:
    ~SuiteInputNodeRequestDataCallBackFinishedTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override

    {
        (void)audioData;
        (void)audioDataSize;
        if (finished != nullptr) {
            *finished = true;
        }
        return 0;
    }
};

class SuiteInputNodeRequestDataCallBackTestImpl
    : public SuiteInputNodeRequestDataCallBackFinishedTestImpl::InputNodeRequestDataCallBack {
public:
    ~SuiteInputNodeRequestDataCallBackTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override
    {
        (void)audioData;
        (void)audioDataSize;
        (void)finished;
        return 0;
    }
};

class AudioSuiteEngineManagerUnitTest : public testing::Test {
public:
    void SetUp()
    {
        if (!OHOS::AudioStandard::AudioSuite::AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    };
    void TearDown() {};
};

#endif // AUDIO_SUITE_MANAGER_TEST_COMMON_H