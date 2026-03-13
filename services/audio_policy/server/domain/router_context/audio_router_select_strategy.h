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

#ifndef AUDIO_ROUTER_SELECT_STRATEGY_H
#define AUDIO_ROUTER_SELECT_STRATEGY_H

#include <memory>
#include "audio_device_descriptor.h"
#include "audio_device_simple_descriptor.h"
#include "audio_select_device_info.h"
#include "audio_source_type.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterSelectStrategy {
public:
    virtual ~AudioRouterSelectStrategy() = default;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaInputDevice(int32_t uid, uint32_t streamId, 
         SourceType sourceType) = 0;
    virtual void SetMediaInputDevice(int32_t uid, uint32_t streamId, 
         SourceType sourceType, const std::shared_ptr<AudioDeviceDescriptor>& device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetMediaOutputDevice(int32_t uid, uint32_t streamId, 
         StreamUsage streamUsage) = 0;
    virtual void SetMediaOutputDevice(int32_t uid, uint32_t streamId, 
         StreamUsage streamUsage, const std::shared_ptr<AudioDeviceDescriptor>& device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallInputDevice(int32_t uid, uint32_t streamId) = 0;
    virtual void SetCallInputDevice(int32_t uid, uint32_t streamId, 
         const std::shared_ptr<AudioDeviceDescriptor>& device) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetCallOutputDevice(int32_t uid, uint32_t streamId) = 0;
    virtual void SetCallOutputDevice(int32_t uid, uint32_t streamId, 
         const std::shared_ptr<AudioDeviceDescriptor>& device) = 0;

protected:
    std::shared_ptr<AudioDeviceDescriptor> ConvertSimpleToDevice(
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& simpleDesc);
    std::shared_ptr<AudioDeviceSimpleDescriptor> ConvertDeviceToSimple(
        const std::shared_ptr<AudioDeviceDescriptor>& deviceDesc);

    std::shared_ptr<AudioDeviceDescriptor> JudgeFinalSelectDevice(
        const std::shared_ptr<AudioDeviceDescriptor>& desc, SourceType sourceType,
        BluetoothAndNearlinkPreferredRecordCategory category);
    std::shared_ptr<AudioDeviceDescriptor> GetPreferDevice(
        BluetoothAndNearlinkPreferredRecordCategory category);

    std::shared_ptr<AudioDeviceDescriptor> GetFinalOutputDeviceForMedia(
        const std::shared_ptr<AudioDeviceDescriptor>& desc);
    std::shared_ptr<AudioDeviceDescriptor> GetFinalOutputDeviceForCall(
        const std::shared_ptr<AudioDeviceDescriptor>& desc);

};

} 
} 
#endif 
