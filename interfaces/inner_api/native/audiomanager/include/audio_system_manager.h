/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_SYSTEM_MANAGER_H
#define ST_AUDIO_SYSTEM_MANAGER_H

#include "audio_system_manager_ext.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief The AudioSystemManager class is an abstract definition of audio manager.
 *        Provides a series of client/interfaces for audio management
 */

class AudioSystemManager {
public:
    static AudioSystemManager *GetInstance();

    /**
     * @brief Map volume to HDI.
     *
     * @param volume volume value.
     * @return Returns current volume.
     * @since 8
     */
    static float MapVolumeToHDI(int32_t volume);

    /**
     * @brief Map volume from HDI.
     *
     * @param volume volume value.
     * @return Returns current volume.
     * @since 8
     */
    static int32_t MapVolumeFromHDI(float volume);

    /**
     * @brief Get audio streamType.
     *
     * @param contentType Enumerates the audio content type.
     * @param streamUsage Enumerates the stream usage.
     * @return Returns Audio streamType.
     * @since 8
     */
    static AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);

    /**
     * @brief Set the stream volume.
     *
     * @param volumeType Enumerates the audio volume type.
     * @param volume The volume to be set for the current stream.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetVolume(AudioVolumeType volumeType, int32_t volume) const;

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
    int32_t SetVolumeWithDevice(AudioVolumeType volumeType, int32_t volume, DeviceType deviceType) const;

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
    int32_t GetVolume(AudioVolumeType volumeType) const;

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
    int32_t SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType = DEVICE_TYPE_NONE) const;

    /**
     * @brief is stream mute.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 8
     */
    bool IsStreamMute(AudioVolumeType volumeType) const;

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
     * @brief Select output device.
     *
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectOutputDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Select input device.
     *
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectInputDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief get selected device info.
     *
     * @param uid identifier.
     * @param pid identifier.
     * @param streamType audio stream type.
     * @return Returns device info.
     * @since 9
     */
    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const;

    /**
     * @brief Select the audio output device according to the filter conditions.
     *
     * @param audioRendererFilter filter conditions.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Select the audio input device according to the filter conditions.
     *
     * @param audioRendererFilter filter conditions.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Exclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Unexclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Unexclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const;

    /**
     * @brief Get the list of excluded audio output devices according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @return Returns the device list is obtained.
     * @since 16
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage) const;

    /**
     * @brief Get the list of audio devices.
     *
     * @param deviceFlag Flag of device type.
     * @param GetAudioParameter Key of audio parameters to be obtained.
     * @return Returns the device list is obtained.
     * @since 9
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    /**
     * @brief Get the list of audio devices (inner).
     *
     * @param deviceFlag Flag of device type.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    /**
     * @brief Get the audio output device according to the filter conditions.
     *
     * @param AudioRendererFilter filter conditions.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(sptr<AudioRendererFilter> audioRendererFilter);

    /**
     * @brief Get the audio input device according to the filter conditions.
     *
     * @param AudioCapturerFilter filter conditions.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter);

    /**
     * @brief Get audio parameter.
     *
     * @param key Key of audio parameters to be obtained.
     * @return Returns the value of the obtained audio parameter
     * @since 9
     */
    const std::string GetAudioParameter(const std::string key);

    /**
     * @brief set audio parameter.
     *
     * @param key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 9
     */
    void SetAudioParameter(const std::string &key, const std::string &value);

    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrAecMode(const AsrAecMode asrAecMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrAecMode(AsrAecMode &asrAecMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrNoiseSuppressionMode(const AsrNoiseSuppressionMode asrNoiseSuppressionMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode &asrNoiseSuppressionMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrWhisperDetectionMode(const AsrWhisperDetectionMode asrWhisperDetectionMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t GetAsrWhisperDetectionMode(AsrWhisperDetectionMode &asrWhisperDetectionMode);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrVoiceControlMode(const AsrVoiceControlMode asrVoiceControlMode, bool on);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t SetAsrVoiceMuteMode(const AsrVoiceMuteMode asrVoiceMuteMode, bool on);
    /**
     * @brief set audio parameter.
     *
     * @parame key The key of the set audio parameter.
     * @param value The value of the set audio parameter.
     * @since 12
     */
    int32_t IsWhispering();

    /**
     * @brief Get audio parameter.
     *
     * @param mainKey Main key of audio parameters to be obtained.
     * @param subKeys subKeys of audio parameters to be obtained.
     * @param result value of sub key parameters.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * @since 11
     */
    int32_t GetExtraParameters(const std::string &mainKey,
        const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result);

    /**
     * @brief Set audio parameters.
     *
     * @param key The main key of the set audio parameter.
     * @param kvpairs The pairs with sub keys and values of the set audio parameter.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * @since 11
     */
    int32_t SetExtraParameters(const std::string &key,
        const std::vector<std::pair<std::string, std::string>> &kvpairs);

    /**
     * @brief Get transaction Id.
     *
     * @param deviceType device type.
     * @param deviceRole device role.
     * @return Returns transaction Id.
     * @since 9
     */
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole);

    /**
     * @brief Set device active.
     *
     * @param deviceType device type.
     * @param flag Device activation status.
     * @param clientPid pid of caller.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientUid = -1) const;

    /**
     * @brief get device active.
     *
     * @param deviceType device type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsDeviceActive(DeviceType deviceType) const;

    /**
     * @brief get active output device.
     *
     * @return Returns device type.
     * @since 9
     */
    DeviceType GetActiveOutputDevice();

    /**
     * @brief get active input device.
     *
     * @return Returns device type.
     * @since 9
     */
    DeviceType GetActiveInputDevice();

    /**
     * @brief Is stream active.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsStreamActive(AudioVolumeType volumeType) const;

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

    /**
     * @brief Registers the deviceChange callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetDeviceChangeCallback(const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>
        &callback);

    /**
     * @brief Unregisters the deviceChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetDeviceChangeCallback(DeviceFlag flag = DeviceFlag::ALL_DEVICES_FLAG,
        std::shared_ptr<AudioManagerDeviceChangeCallback> callback = nullptr);

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
     */
    int32_t RegisterVolumeDegreeCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback);

    /**
     * @brief Unregisters the volumeKeyEvent callback listener
     *
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
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
     * @brief registers the renderer data transfer callback listener
     *
     * @param param {@link DataTransferMonitorParam}
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t RegisterRendererDataTransferCallback(const DataTransferMonitorParam &param,
        const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback);

    /**
     * @brief Unregisters the renderer data transfer callback listener
     *
     * @param param {@link DataTransferMonitorParam}
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnregisterRendererDataTransferCallback(
        const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback);

    /**
     * @brief Set mono audio state
     *
     * @param monoState mono state
     * @since 8
     */
    void SetAudioMonoState(bool monoState);

    /**
     * @brief Set audio balance value
     *
     * @param balanceValue balance value
     * @since 8
     */
    void SetAudioBalanceValue(float balanceValue);

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

    // Below APIs are added to handle compilation error in call manager
    // Once call manager adapt to new interrupt APIs, this will be removed

    /**
     * @brief registers the audioManager callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetAudioManagerCallback(const AudioVolumeType streamType,
                                    const std::shared_ptr<AudioManagerCallback> &callback);

    /**
     * @brief Unregisters the audioManager callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetAudioManagerCallback(const AudioVolumeType streamType) const;

    /**
     * @brief Activate audio Interrupt
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t ActivateAudioInterrupt(AudioInterrupt &audioInterrupt);

    /**
     * @brief Set App Concurrency Mode
     *
     * @param appUid app Uid
     * @param mode concurrency Mode
     * @return Returns {@link SUCCESS} if seting is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAppConcurrencyMode(const int32_t appUid, const int32_t mode);

    /**
     * @brief Set App Slient On Display
     *
     * @param displayId app slient On display id
     * @return Returns {@link SUCCESS} if seting is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAppSlientOnDisplay(const int32_t displayId);

    /**
     * @brief Deactivactivate audio Interrupt
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const;

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
     * @brief registers the Interrupt callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback);

    /**
     * @brief Unregisters the Interrupt callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetAudioManagerInterruptCallback();

    /**
     * @brief Request audio focus
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t RequestAudioFocus(const AudioInterrupt &audioInterrupt);

    /**
     * @brief Abandon audio focus
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t AbandonAudioFocus(const AudioInterrupt &audioInterrupt);

    /**
     * @brief Request independent interrupt
     *
     * @param focusType focus type
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    bool RequestIndependentInterrupt(FocusType focusType);

    /**
     * @brief Abandon independent interrupt
     *
     * @param focusType focus type
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    bool AbandonIndependentInterrupt(FocusType focusType);

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
     * @brief Get Pin Value From Type
     *
     * @param deviceType deviceType
     * @param deviceRole deviceRole
     * @return Returns Enumerate AudioPin
     * @since 8
     */
    AudioPin GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const;

    /**
     * @brief Get type Value From Pin
     *
     * @param pin AudioPin
     * @return Returns Enumerate DeviceType
     * @since 8
     */
    DeviceType GetTypeValueFromPin(AudioPin pin) const;

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

    /**
     * @brief Get volume groups manager
     *
     * @param networkId networkId
     * @return Returns AudioGroupManager
     * @since 8
     */
    std::shared_ptr<AudioGroupManager> GetGroupManager(int32_t groupId);

    /**
     * @brief Get active output deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 8
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetActiveOutputDeviceDescriptors();

    /**
     * @brief Get preferred input device deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 10
     */
    int32_t GetPreferredInputDeviceDescriptors();

    /**
     * @brief Get audio focus info
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);

    /**
     * @brief Register callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t RegisterFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    /**
     * @brief Unregister callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t UnregisterFocusInfoChangeCallback(
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback = nullptr);

    int32_t SetAudioCapturerSourceCallback(const std::shared_ptr<AudioCapturerSourceCallback> &callback);

    int32_t SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback);

    /**
     * @brief Set whether or not absolute volume is supported for the specified Bluetooth device
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

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
    int32_t SetCallDeviceActive(DeviceType deviceType, bool flag, std::string address,
        const int32_t clientUid = -1) const;

    /**
     * @brief get the effect algorithmic latency value for a specified audio stream.
     *
     * @param sessionId the session ID value for the stream
     * @return Returns the effect algorithmic latency in ms.
     * @since 12
     */
    uint32_t GetEffectLatency(const std::string &sessionId);

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

    static void AudioServerDied(pid_t pid, pid_t uid);

    int32_t SetMicrophoneBlockedCallback(const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback);
    int32_t UnsetMicrophoneBlockedCallback(std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback = nullptr);

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

    std::string GetSelfBundleName(int32_t uid);

    int32_t SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback);
    int32_t SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback);
    int32_t SetAudioVKBInfoMgrCallback(const std::shared_ptr<AudioVKBInfoMgrCallback> &callback);
    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid);
    int32_t SetQueryAllowedPlaybackCallback(const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback);
    int32_t SetBackgroundMuteCallback(const std::shared_ptr<AudioBackgroundMuteCallback> &callback);

    int32_t SetQueryBundleNameListCallback(const std::shared_ptr<AudioQueryBundleNameListCallback> &callback);

    /**
     * @brief inject interruption event.
     *
     * @param networkId networkId.
     * @param event Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_interrupt_info.h
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t InjectInterruption(const std::string networkId, InterruptEvent &event);

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
    * @brief Set Custmoized Ring Back Tone mute state.
    *
    * @param isMute Specifies whether the Customized Ring Back Tone is muted.
    * @return Returns {@link SUCCESS} if the settings is successfully; otherwise, returns an error code defined
    * in {@link audio_errors.h}.
    */
    int32_t SetVoiceRingtoneMute(bool isMute);

    /**
     * @brief Set foreground list.
     *
     * @param list The foreground list.
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
     * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
     */
    int32_t SetForegroundList(std::vector<std::string> list);

    /**
    * @brief Get standby state.
    *
    * @param sessionId Specifies which stream to be check.
    * @param isStandby true means the stream is in standby status.
    * @param enterStandbyTime Specifies when the stream enter standby status, in MONOTONIC time.
    * @return Returns {@link SUCCESS} if the operation is successfully.
    * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
    * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
    */
    int32_t GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime);

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
     * @brief Set render whitelist.
     *
     * @param list render whitelist.
     * @return Returns {@link SUCCESS} if the operation is successfully.
     * @return Returns {@link ERR_ILLEGAL_STATE} if the server is not available.
     * @return Returns {@link ERR_INVALID_PARAM} if the sessionId is not exist.
     */
    int32_t SetRenderWhitelist(std::vector<std::string> list);

