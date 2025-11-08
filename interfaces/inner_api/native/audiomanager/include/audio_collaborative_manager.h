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

class AudioCollaborationEnabledChangeForCurrentDeviceCallback {
public:
    virtual ~AudioCollaborationEnabledChangeForCurrentDeviceCallback() = default;
    /**
     * @brief AudioCollaborationEnabledChangeForCurrentDeviceCallback will be executed
     *  when collboration enabled state changes
     *
     * @param enabled the collboration enabled state for current device.
     * @since 20
     */
    virtual void OnCollaborationEnabledChangeForCurrentDevice(const bool &enabled) {}
};

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
     * @brief Set the collboration enabled or disabled by the specified device.
     *
     * @return Returns success or not
     * @since 20
     */
    int32_t SetCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled);

    /**
     * @brief Check whether the collaborative is supported for some device
     *
     * @return Returns <b>true</b> if the collboration is supported; returns <b>false</b> otherwise.
     * @since 20
     */
    bool IsCollaborativePlaybackEnabledForDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);

    /**
     * @brief Register the collboration enabled change for current device callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t RegisterCollaborationEnabledForCurrentDeviceEventListener(
        const std::shared_ptr<AudioCollaborationEnabledChangeForCurrentDeviceCallback> &callback);

    /**
     * @brief Unregister the collboration enabled change for current device callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnregisterCollaborationEnabledForCurrentDeviceEventListener();
private:
    AudioCollaborativeManager();
    virtual ~AudioCollaborativeManager();
};

} // AudioStandard
} // OHOS
#endif
