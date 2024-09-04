/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"
#include "audio_interrupt_callback.h"
#include "audio_group_manager.h"
#include "audio_routing_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceDescriptor;
class AudioDeviceDescriptor : public Parcelable {
    friend class AudioSystemManager;
public:
    DeviceType getType();
    DeviceRole getRole() const;
    DeviceType deviceType_ = DEVICE_TYPE_NONE;
    DeviceRole deviceRole_ = DEVICE_ROLE_NONE;
    int32_t deviceId_ = 0;
    int32_t channelMasks_ = 0;
    int32_t channelIndexMasks_ = 0;
    std::string deviceName_;
    std::string macAddress_;
    int32_t interruptGroupId_ = 0;
    int32_t volumeGroupId_ = 0;
    std::string networkId_;
    std::string displayName_;
    bool exceptionFlag_ = false;
    DeviceStreamInfo audioStreamInfo_ = {};
    DeviceCategory deviceCategory_ = CATEGORY_DEFAULT;
    int64_t connectTimeStamp_ = 0;
    std::shared_ptr<AudioDeviceDescriptor> pairDeviceDescriptor_;
    ConnectState connectState_ = CONNECTED;
    bool isScoRealConnected_ = false;
    bool isEnable_ = true;

    AudioDeviceDescriptor();
    AudioDeviceDescriptor(DeviceType type, DeviceRole role, int32_t interruptGroupId, int32_t volumeGroupId,
        std::string networkId);
    AudioDeviceDescriptor(DeviceType type, DeviceRole role);
    AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor);
    AudioDeviceDescriptor(const sptr<AudioDeviceDescriptor> &deviceDescriptor);
    virtual ~AudioDeviceDescriptor();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioDeviceDescriptor> Unmarshalling(Parcel &parcel);
    void SetDeviceInfo(std::string deviceName, std::string macAddress);
    void SetDeviceCapability(const DeviceStreamInfo &audioStreamInfo, int32_t channelMask,
        int32_t channelIndexMasks = 0);

    bool isSameDevice(const DeviceInfo &deviceInfo);
};

struct AudioSpatialEnabledStateForDevice {
    sptr<AudioDeviceDescriptor> deviceDescriptor;
    bool enabled;
};

struct DistributedRoutingInfo {
    sptr<AudioDeviceDescriptor> descriptor;
    CastType type;
};

class InterruptGroupInfo;
class InterruptGroupInfo : public Parcelable {
    friend class AudioSystemManager;
public:
    int32_t interruptGroupId_ = 0;
    int32_t mappingId_ = 0;
    std::string groupName_;
    std::string networkId_;
    ConnectType connectType_ = CONNECT_TYPE_LOCAL;
    InterruptGroupInfo();
    InterruptGroupInfo(int32_t interruptGroupId, int32_t mappingId, std::string groupName, std::string networkId,
        ConnectType type);
    virtual ~InterruptGroupInfo();
    bool Marshalling(Parcel &parcel) const override;
    static sptr<InterruptGroupInfo> Unmarshalling(Parcel &parcel);
};

class VolumeGroupInfo;
class VolumeGroupInfo : public Parcelable {
    friend class AudioSystemManager;
public:
    int32_t volumeGroupId_ = 0;
    int32_t mappingId_ = 0;
    std::string groupName_;
    std::string networkId_;
    ConnectType connectType_ = CONNECT_TYPE_LOCAL;

    /**
     * @brief Volume group info.
     *
     * @since 9
     */
    VolumeGroupInfo();

    /**
     * @brief Volume group info.
     *
     * @param volumeGroupId volumeGroupId
     * @param mappingId mappingId
     * @param groupName groupName
     * @param networkId networkId
     * @param type type
     * @since 9
     */
    VolumeGroupInfo(int32_t volumeGroupId, int32_t mappingId, std::string groupName, std::string networkId,
        ConnectType type);
    virtual ~VolumeGroupInfo();

    /**
     * @brief Marshall.
     *
     * @since 8
     * @return bool
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshall.
     *
     * @since 8
     * @return Returns volume group info
     */
    static sptr<VolumeGroupInfo> Unmarshalling(Parcel &parcel);
};

/**
 * Describes the device change type and device information.
 *
 * @since 7
 */
struct DeviceChangeAction {
    DeviceChangeType type;
    DeviceFlag flag;
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
};

/**
 * @brief AudioRendererFilter is used for select speficed AudioRenderer.
 */
class AudioRendererFilter;
class AudioRendererFilter : public Parcelable {
    friend class AudioSystemManager;
public:
    AudioRendererFilter();
    virtual ~AudioRendererFilter();

