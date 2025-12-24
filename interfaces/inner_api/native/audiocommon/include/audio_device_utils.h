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

#ifndef AUDIO_DEVICE_UTILS_H
#define AUDIO_DEVICE_UTILS_H

#include <parcel.h>
#include <set>
#include <limits>
#include <unordered_set>
#include <sstream>
#include <list>
#include <vector>
#include "securec.h"
#include <audio_stream_info.h>

namespace OHOS {
namespace AudioStandard {
constexpr size_t AUDIO_DEVICE_INFO_SIZE_LIMIT = 30;
constexpr uint32_t BASE_DEC = 10;

template<typename T> bool MarshallingSetInt32(const std::set<T> &value, Parcel &parcel)
{
    size_t size = value.size();
    if (!parcel.WriteUint64(size)) {
        return false;
    }
    for (const auto &i : value) {
        if (!parcel.WriteInt32(i)) {
            return false;
        }
    }
    return true;
}

template<typename T> std::set<T> UnmarshallingSetInt32(Parcel &parcel,
    const size_t maxSize = std::numeric_limits<size_t>::max())
{
    size_t size = parcel.ReadUint64();
    // due to security concerns, sizelimit has been imposed
    if (size > maxSize) {
        size = maxSize;
    }

    std::set<T> res;
    for (size_t i = 0; i < size; i++) {
        res.insert(static_cast<T>(parcel.ReadInt32()));
    }
    return res;
}

template<typename T> bool MarshallingSetInt64(const std::set<T> &value, Parcel &parcel)
{
    size_t size = value.size();
    if (!parcel.WriteUint64(size)) {
        return false;
    }
    for (const auto &i : value) {
        if (!parcel.WriteInt64(i)) {
            return false;
        }
    }
    return true;
}

template<typename T> std::set<T> UnmarshallingSetInt64(Parcel &parcel,
    const size_t maxSize = std::numeric_limits<size_t>::max())
{
    size_t size = parcel.ReadUint64();
    // due to security concerns, sizelimit has been imposed
    if (size > maxSize) {
        size = maxSize;
    }

    std::set<T> res;
    for (size_t i = 0; i < size; i++) {
        res.insert(static_cast<T>(parcel.ReadInt64()));
    }
    return res;
}

static AudioChannel ConvertLayoutToAudioChannel(AudioChannelLayout layout)
{
    AudioChannel channel = AudioChannel::CHANNEL_UNKNOW;
    switch (layout) {
        case AudioChannelLayout::CH_LAYOUT_MONO:
            channel = AudioChannel::MONO;
            break;
        case AudioChannelLayout::CH_LAYOUT_STEREO:
            channel = AudioChannel::STEREO;
            break;
        case AudioChannelLayout::CH_LAYOUT_2POINT1:
        case AudioChannelLayout::CH_LAYOUT_3POINT0:
            channel = AudioChannel::CHANNEL_3;
            break;
        case AudioChannelLayout::CH_LAYOUT_3POINT1:
        case AudioChannelLayout::CH_LAYOUT_4POINT0:
        case AudioChannelLayout::CH_LAYOUT_QUAD:
            channel = AudioChannel::CHANNEL_4;
            break;
        case AudioChannelLayout::CH_LAYOUT_5POINT0:
        case AudioChannelLayout::CH_LAYOUT_2POINT1POINT2:
            channel = AudioChannel::CHANNEL_5;
            break;
        case AudioChannelLayout::CH_LAYOUT_5POINT1:
        case AudioChannelLayout::CH_LAYOUT_HEXAGONAL:
        case AudioChannelLayout::CH_LAYOUT_3POINT1POINT2:
            channel = AudioChannel::CHANNEL_6;
            break;
        case AudioChannelLayout::CH_LAYOUT_7POINT0:
            channel = AudioChannel::CHANNEL_7;
            break;
        case AudioChannelLayout::CH_LAYOUT_7POINT1:
            channel = AudioChannel::CHANNEL_8;
            break;
        case AudioChannelLayout::CH_LAYOUT_7POINT1POINT2:
            channel = AudioChannel::CHANNEL_10;
            break;
        case AudioChannelLayout::CH_LAYOUT_7POINT1POINT4:
            channel = AudioChannel::CHANNEL_12;
            break;
        default:
            channel = AudioChannel::CHANNEL_UNKNOW;
            break;
    }
    return channel;
}

static AudioChannelLayout ConvertAudioChannelToLayout(AudioChannel channel)
{
    AudioChannelLayout channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;

    switch (channel) {
        case AudioChannel::MONO:
            channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
            break;
        case AudioChannel::STEREO:
            channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
            break;
        case AudioChannel::CHANNEL_3:
            channelLayout = AudioChannelLayout::CH_LAYOUT_2POINT1;
            break;
        case AudioChannel::CHANNEL_4:
            channelLayout = AudioChannelLayout::CH_LAYOUT_3POINT1;
            break;
        case AudioChannel::CHANNEL_5:
            channelLayout = AudioChannelLayout::CH_LAYOUT_2POINT1POINT2;
            break;
        case AudioChannel::CHANNEL_6:
            channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1;
            break;
        case AudioChannel::CHANNEL_7:
            channelLayout = AudioChannelLayout::CH_LAYOUT_7POINT0;
            break;
        case AudioChannel::CHANNEL_8:
            channelLayout = AudioChannelLayout::CH_LAYOUT_7POINT1;
            break;
        case AudioChannel::CHANNEL_10:
            channelLayout = AudioChannelLayout::CH_LAYOUT_7POINT1POINT2;
            break;
        case AudioChannel::CHANNEL_12:
            channelLayout = AudioChannelLayout::CH_LAYOUT_7POINT1POINT4;
            break;
        default:
            channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;
            break;
    }

    return channelLayout;
}

static std::vector<std::string> SplitStr(const std::string &str, const char delimiter)
{
    std::vector<std::string> res;
    std::istringstream iss(str);
    std::string item;
    while (getline(iss, item, delimiter)) {
        res.push_back(item);
    }
    return res;
}

static bool StringToNum(const std::string &str, uint64_t &num)
{
    char *endPtr = nullptr;
    num = std::strtoull(str.c_str(), &endPtr, BASE_DEC);
    return endPtr != nullptr && *endPtr == '\0';
}
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_UTILS_H
