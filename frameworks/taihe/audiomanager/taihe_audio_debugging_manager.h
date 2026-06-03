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
#ifndef TAIHE_AUDIO_DEBUGGING_MANAGER_H
#define TAIHE_AUDIO_DEBUGGING_MANAGER_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_debug_manager.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

inline const std::string AUDIO_DEBUGGING_MGR_TAIHE_CLASS_NAME = "AudioDebuggingManager";

class AudioDebuggingManagerImpl {
public:
    AudioDebuggingManagerImpl();
    explicit AudioDebuggingManagerImpl(OHOS::AudioStandard::AudioDebugManager *audioDebugMngr);
    ~AudioDebuggingManagerImpl();

    static AudioDebuggingManager CreateDebuggingManagerWrapper();

    void PrintAppInfo(int32_t fd);
    void PrintRendererInfo(weak::AudioRenderer renderer, int32_t fd);
    void PrintCapturerInfo(weak::AudioCapturer capturer, int32_t fd);
    void PrintLoopbackInfo(weak::AudioLpback loopback, int32_t fd);
    void PrintSessionInfo(weak::AudioSessionManager session, int32_t fd);

private:
    // Non-owning pointer to singleton AudioDebugManager, do not delete
    OHOS::AudioStandard::AudioDebugManager *audioDebugMngr_;
};
}  // namespace ANI::Audio
#endif // TAIHE_AUDIO_DEBUGGING_MANAGER_H
