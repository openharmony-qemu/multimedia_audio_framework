/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
const size_t MAX_VALID_SIZE = 128;
char *MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char *res = static_cast<char *>(malloc(sizeof(char) * len));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

void Convert2AudioCapturerOptions(AudioCapturerOptions &opions, const CAudioCapturerOptions &cOptions)
{
    opions.capturerInfo.sourceType = static_cast<SourceType>(cOptions.audioCapturerInfo.source);
    opions.streamInfo.channels = static_cast<AudioChannel>(cOptions.audioStreamInfo.channels);
    opions.streamInfo.channelLayout = static_cast<AudioChannelLayout>(cOptions.audioStreamInfo.channelLayout);
    opions.streamInfo.encoding = static_cast<AudioEncodingType>(cOptions.audioStreamInfo.encodingType);
    opions.streamInfo.format = static_cast<AudioSampleFormat>(cOptions.audioStreamInfo.sampleFormat);
    opions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(cOptions.audioStreamInfo.samplingRate);

    /* only support flag 0 */
    opions.capturerInfo.capturerFlags =
        (cOptions.audioCapturerInfo.capturerFlags != 0) ? 0 : cOptions.audioCapturerInfo.capturerFlags;
}

void Convert2CAudioCapturerInfo(CAudioCapturerInfo &cInfo, const AudioCapturerInfo &capturerInfo)
{
    cInfo.capturerFlags = capturerInfo.capturerFlags;
    cInfo.source = static_cast<int32_t>(capturerInfo.sourceType);
}

void Convert2CAudioStreamInfo(CAudioStreamInfo &cInfo, const AudioStreamInfo &streamInfo)
{
    cInfo.channels = static_cast<int32_t>(streamInfo.channels);
    cInfo.encodingType = static_cast<int32_t>(streamInfo.encoding);
    cInfo.sampleFormat = static_cast<int32_t>(streamInfo.format);
    cInfo.samplingRate = static_cast<int32_t>(streamInfo.samplingRate);
    cInfo.channelLayout = static_cast<int64_t>(streamInfo.channelLayout);
}

void Convert2CAudioCapturerChangeInfo(CAudioCapturerChangeInfo &cInfo, const AudioCapturerChangeInfo &changeInfo,
    int32_t *errorCode)
{
    cInfo.muted = changeInfo.muted;
    cInfo.streamId = changeInfo.sessionId;
    Convert2CAudioCapturerInfo(cInfo.audioCapturerInfo, changeInfo.capturerInfo);
    Convert2CArrDeviceDescriptorByDeviceInfo(cInfo.deviceDescriptors, changeInfo.inputDeviceInfo, errorCode);
}

void Convert2CArrDeviceDescriptorByDeviceInfo(CArrDeviceDescriptor &devices, const DeviceInfo &deviceInfo,
    int32_t *errorCode)
{
    size_t deviceSize = 1;
    CDeviceDescriptor *device = static_cast<CDeviceDescriptor *>(malloc(sizeof(CDeviceDescriptor) * deviceSize));
    if (device == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(deviceSize); i++) {
        Convert2CDeviceDescriptor(&(device[i]), deviceInfo, errorCode);
    }
    devices.head = device;
    devices.size = static_cast<int64_t>(deviceSize);
}

