/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_SESSION_MANAGER_CALLBACK_H
#define MULTIMEDIA_AUDIO_SESSION_MANAGER_CALLBACK_H
#include "audio_policy_interface.h"
#include "audio_session_manager.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjAudioSessionCallback : public AudioSessionCallback {
public:
    CjAudioSessionCallback() = default;
    virtual ~CjAudioSessionCallback() = default;
    void RegisterFunc(std::function<void(CAudioSessionDeactiveEvent)> cjCallback);
    void OnAudioSessionDeactive(const AudioSessionDeactiveEvent& deactiveEvent) override;

private:
    std::function<void(CAudioSessionDeactiveEvent)> callback_ {};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_SESSION_MANAGER_CALLBACK_H