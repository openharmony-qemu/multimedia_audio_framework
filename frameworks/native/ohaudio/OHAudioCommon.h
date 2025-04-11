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

#ifndef OH_AUDIO_COMMON_H
#define OH_AUDIO_COMMON_H

#include "audio_stream_info.h"
#include "multimedia/native_audio_channel_layout.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioCommon {
public:
    static AudioChannel ConvertLayoutToChannel(OH_AudioChannelLayout layout);
private:
    OHAudioCommon() = default;
    ~OHAudioCommon() = default;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_COMMON_H
