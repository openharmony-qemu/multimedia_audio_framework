/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "TaiheParamUtils"
#endif

#include "taihe_param_utils.h"
#include "audio_stream_info.h"
#include "audio_log.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {

const std::vector<OHOS::AudioStandard::DeviceRole> DEVICE_ROLE_SET = {
    OHOS::AudioStandard::DEVICE_ROLE_NONE,
    OHOS::AudioStandard::INPUT_DEVICE,
    OHOS::AudioStandard::OUTPUT_DEVICE
};

const std::vector<OHOS::AudioStandard::DeviceType> DEVICE_TYPE_SET = {
    OHOS::AudioStandard::DEVICE_TYPE_NONE,
    OHOS::AudioStandard::DEVICE_TYPE_INVALID,
    OHOS::AudioStandard::DEVICE_TYPE_EARPIECE,
    OHOS::AudioStandard::DEVICE_TYPE_SPEAKER,
    OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET,
    OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES,
    OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_SCO,
    OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP,
    OHOS::AudioStandard::DEVICE_TYPE_MIC,
    OHOS::AudioStandard::DEVICE_TYPE_WAKEUP,
    OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET,
    OHOS::AudioStandard::DEVICE_TYPE_DP,
    OHOS::AudioStandard::DEVICE_TYPE_REMOTE_CAST,
    OHOS::AudioStandard::DEVICE_TYPE_USB_DEVICE,
    OHOS::AudioStandard::DEVICE_TYPE_REMOTE_DAUDIO,
    OHOS::AudioStandard::DEVICE_TYPE_USB_ARM_HEADSET,
    OHOS::AudioStandard::DEVICE_TYPE_FILE_SINK,
    OHOS::AudioStandard::DEVICE_TYPE_FILE_SOURCE,
    OHOS::AudioStandard::DEVICE_TYPE_EXTERN_CABLE,
    OHOS::AudioStandard::DEVICE_TYPE_HDMI,
    OHOS::AudioStandard::DEVICE_TYPE_ACCESSORY,
    OHOS::AudioStandard::DEVICE_TYPE_NEARLINK,
    OHOS::AudioStandard::DEVICE_TYPE_HEARING_AID,
    OHOS::AudioStandard::DEVICE_TYPE_DEFAULT
};

const std::unordered_map<OHOS::AudioStandard::AudioSamplingRate, AudioSamplingRate> gNativeToAniAudioSamplingRate = {
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_8000, AudioSamplingRate::key_t::SAMPLE_RATE_8000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_11025, AudioSamplingRate::key_t::SAMPLE_RATE_11025},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_12000, AudioSamplingRate::key_t::SAMPLE_RATE_12000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_16000, AudioSamplingRate::key_t::SAMPLE_RATE_16000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_22050, AudioSamplingRate::key_t::SAMPLE_RATE_22050},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_24000, AudioSamplingRate::key_t::SAMPLE_RATE_24000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_32000, AudioSamplingRate::key_t::SAMPLE_RATE_32000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_44100, AudioSamplingRate::key_t::SAMPLE_RATE_44100},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_48000, AudioSamplingRate::key_t::SAMPLE_RATE_48000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_64000, AudioSamplingRate::key_t::SAMPLE_RATE_64000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_88200, AudioSamplingRate::key_t::SAMPLE_RATE_88200},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_96000, AudioSamplingRate::key_t::SAMPLE_RATE_96000},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_176400, AudioSamplingRate::key_t::SAMPLE_RATE_176400},
    {OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_192000, AudioSamplingRate::key_t::SAMPLE_RATE_192000},
};

const std::unordered_map<OHOS::AudioStandard::AudioChannel, AudioChannel> gNativeToAniAudioChannel = {
    {OHOS::AudioStandard::AudioChannel::MONO, AudioChannel::key_t::CHANNEL_1},
    {OHOS::AudioStandard::AudioChannel::STEREO, AudioChannel::key_t::CHANNEL_2},
    {OHOS::AudioStandard::AudioChannel::CHANNEL_3, AudioChannel::key_t::CHANNEL_3},
    {OHOS::AudioStandard::AudioChannel::CHANNEL_4, AudioChannel::key_t::CHANNEL_4},
    {OHOS::AudioStandard::AudioChannel::CHANNEL_5, AudioChannel::key_t::CHANNEL_5},
    {OHOS::AudioStandard::AudioChannel::CHANNEL_6, AudioChannel::key_t::CHANNEL_6},
};

const std::unordered_map<OHOS::AudioStandard::AudioSampleFormat, AudioSampleFormat> gNativeToAniAudioSampleFormat = {
    {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_U8, AudioSampleFormat::key_t::SAMPLE_FORMAT_U8},
    {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE, AudioSampleFormat::key_t::SAMPLE_FORMAT_S16LE},
    {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S24LE, AudioSampleFormat::key_t::SAMPLE_FORMAT_S24LE},
    {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S32LE, AudioSampleFormat::key_t::SAMPLE_FORMAT_S32LE},
    {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_F32LE, AudioSampleFormat::key_t::SAMPLE_FORMAT_F32LE},
};

