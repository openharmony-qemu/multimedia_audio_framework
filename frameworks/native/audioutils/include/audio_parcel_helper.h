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
#ifndef AUDIO_PARCEL_HELPER_H
#define AUDIO_PARCEL_HELPER_H

#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
template<typename Parcelable, typename T>
class AudioParcelHelper {
public:
    static bool Marshalling(Parcelable &parcel, const T &t);

    static T Unmarshalling(Parcelable &parcel);
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, int32_t> {
public:
    static bool Marshalling(Parcelable &parcel, const int32_t &t)
    {
        return parcel.WriteInt32(t);
    }

    static int32_t Unmarshalling(Parcelable &parcel)
    {
        return parcel.ReadInt32();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, uint32_t> {
public:
    static bool Marshalling(Parcelable &parcel, const uint32_t &t)
    {
        return parcel.WriteUint32(t);
    }

    static uint32_t Unmarshalling(Parcelable &parcel)
    {
        return parcel.ReadUint32();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, int64_t> {
public:
    static bool Marshalling(Parcelable &parcel, const int64_t &t)
    {
        return parcel.WriteInt64(t);
    }

    static int64_t Unmarshalling(Parcelable &parcel)
    {
        return parcel.ReadInt64();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, uint64_t> {
public:
    static bool Marshalling(Parcelable &parcel, const uint64_t &t)
    {
        return parcel.WriteUint64(t);
    }

    static uint64_t Unmarshalling(Parcelable &parcel)
    {
        return parcel.ReadUInt64();
    }
};

template<typename Parcelable, typename T>
class AudioParcelHelper<Parcelable, std::optional<T>> {
public:
    static bool Marshalling(Parcelable &parcel, const std::optional<T> &t)
    {
        bool hasValue = t.has_value();
        parcel.WriteBool(hasValue);
        if (!hasValue) {
            return true;
        } else {
            return AudioParcelHelper<Parcelable, T>::Marshalling(parcel, t.value());
        }
    }

    static std::optional<T> Unmarshalling(Parcelable &parcel)
    {
        if (!parcel.ReadBool()) {
            return std::nullopt;
        } else {
            return AudioParcelHelper<Parcelable, T>::Unmarshalling(parcel);
        }
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PARCEL_HELPER_H
