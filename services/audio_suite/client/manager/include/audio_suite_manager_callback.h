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

#ifndef AUDIO_SUITE_ENGINE_CALLBACK_H
#define AUDIO_SUITE_ENGINE_CALLBACK_H

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteManagerCallback {
public:
    virtual void OnCreatePipeline(int32_t result, uint32_t pipelineId) = 0;
    virtual void OnDestroyPipeline(int32_t result) = 0;
    virtual void OnStartPipeline(int32_t result) = 0;
    virtual void OnStopPipeline(int32_t result) = 0;
    virtual void OnGetPipelineState(AudioSuitePipelineState state) = 0;
    virtual void OnCreateNode(int32_t result, uint32_t nodeId) = 0;
    virtual void OnDestroyNode(int32_t result) = 0;
    virtual void OnBypassEffectNode(int32_t result) = 0;
    virtual void OnGetNodeBypass(int32_t result, bool bypassStatus) = 0;
    virtual void OnSetAudioFormat(int32_t result) = 0;
    virtual void OnWriteDataCallback(int32_t result) = 0;
    virtual void OnConnectNodes(int32_t result) = 0;
    virtual void OnDisConnectNodes(int32_t result) = 0;
    virtual void OnRenderFrame(int32_t result, uint32_t pipelineId) = 0;
    virtual void OnMultiRenderFrame(int32_t result, uint32_t pipelineId) = 0;
    virtual void OnGetOptions(int32_t result) = 0;

    virtual ~AudioSuiteManagerCallback() = default;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SUITE_ENGINE_CALLBACK_H