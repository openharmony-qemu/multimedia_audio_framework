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

#include "audio_service_log.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief The AudioDeviceDescriptor provides
 *         different sets of audio devices and their roles
 */
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
}

AudioDeviceDescriptor::AudioDeviceDescriptor(DeviceType type, DeviceRole role) : deviceType_(type), deviceRole_(role)
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
}

AudioDeviceDescriptor::AudioDeviceDescriptor()
    : AudioDeviceDescriptor(DeviceType::DEVICE_TYPE_NONE, DeviceRole::DEVICE_ROLE_NONE)
{}

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
}

AudioDeviceDescriptor::~AudioDeviceDescriptor()
{
    pairDeviceDescriptor_ = nullptr;
}

DeviceType AudioDeviceDescriptor::getType()
{
    return deviceType_;
}

DeviceRole AudioDeviceDescriptor::getRole() const
{
    return deviceRole_;
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel) const
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

sptr<AudioDeviceDescriptor> AudioDeviceDescriptor::Unmarshalling(Parcel &in)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (audioDeviceDescriptor == nullptr) {
        return nullptr;
    }

    audioDeviceDescriptor->deviceType_ = static_cast<DeviceType>(in.ReadInt32());
    audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(in.ReadInt32());
    audioDeviceDescriptor->deviceId_ = in.ReadInt32();

    audioDeviceDescriptor->audioStreamInfo_.Unmarshalling(in);

    audioDeviceDescriptor->channelMasks_ = in.ReadInt32();
    audioDeviceDescriptor->channelIndexMasks_ = in.ReadInt32();

    audioDeviceDescriptor->deviceName_ = in.ReadString();
    audioDeviceDescriptor->macAddress_ = in.ReadString();

    audioDeviceDescriptor->interruptGroupId_ = in.ReadInt32();
    audioDeviceDescriptor->volumeGroupId_ = in.ReadInt32();
    audioDeviceDescriptor->networkId_ = in.ReadString();
    audioDeviceDescriptor->displayName_ = in.ReadString();

    audioDeviceDescriptor->deviceCategory_ = static_cast<DeviceCategory>(in.ReadInt32());

    return audioDeviceDescriptor;
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

bool AudioDeviceDescriptor::isSameDevice(const DeviceInfo &deviceInfo)
{
    return deviceInfo.deviceType == deviceType_ &&
        deviceInfo.macAddress == macAddress_ &&
        deviceInfo.networkId == networkId_;
}

bool AudioDeviceDescriptor::isSameDeviceDesc(const std::unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "Invalid device descriptor");
    return deviceDescriptor->deviceType_ == deviceType_ &&
        deviceDescriptor->macAddress_ == macAddress_ &&
        deviceDescriptor->networkId_ == networkId_;
}

AudioRendererFilter::AudioRendererFilter()
{}

AudioRendererFilter::~AudioRendererFilter()
{}

bool AudioRendererFilter::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(uid)
        && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.contentType))
        && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.streamUsage))
        && parcel.WriteInt32(static_cast<int32_t>(streamType))
        && parcel.WriteInt32(rendererInfo.rendererFlags)
        && parcel.WriteInt32(streamId);
}

sptr<AudioRendererFilter> AudioRendererFilter::Unmarshalling(Parcel &in)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {
        return nullptr;
    }

    audioRendererFilter->uid = in.ReadInt32();
    audioRendererFilter->rendererInfo.contentType = static_cast<ContentType>(in.ReadInt32());
    audioRendererFilter->rendererInfo.streamUsage = static_cast<StreamUsage>(in.ReadInt32());
    audioRendererFilter->streamType = static_cast<AudioStreamType>(in.ReadInt32());
    audioRendererFilter->rendererInfo.rendererFlags = in.ReadInt32();
    audioRendererFilter->streamId = in.ReadInt32();

    return audioRendererFilter;
}

AudioCapturerFilter::AudioCapturerFilter()
{}

AudioCapturerFilter::~AudioCapturerFilter()
{}

bool AudioCapturerFilter::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(uid)
        && parcel.WriteInt32(static_cast<int32_t>(capturerInfo.sourceType))
        && parcel.WriteInt32(capturerInfo.capturerFlags);
}

sptr<AudioCapturerFilter> AudioCapturerFilter::Unmarshalling(Parcel &in)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET(audioCapturerFilter != nullptr, nullptr);

    audioCapturerFilter->uid = in.ReadInt32();
    audioCapturerFilter->capturerInfo.sourceType = static_cast<SourceType>(in.ReadInt32());
    audioCapturerFilter->capturerInfo.capturerFlags = in.ReadInt32();

    return audioCapturerFilter;
}
} // namespace AudioStandard
} // namespace OHOS
