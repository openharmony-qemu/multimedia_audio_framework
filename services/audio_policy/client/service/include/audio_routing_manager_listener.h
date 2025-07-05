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

#ifndef AUDIO_ROUTING_MANAGER_LISTENER_H
#define AUDIO_ROUTING_MANAGER_LISTENER_H

#include "audio_routing_manager.h"
#include "standard_audio_routing_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioRoutingManagerListener : public StandardAudioRoutingManagerListenerStub {
public:
    AudioRoutingManagerListener();
    virtual ~AudioRoutingManagerListener();

    int32_t OnDistributedRoutingRoleChange(
        const std::shared_ptr<AudioDeviceDescriptor> &descriptor, int32_t type) override;
    void SetDistributedRoutingRoleCallback(const std::weak_ptr<AudioDistributedRoutingRoleCallback> &callback);
    void SetAudioDeviceRefinerCallback(const std::weak_ptr<AudioDeviceRefiner> &callback);
    int32_t OnAudioOutputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        int32_t routerType, int32_t streamUsage, int32_t clientUid, int32_t audioPipeType) override;
    int32_t OnAudioInputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        int32_t routerType, int32_t sourceType, int32_t clientUid, int32_t audioPipeType) override;
    int32_t GetSplitInfoRefined(std::string &splitInfo) override;
    int32_t OnDistributedOutputChange(bool isRemote) override;
private:
    std::mutex deviceRefinerCallbackMutex_;
    std::weak_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback_;
    std::weak_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ROUTING_MANAGER_LISTENER_H
