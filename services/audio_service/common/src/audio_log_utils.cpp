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
#ifndef LOG_TAG
#define LOG_TAG "AudioLogUtils"
#endif

#include "audio_log_utils.h"
#include <cinttypes>
#include "audio_capturer_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const uint32_t MEDIUM_FREQ_PRINT_LOG = 100;
static const uint32_t LOW_FREQ_PRINT_LOG = 1000;
}
void AudioLogUtils::ProcessVolumeData(const std::string &logTag, const ChannelVolumes &vols, int64_t &count)
{
    bool isDataSilent = true;
    for (int32_t i = 0; i < vols.channel; i++) {
        if (vols.volStart[i] != 0) {
            isDataSilent = false;
            break;
        }
    }
    if (isDataSilent) {
        if (count > 0) {
            AUDIO_INFO_LOG("[%{public}s] not slient %{public}" PRId64 "frames change to slient",
                logTag.c_str(), count);
            count = 0;
        }
        count--;
        IncSilentData(logTag, vols, -count);
    } else {
        if (count < 0) {
            AUDIO_INFO_LOG("[%{public}s] slient %{public}" PRId64 "frames change to not slient",
                logTag.c_str(), -count);
            count = 0;
        }
        count++;
        IncSoundData(logTag, vols, count);
    }
}

void AudioLogUtils::IncSilentData(const std::string &logTag, const ChannelVolumes &vols, int64_t count)
{
    if ((count < LOW_FREQ_PRINT_LOG && count % MEDIUM_FREQ_PRINT_LOG == 0) ||
        (count >= LOW_FREQ_PRINT_LOG && count % LOW_FREQ_PRINT_LOG == 0)) {
        AUDIO_INFO_LOG("[%{public}s], channel: %{public}d, counts: %{public}" PRId64 "",
            logTag.c_str(), vols.channel, count);
    }
}

void AudioLogUtils::IncSoundData(const std::string &logTag, const ChannelVolumes &vols, int64_t count)
{
    if ((count < LOW_FREQ_PRINT_LOG && count % MEDIUM_FREQ_PRINT_LOG == 0) ||
        (count >= LOW_FREQ_PRINT_LOG && count % LOW_FREQ_PRINT_LOG == 0)) {
        if (vols.channel == MONO) {
            AUDIO_INFO_LOG("[%{public}s] MONO = %{public}d, counts: %{public}" PRId64 "",
                logTag.c_str(), vols.volStart[0], count);
        } else {
            AUDIO_INFO_LOG("[%{public}s] channel: %{public}d, L=%{public}d, R=%{public}d, counts: %{public}" PRId64 "",
                logTag.c_str(), vols.channel, vols.volStart[0], vols.volStart[1], count);
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
