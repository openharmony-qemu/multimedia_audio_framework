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

#ifndef AUDIO_DEVICE_CAPABILITY_H
#define AUDIO_DEVICE_CAPABILITY_H

#include <audio_device_stream_info.h>

#define REMOTE_DEFAULT_VOLUME 100

namespace OHOS {
namespace AudioStandard {
class RemoteDeviceCapability {

public:
    std::list<DeviceStreamInfo> streamInfoList_;
    bool isSupportRemoteVolume_ = false;
    int32_t initVolume_ = REMOTE_DEFAULT_VOLUME;
    bool initMuteStatus_ = false;

    std::string GetJsonString() const;
    void FromJsonString(const std::string &jsonString);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_CAPABILITY_H