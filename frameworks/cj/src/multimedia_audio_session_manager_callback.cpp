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

#include "multimedia_audio_session_manager_callback.h"

#include "audio_policy_log.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
void CjAudioSessionCallback::RegisterFunc(std::function<void(CAudioSessionDeactiveEvent)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    callback_ = cjCallback;
}

void CjAudioSessionCallback::OnAudioSessionDeactive(const AudioSessionDeactiveEvent& deactiveEvent)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (callback_ == nullptr) {
        return;
    }
    CAudioSessionDeactiveEvent event {};
    event.deactiveReason = static_cast<int32_t>(deactiveEvent.deactiveReason);
    callback_(event);
}
} // namespace AudioStandard
} // namespace OHOS