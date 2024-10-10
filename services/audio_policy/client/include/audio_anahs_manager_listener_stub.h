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

#ifndef AUDIO_ANAHS_MANAGER_LISTENER_STUB_H
#define AUDIO_ANAHS_MANAGER_LISTENER_STUB_H

#include "audio_anahs_manager.h"
#include "i_standard_audio_anahs_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioAnahsManagerListenerStub : public IRemoteStub<IStandardAudioAnahsManagerListener> {
public:
    AudioAnahsManagerListenerStub();
    virtual ~AudioAnahsManagerListenerStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
    void SetAudioDeviceAnahsCallback(const std::weak_ptr<AudioDeviceAnahs> &callback);
    int32_t OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType) override;
private:
    void OnExtPnpDeviceStatusChangedInternal(MessageParcel &data, MessageParcel &reply);
    std::weak_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RINGERMODE_UPDATE_LISTENER_STUB_H
