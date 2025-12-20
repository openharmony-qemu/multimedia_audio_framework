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
#ifndef AUDIO_DEVICE_STREAM_INFO_H
#define AUDIO_DEVICE_STREAM_INFO_H

#include "audio_device_utils.h"

namespace OHOS {
namespace AudioStandard {

struct DeviceStreamInfo {
    AudioEncodingType encoding = AudioEncodingType::ENCODING_PCM;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    std::set<AudioChannelLayout> channelLayout;
    std::set<AudioSamplingRate> samplingRate;

    DeviceStreamInfo(AudioSamplingRate samplingRate_, AudioEncodingType encoding_, AudioSampleFormat format_,
        AudioChannelLayout channelLayout_)
        : encoding(encoding_), format(format_), channelLayout({channelLayout_}), samplingRate({samplingRate_})
    {}
    DeviceStreamInfo(AudioSamplingRate samplingRate_, AudioEncodingType encoding_, AudioSampleFormat format_,
        AudioChannel channels_) : DeviceStreamInfo(samplingRate_, encoding_, format_,
        ConvertAudioChannelToLayout(channels_))
    {}
    DeviceStreamInfo(AudioStreamInfo audioStreamInfo) : DeviceStreamInfo(audioStreamInfo.samplingRate,
        audioStreamInfo.encoding, audioStreamInfo.format, ConvertAudioChannelToLayout(audioStreamInfo.channels))
    {}
    DeviceStreamInfo() = default;

    std::set<AudioChannel> GetChannels() const
    {
        std::set<AudioChannel> channels;
        for (const auto &layout : channelLayout) {
            channels.insert(ConvertLayoutToAudioChannel(layout));
        }
        return channels;
    }

    // warning: force set default channelLayout by channel
    void SetChannels(const std::set<AudioChannel> &channels)
    {
        channelLayout.clear();
        for (const auto &channel : channels) {
            channelLayout.insert(ConvertAudioChannelToLayout(channel));
        }
    }

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(encoding))
            && parcel.WriteInt32(static_cast<int32_t>(format))
            && MarshallingSetInt32(samplingRate, parcel)
            && MarshallingSetInt64(channelLayout, parcel);
    }
    void Unmarshalling(Parcel &parcel)
    {
        encoding = static_cast<AudioEncodingType>(parcel.ReadInt32());
        format = static_cast<AudioSampleFormat>(parcel.ReadInt32());
        samplingRate = UnmarshallingSetInt32<AudioSamplingRate>(parcel, AUDIO_DEVICE_INFO_SIZE_LIMIT);
        channelLayout = UnmarshallingSetInt64<AudioChannelLayout>(parcel, AUDIO_DEVICE_INFO_SIZE_LIMIT);
    }

    static std::string SerializeList(const std::list<DeviceStreamInfo> &streamInfoList)
    {
        std::string res;
        bool isFirst = true;
        for (const auto &streamInfo : streamInfoList) {
            std::string infoStr = streamInfo.Serialize();
            if (!isFirst) {
                res.append("-");
            }
            res.append(infoStr);
            isFirst = false;
        }
        return res;
    }

    static std::list<DeviceStreamInfo> DeserializeList(const std::string &data)
    {
        std::list<DeviceStreamInfo> res;
        std::vector<std::string> strList = SplitStr(data, '-');
        for (const auto &str : strList) {
            DeviceStreamInfo streamInfo;
            streamInfo.Deserialize(str);
            res.push_back(streamInfo);
        }
        return res;
    }

    std::string Serialize() const
    {
        std::stringstream ss;
        ss << std::to_string(encoding) << "," << std::to_string(format) << ",";
        bool isFirst = true;
        for (const auto &item : samplingRate) {
            if (!isFirst) {
                ss << ":";
            }
            ss << std::to_string(item);
            isFirst = false;
        }
        ss << ",";
        isFirst = true;
        for (const auto &item : channelLayout) {
            if (!isFirst) {
                ss << ":";
            }
            ss << std::to_string(item);
            isFirst = false;
        }
        return ss.str();
    }

    void Deserialize(const std::string &data)
    {
        std::vector<std::string> strList = SplitStr(data, ',');
        if (strList.size() != 4) { // 4: member num
            return;
        }
        uint64_t res = 0;
        encoding = StringToNum(strList[0], res) ? static_cast<AudioEncodingType>(res) : encoding; // 0: encoding
        format = StringToNum(strList[1], res) ? static_cast<AudioSampleFormat>(res) : format; // 1: format
        std::vector<std::string> rateList = SplitStr(strList[2], ':'); // 2: sampling rate
        for (const auto &str : rateList) {
            if (StringToNum(str, res)) {
                samplingRate.insert(static_cast<AudioSamplingRate>(res));
            }
        }
        std::vector<std::string> layoutList = SplitStr(strList[3], ':'); // 3: channel layout
        for (const auto &str : layoutList) {
            if (StringToNum(str, res)) {
                channelLayout.insert(static_cast<AudioChannelLayout>(res));
            }
        }
    }

    bool operator==(const DeviceStreamInfo& info) const
    {
        return encoding == info.encoding && format == info.format &&
            channelLayout == info.channelLayout && samplingRate == info.samplingRate;
    }

    bool CheckParams()
    {
        if (samplingRate.size() == 0) {
            return false;
        }
        if (channelLayout.size() == 0) {
            return false;
        }
        return true;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_STREAM_INFO_H