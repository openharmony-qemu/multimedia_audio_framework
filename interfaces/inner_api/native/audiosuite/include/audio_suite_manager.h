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
#ifndef AUDIO_SUITE_MANAGER_H
#define AUDIO_SUITE_MANAGER_H

#include <cstdint>
#include <memory>
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class InputNodeRequestDataCallBack {
public:
    virtual ~InputNodeRequestDataCallBack() = default;
    virtual int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) = 0;
};

class SuiteNodeReadTapDataCallback {
public:
    virtual ~SuiteNodeReadTapDataCallback() = default;
    virtual void OnReadTapDataCallback(void *audioData, int32_t audioDataSize) = 0;
};

class IAudioSuiteManager {
public:
    virtual ~IAudioSuiteManager() = default;

    static IAudioSuiteManager& GetAudioSuiteManager();

    virtual int32_t Init() = 0;
    virtual int32_t DeInit() = 0;
    virtual int32_t CreatePipeline(uint32_t &pipelineId, PipelineWorkMode workMode) = 0;
    virtual int32_t DestroyPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StartPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StopPipeline(uint32_t pipelineId) = 0;
    virtual int32_t GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state) = 0;
    virtual int32_t CreateNode(uint32_t pipelineId, AudioNodeBuilder &builder, uint32_t &nodeId) = 0;
    virtual int32_t DestroyNode(uint32_t nodeId) = 0;
    virtual int32_t BypassEffectNode(uint32_t nodeId, bool bypass) = 0;
    virtual int32_t GetNodeBypassStatus(uint32_t nodeId, bool &bypass) = 0;
    virtual int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) = 0;
    virtual int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) = 0;
    virtual int32_t GetSoundFiledType(uint32_t nodeId, SoundFieldType &soundFieldType) = 0;
    virtual int32_t GetEqualizerFrequencyBandGains(uint32_t nodeId,
        AudioEqualizerFrequencyBandGains &frequencyBandGains) = 0;
    virtual int32_t GetVoiceBeautifierType(uint32_t nodeId,
        VoiceBeautifierType &voiceBeautifierType) = 0;
    virtual int32_t GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType) = 0;
    virtual int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) = 0;
    virtual int32_t DisConnectNodes(uint32_t srcNode, uint32_t destNode) = 0;
    virtual int32_t SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains);
    virtual int32_t SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType);
    virtual int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType);
    virtual int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType);
    virtual int32_t RenderFrame(
        uint32_t pipelineId, uint8_t *audioData,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) = 0;
    virtual int32_t MultiRenderFrame(uint32_t pipelineId,
        AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag) = 0;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_H