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

#ifndef TAIHE_ASR_PROCESSING_CONTROLLER_H
#define TAIHE_ASR_PROCESSING_CONTROLLER_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string TAIHE_ASR_PROCESSING_CONTROLLER_CLASS_NAME = "AsrProcessingController";

class AsrProcessingControllerImpl {
public:
    AsrProcessingControllerImpl();
    explicit AsrProcessingControllerImpl(std::shared_ptr<AsrProcessingControllerImpl> obj);
    ~AsrProcessingControllerImpl() = default;

    bool SetAsrAecMode(::ohos::multimedia::audio::AsrAecMode mode);
    ::ohos::multimedia::audio::AsrAecMode GetAsrAecMode();
    bool SetAsrNoiseSuppressionMode(::ohos::multimedia::audio::AsrNoiseSuppressionMode mode);
    ::ohos::multimedia::audio::AsrNoiseSuppressionMode GetAsrNoiseSuppressionMode();
    bool SetAsrWhisperDetectionMode(::ohos::multimedia::audio::AsrWhisperDetectionMode mode);
    ::ohos::multimedia::audio::AsrWhisperDetectionMode GetAsrWhisperDetectionMode();
    bool SetAsrVoiceControlMode(::ohos::multimedia::audio::AsrVoiceControlMode mode, bool enable);
    bool SetAsrVoiceMuteMode(::ohos::multimedia::audio::AsrVoiceMuteMode mode, bool enable);
    bool IsWhispering();

    friend AsrProcessingController CreateAsrProcessingController(weak::AudioCapturer audioCapturer);

private:
    OHOS::AudioStandard::AudioSystemManager *audioMngr_;
};
} // namespace ANI::Audio
#endif // TAIHE_ASR_PROCESSING_CONTROLLER_H
