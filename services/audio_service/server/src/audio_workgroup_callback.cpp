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
#ifndef LOG_TAG
#define LOG_TAG "AudioWorkgroupCallbackProxy"
#endif

#include "audio_workgroup_callback.h"
#include "audio_system_manager.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
AudioWorkgroupCallback::AudioWorkgroupCallback(const sptr<IAudioWorkgroupCallback>& listener)
    : listener_(listener)
{
}

AudioWorkgroupCallback::~AudioWorkgroupCallback()
{
}

void AudioWorkgroupCallback::OnWorkgroupChange(const AudioWorkgroupChangeInfo &info)
{
    if (listener_ != nullptr) {
        AudioWorkgroupChangeInfoIpc ipcInfo = {};
        ipcInfo.changeInfo = info;
        listener_->OnWorkgroupChange(ipcInfo);
    }
}
} // namespace AudioStandard
} // namespace OHOS
