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

#ifndef ST_AUDIO_SESSION_MANAGER_H
#define ST_AUDIO_SESSION_MANAGER_H

#include "audio_device_descriptor.h"
#include "audio_session_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioSessionCallback {
public:
    virtual ~AudioSessionCallback() = default;
    /**
     * @brief OnAudioSessionDeactive will be executed when the audio session is deactivated be others.
     *
     * @param deactiveEvent the audio session deactive event info.
     * @since 12
     */
    virtual void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) = 0;
};

class AudioSessionStateChangedCallback {
public:
    virtual ~AudioSessionStateChangedCallback() = default;
    /**
     * @brief The function will be executed when the audio session state changed.
     *
     * @param stateChangedEvent the audio session state changed event.
     * @since 20
     */
    virtual void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent) = 0;
};

/**
 * Audio session device change info.
 * @since 20
 */
struct CurrentDeviceChangedEvent {
    /**
     * Audio device descriptors after changed.
     * @since 20
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    /**
     * Audio device changed reason.
     * @since 20
     */
    AudioStreamDeviceChangeReason changedReason;
    /**
     * Recommend action when device changed.
     * @since 20
     */
    DeviceChangedRecommendedAction recommendedAction;
};

class AudioSessionCurrentDeviceChangedCallback {
public:
    virtual ~AudioSessionCurrentDeviceChangedCallback() = default;
    /**
     * @brief
     *
     * @param deviceChangedEvent the audio session current device changed event.
     * @since 20
     */
    virtual void OnAudioSessionCurrentDeviceChanged(const CurrentDeviceChangedEvent &deviceChangedEvent) = 0;
};

class AudioSessionManager {
public:
    AudioSessionManager() = default;
    virtual ~AudioSessionManager() = default;

    static AudioSessionManager *GetInstance();

    /**
     * @brief Activate audio session.
     *
     * @param strategy Target audio session strategy.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t ActivateAudioSession(const AudioSessionStrategy &strategy);

    /**
     * @brief Deactivate audio session.
     *
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t DeactivateAudioSession();

    /**
     * @brief Query whether the audio session is active.
     *
     * @return Returns <b>true</b> if the audio session is active; returns <b>false</b> otherwise.
     * @since 12
     */
    bool IsAudioSessionActivated();

    /**
     * @brief Set audio session callback.
     *
     * @param audioSessionCallback The audio session callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);

    /**
     * @brief Unset all audio session callbacks.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnsetAudioSessionCallback();

    /**
     * @brief Unset audio session callback.
     *
     * @param audioSessionCallback The audio session callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnsetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);

    /**
     * @brief Set scene for audio session.
     *
     * @param { AudioSessionScene } audioSessionScene - Audio session scene.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAudioSessionScene(const AudioSessionScene audioSessionScene);

    /**
     * @brief Listens for audio session state changed event.
     * When the audio session state change, registered clients will receive the callback.
     *
     * @param stateChangedCallback The audio session state changed callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAudioSessionStateChangedCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    /**
     * @brief Unset all audio session state changed callbacks.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionStateChangedCallback();

    /**
     * @brief Unset the audio session state changed callback.
     *
     * @param stateChangedCallback The audio session state changed callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionStateChangedCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    /**
     * @brief Get current output device or devices.
     *
     * @param deviceInfo The current output device descriptor.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const;

    /**
     * @brief Set the default output device for audio session scene.
     *
     * @param deviceType. The available deviceTypes are EARPIECE/SPEAKER/DEFAULT.
     * @since 20
     */
    int32_t SetDefaultOutputDevice(DeviceType deviceType);

    /**
     * @brief Subscribes device changed event callback.
     * The event is triggered when device changed.
     *
     * @param deviceChangedCallback The audio session device changed callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAudioSessionCurrentDeviceChangedCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);

    /**
     * @brief Unset all audio session device changed callbacks.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionCurrentDeviceChangedCallback();

    /**
     * @brief Unset the audio session device changed callback.
     *
     * @param deviceChangedCallback The audio session device changed callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionCurrentDeviceChangedCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SESSION_MANAGER_H
