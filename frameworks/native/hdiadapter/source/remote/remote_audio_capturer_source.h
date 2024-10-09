/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef REMOTE_AUDIO_CAPTURER_SOURCE_H
#define REMOTE_AUDIO_CAPTURER_SOURCE_H

#include <cstdio>
#include <list>
#include <map>

#include "audio_info.h"
#include "i_audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {
class RemoteAudioCapturerSource : public IAudioCapturerSource {
public:
    static RemoteAudioCapturerSource *GetInstance(const std::string &deviceNetworkId);
    static void GetAllInstance(std::vector<IAudioCapturerSource *> &allInstance);

    RemoteAudioCapturerSource() = default;
    ~RemoteAudioCapturerSource() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_SOURCE_H
