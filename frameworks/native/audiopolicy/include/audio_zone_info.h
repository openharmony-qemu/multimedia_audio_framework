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
#ifndef AUDIO_ZONE_INFO_H
#define AUDIO_ZONE_INFO_H

#include <string>
#include <set>
#include <vector>
#include "refbase.h"
#include "parcel.h"
#include "audio_device_info.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
enum class AudioZoneChangeReason {
    UNKNOWN = 0,
    BIND_NEW_DEVICE = 1,
    BIND_NEW_APP,
    UNBIND_APP,
};

enum class AudioZoneInterruptReason {
    UNKNOWN = 0,
    LOCAL_INTERRUPT = 1,
    REMOTE_INJECT = 2,
    RELEASE_AUDIO_ZONE,
    BIND_APP_TO_ZONE,
    UNBIND_APP_FROM_ZONE,
};

enum class AudioZoneFocusStrategy {
    LOCAL_FOCUS_STRATEGY = 0,
    DISTRIBUTED_FOCUS_STRATEGY = 1,
};

class AudioZoneContext : public Parcelable {
public:
    AudioZoneFocusStrategy focusStrategy_ = AudioZoneFocusStrategy::LOCAL_FOCUS_STRATEGY;

    AudioZoneContext() = default;

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(focusStrategy_));
    }

    static AudioZoneContext *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioZoneContext();
        if (info == nullptr) {
            return nullptr;
        }

        info->focusStrategy_ = static_cast<AudioZoneFocusStrategy>(parcel.ReadInt32());
        return info;
    }
};

class AudioZoneDescriptor : public Parcelable {
public:
    int32_t zoneId_ = -1;
    std::string name_;
    std::set<int32_t> uids_;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices_;

    AudioZoneDescriptor() = default;

    bool Marshalling(Parcel &parcel) const
    {
        bool result = parcel.WriteInt32(zoneId_);
        result &= parcel.WriteString(name_);
        result &= MarshallingSetInt32(uids_, parcel);
        if (!result) {
            return false;
        }

        size_t size = devices_.size();
        if (!parcel.WriteUint64(size)) {
            return false;
        }
        for (const auto &device : devices_) {
            if (!device->Marshalling(parcel)) {
                return false;
            }
        }
        return true;
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        zoneId_ = parcel.ReadInt32();
        name_ = parcel.ReadString();
        uids_ = UnmarshallingSetInt32<int32_t>(parcel);

        size_t size = parcel.ReadUint64();
        if (size > std::numeric_limits<size_t>::max()) {
            size = std::numeric_limits<size_t>::max();
        }

        for (size_t i = 0; i < size; i++) {
            std::shared_ptr<AudioDeviceDescriptor> device(AudioDeviceDescriptor::Unmarshalling(parcel));
            if (device == nullptr) {
                devices_.clear();
                return;
            }
            devices_.emplace_back(device);
        }
    }

    static AudioZoneDescriptor *Unmarshalling(Parcel &parcel)
    {
        auto desc = new(std::nothrow) AudioZoneDescriptor();
        if (desc == nullptr) {
            return nullptr;
        }

        desc->UnmarshallingSelf(parcel);
        return desc;
    }
};

struct AudioZoneStream : public Parcelable {
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    bool isPlay = true;
    bool operator==(const AudioZoneStream &value) const
    {
        return streamUsage == value.streamUsage && sourceType == value.sourceType && isPlay == value.isPlay;
    }

    bool operator<(const AudioZoneStream &value) const
    {
        return streamUsage < value.streamUsage || (streamUsage == value.streamUsage && sourceType < value.sourceType);
    }

    bool operator>(const AudioZoneStream &value) const
    {
        return streamUsage > value.streamUsage || (streamUsage == value.streamUsage && sourceType > value.sourceType);
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(static_cast<int32_t>(streamUsage))
            && parcel.WriteInt32(static_cast<int32_t>(sourceType))
            && parcel.WriteBool(isPlay);
    }

    static AudioZoneStream *Unmarshalling(Parcel &parcel)
    {
        auto stream = new(std::nothrow) AudioZoneStream();
        if (stream == nullptr) {
            return nullptr;
        }

        stream->streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        stream->sourceType = static_cast<SourceType>(parcel.ReadInt32());
        stream->isPlay = parcel.ReadBool();
        return stream;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ZONE_INFO_H