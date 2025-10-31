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

#ifndef AUDIO_SUITE_ENGINE_H
#define AUDIO_SUITE_ENGINE_H

#include <any>
#include <string>
#include <atomic>
#include <memory>
#include <functional>
#include <unordered_map>
#include "hpae_no_lock_queue.h"
#include "audio_suite_manager_thread.h"
#include "audio_suite_manager.h"
#include "audio_suite_manager_callback.h"
#include "i_audio_suite_engine.h"
#include "i_audio_suite_pipeline.h"
#include "audio_suite_msg_channel.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using OHOS::AudioStandard::HPAE::HpaeNoLockQueue;
using OHOS::AudioStandard::HPAE::Request;

namespace {
static const uint32_t MAX_PIPELINE_NUM = 10;
}

struct AudioSuiteEngineCfg {
    uint32_t maxPipelineNum_ = MAX_PIPELINE_NUM;
};

class AudioSuiteEngine : public IAudioSuiteEngine,
                         public IAudioSuiteManagerThread,
                         public ISendMsgCallback,
                         public std::enable_shared_from_this<AudioSuiteEngine> {
public:
    AudioSuiteEngine(AudioSuiteManagerCallback& callback);
    ~AudioSuiteEngine();

    // sync interface
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t CreatePipeline(PipelineWorkMode workMode) override;
    int32_t DestroyPipeline(uint32_t pipelineId) override;
    int32_t StartPipeline(uint32_t pipelineId) override;
    int32_t StopPipeline(uint32_t pipelineId) override;
    int32_t GetPipelineState(uint32_t pipelineId) override;

    int32_t CreateNode(uint32_t pipelineId, AudioNodeBuilder& builder) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t BypassEffectNode(uint32_t nodeId, bool bypass) override;
    int32_t GetNodeBypassStatus(uint32_t nodeId) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t RenderFrame(uint32_t pipelineId,
        uint8_t *audioData,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t MultiRenderFrame(uint32_t pipelineId, AudioDataArray *audioDataArray,
        int32_t *responseSize, bool *finishedFlag) override;

    // async interface
    int32_t SetOptions(uint32_t nodeId, std::string name, std::string value) override;
    int32_t GetOptions(uint32_t nodeId, std::string name, std::string &value) override;

    // for queue and thread
    bool IsRunning(void) override;
    bool IsMsgProcessing() override;
    void HandleMsg() override;
    void SendRequest(Request &&request, std::string funcName);
    void Invoke(PipelineMsgCode cmdID, const std::any &args) override;

private:
    bool IsInit();
    template <typename... Args>
    void RegisterHandler(PipelineMsgCode cmdID, void (AudioSuiteEngine::*func)(Args...));
    void HandleStartPipeline(int32_t result);
    void HandleStopPipeline(int32_t result);
    void HandleGetPipelineState(AudioSuitePipelineState state);
    void HandleCreateNode(int32_t result, uint32_t nodeId, uint32_t pipelineId);
    void HandleDestroyNode(int32_t result, uint32_t nodeId);
    void HandleBypassEffectNode(int32_t result);
    void HandleGetNodeBypassStatus(int32_t result, bool bypassStatus);
    void HandleSetAudioFormat(int32_t result);
    void HandleSetRequestDataCallback(int32_t result);
    void HandleConnectNodes(int32_t result);
    void HandleDisConnectNodes(int32_t result);
    void HandleRenderFrame(int32_t result, uint32_t pipelineId);
    void HandleMultiRenderFrame(int32_t result, uint32_t pipelineId);
    void HandleGetOptions(int32_t result);

private:
    std::atomic<bool> isInit_ = false;
    bool isExistRealtime_ = false;

    AudioSuiteManagerCallback& managerCallback_;
    AudioSuiteEngineCfg engineCfg_;
    std::unordered_map<uint32_t, std::shared_ptr<IAudioSuitePipeline>> pipelineMap_ = {};
    // key : nodeId, value : pipelineId
    std::unordered_map<uint32_t, uint32_t> nodeMap_ = {};

    // for queue and thread
    std::unique_ptr<AudioSuiteManagerThread> engineThread_ = nullptr;
    HpaeNoLockQueue engineNoLockQueue_;

    std::unordered_map<PipelineMsgCode, std::function<void(const std::any &)>> handlers_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_ENGINE_H