/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_DEVICE_DESCRIPTOR_H
#define AUDIO_DEVICE_DESCRIPTOR_H

#include <memory>
#include <map>
#include "parcel.h"
#include "audio_device_info.h"
#include "audio_info.h"
#include <optional>

namespace OHOS {
namespace AudioStandard {

inline bool IsUsb(DeviceType type)
{
    return type == DEVICE_TYPE_USB_HEADSET || type == DEVICE_TYPE_USB_ARM_HEADSET;
}

inline bool IsNearlinkDevice(DeviceType deviceType)
{
    return deviceType == DEVICE_TYPE_NEARLINK || deviceType == DEVICE_TYPE_NEARLINK_IN;
}

/**
 * @brief The AudioDeviceDescriptor provides
 *         different sets of audio devices and their roles
 */
class AudioDeviceDescriptor : public Parcelable {
friend class AudioSystemManager;
public:
    enum {
        AUDIO_DEVICE_DESCRIPTOR,
        DEVICE_INFO,
    };

    class ClientInfo {
    public:
        bool hasBTPermission_ = false;
        bool hasSystemPermission_ = false;
        int32_t apiVersion_ = 0;
        bool isSupportedNearlink_ = true;

        ClientInfo() = default;
        ClientInfo(int32_t apiVersion)
            : apiVersion_(apiVersion) {}
        ClientInfo(bool hasBTPermission, bool hasSystemPermission, int32_t apiVersion)
            : hasBTPermission_(hasBTPermission), hasSystemPermission_(hasSystemPermission), apiVersion_(apiVersion) {}
    };

    AudioDeviceDescriptor(int32_t descriptorType = AUDIO_DEVICE_DESCRIPTOR);

    AudioDeviceDescriptor(DeviceType type, DeviceRole role);

    AudioDeviceDescriptor(DeviceType type, DeviceRole role, int32_t interruptGroupId, int32_t volumeGroupId,
        std::string networkId);

    AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor);

    AudioDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    virtual ~AudioDeviceDescriptor();

    DeviceType getType() const;

    DeviceRole getRole() const;

    DeviceCategory GetDeviceCategory() const;

    bool IsAudioDeviceDescriptor() const;

    bool Marshalling(Parcel &parcel) const override;

    void UnmarshallingSelf(Parcel &parcel);

    static AudioDeviceDescriptor *Unmarshalling(Parcel &parcel);

    static void MapInputDeviceType(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);

    void SetDeviceInfo(std::string deviceName, std::string macAddress);

    void SetDeviceCapability(const std::list<DeviceStreamInfo> &audioStreamInfo, int32_t channelMask,
        int32_t channelIndexMasks = 0);

    bool IsSameDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const;

    bool IsSameDeviceDescPtr(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor) const;

    bool IsSameDeviceInfo(const AudioDeviceDescriptor &deviceInfo) const;

    bool IsPairedDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const;

    bool IsDistributedSpeaker() const;

    bool IsA2dpOffload() const;

    bool IsSpeakerOrEarpiece() const;

    bool IsRemote() const;

    bool IsRemoteDevice() const;

    DeviceType MapInternalToExternalDeviceType(int32_t apiVersion, bool isSupportedNearlink = true) const;

    DeviceStreamInfo GetDeviceStreamInfo(void) const;

    void Dump(std::string &dumpString);

    std::string GetDeviceTypeString();

    std::string GetKey();

    std::string GetName();

    struct AudioDeviceDescriptorHash {
        size_t operator()(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
        {
            if (deviceDescriptor == nullptr) {
                return 0;
            }
            return std::hash<int32_t>{}(static_cast<int32_t>(deviceDescriptor->deviceType_)) ^
                std::hash<std::string>{}(deviceDescriptor->macAddress_) ^
                std::hash<std::string>{}(deviceDescriptor->networkId_);
        }
    };

    struct AudioDeviceDescriptorEqual {
        bool operator()(const std::shared_ptr<AudioDeviceDescriptor> &lhs,
            const std::shared_ptr<AudioDeviceDescriptor> &rhs) const
        {
            if (lhs == nullptr && rhs == nullptr) {
                return true;
            }
            if (lhs == nullptr || rhs == nullptr) {
                return false;
            }
            return lhs->deviceType_ == rhs->deviceType_ &&
                lhs->macAddress_ == rhs->macAddress_ &&
                lhs->networkId_ == rhs->networkId_;
        }
    };

    void SetClientInfo(const ClientInfo &clientInfo) const;
private:
    static void FixApiCompatibility(int apiVersion, DeviceRole deviceRole,
        DeviceType &deviceType, int32_t &deviceId, std::list<DeviceStreamInfo> &streamInfo);

    bool MarshallingInner(Parcel &parcel) const;

    bool MarshallingToDeviceInfo(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission,
        int32_t apiVersion, bool isSupportedNearlink = true) const;
public:
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
    uint16_t dmDeviceType_{0};
    std::string displayName_;
    std::list<DeviceStreamInfo> audioStreamInfo_;
    DeviceCategory deviceCategory_ = CATEGORY_DEFAULT;
    ConnectState connectState_ = CONNECTED;
    DeviceUsage deviceUsage_ = ALL_USAGE;
    // AudioDeviceDescriptor
    bool exceptionFlag_ = false;
    int64_t connectTimeStamp_ = 0;
    std::shared_ptr<AudioDeviceDescriptor> pairDeviceDescriptor_;
    bool isScoRealConnected_ = false;
    bool isEnable_ = true;
    int32_t mediaVolume_ = 0;
    int32_t callVolume_ = 0;
    // DeviceInfo
    bool isLowLatencyDevice_ = false;
    int32_t a2dpOffloadFlag_ = NO_A2DP_DEVICE;
    // Other
    int32_t descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
    bool spatializationSupported_ = false;
    bool hasPair_{false};
    RouterType routerType_ = ROUTER_TYPE_NONE;
    bool isVrSupported_ = true;
    mutable std::optional<ClientInfo> clientInfo_ = std::nullopt;
    VolumeBehavior volumeBehavior_;
    bool modemCallSupported_ = true;
    bool highQualityRecordingSupported_ = false;

private:
    bool IsOutput()
    {
        return deviceRole_ == OUTPUT_DEVICE;
    }
    static const std::map<DeviceType, std::string> deviceTypeStringMap;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_DESCRIPTOR_H
