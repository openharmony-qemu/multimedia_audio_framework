/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef AUDIO_DEVICE_INFO_H
#define AUDIO_DEVICE_INFO_H

#include <parcel.h>
#include <audio_stream_info.h>
#include <set>
#include <limits>
#include <unordered_set>
#include "audio_device_stream_info.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t INVALID_GROUP_ID = -1;
namespace {
const char* LOCAL_NETWORK_ID = "LocalDevice";
const char* REMOTE_NETWORK_ID = "RemoteDevice";
}

enum API_VERSION {
    API_7 = 7,
    API_8,
    API_9,
    API_10,
    API_11,
    API_MAX = 1000
};

enum DeviceFlag {
    /**
     * Device flag none.
     */
    NONE_DEVICES_FLAG = 0,
    /**
     * Indicates all output audio devices.
     */
    OUTPUT_DEVICES_FLAG = 1,
    /**
     * Indicates all input audio devices.
     */
    INPUT_DEVICES_FLAG = 2,
    /**
     * Indicates all audio devices.
     */
    ALL_DEVICES_FLAG = 3,
    /**
     * Indicates all distributed output audio devices.
     */
    DISTRIBUTED_OUTPUT_DEVICES_FLAG = 4,
    /**
     * Indicates all distributed input audio devices.
     */
    DISTRIBUTED_INPUT_DEVICES_FLAG = 8,
    /**
     * Indicates all distributed audio devices.
     */
    ALL_DISTRIBUTED_DEVICES_FLAG = 12,
    /**
     * Indicates all local and distributed audio devices.
     */
    ALL_L_D_DEVICES_FLAG = 15,
    /**
     * Device flag max count.
     */
    DEVICE_FLAG_MAX
};

enum DeviceRole {
    /**
     * Device role none.
     */
    DEVICE_ROLE_NONE = -1,
    /**
     * Input device role.
     */
    INPUT_DEVICE = 1,
    /**
     * Output device role.
     */
    OUTPUT_DEVICE = 2,
    /**
     * Device role max count.
     */
    DEVICE_ROLE_MAX
};

enum DeviceType {
    /**
     * Indicates device type none.
     */
    DEVICE_TYPE_NONE = -1,
    /**
     * Indicates invalid device
     */
    DEVICE_TYPE_INVALID = 0,
    /**
     * Indicates a built-in earpiece device
     */
    DEVICE_TYPE_EARPIECE = 1,
    /**
     * Indicates a speaker built in a device.
     */
    DEVICE_TYPE_SPEAKER = 2,
    /**
     * Indicates a headset, which is the combination of a pair of headphones and a microphone.
     */
    DEVICE_TYPE_WIRED_HEADSET = 3,
    /**
     * Indicates a pair of wired headphones.
     */
    DEVICE_TYPE_WIRED_HEADPHONES = 4,
    /**
     * Indicates a Bluetooth device used for telephony.
     */
    DEVICE_TYPE_BLUETOOTH_SCO = 7,
    /**
     * Indicates a Bluetooth device supporting the Advanced Audio Distribution Profile (A2DP).
     */
    DEVICE_TYPE_BLUETOOTH_A2DP = 8,
    /**
     * Indicates a Bluetooth device supporting the Advanced Audio Distribution Profile (A2DP) recording.
     */
    DEVICE_TYPE_BLUETOOTH_A2DP_IN = 9,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_MIC = 15,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_WAKEUP = 16,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_USB_HEADSET = 22,
    /**
     * Indicates a display device.
     */
    DEVICE_TYPE_DP = 23,
    /**
     * Indicates a virtual remote cast device
     */
    DEVICE_TYPE_REMOTE_CAST = 24,
    /**
     * Indicates a non-headset usb device.
     */
    DEVICE_TYPE_USB_DEVICE = 25,
    /**
     * Indicates a accessory audio device.
     */
    DEVICE_TYPE_ACCESSORY = 26,
    /**
     * Indicates a Distributed virtualization audio device.
     */
    DEVICE_TYPE_REMOTE_DAUDIO = 29,
    /**
     * Indicates a Bluetooth HearingAid device.
     */
    DEVICE_TYPE_HEARING_AID = 30,
    /**
     * Indicates a hdmi device
     */
    DEVICE_TYPE_HDMI = 27,
    /**
     * Indicates a line digital device
     */
    DEVICE_TYPE_LINE_DIGITAL = 28,
    /**
     * Indicates a Nearlink device for output.
     */
    DEVICE_TYPE_NEARLINK = 31,
    /**
     * Indicates a Nearlink device for input.
     */
    DEVICE_TYPE_NEARLINK_IN = 32,
    /**
     * Indicates a debug sink device
     */
    DEVICE_TYPE_FILE_SINK = 50,
    /**
     * Indicates a debug source device
     */
    DEVICE_TYPE_FILE_SOURCE = 51,
    /**
     * Indicates any headset/headphone for disconnect
     */
    DEVICE_TYPE_EXTERN_CABLE = 100,
    /**
     * Indicates default device
     */
    DEVICE_TYPE_DEFAULT = 1000,
    /**
     * Indicates a usb-arm device.
     */
    DEVICE_TYPE_USB_ARM_HEADSET = 1001,
    /**
     * Indicates device type max count.
     */
    DEVICE_TYPE_MAX
};

