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
    namespace {
        enum class AudioCJVolumeType {
            VOLUMETYPE_DEFAULT = -1,
            VOICE_CALL = 0,
            RINGTONE = 2,
            MEDIA = 3,
            ALARM = 4,
            ACCESSIBILITY = 5,
            SYSTEM = 6,
            VOICE_ASSISTANT = 9,
            ULTRASONIC = 10,
            VOLUMETYPE_MAX,
            ALL = 100
        };
        }
        
        AudioVolumeType GetNativeAudioVolumeType(int32_t volumeType)
        {
            AudioVolumeType result = STREAM_MUSIC;
        
            switch (static_cast<AudioCJVolumeType>(volumeType)) {
                case AudioCJVolumeType::VOICE_CALL:
                    result = STREAM_VOICE_CALL;
                    break;
                case AudioCJVolumeType::RINGTONE:
                    result = STREAM_RING;
                    break;
                case AudioCJVolumeType::MEDIA:
                    result = STREAM_MUSIC;
                    break;
                case AudioCJVolumeType::ALARM:
                    result = STREAM_ALARM;
                    break;
                case AudioCJVolumeType::ACCESSIBILITY:
                    result = STREAM_ACCESSIBILITY;
                    break;
                case AudioCJVolumeType::VOICE_ASSISTANT:
                    result = STREAM_VOICE_ASSISTANT;
                    break;
                case AudioCJVolumeType::ULTRASONIC:
                    result = STREAM_ULTRASONIC;
                    break;
                case AudioCJVolumeType::SYSTEM:
                    result = STREAM_SYSTEM;
                    break;
                case AudioCJVolumeType::ALL:
                    result = STREAM_ALL;
                    break;
                default:
                    result = STREAM_MUSIC;
                    break;
            }
        
            return result;
        }

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

void Convert2CArrDeviceDescriptorByDeviceInfo(CArrDeviceDescriptor &devices, const AudioDeviceDescriptor &deviceInfo,
    int32_t *errorCode)
{
    size_t deviceSize = 1;
    int32_t mallocSize = static_cast<int32_t>(sizeof(CDeviceDescriptor) * deviceSize);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(CDeviceDescriptor) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    CDeviceDescriptor *device = static_cast<CDeviceDescriptor *>(malloc(mallocSize));
    if (device == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    if (memset_s(device, mallocSize, 0, mallocSize) != EOK) {
        *errorCode = CJ_ERR_SYSTEM;
        free(device);
        return;
    }
    devices.head = device;
    devices.size = static_cast<int64_t>(deviceSize);
    for (int32_t i = 0; i < static_cast<int32_t>(deviceSize); i++) {
        Convert2CDeviceDescriptor(&(device[i]), deviceInfo, errorCode);
        if (*errorCode != SUCCESS_CODE) {
            return;
        }
    }
}

void InitializeDeviceChannels(CDeviceDescriptor *device, const AudioDeviceDescriptor &deviceInfo,
    int32_t *errorCode)
{
    size_t channelSize = deviceInfo.audioStreamInfo_.channels.size();
    if (channelSize == 0 || channelSize > MAX_MEM_MALLOC_SIZE) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(int32_t) * channelSize);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(int32_t) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto channels = static_cast<int32_t *>(malloc(mallocSize));
    if (channels == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }

    if (memset_s(channels, mallocSize, 0, mallocSize) != EOK) {
        *errorCode = CJ_ERR_SYSTEM;
        free(channels);
        return;
    }
    int32_t iter = 0;
    device->channelCounts.size = static_cast<int64_t>(channelSize);
    device->channelCounts.head = channels;
    for (auto channel : deviceInfo.audioStreamInfo_.channels) {
        channels[iter] = static_cast<int32_t>(channel);
        iter++;
    }
}

void InitializeDeviceRates(CDeviceDescriptor *device, const AudioDeviceDescriptor &deviceInfo,
    int32_t *errorCode)
{
    size_t rateSize = deviceInfo.audioStreamInfo_.samplingRate.size();
    if (rateSize == 0 || rateSize > MAX_MEM_MALLOC_SIZE) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(int32_t) * rateSize);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(int32_t) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto rates = static_cast<int32_t *>(malloc(mallocSize));
    if (rates == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }

    if (memset_s(rates, mallocSize, 0, mallocSize) != EOK) {
        *errorCode = CJ_ERR_SYSTEM;
        free(rates);
        return;
    }
    int32_t iter = 0;
    device->sampleRates.size = static_cast<int64_t>(rateSize);
    device->sampleRates.head = rates;
    for (auto rate : deviceInfo.audioStreamInfo_.samplingRate) {
        rates[iter] = static_cast<int32_t>(rate);
        iter++;
    }
}