#ifdef HAS_FEATURE_INNERCAPTURER
    /**
    * @brief check capture limit
    *
    * @param AudioPlaybackCaptureConfig inner capture filter info
    * @param innerCapId unique identifier of inner capture
    * @return Returns {@link SUCCESS} if the operation is successfully.
    * @test
    */
    int32_t CheckCaptureLimit(const AudioPlaybackCaptureConfig &config, int32_t &innerCapId);

    /**
    * @brief release capture limit
    *
    * @param innerCapId unique identifier of inner capture
    * @return Returns {@link SUCCESS} if the operation is successfully.
    * @test
    */
    int32_t ReleaseCaptureLimit(int32_t innerCapId);
#endif

    int32_t GenerateSessionId(uint32_t &sessionId);
    int32_t SetAudioInterruptCallback(const uint32_t sessionID, const std::shared_ptr<AudioInterruptCallback> &callback,
        uint32_t clientUid, const int32_t zoneID);
    int32_t UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId);

    int32_t SetVirtualCall(const bool isVirtual);

    int32_t OnVoiceWakeupState(bool state);

    uint16_t GetDmDeviceType() const;
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
    * @brief create audio workgroup
    *
    * @return Returns id of workgroup. id < 0 if failed.
    * @test
    */
    int32_t CreateAudioWorkgroup();

    /**
    * @brief release audio workgroup.
    *
    * @param workgroupId audio workgroup id.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t ReleaseAudioWorkgroup(int32_t workgroupId);

    /**
    * @brief add thread to audio workgroup.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param tokenId the thread id of add workgroupId.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t AddThreadToGroup(int32_t workgroupId, int32_t tokenId);

    /**
    * @brief remove thread to audio workgroup.y
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param tokenId the thread id of remove workgroupId.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t RemoveThreadFromGroup(int32_t workgroupId, int32_t tokenId);

    /**
    * @brief the deadline workgroup starts to take effect.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @param startTime timestamp when the deadline task starts to be executed.
    * @param deadlineTime complete a periodic task within the time specified by deadlineTime.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t StartGroup(int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime,
        std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio);

    /**
    * @brief stop the deadline workgroup.
    *
    * @param workgroupId workgroupId audio workgroup id.
    * @return Returns {@link AUDIO_OK} if the operation is successfully.
    * @test
    */
    int32_t StopGroup(int32_t workgroupId);

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
    * @brief clean resouce.
    * @test
    */
    void CleanUpResource();

    class WorkgroupPrioRecorder {
    public:
        WorkgroupPrioRecorder(int32_t grpId);
        ~WorkgroupPrioRecorder() = default;
        void SetRestoreByPermission(bool isByPermission);
        bool GetRestoreByPermission();
        int32_t GetGrpId();
        void RecordThreadPrio(int32_t tokenId);
        int32_t RestoreGroupPrio(bool isByPermission);
        int32_t RestoreThreadPrio(int32_t tokenId);
    private:
        int32_t grpId_;
        std::unordered_map<int32_t, int32_t> threads_;
        bool restoreByPermission_;
        std::mutex workgroupThreadsMutex_;
    };
    std::shared_ptr<WorkgroupPrioRecorder> GetRecorderByGrpId(int32_t grpId);
    int32_t ExcuteAudioWorkgroupPrioImprove(int32_t workgroupId,
        const std::unordered_map<int32_t, bool> threads, bool &needUpdatePrio);

