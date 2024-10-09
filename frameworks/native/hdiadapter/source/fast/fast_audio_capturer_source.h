/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") override;
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

#ifndef FAST_AUDIO_CAPTURER_SOURCE_H
#define FAST_AUDIO_CAPTURER_SOURCE_H

#include <map>

#include "audio_info.h"
#include "i_audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {
class FastAudioCapturerSource : public IMmapAudioCapturerSource {
public:
    static FastAudioCapturerSource *GetInstance();
    static FastAudioCapturerSource *GetVoipInstance();
    FastAudioCapturerSource() = default;
    virtual ~FastAudioCapturerSource() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_SOURCE_H
