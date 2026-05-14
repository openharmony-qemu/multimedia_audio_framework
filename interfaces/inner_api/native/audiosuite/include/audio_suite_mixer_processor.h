/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SUITE_MIXER_PROCESSOR_H
#define AUDIO_SUITE_MIXER_PROCESSOR_H

#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// Audio mix stream data structure
struct AudioMixStream {
    void *data = nullptr;    // Audio data pointer
    uint32_t dataSize = 0;   // Audio data size (bytes)
    float volume = 1.0f;     // Stream volume (default 1.0)
};

// MixerProcessor abstract class (multiple inputs, single output)
class MixerProcessor {
public:
    virtual ~MixerProcessor() = default;

    /**
     * @brief Mix processing interface (multiple inputs, single output)
     * @param streams Input audio stream array
     * @param outData Output data buffer
     * @param outCapacity Output buffer capacity
     * @param outSize Actual output size
     * @return Returns 0 on success, error code on failure
     */
    virtual int32_t Process(const std::vector<AudioMixStream> &streams,
                            void *outData, uint32_t outCapacity, uint32_t *outSize) = 0;

    /**
     * @brief Destroy processor
     */
    virtual void Destroy() = 0;
};

// MixerProcessor manager class
class MixerProcessorManager {
public:
    /**
     * @brief Create mix processor
     * @param format Input and output audio format
     * @return Returns unique_ptr on success, nullptr on failure
     * @param enableLimiter Enable limiter (default true)
     */
    static std::unique_ptr<MixerProcessor> Create(const AudioFormat &format, bool enableLimiter = true);
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_MIXER_PROCESSOR_H