    int32_t uid = -1;
    AudioRendererInfo rendererInfo = {};
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    int32_t streamId = -1;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioRendererFilter> Unmarshalling(Parcel &in);
};

/**
 * @brief AudioCapturerFilter is used for select speficed audiocapturer.
 */
class AudioCapturerFilter;
class AudioCapturerFilter : public Parcelable {
    friend class AudioSystemManager;
public:
    AudioCapturerFilter();
    virtual ~AudioCapturerFilter();

    int32_t uid = -1;
    AudioCapturerInfo capturerInfo = {SOURCE_TYPE_INVALID, 0};

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioCapturerFilter> Unmarshalling(Parcel &in);
};

// AudioManagerCallback OnInterrupt is added to handle compilation error in call manager
// Once call manager adapt to new interrupt APIs, this will be removed
class AudioManagerCallback {
public:
    virtual ~AudioManagerCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param interruptAction Indicates the InterruptAction information needed by client.
     * For details, refer InterruptAction struct in audio_info.h
     */
    virtual void OnInterrupt(const InterruptAction &interruptAction) = 0;
    std::mutex cbMutex_;
};

class AudioManagerInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioManagerInterruptCallbackImpl();
    virtual ~AudioManagerInterruptCallbackImpl();

    /**
     * Called when an interrupt is received.
     *
     * @param interruptAction Indicates the InterruptAction information needed by client.
     * For details, refer InterruptAction struct in audio_info.h
     * @since 7
     */
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback);
private:
    std::weak_ptr<AudioManagerCallback> callback_;
    std::shared_ptr<AudioManagerCallback> cb_;
};

class AudioManagerDeviceChangeCallback {
public:
    virtual ~AudioManagerDeviceChangeCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param deviceChangeAction Indicates the DeviceChangeAction information needed by client.
     * For details, refer DeviceChangeAction struct
     * @since 8
     */
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) = 0;
};

class AudioManagerAvailableDeviceChangeCallback {
public:
    virtual ~AudioManagerAvailableDeviceChangeCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param deviceChangeAction Indicates the DeviceChangeAction information needed by client.
     * For details, refer DeviceChangeAction struct
     * @since 11
     */
    virtual void OnAvailableDeviceChange(const AudioDeviceUsage usage,
        const DeviceChangeAction &deviceChangeAction) = 0;
};

class VolumeKeyEventCallback {
public:
    virtual ~VolumeKeyEventCallback() = default;
    /**
     * @brief VolumeKeyEventCallback will be executed when hard volume key is pressed up/down
     *
     * @param volumeEvent the volume event info.
     * @since 8
     */
    virtual void OnVolumeKeyEvent(VolumeEvent volumeEvent) = 0;
};

class AudioParameterCallback {
public:
    virtual ~AudioParameterCallback() = default;
    /**
     * @brief AudioParameterCallback will be executed when parameter change.
     *
     * @param networkId networkId
     * @param key  Audio paramKey
     * @param condition condition
     * @param value value
     * @since 9
     */
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) = 0;
};

class AudioCapturerSourceCallback {
public:
    virtual ~AudioCapturerSourceCallback() = default;
    virtual void OnCapturerState(bool isActive) = 0;
};

class WakeUpSourceCloseCallback {
public:
    virtual ~WakeUpSourceCloseCallback() = default;
    virtual void OnWakeupClose() = 0;
};

class WakeUpSourceCallback : public AudioCapturerSourceCallback, public WakeUpSourceCloseCallback {
public:
    virtual ~WakeUpSourceCallback() = default;
    // Stop all listening capturers from sending false callbacks;
    // when all capturers have stopped, allow one capturer to start sending true callbacks
    virtual void OnCapturerState(bool isActive) = 0;
    virtual void OnWakeupClose() = 0;
};

class AudioPreferredOutputDeviceChangeCallback;

class AudioFocusInfoChangeCallback {
public:
    virtual ~AudioFocusInfoChangeCallback() = default;
    /**
     * Called when focus info change.
     *
     * @param focusInfoList Indicates the focusInfoList information needed by client.
     * For details, refer audioFocusInfoList_ struct in audio_policy_server.h
     * @since 9
     */
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) = 0;

    virtual void OnAudioFocusRequested(const AudioInterrupt &) {}

    virtual void OnAudioFocusAbandoned(const AudioInterrupt &) {}
};

class AudioFocusInfoChangeCallbackImpl : public AudioFocusInfoChangeCallback {
public:
    explicit AudioFocusInfoChangeCallbackImpl();
    virtual ~AudioFocusInfoChangeCallbackImpl();

