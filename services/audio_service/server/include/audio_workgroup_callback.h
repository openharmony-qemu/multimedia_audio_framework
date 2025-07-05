/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_WORKGROUP_CALLBACK_PROXY_H
#define AUDIO_WORKGROUP_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "audio_workgroup.h"
#include "audio_system_manager.h"
#include "iaudio_workgroup_callback.h"

namespace OHOS {
namespace AudioStandard {
class AudioWorkgroupCallback : public AudioWorkgroupCallbackForMonitor {
public:
    AudioWorkgroupCallback(const sptr<IAudioWorkgroupCallback>& listener);
    virtual ~AudioWorkgroupCallback();
    DISALLOW_COPY_AND_MOVE(AudioWorkgroupCallback);
    void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info) override;
private:
    sptr<IAudioWorkgroupCallback> listener_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_WORKGROUP_CALLBACK_PROXY_H