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
#ifndef HPAE_MANAGER_STREAM_FUZZER_H
#define HPAE_MANAGER_STREAM_FUZZER_H

#include "audio_service_hpae_callback.h"
#include "audio_service_hpae_dump_callback.h"
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

    // Store the result only. These callbacks are registered on a SetUp-scoped
    // object: copying engine-built vectors into members makes them outlive the
    // engine call and get destroyed in the fuzzer TU at SetUp return, where any
    // unrelated in-window heap write detonates in the vector dtor. The vectors
    // are never read by this fuzzer.
    void OnGetAllSinkInputsCb(int32_t result, std::vector<OHOS::AudioStandard::SinkInput> &) override
    {
        getAllSinkInputsResult_ = result;
    }

    void OnGetAllSourceOutputsCb(int32_t result,
        std::vector<OHOS::AudioStandard::SourceOutput> &) override
    {
        getAllSourceOutputsResult_ = result;
    }

    void OnGetAllSinksCb(int32_t result, std::vector<OHOS::AudioStandard::SinkInfo> &) override
    {
        getAllSinksResult_ = result;
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

    void OnGetAudioEffectPropertyCb(int32_t result) override
    {
        getAudioEffectPropertyResult_ = result;
    }

    void OnGetAudioEnhancePropertyCb(int32_t result) override
    {
        getAudioEnhancePropertyResult_ = result;
    }

    void OnGetCollaborativeUpdateCb(int32_t result) override
    {
        getCollaborativeUpdateResult_ = result;
    }

    void OnUpdateSpatializationStateCb(int32_t result) override
    {
        updateSpatializationStateResult_ = result;
    }

    void OnUpdatePersonalizedHRTFBinCb(int32_t result) override
    {
        updatePersonalizedHRTFBinResult_ = result;
    }

    void OnUpdatePersonalizedSpatializationStateCb(int32_t result) override
    {
        updatePersonalizedSpatializationStateResult_ = result;
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

    int32_t GetCollaborativeUpdateResult() const noexcept
    {
        return getCollaborativeUpdateResult_;
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
    int32_t getCollaborativeUpdateResult_ = -1;
    int32_t updateSpatializationStateResult_ = -1;
    int32_t updatePersonalizedHRTFBinResult_ = -1;
    int32_t updatePersonalizedSpatializationStateResult_ = -1;
};

// Stores NOTHING. The previous SetUp-scoped dump callback swapped engine-built
// std::vector<HpaeInputOutputInfo> buffers into its members and destroyed them
// at StreamSetUp return; an unrelated heap write during the SetUp tail then
// detonated in the fuzzer TU's vector dtor (fuzz CI SIGSEGV inside free()).
// The data is never read by this fuzzer, so keep the overrides empty and let
// the engine-side vectors die inside the engine right after the callback.
class HpaeAudioServiceDumpCallbackFuzzTest : public AudioServiceHpaeDumpCallback {
public:
    ~HpaeAudioServiceDumpCallbackFuzzTest() override = default;

    void OnDumpSinkInfoCb(std::string &, int32_t) override { }
    void OnDumpSourceInfoCb(std::string &, int32_t) override { }
    void OnDumpAllAvailableDeviceCb(int32_t, HpaeDeviceInfo &&) override { }
    void OnDumpSinkInputsInfoCb(std::vector<HpaeInputOutputInfo> &, int32_t) override { }
    void OnDumpSourceOutputsInfoCb(std::vector<HpaeInputOutputInfo> &, int32_t) override { }
};

class HpaeManagerFuzzTest {
public:
    void StreamSetUp();
    void TearDown();
    void Fisrt();
    void InitFunc();
    void InitStreamFunc();

    void DumpFuzzTest();
    void StreamManagerFuzzTest();
    void MoveStreamManagerFuzzTest();
    void HpaeManagerEffectTest();
    void HpaeManagerEffectTest2();
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_;
    std::vector<std::string> sourceNameList_;
    std::vector<std::string> sinkNameList_;
    std::vector<std::string> audioPortNameList_;
    std::vector<std::string> libList_;
    std::vector<uint32_t> sourceOutputIdList_;
    std::vector<uint32_t> sinkInputIdList_;
    std::vector<uint32_t> renderSessionIdList;
    std::vector<uint32_t> captureSessionIdList;
    std::vector<uint32_t> sessionIdList_;
    std::vector<std::function<void()>> renderStreamFunc_;
    std::vector<std::function<void()>> capturerStreamFunc_;
    std::vector<std::function<void()>> moveStreamFunc_;
    std::vector<std::function<void()>> errorStreamFunc_;
    std::vector<std::function<void()>> effectFunc_;
    HPAE::HpaeStreamInfo rendererStreamInfo_;
    HPAE::HpaeStreamInfo streamInfo_;
    HPAE::HpaeStreamInfo capStreamInfo_;
    int32_t sinkPortId_ = -1;
    int32_t sinkPortId2_ = -1;
    int32_t sourcePortId_ = -1;
    int32_t sourcePortId2_ = -1;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_MANAGER_STREAM_FUZZER_H