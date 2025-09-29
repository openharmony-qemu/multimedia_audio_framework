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

class SuiteInputNodeWriteDataCallBack {
public:
    virtual ~SuiteInputNodeWriteDataCallBack() = default;
    virtual int32_t OnWriteDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) = 0;
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
    virtual int32_t CreatePipeline(uint32_t &pipelineId) = 0;
    virtual int32_t DestroyPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StartPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StopPipeline(uint32_t pipelineId) = 0;
    virtual int32_t GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state) = 0;
    virtual int32_t CreateNode(uint32_t pipelineId, AudioNodeBuilder &builder, uint32_t &nodeId) = 0;
    virtual int32_t DestroyNode(uint32_t nodeId) = 0;
    virtual int32_t EnableNode(uint32_t nodeId, AudioNodeEnable audioNodeEnable) = 0;
    virtual int32_t GetNodeEnableStatus(uint32_t nodeId, AudioNodeEnable &nodeEnable) = 0;
    virtual int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) = 0;
    virtual int32_t SetOnWriteDataCallback(uint32_t nodeId,
        std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback) = 0;
    virtual int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId,
        AudioNodePortType srcPortType, AudioNodePortType destPortType) = 0;
    virtual int32_t DisConnectNodes(uint32_t srcNode, uint32_t destNode) = 0;
    virtual int32_t SetEqualizerMode(uint32_t nodeId, EqualizerMode eqMode);
    virtual int32_t SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains);
    virtual int32_t SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType);
    virtual int32_t SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType);
    virtual int32_t SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType);
    virtual int32_t InstallTap(uint32_t nodeId, AudioNodePortType portType,
        std::shared_ptr<SuiteNodeReadTapDataCallback> callback) = 0;
    virtual int32_t RemoveTap(uint32_t nodeId, AudioNodePortType portType) = 0;
    virtual int32_t RenderFrame(
        uint32_t pipelineId, uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag) = 0;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SUITE_MANAGER_H