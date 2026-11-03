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

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B8B
namespace OHOS {
namespace AudioStandard {
constexpr int32_t API_VERSION_18 = 18;
constexpr size_t KV_SIZE = 2;
constexpr uint32_t ONLY_SUPPORT_MMAP = 2;

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
    {DEVICE_TYPE_WIRED_MICIN, "WIRED_MICIN"},
    {DEVICE_TYPE_DEFAULT, "DEFAULT"},
    {DEVICE_TYPE_USB_ARM_HEADSET, "USB_ARM_HEADSET"}
};

const DeviceStreamInfo DEFAULT_DEVICE_STREAM_INFO(SAMPLE_RATE_44100, ENCODING_PCM, AudioSampleFormat::INVALID_WIDTH,
    CH_LAYOUT_STEREO);
const AudioStreamInfo DEFAULT_AUDIO_STREAM_INFO(AudioSamplingRate::SAMPLE_RATE_48000, AudioEncodingType::ENCODING_PCM,
    AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);

static const char *DeviceTypeToString(DeviceType type)
{
    if (deviceTypeStringMap.count(type) != 0) {
        return deviceTypeStringMap.at(type).c_str();
    }
    return "UNKNOWN";
}

static bool ConvertToUint32(const std::string &str, uint32_t &result)
{
    if (str == "-0") {
        result = 0;
        return true;
    }
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

static std::string ParseAudioFormat(std::string format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return "s16le";
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return "s24le";
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return "s32le";
    } else {
        return "s16le";
    }
}

static std::map<std::string, AudioSampleFormat> g_formatStrToEnum = {
    {"s8", SAMPLE_U8},
    {"s16", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
    {"s16le", SAMPLE_S16LE},
    {"s24le", SAMPLE_S24LE},
    {"s32le", SAMPLE_S32LE},
};

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

static bool MarshallingAudioStreamInfoList(const std::list<AudioStreamInfo> &audioStreamInfos, Parcel &parcel)
{
    size_t size = audioStreamInfos.size();
    CHECK_AND_RETURN_RET(parcel.WriteUint64(size), false);

    for (const auto &audioStreamInfo : audioStreamInfos) {
        CHECK_AND_RETURN_RET(audioStreamInfo.Marshalling(parcel), false);
    }
    return true;
}

static void UnmarshallingAudioStreamInfoList(Parcel &parcel, std::list<AudioStreamInfo> &audioStreamInfos)
{
    size_t size = parcel.ReadUint64();
    // due to security concerns, sizelimit has been imposed
    CheckDeviceInfoSize(size);

    for (size_t i = 0; i < size; i++) {
        AudioStreamInfo audioStreamInfo;
        audioStreamInfo.UnmarshallingSelf(parcel);
        audioStreamInfos.push_back(audioStreamInfo);
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
    capabilities_ = {};
    channelMasks_ = 0;
    channelIndexMasks_ = 0;
    deviceName_ = "";
    macAddress_ = "";
    volumeGroupId_ = 0;
    interruptGroupId_ = 0;
    networkId_ = LOCAL_NETWORK_ID;
    displayName_ = "";
    model_ = "unknown";
    deviceCategory_ = CATEGORY_DEFAULT;
    connectTimeStamp_ = 0;
    connectState_ = CONNECTED;
    pairDeviceDescriptor_ = nullptr;
    isScoRealConnected_ = false;
    isEnable_ = true;
    isExclusiveMicScoUnavailable_ = false;
    exceptionFlag_ = false;
    isLowLatencyDevice_ = false;
    a2dpOffloadFlag_ = 0;
    descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
    spatializationSupported_ = false;
    isVrSupported_ = true;
    isSupportWearDetectionEnable_ = false;
    modemCallSupported_ = true;
    highQualityRecordingSupported_ = false;
    isUnidirectional_ = false;
    dmDeviceInfo_ = "";
    hdPlaybackMode_ = HD_NOT_SUPPORTED;
    isSupportRemoteMultichannel_ = true;
    deviceTokenId_ = 0;
    isSupportMultiZone_ = false;
    visibleMode_ = GLOBAL_VISIBLE;
    sinkLatencyMs_ = 0;
    sinkLatencyValid_ = false;
    isSupportReload_ = true;
    isSupportCommunicationSharing_ = 0;
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
    capabilities_ = {};
    channelMasks_ = 0;
    channelIndexMasks_ = 0;
    deviceName_ = "";
    macAddress_ = "";
    displayName_ = "";
    model_ = "unknown";
    deviceCategory_ = CATEGORY_DEFAULT;
    connectTimeStamp_ = 0;
    connectState_ = CONNECTED;
    pairDeviceDescriptor_ = nullptr;
    isScoRealConnected_ = false;
    isEnable_ = true;
    isExclusiveMicScoUnavailable_ = false;
    exceptionFlag_ = false;
    isLowLatencyDevice_ = false;
    a2dpOffloadFlag_ = 0;
    descriptorType_ = AUDIO_DEVICE_DESCRIPTOR;
    spatializationSupported_ = false;
    isVrSupported_ = true;
    isSupportWearDetectionEnable_ = false;
    modemCallSupported_ = true;
    highQualityRecordingSupported_ = false;
    isUnidirectional_ = false;
    dmDeviceInfo_ = "";
    hdPlaybackMode_ = HD_NOT_SUPPORTED;
    isSupportRemoteMultichannel_ = true;
    deviceTokenId_ = 0;
    isSupportMultiZone_ = false;
    visibleMode_ = GLOBAL_VISIBLE;
    sinkLatencyMs_ = 0;
    sinkLatencyValid_ = false;
    isSupportReload_ = true;
    isSupportCommunicationSharing_ = 0;
}

void AudioDeviceDescriptor::CopyBaseFields(const AudioDeviceDescriptor &deviceDescriptor)
{
    deviceId_ = deviceDescriptor.deviceId_;
    deviceName_ = deviceDescriptor.deviceName_;
    macAddress_ = deviceDescriptor.macAddress_;
    deviceType_ = deviceDescriptor.deviceType_;
    deviceRole_ = deviceDescriptor.deviceRole_;
    audioStreamInfo_ = deviceDescriptor.audioStreamInfo_;
    capabilities_ = deviceDescriptor.capabilities_;
    channelMasks_ = deviceDescriptor.channelMasks_;
    channelIndexMasks_ = deviceDescriptor.channelIndexMasks_;
    volumeGroupId_ = deviceDescriptor.volumeGroupId_;
    interruptGroupId_ = deviceDescriptor.interruptGroupId_;
    networkId_ = deviceDescriptor.networkId_;
    dmDeviceType_ = deviceDescriptor.dmDeviceType_;
    hasStreamRunning_ = deviceDescriptor.hasStreamRunning_;
    displayName_ = deviceDescriptor.displayName_;
    model_ = deviceDescriptor.model_;
    deviceCategory_ = deviceDescriptor.deviceCategory_;
    connectTimeStamp_ = deviceDescriptor.connectTimeStamp_;
    connectState_ = deviceDescriptor.connectState_;
    pairDeviceDescriptor_ = deviceDescriptor.pairDeviceDescriptor_;
    isScoRealConnected_ = deviceDescriptor.isScoRealConnected_;
    isEnable_ = deviceDescriptor.isEnable_;
    isExclusiveMicScoUnavailable_ = deviceDescriptor.isExclusiveMicScoUnavailable_;
    exceptionFlag_ = deviceDescriptor.exceptionFlag_;
    deviceUsage_ = deviceDescriptor.deviceUsage_;
}

void AudioDeviceDescriptor::CopyExtendedFields(const AudioDeviceDescriptor &deviceDescriptor)
{
    isLowLatencyDevice_ = deviceDescriptor.isLowLatencyDevice_;
    a2dpOffloadFlag_ = deviceDescriptor.a2dpOffloadFlag_;
    descriptorType_ = deviceDescriptor.descriptorType_;
    hasPair_ = deviceDescriptor.hasPair_;
    spatializationSupported_ = deviceDescriptor.spatializationSupported_;
    isVrSupported_ = deviceDescriptor.isVrSupported_;
    isSupportWearDetectionEnable_ = deviceDescriptor.isSupportWearDetectionEnable_;
    clientInfo_ = deviceDescriptor.clientInfo_;
    modemCallSupported_ = deviceDescriptor.modemCallSupported_;
    highQualityRecordingSupported_ = deviceDescriptor.highQualityRecordingSupported_;
    isUnidirectional_ = deviceDescriptor.isUnidirectional_;
    dmDeviceInfo_ = deviceDescriptor.dmDeviceInfo_;
    volumeBehavior_ = deviceDescriptor.volumeBehavior_;
    glassesWearCount_ = deviceDescriptor.glassesWearCount_;
    hdPlaybackMode_ = deviceDescriptor.hdPlaybackMode_;
    isSupportRemoteMultichannel_ = deviceDescriptor.isSupportRemoteMultichannel_;
    deviceTokenId_ = deviceDescriptor.deviceTokenId_;
    privacyType_ = deviceDescriptor.GetDevicePrivacyType();
    isSilent_ = deviceDescriptor.isSilent_;
    isSupportMultiZone_ = deviceDescriptor.isSupportMultiZone_;
    visibleMode_ = deviceDescriptor.visibleMode_;
    sinkLatencyMs_ = deviceDescriptor.sinkLatencyMs_;
    sinkLatencyValid_ = deviceDescriptor.sinkLatencyValid_;
    isSupportReload_ = deviceDescriptor.isSupportReload_;
    isPriorityReduced_ = deviceDescriptor.isPriorityReduced_;
    isSupportCommunicationSharing_ = deviceDescriptor.isSupportCommunicationSharing_;
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor)
{
    CopyBaseFields(deviceDescriptor);
    CopyExtendedFields(deviceDescriptor);
}

AudioDeviceDescriptor::AudioDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    CHECK_AND_RETURN_LOG(deviceDescriptor != nullptr, "Error input parameter");
    CopyBaseFields(*deviceDescriptor);
    CopyExtendedFields(*deviceDescriptor);
}

DeviceType AudioDeviceDescriptor::getType() const
{
    return deviceType_;
}

int32_t AudioDeviceDescriptor::GetDeviceId() const
{
    return deviceId_;
}

std::string AudioDeviceDescriptor::GetMacAddress() const
{
    return macAddress_;
}

std::list<DeviceStreamInfo> AudioDeviceDescriptor::GetAudioStreamInfo() const
{
    return audioStreamInfo_;
}

uint32_t AudioDeviceDescriptor::GetDeviceSupportMmap() const
{
    if (audioStreamInfo_.empty()) {
        return 0;
    }
    const DeviceStreamInfo &info = audioStreamInfo_.back();
    bool fastSupported = !info.fastSamplingRate.empty();
    bool ufastSupported = !info.ufastSamplingRate.empty();
    if (fastSupported && ufastSupported) {
        return 1;
    }
    if (fastSupported && !ufastSupported) {
        return ONLY_SUPPORT_MMAP;
    }
    return 0;
}

std::unordered_map<std::string, std::string> AudioDeviceDescriptor::ParseAudioParameters(
    const std::string &audioParameters)
{
    std::unordered_map<std::string, std::string> ret;
    CHECK_AND_RETURN_RET_LOG(!audioParameters.empty(), ret, "audioParameters is empty, can not parse");
    std::vector<std::string> parameters = Util::SplitString(audioParameters, ';');
    for (const auto &parameter : parameters) {
        std::vector<std::string> kv = Util::SplitString(parameter, ':');
        CHECK_AND_CONTINUE_LOG(kv.size() == KV_SIZE, "error parameter: %{public}s", parameter.c_str());
        const auto &key = kv.front();
        const auto &value = kv.back();
        AUDIO_INFO_LOG("parse key: %{public}s, value: %{public}s", key.c_str(), value.c_str());
        ret[key] = value;
    }
    return ret;
}

static void ParseRateValues(const std::string &value, std::set<AudioSamplingRate> &rates)
{
    for (const auto &rateStr : Util::SplitString(value, '|')) {
        uint32_t samplingRate = 0;
        ConvertToUint32(rateStr, samplingRate);
        if (samplingRate == 0) {
            AUDIO_WARNING_LOG("invalid sampling rate string: %{public}s", rateStr.c_str());
            continue;
        }
        rates.insert(static_cast<AudioSamplingRate>(samplingRate));
    }
}

static void ParseFormatValues(const std::string &value, DeviceStreamInfo &streamInfo)
{
    for (const auto &formatStr : Util::SplitString(value, '|')) {
        AudioSampleFormat fmt = g_formatStrToEnum[ParseAudioFormat(formatStr)];
        streamInfo.formats.insert(fmt);
    }
    // use max format
    if (!streamInfo.formats.empty()) {
        streamInfo.format = *streamInfo.formats.rbegin();
    }
}

static void ParseChannelValues(const std::string &value, DeviceStreamInfo &streamInfo)
{
    for (const auto &chanStr : Util::SplitString(value, '|')) {
        uint32_t channels = 0;
        if (!ConvertToUint32(chanStr, channels) || channels == 0) {
            AUDIO_ERR_LOG("invalid channel string: %{public}s", chanStr.c_str());
            continue;
        }
        AudioChannelLayout layout = CH_LAYOUT_UNKNOWN;
        switch (channels) {
            case AudioChannel::MONO:
                layout = CH_LAYOUT_MONO;
                break;
            case AudioChannel::STEREO:
                layout = CH_LAYOUT_STEREO;
                break;
            case AudioChannel::CHANNEL_3:
                layout = CH_LAYOUT_SURROUND;
                break;
            case AudioChannel::CHANNEL_4:
                layout = CH_LAYOUT_4POINT0;
                break;
            case AudioChannel::CHANNEL_5:
                layout = CH_LAYOUT_5POINT0;
                break;
            case AudioChannel::CHANNEL_6:
                layout = CH_LAYOUT_5POINT1;
                break;
            case AudioChannel::CHANNEL_7:
                layout = CH_LAYOUT_7POINT0;
                break;
            case AudioChannel::CHANNEL_8:
                layout = CH_LAYOUT_7POINT1;
                break;
            default:
                AUDIO_ERR_LOG("unsupported channel count: %{public}u", channels);
                continue;
        }
        streamInfo.channelLayout.insert(layout);
    }
}

void AudioDeviceDescriptor::UpdateDeviceInfoByAudioParameters(const std::string &audioParameters)
{
    auto parameters = ParseAudioParameters(audioParameters);
    std::string key = "device_name";
    auto it = parameters.find(key);
    if (it != parameters.end()) {
        deviceName_ = it->second;
    }
    bool isOutput = getRole() == DeviceRole::OUTPUT_DEVICE;
    DeviceStreamInfo streamInfo = {};
    key = isOutput ? "sink_rate" : "source_rate";
    it = parameters.find(key);
    if (it != parameters.end()) {
        ParseRateValues(it->second, streamInfo.samplingRate);
    }
    key = isOutput ? "sink_format" : "source_format";
    it = parameters.find(key);
    if (it != parameters.end()) {
        ParseFormatValues(it->second, streamInfo);
    }
    key = isOutput ? "sink_chans" : "source_chans";
    it = parameters.find(key);
    if (it != parameters.end()) {
        ParseChannelValues(it->second, streamInfo);
    }
    key = isOutput ? "sink_fast" : "source_fast";
    it = parameters.find(key);
    if (it != parameters.end()) {
        ParseRateValues(it->second, streamInfo.fastSamplingRate);
    }
    key = isOutput ? "sink_ufast" : "source_ufast";
    it = parameters.find(key);
    if (it != parameters.end()) {
        ParseRateValues(it->second, streamInfo.ufastSamplingRate);
    }
    audioStreamInfo_.clear();
    audioStreamInfo_.push_back(streamInfo);
    BuildCapabilitiesFromDeviceStreamInfo();
}

DeviceRole AudioDeviceDescriptor::getRole() const
{
    return deviceRole_;
}

DeviceCategory AudioDeviceDescriptor::GetDeviceCategory() const
{
    return deviceCategory_;
}

void AudioDeviceDescriptor::SetDevicePrivacyType(AudioDevicePrivacyType privacyType)
{
    AUDIO_INFO_LOG("SetDevicePrivacyType, privacyType: %{public}d", privacyType);
    privacyType_ = privacyType;
}

AudioDevicePrivacyType AudioDeviceDescriptor::GetDevicePrivacyType() const
{
    return privacyType_;
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
            clientInfo_.value().isSupportedDevice_);
    }

