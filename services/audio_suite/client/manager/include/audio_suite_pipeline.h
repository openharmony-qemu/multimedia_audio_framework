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
#ifndef AUDIO_SUITE_PIPELINE_H
#define AUDIO_SUITE_PIPELINE_H

#include <string>
#include <atomic>
#include <memory>
#include <functional>
#include <unordered_map>
#include "hpae_no_lock_queue.h"
#include "audio_suite_manager_thread.h"
#include "audio_suite_manager.h"
#include "audio_suite_node.h"
#include "audio_suite_msg_channel.h"
#include "audio_suite_output_node.h"
#include "i_audio_suite_pipeline.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using OHOS::AudioStandard::HPAE::HpaeNoLockQueue;
using OHOS::AudioStandard::HPAE::Request;

namespace {
static const uint32_t MAX_INPUT_NODE_NUM = 5;
static const uint32_t MAX_OUTPUT_NODE_NUM = 1;
static const uint32_t MAX_EFFECT_NODE_NUM = 5;
static const uint32_t MAX_MIX_NODE_NUM = 3;
}

struct AudioEdiPipelineCfg {
    uint32_t maxInputNodeNum_ = MAX_INPUT_NODE_NUM;
    uint32_t maxOutputNodeNum_ = MAX_OUTPUT_NODE_NUM;
    uint32_t maxEffectNodeNum_ = MAX_EFFECT_NODE_NUM;
    uint32_t maxMixNodeNum_ = MAX_MIX_NODE_NUM;
};

class AudioSuitePipeline : public IAudioSuitePipeline,
                           public IAudioSuiteManagerThread,
                           public CallbackSender,
                           public std::enable_shared_from_this<AudioSuitePipeline> {
public:
    AudioSuitePipeline(PipelineWorkMode mode);
    ~AudioSuitePipeline();

    int32_t Init() override;
    int32_t DeInit() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t GetPipelineState() override;
    uint32_t GetPipelineId() override;
    int32_t CreateNode(AudioNodeBuilder builder) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t BypassEffectNode(uint32_t nodeId, bool bypass) override;
    int32_t GetNodeBypassStatus(uint32_t nodeId) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t RenderFrame(
        uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t MultiRenderFrame(
        uint8_t **audioDataArray, int arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t SetOptions(uint32_t nodeId, std::string name, std::string value) override;
    int32_t GetOptions(uint32_t nodeId, std::string name, std::string &value) override;

    // for queue and thread
    bool IsRunning(void) override;
    bool IsMsgProcessing() override;
    void HandleMsg() override;
    void SendRequest(Request &&request, std::string funcName);

private:
    bool IsInit();
    int32_t CreateNodeCheckParme(AudioNodeBuilder builder);
    uint32_t GetMaxNodeNumsForType(AudioNodeType type);
    std::shared_ptr<AudioNode> CreateNodeForType(AudioNodeBuilder builder);
    int32_t DestroyNodeForStop(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    int32_t DestroyNodeForRun(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    bool CheckPipelineNode(uint32_t startNodeId);
    int32_t ConnectNodesForStop(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    int32_t ConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    int32_t DisConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    void RemovceForwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    void RemovceBackwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    void AddNodeConnections(uint32_t srcNodeId, uint32_t destNodeId);
    void ClearNodeConnections(uint32_t srcNodeId, uint32_t destNodeId);
    bool IsDirectConnected(uint32_t srcNodeId, uint32_t destNodeId);
    bool IsConnected(uint32_t srcNodeId, uint32_t destNodeId);

private:
    static std::mutex allocateIdLock;
    static uint32_t allocateId;

    uint32_t id_;
    PipelineWorkMode pipelineWorkMode_;
    std::atomic<bool> isInit_ = false;
    AudioEdiPipelineCfg pipelineCfg_;
    std::vector<uint32_t> nodeCounts_;
    AudioSuitePipelineState pipelineState_ = PIPELINE_STOPPED;

    std::shared_ptr<AudioOutputNode> outputNode_ = nullptr;

    // for node
    std::unordered_map<uint32_t, std::shared_ptr<AudioNode>> nodeMap_ = {};
    std::unordered_map<uint32_t, uint32_t> connections_ = {};
    std::unordered_map<uint32_t, std::vector<uint32_t>> reverseConnections_ = {};

    // for queue and thread
    std::unique_ptr<AudioSuiteManagerThread> pipelineThread_ = nullptr;
    HpaeNoLockQueue pipelineNoLockQueue_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_PIPELINE_H