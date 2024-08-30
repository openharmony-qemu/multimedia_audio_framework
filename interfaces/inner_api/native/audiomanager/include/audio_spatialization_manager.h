/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_SPATIALIZATION_MANAGER_H
#define ST_AUDIO_SPATIALIZATION_MANAGER_H

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"
#include "audio_effect.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationEnabledChangeCallback {
public:
    virtual ~AudioSpatializationEnabledChangeCallback() = default;
    /**
     * @brief AudioSpatializationEnabledChangeCallback will be executed when spatialization enabled state changes
     *
     * @param enabled the spatialization enabled state.
     * @since 11
     */
    virtual void OnSpatializationEnabledChange(const bool &enabled) = 0;

    /**
     * @brief AudioSpatializationEnabledChangeCallback will be executed when spatialization enabled state changes
     *
     * @param deviceDescriptor audio device description.
     * @param enabled the spatialization enabled state.
     * @since 12
     */
    virtual void OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) = 0;
};

class AudioHeadTrackingEnabledChangeCallback {
public:
    virtual ~AudioHeadTrackingEnabledChangeCallback() = default;
    /**
     * @brief AudioHeadTrackingEnabledChangeCallback will be executed when head tracking enabled state changes
     *
     * @param enabled the head tracking enabled state.
     * @since 11
     */
    virtual void OnHeadTrackingEnabledChange(const bool &enabled) = 0;

    /**
     * @brief AudioHeadTrackingEnabledChangeCallback will be executed when head tracking enabled state changes
     *
     * @param deviceDescriptor audio device description.
     * @param enabled the head tracking enabled state.
     * @since 12
     */
    virtual void OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) = 0;
};

class AudioSpatializationStateChangeCallback {
public:
    virtual ~AudioSpatializationStateChangeCallback() = default;
    /**
     * @brief AudioSpatializationStateChangeCallback will be executed when spatialization state changes
     *
     * @param enabled the spatialization state.
     * @since 11
     */
    virtual void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) = 0;
};

class HeadTrackingDataRequestedChangeCallback {
public:
    virtual ~HeadTrackingDataRequestedChangeCallback() = default;
    /**
     * @brief HeadTrackingDataRequestedChangeCallback will be executed when
     * whether head tracking data is requested changes
     *
     * @param isRequested whethet the head tracking data is requested.
     * @since 12
     */
    virtual void OnHeadTrackingDataRequestedChange(bool isRequested) = 0;
};

/**
 * @brief The AudioSpatializationManager class is an abstract definition of audio spatialization manager.
 *        Provides a series of client/interfaces for audio spatialization management
 */

class AudioSpatializationManager {
public:
    static AudioSpatializationManager *GetInstance();

    /**
     * @brief Check whether the spatialization is enabled
     *
     * @return Returns <b>true</b> if the spatialization is successfully enabled; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsSpatializationEnabled();

    /**
     * @brief Check whether the spatialization is enabled by the specified device.
     *
     * @return Returns <b>true</b> if the spatialization is successfully enabled; returns <b>false</b> otherwise.
     * @since 12
     */
    bool IsSpatializationEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice);

    /**
     * @brief Set the spatialization enabled or disabled
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetSpatializationEnabled(const bool enable);

    /**
     * @brief Set the spatialization enabled or disabled by the specified device.
     *
     * @return Returns success or not
     * @since 12
     */
    int32_t SetSpatializationEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable);

    /**
     * @brief Check whether the head tracking is enabled
     *
     * @return Returns <b>true</b> if the head tracking is successfully enabled; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsHeadTrackingEnabled();

    /**
     * @brief Check whether the head tracking is enabled by the specified device.
     *
     * @return Returns <b>true</b> if the head tracking is successfully enabled; returns <b>false</b> otherwise.
     * @since 12
     */
    bool IsHeadTrackingEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice);

    /**
     * @brief Set the head tracking enabled or disabled
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetHeadTrackingEnabled(const bool enable);

    /**
     * @brief Set the head tracking enabled or disabled by the specified device.
     *
     * @return Returns success or not
     * @since 12
     */
    int32_t SetHeadTrackingEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable);

    /**
     * @brief Register the spatialization enabled change callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t RegisterSpatializationEnabledEventListener(
        const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback);

    /**
     * @brief Register the head tracking enabled change callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t RegisterHeadTrackingEnabledEventListener(
        const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback);

    /**
     * @brief Unregister the spatialization enabled change callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t UnregisterSpatializationEnabledEventListener();

    /**
     * @brief Unregister the head tracking enabled change callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t UnregisterHeadTrackingEnabledEventListener();
    
    /**
     * @brief Check whether the spatialization is supported
     *
     * @return Returns <b>true</b> if the spatialization is supported; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsSpatializationSupported();

    /**
     * @brief Check whether the spatialization is supported for some device
     *
     * @return Returns <b>true</b> if the spatialization is supported; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsSpatializationSupportedForDevice(const sptr<AudioDeviceDescriptor> &selectedAudioDevice);

    /**
     * @brief Check whether the Head Tracking is supported
     *
     * @return Returns <b>true</b> if the Head Tracking is supported; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsHeadTrackingSupported();

    /**
     * @brief Check whether the head tracking is supported for some device
     *
     * @return Returns <b>true</b> if the head tracking is supported; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsHeadTrackingSupportedForDevice(const sptr<AudioDeviceDescriptor> &selectedAudioDevice);

    /**
     * @brief Update the state of the spatial Device
     *
     * @since 11
     */
    int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState);

    /**
     * @brief Get current spatialization rendering scene type
     *
     * @return Returns current spatialization scene type enum defined in {@link audio_effect.h}.
     * @since 12
     */
    AudioSpatializationSceneType GetSpatializationSceneType();

    /**
     * @brief Set spatialization rendering scene type
     *
     * @return Returns {@link SUCCESS} if setting spatialization scene type is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType);

    /**
     * @brief Check whether head tracking data is requested
     *
     * @return Returns <b>true</b> if the head tracking data is requested; returns <b>false</b> otherwise.
     * @since 12
     */
    bool IsHeadTrackingDataRequested(const std::string &macAddress);

    /**
     * @brief Register the head tracking data requested change callback listener for the specified device
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t RegisterHeadTrackingDataRequestedEventListener(const std::string &macAddress,
        const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &callback);

    /**
     * @brief Unregister the head tracking data requested change callback listener for the specified device
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnregisterHeadTrackingDataRequestedEventListener(const std::string &macAddress);
private:
    AudioSpatializationManager();
    virtual ~AudioSpatializationManager();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SPATIALIZATION_MANAGER_H
