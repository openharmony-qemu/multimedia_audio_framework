/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CAPTURER_SOURCE_H
#define AUDIO_CAPTURER_SOURCE_H

#include <cstdio>
#include <list>

#include "i_audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {
#define AUDIO_CHANNELCOUNT 2
#define AUDIO_SAMPLE_RATE_48K 48000
#define DEEP_BUFFER_CAPTURE_PERIOD_SIZE 4096
#define INT_32_MAX 0x7fffffff
#define PERIOD_SIZE 1024
#define PATH_LEN 256
#define AUDIO_BUFF_SIZE (16 * 1024)
#define PCM_8_BIT 8
#define PCM_16_BIT 16

class AudioCapturerSource : public IAudioCapturerSource {
public:
    static AudioCapturerSource *GetInstance(const std::string &halName = "primary",
        const SourceType sourceType = SourceType::SOURCE_TYPE_MIC,
        const char *sourceName = "Built_in_wakeup");
    static AudioCapturerSource *GetMicInstance(void);
    static AudioCapturerSource *GetWakeupInstance(bool isMirror = false);
    static AudioCapturerSource *Create(CaptureAttr *attr);

protected:
    AudioCapturerSource() = default;
    ~AudioCapturerSource() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_SOURCE_H
