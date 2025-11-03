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

#include "audio_device_descriptor.h"

#include <cinttypes>
#include "audio_service_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t API_VERSION_18 = 18;
constexpr int32_t API_VERSION_20 = 20;

const std::map<DeviceType, std::string> deviceTypeStringMap = {
    {DEVICE_TYPE_INVALID, "INVALID"},
    {DEVICE_TYPE_EARPIECE, "EARPIECE"},
    {DEVICE_TYPE_SPEAKER, "SPEAKER"},
    {DEVICE_TYPE_WIRED_HEADSET, "WIRED_HEADSET"},
    {DEVICE_TYPE_WIRED_HEADPHONES, "WIRED_HEADPHONES"},
    {DEVICE_TYPE_BLUETOOTH_SCO, "BLUETOOTH_SCO"},
    {DEVICE_TYPE_BLUETOOTH_A2DP, "BLUETOOTH_A2DP"},
    {DEVICE_TYPE_BLUETOOTH_A2DP_IN, "BLUETOOTH_A2DP_IN"},
    {DEVICE_TYPE_HEARING_AID, "HEARING_AID"},
    {DEVICE_TYPE_NEARLINK, "NEARLINK"},
    {DEVICE_TYPE_NEARLINK_IN, "NEARLINK_IN"},
    {DEVICE_TYPE_BT_SPP, "BT_SPP"},
    {DEVICE_TYPE_NEARLINK_PORT, "NEARLINK_PORT"},
    {DEVICE_TYPE_MIC, "MIC"},
    {DEVICE_TYPE_WAKEUP, "WAKEUP"},
    {DEVICE_TYPE_USB_HEADSET, "USB_HEADSET"},
    {DEVICE_TYPE_DP, "DP"},
    {DEVICE_TYPE_REMOTE_CAST, "REMOTE_CAST"},
    {DEVICE_TYPE_USB_DEVICE, "USB_DEVICE"},
    {DEVICE_TYPE_ACCESSORY, "ACCESSORY"},
    {DEVICE_TYPE_REMOTE_DAUDIO, "REMOTE_DAUDIO"},
    {DEVICE_TYPE_HDMI, "HDMI"},
    {DEVICE_TYPE_LINE_DIGITAL, "LINE_DIGITAL"},
    {DEVICE_TYPE_FILE_SINK, "FILE_SINK"},
    {DEVICE_TYPE_FILE_SOURCE, "FILE_SOURCE"},
    {DEVICE_TYPE_EXTERN_CABLE, "EXTERN_CABLE"},
    {DEVICE_TYPE_SYSTEM_PRIVATE, "SYSTEM_PRIVATE"},
    {DEVICE_TYPE_DEFAULT, "DEFAULT"},
    {DEVICE_TYPE_USB_ARM_HEADSET, "USB_ARM_HEADSET"}
};

const DeviceStreamInfo DEFAULT_DEVICE_STREAM_INFO(SAMPLE_RATE_44100, ENCODING_PCM, AudioSampleFormat::INVALID_WIDTH,
    CH_LAYOUT_STEREO);

static const char *DeviceTypeToString(DeviceType type)
{
    if (deviceTypeStringMap.count(type) != 0) {
        return deviceTypeStringMap.at(type).c_str();
    }
    return "UNKNOWN";
}

static void CheckDeviceInfoSize(size_t &size)
{
    CHECK_AND_RETURN(size > AUDIO_DEVICE_INFO_SIZE_LIMIT);
    size = AUDIO_DEVICE_INFO_SIZE_LIMIT;
}

static bool MarshallingDeviceStreamInfoList(const std::list<DeviceStreamInfo> &deviceStreamInfos, Parcel &parcel)
{
    size_t size = deviceStreamInfos.size();
    CHECK_AND_RETURN_RET(parcel.WriteUint64(size), false);

    for (const auto &deviceStreamInfo : deviceStreamInfos) {
        CHECK_AND_RETURN_RET(deviceStreamInfo.Marshalling(parcel), false);
    }
    return true;
}

