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
#ifndef HPAE_MANAGER_HIGH_RISK_INTERFACE_FUZZER_H
#define HPAE_MANAGER_HIGH_RISK_INTERFACE_FUZZER_H

#include "audio_service_hpae_callback.h"
#include "audio_info.h"
#include "hpae_manager.h"
namespace OHOS {
namespace AudioStandard {
class HpaeAudioServiceCallbackFuzzTest : public OHOS::AudioStandard::AudioServiceHpaeCallback {
public:
    ~HpaeAudioServiceCallbackFuzzTest() override {}

    void OnOpenAudioPortCb(int32_t portId) override
    {
        portId_ = portId;
    }

    void OnCloseAudioPortCb(int32_t result) override
    {
        closeAudioPortResult_ = result;
    }

    void OnReloadAudioPortCb(int32_t portId) override
    {
        portId_ = portId;
    }
    
    void OnSetSinkMuteCb(int32_t result) override
    {
        setSinkMuteResult_ = result;
    }

    void OnGetAllSinkInputsCb(int32_t result, std::vector<OHOS::AudioStandard::SinkInput> &sinkInputs) override
    {
        getAllSinkInputsResult_ = result;
        sinkInputs_ = sinkInputs;
    }

    void OnGetAllSourceOutputsCb(int32_t result,
        std::vector<OHOS::AudioStandard::SourceOutput> &sourceOutputs) override
        {
        getAllSourceOutputsResult_ = result;
        sourceOutputs_ = sourceOutputs;
    }

    void OnGetAllSinksCb(int32_t result, std::vector<OHOS::AudioStandard::SinkInfo> &sinks) override
    {
        getAllSinksResult_ = result;
        sinks_ = sinks;
    }

    void OnMoveSinkInputByIndexOrNameCb(int32_t result) override
    {
        moveSinkInputByIndexOrNameResult_ = result;
    }

    void OnMoveSourceOutputByIndexOrNameCb(int32_t result) override
    {
        moveSourceOutputByIndexOrNameResult_ = result;
    }

    void OnSetSourceOutputMuteCb(int32_t result) override
    {
        setSourceOutputMuteResult_ = result;
    }

    void OnGetAudioEffectPropertyCbV3(int32_t result) override
    {
        getAudioEffectPropertyResult_ = result;
    }

    void OnGetAudioEffectPropertyCb(int32_t result) override
    {
        getAudioEffectPropertyResult_ = result;
    }

    void OnGetAudioEnhancePropertyCbV3(int32_t result) override
    {
        getAudioEnhancePropertyResult_ = result;
    }

    void OnGetAudioEnhancePropertyCb(int32_t result) override
    {
        getAudioEnhancePropertyResult_ = result;
    }

    void HandleSourceAudioStreamRemoved(uint32_t sessionId) override {}

    int32_t GetPortId() const noexcept
    {
        return portId_;
    }

    int32_t GetCloseAudioPortResult() const noexcept
    {
        return closeAudioPortResult_;
    }

    int32_t GetSetSinkMuteResult() const noexcept
    {
        return setSinkMuteResult_;
    }

    int32_t GetGetAllSinkInputsResult() const noexcept
    {
        return getAllSinkInputsResult_;
    }

    int32_t GetGetAllSourceOutputsResult() const noexcept
    {
        return getAllSourceOutputsResult_;
    }

    int32_t GetGetAllSinksResult() const noexcept
    {
        return getAllSinksResult_;
    }

    int32_t GetMoveSinkInputByIndexOrNameResult() const noexcept
    {
        return moveSinkInputByIndexOrNameResult_;
    }

    int32_t GetMoveSourceOutputByIndexOrNameResult() const noexcept
    {
        return moveSourceOutputByIndexOrNameResult_;
    }

    int32_t GetSetSourceOutputMuteResult() const noexcept
    {
        return setSourceOutputMuteResult_;
    }

    int32_t GetGetAudioEffectPropertyResult() const noexcept
    {
        return getAudioEffectPropertyResult_;
    }

    int32_t GetGetAudioEnhancePropertyResult() const noexcept
    {
        return getAudioEnhancePropertyResult_;
    }

    std::vector<OHOS::AudioStandard::SinkInput> GetSinkInputs() const noexcept
    {
        return sinkInputs_;
    }

    std::vector<OHOS::AudioStandard::SourceOutput> GetSourceOutputs() const noexcept
    {
        return sourceOutputs_;
    }

    std::vector<OHOS::AudioStandard::SinkInfo> GetSinks() const noexcept
    {
        return sinks_;
    }

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
    std::vector<OHOS::AudioStandard::SinkInput> sinkInputs_;
    std::vector<OHOS::AudioStandard::SourceOutput> sourceOutputs_;
    std::vector<OHOS::AudioStandard::SinkInfo> sinks_;
};

class HpaeManagerFuzzTest {
public:
    void StreamSetUp();
    void AudioPortSetUp();
    void TearDown();

    void HpaeRenderStreamManagerMoveFuzzTest();
    void HpaeCaptureStreamManagerMoveFuzzTest();
    void OpenAudioPortFuzzTest();
    void ReloadAudioPortFuzzTest();
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_;
    std::vector<std::string> sourceNameList_;
    std::vector<std::string> sinkNameList_;
    std::vector<std::string> audioPortNameList_;
    std::vector<std::string> libList_;
    std::vector<uint32_t> sourceOutputIdList_;
    std::vector<uint32_t> sinkInputIdList_;
    std::vector<uint32_t> renderSessionIdList;
    std::vector<uint32_t> captureSessionIdList;

    int32_t sinkPortId_ = -1;
    int32_t sourcePortId_ = -1;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_MANAGER_HIGH_RISK_INTERFACE_FUZZER_H