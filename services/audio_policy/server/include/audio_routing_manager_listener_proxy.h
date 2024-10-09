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

#ifndef AUDIO_ROUTING_MANAGER_LISTENER_PROXY_H
#define AUDIO_ROUTING_MANAGER_LISTENER_PROXY_H

#include "audio_routing_manager.h"
#include "i_standard_audio_routing_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioRoutingManagerListenerProxy : public IRemoteProxy<IStandardAudioRoutingManagerListener> {
public:
    explicit AudioRoutingManagerListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioRoutingManagerListenerProxy();
    DISALLOW_COPY_AND_MOVE(AudioRoutingManagerListenerProxy);
    void OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> desciptor, const CastType type) override;
    int32_t OnAudioOutputDeviceRefined(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, StreamUsage streamUsage, int32_t clientUid, AudioPipeType audioPipeType) override;
    int32_t OnAudioInputDeviceRefined(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType, int32_t clientUid, AudioPipeType audioPipeType) override;

private:
    static inline BrokerDelegator<AudioRoutingManagerListenerProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RINGERMODE_UPDATE_LISTENER_PROXY_H
