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
#ifndef HPAE_MANAGER_FUZZER_H
#define HPAE_MANAGER_FUZZER_H

#include "audio_info.h"
#include "audio_module_info.h"
#include "hpae_define.h"
#include "hpae_info.h"
#include "hpae_manager.h"
#include "i_stream.h"
#include "i_renderer_stream.h"
#include "i_capturer_stream.h"
#include "audio_effect.h"
#include "audio_spatialization_types.h"
#include "audio_service_hpae_callback.h"
#include <memory>
#include <vector>
#include <string>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

using namespace std;

// Fuzzer-local no-op stub for AudioServiceHpaeCallback. Some HpaeManager APIs dereference
// the callback (e.g. MoveToPreferSink calls serviceCallback->OnOpenAudioPortCb when no
// running sessions need moving), so passing a null shared_ptr crashes. Supply this valid
// stub instead. Mirrors the sibling-fuzzer stub pattern (StubEffectInstance / WriteFixedDataCb).
class FuzzHpaeServiceCallback : public AudioServiceHpaeCallback {
public:
    ~FuzzHpaeServiceCallback() override = default;
    void OnOpenAudioPortCb(int32_t portId) override {}
    void OnCloseAudioPortCb(int32_t result) override {}
    void OnReloadAudioPortCb(int32_t portId) override {}
    void OnSetSinkMuteCb(int32_t result) override {}
    void OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs) override {}
    void OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs) override {}
    void OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks) override {}
    void OnMoveSinkInputByIndexOrNameCb(int32_t result) override {}
    void OnMoveSourceOutputByIndexOrNameCb(int32_t result) override {}
    void OnSetSourceOutputMuteCb(int32_t result) override {}
    void OnGetAudioEffectPropertyCb(int32_t result) override {}
    void OnGetAudioEnhancePropertyCb(int32_t result) override {}
    void HandleSourceAudioStreamRemoved(uint32_t sessionId) override {}
    void OnGetCollaborativeUpdateCb(int32_t result) override {}
    void OnUpdateSpatializationStateCb(int32_t result) override {}
    void OnUpdatePersonalizedHRTFBinCb(int32_t result) override {}
    void OnUpdatePersonalizedSpatializationStateCb(int32_t result) override {}
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

#endif // HPAE_MANAGER_FUZZER_H