inline const std::unordered_set<DeviceType> INPUT_DEVICE_TYPE_SET = {
    DeviceType::DEVICE_TYPE_WIRED_HEADSET,
    DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
    DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DeviceType::DEVICE_TYPE_MIC,
    DeviceType::DEVICE_TYPE_WAKEUP,
    DeviceType::DEVICE_TYPE_USB_HEADSET,
    DeviceType::DEVICE_TYPE_USB_ARM_HEADSET,
    DeviceType::DEVICE_TYPE_FILE_SOURCE,
    DeviceType::DEVICE_TYPE_ACCESSORY,
    DeviceType::DEVICE_TYPE_NEARLINK_IN,
};

inline bool IsInputDevice(DeviceType deviceType, DeviceRole deviceRole = DEVICE_ROLE_NONE)
{
    // Arm usb device distinguishes input and output through device roles.
    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET || deviceType == DEVICE_TYPE_USB_HEADSET) {
        return deviceRole == INPUT_DEVICE;
    } else {
        return INPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
    }
}

enum DmDeviceType {
    DM_DEVICE_TYPE_DEFAULT = 0,
    DM_DEVICE_TYPE_PENCIL = 0xA07,
    DM_DEVICE_TYPE_UWB = 0x06C,
    DM_DEVICE_TYPE_NEARLINK_SCO = 0x032,
};

inline const std::unordered_set<DeviceType> OUTPUT_DEVICE_TYPE_SET = {
    DeviceType::DEVICE_TYPE_EARPIECE,
    DeviceType::DEVICE_TYPE_SPEAKER,
    DeviceType::DEVICE_TYPE_WIRED_HEADSET,
    DeviceType::DEVICE_TYPE_WIRED_HEADPHONES,
    DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
    DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP,
    DeviceType::DEVICE_TYPE_USB_HEADSET,
    DeviceType::DEVICE_TYPE_DP,
    DeviceType::DEVICE_TYPE_USB_ARM_HEADSET,
    DeviceType::DEVICE_TYPE_FILE_SINK,
    DeviceType::DEVICE_TYPE_REMOTE_CAST,
    DeviceType::DEVICE_TYPE_HDMI,
    DeviceType::DEVICE_TYPE_LINE_DIGITAL,
    DeviceType::DEVICE_TYPE_REMOTE_DAUDIO,
    DeviceType::DEVICE_TYPE_NEARLINK,
    DeviceType::DEVICE_TYPE_HEARING_AID,
};

inline bool IsOutputDevice(DeviceType deviceType, DeviceRole deviceRole = DEVICE_ROLE_NONE)
{
    // Arm usb device distinguishes input and output through device roles.
    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET || deviceType == DEVICE_TYPE_USB_HEADSET) {
        return deviceRole == OUTPUT_DEVICE;
    } else {
        return OUTPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
    }
}

