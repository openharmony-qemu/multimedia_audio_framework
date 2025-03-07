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

#include "parcel.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {

inline bool IsUsb(DeviceType type)
{
    return type == DEVICE_TYPE_USB_HEADSET || type == DEVICE_TYPE_USB_ARM_HEADSET;
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
    DeviceStreamInfo audioStreamInfo_ = {};
    DeviceCategory deviceCategory_ = CATEGORY_DEFAULT;
    ConnectState connectState_ = CONNECTED;
    // AudioDeviceDescriptor
    bool exceptionFlag_ = false;
    int64_t connectTimeStamp_ = 0;
    std::shared_ptr<AudioDeviceDescriptor> pairDeviceDescriptor_;
    bool isScoRealConnected_ = false;
    bool isEnable_ = true;
    // DeviceInfo
    bool isLowLatencyDevice_ = false;
    int32_t a2dpOffloadFlag_ = NO_A2DP_DEVICE;
    // Other
    int32_t descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
    bool spatializationSupported_ = false;
    bool hasPair_{false};

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

    bool MarshallingToDeviceDescriptor(Parcel &parcel) const;

    bool MarshallingToDeviceInfo(Parcel &parcel) const;

    bool Marshalling(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission, int32_t apiVersion) const;

    bool MarshallingToDeviceInfo(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission,
        int32_t apiVersion) const;

    void Unmarshalling(Parcel &parcel);

    static std::shared_ptr<AudioDeviceDescriptor> UnmarshallingPtr(Parcel &parcel);

    void UnmarshallingToDeviceDescriptor(Parcel &parcel);

    void UnmarshallingToDeviceInfo(Parcel &parcel);

    void SetDeviceInfo(std::string deviceName, std::string macAddress);

    void SetDeviceCapability(const DeviceStreamInfo &audioStreamInfo, int32_t channelMask,
        int32_t channelIndexMasks = 0);

    bool IsSameDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const;

    bool IsSameDeviceInfo(const AudioDeviceDescriptor &deviceInfo) const;

    bool IsPairedDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const;

    DeviceType MapInternalToExternalDeviceType() const;

    struct AudioDeviceDescriptorHash {
        size_t operator()(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
        {
            if (deviceDescriptor == nullptr) {
                return 0;
            }
            return std::hash<int32_t>{}(static_cast<int32_t>(deviceDescriptor->deviceType_)) ^
                std::hash<int32_t>{}(static_cast<int32_t>(deviceDescriptor->deviceRole_)) ^
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
            return lhs->IsSameDeviceDesc(*rhs);
        }
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_DESCRIPTOR_H