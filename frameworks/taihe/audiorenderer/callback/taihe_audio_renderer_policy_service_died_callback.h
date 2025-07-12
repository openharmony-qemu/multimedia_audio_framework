/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_RENDERER_POLICY_SERVICE_DIED_CALLBACK_H
#define TAIHE_AUDIO_RENDERER_POLICY_SERVICE_DIED_CALLBACK_H

#include "taihe_audio_renderer.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
class TaiheAudioRendererPolicyServiceDiedCallback : public OHOS::AudioStandard::AudioRendererPolicyServiceDiedCallback {
public:
    explicit TaiheAudioRendererPolicyServiceDiedCallback(AudioRendererImpl *renderer);
    virtual ~TaiheAudioRendererPolicyServiceDiedCallback();
    void OnAudioPolicyServiceDied() override;
private:
    std::mutex mutex_;
    AudioRendererImpl *renderer_;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RENDERER_POLICY_SERVICE_DIED_CALLBACK_H