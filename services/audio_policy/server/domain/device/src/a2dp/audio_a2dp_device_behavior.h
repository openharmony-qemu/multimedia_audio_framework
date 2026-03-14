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

#ifndef AUDIO_A2DP_DEVICE_BEHAVIOR_H
#define AUDIO_A2DP_DEVICE_BEHAVIOR_H

#include "audio_device_info.h"
#include "audio_pipe_info.h"
#include "audio_stream_info.h"
#include "i_device_behavior_base.h"

namespace OHOS {
namespace AudioStandard {
struct A2dpReloadContext {
    DeviceType deviceType;
    const AudioStreamInfo &audioStreamInfo;
    std::string networkId;
    std::string sinkName;
    SourceType sourceType;
};

class AudioA2dpDeviceBehavior : public IDeviceBehaviorBase {
public:
    AudioA2dpDeviceBehavior() = default;
    virtual ~AudioA2dpDeviceBehavior() = default;

    int32_t ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason) override;
    int32_t DeactivateDevice() override;
    int32_t DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc) override;

private:
    int32_t ActivateA2dpDeviceWhenDescEnabled(std::shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t SwitchActiveA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    int32_t LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
        std::string networkId, std::string sinkName, SourceType sourceType);
    int32_t ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, const A2dpReloadContext &context);
    AudioIOHandle ReloadOrOpenAudioPort(int32_t engineFlag, AudioModuleInfo &moduleInfo,
        uint32_t &paIndex);
};
} // namespace AudioStandard
} // namespace OHOS

#endif
