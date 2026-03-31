/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_ROUTER_FOLLOW_STRATEGY_H
#define AUDIO_ROUTER_FOLLOW_STRATEGY_H

#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterFollowStrategy : public AudioRouterSelectStrategy {
public:
    AudioRouterFollowStrategy() = default;
    ~AudioRouterFollowStrategy() override = default;

    std::shared_ptr<AudioDeviceDescriptor> GetMediaInputDevice(int32_t uid, uint32_t streamId,
        SourceType sourceType) override;
    void SetMediaInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) override;

    std::shared_ptr<AudioDeviceDescriptor> GetMediaOutputDevice(int32_t uid, uint32_t streamId) override;
    void SetMediaOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) override;

    std::shared_ptr<AudioDeviceDescriptor> GetCallInputDevice(int32_t uid, uint32_t streamId) override;
    void SetCallInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device) override;

    std::shared_ptr<AudioDeviceDescriptor> GetCallOutputDevice(int32_t uid, uint32_t streamId) override;
    void SetCallOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller = "") override;

private:
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetMediaInputDeviceWhenHasRunningStream(
        SourceType &effectiveSourceType, BluetoothAndNearlinkPreferredRecordCategory &category);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetMediaInputDeviceWhenNoRunningStream(
        int32_t uid, BluetoothAndNearlinkPreferredRecordCategory &category);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetCallOutputDeviceByUid(int32_t uid);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetCallLatestOutputDevice();
    void ClearCallOutputDevice(int32_t uid, uint32_t streamId, const std::string caller);
    void RefreshCallOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    bool HasVaildSelectDevice(AudioSelectDeviceInfo selectInfo);
};

}
}
#endif