    int32_t apiVersion = 0;
    bool isSupportedDevice = true;
    if (clientInfo_) {
        apiVersion = clientInfo_.value().apiVersion_;
        isSupportedDevice = clientInfo_.value().isSupportedDevice_;
    }
    int32_t devType = IsAudioDeviceDescriptor() ?
        MapInternalToExternalDeviceType(apiVersion, isSupportedDevice) : deviceType_;

    std::list<AudioStreamInfo> capabilities = capabilities_.empty() ?
        std::list<AudioStreamInfo>{DEFAULT_AUDIO_STREAM_INFO} : capabilities_;

    return  MarshallingInnerdata(parcel, devType, capabilities);
}

bool AudioDeviceDescriptor::MarshallingInnerdata(Parcel &parcel, int32_t devType,
    const std::list<AudioStreamInfo> &capabilities) const
{
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
        parcel.WriteString(displayName_) && parcel.WriteString(model_) &&
        MarshallingDeviceStreamInfoList(audioStreamInfo_, parcel) &&
        MarshallingAudioStreamInfoList(capabilities, parcel) &&
        MarshallingExt(parcel);
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
    int32_t apiVersion, bool isSupportedDevice) const
{
    DeviceType devType = MapInternalToExternalDeviceType(apiVersion, isSupportedDevice);
    int32_t devId = deviceId_;
    std::list<DeviceStreamInfo> streamInfo = audioStreamInfo_;

    FixApiCompatibility(apiVersion, deviceRole_, devType, devId, streamInfo);

    std::list<AudioStreamInfo> capabilities = capabilities_.empty() ?
        std::list<AudioStreamInfo>{DEFAULT_AUDIO_STREAM_INFO} : capabilities_;

    return parcel.WriteInt32(static_cast<int32_t>(devType)) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceRole_)) &&
        parcel.WriteInt32(devId) && parcel.WriteInt32(channelMasks_) &&
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
        parcel.WriteString(model_) &&
        MarshallingDeviceStreamInfoList(streamInfo, parcel) &&
        MarshallingAudioStreamInfoList(capabilities, parcel) &&
        MarshallingExt(parcel);
}

