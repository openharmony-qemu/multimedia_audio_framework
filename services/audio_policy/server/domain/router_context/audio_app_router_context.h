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

#ifndef AUDIO_APP_ROUTER_CONTEXT_H
#define AUDIO_APP_ROUTER_CONTEXT_H

#include <vector>
#include <map>
#include <memory>
#include "audio_device_simple_descriptor.h"
#include "audio_select_device_info.h"
#include "audio_source_type.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

struct AudioAppRouterContext {
    bool isForeground_ = true;

    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> currentInputDevices_;
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> currentOutputDevices_;

    AudioAppSelectDevice appSelectDevice_;

    std::map<uint32_t, SourceType> inputStreams_;
    std::map<uint32_t, StreamUsage> outputStreams_;

    std::map<uint32_t, std::shared_ptr<AudioDeviceSimpleDescriptor>> streamSelectDevices_;
    std::map<uint32_t, std::shared_ptr<AudioDeviceSimpleDescriptor>> mediaDefaultDevices_;
    std::map<uint32_t, std::shared_ptr<AudioDeviceSimpleDescriptor>> callDefaultDevices_;

    AudioAppRouterContext() = default;
    AudioAppRouterContext(int32_t pid, int32_t uid, bool isForeground)
        : isForeground_(isForeground) {}
    ~AudioAppRouterContext() = default;
};

}
}
#endif
