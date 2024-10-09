/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "OHAudioDeviceDescriptor.h"

static OHOS::AudioStandard::OHAudioDeviceDescriptor *convertDeviceDescriptor(
    OH_AudioDeviceDescriptor* deviceDescriptor)
{
    return (OHOS::AudioStandard::OHAudioDeviceDescriptor*) deviceDescriptor;
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceRole(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                             OH_AudioDevice_Role *deviceRole)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(deviceRole != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceRole is nullptr");
    return deviceDescriptor->GetDeviceRole(deviceRole);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceType(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                             OH_AudioDevice_Type *deviceType)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(deviceType != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "deviceType is nullptr");
    return deviceDescriptor->GetDeviceType(deviceType);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceId(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                           uint32_t *id)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(id != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "id is nullptr");
    return deviceDescriptor->GetDeviceId(id);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceName(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                             char **name)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(name != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "name is nullptr");
    return deviceDescriptor->GetDeviceName(name);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceAddress(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                                char **address)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(address != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "address is nullptr");
    return deviceDescriptor->GetDeviceAddress(address);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceSampleRates(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                                    uint32_t **sampleRates, uint32_t *size)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(sampleRates != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "sampleRates is nullptr");
    CHECK_AND_RETURN_RET_LOG(size != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "size is nullptr");
    return deviceDescriptor->GetDeviceSampleRates(sampleRates, size);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceChannelCounts(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                                      uint32_t **channelCounts, uint32_t *size)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(channelCounts != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "channelCounts is nullptr");
    CHECK_AND_RETURN_RET_LOG(size != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "size is nullptr");
    return deviceDescriptor->GetDeviceChannelCounts(channelCounts, size);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceDisplayName(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
                                                                    char **displayName)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(displayName != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "displayName is nullptr");
    return deviceDescriptor->GetDeviceDisplayName(displayName);
}

OH_AudioCommon_Result OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(OH_AudioDeviceDescriptor *audioDeviceDescriptor,
    OH_AudioStream_EncodingType **encodingTypes, uint32_t *size)
{
    OHOS::AudioStandard::OHAudioDeviceDescriptor* deviceDescriptor = convertDeviceDescriptor(audioDeviceDescriptor);
    CHECK_AND_RETURN_RET_LOG(deviceDescriptor != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "audioDeviceDescriptor is nullptr");
    CHECK_AND_RETURN_RET_LOG(encodingTypes != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "encodingTypes is nullptr");
    CHECK_AND_RETURN_RET_LOG(size != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "size is nullptr");
    return deviceDescriptor->GetDeviceEncodingTypes(encodingTypes, size);
}

namespace OHOS {
namespace AudioStandard {

OHAudioDeviceDescriptor::OHAudioDeviceDescriptor(sptr<AudioDeviceDescriptor> audioDeviceDescriptor)
    : audioDeviceDescriptor_(audioDeviceDescriptor)
{
    AUDIO_INFO_LOG("OHAudioDeviceDescriptor Constructor is called\n");
}

OHAudioDeviceDescriptor::~OHAudioDeviceDescriptor()
{
    AUDIO_INFO_LOG("~OHAudioDeviceDescriptor is called\n");
    if (audioDeviceDescriptor_ != nullptr) {
        audioDeviceDescriptor_ = nullptr;
    }
    if (audioSamplingRate_ != nullptr) {
        delete audioSamplingRate_;
        audioSamplingRate_ = nullptr;
    }
    if (audioChannel_ != nullptr) {
        delete audioChannel_;
        audioChannel_ = nullptr;
    }
    if (encodingType_ != nullptr) {
        delete encodingType_;
        encodingType_ = nullptr;
    }
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceRole(OH_AudioDevice_Role *deviceRole)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    *deviceRole = (OH_AudioDevice_Role)audioDeviceDescriptor_->deviceRole_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceType(OH_AudioDevice_Type *deviceType)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    *deviceType = (OH_AudioDevice_Type)audioDeviceDescriptor_->deviceType_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceId(uint32_t *id)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    *id = (uint32_t)audioDeviceDescriptor_->deviceId_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceName(char **name)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    const char *deviceName = audioDeviceDescriptor_->deviceName_.c_str();
    *name = const_cast<char*>(deviceName);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceAddress(char **address)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    const char *macAddress = audioDeviceDescriptor_->macAddress_.c_str();
    *address = const_cast<char*>(macAddress);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceSampleRates(uint32_t **sampleRates, uint32_t *size)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    DeviceStreamInfo audioStreamInfo = audioDeviceDescriptor_->audioStreamInfo_;

    uint32_t samplingRateSize = (uint32_t)audioStreamInfo.samplingRate.size();
    if (samplingRateSize == 0) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (audioSamplingRate_ == nullptr) {
        audioSamplingRate_ = new uint32_t[samplingRateSize];
        int index = 0;
        for (const auto samplingRate : audioStreamInfo.samplingRate) {
            audioSamplingRate_[index++]= static_cast<uint32_t>(samplingRate);
        }
    }
    *size = samplingRateSize;
    *sampleRates = audioSamplingRate_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceChannelCounts(uint32_t **channelCounts, uint32_t *size)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    DeviceStreamInfo audioStreamInfo = audioDeviceDescriptor_->audioStreamInfo_;
    uint32_t channelsSize = (uint32_t)audioStreamInfo.channels.size();
    if (channelsSize == 0) {
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (audioChannel_ == nullptr) {
        audioChannel_ = new uint32_t[channelsSize];
        int index = 0;
        for (const auto channels : audioStreamInfo.channels) {
            audioChannel_[index++] = static_cast<uint32_t>(channels);
        }
    }
    *size = channelsSize;
    *channelCounts = audioChannel_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceDisplayName(char **displayName)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    const char *name = audioDeviceDescriptor_->displayName_.c_str();
    *displayName = const_cast<char*>(name);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioDeviceDescriptor::GetDeviceEncodingTypes(OH_AudioStream_EncodingType **encodingTypes,
    uint32_t *size)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor_ != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceDescriptor_ is nullptr");
    DeviceStreamInfo audioStreamInfo = audioDeviceDescriptor_->audioStreamInfo_;
    if (encodingType_ == nullptr) {
        encodingType_ = new OH_AudioStream_EncodingType[1];
        encodingType_[0] = (OH_AudioStream_EncodingType)audioStreamInfo.encoding;
    }
    *size = 1;
    *encodingTypes = encodingType_;
    return AUDIOCOMMON_RESULT_SUCCESS;
}
}
}