bool AudioDeviceDescriptor::MarshallingExt(Parcel &parcel) const
{
    return parcel.WriteInt32(static_cast<int32_t>(deviceCategory_)) &&
        parcel.WriteInt32(static_cast<int32_t>(connectState_)) &&
        parcel.WriteBool(exceptionFlag_) &&
        parcel.WriteInt64(connectTimeStamp_) &&
        parcel.WriteBool(isScoRealConnected_) &&
        parcel.WriteBool(isEnable_) &&
        parcel.WriteBool(isExclusiveMicScoUnavailable_) &&
        parcel.WriteInt32(mediaVolume_) &&
        parcel.WriteInt32(callVolume_) &&
        parcel.WriteBool(isLowLatencyDevice_) &&
        parcel.WriteInt32(a2dpOffloadFlag_) &&
        parcel.WriteBool(descriptorType_) &&
        parcel.WriteBool(spatializationSupported_) &&
        parcel.WriteBool(hasPair_) &&
        parcel.WriteInt32(routerType_) && parcel.WriteInt32(isVrSupported_) &&
        parcel.WriteBool(isSupportWearDetectionEnable_) &&
        parcel.WriteInt32(static_cast<int32_t>(deviceUsage_)) && parcel.WriteBool(modemCallSupported_) &&
        parcel.WriteBool(highQualityRecordingSupported_) && parcel.WriteBool(isUnidirectional_) &&
        parcel.WriteString(dmDeviceInfo_) && parcel.WriteInt32(hdPlaybackMode_) &&
        parcel.WriteBool(isSupportRemoteMultichannel_) && parcel.WriteUint32(deviceTokenId_) &&
        parcel.WriteInt32(static_cast<int32_t>(GetDevicePrivacyType())) &&
        parcel.WriteBool(isSilent_) && parcel.WriteBool(isSupportMultiZone_) &&
        parcel.WriteInt32(static_cast<int32_t>(visibleMode_)) && parcel.WriteUint32(sinkLatencyMs_) &&
        parcel.WriteBool(sinkLatencyValid_) && parcel.WriteBool(isSupportReload_) &&
        parcel.WriteBool(isPriorityReduced_) && parcel.WriteInt32(isSupportCommunicationSharing_);
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
    model_ = parcel.ReadString();
    UnmarshallingDeviceStreamInfoList(parcel, audioStreamInfo_);
    UnmarshallingAudioStreamInfoList(parcel, capabilities_);
    deviceCategory_ = static_cast<DeviceCategory>(parcel.ReadInt32());
    connectState_ = static_cast<ConnectState>(parcel.ReadInt32());
    exceptionFlag_ = parcel.ReadBool();
    connectTimeStamp_ = parcel.ReadInt64();
    isScoRealConnected_ = parcel.ReadBool();
    isEnable_ = parcel.ReadBool();
    isExclusiveMicScoUnavailable_ = parcel.ReadBool();
    mediaVolume_ = parcel.ReadInt32();
    callVolume_ = parcel.ReadInt32();
    isLowLatencyDevice_ = parcel.ReadBool();
    a2dpOffloadFlag_ = parcel.ReadInt32();
    descriptorType_ = parcel.ReadBool();
    spatializationSupported_ = parcel.ReadBool();
    hasPair_ = parcel.ReadBool();
    routerType_ = static_cast<RouterType>(parcel.ReadInt32());
    isVrSupported_ = parcel.ReadInt32();
    isSupportWearDetectionEnable_ = parcel.ReadBool();
    deviceUsage_ = static_cast<DeviceUsage>(parcel.ReadInt32());
    modemCallSupported_ = parcel.ReadBool();
    highQualityRecordingSupported_ = parcel.ReadBool();
    isUnidirectional_ = parcel.ReadBool();
    dmDeviceInfo_ = parcel.ReadString();
    hdPlaybackMode_ = static_cast<HdPlaybackMode>(parcel.ReadInt32());
    isSupportRemoteMultichannel_ = parcel.ReadBool();
    deviceTokenId_ = parcel.ReadUint32();
    privacyType_ = static_cast<AudioDevicePrivacyType>(parcel.ReadInt32());
    isSilent_ = parcel.ReadBool();
    isSupportMultiZone_ = parcel.ReadBool();
    visibleMode_ = static_cast<VisibleMode>(parcel.ReadInt32());
    UnmarshallingSelfExt(parcel);
}