enum DeviceBlockStatus {
    DEVICE_UNBLOCKED = 0,
    DEVICE_BLOCKED = 1,
};

enum DeviceChangeType {
    CONNECT = 0,
    DISCONNECT = 1,
};

enum DeviceVolumeType {
    EARPIECE_VOLUME_TYPE = 0,
    SPEAKER_VOLUME_TYPE = 1,
    HEADSET_VOLUME_TYPE = 2,
};

inline const std::unordered_set<DeviceType> ACTIVE_DEVICE_TYPE_SET = {
    DeviceType::DEVICE_TYPE_EARPIECE,
    DeviceType::DEVICE_TYPE_SPEAKER,
    DeviceType::DEVICE_TYPE_BLUETOOTH_SCO,
    DeviceType::DEVICE_TYPE_USB_HEADSET,
    DeviceType::DEVICE_TYPE_FILE_SINK,
};

inline bool IsActiveDeviceType(DeviceType deviceType)
{
    return ACTIVE_DEVICE_TYPE_SET.count(deviceType) > 0;
}

inline const std::unordered_set<DeviceType> COMMUNICATION_DEVICE_TYPE_SET = {
    DeviceType::DEVICE_TYPE_SPEAKER,
};

inline bool IsCommunicationDeviceType(DeviceType deviceType)
{
    return COMMUNICATION_DEVICE_TYPE_SET.count(deviceType) > 0;
}

enum AudioDeviceManagerType {
    DEV_MGR_UNKNOW = 0,
    LOCAL_DEV_MGR,
    REMOTE_DEV_MGR,
    BLUETOOTH_DEV_MGR,
};

enum AudioDevicePrivacyType {
    TYPE_PRIVACY,
    TYPE_PUBLIC,
    TYPE_NEGATIVE,
};

enum DeviceCategory {
    CATEGORY_DEFAULT = 0,
    BT_HEADPHONE = 1 << 0,
    BT_SOUNDBOX = 1 << 1,
    BT_CAR = 1 << 2,
    BT_GLASSES = 1 << 3,
    BT_WATCH = 1 << 4,
    BT_HEARAID = 1 << 5,
    BT_UNWEAR_HEADPHONE = 1 << 6,
};

enum DeviceUsage {
    MEDIA = 1 << 0,
    VOICE = 1 << 1,
    RECOGNITION = 1 << 2,
    ALL_USAGE = (1 << 3) - 1, // Represents the bitwise OR of all the above usages.
};

enum DeviceInfoUpdateCommand {
    CATEGORY_UPDATE = 1,
    CONNECTSTATE_UPDATE,
    ENABLE_UPDATE,
    USAGE_UPDATE,
    EXCEPTION_FLAG_UPDATE,
};

enum ConnectState {
    CONNECTED,
    SUSPEND_CONNECTED,
    VIRTUAL_CONNECTED,
    DEACTIVE_CONNECTED
};

enum PreferredType {
    AUDIO_MEDIA_RENDER = 0,
    AUDIO_CALL_RENDER = 1,
    AUDIO_CALL_CAPTURE = 2,
    AUDIO_RING_RENDER = 3,
    AUDIO_RECORD_CAPTURE = 4,
    AUDIO_TONE_RENDER = 5,
};

enum BluetoothOffloadState {
    NO_A2DP_DEVICE = 0,
    A2DP_NOT_OFFLOAD = 1,
    A2DP_OFFLOAD = 2,
};

struct VolumeBehavior : public Parcelable {
    bool isReady = false;
    bool isVolumeControlDisabled = false;
    std::string databaseVolumeName = "";

    VolumeBehavior(bool isReady_, bool isVolumeControlDisabled_, std::string databaseVolumeName_)
        : isReady(isReady_), isVolumeControlDisabled(isVolumeControlDisabled_), databaseVolumeName(databaseVolumeName_)
    {}
    VolumeBehavior() = default;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteBool(isReady) &&
            parcel.WriteBool(isVolumeControlDisabled) &&
            parcel.WriteString(databaseVolumeName);
    }

    static VolumeBehavior *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) VolumeBehavior();
        if (info == nullptr) {
            return nullptr;
        }

        info->isReady = parcel.ReadBool();
        info->isVolumeControlDisabled = parcel.ReadBool();
        info->databaseVolumeName = parcel.ReadString();
        return info;
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        isReady = parcel.ReadBool();
        isVolumeControlDisabled = parcel.ReadBool();
        databaseVolumeName = parcel.ReadString();
    }
};

