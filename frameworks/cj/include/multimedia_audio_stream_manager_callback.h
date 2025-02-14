/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#ifndef MULTIMEDIA_AUDIO_STREAM_MANAGER_CALLBACK_H
#define MULTIMEDIA_AUDIO_STREAM_MANAGER_CALLBACK_H
#include "audio_stream_manager.h"
#include "audio_policy_interface.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjAudioCapturerStateChangeCallback : public AudioCapturerStateChangeCallback {
public:
    CjAudioCapturerStateChangeCallback() = default;
    virtual ~CjAudioCapturerStateChangeCallback() = default;

    void RegisterFunc(std::function<void(CArrAudioCapturerChangeInfo)> cjCallback);

    void OnCapturerStateChange(
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;

private:
    std::function<void(CArrAudioCapturerChangeInfo)> func_{};
    std::mutex cbMutex_;
};

class CjAudioRendererStateChangeCallback : public AudioRendererStateChangeCallback {
public:
    CjAudioRendererStateChangeCallback() = default;
    virtual ~CjAudioRendererStateChangeCallback() = default;

    void RegisterFunc(std::function<void(CArrAudioRendererChangeInfo)> cjCallback);

    void OnRendererStateChange(
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;

private:
    std::function<void(CArrAudioRendererChangeInfo)> func_{};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_STREAM_MANAGER_CALLBACK_H
