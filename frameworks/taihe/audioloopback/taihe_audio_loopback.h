/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_LOOPBACK_H
#define TAIHE_AUDIO_LOOPBACK_H
#include <iostream>
#include <map>
#include <queue>

#include "audio_errors.h"
#include "audio_loopback.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class AudioLoopbackImpl {
public:
    AudioLoopbackImpl();
    explicit AudioLoopbackImpl(std::shared_ptr<AudioLoopbackImpl> obj);
    ~AudioLoopbackImpl();

    static AudioLpback CreateAudioLoopbackWrapper(OHOS::AudioStandard::AudioLoopbackMode loopbackMode);
    AudioLoopbackStatus GetStatusSync();
    void SetVolumeSync(double volume);
    bool EnableSync(bool enable);
    void OnStatusChange(callback_view<void(AudioLoopbackStatus data)> callback);
    void OffStatusChange(optional_view<callback<void(AudioLoopbackStatus data)>> callback);
    std::shared_ptr<OHOS::AudioStandard::AudioLoopback> loopback_;

    friend AudioLpback CreateAudioLoopbackSync(AudioLoopbackMode mode);

private:
    static void RegisterLoopbackCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioLoopbackImpl *taiheLoopback);
    static void UnregisterLoopbackCallback(std::shared_ptr<uintptr_t> &callback,
        const std::string &cbName, AudioLoopbackImpl *taiheLoopback);
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
    static OHOS::AudioStandard::AudioLoopbackMode sLoopbackMode_;
    std::shared_ptr<OHOS::AudioStandard::AudioLoopbackCallback> callbackTaihe_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_LOOPBACK_H