static void UnmarshallingDeviceStreamInfoList(Parcel &parcel, std::list<DeviceStreamInfo> &deviceStreamInfos)
{
    size_t size = parcel.ReadUint64();
    // due to security concerns, sizelimit has been imposed
    CheckDeviceInfoSize(size);

    for (size_t i = 0; i < size; i++) {
        DeviceStreamInfo deviceStreamInfo;
        deviceStreamInfo.Unmarshalling(parcel);
        deviceStreamInfos.push_back(deviceStreamInfo);
    }
}

static void SetDefaultStreamInfoIfEmpty(std::list<DeviceStreamInfo> &streamInfo)
{
    if (streamInfo.empty()) {
        streamInfo.push_back(DEFAULT_DEVICE_STREAM_INFO);
    } else {
        for (auto &info : streamInfo) {
            // If does not set sampleRates use SAMPLE_RATE_44100 instead.
            if (info.samplingRate.empty()) {
                info.samplingRate = DEFAULT_DEVICE_STREAM_INFO.samplingRate;
            }
            // If does not set channelCounts use STEREO instead.
            if (info.channelLayout.empty()) {
                info.channelLayout = DEFAULT_DEVICE_STREAM_INFO.channelLayout;
            }
        }
    }
}

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
    spatializationSupported_ = false;
    isVrSupported_ = true;
    modemCallSupported_ = true;
    highQualityRecordingSupported_ = false;
}

AudioDeviceDescriptor::~AudioDeviceDescriptor()
{
    pairDeviceDescriptor_ = nullptr;
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
    spatializationSupported_ = false;
    isVrSupported_ = true;
    modemCallSupported_ = true;
    highQualityRecordingSupported_ = false;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor)
{
    deviceId_ = deviceDescriptor.deviceId_;
    deviceName_ = deviceDescriptor.deviceName_;
    macAddress_ = deviceDescriptor.macAddress_;
    deviceType_ = deviceDescriptor.deviceType_;
    deviceRole_ = deviceDescriptor.deviceRole_;
    audioStreamInfo_ = deviceDescriptor.audioStreamInfo_;
    channelMasks_ = deviceDescriptor.channelMasks_;
    channelIndexMasks_ = deviceDescriptor.channelIndexMasks_;
    volumeGroupId_ = deviceDescriptor.volumeGroupId_;
    interruptGroupId_ = deviceDescriptor.interruptGroupId_;
    networkId_ = deviceDescriptor.networkId_;
    dmDeviceType_ = deviceDescriptor.dmDeviceType_;
    displayName_ = deviceDescriptor.displayName_;
    deviceCategory_ = deviceDescriptor.deviceCategory_;
    connectTimeStamp_ = deviceDescriptor.connectTimeStamp_;
    connectState_ = deviceDescriptor.connectState_;
    pairDeviceDescriptor_ = deviceDescriptor.pairDeviceDescriptor_;
    isScoRealConnected_ = deviceDescriptor.isScoRealConnected_;
    isEnable_ = deviceDescriptor.isEnable_;
    exceptionFlag_ = deviceDescriptor.exceptionFlag_;
    deviceUsage_ = deviceDescriptor.deviceUsage_;
    // DeviceInfo
    isLowLatencyDevice_ = deviceDescriptor.isLowLatencyDevice_;
    a2dpOffloadFlag_ = deviceDescriptor.a2dpOffloadFlag_;
    // Other
    descriptorType_ = deviceDescriptor.descriptorType_;
    hasPair_ = deviceDescriptor.hasPair_;
    spatializationSupported_ = deviceDescriptor.spatializationSupported_;
    isVrSupported_ = deviceDescriptor.isVrSupported_;
    clientInfo_ = deviceDescriptor.clientInfo_;
    modemCallSupported_ = deviceDescriptor.modemCallSupported_;
    highQualityRecordingSupported_ = deviceDescriptor.highQualityRecordingSupported_;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "Error input parameter");
    deviceId_ = deviceDescriptor->deviceId_;
    deviceName_ = deviceDescriptor->deviceName_;
    macAddress_ = deviceDescriptor->macAddress_;
    deviceType_ = deviceDescriptor->deviceType_;
    deviceRole_ = deviceDescriptor->deviceRole_;
    audioStreamInfo_ = deviceDescriptor->audioStreamInfo_;
    channelMasks_ = deviceDescriptor->channelMasks_;
    channelIndexMasks_ = deviceDescriptor->channelIndexMasks_;
    volumeGroupId_ = deviceDescriptor->volumeGroupId_;
    interruptGroupId_ = deviceDescriptor->interruptGroupId_;
    networkId_ = deviceDescriptor->networkId_;
    dmDeviceType_ = deviceDescriptor->dmDeviceType_;
    displayName_ = deviceDescriptor->displayName_;
    deviceCategory_ = deviceDescriptor->deviceCategory_;
    connectTimeStamp_ = deviceDescriptor->connectTimeStamp_;
    connectState_ = deviceDescriptor->connectState_;
    pairDeviceDescriptor_ = deviceDescriptor->pairDeviceDescriptor_;
    isScoRealConnected_ = deviceDescriptor->isScoRealConnected_;
    isEnable_ = deviceDescriptor->isEnable_;
    exceptionFlag_ = deviceDescriptor->exceptionFlag_;
    deviceUsage_ = deviceDescriptor->deviceUsage_;
    // DeviceInfo
    isLowLatencyDevice_ = deviceDescriptor->isLowLatencyDevice_;
    a2dpOffloadFlag_ = deviceDescriptor->a2dpOffloadFlag_;
    // Other
    descriptorType_ = deviceDescriptor->descriptorType_;
    hasPair_ = deviceDescriptor->hasPair_;
    spatializationSupported_ = deviceDescriptor->spatializationSupported_;
    isVrSupported_ = deviceDescriptor->isVrSupported_;
    clientInfo_ = deviceDescriptor->clientInfo_;
    modemCallSupported_ = deviceDescriptor->modemCallSupported_;
    highQualityRecordingSupported_ = deviceDescriptor->highQualityRecordingSupported_;
}

