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

#ifndef AUDIO_ROUTER_SELECTOR_H
#define AUDIO_ROUTER_SELECTOR_H

#include <memory>
#include <vector>
#include "audio_device_descriptor.h"
#include "audio_device_info.h"
#include "audio_stream_info.h"
#include "audio_source_type.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterSelector {
public:
    static AudioRouterSelector& GetInstance()
    {
        static AudioRouterSelector instance;
        return instance;
    }

    int32_t SelectInputDevice(std::shared_ptr<AudioDeviceDescriptor> selectedDesc);

private:
    AudioRouterSelector() = default;
    ~AudioRouterSelector() = default;
    AudioRouterSelector(const AudioRouterSelector&) = delete;
    AudioRouterSelector& operator=(const AudioRouterSelector&) = delete;

    std::shared_ptr<AudioRouterSelectStrategy> GetSelectStrategy();
    int32_t DeviceParamsCheck(DeviceRole role, 
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& devices);
    int32_t RefreshVirtualDevice(std::shared_ptr<AudioDeviceDescriptor>& desc);
    int32_t SetPreferredDevice(AudioScene scene, SourceType sourceType,
        const std::shared_ptr<AudioDeviceDescriptor>& desc);
    void NotifyUserSelectionEventForInput(const std::shared_ptr<AudioDeviceDescriptor>& desc,
        SourceType sourceType);
    void WriteSelectInputSysEvents(const std::shared_ptr<AudioDeviceDescriptor>& desc,
        SourceType sourceType, AudioScene scene);
};

} 
} 
#endif 