void AudioDeviceDescriptor::UnmarshallingSelfExt(Parcel &parcel)
{
    sinkLatencyMs_ = parcel.ReadUint32();
    sinkLatencyValid_ = parcel.ReadBool();
    isSupportReload_ = parcel.ReadBool();
    isPriorityReduced_ = parcel.ReadBool();
    isSupportCommunicationSharing_ = parcel.ReadInt32();
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
        bool isTargetDevice = (descs[index]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN ||
            (descs[index]->deviceType_ == DEVICE_TYPE_BT_SPP && descs[index]->isUnidirectional_));
        CHECK_AND_CONTINUE(isTargetDevice);
        bool hasSame = false;
        for (auto& otherDesc : descs) {
            if (otherDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO &&
                descs[index]->macAddress_ != "" &&
                otherDesc->macAddress_ == descs[index]->macAddress_ &&
                otherDesc->deviceRole_ == descs[index]->deviceRole_ &&
                otherDesc->networkId_ == descs[index]->networkId_) {
                hasSame = true;
                otherDesc->highQualityRecordingSupported_ = descs[index]->highQualityRecordingSupported_;
                otherDesc->isUnidirectional_ = descs[index]->isUnidirectional_;
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

void AudioDeviceDescriptor::SetExtraDeviceInfo(const DStatusInfo &statusInfo, bool hasSystemPermission)
{
    if (statusInfo.model == "hiplay") { model_ = "hiplay"; }
    networkId_ = statusInfo.networkId;
    dmDeviceType_ = statusInfo.dmDeviceType;
    connectState_ = (dmDeviceType_ == DM_DEVICE_TYPE_WIFI_SOUNDBOX || dmDeviceType_ == DM_DEVICE_TYPE_MUSIC_HOST) ?
        VIRTUAL_CONNECTED : CONNECTED;
    dmDeviceInfo_ = hasSystemPermission ? statusInfo.dmDeviceInfo : "";
}

bool AudioDeviceDescriptor::IsSameDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return deviceDescriptor.deviceType_ == deviceType_ &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_ &&
        (!IsUsb(deviceType_) || deviceDescriptor.deviceRole_ == deviceRole_);
}

bool AudioDeviceDescriptor::IsSameDeviceDescPtr(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "input deviceDescriptor is null");
    return deviceDescriptor->deviceType_ == deviceType_ &&
        deviceDescriptor->macAddress_ == macAddress_ &&
        deviceDescriptor->networkId_ == networkId_ &&
        (!IsUsb(deviceType_) || deviceDescriptor->deviceRole_ == deviceRole_);
}

bool AudioDeviceDescriptor::IsSameBtDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return IsBtDevice(deviceDescriptor.deviceType_) &&
        IsBtDevice(deviceType_) &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_;
}

