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

#ifndef AUDIO_ANAHS_MANAGER_LISTENER_H
#define AUDIO_ANAHS_MANAGER_LISTENER_H

#include "audio_anahs_manager.h"
#include "standard_audio_anahs_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioAnahsManagerListener : public StandardAudioAnahsManagerListenerStub {
public:
    AudioAnahsManagerListener();
    virtual ~AudioAnahsManagerListener();

    void SetAudioDeviceAnahsCallback(const std::weak_ptr<AudioDeviceAnahs> &callback);
    int32_t OnExtPnpDeviceStatusChanged(const std::string &anahsStatus, const std::string &anahsShowType) override;
private:
    std::weak_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ANAHS_MANAGER_LISTENER_H
