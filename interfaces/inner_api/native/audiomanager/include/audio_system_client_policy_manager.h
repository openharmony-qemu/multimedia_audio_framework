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
#include "audio_policy_interface.h"
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

    /**
     * @brief Set system sound uri
     *
     * @param key the key of uri
     * @param uri the value of uri
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 10
     */
    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    /**
     * @brief Get system sound uri
     *
     * @param key the key of uri
     * @return Returns the value of uri for the key
     * @since 10
     */
    std::string GetSystemSoundUri(const std::string &key);

    /**
     * @brief Get system sound path
     *
     * @param systemSoundType the system sound type
     * @return Returns the path for the system sound type
     * @since 23
     */
    std::string GetSystemSoundPath(const int32_t systemSoundType);

        /**
     * @brief Set App Silent On Display
     *
     * @param displayId app silent On display id
     * @return Returns {@link SUCCESS} if seting is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAppSilentOnDisplay(const int32_t displayId);

    /**
     * @brief Activactivate preempt audio focus mode
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 18
     */
    int32_t ActivatePreemptMode() const;

    /**
     * @brief Deactivactivate preempt audio focus mode
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 18
     */
    int32_t DeactivatePreemptMode() const;

    /**
     * @brief Update stream state
     *
     * @param clientUid client Uid
     * @param streamSetState streamSetState
     * @param streamUsage streamUsage
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
        StreamUsage streamUsage);

    /**
     * @brief Set nearlink voiceStatus flag
     *
     * @return Returns success or not
     */
    int32_t SetSleVoiceStatusFlag(bool isSleVoiceStatus);

    /**
     * @brief Registers the availbale deviceChange callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t SetAvailableDeviceChangeCallback(const AudioDeviceUsage usage,
        const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback);
    /**
     * @brief UnRegisters the availbale deviceChange callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage);

        /**
     * @brief Set device address.
     *
     * @param deviceType device type.
     * @param flag Device activation status.
     * @param address Device address
     * @param clientPid pid of caller.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t SetCallDeviceActive(DeviceType deviceType, bool flag, std::string &address,
        const int32_t clientUid = -1) const;

    /**
     * @brief set useraction command
     *
     * @param actionCommand action command
     * @param paramInfo information
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t DisableSafeMediaVolume();

     /**
     * @brief Load the split module for audio stream separation.
     *
     * @param splitArgs Specifies the types of audio to be split into different streams.
     * @param networkId The network identifier of the output device.
     * @return Returns {@link SUCCESS} if the module is loaded successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     * @since 12
     */
    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);
    /**
     * @brief Set App AVSession state change.
     *
     * @param uid Specifies uid of app.
     * @param pid Specifies pid of app.
     * @param hasSession Specifies whether app has AVSession.
     * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     */
    int32_t NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession);

    /**
     * @brief Set App Freeze state change.
     *
     * @param pidList Specifies all pid list to change state.
     * @param isFreeze Specifies Freeze or Unfreeze state.
     * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     */
    int32_t NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze);

    /**
     * @brief RSS reboot reset all proxy Freeze state change.
     *
     * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     */
    int32_t ResetAllProxy();

        /**
     * @brief Notify process background state.
     *
     * @param uid Specifies uid of app.
     * @param pid Specifies pid of app.
     * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     */
    int32_t NotifyProcessBackgroundState(const int32_t uid, const int32_t pid);
    
    /**
     * @brief set focus stream type when process volume key event.
     *
     * @param volumeType Audio stream type.
     * @param duration duration time to last or cancel force type.
     * @return Returns {@link AUDIO_OK} if the operation is successfully.
     * @test
     */
    int32_t ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration);
    
    bool GetVirtualCall();
    void CleanUpResource();
    int32_t SetVirtualCall(bool isVirtual);

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
