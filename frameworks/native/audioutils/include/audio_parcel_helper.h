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
    static bool MarShalling(Parcelable &parcel, const T &t);

    static T UnMarShalling(Parcelable &parcel);
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, int32_t> {
public:
    static bool MarShalling(Parcelable &parcel, const int32_t &t)
    {
        return parcel.WriteInt32(t);
    }

    static int32_t UnMarShalling(Parcelable &parcel)
    {
        return parcel.ReadInt32();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, uint32_t> {
public:
    static bool MarShalling(Parcelable &parcel, const uint32_t &t)
    {
        return parcel.WriteUint32(t);
    }

    static uint32_t UnMarShalling(Parcelable &parcel)
    {
        return parcel.ReadUInt32();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, int64_t> {
public:
    static bool MarShalling(Parcelable &parcel, const int64_t &t)
    {
        return parcel.WriteInt64(t);
    }

    static int64_t UnMarShalling(Parcelable &parcel)
    {
        return parcel.ReadInt64();
    }
};

template<typename Parcelable>
class AudioParcelHelper<Parcelable, uint64_t> {
public:
    static bool MarShalling(Parcelable &parcel, const uint64_t &t)
    {
        return parcel.WriteUint64(t);
    }

    static uint64_t UnMarShalling(Parcelable &parcel)
    {
        return parcel.ReadUInt64();
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PARCEL_HELPER_H
