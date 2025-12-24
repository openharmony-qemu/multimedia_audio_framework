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

#ifndef I_AUDIO_SOURCE_CALLBACK_H
#define I_AUDIO_SOURCE_CALLBACK_H

#include <string>
#include "audio_info.h"
#include "audio_errors.h"
#include "audio_engine_callback_types.h"

namespace OHOS {
namespace AudioStandard {
class IAudioSourceCallback {
public:
    virtual ~IAudioSourceCallback() = default;
    virtual void OnCaptureSourceParamChange(const std::string &networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) {}
    virtual void OnCaptureState(bool isActive) {}
    virtual void OnWakeupClose(void) {}
    virtual void OnInputPipeChange(AudioPipeChangeType changeType,
        std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo) {};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_SOURCE_CALLBACK_H