DeviceType AudioDeviceDescriptor::getType() const
{
    return deviceType_;
}

DeviceRole AudioDeviceDescriptor::getRole() const
{
    return deviceRole_;
}

DeviceCategory AudioDeviceDescriptor::GetDeviceCategory() const
{
    return deviceCategory_;
}

bool AudioDeviceDescriptor::IsAudioDeviceDescriptor() const
{
    return descriptorType_ == AUDIO_DEVICE_DESCRIPTOR;
}

void AudioDeviceDescriptor::SetClientInfo(const ClientInfo &clientInfo) const
{
    clientInfo_ = clientInfo;
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel) const
{
    bool ret = MarshallingInner(parcel);
    clientInfo_ = std::nullopt;
    return ret;
}

bool AudioDeviceDescriptor::MarshallingInner(Parcel &parcel) const
{
    if (clientInfo_ && !IsAudioDeviceDescriptor()) {
        return MarshallingToDeviceInfo(parcel, clientInfo_.value().hasBTPermission_,
            clientInfo_.value().hasSystemPermission_, clientInfo_.value().apiVersion_,
            clientInfo_.value().isSupportedNearlink_);
    }

    int32_t apiVersion = 0;
    bool isSupportedNearlink = true;
    if (clientInfo_) {
        apiVersion = clientInfo_.value().apiVersion_;
        isSupportedNearlink = clientInfo_.value().isSupportedNearlink_;
    }
    int32_t devType = deviceType_;
    if (IsAudioDeviceDescriptor()) {
        devType = MapInternalToExternalDeviceType(apiVersion, isSupportedNearlink);
    }

    return  parcel.WriteInt32(devType) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceRole_)) &&
        parcel.WriteInt32(deviceId_) &&
        parcel.WriteInt32(channelMasks_) &&
        parcel.WriteInt32(channelIndexMasks_) &&
        parcel.WriteString(deviceName_) &&
        parcel.WriteString(macAddress_) &&
        parcel.WriteInt32(interruptGroupId_) &&
        parcel.WriteInt32(volumeGroupId_) &&
        parcel.WriteString(networkId_) &&
        parcel.WriteUint16(dmDeviceType_) &&
        parcel.WriteString(displayName_) &&
        MarshallingDeviceStreamInfoList(audioStreamInfo_, parcel) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceCategory_)) &&
        parcel.WriteInt32(static_cast<int32_t>(connectState_)) &&
        parcel.WriteBool(exceptionFlag_) &&
        parcel.WriteInt64(connectTimeStamp_) &&
        parcel.WriteBool(isScoRealConnected_) &&
        parcel.WriteBool(isEnable_) &&
        parcel.WriteInt32(mediaVolume_) &&
        parcel.WriteInt32(callVolume_) &&
        parcel.WriteBool(isLowLatencyDevice_) &&
        parcel.WriteInt32(a2dpOffloadFlag_) &&
        parcel.WriteBool(descriptorType_) &&
        parcel.WriteBool(spatializationSupported_) &&
        parcel.WriteBool(hasPair_) &&
        parcel.WriteInt32(routerType_) &&
        parcel.WriteInt32(isVrSupported_) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceUsage_)) &&
        parcel.WriteBool(modemCallSupported_) &&
        parcel.WriteBool(highQualityRecordingSupported_);
}