bool AudioDeviceDescriptor::IsSameBtDeviceDescPtr(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "input deviceDescriptor is null");
    return IsSameBtDeviceDesc(*deviceDescriptor);
}

bool AudioDeviceDescriptor::IsSameBtOrSppDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return IsBtOrSppDevice(deviceDescriptor.deviceType_) &&
        IsBtOrSppDevice(deviceType_) &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_;
}

bool AudioDeviceDescriptor::IsSameBtOrSppDeviceDescPtr(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "input deviceDescriptor is null");
    return IsSameBtOrSppDeviceDesc(*deviceDescriptor);
}

bool AudioDeviceDescriptor::IsSameNearlinkOrPortDeviceDesc(const AudioDeviceDescriptor &deviceDescriptor) const
{
    return IsNearlinkOrPortDevice(deviceDescriptor.deviceType_) &&
        IsNearlinkOrPortDevice(deviceType_) &&
        deviceDescriptor.macAddress_ == macAddress_ &&
        deviceDescriptor.networkId_ == networkId_;
}

bool AudioDeviceDescriptor::IsSameNearlinkOrPortDeviceDescPtr(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor) const
{
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr, false, "input deviceDescriptor is null");
    return IsSameNearlinkOrPortDeviceDesc(*deviceDescriptor);
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

