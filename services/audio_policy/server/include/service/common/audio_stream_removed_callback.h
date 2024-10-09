/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_STREAM_REMOVED_CALLBACK_H
#define ST_AUDIO_STREAM_REMOVED_CALLBACK_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioStreamRemovedCallback {
public:
    AudioStreamRemovedCallback() = default;
    virtual ~AudioStreamRemovedCallback() = default;

    virtual void OnAudioStreamRemoved(const uint64_t sessionID) = 0;
};
} // namespce AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_STREAM_REMOVED_CALLBACK_H
