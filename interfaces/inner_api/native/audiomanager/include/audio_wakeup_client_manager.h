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

#ifndef AUDIO_WAKEUP_CLIENT_MANAGER_H
#define AUDIO_WAKEUP_CLIENT_MANAGER_H

#include "audio_stream_types.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Lightweight manager for wakeup callbacks. Currently only keeps and dispatches
 * wakeup source and close notifications.
 */
class AudioWakeupClientManager {
public:
    static AudioWakeupClientManager &GetInstance();

    int32_t SetAudioCapturerSourceCallback(const std::shared_ptr<AudioCapturerSourceCallback> &callback);

    int32_t SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback);

    void OnCapturerState(bool isActive);
    void OnWakeupClose();
private:
    AudioWakeupClientManager() = default;
    ~AudioWakeupClientManager() = default;
    int32_t RegisterWakeupSourceCallback();

    std::shared_ptr<AudioCapturerSourceCallback> audioCapturerSourceCallback_ = nullptr;
    std::shared_ptr<WakeUpSourceCloseCallback> audioWakeUpSourceCloseCallback_ = nullptr;
    std::shared_ptr<WakeUpSourceCallback> remoteWakeUpCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_WAKEUP_CLIENT_MANAGER_H
