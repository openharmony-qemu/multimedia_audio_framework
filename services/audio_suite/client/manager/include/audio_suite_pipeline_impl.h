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
#ifndef AUDIO_SUITE_PIPELINE_IMPL_H
#define AUDIO_SUITE_PIPELINE_IMPL_H

#include <string>
#include <atomic>
#include <memory>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <sstream>
#include "audio_suite_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_pipeline.h"
#include "audio_suite_serializer.h"
#include "audio_suite_perf.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static const uint32_t MAX_INPUT_NODE_NUM = 15;
static const uint32_t MAX_OUTPUT_NODE_NUM = 1;
static const uint32_t MAX_EFFECT_NODE_NUM = 15;
static const uint32_t MAX_MIX_NODE_NUM = 3;
static const uint32_t MAX_AUDIO_SEPARATION_NODE_NUM = 1;
}

struct AudioSuitePipelineCfg {
    uint32_t maxInputNodeNum_ = MAX_INPUT_NODE_NUM;
    uint32_t maxOutputNodeNum_ = MAX_OUTPUT_NODE_NUM;
    uint32_t maxEffectNodeNum_ = MAX_EFFECT_NODE_NUM;
    uint32_t maxMixNodeNum_ = MAX_MIX_NODE_NUM;
    uint32_t maxAudioSeparationNodeNum_ = MAX_AUDIO_SEPARATION_NODE_NUM;
};

class AudioSuitePipelineImpl : public AudioSuitePipeline {
public:
    AudioSuitePipelineImpl(PipelineWorkMode mode, uint32_t id);
    ~AudioSuitePipelineImpl();

    // Static factory and global lifecycle
    static int32_t Create(PipelineWorkMode workMode, std::shared_ptr<AudioSuitePipeline> &pipeline);
    static int32_t IsNodeTypeSupported(AudioNodeType nodeType, bool &isSupported);

    int32_t DeInit() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t GetPipelineState(AudioSuitePipelineState &state) override;
    int32_t CreateNode(AudioNodeBuilder builder, uint32_t &nodeId) override;
    int32_t DestroyNode(uint32_t nodeId) override;
    int32_t BypassEffectNode(uint32_t nodeId, bool bypass) override;
    int32_t GetNodeBypassStatus(uint32_t nodeId, bool &bypass) override;
    int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) override;
    int32_t SetRequestDataCallback(uint32_t nodeId, std::shared_ptr<InputNodeRequestDataCallBack> callback) override;
    int32_t SetRequestMetaDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestMetaDataCallBack> callback) override;
    int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t DisconnectNodes(uint32_t srcNodeId, uint32_t destNodeId) override;
    int32_t RenderFrame(
        uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t MultiRenderFrame(
        uint8_t **audioDataArray, int32_t arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) override;
    int32_t MetaRenderFrame(AudioMetaFrame &metaFrame,
        int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag) override;
    int32_t SetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param) override;
    int32_t GetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param) override;
    int32_t GetSystemNodeInOutSize(uint32_t nodeId, uint32_t &inSize, uint32_t &outSize) override;

    // Typed parameter Set/Get (with validation)
    int32_t SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains) override;
    int32_t GetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains &gains) override;
    int32_t SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType) override;
    int32_t GetSoundFieldType(uint32_t nodeId, SoundFieldType &soundFieldType) override;
    int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType) override;
    int32_t GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType) override;
    int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType) override;
    int32_t GetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType &voiceBeautifierType) override;
    int32_t SetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams position) override;
    int32_t GetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams &position) override;
    int32_t SetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams rotation) override;
    int32_t GetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams &rotation) override;
    int32_t SetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams extension) override;
    int32_t GetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams &extension) override;
    int32_t SetTempoAndPitch(uint32_t nodeId, float speed, float pitch) override;
    int32_t GetTempoAndPitch(uint32_t nodeId, float &speed, float &pitch) override;
    int32_t SetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption option) override;
    int32_t GetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption &option) override;
    int32_t SetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType type) override;
    int32_t GetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType &type) override;

    int32_t PrintInfo(int32_t fd) override;

    // Getter methods for PrintInfo (not in AudioSuitePipeline interface; concrete class only)
    const std::unordered_map<uint32_t, std::shared_ptr<AudioNode>> &GetNodeMap() const;
    const std::unordered_map<uint32_t, uint32_t> &GetConnections() const;
    const std::unordered_map<uint32_t, std::vector<uint32_t>> &GetReverseConnections() const;
    AudioSuitePipelineState GetCurrentPipelineState() const;

