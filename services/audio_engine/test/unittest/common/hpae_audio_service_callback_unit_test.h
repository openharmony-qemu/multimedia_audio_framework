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
#ifndef HPAE_AUDIO_AUDIO_SERVICE_CALLBACK_UNIT_TEST_H
#define HPAE_AUDIO_AUDIO_SERVICE_CALLBACK_UNIT_TEST_H
#include "audio_service_hpae_callback.h"
 
namespace OHOS {
namespace AudioStandard {
class HpaeAudioServiceCallbackUnitTest : public AudioServiceHpaeCallback {
public:
    ~HpaeAudioServiceCallbackUnitTest() override;
    
    void OnOpenAudioPortCb(int32_t portId) override;
 
    void OnCloseAudioPortCb(int32_t result) override;

    void OnReloadAudioPortCb(int32_t portId) override;
 
    void OnSetSinkMuteCb(int32_t result) override;
 
    void OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs) override;
 
    void OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs) override;
 
    void OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks) override;
 
    void OnMoveSinkInputByIndexOrNameCb(int32_t result) override;
 
    void OnMoveSourceOutputByIndexOrNameCb(int32_t result) override;
 
    void OnSetSourceOutputMuteCb(int32_t result) override;
 
    void OnGetAudioEffectPropertyCbV3(int32_t result) override;
 
    void OnGetAudioEffectPropertyCb(int32_t result) override;
 
    void OnGetAudioEnhancePropertyCbV3(int32_t result) override;
 
    void OnGetAudioEnhancePropertyCb(int32_t result) override;

    void HandleSourceAudioStreamRemoved(uint32_t sessionId) override;
 
    int32_t GetPortId() const noexcept;
 
    int32_t GetCloseAudioPortResult() const noexcept;
 
    int32_t GetSetSinkMuteResult() const noexcept;
 
    int32_t GetGetAllSinkInputsResult() const noexcept;
 
    int32_t GetGetAllSourceOutputsResult() const noexcept;
 
    int32_t GetGetAllSinksResult() const noexcept;
 
    int32_t GetMoveSinkInputByIndexOrNameResult() const noexcept;
 
    int32_t GetMoveSourceOutputByIndexOrNameResult() const noexcept;
 
    int32_t GetSetSourceOutputMuteResult() const noexcept;
 
    int32_t GetGetAudioEffectPropertyResult() const noexcept;
 
    int32_t GetGetAudioEnhancePropertyResult() const noexcept;
 
    std::vector<SinkInput> GetSinkInputs() const noexcept;
 
    std::vector<SourceOutput> GetSourceOutputs() const noexcept;
 
    std::vector<SinkInfo> GetSinks() const noexcept;
 
private:
    int32_t portId_ = -1;
    int32_t closeAudioPortResult_ = -1;
    int32_t setSinkMuteResult_ = -1;
    int32_t getAllSinkInputsResult_ = -1;
    int32_t getAllSourceOutputsResult_ = -1;
    int32_t getAllSinksResult_ = -1;
    int32_t moveSinkInputByIndexOrNameResult_ = -1;
    int32_t moveSourceOutputByIndexOrNameResult_ = -1;
    int32_t setSourceOutputMuteResult_ = -1;
    int32_t getAudioEffectPropertyResult_ = -1;
    int32_t getAudioEnhancePropertyResult_ = -1;
    std::vector<SinkInput> sinkInputs_;
    std::vector<SourceOutput> sourceOutputs_;
    std::vector<SinkInfo> sinks_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif