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

#include "audio_service_log.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
AudioDeviceDescriptor::AudioDeviceDescriptor(int32_t descriptorType)
    : AudioDeviceDescriptor(DeviceType::DEVICE_TYPE_NONE, DeviceRole::DEVICE_ROLE_NONE)
{
    descriptorType_ = descriptorType;
    if (descriptorType_ == DEVICE_INFO) {
        deviceType_ = DeviceType(0);
        deviceRole_ = DeviceRole(0);
        networkId_ = "";
    }
}

AudioDeviceDescriptor::AudioDeviceDescriptor(DeviceType type, DeviceRole role)
    : deviceType_(type), deviceRole_(role)
{
    deviceId_ = 0;
    audioStreamInfo_ = {};
    channelMasks_ = 0;
    channelIndexMasks_ = 0;
    deviceName_ = "";
    macAddress_ = "";
    volumeGroupId_ = 0;
    interruptGroupId_ = 0;
    networkId_ = LOCAL_NETWORK_ID;
    displayName_ = "";
    deviceCategory_ = CATEGORY_DEFAULT;
    connectTimeStamp_ = 0;
    connectState_ = CONNECTED;
    pairDeviceDescriptor_ = nullptr;
    isScoRealConnected_ = false;
    isEnable_ = true;
    exceptionFlag_ = false;
    isLowLatencyDevice_ = false;
    a2dpOffloadFlag_ = 0;
    descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(DeviceType type, DeviceRole role, int32_t interruptGroupId,
    int32_t volumeGroupId, std::string networkId)
    : deviceType_(type), deviceRole_(role), interruptGroupId_(interruptGroupId), volumeGroupId_(volumeGroupId),
    networkId_(networkId)
{
    deviceId_ = 0;
    audioStreamInfo_ = {};
    channelMasks_ = 0;
    channelIndexMasks_ = 0;
    deviceName_ = "";
    macAddress_ = "";
    displayName_ = "";
    deviceCategory_ = CATEGORY_DEFAULT;
    connectTimeStamp_ = 0;
    connectState_ = CONNECTED;
    pairDeviceDescriptor_ = nullptr;
    isScoRealConnected_ = false;
    isEnable_ = true;
    exceptionFlag_ = false;
    isLowLatencyDevice_ = false;
    a2dpOffloadFlag_ = 0;
    descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor)
{
    deviceId_ = deviceDescriptor.deviceId_;
    deviceName_ = deviceDescriptor.deviceName_;
    macAddress_ = deviceDescriptor.macAddress_;
    deviceType_ = deviceDescriptor.deviceType_;
    deviceRole_ = deviceDescriptor.deviceRole_;
    audioStreamInfo_.channels = deviceDescriptor.audioStreamInfo_.channels;
    audioStreamInfo_.encoding = deviceDescriptor.audioStreamInfo_.encoding;
    audioStreamInfo_.format = deviceDescriptor.audioStreamInfo_.format;
    audioStreamInfo_.samplingRate = deviceDescriptor.audioStreamInfo_.samplingRate;
    channelMasks_ = deviceDescriptor.channelMasks_;
    channelIndexMasks_ = deviceDescriptor.channelIndexMasks_;
    volumeGroupId_ = deviceDescriptor.volumeGroupId_;
    interruptGroupId_ = deviceDescriptor.interruptGroupId_;
    networkId_ = deviceDescriptor.networkId_;
    displayName_ = deviceDescriptor.displayName_;
    deviceCategory_ = deviceDescriptor.deviceCategory_;
    connectTimeStamp_ = deviceDescriptor.connectTimeStamp_;
    connectState_ = deviceDescriptor.connectState_;
    pairDeviceDescriptor_ = deviceDescriptor.pairDeviceDescriptor_;
    isScoRealConnected_ = deviceDescriptor.isScoRealConnected_;
    isEnable_ = deviceDescriptor.isEnable_;
    exceptionFlag_ = deviceDescriptor.exceptionFlag_;
    // DeviceInfo
    isLowLatencyDevice_ = deviceDescriptor.isLowLatencyDevice_;
    a2dpOffloadFlag_ = deviceDescriptor.a2dpOffloadFlag_;
    // Other
    descriptorType_ = deviceDescriptor.descriptorType_;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const sptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "Error input parameter");
    deviceId_ = deviceDescriptor->deviceId_;
    deviceName_ = deviceDescriptor->deviceName_;
    macAddress_ = deviceDescriptor->macAddress_;
    deviceType_ = deviceDescriptor->deviceType_;
    deviceRole_ = deviceDescriptor->deviceRole_;
    audioStreamInfo_.channels = deviceDescriptor->audioStreamInfo_.channels;
    audioStreamInfo_.encoding = deviceDescriptor->audioStreamInfo_.encoding;
    audioStreamInfo_.format = deviceDescriptor->audioStreamInfo_.format;
    audioStreamInfo_.samplingRate = deviceDescriptor->audioStreamInfo_.samplingRate;
    channelMasks_ = deviceDescriptor->channelMasks_;
    channelIndexMasks_ = deviceDescriptor->channelIndexMasks_;
    volumeGroupId_ = deviceDescriptor->volumeGroupId_;
    interruptGroupId_ = deviceDescriptor->interruptGroupId_;
    networkId_ = deviceDescriptor->networkId_;
    displayName_ = deviceDescriptor->displayName_;
    deviceCategory_ = deviceDescriptor->deviceCategory_;
    connectTimeStamp_ = deviceDescriptor->connectTimeStamp_;
    connectState_ = deviceDescriptor->connectState_;
    pairDeviceDescriptor_ = deviceDescriptor->pairDeviceDescriptor_;
    isScoRealConnected_ = deviceDescriptor->isScoRealConnected_;
    isEnable_ = deviceDescriptor->isEnable_;
    exceptionFlag_ = deviceDescriptor->exceptionFlag_;
    // DeviceInfo
    isLowLatencyDevice_ = deviceDescriptor->isLowLatencyDevice_;
    a2dpOffloadFlag_ = deviceDescriptor->a2dpOffloadFlag_;
    // Other
    descriptorType_ = deviceDescriptor->descriptorType_;
}

AudioDeviceDescriptor::~AudioDeviceDescriptor()
{
    pairDeviceDescriptor_ = nullptr;
}

DeviceType AudioDeviceDescriptor::getType() const
{
    return deviceType_;
}

DeviceRole AudioDeviceDescriptor::getRole() const
{
    return deviceRole_;
}

bool AudioDeviceDescriptor::IsAudioDeviceDescriptor() const
{
    return descriptorType_ == AUDIO_DEVICE_DESCRIPTOR;
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel) const
{
    if (IsAudioDeviceDescriptor()) {
        return MarshallingToDeviceDescriptor(parcel);
    }

    return MarshallingToDeviceInfo(parcel);
}

bool AudioDeviceDescriptor::MarshallingToDeviceDescriptor(Parcel &parcel) const
{
    parcel.WriteInt32(deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET ? DEVICE_TYPE_USB_HEADSET : deviceType_);
    parcel.WriteInt32(deviceRole_);
    parcel.WriteInt32(deviceId_);
    audioStreamInfo_.Marshalling(parcel);
    parcel.WriteInt32(channelMasks_);
    parcel.WriteInt32(channelIndexMasks_);
    parcel.WriteString(deviceName_);
    parcel.WriteString(macAddress_);
    parcel.WriteInt32(interruptGroupId_);
    parcel.WriteInt32(volumeGroupId_);
    parcel.WriteString(networkId_);
    parcel.WriteString(displayName_);
    parcel.WriteInt32(deviceCategory_);
    return true;
}

bool AudioDeviceDescriptor::MarshallingToDeviceInfo(Parcel &parcel) const
{
    return parcel.WriteInt32(static_cast<int32_t>(deviceType_)) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceRole_)) &&
        parcel.WriteInt32(deviceId_) &&
        parcel.WriteInt32(channelMasks_) &&
        parcel.WriteInt32(channelIndexMasks_) &&
        parcel.WriteString(deviceName_) &&
        parcel.WriteString(macAddress_) &&
        audioStreamInfo_.Marshalling(parcel) &&
        parcel.WriteString(networkId_) &&
        parcel.WriteString(displayName_) &&
        parcel.WriteInt32(interruptGroupId_) &&
        parcel.WriteInt32(volumeGroupId_) &&
        parcel.WriteBool(isLowLatencyDevice_) &&
        parcel.WriteInt32(a2dpOffloadFlag_) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceCategory_));
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission,
    int32_t apiVersion) const
{
    return MarshallingToDeviceInfo(parcel, hasBTPermission, hasSystemPermission, apiVersion);
}

bool AudioDeviceDescriptor::MarshallingToDeviceInfo(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission,
    int32_t apiVersion) const
{
    DeviceType devType = deviceType_;
    int32_t devId = deviceId_;
    DeviceStreamInfo streamInfo = audioStreamInfo_;

    // If api target version < 11 && does not set deviceType, fix api compatibility.
    if (apiVersion < API_11 && (deviceType_ == DEVICE_TYPE_NONE || deviceType_ == DEVICE_TYPE_INVALID)) {
        // DeviceType use speaker or mic instead.
        if (deviceRole_ == OUTPUT_DEVICE) {
            devType = DEVICE_TYPE_SPEAKER;
            devId = 1; // 1 default speaker device id.
        } else if (deviceRole_ == INPUT_DEVICE) {
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

    return parcel.WriteInt32(static_cast<int32_t>(devType)) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceRole_)) &&
        parcel.WriteInt32(devId) &&
        parcel.WriteInt32(channelMasks_) &&
        parcel.WriteInt32(channelIndexMasks_) &&
        parcel.WriteString((!hasBTPermission && (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : deviceName_) &&
        parcel.WriteString((!hasBTPermission && (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : macAddress_) &&
        streamInfo.Marshalling(parcel) &&
        parcel.WriteString(hasSystemPermission ? networkId_ : "") &&
        parcel.WriteString(displayName_) &&
        parcel.WriteInt32(hasSystemPermission ? interruptGroupId_ : INVALID_GROUP_ID) &&
        parcel.WriteInt32(hasSystemPermission ? volumeGroupId_ : INVALID_GROUP_ID) &&
        parcel.WriteBool(isLowLatencyDevice_) &&
        parcel.WriteInt32(a2dpOffloadFlag_) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceCategory_));
}

void AudioDeviceDescriptor::Unmarshalling(Parcel &parcel)
{
    return UnmarshallingToDeviceInfo(parcel);
}

sptr<AudioDeviceDescriptor> AudioDeviceDescriptor::UnmarshallingPtr(Parcel &parcel)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (audioDeviceDescriptor == nullptr) {
        return nullptr;
    }

    audioDeviceDescriptor->UnmarshallingToDeviceDescriptor(parcel);
    return audioDeviceDescriptor;
}

void AudioDeviceDescriptor::UnmarshallingToDeviceDescriptor(Parcel &parcel)
{
    deviceType_ = static_cast<DeviceType>(parcel.ReadInt32());
    deviceRole_ = static_cast<DeviceRole>(parcel.ReadInt32());
    deviceId_ = parcel.ReadInt32();
    audioStreamInfo_.Unmarshalling(parcel);
    channelMasks_ = parcel.ReadInt32();
    channelIndexMasks_ = parcel.ReadInt32();
    deviceName_ = parcel.ReadString();
    macAddress_ = parcel.ReadString();
    interruptGroupId_ = parcel.ReadInt32();
    volumeGroupId_ = parcel.ReadInt32();
    networkId_ = parcel.ReadString();
    displayName_ = parcel.ReadString();
    deviceCategory_ = static_cast<DeviceCategory>(parcel.ReadInt32());
}

void AudioDeviceDescriptor::UnmarshallingToDeviceInfo(Parcel &parcel)
{
    deviceType_ = static_cast<DeviceType>(parcel.ReadInt32());
    deviceRole_ = static_cast<DeviceRole>(parcel.ReadInt32());
    deviceId_ = parcel.ReadInt32();
    channelMasks_ = parcel.ReadInt32();
    channelIndexMasks_ = parcel.ReadInt32();
    deviceName_ = parcel.ReadString();
    macAddress_ = parcel.ReadString();
    audioStreamInfo_.Unmarshalling(parcel);
    networkId_ = parcel.ReadString();
    displayName_ = parcel.ReadString();
    interruptGroupId_ = parcel.ReadInt32();
    volumeGroupId_ = parcel.ReadInt32();
    isLowLatencyDevice_ = parcel.ReadBool();
    a2dpOffloadFlag_ = parcel.ReadInt32();
    deviceCategory_ = static_cast<DeviceCategory>(parcel.ReadInt32());
}

void AudioDeviceDescriptor::SetDeviceInfo(std::string deviceName, std::string macAddress)
{
    deviceName_ = deviceName;
    macAddress_ = macAddress;
}

void AudioDeviceDescriptor::SetDeviceCapability(const DeviceStreamInfo &audioStreamInfo, int32_t channelMask,
    int32_t channelIndexMasks)
{
    audioStreamInfo_.channels = audioStreamInfo.channels;
    audioStreamInfo_.encoding = audioStreamInfo.encoding;
    audioStreamInfo_.format = audioStreamInfo.format;
    audioStreamInfo_.samplingRate = audioStreamInfo.samplingRate;
    channelMasks_ = channelMask;
    channelIndexMasks_ = channelIndexMasks;
}

bool AudioDeviceDescriptor::IsSameDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return deviceDescriptor.deviceType_ == deviceType_ &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_;
}

bool AudioDeviceDescriptor::IsSameDeviceInfo(const AudioDeviceDescriptor &deviceInfo) const
{
    return deviceType_ == deviceInfo.deviceType_ &&
        deviceRole_ == deviceInfo.deviceRole_ &&
        macAddress_ == deviceInfo.macAddress_ &&
        networkId_ == deviceInfo.networkId_;
}
} // AudioStandard
} // namespace OHOS
