/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioAdapterInfo"
#endif

#include "audio_adapter_info.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
PipeInfo* AudioAdapterInfo::GetPipeByName(const std::string &pipeName)
{
    for (auto &pipeInfo : pipeInfos_) {
        if (pipeInfo.name_ == pipeName) {
            return &pipeInfo;
        }
    }
    return nullptr;
}

AudioPipeDeviceInfo* AudioAdapterInfo::GetDeviceInfoByDeviceType(DeviceType deviceType)
{
    for (auto &deviceInfo : deviceInfos_) {
        auto device = SUPPORTED_DEVICE_TYPE.find(deviceType);
        if (device != SUPPORTED_DEVICE_TYPE.end()) {
            return &deviceInfo;
        }
    }
    return nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
