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
#ifndef LOG_TAG
#define LOG_TAG "NapiParamUtils"
#endif

#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
napi_value NapiParamUtils::GetUndefinedValue(napi_env env)
{
    napi_value result {};
    napi_get_undefined(env, &result);
    return result;
}

napi_status NapiParamUtils::GetParam(const napi_env &env, napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_value thisVar = nullptr;
    void *data;
    return napi_get_cb_info(env, info, &argc, args, &thisVar, &data);
}

napi_status NapiParamUtils::GetValueInt32(const napi_env &env, int32_t &value, napi_value in)
{
    napi_status status = napi_get_value_int32(env, in, &value);
    CHECK_AND_RETURN_RET_PRELOG(status == napi_ok, status, "GetValueInt32 napi_get_value_int32 failed");
    return status;
}

napi_status NapiParamUtils::SetValueInt32(const napi_env &env, const int32_t &value, napi_value &result)
{
    napi_status status = napi_create_int32(env, value, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32 napi_create_int32 failed");
    return status;
}

napi_status NapiParamUtils::GetValueInt32(const napi_env &env, const std::string &fieldStr,
    int32_t &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueInt32 napi_get_named_property failed");
    status = GetValueInt32(env, value, jsValue);
    return status;
}

napi_status NapiParamUtils::SetValueInt32(const napi_env &env, const std::string &fieldStr,
    const int32_t value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueInt32(env, value, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32 napi_create_int32 failed");
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32 napi_create_int32 failed");
    return status;
}

napi_status NapiParamUtils::GetValueUInt32(const napi_env &env, uint32_t &value, napi_value in)
{
    napi_status status = napi_get_value_uint32(env, in, &value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueUInt32 napi_get_value_uint32 failed");
    return status;
}

napi_status NapiParamUtils::SetValueUInt32(const napi_env &env, const uint32_t &value, napi_value &result)
{
    napi_status status = napi_create_uint32(env, value, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueUInt32 napi_create_uint32 failed");
    return status;
}

napi_status NapiParamUtils::GetValueDouble(const napi_env &env, double &value, napi_value in)
{
    napi_status status = napi_get_value_double(env, in, &value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueDouble napi_get_value_double failed");
    return status;
}

napi_status NapiParamUtils::SetValueDouble(const napi_env &env, const double &value, napi_value &result)
{
    napi_status status = napi_create_double(env, value, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueDouble napi_create_double failed");
    return status;
}

napi_status NapiParamUtils::GetValueDouble(const napi_env &env, const std::string &fieldStr,
    double &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueDouble napi_get_named_property failed");
    status = GetValueDouble(env, value, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueDouble failed");
    return status;
}

napi_status NapiParamUtils::SetValueDouble(const napi_env &env, const std::string &fieldStr,
    const double value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueDouble(env, value, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueDouble SetValueDouble failed");
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueDouble napi_set_named_property failed");
    return status;
}

std::string NapiParamUtils::GetPropertyString(napi_env env, napi_value value, const std::string &fieldStr)
{
    std::string invalid = "";
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, fieldStr.c_str(), &exist);
    if (status != napi_ok || !exist) {
        AUDIO_ERR_LOG("can not find %{public}s property", fieldStr.c_str());
        return invalid;
    }

    napi_value item = nullptr;
    if (napi_get_named_property(env, value, fieldStr.c_str(), &item) != napi_ok) {
        AUDIO_ERR_LOG("get %{public}s property fail", fieldStr.c_str());
        return invalid;
    }

    return GetStringArgument(env, item);
}

std::string NapiParamUtils::GetStringArgument(napi_env env, napi_value value)
{
    std::string strValue = "";
    size_t bufLength = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLength);
    if (status == napi_ok && bufLength > 0 && bufLength < PATH_MAX) {
        strValue.reserve(bufLength + 1);
        strValue.resize(bufLength);
        status = napi_get_value_string_utf8(env, value, strValue.data(), bufLength + 1, &bufLength);
        if (status == napi_ok) {
            AUDIO_DEBUG_LOG("argument = %{public}s", strValue.c_str());
        }
    }
    return strValue;
}

napi_status NapiParamUtils::SetValueString(const napi_env &env, const std::string &stringValue, napi_value &result)
{
    napi_status status = napi_create_string_utf8(env, stringValue.c_str(), NAPI_AUTO_LENGTH, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueString napi_create_string_utf8 failed");
    return status;
}

napi_status NapiParamUtils::SetValueString(const napi_env &env, const std::string &fieldStr,
    const std::string &stringValue, napi_value &result)
{
    napi_value value = nullptr;
    napi_status status = SetValueString(env, stringValue.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueString failed");
    status = napi_set_named_property(env, result, fieldStr.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueString napi_set_named_property failed");
    return status;
}

napi_status NapiParamUtils::GetValueBoolean(const napi_env &env, bool &boolValue, napi_value in)
{
    napi_status status = napi_get_value_bool(env, in, &boolValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueBoolean napi_get_boolean failed");
    return status;
}

napi_status NapiParamUtils::SetValueBoolean(const napi_env &env, const bool boolValue, napi_value &result)
{
    napi_status status = napi_get_boolean(env, boolValue, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueBoolean napi_get_boolean failed");
    return status;
}

napi_status NapiParamUtils::GetValueBoolean(const napi_env &env, const std::string &fieldStr,
    bool &boolValue, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueBoolean napi_get_named_property failed");
    status = GetValueBoolean(env, boolValue, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueBoolean failed");
    return status;
}

napi_status NapiParamUtils::SetValueBoolean(const napi_env &env, const std::string &fieldStr,
    const bool boolValue, napi_value &result)
{
    napi_value value = nullptr;
    napi_status status = SetValueBoolean(env, boolValue, value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueBoolean SetValueBoolean failed");
    napi_set_named_property(env, result, fieldStr.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueBoolean napi_get_boolean failed");
    return status;
}

napi_status NapiParamUtils::GetValueInt64(const napi_env &env, int64_t &value, napi_value in)
{
    napi_status status = napi_get_value_int64(env, in, &value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueInt64 napi_get_value_int64 failed");
    return status;
}

napi_status NapiParamUtils::SetValueInt64(const napi_env &env, const int64_t &value, napi_value &result)
{
    napi_status status = napi_create_int64(env, value, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt64 napi_create_int64 failed");
    return status;
}

napi_status NapiParamUtils::GetValueInt64(const napi_env &env, const std::string &fieldStr,
    int64_t &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueInt64 napi_get_named_property failed");
    status = GetValueInt64(env, value, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetValueInt64 failed");
    return status;
}

napi_status NapiParamUtils::SetValueInt64(const napi_env &env, const std::string &fieldStr,
    const int64_t value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueInt64(env, value, jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt64 failed");
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt64 napi_set_named_property failed");
    return status;
}

napi_status NapiParamUtils::GetArrayBuffer(const napi_env &env, void* &data, size_t &length, napi_value in)
{
    napi_status status = napi_get_arraybuffer_info(env, in, &data, &length);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetArrayBuffer napi_get_arraybuffer_info failed");
    return status;
}

napi_status NapiParamUtils::CreateArrayBuffer(const napi_env &env, const std::string &fieldStr, size_t bufferLen,
    uint8_t *bufferData, napi_value &result)
{
    napi_value value = nullptr;

    napi_status status = napi_create_arraybuffer(env, bufferLen, (void**)&bufferData, &value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "napi_create_arraybuffer failed");
    status = napi_set_named_property(env, result, fieldStr.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "napi_set_named_property failed");

    return status;
}

napi_status NapiParamUtils::CreateArrayBuffer(const napi_env &env, const size_t bufferLen,
    const uint8_t *bufferData, napi_value &result)
{
    uint8_t *native = nullptr;
    napi_status status = napi_create_arraybuffer(env, bufferLen, reinterpret_cast<void **>(&native), &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "napi_create_arraybuffer failed");
    if (memcpy_s(native, bufferLen, bufferData, bufferLen)) {
        result = nullptr;
    }
    return status;
}

void NapiParamUtils::ConvertDeviceInfoToAudioDeviceDescriptor(sptr<AudioDeviceDescriptor> audioDeviceDescriptor,
    const DeviceInfo &deviceInfo)
{
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "audioDeviceDescriptor is nullptr");
    audioDeviceDescriptor->deviceRole_ = deviceInfo.deviceRole;
    audioDeviceDescriptor->deviceType_ = deviceInfo.deviceType;
    audioDeviceDescriptor->deviceId_ = deviceInfo.deviceId;
    audioDeviceDescriptor->channelMasks_ = deviceInfo.channelMasks;
    audioDeviceDescriptor->channelIndexMasks_ = deviceInfo.channelIndexMasks;
    audioDeviceDescriptor->deviceName_ = deviceInfo.deviceName;
    audioDeviceDescriptor->macAddress_ = deviceInfo.macAddress;
    audioDeviceDescriptor->interruptGroupId_ = deviceInfo.interruptGroupId;
    audioDeviceDescriptor->volumeGroupId_ = deviceInfo.volumeGroupId;
    audioDeviceDescriptor->networkId_ = deviceInfo.networkId;
    audioDeviceDescriptor->displayName_ = deviceInfo.displayName;
    audioDeviceDescriptor->audioStreamInfo_.samplingRate = deviceInfo.audioStreamInfo.samplingRate;
    audioDeviceDescriptor->audioStreamInfo_.encoding = deviceInfo.audioStreamInfo.encoding;
    audioDeviceDescriptor->audioStreamInfo_.format = deviceInfo.audioStreamInfo.format;
    audioDeviceDescriptor->audioStreamInfo_.channels = deviceInfo.audioStreamInfo.channels;
}

napi_status NapiParamUtils::GetRendererOptions(const napi_env &env, AudioRendererOptions *opts, napi_value in)
{
    napi_value res = nullptr;

    napi_status status = napi_get_named_property(env, in, "rendererInfo", &res);
    if (status == napi_ok) {
        GetRendererInfo(env, &(opts->rendererInfo), res);
    }

    status = napi_get_named_property(env, in, "streamInfo", &res);
    if (status == napi_ok) {
        GetStreamInfo(env, &(opts->streamInfo), res);
    }

    int32_t intValue = {};
    status = GetValueInt32(env, "privacyType", intValue, in);
    if (status == napi_ok) {
        opts->privacyType = static_cast<AudioPrivacyType>(intValue);
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetRendererInfo(const napi_env &env, AudioRendererInfo *rendererInfo, napi_value in)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, in, &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, napi_invalid_arg,
        "GetRendererInfo failed, vauleType is not object");

    int32_t intValue = {0};
    napi_status status = GetValueInt32(env, "content", intValue, in);
    if (status == napi_ok) {
        rendererInfo->contentType = static_cast<ContentType>(intValue);
    }

    status = GetValueInt32(env, "usage", intValue, in);
    if (status == napi_ok) {
        if (NapiAudioEnum::IsLegalInputArgumentStreamUsage(intValue)) {
            rendererInfo->streamUsage = static_cast<StreamUsage>(intValue);
        } else {
            rendererInfo->streamUsage = StreamUsage::STREAM_USAGE_INVALID;
        }
    }

    GetValueInt32(env, "rendererFlags", rendererInfo->rendererFlags, in);

    return napi_ok;
}

napi_status NapiParamUtils::SetRendererInfo(const napi_env &env, const AudioRendererInfo &rendererInfo,
    napi_value &result)
{
    napi_status status = napi_create_object(env, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetRendererInfo napi_create_object failed");
    SetValueInt32(env, "content", static_cast<int32_t>(rendererInfo.contentType), result);
    SetValueInt32(env, "usage", static_cast<int32_t>(rendererInfo.streamUsage), result);
    SetValueInt32(env, "rendererFlags", rendererInfo.rendererFlags, result);

    return napi_ok;
}

napi_status NapiParamUtils::GetStreamInfo(const napi_env &env, AudioStreamInfo *streamInfo, napi_value in)
{
    int32_t intValue = {0};
    napi_status status = GetValueInt32(env, "samplingRate", intValue, in);
    if (status == napi_ok) {
        if (intValue >= SAMPLE_RATE_8000 && intValue <= SAMPLE_RATE_192000) {
            streamInfo->samplingRate = static_cast<AudioSamplingRate>(intValue);
        } else {
            AUDIO_ERR_LOG("invaild samplingRate");
            return napi_generic_failure;
        }
    }

    status = GetValueInt32(env, "channels", intValue, in);
    if (status == napi_ok) {
        streamInfo->channels = static_cast<AudioChannel>(intValue);
    }

    status = GetValueInt32(env, "sampleFormat", intValue, in);
    if (status == napi_ok) {
        streamInfo->format = static_cast<OHOS::AudioStandard::AudioSampleFormat>(intValue);
    }

    status = GetValueInt32(env, "encodingType", intValue, in);
    if (status == napi_ok) {
        streamInfo->encoding = static_cast<AudioEncodingType>(intValue);
    }

    int64_t int64Value = 0;
    status = GetValueInt64(env, "channelLayout", int64Value, in);
    if (status == napi_ok) {
        streamInfo->channelLayout = static_cast<AudioChannelLayout>(int64Value);
    }

    return napi_ok;
}

napi_status NapiParamUtils::SetStreamInfo(const napi_env &env, const AudioStreamInfo &streamInfo, napi_value &result)
{
    napi_status status = napi_create_object(env, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetStreamInfo napi_create_object failed");
    SetValueInt32(env, "samplingRate", static_cast<int32_t>(streamInfo.samplingRate), result);
    SetValueInt32(env, "channels", static_cast<int32_t>(streamInfo.channels), result);
    SetValueInt32(env, "sampleFormat", static_cast<int32_t>(streamInfo.format), result);
    SetValueInt32(env, "encodingType", static_cast<int32_t>(streamInfo.encoding), result);
    SetValueInt64(env, "channelLayout", static_cast<uint64_t>(streamInfo.channelLayout), result);

    return napi_ok;
}

napi_status NapiParamUtils::SetValueInt32Element(const napi_env &env, const std::string &fieldStr,
    const std::vector<int32_t> &values, napi_value &result)
{
    napi_value jsValues = nullptr;
    napi_status status = napi_create_array_with_length(env, values.size(), &jsValues);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32Element napi_create_array_with_length failed");
    size_t count = 0;
    for (const auto &value : values) {
        napi_value jsValue = nullptr;
        status = SetValueInt32(env, value, jsValue);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32Element SetValueInt32 failed");
        status = napi_set_element(env, jsValues, count, jsValue);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32Element napi_set_element failed");
        count++;
    }
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValues);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueInt32Element napi_set_named_property failed");
    return status;
}

napi_status NapiParamUtils::SetDeviceDescriptor(const napi_env &env, const AudioDeviceDescriptor &deviceInfo,
    napi_value &result)
{
    (void)napi_create_object(env, &result);
    SetValueInt32(env, "deviceRole", static_cast<int32_t>(deviceInfo.deviceRole_), result);
    SetValueInt32(env, "deviceType", static_cast<int32_t>(deviceInfo.deviceType_), result);
    SetValueInt32(env, "id", static_cast<int32_t>(deviceInfo.deviceId_), result);
    SetValueString(env, "name", deviceInfo.deviceName_, result);
    SetValueString(env, "address", deviceInfo.macAddress_, result);
    SetValueString(env, "networkId", deviceInfo.networkId_, result);
    SetValueString(env, "displayName", deviceInfo.displayName_, result);
    SetValueInt32(env, "interruptGroupId", static_cast<int32_t>(deviceInfo.interruptGroupId_), result);
    SetValueInt32(env, "volumeGroupId", static_cast<int32_t>(deviceInfo.volumeGroupId_), result);

    napi_value value = nullptr;
    napi_value sampleRates;
    size_t size = deviceInfo.audioStreamInfo_.samplingRate.size();
    napi_create_array_with_length(env, size, &sampleRates);
    size_t count = 0;
    for (const auto &samplingRate : deviceInfo.audioStreamInfo_.samplingRate) {
        napi_create_int32(env, samplingRate, &value);
        napi_set_element(env, sampleRates, count, value);
        count++;
    }
    napi_set_named_property(env, result, "sampleRates", sampleRates);

    napi_value channelCounts;
    size = deviceInfo.audioStreamInfo_.channels.size();
    napi_create_array_with_length(env, size, &channelCounts);
    count = 0;
    for (const auto &channels : deviceInfo.audioStreamInfo_.channels) {
        napi_create_int32(env, channels, &value);
        napi_set_element(env, channelCounts, count, value);
        count++;
    }
    napi_set_named_property(env, result, "channelCounts", channelCounts);

    std::vector<int32_t> channelMasks_;
    channelMasks_.push_back(deviceInfo.channelMasks_);
    SetValueInt32Element(env, "channelMasks", channelMasks_, result);

    std::vector<int32_t> channelIndexMasks_;
    channelIndexMasks_.push_back(deviceInfo.channelIndexMasks_);
    SetValueInt32Element(env, "channelIndexMasks", channelIndexMasks_, result);

    std::vector<int32_t> encoding;
    encoding.push_back(deviceInfo.audioStreamInfo_.encoding);
    SetValueInt32Element(env, "encodingTypes", encoding, result);

    return napi_ok;
}

napi_status NapiParamUtils::SetDeviceDescriptors(const napi_env &env,
    const std::vector<sptr<AudioDeviceDescriptor>> &deviceDescriptors, napi_value &result)
{
    napi_status status = napi_create_array_with_length(env, deviceDescriptors.size(), &result);
    for (size_t i = 0; i < deviceDescriptors.size(); i++) {
        if (deviceDescriptors[i] != nullptr) {
            napi_value valueParam = nullptr;
            SetDeviceDescriptor(env, deviceDescriptors[i], valueParam);
            napi_set_element(env, result, i, valueParam);
        }
    }
    return status;
}
napi_status NapiParamUtils::SetAudioSpatialEnabledStateForDevice(const napi_env &env,
    const AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice, napi_value &result)
{
    (void)napi_create_object(env, &result);
    napi_value jsArray;
    NapiParamUtils::SetDeviceDescriptor(env, audioSpatialEnabledStateForDevice.deviceDescriptor, jsArray);
    napi_set_named_property(env, result, "deviceDescriptor", jsArray);

    NapiParamUtils::SetValueBoolean(env, "enabled", audioSpatialEnabledStateForDevice.enabled, result);
    return napi_ok;
}

napi_status NapiParamUtils::SetValueDeviceInfo(const napi_env &env, const DeviceInfo &deviceInfo, napi_value &result)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    CHECK_AND_RETURN_RET_LOG(audioDeviceDescriptor != nullptr, napi_generic_failure,
        "audioDeviceDescriptor malloc failed");
    ConvertDeviceInfoToAudioDeviceDescriptor(audioDeviceDescriptor, deviceInfo);
    deviceDescriptors.push_back(std::move(audioDeviceDescriptor));
    SetDeviceDescriptors(env, deviceDescriptors, result);
    return napi_ok;
}

napi_status NapiParamUtils::SetInterruptEvent(const napi_env &env, const InterruptEvent &interruptEvent,
    napi_value &result)
{
    napi_create_object(env, &result);
    SetValueInt32(env, "eventType", static_cast<int32_t>(interruptEvent.eventType), result);
    SetValueInt32(env, "forceType", static_cast<int32_t>(interruptEvent.forceType), result);
    SetValueInt32(env, "hintType", static_cast<int32_t>(interruptEvent.hintType), result);
    return napi_ok;
}

napi_status NapiParamUtils::SetNativeAudioRendererDataInfo(const napi_env &env,
    const AudioRendererDataInfo &audioRendererDataInfo, napi_value &result)
{
    napi_status status = napi_create_object(env, &result);

    SetValueInt32(env, "flag", static_cast<int32_t>(audioRendererDataInfo.flag), result);
    CreateArrayBuffer(env, "buffer", audioRendererDataInfo.flag, audioRendererDataInfo.buffer, result);

    return status;
}

napi_status NapiParamUtils::GetCapturerInfo(const napi_env &env, AudioCapturerInfo *capturerInfo, napi_value in)
{
    int32_t intValue = {0};
    napi_status status = NapiParamUtils::GetValueInt32(env, "source", intValue, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetCapturerInfo GetValueInt32 source failed");
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalCapturerType(intValue),
        napi_generic_failure, "Invailed captureType");
    capturerInfo->sourceType = static_cast<SourceType>(intValue);

    status = NapiParamUtils::GetValueInt32(env, "capturerFlags", capturerInfo->capturerFlags, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetCapturerInfo GetValueInt32 capturerFlags failed");
    return status;
}

napi_status NapiParamUtils::SetCapturerInfo(const napi_env &env,
    const AudioCapturerInfo &capturerInfo, napi_value &result)
{
    napi_status status = napi_create_object(env, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetCapturerInfo napi_create_object failed");
    SetValueInt32(env, "source", static_cast<int32_t>(capturerInfo.sourceType), result);
    SetValueInt32(env, "capturerFlags", static_cast<int32_t>(capturerInfo.capturerFlags), result);

    return napi_ok;
}

napi_status NapiParamUtils::GetCaptureFilterOptionsVector(const napi_env &env,
    CaptureFilterOptions *filterOptions, napi_value in)
{
    napi_value usagesValue = nullptr;
    napi_status status = napi_get_named_property(env, in, "usages", &usagesValue);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, napi_ok, "GetUsages failed");

    uint32_t arrayLen = 0;
    napi_get_array_length(env, usagesValue, &arrayLen);

    if (arrayLen == 0) {
        filterOptions->usages = {};
        AUDIO_INFO_LOG("ParseCaptureFilterOptions get empty usage");
        return napi_ok;
    }

    for (size_t i = 0; i < static_cast<size_t>(arrayLen); i++) {
        napi_value element;
        if (napi_get_element(env, usagesValue, i, &element) == napi_ok) {
            int32_t val = {0};
            napi_get_value_int32(env, element, &val);
            filterOptions->usages.emplace_back(static_cast<StreamUsage>(val));
        }
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetPlaybackCaptureConfig(const napi_env &env,
    AudioPlaybackCaptureConfig *captureConfig, napi_value in)
{
    napi_value res = nullptr;

    if (napi_get_named_property(env, in, "filterOptions", &res) == napi_ok) {
        return GetCaptureFilterOptionsVector(env, &(captureConfig->filterOptions), res);
    }

    return NapiParamUtils::GetCaptureFilterOptionsVector(env, &(captureConfig->filterOptions), res);
}

napi_status NapiParamUtils::GetCapturerOptions(const napi_env &env, AudioCapturerOptions *opts, napi_value in)
{
    napi_value result = nullptr;
    napi_status status = napi_get_named_property(env, in, "streamInfo", &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get streamInfo name failed");

    status = GetStreamInfo(env, &(opts->streamInfo), result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "ParseStreamInfo failed");

    status = napi_get_named_property(env, in, "capturerInfo", &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get capturerInfo name failed");

    status = GetCapturerInfo(env, &(opts->capturerInfo), result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "ParseCapturerInfo failed");

    if (napi_get_named_property(env, in, "playbackCaptureConfig", &result) == napi_ok) {
        return GetPlaybackCaptureConfig(env, &(opts->playbackCaptureConfig), result);
    }

    AUDIO_INFO_LOG("ParseCapturerOptions, without playbackCaptureConfig");
    return napi_ok;
}

napi_status NapiParamUtils::SetAudioCapturerChangeInfoDescriptors(const napi_env &env,
    const AudioCapturerChangeInfo &changeInfo, napi_value &result)
{
    napi_status status = napi_create_object(env, &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "audioDeviceDescriptor malloc failed");
    SetValueInt32(env, "streamId", changeInfo.sessionId, result);
    SetValueInt32(env, "clientUid", changeInfo.clientUID, result);
    SetValueInt32(env, "capturerState", static_cast<int32_t>(changeInfo.capturerState), result);
    SetValueBoolean(env, "muted", changeInfo.muted, result);

    napi_value jsCapInfoObj = nullptr;
    SetCapturerInfo(env, changeInfo.capturerInfo, jsCapInfoObj);
    napi_set_named_property(env, result, "capturerInfo", jsCapInfoObj);

    napi_value deviceInfo = nullptr;
    SetValueDeviceInfo(env, changeInfo.inputDeviceInfo, deviceInfo);
    napi_set_named_property(env, result, "deviceDescriptors", deviceInfo);
    return status;
}

napi_status NapiParamUtils::SetMicrophoneDescriptor(const napi_env &env, const sptr<MicrophoneDescriptor> &micDesc,
    napi_value &result)
{
    napi_create_object(env, &result);
    napi_value jsPositionObj = nullptr;
    napi_value jsOrientationObj = nullptr;

    SetValueInt32(env, "id", micDesc->micId_, result);
    SetValueInt32(env, "deviceType", static_cast<int32_t>(micDesc->deviceType_), result);
    SetValueInt32(env, "groupId", micDesc->groupId_, result);
    SetValueInt32(env, "sensitivity", micDesc->sensitivity_, result);
    napi_create_object(env, &jsPositionObj);
    SetValueDouble(env, "x", micDesc->position_.x, jsPositionObj);
    SetValueDouble(env, "y", micDesc->position_.y, jsPositionObj);
    SetValueDouble(env, "z", micDesc->position_.z, jsPositionObj);
    napi_set_named_property(env, result, "position", jsPositionObj);

    napi_create_object(env, &jsOrientationObj);
    SetValueDouble(env, "x", micDesc->orientation_.x, jsOrientationObj);
    SetValueDouble(env, "y", micDesc->orientation_.y, jsOrientationObj);
    SetValueDouble(env, "z", micDesc->orientation_.z, jsOrientationObj);
    napi_set_named_property(env, result, "orientation", jsOrientationObj);
    return napi_ok;
}

napi_status NapiParamUtils::SetMicrophoneDescriptors(const napi_env &env,
    const std::vector<sptr<MicrophoneDescriptor>> &micDescs, napi_value &result)
{
    napi_status status = napi_create_array_with_length(env, micDescs.size(), &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "audioDeviceDescriptor malloc failed");
    int32_t index = 0;
    for (const auto &micDesc : micDescs) {
        napi_value valueParam = nullptr;
        SetMicrophoneDescriptor(env, micDesc, valueParam);
        napi_set_element(env, result, index, valueParam);
        index++;
    }
    return status;
}

napi_status NapiParamUtils::SetValueMicStateChange(const napi_env &env, const MicStateChangeEvent &micStateChangeEvent,
    napi_value &result)
{
    napi_create_object(env, &result);
    NapiParamUtils::SetValueBoolean(env, "mute", micStateChangeEvent.mute, result);
    return napi_ok;
}

napi_status NapiParamUtils::SetVolumeGroupInfos(const napi_env &env,
    const std::vector<sptr<VolumeGroupInfo>> &volumeGroupInfos, napi_value &result)
{
    napi_value valueParam = nullptr;
    napi_status status = napi_create_array_with_length(env, volumeGroupInfos.size(), &result);
    for (size_t i = 0; i < volumeGroupInfos.size(); i++) {
        if (volumeGroupInfos[i] != nullptr) {
            (void)napi_create_object(env, &valueParam);
            SetValueString(env, "networkId", static_cast<std::string>(
                volumeGroupInfos[i]->networkId_), valueParam);
            SetValueInt32(env, "groupId", static_cast<int32_t>(
                volumeGroupInfos[i]->volumeGroupId_), valueParam);
            SetValueInt32(env, "mappingId", static_cast<int32_t>(
                volumeGroupInfos[i]->mappingId_), valueParam);
            SetValueString(env, "groupName", static_cast<std::string>(
                volumeGroupInfos[i]->groupName_), valueParam);
            SetValueInt32(env, "ConnectType", static_cast<int32_t>(
                volumeGroupInfos[i]->connectType_), valueParam);
            napi_set_element(env, result, i, valueParam);
        }
    }
    return status;
}

napi_status NapiParamUtils::SetValueVolumeEvent(const napi_env& env, const VolumeEvent &volumeEvent,
    napi_value &result)
{
    napi_create_object(env, &result);
    SetValueInt32(env, "volumeType",
        NapiAudioEnum::GetJsAudioVolumeType(static_cast<AudioStreamType>(volumeEvent.volumeType)), result);
    SetValueInt32(env, "volume", static_cast<int32_t>(volumeEvent.volume), result);
    SetValueBoolean(env, "updateUi", volumeEvent.updateUi, result);
    SetValueInt32(env, "volumeGroupId", volumeEvent.volumeGroupId, result);
    SetValueString(env, "networkId", volumeEvent.networkId, result);
    return napi_ok;
}

napi_status NapiParamUtils::GetAudioDeviceDescriptor(const napi_env &env,
    sptr<AudioDeviceDescriptor> &selectedAudioDevice, bool &argTransFlag, napi_value in)
{
    int32_t intValue = {0};
    argTransFlag = true;
    bool hasDeviceRole = true;
    bool hasNetworkId  = true;
    napi_status status = napi_has_named_property(env, in, "deviceRole", &hasDeviceRole);
    status = napi_has_named_property(env, in, "networkId", &hasNetworkId);
    if ((!hasDeviceRole) || (!hasNetworkId)) {
        argTransFlag = false;
        return status;
    }

    status = GetValueInt32(env, "deviceRole", intValue, in);
    if (status == napi_ok) {
        if (std::find(DEVICE_ROLE_SET.begin(), DEVICE_ROLE_SET.end(), intValue) == DEVICE_ROLE_SET.end()) {
            argTransFlag = false;
            return status;
        }
        selectedAudioDevice->deviceRole_ = static_cast<DeviceRole>(intValue);
    }

    status = GetValueInt32(env, "deviceType", intValue, in);
    if (status == napi_ok) {
        if (std::find(DEVICE_TYPE_SET.begin(), DEVICE_TYPE_SET.end(), intValue) == DEVICE_TYPE_SET.end()) {
            argTransFlag = false;
            return status;
        }
        selectedAudioDevice->deviceType_ = static_cast<DeviceType>(intValue);
    }

    selectedAudioDevice->networkId_ = GetPropertyString(env, in, "networkId");

    selectedAudioDevice->displayName_ = GetPropertyString(env, in, "displayName");

    status = GetValueInt32(env, "interruptGroupId", intValue, in);
    if (status == napi_ok) {
        selectedAudioDevice->interruptGroupId_ = intValue;
    }

    status = GetValueInt32(env, "volumeGroupId", intValue, in);
    if (status == napi_ok) {
        selectedAudioDevice->volumeGroupId_ = intValue;
    }

    selectedAudioDevice->macAddress_ = GetPropertyString(env, in, "address");

    status = GetValueInt32(env, "id", intValue, in);
    if (status == napi_ok) {
        selectedAudioDevice->deviceId_ = intValue;
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetAudioDeviceDescriptorVector(const napi_env &env,
    std::vector<sptr<AudioDeviceDescriptor>> &deviceDescriptorsVector, bool &argTransFlag, napi_value in)
{
    uint32_t arrayLen = 0;
    napi_get_array_length(env, in, &arrayLen);
    if (arrayLen == 0) {
        deviceDescriptorsVector = {};
        AUDIO_INFO_LOG("Error: AudioDeviceDescriptor vector is NULL!");
    }

    for (size_t i = 0; i < arrayLen; i++) {
        napi_value element;
        napi_get_element(env, in, i, &element);
        sptr<AudioDeviceDescriptor> selectedAudioDevice = new(std::nothrow) AudioDeviceDescriptor();
        GetAudioDeviceDescriptor(env, selectedAudioDevice, argTransFlag, element);
        if (!argTransFlag) {
            return napi_ok;
        }
        deviceDescriptorsVector.push_back(selectedAudioDevice);
    }
    return napi_ok;
}

napi_status NapiParamUtils::GetAudioCapturerFilter(const napi_env &env, sptr<AudioCapturerFilter> &audioCapturerFilter,
    napi_value in)
{
    int32_t intValue = {0};
    audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();

    napi_status status = GetValueInt32(env, "uid", intValue, in);
    if (status == napi_ok) {
        audioCapturerFilter->uid = intValue;
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetAudioCapturerInfo(const napi_env &env, AudioCapturerInfo *capturerInfo, napi_value in)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, in, &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, napi_invalid_arg,
        "GetRendererInfo failed, vauleType is not object");

    int32_t intValue = {0};
    napi_value tempValue = nullptr;
    napi_status status = napi_get_named_property(env, in, "source", &tempValue);
    if (status == napi_ok) {
        GetValueInt32(env, intValue, tempValue);
        if (NapiAudioEnum::IsValidSourceType(intValue)) {
            capturerInfo->sourceType = static_cast<SourceType>(intValue);
        } else {
            capturerInfo->sourceType = SourceType::SOURCE_TYPE_INVALID;
        }
    }

    status = GetValueInt32(env, "capturerFlags", intValue, in);
    if (status == napi_ok) {
        capturerInfo->capturerFlags = intValue;
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetAudioRendererFilter(const napi_env &env, sptr<AudioRendererFilter> &audioRendererFilter,
    bool &argTransFlag, napi_value in)
{
    napi_value tempValue = nullptr;
    int32_t intValue = {0};
    argTransFlag = true;
    audioRendererFilter = new(std::nothrow) AudioRendererFilter();

    napi_status status = GetValueInt32(env, "uid", intValue, in);
    if (status == napi_ok) {
        audioRendererFilter->uid = intValue;
    }

    if (napi_get_named_property(env, in, "rendererInfo", &tempValue) == napi_ok) {
        GetRendererInfo(env, &(audioRendererFilter->rendererInfo), tempValue);
    }

    status = GetValueInt32(env, "rendererId", intValue, in);
    if (status == napi_ok) {
        audioRendererFilter->streamId = intValue;
    }

    return napi_ok;
}

napi_status NapiParamUtils::SetValueDeviceChangeAction(const napi_env& env, const DeviceChangeAction &action,
    napi_value &result)
{
    napi_create_object(env, &result);
    NapiParamUtils::SetValueInt32(env, "type", static_cast<int32_t>(action.type), result);

    napi_value jsArray;
    NapiParamUtils::SetDeviceDescriptors(env, action.deviceDescriptors, jsArray);
    napi_set_named_property(env, result, "deviceDescriptors", jsArray);
    return napi_ok;
}

napi_status NapiParamUtils::SetRendererChangeInfos(const napi_env &env,
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &changeInfos, napi_value &result)
{
    int32_t position = 0;
    napi_value jsChangeInfoObj = nullptr;
    napi_value jsRenInfoObj = nullptr;
    napi_create_array_with_length(env, changeInfos.size(), &result);
    for (const auto &changeInfo : changeInfos) {
        if (changeInfo) {
            napi_create_object(env, &jsChangeInfoObj);
            SetValueInt32(env, "streamId", changeInfo->sessionId, jsChangeInfoObj);
            SetValueInt32(env, "rendererState", static_cast<int32_t>(changeInfo->rendererState), jsChangeInfoObj);
            SetValueInt32(env, "clientUid", changeInfo->clientUID, jsChangeInfoObj);
            SetRendererInfo(env, changeInfo->rendererInfo, jsRenInfoObj);
            napi_set_named_property(env, jsChangeInfoObj, "rendererInfo", jsRenInfoObj);
            napi_value deviceInfo = nullptr;
            SetValueDeviceInfo(env, changeInfo->outputDeviceInfo, deviceInfo);
            napi_set_named_property(env, jsChangeInfoObj, "deviceDescriptors", deviceInfo);
            napi_set_element(env, result, position, jsChangeInfoObj);
            position++;
        }
    }
    return napi_ok;
}

napi_status NapiParamUtils::SetCapturerChangeInfos(const napi_env &env,
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &changeInfos, napi_value &result)
{
    int32_t position = 0;
    napi_value jsChangeInfoObj = nullptr;
    napi_create_array_with_length(env, changeInfos.size(), &result);
    for (const auto &changeInfo : changeInfos) {
        if (changeInfo) {
            SetAudioCapturerChangeInfoDescriptors(env, *changeInfo, jsChangeInfoObj);
            napi_set_element(env, result, position, jsChangeInfoObj);
            position++;
        }
    }
    return napi_ok;
}

napi_status NapiParamUtils::SetEffectInfo(const napi_env &env,
    const AudioSceneEffectInfo &audioSceneEffectInfo, napi_value &result)
{
    int32_t position = 0;
    napi_value jsEffectInofObj = nullptr;
    napi_create_array_with_length(env, audioSceneEffectInfo.mode.size(), &result);
    napi_create_object(env, &jsEffectInofObj);
    for (const auto &mode : audioSceneEffectInfo.mode) {
        SetValueUInt32(env, mode, jsEffectInofObj);
        napi_set_element(env, result, position, jsEffectInofObj);
        position++;
    }
    return napi_ok;
}

napi_status NapiParamUtils::GetAudioInterrupt(const napi_env &env, AudioInterrupt &audioInterrupt,
    napi_value in)
{
    int32_t propValue = -1;
    napi_status status = NapiParamUtils::GetValueInt32(env, "contentType", propValue, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetAudioInterrupt: Failed to retrieve contentType");
    audioInterrupt.contentType = static_cast<ContentType>(propValue);

    status = NapiParamUtils::GetValueInt32(env, "streamUsage", propValue, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetAudioInterrupt: Failed to retrieve streamUsage");
    audioInterrupt.streamUsage = static_cast<StreamUsage>(propValue);

    status = NapiParamUtils::GetValueBoolean(env, "pauseWhenDucked", audioInterrupt.pauseWhenDucked, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "GetAudioInterrupt: Failed to retrieve pauseWhenDucked");
    audioInterrupt.audioFocusType.streamType = AudioSystemManager::GetStreamType(audioInterrupt.contentType,
        audioInterrupt.streamUsage);
    return status;
}

napi_status NapiParamUtils::SetValueInterruptAction(const napi_env &env, const InterruptAction &interruptAction,
    napi_value &result)
{
    napi_create_object(env, &result);
    SetValueInt32(env, "actionType", static_cast<int32_t>(interruptAction.actionType), result);
    SetValueInt32(env, "type", static_cast<int32_t>(interruptAction.interruptType), result);
    SetValueInt32(env, "hint", static_cast<int32_t>(interruptAction.interruptHint), result);
    SetValueBoolean(env, "activated", interruptAction.activated, result);
    return napi_ok;
}

napi_status NapiParamUtils::GetSpatialDeviceState(napi_env env, AudioSpatialDeviceState *spatialDeviceState,
    napi_value in)
{
    napi_value res = nullptr;
    int32_t intValue = {0};
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_get_named_property(env, in, "address", &res);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "Get address name failed");
    napi_typeof(env, res, &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_string, napi_invalid_arg, "Get address type failed");
    spatialDeviceState->address = NapiParamUtils::GetStringArgument(env, res);

    status = GetValueBoolean(env, "isSpatializationSupported", spatialDeviceState->isSpatializationSupported, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "Get isSpatializationSupported failed");

    status = GetValueBoolean(env, "isHeadTrackingSupported", spatialDeviceState->isHeadTrackingSupported, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "Get isHeadTrackingSupported failed");

    status = GetValueInt32(env, "spatialDeviceType", intValue, in);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "Get spatialDeviceType failed");
    CHECK_AND_RETURN_RET_LOG((intValue >= EARPHONE_TYPE_NONE) && (intValue <= EARPHONE_TYPE_OTHERS),
        napi_invalid_arg, "Get spatialDeviceType failed");
    spatialDeviceState->spatialDeviceType = static_cast<AudioSpatialDeviceType>(intValue);

    return napi_ok;
}

napi_status NapiParamUtils::GetExtraParametersSubKV(napi_env env,
    std::vector<std::pair<std::string, std::string>> &subKV, napi_value in)
{
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_status status = napi_get_property_names(env, in, &jsProNameList);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get property name failed");
    status = napi_get_array_length(env, jsProNameList, &jsProCount);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get subKeys length failed");

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t i = 0; i < jsProCount; i++) {
        status = napi_get_element(env, jsProNameList, i, &jsProName);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get sub key failed");

        std::string strProName = NapiParamUtils::GetStringArgument(env, jsProName);
        status = napi_get_named_property(env, in, strProName.c_str(), &jsProValue);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "get sub value failed");

        subKV.push_back(std::make_pair(strProName, NapiParamUtils::GetStringArgument(env, jsProValue)));
    }

    return napi_ok;
}

napi_status NapiParamUtils::GetExtraParametersVector(const napi_env &env,
    std::vector<std::string> &subKeys, napi_value in)
{
    uint32_t arrayLen = 0;
    napi_get_array_length(env, in, &arrayLen);

    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value element;
        if (napi_get_element(env, in, i, &element) == napi_ok) {
            subKeys.push_back(GetStringArgument(env, element));
        }
    }

    return napi_ok;
}

napi_status NapiParamUtils::SetExtraAudioParametersInfo(const napi_env &env,
    const std::vector<std::pair<std::string, std::string>> &extraParameters, napi_value &result)
{
    napi_status status = napi_create_array_with_length(env, extraParameters.size(), &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "malloc array buffer failed");

    for (auto it = extraParameters.begin(); it != extraParameters.end(); it++) {
        status = SetValueString(env, it->first, it->second, result);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "SetValueString failed");
    }

    return status;
}

napi_status NapiParamUtils::GetAudioSessionStrategy(const napi_env &env,
    AudioSessionStrategy &audioSessionStrategy, napi_value in)
{
    int32_t intValue = {0};
    napi_status status = napi_generic_failure;
    status = GetValueInt32(env, "concurrencyMode", intValue, in);
    if (status == napi_ok) {
        audioSessionStrategy.concurrencyMode = static_cast<AudioConcurrencyMode>(intValue);
        return napi_ok;
    } else {
        AUDIO_ERR_LOG("invaild concurrencyMode");
        return napi_generic_failure;
    }
}

napi_status NapiParamUtils::SetAudioSessionDeactiveEvent(
    const napi_env &env, const AudioSessionDeactiveEvent &deactiveEvent, napi_value &result)
{
    napi_create_object(env, &result);
    SetValueInt32(env, "reason", static_cast<int32_t>(deactiveEvent.deactiveReason), result);
    return napi_ok;
}
} // namespace AudioStandard
} // namespace OHOS
