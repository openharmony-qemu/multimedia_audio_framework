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

#ifndef OH_AUDIO_SUITE_ENGINE_H
#define OH_AUDIO_SUITE_ENGINE_H

#include <mutex>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "native_audio_suite_engine.h"
#include "audio_suite_pipeline.h"
#include "OHAudioSuiteNodeBuilder.h"

namespace OHOS {
namespace AudioStandard {

class OHSuiteInputNodeRequestDataCallBack : public AudioSuite::InputNodeRequestDataCallBack {
public:
    explicit OHSuiteInputNodeRequestDataCallBack(
        OH_AudioNode *audioNode, OH_InputNode_RequestDataCallback callback, void *data)
        : audioNode_(audioNode), callback_(callback), userData_(data) {}
    ~OHSuiteInputNodeRequestDataCallBack() = default;

    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override;

private:
    OH_AudioNode *audioNode_ = nullptr;
    OH_InputNode_RequestDataCallback callback_ = nullptr;
    void *userData_ = nullptr;
};

class OHSuiteInputNodeRequestMetaDataCallBack : public AudioSuite::InputNodeRequestMetaDataCallBack {
public:
    explicit OHSuiteInputNodeRequestMetaDataCallBack(
        OH_AudioNode *audioNode, OH_InputNode_RequestMetaDataCallback callback, void *data)
        : audioNode_(audioNode), callback_(callback), userData_(data) {}
    ~OHSuiteInputNodeRequestMetaDataCallBack() = default;

    int32_t OnRequestDataCallBack(
        AudioSuite::AudioMetaFrame &metaFrame, int32_t *responseMetaDataSize, bool *finished) override;

private:
    OH_AudioNode *audioNode_ = nullptr;
    OH_InputNode_RequestMetaDataCallback callback_ = nullptr;
    void *userData_ = nullptr;
};

class OHAudioNode {
public:
    explicit OHAudioNode(uint32_t id, AudioSuite::AudioNodeType type,
                         std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline)
        : nodeId_(id), type_(type), pipeline_(std::move(pipeline)) {};
    ~OHAudioNode() = default;

    uint32_t GetNodeId() const;
    AudioSuite::AudioNodeType GetNodeType() const;
    std::shared_ptr<AudioSuite::AudioSuitePipeline> GetPipeline() const;

private:
    uint32_t nodeId_ = AudioSuite::INVALID_NODE_ID;
    AudioSuite::AudioNodeType type_ = AudioSuite::NODE_TYPE_EMPTY;
    std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline_;
};

class OHAudioSuitePipeline {
public:
    explicit OHAudioSuitePipeline(std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline)
        : pipeline_(std::move(pipeline)) {};
    ~OHAudioSuitePipeline() = default;

    std::shared_ptr<AudioSuite::AudioSuitePipeline> GetPipeline() const;
    void AddNode(OHAudioNode *node);
    void RemoveNode(OHAudioNode *node);
    bool IsNodeExists(OHAudioNode *node);
    void GetAllNodes(std::vector<OHAudioNode*> &nodes);

private:
    std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline_;
    std::unordered_set<OHAudioNode*> nodes_;
    std::mutex mutex_;
};

class OHAudioSuiteEngine {
public:
    ~OHAudioSuiteEngine();

    static OHAudioSuiteEngine &GetInstance();

    // engine
    int32_t CreateEngine();
    int32_t DestroyEngine();

    // pipeline
    int32_t CreatePipeline(OH_AudioSuitePipeline **audioSuitePipeline, OH_AudioSuite_PipelineWorkMode ohWorkMode);
    int32_t DestroyPipeline(OHAudioSuitePipeline *audioPipeline);
    int32_t StartPipeline(OHAudioSuitePipeline *audioPipeline);
    int32_t StopPipeline(OHAudioSuitePipeline *audioPipeline);
    int32_t GetPipelineState(OHAudioSuitePipeline *audioPipeline, OH_AudioSuite_PipelineState *state);
    int32_t RenderFrame(OHAudioSuitePipeline *audioPipeline,
        uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag);
    int32_t MultiRenderFrame(OHAudioSuitePipeline *audioPipeline,
        AudioSuite::AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag);
    int32_t MetaRenderFrame(OHAudioSuitePipeline *audioPipeline,
        AudioSuite::AudioMetaFrame &metaFrame, int32_t *responseAudioSize,
        int32_t *responseMetaSize, bool *finishedFlag);

