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

#ifndef ST_AUDIO_SYSTEM_MANAGER_EXT_H
#define ST_AUDIO_SYSTEM_MANAGER_EXT_H

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_stutter.h"
#include "audio_device_descriptor.h"
#include "audio_stream_change_info.h"
#include "audio_interrupt_callback.h"
#include "audio_group_manager.h"
#include "audio_routing_manager.h"
#include "audio_policy_interface.h"
#include "audio_workgroup_ipc.h"

namespace OHOS {
namespace AudioStandard {

struct AudioSpatialEnabledStateForDevice {
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor;
    bool enabled;
};

struct DistributedRoutingInfo {
    std::shared_ptr<AudioDeviceDescriptor> descriptor;
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
    static InterruptGroupInfo *Unmarshalling(Parcel &parcel);
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
    static VolumeGroupInfo *Unmarshalling(Parcel &parcel);
};

/**
 * Describes the mic phone blocked device information.
 *
 * @since 13
 */
struct MicrophoneBlockedInfo : public Parcelable {
    DeviceBlockStatus blockStatus;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    static constexpr int32_t DEVICE_CHANGE_VALID_SIZE = 128;

    void SetClientInfo(std::shared_ptr<AudioDeviceDescriptor::ClientInfo> clientInfo) const
    {
        for (auto &dev : devices) {
            if (dev != nullptr) {
                dev->SetClientInfo(clientInfo);
            }
        }
    }

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteInt32(static_cast<int32_t>(blockStatus));
        int32_t size = static_cast<int32_t>(devices.size());
        parcel.WriteInt32(size);
        for (auto &dev : devices) {
            if (dev == nullptr) {
                return false;
            }
            dev->Marshalling(parcel);
        }
        return true;
    }

    static MicrophoneBlockedInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) MicrophoneBlockedInfo();
        if (info == nullptr) {
            return nullptr;
        }

        info->blockStatus = static_cast<DeviceBlockStatus>(parcel.ReadInt32());
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size >= DEVICE_CHANGE_VALID_SIZE) {
            delete info;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            auto device = AudioDeviceDescriptor::Unmarshalling(parcel);
            if (device != nullptr) {
                info->devices.emplace_back(std::shared_ptr<AudioDeviceDescriptor>(device));
            }
        }
        return info;
    }
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
    static AudioRendererFilter* Unmarshalling(Parcel &parcel);
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
    static AudioCapturerFilter *Unmarshalling(Parcel &in);
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

class AudioQueryBundleNameListCallback {
public:
    virtual ~AudioQueryBundleNameListCallback() = default;
    virtual bool OnQueryBundleNameIsInList(const std::string &bundleName, const std::string &listType) = 0;
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

class AudioManagerMicrophoneBlockedCallback {
public:
    virtual ~AudioManagerMicrophoneBlockedCallback() = default;
    /**
     * Called when micro phone is blocked.
     *
     * @param microphoneBlockedInfo Indicates the MisPhoneBlockedInfo information needed by client.
     * For details, refer MisPhoneBlockedInfo struct
     * @since 13
     */
    virtual void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) = 0;
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
    virtual void OnDistributedRoutingRoleChange(
        std::shared_ptr<AudioDeviceDescriptor>descriptor, const CastType type) = 0;
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
    void OnDistributedRoutingRoleChange(
        std::shared_ptr<AudioDeviceDescriptor>descriptor, const CastType type) override;
    void SaveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);
    void RemoveCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);
private:
    std::list<std::shared_ptr<AudioDistributedRoutingRoleCallback>> callbackList_;
    std::shared_ptr<AudioDistributedRoutingRoleCallback> cb_;
    std::mutex cbListMutex_;
};

class AudioDeviceAnahs {
public:
    virtual ~AudioDeviceAnahs() = default;

    virtual int32_t OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType) = 0;
};


class AudioRendererDataTransferStateChangeCallback {
public:
    virtual ~AudioRendererDataTransferStateChangeCallback() = default;

    virtual void OnDataTransferStateChange(const AudioRendererDataTransferStateChangeInfo &info) = 0;

    virtual void OnMuteStateChange(const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) = 0;
};

class AudioWorkgroupChangeCallback {
public:
    virtual ~AudioWorkgroupChangeCallback() = default;
    virtual void OnWorkgroupChange(const AudioWorkgroupChangeInfo &info) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SYSTEM_MANAGER_EXT_H
