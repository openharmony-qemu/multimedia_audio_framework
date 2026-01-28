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
#include "hpae_audio_service_callback_unit_test.h"
namespace OHOS {
namespace AudioStandard {
HpaeAudioServiceCallbackUnitTest::~HpaeAudioServiceCallbackUnitTest()
{}
 
void HpaeAudioServiceCallbackUnitTest::OnOpenAudioPortCb(int32_t portId)
{
    portId_ = portId;
}
 
void HpaeAudioServiceCallbackUnitTest::OnCloseAudioPortCb(int32_t result)
{
    closeAudioPortResult_ = result;
}

void HpaeAudioServiceCallbackUnitTest::OnReloadAudioPortCb(int32_t portId)
{
    portId_ = portId;
}
 
void HpaeAudioServiceCallbackUnitTest::OnSetSinkMuteCb(int32_t result)
{
    setSinkMuteResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs)
{
    getAllSinkInputsResult_ = result;
    sinkInputs_ = sinkInputs;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs)
{
    getAllSourceOutputsResult_ = result;
    sourceOutputs_ = sourceOutputs;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks)
{
    getAllSinksResult_ = result;
    sinks_ = sinks;
}
 
void HpaeAudioServiceCallbackUnitTest::OnMoveSinkInputByIndexOrNameCb(int32_t result)
{
    moveSinkInputByIndexOrNameResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnMoveSourceOutputByIndexOrNameCb(int32_t result)
{
    moveSourceOutputByIndexOrNameResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnSetSourceOutputMuteCb(int32_t result)
{
    setSourceOutputMuteResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAudioEffectPropertyCbV3(int32_t result)
{
    getAudioEffectPropertyResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAudioEffectPropertyCb(int32_t result)
{
    getAudioEffectPropertyResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAudioEnhancePropertyCbV3(int32_t result)
{
    getAudioEnhancePropertyResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::OnGetAudioEnhancePropertyCb(int32_t result)
{
    getAudioEnhancePropertyResult_ = result;
}
 
void HpaeAudioServiceCallbackUnitTest::HandleSourceAudioStreamRemoved(uint32_t sessionId)
{}

int32_t HpaeAudioServiceCallbackUnitTest::GetPortId() const noexcept
{
    return portId_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetCloseAudioPortResult() const noexcept
{
    return closeAudioPortResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetSetSinkMuteResult() const noexcept
{
    return setSinkMuteResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetGetAllSinkInputsResult() const noexcept
{
    return getAllSinkInputsResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetGetAllSourceOutputsResult() const noexcept
{
    return getAllSourceOutputsResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetGetAllSinksResult() const noexcept
{
    return getAllSinksResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetMoveSinkInputByIndexOrNameResult() const noexcept
{
    return moveSinkInputByIndexOrNameResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetMoveSourceOutputByIndexOrNameResult() const noexcept
{
    return moveSourceOutputByIndexOrNameResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetSetSourceOutputMuteResult() const noexcept
{
    return setSourceOutputMuteResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetGetAudioEffectPropertyResult() const noexcept
{
    return getAudioEffectPropertyResult_;
}
 
int32_t HpaeAudioServiceCallbackUnitTest::GetGetAudioEnhancePropertyResult() const noexcept
{
    return getAudioEnhancePropertyResult_;
}
 
std::vector<SinkInput> HpaeAudioServiceCallbackUnitTest::GetSinkInputs() const noexcept
{
    return sinkInputs_;
}
 
std::vector<SourceOutput> HpaeAudioServiceCallbackUnitTest::GetSourceOutputs() const noexcept
{
    return sourceOutputs_;
}
 
std::vector<SinkInfo> HpaeAudioServiceCallbackUnitTest::GetSinks() const noexcept
{
    return sinks_;
}
 
}  // namespace AudioStandard
}  // namespace OHOS