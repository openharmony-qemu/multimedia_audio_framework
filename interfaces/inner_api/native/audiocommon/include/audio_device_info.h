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

namespace OHOS {
namespace AudioStandard {
constexpr size_t AUDIO_DEVICE_INFO_SIZE_LIMIT = 30;
constexpr int32_t INVALID_GROUP_ID = -1;

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
    DeviceType::DEVICE_TYPE_MIC,
    DeviceType::DEVICE_TYPE_WAKEUP,
    DeviceType::DEVICE_TYPE_USB_HEADSET,
    DeviceType::DEVICE_TYPE_USB_ARM_HEADSET,
    DeviceType::DEVICE_TYPE_FILE_SOURCE,
};

inline bool IsInputDevice(DeviceType deviceType)
{
    return INPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
}

inline bool IsInputDevice(DeviceType deviceType, DeviceRole deviceRole)
{
    // Arm usb device distinguishes input and output through device roles.
    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
        return deviceRole == INPUT_DEVICE || deviceRole == DEVICE_ROLE_MAX;
    } else {
        return INPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
    }
}

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
};

inline bool IsOutputDevice(DeviceType deviceType)
{
    return OUTPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
}

inline bool IsOutputDevice(DeviceType deviceType, DeviceRole deviceRole)
{
    // Arm usb device distinguishes input and output through device roles.
    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET) {
        return deviceRole == OUTPUT_DEVICE || deviceRole == DEVICE_ROLE_MAX;
    } else {
        return OUTPUT_DEVICE_TYPE_SET.count(deviceType) > 0;
    }
}

enum DeviceChangeType {
    CONNECT = 0,
    DISCONNECT = 1,
};

enum DeviceVolumeType {
    EARPIECE_VOLUME_TYPE = 0,
    SPEAKER_VOLUME_TYPE = 1,
    HEADSET_VOLUME_TYPE = 2,
};

enum ActiveDeviceType {
    ACTIVE_DEVICE_TYPE_NONE = -1,
    EARPIECE = 1,
    SPEAKER = 2,
    BLUETOOTH_SCO = 7,
    USB_HEADSET = 22,
    FILE_SINK_DEVICE = 50,
    ACTIVE_DEVICE_TYPE_MAX
};

enum CommunicationDeviceType {
    /**
     * Speaker.
     * @since 7
     * @syscap SystemCapability.Multimedia.Audio.Communication
     */
    COMMUNICATION_SPEAKER = 2
};

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
    MEDIA = 1,
    VOICE = 2,
    ALL_USAGE = 3,
    RECONGNITION = 4,
};

enum DeviceInfoUpdateCommand {
    CATEGORY_UPDATE = 1,
    CONNECTSTATE_UPDATE,
    ENABLE_UPDATE,
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

struct DevicePrivacyInfo {
    std::string deviceName;
    DeviceType deviceType;
    DeviceRole deviceRole;
    DeviceCategory deviceCategory;
    DeviceUsage deviceUsage;
};

template<typename T> bool MarshallingSetInt32(const std::set<T> &value, Parcel &parcel)
{
    size_t size = value.size();
    if (!parcel.WriteUint64(size)) {
        return false;
    }
    for (const auto &i : value) {
        if (!parcel.WriteInt32(i)) {
            return false;
        }
    }
    return true;
}

template<typename T> std::set<T> UnmarshallingSetInt32(Parcel &parcel,
    const size_t maxSize = std::numeric_limits<size_t>::max())
{
    size_t size = parcel.ReadUint64();
    // due to security concerns, sizelimit has been imposed
    if (size > maxSize) {
        size = maxSize;
    }

    std::set<T> res;
    for (size_t i = 0; i < size; i++) {
        res.insert(static_cast<T>(parcel.ReadInt32()));
    }
    return res;
}

struct DeviceStreamInfo {
    AudioEncodingType encoding = AudioEncodingType::ENCODING_PCM;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    AudioChannelLayout channelLayout  = AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    std::set<AudioSamplingRate> samplingRate;
    std::set<AudioChannel> channels;

    DeviceStreamInfo(AudioSamplingRate samplingRate_, AudioEncodingType encoding_, AudioSampleFormat format_,
        AudioChannel channels_) : encoding(encoding_), format(format_),
        samplingRate({samplingRate_}), channels({channels_})
    {}
    DeviceStreamInfo(AudioStreamInfo audioStreamInfo) : DeviceStreamInfo(audioStreamInfo.samplingRate,
        audioStreamInfo.encoding, audioStreamInfo.format, audioStreamInfo.channels)
    {}
    DeviceStreamInfo() = default;

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(encoding))
            && parcel.WriteInt32(static_cast<int32_t>(format))
            && MarshallingSetInt32(samplingRate, parcel)
            && MarshallingSetInt32(channels, parcel);
    }
    void Unmarshalling(Parcel &parcel)
    {
        encoding = static_cast<AudioEncodingType>(parcel.ReadInt32());
        format = static_cast<AudioSampleFormat>(parcel.ReadInt32());
        samplingRate = UnmarshallingSetInt32<AudioSamplingRate>(parcel, AUDIO_DEVICE_INFO_SIZE_LIMIT);
        channels = UnmarshallingSetInt32<AudioChannel>(parcel, AUDIO_DEVICE_INFO_SIZE_LIMIT);
    }

    bool CheckParams()
    {
        if (samplingRate.size() == 0) {
            return false;
        }
        if (channels.size() == 0) {
            return false;
        }
        return true;
    }
};

