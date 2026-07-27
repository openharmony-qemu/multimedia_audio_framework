/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_RECORD_CONTROLLER_H
#define TAIHE_AUDIO_RECORD_CONTROLLER_H

#include <mutex>
#include <string>
#include "audio_log.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioRecordingManagerImpl {
public:
    AudioRecordingManagerImpl();
    explicit AudioRecordingManagerImpl(std::shared_ptr<AudioRecordingManagerImpl> obj);
    ~AudioRecordingManagerImpl();

    static AudioRecordingManager CreateRecordingManagerWrapper();

    void EnableSystemRecordControllerPromise(bool show, SystemRecordControllerConfig const &config);
    void EnableSystemRecordControllerSync(bool show, SystemRecordControllerConfig const &config);

    void OnSystemRecordControllerEnabledChange(callback_view<void(SystemRecordControllerChangeInfo const&)> callback);
    void OffSystemRecordControllerEnabledChange(
        optional_view<callback<void(SystemRecordControllerChangeInfo const&)>> callback);

private:
    static void RegisterRecordControllerEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioRecordingManagerImpl *recordControllerImpl);
    static void UnregisterRecordControllerEnabledChangeCallback(
        std::shared_ptr<uintptr_t> &callback, AudioRecordingManagerImpl *recordControllerImpl);

    std::mutex mutex_;
    OHOS::AudioStandard::AudioSystemManager *audioMngr_ = nullptr;
    std::shared_ptr<OHOS::AudioStandard::SystemRecordControllerEnabledChangeCallback>
        recordControllerChangeCallbackTaihe_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RECORD_CONTROLLER_H
