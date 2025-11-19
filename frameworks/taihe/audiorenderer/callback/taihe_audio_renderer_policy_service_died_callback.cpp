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
#ifndef LOG_TAG
#define LOG_TAG "TaiheAudioRendererPolicyServiceDiedCallback"
#endif

#include "taihe_audio_renderer_policy_service_died_callback.h"
#include "taihe_audio_renderer_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioRendererPolicyServiceDiedCallback::TaiheAudioRendererPolicyServiceDiedCallback(AudioRendererImpl *renderer)
    : renderer_(renderer)
{
    AUDIO_INFO_LOG("instance create");
}

TaiheAudioRendererPolicyServiceDiedCallback::~TaiheAudioRendererPolicyServiceDiedCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

void TaiheAudioRendererPolicyServiceDiedCallback::OnAudioPolicyServiceDied()
{
    CHECK_AND_RETURN_LOG(renderer_ != nullptr, "renderer_ is null");
    renderer_->DestroyCallbacks();
    AUDIO_INFO_LOG("AudioRendererTaihe::UnegisterRendererDeviceChangeCallback is successful");
}
} // namespace ANI::Audio
