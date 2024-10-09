/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioRendererPolicyServiceDiedCallback"
#endif

#include "napi_audio_renderer_policy_service_died_callback.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRendererPolicyServiceDiedCallback::NapiAudioRendererPolicyServiceDiedCallback(NapiAudioRenderer *rendererNapi)
    : rendererNapi_(rendererNapi)
{
    AUDIO_INFO_LOG("instance create");
}

NapiAudioRendererPolicyServiceDiedCallback::~NapiAudioRendererPolicyServiceDiedCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioRendererPolicyServiceDiedCallback::OnAudioPolicyServiceDied()
{
    rendererNapi_->DestroyCallbacks();
    AUDIO_INFO_LOG("AudioRendererNapi::UnegisterRendererDeviceChangeCallback is successful");
}
}  // namespace AudioStandard
}  // namespace OHOS