HdPlaybackMode AudioDeviceDescriptor::GetHdPlaybackMode() const
{
    return hdPlaybackMode_;
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
    if (networkId_ != LOCAL_NETWORK_ID && deviceType_ == DEVICE_TYPE_MIC) {
        return "Remote_Mic";
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

DeviceType AudioDeviceDescriptor::MapInternalToExternalDeviceType(int32_t apiVersion, bool isSupportedDevice) const
{
    if (connectState_ == SIMULATED_CONNECTED) {
        return DEVICE_TYPE_INVALID;
    }
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
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_MICIN:
            return DEVICE_TYPE_WIRED_HEADSET;
        case DEVICE_TYPE_WIRED_HEADPHONES:
            return DEVICE_TYPE_WIRED_HEADPHONES;
        case DEVICE_TYPE_LINE_DIGITAL:
            return DEVICE_TYPE_LINE_DIGITAL;
        case DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            return DEVICE_TYPE_BLUETOOTH_A2DP;
        case DEVICE_TYPE_HEARING_AID:
            if (!isSupportedDevice) {
                return DEVICE_TYPE_BLUETOOTH_SCO;
            }
            return DEVICE_TYPE_HEARING_AID;
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_NEARLINK_IN:
            if (!isSupportedDevice) {
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

void AudioDeviceDescriptor::BuildCapabilitiesFromDeviceStreamInfo()
{
    if ((deviceType_ == DEVICE_TYPE_SPEAKER && networkId_ != LOCAL_NETWORK_ID) ||
        deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET || isUnidirectional_) {
        capabilities_.clear();
        for (const auto &deviceStreamInfo : audioStreamInfo_) {
            AudioStreamInfo cap;
            AudioChannelLayout channelLayout = !deviceStreamInfo.channelLayout.empty() ?
                *deviceStreamInfo.channelLayout.begin() : AudioChannelLayout::CH_LAYOUT_STEREO;
            AudioSamplingRate samplingRate = !deviceStreamInfo.samplingRate.empty() ?
                *deviceStreamInfo.samplingRate.begin() : AudioSamplingRate::SAMPLE_RATE_48000;
            AudioSampleFormat sampleFormat = deviceStreamInfo.format == AudioSampleFormat::INVALID_WIDTH ?
                AudioSampleFormat::SAMPLE_S16LE : deviceStreamInfo.format;
            AudioEncodingType encodingType = deviceStreamInfo.encoding == AudioEncodingType::ENCODING_INVALID ?
                AudioEncodingType::ENCODING_PCM : deviceStreamInfo.encoding;
            cap.samplingRate = samplingRate;
            cap.encoding = encodingType;
            cap.format = sampleFormat;
            cap.channels = ConvertLayoutToAudioChannel(channelLayout);
            cap.channelLayout = deviceType_ != DEVICE_TYPE_USB_ARM_HEADSET ? channelLayout : CH_LAYOUT_UNKNOWN;
            capabilities_.push_back(cap);
            AUDIO_INFO_LOG("add capability: samplingRate: %{public}d, format: %{public}u, channels: %{public}u "
                "channelLayout: %{public}" PRIu64, cap.samplingRate, cap.format, cap.channels, cap.channelLayout);
        }
    } else if (deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        capabilities_.clear();
        AudioStreamInfo cap(AudioSamplingRate::SAMPLE_RATE_16000, AudioEncodingType::ENCODING_PCM,
            AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);
        capabilities_.push_back(cap);
    }
}

} // AudioStandard
} // namespace OHOS