struct DevicePrivacyInfo {
    std::string deviceName;
    DeviceType deviceType;
    DeviceRole deviceRole;
    DeviceCategory deviceCategory;
    DeviceUsage deviceUsage;
};

struct AffinityDeviceInfo {
    std::string groupName;
    DeviceType deviceType;
    DeviceFlag deviceFlag;
    std::string networkID;
    uint64_t chooseTimeStamp;
    bool isPrimary;
    bool SupportedConcurrency;
};

enum class AudioStreamDeviceChangeReason {
    UNKNOWN = 0,
    NEW_DEVICE_AVAILABLE = 1,
    OLD_DEVICE_UNAVALIABLE = 2,
    OVERRODE = 3,
    AUDIO_SESSION_ACTIVATE = 4,
    STREAM_PRIORITY_CHANGED = 5,
};

class AudioStreamDeviceChangeReasonExt : public Parcelable {
public:
    enum class ExtEnum {
        UNKNOWN = 0,
        NEW_DEVICE_AVAILABLE = 1,
        OLD_DEVICE_UNAVALIABLE = 2,
        OVERRODE = 3,
        AUDIO_SESSION_ACTIVATE = 4,
        STREAM_PRIORITY_CHANGED = 5,
        MIN = 1000,
        OLD_DEVICE_UNAVALIABLE_EXT = 1000,
        SET_AUDIO_SCENE = 1001,
        SET_DEFAULT_OUTPUT_DEVICE = 1002,
        DISTRIBUTED_DEVICE_UNAVAILABLE = 1003,
        SET_INPUT_DEVICE = 1004
    };

    operator AudioStreamDeviceChangeReason() const
    {
        if (reason_ < ExtEnum::MIN) {
            return static_cast<AudioStreamDeviceChangeReason>(reason_);
        } else {
            return AudioStreamDeviceChangeReason::UNKNOWN;
        }
    }

    operator int() const
    {
        return static_cast<int>(reason_);
    }

    AudioStreamDeviceChangeReasonExt()
        : reason_(ExtEnum::UNKNOWN) {}
    AudioStreamDeviceChangeReasonExt(const AudioStreamDeviceChangeReason &reason)
        : reason_(static_cast<ExtEnum>(reason)) {}

    AudioStreamDeviceChangeReasonExt(const ExtEnum &reason) : reason_(reason) {}

    bool IsOldDeviceUnavaliable() const
    {
        return reason_ == ExtEnum::OLD_DEVICE_UNAVALIABLE;
    }

    bool IsOldDeviceUnavaliableExt() const
    {
        return reason_ == ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT;
    }

    bool IsNewDeviceAvailable() const
    {
        return reason_ == ExtEnum::NEW_DEVICE_AVAILABLE;
    }

    bool IsOverride() const
    {
        return reason_ == ExtEnum::OVERRODE;
    }

    bool IsSetAudioScene() const
    {
        return reason_ == ExtEnum::SET_AUDIO_SCENE;
    }

    bool IsSetDefaultOutputDevice() const
    {
        return reason_ == ExtEnum::SET_DEFAULT_OUTPUT_DEVICE;
    }

    bool IsUnknown() const
    {
        return reason_ == ExtEnum::UNKNOWN;
    }

    bool IsDistributedDeviceUnavailable() const
    {
        return reason_ == ExtEnum::DISTRIBUTED_DEVICE_UNAVAILABLE;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(static_cast<int32_t>(reason_));
    }

    static AudioStreamDeviceChangeReasonExt *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioStreamDeviceChangeReasonExt();
        if (info == nullptr) {
            return nullptr;
        }
        info->reason_ = static_cast<ExtEnum>(parcel.ReadInt32());
        return info;
    }

private:
    ExtEnum reason_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_INFO_H