void InitializeDeviceRatesAndChannels(CDeviceDescriptor *device, const DeviceInfo &deviceInfo, int32_t *errorCode)
{
    size_t rateSize = deviceInfo.audioStreamInfo.samplingRate.size();
    if (rateSize == 0 || rateSize > MAX_VALID_SIZE) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto rates = static_cast<int32_t *>(malloc(sizeof(int32_t) * rateSize));
    if (rates == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    int32_t iter = 0;
    for (auto rate : deviceInfo.audioStreamInfo.samplingRate) {
        rates[iter] = static_cast<int32_t>(rate);
        iter++;
    }
    iter = 0;
    device->sampleRates.size = rateSize;
    device->sampleRates.head = rates;

    size_t channelSize = deviceInfo.audioStreamInfo.channels.size();
    if (channelSize == 0 || channelSize > MAX_VALID_SIZE) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto channels = static_cast<int32_t *>(malloc(sizeof(int32_t) * channelSize));
    if (channels == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    for (auto channel : deviceInfo.audioStreamInfo.channels) {
        rates[iter] = static_cast<int32_t>(channel);
        iter++;
    }
    iter = 0;
    device->channelCounts.size = channelSize;
    device->channelCounts.head = channels;
}

void Convert2CDeviceDescriptor(CDeviceDescriptor *device, const DeviceInfo &deviceInfo, int32_t *errorCode)
{
    int32_t deviceSize = 1;
    device->deviceRole = static_cast<int32_t>(deviceInfo.deviceRole);
    device->deviceType = static_cast<int32_t>(deviceInfo.deviceType);
    device->displayName = MallocCString(deviceInfo.displayName);
    device->address = MallocCString(deviceInfo.macAddress);
    device->name = MallocCString(deviceInfo.deviceName);
    device->id = deviceInfo.deviceId;

    InitializeDeviceRatesAndChannels(device, deviceInfo, errorCode);
    auto masks = static_cast<int32_t *>(malloc(sizeof(int32_t) * deviceSize));
    if (masks == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    int32_t iter = 0;
    masks[iter] = static_cast<int32_t>(deviceInfo.channelMasks);
    device->channelMasks.size = deviceSize;
    device->channelMasks.head = masks;

    auto encodings = static_cast<int32_t *>(malloc(sizeof(int32_t) * deviceSize));
    if (encodings == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    encodings[iter] = static_cast<int32_t>(deviceInfo.audioStreamInfo.encoding);
    device->encodingTypes.hasValue = true;
    device->encodingTypes.arr.size = deviceSize;
    device->encodingTypes.arr.head = encodings;
}

void Convert2CArrDeviceDescriptor(CArrDeviceDescriptor &devices,
    const std::vector<sptr<AudioDeviceDescriptor>> &deviceDescriptors, int32_t *errorCode)
{
    if (deviceDescriptors.empty()) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    } else {
        devices.size = static_cast<int64_t>(deviceDescriptors.size());
        CDeviceDescriptor *device = static_cast<CDeviceDescriptor *>(malloc(sizeof(CDeviceDescriptor) * devices.size));
        if (device == nullptr) {
            *errorCode = CJ_ERR_NO_MEMORY;
            return;
        }
        for (int32_t i = 0; i < static_cast<int32_t>(deviceDescriptors.size()); i++) {
            DeviceInfo dInfo;
            ConvertAudioDeviceDescriptor2DeviceInfo(dInfo, deviceDescriptors[i]);
            Convert2CDeviceDescriptor(&(device[i]), dInfo, errorCode);
        }
        devices.head = device;
    }
}

void ConvertAudioDeviceDescriptor2DeviceInfo(DeviceInfo &deviceInfo, sptr<AudioDeviceDescriptor> audioDeviceDescriptor)
{
    deviceInfo.deviceRole = audioDeviceDescriptor->deviceRole_;
    deviceInfo.deviceType = audioDeviceDescriptor->deviceType_;
    deviceInfo.deviceId = audioDeviceDescriptor->deviceId_;
    deviceInfo.channelMasks = audioDeviceDescriptor->channelMasks_;
    deviceInfo.channelIndexMasks = audioDeviceDescriptor->channelIndexMasks_;
    deviceInfo.deviceName = audioDeviceDescriptor->deviceName_;
    deviceInfo.macAddress = audioDeviceDescriptor->macAddress_;
    deviceInfo.interruptGroupId = audioDeviceDescriptor->interruptGroupId_;
    deviceInfo.volumeGroupId = audioDeviceDescriptor->volumeGroupId_;
    deviceInfo.networkId = audioDeviceDescriptor->networkId_;
    deviceInfo.displayName = audioDeviceDescriptor->displayName_;
    deviceInfo.audioStreamInfo.samplingRate = audioDeviceDescriptor->audioStreamInfo_.samplingRate;
    deviceInfo.audioStreamInfo.encoding = audioDeviceDescriptor->audioStreamInfo_.encoding;
    deviceInfo.audioStreamInfo.format = audioDeviceDescriptor->audioStreamInfo_.format;
    deviceInfo.audioStreamInfo.channels = audioDeviceDescriptor->audioStreamInfo_.channels;
}

void FreeCArrDeviceDescriptor(CArrDeviceDescriptor &devices)
{
    if (devices.head == nullptr) {
        return;
    }
    for (int64_t i = 0; i < devices.size; i++) {
        free((devices.head)[i].address);
        free((devices.head)[i].displayName);
        free((devices.head)[i].name);
        if ((devices.head)[i].channelCounts.size != 0) {
            free((devices.head)[i].channelCounts.head);
        }
        (devices.head)[i].channelCounts.head = nullptr;
        if ((devices.head)[i].channelMasks.size != 0) {
            free((devices.head)[i].channelMasks.head);
        }
        (devices.head)[i].channelMasks.head = nullptr;
        if ((devices.head)[i].sampleRates.size != 0) {
            free((devices.head)[i].sampleRates.head);
        }
        (devices.head)[i].sampleRates.head = nullptr;
        if ((devices.head)[i].encodingTypes.hasValue && (devices.head)[i].encodingTypes.arr.size != 0) {
            free((devices.head)[i].encodingTypes.arr.head);
        }
        (devices.head)[i].encodingTypes.arr.head = nullptr;
    }
    free(devices.head);
    devices.head = nullptr;
}

void FreeCArrAudioCapturerChangeInfo(CArrAudioCapturerChangeInfo &infos)
{
    if (infos.head == nullptr) {
        return;
    }
    for (int64_t i = 0; i < infos.size; i++) {
        FreeCArrDeviceDescriptor((infos.head)[i].deviceDescriptors);
    }
    free(infos.head);
    infos.head = nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
