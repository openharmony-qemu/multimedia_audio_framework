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
#ifndef IAUDIO_SUITE_ENGINE_H
#define IAUDIO_SUITE_ENGINE_H

#include <atomic>
#include <memory>
#include <functional>
#include <unordered_map>
#include "hpae_no_lock_queue.h"
#include "audio_suite_manager.h"
#include "audio_suite_manager_thread.h"
#include "audio_suite_manager_callback.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using OHOS::AudioStandard::HPAE::HpaeNoLockQueue;
using OHOS::AudioStandard::HPAE::Request;

class IAudioSuiteEngine {
public:
    virtual ~IAudioSuiteEngine() = default;

    virtual int32_t Init() = 0;
    virtual int32_t DeInit() = 0;
    virtual int32_t CreatePipeline(PipelineWorkMode workMode) = 0;
    virtual int32_t DestroyPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StartPipeline(uint32_t pipelineId) = 0;
    virtual int32_t StopPipeline(uint32_t pipelineId) = 0;
    virtual int32_t GetPipelineState(uint32_t pipelineId) = 0;

    virtual int32_t CreateNode(uint32_t pipelineId, AudioNodeBuilder &builder) = 0;
    virtual int32_t DestroyNode(uint32_t nodeId) = 0;
    virtual int32_t BypassEffectNode(uint32_t nodeId, bool bypass) = 0;
    virtual int32_t GetNodeBypassStatus(uint32_t nodeId) = 0;
    virtual int32_t SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat) = 0;
    virtual int32_t SetRequestDataCallback(uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback) = 0;
    virtual int32_t ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) = 0;
    virtual int32_t DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId) = 0;
    virtual int32_t RenderFrame(uint32_t pipelineId,
        uint8_t *audioData,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag) = 0;
    virtual int32_t MultiRenderFrame(uint32_t pipelineId,
        AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag) = 0;
    virtual int32_t SetOptions(uint32_t nodeId, std::string name, std::string value) = 0;
    virtual int32_t GetOptions(uint32_t nodeId, std::string name, std::string &value) = 0;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // IAUDIO_SUITE_ENGINE_H