private:
    class WakeUpCallbackImpl : public WakeUpSourceCallback {
    public:
        WakeUpCallbackImpl(AudioSystemManager *audioSystemManager)
            :audioSystemManager_(audioSystemManager)
        {
        }
        void OnCapturerState(bool isActive) override
        {
            auto callback = audioSystemManager_ -> audioCapturerSourceCallback_;
            if (callback != nullptr) {
                callback -> OnCapturerState(isActive);
            }
        }
        void OnWakeupClose() override
        {
            auto callback = audioSystemManager_ -> audioWakeUpSourceCloseCallback_;
            if (callback != nullptr) {
                callback -> OnWakeupClose();
            }
        }
    private:
        AudioSystemManager *audioSystemManager_;
    };

    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t CONST_FACTOR = 100;
    static const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_;

    AudioSystemManager();
    virtual ~AudioSystemManager();

    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();
    static void CreateStreamMap(std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> &streamMap);
    int32_t GetCallingPid() const;
    std::string GetSelfBundleName();

    int32_t RegisterWakeupSourceCallback();
    void OtherDeviceTypeCases(DeviceType deviceType) const;
    AudioPin GetPinValueForPeripherals(DeviceType deviceType, DeviceRole deviceRole, uint16_t dmDeviceType) const;

    int32_t cbClientId_ = -1;
    int32_t volumeChangeClientPid_ = -1;
    AudioRingerMode ringModeBackup_ = RINGER_MODE_NORMAL;
    std::shared_ptr<AudioManagerDeviceChangeCallback> deviceChangeCallback_ = nullptr;
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioRingerModeCallback> ringerModeCallback_ = nullptr;
    std::shared_ptr<AudioFocusInfoChangeCallback> audioFocusInfoCallback_ = nullptr;
    std::shared_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback_ = nullptr;
    std::vector<std::shared_ptr<AudioGroupManager>> groupManagerMap_;
    std::mutex ringerModeCallbackMutex_;
    std::mutex groupManagerMapMutex_;

    std::shared_ptr<AudioCapturerSourceCallback> audioCapturerSourceCallback_ = nullptr;
    std::shared_ptr<WakeUpSourceCloseCallback> audioWakeUpSourceCloseCallback_ = nullptr;

    std::shared_ptr<WakeUpCallbackImpl> remoteWakeUpCallback_;

    class AudioWorkgroupChangeCallbackImpl : public AudioWorkgroupChangeCallback {
    public:
        AudioWorkgroupChangeCallbackImpl() {};
        ~AudioWorkgroupChangeCallbackImpl() {};
    private:
        void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info) override;
    };

    std::mutex startGroupPermissionMapMutex_;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, bool>> startGroupPermissionMap_;
    void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info);
    bool IsValidToStartGroup(int32_t workgroupId);
    bool hasSystemPermission_ = false;
    std::unordered_map<int32_t, std::shared_ptr<WorkgroupPrioRecorder>> workgroupPrioRecorderMap_;
    std::mutex workgroupPrioRecorderMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SYSTEM_MANAGER_H