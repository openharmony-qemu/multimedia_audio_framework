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

#ifndef AUDIO_ENGINE_CALLBACK_TYPES_H
#define AUDIO_ENGINE_CALLBACK_TYPES_H

#include <cinttypes>

#include "audio_output_pipe_types.h"
#include "audio_input_pipe_types.h"

namespace OHOS {
namespace AudioStandard {

enum AudioPipeChangeType : uint32_t {
    PIPE_CHANGE_TYPE_PIPE_STATUS = 0,
    PIPE_CHANGE_TYPE_PIPE_STREAM,
    PIPE_CHANGE_TYPE_PIPE_DEVICE,
};

class AudioOutputPipeCallback {
public:
    virtual ~AudioOutputPipeCallback() = default;
    /**
     * Event for pipe state change, including pipe state, stream state and audio device change.
     */
    virtual void OnOutputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo) = 0;
};

class AudioInputPipeCallback {
public:
    virtual ~AudioInputPipeCallback() = default;
    /**
     * Event for pipe state change, including pipe state, stream state and audio device change.
     */
    virtual void OnInputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo) = 0;
};

enum AudioEngineCallbackId : uint32_t {
    CALLBACK_OUTPUT_PIPE_CHANGE = 0,
    CALLBACK_INPUT_PIPE_CHANGE,
    CALLBACK_ID_MAX,
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENGINE_CALLBACK_TYPES_H