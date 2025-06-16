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
#ifndef ST_AUDIO_COLLABORATIVE_MANAGER_H
#define ST_AUDIO_COLLABORATIVE_MANAGER_H
#include <cstdint>
#include <memory>
#include "parcel.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {

class AudioCollaborativeManager {
public:
    static AudioCollaborativeManager *GetInstance();

    /**
     * @brief Check whether the collaborative is supported for local device
     *
     * @return Returns <b>true</b> if the collaborative is successfully enabled; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsCollaborativePlaybackSupported();

    /**
     * @brief Set the spatialization enabled or disabled by the specified device.
     *
     * @return Returns success or not
     * @since 20
     */
    int32_t SetCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled);

    /**
     * @brief Check whether the collaborative is supported for some device
     *
     * @return Returns <b>true</b> if the spatialization is supported; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsCollaborativePlaybackEnabledForDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
private:
    AudioCollaborativeManager();
    virtual ~AudioCollaborativeManager();
};

} // AudioStandard
} // OHOS
#endif
