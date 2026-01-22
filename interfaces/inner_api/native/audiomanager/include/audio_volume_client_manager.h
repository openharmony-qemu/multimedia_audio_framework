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

#ifndef AUDIO_VOLUME_CLIENT_MANAGER_H
#define AUDIO_VOLUME_CLIENT_MANAGER_H

#include "audio_policy_interface.h"
#include "audio_group_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioVolumeClientManager {
public:
    static AudioVolumeClientManager &GetInstance();

    /**
     * @brief Set the stream volume.
     *
     * @param volumeType Enumerates the audio volume type.
     * @param volume The volume to be set for the current stream.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetVolume(AudioVolumeType volumeType, int32_t volume, int32_t uid = 0);

    /**
     * @brief Set the stream volume.
     *
     * @param volumeType Enumerates the audio volume type.
     * @param volume The volume to be set for the current stream.
     * @param deviceType The volume to be set for the device.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t SetVolumeWithDevice(AudioVolumeType volumeType, int32_t volume, DeviceType deviceType);

    /**
     * @brief Set the app volume.
     *
     * @param appUid app uid.
     * @param volume The volume to be set for the current uid app.
     * @param flag Is need update ui
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetAppVolume(const int32_t appUid, const int32_t volume, const int32_t flag = 0);

    /**
     * @brief Set self app volume.
     *
     * @param volume The volume to be set for the current app.
     * @param flag Is need update ui
     * @return self app volume level
     */
    int32_t SetSelfAppVolume(const int32_t volume, const int32_t flag = 0);

    /**
     * @brief Get uid app volume.
     *
     * @param appUid App uid.
     * @param volumeLevel App volume level.
     * @return Get app volume result
     */
    int32_t GetAppVolume(int32_t appUid, int32_t &volumeLevel) const;

    /**
     * @brief Get the uid app volume.
     *
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t GetSelfAppVolume(int32_t &volumeLevel) const;

    /**
     * @brief Set self app volume change callback.
     *
     * @param callback callback when app volume change.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetSelfAppVolumeCallback(const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback);

    /**
     * @brief Unset self app volume change callback.
     *
     * @param callback Unset the callback.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t UnsetSelfAppVolumeCallback(const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback = nullptr);

    /**
     * @brief Set app volume change callback.
     *
     * @param appUid app uid.
     * @param callback callback when app volume changed
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetAppVolumeCallbackForUid(const int32_t appUid,
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback);

    /**
     * @brief Unset app volume change callback.
     *
     * @param callback Unset the callback.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t UnsetAppVolumeCallbackForUid(
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback = nullptr);

    /**
     * @brief Set the uid app volume muted.
     * @param appUid app uid
     * @param muted muted or unmuted.
     * @param flag Is need update ui
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetAppVolumeMuted(const int32_t appUid, const bool muted, const int32_t flag = 0);

    /**
     * @brief Set the mute state of the VoIP ringtone for the specified app.
     * @param appUid The UID of the app.
     * @param muted Set to true to mute the VoIP ringtone, false to unmute.
     * @return Returns {@link SUCCESS} if the app ringtone is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetAppRingMuted(int32_t appUid, bool muted);

    int32_t SetAdjustVolumeForZone(int32_t zoneId);

    /**
     * @brief Check the uid app volume is muted.
     * @param appUid app uid
     * @param owned If true is passed, the result will be indicated your owned muted statesettings to
     * this app. Otherwise if false is passed, the result will be indicated the real muted state.
     *  @param isMute App mute state has seted
     * @return the app uid muted status
     */
    int32_t IsAppVolumeMute(const int32_t appUid, const bool owned, bool &isMute);

    /**
     * @brief Unset active volume type change callback.
     *
     * @param callback Unset the callback.
     * @return Returns {@link SUCCESS} if stream change callback is successfully unset; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t UnsetActiveVolumeTypeCallback(
        const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback = nullptr);

    /**
     * @brief Set active volume type change callback.
     *
     * @param callback callback when active volume type change.
     * @return Returns {@link SUCCESS} if stream change callback is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetActiveVolumeTypeCallback(
        const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback);

    /**
     * @brief Obtains the current stream volume.
     *
     * @param volumeType Enumerates the audio volume type.
     * @return Returns current stream volume.
     * @since 8
     */
    int32_t GetVolume(AudioVolumeType volumeType, int32_t uid = 0) const;

    /**
     * @brief Set volume discount factor.
     *
     * @param streamId stream Unique identification.
     * @param volume Adjustment percentage.
     * @return Whether the operation is effective
     * @since 9
     */
    int32_t SetLowPowerVolume(int32_t streamId, float volume) const;

    /**
     * @brief get volume discount factor.
     *
     * @param streamId stream Unique identification.
     * @return Returns current stream volume.
     * @since 9
     */
    float GetLowPowerVolume(int32_t streamId) const;

    /**
     * @brief get single stream volume.
     *
     * @param streamId stream Unique identification.
     * @return Returns current stream volume.
     * @since 9
     */
    float GetSingleStreamVolume(int32_t streamId) const;

    /**
     * @brief get max stream volume.
     *
     * @param volumeType audio volume type.
     * @return Returns current stream volume.
     * @since 8
     */
    int32_t GetMaxVolume(AudioVolumeType volumeType);

    /**
     * @brief get min stream volume.
     *
     * @param volumeType audio volume type.
     * @return Returns current stream volume.
     * @since 8
     */
    int32_t GetMinVolume(AudioVolumeType volumeType);

    /**
     * @brief get device max stream volume.
     *
     * @param volumeType audio volume type.
     * @param deviceType device type.
     * @return Returns the maxinum stream volume.
     */
    int32_t GetDeviceMaxVolume(AudioVolumeType volumeType, DeviceType deviceType);

    /**
     * @brief get device min stream volume.
     *
     * @param volumeType audio volume type.
     * @param deviceType device type.
     * @return Returns the mininum stream volume.
     */
    int32_t GetDeviceMinVolume(AudioVolumeType volumeType, DeviceType deviceType);

    /**
     * @brief set stream mute.
     *
     * @param volumeType audio volume type.
     * @param mute Specifies whether the stream is muted.
     * @param deivceType Specifies which device to mute.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType = DEVICE_TYPE_NONE);

    /**
     * @brief is stream mute.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 8
     */
    bool IsStreamMute(AudioVolumeType volumeType) const;

    /**
     * @brief Is stream active.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsStreamActive(AudioVolumeType volumeType) const;

    /**
     * @brief get volume db value that system calculate by volume type, volume level and device type.
     *
     * @param volumeType audio volume type.
     * @param volumeLevel volume level.
     * @param device device type.
     * @return Returns volume db value that system calculate by volume type, volume level and device type.
     * @since 20
     */
    float GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device);

    /**
     * @brief Set global microphone mute state.
     *
     * @param mute Specifies whether the Microphone is muted.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetMicrophoneMute(bool isMute);

    /**
     * @brief get global microphone mute state.
     *
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsMicrophoneMute();

    /**
     * @brief Get volume groups manager
     *
     * @param networkId networkId
     * @return Returns AudioGroupManager
     * @since 8
     */
    std::shared_ptr<AudioGroupManager> GetGroupManager(int32_t groupId);
    
    /**
     * @brief Set Custmoized Ring Back Tone mute state.
     *
     * @param isMute Specifies whether the Customized Ring Back Tone is muted.
     * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
     * in {@link audio_errors.h}.
     */
    int32_t SetVoiceRingtoneMute(bool isMute);

    /**
     * @brief registers the volumeKeyEvent callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t RegisterVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_9);

    /**
     * @brief Unregisters the volumeKeyEvent callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnregisterVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback = nullptr);

    /**
     * @brief registers the volume degree callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t RegisterVolumeDegreeCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_11);

    /**
     * @brief Unregisters the volumeKeyEvent callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnregisterVolumeDegreeCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback = nullptr);

    /**
     * @brief registers the systemVolumeChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t RegisterSystemVolumeChangeCallback(const int32_t clientPid,
        const std::shared_ptr<SystemVolumeChangeCallback> &callback);

    /**
     * @brief Unregisters the systemVolumeChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnregisterSystemVolumeChangeCallback(const int32_t clientPid,
        const std::shared_ptr<SystemVolumeChangeCallback> &callback = nullptr);

    /**
    * @brief set focus stream type when process volume key event.
    *
    * @param volumeType Audio stream type.
    * @param duration duration time to last or cancel force type.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration);

    /**
     * @brief set stream volume degree.
     *
     * @param volumeType Audio stream type.
     * @param degree volume degree. It must be an integer with the range [0, 100].
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @since 21
     */
    int32_t SetVolumeDegree(AudioVolumeType volumeType, int32_t degree, int32_t uid = 0);

    /**
     * @brief get stream volume degree.
     *
     * @param volumeType Audio stream type.
     * @return Returns the volume degree for the specified Audio stream type.
     * @since 21
     */
    int32_t GetVolumeDegree(AudioVolumeType volumeType, int32_t uid = 0);

    /**
     * @brief get stream min volume degree.
     *
     * @param volumeType Audio stream type.
     * @return Returns the minimum volume degree for the specified Audio stream type.
     * @since 21
     */
    int32_t GetMinVolumeDegree(AudioVolumeType volumeType);

    /**
     * @brief Get the maximum volume level for the specified stream usage.
     *
     * @param streamUsage Specifies the stream usage.
     * @return Returns the maximum volume level for the specified stream usage.
     * @since 20
     */
    int32_t GetMaxVolumeByUsage(StreamUsage streamUsage);

    /**
     * @brief Get the minimum volume level for the specified stream usage.
     *
     * @param streamUsage Specifies the stream usage.
     * @return Returns the minimum volume level for the specified stream usage.
     * @since 20
     */
    int32_t GetMinVolumeByUsage(StreamUsage streamUsage);

    /**
     * @brief Get the current volume level for the specified stream usage.
     *
     * @param streamUsage Specifies the stream usage.
     * @return Returns the current volume level for the specified stream usage.
     * @since 20
     */
    int32_t GetVolumeByUsage(StreamUsage streamUsage);

    /**
     * @brief Get the mute state of the specified stream usage.
     *
     * @param streamUsage Specifies the stream usage.
     * @param isMute Specifies the mute state.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t IsStreamMuteByUsage(StreamUsage streamUsage, bool &isMute);

    /**
     * @brief Get the volume in unit of db by streamUsage.
     *
     * @param streamUsage Specifies the stream usage.
     * @param volumeLevel Specifies the volume level.
     * @param deviceType Specifies the device type.
     * @return Returns current volume in unit of db by streamUsage
     * @since 20
     */
    float GetVolumeInDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType deviceType);

    /**
     * @brief Get supported audio volume types.
     *
     * @return Returns current supported audio volume types
     * @since 20
     */
    std::vector<AudioVolumeType>GetSupportedAudioVolumeTypes();

    /**
     * @brief Get the audioVolumeType that streamUsage belongs.
     *
     * @param streamUsage Specifies the stream usage.
     * @return Returns the audioVolumeType that streamUsage belongs
     * @since 20
     */
    AudioVolumeType GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage);

    /**
     * @brief Get the streamUsages contained in audioVolumeType
     *
     * @param audioVolumeType Specifies the audio volume type.
     * @return Returns the streamUsages contained in audioVolumeType
     * @since 20
     */
    std::vector<StreamUsage> GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType);

    /**
     * @brief registers the StreamVolumeChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t RegisterStreamVolumeChangeCallback(const int32_t clientPid, const std::set<StreamUsage> &streamUsages,
        const std::shared_ptr<StreamVolumeChangeCallback> &callback);

    /**
     * @brief Unregisters the StreamVolumeChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnregisterStreamVolumeChangeCallback(const int32_t clientPid,
        const std::shared_ptr<StreamVolumeChangeCallback> &callback = nullptr);

    /**
     * @brief Set whether or not absolute volume is supported for the specified Bluetooth device
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support, int32_t volume = 0);

    /**
     * @brief Set the absolute volume value for the specified Bluetooth device
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi);

    /**
     * @brief Set the absolute volume value for the specified Nearlink device
     *
     * @return Returns success or not
     */
    int32_t SetNearlinkDeviceVolume(const std::string &macAddress, AudioVolumeType volumeType,
        const int32_t volume, const bool updateUi);

    /**
     * @brief Get volume groups
     *
     * @param networkId networkId
     * @param info VolumeGroupInfo
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &info);
private:
    AudioVolumeClientManager();
    virtual ~AudioVolumeClientManager();
    std::mutex volumeMutex_;
    int32_t volumeChangeClientPid_ = -1;
    std::vector<std::shared_ptr<AudioGroupManager>> groupManagerMap_;
    std::mutex groupManagerMapMutex_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_VOLUME_CLIENT_MANAGER_H
