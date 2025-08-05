/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_POLICY_MANAGER_LISTENER_STUB_IMPL_H
#define AUDIO_POLICY_MANAGER_LISTENER_STUB_IMPL_H

#include <thread>

#include "audio_system_manager.h"
#include "audio_interrupt_callback.h"
#include "audio_route_callback.h"
#include "standard_audio_policy_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyManagerListenerStubImpl : public StandardAudioPolicyManagerListenerStub {
public:
    AudioPolicyManagerListenerStubImpl();
    virtual ~AudioPolicyManagerListenerStubImpl();

    // IStandardAudioManagerListener override
    int32_t OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    int32_t OnRouteUpdate(uint32_t routeFlag, const std::string &networkId) override;
    int32_t OnAvailableDeviceChange(uint32_t usage, const DeviceChangeAction &deviceChangeAction) override;
    int32_t OnQueryClientType(const std::string &bundleName, uint32_t uid, bool &ret) override;
    int32_t OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid, bool &ret) override;
    int32_t OnCheckVKBInfo(const std::string &bundleName, bool &isValid) override;
    int32_t OnQueryAllowedPlayback(int32_t uid, int32_t pid, bool &ret) override;
    int32_t OnBackgroundMute(const int32_t uid) override;
    int32_t OnQueryBundleNameIsInList(const std::string &bundleName, const std::string &listType, bool &ret) override;
    int32_t OnQueryDeviceVolumeBehavior(VolumeBehavior &volumeBehavior) override;
    // AudioManagerListenerStubImpl
    void SetInterruptCallback(const std::weak_ptr<AudioInterruptCallback> &callback);
    void SetAudioRouteCallback(const std::weak_ptr<AudioRouteCallback> &callback);
    void SetAvailableDeviceChangeCallback(const std::weak_ptr<AudioManagerAvailableDeviceChangeCallback> &cb);
    void SetQueryClientTypeCallback(const std::weak_ptr<AudioQueryClientTypeCallback> &cb);
    void SetAudioClientInfoMgrCallback(const std::weak_ptr<AudioClientInfoMgrCallback> &cb);
    void SetAudioVKBInfoMgrCallback(const std::weak_ptr<AudioVKBInfoMgrCallback> &cb);
    void SetQueryAllowedPlaybackCallback(const std::weak_ptr<AudioQueryAllowedPlaybackCallback> &cb);
    void SetBackgroundMuteCallback(const std::weak_ptr<AudioBackgroundMuteCallback> &cb);
    void SetQueryBundleNameListCallback(const std::weak_ptr<AudioQueryBundleNameListCallback> &cb);
    void SetQueryDeviceVolumeBehaviorCallback(const std::weak_ptr<AudioQueryDeviceVolumeBehaviorCallback> &cb);

private:
    std::weak_ptr<AudioInterruptCallback> callback_;
    std::weak_ptr<AudioRouteCallback> audioRouteCallback_;
    std::weak_ptr<AudioManagerAvailableDeviceChangeCallback> audioAvailableDeviceChangeCallback_;
    std::weak_ptr<AudioQueryClientTypeCallback> audioQueryClientTypeCallback_;
    std::weak_ptr<AudioQueryAllowedPlaybackCallback> audioQueryAllowedPlaybackCallback_;
    std::weak_ptr<AudioBackgroundMuteCallback> audioBackgroundMuteCallback_;
    std::weak_ptr<AudioClientInfoMgrCallback> audioClientInfoMgrCallback_;
    std::weak_ptr<AudioVKBInfoMgrCallback> audioVKBInfoMgrCallback_;
    std::weak_ptr<AudioQueryBundleNameListCallback> audioQueryBundleNameListCallback_;
    std::weak_ptr<AudioQueryDeviceVolumeBehaviorCallback> audioQueryDeviceVolumeBehaviorCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_MANAGER_LISTENER_STUB_IMPL_H