void AudioDeviceDescriptor::FixApiCompatibility(int apiVersion, DeviceRole deviceRole,
    DeviceType &deviceType, int32_t &deviceId, std::list<DeviceStreamInfo> &streamInfo)
{
    // If api target version < 11 && does not set deviceType, fix api compatibility.
    if (apiVersion < API_11 && (deviceType == DEVICE_TYPE_NONE || deviceType == DEVICE_TYPE_INVALID)) {
        // DeviceType use speaker or mic instead.
        if (deviceRole == OUTPUT_DEVICE) {
            deviceType = DEVICE_TYPE_SPEAKER;
            deviceId = 1; // 1 default speaker device id.
        } else if (deviceRole == INPUT_DEVICE) {
            deviceType = DEVICE_TYPE_MIC;
            deviceId = 2; // 2 default mic device id.
        }

        SetDefaultStreamInfoIfEmpty(streamInfo);
    }
}

bool AudioDeviceDescriptor::MarshallingToDeviceInfo(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission,
    int32_t apiVersion, bool isSupportedNearlink) const
{
    DeviceType devType = MapInternalToExternalDeviceType(apiVersion, isSupportedNearlink);
    int32_t devId = deviceId_;
    std::list<DeviceStreamInfo> streamInfo = audioStreamInfo_;

    FixApiCompatibility(apiVersion, deviceRole_, devType, devId, streamInfo);

    return parcel.WriteInt32(static_cast<int32_t>(devType)) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceRole_)) &&
        parcel.WriteInt32(devId) &&
        parcel.WriteInt32(channelMasks_) &&
        parcel.WriteInt32(channelIndexMasks_) &&
        parcel.WriteString((!hasBTPermission && (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : deviceName_) &&
        parcel.WriteString((!hasBTPermission && (deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP ||
            deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) ? "" : macAddress_) &&
        parcel.WriteInt32(hasSystemPermission ? interruptGroupId_ : INVALID_GROUP_ID) &&
        parcel.WriteInt32(hasSystemPermission ? volumeGroupId_ : INVALID_GROUP_ID) &&
        parcel.WriteString(hasSystemPermission ? networkId_ : "") &&
        parcel.WriteUint16(dmDeviceType_) &&
        parcel.WriteString(displayName_) &&
        MarshallingDeviceStreamInfoList(streamInfo, parcel) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceCategory_)) &&
        parcel.WriteInt32(static_cast<int32_t>(connectState_)) &&
        parcel.WriteBool(exceptionFlag_) &&
        parcel.WriteInt64(connectTimeStamp_) &&
        parcel.WriteBool(isScoRealConnected_) &&
        parcel.WriteBool(isEnable_) &&
        parcel.WriteInt32(mediaVolume_) &&
        parcel.WriteInt32(callVolume_) &&
        parcel.WriteBool(isLowLatencyDevice_) &&
        parcel.WriteInt32(a2dpOffloadFlag_) &&
        parcel.WriteBool(descriptorType_) &&
        parcel.WriteBool(spatializationSupported_) &&
        parcel.WriteBool(hasPair_) &&
        parcel.WriteInt32(routerType_) &&
        parcel.WriteInt32(isVrSupported_) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceUsage_)) &&
        parcel.WriteBool(modemCallSupported_) &&
        parcel.WriteBool(highQualityRecordingSupported_);
}