    // node
    int32_t CreateNode(
        OHAudioSuitePipeline *audioSuitePipeline, OHAudioSuiteNodeBuilder *builder, OH_AudioNode **audioNode);
    int32_t DestroyNode(OHAudioNode *node);
    int32_t GetNodeBypassStatus(OHAudioNode *node, bool *bypassStatus);
    int32_t BypassEffectNode(OHAudioNode *node, bool bypass);
    int32_t SetAudioFormat(OHAudioNode *node, OH_AudioFormat *audioFormat);
    int32_t SetAudioFormat(OHAudioNode *node, OH_AudioSuite_SystemNodeFormat *audioFormat);
    int32_t ConnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode);
    int32_t DisconnectNodes(OHAudioNode *srcNode, OHAudioNode *destNode);
    int32_t SetEqualizerFrequencyBandGains(
        OHAudioNode *node, OH_EqualizerFrequencyBandGains frequencyBandGains);
    int32_t SetSoundFieldType(OHAudioNode *node, OH_SoundFieldType soundFieldType);
    int32_t SetEnvironmentType(OHAudioNode *node, OH_EnvironmentType environmentType);
    int32_t SetVoiceBeautifierType(
        OHAudioNode *node, OH_VoiceBeautifierType voiceBeautifierType);
    int32_t GetEnvironmentType(OHAudioNode *node, OH_EnvironmentType *environmentType);
    int32_t GetSoundFieldType(OHAudioNode *node, OH_SoundFieldType *soundFieldType);
    int32_t GetEqualizerFrequencyBandGains(OHAudioNode *node,
        OH_EqualizerFrequencyBandGains *frequencyBandGains);
    int32_t GetVoiceBeautifierType(OHAudioNode *node,
        OH_VoiceBeautifierType *voiceBeautifierType);
    int32_t SetSpaceRenderPositionParams(OHAudioNode *node, OH_AudioSuite_SpaceRenderPositionParams position);
    int32_t GetSpaceRenderPositionParams(OHAudioNode* node, OH_AudioSuite_SpaceRenderPositionParams* position);
    int32_t SetSpaceRenderRotationParams(OHAudioNode* node, OH_AudioSuite_SpaceRenderRotationParams rotation);
    int32_t GetSpaceRenderRotationParams(OHAudioNode* node, OH_AudioSuite_SpaceRenderRotationParams* rotation);
    int32_t SetSpaceRenderExtensionParams(OHAudioNode* node, OH_AudioSuite_SpaceRenderExtensionParams extension);
    int32_t GetSpaceRenderExtensionParams(OHAudioNode* node, OH_AudioSuite_SpaceRenderExtensionParams* extension);
    int32_t SetTempoAndPitch(OHAudioNode* node, float speed, float pitch);
    int32_t GetTempoAndPitch(OHAudioNode* node, float* speed, float* pitch);
    int32_t SetPureVoiceChangeOption(OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption option);
    int32_t GetPureVoiceChangeOption(OHAudioNode* node, OH_AudioSuite_PureVoiceChangeOption* option);
    int32_t SetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType type);
    int32_t GetGeneralVoiceChangeType(OHAudioNode* node, OH_AudioSuite_GeneralVoiceChangeType* type);
    int32_t IsNodeTypeSupported(OH_AudioNode_Type nodeType, bool *isSupported);
    int32_t PrintInfo(OHAudioSuitePipeline *pipeline, int32_t fd);
    int32_t SetSystemNodeParam(OHAudioNode* node, std::vector<uint8_t> &param);
    int32_t GetSystemNodeParam(OHAudioNode* node, std::vector<uint8_t> &param);
    int32_t GetSystemNodeInOutSize(OHAudioNode* node, uint32_t &inSize, uint32_t &outSize);

private:
    explicit OHAudioSuiteEngine() {};
    OHAudioSuiteEngine(const OHAudioSuiteEngine&) = delete;
    OHAudioSuiteEngine& operator=(const OHAudioSuiteEngine&) = delete;

    void CleanupAll();
    int32_t InitInputCallback(std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline,
        OHAudioSuiteNodeBuilder *builder, uint32_t nodeId, OH_AudioNode *audioNode);
    std::shared_ptr<AudioSuite::AudioSuitePipeline> GetAudioSuitePipeline(OHAudioSuitePipeline *pipeline);
    int32_t GetAudioSuitePipelineAndNodeId(OHAudioNode* node, AudioNodeType expectedType,
        std::shared_ptr<AudioSuite::AudioSuitePipeline>& pipeline, uint32_t& nodeId);
    int32_t GetAudioSuitePipelineAndNodeId(OHAudioNode* node,
        std::shared_ptr<AudioSuite::AudioSuitePipeline>& pipeline, uint32_t& nodeId, AudioNodeType& nodeType);
    int32_t LookupPipelineAndNodeId(OHAudioNode* node,
        std::shared_ptr<AudioSuite::AudioSuitePipeline>& pipeline, uint32_t& nodeId, AudioNodeType& nodeType);

    void AddPipeline(OHAudioSuitePipeline *pipeline);
    void RemovePipeline(OHAudioSuitePipeline *pipeline);

    void AddNode(OHAudioNode *node);
    void RemoveNode(OHAudioNode *node);
    void RemoveNodes(std::vector<OHAudioNode*> &nodes);

    std::mutex pipelinesMutex_;
    std::unordered_set<OHAudioSuitePipeline*> pipelines_;

    std::mutex nodesMutex_;
    std::unordered_set<OHAudioNode*> nodes_;

    std::mutex engineMutex_;
    bool isInitialized_ = false;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SUITE_ENGINE_H