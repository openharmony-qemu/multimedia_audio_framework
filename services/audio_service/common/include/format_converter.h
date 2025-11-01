/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include "stdint.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
struct FormatKey {
    AudioChannel srcChn;
    AudioSampleFormat srcFormat;

    AudioChannel dstChn;
    AudioSampleFormat dstFormat;

    bool operator==(const FormatKey& other) const
    {
        return srcChn == other.srcChn &&
               srcFormat == other.srcFormat &&
               dstChn == other.dstChn &&
               dstFormat == other.dstFormat;
    }
};

struct FormatKeyHash {
    size_t operator()(const FormatKey& key) const
    {
        constexpr size_t hashOffset = 0x9e3779b9;
        auto updateHash = [](size_t seed, size_t hashVal) -> size_t {
            return seed ^ (hashVal + hashOffset + (seed << 6) + (seed >> 2));
        };

        size_t seed = std::hash<int32_t>{}(static_cast<int32_t>(key.srcChn));
        seed = updateHash(seed, std::hash<int32_t>{}(static_cast<int32_t>(key.srcFormat)));
        seed = updateHash(seed, std::hash<int32_t>{}(static_cast<int32_t>(key.dstChn)));
        seed = updateHash(seed, std::hash<int32_t>{}(static_cast<int32_t>(key.dstFormat)));
        return seed;
    }
};

using FormatHandler = std::function<int32_t(const BufferDesc&, const BufferDesc&, bool&)>;
using FormatHandlerMap = std::unordered_map<FormatKey, FormatHandler, FormatKeyHash>;

class FormatConverter {
public:
    static bool DataAccumulationFromVolume(const std::vector<AudioStreamData> &srcDataList,
        const AudioStreamData &dstData);

    static bool AutoConvertToS16Stereo(const AudioStreamData &srcData, const BufferDesc &dstData);
    static bool AutoConvertToS16S32Stereo(const AudioSampleFormat format,
        const AudioStreamData &srcData, const AudioStreamData &dstData);
        
    static void InitFormatHandlers();
    static FormatHandlerMap &GetFormatHandlers();
    static int32_t S16MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S32MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S32StereoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S16StereoToS16Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S16MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S16StereoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S32MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32StereoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32StereoToF32Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32StereoToS16Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S16StereoToF32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S16StereoToF32Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t F32StereoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc);
    static int32_t S32MonoToS16Mono(std::vector<char> &audioBuffer, std::vector<char> &audioBufferConverted);
    static int32_t S32StereoToS16Stereo(std::vector<char> &audioBuffer, std::vector<char> &audioBufferConverted);
private:
    static void InitS16ToFormatHandlers(FormatHandlerMap& handlers);
    static void InitF32ToFormatHandlers(FormatHandlerMap& handlers);

    static bool ChannelFormatS16Convert(const AudioStreamData &srcData, const AudioStreamData &dstData);
    static bool ChannelFormatS32Convert(const AudioStreamData &srcData, const AudioStreamData &dstData);
private:
    static FormatHandlerMap formatHandlers;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // FORMAT_CONVERTER_H