int32_t TaiheParamUtils::GetRendererInfo(OHOS::AudioStandard::AudioRendererInfo &rendererInfo,
    AudioRendererInfo const &in)
{
    int32_t intValue = in.usage.get_value();
    if (TaiheAudioEnum::IsLegalInputArgumentStreamUsage(intValue)) {
        rendererInfo.streamUsage = static_cast<OHOS::AudioStandard::StreamUsage>(intValue);
    } else {
        rendererInfo.streamUsage = OHOS::AudioStandard::StreamUsage::STREAM_USAGE_INVALID;
    }

    rendererInfo.rendererFlags = in.rendererFlags;

    if (in.volumeMode.has_value()) {
        intValue = in.volumeMode.value().get_value();
        AUDIO_INFO_LOG("volume mode = %{public}d", intValue);
        if (TaiheAudioEnum::IsLegalInputArgumentVolumeMode(intValue)) {
            rendererInfo.volumeMode = static_cast<OHOS::AudioStandard::AudioVolumeMode>(intValue);
        } else {
            AUDIO_INFO_LOG("AudioVolumeMode is invalid parameter");
            return AUDIO_INVALID_PARAM;
        }
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetStreamInfo(OHOS::AudioStandard::AudioStreamInfo &audioStreamInfo,
    AudioCapturerOptions const &options)
{
    int32_t intValue = options.streamInfo.samplingRate.get_value();
    if (intValue >= OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_8000 &&
        intValue <= OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_192000) {
        audioStreamInfo.samplingRate = static_cast<OHOS::AudioStandard::AudioSamplingRate>(intValue);
    } else {
        AUDIO_ERR_LOG("invaild samplingRate");
        return AUDIO_ERR;
    }
    audioStreamInfo.channels =
        static_cast<OHOS::AudioStandard::AudioChannel>(options.streamInfo.channels.get_value());
    audioStreamInfo.format =
        static_cast<OHOS::AudioStandard::AudioSampleFormat>(options.streamInfo.sampleFormat.get_value());
    audioStreamInfo.encoding =
        static_cast<OHOS::AudioStandard::AudioEncodingType>(options.streamInfo.encodingType.get_value());
    if (options.streamInfo.channelLayout.has_value()) {
        audioStreamInfo.channelLayout =
            static_cast<OHOS::AudioStandard::AudioChannelLayout>(options.streamInfo.channelLayout.value().get_value());
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetStreamInfo(OHOS::AudioStandard::AudioStreamInfo &audioStreamInfo,
    AudioRendererOptions const &options)
{
    int32_t intValue = options.streamInfo.samplingRate.get_value();
    if (intValue >= OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_8000 &&
        intValue <= OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_192000) {
        audioStreamInfo.samplingRate = static_cast<OHOS::AudioStandard::AudioSamplingRate>(intValue);
    } else {
        AUDIO_ERR_LOG("invaild samplingRate");
        return AUDIO_ERR;
    }
    audioStreamInfo.channels =
        static_cast<OHOS::AudioStandard::AudioChannel>(options.streamInfo.channels.get_value());
    audioStreamInfo.format =
        static_cast<OHOS::AudioStandard::AudioSampleFormat>(options.streamInfo.sampleFormat.get_value());
    audioStreamInfo.encoding =
        static_cast<OHOS::AudioStandard::AudioEncodingType>(options.streamInfo.encodingType.get_value());
    if (options.streamInfo.channelLayout.has_value()) {
        audioStreamInfo.channelLayout =
            static_cast<OHOS::AudioStandard::AudioChannelLayout>(options.streamInfo.channelLayout.value().get_value());
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetCapturerInfo(OHOS::AudioStandard::AudioCapturerInfo &audioCapturerInfo,
    AudioCapturerInfo const &in)
{
    int32_t intValue = in.source.get_value();
    CHECK_AND_RETURN_RET_LOG(TaiheAudioEnum::IsLegalCapturerType(intValue),
        AUDIO_ERR, "Invailed captureType");
    audioCapturerInfo.sourceType = static_cast<OHOS::AudioStandard::SourceType>(intValue);
    audioCapturerInfo.capturerFlags = in.capturerFlags;
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetCapturerOptions(OHOS::AudioStandard::AudioCapturerOptions *opts,
    AudioCapturerOptions const &options)
{
    CHECK_AND_RETURN_RET_LOG(opts != nullptr, AUDIO_ERR, "opts is nullptr");
    int32_t status = AUDIO_OK;
    status = GetStreamInfo(opts->streamInfo, options);
    CHECK_AND_RETURN_RET_LOG(status == AUDIO_OK, status, "ParseStreamInfo failed");

    status = GetCapturerInfo(opts->capturerInfo, options.capturerInfo);
    CHECK_AND_RETURN_RET_LOG(status == AUDIO_OK, status, "ParseCapturerInfo failed");

    CHECK_AND_RETURN_RET_LOG(status == AUDIO_OK, status, "ParsePlaybackCaptureConfig failed");
    AUDIO_INFO_LOG("ParseCapturerOptions, without playbackCaptureConfig");
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetRendererOptions(OHOS::AudioStandard::AudioRendererOptions *opts,
    AudioRendererOptions const &options)
{
    CHECK_AND_RETURN_RET_LOG(opts != nullptr, AUDIO_ERR, "opts is nullptr");
    int32_t status = AUDIO_OK;
    status = GetRendererInfo(opts->rendererInfo, options.rendererInfo);
    CHECK_AND_RETURN_RET_LOG(status == AUDIO_OK, status, "Parse RendererInfo failed");

    status = GetStreamInfo(opts->streamInfo, options);
    CHECK_AND_RETURN_RET_LOG(status == AUDIO_OK, status, "Parse StreamInfo failed");

    if (options.privacyType.has_value()) {
        opts->privacyType = static_cast<OHOS::AudioStandard::AudioPrivacyType>(options.privacyType.value().get_value());
    }
    return AUDIO_OK;
}
int32_t TaiheParamUtils::GetSpatialDeviceState(OHOS::AudioStandard::AudioSpatialDeviceState *spatialDeviceState,
    AudioSpatialDeviceState in)
{
    CHECK_AND_RETURN_RET_LOG(spatialDeviceState != nullptr, AUDIO_ERR, "spatialDeviceState is nullptr");
    spatialDeviceState->address = std::string(in.address);
    spatialDeviceState->isSpatializationSupported = in.isSpatializationSupported;
    spatialDeviceState->isHeadTrackingSupported = in.isHeadTrackingSupported;
    int32_t intValue = in.spatialDeviceType.get_value();
    if (!((intValue >= OHOS::AudioStandard::AudioSpatialDeviceType::EARPHONE_TYPE_NONE) &&
        (intValue <= OHOS::AudioStandard::AudioSpatialDeviceType::EARPHONE_TYPE_OTHERS))) {
        AUDIO_ERR_LOG("Get spatialDeviceType failed");
        return AUDIO_INVALID_PARAM;
    }
    spatialDeviceState->spatialDeviceType = static_cast<OHOS::AudioStandard::AudioSpatialDeviceType>(intValue);
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioDeviceDescriptor(
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> &selectedAudioDevice,
    bool &argTransFlag, AudioDeviceDescriptor in)
{
    CHECK_AND_RETURN_RET_LOG(selectedAudioDevice != nullptr, AUDIO_INVALID_PARAM, "selectedAudioDevice is null");
    argTransFlag = true;
    int32_t intValue = in.deviceRole.get_value();
    if (std::find(DEVICE_ROLE_SET.begin(), DEVICE_ROLE_SET.end(), intValue) == DEVICE_ROLE_SET.end()) {
        argTransFlag = false;
        return AUDIO_INVALID_PARAM;
    }
    selectedAudioDevice->deviceRole_ = static_cast<OHOS::AudioStandard::DeviceRole>(intValue);

    intValue = in.deviceType.get_value();
    if (std::find(DEVICE_TYPE_SET.begin(), DEVICE_TYPE_SET.end(), intValue) == DEVICE_TYPE_SET.end()) {
        argTransFlag = false;
        return AUDIO_INVALID_PARAM;
    }
    selectedAudioDevice->deviceType_ = static_cast<OHOS::AudioStandard::DeviceType>(intValue);

    selectedAudioDevice->networkId_ = std::string(in.networkId);

    if (in.dmDeviceType.has_value()) {
        selectedAudioDevice->dmDeviceType_ = static_cast<uint16_t>(in.dmDeviceType.value());
    }

    selectedAudioDevice->displayName_ = std::string(in.displayName);

    selectedAudioDevice->interruptGroupId_ = in.interruptGroupId;

    selectedAudioDevice->volumeGroupId_ = in.volumeGroupId;

    selectedAudioDevice->macAddress_ = std::string(in.address);

    selectedAudioDevice->deviceId_ = in.id;
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioDeviceDescriptorVector(
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &deviceDescriptorsVector,
    bool &argTransFlag, array_view<AudioDeviceDescriptor> in)
{
    if (in.size() == 0) {
        deviceDescriptorsVector = {};
        AUDIO_INFO_LOG("Error: AudioDeviceDescriptor vector is NULL!");
    }

    for (AudioDeviceDescriptor &element : in) {
        std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> selectedAudioDevice =
            std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
        int32_t ret = GetAudioDeviceDescriptor(selectedAudioDevice, argTransFlag, element);
        if (ret != AUDIO_OK) {
            AUDIO_ERR_LOG("GetAudioDeviceDescriptor failed");
        }
        if (!argTransFlag) {
            return AUDIO_OK;
        }
        deviceDescriptorsVector.emplace_back(selectedAudioDevice);
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioCapturerInfo(OHOS::AudioStandard::AudioCapturerInfo &capturerInfo,
    AudioCapturerInfo const &in)
{
    int32_t intValue = in.source.get_value();
    if (TaiheAudioEnum::IsValidSourceType(intValue)) {
        capturerInfo.sourceType = static_cast<OHOS::AudioStandard::SourceType>(intValue);
    } else {
        capturerInfo.sourceType = OHOS::AudioStandard::SourceType::SOURCE_TYPE_INVALID;
    }

    capturerInfo.capturerFlags = in.capturerFlags;
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioCapturerFilter(OHOS::sptr<OHOS::AudioStandard::AudioCapturerFilter>
    &audioCapturerFilter, AudioCapturerFilter const &in)
{
    audioCapturerFilter = new(std::nothrow) OHOS::AudioStandard::AudioCapturerFilter();
    CHECK_AND_RETURN_RET_LOG(audioCapturerFilter != nullptr, AUDIO_INVALID_PARAM, "audioCapturerFilter is null");
    if (in.uid.has_value()) {
        audioCapturerFilter->uid = in.uid.value();
    }
    if (in.capturerInfo.has_value()) {
        int32_t ret = GetCapturerInfo(audioCapturerFilter->capturerInfo, in.capturerInfo.value());
        if (ret != AUDIO_OK) {
            AUDIO_ERR_LOG("GetCapturerInfo failed");
        }
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioRendererFilter(
    OHOS::sptr<OHOS::AudioStandard::AudioRendererFilter> &audioRendererFilter,
    bool &argTransFlag, AudioRendererFilter const &in)
{
    argTransFlag = true;
    audioRendererFilter = new(std::nothrow) OHOS::AudioStandard::AudioRendererFilter();
    CHECK_AND_RETURN_RET_LOG(audioRendererFilter != nullptr, AUDIO_INVALID_PARAM, "audioRendererFilter is null");
    if (in.uid.has_value()) {
        audioRendererFilter->uid = in.uid.value();
    }

    if (in.rendererInfo.has_value()) {
        int32_t ret = GetRendererInfo(audioRendererFilter->rendererInfo, in.rendererInfo.value());
        if (ret != AUDIO_OK) {
            AUDIO_ERR_LOG("GetCapturerInfo failed");
        }
    }

    if (in.rendererId.has_value()) {
        audioRendererFilter->streamId = in.rendererId.value();
    }
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetAudioSessionStrategy(OHOS::AudioStandard::AudioSessionStrategy &audioSessionStrategy,
    AudioSessionStrategy const &in)
{
    int32_t intValue = in.concurrencyMode.get_value();
    audioSessionStrategy.concurrencyMode = static_cast<OHOS::AudioStandard::AudioConcurrencyMode>(intValue);
    return AUDIO_OK;
}

int32_t TaiheParamUtils::UniqueEffectPropertyData(OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray)
{
    CHECK_AND_RETURN_RET_LOG(!propertyArray.property.empty(), 0, "propertyArray.property is empty");
    int32_t propSize = static_cast<int32_t>(propertyArray.property.size());
    std::set<std::string> classSet;
    for (int32_t i = 0; i < propSize; i++) {
        if (propertyArray.property[i].category != "" && propertyArray.property[i].name != "") {
            classSet.insert(propertyArray.property[i].name);
        }
    }
    return static_cast<int32_t>(classSet.size());
}

int32_t TaiheParamUtils::GetEffectPropertyArray(OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray,
    array_view<AudioEffectProperty> in)
{
    OHOS::AudioStandard::AudioEffectPropertyArrayV3 effectArray;
    OHOS::AudioStandard::AudioEffectPropertyArrayV3 enhanceArray;
    for (const auto &inProperty : in) {
        OHOS::AudioStandard::AudioEffectPropertyV3 prop = {
            .name = std::string(inProperty.name),
            .category = std::string(inProperty.category),
            .flag = static_cast<OHOS::AudioStandard::EffectFlag>(inProperty.flag.get_value()),
        };
        propertyArray.property.emplace_back(prop);
        if (prop.flag == OHOS::AudioStandard::EffectFlag::RENDER_EFFECT_FLAG) {
            effectArray.property.push_back(prop);
        } else if (prop.flag == OHOS::AudioStandard::EffectFlag::CAPTURE_EFFECT_FLAG) {
            enhanceArray.property.push_back(prop);
        }
    }

    int32_t effectSize = UniqueEffectPropertyData(effectArray);
    CHECK_AND_RETURN_RET_LOG(effectSize == static_cast<int32_t>(effectArray.property.size()),
        AUDIO_INVALID_PARAM, "audio effect property array exist duplicate data");

    int32_t enhanceSize = UniqueEffectPropertyData(enhanceArray);
    CHECK_AND_RETURN_RET_LOG(enhanceSize == static_cast<int32_t>(enhanceArray.property.size()),
        AUDIO_INVALID_PARAM, "audio enhance property array exist duplicate data");

    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= OHOS::AudioStandard::AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        AUDIO_INVALID_PARAM, "Audio enhance property array size invalid");
    return AUDIO_OK;
}

int32_t TaiheParamUtils::GetExtraParametersSubKV(std::vector<std::pair<std::string, std::string>> &subKV,
    map_view<string, string> in)
{
    for (const auto &[key, value] : in) {
        std::pair<std::string, std::string> pair;
        pair.first = std::string(key);
        pair.second = std::string(value);
        subKV.push_back(pair);
    }
    return AUDIO_OK;
}

AudioSamplingRate TaiheParamUtils::ToTaiheAudioSamplingRate(OHOS::AudioStandard::AudioSamplingRate audioSamplingRate)
{
    auto itr = gNativeToAniAudioSamplingRate.find(audioSamplingRate);
    if (itr == gNativeToAniAudioSamplingRate.end()) {
        AUDIO_ERR_LOG("ToTaiheAudioSamplingRate fail");
        return AudioSamplingRate::key_t::SAMPLE_RATE_8000;
    }
    return itr->second;
}

AudioChannel TaiheParamUtils::ToTaiheAudioChannel(OHOS::AudioStandard::AudioChannel audioChannel)
{
    auto itr = gNativeToAniAudioChannel.find(audioChannel);
    if (itr == gNativeToAniAudioChannel.end()) {
        AUDIO_ERR_LOG("ToTaiheAudioChannel fail");
        return AudioChannel::key_t::CHANNEL_1;
    }
    return itr->second;
}

AudioSampleFormat TaiheParamUtils::ToTaiheAudioSampleFormat(OHOS::AudioStandard::AudioSampleFormat audioSampleFormat)
{
    auto itr = gNativeToAniAudioSampleFormat.find(audioSampleFormat);
    if (itr == gNativeToAniAudioSampleFormat.end()) {
        AUDIO_ERR_LOG("ToTaiheAudioSampleFormat fail");
        return AudioSampleFormat::key_t::SAMPLE_FORMAT_INVALID;
    }
    return itr->second;
}

AudioStreamInfo TaiheParamUtils::ToTaiheAudioStreamInfo(std::shared_ptr<OHOS::AudioStandard::AudioStreamInfo> &src)
{
    AudioStreamInfo streamInfo = {
        .samplingRate = ToTaiheAudioSamplingRate(src->samplingRate),
        .channels = ToTaiheAudioChannel(src->channels),
        .sampleFormat = ToTaiheAudioSampleFormat(src->format),
        .encodingType = TaiheAudioEnum::ToTaiheAudioEncodingType(src->encoding),
        .channelLayout = taihe::optional<AudioChannelLayout>(std::in_place_t{},
            TaiheAudioEnum::ToTaiheAudioChannelLayout(src->channelLayout)),
    };
    return streamInfo;
}

AudioTimestampInfo TaiheParamUtils::ToTaiheAudioTimestampInfo(OHOS::AudioStandard::Timestamp &src)
{
    static const int64_t secToNano = 1000000000;
    int64_t time = src.time.tv_sec * secToNano + src.time.tv_nsec;
    AudioTimestampInfo audioTimestampInfo = {
        .framePos = static_cast<int64_t>(src.framePosition),
        .timestamp = time,
    };
    return audioTimestampInfo;
}

AudioRendererInfo TaiheParamUtils::ToTaiheRendererInfo(const OHOS::AudioStandard::AudioRendererInfo &rendererInfo)
{
    AudioVolumeMode volumeMode = TaiheAudioEnum::ToTaiheAudioVolumeMode(rendererInfo.volumeMode);
    AudioRendererInfo result = {
        .usage = TaiheAudioEnum::ToTaiheStreamUsage(rendererInfo.streamUsage),
        .rendererFlags = rendererInfo.rendererFlags,
        .volumeMode = taihe::optional<AudioVolumeMode>(std::in_place_t{}, volumeMode),
    };
    return result;
}

AudioRendererChangeInfo TaiheParamUtils::ToTaiheAudioRendererChangeInfo(
    const std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo> &src)
{
    AudioRendererChangeInfo changeInfo = {
        .streamId = src->sessionId,
        .clientUid = src->clientUID,
        .rendererInfo = ToTaiheRendererInfo(src->rendererInfo),
        .rendererState = TaiheAudioEnum::ToTaiheAudioState(src->rendererState),
        .deviceDescriptors = SetValueDeviceInfo(src->outputDeviceInfo),
    };
    return changeInfo;
}

AudioCapturerInfo TaiheParamUtils::ToTaiheCapturerInfo(const OHOS::AudioStandard::AudioCapturerInfo &capturerInfo)
{
    AudioCapturerInfo result = {
        .source = TaiheAudioEnum::ToTaiheSourceType(capturerInfo.sourceType),
        .capturerFlags = capturerInfo.capturerFlags,
    };
    return result;
}

AudioCapturerChangeInfo TaiheParamUtils::ToTaiheAudioCapturerChangeInfo(
    const std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo> &src)
{
    AudioCapturerChangeInfo changeInfo = {
        .streamId = src->sessionId,
        .clientUid = src->clientUID,
        .capturerInfo = ToTaiheCapturerInfo(src->capturerInfo),
        .capturerState = TaiheAudioEnum::ToTaiheAudioState(src->capturerState),
        .deviceDescriptors = SetValueDeviceInfo(src->inputDeviceInfo),
        .muted = taihe::optional<bool>(std::in_place_t{}, src->muted),
    };
    return changeInfo;
}

VolumeGroupInfo TaiheParamUtils::ToTaiheVolumeGroupInfo(const OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo>
    &src)
{
    VolumeGroupInfo volumeGroupInfo = {
        .networkId = ToTaiheString(src->networkId_),
        .groupId = src->volumeGroupId_,
        .mappingId = src->mappingId_,
        .groupName = ToTaiheString(src->groupName_),
        .type = TaiheAudioEnum::ToTaiheConnectType(src->connectType_),
    };
    return volumeGroupInfo;
}

string TaiheParamUtils::ToTaiheString(const std::string &src)
{
    return ::taihe::string(src);
}

taihe::array<AudioEffectProperty> TaiheParamUtils::ToTaiheEffectPropertyArray(
    const OHOS::AudioStandard::AudioEffectPropertyArrayV3 &propertyArray)
{
    std::vector<AudioEffectProperty> resultVec;
    for (const auto &property : propertyArray.property) {
        AudioEffectProperty result = {
            .name = taihe::string(property.name),
            .category = taihe::string(property.category),
            .flag = TaiheAudioEnum::ToTaiheEffectFlag(property.flag),
        };
        resultVec.emplace_back(result);
    }
    return taihe::array<AudioEffectProperty>(resultVec);
}

AudioSpatialEnabledStateForDevice TaiheParamUtils::ToTaiheAudioSpatialEnabledStateForDevice(
    const OHOS::AudioStandard::AudioSpatialEnabledStateForDevice &audioSpatialEnabledStateForDevice)
{
    AudioSpatialEnabledStateForDevice spatialEnabledStateForDevice = {
        .deviceDescriptor = SetDeviceDescriptor(audioSpatialEnabledStateForDevice.deviceDescriptor),
        .enabled = audioSpatialEnabledStateForDevice.enabled,
    };
    return spatialEnabledStateForDevice;
}

AudioSessionDeactivatedEvent TaiheParamUtils::ToTaiheSessionDeactivatedEvent(
    const OHOS::AudioStandard::AudioSessionDeactiveEvent &audioSessionDeactiveEvent)
{
    AudioSessionDeactivatedEvent sessionDeactiveEvent = {
        .reason = TaiheAudioEnum::ToTaiheSessionDeactiveReason(audioSessionDeactiveEvent.deactiveReason),
    };
    return sessionDeactiveEvent;
}

MicStateChangeEvent TaiheParamUtils::SetValueMicStateChange(
    const OHOS::AudioStandard::MicStateChangeEvent &micStateChangeEvent)
{
    MicStateChangeEvent taihemicStateChangeEvent {
        .mute = micStateChangeEvent.mute,
    };
    return taihemicStateChangeEvent;
}

DeviceBlockStatusInfo TaiheParamUtils::SetValueBlockedDeviceAction(
    const OHOS::AudioStandard::MicrophoneBlockedInfo &microphoneBlockedInfo)
{
    DeviceBlockStatusInfo taiheDeviceBlockStatusInfo {
        .blockStatus = TaiheAudioEnum::ToTaiheDeviceBlockStatus(microphoneBlockedInfo.blockStatus),
        .devices = TaiheParamUtils::SetDeviceDescriptors(microphoneBlockedInfo.devices),
    };

    return taiheDeviceBlockStatusInfo;
}

VolumeEvent TaiheParamUtils::SetValueVolumeEvent(const OHOS::AudioStandard::VolumeEvent &volumeEvent)
{
    OHOS::AudioStandard::AudioStreamType audioVolumeType =
        static_cast<OHOS::AudioStandard::AudioStreamType>(volumeEvent.volumeType);
    VolumeEvent taiheVolumeEvent {
        .volumeType = TaiheAudioEnum::GetJsAudioVolumeType(audioVolumeType),
        .volume = volumeEvent.volume,
        .updateUi = volumeEvent.updateUi,
        .volumeGroupId = volumeEvent.volumeGroupId,
        .networkId = ::taihe::string(volumeEvent.networkId),
    };
    return taiheVolumeEvent;
}

StreamVolumeEvent TaiheParamUtils::SetValueStreamVolumeEvent(const OHOS::AudioStandard::StreamVolumeEvent &volumeEvent)
{
    StreamVolumeEvent taiheStreamVolumeEvent {
        .streamUsage = TaiheAudioEnum::ToTaiheStreamUsage(volumeEvent.streamUsage),
        .volume = volumeEvent.volume,
        .updateUi = volumeEvent.updateUi,
    };
    return taiheStreamVolumeEvent;
}

taihe::array<StreamUsage> TaiheParamUtils::SetValueStreamUsageArray(
    const std::vector<OHOS::AudioStandard::StreamUsage> &streamUsageArray)
{
    std::vector<StreamUsage> result;
    for (const auto &streamUsage : streamUsageArray) {
        result.emplace_back(TaiheAudioEnum::GetJsStreamUsage(streamUsage));
    }
    return taihe::array<StreamUsage>(result);
}

taihe::array<AudioVolumeType> TaiheParamUtils::SetValueAudioVolumeTypeArray(
    const std::vector<OHOS::AudioStandard::AudioVolumeType> &volumeTypeArray)
{
    std::vector<AudioVolumeType> result;
    for (const auto &volumeType : volumeTypeArray) {
        result.emplace_back(TaiheAudioEnum::GetJsAudioVolumeType(volumeType));
    }
    return taihe::array<AudioVolumeType>(result);
}

AudioCapturerChangeInfo TaiheParamUtils::SetAudioCapturerChangeInfoDescriptors(
    const OHOS::AudioStandard::AudioCapturerChangeInfo &changeInfo)
{
    AudioCapturerChangeInfo audioCapturerChangeInfo {
        .streamId = changeInfo.sessionId,
        .clientUid = changeInfo.clientUID,
        .capturerInfo = ToTaiheCapturerInfo(changeInfo.capturerInfo),
        .capturerState = TaiheAudioEnum::ToTaiheAudioState(changeInfo.capturerState),
        .deviceDescriptors = SetValueDeviceInfo(changeInfo.inputDeviceInfo),
        .muted = taihe::optional<bool>(std::in_place_t{}, changeInfo.muted),
    };
    return audioCapturerChangeInfo;
}

AudioDeviceDescriptor TaiheParamUtils::SetDeviceDescriptor(
    const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo)
{
    std::vector<int32_t> sampleRatesVec;
    sampleRatesVec.reserve(deviceInfo.GetDeviceStreamInfo().samplingRate.size());
    for (const auto &samplingRate : deviceInfo.GetDeviceStreamInfo().samplingRate) {
        sampleRatesVec.emplace_back(static_cast<int32_t>(samplingRate));
    }
    std::vector<int32_t> channelCountsVec;
    std::set<OHOS::AudioStandard::AudioChannel> channelSet = deviceInfo.GetDeviceStreamInfo().GetChannels();
    channelCountsVec.reserve(channelSet.size());
    for (const auto &channel : channelSet) {
        channelCountsVec.emplace_back(static_cast<int32_t>(channel));
    }
    std::vector<int32_t> channelMasksVec = {deviceInfo.channelMasks_};
    std::vector<int32_t> channelIndexMasksVec = {deviceInfo.channelIndexMasks_};
    std::vector<AudioEncodingType> encodingVec = {
        TaiheAudioEnum::ToTaiheAudioEncodingType(deviceInfo.GetDeviceStreamInfo().encoding)
    };
    taihe::array<AudioEncodingType> encodingArray = taihe::array<AudioEncodingType>(encodingVec);
    AudioDeviceDescriptor taiheDescriptor {
        .deviceRole = TaiheAudioEnum::ToTaiheDeviceRole(deviceInfo.deviceRole_),
        .deviceType = TaiheAudioEnum::ToTaiheDeviceType(deviceInfo.deviceType_),
        .id = deviceInfo.deviceId_,
        .name = taihe::string(deviceInfo.deviceName_),
        .address = taihe::string(deviceInfo.macAddress_),
        .sampleRates = taihe::array<int32_t>(sampleRatesVec),
        .channelCounts = taihe::array<int32_t>(channelCountsVec),
        .channelMasks = taihe::array<int32_t>(channelMasksVec),
        .networkId = taihe::string(deviceInfo.networkId_),
        .interruptGroupId = deviceInfo.interruptGroupId_,
        .volumeGroupId = deviceInfo.volumeGroupId_,
        .displayName = taihe::string(deviceInfo.displayName_),
        .encodingTypes = optional<taihe::array<AudioEncodingType>>(std::in_place_t{}, encodingArray),
        .spatializationSupported = optional<bool>(std::in_place_t{}, deviceInfo.spatializationSupported_),
        .dmDeviceType = optional<int32_t>(std::in_place_t{}, static_cast<int32_t>(deviceInfo.dmDeviceType_)),
    };
    return taiheDescriptor;
}

DeviceChangeAction TaiheParamUtils::SetValueDeviceChangeAction(const OHOS::AudioStandard::DeviceChangeAction &action)
{
    DeviceChangeAction deviceChangeAction {
        .type = TaiheAudioEnum::ToTaiheDeviceChangeType(action.type),
        .deviceDescriptors = TaiheParamUtils::SetDeviceDescriptors(action.deviceDescriptors),
    };
    return deviceChangeAction;
}

AudioSessionStateChangedEvent TaiheParamUtils::SetValueAudioSessionStateChangedEvent(
    const OHOS::AudioStandard::AudioSessionStateChangedEvent &event)
{
    AudioSessionStateChangedEvent sessionStateChangedEvent {
        .stateChangeHint = TaiheAudioEnum::ToTaiheAudioSessionStateChangeHint(event.stateChangeHint),
    };
    return sessionStateChangedEvent;
}

CurrentOutputDeviceChangedEvent TaiheParamUtils::SetValueCurrentOutputDeviceChangedEvent(
    const OHOS::AudioStandard::CurrentOutputDeviceChangedEvent &event)
{
    CurrentOutputDeviceChangedEvent currentOutputDeviceChangedEvent {
        .devices = TaiheParamUtils::SetDeviceDescriptors(event.devices),
        .changeReason = TaiheAudioEnum::ToTaiheAudioStreamDeviceChangeReason(event.changeReason),
        .recommendedAction = TaiheAudioEnum::ToTaiheOutputDeviceChangeRecommendedAction(event.recommendedAction),
    };
    return currentOutputDeviceChangedEvent;
}

taihe::array<AudioDeviceDescriptor> TaiheParamUtils::SetDeviceDescriptors(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &deviceDescriptors)
{
    std::vector<AudioDeviceDescriptor> result;
    for (const auto &deviceDescriptor : deviceDescriptors) {
        if (deviceDescriptor != nullptr) {
            result.emplace_back(SetDeviceDescriptor(deviceDescriptor));
        }
    }
    return taihe::array<AudioDeviceDescriptor>(result);
}

void TaiheParamUtils::ConvertDeviceInfoToAudioDeviceDescriptor(
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor,
    const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo)
{
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "audioDeviceDescriptor is nullptr");
    audioDeviceDescriptor->deviceRole_ = deviceInfo.deviceRole_;
    audioDeviceDescriptor->deviceType_ = deviceInfo.deviceType_;
    audioDeviceDescriptor->deviceId_ = deviceInfo.deviceId_;
    audioDeviceDescriptor->channelMasks_ = deviceInfo.channelMasks_;
    audioDeviceDescriptor->channelIndexMasks_ = deviceInfo.channelIndexMasks_;
    audioDeviceDescriptor->deviceName_ = deviceInfo.deviceName_;
    audioDeviceDescriptor->macAddress_ = deviceInfo.macAddress_;
    audioDeviceDescriptor->interruptGroupId_ = deviceInfo.interruptGroupId_;
    audioDeviceDescriptor->volumeGroupId_ = deviceInfo.volumeGroupId_;
    audioDeviceDescriptor->networkId_ = deviceInfo.networkId_;
    audioDeviceDescriptor->displayName_ = deviceInfo.displayName_;
    audioDeviceDescriptor->audioStreamInfo_ = deviceInfo.audioStreamInfo_;
}

taihe::array<AudioDeviceDescriptor> TaiheParamUtils::SetValueDeviceInfo(
    const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo)
{
    std::vector<AudioDeviceDescriptor> emptyResult;
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> deviceDescriptors;
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> audioDeviceDescriptor =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor != nullptr, array<AudioDeviceDescriptor>(emptyResult),
        "audioDeviceDescriptor malloc failed");
    ConvertDeviceInfoToAudioDeviceDescriptor(audioDeviceDescriptor, deviceInfo);
    deviceDescriptors.push_back(std::move(audioDeviceDescriptor));
    return SetDeviceDescriptors(deviceDescriptors);
}

taihe::array<AudioRendererChangeInfo> TaiheParamUtils::SetRendererChangeInfos(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> &changeInfos)
{
    std::vector<AudioRendererChangeInfo> result;
    for (const auto &changeInfo : changeInfos) {
        if (changeInfo != nullptr) {
            result.emplace_back(ToTaiheAudioRendererChangeInfo(changeInfo));
        }
    }
    return taihe::array<AudioRendererChangeInfo>(result);
}

taihe::array<AudioCapturerChangeInfo> TaiheParamUtils::SetCapturerChangeInfos(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> &changeInfos)
{
    std::vector<AudioCapturerChangeInfo> result;
    for (const auto &changeInfo : changeInfos) {
        if (changeInfo != nullptr) {
            result.emplace_back(ToTaiheAudioCapturerChangeInfo(changeInfo));
        }
    }
    return taihe::array<AudioCapturerChangeInfo>(result);
}

taihe::array<AudioEffectMode> TaiheParamUtils::SetEffectInfo(
    const OHOS::AudioStandard::AudioSceneEffectInfo &audioSceneEffectInfo)
{
    std::vector<AudioEffectMode> result;
    for (const auto &mode : audioSceneEffectInfo.mode) {
        result.emplace_back(TaiheAudioEnum::ToTaiheAudioEffectMode(mode));
    }
    return taihe::array<AudioEffectMode>(result);
}

taihe::array<VolumeGroupInfo> TaiheParamUtils::SetVolumeGroupInfos(
    const std::vector<OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo>> &volumeGroupInfos)
{
    std::vector<VolumeGroupInfo> result;
    for (const auto &volumeGroupInfo : volumeGroupInfos) {
        if (volumeGroupInfo != nullptr) {
            result.emplace_back(ToTaiheVolumeGroupInfo(volumeGroupInfo));
        }
    }
    return taihe::array<VolumeGroupInfo>(result);
}

taihe::array<taihe::string> TaiheParamUtils::ToTaiheArrayString(const std::vector<std::string> &src)
{
    std::vector<::taihe::string> vec;
    for (const auto &item : src) {
        vec.emplace_back(item);
    }
    return taihe::array<string>(vec);
}

taihe::array<uint8_t> TaiheParamUtils::ToTaiheArrayBuffer(uint8_t *src, size_t srcLen)
{
    if (src == nullptr || srcLen == 0) {
        return taihe::array<uint8_t>(0);
    }
    return taihe::array<uint8_t>(copy_data_t{}, src, srcLen);
}

bool TaiheParamUtils::IsSameRef(std::shared_ptr<uintptr_t> src, std::shared_ptr<uintptr_t> dst)
{
    CHECK_AND_RETURN_RET_LOG(src != nullptr, false, "src is null");
    CHECK_AND_RETURN_RET_LOG(dst != nullptr, false, "dst is null");
    std::shared_ptr<taihe::callback<void()>> srcPtr = std::reinterpret_pointer_cast<taihe::callback<void()>>(src);
    std::shared_ptr<taihe::callback<void()>> dstPtr = std::reinterpret_pointer_cast<taihe::callback<void()>>(dst);
    CHECK_AND_RETURN_RET_LOG(srcPtr != nullptr, false, "srcPtr is null");
    CHECK_AND_RETURN_RET_LOG(dstPtr != nullptr, false, "dstPtr is null");
    return *srcPtr == *dstPtr;
}
} // namespace ANI::Audio
