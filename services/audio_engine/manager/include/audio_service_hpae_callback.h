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

#ifndef AUDIO_SERVICE_HPAE_CALLBACK_H
#define AUDIO_SERVICE_HPAE_CALLBACK_H
#include "audio_effect.h"
namespace OHOS {
namespace AudioStandard {
class AudioServiceHpaeCallback {
public:
    virtual void OnOpenAudioPortCb(int32_t portId) = 0;

    virtual void OnCloseAudioPortCb(int32_t result) = 0;
    
    virtual void OnReloadAudioPortCb(int32_t portId) = 0;

    virtual void OnSetSinkMuteCb(int32_t result) = 0;

    virtual void OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs) = 0;

    virtual void OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs) = 0;

    virtual void OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks) = 0;

    virtual void OnMoveSinkInputByIndexOrNameCb(int32_t result) = 0;

    virtual void OnMoveSourceOutputByIndexOrNameCb(int32_t result) = 0;
    
    virtual void OnSetSourceOutputMuteCb(int32_t result) = 0;

    virtual void OnGetAudioEffectPropertyCbV3(int32_t result) = 0;

    virtual void OnGetAudioEffectPropertyCb(int32_t result) = 0;

    virtual void OnGetAudioEnhancePropertyCbV3(int32_t result) = 0;

    virtual void OnGetAudioEnhancePropertyCb(int32_t result) = 0;

    virtual void HandleSourceAudioStreamRemoved(uint32_t sessionId) = 0;

    virtual ~AudioServiceHpaeCallback()
    {}
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SERVICE_HPAE_CALLBACK_H