    /**
     * Called when focus info change.
     *
     * @param focusInfoList Indicates the focusInfoList information needed by client.
     * For details, refer audioFocusInfoList_ struct in audio_policy_server.h
     * @since 9
     */
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnAudioFocusRequested(const AudioInterrupt &requestFocus) override;
    void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) override;
    void SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);

    /**
     *  Cancel when focus info change.
     *
     * @since 9
     */
    void RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);
private:
    std::list<std::weak_ptr<AudioFocusInfoChangeCallback>> callbackList_;
    std::shared_ptr<AudioFocusInfoChangeCallback> cb_;
    std::mutex cbListMutex_;
};

class AudioDistributedRoutingRoleCallback {
public:
    virtual ~AudioDistributedRoutingRoleCallback() = default;

    /**
     * Called when audio device descriptor change.
     *
     * @param descriptor Indicates the descriptor needed by client.
     * For details, refer AudioDeviceDescriptor in audio_system_manager.h
     * @since 9
     */
    virtual void OnDistributedRoutingRoleChange(const AudioDeviceDescriptor *descriptor, const CastType type) = 0;
    std::mutex cbMutex_;
};

class AudioDistributedRoutingRoleCallbackImpl : public AudioDistributedRoutingRoleCallback {
public:
    explicit AudioDistributedRoutingRoleCallbackImpl();
    virtual ~AudioDistributedRoutingRoleCallbackImpl();

    /**
     * Called when audio device descriptor change.
     *
     * @param descriptor Indicates the descriptor needed by client.
     * For details, refer AudioDeviceDescriptor in audio_system_manager.h
     * @since 9
     */
    void OnDistributedRoutingRoleChange(const AudioDeviceDescriptor *descriptor, const CastType type) override;
    void SaveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);
    void RemoveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);
private:
    std::list<std::shared_ptr<AudioDistributedRoutingRoleCallback>> callbackList_;
    std::shared_ptr<AudioDistributedRoutingRoleCallback> cb_;
    std::mutex cbListMutex_;
};

class AudioDeviceRefiner {
public:
    virtual ~AudioDeviceRefiner() = default;

    virtual int32_t OnAudioOutputDeviceRefined(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, StreamUsage streamUsage, int32_t clientUid, AudioPipeType audioPipeType) = 0;
    virtual int32_t OnAudioInputDeviceRefined(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType, int32_t clientUid, AudioPipeType audioPipeType) = 0;
};

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
     * @brief set stream mute.
     *
     * @param volumeType audio volume type.
     * @param mute Specifies whether the stream is muted.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetMute(AudioVolumeType volumeType, bool mute) const;

    /**
     * @brief is stream mute.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 8
     */
    bool IsStreamMute(AudioVolumeType volumeType) const;

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
    int32_t SelectOutputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Select input device.
     *
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectInputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

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
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

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
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Get the list of audio devices.
     *
     * @param deviceFlag Flag of device type.
     * @param GetAudioParameter Key of audio parameters to be obtained.
     * @return Returns the device list is obtained.
     * @since 9
     */
    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    /**
     * @brief Get the list of audio devices (inner).
     *
     * @param deviceFlag Flag of device type.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

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
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SetDeviceActive(ActiveDeviceType deviceType, bool flag) const;

    /**
     * @brief get device active.
     *
     * @param deviceType device type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsDeviceActive(ActiveDeviceType deviceType) const;

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
    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt);

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
     * @brief Reconfigure audio channel
     *
     * @param count count
     * @param deviceType device type
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);

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
     * @brief Get audio latency from Xml
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t GetAudioLatencyFromXml() const;

    /**
     * @brief Get audio sink from Xml
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    uint32_t GetSinkLatencyFromXml() const;

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
    std::vector<sptr<AudioDeviceDescriptor>> GetActiveOutputDeviceDescriptors();

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

    /**
     * @brief Ask audio native process to request thread priority for client
     *
     * @param tid Target thread id
     * @since 10
     */
    void RequestThreadPriority(uint32_t tid);

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
    int32_t ConfigDistributedRoutingRole(AudioDeviceDescriptor *desciptor, CastType type);

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
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t SetCallDeviceActive(ActiveDeviceType deviceType, bool flag, std::string address) const;

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

    static void AudioServerDied(pid_t pid);

    std::string GetSelfBundleName(int32_t uid);

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
    int32_t GetCallingPid();
    std::string GetSelfBundleName();

    int32_t RegisterWakeupSourceCallback();
    void OtherDeviceTypeCases(DeviceType deviceType) const;

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

    std::shared_ptr<AudioCapturerSourceCallback> audioCapturerSourceCallback_ = nullptr;
    std::shared_ptr<WakeUpSourceCloseCallback> audioWakeUpSourceCloseCallback_ = nullptr;

    std::shared_ptr<WakeUpCallbackImpl> remoteWakeUpCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SYSTEM_MANAGER_H
