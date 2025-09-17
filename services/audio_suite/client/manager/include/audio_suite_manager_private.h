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
#ifndef AUDIO_SUITE_MANAGER_PRIVATE_H
#define AUDIO_SUITE_MANAGER_PRIVATE_H

#include <memory>
#include "audio_suite_manager.h"
#include "audio_suite_manager_callback.h"
#include "audio_suite_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteManager : public IAudioSuiteManager,
                          public AudioSuiteManagerCallback {
public:
    AudioSuiteManager() = default;
    ~AudioSuiteManager() = default;

    // engine
    int32_t Init() override;
    int32_t DeInit() override;

    // pipeline
    int32_t CreatePipeline(uint32_t &pipelineId) override;
    int32_t DestroyPipeline(uint32_t pipelineId) override;
    int32_t StartPipeline(uint32_t pipelineId) override;
    int32_t StopPipeline(uint32_t pipelineId) override;
    int32_t GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state) override;

    // node
    uint32_t CreateNode(
        uint32_t pipelineId, AudioNodeBuilder& builder) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t EnableNode(uint32_t nodeId, AudioNodeEnable audioNoedEnable) override;
    int32_t GetNodeEnableStatus(uint32_t nodeId, AudioNodeEnable &nodeEnable) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetOnWriteDataCallback(uint32_t nodeId,
        std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId,
        AudioNodePortType srcPortType, AudioNodePortType destPortType) override;
    int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t InstallTap(uint32_t nodeId, AudioNodePortType portType,
        std::shared_ptr<SuiteNodeReadTapDataCallback> callback) override;
    int32_t RemoveTap(uint32_t nodeId, AudioNodePortType portType) override;
    int32_t RenderFrame(uint32_t pipelineId,
        uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag) override;
    int32_t SetEquailizerMode(uint32_t nodeId, EqualizerMode eqMode) override;
    int32_t SetEqualizerFrequencyBandGains(
        uint32_t nodeId, AudioEqualizerFrequencyBandGains frequencyBandGains) override;
    int32_t SetSoundFiledType(uint32_t nodeId, SoundFieldType soundFieldType) override;
    int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType enviromentType) override;
    int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType) override;

    // callback Member functions
    void OnCreatePipeline(int32_t result, uint32_t pipelineId) override;
    void OnDestoryPipeline(int32_t result) override;
    void OnStartPipeline(int32_t result) override;
    void OnStopPipeline(int32_t result) override;
    void OnGetPipelineState(AudioSuitePipelineState state) override;
    void OnCreateNode(uint32_t nodeId) override;
    void OnDestroyNode(int32_t result) override;
    void OnEnableNode(int32_t result) override;
    void OnGetNodeEnable(AudioNodeEnable enable) override;
    void OnSetAudioFormat(int32_t result) override;
    void OnWriteDataCallback(int32_t result) override;
    void OnConnectNodes(int32_t result) override;
    void OnDisConnectNodes(int32_t result) override;
    void OnInstallTap(int32_t result) override;
    void OnRemoveTap(int32_t result) override;
    void OnRenderFrame(int32_t result) override;

private:
    std::mutex lock_;
    std::shared_ptr<IAudioSuiteEngine> suiteEngine_ = nullptr;

    // for status operation wait and notify
    std::mutex callbackMutex_;
    std::condition_variable callbackCV_;

    bool isFinishCreatePipeline_ = false;
    uint32_t engineCreatePipelineId_ = INVALID_PIPELINE_ID;
    int32_t engineCreateResult_ = 0;
    bool isFinishDestoryPipeline_ = false;
    int32_t destoryPipelineResult_ = 0;
    bool isFinishStartPipeline_ = false;
    int32_t startPipelineResult_ = 0;
    bool isFinishStopPipeline_ = false;
    int32_t stopPipelineResult_ = 0;
    bool isFinishGetPipelineState_ = false;
    AudioSuitePipelineState getPipelineState_ = PIPELINE_STOPPED;
    bool isFinishCreateNode_ = false;
    uint32_t engineCreateNodeId_ = INVALID_NODE_ID;
    bool isFinishDestroyNode_ = false;
    int32_t destroyNodeResult_ = 0;
    bool isFinishEnableNode_ = false;
    int32_t enableNodeResult_ = 0;
    bool isFinishGetNodeEnable_ = false;
    AudioNodeEnable getNodeEnable_ = NODE_DISABLE;
    bool isFinishSetFormat_ = false;
    int32_t setFormatResult_ = 0;
    bool isFinishSetWriteData_ = false;
    int32_t setWriteDataResult_ = 0;
    bool isFinishConnectNodes_ = false;
    int32_t connectNodesResult_ = 0;
    bool isFinishDisConnectNodes_ = false;
    int32_t disConnectNodesResult_ = 0;
    bool isFinisRenderFrame_ = false;
    int32_t renderFrameResult_ = 0;
    bool isFinisInstallTap_ = false;
    int32_t installTapResult_ = 0;
    bool isFinisRemoveTap_ = false;
    int32_t removeTapResult_ = 0;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_PRIVATE_H