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
#include <memory>
#include <vector>
#include <cstdint>
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class InputNodeRequestDataCallBack {
public:
    virtual ~InputNodeRequestDataCallBack() = default;
    virtual int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) = 0;
};

class InputNodeRequestMetaDataCallBack {
public:
    virtual ~InputNodeRequestMetaDataCallBack() = default;
    virtual int32_t OnRequestDataCallBack(AudioMetaFrame &metaFrame, int32_t* responseMetaDataSize, bool *finished) = 0;
};

class AudioSuitePipeline {
public:
    virtual ~AudioSuitePipeline() = default;

    static int32_t Create(PipelineWorkMode workMode, std::shared_ptr<AudioSuitePipeline> &pipeline);
    static int32_t IsNodeTypeSupported(AudioNodeType nodeType, bool &isSupported);

    virtual int32_t DeInit() = 0;
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t GetPipelineState(AudioSuitePipelineState &state) = 0;

    virtual int32_t CreateNode(AudioNodeBuilder builder, uint32_t &nodeId) = 0;
    virtual int32_t DestroyNode(uint32_t nodeId) = 0;
    virtual int32_t BypassEffectNode(uint32_t nodeId, bool bypass) = 0;
    virtual int32_t GetNodeBypassStatus(uint32_t nodeId, bool &bypass) = 0;
    virtual int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) = 0;
    virtual int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) = 0;
    virtual int32_t SetRequestMetaDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestMetaDataCallBack> callback) = 0;
    virtual int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) = 0;
    virtual int32_t DisconnectNodes(uint32_t srcNodeId, uint32_t destNodeId) = 0;
    virtual int32_t RenderFrame(
        uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) = 0;
    virtual int32_t MultiRenderFrame(
        uint8_t **audioDataArray, int32_t arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) = 0;
    virtual int32_t MetaRenderFrame(AudioMetaFrame &metaFrame,
        int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag) = 0;
    virtual int32_t SetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param) = 0;
    virtual int32_t GetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param) = 0;
    virtual int32_t GetSystemNodeInOutSize(uint32_t nodeId, uint32_t &inSize, uint32_t &outSize) = 0;

    // Typed parameter Set/Get (with validation)
    virtual int32_t SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains) = 0;
    virtual int32_t GetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains &gains) = 0;
    virtual int32_t SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType) = 0;
    virtual int32_t GetSoundFieldType(uint32_t nodeId, SoundFieldType &soundFieldType) = 0;
    virtual int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType) = 0;
    virtual int32_t GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType) = 0;
    virtual int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType) = 0;
    virtual int32_t GetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType &voiceBeautifierType) = 0;
    virtual int32_t SetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams position) = 0;
    virtual int32_t GetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams &position) = 0;
    virtual int32_t SetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams rotation) = 0;
    virtual int32_t GetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams &rotation) = 0;
    virtual int32_t SetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams extension) = 0;
    virtual int32_t GetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams &extension) = 0;
    virtual int32_t SetTempoAndPitch(uint32_t nodeId, float speed, float pitch) = 0;
    virtual int32_t GetTempoAndPitch(uint32_t nodeId, float &speed, float &pitch) = 0;
    virtual int32_t SetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption option) = 0;
    virtual int32_t GetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption &option) = 0;
    virtual int32_t SetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType type) = 0;
    virtual int32_t GetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType &type) = 0;

    virtual int32_t PrintInfo(int32_t fd) = 0;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_PIPELINE_H