void Convert2CDeviceDescriptor(CDeviceDescriptor *device, const AudioDeviceDescriptor &deviceInfo, int32_t *errorCode)
{
    int32_t deviceSize = 1;
    device->deviceRole = static_cast<int32_t>(deviceInfo.deviceRole_);
    device->deviceType = static_cast<int32_t>(deviceInfo.deviceType_);
    device->displayName = MallocCString(deviceInfo.displayName_);
    device->address = MallocCString(deviceInfo.macAddress_);
    device->name = MallocCString(deviceInfo.deviceName_);
    device->id = deviceInfo.deviceId_;

    InitializeDeviceRates(device, deviceInfo, errorCode);
    InitializeDeviceChannels(device, deviceInfo, errorCode);
    if (*errorCode != SUCCESS_CODE) {
        return;
    }
    int32_t mallocSize = static_cast<int32_t>(sizeof(int32_t)) * deviceSize;
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(int32_t) * MAX_MEM_MALLOC_SIZE)) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto masks = static_cast<int32_t *>(malloc(mallocSize));
    if (masks == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }

    if (memset_s(masks, mallocSize, 0, mallocSize) != EOK) {
        *errorCode = CJ_ERR_SYSTEM;
        free(masks);
        return;
    }
    int32_t iter = 0;
    device->channelMasks.size = static_cast<int64_t>(deviceSize);
    device->channelMasks.head = masks;
    masks[iter] = static_cast<int32_t>(deviceInfo.channelMasks_);
    
    auto encodings = static_cast<int32_t *>(malloc(mallocSize));
    if (encodings == nullptr) {
        *errorCode = CJ_ERR_NO_MEMORY;
        return;
    }
    if (memset_s(encodings, mallocSize, 0, mallocSize) != EOK) {
        *errorCode = CJ_ERR_SYSTEM;
        free(encodings);
        return;
    }
    device->encodingTypes.hasValue = true;
    device->encodingTypes.arr.size = static_cast<int64_t>(deviceSize);
    device->encodingTypes.arr.head = encodings;
    encodings[iter] = static_cast<int32_t>(deviceInfo.audioStreamInfo_.encoding);
}

void Convert2CArrDeviceDescriptor(CArrDeviceDescriptor &devices,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescriptors, int32_t *errorCode)
{
    if (deviceDescriptors.empty()) {
        *errorCode = CJ_ERR_SYSTEM;
        return;
    } else {
        auto deviceSize = deviceDescriptors.size();
        devices.size = static_cast<int64_t>(deviceSize);
        int32_t mallocSize = static_cast<int32_t>(sizeof(CDeviceDescriptor)) * static_cast<int32_t>(deviceSize);
        if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(CDeviceDescriptor) * MAX_MEM_MALLOC_SIZE)) {
            *errorCode = CJ_ERR_SYSTEM;
            return;
        }
        CDeviceDescriptor *device = static_cast<CDeviceDescriptor *>(malloc(mallocSize));
        if (device == nullptr) {
            *errorCode = CJ_ERR_NO_MEMORY;
            return;
        }
        if (memset_s(device, mallocSize, 0, mallocSize) != EOK) {
            *errorCode = CJ_ERR_SYSTEM;
            free(device);
            return;
        }
        devices.head = device;
        for (int32_t i = 0; i < static_cast<int32_t>(deviceSize); i++) {
            AudioDeviceDescriptor dInfo(AudioDeviceDescriptor::DEVICE_INFO);
            ConvertAudioDeviceDescriptor2DeviceInfo(dInfo, deviceDescriptors[i]);
            Convert2CDeviceDescriptor(&(device[i]), dInfo, errorCode);
            if (*errorCode != SUCCESS_CODE) {
                return;
            }
        }
    }
}

