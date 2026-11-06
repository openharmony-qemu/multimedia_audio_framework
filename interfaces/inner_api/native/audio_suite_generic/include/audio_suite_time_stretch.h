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

#ifndef AUDIO_SUITE_TIME_STRETCH_H
#define AUDIO_SUITE_TIME_STRETCH_H

#include <cstdint>
#include <memory>
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

// AudioSuiteTimeStretch abstract class (single input, single output)
// Provides time-stretch (tempo) and pitch-shift processing for PCM audio data.
class AudioSuiteTimeStretch {
public:
    virtual ~AudioSuiteTimeStretch() = default;

    /**
     * @brief Create a time-stretch processor instance
     * @param sampleRate Sample rate in Hz, range [8000, 384000]
     * @param channels Channel count, range [1, 16]
     * @param format PCM sample format, supported: SAMPLE_U8, SAMPLE_S16LE, SAMPLE_S24LE,
     *               SAMPLE_S32LE, SAMPLE_F32LE
     * @return Returns shared_ptr on success, nullptr on invalid parameter or out-of-memory
     */
    static std::shared_ptr<AudioSuiteTimeStretch> Create(
        uint32_t sampleRate, uint32_t channels, AudioSampleFormat format);

    /**
     * @brief Check whether the time-stretch algorithm is supported on this device
     * @return Returns true if the algorithm library file exists, false otherwise.
     *         This call only checks the file existence; it does not verify loadability
     *         or create any processor instance. Use this before Create() to decide
     *         whether to use time-stretch or fall back to an alternative path.
     */
    static bool IsTimeStretchSupported();

    /**
     * @brief Set playback speed (tempo multiplier)
     * @param speed Speed value, range [0.125, 8.0]. speed=2.0 means 2x fast (output duration
     *              ~= input duration / speed). Takes effect from the next Process call;
     *              buffered pending output keeps the old speed.
     * @return Returns 0 on success, error code on failure
     */
    virtual int32_t SetSpeed(float speed) = 0;

    /**
     * @brief Get the currently effective speed
     * @return Returns the speed value last accepted by SetSpeed (default 1.0)
     */
    virtual float GetSpeed() const = 0;

    /**
     * @brief Set playback pitch multiplier
     * @param pitch Pitch value, range [0.25, 4.0]. pitch=2.0
     * @return Returns 0 on success, error code on failure
     */
    virtual int32_t SetPitch(float pitch) = 0;

    /**
     * @brief Get the currently effective pitch multiplier
     * @return Returns the pitch value last accepted by SetPitch (default 1.0)
     */
    virtual float GetPitch() const = 0;

    /**
     * @brief Get algorithmic delay frames
     * @return Returns the number of frames the algorithm buffers before first output.
     *         Returns 0 if the processor is not initialized.
     */
    virtual int32_t GetDelayFrames() const = 0;

    /**
     * @brief Process audio data (single input, single output)
     * @param inputData Input PCM data buffer (interleaved, frame-aligned)
     * @param inputSize Input data size in bytes (must be a multiple of bytesPerFrame)
     * @param outputData Output data buffer
     * @param outputCapacity Output buffer capacity in bytes, must be at least bytesPerFrame
     *                       (= channels * bytesPerSample). Recommended to be as large as
     *                       possible, up to 8x inputSize, because the slowest speed is 0.125
     *                       and the maximum output is 8x the input length. If outputCapacity
     *                       is too small, the unreturned data is buffered in the algorithm
     *                       and will be output in the next Process or Flush call.
     * @param outSize Actual output size in bytes written to outputData
     * @return Returns 0 on success, error code on failure
     */
    virtual int32_t Process(const uint8_t *inputData, int32_t inputSize,
                            uint8_t *outputData, int32_t outputCapacity, int32_t &outSize) = 0;

    /**
     * @brief Flush residual frames at end of stream
     * @param outputData Output data buffer for flushed frames
     * @param outputCapacity Output buffer capacity in bytes, must be at least bytesPerFrame
     * @param outSize Actual output size in bytes written to outputData
     * @param residualFrames Remaining frames still buffered in the algorithm after this call.
     *                       If non-zero, call Flush again to drain them.
     * @return Returns 0 on success, error code on failure
     */
    virtual int32_t Flush(uint8_t *outputData, int32_t outputCapacity, int32_t &outSize,
                          int32_t &residualFrames) = 0;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SUITE_TIME_STRETCH_H
