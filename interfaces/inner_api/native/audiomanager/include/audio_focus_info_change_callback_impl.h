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

#ifndef ST_AUDIO_FOCUS_INFO_CHANGE_CALLBACK_IMPL_H
#define ST_AUDIO_FOCUS_INFO_CHANGE_CALLBACK_IMPL_H

#include <list>
#include <mutex>

#include "audio_interrupt_info.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {

class AudioFocusInfoChangeCallbackImpl : public AudioFocusInfoChangeCallback {
public:
    explicit AudioFocusInfoChangeCallbackImpl();
    virtual ~AudioFocusInfoChangeCallbackImpl();

    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnAudioFocusRequested(const AudioInterrupt &requestFocus) override;
    void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) override;
    void SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);
    void RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);
private:
    std::list<std::weak_ptr<AudioFocusInfoChangeCallback>> callbackList_;
    std::shared_ptr<AudioFocusInfoChangeCallback> cb_;
    std::mutex cbListMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_FOCUS_INFO_CHANGE_CALLBACK_IMPL_H