void ConvertAudioDeviceDescriptor2DeviceInfo(AudioDeviceDescriptor &deviceInfo,
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor)
{
    deviceInfo.deviceRole_ = audioDeviceDescriptor->deviceRole_;
    deviceInfo.deviceType_ = audioDeviceDescriptor->deviceType_;
    deviceInfo.deviceId_ = audioDeviceDescriptor->deviceId_;
    deviceInfo.channelMasks_ = audioDeviceDescriptor->channelMasks_;
    deviceInfo.channelIndexMasks_ = audioDeviceDescriptor->channelIndexMasks_;
    deviceInfo.deviceName_ = audioDeviceDescriptor->deviceName_;
    deviceInfo.macAddress_ = audioDeviceDescriptor->macAddress_;
    deviceInfo.interruptGroupId_ = audioDeviceDescriptor->interruptGroupId_;
    deviceInfo.volumeGroupId_ = audioDeviceDescriptor->volumeGroupId_;
    deviceInfo.networkId_ = audioDeviceDescriptor->networkId_;
    deviceInfo.displayName_ = audioDeviceDescriptor->displayName_;
    deviceInfo.audioStreamInfo_.samplingRate = audioDeviceDescriptor->audioStreamInfo_.samplingRate;
    deviceInfo.audioStreamInfo_.encoding = audioDeviceDescriptor->audioStreamInfo_.encoding;
    deviceInfo.audioStreamInfo_.format = audioDeviceDescriptor->audioStreamInfo_.format;
    deviceInfo.audioStreamInfo_.channels = audioDeviceDescriptor->audioStreamInfo_.channels;
}

void FreeCDeviceDescriptor(CDeviceDescriptor &device)
{
    free(device.address);
    device.address = nullptr;
    free(device.displayName);
    device.displayName = nullptr;
    free(device.name);
    device.name = nullptr;
    if (device.channelCounts.size != 0) {
        free(device.channelCounts.head);
    }
    device.channelCounts.head = nullptr;
    if (device.channelMasks.size != 0) {
        free(device.channelMasks.head);
    }
    device.channelMasks.head = nullptr;
    if (device.sampleRates.size != 0) {
        free(device.sampleRates.head);
    }
    device.sampleRates.head = nullptr;
    if (device.encodingTypes.hasValue && device.encodingTypes.arr.size != 0) {
        free(device.encodingTypes.arr.head);
    }
    device.encodingTypes.arr.head = nullptr;
}

void FreeCArrDeviceDescriptor(CArrDeviceDescriptor &devices)
{
    if (devices.head == nullptr) {
        return;
    }
    for (int64_t i = 0; i < devices.size; i++) {
        FreeCDeviceDescriptor((devices.head)[i]);
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

void FreeCArrAudioRendererChangeInfo(CArrAudioRendererChangeInfo &infos)
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

void Convert2AudioRendererOptions(AudioRendererOptions &opions, const CAudioRendererOptions &cOptions)
{
    opions.rendererInfo.streamUsage = static_cast<StreamUsage>(cOptions.audioRendererInfo.usage);
    opions.streamInfo.channels = static_cast<AudioChannel>(cOptions.audioStreamInfo.channels);
    opions.streamInfo.channelLayout = static_cast<AudioChannelLayout>(cOptions.audioStreamInfo.channelLayout);
    opions.streamInfo.encoding = static_cast<AudioEncodingType>(cOptions.audioStreamInfo.encodingType);
    opions.streamInfo.format = static_cast<AudioSampleFormat>(cOptions.audioStreamInfo.sampleFormat);
    opions.streamInfo.samplingRate = static_cast<AudioSamplingRate>(cOptions.audioStreamInfo.samplingRate);
    opions.privacyType = static_cast<AudioPrivacyType>(cOptions.privacyType);

    /* only support flag 0 */
    opions.rendererInfo.rendererFlags =
        (cOptions.audioRendererInfo.rendererFlags != 0) ? 0 : cOptions.audioRendererInfo.rendererFlags;
}

void Convert2AudioRendererInfo(CAudioRendererInfo &cInfo, const AudioRendererInfo &rendererInfo)
{
    cInfo.usage = static_cast<int32_t>(rendererInfo.streamUsage);
    cInfo.rendererFlags = rendererInfo.rendererFlags;
}

void Convert2CAudioRendererChangeInfo(CAudioRendererChangeInfo &cInfo, const AudioRendererChangeInfo &changeInfo,
    int32_t *errorCode)
{
    cInfo.streamId = changeInfo.sessionId;
    Convert2CArrDeviceDescriptorByDeviceInfo(cInfo.deviceDescriptors, changeInfo.outputDeviceInfo, errorCode);
    Convert2AudioRendererInfo(cInfo.rendererInfo, changeInfo.rendererInfo);
}
} // namespace AudioStandard
} // namespace OHOS