void AudioDeviceDescriptor::UnmarshallingSelf(Parcel &parcel)
{
    deviceType_ = static_cast<DeviceType>(parcel.ReadInt32());
    deviceRole_ = static_cast<DeviceRole>(parcel.ReadInt32());
    deviceId_ = parcel.ReadInt32();
    channelMasks_ = parcel.ReadInt32();
    channelIndexMasks_ = parcel.ReadInt32();
    deviceName_ = parcel.ReadString();
    macAddress_ = parcel.ReadString();
    interruptGroupId_ = parcel.ReadInt32();
    volumeGroupId_ = parcel.ReadInt32();
    networkId_ = parcel.ReadString();
    dmDeviceType_ = parcel.ReadUint16();
    displayName_ = parcel.ReadString();
    UnmarshallingDeviceStreamInfoList(parcel, audioStreamInfo_);
    deviceCategory_ = static_cast<DeviceCategory>(parcel.ReadInt32());
    connectState_ = static_cast<ConnectState>(parcel.ReadInt32());
    exceptionFlag_ = parcel.ReadBool();
    connectTimeStamp_ = parcel.ReadInt64();
    isScoRealConnected_ = parcel.ReadBool();
    isEnable_ = parcel.ReadBool();
    mediaVolume_ = parcel.ReadInt32();
    callVolume_ = parcel.ReadInt32();
    isLowLatencyDevice_ = parcel.ReadBool();
    a2dpOffloadFlag_ = parcel.ReadInt32();
    descriptorType_ = parcel.ReadBool();
    spatializationSupported_ = parcel.ReadBool();
    hasPair_ = parcel.ReadBool();
    routerType_ = static_cast<RouterType>(parcel.ReadInt32());
    isVrSupported_ = parcel.ReadInt32();
    deviceUsage_ = static_cast<DeviceUsage>(parcel.ReadInt32());
    modemCallSupported_ = parcel.ReadBool();
    highQualityRecordingSupported_ = parcel.ReadBool();
}

AudioDeviceDescriptor *AudioDeviceDescriptor::Unmarshalling(Parcel &parcel)
{
    auto deviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (deviceDescriptor == nullptr) {
        return nullptr;
    }

    deviceDescriptor->UnmarshallingSelf(parcel);
    return deviceDescriptor;
}

void AudioDeviceDescriptor::MapInputDeviceType(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    for (int index = static_cast<int>(descs.size()) - 1; index >= 0; index--) {
        if ((descs[index]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP &&
            descs[index]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP_IN) ||
            descs[index]->deviceRole_ != INPUT_DEVICE) {
            continue;
        }
        bool hasSame = false;
        for (auto& otherDesc : descs) {
            if (otherDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                descs[index]->macAddress_ != "" &&
                otherDesc->macAddress_ == descs[index]->macAddress_ &&
                otherDesc->deviceRole_ == descs[index]->deviceRole_ &&
                otherDesc->networkId_ == descs[index]->networkId_) {
                hasSame = true;
                otherDesc->highQualityRecordingSupported_ = descs[index]->highQualityRecordingSupported_;
                break;
            }
        }

        if (hasSame) {
            descs.erase(descs.begin() + index);
        } else {
            descs[index]->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
        }
    }
}

void AudioDeviceDescriptor::SetDeviceInfo(std::string deviceName, std::string macAddress)
{
    deviceName_ = deviceName;
    macAddress_ = macAddress;
}

void AudioDeviceDescriptor::SetDeviceCapability(const std::list<DeviceStreamInfo> &audioStreamInfo, int32_t channelMask,
    int32_t channelIndexMasks)
{
    audioStreamInfo_ = audioStreamInfo;
    channelMasks_ = channelMask;
    channelIndexMasks_ = channelIndexMasks;
}

bool AudioDeviceDescriptor::IsSameDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return deviceDescriptor.deviceType_ == deviceType_ &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_ &&
        (!IsUsb(deviceType_) || deviceDescriptor.deviceRole_ == deviceRole_);
}

bool AudioDeviceDescriptor::IsSameDeviceDescPtr(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor) const
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "input deviceDescriptor is null");
    return deviceDescriptor->deviceType_ == deviceType_ &&
        deviceDescriptor->macAddress_ == macAddress_ &&
        deviceDescriptor->networkId_ == networkId_ &&
        (!IsUsb(deviceType_) || deviceDescriptor->deviceRole_ == deviceRole_);
}

