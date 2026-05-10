/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef HDI_ADAPTER_TYPE_H
#define HDI_ADAPTER_TYPE_H

#include <iostream>
#include <cstring>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
// if attr struct change, please check ipc serialize and deserialize code
typedef struct IAudioSinkAttr : public Parcelable {
    std::string adapterName = "";
    std::string sinkName = "";
    uint32_t openMicSpeaker = 0;
    bool auxSinkEnable = false;
    AudioEncodingType encodingType = ENCODING_PCM;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    uint32_t period = 0;
    float volume = 0.0f;
    std::string filePath;
    std::string deviceNetworkId;
    int32_t deviceRole = 0;
    int32_t deviceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    std::string address;
    std::string aux = "";
    uint32_t routeFlag = 0;
    bool isLoopback = false;
    AudioPin pin = AUDIO_PIN_NONE;
    int32_t hdPlayBackMode = 0;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(adapterName) &&
            parcel.WriteString(sinkName) &&
            parcel.WriteUint32(openMicSpeaker) &&
            parcel.WriteBool(auxSinkEnable) &&
            parcel.WriteUint32(static_cast<uint32_t>(encodingType)) &&
            parcel.WriteUint8(static_cast<uint8_t>(format)) &&
            parcel.WriteUint32(sampleRate) &&
            parcel.WriteUint32(channel) &&
            parcel.WriteUint32(period) &&
            parcel.WriteFloat(volume) &&
            parcel.WriteString(filePath) &&
            parcel.WriteString(deviceNetworkId) &&
            parcel.WriteInt32(deviceType) &&
            parcel.WriteUint64(channelLayout) &&
            parcel.WriteInt32(audioStreamFlag) &&
            parcel.WriteString(address) &&
            parcel.WriteString(aux) &&
            parcel.WriteUint32(routeFlag) &&
            parcel.WriteInt32(hdPlayBackMode);
    }

    static IAudioSinkAttr *Unmarshalling(Parcel &parcel)
    {
        auto attr = new(std::nothrow) IAudioSinkAttr();
        if (attr == nullptr) {
            return nullptr;
        }

        attr->adapterName = parcel.ReadString();
        attr->sinkName = parcel.ReadString();
        attr->openMicSpeaker = parcel.ReadUint32();
        attr->auxSinkEnable = parcel.ReadBool();
        attr->encodingType = static_cast<AudioEncodingType>(parcel.ReadUint32());
        attr->format = static_cast<AudioSampleFormat>(parcel.ReadUint8());
        attr->sampleRate = parcel.ReadUint32();
        attr->channel = parcel.ReadUint32();
        attr->period = parcel.ReadUint32();
        attr->volume = parcel.ReadFloat();
        attr->filePath = parcel.ReadString();
        attr->deviceNetworkId = parcel.ReadString();
        attr->deviceType = parcel.ReadInt32();
        attr->channelLayout = parcel.ReadUint64();
        attr->audioStreamFlag = parcel.ReadInt32();
        attr->address = parcel.ReadString();
        attr->aux = parcel.ReadString();
        attr->routeFlag = parcel.ReadUint32();
        attr->hdPlayBackMode = parcel.ReadInt32();
        return attr;
    }
} IAudioSinkAttr;

typedef struct IAudioSourceAttr : public Parcelable {
    std::string adapterName = "";
    uint32_t openMicSpeaker = 0;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRate = 0;
    uint32_t channel = 0;
    float volume = 0.0f;
    uint32_t bufferSize = 0;
    bool isBigEndian = false;
    std::string filePath = "";
    std::string deviceNetworkId = "";
    std::string macAddress = "";
    int32_t deviceType = 0;
    int32_t sourceType = 0;
    uint64_t channelLayout = 0;
    int32_t audioStreamFlag = 0;
    bool hasEcConfig = false;
    AudioSampleFormat formatEc = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRateEc = 0;
    uint32_t channelEc = 0;
    std::string hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    AudioSampleFormat formatMicIn = AudioSampleFormat::INVALID_WIDTH;
    uint32_t sampleRateMicIn = 0;
    uint32_t channelMicIn = 0;
    uint32_t routeFlag = 0;
    bool isPrimarySinkExist_ = true;

    // struct should use Marshalling Serialization
    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(adapterName) &&
            parcel.WriteUint32(openMicSpeaker) &&
            parcel.WriteUint8(static_cast<uint8_t>(format)) &&
            parcel.WriteUint32(sampleRate) &&
            parcel.WriteUint32(channel) &&
            parcel.WriteFloat(volume) &&
            parcel.WriteUint32(bufferSize) &&
            parcel.WriteBool(isBigEndian) &&
            parcel.WriteString(filePath) &&
            parcel.WriteString(deviceNetworkId) &&
            parcel.WriteString(macAddress) &&
            parcel.WriteInt32(deviceType) &&
            parcel.WriteInt32(sourceType) &&
            parcel.WriteUint64(channelLayout) &&
            parcel.WriteInt32(audioStreamFlag) &&
            parcel.WriteBool(hasEcConfig) &&
            parcel.WriteUint8(static_cast<uint8_t>(formatEc)) &&
            parcel.WriteUint32(sampleRateEc) &&
            parcel.WriteUint32(channelEc) &&
            parcel.WriteString(hdiSourceType) &&
            parcel.WriteUint8(static_cast<uint8_t>(formatMicIn)) &&
            parcel.WriteUint32(sampleRateMicIn) &&
            parcel.WriteUint32(channelMicIn) &&
            parcel.WriteUint32(routeFlag);
    }

    static IAudioSourceAttr *Unmarshalling(Parcel &parcel)
    {
        auto attr = new(std::nothrow) IAudioSourceAttr();
        if (attr == nullptr) {
            return nullptr;
        }

        attr->adapterName = parcel.ReadString();
        attr->openMicSpeaker = parcel.ReadUint32();
        attr->format = static_cast<AudioSampleFormat>(parcel.ReadUint8());
        attr->sampleRate = parcel.ReadUint32();
        attr->channel = parcel.ReadUint32();
        attr->volume = parcel.ReadFloat();
        attr->bufferSize = parcel.ReadUint32();
        attr->isBigEndian = parcel.ReadBool();
        attr->filePath = parcel.ReadString();
        attr->deviceNetworkId = parcel.ReadString();
        attr->macAddress = parcel.ReadString();
        attr->deviceType = parcel.ReadInt32();
        attr->sourceType = parcel.ReadInt32();
        attr->channelLayout = parcel.ReadUint64();
        attr->audioStreamFlag = parcel.ReadInt32();
        attr->hasEcConfig = parcel.ReadBool();
        attr->formatEc = static_cast<AudioSampleFormat>(parcel.ReadUint8());
        attr->sampleRateEc = parcel.ReadUint32();
        attr->channelEc = parcel.ReadUint32();
        attr->hdiSourceType = parcel.ReadString();
        attr->formatMicIn = static_cast<AudioSampleFormat>(parcel.ReadUint8());
        attr->sampleRateMicIn = parcel.ReadUint32();
        attr->channelMicIn = parcel.ReadUint32();
        attr->routeFlag = parcel.ReadUint32();
        return attr;
    }
} IAudioSourceAttr;

typedef struct FrameDesc {
    char *frame;
    uint64_t frameLen;
} FrameDesc;

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_ADAPTER_TYPE_H
