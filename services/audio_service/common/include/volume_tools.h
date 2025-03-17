/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef VOLUME_TOOLS_H
#define VOLUME_TOOLS_H
#include "audio_log_utils.h"

namespace OHOS {
namespace AudioStandard {

static inline bool IsVolumeSame(const float& x, const float& y, const float& epsilon)
{
    return (std::abs((x) - (y)) <= std::abs(epsilon));
}

class VolumeTools {
public:
    static double GetVolDb(AudioSampleFormat format, int32_t vol);
    static bool IsVolumeValid(float volFloat); // 0.0 <= volFloat <= 1.0
    static bool IsVolumeValid(int32_t volInt); // 0 <= volInt <= 65536
    static bool IsVolumeValid(ChannelVolumes vols);
    static size_t GetByteSize(AudioSampleFormat format);
    static int32_t GetInt32Vol(float volFloat);
    static ChannelVolumes GetChannelVolumes(AudioChannel channel, int32_t volStart, int32_t volEnd);
    static ChannelVolumes GetChannelVolumes(AudioChannel channel, float volStart, float volEnd);

    // Data size should be rounded to each sample size
    // There will be significant sound quality loss when process uint8_t samples.
    static int32_t Process(const BufferDesc &buffer, AudioSampleFormat format, ChannelVolumes vols);

    // will count volume for each channel, vol sum will be kept in volStart
    static ChannelVolumes CountVolumeLevel(const BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel,
        size_t split = 1);
    static void DfxOperation(BufferDesc &buffer, AudioStreamInfo streamInfo, std::string logTag,
        int64_t &volumeDataCount, size_t split = 1);

    static void CalcMuteFrame(BufferDesc &buffer, AudioStreamInfo streamInfo, std::string logTag,
        int64_t &volumeDataCount, int64_t &muteFrameCnt, size_t split = 1);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // VOLUME_TOOLS_H