class DeviceInfo {
public:
    DeviceType deviceType;
    DeviceRole deviceRole;
    int32_t deviceId;
    int32_t channelMasks;
    int32_t channelIndexMasks;
    std::string deviceName;
    std::string macAddress;
    DeviceStreamInfo audioStreamInfo;
    std::string networkId;
    std::string displayName;
    int32_t interruptGroupId;
    int32_t volumeGroupId;
    bool isLowLatencyDevice;
    int32_t a2dpOffloadFlag;
    ConnectState connectState = CONNECTED;
    DeviceCategory deviceCategory = CATEGORY_DEFAULT;

    DeviceInfo() = default;
    ~DeviceInfo() = default;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(deviceType))
            && parcel.WriteInt32(static_cast<int32_t>(deviceRole))
            && parcel.WriteInt32(deviceId)
            && parcel.WriteInt32(channelMasks)
            && parcel.WriteInt32(channelIndexMasks)
            && parcel.WriteString(deviceName)
            && parcel.WriteString(macAddress)
            && audioStreamInfo.Marshalling(parcel)
            && parcel.WriteString(networkId)
            && parcel.WriteString(displayName)
            && parcel.WriteInt32(interruptGroupId)
            && parcel.WriteInt32(volumeGroupId)
            && parcel.WriteBool(isLowLatencyDevice)
            && parcel.WriteInt32(a2dpOffloadFlag)
            && parcel.WriteInt32(static_cast<int32_t>(deviceCategory));
    }
    bool Marshalling(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission, int32_t apiVersion) const
    {
        DeviceType devType = deviceType;
        int32_t devId = deviceId;
        DeviceStreamInfo streamInfo = audioStreamInfo;

        // If api target version < 11 && does not set deviceType, fix api compatibility.
        if (apiVersion < API_11 && (deviceType == DEVICE_TYPE_NONE || deviceType == DEVICE_TYPE_INVALID)) {
            // DeviceType use speaker or mic instead.
            if (deviceRole == OUTPUT_DEVICE) {
                devType = DEVICE_TYPE_SPEAKER;
                devId = 1; // 1 default speaker device id.
            } else if (deviceRole == INPUT_DEVICE) {
                devType = DEVICE_TYPE_MIC;
                devId = 2; // 2 default mic device id.
            }

            //If does not set sampleRates use SAMPLE_RATE_44100 instead.
            if (streamInfo.samplingRate.empty()) {
                streamInfo.samplingRate.insert(SAMPLE_RATE_44100);
            }
            // If does not set channelCounts use STEREO instead.
            if (streamInfo.channels.empty()) {
                streamInfo.channels.insert(STEREO);
            }
        }

        return parcel.WriteInt32(static_cast<int32_t>(devType))
            && parcel.WriteInt32(static_cast<int32_t>(deviceRole))
            && parcel.WriteInt32(devId)
            && parcel.WriteInt32(channelMasks)
            && parcel.WriteInt32(channelIndexMasks)
            && parcel.WriteString((!hasBTPermission && (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP
                || deviceType == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : deviceName)
            && parcel.WriteString((!hasBTPermission && (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP
                || deviceType == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : macAddress)
            && streamInfo.Marshalling(parcel)
            && parcel.WriteString(hasSystemPermission ? networkId : "")
            && parcel.WriteString(displayName)
            && parcel.WriteInt32(hasSystemPermission ? interruptGroupId : INVALID_GROUP_ID)
            && parcel.WriteInt32(hasSystemPermission ? volumeGroupId : INVALID_GROUP_ID)
            && parcel.WriteBool(isLowLatencyDevice)
            && parcel.WriteInt32(a2dpOffloadFlag)
            && parcel.WriteInt32(static_cast<int32_t>(deviceCategory));
    }
    void Unmarshalling(Parcel &parcel)
    {
        deviceType = static_cast<DeviceType>(parcel.ReadInt32());
        deviceRole = static_cast<DeviceRole>(parcel.ReadInt32());
        deviceId = parcel.ReadInt32();
        channelMasks = parcel.ReadInt32();
        channelIndexMasks = parcel.ReadInt32();
        deviceName = parcel.ReadString();
        macAddress = parcel.ReadString();
        audioStreamInfo.Unmarshalling(parcel);
        networkId = parcel.ReadString();
        displayName = parcel.ReadString();
        interruptGroupId = parcel.ReadInt32();
        volumeGroupId = parcel.ReadInt32();
        isLowLatencyDevice = parcel.ReadBool();
        a2dpOffloadFlag = parcel.ReadInt32();
        deviceCategory = static_cast<DeviceCategory>(parcel.ReadInt32());
    }
};

enum class AudioStreamDeviceChangeReason {
    UNKNOWN = 0,
    NEW_DEVICE_AVAILABLE = 1,
    OLD_DEVICE_UNAVALIABLE = 2,
    OVERRODE = 3
};

class AudioStreamDeviceChangeReasonExt {
public:
    enum class ExtEnum {
        UNKNOWN = 0,
        NEW_DEVICE_AVAILABLE = 1,
        OLD_DEVICE_UNAVALIABLE = 2,
        OVERRODE = 3,
        MIN = 1000,
        OLD_DEVICE_UNAVALIABLE_EXT = 1000,
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

    AudioStreamDeviceChangeReasonExt(const AudioStreamDeviceChangeReason &reason)
        : reason_(static_cast<ExtEnum>(reason)) {}

    AudioStreamDeviceChangeReasonExt(const ExtEnum &reason) : reason_(reason) {}

    bool IsOldDeviceUnavaliable() const
    {
        return ((reason_ == ExtEnum::OLD_DEVICE_UNAVALIABLE) || (reason_ == ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT));
    }

    bool isOverride() const
    {
        return reason_ == ExtEnum::OVERRODE;
    }

private:
    ExtEnum reason_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_INFO_H