private:
    static void InitGlobal();
    static void DeInitGlobal();
    static uint32_t AllocatePipeLineId();

    int32_t SetOptions(uint32_t nodeId, std::string name, std::string value);
    int32_t GetOptions(uint32_t nodeId, std::string name, std::string &value);
    template<typename T>
    int32_t SetParamsTemplate(uint32_t nodeId, const std::string& name, const T& param);
    template<typename T>
    int32_t GetParamsTemplate(uint32_t nodeId, const std::string& name, T& param);

    int32_t CreateNodeCheckParam(AudioNodeBuilder builder);
    uint32_t GetMaxNodeNumsForType(AudioNodeType type);
    std::shared_ptr<AudioNode> CreateNodeForType(AudioNodeBuilder builder);
    int32_t DestroyNodeForStop(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    int32_t DestroyNodeForRun(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    bool CheckPipelineNode(uint32_t startNodeId);
    int32_t ConnectNodesForStop(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    int32_t ConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    int32_t DisconnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
        std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode);
    void RemovceForwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    void RemovceBackwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node);
    void AddNodeConnections(uint32_t srcNodeId, uint32_t destNodeId);
    void ClearNodeConnections(uint32_t srcNodeId, uint32_t destNodeId);
    bool IsDirectConnected(uint32_t srcNodeId, uint32_t destNodeId);
    bool IsConnected(uint32_t srcNodeId, uint32_t destNodeId);
    // for dfx
    int32_t GetFrameDuration(int32_t frameSize, const AudioFormat &nodeFormat);
    void CheckRenderFrameTime(int32_t frameDurationMS, uint64_t processDurationUS);
    void CheckRenderFrameOverTimeCount();
    void SetMixerNodeSampleRate();
    bool CheckHoaInputNodeConnection(uint32_t nodeId, const AudioFormat &format);
    bool CheckHoaRenderPreNode(uint32_t nodeId);
    bool CheckSystemNodeConnection(uint32_t nodeId);
    void WriteSuiteEngineUtilizationStatsEvent(AudioNodeType nodeType);
    void WriteSuiteEngineExceptionEvent(uint32_t scene, uint32_t result, const std::string& description);
    // for ddl
    void StartGroup();
    void StopGroup();
    // validation
    bool ValidateEqualizerGains(const AudioEqualizerFrequencyBandGains &gains);
    bool ValidateSpaceRenderPosition(const AudioSpaceRenderPositionParams &position);
    bool ValidateSpaceRenderRotation(const AudioSpaceRenderRotationParams &rotation);
    bool ValidateSpaceRenderExtension(const AudioSpaceRenderExtensionParams &extension);
    bool ValidateTempoPitch(float speed, float pitch);
    bool ValidatePureVoiceChange(const AudioPureVoiceChangeOption &option);
    // utility
    int32_t StringToInt32(std::string &str);
    // PrintInfo helpers
    std::string GenerateTextSnapshot();
    void PrintPipelineSnapshotText(std::stringstream &ss);
    void PrintNodeSnapshotText(std::shared_ptr<AudioNode> node, std::stringstream &ss);
    int32_t WriteOutput(const std::string &content, int32_t fd);
    uint32_t GetPipelineId();
    PipelineWorkMode GetWorkMode();

private:
    static std::mutex createMutex_;
    static bool globalInit_;
    static uint32_t allocateId_;
    static int32_t workGroupId_;
    static uint32_t pipelineCount_;

    // for thread safety
    std::mutex pipelineMutex_;
    PipelineWorkMode pipelineWorkMode_;
    uint32_t id_;
    AudioSuitePipelineState pipelineState_ = PIPELINE_STOPPED;

    // for node
    std::shared_ptr<AudioOutputNode> outputNode_ = nullptr;
    std::unordered_map<AudioNodeType, uint32_t> nodeCounts_;
    std::unordered_map<uint32_t, std::shared_ptr<AudioNode>> nodeMap_ = {};
    std::unordered_map<uint32_t, uint32_t> connections_ = {};
    std::unordered_map<uint32_t, std::vector<uint32_t>> reverseConnections_ = {};

    // for dfx
    int32_t renderFrameTotalCount_ = 0;
    std::array<int32_t, RTF_OVERTIME_LEVELS> renderFrameOvertimeCounters_{};
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_PIPELINE_IMPL_H
