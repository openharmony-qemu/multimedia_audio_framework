/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef VA_DEVICE_INFO_H
#define VA_DEVICE_INFO_H

#include "audio_device_info.h"
#include "message_parcel.h"
#include "audio_info.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

static void CheckVADeviceInfoSize(size_t &size)
{
    if (size > AUDIO_DEVICE_INFO_SIZE_LIMIT) {
        size = AUDIO_DEVICE_INFO_SIZE_LIMIT;
    }
}

enum VADeviceRole {
    VA_DEVICE_ROLE_IN = 1,
    VA_DEVICE_ROLE_OUT = 2,
};

enum VADeviceType {
    VA_DEVICE_TYPE_NONE = 0,
    VA_DEVICE_TYPE_BT_SPP = 1,
};

struct VAAudioStreamProperty : public Parcelable {
    AudioEncodingType encoding_;
    uint32_t sampleRate_;
    uint32_t samplesPerCycle_;
    AudioSampleFormat sampleFormat_;
    AudioChannelLayout channelLayout_;

    VAAudioStreamProperty() = default;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(static_cast<int32_t>(encoding_)) &&
            parcel.WriteUint32(sampleRate_) &&
            parcel.WriteUint32(samplesPerCycle_) &&
            parcel.WriteInt32(static_cast<uint8_t>(sampleFormat_)) &&
            parcel.WriteUint64(static_cast<uint64_t>(channelLayout_));
    }

    void UnmarshallingSelf(Parcel& parcel)
    {
        encoding_ = static_cast<AudioEncodingType>(parcel.ReadInt32());
        sampleRate_ = parcel.ReadUint32();
        samplesPerCycle_ = parcel.ReadUint32();
        sampleFormat_ = static_cast<AudioSampleFormat>(parcel.ReadUint8());
        channelLayout_ = static_cast<AudioChannelLayout>(parcel.ReadUint64());
    }

    static VAAudioStreamProperty* Unmarshalling(Parcel& parcel)
    {
        auto streamProperty = new VAAudioStreamProperty();
        if (streamProperty == nullptr) {
            return nullptr;
        }
        streamProperty->UnmarshallingSelf(parcel);
        return streamProperty;
    }
};

static bool MarshallingVAAudioStreamPropertyList(const std::list<VAAudioStreamProperty>& streamProperties,
    Parcel& parcel)
{
    size_t size = streamProperties.size();
    if (!parcel.WriteUint64(size)) {
        return false;
    }

    for (const auto& streamProperty : streamProperties) {
        bool isMarshSuccess = streamProperty.Marshalling(parcel);
        if (!isMarshSuccess) {
            return false;
        }
    }
    return true;
}

static void UnmarshallingVAAudioStreamPropertyList(Parcel& parcel, std::list<VAAudioStreamProperty>& streamProperties)
{
    size_t size = parcel.ReadUint64();
    // due to security concerns,sizelimit has been imposed
    CheckVADeviceInfoSize(size);

    for (size_t i = 0; i < size; i++) {
        VAAudioStreamProperty streamProperty;
        streamProperty.UnmarshallingSelf(parcel);
        streamProperties.push_back(streamProperty);
    }
}

struct VADeviceConfiguration :public Parcelable {
    std::string name_;
    std::string address_;
    VADeviceRole role_;
    VADeviceType type_;
    std::list<VAAudioStreamProperty> properties_;

    VADeviceConfiguration() = default;
    VADeviceConfiguration(const std::string &name, const std::string &address, VADeviceRole role, VADeviceType type)
        : name_(name), address_(address), role_(role), type_(type)
    {}

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(name_) && parcel.WriteString(address_) &&
               parcel.WriteInt32(static_cast<int32_t>(role_)) && parcel.WriteInt32(static_cast<int32_t>(type_)) &&
               MarshallingVAAudioStreamPropertyList(properties_, parcel);
    }
    void UnmarshallingSelf(Parcel &parcel)
    {
        name_ = parcel.ReadString();
        address_ = parcel.ReadString();
        role_ = static_cast<VADeviceRole>(parcel.ReadInt32());
        type_ = static_cast<VADeviceType>(parcel.ReadInt32());
        UnmarshallingVAAudioStreamPropertyList(parcel, properties_);
    }
    static VADeviceConfiguration *Unmarshalling(Parcel &parcel)
    {
        auto deviceConfig = new VADeviceConfiguration();
        if (deviceConfig == nullptr) {
            return nullptr;
        }
        deviceConfig->UnmarshallingSelf(parcel);
        return deviceConfig;
    }
};

struct VAInputStreamAttribute : public Parcelable {
    SourceType type;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(type));
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        type = static_cast<SourceType>(parcel.ReadInt32());
    }

    static VAInputStreamAttribute *Unmarshalling(Parcel &parcel)
    {
        auto streamAttribute = new VAInputStreamAttribute();
        if (streamAttribute == nullptr) {
            return nullptr;
        }
        streamAttribute->UnmarshallingSelf(parcel);
        return streamAttribute;
    }
};

struct VASharedMemInfo : public Parcelable {
    int dataFd_;
    int dataMemCapacity_;
    int statusFd_;
    int statusMemCapacity_;

    VASharedMemInfo() = default;

    bool Marshalling(Parcel &parcel) const override
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        return msgParcel.WriteFileDescriptor(dataFd_) &&
               msgParcel.WriteInt32(dataMemCapacity_) &&
               msgParcel.WriteFileDescriptor(statusFd_) &&
               msgParcel.WriteInt32(statusMemCapacity_);
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        dataFd_ = msgParcel.ReadFileDescriptor();
        dataMemCapacity_ = msgParcel.ReadInt32();
        statusFd_ = msgParcel.ReadFileDescriptor();
        statusMemCapacity_ = msgParcel.ReadInt32();
    }

    static VASharedMemInfo *Unmarshalling(Parcel &parcel)
    {
        auto memInfo = new VASharedMemInfo();
        if (memInfo == nullptr) {
            return nullptr;
        }
        memInfo->UnmarshallingSelf(parcel);
        return memInfo;
    }
};

} //namespace AudioStandard
} //namespace OHOS
#endif //VA_DEVICE_INFO_H