bool AudioDeviceDescriptor::IsSameDeviceInfo(const AudioDeviceDescriptor &deviceInfo) const
{
    return deviceType_ == deviceInfo.deviceType_ &&
        deviceRole_ == deviceInfo.deviceRole_ &&
        macAddress_ == deviceInfo.macAddress_ &&
        networkId_ == deviceInfo.networkId_;
}

bool AudioDeviceDescriptor::IsPairedDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return ((deviceDescriptor.deviceRole_ == INPUT_DEVICE && deviceRole_ == OUTPUT_DEVICE) ||
        (deviceDescriptor.deviceRole_ == OUTPUT_DEVICE && deviceRole_ == INPUT_DEVICE)) &&
        (deviceDescriptor.deviceType_ == deviceType_ ||
            (IsNearlinkDevice(deviceDescriptor.deviceType_) && IsNearlinkDevice(deviceType_))) &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_;
}

bool AudioDeviceDescriptor::IsDistributedSpeaker() const
{
    return deviceType_ == DEVICE_TYPE_SPEAKER && networkId_ != LOCAL_NETWORK_ID;
}

bool AudioDeviceDescriptor::IsA2dpOffload() const
{
    return deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && a2dpOffloadFlag_ == A2DP_OFFLOAD;
}


bool AudioDeviceDescriptor::IsSpeakerOrEarpiece() const
{
    return (networkId_ == LOCAL_NETWORK_ID && deviceType_ == DEVICE_TYPE_SPEAKER) ||
        deviceType_ == DEVICE_TYPE_EARPIECE;
}

bool AudioDeviceDescriptor::IsRemote() const
{
    return networkId_ != LOCAL_NETWORK_ID;
}

bool AudioDeviceDescriptor::IsRemoteDevice() const
{
    return networkId_ != LOCAL_NETWORK_ID || deviceType_ == DEVICE_TYPE_REMOTE_CAST;
}

void AudioDeviceDescriptor::Dump(std::string &dumpString)
{
    AppendFormat(dumpString, "      - device %d: role %s type %d (%s) name: %s\n",
        deviceId_, IsOutput() ? "Output" : "Input",
        deviceType_, DeviceTypeToString(deviceType_), deviceName_.c_str());
}

std::string AudioDeviceDescriptor::GetName()
{
    if (networkId_ != LOCAL_NETWORK_ID && deviceType_ == DEVICE_TYPE_SPEAKER) {
        return "DMSDP";
    }
    return GetDeviceTypeString();
}

std::string AudioDeviceDescriptor::GetDeviceTypeString()
{
    return std::string(DeviceTypeToString(deviceType_));
}

std::string AudioDeviceDescriptor::GetKey()
{
    return networkId_ + "_" + std::to_string(deviceType_);
}

DeviceType AudioDeviceDescriptor::MapInternalToExternalDeviceType(int32_t apiVersion, bool isSupportedNearlink) const
{
    switch (deviceType_) {
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (!hasPair_ && apiVersion >= API_VERSION_18) {
#ifdef DETECT_SOUNDBOX
                return DEVICE_TYPE_USB_DEVICE;
#else
                if (deviceRole_ == INPUT_DEVICE) {
                    return DEVICE_TYPE_USB_DEVICE;
                }
#endif
            }
            return DEVICE_TYPE_USB_HEADSET;
        case DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            return DEVICE_TYPE_BLUETOOTH_A2DP;
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_NEARLINK_IN:
            if (apiVersion < API_VERSION_20 || !isSupportedNearlink) {
                return DEVICE_TYPE_BLUETOOTH_SCO;
            }
            return DEVICE_TYPE_NEARLINK;
        default:
            return deviceType_;
    }
}

DeviceStreamInfo AudioDeviceDescriptor::GetDeviceStreamInfo(void) const
{
    DeviceStreamInfo streamInfo;
    CHECK_AND_RETURN_RET_LOG(!audioStreamInfo_.empty(), streamInfo, "streamInfo empty");
    return *audioStreamInfo_.rbegin();
}
} // AudioStandard
} // namespace OHOS
