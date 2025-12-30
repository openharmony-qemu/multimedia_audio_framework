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

#ifndef AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_H
#define AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_H

#include "audio_info.h"
#include "audio_system_manager_ext.h"
#include "audio_group_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioSystemClientPolicyManager {
public:
    static AudioSystemClientPolicyManager &GetInstance();

        /**
     * @brief Switch the output device accoring different cast type.
     *
     * @return Returns {@link SUCCESS} if device is successfully switched; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t ConfigDistributedRoutingRole(std::shared_ptr<AudioDeviceDescriptor> desciptor, CastType type);

    /**
     * @brief Registers the descriptor Change callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t SetDistributedRoutingRoleCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);

    /**
     * @brief UnRegisters the descriptor Change callback callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t UnsetDistributedRoutingRoleCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);

    /**
     * @brief Registers the audioScene change callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t SetAudioSceneChangeCallback(const std::shared_ptr<AudioManagerAudioSceneChangedCallback>& callback);

    /**
     * @brief Registers the audioScene change callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t UnsetAudioSceneChangeCallback(std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback = nullptr);

    /**
     * @brief Set ringer mode.
     *
     * @param ringMode audio ringer mode.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetRingerMode(AudioRingerMode ringMode);

    /**
     * @brief Get ringer mode.
     *
     * @return Returns audio ringer mode.
     * @since 8
     */
    AudioRingerMode GetRingerMode();

    /**
     * @brief Registers the ringerMode callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetRingerModeCallback(const int32_t clientId,
                                  const std::shared_ptr<AudioRingerModeCallback> &callback);

    /**
     * @brief Unregisters the VolumeKeyEvent callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetRingerModeCallback(const int32_t clientId) const;

    /**
     * @brief Set audio scene.
     *
     * @param scene audio scene.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetAudioScene(const AudioScene &scene);

    /**
     * @brief Get audio scene.
     *
     * @return Returns audio scene.
     * @since 8
     */
    AudioScene GetAudioScene() const;

    int32_t SetMicrophoneBlockedCallback(const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback);
    int32_t UnsetMicrophoneBlockedCallback(std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback = nullptr);
    int32_t SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback);
    int32_t SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback);
    int32_t SetAudioVKBInfoMgrCallback(const std::shared_ptr<AudioVKBInfoMgrCallback> &callback);
    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid);
    int32_t SetQueryAllowedPlaybackCallback(const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback);
    int32_t SetBackgroundMuteCallback(const std::shared_ptr<AudioBackgroundMuteCallback> &callback);
    int32_t SetQueryBundleNameListCallback(const std::shared_ptr<AudioQueryBundleNameListCallback> &callback);
private:
    AudioSystemClientPolicyManager() = default;
    ~AudioSystemClientPolicyManager();

    int32_t cbClientId_ = -1;
    AudioRingerMode ringModeBackup_ = RINGER_MODE_NORMAL;
    std::shared_ptr<AudioRingerModeCallback> ringerModeCallback_ = nullptr;
    std::mutex ringerModeCallbackMutex_;

    std::shared_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SYSTEM_CLIENT_POLICY_MANAGER_H
