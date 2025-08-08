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

#include "audio_system_manager.h"
#include "audio_session_device_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioSessionRestoreParame {
public:
    enum class OperationType {
        AUDIO_SESSION_ACTIVATE,
        AUDIO_SESSION_SET_SCENE,
    };

    struct AudioSessionAction {
        OperationType type;
        int32_t optValue;

        AudioSessionAction(const OperationType type, const int32_t value)
            : type(type), optValue(value) {}

        ~AudioSessionAction() = default;
    };

    explicit AudioSessionRestoreParame() = default;

    ~AudioSessionRestoreParame() = default;

    void OnAudioSessionDeactive();
    void OnAudioSessionStateChanged(AudioSessionStateChangeHint audioSessionStateChangeHint);
    void RecordAudioSessionOpt(const OperationType type, const int32_t value);
    bool RestoreParame(void);

private:
    std::mutex actionsMutex_;
    std::vector<std::unique_ptr<AudioSessionAction>> actions_;
};

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

class AudioSessionCurrentDeviceChangedCallback {
public:
    virtual ~AudioSessionCurrentDeviceChangedCallback() = default;
    /**
     * @brief
     *
     * @param deviceChangedEvent the audio session current device changed event.
     * @since 20
     */
    virtual void OnAudioSessionCurrentDeviceChanged(const CurrentOutputDeviceChangedEvent &deviceChangedEvent) = 0;
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
     * @param audioSessionScene - Audio session scene.
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
    int32_t SetAudioSessionStateChangeCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    /**
     * @brief Unset all audio session state changed callbacks.
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionStateChangeCallback();

    /**
     * @brief Unset the audio session state changed callback.
     *
     * @param stateChangedCallback The audio session state changed callback.
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionStateChangeCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    /**
     * @brief Get default output device type.
     *
     * @param deviceType The default output device type.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t GetDefaultOutputDevice(DeviceType &deviceType);

    /**
     * @brief Set the default output device for audio session scene.
     *
     * @param deviceType. The available deviceTypes are EARPIECE/SPEAKER/DEFAULT.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
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
    int32_t SetAudioSessionCurrentDeviceChangeCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);

    /**
     * @brief Unset all audio session device changed callbacks.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionCurrentDeviceChangeCallback();

    /**
     * @brief Unset the audio session device changed callback.
     *
     * @param deviceChangedCallback The audio session device changed callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnsetAudioSessionCurrentDeviceChangeCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);

    /**
     * @brief Register AudioPolicyServer died callback.
     *
     * @since 20
     */
    void RegisterAudioPolicyServerDiedCb();

    /**
     * @brief Restore all audio session parame when AudioPolicyServer died.
     *
     * @since 20
     */
    bool Restore();

    /**
     * @brief Clear restoreParame opt when session deactived.
     *
     * @since 20
     */
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent);

    /**
     * @brief Clear restoreParame opt when session scene state changed.
     *
     * @since 20
     */
    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent);

private:
    std::mutex setDefaultOutputDeviceMutex_;
    bool setDefaultOutputDevice_ = false;
    DeviceType setDeviceType_ = DEVICE_TYPE_INVALID;

    // used by restore
    std::mutex sessionManagerRestoreMutex_;
    bool policyServerDiedCbRegistered_ = false;
    std::shared_ptr<AudioSessionManagerPolicyServiceDiedCallback> sessionManagerRestoreCb_ = nullptr;

    AudioSessionRestoreParame restoreParame_;
};

class AudioSessionManagerServiceDiedRestore : public AudioSessionManagerPolicyServiceDiedCallback {
public:
    AudioSessionManagerServiceDiedRestore() = default;

    void OnAudioPolicyServiceDied() override;

    ~AudioSessionManagerServiceDiedRestore() = default;
};

class AudioSessionManagerStateCallback : public AudioSessionStateChangedCallback {
public:
    explicit AudioSessionManagerStateCallback() = default;

    void OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateChangedEvent) override;

    ~AudioSessionManagerStateCallback() = default;
};

class AudioSessionManagerDeactivedCallback : public AudioSessionCallback {
public:
    explicit AudioSessionManagerDeactivedCallback() = default;

    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) override;

    ~AudioSessionManagerDeactivedCallback() = default;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SESSION_MANAGER_H
